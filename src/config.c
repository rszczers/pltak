#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "jpk.h"

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
    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char *token;
    while ((read = getline(&line, &len, fp)) != -1) {
        token = strtok(line, "=\n");
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
            token = strtok(NULL, ":\n");
            while (token != NULL) {
                addColumn(config->sellColumns, token);
                printf("token: %s\n", token);
                token = strtok(NULL, ":\n");
            }
        } else if (strcmp(token, "purchaseColumns") == 0) {
            token = strtok(NULL, ":\n");
            while (token != NULL) {
                addColumn(config->purchaseColumns, token);
                token = strtok(NULL, ":\n");
            }
        }
    }
    return config;
}

char* printTakCols(JPKColumns* data) {
    char* out = (char*)malloc(sizeof(char)*4096);
    sprintf(out, "");
    char* buffer;
    while (data != NULL) {
        asprintf(&buffer, "%s", data->title);
        printf("buffer=%s\n", buffer);
        strcat(out, buffer);
        data = data->next;
    }
    return out;
}

void printTakConfig(TakConfig* takData) {
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
            takData->Poczta,
            printTakCols(takData->sellColumns),
            printTakCols(takData->purchaseColumns));
}
