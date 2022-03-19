
typedef struct VoidList_s {
    int capacity;
    int count;
    void** data;
} VoidList;

void VoidList_init(VoidList* vlist) {
    vlist->capacity = 0;
    vlist->count = 0;
    vlist->data = 0;
}

char* VoidList_add(VoidList* vlist, void* entry) {

    //TODO: There has to be a fast math way to calculate the
    //      next highest power of 2 from an arbitrary number
    if(vlist->capacity < (vlist->count + 1)) {

        vlist->capacity = vlist->capacity == 0
            ? 1
            : (2 * vlist->capacity);

        vlist->data = (void**)realloc(vlist->data, vlist->capacity * sizeof(void*));

        if(!vlist->data) return "Failed to allocate space for a list";
    }
    
    vlist->data[vlist->count++] = entry;

    return 0;
}

void VoidList_cleanUp(VoidList* vlist) {

    if(vlist->capacity > 0) free(vlist->data);
}

