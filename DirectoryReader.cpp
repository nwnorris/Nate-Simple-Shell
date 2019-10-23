#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <bits/stdc++.h>
#include "DirectoryReader.h"

using namespace std;

DirectoryReader::DirectoryReader(string name)
{
	dir_name = name;
}

struct dirInfo{
	string name;
	unsigned char type;
	string flag;
};

vector<string> * DirectoryReader::getFiles(int adate, int name, int type)
{
	DIR * dir_stream = opendir(dir_name.c_str());
	vector<string> * file_names = new vector<string>();
	vector<unsigned char> * file_types = new vector<unsigned char>();
	vector<dirInfo> * files = new vector<dirInfo>();

	struct dirent * first_element;
	while((first_element = readdir(dir_stream)) && (first_element != NULL))
	{
		string * element_name = new string((first_element->d_name));
		unsigned char element_type = first_element->d_type;
		//No point pushing the . and .. FDs that we assume exist everywhere
		if(*element_name != ".." && *element_name != ".")
		{
			file_names->push_back(*element_name);
			file_types->push_back(element_type);
			files->push_back({*element_name, element_type, "T"});
		}
	}

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
					if(!used)
					{
					//Collect all entries of this file type
					for(int j = 0; j < file_types->size(); j++)
					{
						if(file_types->at(j) == type1)
						{
							sorted_names->push_back(file_names->at(i));
						}
					}
					used_types->push_back(type1);
					}
				}
				
			}
			return sorted_names;
		}
	}
	
	
}