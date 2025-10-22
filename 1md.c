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

#define VERSION "0.25.10_22"
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

char *scrch_results[] = {
    "&amp;",
    "&gt;",
    "&lt;"
};

char scrch(char ch) {
    switch (ch) {
        case '>':
            return 2;
        case '<':
            return 1;
        case '&':
            return 0;
        default:
            return ch;
    }
}

#define printch(x) \
char c = scrch(x);\
if (c < 3) { printf("%s", scrch_results[c]); }\
else { printf("%c", c); }

int is_digit(char ch) {
    if (ch <= '9' && ch >= '0') {
        return 1;
    } return 0;
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
    while ((read_bytes = fread(buf, 1, BUFFSIZE, f))) {
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
             is_code          = false,
             is_inline        = false,
             is_list          = false,
             nlist            = false;
        
        char listch = ' ';

        bool p = false;
        int empty_count = 0;
        
        #define MAXCOUNT 512
        
        while (true) {
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
            
            #define cur ln.data[i]
            
            /* local */
            
            int h = 0;
            int code = 0;
            int dash = 0;
            int ast  = 0;

            bool skip      = false;
            bool skip_hr   = false;
            bool skip_list = false;
            bool skip_code = false;
            bool alr = false;
            bool list_cancel = true;

            for (size_t i = 0; i <= ln.len; i++) {
                if (skip) { break; }
                //if (skip_hr && i != 0) { i--; }
                while (!alr && (cur == ' ' || cur == '\t')) { i++; }
                if (!cur) {
                    if (!alr) { empty_count++; }
                    break;
                }
                if (cur == ' ') {
                    i++;
                    if (!cur) { i--; }
                    if (cur == ' ') {
                        i++;
                        if (!cur) { printf("<br>"); break; }
                        else { i -= 2; }
                    } else { i--; }
                }
                if (empty_count) {
                    if (p) {
                        p = false;
                        printf("</p>\n");
                    }
                    empty_count = 0;
                }
                if (cur == '\\') {
                    i++;
                    if (!cur) { break; }
                    printch(cur);
                    alr = true;
                } else if (!alr && cur == '#') {
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
                    printf("<h%d><a class=\"header-link\" id=\"%s\" href=\"#%s\">",
                           h, wp, wp);
                    for (; cur; i++) {
                        printch(cur);
                    }
                    printf("</a></h%d>\n", h);
                    free(wp);
                    skip = true;
                } else if (!alr && !skip_hr && (cur == '-' || cur == '_' || cur == '*')) {
                    char a = cur;
                    i++;
                    dash = 1;
                    while (cur == a) {
                        i++, dash++;
                    }
                    if (dash >= 3) {
                        if (p) {
                            p = false;
                            printf("</p>\n");
                        }
                        printf("<hr>\n");
                        break;
                    } else { skip_hr = true; i = (size_t)-1; continue; }
                } else if (!alr && !skip_list && (cur == '-' || cur == '*' || is_digit(cur))) {
                    if (is_digit(cur)) {
                        
                        nlist = true;
                        i++;
                        if (cur != '.') {
                            if (is_list) {
                                is_list = false;
                                printf("</li></%s>", nlist ? "ol" : "ul");
                                nlist = false;
                            }
                            skip_list = true; i = (size_t)-1; continue;
                        }
                    }
                    i++;
                    if (cur != ' ') {
                        if (is_list) {
                            is_list = false;
                            printf("</li></%s>", nlist ? "ol" : "ul");
                            nlist = false;
                        }
                        skip_list = true; i = (size_t)-1; continue;
                    }
                    if (is_list) { 
                        printf("</li>\n<li>");
                    } else {
                        is_list = true;
                        printf("<%s><li>", nlist ? "ol" : "ul");
                    }
                    list_cancel = false, alr = true;
                }
                else if (!alr && !skip_code && cur == '`') {
                    i++;
                    code = 1;
                    while (cur == '`') {
                        i++, code++;
                    }
                    if (code != 3) { skip_code = true; i = (size_t)-1; continue; }
                    if (!is_code) {
                        size_t start = i;
                        bool ws = false;
                        if (cur) {
                            while (cur) {
                                if (cur == ' ' || cur == '\t') {
                                    ws = true; break;
                                }
                                i++;
                            }
                            if (ws) { skip_code = true; i = (size_t)-1; continue; }
                            if (p) {
                                p = false;
                                printf("</p>\n");
                            }
                            printf("<pre>");
                            if (ln.data[start]) {
                                printf("<code class=\"language-");
                                i = start;
                                while (cur) {
                                    printch(cur);
                                    i++;
                                }
                                printf("\">\n");
                            } else { printf("<code>\n"); }
                            is_code = true;
                        }
                    } else {
                        is_code = false;
                        printf("</code></pre>\n");
                    }
                }
                
                else {
                    if (!alr) { alr = true; }
                    if (is_list && list_cancel) {
                        printf("</li></%s>\n", nlist ? "ol" : "ul");
                        nlist = false;
                        is_list = false;
                    }
                    if (!is_code && !p) {
                        p = true;
                        printf("<p>");
                    }
                    if ((!is_inline && !is_code) && cur == '*') {
                        i++;
                        ast = 1;
                        while (cur == '*' && ast < 3) {
                            i++, ast++;
                        }
                        i--;
                        if (ast == 1) {
                            if (is_italic) {
                                printf("</i>");
                            } else { printf("<i>"); }
                            is_italic = !is_italic;
                        } else if (ast == 2) {
                            if (is_bold) {
                                printf("</b>");
                            } else { printf("<b>"); }
                            is_bold = !is_bold;
                        }
                    } else if ((!is_inline && !is_code) && cur == '$') {
                        if (is_italic) {
                            printf("</i>");
                        } else { printf("<i>"); }
                        is_italic = !is_italic;
                    } else if (cur == '`') {
                        if (is_inline) {
                            printf("</code>");
                        } else { printf("<code>"); }
                        is_inline = !is_inline;
                    }
                    else { printch(cur); }
                }
            }
            if (alr) { printf("\n"); }
            pos += ln.len + 1;
            free(ln.data);
            ln.data = NULL;
        }
        if (is_list) {
            printf("</li></%s>\n", nlist ? "ol" : "ul");
            nlist = false;
            is_list = false;
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
