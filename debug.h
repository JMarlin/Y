#ifndef DEBUG_H
#define DEBUG_H

//#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE

#define DEBUG_INDENT_PRINTF(l, s, ...) print_indent(l); printf((s), __VA_ARGS__)
#define DEBUG_INDENT_PRINT(l, s) print_indent(l); printf(s)
#define DEBUG_PRINTF(s, ...) printf(s, __VA_ARGS__)
#define DEBUG_PRINT(s) printf(s)

#else

#define DEBUG_INDENT_PRINTF(l, s, ...)
#define DEBUG_INDENT_PRINT(l, s)
#define DEBUG_PRINTF(s, ...)
#define DEBUG_PRINT(s)

#endif

#endif //DEBUG_H
