cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 23)
include(CTest)

enable_testing()

include_directories(/opt/homebrew/include/)
link_directories(/opt/homebrew/lib/)
# create an executable, which instantiates a runner from
# Cmocka
add_executable(c_tests
        src/string.c
        src/opcode.c
        src/parameter.c
        src/symbol.c
        src/tokenizer.c
        c_tests/main.c
        c_tests/string_tests.c
        c_tests/string_buffer_tests.c
        c_tests/string_iterator_tests.c
        c_tests/opcode_tests.c
        c_tests/symbol_tests.c
        c_tests/tokenizer_tests.c
        c_tests/fixups_tests.c)
target_link_libraries(c_tests cmocka)
add_test(NAME c_tests COMMAND c_tests)
