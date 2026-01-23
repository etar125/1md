/*
 * Данный заголовок содержит VERSION и некоторые макросы.
 * Copyright (c) 2026 etar125
 * Licensed under ISC (see LICENSE)
 */

#ifndef _1INFO_H
#define _1INFO_H

typedef enum {
    ERR_OK = 0,
    ERR_FAILURE = 1,
    ERR_FILE_ACCESS,
    ERR_FILE_OPEN,
    ERR_MALLOC,
    ERR_UNKNOWN_COMMAND,
    ERR_BAD_SYNTAX,
    ERR_D_ADDCH,
    ERR_DSTR_TO_STR,
    ERR_CMD_ERROR
} err;

#define VERSION "0.4.0"
#define error(x) retcode = x; goto error
#define log(x) fprintf(stderr, x)

#endif
