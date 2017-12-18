#ifndef JPK_H_
#define JPK_H_

typedef struct sHeader JPKHeader;
typedef struct sProfile JPKProfile;
typedef struct sSold JPKSold;
typedef struct sPurchase JPKPurchase;

int countSells(tData* data);
int countPurchases(tData* data);
#endif // JPK_H_
