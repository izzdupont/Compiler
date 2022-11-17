#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include "CLexer.h"
#include "CToken.h"
#include <algorithm>
#include "Scope.h"
#include "SymbolTable.h"

using namespace std;

struct Regs
{
	string RegName;
	bool lit;
	int val;
};

struct Condition
{
	int count;
	string condType;
};

class Pascal
{
private:
	CToken myToken;
	CLex Lexer;
	ifstream testFile;
	SymbolTable symbolTable;
	Scope scopes;
	bool stat;

	stack <string> DTstack;
	stack <string> asmStack;
	stack <string> procStack;
	//stack <pair <std::string, int>> condStack; //for if while
	stack <Condition> condStack;

	vector <string> VPvector;
	vector <string> ArrVector;

	string DTname;
	int DTsize;
	bool PassByRef;
	string tempDTtype;
	string paramName;
	bool arr2Check;
	int start1; 
	int start2;
	int end1;
	int end2;

	int returnSize = 0;
	int curRegister = 0;
	Regs regArr[4];

	//if while
	int orCount = 1;
	int ifCount = 1;
	int endCount = 1;
	int elseCount = 1;
	int whileCOunt = 1;
	
	bool OrUsed = false;
	bool ifCond = false;
	bool end_IfConds = false;
	bool whileCond = false;
	bool end_whileConds = false;

	string lastComp = "";
	string label = "";
	string lastToken = "";
	Condition Conds;

	//array
	int arrVal1;
	int arrVal2;
	int arr2Val1;
	int arr2Val2;
	bool arrCheck = false;

public:


	void GetToken()
	{

		if (Lexer.getToken(myToken))
		{
			cout << myToken.tokenValue + " ";
			stat = true;
		}
		else
		{
			stat = false;
		}
	}
	ofstream out;
	void pprogram()
	{
		printHead();

		regArr[0].RegName = "eax";
		regArr[1].RegName = "ebx";
		regArr[2].RegName = "ecx";
		regArr[3].RegName = "edx";

		
		GetToken();
		program();

		if (stat == false)
		{
			cout << "Ok!!" << endl;
			printFoot();

  			exit(0);
		}
		else
		{
			cout << "Bad!!" << endl;

			exit(0);
		}
	}

	void program()
	{
		block();
		if (myToken.tokenValue == ".")
		{
			GetToken();
			
		}
		else
		{
			cout << "Error in ." << endl;
			exit(0);
		}
	}

	void block()
	{
		PFV();
		if (myToken.tokenValue == "begin")
		{
			bool trackBegin = false;
			
			if (!symbolTable.curScope->prevScope)
			{
				out << endl << endl;

				out << "	kmain:  " << " //  main function " << endl;
			}
			else if (printParamHead())
			{
				trackBegin = true;
			}

			GetToken();
			statement();
			mstatement();

 			if (myToken.tokenValue == "end")
			{
				if (trackBegin)
				{
					printParamFoot();
				}
				GetToken();
			}
			else
			{
				cout << "ERROR in end - block!!!" << endl;
				exit(0);
			}
		}
		else
		{
			cout << "ERROR in begin - block!!!" << endl;
			exit(0);
		}
	}

