#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpk.h"
#include "utils.h"

#define ENDL "\r\n"
#define SEP ";"
#define CATEGORIES "KodFormularza;kodSystemowy;wersjaSchemy;WariantFormularza;CelZlozenia;DataWytworzeniaJPK;DataOd;DataDo;NazwaSystemu;NIP;PelnaNazwa;Email;LpSprzedazy;NrKontrahenta;NazwaKontrahenta;AdresKontrahenta;DowodSprzedazy;DataWystawienia;DataSprzedazy;K_10;K_11;K_12;K_13;K_14;K_15;K_16;K_17;K_18;K_19;K_20;K_21;K_22;K_23;K_24;K_25;K_26;K_27;K_28;K_29;K_30;K_31;K_32;K_33;K_34;K_35;K_36;K_37;K_38;K_39;LiczbaWierszySprzedazy;PodatekNalezny;LpZakupu;NrDostawcy;NazwaDostawcy;AdresDostawcy;DowodZakupu;DataZakupu;DataWplywu;K_43;K_44;K_45;K_46;K_47;K_48;K_49;K_50;LiczbaWierszyZakupow;PodatekNaliczony"
#define CLINE ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;"

char* genHeader(JPK* jpk) {
    const char* tail = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    char* header;
    char kodUrzedu[5];
    kodUrzedu[4] = '\0';
    asprintf(&header, "%s" SEP
            "%s" SEP
            "%s" SEP
            "%d" SEP
            "%d" SEP
            "%s" SEP
            "%s" SEP
            "%s" SEP
            "%s" "%s" ENDL,
            jpk->header->kodFormularza,
            jpk->header->kodSystemowy,
            jpk->header->wersjaSchemy,
            jpk->header->wariantFormularza,
            jpk->header->celZlozenia,
            getDate()->timestamp,
            jpk->header->dataOd,
            jpk->header->dataDo,
            jpk->header->nazwaSystemu,
            tail);
    return header;
}

char* genProfile(JPK* jpk) {
    const char* head = ";;;;;;;;;";
    const char* tail = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    char* profile;
    asprintf(&profile,
   "%s" "%s" SEP
        "%s" SEP
        "%s" "%s" ENDL,
        head,
        jpk->profile->nip == NULL ? "" : jpk->profile->nip,
        jpk->profile->pelnaNazwa == NULL ? "" : jpk->profile->pelnaNazwa,
        jpk->profile->email == NULL ? "" : jpk->profile->email,
        tail);
    return profile;
}

char* genSold(JPK* jpk) {
    JPKSoldList* row = jpk->sold;
    const char* head = ";;;;;;;;;;;;";
    const char* tail = ";;;;;;;;;;;;;;;;;;;";
    const char* total_head = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    const char* total_tail = ";;;;;;;;;;;;;;;;;";
    char* soldField = "";
    char* soldRow;
    char* buffer;
    if (jpk->soldCount > 0)
        while (row != NULL) {
            asprintf(&soldRow,
           "%s" "%d" SEP
                "%s" SEP
                "%s" SEP
                "%s" SEP
                "%s" SEP
                "%s" SEP
                "%s" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" SEP
                "%.2lf" "%s" ENDL,
                head,
                row->val->lpSprzedazy,        //int
                row->val->nrKontrahenta,      //int
                row->val->nazwaKontrahenta,     //char*
                row->val->adresKontrahenta,     //char*
                row->val->dowodSprzedazy,       //char*
                row->val->dataWystawienia,      //char*
                row->val->dataSprzedazy,        //char*
                row->val->k_10,        //double
                row->val->k_11,        //double
                row->val->k_12,        //double
                row->val->k_13,        //double
                row->val->k_14,        //double
                row->val->k_15,        //double
                row->val->k_16,        //double
                row->val->k_17,        //double
                row->val->k_18,        //double
                row->val->k_19,        //double
                row->val->k_20,        //double
                row->val->k_21,        //double
                row->val->k_22,        //double
                row->val->k_23,        //double
                row->val->k_24,        //double
                row->val->k_25,        //double
                row->val->k_26,        //double
                row->val->k_27,        //double
                row->val->k_28,        //double
                row->val->k_29,        //double
                row->val->k_30,        //double
                row->val->k_31,        //double
                row->val->k_32,        //double
                row->val->k_33,        //double
                row->val->k_34,        //double
                row->val->k_35,        //double
                row->val->k_36,        //double
                row->val->k_37,        //double
                row->val->k_38,        //double
                row->val->k_39,        //double
                tail);

        asprintf(&buffer, "%s%s", soldField, soldRow);
        free(soldRow);

        soldField = buffer;
        row = row->next;
    }
    char* total;
    asprintf(&total, "%s" "%d" SEP "%.2lf" "%s" ENDL, total_head, jpk->soldCount, jpk->soldTotal, total_tail);
    asprintf(&buffer, "%s%s", soldField, total);

    soldField = buffer;
    return soldField;
}

char* genPurchase(JPK* jpk) {
    JPKPurchaseList* row = jpk->purchase;
    const char* head = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    const char* tail = ";;";
    const char* total_head = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    char* purchaseField = "";
    char* purchaseRow;
    char* buffer;
    if (jpk->purchaseCount > 0)
    while (row != NULL) {
        asprintf(&purchaseRow,
       "%s" "%d" SEP
            "%s" SEP
            "%s" SEP
            "%s" SEP
            "%s" SEP
            "%s" SEP
            "%s" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" SEP
            "%.2lf" "%s" ENDL,
            head,
            row->val->lpZakupu,       //int
            row->val->nrDostawcy,     //int
            row->val->nazwaDostawcy,        //char*
            row->val->adresDostawcy,        //char*
            row->val->dowodZakupu,      //char*
            row->val->dataZakupu,       //char*
            row->val->dataWplywu,       //char*
            row->val->k_43,        //double
            row->val->k_44,        //double
            row->val->k_45,        //double
            row->val->k_46,        //double
            row->val->k_47,        //double
            row->val->k_48,        //double
            row->val->k_49,        //double
            row->val->k_50,        //double
            tail);
        asprintf(&buffer, "%s%s", purchaseField, purchaseRow);
        free(purchaseRow);

        purchaseField = buffer;
        row = row->next;
    }
    char* total;
    asprintf(&total, "%s" "%d" SEP "%.2lf" ENDL, total_head, jpk->purchaseCount, jpk->purchaseTotal);
    asprintf(&buffer, "%s%s", purchaseField, total);

    purchaseField = buffer;
    return purchaseField;
}

void csvExport(char* filename, JPK* jpk) {
    char* csv;
    asprintf(&csv, CATEGORIES ENDL "%s" "%s" CLINE ENDL "%s" "%s",
        genHeader(jpk),
        genProfile(jpk),
        jpk->soldCount > 0 ? genSold(jpk) : "",
        jpk->purchaseCount > 0 ? genPurchase(jpk) : "");
    FILE *fp = fopen(filename, "w");
    if (fp != NULL) {
        fputs(csv, fp);
        fclose(fp);
    }
    free(csv);
}
