#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "Terminal.h"
#include "DirectoryReader.h"

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

void Terminal::dir(vector<string> * args)
{
	int adate = 0;
	int name = 0;
	int type = 0;
	if(args->size() > 1 && args->size() < 3)
	{	
		cout << args->at(1).substr(1,2) << endl;
		if(args->at(1).substr(1,2) == "s=")
		{
			cout << "\033[32mU didn't handle this one bud\033[0;m\n";
			string flag = args->at(1).substr(3);
			if(flag == "adate") adate = 1;
			else if(flag == "name") name = 1;
			else if(flag == "type") type = 1;
		}
	}

	DirectoryReader * dir = new DirectoryReader(get_cwd_string());
	vector<string> * names = dir->getFiles(adate, name, type);

	//Help with ASCII color codes: https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
	for(int i = 0; i < names->size(); i++)
	{
		cout << "\033[1;36m" <<  names->at(i) << "\033[0;m" << endl;
	}
}

int Terminal::process_cmd(vector<string> * args)
{
	//We know the first argument is always the command name
	string cmd = args->at(0);

	//command: "exit"
	if(cmd == "exit")
	{
		return 0;
	}

	//command: "cwd"
	if(cmd == "cwd")
	{
		cwd();
		write(STDOUT_FILENO, "\n", 1);
	}

	if(cmd == "setprompt")
	{
		if(args->size()-1 < 1)
		{
			write(STDOUT_FILENO, "usage: setprompt <prompt string>\n", 33);
		} else {
			string newprompt = args->at(1);
			newprompt = replace_cwd_wildcard(newprompt);
			prompt_phrase = newprompt;
		}
	}

	if(cmd == "cd")
	{
		if(args->size()-1 < 1)
		{
			write(STDOUT_FILENO, "usage: cd <path>\n", 17);
		} else {
			chdir(args->at(1).c_str());
			update_prompt();
		}
	}

	if(cmd == "dir" || cmd == "ls")
	{
		dir(args);
	}

	return 1;
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
	
	//Process command
	return process_cmd(args);
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
		if(result == 0)
		{
			running = 0;
		}
	}
}