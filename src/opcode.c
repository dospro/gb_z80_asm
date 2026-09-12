#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "opcode.h"

#include <stdlib.h>

#include "parameter.h"

struct Opcode opcode_table[];

/**
 * Makes a search in the look-up table and return the corresponding opcode number
 * @param opcode_name String with the opcode name
 * @param arguments String with the arguments
 * @return Opcode number
 */
int search_opcode(char* opcode_name, char* arguments)
{
    for (int i = 0; i < 512; i++)
    {
        if (strcmp(opcode_table[i].name, opcode_name) == 0)
            if (strcmp(opcode_table[i].arg_string, arguments) == 0)
            {
                if (opcode_table[i].value == 0xCB)
                    return (opcode_table[i].value << 8) | (opcode_table[i].high_value);
                else
                    return opcode_table[i].value;
            }
    }
    return -1;
}

/**
 * Parses a hexadecimal number from a String argument.
 * Supports both "$" and "0x" prefixes.
 * @param operand_string The string argument to parse
 * @param error_code Output error code
 * @return Parameter containing the parsed value and type
 */
Operand parse_operand(const String operand_string, ErrorCode* error_code)
{
    *error_code = GBASM_CODE_OK;
    int state = 0;
    size_t index = 0;
    int parsed_value = 0;
    bool parse_successful = false;
    bool continue_parsing = true;

    while (index < operand_string.size && continue_parsing)
    {
        const char current_char = string_at(operand_string, index);

        switch (state)
        {
            case 0:
                if (current_char == '0')
                {
                    state = 1;
                    index++;
                }
                else if (current_char == '$')
                {
                    state = 2;
                    index++;
                }
                else
                {
                    continue_parsing = false;
                }
                break;

            case 1:
                if (current_char == 'x' || current_char == 'X')
                {
                    state = 2;
                    index++;
                }
                else
                {
                    continue_parsing = false;
                }
                break;

            case 2:
                if (isxdigit(current_char))
                {
                    int digit_value = 0;
                    if (current_char >= '0' && current_char <= '9')
                    {
                        digit_value = current_char - '0';
                    }
                    else if (current_char >= 'a' && current_char <= 'f')
                    {
                        digit_value = current_char - 'a' + 10;
                    }
                    else if (current_char >= 'A' && current_char <= 'F')
                    {
                        digit_value = current_char - 'A' + 10;
                    }
                    parsed_value = digit_value;
                    state = 3;
                    index++;
                }
                else
                {
                    continue_parsing = false;
                }
                break;

            case 3:
                if (isxdigit(current_char))
                {
                    int digit_value = 0;
                    if (current_char >= '0' && current_char <= '9')
                    {
                        digit_value = current_char - '0';
                    }
                    else if (current_char >= 'a' && current_char <= 'f')
                    {
                        digit_value = current_char - 'a' + 10;
                    }
                    else if (current_char >= 'A' && current_char <= 'F')
                    {
                        digit_value = current_char - 'A' + 10;
                    }
                    parsed_value = (parsed_value << 4) | digit_value;
                    index++;
                }
                else
                {
                    parse_successful = true;
                    continue_parsing = false;
                }
                break;
        }
    }

    if (state == 3)
    {
        parse_successful = true;
    }

    if (!parse_successful)
    {
        *error_code = GBASM_CODE_ERROR;
        return (Operand){};
    }

    return (Operand){
        .value = parsed_value,
        .type = BYTE
    };
}

/**
 * @brief Parse an assembly opcode line into its machine code representation.
 *
 * Looks up the opcode name and arguments in the opcode table and produces
 * the corresponding machine code bytes, including any immediate operands.
 *
 * @param[in]  opcode_parts  The opcode mnemonic and up to two arguments.
 * @param[out] error_code    Set to GBASM_CODE_OK on success or GBASM_CODE_ERROR on failure.
 * @return The resulting MachineCode with the opcode value and parsed operand bytes.
 *         Returns a zero-initialized MachineCode on error.
 */
