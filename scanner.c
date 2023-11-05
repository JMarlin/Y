#include "scanner.h"

ScanResult _Scanner_getc(Scanner s) {

    ScanResult sr = { 0 };

    sr.err = fread(&(sr.val), 1, 1, s) != 1;

    if(sr.err) {

        //printf("Scanner: EOF\n");
    } else {

        //printf("Scanner: '%c'\n", sr.val);
    }

    return sr;
}

ScanResult _Scanner_GetNextImpl(Scanner s, fpos_t old_pos, char* expected, int expect) {

    ScanResult sr = { .err = 0, .val = 0 };
    
    for(int i = 0; ((expect >= 1 && i < expect) || (expect == 0 && i < 1) )&& !sr.err; i++) {

        sr = _Scanner_getc(s);

        if(expected && (expected[i] != sr.val)) sr.err = 1;
    }

    if(sr.err && (expect || expected)) fsetpos(s, &old_pos);

    return sr;
}

void ScannerSkipWhitespace(Scanner s) {
    
    ScanResult sr = { 0 };
    fpos_t last_pos;
    
    while(1) {

        fgetpos(s, &last_pos);

        sr = _Scanner_getc(s);

        if(sr.err) return;

        if(sr.val > 0x20) {
        
            fsetpos(s, &last_pos);
            
            return;
        }
    }
}


