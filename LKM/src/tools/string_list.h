#ifndef _RK_STRING_LIST_H
#define _RK_STRING_LIST_H

#include <linux/types.h>

typedef struct _string_list_entry{
    char *value;
    struct _string_list_entry* next;
} StringListEntry, *PStringListEntry;

typedef struct _string_list{
    PStringListEntry head;
    ssize_t count;
} StringList, *PStringList;


PStringList list_create(void);
bool list_insert(PStringList list, char* value);
bool list_delete(PStringList list, char* value);
bool list_have(PStringList list, char* value);
void list_destroy(PStringList list);

#endif
