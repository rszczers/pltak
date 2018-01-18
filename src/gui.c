#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include "tocsv.h"
#include "jpk.h"
#include "config.h"
#include "utils.h"
#include "history.h"

#define HISTORY_OPEN_FILENAME "open.dat"

GtkWidget *label_sell_sum;
GtkWidget *label_pur_sum;
GtkWidget *date_menu;
GtkWidget *entry_year;
History* open_history;

int current_notebook;
int default_month;

// lista do cofania zmian danych
typedef struct _JPKHistory {
    JPK* data;
    TakConfig cfg;
    struct _JPKHistory* next;
} JPKHistory;

typedef struct USNode {
    char* code;
    char* name;
    struct USNode *next;
} USList;

static void sell_filter_callback(GtkWidget* widget, gpointer data);
static void purchase_filter_callback(GtkWidget*, gpointer);
static void sell_filter_from_table_callback(GtkWidget*, gpointer);
static void pur_filter_from_table_callback(GtkWidget*, gpointer);
static void sell_rmrow_callback(GtkWidget*, gpointer);
static void pur_rmrow_callback(GtkWidget*, gpointer);
//static void sell_entry_insert_callback(GtkEntry*,const gchar*, gint, gint*, gpointer);
//static void sell_entry_delete_callback(GtkEntry*, const gchar*, gint, gint*, gpointer);

static void sell_entry_callback(GtkWidget*, gpointer);
static void pur_entry_callback(GtkWidget*, gpointer);
static void sell_addrow_callback(GtkWidget*, gpointer);
static void pur_addrow_callback(GtkWidget*, gpointer);
static void importcsv_open_dialog(GtkWidget*, gpointer);
static void savecsv_as_dialog(GtkWidget*, gpointer);
static void savecsv_dialog(GtkWidget*, gpointer);
static void new_file_callback(GtkWidget*, gpointer);
void importcsv_lastopen_dialog(GtkWidget*, gpointer);

void addUS(USList* list, char* code, char* name) {
    while (list->next != NULL) {
        list = list->next;
    }
    if (list->name == NULL) {
        list->name = name;
        list->code = code;
    } else {
        USList* nextNode = (USList*)malloc(sizeof(nextNode));
        nextNode->code = code;
        nextNode->name = name;
        nextNode->next = NULL;
        list->next = nextNode;
    }
}

USList* loadUSCodes() {
    USList* codes = (USList*)malloc(sizeof(USList));
    codes->name = NULL;
    codes->next = NULL;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *filename;
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    asprintf(&filename, "%s/.pltak/us_codes.dat", homedir);

    if (access(filename, R_OK | F_OK) != -1) {
        fp = fopen(filename, "r");
        char *code;
        char *name;
        while ((read = getline(&line, &len, fp)) != -1) {
            code = (char*)malloc(5);
            memcpy(code, line, 4);
            code[4] = '\0';
            asprintf(&name, "%s", line + 5);
            name[strlen(name) - 1] = '\0';
            addUS(codes, code, name);
        }
        fclose(fp);
    }
    return codes;
}

GList* appendUSCodes(GtkWidget* combo) {
    GList *cbitems = NULL;
    USList* list = loadUSCodes();
    while (list != NULL) {
        char* s;
        asprintf(&s, "%s, %s", list->code, list->name);
        cbitems = g_list_append(cbitems, s);
        list = list->next;
    }
    return cbitems;
}

typedef struct _Completion {
    GtkWidget* table;
    GtkWidget* combo;
    TakConfig* config;
    JPK* jpk;
} Completion;

/**
 * To chyba najbrzydszy kod jaki kiedykolwiek napisałem.
 * Podpowiada kod urzędu. Niewazne, działa.
 */
static void uscode_callback(GtkWidget* entry, gpointer data) {
    // Wyczyść dotychczasową listę kodów
    Completion* compl = (Completion*) data;
    GtkWidget* table = compl->table;
    GtkWidget* combo = compl->combo;
    JPK* jpk = compl->jpk;
    TakConfig* config = compl->config;

    char* filter = strdup((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
    for (int i = 0; i < strlen(filter); ++i) {
       filter[i] = toupper(filter[i]);
    }
    gtk_widget_destroy(combo);

    GList *cbitems = NULL;
    GtkWidget* newCombo = gtk_combo_new();
    // Dodaj kody wg. filtra
    USList* codes = loadUSCodes();

    int changes = 0;
    while (codes != NULL) {
        if (strstr(codes->name, filter) != NULL) {
            char* s;
            asprintf(&s, "%s, %s", codes->code, codes->name);
            cbitems = g_list_append(cbitems, s);
            changes++;
        }
        codes = codes->next;
    }
    if (changes == 0) {
        cbitems = g_list_append(cbitems, "Nie znaleziono wzorca.");
    }
    char* head = (char*) cbitems->data;
    gtk_combo_set_popdown_strings(GTK_COMBO(newCombo), cbitems);
    gtk_widget_show_now(newCombo);
    gtk_table_attach_defaults(GTK_TABLE(table), newCombo, 1, 2, 1, 2);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(newCombo)->entry), head);
    gtk_widget_grab_focus(GTK_WIDGET(GTK_COMBO(newCombo)->entry));
    Completion* newData = (Completion*)malloc(sizeof(Completion));
    config->KodUrzedu = head;
    saveConfig(config);

    jpk->header->kodUrzedu = (char*)malloc(5);
    strncpy(jpk->header->kodUrzedu, head, 4);
    jpk->header->kodUrzedu[4] = '\0';

    newData->jpk = jpk;
    newData->combo = newCombo;
    newData->table = table;
    newData->config = config;
    g_signal_connect(GTK_ENTRY(GTK_COMBO(newCombo)->entry), "activate", G_CALLBACK(uscode_callback), newData);
//    g_signal_connect(GTK_COMBO(newCombo), "activate", G_CALLBACK(uscode_callback), newData);
}

typedef struct {
    char* data;
    TakConfig *config;
    JPK* jpk;
    GtkWidget* parent;
} OpenCallbackData;

typedef struct {
    char *name;
    TakConfig *config;
    JPK* jpk;
} ColFilter;

typedef struct {
    int i;
    int j;
    JPK* jpk;
    TakConfig* tak;
} JPKChange;

