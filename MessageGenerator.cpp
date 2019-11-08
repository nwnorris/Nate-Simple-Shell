#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int main()
{	
	//Setup variables
	int running = 1;
	char * choices[4] = {"Send High Priority Message", "Send Normal Message", "Check for responses", "Quit"};
	int userSelection;

	//Open fifos
	int fifos[3];
	fifos[2] = open("/tmp/MyShellNormal", O_CREAT|O_WRONLY); 

	while(running)
	{
		for(int i = 0; i < 4; i++)
		{
			cout << "[" << (i+1) << "] " << choices[i] << endl;
		}
		cout << ">";

		cin >> userSelection;

		if(userSelection > 4 || userSelection < 1)
		{
			cout << "Invalid selection.\n";
		} else if(userSelection == 2)
		{
			write(STDOUT_FILENO, "Message to send: ", 17);
			char * buff[256];
			int amt_read = read(STDIN_FILENO, buff, 256);
			write(fifos[userSelection], buff, amt_read);

		} else if(userSelection == 4)
		{
			running = 0;
		}
	}
}