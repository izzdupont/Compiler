#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;

struct PFVRword
{
	ifstream rstream;
	unordered_map <string, bool> PFVList;

	void readPFVRWord()
	{
		string RWords;
		rstream.open("PFV-rword.txt");
		while (rstream >> RWords)
		{
			PFVList[RWords] = true;
		}
		rstream.close();
	}
};