#ifndef VOIDLIST_H
#define VOIDLIST_H

typedef struct VoidList_s {
    int capacity;
    int count;
    void** data;
} VoidList;

void VoidList_init(VoidList* vlist);

char* VoidList_add(VoidList* vlist, void* entry);

void VoidList_cleanUp(VoidList* vlist);

#endif //VOIDLIST_H

