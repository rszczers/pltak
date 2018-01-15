#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <assert.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <unistd.h>
#include "parse.h"
#include "utils.h"


typedef enum {
    COLUMN_NAME = 1,
    HEADER,
    PROFILE_1,
    PROFILE_2,
    SELLS,
} JPKRow;

int PURCHASES;

typedef enum {
    KODFORMULARZA = 1,
    KODSYSTEMOWY,
    WERSJASCHEMY,
    WARIANTFORMULARZA,
    CELZLOZENIA,
    DATAWYTWORZENIAJPK,
    DATAOD,
    DATADO,
    DOMYSLNYKODWALUTY,
    KODURZEDU,
    NIP,
    PELNANAZWA,
    REGON,
    KODKRAJU,
    WOJEWODZTWO,
    POWIAT,
    GMINA,
    ULICA,
    NRDOMU,
    NRLOKALU,
    MIEJSCOWOSC,
    KODPOCZTOWY,
    POCZTA,
    TYPSPRZEDAZY,
    LPSPRZEDAZY,
    NRKONTRAHENTA,
    NAZWAKONTRAHENTA,
    ADRESKONTRAHENTA,
    DOWODSPRZEDAZY,
    DATAWYSTAWIENIA,
    DATASPRZEDAZY,
    K_10,
    K_11,
    K_12,
    K_13,
    K_14,
    K_15,
    K_16,
    K_17,
    K_18,
    K_19,
    K_20,
    K_21,
    K_22,
    K_23,
    K_24,
    K_25,
    K_26,
    K_27,
    K_28,
    K_29,
    K_30,
    K_31,
    K_32,
    K_33,
    K_34,
    K_35,
    K_36,
    K_37,
    K_38,
    K_39,
    LICZBAWIERSZYSPRZEDAZY,
    PODATEKNALEZNY,
    TYPZAKUPU,
    LPZAKUPU,
    NRDOSTAWCY,
    NAZWADOSTAWCY,
    ADRESDOSTAWCY,
    DOWODZAKUPU,
    DATAZAKUPU,
    DATAWPLYWU,
    K_43,
    K_44,
    K_45,
    K_46,
    K_47,
    K_48,
    K_49,
    K_50,
    LICZBAWIERSZYZAKUPOW,
    PODATEKNALICZONY
} JPKCol;

typedef struct sHeader {
    char* kodFormularza;
    char* kodSystemowy;
    char* wersjaSchemy;
    int wariantFormularza;
    int celZlozenia;
    char* dataWytworzeniaJPK;
    char* dataOd;
    char* dataDo;
    char* domyslnyKodWaluty;
    char* kodUrzedu;
} JPKHeader;

typedef struct sProfile {
    char* nip;
    char* pelnaNazwa;
    char* regon;
    char* kodKraju;
    char* wojewodztwo;
    char* powiat;
    char* gmina;
    char* ulica;
    char* nrDomu;
    char* nrLokalu;
    char* miejscowosc;
    char* kodPocztowy;
    char* poczta;
} JPKProfile;

typedef struct sSold {
    char* typSprzedazy;
    unsigned int lpSprzedazy;
    char* nrKontrahenta;
    char* nazwaKontrahenta;
    char* adresKontrahenta;
    char* dowodSprzedazy;
    char* dataWystawienia;
    char* dataSprzedazy;
    double k_10;
    double k_11;
    double k_12;
    double k_13;
    double k_14;
    double k_15;
    double k_16;
    double k_17;
    double k_18;
    double k_19;
    double k_20;
    double k_21;
    double k_22;
    double k_23;
    double k_24;
    double k_25;
    double k_26;
    double k_27;
    double k_28;
    double k_29;
    double k_30;
    double k_31;
    double k_32;
    double k_33;
    double k_34;
    double k_35;
    double k_36;
    double k_37;
    double k_38;
    double k_39;
} JPKSold;

typedef struct sPurchase {
    char* typZakupu;
    unsigned int lpZakupu;
    char* nrDostawcy;
    char* nazwaDostawcy;
    char* adresDostawcy;
    char* dowodZakupu;
    char* dataZakupu;
    char* dataWplywu;
    double k_43;
    double k_44;
    double k_45;
    double k_46;
    double k_47;
    double k_48;
    double k_49;
    double k_50;
} JPKPurchase;

typedef struct SoldNode {
    JPKSold* val;
    struct SoldNode *next;
} JPKSoldList;

void addSold(JPKSoldList* list, JPKSold* row) {
    while(list->next != NULL) {
        list = list->next;
    }
    if (list->val != NULL) {
        JPKSoldList* newData = (JPKSoldList*)malloc(sizeof(JPKSoldList));
        newData->val = row;
        newData->next = NULL;
        list->next = newData;
    } else {
        list->val = row;
    }
}

typedef struct PurchaseNode {
    JPKPurchase* val;
    struct PurchaseNode *next;
} JPKPurchaseList;

void addPurchase(JPKPurchaseList* list, JPKPurchase* row) {
    while(list->next != NULL) {
        list = list->next;
    }
    if (list->val != NULL) {
        JPKPurchaseList* newData = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
        newData->val = row;
        newData->next = NULL;
        list->next = newData;
    } else {
        list->val = row;
    }
}

typedef struct ColNode {
    char* title;
    struct ColNode *next;
} JPKColumns;

void addColumn(JPKColumns* col, char* title) {
    if (col->title == NULL && col->next == NULL) {
        col->title = title;
        col->next = NULL;
    } else if (col->next == NULL) {
        JPKColumns* newNode = (JPKColumns*)malloc(sizeof(JPKColumns));
        newNode->title = title;
        newNode->next = NULL;
        col->next = newNode;
    } else {
        addColumn(col->next, title);
    }
}

void rmColumn(JPKColumns** col, char* title) {
    JPKColumns* prev = (*col);
    while (*col != NULL) {
        if (strcmp((*col)->title, title) == 0) {
            // Jeśli to czoło listy
            if ((*col) == prev) (*col) = (*col)->next;
            else if ((*col)->next != NULL) { // Jeśli są następniki
                prev->next = (*col)->next;
            } else { // Jeśli element jest ostatni
                // Jeśli to czoło listy
                prev->next = NULL;
                *col = NULL;
            }
            return;
        }
        prev = *col;
        col = &((*col)->next);
    }
}

int isElem(JPKColumns* col, char* title) {
    while (col != NULL) {
        if(strcmp(col->title, title) == 0)
            return 1;
        col = col->next;
    }
    return 0;
}