	void statement()
	{
		if (myToken.tokenType == "words")
		{
			//8. Check to see if the word is in the symbol table.  
			//(FindVariable) Push datatype onto the stack.
			int tempOffset;
			int tempSize;
			string tempDT;
			string tempName;
			CToken tempTk = myToken;
			int tempVFPP;
			bool check;
			bool global;
			bool isArrCheck;
			if (symbolTable.FindVariable(tempTk.tokenValue, tempOffset, tempDT, tempVFPP,check, tempSize, global, isArrCheck))
			{
				//push only on param or var
				tempName = myToken.tokenValue;
				if (tempVFPP <= 1)
				{
					DTstack.push(tempDT);
					asmStack.push(""); //push empty stack to asmStack
					//CToken tempToken = myToken;
					tempTk = myToken;
					GetToken();
					//check for 1D arr
					if (myToken.tokenValue == "[")
					{
						arrCheck = true;
						GetToken();
						E();
						if (myToken.tokenValue == ",")
						{
							arr2Check = true;
						}
						if (myToken.tokenValue == "]" && arr2Check == false)
						{
							string tempStruct = asmStack.top();
							out << tempStruct << endl;
							asmStack.pop();

							symbolTable.getArrInfo(tempName, arr2Val1, arr2Val2, arrVal1, arrVal2);
							//OUT ASM
							out << "		sub " + regArr[curRegister - 1].RegName + ", " + to_string(arrVal1) << endl;
							//check for dt
							if (DTname == "char" || DTname == "boolean")
							{
								out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(1) << endl;
							}
							else if (DTname == "integer")
							{
								out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(4) << endl;
							}
							out << "		add " + regArr[curRegister - 1].RegName + ", " + to_string(tempOffset) << endl;
							
						}
						else if (arr2Check == true)
						{
							if (myToken.tokenValue == ",")
							{
								GetToken();
								E();
							}
							if (myToken.tokenValue == "]")
							{
								symbolTable.getArrInfo(tempName, arr2Val1, arr2Val2, arrVal1, arrVal2);
								string tempStruct1 = asmStack.top();
								asmStack.pop();
								string tempStruct2 = asmStack.top();
								asmStack.pop();

								out << tempStruct2 << endl;
								out << tempStruct1 << endl;
								//OUT ASM
								out << "		sub " + regArr[curRegister - 2].RegName + ", " + to_string(arr2Val1) << endl;
								//check for dt
								if (DTname == "char" || DTname == "boolean")
								{
									out << "		imul " + regArr[curRegister - 2].RegName + ", " + to_string(((arr2Val2 - arr2Val1) + 1) * 1) << endl;
								}
								else if (DTname == "integer")
								{
									out << "		imul " + regArr[curRegister - 2].RegName + ", " + to_string(((arr2Val2 - arr2Val1) + 1) * 4) << endl;
								}

								out << "		sub " + regArr[curRegister - 1].RegName + ", " + to_string(arr2Val2) << endl;

								if (DTname == "integer") {
									out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(4) << endl;
								}
								else if (DTname == "char" || DTname == "boolean") {

									out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(1) << endl;
								}
								if (curRegister > 0)
								{
									curRegister--;
								}
								out << "		add " + regArr[curRegister - 1].RegName + ", " + regArr[curRegister].RegName << endl;
								
								out << "		add " + regArr[curRegister - 1].RegName + ", " + to_string(tempOffset) << endl;
								//out << "		mov [ebp + " + regArr[curRegister - 1].RegName + "]," + regArr[curRegister].RegName << endl;
							}
						}
						else
						{
							cout << "missing ] in arr statement" << endl;

							exit(0);
						}
						GetToken();
					}
					if (myToken.tokenValue == ":=")
					{
						GetToken();
						E();

						//10.Pop two datatypes off of the stack. Make sure the two datatypes are the same

						string tempDT1 = DTstack.top();
						DTstack.pop();
						string tempDT2 = DTstack.top();
						DTstack.pop();
						string tempAsmStack1;
						string tempAsmStack2;
						if (asmStack.size() > 0 && asmStack.size() < 2) {
							asmStack.push("		");
						}
						else if (asmStack.size() != 0)
						{
							tempAsmStack1 = asmStack.top();
							asmStack.pop();
							tempAsmStack2 = asmStack.top();
							asmStack.pop();
						}
						if (curRegister > 0)
						{
							curRegister--;
						}
						if (tempDT1 != tempDT2)
						{
							cout << "Error in popping two DT off the stack" << endl;
							exit(0);
						}
						else if (tempDT == tempDT2)
						{
							//in global !symbolTable.curScope->prevScope 
							if (global == true)
							{
								if (!isArrCheck) {
									if (curRegister > 0)
									{
										curRegister--;
									}
									//1 if else statement to check if its variable or param
									else if (tempVFPP == 0) //var
									{
										out << tempAsmStack2 << endl;
										out << tempAsmStack1 << endl;
										out << "		mov [ebp + " << tempOffset << "] , " + regArr[curRegister].RegName << endl << endl;
									}
									else if (tempVFPP == 1) //param
									{
										//ref
										if (check == true)
										{
											out << tempAsmStack2 << endl;
											out << tempAsmStack1 << endl;
											out << "		mov esi,[edi + " << tempOffset << "]" << endl;
											out << "		mov [esi] ," + regArr[curRegister].RegName << endl << endl;
										}
										//copy
										else
										{
											out << tempAsmStack2 << endl;
											out << tempAsmStack1 << endl;
											out << "		mov " + regArr[curRegister].RegName + " , " "[edi + " << tempOffset << "] " << endl << endl;
										}
									}
								}
								else {
										out << tempAsmStack2 << endl;
										out << tempAsmStack1 << endl;
										out << "		mov [ebp + " + regArr[curRegister - 1].RegName + "]," + regArr[curRegister].RegName << endl;
									
									if (curRegister > 0)
									{
										curRegister--;
									}
								}
							}
							else  //local
							{
								if (tempVFPP == 0) //var
								{
									if (curRegister > 0)
									{
										curRegister--;
									}
									out << tempAsmStack2 << tempAsmStack1 << endl;
									out << "		mov [edi - " << tempOffset + tempSize << "] , " << regArr[curRegister].RegName << endl << endl;
								}
								else if (tempVFPP == 1) //param
								{
									//ref
									if (check == true)
									{
										if (curRegister > 0)
										{
											curRegister--;
										}
										out << tempAsmStack2 << endl;
										out << tempAsmStack1 << endl;
										out << "		mov esi, [edi + " << tempOffset << "]" << endl;;
										out << "		mov [esi], " << regArr[curRegister].RegName << endl << endl;
									}
									//copy
									else
									{
										if (curRegister > 0)
										{
											curRegister--;
										}
										out << tempAsmStack2 << endl;
										out << tempAsmStack1 << endl;
										out << "		mov [edi + " << tempOffset << "] , " << regArr[curRegister].RegName << endl << endl;
									}
								}
							}
						}
						//track the reg
						if (curRegister > 0)
						{
							curRegister--;
						}
					}
				}
				else if (tempVFPP == 2)
				{
					CToken tempToken = myToken;
					GetToken();
					if (myToken.tokenValue == "(")
					{
						GetToken();
						consumeParam(tempToken.tokenValue);
						if (myToken.tokenValue == ")")
						{
							GetToken();
						}
						else
						{
							cout << "Error missing )(" << endl;
							exit(1);
						}
					}
					else
					{
						cout << "Error need ) in statement in tempVFPP2" << endl;
						exit(1);
					}
				}
				else if (tempVFPP == 3)
				{
					if (myToken.tokenValue == symbolTable.curScope->ScopeName)
					{
						DTstack.push(symbolTable.curScope->returnType);
						CToken tempToken = myToken;
						GetToken();
						if (myToken.tokenValue == ":=")
						{
							GetToken();
							E();
							string tempDT1 = DTstack.top();
							DTstack.pop();
							string tempDT2 = DTstack.top();
							DTstack.pop();

							if (tempDT1 != tempDT2)
							{
								cout << "Error in popping two DT off the stack in tempVFPP3" << endl;
								exit(1);
							}
						}
						else if (myToken.tokenValue == "(")
						{
							GetToken();
							consumeParam(tempToken.tokenValue);


							if (myToken.tokenValue == ")")
							{
								GetToken();
							}
							else
							{
								cout << "Error missing )/( in tempVFPP3" << endl;
								exit(1);
							}
						}
						else
						{
							cout << "Error in tempVFPP3" << endl;
							exit(1);
						}
					}
				}
				else
				{
					cout << "ERROR in not in tempVFPP3" << endl;
					exit(1);
				}
			}
			else
			{
				cout << "Error finding value in SymbolTable" << endl;
				exit(0);
			}
		}
		if (myToken.tokenType == "rword")
		{
			if (myToken.tokenValue == "begin")
			{
				if (ifCond && condStack.top().condType == "if")
				{
					out << "    " << condStack.top().condType << to_string(condStack.top().count) << ":" << endl;
				}
				else if (!ifCond && condStack.top().condType == "if")
				{
					out << "	ELSE " << condStack.top().condType << to_string(condStack.top().count) << ":" << endl;
				}
				GetToken();
				statement();
				mstatement();
				if (myToken.tokenValue == "end")
				{
					//if it is from IF
					if (ifCond && condStack.top().condType == "if")
					{
						out << "		JMP END_" << condStack.top().condType << to_string(condStack.top().count) << endl;
					}//If it is from WHILE
					else if (whileCond && condStack.top().condType == "while")
					{
						out << "		JMP top" << condStack.top().condType << to_string(condStack.top().count) << endl << endl;
						out << "	END_" << condStack.top().condType << to_string(condStack.top().count) + ":" << endl;
					}
					else
					{
						out << "	END" << to_string(condStack.top().count) << ":" << endl;
					}
					
					GetToken();
				}
				else
				{
					cout << "ERROR in statement!!!" << endl;
					exit(0);
				}
			}
			else if (myToken.tokenValue == "while")
			{
				Conds.condType = myToken.tokenValue;
				Conds.count = whileCOunt;
				condStack.push(Conds);
				whileCond = true;

				//print while label
				out << "	TopWhile " << to_string(whileCOunt) << ": " << endl;

				whileCOunt++;
				GetToken();
				E();
				//do
				if (myToken.tokenValue == "do")
				{
					if (lastComp == "=")
					{
						out << "		JE " << condStack.top().condType << to_string(condStack.top().count) << endl;
						if (OrUsed)
						{
							out << "	OR" + to_string(condStack.top().count) + ":" << endl;
						}
						out << "		JMP END " << condStack.top().condType << to_string(condStack.top().count) << endl << endl;
						out << "	" + condStack.top().condType + to_string(condStack.top().count) + ":" << endl;
					}
					else if (lastComp == ">")
					{
						out << "		JG " << condStack.top().condType << to_string(condStack.top().count) << endl;
						if (OrUsed)
						{
							out << "	OR" + to_string(condStack.top().count) + ":" << endl;
						}
						out << "		JMP END" << condStack.top().condType << to_string(condStack.top().count) << endl << endl;
						out << "	" + condStack.top().condType + to_string(condStack.top().count) + ":" << endl;
					}
					else if (lastComp == "<")
					{
						out << "		JL " << condStack.top().condType << to_string(condStack.top().count) << endl;
						if (OrUsed) 
						{ 
							out << "	OR" + to_string(condStack.top().count) + ":" << endl;
						}
						out << "		JMP END" << condStack.top().condType << to_string(condStack.top().count) << endl << endl;
						out << "	" + condStack.top().condType + to_string(condStack.top().count) + ":" << endl;
					}
					GetToken();
					statement();
				}
				else
				{
					cout << "ERROR in while statement!!!" << endl;
					exit(0);
				}
			}
			else if (myToken.tokenValue == "if")
			{
				//push if to condstack
				Conds.condType = myToken.tokenValue;
				Conds.count = ifCount;
				condStack.push(Conds);
				ifCond = true;
				GetToken();
				E();
				if (DTstack.top() != "boolean")
				{
					cout << "ERROR not boolean!!!" << endl;
					exit(0);
				}
				//then
				if (myToken.tokenValue == "then")
				{
					if (lastComp == "=")
					{
						out << "		JE " << condStack.top().condType << to_string(condStack.top().count) << endl;
					}
					else if (lastComp == "<")
					{
						out << "		JL " << condStack.top().condType << to_string(condStack.top().count) << endl;
					}
					else if (lastComp == ">")
					{
						out << "		JG " << condStack.top().condType << to_string(condStack.top().count) << endl;
					}
					//check if go to OR and it is from an IF
					if (OrUsed)
					{
						out << "	OR" << to_string(condStack.top().count) << ": " << endl;
						out << "		JMP ELSE" << to_string(condStack.top().count) << endl;
						orCount++;
						OrUsed = false;
					}
					else
					{
						out << "		JMP ELSE" << to_string(condStack.top().count) << endl;
					}
					GetToken();
					statement();
					if (myToken.tokenValue == "else")
					{
						ifCond = false;
						GetToken();
						statement();
					}
				}
				else
				{
					cout << "ERROR in if statement!!!" << endl;
					exit(0);
				}
			}
		}
	}

