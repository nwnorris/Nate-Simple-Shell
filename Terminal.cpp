#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <map>
#include "Terminal.h"
#include "DirectoryReader.h"
#include <sys/wait.h>
#include <algorithm>

using namespace std;

Terminal::Terminal()
{
	update_prompt();
	run();
}

Terminal::~Terminal(){}

void Terminal::cwd()
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

string Terminal::get_cwd_string()
{
	char buff[256];
	char * wd = getcwd(buff, 256);
	string cwd_str = *(new string(wd));
	return cwd_str;
}

char * Terminal::get_cwd_char()
{
	char buff[256];
	char * wd = getcwd(buff, 256);
	return wd;
}

void Terminal::update_prompt()
{
	prompt_phrase = get_cwd_string() + ">";
}
void Terminal::prompt()
{
	//cwd();
	//write(STDOUT_FILENO, &">", 1);
	write(STDOUT_FILENO, (prompt_phrase.c_str()), prompt_phrase.length());
}

string Terminal::replace_cwd_wildcard(string phrase)
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

void Terminal::dir(string dirName, vector<string> * args)
{
	//Identify and parse flags
	int adate = 0;
	int name = 0;
	int type = 0;
	int recursive = 0;
	map<string, int> * flags = new map<string, int>();

	int i = 0;
	for(string arg: *args)
	{
		string identifier = arg.substr(0,2);
		flags->insert(make_pair(identifier, i));
		i++;
	}

	if(flags->find("-r") != flags->end())
	{
		if(args->at(flags->find("-r")->second).length() <= 2)
		{
			recursive = 1;
		}
	}

	if(flags->find("-s") != flags->end())
	{
		//cout << "Found -s flag" << endl;
		string flag = args->at(flags->find("-s")->second).substr(3);
		if(flag == "adate") adate = 1;
		else if(flag == "name") name = 1;
		else if(flag == "type") type = 1;
	}

	delete(flags);
	//Now that flags are set, execute command
	DirectoryReader * dreader = new DirectoryReader(dirName);
	vector<string> dirs = *(dreader->getFiles()); //Retrieve files
	vector<string> names = *(dreader->sortFiles(adate, name, type)); //Will sort if flag was passed
	delete(dreader);
	//Help with ASCII color codes: https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
	for(int i = 0; i < names.size(); i++)
	{
		string out = "\033[1;36m" +  names.at(i) + "\033[0;m\n";
		write(STDOUT_FILENO, out.c_str(), out.length());
	}
	write(STDOUT_FILENO, "\n", 1);
	if(recursive && dirs.size() > 0)
	{
		for(string subdir : dirs)
		{
			write(STDOUT_FILENO, ("./" + subdir + "\n").c_str(), subdir.length()+3);
			dir(subdir, args);
		}
	}
}

//Executes a command not found in this Terminal. Will execute any matching file in /usr/bin.
void sys_cmd(string cmd, vector<string> * args)
{
	//Check if command exists in /usr/bin
	string path = "/bin/";
	DirectoryReader * usrbin = new DirectoryReader(string("/usr/bin"));
	usrbin->getFiles();
	vector<string> cmds = *(usrbin->sortFiles(0, 0, 0));

	if(find(cmds.begin(), cmds.end(), cmd) != cmds.end()) path = "/usr/bin/";

		//Create char * [] of arguments for exec
		char * new_args[(args->size()+1) * sizeof(char *)];
		for(int i = 0; i < args->size(); i++)
		{
			//Must cast out of const char * 
			new_args[i] = const_cast<char*>((args->at(i)).c_str());
		}
		new_args[(args->size())] = NULL;

		/** DEBUG
		for(int i  = 0; i < args->size(); i++)
		{
			cout << "\tIndex " << i << ": ";
			cout << new_args[i] << endl;
		}
		*/

		pid_t child = fork();
		if(child == 0)
		{
			path = path + cmd;
			//cout << "[Kid] Execing: " << path << endl;
			//cout << "[Kid] STDOUT is " << STDOUT_FILENO << endl;
			execv((path).c_str(), new_args);
			
			exit(0);
		} else {
			
			waitpid(child, NULL, 0);

			//cout << "[Parent] ID " << child << " returned." << endl;
		}
	
}

int Terminal::process_cmd(vector<string> * args)
{
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
		write(STDOUT_FILENO, "\n", 1);
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
		dir(get_cwd_string(), args);
		return 0;
	} else {
 		//Unknown command, try to exec it 
		sys_cmd(cmd, args);
		return 0;
	}
	return -1;
}

int * newPipe()
{
	int * p = new int[2];
	return p;
}

int Terminal::parse_cmd(char * cmd_word, int cmd_len)
{
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

	//Parse entire commands with separators

	vector<vector<string> *> * cmds = new vector<vector<string> *>();
	vector<string> * separators = new vector<string>();
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

	int stdout_save = dup(STDOUT_FILENO);

	if(cmds->size() > 1 && separators->at(0) == "|")
	{
		int p[2];
		pipe(p);

		if(fork() == 0)
		{
			dup2(p[1], STDOUT_FILENO);
			process_cmd(cmds->at(0));
			close(p[0]);
			return -1;
		} else {
			waitpid(-1, NULL, 0);
			close(p[1]); // Close write end of pipe
		}
		
		if(fork() == 0)
		{
			close(p[1]);
			dup2(p[0], STDIN_FILENO);
			dup2(stdout_save, STDOUT_FILENO);
			process_cmd(cmds->at(1));
			return -1;
		} else {
			waitpid(-1, NULL, 0);
		}

		
	} else if(cmds->size() > 1 && separators->at(0) == ">")
	{
		char * name = const_cast<char*>(cmds->at(1)->at(0).c_str());
		int fd = open(name, O_CREAT|O_WRONLY, S_IRWXU);
		if(fork() == 0)
		{
			dup2(fd, STDOUT_FILENO);
			process_cmd(cmds->at(0));
			return -1;
		} else {
			waitpid(-1, NULL, 0);
		}
	}  else if(cmds->size() > 1 && separators->at(0) == "<")
	{
		char * name = const_cast<char*>(cmds->at(1)->at(0).c_str());
		int fd = open(name, O_RDONLY, S_IRWXU);
		if(fork() == 0)
		{
			dup2(fd, STDIN_FILENO);
			process_cmd(cmds->at(0));
			return -1;
		} else {
			waitpid(-1, NULL, 0);
		}
	}
	else {
		return process_cmd(cmds->at(0));
	}

	return 0;
	
	
	
	


	//return process_cmd(cmds->at(0));
}

void Terminal::run()
{
	int running = 1;
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
	}
}