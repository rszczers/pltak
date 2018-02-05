#define _GNU_SOURCE
#include <stdio.h>
#include <locale.h>
#include <pwd.h>
#include <unistd.h>
#include "jpk.h"
#include "gui.h"

int main(int argc, char *argv[])
{
    setlocale(LC_NUMERIC, "pl_PL.utf8");
    JPK* newData = newJPK();
    drawGui(newData);
    return 0;
}