	void consumeParam(string tempProN)
	{
		int totalParam = symbolTable.curScope->Table[tempProN].nextScope->paramOrder.size();
		int curParam = 0;
		while (curParam < totalParam)
		{
			string tempDT1;
			int tempOffSet;
			bool PbyRef;

			if (!symbolTable.getParamInfo(tempProN, curParam, tempDT1, PbyRef, tempOffSet))
			{
				cout << "Error in consumeParam" << endl;
				exit(0);
			}

			if (PbyRef == true)
			{
				if (myToken.tokenType == "words")
				{
					string tempDT2;
					CToken tempTk = myToken;
					int tempOffSet2;
					int tempVFPP;
					int tempSize;
					bool check;
					bool global;
					bool isArrCheck;
					if (!symbolTable.FindVariable(tempTk.tokenValue, tempOffSet2, tempDT2, tempVFPP,check, tempSize, global, isArrCheck))
					{
						cout << "Error in pass by reference" << endl;
						exit(1);
					}
					if (tempDT1 != tempDT2)
					{
						cout << "Error DataType did not match" << endl;
						exit(1);
					}
					tempTk = myToken;
					procStack.push("		mov " + regArr[curRegister].RegName + "," + to_string(tempOffSet2) + "\n" + "		add " + regArr[curRegister].RegName + ",ebp \n" + "		push " + regArr[curParam - 2].RegName + "\n");
					GetToken();
				}
				else
				{
					cout << "Error in consumeparam words" << endl;
					exit(0);
				}
			}
			else
			{
				E();
				procStack.push(asmStack.top() + "\n" + "		push " + regArr[curRegister - 1].RegName + "\n");
				if (tempDT1 != DTstack.top())
				{
					cout << "Error 69: dt dont match in param" << endl;
					exit(0);
				}
				else
				{
					DTstack.pop();
					asmStack.pop();
					if (curRegister > 0)
					{
						curRegister--;
					}
				}
			}
			curParam++;
			if (curParam < totalParam)
			{
				if (myToken.tokenValue == ",")
				{
					GetToken();
				}
				else
				{
					cout << "Error 99" << endl;
					exit(0);
				}
			}
		}
		for (int i = 0; i < totalParam; i++)
		{
			out << procStack.top()  << endl;
			procStack.pop();
		}
		out << endl;
		out << "		call " << tempProN << endl << endl;
	}



