#ifndef GB_Z80_ASM_OPCODE_H
#define GB_Z80_ASM_OPCODE_H
#include "string.h"
#include "error_handler.h"

/**
 * @brief Represents the textual components of an assembly opcode line.
 *
 * Holds the mnemonic name and up to two arguments extracted from a single
 * line of assembly source code (e.g., "ld a,$FF" -> name="ld", arg1="a", arg2="$FF").
 * Empty strings indicate unused arguments.
 */
typedef struct OpcodeTextParts OpcodeTextParts;
struct OpcodeTextParts
{
    String name; /**< @brief The opcode mnemonic (e.g., "ld", "jp", "nop"). */
    String arg1; /**< @brief The first argument, or empty if none. */
    String arg2; /**< @brief The second argument, or empty if only one argument. */
};

typedef struct Opcode Opcode;
struct Opcode
{
    char name[6];
    char arg_string[12];
    int value, high_value;
};

typedef struct MachineCode MachineCode;
struct MachineCode
{
    int opcode;
    int params_size;
    int params[3];
};


MachineCode parse_opcode(OpcodeTextParts opcode_parts, ErrorCode *error_code);

bool get_general_opcode(struct MachineCode* machine_code_out, char* opcode_name, char* arg1, char* arg2);

int search_opcode(char* opcode_name, char* arguments);

OpcodeTextParts split_line_new(String line, ErrorCode *error_code);

// #define split_line(line) split_line_new(line)


#endif //GB_Z80_ASM_OPCODE_H
