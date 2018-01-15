#ifndef JPK_H_
#define JPK_H_
#include "parse.h"

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
    double liczbaWierszySprzedazy;
    double podatekNalezny;
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
    double liczbaWierszyZakupow;
    double podatekNaliczony;
} JPKPurchase;


typedef struct SoldNode {
    JPKSold* val;
    struct SoldNode *next;
} JPKSoldList;

typedef struct PurchaseNode {
    JPKPurchase* val;
    struct PurchaseNode *next;
} JPKPurchaseList;

typedef struct ColNode {
    char* title;
    struct ColNode *next;
} JPKColumns;

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

JPK* changeSellData(JPK*, int, int, char*);
JPK* changePurData(JPK*, int, int, char*);
char* sell_d2m(JPK*, int, int);
void rmColumn(JPKColumns**, char*); 
int isElem(JPKColumns*, char*);
void addColumn(JPKColumns*, char*);
JPK* loadJPK(char*);
void printSold(JPK*);
void printPurchases(JPK*);
double m2d(char*);
void rmSellRow(JPK*, int);
void addSellRow(JPK*);
void addPurchaseRow(JPK*);
char* pur_d2m(JPK*, int, int); 
void rmPurchaseRow(JPK*, int);
char* mf2human(char*);
JPK* newJPK();
int getJPKMonth(JPK*);
char* getJPKYear(JPK*);

#endif // JPK_H_
