/*
 *     Proyect: gbz80Asm
 *    Filename: main.cpp
 *     Version: v2.0
 * Description: Gameboy Z80 Asembler
 *     License: GPLv2
 *
 *      Author: Copyright (C) Ruben Daniel Gutierrez Cruz <dospro@gmail.com>
 *        Date: 14-8-2007
 *
 *
 *	This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"opcodes.h"
#include "routine.h"
#include "jumps.h"

/*globals*/
FILE *asm_file, *gb_file;
int assemble(FILE *in, FILE *out);
int op_compare(char *opcode_name, char *arguments);
int process_all(char *opcode, char *arg1, char *arg2, char *final_arg, int line_number);
int atoh(char *number);
int main(int argc, char *argv[])
{
	char filename[128];
	int i=0;
	ROUTINE *temp, *temp2;
	JUMPS *j_temp, *j_temp2;
	unsigned char nintendo_logo[]={0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 
                      			0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 
					0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 
					0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
					0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};
	printf("GB-z80 assembler v2 by dospro(dospro@gmail.com)\n");
	printf("Web: http://boigb.sourceforge.net\n");
	if(argc<2)
	{
		printf("gbz80_asm.exe <asm_file.asm>\n");
		exit(0);
	}

	asm_file=fopen(argv[1], "r");
	if(asm_file==NULL)
	{
		printf("Couldn't open %s\n", argv[1]);
		exit(0);
	}
	strcpy(filename, argv[1]);
	strcat(filename, ".gb");
	gb_file=fopen(filename, "w+");
	if(gb_file==NULL)
	{
		printf("Couldn't create gb file\n");
		exit(0);
	}
	printf("Making header:\n");
	for(i=0; i<1024*32; i++)
		fputc(0, gb_file);//Fill the rom with 0
	fseek(gb_file, 0x104, SEEK_SET);
	fwrite(&nintendo_logo, 1, sizeof(nintendo_logo), gb_file);
	fseek(gb_file, 0x100, SEEK_SET);
	fputc(0xC3, gb_file);
	fputc(0x50, gb_file);
	fputc(0x01, gb_file);//Put the jump to the 150 address
	fseek(gb_file, 0x150, SEEK_SET);
	printf("Assembling...please wait\n");
	assemble(asm_file, gb_file);
	printf("Freeing memory\n");
	
	temp = r_list;
	temp2 = NULL;
	while(temp)
	{
		temp2 = temp->next;
		free(temp);
		temp = temp2;
	}

	j_temp = j_list;
	j_temp2 = NULL;
	while(temp)
	{
		j_temp2 = j_temp->next;
		free(j_temp);
		j_temp = j_temp2;
	}

			


	fclose(asm_file);
	fclose(gb_file);
	printf("Finish...exiting\n");
	return 0;
}

/*Some globals we will need*/
int n_activator=0, nn_activator=0, n, nn;//This are needed when there is more 
//than one opcode per instruction
int n_activator2=0, nn_activator2=0, n2, nn2;
int bit_indicator=0, bit_number;//This is used to stor the bit number to set, check or test
int bit_indicator2=0, bit_number2;
//int i=1;//Utility for fors or temp values

/*
 * Creates a new string with all leading and trailing spaces removed
 * The string MUST be freed manually
 */
char *strtrim(char *in_str) {
    size_t len = strlen(in_str);
    size_t start_index = 0;
    while (isspace(in_str[start_index]) != 0 && start_index < len) {
        ++start_index;
    }
    if (start_index == len) {
        char *out = (char *)malloc(sizeof(char));
        out[0] = 0;
        return out;
    }

    size_t end_index = len - 1;
    while (isspace(in_str[end_index]) != 0 && end_index > start_index) {
        --end_index;
    }
    end_index++;

    char *out = (char *) malloc(sizeof(char) * (end_index - start_index + 1));
    size_t out_index = 0;
    while (start_index < end_index) {
        out[out_index] = in_str[start_index];
        ++out_index;
        ++start_index;
    }
    out[out_index] = 0;
    return out;
}

