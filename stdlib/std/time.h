//
// Created by cout970 on 2016-10-28.
//

#ifndef COMPUTER_TIME_H
#define COMPUTER_TIME_H

//#ifndef size_t
//typedef unsigned int size_t;
//#endif

#ifndef clock_t
typedef unsigned int clock_t;
#endif

//#ifndef time_t
//typedef int time_t;
//#endif

#define CLOCKS_PER_SEC 20

//#ifndef NULL
//#define NULL (void*)0
//#endif

//char *asctime(const struct tm *timeptr);
//char *ctime(const time_t *timer);
//double difftime(time_t time1, time_t time2);
//struct tm *gmtime(const time_t *timer);
//struct tm *localtime(const time_t *timer);
//time_t mktime(struct tm *timeptr);
//size_t strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr);
//time_t time(time_t *timer);

/**
 *
 * @return current world time in minecraft
 */
clock_t clock(void);

/**
 *
 * @return The number of clock cicles in the CPU since it started
 */
unsigned int cpu_clock(void);

#endif //COMPUTER_TIME_H