static void waluta_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->DomyslnyKodWaluty = input;
    jpk->header->domyslnyKodWaluty = input;
    saveConfig(config);
}
static void nip_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->NIP = input;
    jpk->profile->nip = input;
    saveConfig(config);
}
static void pelnanazwa_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->PelnaNazwa = input;
    jpk->profile->pelnaNazwa = input;
    saveConfig(config);
}
static void regon_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->REGON = input;
    jpk->profile->regon = input;
    saveConfig(config);
}
static void kraj_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->KodKraju = input;
    jpk->profile->kodKraju = input;
    saveConfig(config);
}
static void wojewodztwo_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Wojewodztwo = input;
    jpk->profile->wojewodztwo = input;
    saveConfig(config);
}
static void powiat_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Powiat = input;
    jpk->profile->powiat = input;
    saveConfig(config);
}
static void gmina_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Gmina = input;
    jpk->profile->gmina = input;
    saveConfig(config);
}
static void ulica_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Ulica = input;
    jpk->profile->ulica = input;
    saveConfig(config);
}
static void nrdomu_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->NrDomu = input;
    jpk->profile->nrDomu = input;
    saveConfig(config);
}
static void nrlokalu_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->NrLokalu = input;
    jpk->profile->nrDomu = input;
    saveConfig(config);
}
static void miejscowosc_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Miejscowosc = input;
    jpk->profile->miejscowosc = input;
    saveConfig(config);
}
static void kodpocztowy_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->KodPocztowy = input;
    jpk->profile->kodPocztowy = input;
    saveConfig(config);
}
static void poczta_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    TakConfig* config = ch->tak;
    JPK* jpk = ch->jpk;
    char* input = strdup((char*)gtk_entry_get_text(GTK_ENTRY(widget)));
    config->Poczta = input;
    jpk->profile->poczta = input;
    saveConfig(config);
}

static void create_sell_col_filter(GtkWidget* widget, JPK* jpk, TakConfig* config) {
    GtkWidget* check_sell;
    JPKColumns* title = jpk->colNames;
    while(strcmp(title->title, "NrKontrahenta") != 0)
         title = title->next;
    while(strcmp(title->title, "LiczbaWierszySprzedazy") != 0) {
        ColFilter* data = (ColFilter*)malloc(sizeof(ColFilter));
        data->config = config;
        data->name = title->title;
        data->jpk = jpk;
        check_sell = gtk_check_button_new_with_label(title->title);
        gtk_widget_set_tooltip_text(check_sell, mf2human(title->title));
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON (check_sell),
                isElem(config->sellColumns, title->title));
        g_signal_connect(GTK_TOGGLE_BUTTON(check_sell), "clicked",
                        G_CALLBACK(sell_filter_callback), data);
        gtk_box_pack_start(GTK_BOX(widget), check_sell, 0, 0, 0);
        title = title->next;
    }
}

static void create_pur_col_filter(GtkWidget* widget, JPK* jpk, TakConfig* config) {
    GtkWidget* check_purchase;
    JPKColumns* title = jpk->colNames;
    while(strcmp(title->title, "NrDostawcy") != 0)
         title = title->next;
    while(strcmp(title->title, "LiczbaWierszyZakupow") != 0) {
        ColFilter* data = (ColFilter*)malloc(sizeof(ColFilter));
        data->config = config;
        data->name = title->title;
        data->jpk = jpk;
        check_purchase = gtk_check_button_new_with_label(title->title);
        gtk_widget_set_tooltip_text(check_purchase, mf2human(title->title));
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON (check_purchase),
                isElem(config->purchaseColumns, title->title));
        g_signal_connect(GTK_TOGGLE_BUTTON(check_purchase), "clicked",
                        G_CALLBACK(purchase_filter_callback), data);
        gtk_box_pack_start(GTK_BOX(widget), check_purchase, 0, 0, 0);
        title = title->next;
    }
}

static GtkWidget* draw_sell_spreadsheet(TakConfig* config, JPK* data) {
    JPKColumns* col = config->sellColumns;
    int length = 0;

    while (col != NULL) {
        length++;
        col = col->next;
    }

    col = config->sellColumns;
    int whichCols[length]; // Zawiera numery kolumn do wyświetlenia w porządku występowania
    JPKColumns* titles;    // w konfiguracji
    int j;
    for (int i = 0; i < length; ++i) {
        j = 0;
        titles = data->colNames;
        while (titles != NULL) {
            if (strcmp(col->title, titles->title) == 0) {
                    whichCols[i] = j;
            }
            j++;
            titles = titles->next;
        }
        col = col->next;
    }

    //Tę tablicę trzeba przesortować
    qsort(whichCols, sizeof(whichCols)/sizeof(*whichCols), sizeof(*whichCols), comp);

    //Na jej podstawie należy przefiltrować dane do tabeli.
    GtkWidget* table_sell = gtk_table_new(length + 2, data->soldCount + 2, FALSE);
    GtkWidget *entry, *button, *hbox_title;
    GtkWidget *hbox;
    JPKChange* change;

    gtk_table_set_homogeneous(GTK_TABLE(table_sell), FALSE);
    char* buffer = (char*)malloc(64);
    char* text_buffer;
    col = config->sellColumns;
    for (int i = 0; i < length + 2; i++) {
        if (i > 1) {
            hbox_title = gtk_hbox_new(0, 0);
            button = gtk_button_new_with_label("×");
            gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Ukryj kolumnę");
            ColFilter* filter = (ColFilter*)malloc(sizeof(ColFilter));
            filter->config = config;
            filter->name = sell_d2m(data, 0, whichCols[i-2]);
            filter->jpk = data;
            gtk_widget_set_size_request(button, 20, 20);
            gtk_box_pack_start(GTK_BOX(hbox_title), button, 0, 0, 0);

            char* aprop_lab = sell_d2m(data, 0, whichCols[i-2]);
            GtkWidget* col_title_label = gtk_label_new(aprop_lab);
            gtk_widget_set_tooltip_text (col_title_label, mf2human(aprop_lab));

            gtk_box_pack_start(GTK_BOX(hbox_title), col_title_label, 1, 1, 0);
            g_signal_connect(GTK_BUTTON(button), "clicked",
                            G_CALLBACK(sell_filter_from_table_callback), filter);
            gtk_table_attach_defaults(GTK_TABLE(table_sell), hbox_title, i, i+1, 0, 1);
        }
        for (int j = 1; j < data->soldCount + 1; j++) {
            if (i == 0) {
                asprintf(&text_buffer, "%d", j);
                gtk_table_attach_defaults (GTK_TABLE(table_sell),
                        gtk_label_new(text_buffer),
                        0, 1, j, j+1);
            } else if (i == 1) {
                hbox = gtk_hbox_new(0, 0);
                button = gtk_button_new_with_label("×");
                gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Usuń wiersz");
                change = (JPKChange*)malloc(sizeof(JPKChange));
                change->i = j;
                change->jpk = data;
                change->tak = config;
                g_signal_connect(GTK_BUTTON(button), "clicked",
                        G_CALLBACK(sell_rmrow_callback), change);
                gtk_widget_set_size_request(button, 20, 20);
                gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
                gtk_table_attach_defaults (GTK_TABLE(table_sell),
                        hbox,
                        1, 2, j, j+1);
            } else {
                sprintf(buffer, "%s", sell_d2m(data, j, whichCols[i-2]+1));
                entry = gtk_entry_new();
                change = (JPKChange*)malloc(sizeof(JPKChange));
                change->i = j;
                change->j = whichCols[i-2]+1;
                change->jpk = data;
                change->tak = config;
                gtk_widget_set_size_request(entry, 50, -1);
                gtk_entry_set_text(GTK_ENTRY(entry), buffer);
                gtk_table_attach_defaults (GTK_TABLE(table_sell),
                        entry,
                        i, i+1, j, j+1);
                g_signal_connect(GTK_ENTRY(entry), "changed",
                        G_CALLBACK(sell_entry_callback), change);
//                g_signal_connect(GTK_ENTRY(entry), "delete_text",
//                        G_CALLBACK(sell_entry_delete_callback), change);
            }
        }
    }
    GdkColor color;
    gdk_color_parse("#A3BE8C", &color);
    GtkWidget *button_add_row = gtk_button_new_with_label("+");
    gtk_widget_modify_bg(GTK_WIDGET(button_add_row), GTK_STATE_NORMAL, &color);
    gtk_widget_set_tooltip_text(GTK_WIDGET(button_add_row), "Dodaj wiersz");
    gtk_table_attach_defaults(GTK_TABLE(table_sell), button_add_row, 0, 2, data->soldCount + 1, data->soldCount + 2);

    change = (JPKChange*)malloc(sizeof(JPKChange));
    change->jpk = data;
    change->tak = config;

    g_signal_connect(GTK_BUTTON(button_add_row), "clicked",
            G_CALLBACK(sell_addrow_callback), change);
    return table_sell;
}