int split_line(char *in_buffer, char *opcode_out, char *arg1_out, char *arg2_out) {
    char delimiters[] = " ,\n\t";//This are used with strtok to separate the arguments
    // Eliminate comments
    char *token = strtok(in_buffer, ";"); // Will place a 0 where the ; is
    if (token != NULL)
        strcpy(in_buffer, token); // No, copy the new buffer without comments
    // Now lets separate the tokens
    token = strtok(in_buffer, delimiters); // Get the first argument
    if (token == NULL) {
        printf("ERROR: Could not parse line: %s\n", in_buffer);
        return -1;
    }
    strcpy(opcode_out, token);
    token = strtok(NULL, delimiters); // Get the second argument
    if (token == NULL) {
        strcpy(arg1_out, "-"); // No arguments
        strcpy(arg2_out, "");
        return 0;
    }
    strcpy(arg1_out, token);              // Copy the first argument
    token = strtok(NULL, delimiters); // Get the third argument
    if (token != NULL) {
        strcpy(arg2_out, token);
    }

    return 0;
}

int is_conditional_flag(char *arg) {
    if (strcmp(arg, "nz") == 0 || strcmp(arg, "z") == 0 || strcmp(arg, "nc") == 0 || strcmp(arg, "c") == 0) {
        return 0;
    }
    return 1;
}

