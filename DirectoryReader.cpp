#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include "DirectoryReader.h"

using namespace std;

DirectoryReader::DirectoryReader(string name)
{
	this->dir_name = name;
}

struct dirInfo{
	string name;
	time_t atime;
};

bool sort_di(dirInfo d1, dirInfo d2)
{
	return (d1.atime < d2.atime);
}

vector<string> * DirectoryReader::getFiles()
{
	DIR * dir_stream = opendir(dir_name.c_str());
	vector<string> * file_names = new vector<string>();
	vector<unsigned char> * file_types = new vector<unsigned char>();
	vector<string> * dirs = new vector<string>();

	struct dirent * first_element;
	while((first_element = readdir(dir_stream)) && (first_element != NULL))
	{

		string * element_name = new string((first_element->d_name));
		unsigned char element_type = first_element->d_type;
		//No point pushing the . and .. FDs that we assume exist everywhere
		//cout << "Successfully retrieved name and type for file." << endl;
		
		if(*element_name != ".." && *element_name != ".")
		{
			file_names->push_back(*element_name);
			file_types->push_back(element_type);

			if(element_type == DT_DIR)
			{
				dirs->push_back(*element_name);
			}
		}
	}

	files = *file_names;
	types = *file_types;
	delete(file_names);
	delete(file_types);
	return dirs;
}

DirectoryReader::~DirectoryReader()
{
	
}

vector<string> * DirectoryReader::sortFiles(int adate, int name, int type)
{
	vector<string> * file_names = &(this->files);
	vector<unsigned char> * file_types = &(this->types);

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