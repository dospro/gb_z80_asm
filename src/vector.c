#include "vector.h"

#include <stdint.h>
#include <string.h>

#include "allocator.h"

Vector Vector_new(const size_t element_size, const size_t capacity)
{
    return Vector_new_with_allocator(element_size, capacity, allocator_default());
}

Vector Vector_new_with_allocator(const size_t element_size, const size_t capacity, const Allocator allocator)
{
    if (element_size == 0)
    {
        return (Vector){.allocator = allocator};
    }
    if (capacity > SIZE_MAX / element_size)
    {
        return (Vector){.element_size = element_size, .allocator = allocator};
    }
    void* const data = capacity == 0 ? nullptr : allocator.alloc(allocator.context, capacity * element_size);
    return (Vector){
        .data = data,
        .size = 0,
        .capacity = data == nullptr ? 0 : capacity,
        .element_size = element_size,
        .allocator = allocator,
    };
}

void Vector_free(Vector* const vector)
{
    if (vector == nullptr) return;
    if (vector->data != nullptr)
    {
        vector->allocator.free(vector->allocator.context, vector->data);
        vector->data = nullptr;
    }
    vector->size = 0;
    vector->capacity = 0;
}

bool Vector_push(Vector* const vector, const void* const element)
{
    return Vector_append(vector, element, 1);
}

void* Vector_at(const Vector* const vector, const size_t index)
{
    if (vector == nullptr || index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}

bool Vector_append(Vector* const vector, const void* const elements, const size_t count)
{
    if (vector == nullptr || vector->element_size == 0)
    {
        return false;
    }
    if (count == 0)
    {
        return true;
    }
    const size_t element_size = vector->element_size;
    const size_t max_capacity = SIZE_MAX / element_size;
    if (elements == nullptr || count > max_capacity - vector->size)
    {
        return false;
    }
    const size_t required = vector->size + count;
    const size_t old_bytes = vector->size * element_size;

    // The old block is released only after copying: elements may point into it.
    void* old_data = nullptr;
    if (required > vector->capacity)
    {
        size_t new_capacity = vector->capacity == 0
                                  ? 4
                                  : vector->capacity > max_capacity / 2
                                  ? max_capacity
                                  : vector->capacity * 2;
        if (new_capacity < required) new_capacity = required;
        if (new_capacity > max_capacity) new_capacity = max_capacity;

        void* const new_data = vector->allocator.alloc(vector->allocator.context, new_capacity * element_size);
        if (new_data == nullptr) return false;
        if (old_bytes > 0)
        {
            memcpy(new_data, vector->data, old_bytes);
        }
        old_data = vector->data;
        vector->data = new_data;
        vector->capacity = new_capacity;
    }

    memcpy((char*)vector->data + old_bytes, elements, count * element_size);
    if (old_data != nullptr)
    {
        vector->allocator.free(vector->allocator.context, old_data);
    }
    vector->size = required;
    return true;
}
