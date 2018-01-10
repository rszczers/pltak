#ifndef PARSE_H_
#define PARSE_H_

typedef struct sToken {
    int colNum;
    char *val;
    struct sToken *next;
} tToken;

typedef struct sData {
    int rowNum;
    tToken *row;
    struct sData *next;
} tData;

tData* parse(char* filename);
tToken* getRow(tData* data, int row);
void showData(tData *currRow);
char* getCell(tData* data, int row, int cal);

#endif // PARSE_H_