typedef struct {
    JPKColumns* colNames;
    int colsCount;
    JPKHeader* header;
    JPKProfile* profile;
    JPKSoldList* sold;
    int soldCount;
    double soldTotal;
    JPKPurchaseList* purchase;
    int purchaseCount;
    double purchaseTotal;
} JPK;

void printPurchases(JPK*);

JPKColumns* getColumns(tData* data) {
    JPKColumns* cols = (JPKColumns*)malloc(sizeof(JPKColumns));
    cols->next = NULL;
    cols->title = NULL;
    tToken* titles = data->row;
    while (titles != NULL) {
        addColumn(cols, titles->val);
        titles = titles->next;
    }
    return cols;
}

JPKHeader* getHeader(tData* data) {
    JPKHeader* header = (JPKHeader*)malloc(sizeof(JPKHeader));
    header->kodFormularza = getCell(data, HEADER, KODFORMULARZA);
    header->kodSystemowy = getCell(data, HEADER, KODSYSTEMOWY);
    header->wersjaSchemy = getCell(data, HEADER, WERSJASCHEMY);
    header->wariantFormularza = strtol(getCell(data, HEADER, WARIANTFORMULARZA), NULL, 10);
    header->celZlozenia = strtol(getCell(data, HEADER, CELZLOZENIA), NULL, 10);
    header->dataWytworzeniaJPK = getCell(data, HEADER, DATAWYTWORZENIAJPK);
    header->dataOd = getCell(data, HEADER, DATAOD);
    header->dataDo = getCell(data, HEADER, DATADO);
    header->domyslnyKodWaluty = getCell(data, HEADER, DOMYSLNYKODWALUTY);
    header->kodUrzedu = getCell(data, HEADER, KODURZEDU);
    return header;
}

JPKProfile* getProfile(tData* data) {
    JPKProfile* profile = (JPKProfile*)malloc(sizeof(JPKProfile));
    profile->nip = getCell(data, PROFILE_1, NIP);
    profile->pelnaNazwa = getCell(data, PROFILE_1, PELNANAZWA);
    profile->regon = getCell(data, PROFILE_1, REGON);
    profile->kodKraju = getCell(data, PROFILE_2, KODKRAJU);
    profile->wojewodztwo = getCell(data, PROFILE_2, WOJEWODZTWO);
    profile->powiat = getCell(data, PROFILE_2, POWIAT);
    profile->gmina = getCell(data, PROFILE_2, GMINA);
    profile->ulica = getCell(data, PROFILE_2, ULICA);
    profile->nrDomu = getCell(data, PROFILE_2, NRDOMU);
    profile->nrLokalu = getCell(data, PROFILE_2, NRLOKALU);
    profile->miejscowosc = getCell(data, PROFILE_2, MIEJSCOWOSC);
    profile->kodPocztowy = getCell(data, PROFILE_2, KODPOCZTOWY);
    profile->poczta = getCell(data, PROFILE_2, POCZTA);
    return profile;
}

double m2d(char* cell) {
    char *buffer = (char*)malloc(strlen(cell) + 1);
    buffer = strdup(cell);
    bool frac = false;

    // Zamień przecinki z notacji europejskiej na kropki
    for (char *p = buffer; *p != '\0'; ++p) {
        if (*p == ',') {
            frac = true;
        }
    }

    // Wyzeruj wszystkie cyfry od drugiego miejsca dziesiętnego
    if (frac) {
        char *p = &buffer[0];
        while (*p != ',')
            p++;
        p = p + 3;
        *p = '\0';
    }
    return atof(buffer);
}


char* sell_d2m(JPK* data, int i, int j) {
    char *out = "";
    if (data->sold != NULL) {
        if (i == 0) {
            JPKColumns* category = data->colNames;
            for (int k = 0; k < j; ++k) {
                category = category->next;
            }
            out = category->title;
        } else {
            JPKSoldList* row = data->sold;
            for (int k = 0; k < i-1; ++k) {
                row = row->next;
            }
            JPKSold* col = row->val;

            switch(j) {
                case TYPSPRZEDAZY:
                    asprintf(&out, "%s", col->typSprzedazy);
                break;
                case NRKONTRAHENTA:
                    asprintf(&out, "%s", col->nrKontrahenta);
                break;
                case NAZWAKONTRAHENTA:
                    asprintf(&out, "%s", col->nazwaKontrahenta);
                break;
                case ADRESKONTRAHENTA:
                    asprintf(&out, "%s", col->adresKontrahenta);
                break;
                case DOWODSPRZEDAZY:
                    asprintf(&out, "%s", col->dowodSprzedazy);
                break;
                case DATAWYSTAWIENIA:
                    asprintf(&out, "%s", col->dataWystawienia);
                break;
                case DATASPRZEDAZY:
                    asprintf(&out, "%s", col->dataSprzedazy);
                break;
                case LPSPRZEDAZY:
                    asprintf(&out, "%d", col->lpSprzedazy);
                break;
                case K_10:
                    asprintf(&out, "%.2lf", col->k_10);
                break;
                case K_11:
                    asprintf(&out, "%.2lf", col->k_11);
                break;
                case K_12:
                    asprintf(&out, "%.2lf", col->k_12);
                break;
                case K_13:
                    asprintf(&out, "%.2lf", col->k_13);
                break;
                case K_14:
                    asprintf(&out, "%.2lf", col->k_14);
                break;
                case K_15:
                    asprintf(&out, "%.2lf", col->k_15);
                break;
                case K_16:
                    asprintf(&out, "%.2lf", col->k_16);
                break;
                case K_17:
                    asprintf(&out, "%.2lf", col->k_17);
                break;
                case K_18:
                    asprintf(&out, "%.2lf", col->k_18);
                break;
                case K_19:
                    asprintf(&out, "%.2lf", col->k_19);
                break;
                case K_20:
                    asprintf(&out, "%.2lf", col->k_20);
                break;
                case K_21:
                    asprintf(&out, "%.2lf", col->k_21);
                break;
                case K_22:
                    asprintf(&out, "%.2lf", col->k_22);
                break;
                case K_23:
                    asprintf(&out, "%.2lf", col->k_23);
                break;
                case K_24:
                    asprintf(&out, "%.2lf", col->k_24);
                break;
                case K_25:
                    asprintf(&out, "%.2lf", col->k_25);
                break;
                case K_26:
                    asprintf(&out, "%.2lf", col->k_26);
                break;
                case K_27:
                    asprintf(&out, "%.2lf", col->k_27);
                break;
                case K_28:
                    asprintf(&out, "%.2lf", col->k_28);
                break;
                case K_29:
                    asprintf(&out, "%.2lf", col->k_29);
                break;
                case K_30:
                    asprintf(&out, "%.2lf", col->k_30);
                break;
                case K_31:
                    asprintf(&out, "%.2lf", col->k_31);
                break;
                case K_32:
                    asprintf(&out, "%.2lf", col->k_32);
                break;
                case K_33:
                    asprintf(&out, "%.2lf", col->k_33);
                break;
                case K_34:
                    asprintf(&out, "%.2lf", col->k_34);
                break;
                case K_35:
                    asprintf(&out, "%.2lf", col->k_35);
                break;
                case K_36:
                    asprintf(&out, "%.2lf", col->k_36);
                break;
                case K_37:
                    asprintf(&out, "%.2lf", col->k_37);
                break;
                case K_38:
                    asprintf(&out, "%.2lf", col->k_38);
                break;
                case K_39:
                    asprintf(&out, "%.2lf", col->k_39);
                break;
            }
        }
    }
    return out;
}

