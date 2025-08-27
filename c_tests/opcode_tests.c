#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "../src/opcode.h"
#include "opcode_tests.h"

/**
 * Test split_line function returns a OpcodesParts
 * struct with the name of the opcode set correctly
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_name(void** state)
{
    const String opcode_text = string_from_cstr("ld a,b");
    const OpcodeParts opcode_parts = split_line_new(opcode_text);
    assert_true(string_is_equal_cstr(opcode_parts.name, "ld"));
}

/**
 * Test split_line function returns a OpcodesParts
 * struct with the name and the first argument
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_arg1(void** state)
{
    const String opcode_text = string_from_cstr("add a,b");
    const OpcodeParts opcode_parts = split_line_new(opcode_text);
    assert_true(string_is_equal_cstr(opcode_parts.name, "add"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
}

/**
 * Test split_line function returns a OpcodesParts
 * struct with the second argument set correctly
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_arg2(void** state)
{
    const String opcode_text = string_from_cstr("sbc a,h");
    const OpcodeParts opcode_parts = split_line_new(opcode_text);
    assert_true(string_is_equal_cstr(opcode_parts.name, "sbc"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "h"));
}

/**
 * Test split_line function with an opcode that has only
 * one argument. arg2 should be an empty string.
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_single_arg(void** state)
{
    const String opcode_text = string_from_cstr("or l");
    const OpcodeParts opcode_parts = split_line_new(opcode_text);
    assert_true(string_is_equal_cstr(opcode_parts.name, "or"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "l"));
    assert_true(string_is_empty(opcode_parts.arg2));
}

/**
 * Test split_line function with a full complex opcode
 * is split correctly
 *
 * @param state Used by CMocka
 */
void test_split_line_complex_opcode(void** state)
{
    const String opcode_text = string_from_cstr("ld [ff00+c],a");
    const OpcodeParts opcode_parts = split_line_new(opcode_text);
    assert_true(string_is_equal_cstr(opcode_parts.name, "ld"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "[ff00+c]"));
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "a"));
}