#include "string.h"
#include <string.h>
#include <stdlib.h>

void String_init(String* string, char* s) {

    string->length = s == 0 ? 0 : strlen(s);
    string->capacity = 0;
    string->data = s;
}

String* String_new(char* s) {

    String* string = (String*)malloc(sizeof(String));

    if(!string) return string;

    String_init(string, s);

    return string;
}

char* String_append(String* target, String* source) {

    int reallocate = 0;
    int original_capacity = target->capacity;
    
    //TODO: There has to be a fast math way to calculate the
    //      next highest power of 2 from an arbitrary number
    while((target->length + source->length) > target->capacity) {

        target->capacity = target->capacity == 0
            ? 1
            : (2 * target->capacity);

        reallocate = 1;
    }

    if(reallocate) {

        char* last_data = target->data;
        int precopy = original_capacity == 0 && target->data != 0;

        if(original_capacity == 0) target->data = 0;

        target->data = (char*)realloc(target->data, target->capacity);

        if(!target->data) return "Failed to reallocate string buffer";
     
        //TODO: error handle
        if(precopy) strncpy(target->data, last_data, target->length);
    }

    //TODO: error handle
    strncpy(&target->data[target->length], source->data, source->length);

    target->length = target->length + source->length;

    return 0;
}

char* String_appendChar(String* string, char c) {

    String temp;

    temp.capacity = 0;
    temp.length = 1;
    temp.data = &c;

    return String_append(string, &temp);
}

char* String_appendCString(String* string, char* s) {

    String temp;

    temp.capacity = 0;
    temp.length = strlen(s);
    temp.data = s;

    return String_append(string, &temp);
}

char* String_sliceCString(char* start, char* end, String** string) {

    *string = (String*)malloc(sizeof(String));

    if(*string == 0) return "Failed to allocate space for a new sliced string";

    (*string)->capacity = 0;
    (*string)->length = (size_t)end - (size_t)start;
    (*string)->data = start;

    return 0;
}

void String_cleanUp(String* string) {

    if(string->capacity > 0) free(string->data);

    free(string);
}
