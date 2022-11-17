#include <iostream>
using namespace std;
char DataSegment[65536];
int *look;
int main()
{
look = (int*)DataSegment;
	_asm{
		push eax         // Assembler header for all programs (Top)
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
		push esp

		lea ebp, DataSegment
		jmp kmain       // Assembler header for all programs (Bottom)


		push edi
		mov edi,esp
		sub esp,4
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
		push esp


