//
// Created by cout970 on 2016-10-28.
//

#include "string.h"
#include "assert.h"

void *memchr(const void *str, int c, size_t n) {
    for (int i = 0; i < (int) n; i++) {
        if (((uint8_t *) str)[i] == (uint8_t) c)
            return ((uint8_t *) str) + i;
    }
    return NULL;
}

int memcmp(const void *str1, const void *str2, size_t n) {
    for (int i = 0; i < (int) n; i++) {
        if (((uint8_t *) str1)[i] != ((uint8_t *) str2)[i]) {
            return ((int8_t *) str1)[i] - ((int8_t *) str2)[i];
        }
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
    for (int i = 0; i < (int) n; i++) {
        ((uint8_t *) dest)[i] = ((uint8_t *) src)[i];
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t a[n];
    for (int i = 0; i < (int) n; ++i) {
        a[i] = ((uint8_t *) src)[i];
    }
    for (int i = 0; i < (int) n; ++i) {
        ((uint8_t *) dest)[i] = a[i];
    }
    return dest;
}

void *memset(void *str, int c, size_t n) {
    for (int i = 0; i < (int) n; i++) {
        ((uint8_t *) str)[i] = (uint8_t) c;
    }
    return str;
}

char *strcat(char *dest, const char *src) {
    int i = 0, j = 0;
    i = strlen(dest);
    do {
        dest[i] = src[j];
        i++;
    } while (src[j++] != '\0');

    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    int i = 0, j = 0;
    i = strlen(dest);
    do {
        dest[i] = src[j];
        j++;
        i++;
    } while (src[j] != '\0' && j++ < (int) n);

    return dest;
}

char *strchr(const char *str, int c) {
    int i = 0;
    do {
        if (str[i] == (char) c)
            return (char *) str + i;
    } while (str[i++] != '\0');

    return (char *) NULL;
}

int strcmp(const char *a, const char *b) {
    int i = 0;
    while (1) {
        if (a[i] != b[i]) return 0;
        if (a[i] == '\0') break;
        i++;
    }
    return 1;
}

int strncmp(const char *a, const char *b, size_t n) {
    int i = 0;
    while (1) {
        if (a[i] != b[i]) return 0;
        if (a[i] == '\0' || i >= (int) n) break;
        i++;
    }
    return 1;
}

int strcoll(const char *str1, const char *str2) {
    return strcmp(str1, str2);
}

char *strcpy(char *dest, const char *src) {
    int i = 0;
    while (1) {
        dest[i] = src[i];
        if (src[i] == '\0') break;
        i++;
    }
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    int i = 0;
    while (1) {
        if (i >= (int) n)break;
        dest[i] = src[i];
        if (src[i] == '\0') break;
        i++;
    }
    return dest;
}

size_t strcspn(const char *str1, const char *str2) {
    int i = 0, j, f;
    do {
        f = 0;
        j = 0;
        do {
            if (str1[i] == str2[j]) {
                f = 1;
                break;
            }
        } while (str2[++j] != '\0');

        if (!f) {
            if (i <= 0) {
                return 0;
            }

            return (size_t) (i - 1);
        }
    } while (str1[++i] != '\0');

    return (size_t) (i - 1);
}

char *strerror(int errnum IGNORED) {
    return (char *) NULL;
}

size_t strlen(const char *str) {
    int i = 0;
    while (str[i++] != '\0');
    return (size_t) (i - 1);
}

char *strpbrk(const char *str1, const char *str2) {
    int i = 0, j, f;
    do {
        f = 0;
        j = 0;
        do {
            if (str1[i] == str2[j]) {
                f = 1;
                break;
            }
        } while (str2[++j] != '\0');

        if (!f) {
            return (char *) str1 + i;
        }
    } while (str1[++i] != '\0');

    return (char *) NULL;
}

char *strrchr(const char *str1, int c) {
    int i = 0;
    char *last = (char *) NULL;
    do {
        if (str1[i] == (char) c) {
            last = (char *) str1 + i;
        }
    } while (str1[++i] != '\0');

    return last;
}

size_t strspn(const char *str1, const char *str2) {
    int i = 0, j, f;
    do {
        f = 0;
        j = 0;
        do {
            if (str1[i] == str2[j]) {
                f = 1;
                break;
            }
        } while (str2[++j] != '\0');

        if (!f) {
            return (size_t) i;
        }
    } while (str1[++i] != '\0');

    return 0;
}

char *strstr(const char *str1, const char *str2) {
    int i = 0, j = 0, k = 0;
    do {
        if (str2[j] == '\0') {
            return (char *) str1 + k;
        }
        if (str1[i] == str2[j]) {
            if (j == 0) k = i;
            j++;
        } else {
            if (j > 1) {
                i = k;
            }
            j = 0;
        }

    } while (str1[++i] != '\0');

    return (char *) NULL;
}

int strcnt(const char *str, char c) {
    int i = 0;
    do {
        if (str[i] == c) return 1;

    } while (str[++i] != '\0');

    return 0;
}

static char *tokstr;
static int tokIndex;

char *strtok(char *str, const char *delim) {
    if (str != (char *) NULL) {
        tokstr = str;
        tokIndex = 0;
    }
    int start = -1;
    do {
        if (start == -1) {
            if (!strcnt(delim, tokstr[tokIndex])) {
                start = tokIndex;
            } else {
                continue;
            };
        } else {
            if (strcnt(delim, tokstr[tokIndex])) {
                tokstr[tokIndex++] = '\0';
                return tokstr + start;
            }
        }
    } while (tokstr[++tokIndex] != '\0');
    return (char *) NULL;
}