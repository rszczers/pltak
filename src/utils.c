#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getLastDayOfMonth(int month, int year) {
    int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 || year % 100 == 0 || year % 400)
        monthDays[1] = 29;
    return monthDays[month-1];
}

int comp(const void * elem1, const void * elem2) {
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

typedef struct _Date {
    char* timestamp;
    char* day;
    char* month;
    char* year;
} Date;

Date* getDate() {
    Date* date = (Date*)malloc(sizeof(Date));
    date->day = (char*)malloc(3);
    date->month = (char*)malloc(3);
    date->year = (char*)malloc(5);

    time_t timer;
    struct tm* tm_info;
    time(&timer);

    char* temp_time = (char*)malloc(9);

    tm_info = localtime(&timer);
    strftime(temp_time, 9, "%X", tm_info);
    strftime(date->day, 3, "%d", tm_info);
    strftime(date->month, 3, "%m", tm_info);
    strftime(date->year, 5, "%Y", tm_info);

    asprintf(&(date->timestamp), "%s-%s-%sT%s", date->year, date->month, date->day, temp_time);
    //YYYY-MM-DDThh:mm:ss

    return date;
}

int getMonth(Date* date) {
    int month;
    if (date->month[0] == '0') {
        month = date->month[1] - '0';
    } else {
        month = atoi(date->month);
    }
    return month;
}

char* getMonthName(int month) {
    char* months[12] = {"Styczeń", "Luty", "Marzec", "Kwiecień", "Maj",
        "Czerwiec", "Lipiec", "Sierpień", "Wrzesień", "Październik",
        "Listopad", "Grudzień"};
    return months[(month - 1) % 12];
}
