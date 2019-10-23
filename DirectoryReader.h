#ifndef DIRECTORYREADER_H
#define DIRECTORYREADER_H

#include <dirent.h>
#include <vector>

using namespace std;

class DirectoryReader {
public:
	DirectoryReader(string);
	vector<string> * getFiles(int, int, int);
private:
	string dir_name;
};

#endif