//This is the ultrapowerfull routine which will create a gb file from an asm file
//Really a complex motor
int assemble(FILE *in, FILE *out) {
    int line_counter = 0;
    int result, j;//Here we will get the resulting opcode(s)
    int seek_temp;
    char buffer[512];//Max number of caracters per line
    char arg1[64];//First argument(it should not even get bigger than 16)
    char arg2[64];//Second argumnet if there is
    char final_arg[64];//This is the string that will be compared with the opcodes table
    //To get the correct opcode
    char opcode[64];//This will have the opcode string(adc, ld, or, srl....)
    char delimiters[] = " ,\n\t";//This are used with strtok to separate the arguments
    char temp_arg[64];
    char *point;//This is the pointers used with strtok which contains the result token
    struct RoutineList routines_list;
    struct JumpList jumps_list;
    INCLUDES *incs = NULL;
    FILE *inc_file;
    //temp=r_list;
    //j_temp=j_list;
    while (!feof(in))//While we havent reach the end of the file, continue assembling
    {
        /*Ok, first we must read the opcode, so we use te normal scanf, after that
        we read with fgets to read until the comments end(if there so).
        We separate the arguments, eliminate any unecesary spaces and
        finally produce the new opcode*/

        /*Lets clear the buffers*/
        memset(buffer, 0, sizeof(buffer));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        memset(final_arg, 0, sizeof(final_arg));
        memset(opcode, 0, sizeof(opcode));
        n_activator = nn_activator = n = nn = 0;//Put all to zero
        n_activator2 = nn_activator2 = n2 = nn2 = 0;
        bit_indicator = bit_number = 0;
        bit_indicator2 = bit_number2 = 0;

        fgets(buffer, 511, in);//Get everything else
        char *clean_buffer = strtrim(buffer);
        if (strlen(clean_buffer) == 0 || clean_buffer[0] == ';') {
            ++line_counter;
            continue;
        }
        split_line(clean_buffer, opcode, arg1, arg2);
        free(clean_buffer);

        /*now we have the two argument separated in different variable*/

        //well, here we still dont know if we really have an opcode or something else
        //Lets first check for any preassembly instructions and then for routines
        /*New version add*/

        //If we have a special instruction .include then we must include this.
        if (strcmp(opcode, ".include") == 0) {
            //The file will be assembled exactly where the include is.
            inc_file = fopen(arg1, "r");
            if (inc_file == NULL)
                printf("Couldn't include %s\n", arg1);
            else {
                assemble(inc_file, out);
            }
        } else if (strcmp(opcode, ".main") == 0) {
            //If we find the option .main then the program starts there not in 0x150
            seek_temp = ftell(out);//Get the current position of the file
            fseek(out, 0x100, SEEK_SET);//get to the program start
            fputc(0xC3, out);//Wrtie the new jump
            fputc(seek_temp & 0xFF, out);
            fputc((seek_temp >> 8) & 0xFF, out);
            fseek(out, seek_temp, SEEK_SET);
        } else if (strcmp(opcode, ".db") == 0) {//If we find a .db option then we write the byte as it is.
            fputc(atoh(arg1), out);
        } else if (strchr(opcode, ':') != NULL) {
            // If there is a : in the opcode, then its a routine
            add_routine(&routines_list, strtok(opcode, ":"), ftell(out));
        } else if (strcmp(opcode, "call") == 0) {
            // If we have a call, then we have a routine name
            // We must take this routine name and keep so after normal assembler
            // We will put the right address
            char routine_name[64];
            if (is_conditional_flag(arg1)) {
                //If in the first argument we have conditionals then we use the second argument
                strcpy(routine_name, arg2);
                strcpy(final_arg, arg1);
                strcat(final_arg, ",dir");
            } else//If not, we use the first argument
            {
                strcpy(routine_name, arg1);
                strcpy(final_arg, "dir");
            }
            //Now temp_arg has the name of the routine, lets keep it in our dynamic array
            add_jump(&jumps_list, routine_name, ftell(out) + 1, 0, 16);

            result = op_compare(opcode, final_arg); // Get the correct opcode number
            if (result == -1) {
                printf("Error line %d, opcode: %s with arguments %s has an error or its an assembler bug\n",
                       line_counter, opcode,
                       final_arg);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
            fputc(0, out);//Fill with generic 0
        } else if (strcmp(opcode, "jp") == 0 && strcmp(arg1, "(hl)") != 0 && strcmp(arg1, "[hl]") != 0) {
            char routine_name[64];
            if (is_conditional_flag(arg1) == 0) {
                //If in the first argument we have conditionals then we use the second argument
                strcpy(routine_name, arg2);
                strcpy(final_arg, arg1);
                strcat(final_arg, ",dir");
            } else {
                //If not, we use the dirst argument
                strcpy(routine_name, arg1);
                strcpy(final_arg, "dir");
            }
            //Now temp_arg has the name of the routine, lets keep it in our dinamic array

            add_jump(&jumps_list, routine_name, ftell(out) + 1, 0, 16);

            result = op_compare(opcode, final_arg);//Get the correct opcode number
            if (result == -1) {
                printf("Error line %d, opcode: %s with arguments %s has an error or its an assembler bug\n",
                       line_counter, opcode,
                       final_arg);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
            fputc(0, out);//Fill with generic 0
        } else if (strcmp(opcode, "jr") == 0) {
            char routine_name[64];
            if (is_conditional_flag(arg1) == 0) {
                //If in the first argument we have conditionals then we use the second argument
                strcpy(routine_name, arg2);
                strcpy(final_arg, arg1);
                strcat(final_arg, ",dir");
            } else {
                //If not, we use the dirst argument
                strcpy(routine_name, arg1);
                strcpy(final_arg, "dir");
            }
            //Now temp_arg has the name of the routine, lets keep it in our dinamic array
            add_jump(&jumps_list, routine_name, ftell(out) + 1, 0, 8);

            result = op_compare(opcode, final_arg);//Get the correct opcode number
            if (result == -1) {
                printf("Error line %d, opcode: %s with arguments %s has an error or its an asembler bug\n",
                       line_counter, opcode,
                       final_arg);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
        } else {
            /*Lets start analizing the first argument begin building the table opcode*/
            process_all(opcode, arg1, arg2, final_arg, line_counter);
            //Finally, we have all the arguments ready to pass them to the opcode table.
            //Here it will be compared and it will return the opcode to write.
            result = op_compare(opcode, final_arg);
            if (result == -1)//If not found, generate an error message
            {
                printf("error: line %d \n", line_counter);
                printf("Opcode %s with arguments %s was no found\n", opcode, final_arg);
                //return 1;
            }//Now lets write it based in some rules
            else if ((result >> 8) == 0xCB)//If we have a CB opcode, then we must write the 2 values
            {
                if (bit_indicator == 1)//If we have a bit opcode then we introduce it inside the value
                {
                    j = result & 0xFF;
                    j = j | (bit_number << 3);//Here we introduce the number of the bit
                    fputc(0xCB, out);//Write the CB and
                    fputc(j, out);//then the ready opcode
                } else {
                    fputc(0xCB, out);//Write the CB and
                    fputc(result & 0xFF, out);//then the ready opcode
                }
            } else if (result == 0x10)//Lets remember the stop which has 2 opcodes
            {
                fputc(0x10, out);//Write the CB and
                fputc(0, out);//then the ready opcode
            } else {
                if (n_activator == 1)//If there is an n value
                {
                    fputc(result, out);
                    fputc(n, out);
                } else if (nn_activator == 1)//If there is a 2 bytes value
                {
                    fputc(result, out);//write the opcode
                    fputc((nn & 0xFF), out);//write the less significant byte
                    fputc((nn >> 8) & 0xFF, out);//write the most significant byte
                } else if (n_activator2 == 1)//If there is an n value
                {
                    fputc(result, out);
                    fputc(n2, out);
                } else if (nn_activator2 == 1) {
                    fputc(result, out);//write the opcode
                    fputc((nn2 & 0xFF), out);//write the less significant byte
                    fputc((nn2 >> 8) & 0xFF, out);//write the most significant byte
                } else
                    fputc(result, out);
            }
        }


        //Increment line counter
        line_counter++;
    }//Ends the assembly loop
    //Now lets finish with the routines(calls) and jumps
    //Fisrt we will go for each call/jp made and search it in the routines list
    struct Jump *jump = pop_jump(jumps_list);
    while (jump != NULL) {
        struct Routine *routine = search_routine_by_name(routines_list, jump->name);
        if (routine == NULL) {
            printf("ERROR: There is a jump to an undefined routine %s\n", jump->name);
        }
        fseek(out, jump->address, SEEK_SET);
        result = fgetc(out);
        if (result != 0) {
            //If we dont have the generic 0, then something bad happened
            printf("Something bad happened the %d should be a 0\n", result);
            printf("The opcode will be written, so be carefull\n");
            printf("This may be because a programs bug, contact me to report this\n");
        }

        fseek(out, -1, SEEK_CUR);
        if (jump->bits == 16) {
            int high_address = (int) (routine->address >> 8) & 0xFF;
            int low_address = (int) routine->address & 0xFF;
            fputc(low_address, out);
            fputc(high_address & 0xFF, out);
        } else {
            int address = (int) (routine->address - jump->address - 1) & 0xFF;
            fputc(address, out);
        }
    }
    //Everything resulted ok, no errors
    return 0;
}

//This routine will process opcode and arguments and will produce a string 
//which is compatible with the opcode_table so it can be compared and am
//opcode number returned.
/*Note: This routine will only precess opcodes, arguments and comments.
It will not know what a routine is, or special instructions of the assembler
Useu this only when opcode is a valid opcode, if it is something like "main:"
or ".start" it will produce an error string.
*/
int process_all(char *opcode, char *arg1, char *arg2, char *final_arg, int line_number)
{
	/*Lets go first with irreularities*/
	//Check if there is argument
	if(arg1[0]=='-')//If there is no argument
		final_arg[0]='-';
	//Numbers:

	else if(arg1[0]=='0' || arg1[0]=='$')//If the first letter of the argument is 0 or $, then its a number
	{
		if(arg1[1]=='x')//This is an optional letter to identify hexadecimal numbers
		{
			n=atoh(arg1+2);//Then put the number into the n variable
		}
		else
		{
			n=atoh(arg1+1);
		}
		if(n>0xFF)//If its more than 1 byte
		{
			nn_activator=1;//Tell the assembler to write this number after the opcode
			nn=n;//Keep the number in the correct variable
			n=0;
			strcpy(final_arg, "**");
		}
		else
		{
			n_activator=1;
			strcpy(final_arg, "*");
		}
	}
	//Bits number(0-7)
	else if(arg1[0]=='0' || arg1[0]=='1' || arg1[0]=='2' || arg1[0]=='3' ||
		arg1[0]=='4' || arg1[0]=='5' || arg1[0]=='6' || arg1[0]=='7' )
	{
		bit_number=atoi(arg1);//Keep the bit number
		bit_indicator=1;//Trun on the bit indicator
		strcpy(final_arg, "n");
		//Now check if this bit is 0-7
		if(bit_number>7)
		{
			printf("Error in line %d: there is no bit %d\n", line_number, bit_number);
			return 1;
		}
	}

	//Parentesis:

	else if(arg1[0]=='(' || arg1[0]=='[')//Ok, if we have a parenthesis or the "corchet" 
	{	                            //then we must know whats inside

		//Ok, if we have an number, we do the same as before except for the  final_arg
		if(arg1[1]=='0' || arg1[1]=='$')//If the first letter of the argument is 0 or $, then its a number
		{//We must be carefull not to write ld (0xFF00+n),x...
			if(strlen(arg1)>9)
			{
				printf("Ops, error en la linea %d, el primer argumento no puede ser tan grande\nSaliendo...\n", line_number);
				return 1;
			}
			if(arg1[2]=='x')//This is an optional letter to identify hexadecimal numbers
			{
				n=atoh(arg1+3);//Then put the number into the n variable
			}
			else
			{
				n=atoh(arg1+2);
			}
			if(n>0xFF)//If its more than 1 byte
			{
				nn_activator=1;//Tell the assembler to write this number after the opcode
				nn=n;//Keep the number in the correct variable
				n=0;
				strcpy(final_arg, "(**)");
			}
			else
			{
				n_activator=1;
				strcpy(final_arg, "(*)");
			}
		}
		else if(arg1[1]=='f' && arg1[2]=='f' && arg1[3]==0 && arg1[4]==0//If there is no simbol, then it is ff00+number
			&& arg1[5]=='+' && arg1[6]!='c')
		{
			n=atoh(arg1+6);
			n_activator=1;
			strcpy(final_arg, "(ff00+*)");
		}
		else
			strcpy(final_arg, arg1);
	}//finish parentesis
	//Here we are ready with irregularities, lets just compare and autocomplete
	else		
		strcpy(final_arg, arg1);
	//With this we finish with the first argument
	//Lets go with the second argument ehich is the same process.
	/**********************************************************/
	//Check if there is argument
	//If there is no argument we do nothing		
	//Numbers:

	if(arg2[0]=='0' || arg2[0]=='$')//If the first letter of the argument is 0 or $, then its a number
	{
		if(arg2[1]=='x')//This is an optional letter to identify hexadecimal numbers
		{
			n2=atoh(arg2+2);//Then put the number into the n variable
		}
		else
		{
			n2=atoh(arg2+1);
		}
		if(n>0xFF)//If its more than 1 byte
		{
			nn_activator2=1;//Tell the assembler to write this number after the opcode
			nn2=n2;//Keep the number in the correct variable				
			n2=0;
			strcat(final_arg, ",**");
		}
		else
		{
			n_activator2=1;
			strcat(final_arg, ",*");
		}
	}
	//Bits number(0-7)
	else if(arg2[0]=='0' || arg2[0]=='1' || arg2[0]=='2' || arg2[0]=='3' ||
		arg2[0]=='4' || arg2[0]=='5' || arg2[0]=='6' || arg2[0]=='7' )
	{
		bit_number2=atoi(arg2);//Keep the bit number
		bit_indicator2=1;//Trun on the bit indicator
		strcat(final_arg, ",n");
		//Now check if this bit is 0-7
		if(bit_number2>7)
		{
			printf("Error in line %d: there is no bit %d\n", line_number, bit_number);
			return 1;
		}
	}

	//Parentesis:

	else if(arg2[0]=='(' || arg2[0]=='[')//Ok, if we have a parenthesis or the "corchet" 
	{	                            //then we must know whats inside

		//Ok, if we have an number, we do the same as before except for the  final_arg
		if(arg2[1]=='0' || arg2[1]=='$')//If the first letter of the argument is 0 or $, then its a number
		{//We must be carefull not to write ld (0xFF00+n),x...
			if(strlen(arg2)>9)
			{
				printf("Ops, error en la linea %d, el primer argumento no puede ser tan grande\nSaliendo...\n", line_number);
				return 1;
			}
			if(arg2[2]=='x')//This is an optional letter to identify hexadecimal numbers
			{
				n2=atoh(arg2+3);//Then put the number into the n variable
			}
			else
			{
				n2=atoh(arg2+2);
			}
			if(n>0xFF)//If its more than 1 byte
			{
				nn_activator2=1;//Tell the assembler to write this number after the opcode
				nn2=n2;//Keep the number in the correct variable
				n2=0;
				strcat(final_arg, ",(**)");
			}
			else
			{
				n_activator2=1;
				strcat(final_arg, ",(*)");
			}
		}
		else if(arg2[1]=='f' && arg2[2]=='f' && arg2[3]==0 && arg2[4]==0//If there is no simbol, then it is ff00+number
			&& arg2[5]=='+' && arg2[6]!='c')
		{
			n2=atoh(arg2+6);
			n_activator2=1;
			strcat(final_arg, ",(ff00+*)");
		}
	}//finish parentesis
	//Here we are ready with irregularities, lets just compare and autocomplete
	else 
	{
		if(arg2[0]!=0)
		{
			strcat(final_arg, ",");//We put the comma 
			strcat(final_arg, arg2);//then the second argument
		}
	}
	return 0;
}

int op_compare(char *opcode_name, char *arguments)
{
	//This will compare the opcode and the arguments
	//with the table and return its opcode.
	int i;
	for(i=0; i<512; i++)//Go through all the table
	{
		if(strcmp(opcode_table[i].op_name, opcode_name)==0)
			if(strcmp(opcode_table[i].op_args, arguments)==0)
			{
				if(opcode_table[i].op_val==0xCB)
					return (opcode_table[i].op_val<<8)|(opcode_table[i].op_val2);
				else
					return opcode_table[i].op_val;
			}
	}
	return -1;
}

int atoh(char *number)
{
	char f, s, t, fo;
	int result;
	int fn, sn, tn, fon, flag=0;
	f=number[0];
	s=number[1];
	t=number[2];
	fo=number[3];

	if(f=='0')	
		fn=0;
	else if(f=='1')
		fn=1;
	else if(f=='2')
		fn=2;
	else if(f=='3')
		fn=3;
	else if(f=='4')
		fn=4;
	else if(f=='5')
		fn=5;
	else if(f=='6')
		fn=6;
	else if(f=='7')
		fn=7;
	else if(f=='8')
		fn=8;
	else if(f=='9')
		fn=9;
	else if(f=='a' || f=='A')
		fn=0xa;
	else if(f=='b' || f=='B')
		fn=0xb;
	else if(f=='c' || f=='C')
		fn=0xc;
	else if(f=='d' || f=='D')
		fn=0xd;
	else if(f=='e' || f=='E')
		fn=0xe;
	else if(f=='f' || f=='F')
		fn=0xf;
	else
		fn=0;

	if(s=='0')	
		sn=0;
	else if(s=='1')
		sn=1;
	else if(s=='2')
		sn=2;
	else if(s=='3')
		sn=3;
	else if(s=='4')
		sn=4;
	else if(s=='5')
		sn=5;
	else if(s=='6')
		sn=6;
	else if(s=='7')
		sn=7;
	else if(s=='8')
		sn=8;
	else if(s=='9')
		sn=9;
	else if(s=='a' || s=='A')
		sn=0xa;
	else if(s=='b' || s=='B')
		sn=0xb;
	else if(s=='c' || s=='C')
		sn=0xc;
	else if(s=='d' || s=='D')
		sn=0xd;
	else if(s=='e' || s=='E')
		sn=0xe;
	else if(s=='f' || s=='F')
		sn=0xf;
	else
		sn=0;

	if(t=='0')	
		tn=0;
	else if(t=='1')
		tn=1;
	else if(t=='2')
		tn=2;
	else if(t=='3')
		tn=3;
	else if(t=='4')
		tn=4;
	else if(t=='5')
		tn=5;
	else if(t=='6')
		tn=6;
	else if(t=='7')
		tn=7;
	else if(t=='8')
		tn=8;
	else if(t=='9')
		tn=9;
	else if(t=='a' || t=='A')
		tn=0xa;
	else if(t=='b' || t=='B')
		tn=0xb;
	else if(t=='c' || t=='C')
		tn=0xc;
	else if(t=='d' || t=='D')
		tn=0xd;
	else if(t=='e' || t=='E')
		tn=0xe;
	else if(t=='f' || t=='F')
		tn=0xf;
	else if(t==')' || t==0 || t==']')
		tn=0;
	else
	{
		tn=0;
		flag=1;
	}

	if(fo=='0')	
		fon=0;
	else if(fo=='1')
		fon=1;
	else if(fo=='2')
		fon=2;
	else if(fo=='3')
		fon=3;
	else if(fo=='4')
		fon=4;
	else if(fo=='5')
		fon=5;
	else if(fo=='6')
		fon=6;
	else if(fo=='7')
		fon=7;
	else if(fo=='8')
		fon=8;
	else if(fo=='9')
		fon=9;
	else if(fo=='a' || fo=='A')
		fon=0xa;
	else if(fo=='b' || fo=='B')
		fon=0xb;
	else if(fo=='c' || fo=='C')
		fon=0xc;
	else if(fo=='d' || fo=='D')
		fon=0xd;
	else if(fo=='e' || fo=='E')
		fon=0xe;
	else if(fo=='f' || fo=='F')
		fon=0xf;
	else
		fon=0;

	if(flag==0)
		result=((fn<<4)|sn)&0xFF;
	else
		result=((fn<<12)|(sn<<8)|(tn<<4)|fon)&0xFFFF;
	return result;
}
