#ifndef PARSE_H_
#define PARSE_H_

typedef struct sToken tToken;
typedef struct sData tData;

tData* parse(const char* filename);
tToken* getRow(tData* data, int row);
void showData(tData *currRow);
char* getCell(tData* data, int row, int cal);

#endif // PARSE_H_
