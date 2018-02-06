#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "jpk.h"
#include <assert.h>

typedef struct {
    char* NIP;
    char* PelnaNazwa;
    char* Email;
    JPKColumns* sellColumns;
    JPKColumns* purchaseColumns;
} TakConfig;

void printTakConfig(TakConfig*);

TakConfig* parseConfig() {
    TakConfig* config = (TakConfig*)malloc(sizeof(TakConfig));

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *filename;
    asprintf(&filename, "%s/.pltak/config", homedir);

    if (access(filename, F_OK) == 0) {
        fp = fopen(filename, "r");
        char *token;
        while ((read = getline(&line, &len, fp)) != -1) {
            token = strtok(line, "=\n");
            if (token != NULL) {
                if (strcmp(token, "NIP") == 0) {
                    char *cur = strtok(NULL, "\n");
                    asprintf(&(config->NIP), "%s",
                           cur == NULL ? "" : cur);
                } else if (strcmp(token, "PelnaNazwa") == 0) {
                    char *cur = strtok(NULL, "\n");
                    asprintf(&(config->PelnaNazwa), "%s",
                           cur == NULL ? "" : cur);
                } else if (strcmp(token, "Email") == 0) {
                    char *cur = strtok(NULL, "\n");
                    asprintf(&(config->Email), "%s",
                           cur == NULL ? "" : cur);
                } else if (strcmp(token, "sellColumns") == 0) {
                    token = strtok(NULL, "=\n");
                    if (token != NULL) {
                        config->sellColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
                        config->sellColumns->next = NULL;
                        config->sellColumns->title = NULL;
                        char *buffer;
                        asprintf(&buffer, "%s", token);
                        buffer = strtok(token, ":\n");
                        while (buffer != NULL) {
                            char* buffer_2;
                            asprintf(&buffer_2, "%s", buffer);
                            addColumn(config->sellColumns, buffer_2);
                            buffer = strtok(NULL, ":\n");
                        }
                    } else {
                        config->sellColumns = NULL;
                    }
                } else if (strcmp(token, "purchaseColumns") == 0) {
                    token = strtok(NULL, "=\n");
                    if (token != NULL) {
                        config->purchaseColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
                        config->purchaseColumns->next = NULL;
                        config->purchaseColumns->title = NULL;
                        char *buffer;
                        asprintf(&buffer, "%s", token);
                        buffer = strtok(token, ":\n");
                        while (buffer != NULL) {
                            char* buffer_2;
                            asprintf(&buffer_2, "%s", buffer);
                            addColumn(config->purchaseColumns, buffer_2);
                            buffer = strtok(NULL, ":\n");
                        }
                    } else {
                        config->purchaseColumns = NULL;
                    }
                }
            }
        }
        fclose(fp);
    } else { // stwórz domyślną konfigurację
        fp = fopen(filename, "ab");
        char* defaultConfig = "NIP=\n"
                              "PelnaNazwa=\n"
                              "Email=\n"
                              "sellColumns=NrKontrahenta:NazwaKontrahenta:AdresKontrahenta:DowodSprzedazy:DataWystawienia:DataSprzedazy:K_19:K_20\n"
                              "purchaseColumns=NrDostawcy:NazwaDostawcy:AdresDostawcy:DowodZakupu:DataZakupu:DataWplywu:K_45:K_46";
        if (fp != NULL) {
            fputs(defaultConfig, fp);
            fclose(fp);
        }
        config->Email =  "";
        config->PelnaNazwa = "";
        config->NIP = "";
    }
    return config;
}

void saveConfig(TakConfig* tak) {
    char* config;
    char* sell = "";
    char* purchase = "";
    char* token;

    // Zamieniam listy na string wartosc1:wartosc2:…:wartoscN
    JPKColumns* col = tak->purchaseColumns;
    while (col != NULL) {
        token = col->title;
        int oldLength = strlen(purchase);
        if (oldLength == 0) {
            asprintf(&purchase, "%s", token);
        } else {
            char *buffer;
            int tokenLength = strlen(token);
            buffer = purchase;
            purchase = (char*)malloc(tokenLength + 1 + oldLength + 1);
            memcpy(purchase, buffer, oldLength);
            purchase[oldLength] = ':';
            memcpy(purchase + oldLength + 1, token, tokenLength);
            purchase[oldLength + tokenLength + 1] = '\0';
        }
        col = col->next;
    }

    col = tak->sellColumns;
    while (col != NULL) {
        token = col->title;
        int oldLength = strlen(sell);
        if (oldLength == 0 && token != NULL) { // jeśli pole w pliku konfiguracyjnym
            asprintf(&sell, "%s", token);      // jest puste, to wsadź tam pierwszy
        } else {                               // z brzegu napis
            char *buffer;
            int tokenLength = strlen(token);
            buffer = sell;
            sell = (char*)malloc(tokenLength + 1 + oldLength + 1);
            memcpy(sell, buffer, oldLength);
            sell[oldLength] = ':';
            memcpy(sell + oldLength + 1, token, tokenLength);
            sell[oldLength + tokenLength + 1] = '\0';
        }
        col = col->next;
    }

    asprintf(&config, "NIP=%s\n"
                      "PelnaNazwa=%s\n"
                      "Email=%s\n"
                      "sellColumns=%s\n"
                      "purchaseColumns=%s",
                tak->NIP != NULL ? tak->NIP : "",
                tak->PelnaNazwa != NULL ? tak->PelnaNazwa : "",
                tak->Email != NULL ? tak->Email: "",
                sell,
                purchase);

    // Pozostaje wycziścić plik konfiguracyjny
    // i zapisać nową zawartość
    FILE *fp;
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *filename;
    asprintf(&filename, "%s/.pltak/config", homedir);

    fp = fopen(filename, "w");
    if (fp != NULL) {
        fputs(config, fp);
        fclose(fp);
    }
}

void printTakCols(JPKColumns* data) {
    while (data != NULL) {
        printf(" \t%s\n", data->title);
        data = data->next;
    }
}

void printTakConfig(TakConfig* takData) {
//    assert(takData->sellColumns != NULL);
    printf("Konfiguracja: %s,\n"
            "\t%s,\n"
            "\t%s,\n",
            takData->NIP,
            takData->PelnaNazwa,
            takData->Email);
    printTakCols(takData->sellColumns);
    printTakCols(takData->purchaseColumns);
}

TakConfig* getConfig(JPK* jpk) {
    TakConfig* config = parseConfig();
    config->NIP = jpk->profile->nip;
    config->PelnaNazwa = jpk->profile->pelnaNazwa;
    config->Email = jpk->profile->email;
    return config;
}

JPK* configToJPK(JPK* jpk, TakConfig* tak) {
    jpk->profile->nip = tak->NIP;
    jpk->profile->pelnaNazwa = tak->PelnaNazwa;
    jpk->profile->email = tak->Email;
    return jpk;
}
