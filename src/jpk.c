#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

tData* parsedData = NULL;
int sellRegLen;
int purchaseRegLen;

typedef enum {
    COLUMN_NAME = 1,
    HEADER,
    PROFILE,
    ADDRESS,
    SELLS,
    PURCHASES = 16
} JPKRow;

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
    int lpSprzedazy;
    int nrKontrahenta;
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
    double liczbaWierszySprzedazy;
    double podatekNalezny;
} JPKSold;

typedef struct sPurchase {
    char* typZakupu;
    int lpZakupu;
    int nrDostawcy;
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
    double liczbaWierszyZakupow;
    double podatekNaliczony;
} JPKPurchase;

typedef struct SoldNode {
    JPKSold* val;
    struct SoldNode *next;
} JPKSoldList;

void addSold(JPKSoldList* list, JPKSold* row) {
    while(list->next != NULL) {
        list = list->next;
    }
    JPKSoldList* newData = (JPKSoldList*)malloc(sizeof(JPKSoldList));
    newData->val = row;
    newData->next = NULL;
    list->next = newData;
}

typedef struct PurchaseNode {
    JPKPurchase* val;
    struct PurchaseNode *next;
} JPKPurchaseList;

void addPurchase(JPKPurchaseList* list, JPKPurchase* row) {
    while(list->next != NULL) {
        list = list->next;
    }
    JPKPurchaseList* newData = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
    newData->val = row;
    newData->next = NULL;
    list->next = newData;
}

