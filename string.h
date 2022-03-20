#ifndef STRING_H
#define STRING_H

typedef struct String_s {
    int length;
    int capacity;
    char* data;
} String;

void String_init(String* string, char* s);

String* String_new(char* s);

char* String_append(String* target, String* source);

char* String_appendChar(String* string, char c);

char* String_appendCString(String* string, char* s);

char* String_sliceCString(char* start, char* end, String** string);

void String_cleanUp(String* string);

#endif //STRING_H

