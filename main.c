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

struct Operation {
    char opcode_args[64];
    int first_arg_value;
    int second_arg_value;
    int is_first_arg_word;
    int is_second_arg_word;
    int is_first_arg_bit;
    int is_second_arg_bit;
    int first_arg_bits;
    int second_arg_bits;
};

/*globals*/
FILE *asm_file, *gb_file;
int assemble(FILE *in, FILE *out);
int op_compare(char *opcode_name, char *arguments);
int process_all(struct Operation *operation, char *arg1, char *arg2, int line_number);
int atoh(char *number);

int main(int argc, char *argv[]) {
    char filename[128];
    unsigned char nintendo_logo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73,
                                     0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F,
                                     0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD,
                                     0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
                                     0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};
    printf("GB-z80 assembler v2 by dospro(dospro@gmail.com)\n");
    printf("Web: http://boigb.sourceforge.net\n");
    if (argc < 2) {
        printf("gbz80_asm.exe <asm_file.asm>\n");
        exit(0);
    }

    asm_file = fopen(argv[1], "r");
    if (asm_file == NULL) {
        printf("Couldn't open %s\n", argv[1]);
        exit(0);
    }
    strcpy(filename, argv[1]);
    strcat(filename, ".gb");
    gb_file = fopen(filename, "w+");
    if (gb_file == NULL) {
        printf("Couldn't create gb file\n");
        exit(0);
    }
    printf("Making header:\n");
    for (int i = 0; i < 1024 * 32; i++)
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

    fclose(asm_file);
    fclose(gb_file);
    printf("Finish...exiting\n");
    return 0;
}

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
        char *out = (char *) malloc(sizeof(char));
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

/*
 * Goes line by line and translates the assembly code into binary representation
 *
 * It also handles routine names and special instructions
 */
int assemble(FILE *in, FILE *out) {
    int line_counter = 0;
    char buffer[512];//Max number of caracters per line
    char opcode[64];//This will have the opcode string(adc, ld, or, srl....)
    char arg1[64];//First argument(it should not even get bigger than 16)
    char arg2[64];//Second argumnet if there is
    char final_arg[64];//This is the string that will be compared with the opcodes table
    //To get the correct opcode
    struct RoutineList routines_list;
    struct JumpList jumps_list;
    FILE *inc_file;

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
            long seek_temp = ftell(out);//Get the current position of the file
            fseek(out, 0x100, SEEK_SET);//get to the program start
            fputc(0xC3, out);//Wrtie the new jump
            fputc((int) seek_temp & 0xFF, out);
            fputc((int) (seek_temp >> 8) & 0xFF, out);
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

            int result = op_compare(opcode, final_arg); // Get the correct opcode number
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

            int result = op_compare(opcode, final_arg);//Get the correct opcode number
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

            int result = op_compare(opcode, final_arg);//Get the correct opcode number
            if (result == -1) {
                printf("Error line %d, opcode: %s with arguments %s has an error or its an asembler bug\n",
                       line_counter, opcode,
                       final_arg);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
        } else {
            /*Lets start analizing the first argument begin building the table opcode*/
            struct Operation op;
            process_all(&op, arg1, arg2, line_counter);
            //Finally, we have all the arguments ready to pass them to the opcode table.
            //Here it will be compared and it will return the opcode to write.
            int result = op_compare(opcode, op.opcode_args);
            if (result == -1) {
                //If not found, generate an error message
                printf("error: line %d \n", line_counter);
                printf("Opcode %s with arguments %s was no found\n", opcode, final_arg);
                //return 1;
            } else if ((result >> 8) == 0xCB) {
                //Now lets write it based in some rules
                //If we have a CB opcode, then we must write the 2 values
                if (op.is_first_arg_bit == 1) {
                    //If we have a bit opcode then we introduce it inside the value
                    result = result & 0xFF;
                    result = result | (op.first_arg_bits << 3);//Here we introduce the number of the bit
                    fputc(0xCB, out);//Write the CB and
                    fputc(result, out);//then the ready opcode
                } else {
                    fputc(0xCB, out);//Write the CB and
                    fputc(result & 0xFF, out);//then the ready opcode
                }
            } else if (result == 0x10) {
                // stop which has 2 opcodes
                fputc(0x10, out);
                fputc(0, out);
            } else {
                if (op.is_first_arg_word == 0) {
                    //If there is an n value
                    fputc(result, out);
                    fputc(op.first_arg_value, out);
                } else if (op.is_first_arg_word == 1) {
                    //If there is a 2 bytes value
                    fputc(result, out);//write the opcode
                    fputc((op.first_arg_value & 0xFF), out);//write the less significant byte
                    fputc((op.first_arg_value >> 8) & 0xFF, out);//write the most significant byte
                } else if (op.is_second_arg_word == 0) {
                    //If there is an n value
                    fputc(result, out);
                    fputc(op.second_arg_value, out);
                } else if (op.is_second_arg_word == 1) {
                    fputc(result, out); // write the opcode
                    fputc((op.second_arg_value & 0xFF), out); // write the less significant byte
                    fputc((op.second_arg_value >> 8) & 0xFF, out); // write the most significant byte
                } else {
                    fputc(result, out);
                }
            }
        }
        line_counter++;
    }
    //Now lets finish with the routines(calls) and jumps
    //Fisrt we will go for each call/jp made and search it in the routines list
    for (struct Jump *jump = pop_jump(jumps_list); jump != NULL; jump = pop_jump(jumps_list)) {
        struct Routine *routine = search_routine_by_name(routines_list, jump->name);
        if (routine == NULL) {
            printf("ERROR: There is a jump to an undefined routine %s\n", jump->name);
        }
        fseek(out, jump->address, SEEK_SET);
        int result = fgetc(out);
        if (result != 0) {
            //If we dont have the generic 0, then something bad happened
            printf("Something bad happened the %d should be a 0\n", result);
            printf("The opcode will be written, so be careful\n");
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

        free(jump);
    }
    free_routines(routines_list);
    //Everything resulted ok, no errors
    return 0;
}


