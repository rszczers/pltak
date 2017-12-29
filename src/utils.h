#ifndef UTILS_H_
#define UTILS_H_

typedef struct _Date {
    char* timestamp;
    char* day;
    char* month;
    char* year;
} Date;

Date* getDate();
int getMonth(Date*);
int getLastDayOfMonth(int, int);
int comp(const void*, const void*);
#endif /* ifndef UTILS_H_ */