	void mstatement()
	{
		if (myToken.tokenValue == ";")
		{
			GetToken();
			statement();
			mstatement();
		}
	}

	void E()
	{
		S();
	}

	void S()
	{
		SR();
		SPrime();
	}

	void SR()
	{
		T();
	}

	void SPrime()
	{
		if (myToken.tokenValue == "<" || myToken.tokenValue == ">" || myToken.tokenValue == "=")
		{
			string tempToken = myToken.tokenValue;
			//set last comperison
			lastComp = tempToken;

			GetToken();
			SR();
			//9. Pop two datatypes off of the stack. Make sure you can do the current operation on the two datatypes. 
			//Put the new datatype onto the stack.
			string tempDT1 = DTstack.top();
			DTstack.pop();
			string tempDT2 = DTstack.top();
			DTstack.pop();
			if (tempDT1 == tempDT2)
			{
				DTstack.push("boolean");
				string tempAsm1 = asmStack.top();
				asmStack.pop();
				string tempAsm2 = asmStack.top();
				asmStack.pop();

				//print out
				out << "     " + tempAsm2 << endl;
				out << "     " + tempAsm1 << endl;
				out << "		cmp " + regArr[curRegister - 2].RegName + " , " + regArr[curRegister - 1].RegName << endl;
				if (curRegister > 0)
				{
					curRegister--;
				}
			}
			else
			{
				cout << "Error in comparing different type Sprime";
				exit(1);
			}
			SPrime();
		}
	}

	void T()
	{
		TR();
		Tprime();
	}

	void Tprime()
	{
		if (myToken.tokenValue == "+" || myToken.tokenValue == "-" || myToken.tokenValue == "or")
		{
			string SavedT = myToken.tokenValue;

			//check for or
			if (SavedT == "or")
			{
				lastToken = SavedT;

				if (lastComp == ">")
				{
					out << "		JG " << condStack.top().condType <<  to_string(condStack.top().count) << endl;
				}
				else if (lastComp == "<")
				{
					out << "		JL " << condStack.top().condType << to_string(condStack.top().count) << endl;
				}
				else if (lastComp == "=")
				{
					out << "		JE " << condStack.top().condType << to_string(condStack.top().count) << endl;
				}

				//print OR label if OrUsed is true
				if (OrUsed)
				{
					out << "	OR" + to_string(condStack.top().count) + ": " << endl;
					orCount++;
				}
				//set OrUsed back to false
				OrUsed = false;
				//orCount++;
			}

			GetToken();
			TR();
			//9. Pop two datatypes off of the stack. Make sure you can do the current operation on the two datatypes. 
			//Put the new datatype onto the stack.
			string tempDT1 = DTstack.top();
			DTstack.pop();
			string tempDT2 = DTstack.top();
			DTstack.pop();
			//need to check for int n float
			if (tempDT1 == tempDT2)
			{
				if (tempDT1 == "integer")
				{
					DTstack.push("integer");
					asmStack.push("");
					string tempAsm1 = asmStack.top();
					asmStack.pop();
					string tempAsm2 = asmStack.top();
					asmStack.pop();
					string temp3 = "";
					if (asmStack.size() >= 2)
					{
						temp3 = asmStack.top();
						asmStack.pop();
						asmStack.push("");
					}

					if (regArr[curRegister - 1].lit == false || regArr[curRegister - 2].lit == false || arr2Check)
					{
						//print out
						out << "	 " + temp3 << endl;
						out << "     " + tempAsm2 << endl;
						out << "     " + tempAsm1 << endl;
						asmStack.push("");
						if (SavedT == "+")
						{
							out << "		add " + regArr[curRegister - 2].RegName + " , " + regArr[curRegister - 1].RegName<<endl;
						}
						if (SavedT == "-")
						{
							out << "		sub " + regArr[curRegister - 2].RegName + " , " + regArr[curRegister - 1].RegName<<endl;
						}
					}
					else if (regArr[curRegister - 1].lit == true && regArr[curRegister - 2].lit == true && !arr2Check)
					{
						if (SavedT == "+")
						{
							regArr[curRegister - 2].val += regArr[curRegister - 1].val;
							asmStack.push("			mov " + regArr[curRegister - 1].RegName + ", " + to_string(regArr[curRegister - 1].val));
							
						}
						else if (SavedT == "-")
						{
							regArr[curRegister - 2].val -= regArr[curRegister - 1].val;
							asmStack.push("			mov " + regArr[curRegister - 1].RegName + ", " + to_string(regArr[curRegister - 1].val));
							
						}
					}
					if (curRegister > 0)
					{
						curRegister--;
					}
				}
				else if (tempDT1 == "boolean")
				{
					DTstack.push("boolean");
				}
				else if (tempDT1 == "real")
				{
					DTstack.push("real");
				}
				else
				{
					cout << "Error in checking two DT Tprime" << endl;
					exit(1);
				}
			}
			else
			{
				cout << "Error 06 in Tprime popping two datatype" << endl;
				exit(1);
			}
			Tprime();
		}
		else {}
	}

	void TR()
	{
		F();
	}

	void F()
	{
		FR();
		Fprime();
	}

