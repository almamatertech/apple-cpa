// The functions the post compiles for both slices, to read their disassembly. Nothing calls
// them. Each one is valid C for in-bounds arguments, except bad_move.
#include <stddef.h>
#include <stdint.h>

// A pointer minus a 64-bit index.
int *sub_index(int *base, size_t index)
{
    return base - index;
}

// Reading an element.
int read_index(const int *base, size_t index)
{
    return base[index];
}

// Writing an element.
void write_index(int *base, size_t index, int value)
{
    base[index] = value;
}

// A pointer plus a constant.
int *add_const(int *p)
{
    return p + 4;
}

// A field's address, a constant offset.
struct pair { long first, second; };

long *field_addr(struct pair *s)
{
    return &s->second;
}

// A field of an element, 16-byte elements.
long *field_addr_index(struct pair *s, size_t index)
{
    return &s[index].second;
}

// 48-byte elements, not a power of two.
struct rec { char bytes[48]; };

struct rec *add_index_48b(struct rec *base, size_t index)
{
    return base + index;
}

// 256-byte elements, beyond what ADDPT's shift can reach.
struct big { char bytes[256]; };

struct big *add_index_256b(struct big *base, size_t index)
{
    return base + index;
}

// The same sum as add_index, done on an integer.
int *add_index_uintptr(int *p, size_t index)
{
    uintptr_t raw = (uintptr_t)p;
    raw += index * sizeof(int);
    return (int *)raw;
}

// Walking a pointer through an array.
long sum_array(const int *begin, const int *end)
{
    long total = 0;
    for (const int *p = begin; p != end; ++p)
        total += *p;
    return total;
}

// A 32-bit signed index.
int *add_index_i32(int *base, int index)
{
    return base + index;
}

// A constant too big for one ADD, 64 MiB.
int *add_large_const(int *p)
{
    return p + 0x1000000;
}

// A pointer minus a constant.
int *sub_const(int *p)
{
    return p - 4;
}

// A 64-bit signed index.
int *add_index_i64(int *base, ptrdiff_t index)
{
    return base + index;
}

// A 32-bit unsigned index.
int *add_index_u32(int *base, unsigned index)
{
    return base + index;
}

// Moving a pointer into a new block after realloc. The distance between the blocks is worked
// out on integers, because subtracting pointers into different objects is undefined in C, then
// added to a pointer into the old block. That addition is undefined too, hence the name.
char *bad_move(char *field, char *old_block, char *new_block)
{
    intptr_t delta = (intptr_t)new_block - (intptr_t)old_block;
    return field + delta;
}
