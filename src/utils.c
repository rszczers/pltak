#include <stdio.h>

int getLastDayOfMonth(int month, int year) {
    int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 || year % 100 == 0 || year % 400)
        monthDays[1] = 29;
    return monthDays[month-1];
}
