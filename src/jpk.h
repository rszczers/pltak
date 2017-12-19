#ifndef JPK_H_
#define JPK_H_

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

typedef struct PurchaseNode {
    JPKPurchase* val;
    struct PurchaseNode *next;
} JPKPurchaseList;

typedef struct {
    JPKHeader* header;
    JPKProfile* profile;
    JPKSoldList* sold;
    int soldCount;
    double soldTotal;
    JPKPurchaseList* purchase;
    int purchaseCount;
    double purchaseTotal;
} JPK;


JPK* loadJPK(char*);
void printSold(JPK*);
void printPurchases(JPK*);
double m2d(char*);

#endif // JPK_H_