/*
 * Routine that will take the opcode name and its arguments and produce
 * a string for doing a look up in the opcodes table
 */
int process_all(struct Operation *operation, char *arg1, char *arg2, int line_number) {
    /*Lets go first with irregularities*/
    //Check if there is argument

    //If there is no argument
    if (arg1[0] == '-') {
        strcpy(operation->opcode_args, "-");
    } else if (arg1[0] == '0' || arg1[0] == '$') {
        //If the first letter of the argument is 0 or $, then its a hex number
        // This is an optional letter to identify hexadecimal numbers
        if (arg1[1] == 'x') {
            operation->first_arg_value = atoh(&arg1[2]);
        } else {
            operation->first_arg_value = atoh(&arg1[1]);
        }
        //If its more than 1 byte
        if (operation->first_arg_value > 0xFF) {
            operation->is_first_arg_word = 1;
            strcpy(operation->opcode_args, "**");
        } else {
            operation->is_first_arg_word = 0;
            strcpy(operation->opcode_args, "*");
        }
    } else if (arg1[0] == '0' || arg1[0] == '1' || arg1[0] == '2' || arg1[0] == '3' || arg1[0] == '4' ||
               arg1[0] == '5' || arg1[0] == '6' || arg1[0] == '7') {
        //Bits number(0-7)
        operation->first_arg_bits = atoi(arg1);
        operation->is_first_arg_bit = 1;
        strcpy(operation->opcode_args, "n");

        //Now check if this bit is 0-7
        if (operation->first_arg_bits > 7) {
            printf("Error in line %d: there is no bit %d\n", line_number, operation->first_arg_bits);
            return -1;
        }
    } else if (arg1[0] == '(' || arg1[0] == '[') {
        /* If we have a parenthesis or a bracket
        then we must know whats inside */

        //Ok, if we have an number, we do the same as before except for the  final_arg
        if (arg1[1] == '0' || arg1[1] == '$') {
            //If the first letter of the argument is 0 or $, then its a number
            //We must be careful not to write ld (0xFF00+n),x...
            if (strlen(arg1) > 9) {
                printf("Error in line %d, unknown argument %s\n", line_number, arg1);
                return -1;
            }
            if (arg1[2] == 'x') {
                //This is an optional letter to identify hexadecimal numbers
                operation->first_arg_value = atoh(arg1 + 3);
            } else {
                operation->first_arg_value = atoh(arg1 + 2);
            }
            if (operation->first_arg_value > 0xFF) {
                //If its more than 1 byte
                operation->is_first_arg_word = 1;
                strcpy(operation->opcode_args, "(**)");
            } else {
                operation->is_first_arg_word = 0;
                strcpy(operation->opcode_args, "(*)");
            }
        } else if (arg1[1] == 'f' && arg1[2] == 'f' && arg1[3] == 0 && arg1[4] == 0 && arg1[5] == '+' &&
                   arg1[6] != 'c') {
            //If there is no symbol, then it is ff00+number
            operation->first_arg_value = atoh(arg1 + 6);
            operation->is_first_arg_word = 0;
            strcpy(operation->opcode_args, "(ff00+*)");
        } else {
            strcpy(operation->opcode_args, arg1);
        }
    } else {
        //Here we are ready with irregularities, lets just compare and autocomplete
        strcpy(operation->opcode_args, arg1);
    }
    //With this we finish with the first argument
    //Lets go with the second argument which is the same process.
    /**********************************************************/
    //Check if there is argument
    //If there is no argument we do nothing
    //Numbers:

    if (arg2[0] == '0' || arg2[0] == '$') {
        //If the first letter of the argument is 0 or $, then its a number
        if (arg2[1] == 'x') {
            //This is an optional letter to identify hexadecimal numbers
            operation->second_arg_value = atoh(arg2 + 2);
        } else {
            operation->second_arg_value = atoh(arg2 + 1);

        }
        if (operation->second_arg_value > 0xFF) {
            //If its more than 1 byte
            operation->is_second_arg_word = 1;
            strcat(operation->opcode_args, ",**");
        } else {
            operation->is_second_arg_word = 0;
            strcat(operation->opcode_args, ",*");
        }
    } else if (arg2[0] == '0' || arg2[0] == '1' || arg2[0] == '2' || arg2[0] == '3' || arg2[0] == '4' ||
               arg2[0] == '5' || arg2[0] == '6' || arg2[0] == '7') {
        //Bits number(0-7)
        operation->second_arg_bits = atoi(arg2);
        operation->is_second_arg_bit = 1;
        strcat(operation->opcode_args, ",n");
        //Now check if this bit is 0-7
        if (operation->second_arg_bits > 7) {
            printf("Error in line %d: there is no bit %d\n", line_number, operation->second_arg_bits);
            return 1;
        }
    } else if (arg2[0] == '(' || arg2[0] == '[') {
        // Parenthesis:
        // Ok, if we have a parenthesis or a square bracket
        // then we must know whats inside

        //Ok, if we have an number, we do the same as before except for the  final_arg
        if (arg2[1] == '0' || arg2[1] == '$') {
            //If the first letter of the argument is 0 or $, then its a number
            //We must be careful not to write ld (0xFF00+n),x...
            if (strlen(arg2) > 9) {
                printf("Error in line %d, unknown argument %s\n", line_number, arg1);
                return -1;
            }
            if (arg2[2] == 'x') {
                //This is an optional letter to identify hexadecimal numbers
                operation->second_arg_value = atoh(arg2 + 3);
            } else {
                operation->second_arg_value = atoh(arg2 + 2);
            }
            if (operation->second_arg_value > 0xFF) {
                //If its more than 1 byte
                operation->is_second_arg_word = 1;
                strcpy(operation->opcode_args, ",(**)");
            } else {
                operation->is_second_arg_word = 0;
                strcpy(operation->opcode_args, ",(*)");
            }
        } else if (arg2[1] == 'f' && arg2[2] == 'f' && arg2[3] == 0 && arg2[4] == 0 && arg2[5] == '+' &&
                   arg2[6] != 'c') {
            //If there is no symbol, then it is ff00+number
            operation->second_arg_value = atoh(arg1 + 6);
            operation->is_second_arg_word = 0;
            strcpy(operation->opcode_args, ",(ff00+*)");
        }
    } else {
        //Here we are ready with irregularities, lets just compare and autocomplete
        if (arg2[0] != 0) {
            strcat(operation->opcode_args, ",");
            strcat(operation->opcode_args, arg2);
        }
    }
    return 0;
}

