
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"



#define MAX_TBUF_SIZE 64
#define READ_NAME 0
#define READ_VALUE 1

int read_cfgvar(platform_file_t* cfgfile, const char* name, char* outbuf, size_t outbuf_size) {
    int found = 0;
    if(outbuf_size <= 1) {
        fprintf(stderr, "\033[31m(ERROR) '%s': outbuf size is too small\033[0m\n",
                __func__);
        goto error;
    }

    memset(outbuf, 0, outbuf_size);
    
    char tbuf[MAX_TBUF_SIZE+1] = { 0 };
    size_t tbuf_i = 0;
    int read = READ_NAME;

    for(size_t i = 0; i < cfgfile->size; i++) {
        char c = cfgfile->data[i];

        if(c == 0x20 || c == '=' || c == '\n') {
            memset(tbuf, 0, tbuf_i);
            tbuf_i = 0;
            continue;
        }

        if(read == READ_NAME) {
            tbuf[tbuf_i] = c;
            //printf("\033[90m'%s'\033[0m\n", tbuf);
            if(strcmp(tbuf, name) == 0) {
                printf("Found variable name: '%s'\n", tbuf);
                read = READ_VALUE;
                memset(tbuf, 0, tbuf_i);
                tbuf_i = 0;
                continue;
            }

            tbuf_i++;
            if(tbuf_i >= MAX_TBUF_SIZE) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Variable name is too big?\033[0m\n",
                        __func__);
                break;
            }
        }
        else
        if(read == READ_VALUE) {
            if(c == ';') {
                memmove(outbuf, tbuf, tbuf_i);
                found = 1;
                break;
            }
            tbuf[tbuf_i] = c;
            tbuf_i++;
            if(tbuf_i >= MAX_TBUF_SIZE-1) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Value is too big\033[0m\n",
                        __func__);
                break;
            }
        }
    }

    if(!found || (read == READ_NAME)) {
        fprintf(stderr, "\033[35m(WARNING) '%s': \"%s\" Not found in config file\033[0m\n",
                __func__, name);
    }


error:

    return found;
}