	void Fprime()
	{
		if (myToken.tokenValue == "*" || myToken.tokenValue == "/" || myToken.tokenValue == "and")
		{
			string SavedT = myToken.tokenValue;
			lastToken = SavedT;
			
			if (lastToken == "and")
			{
				OrUsed = true;

				if (lastComp == "=")
				{
					out << "		JNE OR" << to_string(condStack.top().count) << endl;
				}
				else if (lastComp == ">")
				{
					out << "		JLE OR" << to_string(condStack.top().count) << endl;
				}
				else if (lastComp == "<")
				{
					out << "		JGE OR" << to_string(condStack.top().count) << endl;
				}
			}

			GetToken();
			FR();
			//9. Pop two datatypes off of the stack. Make sure you can do the current operation on the two datatypes. 
			//Put the new datatype onto the stack.
			string tempDT1 = DTstack.top();
			DTstack.pop();
			string tempDT2 = DTstack.top();
			DTstack.pop();
			//need to check for int n float
			if (tempDT1 == tempDT2)
			{
				if (tempDT1 == "integer")
				{
					DTstack.push("integer");

					string tempAsm1 = asmStack.top();
					asmStack.pop();
					string tempAsm2 = asmStack.top();
					asmStack.pop();

					if (regArr[curRegister].lit == false || regArr[curRegister - 1].lit == false)
					{
						out << tempAsm2 << endl;
						out << tempAsm1 << endl << endl;;
						asmStack.push("");

						if (SavedT == "*")
						{
							out << "		imul " + regArr[curRegister - 1].RegName + " , " + regArr[curRegister].RegName << endl;

							out << "		mov " << regArr[curRegister].RegName << "," << regArr[curRegister].val << endl;
						}
						else if (SavedT == "/")
						{
							out << "		push edx" << endl;
							out << "		cdq" << endl;

							out << "		idiv " << regArr[curRegister].RegName << endl;
							out << "		pop edx";
						}
					}
					else if (regArr[curRegister].lit == true && regArr[curRegister - 1].lit == true)
					{
						if (SavedT == "*")
						{
							regArr[curRegister - 2].val *= regArr[curRegister - 1].val;
							asmStack.push("			mov " + regArr[curRegister - 1].RegName + " , " + to_string(regArr[curRegister - 1].val));
							
						}
						else if (SavedT == "/")
						{
							regArr[curRegister - 1].val /= regArr[curRegister].val;
							asmStack.push("			mov " + regArr[curRegister - 1].RegName + " , " + to_string(regArr[curRegister - 1].val));
						}
					}
					if (curRegister > 0)
					{
						curRegister--;
					}
				}
				else if (SavedT == "real")
				{
					DTstack.push("real");
				}
				else if (SavedT == "and")
				{
					DTstack.push("boolean");
				}
				else
				{
					cout << "Error in checking two DT Fprime" << endl;
				}
			}
			else
			{
				cout << "Error 07 in Fprime popping two datatype" << endl;
			}
			Fprime();
		}
		else {}
	}