int op_compare(char *opcode_name, char *arguments) {
    //This will compare the opcode and the arguments
    //with the table and return its opcode.
    int i;
    for (i = 0; i < 512; i++)//Go through all the table
    {
        if (strcmp(opcode_table[i].op_name, opcode_name) == 0)
            if (strcmp(opcode_table[i].op_args, arguments) == 0) {
                if (opcode_table[i].op_val == 0xCB)
                    return (opcode_table[i].op_val << 8) | (opcode_table[i].op_val2);
                else
                    return opcode_table[i].op_val;
            }
    }
    return -1;
}

int atoh(char *number) {
    char f, s, t, fo;
    int result;
    int fn, sn, tn, fon, flag = 0;
    f = number[0];
    s = number[1];
    t = number[2];
    fo = number[3];

    if (f == '0')
        fn = 0;
    else if (f == '1')
        fn = 1;
    else if (f == '2')
        fn = 2;
    else if (f == '3')
        fn = 3;
    else if (f == '4')
        fn = 4;
    else if (f == '5')
        fn = 5;
    else if (f == '6')
        fn = 6;
    else if (f == '7')
        fn = 7;
    else if (f == '8')
        fn = 8;
    else if (f == '9')
        fn = 9;
    else if (f == 'a' || f == 'A')
        fn = 0xa;
    else if (f == 'b' || f == 'B')
        fn = 0xb;
    else if (f == 'c' || f == 'C')
        fn = 0xc;
    else if (f == 'd' || f == 'D')
        fn = 0xd;
    else if (f == 'e' || f == 'E')
        fn = 0xe;
    else if (f == 'f' || f == 'F')
        fn = 0xf;
    else
        fn = 0;

    if (s == '0')
        sn = 0;
    else if (s == '1')
        sn = 1;
    else if (s == '2')
        sn = 2;
    else if (s == '3')
        sn = 3;
    else if (s == '4')
        sn = 4;
    else if (s == '5')
        sn = 5;
    else if (s == '6')
        sn = 6;
    else if (s == '7')
        sn = 7;
    else if (s == '8')
        sn = 8;
    else if (s == '9')
        sn = 9;
    else if (s == 'a' || s == 'A')
        sn = 0xa;
    else if (s == 'b' || s == 'B')
        sn = 0xb;
    else if (s == 'c' || s == 'C')
        sn = 0xc;
    else if (s == 'd' || s == 'D')
        sn = 0xd;
    else if (s == 'e' || s == 'E')
        sn = 0xe;
    else if (s == 'f' || s == 'F')
        sn = 0xf;
    else
        sn = 0;

    if (t == '0')
        tn = 0;
    else if (t == '1')
        tn = 1;
    else if (t == '2')
        tn = 2;
    else if (t == '3')
        tn = 3;
    else if (t == '4')
        tn = 4;
    else if (t == '5')
        tn = 5;
    else if (t == '6')
        tn = 6;
    else if (t == '7')
        tn = 7;
    else if (t == '8')
        tn = 8;
    else if (t == '9')
        tn = 9;
    else if (t == 'a' || t == 'A')
        tn = 0xa;
    else if (t == 'b' || t == 'B')
        tn = 0xb;
    else if (t == 'c' || t == 'C')
        tn = 0xc;
    else if (t == 'd' || t == 'D')
        tn = 0xd;
    else if (t == 'e' || t == 'E')
        tn = 0xe;
    else if (t == 'f' || t == 'F')
        tn = 0xf;
    else if (t == ')' || t == 0 || t == ']')
        tn = 0;
    else {
        tn = 0;
        flag = 1;
    }

    if (fo == '0')
        fon = 0;
    else if (fo == '1')
        fon = 1;
    else if (fo == '2')
        fon = 2;
    else if (fo == '3')
        fon = 3;
    else if (fo == '4')
        fon = 4;
    else if (fo == '5')
        fon = 5;
    else if (fo == '6')
        fon = 6;
    else if (fo == '7')
        fon = 7;
    else if (fo == '8')
        fon = 8;
    else if (fo == '9')
        fon = 9;
    else if (fo == 'a' || fo == 'A')
        fon = 0xa;
    else if (fo == 'b' || fo == 'B')
        fon = 0xb;
    else if (fo == 'c' || fo == 'C')
        fon = 0xc;
    else if (fo == 'd' || fo == 'D')
        fon = 0xd;
    else if (fo == 'e' || fo == 'E')
        fon = 0xe;
    else if (fo == 'f' || fo == 'F')
        fon = 0xf;
    else
        fon = 0;

    if (flag == 0)
        result = ((fn << 4) | sn) & 0xFF;
    else
        result = ((fn << 12) | (sn << 8) | (tn << 4) | fon) & 0xFFFF;
    return result;
}
