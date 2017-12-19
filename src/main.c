#include <stdio.h>
#include "jpk.h"
#include "tocsv.h"

int main(int argc, char *argv[])
{
    JPK* data = loadJPK("./data/JPK_VAT_02.csv");
    csvExport("./data/test.csv", data);
    //printSold(data);
    //printPurchases(data);
    //
    //tData* data = parse("./data/JPK_VAT_02.csv");
    //showData(data);
    //printf("\nLiczba sprzedaży: %d,", countSells(data));
    //printf("\nLiczba zakupów: %d.\n", countPurchases(data));
    return 0;
}