	void FR()
	{
		if (myToken.tokenType == "real" || myToken.tokenType == "float")
		{
			regArr[curRegister].lit = true;
			regArr[curRegister].val = stoi(myToken.tokenValue);
			asmStack.push("		mov " + regArr[curRegister].RegName + ", " + to_string(regArr[curRegister].val));

			GetToken();
			DTstack.push("real");
			curRegister++;
		}
		else if (myToken.tokenType == "integer")
		{
			regArr[curRegister].lit = true;
			regArr[curRegister].val = stoi(myToken.tokenValue);
			asmStack.push("		mov " + regArr[curRegister].RegName + ", " + to_string(regArr[curRegister].val));

			GetToken();
			DTstack.push("integer");
			curRegister++;
		}
		else if (myToken.tokenValue == "+" || myToken.tokenValue == "-")
		{
			GetToken();
			TR();
		}
		else if (myToken.tokenValue == "(")
		{
			GetToken();
			E();
			if (myToken.tokenValue == ")")
			{
				GetToken();
			}
			else
			{
				cout << "BAD!!! in FR" << endl;
				system("pause");
				exit(0);
			}
		}
		else if (myToken.tokenType == "words")
		{
			if (myToken.tokenValue == "true" || myToken.tokenValue == "false")
			{
				DTstack.push("boolean");
				GetToken();
			}
			else
			{
				//8.Check to see if the word is in the symbol table.(FindVariable) Push datatype onto the stack.
				int tempOffset;
				int tempVFPP;
				int tempSiZe;
				string tempDT;
				bool check;
				bool global;
				bool isArrCheck;
				string tempName;
				if (symbolTable.FindVariable(myToken.tokenValue, tempOffset, tempDT, tempVFPP,check, tempSiZe, global, isArrCheck))
				{
					tempName = myToken.tokenValue;
					if (isArrCheck == true )
					{
						DTstack.push("integer");
						GetToken();
						if (myToken.tokenValue == "[")
						{
							arrCheck = isArrCheck;
							GetToken();
							E();
							if (myToken.tokenValue == ",")
							{
								arr2Check = true;
							}
							else
							{
								arr2Check = false;
							}
							if (arr2Check == true)
							{
								
								if (myToken.tokenValue == ",")
								{
									GetToken();
									E();
									
								}
								if (myToken.tokenValue == "]")
								{
									symbolTable.getArrInfo(tempName, arr2Val1, arr2Val2, arrVal1, arrVal2);
									string tempStruct1 = asmStack.top();
									asmStack.pop();
									string tempStruct2 = asmStack.top();
									asmStack.pop();

									out << tempStruct2 << endl;
									out << tempStruct1 << endl;
									//OUT ASM
									out << "		sub " + regArr[curRegister - 2].RegName + ", " + to_string(arr2Val1) << endl;
									//check for dt
									if (DTname == "char" || DTname == "boolean")
									{
										out << "		imul " + regArr[curRegister - 2].RegName + ", " + to_string(((arr2Val2 - arr2Val1) + 1) * 1) << endl;
									}
									else if (DTname == "integer")
									{
										out << "		imul " + regArr[curRegister - 2].RegName + ", " + to_string(((arr2Val2 - arr2Val1) + 1) * 4) << endl;
									}

									out << "		sub " + regArr[curRegister - 1].RegName + ", " + to_string(arr2Val2) << endl;

									if (DTname == "integer") {
										out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(4) << endl;
									}
									else if (DTname == "char" || DTname == "boolean") {

										out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(1) << endl;
									}
									if (curRegister > 0)
									{
										curRegister--;
									}
									out << "		add " + regArr[curRegister - 1].RegName + ", " + regArr[curRegister].RegName << endl;

									out << "		add " + regArr[curRegister - 1].RegName + ", " + to_string(tempOffset) << endl;
									out << "		mov " + regArr[curRegister - 1].RegName + ", [ebp+ " + regArr[curRegister - 1].RegName + "]" << endl;
									GetToken();
								}
							}
							else if (myToken.tokenValue == "]")
							{
								if (asmStack.size() == 0) {
									asmStack.push("	");
								}
								string tempStruct = asmStack.top();
								out << tempStruct << endl;
								asmStack.pop();

								symbolTable.getArrInfo(tempName, arr2Val1, arr2Val2, arrVal1, arrVal2);
								//OUT ASM
								out << "		sub " + regArr[curRegister - 1].RegName + ", " + to_string(arrVal1) << endl;
								//check for dt
								if (DTname == "char" || DTname == "boolean")
								{
									out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(1) << endl;
								}
								else if (DTname == "integer")
								{
									out << "		imul " + regArr[curRegister - 1].RegName + ", " + to_string(4) << endl;
								}
								out << "		add " + regArr[curRegister - 1].RegName + ", " + to_string(tempOffset) << endl;

								out << "		mov " + regArr[curRegister - 1].RegName + " , " + "[ebp + " + regArr[curRegister - 1].RegName + "]" << endl;

								GetToken();
								arr2Check = false;
							}
							else
							{
								cout << "missing ] in arr statement" << endl;
								exit(0);
							}
						}
					}
					
					else
					{
						if (tempVFPP <= 1) //CHECK GLOBAL OR LOCAL PARAM OR VAR
						{
							DTstack.push(tempDT);

							//IF GLOBAL
							if (global == true)
							{
								regArr[curRegister].lit = false;
								regArr[curRegister].val = NULL;
								asmStack.push("	mov " + regArr[curRegister].RegName + " , " + "[ebp + " + to_string(tempOffset) + "]");
								out << endl;
								curRegister++;
							}
							//ELSE PARAM OR VAR
							else
							{
								if (tempVFPP == 0) //var
								{
									asmStack.push("		mov " + regArr[curRegister].RegName + " , " + "[edi - " + to_string(tempOffset + tempSiZe) + "]");
									curRegister++;
								}
								else if (tempVFPP == 1) //param
								{
									//ref
									if (check == true)
									{

										regArr[curRegister].lit = false;
										asmStack.push("		mov esi, [edi +" + to_string(tempOffset) + "]" "\n" "		mov " + regArr[curRegister].RegName + " , " + "[esi]");
										
										curRegister++;
									}
									//copy
									else
									{
										asmStack.push("	mov " + regArr[curRegister].RegName + " , " + "[edi + " + to_string(tempOffset) + "]");
										curRegister++;
									}
								}
							}
							GetToken();
						}
						else if (tempVFPP == 3) //a function
						{
							CToken tempToken = myToken;
							GetToken();
							if (myToken.tokenValue == "(")
							{
								GetToken();
								consumeParam(tempToken.tokenValue);
								if (myToken.tokenValue == ")")
								{
									DTstack.push(symbolTable.curScope->Table[tempToken.tokenValue].nextScope->returnType);
									GetToken();
								}
								else
								{
									cout << "Error, need ) FR1" << endl;
									exit(0);
								}
							}
							else
							{
								cout << "Error, need ) FR2" << endl;
								exit(0);
							}
						}
					}
					

				}
				else
				{
					cout << "Error in pushing datatype to stack in FR" << endl;
					exit(0);
				}
			}
		}
	
		else
		{
			cout << "BAD!!! end of FR" << endl;
			system("pause");
			exit(0);
		}
	}

