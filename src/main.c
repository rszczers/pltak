#include <stdio.h>
#include <locale.h>
#include "jpk.h"
#include "tocsv.h"
#include "gui.h"

int main(int argc, char *argv[])
{
    setlocale(LC_NUMERIC, "pl_PL.utf8");
    JPK* data = loadJPK("./data/JPK_VAT_02.csv");
    drawGui(data);
    return 0;
}
