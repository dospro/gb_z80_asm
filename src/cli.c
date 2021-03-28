#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"

struct Config parse_arguments(char **argv, int argc) {
    enum ParserStates {
        INITIAL,
        READ_OUTPUT,
        MBC_TYPE,
        ROM_SIZE,
        RAM_SIZE,
        ERROR,
    } state = INITIAL;

    struct Config config;
    config.output_filename = NULL;
    config.input_filenames = create_list();

    for (size_t i = 1; i < argc; ++i) {
        switch (state) {
            case INITIAL:
                if (argv[i][0] == '-') {
                    switch (argv[i][1]) {
                        case 'o':
                            state = READ_OUTPUT;
                            break;
                        case 'm':
                            state = MBC_TYPE;
                            break;
                        case 'g':
                            state = ROM_SIZE;
                            break;
                        case 'h':
                            state = RAM_SIZE;
                            break;
                        default:
                            state = ERROR;
                            break;
                    }
                } else {
                    printf("This may be an input file. %s\n", argv[i]);
                    char *new_argument = (char *) malloc(sizeof(char) * strlen(argv[i]) + 1);
                    strcpy(new_argument, argv[i]);
                    list_push(&config.input_filenames, new_argument);
                }
                break;
            case READ_OUTPUT:
                config.output_filename = (char *) malloc(sizeof(char) * strlen(argv[i]) + 1);
                strcpy(config.output_filename, argv[i]);
                printf("Got output file %s\n", argv[i]);
                state = INITIAL;
                break;
            case MBC_TYPE:
                if (strcmp(argv[i], "rom_only") == 0) {
                    config.gb_header.cartridge_type = ROM_ONLY;
                } else if (strcmp(argv[i], "mbc1") == 0) {
                    config.gb_header.cartridge_type = MBC1;
                } else if (strcmp(argv[i], "mbc3") == 0) {
                    config.gb_header.cartridge_type = MBC3;
                }
                state = INITIAL;
                break;
            case ROM_SIZE:
                config.gb_header.rom_banks = (int) strtoul(argv[i], NULL, 10);
                state = INITIAL;
                break;
            default:
                printf("Unimplemented %s\n", argv[i]);
                break;
        }
    }
    return config;
}