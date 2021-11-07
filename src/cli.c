#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"

/*
 * Types of command line options
 *
 * Modifier
 *
 * This works like a flag, to indicate the application to do something
 * different from the default behavior.
 * Examples:
 * -a, --config, -q
 *
 * Modifier + Value
 *
 * This is for specifying specific values to the application
 * Examples:
 * -o file, --env staging, -g 5
 *
 * Modifier and simple value
 *
 * This special case only applies to short modifiers (eg. -x)
 * The main difference is that it doesn't require an extra space
 * Useful for numbers
 * Examples:
 *
 * -g1, -a9, -c4
 * Note: As for now, it only works for numbers
 *
 * Positional value
 *
 * Any text value that comes after the command name and before any modifier.
 * These are normally used for specifying inputs
 * Examples:
 *
 * command input1 input2 -f --key value
 *
 * Anything else that doesn't follow this rules is marked as an error.
 */


struct List *create_cli_parser(const char *name, const char *description);

void add_command(
        struct CLISchema *cli_schema,
        const char *name,
        enum CommandType type,
        const char short_modifier,
        const char *long_modifier,
        const char *description
) {
    struct ModifierSchema *schema = (struct ModifierSchema *) malloc(sizeof(struct ModifierSchema));
    strcpy(schema->name, name);
    schema->type = type;
    schema->short_modifier = short_modifier;
    strcpy(schema->long_modifier, long_modifier);
    strcpy(schema->description, description);
    list_push(&cli_schema->modifiers_schemas, (void *)schema);
}

void print_modifier(struct Node *node, void *context) {
    struct ModifierSchema *schema = (struct ModifierSchema *)node->data;
    printf("-%c\t--%s\t\t%s\n", schema->short_modifier, schema->long_modifier, schema->description);
}

void print_help_text(struct CLISchema *schema) {
    printf("%s\n\n", schema->name);
    printf("%s\n\n", schema->description);
    foreach(&schema->modifiers_schemas, &print_modifier, NULL);
}

//void parse_parameters()


//struct Config parse_arguments(struct CLI *cli, char **argv, int argc) {
//    enum ParserStates {
//        INITIAL,
//        READ_OUTPUT,
//        MBC_TYPE,
//        ROM_SIZE,
//        RAM_SIZE,
//        ERROR,
//    } state = INITIAL;
//
//    struct Config config;
//    config.output_filename = NULL;
//    config.input_filenames = create_list();
//
//    for (size_t i = 1; i < argc; ++i) {
//        switch (state) {
//            case INITIAL:
//                if (argv[i][0] == '-') {
//                    switch (argv[i][1]) {
//                        case 'o':
//                            state = READ_OUTPUT;
//                            break;
//                        case 'm':
//                            state = MBC_TYPE;
//                            break;
//                        case 'g':
//                            state = ROM_SIZE;
//                            break;
//                        case 'h':
//                            state = RAM_SIZE;
//                            break;
//                        default:
//                            state = ERROR;
//                            break;
//                    }
//                } else {
//                    printf("This may be an input file. %s\n", argv[i]);
//                    char *new_argument = (char *) malloc(sizeof(char) * strlen(argv[i]) + 1);
//                    strcpy(new_argument, argv[i]);
//                    list_push(&config.input_filenames, new_argument);
//                }
//                break;
//            case READ_OUTPUT:
//                config.output_filename = (char *) malloc(sizeof(char) * strlen(argv[i]) + 1);
//                strcpy(config.output_filename, argv[i]);
//                printf("Got output file %s\n", argv[i]);
//                state = INITIAL;
//                break;
//            case MBC_TYPE:
//                if (strcmp(argv[i], "rom_only") == 0) {
//                    config.gb_header.cartridge_type = ROM_ONLY;
//                } else if (strcmp(argv[i], "mbc1") == 0) {
//                    config.gb_header.cartridge_type = MBC1;
//                } else if (strcmp(argv[i], "mbc3") == 0) {
//                    config.gb_header.cartridge_type = MBC3;
//                }
//                state = INITIAL;
//                break;
//            case ROM_SIZE:
//                config.gb_header.rom_banks = (int) strtoul(argv[i], NULL, 10);
//                state = INITIAL;
//                break;
//            default:
//                printf("Unimplemented %s\n", argv[i]);
//                break;
//        }
//    }
//    return config;
//}