#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "../src/tokenizer.h"
#include "tokenizer_tests.h"

/**
 * Tests that the tokenizer reports a comma as a token of its own rather than
 * consuming it as a delimiter.
 *
 * This is the decision the rest of the parser layer rests on. Were commas mere
 * delimiters, the well-formed "ld a,b" and the malformed "ld a b" would produce
 * identical token sequences and no consumer could tell them apart. Emitting ","
 * keeps operand separation visible to the parser, and is also what lets a
 * variable-length directive such as .db find its item boundaries.
 *
 * The input omits spaces around the comma deliberately: the tokenizer has to
 * split it away from the operands on either side, not merely recognize it when
 * whitespace has already done the separating. The closing assertion pins the
 * end-of-input contract, because every loop over a tokenizer depends on an
 * end token to terminate.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_opcode(void** state)
{
    const String line = string_from_cstr("ld a,b");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "ld"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "a"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_COMMA);
    assert_true(string_is_equal_cstr(token.text, ","));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "b"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that whitespace is only a separator and never part of a token.
 *
 * Leading indentation, runs of mixed spaces and tabs between tokens, and a
 * CRLF line ending are all common in real source files. None of them should
 * leak into token text or produce spurious empty tokens.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_skips_whitespace(void** state)
{
    const String line = string_from_cstr("  \tld \t a ,\t b\r\n");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "ld"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "a"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_COMMA);
    assert_true(string_is_equal_cstr(token.text, ","));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "b"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that consecutive commas are reported individually.
 *
 * The tokenizer must not collapse or swallow ",,": whether that is an error
 * is a decision for the parser, which can only make it if every comma is
 * visible in the stream.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_consecutive_commas(void** state)
{
    const String line = string_from_cstr("a,,b");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "a"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_COMMA);
    assert_true(string_is_equal_cstr(token.text, ","));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_COMMA);
    assert_true(string_is_equal_cstr(token.text, ","));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "b"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that an empty line yields end immediately.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_empty_line(void** state)
{
    const String line = string_from_cstr("");
    Tokenizer tokenizer = tokenizer_new(line);

    const Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that a whitespace-only line yields end immediately, with no empty
 * word token in between.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_blank_line(void** state)
{
    const String line = string_from_cstr("   \t  ");
    Tokenizer tokenizer = tokenizer_new(line);

    const Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that the end token is sticky.
 *
 * Callers commonly loop with a lookahead and may call next once or twice
 * past the last real token. Doing so must keep returning end rather than
 * reading past the line or re-emitting a token.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_end_is_sticky(void** state)
{
    const String line = string_from_cstr("nop");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "nop"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that a quoted string is a single token whose text excludes the quotes.
 *
 * Spaces and commas inside the quotes must not split the token; this is the
 * one place the tokenizer has to look past its usual separators. The string
 * is surrounded by ordinary tokens to check that scanning resumes correctly
 * after the closing quote.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_string_literal(void** state)
{
    const String line = string_from_cstr(".db \"a, b\",1");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, ".db"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_STRING);
    assert_true(string_is_equal_cstr(token.text, "a, b"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_COMMA);
    assert_true(string_is_equal_cstr(token.text, ","));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, "1"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that an empty string literal is a string token with no text.
 *
 * This is the case that makes a distinct end kind necessary: with a bare
 * String return, an empty literal would be indistinguishable from end of
 * input.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_empty_string_literal(void** state)
{
    const String line = string_from_cstr("\"\"");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_STRING);
    assert_true(string_is_empty(token.text));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}

/**
 * Tests that an unterminated string literal is reported as an error token.
 *
 * The error text spans from the opening quote to the end of the line so the
 * reporter can point at where the string started. After the error the
 * tokenizer is at end of input.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_tokenizer_unterminated_string(void** state)
{
    const String line = string_from_cstr(".db \"abc");
    Tokenizer tokenizer = tokenizer_new(line);

    Token token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_WORD);
    assert_true(string_is_equal_cstr(token.text, ".db"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_ERROR);
    assert_true(string_is_equal_cstr(token.text, "\"abc"));

    token = tokenizer_next(&tokenizer);
    assert_int_equal(token.kind, TOKEN_END);
    assert_true(string_is_empty(token.text));
}