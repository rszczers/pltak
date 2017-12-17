#include <stdio.h>
#include "jpk.h"
#include "parse.h"

int main(int argc, char *argv[])
{
    printf("test");
    tData* data = parse("../data/JPK_VAT_02.csv");
    showData(data);
    return 0;
}
