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

TakConfig* getConfig(JPK* jpk) {
    TakConfig* config = (TakConfig*)malloc(sizeof(TakConfig));
    config->DomyslnyKodWaluty = jpk->header->domyslnyKodWaluty;
    config->Gmina = jpk->profile->gmina;
    config->KodKraju = jpk->profile->kodKraju;
    config->KodPocztowy = jpk->profile->kodPocztowy;
    config->KodUrzedu = jpk->header->kodUrzedu;
    config->Miejscowosc = jpk->profile->miejscowosc;
    config->NrDomu = jpk->profile->nrDomu;
    config->NrLokalu = jpk->profile->nrLokalu;
    config->NIP = jpk->profile->nip;
    config->PelnaNazwa = jpk->profile->pelnaNazwa;
    config->Poczta = jpk->profile->poczta;
    config->Powiat = jpk->profile->powiat;
    config->REGON = jpk->profile->regon;
    config->Ulica = jpk->profile->ulica;
    config->Wojewodztwo = jpk->profile->wojewodztwo;
    return config;
}

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
                if (strcmp(token, "DomyslnyKodWaluty") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->DomyslnyKodWaluty), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "KodUrzedu") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->KodUrzedu), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "NIP") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->NIP), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "PelnaNazwa") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->PelnaNazwa), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "REGON") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->REGON), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "KodKraju") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->KodKraju), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Wojewodztwo") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Wojewodztwo), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Powiat") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Powiat), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Gmina") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Gmina), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Ulica") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Ulica), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "NrDomu") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->NrDomu), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "NrLokalu") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->NrLokalu), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Miejscowosc") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Miejscowosc), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "KodPocztowy") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->KodPocztowy), "%s",
                               cur == NULL ? "" : cur);
                } else if (strcmp(token, "Poczta") == 0) {
                    char *cur = strtok(NULL, "\n");
                    if (cur != NULL)
                        asprintf(&(config->Poczta), "%s",
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
        config->DomyslnyKodWaluty = "PLN";
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
                tak->DomyslnyKodWaluty != NULL ? tak->DomyslnyKodWaluty : "",
                tak->KodUrzedu != NULL ? tak->KodUrzedu : "",
                tak->NIP != NULL ? tak->NIP : "",
                tak->PelnaNazwa != NULL ? tak->PelnaNazwa : "",
                tak->REGON != NULL ? tak->REGON : "",
                tak->KodKraju != NULL ? tak->KodKraju : "",
                tak->Wojewodztwo != NULL ? tak->Wojewodztwo : "",
                tak->Powiat != NULL ? tak->Powiat : "",
                tak->Gmina != NULL ? tak->Gmina : "",
                tak->Ulica != NULL ? tak->Ulica : "",
                tak->NrDomu != NULL ? tak->NrDomu : "",
                tak->NrLokalu != NULL ? tak->NrLokalu : "",
                tak->Miejscowosc != NULL ? tak->Miejscowosc : "",
                tak->KodPocztowy != NULL ? tak->KodPocztowy : "",
                tak->Poczta != NULL ? tak->Poczta : "",
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

