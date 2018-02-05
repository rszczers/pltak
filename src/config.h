#ifndef CONFIG_H_
#define CONFIG_H_
#include "jpk.h"

typedef struct {
    char* NIP;
    char* PelnaNazwa;
    char* Email;
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
