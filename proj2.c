
#include <string.h>
#include <unistd.h>
#include <stdio.h>
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

void prompt()
{
	cwd();
	write(STDOUT_FILENO, &">", 1);
}

int parse_cmd(char * cmd_word, int cmd_len)
{
	char * args[10];

	int start = 0;
	int end = 0;
	int arg_ct = 0;

	for(int i=0; i<cmd_len; i++)
	{
		if(*(cmd_word+i) == ' ')
		{
			end = i;
			int wordlen = end-start;
			char arg[wordlen];
			for(int j = start; j < end; j++)
			{
				arg[j-start] = cmd_word[j];
			}
			args[0] = arg;
		}
	}

	if(strcmp(cmd_word, "exit\n") == 0)
	{
		return 0;
	} else if(strcmp(cmd_word, "cwd\n") == 0)
	{
		//write(STDOUT_FILENO, "\n", 1);
		cwd();
		write(STDOUT_FILENO, "\n", 1);
	}

	return 1;
}

int main()
{
	
	//cwd();
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