static GtkWidget* draw_pur_spreadsheet(TakConfig* config, JPK* data) {
    JPKColumns* col = config->purchaseColumns;
    int length = 0;

    while (col != NULL) {
        length++;
        col = col->next;
    }

    col = config->purchaseColumns;
    int whichCols[length]; // Zawiera numery kolumn do wyświetlenia w porządku występowania
    JPKColumns* titles;    // w konfiguracji
    int j;
    for (int i = 0; i < length; ++i) {
        j = 0;
        titles = data->colNames;
        while (titles != NULL) {
            if (strcmp(col->title, titles->title) == 0) {
                    whichCols[i] = j;
            }
            j++;
            titles = titles->next;
        }
        col = col->next;
    }

    //Tę tablicę trzeba przesortować
    qsort(whichCols, sizeof(whichCols)/sizeof(*whichCols), sizeof(*whichCols), comp);

    //Na jej podstawie należy przefiltrować dane do tabeli.
    GtkWidget* table_pur = gtk_table_new(length + 2, data->purchaseCount + 2, FALSE);
    GtkWidget *entry, *button, *hbox_title;
    GtkWidget *hbox;
    JPKChange* change;

    gtk_table_set_homogeneous(GTK_TABLE(table_pur), FALSE);
    char* buffer = (char*)malloc(64);
    char* text_buffer;
    col = config->purchaseColumns;
    for (int i = 0; i < length + 2; i++) {
        if (i > 1) {
            hbox_title = gtk_hbox_new(0, 0);
            button = gtk_button_new_with_label("×");
            gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Ukryj kolumnę");
            ColFilter* filter = (ColFilter*)malloc(sizeof(ColFilter));
            filter->config = config;
            filter->name = pur_d2m(data, 0, whichCols[i-2]);
            filter->jpk = data;
            gtk_widget_set_size_request(button, 20, 20);
            gtk_box_pack_start(GTK_BOX(hbox_title), button, 0, 0, 0);

            char* aprop_lab = pur_d2m(data, 0, whichCols[i-2]);
            GtkWidget* col_title_label = gtk_label_new(aprop_lab);
            gtk_widget_set_tooltip_text(col_title_label, mf2human(aprop_lab));

            gtk_box_pack_start(GTK_BOX(hbox_title), col_title_label, 1, 1, 0);
            g_signal_connect(GTK_BUTTON(button), "clicked",
                            G_CALLBACK(pur_filter_from_table_callback), filter);
            gtk_table_attach_defaults(GTK_TABLE(table_pur), hbox_title, i, i+1, 0, 1);
        }
        for (int j = 1; j < data->purchaseCount + 1; j++) {
            if (i == 0) {
                asprintf(&text_buffer, "%d", j);
                gtk_table_attach_defaults(GTK_TABLE(table_pur),
                        gtk_label_new(text_buffer),
                        0, 1, j, j+1);
            } else if (i == 1) {
                hbox = gtk_hbox_new(0, 0);
                button = gtk_button_new_with_label("×");
                gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Usuń wiersz");
                change = (JPKChange*)malloc(sizeof(JPKChange));
                change->i = j;
                change->jpk = data;
                change->tak = config;
                g_signal_connect(GTK_BUTTON(button), "clicked",
                        G_CALLBACK(pur_rmrow_callback), change);
                gtk_widget_set_size_request(button, 20, 20);
                gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
                gtk_table_attach_defaults (GTK_TABLE(table_pur),
                        hbox,
                        1, 2, j, j+1);
            } else {
                sprintf(buffer, "%s", pur_d2m(data, j, whichCols[i-2]+1));
                entry = gtk_entry_new();
                change = (JPKChange*)malloc(sizeof(JPKChange));
                change->i = j;
                change->j = whichCols[i-2]+1;
                change->jpk = data;
                change->tak = config;
                gtk_widget_set_size_request(entry, 50, -1);
                gtk_entry_set_text(GTK_ENTRY(entry), buffer);
                gtk_table_attach_defaults(GTK_TABLE(table_pur),
                        entry,
                        i, i+1, j, j+1);
                g_signal_connect(GTK_ENTRY(entry), "changed",
                        G_CALLBACK(pur_entry_callback), change);
            }
        }
    }
    GdkColor color;
    gdk_color_parse("#A3BE8C", &color);
    GtkWidget *button_add_row = gtk_button_new_with_label("+");
    gtk_widget_modify_bg(GTK_WIDGET(button_add_row), GTK_STATE_NORMAL, &color);
    gtk_widget_set_tooltip_text(GTK_WIDGET(button_add_row), "Dodaj wiersz");
    gtk_table_attach_defaults(GTK_TABLE(table_pur), button_add_row, 0, 2, data->purchaseCount + 1, data->purchaseCount + 2);

    change = (JPKChange*)malloc(sizeof(JPKChange));
    change->jpk = data;
    change->tak = config;

    g_signal_connect(GTK_BUTTON(button_add_row), "clicked",
            G_CALLBACK(pur_addrow_callback), change);
    return table_pur;
}

