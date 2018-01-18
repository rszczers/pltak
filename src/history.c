#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "jpk.h"
#include "utils.h"

#define CONSTR 10

typedef struct _History {
    char* path;
    struct _History *next;
    struct _History *prev;
} History;

History* newHistory() {
    History* n = (History*)malloc(sizeof(History));
    n->path = NULL;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

bool history_isEmpty(History* h) {
    if (h->path == NULL &&
        h->prev == NULL &&
        h->next == NULL)
        return true;
    else return false;
}

History* addHistory(History** hist, char* path) {
    assert(hist != NULL);

    // Jesli lista jest pusta
    if (history_isEmpty(*hist)) {
        (*hist)->path = strdup(path);
        (*hist)->next = NULL;
        (*hist)->prev = NULL;

        // nie dodajemy tych smych sciezek
    } else if (strcmp((*hist)->path, path) != 0) {
        History* n = (History*)malloc(sizeof(History));
        n->path = strdup(path);
        n->next = *hist;
        n->prev = NULL;
        (*hist)->prev = n;
        *hist = n;
    }

    // Usuń ewentualne powtórzenia
    History* cur = (*hist)->next;
    while(cur != NULL) {
        if (strcmp(cur->path, path) == 0) {
            cur->prev->next = cur->next;
        }
        cur = cur->next;
    }

    return *hist;
}

void saveHistory(History* h, char* type) {
    assert(!history_isEmpty(h));
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *filename;
    asprintf(&filename, "%s/.pltak/%s", homedir, type);
    FILE * file = fopen(filename, "wb");
    if (file != NULL) {
        int count = 0;
        while (h != NULL && count < CONSTR) {
            char* out;
            if (h->next != NULL) asprintf(&out, "%s\n", h->path);
            else asprintf(&out, "%s", h->path);
            fputs(out, file);            
            h = h->next;
            count++;
        }
        fclose(file);
    }
}

History* loadHistory(char* name) {
    History* h = newHistory();

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *file;
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *filename;
    asprintf(&filename, "%s/.pltak/%s", homedir, name);

    file = fopen(filename, "r");

    if (file != NULL) {
        while ((read = getline(&line, &len, file)) != -1) {
            if (line[read-1] == '\n') line[read-1] = '\0';
            addHistory(&h, line);
        }
        fclose(file);
    }
    //reverse
    if (!history_isEmpty(h)) {
        History* tmp = h->prev;
        History* prev;
        while (h != NULL) {
            tmp = h->next;
            h->next = h->prev;
            h->prev = tmp;
            prev = h;
            h = tmp;
        }
        return prev;
    }
    return h;
}
