#pragma once

#include "Scope.h"

using namespace std;

class SymbolTable
{
public:
	Scope* curScope;

	SymbolTable()
	{
		curScope = new Scope;
	}

	bool AddVariable(string name, string type, int size, bool arr2Check, int start1, int start2, int end1, int end2)
	{
		if (curScope->Table.find(name) != curScope->Table.end())
		{
			return false;
		}
		curScope->Table[name].ref = 0;
		curScope->Table[name].dataType = type;
		curScope->Table[name].size = size;
		curScope->Table[name].arrCheck = false;
		curScope->Table[name].offSet = curScope->offSet;
		curScope->Table[name].nextScope = NULL;
		curScope->offSet += size;
		return true;
	}
	bool AddArray(string name, string type, int size, bool arrCheck, int start1, int start2, int end1, int end2)
	{
		if (curScope->Table.find(name) != curScope->Table.end())
		{
			return false;
		}
		curScope->Table[name].ref = 0;
		curScope->Table[name].dataType = type;
		curScope->Table[name].size = size;
		curScope->Table[name].offSet = curScope->offSet;
		curScope->Table[name].nextScope = NULL;
		curScope->offSet += size;
		curScope->Table[name].arrCheck = true;
		curScope->Table[name].start1 = start1;
		curScope->Table[name].start2 = start2;
		curScope->Table[name].end1 = end1;
		curScope->Table[name].end2 = end2;
		return true;
	}

	bool AddParam(string name, string dataType, bool passByRef)
	{
		if (curScope->Table.find(name) != curScope->Table.end())
		{
			return false;
		}
		if (passByRef == false)
		{
			curScope->Table[name].ref = 4;
		}
		else if (passByRef == true)
		{
			curScope->Table[name].ref = 1;
		}
		//curScope->Table[name].ref = 1;
		curScope->Table[name].dataType = dataType;
		curScope->Table[name].size = 4;
		curScope->Table[name].offSet = curScope->paramOffset;
		curScope->paramOrder.push_back(name);
		curScope->paramOffset += 4;
		return true;
	}

	bool getArrInfo(string ArrName, int& End1, int& End2, int& Start1, int& Start2)
	{
		Scope* ArrScope = curScope;
		while (ArrScope != NULL)
		{
			if (ArrScope->Table.find(ArrName) == ArrScope->Table.end())
			{
				ArrScope = ArrScope->prevScope;
			}
			else
			{
				Start1 = ArrScope->Table[ArrName].start1;
				Start2 = ArrScope->Table[ArrName].start2;
				End1 = ArrScope->Table[ArrName].end1;
				End2 = ArrScope->Table[ArrName].end2;
				return true;
			}
		}
		return false;
	}

	bool FindVariable(string name, int& offset, string& dtType, int&VFParProc, bool&refornot, int&size, bool&globalC, bool&anArray)
	{
		Scope* TravScope = curScope;
		while (TravScope != NULL)
		{
			//local
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->prevScope;
				
			}
			//global
			else
			{
				
				if (TravScope->Table[name].ref == 0) //variable
				{
					VFParProc = 0;
					size = TravScope->Table[name].size;
				}
				//1 is ref 4 is copy
				else if (TravScope->Table[name].ref == 1 || TravScope->Table[name].ref == 4) //param
				{
					VFParProc = 1;
					if (TravScope->Table[name].ref == 1) { refornot = true; }
					if (TravScope->Table[name].ref == 4) { refornot = false; }
					size = TravScope->Table[name].size;
				}
				else if (TravScope->Table[name].dataType == "procedure" && TravScope->Table[name].nextScope)
				{
					VFParProc = 2;
					size = TravScope->Table[name].size;
				}
				else if (TravScope->Table[name].dataType == "function" && TravScope->Table[name].nextScope)
				{
					VFParProc = 3;
					size = TravScope->Table[name].size;
				}
				dtType = TravScope->Table[name].dataType;
				offset = TravScope->Table[name].offSet;
				anArray = TravScope->Table[name].arrCheck;
				if (TravScope->prevScope) {
					globalC = false;
				}
				else {
					globalC = true;
				}
				return true;
			}
		}
		return false;
	}

	bool AddProcFunc(string name, string type)
	{
		Scope* TravScope = curScope;
		if (curScope->Table.find(name) != curScope->Table.end())
		{
			return false;
		}
		curScope->Table[name].dataType = type;
		curScope->Table[name].size = 0;
		curScope->Table[name].offSet = 0;

		if (type == "procedure")
		{
			curScope->Table[name].ref = 2;
		}
		else if (type == "function")
		{
			curScope->Table[name].ref = 3;
		}

		Scope* OldCurrent = curScope;

		curScope->Table[name].nextScope = new Scope;
		curScope = curScope->Table[name].nextScope;
		curScope->offSet = 0;
		curScope->ScopeName = name;
		curScope->prevScope = OldCurrent;

		return true;

	}

	void BackOut()
	{
		if (curScope->prevScope) curScope = curScope->prevScope;
	}

	bool getParamInfo(string Name, int curParam, string& dataType, bool& passByRef, int& offSet)
	{
		Scope* temp = curScope;
		while (temp != NULL)
		{
			if (temp->Table.find(Name) == temp->Table.end())
			{
				temp = temp->prevScope;
				return false;
			}
			else 
			{
				temp = temp->Table[Name].nextScope;
				dataType = temp->Table[temp->paramOrder[curParam]].dataType;
				if (temp->Table[temp->paramOrder[curParam]].ref == 1)
				{
					passByRef = true;
					offSet = temp->Table[Name].offSet;
				}
				else if (temp->Table[temp->paramOrder[curParam]].ref == 2)
				{
					passByRef = false;
					offSet = temp->Table[Name].offSet;
				}
				return true;
			}
		}
	}

	int trackCount()
	{
		return curScope->offSet;
	}
};