/*static void open_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Wybierz plik", GTK_WINDOW(widget),
            GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
            GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        g_print("%s\n", gtk_file_chooser_get_filename(
                    GTK_FILE_CHOOSER(dialog)));
    } else {
        g_print("Naciśnięto anuluj\n");
    }
    gtk_widget_destroy(dialog);
}

static void save_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Wybierz plik", GTK_WINDOW(widget),
            GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE,
            GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_ACCEPT) {
        g_print("%s\n", gtk_file_chooser_get_filename(
                    GTK_FILE_CHOOSER(dialog)));
    } else {
        g_print("Naciśnięto anuluj\n");
    }
    gtk_widget_destroy(dialog);
}*/

static void about_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *label;
    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "O programie");
    gtk_container_border_width(GTK_CONTAINER(dialog), 5);
    label = gtk_label_new("O problemach i błędach proszę pisać na adres");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 0);
    label = gtk_label_new("<a href=\"mailto:rafal.szczerski@gmail.com\">rafal.szczerski@gmail.com</a>");
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_padding(GTK_MISC(label), 10, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, 0, 0, 10);
    gtk_widget_show_all(dialog);
    gtk_grab_add(dialog);
}

static GtkWidget* create_menu_bar(JPK* jpk, TakConfig* config, GtkWidget* window) {
    GtkWidget* menu_bar = gtk_menu_bar_new();
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* help_menu = gtk_menu_new();
    GtkWidget* menu_item;

    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    menu_item = gtk_image_menu_item_new_with_mnemonic("_Rejestr");
    GtkWidget *img = gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

    menu_item = gtk_image_menu_item_new_with_label("Pomoc");
    img = gtk_image_new_from_stock(GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

    menu_item = gtk_image_menu_item_new_with_label("Nowy");
    img = gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(new_file_callback), menu_bar);
    gtk_widget_add_accelerator(menu_item, "activate", accel_group,
            0x06e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    menu_item = gtk_image_menu_item_new_with_label("Otwórz");
    img = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_open_dialog), menu_bar);
    gtk_widget_add_accelerator(menu_item, "activate", accel_group,
            0x06f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

//    menu_item = gtk_menu_item_new_with_label("Importuj xls");
//    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
//    g_signal_connect(menu_item, "activate", G_CALLBACK(open_dialog), NULL);

//    menu_item = gtk_menu_item_new_with_label("Importuj ods");
//    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
//    g_signal_connect(menu_item, "activate", G_CALLBACK(open_dialog), NULL);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    JPKChange *change = (JPKChange*)malloc(sizeof(JPKChange));
    change->tak = config;
    change->jpk = jpk;

    menu_item = gtk_image_menu_item_new_with_label("Zapisz");
    img = gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(savecsv_dialog), change);
    gtk_widget_add_accelerator(menu_item, "activate", accel_group,
            0x073, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    menu_item = gtk_image_menu_item_new_with_label("Zapisz jako");
    img = gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(savecsv_as_dialog), change);
    gtk_widget_add_accelerator(menu_item, "activate", accel_group,
            0x073, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);



    History* cur = open_history;
    if (cur != NULL && !history_isEmpty(cur)) {
        OpenCallbackData* ocd = (OpenCallbackData*)malloc(sizeof(OpenCallbackData));
        ocd->config = config;
        ocd->jpk = jpk;
        ocd->parent = menu_bar;
        menu_item = gtk_separator_menu_item_new();

        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

        ocd->data = strdup(cur->path); 
        menu_item = gtk_menu_item_new_with_label(g_path_get_basename(cur->path));
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
        g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_lastopen_dialog), ocd);
        cur = cur->next;
    }
    if (cur != NULL && !history_isEmpty(cur)) {    
        OpenCallbackData* ocd = (OpenCallbackData*)malloc(sizeof(OpenCallbackData));
        ocd->config = config;
        ocd->jpk = jpk;
        ocd->parent = menu_bar;
        ocd->data = strdup(cur->path); 

        menu_item = gtk_menu_item_new_with_label(g_path_get_basename(cur->path));
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
        g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_lastopen_dialog), ocd);
        cur = cur->next;
    }
    if (cur != NULL && !history_isEmpty(cur)) {    
        OpenCallbackData* ocd = (OpenCallbackData*)malloc(sizeof(OpenCallbackData));
        ocd->config = config;
        ocd->jpk = jpk;
        ocd->parent = menu_bar;
        ocd->data = strdup(cur->path); 

        menu_item = gtk_menu_item_new_with_label(g_path_get_basename(cur->path));
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
        g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_lastopen_dialog), ocd);
        cur = cur->next;
    }
    if (cur != NULL && !history_isEmpty(cur)) {    
        OpenCallbackData* ocd = (OpenCallbackData*)malloc(sizeof(OpenCallbackData));
        ocd->config = config;
        ocd->jpk = jpk;
        ocd->parent = menu_bar;
        ocd->data = strdup(cur->path); 

        menu_item = gtk_menu_item_new_with_label(g_path_get_basename(cur->path));
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
        g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_lastopen_dialog), ocd);
        cur = cur->next;
    }
    if (cur != NULL && !history_isEmpty(cur)) {    
        OpenCallbackData* ocd = (OpenCallbackData*)malloc(sizeof(OpenCallbackData));
        ocd->config = config;
        ocd->jpk = jpk;
        ocd->parent = menu_bar;
        ocd->data = strdup(cur->path); 

        menu_item = gtk_menu_item_new_with_label(g_path_get_basename(cur->path));
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
        g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_lastopen_dialog), ocd);
    }
    
    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    menu_item = gtk_image_menu_item_new_with_label("Wyjście");
    img = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_add_accelerator(menu_item, "activate", accel_group,
            0x071, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    menu_item = gtk_image_menu_item_new_with_label("O programie");
    img = gtk_image_new_from_stock(GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), img);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(about_dialog), NULL);
    return menu_bar;
}