char* pur_d2m(JPK* data, int i, int j) {
    char *out = "";
    if (data->purchase != NULL) {
        if (i == 0) {
            JPKColumns* category = data->colNames;
            for (int k = 0; k < j; ++k) {
                category = category->next;
            }
            out = category->title;
        } else {
            JPKPurchaseList* row = data->purchase;
            for (int k = 0; k < i-1; ++k) {
                row = row->next;
            }
            JPKPurchase* col = row->val;

            switch(j) {
                case TYPZAKUPU:
                    asprintf(&out, "%s", col->typZakupu);  //char*
                    break;
                case LPZAKUPU:
                    asprintf(&out, "%d", col->lpZakupu); //unsigned int
                    break;
                case NRDOSTAWCY:
                    asprintf(&out, "%s", col->nrDostawcy); //char*
                    break;
                case NAZWADOSTAWCY:
                    asprintf(&out, "%s", col->nazwaDostawcy); //char*
                    break;
                case ADRESDOSTAWCY:
                    asprintf(&out, "%s", col->adresDostawcy); //char*
                    break;
                case DOWODZAKUPU:
                    asprintf(&out, "%s", col->dowodZakupu); //char*
                    break;
                case DATAZAKUPU:
                    asprintf(&out, "%s", col->dataZakupu); //char*
                    break;
                case DATAWPLYWU:
                    asprintf(&out, "%s", col->dataWplywu); //char*
                    break;
                case K_43:
                    asprintf(&out, "%.2lf", col->k_43); //double
                    break;
                case K_44:
                    asprintf(&out, "%.2lf", col->k_44); //double
                    break;
                case K_45:
                    asprintf(&out, "%.2lf", col->k_45); //double
                    break;
                case K_46:
                    asprintf(&out, "%.2lf", col->k_46); //double
                    break;
                case K_47:
                    asprintf(&out, "%.2lf", col->k_47); //double
                    break;
                case K_48:
                    asprintf(&out, "%.2lf", col->k_48); //double
                    break;
                case K_49:
                    asprintf(&out, "%.2lf", col->k_49); //double
                    break;
                case K_50:
                    asprintf(&out, "%.2lf", col->k_50); //double
                    break;
            }
        }
    }
    return out;
}


/* Uzupełnia strukturę sprzedaży
 * row - numer wiersza sprzedaży do wczytania do struktury
 */
JPKSold* rowToSold(tData* data, int row) {
    JPKSold* raport = (JPKSold*)malloc(sizeof(JPKSold));
    raport->typSprzedazy = getCell(data, row, TYPSPRZEDAZY);
    raport->lpSprzedazy = strtol(getCell(data, row, LPSPRZEDAZY), NULL, 10);
    raport->nrKontrahenta = getCell(data, row, NRKONTRAHENTA);
    raport->nazwaKontrahenta = getCell(data, row, NAZWAKONTRAHENTA);
    raport->adresKontrahenta = getCell(data, row, ADRESKONTRAHENTA);
    raport->dowodSprzedazy = getCell(data, row, DOWODSPRZEDAZY);
    raport->dataWystawienia = getCell(data, row, DATAWYSTAWIENIA);
    raport->dataSprzedazy = getCell(data, row, DATASPRZEDAZY);
    raport->k_10 = m2d(getCell(data, row, K_10));
    raport->k_11 = m2d(getCell(data, row, K_11));
    raport->k_12 = m2d(getCell(data, row, K_12));
    raport->k_13 = m2d(getCell(data, row, K_13));
    raport->k_13 = m2d(getCell(data, row, K_13));
    raport->k_14 = m2d(getCell(data, row, K_14));
    raport->k_15 = m2d(getCell(data, row, K_15));
    raport->k_16 = m2d(getCell(data, row, K_16));
    raport->k_17 = m2d(getCell(data, row, K_17));
    raport->k_18 = m2d(getCell(data, row, K_18));
    raport->k_19 = m2d(getCell(data, row, K_19));
    raport->k_20 = m2d(getCell(data, row, K_20));
    raport->k_21 = m2d(getCell(data, row, K_21));
    raport->k_22 = m2d(getCell(data, row, K_22));
    raport->k_23 = m2d(getCell(data, row, K_23));
    raport->k_24 = m2d(getCell(data, row, K_24));
    raport->k_25 = m2d(getCell(data, row, K_25));
    raport->k_26 = m2d(getCell(data, row, K_26));
    raport->k_27 = m2d(getCell(data, row, K_27));
    raport->k_28 = m2d(getCell(data, row, K_28));
    raport->k_29 = m2d(getCell(data, row, K_29));
    raport->k_30 = m2d(getCell(data, row, K_30));
    raport->k_31 = m2d(getCell(data, row, K_31));
    raport->k_32 = m2d(getCell(data, row, K_32));
    raport->k_33 = m2d(getCell(data, row, K_33));
    raport->k_34 = m2d(getCell(data, row, K_34));
    raport->k_35 = m2d(getCell(data, row, K_35));
    raport->k_36 = m2d(getCell(data, row, K_36));
    raport->k_37 = m2d(getCell(data, row, K_37));
    raport->k_38 = m2d(getCell(data, row, K_38));
    raport->k_39 = m2d(getCell(data, row, K_39));
    return raport;
}