MachineCode parse_opcode(const OpcodeTextParts opcode_parts, ErrorCode* error_code)
{
    constexpr size_t table_size = 512;
    *error_code = GBASM_CODE_OK;
    StringBuffer buffer = StringBuffer_new(1, malloc);
    Operand operand = {}, operand2 = {};
    if (string_is_empty(opcode_parts.arg1) && string_is_empty(opcode_parts.arg2))
    {
        StringBuffer_append_cstr(&buffer, "-");
    }
    else if (string_is_empty(opcode_parts.arg2))
    {
        // When the second operand is empty, then we just need to parse the first operand.
        ErrorCode parse_error;
        operand = parse_operand(opcode_parts.arg1, &parse_error);

        // TODO: Not the best detection logic here.
        if (parse_error == GBASM_CODE_OK)
        {
            StringBuffer_append_cstr(&buffer, "*");
        }
        else
        {
            StringBuffer_append_string(&buffer, opcode_parts.arg1);
        }
    }
    else if (string_is_empty(opcode_parts.arg1) && !string_is_empty(opcode_parts.arg2))
    {
        // When the first operand is empty, we cannot have a second operand.
        StringBuffer_free(&buffer);
        *error_code = GBASM_CODE_ERROR;
        return (MachineCode){};
    }
    else
    {
        // In this case, both operands are non-empty, so we need to parse both.
        StringBuffer_append_string(&buffer, opcode_parts.arg1);
        StringBuffer_append_cstr(&buffer, ",");
        ErrorCode parse_error;
        operand2 = parse_operand(opcode_parts.arg2, &parse_error);
        if (parse_error == GBASM_CODE_OK)
        {
            StringBuffer_append_cstr(&buffer, "*");
        }
        else
        {
            StringBuffer_append_string(&buffer, opcode_parts.arg2);
        }
    }

    // Let's do the lookout
    const String arg_string = string_from_string_buffer(buffer);
    for (size_t i = 0; i < table_size; i++)
    {
        if (string_is_equal_cstr(opcode_parts.name, opcode_table[i].name) &&
            string_is_equal_cstr(arg_string, opcode_table[i].arg_string))
        {
            if (operand.type == BYTE)
            {
                StringBuffer_free(&buffer);
                return (MachineCode){.opcode = opcode_table[i].value, .params_size = 1, .params = {operand.value}};
            }
            else if (operand2.type == BYTE)
            {
                StringBuffer_free(&buffer);
                return (MachineCode){.opcode = opcode_table[i].value, .params_size = 1, .params = {operand2.value}};
            }
            else
            {
                StringBuffer_free(&buffer);
                return (MachineCode){.opcode = opcode_table[i].value, .params_size = 0, .params = {0}};
            }
        }
    }
    StringBuffer_free(&buffer);
    *error_code = GBASM_CODE_ERROR;
    return (MachineCode){};
}

bool get_general_opcode(struct MachineCode *machine_code_out, char *opcode_name, char *arg1, char *arg2) {
    OpcodeOperands arguments;
    if(scan_arguments(&arguments, arg1, arg2) == false) {
        return false;
    }
    int result = search_opcode(opcode_name, arguments.opcode_args);
    printf("[%s %s] -> ", opcode_name, arguments.opcode_args);
    if (result == -1) {
        printf("Opcode %s with arguments %s was no found\n", opcode_name, arguments.opcode_args);
        return false;
    } else if ((result >> 8) == 0xCB) {
        //Now lets write it based in some rules
        //If we have a CB opcode_name, then we must write the 2 values
        if (arguments.arg1.type == BIT) {
            //If we have a bit opcode_name then we introduce it inside the value
            result = result & 0xFF;
            result = result | (arguments.arg1.value << 3); // Here we introduce the number of the bit
            machine_code_out->opcode = 0xCB;
            machine_code_out->params_size = 1;
            machine_code_out->params[0] = result;
        } else {
            machine_code_out->opcode = 0xCB;
            machine_code_out->params_size = 1;
            machine_code_out->params[0] = result & 0xFF;
        }
    } else if (result == 0x10) {
        // stop which has 2 opcodes
        machine_code_out->opcode = 0x10;
        machine_code_out->params_size = 1;
        machine_code_out->params[0] = 0;
    } else {
        machine_code_out->opcode = result;
        if (arguments.arg1.type == BYTE) {
            machine_code_out->params_size = 1;
            machine_code_out->params[0] = arguments.arg1.value;
        } else if (arguments.arg1.type == WORD) {
            machine_code_out->params_size = 2;
            machine_code_out->params[0] = arguments.arg1.value & 0xFF;
            machine_code_out->params[1] = (arguments.arg1.value >> 8) & 0xFF;
        } else if (arguments.arg2.type == BYTE) {
            machine_code_out->params_size = 1;
            machine_code_out->params[0] = arguments.arg2.value;
        } else if (arguments.arg2.type == WORD) {
            machine_code_out->params_size = 2;
            machine_code_out->params[0] = arguments.arg2.value & 0xFF;
            machine_code_out->params[1] = (arguments.arg2.value >> 8) & 0xFF;
        } else {
            machine_code_out->params_size = 0;
        }
    }
    return true;
}


