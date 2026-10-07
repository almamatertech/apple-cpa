// A pointer kept in a field signed with pointer authentication, moved two ways.
#include <ptrauth.h>
#include <stddef.h>

struct list {
    int *__ptrauth(ptrauth_key_process_dependent_data, 1, 0x1234) items;
};

// Moves the pointer and stores it straight back.
void move_signed_field(struct list *l, size_t n)
{
    l->items += n;
}

// Moves the pointer, stores it back and also returns it.
int *move_signed_field_and_return(struct list *l, size_t n)
{
    int *p = l->items + n;
    l->items = p;
    return p;
}
