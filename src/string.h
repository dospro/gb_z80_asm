#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    void* data;
    size_t size;
} Vector;

typedef struct
{
    void* data;
    size_t next_free_space;
    size_t size;
} MemoryBuffer;

void* allocate(MemoryBuffer* buffer, size_t size);

typedef void*(*allocator_callback)(size_t size);

typedef struct
{
    char* data;
    size_t size;
} String;

typedef struct
{
    String source;
    size_t position;
} StringIterator;

typedef struct
{
    char* data;
    size_t size;
    size_t capacity;
    allocator_callback allocator;
} StringBuffer;

StringBuffer StringBuffer_new(const size_t capacity, const allocator_callback allocator);
void StringBuffer_free(StringBuffer* string_buffer);
StringBuffer StringBuffer_from_cstr(const char* const source, const allocator_callback allocator);
bool StringBuffer_is_ok(const StringBuffer *buffer);
StringBuffer StringBuffer_from_file(FILE *__file);
StringIterator StringBuffer_create_iterator(const StringBuffer *buffer);
bool StringBuffer_append_cstr(StringBuffer *buffer, const char* const source);
bool StringBuffer_append_string(StringBuffer* buffer, const String string);
// String StringBuffer_to_string(const StringBuffer self);

// StringIterator StringIterator_new(const String str);
// String StringIterator_next(StringIterator* self);
bool StringIterator_is_ok(const StringIterator *iterator);
String StringIterator_next_line(StringIterator *iterator);
String StringIterator_next_raw_line(StringIterator *iterator);

String string_from_cstr(char* const source);
String string_trim(const String str);
bool string_is_empty(const String str);
bool string_is_equal(const String str1, const String str2);
bool string_is_equal_cstr(const String str1, const char *str2);
char string_at(const String str, const size_t index);
int string_index(const char character);
Vector string_split(const String str, const String pattern);
bool string_contains_char(const String str, const char character);
bool string_contains(const String str, const String pattern);
String string_from_string_buffer(const StringBuffer buffer);

#endif //STRING_H