/**
 * Advances *pos past any leading whitespace in line.
 *
 * @param line The string to scan.
 * @param pos  Pointer to the current position; updated in place.
 */
static void skip_spaces(const String line, size_t* pos)
{
    while (*pos < line.size && isspace((unsigned char)line.data[*pos]))
    {
        ++(*pos);
    }
}

/**
 * Reads the next token from line starting at *pos, stopping at a
 * space, comma, or end of string. Returns a String view into line
 * and advances *pos past the token.
 *
 * @param line The string to read from.
 * @param pos  Pointer to the current position; updated past the token.
 * @return A String view of the extracted token (empty if *pos is at end).
 */
static String read_token(const String line, size_t* const pos)
{
    const size_t start = *pos;
    while (*pos < line.size && !isspace((unsigned char)line.data[*pos]) && line.data[*pos] != ',')
    {
        ++(*pos);
    }
    return (String){.data = &line.data[start], .size = (*pos) - start};
}

/**
 * Splits an assembly line into its component parts: opcode name,
 * first operand, and second operand. Uses spaces and commas as
 * delimiters. The returned strings are views into the original line.
 *
 * @param line       The raw assembly line to parse (e.g. "ld a, b").
 * @param error_code Output parameter for any parsing errors.
 * @return           OpcodeTextParts with name, arg1, and arg2 fields.
 */
OpcodeTextParts split_line_new(const String line, ErrorCode *error_code)
{
    size_t pos = 0;
    *error_code = GBASM_CODE_OK;

    const String opcode_name = read_token(line, &pos);
    skip_spaces(line, &pos);

    const String arg1 = read_token(line, &pos);

    // Let's skip the comma character
    if (pos < line.size && line.data[pos] == ',')
    {
        ++pos;
    }
    skip_spaces(line, &pos);
    const String arg2 = read_token(line, &pos);

    // Is there more text?
    skip_spaces(line, &pos);
    if (pos < line.size)
    {
        *error_code = GBASM_ERR_MALFORMED_LINE;
    }

    return (OpcodeTextParts){
        .name = opcode_name,
        .arg1 = arg1,
        .arg2 = arg2,
    };
}

