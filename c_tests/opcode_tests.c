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
    assert_int_equal(opcode_parts.name.size, 2);
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
    assert_int_equal(opcode_parts.name.size, 3);
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_int_equal(opcode_parts.arg1.size, 1);
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
    assert_int_equal(opcode_parts.name.size, 3);
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_int_equal(opcode_parts.arg1.size, 1);
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "h"));
    assert_int_equal(opcode_parts.arg2.size, 1);
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

/**
 * Test parse_opcode parses a simple opcode
 *
 * In this test we use "adc a,a" one of the simplest opcodes
 * and should produce a MachineOpcode with the machine code number
 * of 0x8F with no parameters.
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_simple(void** state)
{
    const OpcodeParts opcode_parts = split_line_new(string_from_cstr("adc a,a"));
    ErrorCode error;
    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0x8F);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode parses an opcode with no parameters
 *
 * In this test we use "ccf" opcode which has no parameter
 * and should produce a MachineOpcode with the machine code number
 * of 0x3F with no parameters.
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_no_params(void** state)
{
    const OpcodeParts opcode_parts = split_line_new(string_from_cstr("ccf"));
    ErrorCode error;
    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0x3F);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode parses an opcode with single parameter
 *
 * In this test we use "cp [hl]" opcode which only has one parameter
 * and should produce a MachineOpcode with the machine code number
 * of 0xBE with no parameters.
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_single_param(void** state)
{
    const OpcodeParts opcode_parts = split_line_new(string_from_cstr("cp [hl]"));
    ErrorCode error;
    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0xBE);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode parses an opcode with single variable parameter
 *
 * In this test we use "or 0x5" which should produce a MachineOpcode number
 * of 0xF6, also set the param size to 1 and save the 0x5 parameter in the list
 * of parameters
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_single_variable(void** state)
{
    const OpcodeParts opcode_parts = split_line_new(string_from_cstr("or 0x5"));
    ErrorCode error;
    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0xF6);
    assert_int_equal(result.params_size, 1);
    assert_int_equal(result.params[0], 0x5);
}

void test_search_opcode(void** state)
{
    const String opcode_text = string_from_cstr("add a,b");
    const int result = search_opcode("adc", "a,b");
    assert_int_equal(result, 0x88);
}
