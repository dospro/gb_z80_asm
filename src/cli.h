#ifndef GB_Z80_ASM_CLI_H
#define GB_Z80_ASM_CLI_H

#include <stdbool.h>

#include "list.h"

enum MBCType {
    ROM_ONLY,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATT,
    MBC2,
    MBC2_BATT,
    MBC3,
    MBC5,
    CUSTOM
};

struct GBHeader {
    char *title;
    bool is_color;
    bool has_sgb;
    enum MBCType cartridge_type;
    int rom_banks;
    int ram_banks;
};

struct Config {
    struct List input_filenames;
    char *output_filename;
    struct GBHeader gb_header;
};

enum CommandType {
    MODIFIER,
    MODIFIER_VALUE,
    SHORT_MODIFIER,
    TEXT_INPUT
};

struct ModifierSchema {
    char name[32]; // Name of the command so we can get it's value by name
    enum CommandType type; // What type of command we are expecting
    char short_modifier; // Form of the short version of the modifier
    char long_modifier[32]; // Form of the long version of the modifier.
    char description[200];
};

struct CLISchema {
    struct List modifiers_schemas;
    char name[32];
    char description[200];
};

struct Modifier {
    char short_modifier;
    char long_modifier[32];
    char description[200];
    char value[32];
};

struct CLI {
    struct Modifier *modifiers;
    char *positional_values[];
};

void add_command(
        struct CLISchema *cli_schema,
        const char *name,
        enum CommandType type,
        const char short_modifier,
        const char *long_modifier,
        const char *description
);
void print_help_text(struct CLISchema *schema);

//struct Config parse_arguments(char **argv, int argc);

#endif //GB_Z80_ASM_CLI_H
