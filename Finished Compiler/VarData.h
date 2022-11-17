#pragma once
#include <iostream>
#include <string>
#include "Scope.h"

using namespace std;
struct Scope;
struct VarData
{
	string dataType;
	string name;
	int ref; //var = 0,pass by ref = 1, cp =2
	int offSet;
	int size;
	bool arrCheck;
	int start1;
	int start2;
	int end1;
	int end2;
	Scope* nextScope;

	VarData()
	{
		name = "";
		ref = NULL;
		offSet = NULL;
		size = NULL;
		dataType = "";
		nextScope = NULL;
		arrCheck = false;
		start1 = NULL;
		start2 = NULL;
		end1 = NULL;
		end2 = NULL;
	}
	VarData(string name, int offset, int size, int ref, bool arrCheck, int start1, int start2, string end1, string end2)
	{
		name = name;
		offset = offset;
		size = size;
		ref = ref;
		end1 = end1;
		end2 = end2;
		start1 = start1;
		start2 = start2;
		arrCheck = arrCheck;
		dataType = "";
		nextScope = NULL;
	}

};