#ifndef HISTORY_H_
#define HISTORY_H_
#include "jpk.h"
#include <stdbool.h>

typedef struct _History {
    char* path;
    struct _History *next;
    struct _History *prev;
} History;

History* newHistory();
History* addHistory(History**, char*);
History* loadHistory(char *);
bool history_isEmpty(History*);
void saveHistory(History*, char*);

#endif // JPK_H_
