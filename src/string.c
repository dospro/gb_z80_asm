#include "string.h"
#include <string.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

StringBuffer StringBuffer_new(const size_t capacity, const allocator_callback allocator)
{
    StringBuffer string;
    string.size = 0;
    string.capacity = capacity;
    string.data = allocator(string.capacity * sizeof(char));
    string.allocator = allocator;
    return string;
}

void StringBuffer_free(StringBuffer* string_buffer)
{
    if (string_buffer->data != nullptr)
    {
        free(string_buffer->data);
        string_buffer->data = nullptr;
        string_buffer->size = 0;
        string_buffer->capacity = 0;
        string_buffer->allocator = nullptr;
    }
}

StringBuffer StringBuffer_from_cstr(const char* const source, const allocator_callback allocator)
{
    const size_t source_size = strlen(source);
    char* copy = allocator(source_size);
    for (size_t i = 0; i < source_size; i++)
    {
        copy[i] = source[i];
    }
    return (StringBuffer){
        .size = source_size,
        .capacity = source_size,
        .data = copy,
        .allocator = allocator
    };
}

bool StringBuffer_is_ok(const StringBuffer* buffer)
{
    if (buffer->data == nullptr)
    {
        return false;
    }
    return true;
}

StringBuffer StringBuffer_from_file(FILE* __file)
{
    if (__file == nullptr)
    {
        return (StringBuffer){.size = 0, .capacity = 0, .allocator = nullptr, .data = nullptr};
    }
    fseek(__file, 0, SEEK_END);
    const size_t file_size = ftell(__file);
    fseek(__file, 0, SEEK_SET);

    StringBuffer buffer = StringBuffer_new(file_size, malloc);
    if (!StringBuffer_is_ok(&buffer))
    {
        return (StringBuffer){.size = 0, .capacity = 0, .allocator = nullptr, .data = nullptr};
    }
    fread(buffer.data, file_size, 1, __file);
    buffer.size = file_size;
    return buffer;
}

StringIterator StringBuffer_create_iterator(const StringBuffer* buffer)
{
    const String string = {.data = buffer->data, .size = buffer->size};
    return (StringIterator){
        .source = string,
        .position = 0,
    };
}

bool StringBuffer_append_cstr(StringBuffer* buffer, const char* const source)
{
    size_t source_size = 0;
    while (source[source_size] != '\0')
    {
        source_size++;
    }
    const String string = {.data = source, .size = source_size};
    return StringBuffer_append_string(buffer, string);
}

bool StringBuffer_append_string(StringBuffer* buffer, const String string)
{
    if (buffer->size + string.size > buffer->capacity)
    {
        const size_t new_capacity = buffer->capacity + string.size;
        buffer->data = realloc(buffer->data, new_capacity * sizeof(char));
        if (buffer->data == nullptr)
        {
            return false;
        }
        buffer->capacity = new_capacity;
    }
    for (size_t i = 0; i < string.size; i++)
    {
        buffer->data[buffer->size++] = string.data[i];
    }
    return true;
}

/* Starts String Iterator */

String StringIterator_next_line(StringIterator* iterator)
{
    while (iterator->position < iterator->source.size && iterator->source.data[iterator->position] == '\n')
    {
        ++iterator->position;
    }
    String string = {.data = &iterator->source.data[iterator->position], .size = 0};

    while (iterator->position < iterator->source.size)
    {
        if (iterator->source.data[iterator->position] == '\n')
        {
            ++iterator->position;
            break;
        }
        ++iterator->position;
        ++string.size;
    }
    return string;
}

String StringIterator_next_raw_line(StringIterator* iterator)
{
    String string = {.data = &iterator->source.data[iterator->position], .size = 0};

    while (iterator->position < iterator->source.size)
    {
        if (iterator->source.data[iterator->position] == '\n')
        {
            ++string.size;
            ++iterator->position;
            break;
        }
        ++iterator->position;
        ++string.size;
    }
    return string;
}

StringIterator StringIterator_new(const String str)
{
    return (StringIterator){.source = str, .position = 0};
}

bool StringIterator_is_ok(const StringIterator* iterator)
{
    if (iterator->source.data == nullptr)
    {
        return false;
    }
    return true;
}

// String StringIterator_next(StringIterator* self)
// {
//     const String delimiters = {.data = " ,\n\t", .size = 4}; // These are used with strtok to separate the arguments
//     if (self->source.data[self->position] == ';')
//     {
//         return (String){};
//     }
//     while (self->position < self->source.size)
//     {
//         if (is_delimiter(self->source.data[self->position], delimiters))
//         {
//             self->position++;
//         }
//         else
//         {
//             break;
//         }
//     }
//
//     size_t token_length = 0;
//     while (self->position + token_length < self->source.size)
//     {
//         if (is_delimiter(self->source.data[self->position + token_length], delimiters))
//         {
//             const String result = {.data = &self->source.data[self->position], .size = token_length};
//             string_print(result);
//             self->position += token_length;
//             return result;
//         }
//         token_length++;
//     }
//     if (self->position < self->source.size)
//     {
//         const String result = {.data = &self->source.data[self->position], .size = token_length};
//         self->position += token_length;
//         return result;
//     }
//     else
//     {
//         return (String){};
//     }
// }

String string_from_cstr(char* const source)
{
    const size_t source_size = strlen(source);
    return (String){.data = source, .size = source_size};
}

bool string_is_empty(const String str)
{
    return str.size == 0;
}

bool string_is_equal(const String str1, const String str2)
{
    if (str1.size != str2.size)
    {
        return false;
    }
    for (size_t i = 0; i < str1.size; i++)
    {
        if (str1.data[i] != str2.data[i])
        {
            return false;
        }
    }
    return true;
}

bool string_is_equal_cstr(const String str1, const char* str2)
{
    if (str2[str1.size] != 0)
    {
        // C string must be zero terminated
        return false;
    }
    for (size_t i = 0; i < str1.size; i++)
    {
        if (str1.data[i] != str2[i])
        {
            return false;
        }
    }
    return true;
}

char string_at(const String str, const size_t index)
{
    if (index >= str.size)
    {
        return -1;
    }
    return str.data[index];
}

int string_index(const char character)
{
    return 0;
}

/*
 * Removes empty spaces at the beginning and
 * at the end of the string.
 *
 * New lines are also considered spaces
 *
 * Returns a String with the trimmed string
 */
String string_trim(const String str)
{
    size_t start_index = 0;
    while (isspace(str.data[start_index]) != 0 && start_index < str.size)
    {
        ++start_index;
    }
    if (start_index == str.size)
    {
        return (String){
            .data = nullptr,
            .size = 0
        };
    }

    size_t end_index = str.size - 1;
    while (isspace(str.data[end_index]) != 0 && end_index > start_index)
    {
        --end_index;
    }
    end_index++;

    return (String){
        .data = &str.data[start_index],
        .size = end_index - start_index
    };
}

void string_print(const String self)
{
    for (size_t i = 0; i < self.size; i++)
    {
        putchar(self.data[i]);
    }
    putchar('\n');
}

bool is_delimiter(const char c, const String delimiters)
{
    for (size_t i = 0; i < delimiters.size; i++)
    {
        if (delimiters.data[i] == c)
        {
            return true;
        }
    }
    return false;
}

bool string_contains_char(const String str, const char character)
{
    for (size_t i = 0; i < str.size; i++)
    {
        if (str.data[i] == character)
        {
            return true;
        }
    }
    return false;
}

String string_from_string_buffer(const StringBuffer buffer)
{
    return (String){.data = buffer.data, .size = buffer.size};
}
