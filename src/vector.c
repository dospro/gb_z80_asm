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
    if (vector == nullptr || element == nullptr || vector->element_size == 0)
    {
        return false;
    }
    if (vector->size >= vector->capacity)
    {
        const size_t new_capacity = vector->capacity == 0 ? 4 : vector->capacity * 2;
        if (new_capacity > SIZE_MAX / vector->element_size)
        {
            return false;
        }
        void* const new_data = vector->allocator.alloc(vector->allocator.context, new_capacity * vector->element_size);
        if (new_data == nullptr) return false;
        if (vector->size > 0)
        {
            memcpy(new_data, vector->data, vector->size * vector->element_size);
        }
        if (vector->data != nullptr)
        {
            vector->allocator.free(vector->allocator.context, vector->data);
        }
        vector->data = new_data;
        vector->capacity = new_capacity;
    }

    memcpy((char*)vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size++;

    return true;
}

void* Vector_at(const Vector* const vector, const size_t index)
{
    if (vector == nullptr || index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}
