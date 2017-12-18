#include <stdio.h>
#include "parse.h"
#include "jpk.h"

int main(int argc, char *argv[])
{
    tData* data = parse("./data/JPK_VAT_02.csv");
    showData(data);
    printf("\nLiczba sprzedaży: %d,", countSells(data));
    printf("\nLiczba zakupów: %d.\n", countPurchases(data));
    return 0;
}
