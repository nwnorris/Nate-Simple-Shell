#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "Terminal.h"

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

//This function sucks. Fix pls. Sincerely you.
string Terminal::replace_cwd_wildcard(string phrase)
{	
	string original = phrase;
	int loc = phrase.find("%");
	if(loc > -1)
	{
		int start = loc;
		phrase = phrase.substr(start+1, phrase.length());
		loc = phrase.find("%");
		if(loc > -1)
		{
			int end = loc+(start-1);
			string wildcard_phrase = original.substr(start+1, end-2);
			string content = "";
			cout << wildcard_phrase << endl;
			if(wildcard_phrase == "PWD")
			{
				content = get_cwd_string();
			}
			cout << "\t" << original.substr(0, start) << endl;
			cout << "\t" << original.substr(end, original.length()) << endl;

			original = original.substr(0, start) + content + original.substr(end, original.length());
		}
	}

	return original;
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