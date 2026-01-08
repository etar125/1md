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
#include <e1_sarr.h>

#define VERSION "0.2.0"

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
    int retcode = 1;
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
        fprintf(stderr, "%s not found", filename);
        goto error;
    }
    f = fopen(filename, "r");
    if (!f) {
        perror("File open error");
        goto error;
    }
    
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    rewind(f);
    
    char *buf = malloc(filesize + 1);
    if (!buf) { goto error; }
    
    fread(buf, 1, filesize, f);
    buf[filesize] = '\0';
    file = cstr_to_str(buf, false);
    
    #define dat ln.data
    
    for (; cur < sarr_count(&file); cur++) {
        ln = sarr_getdup(&file, cur);
        if (ln.size == 0) { continue; }
        if (!dat) { goto error; }
        size_t cmdend = 0;
        while (cmdend < ln.size && dat[cmdend] != ' ') { cmdend++; }
        cmd = emptystr();
        cmd.size = cmdend;
        cmd.data = malloc(cmdend + 1);
        if (cmd.data) { goto error; }
        memcpy(cmd.data, dat, cmdend);
        cmd.data[cmdend] = '\0';
        
        if (strcmp(cmd.data, "+p") == 0) { puts("<p>"); }
        else if (strcmp(cmd.data, "-p") == 0) { puts("</p>"); }
        else if (strcmp(cmd.data, "+text") == 0) {
            // Надо по символам читать строку и экранизировать что надо
            if (cmdend + 1 < ln.size) {
                printf("%s\n", &dat[cmdend + 1]);
            }
        } else if (strcmp(cmd.data, "+bold") == 0) { puts("<b>"); }
        else if (strcmp(cmd.data, "-bold") == 0) { puts("</b>"); }
        else if (strcmp(cmd.data, "+italic") == 0) { puts("<i>"); }
        else if (strcmp(cmd.data, "-italic") == 0) { puts("</i>"); }
        else if (strcmp(cmd.data, "+hr") == 0) { puts("<hr>"); }
        else if (strcmp(cmd.data, "+br") == 0) { puts("<br>"); }
        else {
            // Тут должна быть ошибка о неизвестной команде
        }
        
        free(ln.data);
        free(cmd.data);
        ln.data = cmd.data = NULL;
        ln.size = cmd.size = 0;
    }
    
    retcode = 0;
error:
    if (retcode) { fprintf(stderr, "err\n"); }
    if (file.data) { free(file.data); }
    if (ln.data) { free(ln.data); }
    if (cmd.data) { free(cmd.data); }
    if (f) { fclose(f); }
    return retcode;
}