void refreshSellSum(JPK* jpk, TakConfig* config) {
    char* buffer;
    asprintf(&buffer, "%s: %.2lf %s", "Podatek należny", jpk->soldTotal, config->DomyslnyKodWaluty);
    gtk_label_set_text(GTK_LABEL(label_sell_sum), buffer);
}

void refreshPurSum(JPK* jpk, TakConfig* config) {
    char* buffer;
    asprintf(&buffer, "%s: %.2lf %s", "Podatek naliczony", jpk->purchaseTotal, config->DomyslnyKodWaluty);
    gtk_label_set_text(GTK_LABEL(label_pur_sum), buffer);
}

static void create_sell_notebook(GtkWidget *notebook, JPK* jpk, TakConfig* config) {
    GtkWidget* label_tab = gtk_label_new("Sprzedaże");
    GtkWidget* hbox_sell = gtk_hbox_new(0, 0);
    GtkWidget* vbox_spread = gtk_vbox_new(0, 0);
    GtkWidget* vbox_col_sell = gtk_vbox_new(0, 0);
    GtkWidget* vbox_meta = gtk_vbox_new(0, 0);
    create_sell_col_filter(vbox_col_sell, jpk, config);
    GtkWidget* scroll_col_sell = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_col_sell),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_col_sell), vbox_col_sell);
    gtk_widget_set_size_request(scroll_col_sell, 180, 480);
    GtkWidget* scroll_sell = gtk_scrolled_window_new(NULL, NULL);
    // Pseudoarkusz
    GtkWidget* table_sell = draw_sell_spreadsheet(config, jpk);
    gtk_table_set_row_spacings(GTK_TABLE(table_sell), 1);
    gtk_table_set_col_spacings(GTK_TABLE(table_sell), 1);
    char* buffer;
    asprintf(&buffer, "%s: %.2lf %s", "Podatek należny", jpk->soldTotal, config->DomyslnyKodWaluty);
    label_sell_sum = gtk_label_new(buffer);
    gtk_widget_set_tooltip_text(label_sell_sum, mf2human("PodatekNalezny"));
    GtkWidget *hbox_space = gtk_hbox_new(0, 0);
    GtkWidget *hbox = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), table_sell, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), hbox, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), hbox_space, 0, 1, 0);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_sell), vbox_spread);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_sell),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start(GTK_BOX(vbox_meta), scroll_sell, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox_meta), label_sell_sum, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_sell), vbox_meta, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_sell), scroll_col_sell, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_sell, label_tab);
    gtk_widget_show(notebook);
}

static void create_purchase_notebook(GtkWidget *notebook, JPK* jpk, TakConfig* config) {
    GtkWidget* label_tab = gtk_label_new("Zakupy");
    GtkWidget* hbox_pur = gtk_hbox_new(0, 0);
    GtkWidget* vbox_spread = gtk_vbox_new(0, 0);
    GtkWidget* vbox_col_pur = gtk_vbox_new(0, 0);
    GtkWidget* vbox_meta = gtk_vbox_new(0, 0);
    create_pur_col_filter(vbox_col_pur, jpk, config);
    GtkWidget* scroll_col_pur = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_col_pur),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_col_pur), vbox_col_pur);
    gtk_widget_set_size_request(scroll_col_pur, 180, 480);
    GtkWidget* scroll_pur = gtk_scrolled_window_new(NULL, NULL);
    // Pseudoarkusz
    GtkWidget* table_pur = draw_pur_spreadsheet(config, jpk);
    gtk_table_set_row_spacings(GTK_TABLE(table_pur), 1);
    gtk_table_set_col_spacings(GTK_TABLE(table_pur), 1);
    char* buffer;
    asprintf(&buffer, "%s: %.2lf %s", "Podatek naliczony", jpk->purchaseTotal, config->DomyslnyKodWaluty);
    label_pur_sum = gtk_label_new(buffer);
    gtk_widget_set_tooltip_text(label_pur_sum, mf2human("PodatekNaliczony"));
    GtkWidget *hbox_space = gtk_hbox_new(0, 0);
    GtkWidget *hbox = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), table_pur, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), hbox, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), hbox_space, 0, 1, 0);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_pur), vbox_spread);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_pur),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start(GTK_BOX(vbox_meta), scroll_pur, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox_meta), label_pur_sum, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_pur), vbox_meta, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_pur), scroll_col_pur, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_pur, label_tab);
    gtk_widget_show(notebook);

/*    GtkWidget *label_tab = gtk_label_new("Zakupy");
    GtkWidget* label = gtk_label_new("To jest przykladowy tekst 2 ");
    GtkWidget* hbox_purchase = gtk_hbox_new(0, 0);
    GtkWidget* vbox_col_pur = gtk_vbox_new(0, 0);
    GtkWidget* scroll_col_pur = gtk_scrolled_window_new (NULL, NULL);
    create_pur_col_filter(vbox_col_pur, jpk, config);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_col_pur),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_col_pur), vbox_col_pur);
    gtk_widget_set_size_request(scroll_col_pur, 180, 480);
    gtk_box_pack_start(GTK_BOX(hbox_purchase), label, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_purchase), scroll_col_pur, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_purchase, label_tab);*/
}

