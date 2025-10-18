/*
 * 1md
 * Copyright (c) 2025 etar125
 * Licensed under ISC (see LICENSE)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define VERSION "0.25.10_18"
#define BUFFSIZE 4096
#define MAXSIZE 32768

char *progname;

int usage() {
    printf("1md v"VERSION"\n"
           "Usage: %s file.md\n"
           "Example:\n"
           "  %s README.md > README.html\n",
           progname, progname);
    return 0;
}

typedef struct {
    char *data;
    size_t len;
} str_t;    

str_t concat(char *s1, char *s2) {
    str_t ret;
    ret.data = NULL;
    if (!s1 || !s2) { return ret; }
    size_t l1 = strlen(s1),
           l2 = strlen(s2),
           n  = l1 + l2;
    char *res = malloc(n + 1);
    if (!res) { perror("malloc"); return ret; }
    memcpy(res, s1, l1);
    memcpy(res + l1, s2, l2);
    res[n] = '\0';
    ret.data = res;
    ret.len = n;
    return ret;
}

str_t getln(char *buf, size_t size) {
    str_t ret;
    ret.data = NULL;
    if (!buf || size == 0) { ret.data = (char*)2; return ret; }
    size_t end = 0;
    for (; end < size; end++ ) {
        if (buf[end] == '\n' || buf[end] == '\0') {
            if (end == 0 || buf[end - 1] != '\\') {
                break;
            }
        }
    }
    if (end == size) { ret.data = (char*)1; return ret; }
    char *res = malloc(end + 1);
    if (!res) { perror("malloc"); return ret; }
    memcpy(res, buf, end);
    res[end] = '\0';
    ret.data = res;
    ret.len = end;
    return ret;
}

int main(int argc, char **argv) {
    progname = argv[0];
    if (argc != 2) {
        return usage();
    }
    char *buf = malloc(BUFFSIZE + 1);
    size_t sz = BUFFSIZE;
    size_t pos = 0;
    if (!buf) { perror("malloc"); return 1; }
    buf[BUFFSIZE] = '\0';
    char *tmp = NULL;
    size_t read_bytes;
    FILE *f = NULL;
    
    

    char *filename = argv[1];

    str_t ln;
    ln.data = NULL;

    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "%s not found", filename);
        goto error;
    }
    f = fopen(filename, "r");
    if (!f) {
        perror("File open error");
        goto error;
    }
    while((read_bytes = fread(buf, 1, BUFFSIZE, f))) {
        if (read_bytes != BUFFSIZE) {
            char *t = malloc(read_bytes + 1);
            if (!t) { perror("malloc"); goto error; }
            memcpy(t, buf, read_bytes);
            t[read_bytes] = '\0';
            sz = read_bytes;
            free(buf);
            buf = t;
        } else if (sz != BUFFSIZE) { sz = BUFFSIZE; }
        if (tmp) {
            str_t new = concat(tmp, buf);
            if (!new.data) { goto error; }
            free(buf);
            free(tmp);
            tmp = NULL;
            buf = new.data;
            sz = new.len;
        }
        
        /* global */

        bool is_bold          = false,
             is_italic        = false,
             is_underline     = false,
             is_strikethrough = false,
             is_code          = false;

        int code = 0;
        int dash = 0;
        int ast  = 0;
        bool p = false;
        int empty_count = 0;
        
        #define MAXCOUNT 512
        
        //int maxco = 0;
        while (true) {
            //maxco++;
            ln = getln(buf + pos, sz - pos);
            if (ln.data == (char*)2) { break; }
            if (!ln.data) { goto error; }
            if (ln.data == (char*)1) {
                if (!tmp) { tmp = buf; }
                else {
                    char *t = tmp;
                    str_t new = concat(tmp, buf);
                    if (!new.data) { free(t); goto error; }
                    tmp = new.data;
                    free(t);
                }
                pos = 0;
                buf = malloc(BUFFSIZE + 1);
                if (!buf) { perror("malloc"); free(tmp); goto error; }
                buf[BUFFSIZE] = '\0';
                continue;
            }
            
            /*if (maxco > MAXCOUNT) {
                printf("Ахтунг 1!\n%s\n%lu\n", ln.data, pos);
            }*/


            #define cur ln.data[i]
            /* deprecated?
            bool bold          = false,
                 italic        = false,
                 underline     = false,
                 strikethrough = false,
                 header = false,
                 code   = false;*/
            
            /* local */
            
            int h = 0;

            bool skip = false;
            bool skip_hr = false;
            bool alr = false;

            //size_t sum = 0;
            for (size_t i = 0; i <= ln.len; i++) {
                /*sum++;
                if (sum > MAXCOUNT) {
                    printf("Ахтунг 2!\n%s\n%lu\n", ln.data, i);
                }*/
                if (skip) { break; }
                //if (skip_hr && i != 0) { i--; }
                while (!alr && (cur == ' ' || cur == '\t')) { i++; }
                if (!cur) {
                    empty_count++;
                    break;
                }
                if (empty_count) {
                    if (p) {
                        p = false;
                        printf("</p>\n");
                    }
                    empty_count = 0;
                }
                if (!alr && cur == '#') {
                    h++;
                    if (p) {
                        p = false;
                        printf("</p>\n");
                    }
                    i++;
                    while (cur && cur == '#' ) {
                        h++, i++;
                    }
                    if (!cur) { break; }
                    if (cur == ' ') { i++; }
                    if (!cur) { break; }
                    size_t len = ln.len - i;
                    char *wp = malloc(len + 1);
                    if (!wp) { perror("malloc"); goto error; }
                    memcpy(wp, ln.data + i, len);
                    for (size_t b = 0; b < len; b++) {
                        if (wp[b] == ' ' || wp[b] == '<' || wp[b] == '>' || wp[b] == '&') {
                            wp[b] = '_';
                        }
                    }
                    wp[len] = '\0';
                    printf("<a id=\"%s\" href=\"#%s\"><h%d>",
                           wp, wp, h);
                    for (; cur; i++) {
                        switch (cur) {
                            case '<':
                                printf("&lt;");
                                break;
                            case '>':
                                printf("&gt;");
                                break;
                            case '&':
                                printf("&amp;");
                                break;
                            default:
                                printf("%c", cur);
                                break;
                        }
                    }
                    printf("</h%d></a>\n", h);
                    free(wp);
                    skip = true;
                } else if (!alr && !skip_hr && (cur == '-' || cur == '_' || cur == '*')) {
                    char a = cur;
                    i++;
                    int count = 1;
                    while (cur && cur == a) {
                        i++, count++;
                    }
                    if (count >= 3) {
                        if (p) {
                            p = false;
                            printf("</p>\n");
                        }
                        printf("<hr>\n");
                        break;
                    } else { skip_hr = true; i = (size_t)-1; continue; }
                } else {
                    if (!alr) { alr = true; }
                    if (!p) {
                        p = true;
                        printf("<p>");
                    }
                    printf("%c", cur);
                }
                /*switch(cur) {
                    case '#':
                        h++;
                        break;
                    case '`':
                        code++;
                        break;
                    case '-':
                        dash++;
                        break;
                    case '*':
                        ast++;
                        break;
                    default:
                        if (h) {
                            if (cur == ' ') {
                                i++;
                                if (!cur) { break; }
                            }
                            size_t len = ln.len - i;
                            char *wp = malloc(len + 1);
                            if (!wp) { perror("malloc"); goto error; }
                            memcpy(wp, ln.data + i, len);
                            for (size_t i = 0; i < len; i++) {
                                if (wp[i] == ' ') {
                                    wp[i] = '_';
                                }
                            }
                            printf("<a id=\"%s\" href=\"#%s\"><h%d>%s</h%d></a>\n",
                                   wp, wp, h, ln.data + i, h);
                            free(wp);
                            skip = true;
                        } else {
                            printf("%s\n", ln.data);
                            skip = true;
                        }
                        break;
                }*/
            }
            if (alr) { printf("\n"); }
            pos += ln.len + 1;
            free(ln.data);
            ln.data = NULL;
        }
        if (p) {
            printf("</p>\n");
        }
        pos = 0;
    }
    
    return 0;
error:
    free(ln.data);
    free(buf);
    fclose(f);
    return 1;
}
