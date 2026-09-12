#include "tokenizer.h"

constexpr char COMMA = ',';
constexpr char QUOTE = '"';

/**
 * Reports whether a byte separates tokens.
 *
 * Deliberately not isspace(): that function is locale-dependent, so the set of
 * separators would change if anything in the process called setlocale(), and
 * it also accepts \v and \f, which have no place in assembly source. It also
 * takes an int and has undefined behavior for values outside unsigned char,
 * forcing a cast at every call site. Enumerating the four bytes a source file
 * can actually contain keeps the lexer's behavior fixed and the call sites
 * plain.
 *
 * @param c Byte to classify.
 * @return true if c is a space, tab, carriage return or line feed.
 */
static bool is_blank(const char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool at_end(const Tokenizer* const tokenizer)
{
    return tokenizer->position >= tokenizer->line.size;
}

static char peek(const Tokenizer* const tokenizer)
{
    return tokenizer->line.data[tokenizer->position];
}

// Precondition: peek(tokenizer) == QUOTE.
static Token read_string(Tokenizer* const tokenizer)
{
    const size_t opening_quote = tokenizer->position;
    tokenizer->position++;
    const size_t start = tokenizer->position;

    while (!at_end(tokenizer) && peek(tokenizer) != QUOTE)
    {
        tokenizer->position++;
    }
    if (at_end(tokenizer))
    {
        return (Token){
            .kind = TOKEN_ERROR,
            .text = {
                .data = &tokenizer->line.data[opening_quote],
                .size = tokenizer->position - opening_quote,
            },
        };
    }

    const size_t end = tokenizer->position;
    tokenizer->position++;
    return (Token){
        .kind = TOKEN_STRING,
        .text = {
            .data = &tokenizer->line.data[start],
            .size = end - start,
        },
    };
}

static Token read_word(Tokenizer* const tokenizer)
{
    const size_t start = tokenizer->position;
    while (!at_end(tokenizer) && !is_blank(peek(tokenizer)) && peek(tokenizer) != COMMA)
    {
        tokenizer->position++;
    }
    return (Token){
        .kind = TOKEN_WORD,
        .text = {
            .data = &tokenizer->line.data[start],
            .size = tokenizer->position - start,
        },
    };
}

Tokenizer tokenizer_new(const String line)
{
    return (Tokenizer){
        .line = line,
        .position = 0,
    };
}

Token tokenizer_next(Tokenizer* const tokenizer)
{
    while (!at_end(tokenizer) && is_blank(peek(tokenizer)))
    {
        tokenizer->position++;
    }
    if (at_end(tokenizer))
    {
        return (Token){.kind = TOKEN_END};
    }

    switch (peek(tokenizer))
    {
    case COMMA:
        {
            const size_t start = tokenizer->position;
            tokenizer->position++;
            return (Token){
                .kind = TOKEN_COMMA,
                .text = {
                    .data = &tokenizer->line.data[start],
                    .size = 1,
                },
            };
        }
    case QUOTE:
        return read_string(tokenizer);
    default:
        return read_word(tokenizer);
    }
}