/* Uzupenia strukturę zakupów
 * row – numer wiersza pole zakupów do wczytania do struktury
 */
JPKPurchase* rowToPurchase(tData* data, int row) {
    JPKPurchase* raport = (JPKPurchase*)malloc(sizeof(JPKPurchase));
    raport->typZakupu = getCell(data, row, TYPZAKUPU);
    raport->lpZakupu = strtol(getCell(data, row, LPZAKUPU), NULL, 10);
    raport->nrDostawcy = getCell(data, row, NRDOSTAWCY);
    raport->nazwaDostawcy = getCell(data, row, NAZWADOSTAWCY);
    raport->adresDostawcy = getCell(data, row, ADRESDOSTAWCY);
    raport->dowodZakupu = getCell(data, row, DOWODZAKUPU);
    raport->dataZakupu = getCell(data, row, DATAZAKUPU);
    raport->dataWplywu = getCell(data, row, DATAWPLYWU);
    raport->k_43 = m2d(getCell(data, row, K_43));
    raport->k_44 = m2d(getCell(data, row, K_44));
    raport->k_45 = m2d(getCell(data, row, K_45));
    raport->k_46 = m2d(getCell(data, row, K_46));
    raport->k_47 = m2d(getCell(data, row, K_47));
    raport->k_48 = m2d(getCell(data, row, K_48));
    raport->k_49 = m2d(getCell(data, row, K_49));
    raport->k_50 = m2d(getCell(data, row, K_50));
    return raport;
}

int countSells(tData* data) {
    int c = 0;
    tData* cur = data;
    for (int i = 0; i < SELLS - 1; ++i) {
        cur = cur->next;
    }
    while (cur != NULL && cur->row->colNum == TYPSPRZEDAZY) {
        c++;
        cur = cur->next;
    }
    return c;
}

int countPurchases(tData* data) {
    int c = 0;
    tData* cur = data;
    // Przewiń do 16-tego wiersza
    for (int i = 0; i < PURCHASES - 1; ++i) {
        cur = cur->next;
    }
    while (cur != NULL && cur->row->colNum == TYPZAKUPU) {
        c++;
        cur = cur->next;
    }
    return c;
}

JPKSoldList* getSoldList(tData* parsedData, int soldCount) {
    JPKSoldList* sells = (JPKSoldList*)malloc(sizeof(JPKSoldList));
    sells->val = NULL;
    sells->next = NULL;
    for (int i = SELLS; i < SELLS + soldCount; ++i) {
        addSold(sells, rowToSold(parsedData, i));
    }
    return sells;
}

JPKPurchaseList* getPurchaseList(tData* parsedData, int purchaseCount) {
    JPKPurchaseList* purchases = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
    purchases->val = NULL;
    purchases->next = NULL;
    for (int i = PURCHASES; i < PURCHASES + purchaseCount ; ++i) {
        addPurchase(purchases, rowToPurchase(parsedData, i));
    }
    return purchases;
}

double evalTotalSold(JPKSoldList* jpk) {
    double eval = 0.0;
    while (jpk != NULL) {
        eval += jpk->val->k_16 +
            jpk->val->k_18 + jpk->val->k_20 + jpk->val->k_24 + jpk->val->k_26 +
            jpk->val->k_28 + jpk->val->k_30 + jpk->val->k_33 + jpk->val->k_35 +
            jpk->val->k_36 + jpk->val->k_37 - jpk->val->k_38 - jpk->val->k_39;
        jpk = jpk->next;
    }
    return eval;
}

double evalTotalPurchase(JPKPurchaseList* jpk) {
    double eval = 0.0;
    while (jpk != NULL) {
        eval += jpk->val->k_44 + jpk->val->k_46 + jpk->val->k_47 +
            jpk->val->k_48 + jpk->val->k_49 + jpk->val->k_50;
        jpk = jpk->next;
    }
    return eval;
}

int countCols(JPK* jpk) {
    int c = 0;
    JPKColumns* cur = jpk->colNames;
    while(cur != NULL) {
        cur = cur->next;
        c++;
    }
    return c;
}

JPK* loadJPK(char *filename) {
    tData* parsedData = parse(filename);
    JPK* data = (JPK*)malloc(sizeof(JPK));

    data->colNames = getColumns(parsedData);
    data->colsCount = countCols(data); //ta linia musi być wykonana po getColumns
    data->header = getHeader(parsedData);
    data->profile = getProfile(parsedData);

    // Te dwa musza być zdefiniowane wcześniej, bo
    // pozostałe funkcje uzywają tych pól
    data->soldCount = countSells(parsedData);
    PURCHASES = SELLS + data->soldCount + 2;
    data->purchaseCount = countPurchases(parsedData);
    data->sold = getSoldList(parsedData, data->soldCount);
    data->soldTotal = evalTotalSold(data->sold);
    data->purchase = getPurchaseList(parsedData, data->purchaseCount);
    data->purchaseTotal = evalTotalPurchase(data->purchase);
    return data;
}

void printSold(JPK* jpk) {
    JPKSoldList* row = jpk->sold;
    int i = SELLS;
    while (row != NULL) {
        printf("%d) \t typSprzedazy: %s, \n"
                "\t lpSprzedazy: %d, \n"
                "\t nrKontrahenta: %s, \n"
                "\t nazwaKontrahenta: %s, \n"
                "\t adresKontrahenta: %s, \n"
                "\t dowodSprzedazy: %s, \n"
                "\t dataWystawienia: %s, \n"
                "\t dataSprzedazy: %s, \n"
                "\t k_10: %.2lf, \n"
                "\t k_11: %.2lf, \n"
                "\t k_12: %.2lf, \n"
                "\t k_13: %.2lf, \n"
                "\t k_14: %.2lf, \n"
                "\t k_15: %.2lf, \n"
                "\t k_16: %.2lf, \n"
                "\t k_17: %.2lf, \n"
                "\t k_18: %.2lf, \n"
                "\t k_19: %.2lf, \n"
                "\t k_20: %.2lf, \n"
                "\t k_21: %.2lf, \n"
                "\t k_22: %.2lf, \n"
                "\t k_23: %.2lf, \n"
                "\t k_24: %.2lf, \n"
                "\t k_25: %.2lf, \n"
                "\t k_26: %.2lf, \n"
                "\t k_27: %.2lf, \n"
                "\t k_28: %.2lf, \n"
                "\t k_29: %.2lf, \n"
                "\t k_30: %.2lf, \n"
                "\t k_31: %.2lf, \n"
                "\t k_32: %.2lf, \n"
                "\t k_33: %.2lf, \n"
                "\t k_34: %.2lf, \n"
                "\t k_35: %.2lf, \n"
                "\t k_36: %.2lf, \n"
                "\t k_37: %.2lf, \n"
                "\t k_38: %.2lf, \n"
                "\t k_39: %.2lf, \n"
                "\n",
                i,
                row->val->typSprzedazy,
                row->val->lpSprzedazy,
                row->val->nrKontrahenta,
                row->val->nazwaKontrahenta,
                row->val->adresKontrahenta,
                row->val->dowodSprzedazy,
                row->val->dataWystawienia,
                row->val->dataSprzedazy,
                row->val->k_10,
                row->val->k_11,
                row->val->k_12,
                row->val->k_13,
                row->val->k_14,
                row->val->k_15,
                row->val->k_16,
                row->val->k_17,
                row->val->k_18,
                row->val->k_19,
                row->val->k_20,
                row->val->k_21,
                row->val->k_22,
                row->val->k_23,
                row->val->k_24,
                row->val->k_25,
                row->val->k_26,
                row->val->k_27,
                row->val->k_28,
                row->val->k_29,
                row->val->k_30,
                row->val->k_31,
                row->val->k_32,
                row->val->k_33,
                row->val->k_34,
                row->val->k_35,
                row->val->k_36,
                row->val->k_37,
                row->val->k_38,
                row->val->k_39);
        i++;
        row = row->next;
    }
}

