#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "opcode.h"
#include "list.h"
#include "routine.h"
#include "jumps.h"
#include "cli.h"

#define ENTRY_POINT 0x100


int assemble(FILE *in, FILE *out);
void emit_machine_code(FILE *file, struct MachineCode *code);
void emit_entry_point(FILE *file);

void emit_header(FILE *file, struct GBHeader *header) {
    unsigned char nintendo_logo[] = {
            0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
            0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
            0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
            0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    unsigned char zeros[0x400] = {0};
    fwrite(zeros, sizeof(unsigned char), 0x400, file);

    fseek(file, 0x104, SEEK_SET);
    fwrite(&nintendo_logo, 1, sizeof(nintendo_logo), file);
    fseek(file, 0x100, SEEK_SET);
    fputc(0xC3, file);
    fputc(0x50, file);
    fputc(0x01, file);//Put the jump to the 150 address
    fseek(file, 0x150, SEEK_SET);
}


int main(int argc, char *argv[]) {
    char filename[128];
    FILE *asm_file, *gb_file;

    printf("GB-z80 assembler v3 by dospro\n");
    printf("Web: https://github.com/dospro/gb_z80_asm\n");
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
    for (size_t i = strlen(filename); i >= 0; --i) {
        if (filename[i] == '.') {
            filename[i] = 0;
            break;
        }
    }
    strcat(filename, ".gb");
    gb_file = fopen(filename, "w+");
    if (gb_file == NULL) {
        printf("Couldn't create gb file\n");
        fclose(asm_file);
        exit(0);
    }
    emit_header(gb_file, NULL);
    printf("Assembling...\n");

    assemble(asm_file, gb_file);

    fclose(asm_file);
    fclose(gb_file);
    printf("Done\n");
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
    char buffer[512]; // Max number of characters per line
    char opcode[64]; // This will have the opcode string(adc, ld, or, srl....)
    char arg1[64]; // First argument(it should not even get bigger than 16)
    char arg2[64]; // Second argument if there is
    struct List routines_list = create_list();
    struct List jumps_list = create_jumps_list();

    while (!feof(in)) {
        memset(buffer, 0, sizeof(buffer));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        memset(opcode, 0, sizeof(opcode));

        fgets(buffer, 511, in);
        char *clean_buffer = strtrim(buffer);
        if (strlen(clean_buffer) == 0 || clean_buffer[0] == ';') {
            free(clean_buffer);
            ++line_counter;
            continue;
        }
        split_line(clean_buffer, opcode, arg1, arg2);
        free(clean_buffer);

        /* now we have the two argument separated in different variables */

        //If we have a special instruction .include then we must include this.
        if (strcmp(opcode, ".main") == 0) {
            //If we find the option .main then the program starts there not in 0x150
            emit_entry_point(out);
        } else if (strcmp(opcode, ".db") == 0) {
            /* TODO: emit_raw_data(values, size) */
            //If we find a .db option then we write the byte as it is.
            //fputc(atoh(arg1), out);
            printf("Not implemented yet\n");
        } else if (strchr(opcode, ':') != NULL) {
            // If there is a : in the opcode, then its a routine
            /* TODO: we can have a name and an opcode in the same line */
            add_routine(&routines_list, opcode, ftell(out));
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

            struct MachineCode code;
            if ((code.opcode = search_opcode(opcode, argument)) == -1) {
                printf("ERROR: Line %d. opcode: %s with arguments %s has an error\n", line_counter, opcode, argument);
            } else {
                printf("[%s %s] -> ", opcode, argument);
                code.params_size = 2;
                code.params[0] = 0;
                code.params[1] = 0;
                emit_machine_code(out, &code);
            }
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

            struct MachineCode code;
            code.opcode = search_opcode(opcode, argument);
            if (code.opcode == -1) {
                printf("ERROR Line %d. Unknown Opcode: %s with arguments %s\n", line_counter, opcode, argument);
            } else {
                printf("[%s %s] -> ", opcode, argument);
                code.params_size = 1;
                code.params[0] = 0;
                emit_machine_code(out, &code);
            }
        } else {
            /* All other instructions */
            struct MachineCode code;
            if (get_general_opcode(&code, opcode, arg1, arg2) == false) {
                printf("Error occurred on line %d\n", line_counter);
            } else {
                emit_machine_code(out, &code);
            }
        }
        line_counter++;
    }
    // Now lets finish with the routines(calls) and jumps
    // First we will go for each call/jp made and search it in the routines list
    for (struct Jump *jump = pop_jump(&jumps_list); jump != NULL; jump = pop_jump(&jumps_list)) {
        struct Routine *routine = search_routine_by_name(&routines_list, jump->name);
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
    free_routines(&routines_list);
    //Everything resulted ok, no errors
    return 0;
}

/* Emitter */

void emit_machine_code(FILE *file, struct MachineCode *code) {
    printf("Emitting: [%x]:[%x] ", ftell(file) + 1, code->opcode);
    fputc(code->opcode, file);
    for (int i = 0; i < code->params_size; ++i) {
        printf("Emitting arg: [%x]:[%x] ", ftell(file) + 1, code->params[i]);
        fputc(code->params[i], file);
    }
    printf("\n");
}

void emit_entry_point(FILE *file) {
    long seek_temp = ftell(file);
    fseek(file, ENTRY_POINT, SEEK_SET); // get to the program start
    fputc(0xC3, file); // Write the new jump
    fputc((int) seek_temp & 0xFF, file);
    fputc((int) (seek_temp >> 8) & 0xFF, file);
    fseek(file, seek_temp, SEEK_SET);
}
