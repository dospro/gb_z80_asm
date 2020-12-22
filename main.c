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
#include "parameter.h"

struct Arguments {
    char opcode_args[64];
    struct Parameter arg1;
    struct Parameter arg2;
};


int assemble(FILE *in, FILE *out);
int search_opcode(char *opcode_name, char *arguments);
int scan_arguments(struct Arguments *operation, char *arg1, char *arg2, int line_number);

int main(int argc, char *argv[]) {
    char filename[128];
    FILE *asm_file, *gb_file;
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
    strtok(in_buffer, ";"); // Will place a 0 where the ; is
    char *token;
    //if (token != NULL)
    //    strcpy(in_buffer, token); // No, copy the new buffer without comments
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

bool is_conditional_flag(char *arg) {
    if (strcmp(arg, "nz") == 0 || strcmp(arg, "z") == 0 || strcmp(arg, "nc") == 0 || strcmp(arg, "c") == 0) {
        return true;
    }
    return false;
}

bool is_branch_opcode(char *opcode, char *arg) {
    if (strcmp(opcode, "call") == 0) {
        return true;
    }
    if (strcmp(opcode, "jp") == 0 && strcmp(arg, "(hl)") != 0 && strcmp(arg, "[hl]") != 0) {
        return true;
    }
    return false;
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
    struct RoutineList routines_list = create_routines_list();
    struct JumpList jumps_list = create_jumps_list();
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
            free(clean_buffer);
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
            long seek_temp = ftell(out);
            fseek(out, 0x100, SEEK_SET);//get to the program start
            fputc(0xC3, out);//Wrtie the new jump
            fputc((int) seek_temp & 0xFF, out);
            fputc((int) (seek_temp >> 8) & 0xFF, out);
            fseek(out, seek_temp, SEEK_SET);
        } else if (strcmp(opcode, ".db") == 0) {
            //If we find a .db option then we write the byte as it is.
            //fputc(atoh(arg1), out);
            printf("Not implemented yet\n");
        } else if (strchr(opcode, ':') != NULL) {
            // If there is a : in the opcode, then its a routine
            add_routine(&routines_list, strtok(opcode, ":"), ftell(out));
        } else if (is_branch_opcode(opcode, arg1) == true) {
            /* If we get a call or a jp then there is a routine name
             * We must take this routine name and keep it so at the end
             * we will put the right address
             */
            char routine_name[64];
            char argument[64];
            if (is_conditional_flag(arg1) == true) {
                strcpy(routine_name, arg2);
                strcpy(argument, arg1);
                strcat(argument, ",dir");
            } else {
                strcpy(routine_name, arg1);
                strcpy(argument, "dir");
            }
            /* ftell(out) + 1 is the position where we will fill the real routine address */
            add_jump(&jumps_list, routine_name, ftell(out) + 1, 0, 16);

            int result = search_opcode(opcode, argument);
            if (result == -1) {
                printf("ERROR: Line %d. opcode: %s with arguments %s has an error\n", line_counter, opcode, argument);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
            fputc(0, out);//Fill with generic 0
        } else if (strcmp(opcode, "jr") == 0) {
            /*
             * jr is almost the same as call and jp but it only uses a 1-byte address
             */
            char routine_name[64];
            char argument[64];
            if (is_conditional_flag(arg1) == true) {
                strcpy(routine_name, arg2);
                strcpy(argument, arg1);
                strcat(argument, ",dir");
            } else {
                strcpy(routine_name, arg1);
                strcpy(argument, "dir");
            }

            add_jump(&jumps_list, routine_name, ftell(out) + 1, 0, 8);

            int result = search_opcode(opcode, argument);
            if (result == -1) {
                printf("Error line %d, opcode: %s with arguments %s has an error or its an asembler bug\n",
                       line_counter, opcode,
                       argument);
            }
            fputc(result, out);//Write the opcode
            fputc(0, out);//Fill with generic 0
        } else {
            /* All other instructions */
            struct Arguments arguments;
            scan_arguments(&arguments, arg1, arg2, line_counter);
            int result = search_opcode(opcode, arguments.opcode_args);
            if (result == -1) {
                printf("error: line %d \n", line_counter);
                printf("Opcode %s with arguments %s was no found\n", opcode, final_arg);
            } else if ((result >> 8) == 0xCB) {
                //Now lets write it based in some rules
                //If we have a CB opcode, then we must write the 2 values
                if (arguments.arg1.type == BIT) {
                    //If we have a bit opcode then we introduce it inside the value
                    result = result & 0xFF;
                    result = result | (arguments.arg1.value << 3); // Here we introduce the number of the bit
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
                if (arguments.arg1.type == BYTE) {
                    fputc(result, out);
                    fputc(arguments.arg1.value, out);
                } else if (arguments.arg1.type == WORD) {
                    fputc(result, out);
                    fputc((arguments.arg1.value & 0xFF), out);//write the less significant byte
                    fputc((arguments.arg1.value >> 8) & 0xFF, out);//write the most significant byte
                } else if (arguments.arg2.type == BYTE) {
                    fputc(result, out);
                    fputc(arguments.arg2.value, out);
                } else if (arguments.arg2.type == WORD) {
                    fputc(result, out); // write the opcode
                    fputc((arguments.arg2.value & 0xFF), out); // write the less significant byte
                    fputc((arguments.arg2.value >> 8) & 0xFF, out); // write the most significant byte
                } else {
                    fputc(result, out);
                }
            }
        }
        line_counter++;
    }
    //Now lets finish with the routines(calls) and jumps
    //Fisrt we will go for each call/jp made and search it in the routines list
    for (struct Jump *jump = pop_jump(&jumps_list); jump != NULL; jump = pop_jump(&jumps_list)) {
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
 * Routine that will take an opcode arguments and produce
 * data structure Arguments with information about the
 * scanned arguments.
 */
int scan_arguments(struct Arguments *operation, char *arg1, char *arg2, int line_number) {
    if (process_first_argument(&operation->arg1, arg1) == false) {
        printf("ERROR: Line %d Invalid first argument.\n", line_number);
        return -1;
    }
    if (process_second_parameter(&operation->arg2, arg2) == false) {
        printf("ERROR: Line %d Invalid second argument.\n", line_number);
        return -1;
    }
    strcpy(operation->opcode_args, operation->arg1.string);
    strcat(operation->opcode_args, operation->arg2.string);

    return 0;
}

int search_opcode(char *opcode_name, char *arguments) {
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
