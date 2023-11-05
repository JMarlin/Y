#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <string.h>

typedef FILE* Scanner;

typedef struct ScanResult_S {
    char val;
    int err;
} ScanResult;

ScanResult _Scanner_GetNextImpl(Scanner s, fpos_t old_pos, char* expected, int expect);
void ScannerSkipWhitespace(Scanner s);

#define NewScanner(f) \
    (Scanner)(f)

#define ScannerCheckpoint(s) \
    (fgetpos((s), &S_last_pos) == 0)

#define ScannerDeclareHiddenLocals \
    fpos_t S_original_pos; \
    fpos_t S_last_pos; \
    char S_tmp_c; 

#define ScannerBegin(s) \
    ScannerDeclareHiddenLocals \
    if(fgetpos((s), &S_original_pos)) return "Failed to get file position"; \
    S_last_pos = S_original_pos

#define ScannerRollbackLast(s) \
    fsetpos((s), &S_last_pos)

#define ScannerRollbackFull(s) \
    fsetpos((s), &S_original_pos)

#define ScannerNextIs(s, c) \
    (_Scanner_GetNextImpl((s), S_original_pos, (S_tmp_c = c, &S_tmp_c), 1).err == 0)

#define ScannerNextIsStr(s, str) \
    (_Scanner_GetNextImpl((s), S_original_pos, (str), strlen(str)).err == 0)

#define ScannerGetNext(s) \
    _Scanner_GetNextImpl((s), S_original_pos, 0, 0) 

#define ScannerGetNextStrict(s) \
    _Scanner_GetNextImpl((s), S_original_pos, 0, 1)

#define ScannerAtEnd(s) \
    feof(s)

#endif //SCANNER_H
