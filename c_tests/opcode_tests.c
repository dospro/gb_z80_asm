#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "../src/opcode.h"
#include "opcode_tests.h"

/**
 * Test split_line function returns an OpcodesParts
 * struct with the name of the opcode set correctly
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_name(void** state)
{
    const String opcode_text = string_from_cstr("ld a,b");
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
    assert_true(string_is_equal_cstr(opcode_parts.name, "ld"));
    assert_int_equal(opcode_parts.name.size, 2);
}

/**
 * Test split_line function returns an OpcodesParts
 * struct with the name and the first argument
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_arg1(void** state)
{
    const String opcode_text = string_from_cstr("add a,b");
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
    assert_true(string_is_equal_cstr(opcode_parts.name, "add"));
    assert_int_equal(opcode_parts.name.size, 3);
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_int_equal(opcode_parts.arg1.size, 1);
}

/**
 * Test split_line function returns an OpcodesParts
 * struct with the second argument set correctly
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_arg2(void** state)
{
    const String opcode_text = string_from_cstr("sbc a,h");
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
    assert_true(string_is_equal_cstr(opcode_parts.name, "sbc"));
    assert_int_equal(opcode_parts.name.size, 3);
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_int_equal(opcode_parts.arg1.size, 1);
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "h"));
    assert_int_equal(opcode_parts.arg2.size, 1);
}

/**
 * Test the split_line function with an opcode that has only
 * one argument. arg2 should be an empty string.
 *
 * @param state Used by CMocka
 */
void test_split_line_gets_single_arg(void** state)
{
    const String opcode_text = string_from_cstr("or l");
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
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
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
    assert_true(string_is_equal_cstr(opcode_parts.name, "ld"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "[ff00+c]"));
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "a"));
}

/**
 * Test split line with spaces between operands
 *
 * @param state Used by CMocka
 */
void test_split_line_spaces_between_operands(void** state)
{
    const String opcode_text = string_from_cstr("ld a, b");
    ErrorCode error_code = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error_code);
    assert_int_equal(error_code, GBASM_CODE_OK);
    assert_true(string_is_equal_cstr(opcode_parts.name, "ld"));
    assert_true(string_is_equal_cstr(opcode_parts.arg1, "a"));
    assert_true(string_is_equal_cstr(opcode_parts.arg2, "b"));
}


/**
 * Test split line with malformed opcode
 *
 * In this case we use an invalid number of parameters
 * ld a, b, c
 *
 *
 * @param state Used by CMocka
 */
void test_split_line_invalid_parameters(void** state)
{
    const String opcode_text = string_from_cstr("ld a, b, c");
    ErrorCode error = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error);
    assert_int_equal(error, GBASM_ERR_MALFORMED_LINE);
}

/**
 * Test split line with a comma in a bad place
 *
 * In this case we use an invalid number of parameters
 * ld a, b, c
 *
 * @param state Used by CMocka
 */
void test_split_line_bad_commas(void** state)
{
    const String opcode_text = string_from_cstr("ld, a,b");
    ErrorCode error = {};
    const OpcodeTextParts opcode_parts = split_line_new(opcode_text, &error);
    assert_int_equal(error, GBASM_ERR_MALFORMED_LINE);

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
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("adc a,a"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

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
void test_parse_opcode_no_operands(void** state)
{
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("ccf"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0x3F);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode parses an opcode with a single operand
 *
 * In this test we use "cp [hl]" opcode which only has one operand "[hl]"
 * and should produce a MachineOpcode with the machine code number
 * of 0xBE with no parameters.
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_single_operand(void** state)
{
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("cp [hl]"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0xBE);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode parses an opcode with a single variable operand
 *
 * In this test we use "or 0x5" which should produce a MachineOpcode number
 * of 0xF6, also set the param size to 1, and save the 0x5 parameter in the list
 * of parameters
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_single_variable_operand(void** state)
{
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("or 0x5"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0xF6);
    assert_int_equal(result.params_size, 1);
    assert_int_equal(result.params[0], 0x5);
}

/**
 * Test parse_opcode parses an opcode with two static operands
 *
 * In this test we use "ld a,(bc)" which should produce a MachineOpcode number
 * of 0x0A, also set the param size to 0
 *
 * @param state Used by CMocka
 */
void test_parse_opcode_two_operands(void** state)
{
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("ld a, (bc)"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0x0A);
    assert_int_equal(result.params_size, 0);
}

/**
 * Test parse_opcode function correctly parses an opcode with 2 operands where
 * the first is static and the second is variable,
 * and returns the expected machine code with the correct opcode byte and parameter
 *
 * @param state Used by CMocka test framework
 */
void test_parse_opcode_second_param_variable(void** state)
{
    ErrorCode error;
    const OpcodeTextParts opcode_parts = split_line_new(string_from_cstr("sbc a,0xF"), &error);
    assert_int_equal(error, GBASM_CODE_OK);

    const MachineCode result = parse_opcode(opcode_parts, &error);
    assert_int_equal(error, GBASM_CODE_OK);
    assert_int_equal(result.opcode, 0xDE);
    assert_int_equal(result.params_size, 1);
    assert_int_equal(result.params[0], 0xF);
}

/*
 * Next cases
 * bit: "bit 6,a" -> Has special cases and uses CB Opcode (res, set)
 * call nz,address -> Address is 16bits
 * jp and jr
 * The 4 cases of the same opcode:
 *     ld (ff00+c),a
 *     ld [ff00+c],a
 *     ld (c),a
 *     ld [c],a
 * Same for ld (hli),a
 * Make sure () and [] are equivalent
 * Parse "ld a,(ff00+*) and variations
 * ld bc, 0xFFFF
 * rst
 */

void test_search_opcode(void** state)
{
    const String opcode_text = string_from_cstr("add a,b");
    const int result = search_opcode("adc", "a,b");
    assert_int_equal(result, 0x88);
}
