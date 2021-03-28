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

struct Config parse_arguments(char **argv, int argc);

#endif //GB_Z80_ASM_CLI_H
