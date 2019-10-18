#ifndef TERMINAL_H
#define TERMINAL_H

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

class Terminal {
public:
	Terminal();
	virtual ~Terminal();
private:
	void run();
	void update_prompt();
	int parse_cmd(char *, int);
	int process_cmd(vector<string> *);
	string replace_cwd_wildcard(string);
	void prompt();
	string get_cwd_string();
	void cwd();
	string prompt_phrase;
};

#endif