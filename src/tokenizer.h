#ifndef GB_Z80_ASM_TOKENIZER_H
#define GB_Z80_ASM_TOKENIZER_H
#include <stddef.h>
#include "string.h"

typedef enum TokenKind
{
    TOKEN_END,    // No more tokens. text is empty. Sticky: further calls keep returning it.
    TOKEN_WORD,   // Run of bytes delimited by whitespace, comma or end of line.
    TOKEN_COMMA,  // A single ','.
    TOKEN_STRING, // Quoted literal. text is the contents without the quotes.
    TOKEN_ERROR,  // Lexical error (unterminated string). text spans from the opening quote to end of line.
} TokenKind;

typedef struct Token
{
    TokenKind kind;
    String text; // View into the tokenized line; valid as long as the line is.
} Token;

typedef struct Tokenizer
{
    String line;
    size_t position;
} Tokenizer;

/**
 * Creates a tokenizer positioned at the start of a line.
 *
 * The tokenizer does not copy or own the line. Every Token it produces is a
 * view into it, so the line's storage must outlive both the tokenizer and any
 * Token still in use. line.data may be null only when line.size is 0.
 *
 * @param line Source line to split. Treated as a byte span; no NUL terminator
 *             is required or looked for.
 * @return A tokenizer whose first call to tokenizer_next() yields the first
 *         token of the line.
 */
Tokenizer tokenizer_new(String line);

/**
 * Returns the next token and advances past it.
 *
 * Tokens are separated by spaces, tabs, carriage returns and line feeds, which
 * are never part of a token. A comma is always its own token, even with no
 * surrounding whitespace, so that the parser can tell "ld a,b" from "ld a b".
 * A double quote starts a string literal that runs to the next double quote;
 * whitespace and commas inside it do not split it, and the returned text
 * excludes the quotes. There is no escape handling. Everything else is a word.
 *
 * Once the line is exhausted, TOKEN_END is returned with empty text, and every
 * later call returns TOKEN_END again. The only lexical error is an
 * unterminated string, reported as TOKEN_ERROR with text from the opening
 * quote to the end of the line; the tokenizer is then at end of input.
 *
 * Marked nodiscard because dropping the result silently skips a token.
 *
 * @param tokenizer Tokenizer to read from. Must not be null.
 * @return The next token. Its text is a view into the tokenized line.
 */
[[nodiscard]] Token tokenizer_next(Tokenizer* tokenizer);

#endif //GB_Z80_ASM_TOKENIZER_H
