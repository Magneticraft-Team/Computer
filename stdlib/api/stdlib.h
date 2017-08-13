//
// Created by cout970 on 2016-10-21.
//

#ifndef COMPUTER_STDLIB_H
#define COMPUTER_STDLIB_H

#ifndef size_t
typedef unsigned int size_t;
#endif

//#ifndef wchar_t
//typedef short int wchar_t;
//#endif

#ifndef div_t
typedef struct {
    int quot;
    int rem;
} div_t;
#endif

#ifndef ldiv_t
typedef struct {
    long int quot;
    long int rem;
} ldiv_t;
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0
//#define ‭RAND_MAX 4294967295‬

//double atof(const char *str);

int atoi(const char *str);

long int atol(const char *str);

//double strtod(const char *str, char **endptr);

long int strtol(const char *str, char **endptr, int base);

unsigned long int strtoul(const char *str, char **endptr, int base);

void *calloc(size_t nitems, size_t size);

void free(void *ptr);

void *malloc(size_t size);

void malloc_compact();

void *realloc(void *ptr, size_t size);

void abort();

int atexit(void (*func)());

void exit(int status);

char *getenv(const char *str);

int system(const char *str);

void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

int abs(int x);

div_t div(int numer, int demom);

long int labs(long int x);

ldiv_t ldiv(long int numer, long int demom);

//int mblen(const char *str, size_t n);

//size_t mbstowcs(schar_t *pwcs, const char *str, size_t n)

//int mbtowc(whcar_t *pwc, const char *str, size_t n)

//size_t wcstombs(char *str, const wchar_t *pwcs, size_t n)

//int wctomb(char *str, wchar_t wchar)

#endif //COMPUTER_STDLIB_H

