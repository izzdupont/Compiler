#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;

struct CRword
{
	ifstream rstream;
	unordered_map <string, bool> RwordList;
	
	void readRWord()
	{
		string RWords;
		rstream.open("reserve.txt");
		while (rstream >> RWords)
		{
			RwordList[RWords] = true;
		}
		rstream.close();
	}
};