static void create_profile_notebook(GtkWidget *notebook, JPK* jpk, TakConfig* config) {
    GtkWidget *hbox_profile = gtk_hbox_new(0, 0);
    GtkWidget* scroll_profile = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *label_tab = gtk_label_new("Firma");
    GtkWidget *label_profile;
    GtkWidget* table_profile = gtk_table_new(10, 2, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE(table_profile), 20);
    GtkWidget *entry;

    JPKChange* change = (JPKChange*)malloc(sizeof(JPKChange));
    change->jpk = jpk;
    change->tak = config;

    label_profile = gtk_label_new("Waluta");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 0, 1);
    entry = gtk_entry_new();
    if (config->DomyslnyKodWaluty == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->DomyslnyKodWaluty);
    }
    gtk_widget_set_size_request(entry, 400, 30);
    g_signal_connect(entry, "changed", G_CALLBACK(waluta_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 0, 1);

    label_profile = gtk_label_new("Urząd skarbowy");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 1, 2);

    GtkWidget *combo = gtk_combo_new();
    gtk_combo_set_popdown_strings (GTK_COMBO(combo), appendUSCodes(combo));

    Completion* compl = (Completion*)malloc(sizeof(Completion));
    compl->combo = combo;
    compl->table = table_profile;
    compl->config = config;
    compl->jpk = jpk;
    if (config->KodUrzedu == NULL) {
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), config->KodUrzedu);
    }
    g_signal_connect(GTK_ENTRY(GTK_COMBO(combo)->entry), "activate", G_CALLBACK(uscode_callback), compl);
    //g_signal_connect(GTK_COMBO(combo), "activate", G_CALLBACK(uscode_callback), compl);
    gtk_table_attach_defaults(GTK_TABLE(table_profile), combo, 1, 2, 1, 2);

    label_profile = gtk_label_new("NIP firmy");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 2, 3);
    entry = gtk_entry_new();
    if (config->NIP == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->NIP);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(nip_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 2, 3);

    label_profile = gtk_label_new("Pełna nazwa");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 3, 4);
    entry = gtk_entry_new();
    if (config->PelnaNazwa == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->PelnaNazwa);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(pelnanazwa_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 3, 4);

    label_profile = gtk_label_new("REGON");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 4, 5);
    entry = gtk_entry_new();
    if (config->REGON == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->REGON);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(regon_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 4, 5);

    label_profile = gtk_label_new("Kraj");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 5, 6);
    entry = gtk_entry_new();
    if (config->KodKraju == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->KodKraju);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(kraj_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 5, 6);

    label_profile = gtk_label_new("Województwo");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 6, 7);
    entry = gtk_entry_new();
    if (config->Wojewodztwo == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Wojewodztwo);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(wojewodztwo_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 6, 7);

    label_profile = gtk_label_new("Powiat");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 7, 8);
    entry = gtk_entry_new();
    if (config->Powiat == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Powiat);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(powiat_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 7, 8);

    label_profile = gtk_label_new("Gmina");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 8, 9);
    entry = gtk_entry_new();
    if (config->Gmina == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Gmina);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(gmina_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 8, 9);

    label_profile = gtk_label_new("Ulica");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 9, 10);
    entry = gtk_entry_new();
    if (config->Ulica == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Ulica);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(ulica_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 9, 10);

    label_profile = gtk_label_new("Numer domu");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 10, 11);
    entry = gtk_entry_new();
    if (config->NrDomu == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->NrDomu);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(nrdomu_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 10, 11);

    label_profile = gtk_label_new("Numer lokalu");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 11, 12);
    entry = gtk_entry_new();
    if (config->NrLokalu == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->NrLokalu);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(nrlokalu_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 11, 12);

    label_profile = gtk_label_new("Miejcowość");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 12, 13);
    entry = gtk_entry_new();
    if (config->Miejscowosc == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Miejscowosc);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(miejscowosc_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 12, 13);

    label_profile = gtk_label_new("Kod pocztowy");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 13, 14);
    entry = gtk_entry_new();
    if (config->KodPocztowy == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->KodPocztowy);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(kodpocztowy_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 13, 14);

    label_profile = gtk_label_new("Poczta");
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            label_profile,
            0, 1, 14, 15);
    entry = gtk_entry_new();
    if (config->Poczta == NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), config->Poczta);
    }
    g_signal_connect(entry, "changed", G_CALLBACK(poczta_callback), change);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 14, 15);

    gtk_box_pack_start(GTK_BOX(hbox_profile), table_profile, 1, 0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_profile),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_profile), hbox_profile);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll_profile, label_tab);
}

static GtkWidget* create_notebooks(JPK* jpk, TakConfig* config) {
    GtkWidget *notebook = gtk_notebook_new();

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    create_sell_notebook(notebook, jpk, config);
    create_purchase_notebook(notebook, jpk, config);
    create_profile_notebook(notebook, jpk, config);
    return notebook;
}

void month_callback(GtkWidget* widget, gpointer data) {
    JPK* jpk = (JPK*)data;
    char* label = (char*)gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    char* months[12] = {"Styczeń", "Luty", "Marzec", "Kwiecień", "Maj",
        "Czerwiec", "Lipiec", "Sierpień", "Wrzesień", "Październik",
        "Listopad", "Grudzień"};

    int month = 1;
    while (strcmp(label, months[month-1]) != 0) ++month;

    char* year = (char*)gtk_entry_get_text(GTK_ENTRY(entry_year));
    char *start_date, *end_date;
    if (month < 9) {
        asprintf(&start_date, "%s-0%d-%s", year, month, "01");
        asprintf(&end_date, "%s-0%d-%d", year, month, getLastDayOfMonth(month, atoi(year)));
    } else  {
        asprintf(&start_date, "%s-%d-%s", year, month, "01");
        asprintf(&end_date, "%s-%d-%d", year, month, getLastDayOfMonth(month, atoi(year)));
    }
    default_month = month;
    jpk->header->dataOd = start_date;
    jpk->header->dataDo = end_date;
}

void year_callback(GtkWidget* widget, gpointer data) {
    JPK* jpk = (JPK*)data;
    char* year = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    int month = default_month;

    char *start_date, *end_date;
    if (month < 9) {
        asprintf(&start_date, "%s-0%d-%s", year, month, "01");
        asprintf(&end_date, "%s-0%d-%d", year, month, getLastDayOfMonth(month, atoi(year)));
    } else  {
        asprintf(&start_date, "%s-%d-%s", year, month, "01");
        asprintf(&end_date, "%s-%d-%d", year, month, getLastDayOfMonth(month, atoi(year)));
    }

//    entry_year = widget;
    jpk->header->dataOd = start_date;
    jpk->header->dataOd = start_date;
    jpk->header->dataDo = end_date;
}

