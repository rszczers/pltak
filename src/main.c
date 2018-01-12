#define _GNU_SOURCE
#include <stdio.h>
#include <locale.h>
#include <pwd.h>
#include <unistd.h>
#include "jpk.h"
#include "tocsv.h"
#include "gui.h"

int main(int argc, char *argv[])
{
    setlocale(LC_NUMERIC, "pl_PL.utf8");

    struct passwd *pw = getpwuid(getuid());
    const char *homeDir = pw->pw_dir;
    char *newFile;
    asprintf(&newFile, "%s/.pltak/JPK_VAT_02.csv", homeDir);

    JPK* data = loadJPK(newFile);
    drawGui(data);
    return 0;
}
