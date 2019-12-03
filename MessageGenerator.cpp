#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

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
	int fd = open("/tmp/MyShellNormal", O_CREAT|O_WRONLY, S_IRWXU);
	fifos[2] = fd;

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
			
			//Do this part, idiot

		} else if(userSelection == 4)
		{

			running = 0;

		}
	}
}