static GtkWidget* create_date_menu(JPK *jpk) {
    GtkWidget* hbox_date = gtk_hbox_new(0, 0);
    GtkWidget* opt_menu = gtk_option_menu_new();
    gtk_widget_set_size_request(opt_menu, 142, -1);
    date_menu = gtk_menu_new();
    GtkWidget* item;
    for (int i = 0; i < 12; ++i) {
        item = gtk_menu_item_new_with_label(getMonthName(getJPKMonth(jpk) + i));
        gtk_widget_show(item);
        gtk_menu_shell_append(GTK_MENU_SHELL(date_menu), item);
        if (i == 0) {
            gtk_menu_set_active(GTK_MENU(date_menu), i);
            default_month = i + 1;
        }
        g_signal_connect(item, "activate", G_CALLBACK(month_callback), jpk);
    }

    char* year = getJPKYear(jpk);

    //if (getMonth(date) == 1) asprintf(&year, "%d", atoi(date->year) - 1);
    //else asprintf(&year, "%d", atoi(date->year));

    GtkWidget* hbox_space = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_date), hbox_space, 1, 1, 0);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(opt_menu), date_menu);
    GtkWidget* label = gtk_label_new("Za: ");
    gtk_box_pack_start(GTK_BOX(hbox_date), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_date), opt_menu, 0, 0, 0);

    gtk_widget_show(opt_menu);
    entry_year = gtk_entry_new();

    gtk_widget_set_size_request(entry_year, 42, -1);
    gtk_entry_set_alignment(GTK_ENTRY(entry_year), 1);
    gtk_entry_set_max_length(GTK_ENTRY(entry_year), 4);
    gtk_entry_set_text(GTK_ENTRY(entry_year), year);
    gtk_box_pack_start(GTK_BOX(hbox_date), entry_year, 0, 0, 0);
    g_signal_connect(entry_year, "changed", G_CALLBACK(year_callback), jpk);

    return hbox_date;
}

static void aim_first_callback(GtkWidget* widget, gpointer data) {
    JPK* jpk = (JPK*)data;
    jpk->header->celZlozenia = 1;
}

static void aim_second_callback(GtkWidget* widget, gpointer data) {
    JPK* jpk = (JPK*)data;
    jpk->header->celZlozenia = 2;
}

static void aim_third_callback(GtkWidget* widget, gpointer data) {
    JPK* jpk = (JPK*)data;
    jpk->header->celZlozenia = 3;
}

static GtkWidget* create_box_bottom(JPK* jpk) {
    GtkWidget* hbox_bottom = gtk_hbox_new(0, 15);
    GtkWidget* radio_aim_gr = gtk_radio_button_new_with_label(NULL,
            "Złożenie po raz pierwszy");
    if (jpk->header->celZlozenia == 1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_aim_gr), 1);
    g_signal_connect(radio_aim_gr, "toggled", G_CALLBACK(aim_first_callback), jpk);
    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim_gr, 0, 1, 0);

    GtkWidget* radio_aim = gtk_radio_button_new_with_label(gtk_radio_button_get_group(
          GTK_RADIO_BUTTON(radio_aim_gr)), "Pierwsza korekta");
    if (jpk->header->celZlozenia == 2)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_aim), 1);
    g_signal_connect(radio_aim_gr, "toggled", G_CALLBACK(aim_second_callback), jpk);
    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim, 0, 1, 0);

    radio_aim = gtk_radio_button_new_with_label(gtk_radio_button_get_group(
          GTK_RADIO_BUTTON(radio_aim_gr)), "Druga korekta");
    if (jpk->header->celZlozenia == 3)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_aim), 1);
    g_signal_connect(radio_aim_gr, "toggled", G_CALLBACK(aim_third_callback), jpk);

    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim, 0, 1, 0);

    gtk_box_pack_start(GTK_BOX(hbox_bottom), create_date_menu(jpk), 1, 1, 0);
    return hbox_bottom;
}

