/*
Author: Nate Norris
Project: Linux Programming | Project 3 | Fall '19
*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include "DirectoryReader.h"
#include <sys/wait.h>
#include <algorithm>
#include <sys/stat.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#define WRITELN write(STDOUT_FILENO, "\n", 1)

using namespace std;

string prompt_phrase;
int running = 1;
vector<pid_t> * foreground_pids;
vector<pid_t> * background_pids;
pthread_t * background_thread;

int normalFifo;
int highPriorityFifo;
int bgflag = 0;

void prompt()
{
	write(STDOUT_FILENO, (prompt_phrase.c_str()), prompt_phrase.length());
}

void sendResponse(vector<string> * args) {
	string response_towrite = "";
	for(int i = 1; i < args->size(); i++) {
		response_towrite += args->at(i) + " ";
	}

	int result = open("/tmp/MyShellResponse", O_WRONLY, S_IRWXU);
	if(result == -1) {
		while(result = open("/tmp/MyShellResponse", O_WRONLY, S_IRWXU) && result == -1){}
	}
	int amt_written = write(result, response_towrite.c_str(), response_towrite.length());
	close(result);
}

void printMessage(int fifo)
{
	char * buff[256];
	int amtRead = read(fifo, buff, 256);
	write(STDOUT_FILENO, buff, amtRead);
}

void sigint_handler(int signo)
{
	if(foreground_pids->size() > 0) {
		kill(foreground_pids->at(0), SIGKILL);
	}

	printMessage(highPriorityFifo);
}

void set_sigint_handler()
{
	struct sigaction * sigint_hand = new struct sigaction();
	sigint_hand -> sa_handler = sigint_handler;
	sigaction(SIGINT, sigint_hand, NULL);
}

void create_fifos()
{
	string fifo_names[3] = {"MyShellNormal", "MyShellHighPriority", "MyShellResponse"};
	for(int i = 0; i < 3; i++)
	{
		mkfifo(("/tmp/" + fifo_names[i]).c_str(), S_IRWXU);
	}
}

void * monitor_high_priority(void * args) {
	struct pollfd * fds = new pollfd[1];
	fds[0].fd = open("/tmp/MyShellHighPriority", O_RDONLY);
	fds[0].events = POLLRDNORM | POLLIN;
	highPriorityFifo = fds[0].fd;

	while(running){
		int fifo_val = poll(fds, 1, 1);

		if(fifo_val > 0) {
			raise(SIGINT);
		}
	}

}

void * wait_background_pids_helper(void * args) {
	waitpid(background_pids->at(0), NULL, 0);
	background_pids->clear();
	prompt();
}

void wait_background_pids() {
	pthread_t background_wait;
	pthread_create(&background_wait, NULL, wait_background_pids_helper, NULL);
	pthread_detach(background_wait);
}

void cwd()
{
	char buff[256];
	char * wd = getcwd(buff, 256);
	for(int i = 0; i < sizeof(buff); i++)
	{
		if(*(wd+i) == '\0')
		{
			break;
		}
		write(STDOUT_FILENO, wd + i, 1);
	}
}

string get_cwd_string()
{
	char buff[256];
	char * wd = getcwd(buff, 256);
	string cwd_str = *(new string(wd));
	return cwd_str;
}

char * get_cwd_char()
{
	char buff[256];
	char * wd = getcwd(buff, 256);
	return wd;
}

void update_prompt()
{
	prompt_phrase = get_cwd_string() + ">";
}

string replace_cwd_wildcard(string phrase)
{
	string original = phrase;

	int loc1, loc2;
	loc1 = phrase.find("%");
	phrase = phrase.substr(loc1+1, phrase.length());
	loc2 = phrase.find("%");
	if(loc1 >= 0 && loc2 >= 0)
	{
		string part1 = original.substr(0, loc1);
		string part2 = original.substr(loc2 + loc1+2);
		loc1++;
		string wildcard_contents = original.substr(loc1, loc2);
		string contents = "";
		if(wildcard_contents == "PWD")
		{
			contents = get_cwd_string();
		}
		return part1 + contents + part2;
	}
	return original;
}

int fork_exec(char * file_name, char ** args) {
	pid_t child = fork();
	if(child == 0)
	{

		execv(file_name, args);
		exit(0);
	} else {
		if(bgflag) {
			background_pids->push_back(child);
			wait_background_pids();
		} else {
			foreground_pids->push_back(child);
			waitpid(child, NULL, 0);
			foreground_pids->clear();
		}
		return 0;
	}
}

int dir(vector<string> * args) {

	//Create char * [] of arguments for exec
	char * new_args[(args->size()+1) * sizeof(char *)];
	for(int i = 0; i < args->size(); i++)
	{
		//Must cast out of const char *
		new_args[i] = const_cast<char*>((args->at(i)).c_str());
	}
	new_args[(args->size())] = NULL;
	return fork_exec("dir.o", new_args);

}

int file_exists_in_directory(string cmd, string directory) {
	DirectoryReader * dir = new DirectoryReader(directory);
	dir->getFiles();
	vector<string> cmds = *(dir->sortFiles(0,0,0));
	if(find(cmds.begin(), cmds.end(), cmd) != cmds.end()) {
			return 0;
	}
	return -1;
}

string sys_cmd_searchpath(string cmd) {

	string path = "";
	//Check /bin/
	if(file_exists_in_directory(cmd, string("/bin")) == 0) path = "/bin/";
	//Check /usr/bin/
	if(file_exists_in_directory(cmd, string("/usr/bin")) == 0) path = "/usr/bin/";

	return path;
}

//Executes a command not found in this Terminal. Will execute any matching file in /usr/bin.
int sys_cmd(string cmd, vector<string> * args)
{
	string path = sys_cmd_searchpath(cmd);
	if(path.length() > 0) {
			//Create char * [] of arguments for exec
			char * new_args[(args->size()+1) * sizeof(char *)];
			for(int i = 0; i < args->size(); i++)
			{
				//Must cast out of const char *
				new_args[i] = const_cast<char*>((args->at(i)).c_str());
			}
			new_args[(args->size())] = NULL;
			char * file_name = const_cast<char *>((path+cmd).c_str());
			fork_exec(file_name, new_args);
		}
}

int process_cmd(vector<string> * args)
{
	if(args->size() >= 1) {
		//We know the first argument is always the command name
		string cmd = args->at(0);

		//command: "exit"
		if(cmd == "exit")
		{
			return -1;
		}
		else if(cmd == "cwd")
		{
			cwd();
			WRITELN;
			return 0;
		}
		else if(cmd == "setprompt")
		{
			if(args->size()-1 < 1)
			{
				write(STDOUT_FILENO, "usage: setprompt <prompt string>\n", 33);
			} else {
				string newprompt = args->at(1);
				newprompt = replace_cwd_wildcard(newprompt);
				prompt_phrase = newprompt;
			}
			return 0;
		}
		else if(cmd == "cd")
		{
			if(args->size()-1 < 1)
			{
				write(STDOUT_FILENO, "usage: cd <path>\n", 17);
			} else {
				chdir(args->at(1).c_str());
				update_prompt();
			}
			return 0;
		}
		else if(cmd == "dir" || cmd == "ls")
		{
			return dir(args);
		} else if (cmd == "respond") {
			sendResponse(args);
		} else {
	 		//Unknown command, try to exec it
			return sys_cmd(cmd, args);
		}

	}
}

int * newPipe()
{
	int * p = new int[2];
	return p;
}

//Given full commands and their separators, pipe or redirect.
//This method sucks. Sorry you have to look at this. Not proud of this one.
//Only works for piping OR redirection....not both.
int parse_cmd_with_separators(vector<vector<string> *> * cmds, vector<string> * separators) {
	int stdout_save = dup(STDOUT_FILENO);

	//Handle piping
	if(cmds->size() > 1 && separators->at(0) == "|")
	{
		int p[2];
		pipe(p);

		pid_t child = fork();
		if(child == 0)
		{
			dup2(p[1], STDOUT_FILENO);
			int result = process_cmd(cmds->at(0));
			close(p[0]);
			exit(0);
		} else {
			waitpid(child, NULL, 0);
			close(p[1]); // Close write end of pipe
		}

		child = fork();
		if(child == 0)
		{
			close(p[1]);
			dup2(p[0], STDIN_FILENO);
			dup2(stdout_save, STDOUT_FILENO);
			process_cmd(cmds->at(1));
			exit(0);
		} else {
			waitpid(child, NULL, 0);
			return 0;
		}

		//Handle redirection
	} else if(cmds->size() > 1 && separators->at(0) == ">")
	{
		char * name = const_cast<char*>(cmds->at(1)->at(0).c_str());
		int fd = open(name, O_CREAT|O_WRONLY, S_IRWXU);
		pid_t child = fork();
		if(child== 0)
		{
			dup2(fd, STDOUT_FILENO);
			int result = process_cmd(cmds->at(0));
			exit(0);
		} else {
			waitpid(child, NULL, 0);
			return 0;
		}
	}  else if(cmds->size() > 1 && separators->at(0) == "<")
	{
		char * name = const_cast<char*>(cmds->at(1)->at(0).c_str());
		int fd = open(name, O_RDONLY, S_IRWXU);
		pid_t child = fork();
		if(child == 0)
		{
			dup2(fd, STDIN_FILENO);
			int result = process_cmd(cmds->at(0));
			exit(0);
		} else {
			waitpid(child, NULL, 0);
			return 0;
		}
	}
	else {
		int result = process_cmd(cmds->at(0));
		return result;
	}
	return -1;
}

int parse_cmd(char * cmd_word, int cmd_len)
{
	if(cmd_len == 1){
		return 0;
	}
	//Convert to string and strip new line.
	string cmd = (new string(cmd_word))->substr(0,cmd_len-1);
	vector<string> * args = new vector<string>();

	//Parse cmd + arguments into a vector
	int space;
	while((space = cmd.find(" ")) && space <= cmd.length())
	{
		args->push_back(cmd.substr(0, space));
		cmd = cmd.substr(space+1, cmd.length());
	}
	args->push_back(cmd);

	int begin = 0;
	int end  = 0;

	//Parse separators so we can recognize entire commands
	vector<vector<string> *> * cmds = new vector<vector<string> *>();
	vector<string> * separators = new vector<string>();

	int arg_endpoint = args->size();
	//Check for background thread flag
	if(args->at(args->size()-1) == "&" && args->size() > 1) {
		if(background_pids->size() > 0) {
			return -1;
		} else {
			arg_endpoint -= 1;
			bgflag = 1;
			args->pop_back();
		}
	} else {
		bgflag = 0;
	}

	for(int i = 0; i < args->size(); i++)
	{
		if(args->at(i) == "|" || args->at(i) == ">" || args->at(i) == "<" || i == (args->size()-1))
		{
			if(args->at(i) == "|" || args->at(i) == ">" || args->at(i) == "<") separators->push_back(args->at(i));

			end = i;
			if(i == (args->size() - 1)) end = i+1;

			vector<string> * cmd = new vector<string>();
			for(int j = begin; j < end; j++)
			{
				cmd->push_back(args->at(j));
			}
			cmds->push_back(cmd);
			begin = i+1;
		}
	}

	if(cmds->size() == 0)
	{
		cmds->push_back(args);
	}
	//Seems like we are ready to fork &/or redirect
	return parse_cmd_with_separators(cmds, separators);
}

void run()
{

	struct pollfd * fds = new pollfd[1];
	fds[0].fd = open("/tmp/MyShellNormal", O_NONBLOCK | O_RDONLY);
	fds[0].events = POLLRDNORM | POLLIN;
	normalFifo = fds[0].fd;

	int normalFifoVal;

	while(running)
	{

		prompt();
		char buff[256];
		int cmdlen = read(STDIN_FILENO, buff, 256);
		int result = parse_cmd(buff, cmdlen);
		//Some commands get run
		if(result == -1)
		{
			running = 0;
		}

		normalFifoVal = poll(fds, 1, 1);
		for(int i = 0; i < 1; i++)
		{
			short revents = fds[i].revents;
			if(revents && POLLIN)
			{
				printMessage(fds[0].fd);
			}
		}
	}
}

pthread_t * launch_monitor_thread() {
	pthread_t monitor_thread;
	pthread_create(&monitor_thread, NULL, monitor_high_priority, NULL);
	return &monitor_thread;
}

int main()
{
	foreground_pids = new vector<pid_t>();
	background_pids = new vector<pid_t>();
	set_sigint_handler();
	create_fifos();
	update_prompt();

	pthread_t * monitor_thread = launch_monitor_thread();
	run();

	pthread_join(*monitor_thread, NULL);

	delete(foreground_pids);
	delete(background_pids);
}
