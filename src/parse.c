#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void showLast(tToken*); 

void addData(tData *head, int rowNum, tToken *row) {
    if (head == NULL) exit(EXIT_FAILURE);

    tData* prev;
    while (head != NULL) {
        prev = head;
        head = head->next;
    }

    if (prev->row == NULL) {
        prev->rowNum = rowNum;
        prev->row = row;
    } else {
        tData *newData = (tData*)malloc(sizeof(tData));
        newData->rowNum = rowNum;
        newData->row = row;
        newData->next = NULL;
        prev->next = newData;
    }
}

void addToken(tToken *head, int colNum, char *val) {
    if (head == NULL) exit(EXIT_FAILURE);

    tToken* prev;
    while (head != NULL) {
        prev = head;
        head = head->next;
    }

    if (prev->val == NULL) { //list jest pusta
        asprintf(&(prev->val), "%s", val);
        prev->colNum = colNum;
    } else {
        tToken *newToken = (tToken*)malloc(sizeof(tToken));
        newToken->colNum = colNum;
        newToken->next = NULL;
        newToken->val = val;
        prev->next = newToken;
    }
}

/* Zwraca row-ty wiersz w postaci
 * listy tToken.
 */
tToken* getRow(tData* data, int row) {
    if (data != NULL) {
        if (data->rowNum == row) {
            return data->row;
        } else {
            if (data->next != NULL) {
                return getRow(data->next, row);
            } else {
                return NULL;
            }
        }
    }
    return NULL;
}

tToken* getCol(tToken* tokenList, int col) {
    if (tokenList != NULL) {
        if (tokenList->colNum == col) {
            return tokenList;
        } else {
            if (tokenList->next != NULL) {
                return getCol(tokenList->next, col);
            } else {
                return NULL;
            }
        }
    }
    return NULL;
}

/* Zwraca zawartość ij-tej komórki
 * Indeksacja zaczyna się od 1.
 */
char* getCell(tData* data, int row, int col) {
    tToken* cell = getCol(getRow(data, row), col);
    char* val = "";
    if (cell != NULL) {
        val = cell->val;
    }
    return val;
}

void showData(tData *currRow) {
    while (currRow != NULL) {
        printf("%d:  ", currRow->rowNum);
        tToken *content = currRow->row;
        if (content == NULL) {
            printf("Pusto\n");
        } else {
            while (content != NULL) {
                printf("\t%-40s (w %d, k %d)\n", content->val, currRow->rowNum, content->colNum);
                content = content->next;
            }
            printf("\n");
        }
        currRow = currRow->next;
    }
}

void showLast(tToken *t) {
    if (t != NULL) {
        printf("%d: %s\n", t->colNum, t->val);
        tToken* b = t->next;
        showLast(b);
    }
}

tData* parse(char* filename) {
    FILE *fp;
    char *line = NULL;
    char *buffer = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Błąd. Nie można otworzyć pliku.\n");
        exit(EXIT_FAILURE);
    } 
    tData *parsedData = malloc(sizeof(tData));
    parsedData->row = NULL;
    parsedData->next = NULL;
    
    int rowNum = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        tToken *tokenizedRow = (tToken*)malloc(sizeof(tToken));
        tokenizedRow->val = NULL;
        tokenizedRow->next = NULL;
        char *token;
        int colNum = 1;
        char *p = line;
        char *k;
        while (*p != '\r' && *p+1 != '\n' && *p != '\0') {
            token = (char*)malloc(1024*sizeof(char));
            if (*p == '\"') {
                for (k = p+1; *k != '\"'; k++) {
                    if (*k == '\0') {
                        if (*(p+1) == ';') {
                            sprintf(token, "%c", '\"');
                            addToken(tokenizedRow, colNum, token);
                        } else {
                            exit(EXIT_FAILURE); //Zły format pliku
                        }
                    }
                }
                strncpy(token, p, k-p+1);
                token[k-p+1] = '\0';
                p = k;
                addToken(tokenizedRow, colNum, token);
            } else if (*p == ';') {
                colNum++;
            } else {
                k = p;
                while (*k != ';' && *k != '\r' && *k != '\n') {
                    k++;
                }
                strncpy(token, p, k-p);
                token[k-p] = '\0';
                p = k - 1;
                if (strlen(token) > 0)
                    addToken(tokenizedRow, colNum, token);
            }
            p++;
        }
        addData(parsedData, rowNum, tokenizedRow);
        rowNum++;
    }
    fclose(fp);

    if (line)
        free(line);
    return parsedData;
}
