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

#define VERSION "0.25.10_29"
#define BUFFSIZE 4096
#define MAXSIZE 32768

#define log(x) fprintf(stderr, x)

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
            return 1;
        case '<':
            return 2;
        case '&':
            return 0;
        default:
            return ch;
    }
}

void printch(char ch) {
    if ((unsigned char)ch > 127) {
        printf("%c", ch);
        return;
    }
    char c = scrch(ch);
    if (c < 3) { printf("%s", scrch_results[c]); }
    else { printf("%c", c); }
}

int is_digit(char ch) {
    if (ch <= '9' && ch >= '0') {
        return 1;
    } return 0;
}

int main(int argc, char **argv) {
    progname = argv[0];
    char *class = NULL;
    char *id = NULL;
    char *filename = NULL;
    if (argc < 2) {
        return usage();
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-id") == 0) {
                i++;
                if (i < argc) {
                    id = argv[i];
                } else { return usage(); }
            } else if (strcmp(argv[i], "-class") == 0) {
                i++;
                if (i < argc) {
                    class = argv[i];
                } else { return usage(); }
            } else {
                return usage();
            }
        } else { filename = argv[i]; }
    }
    if (!filename) {
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
    
    if (id || class) {
        printf("<div");
        if (class) { printf(" class=\"%s\"", class); }
        if (id) { printf(" id=\"%s\"", id); }
        printf(">\n");
    }

    

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
        
        int add_br = 0;
        
        char listch = ' ';

        bool p = false;
        int empty_count = 0;
        int list_level = 0;
        
        bool lvls_nlist[6] = {
            false, false, false, false, false, false
        };
        bool lvls_is_list[6] = {
            false, false, false, false, false, false
        };
        
        size_t latest_list_i = 0;
        
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
            bool skip_link = false;
            bool alr = false;
            bool list_cancel = true;
            bool my_br = false;
            
            int indent_level = 0;
            
            bool link_is_image = false;

            for (size_t i = 0; i <= ln.len; i++) {
                if (skip) { break; }
                //if (skip_hr && i != 0) { i--; }
                int spaces_count = 0;
                while (!alr && (cur == ' ' || cur == '\t')) {
                    if (cur == ' ') {
                        spaces_count++;
                        if (spaces_count == 3) {
                            spaces_count = 0;
                            indent_level++;
                        }
                    } else {
                        indent_level++;
                    }
                    i++;
                }
                if (indent_level > 5) {
                    fprintf(stderr, "max list level reached, setting to 6\n");
                    indent_level = 5;
                }
                if (!cur) {
                    if (!alr) { empty_count++; }
                    break;
                }
                if (!is_code && cur == ' ') {
                    i++;
                    if (!cur) { i--; }
                    if (cur == ' ') {
                        i++;
                        if (!cur) { add_br++; my_br = true; break; }
                        else { i -= 2; }
                    } else { i--; }
                }
                if (empty_count) {
                    if (p) {
                        p = false;
                        add_br = 0;
                        printf("</p>\n");
                    }
                    empty_count = 0;
                }
                if (lvls_is_list[indent_level] && i == latest_list_i) {
                    list_cancel = false;
                }
                if (cur == '\\') {
                    i++;
                    if (!cur) { break; }
                    if ((add_br == 1 && !my_br) || add_br > 1) { add_br--; printf("<br>\n"); }
                    printch(cur);
                    alr = true;
                } else if (!alr && cur == '#') {
                    h++;
                    if (p) {
                        p = false;
                        add_br = 0;
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
                        if (wp[b] == '?' || wp[b] == ' ' || wp[b] == '<' || wp[b] == '>' || wp[b] == '&') {
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
                            add_br = 0;
                            printf("</p>\n");
                        }
                        printf("<hr>\n");
                        break;
                    } else { skip_hr = true; i = (size_t)-1; continue; }
                } else if (!alr && !skip_list && (cur == '-' || cur == '*' || is_digit(cur))) {
                    for (int i = 5; i > indent_level; i--) {
                        if (lvls_is_list[i]) {
                            lvls_is_list[i] = false;
                            printf("</li></%s>", lvls_nlist[i] ? "ol" : "ul");
                            lvls_nlist[i] = false;
                        }
                    }
                    if (is_digit(cur)) {
                        lvls_nlist[indent_level] = true;
                        i++;
                        while (is_digit(cur)) { i++; }
                        if (cur != '.') {
                            if (lvls_is_list[indent_level]) {
                                lvls_is_list[indent_level] = false;
                                printf("</li></%s>", lvls_nlist[indent_level] ? "ol" : "ul");
                                lvls_nlist[indent_level] = false;
                            }
                            skip_list = true; i = (size_t)-1; continue;
                        }
                    }
                    i++;
                    if (cur != ' ') {
                        if (lvls_is_list[indent_level]) {
                            lvls_is_list[indent_level] = false;
                            printf("</li></%s>", lvls_nlist[indent_level] ? "ol" : "ul");
                            lvls_nlist[indent_level] = false;
                        }
                        skip_list = true; i = (size_t)-1; continue;
                    }
                    if (lvls_is_list[indent_level]) { 
                        printf("</li>\n<li>");
                    } else {
                        if (p) {
                            p = false;
                            add_br = 0;
                            printf("</p>\n");
                        }
                        lvls_is_list[indent_level] = true;
                        printf("<%s><li>", lvls_nlist[indent_level] ? "ol" : "ul");
                    }
                    list_cancel = false, alr = true, latest_list_i = i + 1;
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
                        if (p) {
                            p = false;
                            add_br = 0;
                            printf("</p>\n");
                        }
                        printf("<pre>");
                        if (cur) {
                            while (cur) {
                                if (cur == ' ' || cur == '\t') {
                                    ws = true; break;
                                }
                                i++;
                            }
                            if (ws) { skip_code = true; i = (size_t)-1; continue; }
                            i = start;
                            printf("<code class=\"language-");
                            while (cur) {
                                printch(cur);
                                i++;
                            }
                            printf("\">\n");
                        } else { printf("<code>\n"); }
                        is_code = true;
                    } else {
                        is_code = false;
                        printf("</code></pre>\n");
                        break;
                    }
                }
                
                else {
                    if (!alr) {
                        if ((add_br == 1 && !my_br) || add_br > 1) { add_br--; printf("<br>\n"); }
                        alr = true;
                    }
                    /*if (lvls_is_list[indent_level] && i == latest_list_i) {
                        list_cancel = false;
                    }*/
                    if (list_cancel) {
                        for (int i = 5; i >= indent_level; i--) {
                            if (lvls_is_list[i]) {
                                lvls_is_list[i] = false;
                                printf("</li></%s>", lvls_nlist[i] ? "ol" : "ul");
                                lvls_nlist[i] = false;
                            }
                        }
                    }
                    /*if (lvls_is_list[indent_level] && list_cancel) {
                        printf("</li></%s>\n", lvls_nlist[indent_level] ? "ol" : "ul");
                        lvls_nlist[indent_level] = false;
                        lvls_is_list[indent_level] = false;
                    }*/
                    if (!is_code && !lvls_is_list[indent_level] && !p) {
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
                    } else if (!skip_link && cur == '!') {
                        i++;
                        if (cur == '[') {
                            link_is_image = true;
                            i--;
                        } else { i--; printch(cur); }
                    } else if (!skip_link && cur == '[') {
                        size_t end_text, start_link, end_link;
                        size_t t = i;
                        while (cur && cur != ']') { i++; }
                        #define SKIP skip_link = true; i = t - 1; continue
                        if (!cur) { SKIP; }
                        end_text = i++;
                        if (cur != '(') { SKIP; }
                        i++;
                        if (!cur) { SKIP; }
                        start_link = i;
                        while (cur && cur != ')') { i++; }
                        if (!cur) { SKIP; }
                        end_link = i;
                        if (!link_is_image) {
                            printf("<a href=\"");
                            for (i = start_link; i < end_link; i++) {
                                printch(cur);
                            }
                            printf("\">");
                            i = t;
                        } else {
                            printf("<img src=\"");
                            for (i = start_link; i < end_link; i++) {
                                printch(cur);
                            }
                            printf("\" alt=\"");
                            for (i = t + 1; i < end_text; i++) {
                                printch(cur);
                            }
                            i = end_link;
                            printf("\">");
                        }
                    } else if (!skip_link && cur == ']') {
                        printf("</a>");
                        while (cur && cur != ')') { i++; }
                    }
                    else { printch(cur); }
                }
                //if (add_br) { add_br--; }
            }
            if (alr) {
                if ((add_br == 1 && !my_br) || add_br > 1) { add_br--; printf("<br>\n"); }
                printf("\n");
            }
            pos += ln.len + 1;
            free(ln.data);
            ln.data = NULL;
        }
        for (int indent_level = 5; indent_level >= 0; indent_level--) {
            if (lvls_is_list[indent_level]) {
                printf("</li></%s>\n", lvls_nlist[indent_level] ? "ol" : "ul");
                lvls_nlist[indent_level] = false;
                lvls_is_list[indent_level] = false;
            }
        }
        if (p) {
            printf("</p>\n");
        }
        pos = 0;
    }
    if (id || class) { printf("</div>\n"); }
    return 0;
error:
    free(ln.data);
    free(buf);
    fclose(f);
    return 1;
}
