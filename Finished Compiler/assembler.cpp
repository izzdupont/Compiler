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




	kmain:   //  main function 

		mov eax, 5
		mov [ebp + 0] , eax


		mov eax, 5
	mov ebx , [ebp + 0]
		sub eax, 3
		imul eax, 12
		sub ebx, 5
		imul ebx, 4
		add eax, ebx
		add eax, 12

		mov ebx, 10
		mov [ebp + eax],ebx


	 	mov ebx , [ebp + 0]
     		mov ecx, 1
     
		add ebx , ecx


		sub eax, 3
		imul eax, 12
		sub ebx, 5
		imul ebx, 4
		add eax, ebx
		add eax, 12


	mov ebx , [ebp + 0]
	mov ecx , [ebp + 0]
		sub ebx, 3
		imul ebx, 12
		sub ecx, 5
		imul ecx, 4
		add ebx, ecx
		add ebx, 12
		mov ebx, [ebp+ ebx]

	mov eax , [ebp + 0]
		mov [ebp + eax],ebx

		pop esp    // Assembler footer for all programs (Top)
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax    // Assembler footer for all programs (Bottom)
	}
return 0;
}
