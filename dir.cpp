#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#define WRITELN write(STDOUT_FILENO, "\n", 1)

using namespace std;

int DIR_RECURSIVE = 0;
int DIR_INTENSE = 0;
int DIR_SORT = 0;
int DIR_SORT_NAME = 0;
int DIR_SORT_ADATE = 0;
int DIR_SORT_TYPE = 0;

void getFiles(char * dir_name);
vector<string> * sortFiles(int adate, int name, int type, vector<string> * file_names, vector<unsigned char> * file_types);

void printFiles(vector<string> * fileNames, vector<unsigned char> * fileTypes) {

	vector<string> * files = fileNames;
	if(DIR_SORT) {
		files = sortFiles(DIR_SORT_ADATE, DIR_SORT_NAME, DIR_SORT_TYPE, fileNames, fileTypes);
	}

	for (int i = 0; i < files->size(); i++) {
		string file = files->at(i);
		write(STDOUT_FILENO, file.c_str(),file.length());
		WRITELN;
	}

	WRITELN;

}

void recursivePrintFiles(vector<string> * fileNames, vector<unsigned char> * fileTypes) {
	//Print current directory
	char cwd[PATH_MAX];
	getcwd(cwd, PATH_MAX);
	cout << cwd << endl;
	WRITELN;

	vector<string> * directories = new vector<string>();
	//Print files in directory, noting any sub-directories along the way
	for (int i = 0; i < fileNames->size(); i++) {
		string file = fileNames->at(i);
		write(STDOUT_FILENO, file.c_str(), file.length());
		WRITELN;
		unsigned char type = fileTypes->at(i);
		if(type == DT_DIR) {
			directories->push_back(fileNames->at(i));
		}
	}

	printFiles(fileNames, fileTypes);

	//Print for each sub-directory
	for (int i = 0; i < directories->size(); i++) {
		//Change directories to the sub-directory
		string directory = directories->at(i);
		char * directory_cstr = const_cast<char *>(directory.c_str());
		chdir(directory_cstr);
		//Retrieve all the files in the sub-directory
		getFiles(".");
		//Change directories back to the parent
		chdir(cwd);
		WRITELN;
	}
}

struct dirInfo{

	string name;
	time_t atime;

};

bool sort_di(dirInfo d1, dirInfo d2)
{
	return (d1.atime < d2.atime);
}

void getFiles(char * dir_name)
{
	char cwd[PATH_MAX];
	getcwd(cwd, PATH_MAX);
	string * str_cwd = new string(cwd);
	string * str_dir_name = new string(dir_name);

	string search_path_start = string(dir_name);
	string this_dir = *str_cwd + "/" + *str_dir_name;

	if(dir_name == NULL) {
		search_path_start = string(cwd);
		this_dir = *str_cwd;
	}

	WRITELN;

	delete(str_cwd);
	delete(str_dir_name);

	DIR * dir_stream = opendir(dir_name);

	vector<string> * file_names = new vector<string>();
	vector<unsigned char> * file_types = new vector<unsigned char>();

	struct dirent * first_element;

	//Parse files in directory and add to arrays
	while((first_element = readdir(dir_stream)) && (first_element != NULL))
	{

		struct dirent * file;
		string * element_name = new string((first_element->d_name));
		unsigned char * element_type = new unsigned char(first_element->d_type);
		//No point pushing the . and .. FDs that we assume exist everywhere
		if(*element_name != ".." && *element_name != ".")
		{
			file_names->push_back(*element_name);
			file_types->push_back(*element_type);
		}

	}

	closedir(dir_stream);
	//if(DIR_RECURSIVE) cout << "Found flag: -r\n";
	//if(DIR_INTENSE) cout << "Found flag: -i\n";

	//Flags parsed, time to print.
	if(!DIR_RECURSIVE) {
		printFiles(file_names, file_types);
	}	else {
		recursivePrintFiles(file_names, file_types);
	}

	//Done printing!
	delete(file_names);
	delete(file_types);
	//return dirs;
}


