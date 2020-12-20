#include "parameter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool is_bit_number(char number) {
    if (number == '0' || number == '1' || number == '2' || number == '3' || number == '4' || number == '5' ||
        number == '6' || number == '7') {
        return true;
    }
    return false;
}

bool process_first_argument(struct Parameter *parameter, char *arg) {
    parameter->string[0] = 0;
    if (arg[0] == '-') {
        strcpy(parameter->string, "-");
    } else if (arg[0] == '0' || arg[0] == '$') {
        //If the first letter of the argument is 0 or $, then its a hex number
        // This is an optional letter to identify hexadecimal numbers
        if (arg[1] == 'x') {
            parameter->value = (int) strtoul(&arg[2], NULL, 16);
        } else {
            parameter->value = (int) strtoul(&arg[1], NULL, 16);
        }
        //If its more than 1 byte
        if (parameter->value > 0xFF) {
            parameter->is_word = true;
            strcpy(parameter->string, "**");
        } else {
            parameter->is_word = false;
            strcpy(parameter->string, "*");
        }
    } else if (is_bit_number(arg[0])) {
        //Bits number(0-7)
        parameter->bits = strtoul(arg, NULL, 10);
        parameter->is_bit = true;
        strcpy(parameter->string, "n");
    } else if (arg[0] == '(' || arg[0] == '[') {
        /* If we have a parenthesis or a bracket
        then we must know whats inside */

        //Ok, if we have an number, we do the same as before except for the  final_arg
        if (arg[1] == '0' || arg[1] == '$') {
            //If the first letter of the argument is 0 or $, then its a number
            //We must be careful not to write ld (0xFF00+n),x...
            if (strlen(arg) > 9) {
                printf("Unknown argument %s\n", arg);
                return false;
            }
            if (arg[2] == 'x') {
                //This is an optional letter to identify hexadecimal numbers
                parameter->value = strtoul(&arg[3], NULL, 16);
            } else {
                parameter->value = strtoul(&arg[2], NULL, 16);
            }
            if (parameter->value > 0xFF) {
                //If its more than 1 byte
                parameter->is_word = true;
                strcpy(parameter->string, "(**)");
            } else {
                parameter->is_word = false;
                strcpy(parameter->string, "(*)");
            }
        } else if (arg[1] == 'f' && arg[2] == 'f' && arg[3] == 0 && arg[4] == 0 && arg[5] == '+' &&
                   arg[6] != 'c') {
            //If there is no symbol, then it is ff00+number
            parameter->value = strtoul(&arg[6], NULL, 16);
            parameter->is_word = false;
            strcpy(parameter->string, "(ff00+*)");
        } else {
            strcpy(parameter->string, arg);
        }
    } else {
        //Here we are ready with irregularities, lets just compare and autocomplete
        strcpy(parameter->string, arg);
    }
    return true;
}

bool process_second_parameter(struct Parameter *parameter, char *arg) {
    parameter->string[0] = 0;
    if (arg[0] == '-') {
        strcpy(parameter->string, "-");
    } else if (arg[0] == '0' || arg[0] == '$') {
        //If the first letter of the argument is 0 or $, then its a number
        if (arg[1] == 'x') {
            //This is an optional letter to identify hexadecimal numbers
            parameter->value = strtoul(&arg[2], NULL, 16);
        } else {
            parameter->value = strtoul(&arg[1], NULL, 16);
        }
        if (parameter->value > 0xFF) {
            parameter->is_word = true;
            strcpy(parameter->string, ",**");
        } else {
            parameter->is_word = false;
            strcpy(parameter->string, ",*");
        }
    } else if (is_bit_number(arg[0])) {
        //Bits number(0-7)
        parameter->bits = strtoul(arg, NULL, 10);
        parameter->is_bit = true;
        strcpy(parameter->string, ",n");
    } else if (arg[0] == '(' || arg[0] == '[') {
        // Parenthesis:
        // Ok, if we have a parenthesis or a square bracket
        // then we must know whats inside

        //Ok, if we have an number, we do the same as before except for the  final_arg
        if (arg[1] == '0' || arg[1] == '$') {
            //If the first letter of the argument is 0 or $, then its a number
            //We must be careful not to write ld (0xFF00+n),x...
            if (strlen(arg) > 9) {
                printf("Unknown argument %s\n", arg);
                return false;
            }
            if (arg[2] == 'x') {
                //This is an optional letter to identify hexadecimal numbers
                parameter->value = strtoul(&arg[3], NULL, 16);
            } else {
                parameter->value = strtoul(&arg[2], NULL, 16);
            }
            if (parameter->value > 0xFF) {
                //If its more than 1 byte
                parameter->is_word = true;
                strcpy(parameter->string, ",(**)");
            } else {
                parameter->is_word = false;
                strcpy(parameter->string, ",(*)");
            }
        } else if (arg[1] == 'f' && arg[2] == 'f' && arg[3] == 0 && arg[4] == 0 && arg[5] == '+' &&
                   arg[6] != 'c') {
            //If there is no symbol, then it is ff00+number
            parameter->value = strtoul(&arg[6], NULL, 16);
            parameter->is_word = false;
            strcpy(parameter->string, ",(ff00+*)");
        }
    } else {
        //Here we are ready with irregularities, lets just compare and autocomplete
        if (arg[0] != 0) {
            strcat(parameter->string, ",");
            strcat(parameter->string, arg);
        }
    }
    return true;
}