	void PFV()
	{
		if (myToken.tokenType == "PFV-rword")
		{
			if (myToken.tokenValue == "var")
			{
				GetToken();
				if (myToken.tokenType == "words")
				{
					//1. store word in parsers vector
					VPvector.push_back(myToken.tokenValue);
					GetToken();
					Varlist();
					if (myToken.tokenValue == ":")
					{
						GetToken();
						DataType();
						//2. take words from vector n add them to symboltable, clear vector
						//name, dt, size
						for (int i = 0; i < VPvector.size(); i++)
						{
							if (arrCheck == true)
							{
								if (!symbolTable.AddArray(VPvector[i], DTname, DTsize, arrCheck, arr2Val1, arr2Val2, arrVal1, arrVal2))
								{
									cout << "Error in adding words to array" << endl;
									exit(0);
								}
							}
							else
							{
								if (!symbolTable.AddVariable(VPvector[i], DTname, DTsize, arrCheck, start1, start2, end1, end2))
								{
									cout << "Error in adding words to vector" << endl;
									exit(0);
								}
								arrCheck = false;
							}
						}
						VPvector.clear();
						if (myToken.tokenValue == ";")
						{
							GetToken();
							MVar();
							PFV();
						}
						else
						{
							cout << "ERROR in PFV-words ; !!!" << endl;
							exit(0);
						}
					}
					else
					{
						cout << "ERROR in PFV-words : !!!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "ERROR in Var-Words!!!" << endl;
					exit(0);
				}
				arr2Check = false;
			}
			else if (myToken.tokenValue == "procedure")
			{
				GetToken();
				out << myToken.tokenValue << " :  //  top of the procedure " << endl;

				if (myToken.tokenType == "words")
				{
					//3. add procedure to symbolTable
					if (!symbolTable.AddProcFunc(myToken.tokenValue, "procedure"))
					{
						cout << "Error in checking ProcFunc" << endl;
						exit(1);
					}
					GetToken();
					if (myToken.tokenValue == "(")
					{
						GetToken();
						Param();
						if (myToken.tokenValue == ")")
						{
							GetToken();
							if (myToken.tokenValue == ";")
							{
								GetToken();
								block();
								if (myToken.tokenValue == ";")
								{
									//6. back out of scope in symbolTable
									symbolTable.BackOut();
									GetToken();
									PFV();
								}
								else
								{
									cout << "ERROR in procedure1 ; !!!" << endl;
									exit(0);
								}
							}
							else
							{
								cout << "ERROR in procedure2 ; !!!" << endl;
								exit(0);
							}
						}
						else
						{
							cout << "ERROR in procedure ) !!!" << endl;
							exit(0);
						}
					}
					else
					{
						cout << "ERROR in procedure ( !!!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "ERROR in procedure words !!!" << endl;
					exit(0);
				}
			}
			else if (myToken.tokenValue == "function")
			{
				GetToken();
				if (myToken.tokenType == "words")
				{
					//3. add procedure to symbolTable
					symbolTable.AddProcFunc(myToken.tokenValue, "function");
					string funcTempName = myToken.tokenValue;
					GetToken();
					if (myToken.tokenValue == "(")
					{
						GetToken();
						Param();
						if (myToken.tokenValue == ")")
						{
							GetToken();
							if (myToken.tokenValue == ":")
							{
								GetToken();
								DataType();
								//7. set return type function in symbolTable
								symbolTable.curScope->returnType = DTname;
								if (myToken.tokenValue == ";")
								{
									GetToken();
									block();
									if (myToken.tokenValue == ";")
									{
										//6. back out of scope in symbolTable
										symbolTable.BackOut();
										GetToken();
										PFV();
									}
									else
									{
										cout << "ERROR in function-words ; inside!!!" << endl;
										exit(0);
									}
								}
								else
								{
									cout << "ERROR in function-words ; !!!" << endl;
									exit(0);
								}
							}
							else
							{
								cout << "ERROR in function-words : !!!" << endl;
								exit(0);
							}
						}
						else
						{
							cout << "ERROR in function-words ) !!!" << endl;
							exit(0);
						}
					}
					else
					{
						cout << "ERROR in function-words ( !!!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "ERROR in function-words!!!" << endl;
					exit(0);
				}
			}
			else
			{
				cout << "ERROR in PFV!!!" << endl;
				exit(0);
			}
		}
		//else PFV to epsilon
	}

	void Varlist()
	{
		if (myToken.tokenValue == ",")
		{
			GetToken();
			if (myToken.tokenType == "words")
			{
				//1. store word in parsers vector
				VPvector.push_back(myToken.tokenValue);
				GetToken();
				Varlist();
			}
			else
			{
				cout << "ERROR in Varlist words !!!" << endl;
				exit(0);
			}
		}
		//else Varlist Epsilon
	}

	void MVar()
	{
		if (myToken.tokenType == "words")
		{
			//1. store word in parsers vector
			VPvector.push_back(myToken.tokenValue);
			GetToken();
			Varlist();
			if (myToken.tokenValue == ":")
			{
				GetToken();
				DataType();
				//2. take words from vec add to symbolTable. AddVar, clear vec
				for (int i = 0; i < VPvector.size(); i++)
				{
					if (!symbolTable.AddVariable(VPvector[i], DTname, DTsize, arr2Check, start1, start2, end1, end2))
					{
						cout << "Error in adding words to vector" << endl;
						exit(0);
					}
				}
				VPvector.clear();
				if (myToken.tokenValue == ";")
				{
					GetToken();
					MVar();
				}
				else
				{
					cout << "Error need ;" << endl;
					exit(1);
				}
			}
			else
			{
				cout << "ERROR in Mvar : !!!" << endl;
				exit(0);
			}
		}
		//else Mvar to Epsilon
	}

	void Param()
	{
		if (myToken.tokenType == "words")
		{
			//1. store word in parsers vector
			VPvector.push_back(myToken.tokenValue);
			GetToken();
			Varlist();
			if (myToken.tokenValue == ":")
			{
				GetToken();
				DataType();
				PassByRef = false;
				//5.take words from vec add to symbol table. addParam, clear vec, passbyref=false
				for (int i = 0; i < VPvector.size(); i++)
				{
					//symbolTable.AddVariable(VPvector[i], DTname, DTsize);
					symbolTable.AddParam(VPvector[i], DTname, PassByRef);
				}

				VPvector.clear();
				Mparam();
			}
			else
			{
				cout << "ERROR in Param : !!!" << endl;
				exit(0);
			}
		}
		else if (myToken.tokenType == "PFV-rword")
		{
			if (myToken.tokenValue == "var")
			{
				GetToken();
				if (myToken.tokenType == "words")
				{
					VPvector.push_back(myToken.tokenValue);
					GetToken();
					Varlist();
					if (myToken.tokenValue == ":")
					{
						GetToken();
						DataType();
						for (int i = 0; i < VPvector.size(); i++)
						{
							if (!symbolTable.AddParam(VPvector[i], DTname, PassByRef))
							{
								cout << "Error in adding words to vector in param" << endl;
								exit(1);
							}
						}
						VPvector.clear();
						PassByRef = false;
						Mparam();
					}
					else
					{
						cout << "ERROR in Param : !!!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "ERROR in Param words !!!" << endl;
					exit(0);
				}
			}
			else
			{
				cout << "ERROR in Param var !!!" << endl;
				exit(0);
			}
		}
		//else Param to Epsion
	}

	void Mparam()
	{
		if (myToken.tokenValue == ";")
		{
			GetToken();
			Ovar();
			if (myToken.tokenType == "words")
			{
				//1. store word in parsers vector
				VPvector.push_back(myToken.tokenValue);
				GetToken();
				Varlist();
				if (myToken.tokenValue == ":")
				{
					GetToken();
					DataType();
					//5. take words from vec add to symbol table. addParam, clear vec, passbyref=false
					for (int i = 0; i < VPvector.size(); i++)
					{
						if (!symbolTable.AddParam(VPvector[i], DTname, PassByRef))
						{
							cout << "Error in adding words to vector in Mparam" << endl;
							exit(0);
						}
					}
					PassByRef = false;
					VPvector.clear();
					Mparam();
				}
				else
				{
					cout << "ERROR in Mparam : !!!" << endl;
					exit(0);
				}
			}
			else
			{
				cout << "ERROR in Mparam word !!!" << endl;
				exit(0);
			}
		}
	}

	void Ovar()
	{
		if (myToken.tokenValue == "var")
		{
			//4.set by ref to true
			PassByRef = true;
			GetToken();
		}
		//else Ovar to Epsilon
	}

	void DataType()
	{
		if (myToken.tokenType == "dataType")
		{
			DTname = myToken.tokenValue;
			if (DTname == "integer")
			{
				DTsize = 4;
			}
			else if (DTname == "char" || DTname == "boolean")
			{
				DTsize = 1;
			}
			else if (DTname == "array")
			{
				arrCheck = true;
				GetToken();
				DIM();
				if (myToken.tokenValue == "of")
				{
					GetToken();
					SimDT();
				}
			}
			GetToken();
		}
		else
		{
			cout << "ERROR in DataType!!!" << endl;
			exit(0);
		}
	}

	void SimDT()
	{
		if (myToken.tokenType == "dataType")
		{
			DTname = myToken.tokenValue;
			if (DTname == "integer")
			{
				//2D arr
				if (arr2Check)
				{
					//end-start +1 * 4
					DTsize = ((arr2Val2 - arr2Val1) + 1)* ((arrVal2 - arrVal1) + 1) * 4;
				}
				//1D arr
				else if (!arr2Check && arrCheck)
				{
					DTsize = ((arrVal2 - arrVal1) + 1) * 4;
				}
				
				
			}
			else if (DTname == "char" || DTname == "boolean")
			{
				DTsize = 1;
			}
		}
		else
		{
			cout << "ERROR in SIMDT!!!" << endl;
			exit(0);
		}
	}

	void DIM()
	{
		if (myToken.tokenValue == "[")
		{
			GetToken();
			if (myToken.tokenType == "integer")
			{
				arrVal1 = stoi(myToken.tokenValue);
				GetToken();
				if (myToken.tokenValue == "..")
				{
					GetToken();
					if (myToken.tokenType == "integer")
					{
						arrVal2 = stoi(myToken.tokenValue);
						GetToken();
						if (myToken.tokenValue == "]")
						{
							arrCheck = true;
							GetToken();
						}
						else if (myToken.tokenValue == ",")
						{
							arr2Check = true;
							DIM2();
						}
						else
						{
							cout << "arr missing ]" << endl;
							exit(0);
						}
					}
					else
					{
						cout << "arr missing second val" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "arr missing .." << endl;
					exit(0);
				}	
			}
			else
			{
				cout << "arr did not get any int" << endl;
				exit(0);
			}
		}
		else
		{
			cout << "did not get [ in arr" << endl;
		}
	}

	void DIM2()
	{
		if (arr2Check == true && myToken.tokenValue == ",")
		{
			//2D array
			GetToken();
			if (myToken.tokenType == "integer")
			{
				arr2Val1 = stoi(myToken.tokenValue);
				GetToken();
				if (myToken.tokenValue == "..")
				{
					GetToken();
					if (myToken.tokenType == "integer")
					{
						arr2Val2 = stoi(myToken.tokenValue);
						GetToken();
						if (myToken.tokenValue == "]")
						{
							GetToken();
						}
						else
						{
							cout << "arr missing ]" << endl;
							exit(0);
						}
					}
					else
					{
						cout << "arr missing second val" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "arr missing .." << endl;
					exit(0);
				}
			}
			else
			{
				cout << "arr did not get any int" << endl;
				exit(0);
			}
		}
		else
		{
			cout << "Error in DIM2 missing ," << endl;
			exit(0);
		}
	}

	void printHead()
	{
		out.open("assembler.cpp");

		out << "#include <iostream>" << endl;
		out << "using namespace std;" << endl;
		out << "char DataSegment[65536];" << endl;
		out << "int *look;" << endl;
		out << "int main()" << endl;
		out << "{" << endl;

		out << "look = (int*)DataSegment;" << endl;

		out << "	_asm{" << endl;
		out << "		push eax         // Assembler header for all programs (Top)" << endl;
		out << "		push ebx" << endl;
		out << "		push ecx" << endl;
		out << "		push edx" << endl;
		out << "		push ebp" << endl;
		out << "		push edi" << endl;
		out << "		push esi" << endl;
		out << "		push esp" << endl << endl;
		out << "		lea ebp, DataSegment" << endl;
		out << "		jmp kmain       // Assembler header for all programs (Bottom)" << endl << endl << endl;
	}
	void printFoot()
	{
		out << endl;
		out << "		pop esp    // Assembler footer for all programs (Top)" << endl;
		out << "		pop esi" << endl;
		out << "		pop edi" << endl;
		out << "		pop ebp" << endl;
		out << "		pop edx" << endl;
		out << "		pop ecx" << endl;
		out << "		pop ebx" << endl;
		out << "		pop eax    // Assembler footer for all programs (Bottom)" << endl;
		out << "	}" << endl;
		out << "return 0;" << endl;
		out << "}" << endl;

		out.close();
	}

	bool printParamHead()
	{
		if (symbolTable.curScope->prevScope)
		{
			out << "		push edi" << endl;
			out << "		mov edi,esp" << endl;
			out << "		sub esp," << symbolTable.trackCount() << endl <<endl;
			out << "		push eax" << endl;
			out << "		push ebx" << endl;
			out << "		push ecx" << endl;
			out << "		push edx" << endl;
			out << "		push ebp" << endl;
			out << "		push edi" << endl;
			out << "		push esi" << endl;
			out << "		push esp" << endl << endl << endl;;
			return true;
		}
		return false;
	}

	void printParamFoot()
	{
		out << "		pop esp" << endl;
		out << "		pop esi" << endl;
		out << "		pop edi" << endl;
		out << "		pop ebp" << endl;
		out << "		pop edx" << endl;
		out << "		pop ecx" << endl;
		out << "		pop ebx" << endl;
		out << "		pop eax" << endl << endl;
		out << "		add esp," << symbolTable.trackCount() << endl;
		out << "		pop edi" << endl;
		out << "		ret " << symbolTable.curScope->paramOrder.size() * 4 << endl << endl;
	}
};

