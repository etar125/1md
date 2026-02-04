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

#include "1info.h"

char *progname;

int usage() {
    printf("1md v"VERSION"\n"
           "Usage: %s file.md\n"
           "Example:\n"
           "  %s README.md > README.1md\n",
           progname, progname);
    return 0;
}

#define isdigit(x) x >= '0' && x <= '9'

str_t file, ln, end_cmds;
size_t cur;

int cmd(str_t *ln) {
    char *l = ln->data;
    size_t sz = ln->size;
    if (!l || sz == 0) {
        fprintf(stderr, "1cmd: bad args\n");
        return 1;
    }
    #define dat l
    str_t cmd = emptystr();
    size_t cmdend = 1;
    while (cmdend < sz && dat[cmdend] != ' ') { cmdend++; }
    if (cmdend == 1) {
        fprintf(stderr, "1cmd: command is missing\n");
        return 1;
    }
    cmd.size = cmdend - 1;
    cmd.data = malloc(cmd.size + 1);
    if (!cmd.data) {
        fprintf(stderr, "1cmd: malloc error\n");
        return 1;
    }
    memcpy(cmd.data, &dat[1], cmd.size);
    cmd.data[cmd.size] = '\0';
    
    if (strcmp(cmd.data, "raw") == 0) {
        cmdend++;
        while (cmdend < sz) { printf("%c", dat[cmdend++]); }
        printf("\n");
    } else {
        fprintf(stderr, "1cmd: unknown command '%s'\n", cmd.data);
    }
    
    
    #undef dat
    
    return 0;
}

