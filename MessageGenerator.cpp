/*
Author: Nate Norris
Project: Linux Programming | Project 3 | Fall '19
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#define WRITELN write(STDOUT_FILENO, "\n", 1)

using namespace std;

int writeInputToFifo(int fifo_id, int * fifos) {

	write(STDOUT_FILENO, "Message to send: ", 17);
	char * buff[256];
	int amt_read = read(STDIN_FILENO, buff, 256);
	int result = write(fifos[fifo_id], buff, amt_read);
	return result;
}


int main()
{
	//Setup variables
	int running = 1;
	char * choices[4] = {"Send High Priority Message", "Send Normal Message", "Check for responses", "Quit"};
	int userSelection;

	//Open fifos
	int fifos[3];
	fifos[2] = open("/tmp/MyShellNormal", O_WRONLY, S_IRWXU);
	fifos[1] = open("/tmp/MyShellHighPriority", O_WRONLY, S_IRWXU);

	while (running)
	{
		for (int i = 0; i < 4; i++)
		{
			cout << "[" << (i+1) << "] " << choices[i] << endl;
		}
		cout << ">";

		cin >> userSelection;

		if (userSelection > 4 || userSelection < 1)
		{
			cout << "Invalid selection.\n";
		} else if (userSelection == 1){

			writeInputToFifo(userSelection, fifos);

		} else if (userSelection == 2)
		{

			writeInputToFifo(userSelection, fifos);

		} else if (userSelection == 3) {
			fifos[0] = open("/tmp/MyShellResponse", O_RDONLY, S_IRWXU);
			char * buff[256];
			int amtRead = read(fifos[0], buff, 256);
			write(STDOUT_FILENO, buff, amtRead);
			WRITELN;
			close(fifos[0]);

		} else if(userSelection == 4)
		{

			running = 0;

		}
	}
}