Opcode opcode_table[] = {
        {"adc",  "a,a",        0x8F, 0},/*Add with carry*/
        {"adc",  "a,b",        0x88, 0},
        {"adc",  "a,c",        0x89, 0},
        {"adc",  "a,d",        0x8a, 0},
        {"adc",  "a,e",        0x8b, 0},
        {"adc",  "a,h",        0x8c, 0},
        {"adc",  "a,l",        0x8d, 0},
        {"adc",  "a,(hl)",     0x8e, 0},
        {"adc",  "a,[hl]",     0x8e, 0},
        {"adc",  "a,*",        0xce},

        {"add",  "a,a",        0x87},/*Addition*/
        {"add",  "a,b",        0x80},
        {"add",  "a,c",        0x81},
        {"add",  "a,d",        0x82},
        {"add",  "a,e",        0x83},
        {"add",  "a,h",        0x84},
        {"add",  "a,l",        0x85},
        {"add",  "a,(hl)",     0x86},
        {"add",  "a,[hl]",     0x86},
        {"add",  "a,*",        0xC6},

        {"add",  "hl,bc",      0x09},
        {"add",  "hl,de",      0x19},
        {"add",  "hl,hl",      0x29},
        {"add",  "hl,sp",      0x39},
        {"add",  "sp,*",       0xE8},

        {"and",  "a",          0xa7},
        {"and",  "b",          0xa0},
        {"and",  "c",          0xa1},
        {"and",  "d",          0xa2},
        {"and",  "e",          0xa3},
        {"and",  "h",          0xa4},
        {"and",  "l",          0xa5},
        {"and",  "(hl)",       0xa6},
        {"and",  "[hl]",       0xa6},
        {"and",  "*",          0xe6},

        {"bit",  "n,a",        0xCB, 0x47},
        {"bit",  "n,b",        0xCB, 0x40},
        {"bit",  "n,c",        0xCB, 0x41},
        {"bit",  "n,d",        0xCB, 0x42},
        {"bit",  "n,e",        0xCB, 0x43},
        {"bit",  "n,h",        0xCB, 0x44},
        {"bit",  "n,l",        0xCB, 0x45},
        {"bit",  "n,(hl)",     0xCB, 0x46},
        {"bit",  "n,[hl]",     0xCB, 0x46},

        {"call", "nz,dir",     0xC4},
        {"call", "z,dir",      0xCC},
        {"call", "nc,dir",     0xD4},
        {"call", "c,dir",      0xDC},
        {"call", "dir",        0xC4},

        {"ccf",  "-",          0x3f},
        {"cpl",  "-",          0x2F},

        {"cp",   "a",          0xBF},
        {"cp",   "b",          0xB8},
        {"cp",   "c",          0xB9},
        {"cp",   "d",          0xBa},
        {"cp",   "e",          0xBb},
        {"cp",   "h",          0xBc},
        {"cp",   "l",          0xBd},
        {"cp",   "(hl)",       0xBe},
        {"cp",   "[hl]",       0xBe},//The same as before
        {"cp",   "*",          0xFe},

        {"daa",  "-",          0x27},

        {"dec",  "a",          0x3D},
        {"dec",  "b",          0x05},
        {"dec",  "c",          0x0D},
        {"dec",  "d",          0x15},
        {"dec",  "e",          0x1D},
        {"dec",  "h",          0x25},
        {"dec",  "l",          0x2D},
        {"dec",  "(hl)",       0x35},
        {"dec",  "[hl]",       0x35},//the same as before
        {"dec",  "bc",         0x0b},
        {"dec",  "de",         0x1b},
        {"dec",  "hl",         0x2b},
        {"dec",  "sp",         0x3b},

        {"di",   "-",          0xF3},
        {"ei",   "-",          0xfb},
        {"halt", "-",          0x76},

        {"inc",  "a",          0x3C},
        {"inc",  "b",          0x04},
        {"inc",  "c",          0x0c},
        {"inc",  "d",          0x14},
        {"inc",  "e",          0x1C},
        {"inc",  "h",          0x24},
        {"inc",  "l",          0x2C},
        {"inc",  "(hl)",       0x34},
        {"inc",  "[hl]",       0x34},
        {"inc",  "bc",         0x03},
        {"inc",  "de",         0x13},
        {"inc",  "hl",         0x23},
        {"inc",  "sp",         0x33},

        {"jp",   "(hl)",       0xe9},
        {"jp",   "[hl]",       0xe9},
        {"jp",   "nz,dir",     0xc2},
        {"jp",   "z,dir",      0xca},
        {"jp",   "nc,dir",     0xd2},
        {"jp",   "c,dir",      0xda},
        {"jp",   "dir",        0xC3},

        {"jr",   "nz,dir",     0x20},
        {"jr",   "z,dir",      0x28},
        {"jr",   "nc,dir",     0x30},
        {"jr",   "c,dir",      0x38},
        {"jr",   "dir",        0x18},

        /*All these are the different ways of writing the opcode parameters*/
        /*The next opcodes mean the same*/
        {"ld",   "(ff00+c),a", 0xE2},
        {"ld",   "[ff00+c],a", 0xE2},
        {"ld",   "(c),a",      0xE2},
        {"ld",   "[c],a",      0xE2},
        /*Finish the opcodes that means the same*/

        /*All this is the same opcode*/
        {"ld",   "(hli),a",    0x22},
        {"ld",   "[hli],a",    0x22},
        {"ld",   "(hl+),a",    0x22},
        {"ld",   "[hl+],a",    0x22},
        {"ldi",  "(hl),a",     0x22},
        {"ldi",  "[hl],a",     0x22},
        /*Ends*/

        /*All these are the same*/
        {"ld",   "(hld),a",    0x32},
        {"ld",   "[hld],a",    0x32},
        {"ld",   "(hl-),a",    0x32},
        {"ld",   "[hl-],a",    0x32},
        {"ldd",  "(hl),a",     0x32},
        {"ldd",  "[hl],a",     0x32},
        /*ends*/

        {"ld",   "(**),SP",    0x08},
        {"ld",   "[**],SP",    0x08},//the same as before

        /*The same*/
        {"ld",   "a,(ff00+c)", 0xF2},
        {"ld",   "a,[ff00+c]", 0xF2},
        {"ld",   "a,(c)",      0xF2},
        {"ld",   "a,[c]",      0xF2},
        /*ends*/

        /*All this is the same opcode*/
        {"ld",   "a,(hli)",    0x2A},
        {"ld",   "a,[hli]",    0x2a},
        {"ld",   "a,(hl+)",    0x2a},
        {"ld",   "a,[hl+]",    0x2a},
        {"ldi",  "a,(hl)",     0x2a},
        {"ldi",  "a,[hl]",     0x2a},
        /*Ends*/

        /*All these are the same*/
        {"ld",   "a,(hld)",    0x3a},
        {"ld",   "a,[hld]",    0x3a},
        {"ld",   "a,(hl-)",    0x3a},
        {"ld",   "a,[hl-]",    0x3a},
        {"ldd",  "a,(hl)",     0x3a},
        {"ldd",  "a,[hl]",     0x3a},
        /*ends*/

        {"ld",   "a,a",        0x7f},
        {"ld",   "a,b",        0x78},
        {"ld",   "a,c",        0x79},
        {"ld",   "a,d",        0x7a},
        {"ld",   "a,e",        0x7b},
        {"ld",   "a,h",        0x7c},
        {"ld",   "a,l",        0x7d},
        {"ld",   "a,(bc)",     0x0a},
        {"ld",   "a,[bc]",     0x0a},////
        {"ld",   "a,(de)",     0x1a},
        {"ld",   "a,[de]",     0x1a},///
        {"ld",   "a,(hl)",     0x7e},
        {"ld",   "a,[hl]",     0x7e},///
        {"ld",   "a,(**)",     0xfa},
        {"ld",   "a,[**]",     0xfa},///
        {"ld",   "a,*",        0x3e},

        /*the same*/
        {"ld",   "(ff00+*),a", 0xe0},
        {"ld",   "[ff00+*],a", 0xe0},
        {"ldh",  "(*),a",      0xe0},
        {"ldh",  "[*],a",      0xe0},
        /*ends*/

        /*the same*/
        {"ld",   "a,(ff00+*)", 0xf0},
        {"ld",   "a,[ff00+*]", 0xf0},
        {"ldh",  "a,(*)",      0xf0},
        {"ldh",  "a,[*]",      0xf0},
        /*ends*/

        {"ld",   "b,a",        0x47},
        {"ld",   "c,a",        0x4f},
        {"ld",   "d,a",        0x57},
        {"ld",   "e,a",        0x5f},
        {"ld",   "h,a",        0x67},
        {"ld",   "l,a",        0x6f},
        {"ld",   "(bc),a",     0x02},
        {"ld",   "[bc],a",     0x02},
        {"ld",   "(de),a",     0x12},
        {"ld",   "[de],a",     0x12},
        {"ld",   "(hl),a",     0x77},
        {"ld",   "[hl],a",     0x77},
        {"ld",   "(**),a",     0xea},
        {"ld",   "[**],a",     0xea},

        {"ld",   "bc,**",      0x01},
        {"ld",   "de,**",      0x11},
        {"ld",   "hl,**",      0x21},
        {"ld",   "sp,**",      0x31},

        {"ld",   "b,*",        0x06},
        {"ld",   "c,*",        0x0e},
        {"ld",   "d,*",        0x16},
        {"ld",   "e,*",        0x1e},
        {"ld",   "h,*",        0x26},
        {"ld",   "l,*",        0x2e},

        {"ld",   "b,b",        0x40},
        {"ld",   "b,c",        0x41},
        {"ld",   "b,d",        0x42},
        {"ld",   "b,e",        0x43},
        {"ld",   "b,h",        0x44},
        {"ld",   "b,l",        0x45},
        {"ld",   "b,(hl)",     0x46},
        {"ld",   "b,[hl]",     0x46},
        {"ld",   "c,b",        0x48},
        {"ld",   "c,c",        0x49},
        {"ld",   "c,d",        0x4a},
        {"ld",   "c,e",        0x4b},
        {"ld",   "c,h",        0x4c},
        {"ld",   "c,l",        0x4d},
        {"ld",   "c,(hl)",     0x4e},
        {"ld",   "c,[hl]",     0x4e},
        {"ld",   "d,b",        0x50},
        {"ld",   "d,c",        0x51},
        {"ld",   "d,d",        0x52},
        {"ld",   "d,e",        0x53},
        {"ld",   "d,h",        0x54},
        {"ld",   "d,l",        0x55},
        {"ld",   "d,(hl)",     0x56},
        {"ld",   "d,[hl]",     0x56},
        {"ld",   "e,b",        0x58},
        {"ld",   "e,c",        0x59},
        {"ld",   "e,d",        0x5a},
        {"ld",   "e,e",        0x5b},
        {"ld",   "e,h",        0x5c},
        {"ld",   "e,l",        0x5d},
        {"ld",   "e,(hl)",     0x5e},
        {"ld",   "e,[hl]",     0x5e},
        {"ld",   "h,b",        0x60},
        {"ld",   "h,c",        0x61},
        {"ld",   "h,d",        0x62},
        {"ld",   "h,e",        0x63},
        {"ld",   "h,h",        0x64},
        {"ld",   "h,l",        0x65},
        {"ld",   "h,(hl)",     0x66},
        {"ld",   "h,[hl]",     0x66},
        {"ld",   "l,b",        0x68},
        {"ld",   "l,c",        0x69},
        {"ld",   "l,d",        0x6a},
        {"ld",   "l,e",        0x6b},
        {"ld",   "l,h",        0x6c},
        {"ld",   "l,l",        0x6d},
        {"ld",   "l,(hl)",     0x6e},
        {"ld",   "l,[hl]",     0x6e},
        {"ld",   "(hl),b",     0x70},
        {"ld",   "(hl),c",     0x71},
        {"ld",   "(hl),d",     0x72},
        {"ld",   "(hl),e",     0x73},
        {"ld",   "(hl),h",     0x74},
        {"ld",   "(hl),l",     0x75},
        {"ld",   "(hl),*",     0x36},
        {"ld",   "[hl],b",     0x70},
        {"ld",   "[hl],c",     0x71},
        {"ld",   "[hl],d",     0x72},
        {"ld",   "[hl],e",     0x73},
        {"ld",   "[hl],h",     0x74},
        {"ld",   "[hl],l",     0x75},
        {"ld",   "[hl],*",     0x36},

        {"ld",   "sp,hl",      0xf9},

        {"nop",  "-",          0x0},

        {"or",   "a",          0xB7},
        {"or",   "b",          0xB0},
        {"or",   "c",          0xB1},
        {"or",   "d",          0xB2},
        {"or",   "e",          0xB3},
        {"or",   "h",          0xB4},
        {"or",   "l",          0xB5},
        {"or",   "(hl)",       0xB6},
        {"or",   "[hl]",       0xB6},
        {"or",   "*",          0xF6},

        {"pop",  "af",         0xF1},
        {"pop",  "bc",         0xC1},
        {"pop",  "de",         0xD1},
        {"pop",  "hl",         0xE1},

        {"push", "af",         0xF5},
        {"push", "bc",         0xC5},
        {"push", "de",         0xD5},
        {"push", "hl",         0xE5},

        {"res",  "n,a",        0xCB, 0x87},
        {"res",  "n,b",        0xCB, 0x80},
        {"res",  "n,c",        0xCB, 0x81},
        {"res",  "n,d",        0xCB, 0x82},
        {"res",  "n,e",        0xCB, 0x83},
        {"res",  "n,h",        0xCB, 0x84},
        {"res",  "n,l",        0xCB, 0x85},
        {"res",  "n,(hl)",     0xCB, 0x86},
        {"res",  "n,[hl]",     0xCB, 0x86},

        {"ret",  "-",          0xC9},
        {"ret",  "nz",         0xC0},
        {"ret",  "z",          0xC8},
        {"ret",  "nc",         0xD0},
        {"ret",  "c",          0xD8},
        {"reti", "-",          0xD9},

        {"rla",  "-",          0x17},
        {"rlca", "-",          0x07},
        {"rlc",  "a",          0xCB, 0x07},
        {"rlc",  "b",          0xCB, 0x00},
        {"rlc",  "c",          0xCB, 0x01},
        {"rlc",  "d",          0xCB, 0x02},
        {"rlc",  "e",          0xCB, 0x03},
        {"rlc",  "h",          0xCB, 0x04},
        {"rlc",  "l",          0xCB, 0x05},
        {"rlc",  "(hl)",       0xCB, 0x06},
        {"rlc",  "[hl]",       0xCB, 0x06},

        {"rl",   "a",          0xCB, 0x17},
        {"rl",   "b",          0xCB, 0x10},
        {"rl",   "c",          0xCB, 0x11},
        {"rl",   "d",          0xCB, 0x12},
        {"rl",   "e",          0xCB, 0x13},
        {"rl",   "h",          0xCB, 0x14},
        {"rl",   "l",          0xCB, 0x15},
        {"rl",   "(hl)",       0xCB, 0x16},
        {"rl",   "[hl]",       0xCB, 0x16},

        {"rra",  "-",          0x1F},
        {"rrca", "-",          0x0F},

        {"rrc",  "a",          0xCB, 0x0F},
        {"rrc",  "b",          0xCB, 0x08},
        {"rrc",  "c",          0xCB, 0x09},
        {"rrc",  "d",          0xCB, 0x0A},
        {"rrc",  "e",          0xCB, 0x0B},
        {"rrc",  "h",          0xCB, 0x0C},
        {"rrc",  "l",          0xCB, 0x0D},
        {"rrc",  "(hl)",       0xCB, 0x0E},
        {"rrc",  "[hl]",       0xCB, 0x0E},

        {"rr",   "a",          0xCB, 0x1F},
        {"rr",   "b",          0xCB, 0x18},
        {"rr",   "c",          0xCB, 0x19},
        {"rr",   "d",          0xCB, 0x1A},
        {"rr",   "e",          0xCB, 0x1B},
        {"rr",   "h",          0xCB, 0x1C},
        {"rr",   "l",          0xCB, 0x1D},
        {"rr",   "(hl)",       0xCB, 0x1E},
        {"rr",   "[hl]",       0xCB, 0x1E},

        {"rst",  "00",         0xC7},
        {"rst",  "08",         0xCF},
        {"rst",  "10",         0xD7},
        {"rst",  "18",         0xDF},
        {"rst",  "20",         0xE7},
        {"rst",  "28",         0xEF},
        {"rst",  "30",         0xF7},
        {"rst",  "38",         0xFF},

        {"sbc",  "a,a",        0x9F},
        {"sbc",  "a,b",        0x98},
        {"sbc",  "a,c",        0x99},
        {"sbc",  "a,d",        0x9A},
        {"sbc",  "a,e",        0x9B},
        {"sbc",  "a,h",        0x9C},
        {"sbc",  "a,l",        0x9D},
        {"sbc",  "a,(hl)",     0x9E},
        {"sbc",  "a,[hl]",     0x9E},
        {"sbc",  "a,*",        0xDE},

        {"scf",  "-",          0x37},

        {"set",  "n,a",        0xCB, 0xC7},
        {"set",  "n,b",        0xCB, 0xC0},
        {"set",  "n,c",        0xCB, 0xC1},
        {"set",  "n,d",        0xCB, 0xC2},
        {"set",  "n,e",        0xCB, 0xC3},
        {"set",  "n,h",        0xCB, 0xC4},
        {"set",  "n,l",        0xCB, 0xC5},
        {"set",  "n,(hl)",     0xCB, 0xC6},
        {"set",  "n,[hl]",     0xCB, 0xC6},

        {"sla",  "a",          0xCB, 0x27},
        {"sla",  "b",          0xCB, 0x20},
        {"sla",  "c",          0xCB, 0x21},
        {"sla",  "d",          0xCB, 0x22},
        {"sla",  "e",          0xCB, 0x23},
        {"sla",  "h",          0xCB, 0x24},
        {"sla",  "l",          0xCB, 0x25},
        {"sla",  "(hl)",       0xCB, 0x26},
        {"sla",  "[hl]",       0xCB, 0x26},

        {"sra",  "a",          0xCB, 0x2F},
        {"sra",  "b",          0xCB, 0x28},
        {"sra",  "c",          0xCB, 0x29},
        {"sra",  "d",          0xCB, 0x2A},
        {"sra",  "e",          0xCB, 0x2B},
        {"sra",  "h",          0xCB, 0x2C},
        {"sra",  "l",          0xCB, 0x2D},
        {"sra",  "(hl)",       0xCB, 0x2E},
        {"sra",  "[hl]",       0xCB, 0x2E},

        {"srl",  "a",          0xCB, 0x3F},
        {"srl",  "b",          0xCB, 0x38},
        {"srl",  "c",          0xCB, 0x39},
        {"srl",  "d",          0xCB, 0x3A},
        {"srl",  "e",          0xCB, 0x3B},
        {"srl",  "h",          0xCB, 0x3C},
        {"srl",  "l",          0xCB, 0x3D},
        {"srl",  "(hl)",       0xCB, 0x3E},
        {"srl",  "[hl]",       0xCB, 0x3E},

        {"stop", "-",          0x10, 00},

        {"sub",  "a",          0x97},
        {"sub",  "b",          0x90},
        {"sub",  "c",          0x91},
        {"sub",  "d",          0x92},
        {"sub",  "e",          0x93},
        {"sub",  "h",          0x94},
        {"sub",  "l",          0x95},
        {"sub",  "(hl)",       0x96},
        {"sub",  "[hl]",       0x96},
        {"sub",  "*",          0xD6},

        {"swap", "a",          0xCB, 0x37},
        {"swap", "b",          0xCB, 0x30},
        {"swap", "c",          0xCB, 0x31},
        {"swap", "d",          0xCB, 0x32},
        {"swap", "e",          0xCB, 0x33},
        {"swap", "h",          0xCB, 0x34},
        {"swap", "l",          0xCB, 0x35},
        {"swap", "(hl)",       0xCB, 0x36},
        {"swap", "[hl]",       0xCB, 0x36},

        {"xor",  "a",          0xAF},
        {"xor",  "b",          0xA8},
        {"xor",  "c",          0xA9},
        {"xor",  "d",          0xAA},
        {"xor",  "e",          0xAB},
        {"xor",  "h",          0xAC},
        {"xor",  "l",          0xAD},
        {"xor",  "(hl)",       0xAE},
        {"xor",  "[hl]",       0xAE},
        {"xor",  "*",          0xEE},

        {"ld",   "hl,sp+*",    0xF8},
        {"ldhl", "sp,*",       0xF8},
};