void printPurchases(JPK* jpk) {
    JPKPurchaseList* row = jpk->purchase;
    int i = PURCHASES;
    while (row != NULL) {
        printf("%d) \t typZakupu: %s, \n"
                "\t lpZakupu: %d, \n"
                "\t nrDostawcy: %s, \n"
                "\t nazwaDostawcy: %s, \n"
                "\t adresDostawcy: %s, \n"
                "\t dowodZakupu: %s, \n"
                "\t dataZakupu: %s, \n"
                "\t dataWplywu: %s, \n"
                "\t k_43: %.2lf, \n"
                "\t k_44: %.2lf, \n"
                "\t k_45: %.2lf, \n"
                "\t k_46: %.2lf, \n"
                "\t k_47: %.2lf, \n"
                "\t k_48: %.2lf, \n"
                "\t k_49: %.2lf, \n"
                "\t k_50: %.2lf, \n"
                "\n",
                i,
                row->val->typZakupu,
                row->val->lpZakupu,
                row->val->nrDostawcy,
                row->val->nazwaDostawcy,
                row->val->adresDostawcy,
                row->val->dowodZakupu,
                row->val->dataZakupu,
                row->val->dataWplywu,
                row->val->k_43,
                row->val->k_44,
                row->val->k_45,
                row->val->k_46,
                row->val->k_47,
                row->val->k_48,
                row->val->k_49,
                row->val->k_50);
        i++;
        row = row->next;
    }
}

void addSellRow(JPK* jpk) {
    JPKSoldList* new = (JPKSoldList*)malloc(sizeof(JPKSoldList));
    new->val = (JPKSold*)malloc(sizeof(JPKSold));
    new->next = NULL;
    JPKSoldList* cur = jpk->sold;
    JPKSoldList* prev = NULL;
    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
    }
    Date* data = getDate();
    asprintf(&new->val->dataSprzedazy, "%s-%s-%s", data->year, data->month, data->day);

    asprintf(&new->val->dataWystawienia, "%s-%s-%s", data->year, data->month, data->day);
    new->val->typSprzedazy = "G";
    new->val->adresKontrahenta = "";
    new->val->dowodSprzedazy = "";
    new->val->k_10 = 0.0;
    new->val->k_11 = 0.0;
    new->val->k_12 = 0.0;
    new->val->k_13 = 0.0;
    new->val->k_14 = 0.0;
    new->val->k_15 = 0.0;
    new->val->k_16 = 0.0;
    new->val->k_17 = 0.0;
    new->val->k_18 = 0.0;
    new->val->k_19 = 0.0;
    new->val->k_20 = 0.0;
    new->val->k_21 = 0.0;
    new->val->k_22 = 0.0;
    new->val->k_23 = 0.0;
    new->val->k_24 = 0.0;
    new->val->k_25 = 0.0;
    new->val->k_26 = 0.0;
    new->val->k_27 = 0.0;
    new->val->k_28 = 0.0;
    new->val->k_29 = 0.0;
    new->val->k_30 = 0.0;
    new->val->k_31 = 0.0;
    new->val->k_32 = 0.0;
    new->val->k_33 = 0.0;
    new->val->k_34 = 0.0;
    new->val->k_35 = 0.0;
    new->val->k_36 = 0.0;
    new->val->k_37 = 0.0;
    new->val->k_38 = 0.0;
    new->val->k_39 = 0.0;
    new->val->lpSprzedazy = prev == NULL ? 1 : prev->val->lpSprzedazy + 1;
    new->val->nazwaKontrahenta = "";
    new->val->nrKontrahenta = "";
    if (prev != NULL)
        prev->next = new;
    else {
        JPKSoldList* newSold = (JPKSoldList*)malloc(sizeof(JPKSoldList));
        jpk->sold = newSold;
        jpk->sold->next = NULL;
        jpk->sold->val = new->val;
    }
    jpk->soldCount++;
}

void addPurchaseRow(JPK* jpk) {
    JPKPurchaseList* new = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
    new->val = (JPKPurchase*)malloc(sizeof(JPKPurchase));
    new->next = NULL;
    JPKPurchaseList* cur = jpk->purchase;
    JPKPurchaseList* prev = NULL;
    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
    }
    Date* data = getDate();
    asprintf(&new->val->dataZakupu, "%s-%s-%s", data->year, data->month, data->day);
    asprintf(&new->val->dataWplywu, "%s-%s-%s", data->year, data->month, data->day);
    new->val->typZakupu = "G";
    new->val->nrDostawcy = "";
    new->val->adresDostawcy = "";
    new->val->nazwaDostawcy = "";
    new->val->dowodZakupu = "";
    new->val->k_43 = 0.0;
    new->val->k_44 = 0.0;
    new->val->k_45 = 0.0;
    new->val->k_46 = 0.0;
    new->val->k_47 = 0.0;
    new->val->k_48 = 0.0;
    new->val->k_49 = 0.0;
    new->val->k_50 = 0.0;
    new->val->lpZakupu = prev == NULL ? 1 : prev->val->lpZakupu + 1;
    if (prev != NULL)
        prev->next = new;
    else {
        JPKPurchaseList* newPurch = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
        jpk->purchase = newPurch;
        jpk->purchase->next = NULL;
        jpk->purchase->val = new->val;
    }
    jpk->purchaseCount++;
}

