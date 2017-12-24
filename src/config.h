#ifndef CONFIG_H_
#define CONFIG_H_
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
    char** sellColumns;
    char** purchaseColumns;
} TakConfig;

TakConfig* parseConfig();
void printTakConfig(TakConfig*);
#endif /* ifndef CONFIG_H_ */
