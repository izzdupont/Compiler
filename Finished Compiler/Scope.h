#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "VarData.h"

struct Scope
{
	int offSet;
	int paramOffset;

	string returnType;
	string DtType;
	string ScopeName;
	Scope* prevScope;

	vector <string> paramOrder;
	unordered_map <string, VarData> Table;

	Scope()
	{
		DtType = "";
		ScopeName = "";
		returnType = "";
		offSet = 0;
		paramOffset = 8;
		prevScope = NULL;
	}
};