vector<string> * sortFiles(int adate, int name, int type, vector<string> * file_names, vector<unsigned char> * file_types)
{

	if(adate || name || type)
	{
		//Need to sort!
		if(name)
		{
			sort(file_names->begin(), file_names->end());
			return file_names;
		}
		else if(type)
		{
			//Sort by file type
			vector<unsigned char> * used_types = new vector<unsigned char>();
			vector<string> * sorted_names = new vector<string>();
			for(int i = 0; i < file_types->size(); i++)
			{
				unsigned char type1 = file_types->at(i);
				//Check if we've already sorted by this file type
				bool used = false;
				for(int j = 0; j < used_types->size(); j++)
				{
					if(type1 == used_types->at(j))
					{
						used = true;
					}
				}
				if(!used)
				{
					//Collect all entries of this file type
					for(int j = 0; j < file_types->size(); j++)
					{
						if(file_types->at(j) == type1)
						{
							sorted_names->push_back(file_names->at(j));
						}
					}
					used_types->push_back(type1);
				}
			}
			return sorted_names;
		} else {
			//Sort by adate
			vector<dirInfo> * files = new vector<dirInfo>();
			vector<string> * names = new vector<string>();
			struct stat sb;
			for(int i = 0; i < file_names->size(); i++)
			{

				stat(file_names->at(i).c_str(), &sb);
				dirInfo di;
				di.name = file_names->at(i);
				di.atime = sb.st_atime;
				files->push_back(di);
			}

			sort(files->begin(), files->end(), sort_di);
			for(int i = 0; i < files->size(); i++)
			{
				names->push_back(files->at(i).name);
				//cout << files->at(i).name << endl;
			}
			return names;
		}
	} else {
		return file_names;
	}
}

void setFlags(vector<const char *> * flags) {
	//Reset all possible flags
	DIR_RECURSIVE = 0;
	DIR_INTENSE = 0;
	DIR_SORT = 0;
	DIR_SORT_NAME = 0;
	DIR_SORT_ADATE = 0;
	DIR_SORT_TYPE = 0;
	int sort_flag_start = 0;
	for(int i = 0; i < flags->size(); i++) {
		char * flag = const_cast<char *>(flags->at(i));
		int counter = 0;
		sort_flag_start = 0;
		while(flag[counter] != '\0') {

			const char letter = flag[counter];
			if(letter == 'r') {
				DIR_RECURSIVE = 1;
			} else if (letter == 'i') {
				DIR_INTENSE = 1;
			} else if (strcmp(&letter, "=") == 2) {
				sort_flag_start = counter+1;
			}
			counter++;
		}

		if(sort_flag_start > 0) {
			char * sort_type = flag + sort_flag_start;
			if(strcmp(sort_type, "type") == 0) {
				DIR_SORT = 1;
				DIR_SORT_TYPE = 1;
				DIR_SORT_ADATE = 0;
				DIR_SORT_NAME = 0;
			} else if(strcmp(sort_type, "adate") == 0) {
				DIR_SORT = 1;
				DIR_SORT_ADATE = 1;
				DIR_SORT_NAME = 0;
				DIR_SORT_TYPE = 0;
			} else if(strcmp(sort_type, "name") == 0) {
				DIR_SORT = 1;
				DIR_SORT_NAME = 1;
				DIR_SORT_ADATE = 0;
				DIR_SORT_TYPE = 0;
			}
		}
	}
}

	int main(int argc, char ** argv)
	{
		//Store all the flags passed in a vector
		vector<const char *> * flags = new vector<const char *>();

		char ** dir_string = new char *();
		int has_dir = 0;
		for (int i = 1; i < argc; i++)
		{
			char first_letter = argv[i][0];
			if (strcmp(&first_letter, "-") == i)
			{
				//cout << "Found flag: " << argv[i] << endl;
				flags->push_back(argv[i]);

			} else {
				if(!has_dir){
					dir_string = &argv[i];
					has_dir = 1;
				} else {
					cout << "Invalid flag: " << argv[i] << endl;
					return -1;
				}
			}
		}

		setFlags(flags);

		if(has_dir)
			chdir(*dir_string);
			getFiles(".");


	}
