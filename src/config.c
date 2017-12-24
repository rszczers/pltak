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
    char* DomyslnyKodWaluty;
    char* KodUrzedu;
    char* NIP;
    char* PelnaNazwa;
    char* REGON;
    char* KodKraju;
    char* Wojewodztwo;
    char* Powiat;
    char* Gmina;
    char* Ulica;
    char* NrDomu;
    char* NrLokalu;
    char* Miejscowosc;
    char* KodPocztowy;
    char* Poczta;
    JPKColumns* sellColumns;
    JPKColumns* purchaseColumns;
} TakConfig;

TakConfig* parseConfig() {
    TakConfig* config = (TakConfig*)malloc(sizeof(TakConfig));
    config->purchaseColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
    config->sellColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
    config->purchaseColumns->next = NULL;
    config->sellColumns->next = NULL;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *filename;
    asprintf(&filename, "%s/.pltak/config", homedir);

    if (access(filename, R_OK) != -1) {
        fp = fopen(filename, "r");
        char *token;
        while ((read = getline(&line, &len, fp)) != -1) {
            token = strtok(line, "=\n");
            if (token != NULL) {
                if (strcmp(token, "DomyslnyKodWaluty") == 0) {
                    asprintf(&(config->DomyslnyKodWaluty), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "KodUrzedu") == 0) {
                    asprintf(&(config->KodUrzedu), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "NIP") == 0) {
                    asprintf(&(config->NIP), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "PelnaNazwa") == 0) {
                    asprintf(&(config->PelnaNazwa), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "REGON") == 0) {
                    asprintf(&(config->REGON), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "KodKraju") == 0) {
                    asprintf(&(config->KodKraju), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Wojewodztwo") == 0) {
                    asprintf(&(config->Wojewodztwo), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Powiat") == 0) {
                    asprintf(&(config->Powiat), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Gmina") == 0) {
                    asprintf(&(config->Gmina), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Ulica") == 0) {
                    asprintf(&(config->Ulica), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "NrDomu") == 0) {
                    asprintf(&(config->NrDomu), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "NrLokalu") == 0) {
                    asprintf(&(config->NrLokalu), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Miejscowosc") == 0) {
                    asprintf(&(config->Miejscowosc), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "KodPocztowy") == 0) {
                    asprintf(&(config->KodPocztowy), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "Poczta") == 0) {
                    asprintf(&(config->Poczta), "%s", strtok(NULL, "\n"));
                } else if (strcmp(token, "sellColumns") == 0) {
                    token = strtok(NULL, "=");
                    char *buffer;
                    asprintf(&buffer, "%s", token);
                    buffer = strtok(token, ":\n");
                    while (buffer != NULL) {
                        char* buffer_2;
                        asprintf(&buffer_2, "%s", buffer);
                        addColumn(config->sellColumns, buffer_2);
                        buffer = strtok(NULL, ":\n");
                    }
                } else if (strcmp(token, "purchaseColumns") == 0) {
                    token = strtok(NULL, "=");
                    char *buffer;
                    asprintf(&buffer, "%s", token);
                    buffer = strtok(token, ":\n");
                    while (buffer != NULL) {
                        char* buffer_2;
                        asprintf(&buffer_2, "%s", buffer);
                        addColumn(config->purchaseColumns, buffer_2);
                        buffer = strtok(NULL, ":\n");
                    }
                }
            }
        }
    } else { // stwórz domyślną konfigurację
        fp = fopen(filename, "ab");
        char* defaultConfig = "DomyslnyKodWaluty=PLN\n"
                              "KodUrzedu=\n"
                              "NIP=\n"
                              "PelnaNazwa=\n"
                              "REGON=\n"
                              "KodKraju=\n"
                              "Wojewodztwo=\n"
                              "Powiat=\n"
                              "Gmina=\n"
                              "Ulica=\n"
                              "NrDomu=\n"
                              "NrLokalu=\n"
                              "Miejscowosc=\n"
                              "KodPocztowy=\n"
                              "Poczta=\n"
                              "sellColumns=\n"
                              "purchaseColumns=";

        if (fp != NULL) {
            fputs(defaultConfig, fp);
            fclose(fp);
        }
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
            purchase = token;
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
        if (oldLength == 0) {
            sell = token;
        } else {
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

    asprintf(&config, "DomyslnyKodWaluty=%s\n"
                      "KodUrzedu=%s\n"
                      "NIP=%s\n"
                      "PelnaNazwa=%s\n"
                      "REGON=%s\n"
                      "KodKraju=%s\n"
                      "Wojewodztwo=%s\n"
                      "Powiat=%s\n"
                      "Gmina=%s\n"
                      "Ulica=%s\n"
                      "NrDomu=%s\n"
                      "NrLokalu=%s\n"
                      "Miejscowosc=%s\n"
                      "KodPocztowy=%s\n"
                      "Poczta=%s\n"
                      "sellColumns=%s\n"
                      "purchaseColumns=%s",
                    tak->DomyslnyKodWaluty,
                    tak->KodUrzedu,
                    tak->NIP,
                    tak->PelnaNazwa,
                    tak->REGON,
                    tak->KodKraju,
                    tak->Wojewodztwo,
                    tak->Powiat,
                    tak->Gmina,
                    tak->Ulica,
                    tak->NrDomu,
                    tak->NrLokalu,
                    tak->Miejscowosc,
                    tak->KodPocztowy,
                    tak->Poczta,
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
        printf("\t%s\n", data->title);
        data = data->next;
    }
}

void printTakConfig(TakConfig* takData) {
    assert(takData->sellColumns != NULL);
    printf("Konfiguracja: %s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n"
            "\t%s,\n",
            takData->DomyslnyKodWaluty,
            takData->KodUrzedu,
            takData->NIP,
            takData->PelnaNazwa,
            takData->REGON,
            takData->KodKraju,
            takData->Wojewodztwo,
            takData->Powiat,
            takData->Gmina,
            takData->Ulica,
            takData->NrDomu,
            takData->NrLokalu,
            takData->Miejscowosc,
            takData->KodPocztowy,
            takData->Poczta);
      printTakCols(takData->sellColumns);
      printTakCols(takData->purchaseColumns);
}
