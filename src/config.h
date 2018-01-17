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
    JPKColumns* sellColumns;
    JPKColumns* purchaseColumns;
} TakConfig;

TakConfig* getConfig(JPK*);
TakConfig* parseConfig();
void saveConfig(TakConfig*);
void printTakConfig(TakConfig*);
void printTakCols(JPKColumns*); 
JPK* configToJPK(JPK*, TakConfig*);

#endif /* ifndef CONFIG_H_ */
