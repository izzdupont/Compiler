#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "CToken.h"
#include <unordered_map>
#include "CRword.h"
#include <algorithm>
#include "PFVRword.h"
#include "DataType.h"

using namespace std;

class CLex
{
	int rowCount = 128;
	int** DFA = new int* [rowCount]; //for 2D arrray
	ifstream f1stream;
	ifstream DFAStream;
	ifstream rstream;
	string rword;
	CRword reserve;
	PFVRword reserveW;
	DataTypeList typeList;

public:
	CLex()
	{
		reserve.readRWord();
		reserveW.readPFVRWord();
		typeList.readDTWord();
		f1stream.open("array1.txt"); //the  program
		DFAStream.open("newDFA.txt"); //for the exel
		string value;

		for (int i = 0; i < 128; i++)
		{
			DFA[i] = new int[14];
			for (int j = 0; j < 14; j++)
			{
				DFAStream >> value;
				int num = stoi(value); //stoi change string to int
				DFA[i][j] = num;
			}
		}
	}
	~CLex()
	{
		for (int i = 0; i < 128; i++)
		{
			delete[] DFA[i];
		}
		delete[] DFA;
	}

	bool getToken(CToken& token)
	{
		char getChar;
		int CurState = 0;
		int PrevState = 0;

		token.tokenType = "";
		token.tokenValue = "";

		while (!f1stream.eof())
		{
			getChar = f1stream.get();
			PrevState = CurState;

			if (getChar == -1)
			{
				getChar = 10;
			}

			CurState = DFA[getChar][CurState];

			if (CurState != 0 && CurState != 55 && CurState != 99) {
				token.tokenValue = token.tokenValue + getChar;
			}
			else if (CurState == 55 && getChar != 9 && getChar != 10 && getChar != 32)
			{
				f1stream.unget();
				if (PrevState == 1)
				{
					transform(token.tokenValue.begin(), token.tokenValue.end(), token.tokenValue.begin(), ::tolower);
					if (reserve.RwordList[token.tokenValue])
					{
						token.tokenType = "rword";
					}
					else 
					{
						token.tokenType = "words";
						if (reserveW.PFVList[token.tokenValue])
						{
							token.tokenType = "PFV-rword";
						}
						else if (typeList.TypeList[token.tokenValue])
						{
							token.tokenType = "dataType";
						}
					}
					return true;
				}
				else if (PrevState == 2)
				{
					token.tokenType = "integer";
					return true;
				}
				else if (PrevState == 4)
				{
					token.tokenType = "float";
					return true;
				}
				else if (PrevState == 9)
				{
					token.tokenType = "real";
					return true;
				}
				else if (PrevState == 10 || PrevState == 11 || PrevState == 12)
				{
					token.tokenType = "special";
					if (PrevState == 12)
					{
						token.tokenType = ":=";
					}
					return true;
				}
			}
			else if (CurState == 55)
			{
				if (PrevState == 1)
				{
					transform(token.tokenValue.begin(), token.tokenValue.end(), token.tokenValue.begin(), ::tolower);
					if (reserve.RwordList[token.tokenValue])
					{
						token.tokenType = "rword";
					}
					else
					{
						token.tokenType = "words";
						if (reserveW.PFVList[token.tokenValue])
						{
							token.tokenType = "PFV-rword";
						}
						else if (typeList.TypeList[token.tokenValue])
						{
							token.tokenType = "dataType";
						}
					}
					//cout << token.tokenType << " | " << token.tokenValue << endl;
					return true;
				}
				else if (PrevState == 2)
				{
					token.tokenType = "integer";
					return true;
				}
				else if (PrevState == 4)
				{
					token.tokenType = "float";
					return true;
				}
				else if (PrevState == 9)
				{
					token.tokenType = "real";
					return true;
				}
				else if (PrevState == 10 || PrevState == 11 || PrevState == 12 || PrevState == 13)
				{
					token.tokenType = "special";
					if (PrevState == 12)
					{
						token.tokenType = ":=";
					}
					return true;
				}
			}			else if (CurState == 99)
			{
				cout << "Lex Error" << endl;
				return false;
			}
		}
		f1stream.close();
		return false;
	}
};