int main(int argc, char **argv) {
    err retcode = 1;
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
        fprintf(stderr, "can't access %s", filename);
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
    //int newline = 0;
    int empty_count = 0;
    bool bold = false;
    bool italic = false;
    int listlvl = -1;
    #define MAXLISTLVL 6
    bool numl[MAXLISTLVL];
    bool listarted[MAXLISTLVL];
    size_t liststart[MAXLISTLVL];
    for (int i = 0; i < MAXLISTLVL; i++) {
        numl[i] = false, liststart[i] = 0, listarted[i] = 0;
    }
    
    for (; cur < sarr_count(&file); cur++) {
        ln = sarr_getdup(&file, cur);
        
        if (ln.size == 0) { empty_count++; continue; }
        if (!dat) { error(ERR_MALLOC); }
        if (empty_count) {
            empty_count = 0;
            newline = false;
            if (p) { p = false; puts("-p"); }
            while (listlvl != -1) {
                if (listarted[listlvl]) {
                    listarted[listlvl] = false;
                    puts("-el");
                }
                puts(numl[listlvl] ? "-nlist" : "-list");
                numl[listlvl] = false;
                liststart[listlvl] = 0;
                listlvl--;
            }
        }
        if (dat[0] == '?' && cmd(&ln) != 0) {
            error(ERR_CMD_ERROR);
        }
        
        bool started = false;
        bool skipws = false;
        bool text = false;
        size_t k = 0;
        bool addnl = false;
        for (; k < ln.size; k++) {
            if (!skipws) {
                while (k < ln.size && (dat[k] == ' ' || dat[k] == '\t')) {
                    k++;
                } skipws = true;
            }
            while (listlvl > -1 && k < liststart[listlvl]) {
                if (listarted[listlvl]) {
                    listarted[listlvl] = false;
                    puts("-el");
                }
                puts(numl[listlvl] ? "-nlist" : "-list");
                numl[listlvl] = false;
                liststart[listlvl] = 0;
                listlvl--;
            }
            if (!text) {
                if (isdigit(dat[k])) {
                    size_t start = k;
                    while (k < ln.size && isdigit(dat[k])) { k++; }
                    k++;
                    if (k >= ln.size || dat[k - 1] != '.' || dat[k] != ' ') {
                        k = start;
                        goto skipnt;
                    }
                    if (p) { p = false; puts("-p"); }
                    newline = false;
                    k++;
                    if (listlvl > -1) {
                        if (liststart[listlvl] == start) {
                            if (listarted[listlvl]) {
                                puts("-el");
                            }
                            if (!numl[listlvl]) {
                                puts("-list");
                                puts("+nlist");
                            }
                            numl[listlvl] = true;
                            puts("+el");
                            goto skipnt;
                        } else if (start - liststart[listlvl] >= 4) {
                            if (listlvl == 5) {
                                error(ERR_MAX_LIST_LVL_REACHED);
                            }
                        }
                    }
                    listlvl++;
                    numl[listlvl] = true;
                    liststart[listlvl] = start;
                    listarted[listlvl] = true;
                    puts("+nlist");
                    puts("+el");
                    goto skipnt;
                } else if (dat[k] == '-') {
                    char ch = dat[k];
                    size_t start = k;
                    while (k < ln.size && dat[k] == ch) { k++; }
                    if (dat[k] != '\0') {
                        if (p) { p = false; puts("-p"); }
                        newline = false;
                        if (k - start == 1 && dat[k] == ' ') {
                            k++;
                            if (listlvl > -1) {
                                if (liststart[listlvl] == start) {
                                    if (listarted[listlvl]) {
                                        puts("-el");
                                    }
                                    if (numl[listlvl]) {
                                        puts("-nlist");
                                        puts("+list");
                                    }
                                    puts("+el");
                                    goto skipnt;
                                } else if (start - liststart[listlvl] >= 4) {
                                    if (listlvl == 5) {
                                        error(ERR_MAX_LIST_LVL_REACHED);
                                    }
                                }
                            }
                            listlvl++;
                            numl[listlvl] = false;
                            liststart[listlvl] = start;
                            listarted[listlvl] = true;
                            puts("+list");
                            puts("+el");
                            goto skipnt;
                        } else {
                            k = start;
                            goto skipnt;
                        }
                    }
                    else {
                        if (p) { p = false; puts("-p"); }
                        printf("+hr\n");
                        break;
                    }
                } else if (dat[k] == '#') {
                    if (p) { p = false; puts("-p"); }
                    int hlvl = 0;
                    while (k < ln.size && dat[k] == '#') { k++, hlvl++; }
                    if (hlvl > 6) { hlvl = 6; }
                    if (k + 2 < ln.size) {
                        k++;
                        printf("+h %d %s", hlvl, &dat[k]);
                    }
                    puts("");
                    break;
                }
                else {
skipnt:
                    if (listlvl == -1 && !p) { p = true; puts("+p"); }
                    if (newline) { newline = false; puts("+newline"); }
                    text = true;
                    k--;
                }
            } else {
                if (listlvl != -1 && !listarted[listlvl]) {
                    listarted[listlvl] = true;
                    puts("+el");
                }
                if (dat[k] == '*') {
                    addnl = false;
                    if (started) { started = false; printf("\n"); }
                    if (k + 1 != ln.size && dat[k + 1] == '*') {
                        k++;
                        if (bold) { bold = false; puts("-bold"); }
                        else { bold = true; puts("+bold"); }
                    } else {
                        if (italic) { italic = false; puts("-italic"); }
                        else { italic = true; puts("+italic"); }
                    }
                } else if (dat[k] == '$') {
                    addnl = false;
                    if (started) { started = false; printf("\n"); }
                    if (italic) { italic = false; puts("-italic"); }
                    else { italic = true; puts("+italic"); }
                } else if (dat[k] == '\\' && k + 1 != ln.size) {
                    if (!started) { started = true; printf("+text "); }
                    printf("%c", dat[k + 1]);
                    k++;
                } else if (dat[k] == ' ' && (ln.size - k) == 2 && dat[k + 1] == ' ') {
                    newline += 2;
                    break;
                }
                
                else {
                    if (!started) { started = true; printf("+text "); }
                    printf("%c", dat[k]);
                    addnl = true;
                }
            }
        }
        if (addnl) { printf("\n"); }
        if (text) { puts("+eol"); }
        
        free(ln.data);
        ln.data = NULL;
        ln.size = 0;
    }
    
    while (listlvl != -1) {
        if (listarted[listlvl]) {
            listarted[listlvl] = false;
            puts("-el");
        }
        puts(numl[listlvl] ? "-nlist" : "-list");
        numl[listlvl] = false;
        liststart[listlvl] = 0;
        listlvl--;
    }
    
    if (p) {
        puts("-p");
    }
    
    retcode = 0;
error:
    if (retcode) { fprintf(stderr, "err %d line %zu\n", retcode, cur + 1); }
    if (file.data) { free(file.data); }
    if (ln.data) { free(ln.data); }
    if (end_cmds.data) { free(end_cmds.data); }
    if (f) { fclose(f); }
    return retcode;
}