JPKHeader* convHeader(tData* data) {
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

JPKProfile* convProfile(tData* data) {
    JPKProfile* profile = (JPKProfile*)malloc(sizeof(JPKProfile));
    profile->nip = getCell(data, PROFILE, NIP);
    profile->pelnaNazwa = getCell(data, PROFILE, PELNANAZWA);
    profile->regon = getCell(data, PROFILE, REGON);
    profile->kodKraju = getCell(data, PROFILE, KODKRAJU);
    profile->wojewodztwo = getCell(data, PROFILE, WOJEWODZTWO);
    profile->powiat = getCell(data, PROFILE, POWIAT);
    profile->gmina = getCell(data, PROFILE, GMINA);
    profile->ulica = getCell(data, PROFILE, ULICA);
    profile->nrDomu = getCell(data, PROFILE, NRDOMU);
    profile->nrLokalu = getCell(data, PROFILE, NRLOKALU);
    profile->miejscowosc = getCell(data, PROFILE, MIEJSCOWOSC);
    profile->kodPocztowy = getCell(data, PROFILE, KODPOCZTOWY);
    profile->poczta = getCell(data, PROFILE, POCZTA);
    return profile;
}

/* Uzupełnia strukturę sprzedaży
 * row - numer wiersza sprzedaży do wczytania do struktury
 */
JPKSold* rowToSold(tData* data, int row) {
    JPKSold* raport = (JPKSold*)malloc(sizeof(JPKSold));
    raport->typSprzedazy = getCell(data, row, TYPSPRZEDAZY);
    raport->lpSprzedazy = strtol(getCell(data, row, LPSPRZEDAZY), NULL, 10);
    raport->nrKontrahenta = strtol(getCell(data, row, NRKONTRAHENTA), NULL, 10);
    raport->nazwaKontrahenta = getCell(data, row, NAZWAKONTRAHENTA);
    raport->adresKontrahenta = getCell(data, row, ADRESKONTRAHENTA);
    raport->dowodSprzedazy = getCell(data, row, DOWODSPRZEDAZY);
    raport->dataWystawienia = getCell(data, row, DATAWYSTAWIENIA);
    raport->dataSprzedazy = getCell(data, row, DATASPRZEDAZY);
    sscanf(getCell(data, row, K_10), "%lf", raport->k_10);
    sscanf(getCell(data, row, K_11), "%lf", raport->k_11);
    sscanf(getCell(data, row, K_13), "%lf", raport->k_13);
    sscanf(getCell(data, row, K_13), "%lf", raport->k_13);
    sscanf(getCell(data, row, K_14), "%lf", raport->k_14);
    sscanf(getCell(data, row, K_15), "%lf", raport->k_15);
    sscanf(getCell(data, row, K_16), "%lf", raport->k_16);
    sscanf(getCell(data, row, K_17), "%lf", raport->k_17);
    sscanf(getCell(data, row, K_18), "%lf", raport->k_18);
    sscanf(getCell(data, row, K_19), "%lf", raport->k_19);
    sscanf(getCell(data, row, K_20), "%lf", raport->k_20);
    sscanf(getCell(data, row, K_21), "%lf", raport->k_21);
    sscanf(getCell(data, row, K_22), "%lf", raport->k_22);
    sscanf(getCell(data, row, K_23), "%lf", raport->k_23);
    sscanf(getCell(data, row, K_24), "%lf", raport->k_24);
    sscanf(getCell(data, row, K_25), "%lf", raport->k_25);
    sscanf(getCell(data, row, K_26), "%lf", raport->k_26);
    sscanf(getCell(data, row, K_27), "%lf", raport->k_27);
    sscanf(getCell(data, row, K_28), "%lf", raport->k_28);
    sscanf(getCell(data, row, K_29), "%lf", raport->k_29);
    sscanf(getCell(data, row, K_30), "%lf", raport->k_30);
    sscanf(getCell(data, row, K_31), "%lf", raport->k_31);
    sscanf(getCell(data, row, K_32), "%lf", raport->k_32);
    sscanf(getCell(data, row, K_33), "%lf", raport->k_33);
    sscanf(getCell(data, row, K_34), "%lf", raport->k_34);
    sscanf(getCell(data, row, K_35), "%lf", raport->k_35);
    sscanf(getCell(data, row, K_36), "%lf", raport->k_36);
    sscanf(getCell(data, row, K_37), "%lf", raport->k_37);
    sscanf(getCell(data, row, K_38), "%lf", raport->k_38);
    sscanf(getCell(data, row, K_39), "%lf", raport->k_39);
    sscanf(getCell(data, row + 1, LICZBAWIERSZYSPRZEDAZY), "%lf", raport->liczbaWierszySprzedazy);
    sscanf(getCell(data, row + 1, PODATEKNALEZNY), "%lf", raport->podatekNalezny);
    return raport;
}

/* Uzupenia strukturę zakupów
 * row – numer wiersza pole zakupów do wczytania do struktury
 */
JPKPurchase* rowToPurchase(tData* data, int row) {
    JPKPurchase* raport = (JPKPurchase*)malloc(sizeof(JPKPurchase));
    raport->typZakupu = getCell(data, row, TYPZAKUPU);
    raport->lpZakupu = strtol(getCell(data, row, LPZAKUPU), NULL, 10);
    raport->nrDostawcy = strtol(getCell(data, row, NRDOSTAWCY), NULL, 10);
    raport->nazwaDostawcy = getCell(data, row, NAZWADOSTAWCY);
    raport->adresDostawcy = getCell(data, row, ADRESDOSTAWCY);
    raport->dowodZakupu = getCell(data, row, DOWODZAKUPU);
    raport->dataZakupu = getCell(data, row, DATAZAKUPU);
    raport->dataWplywu = getCell(data, row, DATAWPLYWU);
    scanf(getCell(data, row, K_43), "%lf", raport->k_43);
    scanf(getCell(data, row, K_44), "%lf", raport->k_44);
    scanf(getCell(data, row, K_45), "%lf", raport->k_45);
    scanf(getCell(data, row, K_46), "%lf", raport->k_46);
    scanf(getCell(data, row, K_47), "%lf", raport->k_47);
    scanf(getCell(data, row, K_48), "%lf", raport->k_48);
    scanf(getCell(data, row, K_49), "%lf", raport->k_49);
    scanf(getCell(data, row, K_50), "%lf", raport->k_50);
    scanf(getCell(data, row, LICZBAWIERSZYZAKUPOW), "%lf", raport->liczbaWierszyZakupow);
    scanf(getCell(data, row, PODATEKNALICZONY), "%lf", raport->podatekNaliczony);
    return raport;
}

int countSells(tData* data) {
    int c = 0;
    for (int i = 0; i < SELLS - 1; ++i) {
        data = data->next;
    }
    while (data->next != NULL && data->row != NULL) {
        c++;
        data = data->next;
    }
    return c;
}

int countPurchases(tData* data) {
    int c = 0;
    for (int i = 0; i < PURCHASES - 1; ++i) {
        data = data->next;
    }
    while (data->next != NULL && strcmp(data->row->val, "G") == 0) {
        c++;
        data = data->next;
    }
    return c;
}

void loadJPK(char *filename) {
    parsedData = parse(filename);
    sellRegLen = countSells(parsedData);
    purchaseRegLen = countPurchases(parsedData);
}

void closeJPK() {
    free(parsedData);
}

JPKHeader* getHeader() {
    JPKHeader* header = NULL;
    if (parsedData != NULL) {
        header = convHeader(parsedData);
    }
    return header;
}

JPKProfile* getProfile() {
    JPKProfile* profile = NULL;
    if (parsedData != NULL) {
        profile = convProfile(parsedData);
    }
    return profile;
}

JPKSoldList* getSoldList() {
    JPKSoldList* sells = (JPKSoldList*)malloc(sizeof(JPKSoldList));
    for (int i = SELLS; i < SELLS + sellRegLen; ++i) {
        addSold(sells, rowToSold(parsedData, i));
    }
    return sells;
}

JPKPurchaseList* getPurchasesList() {
    JPKPurchaseList* purchases = (JPKPurchaseList*)malloc(sizeof(JPKPurchaseList));
    for (int i = PURCHASES; i < PURCHASES + purchaseRegLen; ++i) {
        addPurchase(purchases, rowToPurchase(parsedData, i));
    }
    return purchases;
}