void rmSellRow(JPK* jpk, int row) {
    JPKSoldList* prev = jpk->sold;
    JPKSoldList* cur = jpk->sold;
    for (int i = 1; i < row; ++i) {
         prev = cur;
         cur = cur->next;
    }

    if (cur->next != NULL) {
        if (row == 1) {
            jpk->sold = jpk->sold->next;
        } else {
            prev->next = cur->next;
        }
    } else {
        if (cur == prev) {
            jpk->sold = NULL;
        } else  {
            prev->next = NULL;
        }
    }
    jpk->soldCount = jpk->soldCount - 1;
    jpk->soldTotal = evalTotalSold(jpk->sold);
}

void rmPurchaseRow(JPK* jpk, int row) {
    JPKPurchaseList* prev = jpk->purchase;
    JPKPurchaseList* cur = jpk->purchase;
    for (int i = 1; i < row; ++i) {
         prev = cur;
         cur = cur->next;
    }

    if (cur->next != NULL) {
        if (row == 1) {
            jpk->purchase = jpk->purchase->next;
        } else {
            prev->next = cur->next;
        }
    } else {
        if (cur == prev) {
            jpk->purchase = NULL;
        } else  {
            prev->next = NULL;
        }
    }
    jpk->purchaseCount = jpk->purchaseCount - 1;
    jpk->purchaseTotal = evalTotalPurchase(jpk->purchase);
}

JPK* changeSellData(JPK* data, int i, int j, char* input) {
    char *out = "";
    //przejdz do i-tego wiersza
    JPKSoldList *current = data->sold;
    for (int k = 1; k < i; k++) {
        current = current->next;
    }

    switch(j) {
        case NRKONTRAHENTA:
             asprintf(&current->val->nrKontrahenta, "%s", input);
        break;
        case NAZWAKONTRAHENTA:
            asprintf(&current->val->nazwaKontrahenta, "%s", input);
        break;
        case ADRESKONTRAHENTA:
            asprintf(&current->val->adresKontrahenta, "%s", input);
        break;
        case DOWODSPRZEDAZY:
            asprintf(&current->val->dowodSprzedazy, "%s", input);
        break;
        case DATAWYSTAWIENIA:
            asprintf(&current->val->dataWystawienia, "%s", input);
        break;
        case DATASPRZEDAZY:
            asprintf(&current->val->dataSprzedazy, "%s", input);
        break;
        case LPSPRZEDAZY:
            current->val->lpSprzedazy = atoi(input);
        break;
        case K_10:
            current->val->k_10 = m2d(input);
        break;
        case K_11:
            current->val->k_11 = m2d(input);
        break;
        case K_12:
            current->val->k_12 = m2d(input);
        break;
        case K_13:
            current->val->k_13 = m2d(input);
        break;
        case K_14:
            current->val->k_14 = m2d(input);
        break;
        case K_15:
            current->val->k_15 = m2d(input);
        break;
        case K_16:
            current->val->k_16 = m2d(input);
        break;
        case K_17:
            current->val->k_17 = m2d(input);
        break;
        case K_18:
            current->val->k_18 = m2d(input);
        break;
        case K_19:
            current->val->k_19 = m2d(input);
        break;
        case K_20:
            current->val->k_20 = m2d(input);
        break;
        case K_21:
            current->val->k_21 = m2d(input);
        break;
        case K_22:
            current->val->k_22 = m2d(input);
        break;
        case K_23:
            current->val->k_23 = m2d(input);
        break;
        case K_24:
            current->val->k_24 = m2d(input);
        break;
        case K_25:
            current->val->k_25 = m2d(input);
        break;
        case K_26:
            current->val->k_26 = m2d(input);
        break;
        case K_27:
            current->val->k_27 = m2d(input);
        break;
        case K_28:
            current->val->k_28 = m2d(input);
        break;
        case K_29:
            current->val->k_29 = m2d(input);
        break;
        case K_30:
            current->val->k_30 = m2d(input);
        break;
        case K_31:
            current->val->k_31 = m2d(input);
        break;
        case K_32:
            current->val->k_32 = m2d(input);
        break;
        case K_33:
            current->val->k_33 = m2d(input);
        break;
        case K_34:
            current->val->k_34 = m2d(input);
        break;
        case K_35:
            current->val->k_35 = m2d(input);
        break;
        case K_36:
            current->val->k_36 = m2d(input);
        break;
        case K_37:
            current->val->k_37 = m2d(input);
        break;
        case K_38:
            current->val->k_38 = m2d(input);
        break;
        case K_39:
            current->val->k_39 = m2d(input);
        break;
    }

    data->purchaseTotal = evalTotalPurchase(data->purchase);
    data->soldTotal = evalTotalSold(data->sold);
    return data;
}

JPK* changePurData(JPK* data, int i, int j, char* input) {
    char *out = "";
    //przejdz do i-tego wiersza
    JPKPurchaseList *current = data->purchase;
    for (int k = 1; k < i; k++) {
        current = current->next;
    }

    switch(j) {
        case TYPZAKUPU:
            asprintf(&(current->val->typZakupu), "%s", input);  //char*
            break;
        case LPZAKUPU:
            current->val->lpZakupu = atoi(input); //unsigned int
            break;
        case NRDOSTAWCY:
            asprintf(&(current->val->nrDostawcy), "%s", input); //char*
            break;
        case NAZWADOSTAWCY:
            asprintf(&(current->val->nazwaDostawcy), "%s", input); //char*
            break;
        case ADRESDOSTAWCY:
            asprintf(&(current->val->adresDostawcy), "%s", input); //char*
            break;
        case DOWODZAKUPU:
            asprintf(&(current->val->dowodZakupu), "%s", input); //char*
            break;
        case DATAZAKUPU:
            asprintf(&(current->val->dataZakupu), "%s", input); //char*
            break;
        case DATAWPLYWU:
            asprintf(&(current->val->dataWplywu), "%s", input); //char*
            break;
        case K_43:
            current->val->k_43 = m2d(input); //double
            break;
        case K_44:
            current->val->k_44 = m2d(input); //double
            break;
        case K_45:
            current->val->k_45 = m2d(input); //double
            break;
        case K_46:
            current->val->k_46 = m2d(input); //double
            break;
        case K_47:
            current->val->k_47 = m2d(input); //double
            break;
        case K_48:
            current->val->k_48 = m2d(input); //double
            break;
        case K_49:
            current->val->k_49 = m2d(input); //double
            break;
        case K_50:
            current->val->k_50 = m2d(input); //double
            break;
    }

    data->purchaseTotal = evalTotalPurchase(data->purchase);
    return data;
}

