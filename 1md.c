/*
 * 1md
 * Copyright (c) 2025-2026 etar125
 * Licensed under ISC (see LICENSE)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <e1_str.h>
#include <e1_sarr.h>

#define VERSION "0.3.0"
#define error(x) retplace = x; goto error

char *progname;

int usage() {
    printf("1md v"VERSION"\n"
           "Usage: %s file.md\n"
           "Example:\n"
           "  %s README.md > README.1md\n",
           progname, progname);
    return 0;
}

str_t file, ln, end_cmds;
size_t cur;

int cmd(char *str) {
    fprintf(stderr, "1cmd: Not implemented");
    return 0;
}

int main(int argc, char **argv) {
    int retcode = 1;
    int retplace = 0;
    file = ln = end_cmds = emptystr();
    cur = 0;
    progname = argv[0];
    char *filename = NULL;
    if (argc != 2) { return usage(); }
    for (int i = 1; i < argc; i++) {
        filename = argv[i];
    }
    if (!filename) { return usage(); }
    
    FILE *f = NULL;
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "%s not found", filename);
        error(0);
    }
    f = fopen(filename, "r");
    if (!f) {
        perror("File open error");
        error(1);
    }
    
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    rewind(f);
    
    char *buf = malloc(filesize + 1);
    if (!buf) { error(2); }
    
    fread(buf, 1, filesize, f);
    buf[filesize] = '\0';
    file = cstr_to_str(buf, false);
    
    /* OLD */
    /*
    bool is_bold          = false,
         is_italic        = false,
         is_underline     = false,
         is_strikethrough = false,
         is_code          = false,
         is_inline        = false,
         is_list          = false,
         nlist            = false;
    int add_br = 0;
    int empty_count = 0;
    int list_level = 0;
    
    bool lvls_nlist[6] = {
        false, false, false, false, false, false
    };
    bool lvls_is_list[6] = {
        false, false, false, false, false, false
    };
    
    size_t latest_list_i = 0;
    */
    /* =-=-=-=-=-=-=-= */
    
    #define dat ln.data
    bool p = false;
    bool newline = false;
    int empty_count = 0;
    bool bold = false;
    bool italic = false;
    
    for (; cur < sarr_count(&file); cur++) {
        ln = sarr_getdup(&file, cur);
        
        if (ln.size == 0) { empty_count++; continue; }
        if (!dat) { error(3); }
        if (empty_count) {
            empty_count = 0;
            newline = false;
            if (p) { p = false; puts("-p"); }
        }
        if (dat[0] == '?' && cmd(dat) != 0) {
            error(4);
        }
        
        bool started = false;
        bool skipws = false;
        bool text = false;
        size_t k = 0;
        for (; k < ln.size; k++) {
            if (!skipws) {
                while (k < ln.size && (dat[k] == ' ' || dat[k] == '\t')) {
                    k++;
                } skipws = true;
            }
            if (!text) {
                if (dat[k] == '-') {
                    char ch = dat[k];
                    size_t start = k;
                    while (k < ln.size && dat[k] == ch) { k++; }
                    if (dat[k] != '\0') { k = start; }
                    else {
                        if (p) { p = false; puts("-p"); }
                        printf("+hr");
                        break;
                    }
                }
                
                else {
                    if (!p) { p = true; puts("+p"); }
                    if (newline) { newline = false; puts("+newline"); }
                    text = true;
                    k--;
                    continue;
                }
            } else {
                if (dat[k] == '*') {
                    if (started) { started = false; printf("\n"); }
                    if (k + 1 != ln.size && dat[k + 1] == '*') {
                        k++;
                        if (bold) { bold = false; puts("-bold"); }
                        else { bold = true; puts("+bold"); }
                    } else {
                        if (italic) { italic = false; puts("-italic"); }
                        else { italic = true; puts("+italic"); }
                    }
                    //printf("+text ");
                } else if (dat[k] == '$') {
                    if (started) { started = false; printf("\n"); }
                    if (italic) { italic = false; puts("-italic"); }
                    else { italic = true; puts("+italic"); }
                    //printf("+text ");
                } else if (dat[k] == '\\' && k + 1 != ln.size) {
                    if (!started) { started = true; printf("+text "); }
                    printf("%c", dat[k + 1]);
                    k++;
                } else if (dat[k] == ' ' && (ln.size - k) == 2 && dat[k + 1] == ' ') {
                    newline = true;
                    break;
                }
                
                else {
                    if (!started) { started = true; printf("+text "); }
                    printf("%c", dat[k]);
                }
            }
        }
        printf("\n");
        if (text) { puts("+eol"); }
        
        free(ln.data);
        ln.data = NULL;
        ln.size = 0;
    }
    
    if (p) {
        puts("-p");
    }
    
    retcode = 0;
error:
    if (retcode) { fprintf(stderr, "err %d\n", retplace); }
    if (file.data) { free(file.data); }
    if (ln.data) { free(ln.data); }
    if (end_cmds.data) { free(end_cmds.data); }
    if (f) { fclose(f); }
    return retcode;
}
