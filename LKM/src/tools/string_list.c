#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/limits.h>
#include <linux/uaccess.h>

#include "string_list.h"

PStringList list_create(void){
    return (PStringList)kzalloc(sizeof(StringList), GFP_KERNEL);
}

bool list_insert(PStringList list, char* value){
    size_t v_len;
    PStringListEntry current_entry, new_entry;
    if (list == NULL || value == NULL){
        return false;
    }

    new_entry = (PStringListEntry)kzalloc(sizeof(StringListEntry), GFP_KERNEL);
    if (new_entry == NULL){
        return false;
    }
    
    v_len = strnlen(value, PATH_MAX);
    if (v_len == 0){
        kfree(new_entry);
        return false;
    }

    new_entry->value = (char*)kzalloc(v_len+1, GFP_KERNEL);
    if(new_entry->value == NULL){
        kfree(new_entry);
        return false;
    }
    
    strncpy(new_entry->value, value, v_len);
    if(list->head == NULL){
        list->head = new_entry;
    }
    else{
        current_entry = list->head;
        while(current_entry->next!=NULL){
            current_entry = current_entry->next;
        }
        current_entry->next = new_entry;
    }
    list->count+=1;
    return true;
    
}

bool list_delete(PStringList list, char* value){
    PStringListEntry prev_entry, cur_entry;
    if(list == NULL || value == NULL)
        return false;
    if(list->head == NULL){
        return true;
    }
    prev_entry = NULL;
    cur_entry = list->head;
    while(cur_entry != NULL){
        if (!strncmp(cur_entry->value, value, PATH_MAX)){
            if(prev_entry == NULL){
                list->head = cur_entry->next;
                kfree(cur_entry->value);
                kfree(cur_entry);
                cur_entry = list->head;
            }else{
                prev_entry->next = cur_entry->next;
                kfree(cur_entry->value);
                kfree(cur_entry);
                cur_entry = prev_entry->next;
            }
        }else{
            prev_entry = cur_entry;
            cur_entry = cur_entry->next;
        }
    }
    return true;
}

bool list_have(PStringList list, char* value){
    PStringListEntry cur_entry;
    if(list == NULL || value == NULL){
        return false;
    }
    cur_entry = list->head;
    while(cur_entry!=NULL){
        if(!strncmp(cur_entry->value, value, PATH_MAX))
            return true;
        cur_entry = cur_entry->next;
    }
    return false;
}

void list_destroy(PStringList list){
    PStringListEntry cur_entry;
    if(list == NULL)
        return;
    cur_entry = list->head;
    while(cur_entry!=NULL){
        PStringListEntry del_entry = cur_entry;
        cur_entry = cur_entry->next;
        kfree(del_entry->value);
        kfree(del_entry);
    }
    kfree(list);
}
