#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;

struct DataTypeList
{
	ifstream rstream;
	unordered_map <string, bool> TypeList;

	void readDTWord()
	{
		string RWords;
		rstream.open("datatype.txt");
		while (rstream >> RWords)
		{
			TypeList[RWords] = true;
		}
		rstream.close();
	}
};