Table o' contents:

[Crushing](#crushing)

[Pre-reversing](#pre-reversing)

[File Properties](#file-properties)

[Symbol analysis](#symbol-analysis)

[Program interraction](#program-interraction)

[Reversing](#reversing)

[Main](#main)

[add_char_to_map](#add_char_to_map)

[Serialize_and_output](#serialize_and_output)

[List Len](#list-len)

[Full C code](#full-c-code)

[Solution](#solution)

# Crushing
**Crushing** is a an easy HackTheBox Challenge of the "Reversing" category. You are given a binary file that encrypts your input and outputs it to stdout, and a file containing a message encoded with
the encoded algorithm. You are to reverse the algorithm and decrypt the message.

## Pre-reversing
Before I jump into the file's code, I always try to squeeze out as much information about the exec file as I can.

### File Properties
The first step of pre-reversing for me is the _file_ command:
```bash
file crush
```
```bash
crush: ELF 64-bit LSB pie executable, 
x86-64, 
version 1 (SYSV), 
dynamically linked, 
interpreter /lib64/ld-linux-x86-64.so.2, 
BuildID[sha1]=fdbeaffc5798388ef49c3f3716903d37a53e8e03, 
for GNU/Linux 3.2.0, 
not stripped
```
What this tells me is:
* The exec is build for my architecture in mind ( not some embedded device!)
* it uses dynamically linked functions -> I can find something juicy with a library call trace!
* The binary is not stripped -> objdump, nm and gdb will easily help to analyse the binary.

### Symbol analysis
The best result I got was from _objdump_:
```bash
objdump -M intel -d crush

0000000000001155 <add_char_to_map>:                                                                                  
00000000000011f6 <list_len>:
000000000000124a <serialize_and_output>:
00000000000012ff <main>:
```
This tells me that these 4 functions were defined by the programmer, not the system.

### Program interraction
As far as for trying to use the program blind, I have not found any luck there. The program itself did not respond to my input while library call traces only showed me some getchar() and malloc()
calls.

## Reversing
In order to practice my Assembly a bit, I hand-decompiled the binary, using gdb as a debugger/ intel assembly dumper.

### Main 
#### Assembly Code
```bash
(gdb) disassemble main
Dump of assembler code for function main:
1  0x00000000000012ff <+0>:     push   rbp
   0x0000000000001300 <+1>:     mov    rbp,rsp
   0x0000000000001303 <+4>:     sub    rsp,0x810
   0x000000000000130a <+11>:    lea    rdx,[rbp-0x810]
   0x0000000000001311 <+18>:    mov    eax,0x0
   0x0000000000001316 <+23>:    mov    ecx,0xff
   0x000000000000131b <+28>:    mov    rdi,rdx
   0x000000000000131e <+31>:    rep stos QWORD PTR es:[rdi],rax
2  0x0000000000001321 <+34>:    mov    QWORD PTR [rbp-0x8],0x0
   0x0000000000001329 <+42>:    jmp    0x134b <main+76>
4  0x000000000000132b <+44>:    mov    eax,DWORD PTR [rbp-0xc]
   0x000000000000132e <+47>:    movzx  ecx,al
   0x0000000000001331 <+50>:    mov    rdx,QWORD PTR [rbp-0x8]
   0x0000000000001335 <+54>:    lea    rax,[rbp-0x810]
   0x000000000000133c <+61>:    mov    esi,ecx
   0x000000000000133e <+63>:    mov    rdi,rax
   0x0000000000001341 <+66>:    call   0x1155 <add_char_to_map>
   0x0000000000001346 <+71>:    add    QWORD PTR [rbp-0x8],0x1
3  0x000000000000134b <+76>:    call   0x1030 <getchar@plt>
   0x0000000000001350 <+81>:    mov    DWORD PTR [rbp-0xc],eax
   0x0000000000001353 <+84>:    cmp    DWORD PTR [rbp-0xc],0xffffffff
   0x0000000000001357 <+88>:    jne    0x132b <main+44>
5  0x0000000000001359 <+90>:    lea    rax,[rbp-0x810]
   0x0000000000001360 <+97>:    mov    rdi,rax
   0x0000000000001363 <+100>:   call   0x124a <serialize_and_output>
   0x0000000000001368 <+105>:   mov    eax,0x0
   0x000000000000136d <+110>:   leave
   0x000000000000136e <+111>:   ret
End of assembler dump.
```

##### 1-2
```bash
   0x00000000000012ff <+0>:     push   rbp
   0x0000000000001300 <+1>:     mov    rbp,rsp
   0x0000000000001303 <+4>:     sub    rsp,0x810
   0x000000000000130a <+11>:    lea    rdx,[rbp-0x810]
   0x0000000000001311 <+18>:    mov    eax,0x0
   0x0000000000001316 <+23>:    mov    ecx,0xff
   0x000000000000131b <+28>:    mov    rdi,rdx
   0x000000000000131e <+31>:    rep stos QWORD PTR es:[rdi],rax
```
This is the first part of the main function, where variables on the stack are referenced.
What I don't commonly see is the [rep stos instruction][https://www.youtube.com/watch?v=W33kNdh9gxY], which basically nulls sequentially a large space of memory.
The C equivallent:
```bash
	long character_map[256] = {0}	//[rbp-0x810]
```

##### 2-3 (jump)
```bash
   0x000000000000131e <+31>:    rep stos QWORD PTR es:[rdi],rax
   0x0000000000001321 <+34>:    mov    QWORD PTR [rbp-0x8],0x0
   0x0000000000001329 <+42>:    jmp    0x134b <main+76>
   0x000000000000134b <+76>:    call   0x1030 <getchar@plt>
   0x0000000000001350 <+81>:    mov    DWORD PTR [rbp-0xc],eax
```
The program here first initializes some 8 byte value on the stack, then jumps, calls getchar, and allocates the result on the stack also.
The C equivallent;
```bash
	long characters_registered; 	//[rbp-0x8]
	int character;					      //[rbp-0xc]
	characters_registered = 0;

  character = getchar();
```

##### 3
```bash
   0x0000000000001350 <+81>:    mov    DWORD PTR [rbp-0xc],eax
   0x0000000000001353 <+84>:    cmp    DWORD PTR [rbp-0xc],0xffffffff
   0x0000000000001357 <+88>:    jne    0x132b <main+44>
```
This code compares the value on the stack (getchar result) to -1, or EOF.
In order to understand this more, we should check what could happen after the JNE jump.

##### 3(4)
```bash
   0x0000000000001357 <+88>:    jne    0x132b <main+44>
   0x000000000000132b <+44>:    mov    eax,DWORD PTR [rbp-0xc]
   0x000000000000132e <+47>:    movzx  ecx,al
   0x0000000000001331 <+50>:    mov    rdx,QWORD PTR [rbp-0x8]
   0x0000000000001335 <+54>:    lea    rax,[rbp-0x810]
   0x000000000000133c <+61>:    mov    esi,ecx
   0x000000000000133e <+63>:    mov    rdi,rax
   0x0000000000001341 <+66>:    call   0x1155 <add_char_to_map>
   0x0000000000001346 <+71>:    add    QWORD PTR [rbp-0x8],0x1
   0x000000000000134b <+76>:    call   0x1030 <getchar@plt>
```
This is code ends at the getchar() instruction we saw earlier. The bulk of this code is spent as a set up for the add_char_to_map function.
The arguments:
* first argument: pointer to the rep stos nulled stack memory.
* second argument: the charcode returned from getchar()
* third argument: value on the stack that was nulled before (we have seen it)
After the call, the value that was nulled on the stack is incremented.
I believe it is not a sin to assume that this mystery value keeps track of the characters read from the user.
The C equivallent:
```bash
    add_char_to_map(character_map, (long) character, characters_registered);
		++characters_registered;
```

##### 3(5)
```bash
   0x0000000000001357 <+88>:    jne    0x132b <main+44>
   0x0000000000001359 <+90>:    lea    rax,[rbp-0x810]
   0x0000000000001360 <+97>:    mov    rdi,rax
   0x0000000000001363 <+100>:   call   0x124a <serialize_and_output>
   0x0000000000001368 <+105>:   mov    eax,0x0
   0x000000000000136d <+110>:   leave
   0x000000000000136e <+111>:   ret
```
This calls serialize_and_output with the pointer to the stack memory space and exits.

##### full 3
Now we can truly recreate what happens at 3:
```bash
	while (1){
		character = getchar();
		if ( character == EOF){
			break;
		}
		add_char_to_map(character_map, (long) character, characters_registered);
		++characters_registered;
	}
		
	serialize_and_output(character_map);
	
	return 0;
```
#### C code
```bash
int main(void){
	long characters_registered; 	//[rbp-0x8]
	long character_map[256] = {0}	//[rbp-0x810]
	int character;					//[rbp-0xc]
	
	characters_registered = 0;
	
	while (1){
		character = getchar();
		if ( character == EOF){
			break;
		}
		add_char_to_map(character_map, (long) character, characters_registered);
		++characters_registered;
	}
		
	serialize_and_output(character_map);
	
	return 0;
}
```
### add_char_to_map 
#### Assembly code
```bash
(gdb) disassemble add_char_to_map 
Dump of assembler code for function add_char_to_map:
   0x0000555555555155 <+0>:     push   rbp
   0x0000555555555156 <+1>:     mov    rbp,rsp
   0x0000555555555159 <+4>:     sub    rsp,0x30
   0x000055555555515d <+8>:     mov    QWORD PTR [rbp-0x18],rdi
   0x0000555555555161 <+12>:    mov    eax,esi
   0x0000555555555163 <+14>:    mov    QWORD PTR [rbp-0x28],rdx
   0x0000555555555167 <+18>:    mov    BYTE PTR [rbp-0x1c],al
   0x000055555555516a <+21>:    movzx  eax,BYTE PTR [rbp-0x1c]
   0x000055555555516e <+25>:    lea    rdx,[rax*8+0x0]
   0x0000555555555176 <+33>:    mov    rax,QWORD PTR [rbp-0x18]
   0x000055555555517a <+37>:    add    rax,rdx
   0x000055555555517d <+40>:    mov    rax,QWORD PTR [rax]
   0x0000555555555180 <+43>:    mov    QWORD PTR [rbp-0x8],rax
   0x0000555555555184 <+47>:    mov    edi,0x10
   0x0000555555555189 <+52>:    call   0x555555555040 <malloc@plt>
   0x000055555555518e <+57>:    mov    QWORD PTR [rbp-0x10],rax
   0x0000555555555192 <+61>:    mov    rax,QWORD PTR [rbp-0x10]
   0x0000555555555196 <+65>:    mov    rdx,QWORD PTR [rbp-0x28]
   0x000055555555519a <+69>:    mov    QWORD PTR [rax],rdx
   0x000055555555519d <+72>:    mov    rax,QWORD PTR [rbp-0x10]
   0x00005555555551a1 <+76>:    mov    QWORD PTR [rax+0x8],0x0
   0x00005555555551a9 <+84>:    cmp    QWORD PTR [rbp-0x8],0x0
   0x00005555555551ae <+89>:    je     0x5555555551d9 <add_char_to_map+132>
   0x00005555555551b0 <+91>:    jmp    0x5555555551be <add_char_to_map+105>
   0x00005555555551b2 <+93>:    mov    rax,QWORD PTR [rbp-0x8]
   0x00005555555551b6 <+97>:    mov    rax,QWORD PTR [rax+0x8]
   0x00005555555551ba <+101>:   mov    QWORD PTR [rbp-0x8],rax
   0x00005555555551be <+105>:   mov    rax,QWORD PTR [rbp-0x8]
   0x00005555555551c2 <+109>:   mov    rax,QWORD PTR [rax+0x8]
   0x00005555555551c6 <+113>:   test   rax,rax
   0x00005555555551c9 <+116>:   jne    0x5555555551b2 <add_char_to_map+93>
   0x00005555555551cb <+118>:   mov    rax,QWORD PTR [rbp-0x8]
   0x00005555555551cf <+122>:   mov    rdx,QWORD PTR [rbp-0x10]
   0x00005555555551d3 <+126>:   mov    QWORD PTR [rax+0x8],rdx
   0x00005555555551d7 <+130>:   jmp    0x5555555551f3 <add_char_to_map+158>
   0x00005555555551d9 <+132>:   movzx  eax,BYTE PTR [rbp-0x1c]
   0x00005555555551dd <+136>:   lea    rdx,[rax*8+0x0]
   0x00005555555551e5 <+144>:   mov    rax,QWORD PTR [rbp-0x18]
   0x00005555555551e9 <+148>:   add    rdx,rax
   0x00005555555551ec <+151>:   mov    rax,QWORD PTR [rbp-0x10]
   0x00005555555551f0 <+155>:   mov    QWORD PTR [rdx],rax
   0x00005555555551f3 <+158>:   nop
   0x00005555555551f4 <+159>:   leave
   0x00005555555551f5 <+160>:   ret
End of assembler dump.
```
To keep it short, this function will malloc 16 bytes of memory and populate the first 8 bytes with the value of current characters_registered value. The pointer to the malloc will be put inside
the character_map stack memory, in the space offset by the character code of the current input character. If a pointer already exists there, you follow it and check if it's 8 last bytes are zero, and if not,
you put a pointer for the malloc there. As we can see, the character_map is a byte map consisting of 256 linked lists, where the charcode is referenced by position, while the position in the message is
referenced by the value stored in the elements of the linked list.
#### C code
The full C equivalent:
```bash
void add_char_to_map(long * character_map, long character, long characters_registered){
	/* Creates a map of 256 8-byte memory spaces
	each memory space corresponds to a character code (ascii)
	When a character is registered, its corresponding memory space
		will be populated with a pointer to a Linked List 16 byte construct:
		* first 8 bytes -> index place in inputted string (characters_registered)
		* last 8 bytes -> null ( to be populated with another pointer)
	*/	
	long * character_map;			//[rbp-0x18]
	long character;					//[rbp-0x1c]
	long characters_registered;		//[rbp-0x28]
	long * malloc_ptr;				//[rbp-0x10]
	long ??? ;						//[rbp-0x8]
	
	long ??? = * (character_map + character * 8)
	
	malloc_ptr = (long *) malloc( 16 );
	
	*(malloc_ptr) = characters_registered;
	*(malloc_ptr + 8) = 0;
	
	if (??? == 0){
		*(character_map + character * 8) = malloc_ptr;
		return 0;
	}
	
	while ( *(???+0x8) != 0){
		??? = *(???+0x8);
	}
	*(???+0x8) = malloc_ptr;
	
	return 0	
}
```

### serialize_and_output
#### Assembly
```bash
(gdb) disassemble serialize_and_output 
Dump of assembler code for function serialize_and_output:
   0x000055555555524a <+0>:     push   rbp
   0x000055555555524b <+1>:     mov    rbp,rsp
   0x000055555555524e <+4>:     sub    rsp,0x30
   0x0000555555555252 <+8>:     mov    QWORD PTR [rbp-0x28],rdi
   0x0000555555555256 <+12>:    mov    DWORD PTR [rbp-0x4],0x0
   0x000055555555525d <+19>:    jmp    0x5555555552ee <serialize_and_output+164>
   0x0000555555555262 <+24>:    mov    eax,DWORD PTR [rbp-0x4]
   0x0000555555555265 <+27>:    cdqe
   0x0000555555555267 <+29>:    lea    rdx,[rax*8+0x0]
   0x000055555555526f <+37>:    mov    rax,QWORD PTR [rbp-0x28]
   0x0000555555555273 <+41>:    add    rax,rdx
   0x0000555555555276 <+44>:    mov    QWORD PTR [rbp-0x18],rax
   0x000055555555527a <+48>:    mov    rax,QWORD PTR [rbp-0x18]
   0x000055555555527e <+52>:    mov    rdi,rax
   0x0000555555555281 <+55>:    call   0x5555555551f6 <list_len>
   0x0000555555555286 <+60>:    mov    QWORD PTR [rbp-0x20],rax
   0x000055555555528a <+64>:    mov    rdx,QWORD PTR [rip+0x2daf]        # 0x555555558040 <stdout@GLIBC_2.2.5>
   0x0000555555555291 <+71>:    lea    rax,[rbp-0x20]
   0x0000555555555295 <+75>:    mov    rcx,rdx
   0x0000555555555298 <+78>:    mov    edx,0x1
   0x000055555555529d <+83>:    mov    esi,0x8
   0x00005555555552a2 <+88>:    mov    rdi,rax
   0x00005555555552a5 <+91>:    call   0x555555555050 <fwrite@plt>
   0x00005555555552aa <+96>:    mov    rax,QWORD PTR [rbp-0x18]
   0x00005555555552ae <+100>:   mov    rax,QWORD PTR [rax]
   0x00005555555552b1 <+103>:   mov    QWORD PTR [rbp-0x10],rax
   0x00005555555552b5 <+107>:   jmp    0x5555555552e3 <serialize_and_output+153>
   0x00005555555552b7 <+109>:   mov    rdx,QWORD PTR [rip+0x2d82]        # 0x555555558040 <stdout@GLIBC_2.2.5>
   0x00005555555552be <+116>:   mov    rax,QWORD PTR [rbp-0x10]
   0x00005555555552c2 <+120>:   mov    rcx,rdx
   0x00005555555552c5 <+123>:   mov    edx,0x1
   0x00005555555552ca <+128>:   mov    esi,0x8
   0x00005555555552cf <+133>:   mov    rdi,rax
   0x00005555555552d2 <+136>:   call   0x555555555050 <fwrite@plt>
   0x00005555555552d7 <+141>:   mov    rax,QWORD PTR [rbp-0x10]
   0x00005555555552db <+145>:   mov    rax,QWORD PTR [rax+0x8]
   0x00005555555552df <+149>:   mov    QWORD PTR [rbp-0x10],rax
   0x00005555555552e3 <+153>:   cmp    QWORD PTR [rbp-0x10],0x0
   0x00005555555552e8 <+158>:   jne    0x5555555552b7 <serialize_and_output+109>
   0x00005555555552ea <+160>:   add    DWORD PTR [rbp-0x4],0x1
   0x00005555555552ee <+164>:   cmp    DWORD PTR [rbp-0x4],0xfe
   0x00005555555552f5 <+171>:   jle    0x555555555262 <serialize_and_output+24>
   0x00005555555552fb <+177>:   nop
   0x00005555555552fc <+178>:   nop
   0x00005555555552fd <+179>:   leave
   0x00005555555552fe <+180>:   ret
End of assembler dump.
```
This function will iterate through every space on the character_map, it will get the length of the linked list saved there and write it out, and if the length is non-zero, it will node-by-node
write out the positions where the specific charcode is found.
#### C code
```bash
void serialize_and_output(character_map){
	/* loops through every character space
	Get amount of positions for that character code
	writes it out.
	Iterates through every 
	*/
	long * character_map;			//[rbp-0x28]
	int index?;						//[rbp-0x4]
	long offset_pointer;			//[rbp-0x18]
	long list_len_return;			//[rbp-0x20]
	long offset_pointer_value;		//[rbp-0x10]
	
	
	
	while (index <= 0xfe){
		offset_pointer = character_map + index * 8;
		list_len_return = list_len(offset_pointer);
		fwrite( &list_len_return, 0x8, 0x1, stdout);
		
		while(*(offset_pointer + 0x8) != 0){
			fwrite(offset_pointer,0x8,0x1,stdout);
			offset_pointer = *(offset_pointer + 0x8)
			++index
		}
		
	}
}
```

### List Len
#### Assembly
```bash
(gdb) disassemble list_len 
Dump of assembler code for function list_len:
   0x00000000000011f6 <+0>:     push   rbp
   0x00000000000011f7 <+1>:     mov    rbp,rsp
   0x00000000000011fa <+4>:     mov    QWORD PTR [rbp-0x18],rdi
   0x00000000000011fe <+8>:     mov    rax,QWORD PTR [rbp-0x18]
   0x0000000000001202 <+12>:    mov    rax,QWORD PTR [rax]
   0x0000000000001205 <+15>:    test   rax,rax
   0x0000000000001208 <+18>:    jne    0x1211 <list_len+27>
   0x000000000000120a <+20>:    mov    eax,0x0
   0x000000000000120f <+25>:    jmp    0x1248 <list_len+82>
   0x0000000000001211 <+27>:    mov    QWORD PTR [rbp-0x8],0x1
   0x0000000000001219 <+35>:    mov    rax,QWORD PTR [rbp-0x18]
   0x000000000000121d <+39>:    mov    rax,QWORD PTR [rax]
   0x0000000000001220 <+42>:    mov    QWORD PTR [rbp-0x10],rax
   0x0000000000001224 <+46>:    jmp    0x1237 <list_len+65>
   0x0000000000001226 <+48>:    add    QWORD PTR [rbp-0x8],0x1
   0x000000000000122b <+53>:    mov    rax,QWORD PTR [rbp-0x10]
   0x000000000000122f <+57>:    mov    rax,QWORD PTR [rax+0x8]
   0x0000000000001233 <+61>:    mov    QWORD PTR [rbp-0x10],rax
   0x0000000000001237 <+65>:    mov    rax,QWORD PTR [rbp-0x10]
   0x000000000000123b <+69>:    mov    rax,QWORD PTR [rax+0x8]
   0x000000000000123f <+73>:    test   rax,rax
   0x0000000000001242 <+76>:    jne    0x1226 <list_len+48>
   0x0000000000001244 <+78>:    mov    rax,QWORD PTR [rbp-0x8]
   0x0000000000001248 <+82>:    pop    rbp
   0x0000000000001249 <+83>:    ret
End of assembler dump.
```
This is used by the serialize_and_output function to calculate the length of Linked Lists inside the character_map.
#### C code
```bash
long list_len( long offset_pointer){
	long offset_pointer;					//[rbp-0x18]
	long offset_pointer_value;		//[rbp-0x10]
	???										        //[rbp-0x8]
	
	
	if (*offset_pointer == 0){
		return 0;
	}
	
	??? = 1;
	offset_pointer_value = *offset_pointer;
	
	while ( *(offset_pointer_value + 0x8) != 0){
		offset_pointer_value = *(offset_pointer_value + 0x8);
		???++;
	}
	
	return ???;
}
```
### Full C code
```bash
int main(void){
	long characters_registered; 	//[rbp-0x8]
	long character_map[256] = {0}	//[rbp-0x810]
	int character;					//[rbp-0xc]
	
	characters_registered = 0;
	
	while (1){
		character = getchar();
		if ( character == EOF){
			break;
		}
		add_char_to_map(character_map, (long) character, characters_registered);
		++characters_registered;
	}
		
	serialize_and_output(character_map);
	
	return 0;
}

long list_len( long offset_pointer){
	long offset_pointer;					//[rbp-0x18]
	long offset_pointer_value;				//[rbp-0x10]
	???										//[rbp-0x8]
	
	
	if (*offset_pointer == 0){
		return 0;
	}
	
	??? = 1;
	offset_pointer_value = *offset_pointer;
	
	while ( *(offset_pointer_value + 0x8) != 0){
		offset_pointer_value = *(offset_pointer_value + 0x8);
		???++;
	}
	
	return ???;
}

void serialize_and_output(character_map){
	/* loops through every character space
	Get amount of positions for that character code
	writes it out.
	Iterates through every 
	*/
	long * character_map;			//[rbp-0x28]
	int index?;						//[rbp-0x4]
	long offset_pointer;			//[rbp-0x18]
	long list_len_return;			//[rbp-0x20]
	long offset_pointer_value;		//[rbp-0x10]
	
	
	
	while (index <= 0xfe){
		offset_pointer = character_map + index * 8;
		list_len_return = list_len(offset_pointer);
		fwrite( &list_len_return, 0x8, 0x1, stdout);
		
		while(*(offset_pointer + 0x8) != 0){
			fwrite(offset_pointer,0x8,0x1,stdout);
			offset_pointer = *(offset_pointer + 0x8)
			++index
		}
		
	}
}

void add_char_to_map(long * character_map, long character, long characters_registered){
	/* Creates a map of 256 8-byte memory spaces
	each memory space corresponds to a character code (ascii)
	When a character is registered, its corresponding memory space
		will be populated with a pointer to a Linked List 16 byte construct:
		* first 8 bytes -> index place in inputted string (characters_registered)
		* last 8 bytes -> null ( to be populated with another pointer)
	*/	
	long * character_map;			//[rbp-0x18]
	long character;					//[rbp-0x1c]
	long characters_registered;		//[rbp-0x28]
	long * malloc_ptr;				//[rbp-0x10]
	long ??? ;						//[rbp-0x8]
	
	long ??? = * (character_map + character * 8)
	
	malloc_ptr = (long *) malloc( 16 );
	
	*(malloc_ptr) = characters_registered;
	*(malloc_ptr + 8) = 0;
	
	if (??? == 0){
		*(character_map + character * 8) = malloc_ptr;
		return 0;
	}
	
	while ( *(???+0x8) != 0){
		??? = *(???+0x8);
	}
	*(???+0x8) = malloc_ptr;
	
	return 0	
}
```

## Solution
What we must do in order to solve this is we need to iteratively read the message file, where the first thing we will encounter is the number of occurences of a character, followed by index positions of
these occurences. The file starts out with zeros (non-printable characters were not used in the message), until we encounter a 0xD (13 decimal), followed by 13 positions of the specific character.
This character is 0xa or "\n" - meaning there are 13/14 lines in the message. 
NOTE: In our keeping track of the charcode in question, we need to keep in mind we must not increment the charcode counter while reading the number of occurences of previous charcodes!

I have created a C and a Python version of the solution, with the name of the file hardcoded inside them.
The C solution was a bit more complicated to implement, as C does not give you a Linked List datatype, unlike Python, so I implemented it myself.