char* mf2human(char* name) {
    char* out;
    if (strcmp("NrKontrahenta", name) == 0) {
        out = "Numer, za pomocą którego kontrahent jest zidentyfikowany na potrzeby podatku lub podatku od wartości dodanej. W przypadkach, w których zgodnie z ustawą podanie numeru nie jest wymagane, należy wpisać \"brak\" ";
    } else if (strcmp("NazwaKontrahenta", name) == 0) {
        out = "Imię i nazwisko lub nazwa kontrahenta";
    } else if (strcmp("AdresKontrahenta", name) == 0) {
        out = "";
    } else if (strcmp("DowodSprzedazy", name) == 0) {
        out = "Numer dowodu sprzedaży";
    } else if (strcmp("DataWystawienia", name) == 0) {
        out = "Data wystawienia dowodu sprzedaży ";
    } else if (strcmp("DataSprzedazy", name) == 0) {
        out = "Data sprzedaży, o ile jest określona i różni się od daty wystawienia faktury. W przeciwnym przypadku – pole puste (pole opcjonalne)";
    } else if (strcmp("K_10", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług na terytorium kraju, zwolnione od podatku (pole opcjonalne) ";
    } else if (strcmp("K_11", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług poza terytorium kraju (pole opcjonalne) ";
    } else if (strcmp("K_12", name) == 0) {
        out = "Kwota netto – w tym świadczenie usług, o których mowa w art. 100 ust. 1 pkt 4 ustawy (pole opcjonalne)";
    } else if (strcmp("K_13", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 0% (pole opcjonalne)";
    } else if (strcmp("K_14", name) == 0) {
        out = "Kwota netto – w tym dostawa towarów, o której mowa w art. 129 ustawy (pole opcjonalne)";
    } else if (strcmp("K_15", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 5% (pole opcjonalne)";
    } else if (strcmp("K_16", name) == 0) {
        out = "Kwota podatku należnego – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 5% (pole opcjonalne)";
    } else if (strcmp("K_17", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 7% albo 8% (pole opcjonalne)";
    } else if (strcmp("K_18", name) == 0) {
        out = "Kwota podatku należnego – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 7% albo 8% (pole opcjonalne)";
    } else if (strcmp("K_19", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 22% albo 23% (pole opcjonalne)";
    } else if (strcmp("K_20", name) == 0) {
        out = "Kwota podatku należnego – Dostawa towarów oraz świadczenie usług na terytorium kraju, opodatkowane stawką 22% albo 23% (pole opcjonalne)";
    } else if (strcmp("K_21", name) == 0) {
        out = "Kwota netto – Wewnątrzwspólnotowa dostawa towarów (pole opcjonalne) ";
    } else if (strcmp("K_22", name) == 0) {
        out = "Kwota netto – Eksport towarów (pole opcjonalne)";
    } else if (strcmp("K_23", name) == 0) {
        out = "Kwota netto – Wewnątrzwspólnotowe nabycie towarów (pole opcjonalne)";
    } else if (strcmp("K_24", name) == 0) {
        out = "Kwota podatku należnego – Wewnątrzwspólnotowe nabycie towarów (pole opcjonalne)";
    } else if (strcmp("K_25", name) == 0) {
        out = "Kwota netto – Import towarów podlegający rozliczeniu zgodnie z art.  33a ustawy (pole opcjonalne)";
    } else if (strcmp("K_26", name) == 0) {
        out = "Kwota podatku należnego – Import towarów podlegający rozliczeniu zgodnie z art. 33a ustawy (pole opcjonalne)";
    } else if (strcmp("K_27", name) == 0) {
        out = "Kwota netto – Import usług z wyłączeniem usług nabywanych od podatników podatku od wartości dodanej, do których stosuje się art.  28b ustawy (pole opcjonalne)";
    } else if (strcmp("K_28", name) == 0) {
        out = "Kwota podatku należnego – Import usług z wyłączeniem usług nabywanych od podatników podatku od wartości dodanej, do których stosuje się art. 28b ustawy (pole opcjonalne)";
    } else if (strcmp("K_29", name) == 0) {
        out = "Kwota netto – Import usług nabywanych od podatników podatku od wartości dodanej, do których stosuje się art. 28b ustawy (pole opcjonalne)";
    } else if (strcmp("K_30", name) == 0) {
        out = "Kwota podatku należnego – Import usług nabywanych od podatników podatku od wartości dodanej, do których stosuje się art. 28b ustawy (pole opcjonalne)";
    } else if (strcmp("K_31", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług, dla których podatnikiem jest nabywca zgodnie z art. 17 ust. 1 pkt 7 lub 8 ustawy (wypełnia dostawca) (pole opcjonalne)";
    } else if (strcmp("K_32", name) == 0) {
        out = "Kwota netto – Dostawa towarów, dla których podatnikiem jest nabywca zgodnie z art. 17 ust. 1 pkt 5 ustawy (wypełnia nabywca) (pole opcjonalne)";
    } else if (strcmp("K_33", name) == 0) {
        out = "Kwota podatku należnego – Dostawa towarów, dla których podatnikiem jest nabywca zgodnie z art. 17 ust. 1 pkt 5 ustawy (wypełnia nabywca) (pole opcjonalne)";
    } else if (strcmp("K_34", name) == 0) {
        out = "Kwota netto – Dostawa towarów oraz świadczenie usług, dla których podatnikiem jest nabywca zgodnie z art. 17 ust. 1 pkt 7 lub 8 ustawy (wypełnia nabywca) (pole opcjonalne)";
    } else if (strcmp("K_35", name) == 0) {
        out = "Kwota podatku należnego – Dostawa towarów oraz świadczenie usług, dla których podatnikiem jest nabywca zgodnie z art. 17 ust. 1 pkt 7 lub 8 ustawy (wypełnia nabywca) (pole opcjonalne)";
    } else if (strcmp("K_36", name) == 0) {
        out = "Kwota podatku należnego od towarów i usług objętych spisem z natury, o którym mowa w art. 14 ust. 5 ustawy (pole opcjonalne)";
    } else if (strcmp("K_37", name) == 0) {
        out = "Zwrot odliczonej lub zwróconej kwoty wydatkowanej na zakup kas rejestrujących, o którym mowa w art. 111 ust. 6 ustawy (pole opcjonalne)";
    } else if (strcmp("K_38", name) == 0) {
        out = "Kwota podatku należnego od wewnątrzwspólnotowego nabycia środków transportu, wykazanego w poz. 24, podlegająca wpłacie w terminie, o którym mowa w art. 103 ust. 3, w związku z ust. 4 ustawy (pole opcjonalne)";
    } else if (strcmp("K_39", name) == 0) {
        out = "Kwota podatku od wewnątrzwspólnotowego nabycia paliw silnikowych, podlegająca wpłacie w terminach, o których mowa w art. 103 ust. 5a i 5b ustawy (pole opcjonalne) ";
    } else if (strcmp("LiczbaWierszySprzedazy", name) == 0) {
        out = "Liczba wierszy ewidencji sprzedaży, w okresie którego dotyczy JPK";
    } else if (strcmp("PodatekNalezny", name) == 0) {
        out = "Podatek należny wg ewidencji sprzedaży w okresie, którego dotyczy JPK – suma kwot z elementów K_16, K_18, K_20, K_24, K_26, K_28, K_30, K_33, K_35, K_36 i K_37 pomniejszona o kwotę z elementów K_38 i K_39";
    } else if (strcmp("typZakupu", name) == 0) {
        out = "Atrybut oznaczający tabelę – we wszystkich wierszach tabeli przyjmuje wartość tekstową dużej litery „G” – grupa";
    } else if (strcmp("LpZakupu", name) == 0) {
        out = "Lp. wiersza ewidencji zakupu VAT";
    } else if (strcmp("NrDostawcy", name) == 0) {
        out = "Numer, za pomocą którego dostawca (kontrahent) jest zidentyfikowany na potrzeby podatku lub podatku od wartości dodanej";
    } else if (strcmp("NazwaDostawcy", name) == 0) {
        out = "Imię i nazwisko lub nazwa dostawcy (kontrahenta)";
    } else if (strcmp("AdresDostawcy", name) == 0) {
        out = "Adres dostawcy (kontrahenta)";
    } else if (strcmp("DowodZakupu", name) == 0) {
        out = "Numer dowodu zakupu";
    } else if (strcmp("DataZakupu", name) == 0) {
        out = "Data wystawienia dowodu zakupu ";
    } else if (strcmp("DataWplywu", name) == 0) {
        out = "Data wpływu dowodu zakupu (pole opcjonalne)";
    } else if (strcmp("K_43", name) == 0) {
        out = "Kwota netto – Nabycie towarów i usług zaliczanych u podatnika do środków trwałych (pole opcjonalne)";
    } else if (strcmp("K_44", name) == 0) {
        out = "Kwota podatku naliczonego – Nabycie towarów i usług zaliczanych u podatnika do środków trwałych (pole opcjonalne)";
    } else if (strcmp("K_45", name) == 0) {
        out = "Kwota netto – Nabycie towarów i usług pozostałych (pole opcjonalne).\nKomentarz: 23%, 8%, zw, 0%.";
    } else if (strcmp("K_46", name) == 0) {
        out = "Kwota podatku naliczonego – Nabycie towarów i usług pozostałych (pole opcjonalne).\nKomentarz: 23%, 8%, zw, 0%.";
    } else if (strcmp("K_47", name) == 0) {
        out = "Korekta podatku naliczonego od nabycia środków trwałych (pole opcjonalne)";
    } else if (strcmp("K_48", name) == 0) {
        out = "Korekta podatku naliczonego od pozostałych nabyć (pole opcjonalne)";
    } else if (strcmp("K_49", name) == 0) {
        out = "Korekta podatku naliczonego, o której mowa w art. 89b ust. 1 ustawy (pole opcjonalne)";
    } else if (strcmp("K_50", name) == 0) {
        out = "Korekta podatku naliczonego, o której mowa w art. 89b ust. 4 ustawy (pole opcjonalne) ";
    } else if (strcmp("PodatekNaliczony", name) == 0) {
        out = "Razem kwota podatku naliczonego do odliczenia – suma kwot z elementów K_44, K_46, K_47, K_48, K_49 i K_50";
    }
    return out;
}

JPK* newJPK() {
    struct passwd *pw = getpwuid(getuid());
    const char *homeDir = pw->pw_dir;
    char *newFile;
    asprintf(&newFile, "%s/.pltak/default.csv", homeDir);
    JPK* e = loadJPK(newFile);
    
    Date* date = getDate();
    
    char* year;
    char* month;
    if (getMonth(date) == 1) {
        asprintf(&year, "%d", atoi(date->year) - 1);
        asprintf(&month, "12");
    } else { 
        asprintf(&year, "%d", atoi(date->year));
        month = date->month;
    }

    int month_i;
    if (month[0] == '0') month_i = month[1] - '0';
    else month_i = atoi(month);

    char* timestamp;
    char* timestamp_start; 
    char* timestamp_end; 

    asprintf(&timestamp, "%s-%s-%s", date->year, date->month, date->day);
    asprintf(&timestamp_start, "%s-%s-%s", year, month, "1"); 
    asprintf(&timestamp_end, "%s-%s-%d", year, month, getLastDayOfMonth(month_i, atoi(year))); 
  
    e->header->dataWytworzeniaJPK = date->timestamp; 
    e->header->dataOd = timestamp_start;
    e->header->dataDo = timestamp_end;
    e->purchase->val->dataZakupu = timestamp;
    e->purchase->val->dataWplywu = timestamp;
    e->sold->val->dataSprzedazy = timestamp; 
    e->sold->val->dataWystawienia = timestamp;
    return e;
}

int getJPKMonth(JPK* jpk) {
    char* t = strdup(jpk->header->dataOd);
    strtok(t, "-");
    char* month = strtok(NULL, "-");

    int m;
    char tmp[3];
    if (month[0] == '0') {
        m = month[1] - '0';
    } else {
        tmp[0] = month[0];
        tmp[1] = month[1];
        tmp[2] = '\0';
        m = atoi(tmp);
    }
    return m;
}

char* getJPKYear(JPK* jpk) {
    char* t = strdup(jpk->header->dataOd);
    char* out = strtok(t, "-");
    return out;
}
