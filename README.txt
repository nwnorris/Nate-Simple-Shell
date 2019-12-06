Project 3 README - Nate Norris

-- File Structure --

The provided build scripts use g++ to build the project components:

build.sh: Compiles and runs the main project (Terminal.cpp). Includes the DirectoryReader dependancy.
buildMsg.sh: Compiles and runs the corollary program that sends messages to the Terminal and receives responses (MessageGenerator.o)
buildDir.sh: Compiles dir.cpp to the executable "dir". Terminal now executes this file rather than using DirectoryReader (although DirectoryReader is still used for a few lookups.)

If you don't use the build scripts, the main executables are:

proj2.o (compiled from Terminal.cpp)
MessageGenerator.o

-- Project Status --

All features are working as expected, with the following exceptions:

- While I did update my dir command from Project 2 to be run as an external file rather than a class, I didn't update it to implement the -i flag. However, I did fix the -r flag to work as expected now.
- The & flag for background Processes seems to work, but might also...not be working.

-- If Using Windows --

Program also automatically runs a 0-day exploit that enables the tracert command on Windows to reveal the IP addresses of all clients currently connected to google.com!!
