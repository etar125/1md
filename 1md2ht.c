/*
 * 1md to html
 * Copyright (c) 2025-2026 etar125
 * Licensed under ISC (see LICENSE)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <e1_str.h>
#include <e1_dstr.h>
#include <e1_sarr.h>

#include "1info.h"

char *progname;

int usage() {
    printf("1md2ht v"VERSION"\n"
           "Usage: %s file.1md\n"
           "Example:\n"
           "  %s README.1md > README.html\n",
           progname, progname);
    return 0;
}

int main(int argc, char **argv) {
    err retcode = 1;
    progname = argv[0];
    char *filename = NULL;
    str_t file, ln, cmd;
    size_t cur = 0;
    file = ln = cmd = emptystr();
    if (argc != 2) { return usage(); }
    for (int i = 1; i < argc; i++) {
        filename = argv[i];
    }
    if (!filename) { return usage(); }
    
    FILE *f = NULL;
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "can't access %s", filename);
        perror("");
        error(ERR_FILE_ACCESS);
    }
    f = fopen(filename, "r");
    if (!f) {
        perror("File open error");
        error(ERR_FILE_OPEN);
    }
    
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    rewind(f);
    
    char *buf = malloc(filesize + 1);
    if (!buf) { error(ERR_MALLOC); }
    
    fread(buf, 1, filesize, f);
    buf[filesize] = '\0';
    file = cstr_to_str(buf, false);
    
    #define dat ln.data
    
    for (; cur < sarr_count(&file); cur++) {
        ln = sarr_getdup(&file, cur);
        if (ln.size == 0) { continue; }
        if (!dat) { error(ERR_MALLOC); }
        size_t cmdend = 0;
        while (cmdend < ln.size && dat[cmdend] != ' ') { cmdend++; }
        cmd = emptystr();
        cmd.size = cmdend;
        cmd.data = malloc(cmdend + 1);
        if (!cmd.data) { error(ERR_MALLOC); }
        memcpy(cmd.data, dat, cmdend);
        cmd.data[cmdend] = '\0';
        
        if (strcmp(cmd.data, "+p") == 0) { puts("<p>"); }
        else if (strcmp(cmd.data, "-p") == 0) { puts("</p>"); }
        else if (strcmp(cmd.data, "+text") == 0) {
            for (size_t i = cmdend + 1; i < ln.size; i++) {
                switch (dat[i]) {
                    case '&': printf("&amp;"); break;
                    case '<': printf("&lt;"); break;
                    case '>': printf("&gt;"); break;
                    case '"': printf("&quot;"); break;
                    default: printf("%c", dat[i]);
                }
            }
        } else if (strcmp(cmd.data, "+bold") == 0) { printf("<b>"); }
        else if (strcmp(cmd.data, "-bold") == 0) { printf("</b>"); }
        else if (strcmp(cmd.data, "+italic") == 0) { printf("<i>"); }
        else if (strcmp(cmd.data, "-italic") == 0) { printf("</i>"); }
        else if (strcmp(cmd.data, "+hr") == 0) { puts("<hr>"); }
        else if (strcmp(cmd.data, "+newline") == 0) { puts("<br>"); }
        else if (strcmp(cmd.data, "+eol") == 0) { puts(""); }
        else if (strcmp(cmd.data, "+h") == 0) {
            size_t lvlend = cmdend + 1;
            while (lvlend < ln.size && dat[lvlend] != ' ') { lvlend++; }
            if (lvlend - 1 == cmdend || lvlend + 1 >= ln.size) { error(ERR_BAD_SYNTAX); }
            
            size_t sz = lvlend - cmdend;
            char *t = malloc(sz + 1);
            if (!t) { error(ERR_MALLOC); }
            memcpy(t, &dat[cmdend + 1], sz);
            t[sz] = '\0';
            int lvl = atoi(t);
            free(t);
            if (lvl > 6) { lvl = 6; }
            
            dstr_t id = emptydstr();
            
            for (size_t i = lvlend + 1; i < ln.size; i++) {
                char ch = dat[i];
                switch (ch) {
                    case '<':
                    case '>':
                    case '(':
                    case ')':
                    case '[':
                    case ']':
                    case '.':
                    case '&':
                    case '#':
                    case '/':
                        ch = '-';
                        break;
                    case ' ':
                        ch = '_';
                        break;
                }
                if (d_addch(&id, ch) != 0) {
                    if (id.data) { free(id.data); }
                    error(ERR_D_ADDCH);
                }
            }
            
            str_t nid = dstr_to_str(&id, true);
            if (!nid.data) {
                if (id.data) { free(id.data); }
                error(ERR_DSTR_TO_STR);
            }
            
            printf("<h%d><a class=\"header-link\" id=\"%s\" href=\"#%s\">", lvl, nid.data, nid.data);
            for (size_t i = lvlend + 1; i < ln.size; i++) {
                switch (dat[i]) {
                    case '&': printf("&amp;"); break;
                    case '<': printf("&lt;"); break;
                    case '>': printf("&gt;"); break;
                    case '"': printf("&quot;"); break;
                    default: printf("%c", dat[i]);
                }
            }
            printf("</a></h%d>", lvl);
            free(nid.data);
        }
        else {
            fprintf(stderr, "unknown command '%s'\n", cmd.data);
            error(ERR_UNKNOWN_COMMAND);
        }
        
        free(ln.data);
        free(cmd.data);
        ln.data = cmd.data = NULL;
        ln.size = cmd.size = 0;
    }
    
    retcode = 0;
error:
    if (retcode) { fprintf(stderr, "err %d line %zu\n", retcode, cur + 1); }
    if (file.data) { free(file.data); }
    if (ln.data) { free(ln.data); }
    if (cmd.data) { free(cmd.data); }
    if (f) { fclose(f); }
    return retcode;
}