void drawGui(JPK* jpk) {
    TakConfig* config = parseConfig();
    open_history = loadHistory(HISTORY_OPEN_FILENAME);

    gtk_init(NULL, NULL);
    GtkWidget *window, *vbox;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
    vbox = gtk_vbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(jpk, config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_set_size_request(window, 800, 600);
    gtk_widget_show_all(window);
    gtk_main();
}

static void sell_filter_from_table_callback(GtkWidget* widget, gpointer data) {
    ColFilter* t = (ColFilter*) data;
    char* colName = t->name;

    rmColumn(&(t->config->sellColumns), colName);
    saveConfig(t->config);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(t->jpk, t->config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(t->jpk, t->config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(t->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

static void pur_filter_from_table_callback(GtkWidget* widget, gpointer data) {
    ColFilter* t = (ColFilter*) data;
    char* colName = t->name;

    rmColumn(&(t->config->purchaseColumns), colName);
    saveConfig(t->config);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);
    GtkWidget* notebook = create_notebooks(t->jpk, t->config);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(t->jpk, t->config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook,1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(t->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
}

static void sell_filter_callback(GtkWidget* widget, gpointer data) {
    ColFilter* t = (ColFilter*) data;
    char* colName = t->name;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        if (t->config->sellColumns == NULL) {
            t->config->sellColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
            t->config->sellColumns->title = NULL;
            t->config->sellColumns->next = NULL;
        }
        addColumn(t->config->sellColumns, colName);
        saveConfig(t->config);
    } else {
        rmColumn(&(t->config->sellColumns), colName);
        saveConfig(t->config);
    }

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(t->jpk, t->config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(t->jpk, t->config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(t->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

static void purchase_filter_callback(GtkWidget* widget, gpointer data) {
    ColFilter* t = (ColFilter*) data;
    char* colName = t->name;
    TakConfig *config = t->config;
    JPK* jpk = t->jpk;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        if (config->purchaseColumns == NULL) {
            config->purchaseColumns = (JPKColumns*)malloc(sizeof(JPKColumns));
            config->purchaseColumns->title = NULL;
            config->purchaseColumns->next = NULL;
        }
        addColumn(config->purchaseColumns, colName);
        saveConfig(config);
    } else {
        rmColumn(&(config->purchaseColumns), colName);
        saveConfig(config);
    }

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);
    GtkWidget *notebook = create_notebooks(jpk, config);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(t->jpk, t->config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(t->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
}

static void sell_rmrow_callback(GtkWidget* widget, gpointer data) {
    JPKChange* change = (JPKChange*)data;
    rmSellRow(change->jpk, change->i);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(change->jpk, change->tak, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(change->jpk, change->tak), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(change->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

static void pur_rmrow_callback(GtkWidget* widget, gpointer data) {
    JPKChange* change = (JPKChange*)data;
    rmPurchaseRow(change->jpk, change->i);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    GtkWidget* notebook = create_notebooks(change->jpk, change->tak);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(change->jpk, change->tak, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(change->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
}
/*
static void sell_entry_insert_callback(GtkEntry* widget,
                                       const gchar *text,
                                       gint length,
                                       gint *position,
                                       gpointer data) {
    GtkEditable* entry = GTK_EDITABLE(widget);
    JPKChange* change = (JPKChange*)data;
    char* entry_data = (char*)gtk_editable_get_chars(entry, 0, -1);

    char *p = (char*)text;
    while (*p != '\0') {
        if (*p == ';') text = "";
        p++;
    }

    char* in = (char*)text;
    int in_len = strlen(text);
    int old_len = strlen(entry_data);
    int new_len = old_len + in_len;
    int offset = *position;

    char* buffer = (char*)malloc(new_len + 1);
    memcpy(buffer, entry_data, offset);
    memcpy(buffer + offset, in, in_len);
    memcpy(buffer + offset + in_len, entry_data + offset, old_len - offset);
    buffer[new_len] = '\0';

    char* safe_input = sanitizeSellEntry(change->i, change->j, buffer);
    int safe_len = strlen(safe_input);
    changeSellData(change->jpk, change->i, change->j, safe_input);
    refreshSellSum(change->jpk, change->tak);

    g_signal_handlers_block_by_func(G_OBJECT(entry), G_CALLBACK(sell_entry_insert_callback), data);
    gtk_editable_delete_text(entry, 0, -1);
    gtk_editable_insert_text(entry, safe_input, safe_len, position);
    g_signal_handlers_unblock_by_func(G_OBJECT(entry), G_CALLBACK(sell_entry_insert_callback), data);
    g_signal_stop_emission_by_name(G_OBJECT(entry), "insert_text");
    g_free(buffer);
}

static void sell_entry_delete_callback(GtkEntry* widget,
                                       const gchar *text,
                                       gint length,
                                       gint *position,
                                       gpointer data) {
    GtkEditable* entry = GTK_EDITABLE(widget);
    JPKChange* change = (JPKChange*)data;
    gchar* entry_data = gtk_editable_get_chars(entry, 0, -1);
    char* safe_input = sanitizeSellEntry(change->i, change->j, entry_data);

    changeSellData(change->jpk, change->i, change->j, safe_input);
    refreshSellSum(change->jpk, change->tak);
    g_signal_handlers_block_by_func(G_OBJECT(entry), G_CALLBACK(sell_entry_delete_callback), data);
    gtk_editable_delete_text(entry, 0, -1);
    gint new_position = gtk_editable_get_position(entry);
    gtk_editable_insert_text(entry, safe_input, strlen(safe_input), &new_position);
    g_signal_handlers_unblock_by_func(G_OBJECT(entry), G_CALLBACK(sell_entry_delete_callback), data);
    g_signal_stop_emission_by_name(G_OBJECT(entry), "delete_text");
}
*/

static void sell_entry_callback(GtkWidget* widget, gpointer data) {
    JPKChange* change = (JPKChange*)data;
    char* input = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    changeSellData(change->jpk, change->i, change->j, input);
    refreshSellSum(change->jpk, change->tak);
//    gtk_entry_set_text(GTK_ENTRY(widget), sell_d2m(change->jpk, change->i, change->j));
}

static void pur_entry_callback(GtkWidget* widget, gpointer data) {
    JPKChange* change = (JPKChange*)data;
    char* input = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    changePurData(change->jpk, change->i, change->j, input);
    refreshPurSum(change->jpk, change->tak);
//    gtk_entry_set_text(GTK_ENTRY(widget), sell_d2m(change->jpk, change->i, change->j));
}
static void new_file_callback(GtkWidget* widget, gpointer data) {
        JPK* jpk = newJPK();

        TakConfig* config = getConfig(jpk);
        GtkWidget* menu = GTK_WIDGET(data);
        GtkWidget* root_box = menu->parent;
        GtkWidget *window = gtk_widget_get_toplevel(menu);

        gtk_widget_destroy(root_box);
        GtkWidget *vbox = gtk_vbox_new(0, 0);

        gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(jpk, config, window), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(jpk), 0, 0, 0);
        gtk_container_add(GTK_CONTAINER(window), vbox);
        gtk_widget_show_all(window);
}

static void sell_addrow_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*) data;
    addSellRow(ch->jpk);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(ch->jpk, ch->tak, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(ch->jpk, ch->tak), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(ch->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

static void pur_addrow_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*) data;
    addPurchaseRow(ch->jpk);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);
    GtkWidget* notebook = create_notebooks(ch->jpk, ch->tak);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(ch->jpk, ch->tak, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(ch->jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
}

void importcsv_open_dialog(GtkWidget* widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Wybierz plik", GTK_WINDOW(widget),
            GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
            GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

    if (resp == GTK_RESPONSE_OK) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        //char* filename = g_path_get_basename(path);
        JPK* jpk = loadJPK(path);

        TakConfig* config = getConfig(jpk);
        GtkWidget* menu = GTK_WIDGET(data);
        GtkWidget* root_box = menu->parent;
        GtkWidget *window = gtk_widget_get_toplevel(menu);

        gtk_widget_destroy(root_box);
        GtkWidget *vbox = gtk_vbox_new(0, 0);

        gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(jpk, config, window), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(jpk), 0, 0, 0);
        gtk_container_add(GTK_CONTAINER(window), vbox);
        gtk_widget_show_all(window);

        open_history = addHistory(&open_history, path);
        saveHistory(open_history, HISTORY_OPEN_FILENAME);
    }
    gtk_widget_destroy(dialog);
}

void importcsv_lastopen_dialog(GtkWidget* widget, gpointer data) {
    OpenCallbackData* ocd = (OpenCallbackData*)data;
    char* path = strdup(ocd->data);
    JPK* jpk = loadJPK(path);

    open_history = addHistory(&open_history, path);
    saveHistory(open_history, HISTORY_OPEN_FILENAME);

    TakConfig* config = getConfig(jpk);
    GtkWidget* menu = GTK_WIDGET(ocd->parent);
    GtkWidget* root_box = menu->parent;
    GtkWidget *window = gtk_widget_get_toplevel(menu);

    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(jpk, config, window), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(jpk), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

void savecsv_dialog(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    JPK* jpk = ch->jpk;
    TakConfig *config = ch->tak;
    if (open_history->path != NULL) {
        csvExport(open_history->path, configToJPK(jpk, config));
    }
}

void savecsv_as_dialog(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*)data;
    JPK* jpk = ch->jpk;
    TakConfig *config = ch->tak;

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Wybierz plik", GTK_WINDOW(widget),
            GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE,
            GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_ACCEPT) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
//        char* filename = g_path_get_basename(path);
        csvExport(path, configToJPK(jpk, config));

        open_history = addHistory(&open_history, path);
        saveHistory(open_history, HISTORY_OPEN_FILENAME);
    }
    gtk_widget_destroy(dialog);
}

