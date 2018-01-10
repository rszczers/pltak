#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include "jpk.h"
#include "config.h"
#include "utils.h"

GtkWidget *label_sum;

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
static void sell_rmrow_callback(GtkWidget*, gpointer);
static void sell_entry_callback(GtkWidget*, gpointer);
static void sell_addrow_callback(GtkWidget*, gpointer);
static void importcsv_open_dialog(GtkWidget*, gpointer);

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
    TakConfig* config = compl->config;

    char* filter;
    asprintf(&filter, "%s", (char *)gtk_entry_get_text(GTK_ENTRY(entry)));
    for (int i = 0; i < strlen(filter); ++i) {
       filter[i] = toupper(filter[i]);
    }
    gtk_widget_destroy(combo);

    GList *cbitems = NULL;
    GtkWidget* newCombo = gtk_combo_new();
    // Dodaj kody wg. filtra
    USList* codes = loadUSCodes();

    while (codes != NULL) {
        if (strstr(codes->name, filter) != NULL) {
            char* s;
            asprintf(&s, "%s, %s", codes->code, codes->name);
            cbitems = g_list_append(cbitems, s);
        }
        codes = codes->next;
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
    newData->combo = newCombo;
    newData->table = table;
    newData->config = config;
    g_signal_connect(GTK_ENTRY(GTK_COMBO(newCombo)->entry), "activate", G_CALLBACK(uscode_callback), newData);
//    g_signal_connect(GTK_COMBO(newCombo), "activate", G_CALLBACK(uscode_callback), newData);
}

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
    TakConfig* config = (TakConfig*) data;
    config->DomyslnyKodWaluty = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void nip_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->NIP = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void pelnanazwa_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->PelnaNazwa = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void regon_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->REGON = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void kraj_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->KodKraju = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void wojewodztwo_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Wojewodztwo = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void powiat_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Powiat = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void gmina_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Gmina = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void ulica_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Ulica = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void nrdomu_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->NrDomu = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void nrlokalu_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->NrLokalu = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void miejscowosc_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Miejscowosc = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void kodpocztowy_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->KodPocztowy = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    saveConfig(config);
}
static void poczta_callback(GtkWidget* widget, gpointer data) {
    TakConfig* config = (TakConfig*) data;
    config->Poczta = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
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
            ColFilter* filter = (ColFilter*)malloc(sizeof(ColFilter));
            filter->config = config;
            filter->name = sell_d2m(data, 0, whichCols[i-2]);
            filter->jpk = data;
            gtk_widget_set_size_request(button, 20, 20);
            gtk_box_pack_start(GTK_BOX(hbox_title), button, 0, 0, 0);
            gtk_box_pack_start(GTK_BOX(hbox_title), gtk_label_new(sell_d2m(data, 0, whichCols[i-2])), 1, 1, 0);
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

            }
        }
    }
    GdkColor color;
    gdk_color_parse("#A3BE8C", &color);
    GtkWidget *button_add_row = gtk_button_new_with_label("+");
    gtk_widget_modify_bg(GTK_WIDGET(button_add_row), GTK_STATE_NORMAL, &color);
    gtk_table_attach_defaults(GTK_TABLE(table_sell), button_add_row, 0, 2, data->soldCount + 1, data->soldCount + 2);

    change = (JPKChange*)malloc(sizeof(JPKChange));
    change->jpk = data;
    change->tak = config;

    g_signal_connect(GTK_BUTTON(button_add_row), "clicked",
            G_CALLBACK(sell_addrow_callback), change);
    return table_sell;
}

static void open_dialog(GtkWidget *widget, gpointer data) {
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
    if (resp == GTK_RESPONSE_OK) {
        g_print("%s\n", gtk_file_chooser_get_filename(
                    GTK_FILE_CHOOSER(dialog)));
    } else {
        g_print("Naciśnięto anuluj\n");
    }
    gtk_widget_destroy(dialog);
}

static void about_dialog(GtkWidget *widget, gpointer data) {}

static GtkWidget* create_menu_bar() {
    GtkWidget* menu_bar = gtk_menu_bar_new();
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* help_menu = gtk_menu_new();
    GtkWidget* menu_item;

    menu_item = gtk_menu_item_new_with_label("Rejestr");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

    menu_item = gtk_menu_item_new_with_label("Pomoc");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

    menu_item = gtk_menu_item_new_with_label("Importuj csv");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(importcsv_open_dialog), menu_bar);

    menu_item = gtk_menu_item_new_with_label("Importuj xls");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(open_dialog), NULL);

    menu_item = gtk_menu_item_new_with_label("Importuj ods");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(open_dialog), NULL);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Eksportuj csv");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(save_dialog), NULL);

    menu_item = gtk_menu_item_new_with_label("Eksportuj xml");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(save_dialog), NULL);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Wyjście");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(gtk_main_quit), NULL);

    menu_item = gtk_menu_item_new_with_label("O programie");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(about_dialog), NULL);
    return menu_bar;
}

void refreshSellSum(JPK* jpk, TakConfig* config) {
    char* buffer;
    asprintf(&buffer, "%s: %.2lf %s", "Podatek należny", jpk->soldTotal, config->DomyslnyKodWaluty);
    gtk_label_set_text(GTK_LABEL(label_sum), buffer);
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
    label_sum = gtk_label_new(buffer);
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
    gtk_box_pack_start(GTK_BOX(vbox_meta), label_sum, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_sell), vbox_meta, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_sell), scroll_col_sell, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_sell, label_tab);
    gtk_widget_show(notebook);
}

static void create_purchase_notebook(GtkWidget *notebook, JPK* jpk, TakConfig* config) {
    GtkWidget *label_tab = gtk_label_new("Zakupy");
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
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_purchase, label_tab);
}

static void create_profile_notebook(GtkWidget *notebook, TakConfig* config) {
    GtkWidget *hbox_profile = gtk_hbox_new(0, 0);
    GtkWidget* scroll_profile = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *label_tab = gtk_label_new("Firma");
    GtkWidget *label_profile;
    GtkWidget* table_profile = gtk_table_new(10, 2, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE(table_profile), 20);
    GtkWidget *entry;

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
    g_signal_connect(entry, "changed", G_CALLBACK(waluta_callback), config);
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
    //GtkWidget *combo_entry;
    //combo_entry = gtk_combo_new();
    //GList* combo_list = comboUSCodes();
    //gtk_combo_set_popdown_strings(GTK_COMBO(combo_entry), combo_list);
    //gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_entry)->entry), config->KodUrzedu);
    //gtk_table_attach_defaults(GTK_TABLE(table_profile), combo_entry, 1, 2, 1, 2);
    //
    Completion* compl = (Completion*)malloc(sizeof(Completion));
    compl->combo = combo;
    compl->table = table_profile;
    compl->config = config;
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
    g_signal_connect(entry, "changed", G_CALLBACK(nip_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(pelnanazwa_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(regon_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(kraj_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(wojewodztwo_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(powiat_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(gmina_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(ulica_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(nrdomu_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(nrlokalu_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(miejscowosc_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(kodpocztowy_callback), config);
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
    g_signal_connect(entry, "changed", G_CALLBACK(poczta_callback), config);
    gtk_table_attach_defaults(
            GTK_TABLE(table_profile),
            entry,
            1, 2, 14, 15);

    gtk_box_pack_start(GTK_BOX(hbox_profile), table_profile, 1, 1, 0);
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
    create_profile_notebook(notebook, config);
    return notebook;
}

static GtkWidget* create_date_menu() {
    GtkWidget* hbox_date = gtk_hbox_new(0, 0);
    Date* date = getDate();
    GtkWidget* opt_menu = gtk_option_menu_new();
    gtk_widget_set_size_request(opt_menu, 142, -1);
    GtkWidget* menu = gtk_menu_new();
    char* months[12] = {"Styczeń", "Luty", "Marzec", "Kwiecień", "Maj",
        "Czerwiec", "Lipiec", "Sierpień", "Wrzesień", "Październik",
        "Listopad", "Grudzień"};
    for (int i = 0; i < 12; ++i) {
        GtkWidget* item = gtk_menu_item_new_with_label(
                months[(getMonth(date) - 1 + i) % 12]);
        gtk_widget_show(item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }
    GtkWidget* hbox_space = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(hbox_date), hbox_space, 1, 1, 0);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(opt_menu), menu);
    gtk_box_pack_start(GTK_BOX(hbox_date), opt_menu, 0, 0, 0);
    gtk_widget_show(opt_menu);

    GtkWidget* entry_year = gtk_entry_new();

    gtk_widget_set_size_request(entry_year, 42, -1);
    gtk_entry_set_alignment(GTK_ENTRY(entry_year), 1);
    gtk_entry_set_max_length(GTK_ENTRY(entry_year), 4);
    gtk_entry_set_text(GTK_ENTRY(entry_year), date->year);
    gtk_box_pack_start(GTK_BOX(hbox_date), entry_year, 0, 0, 0);
    return hbox_date;
}

static GtkWidget* create_box_bottom() {
    GtkWidget* hbox_bottom = gtk_hbox_new(0, 15);
    GtkWidget* radio_aim_gr = gtk_radio_button_new_with_label(NULL,
            "Złożenie po raz pierwszy");
    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim_gr, 0, 1, 0);
    GtkWidget* radio_aim = gtk_radio_button_new_with_label(
         gtk_radio_button_get_group(
          GTK_RADIO_BUTTON(radio_aim_gr)), "Pierwsza korekta");
    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim, 0, 1, 0);
    radio_aim = gtk_radio_button_new_with_label(
         gtk_radio_button_get_group(
          GTK_RADIO_BUTTON(radio_aim_gr)), "Druga korekta");
    gtk_box_pack_start(GTK_BOX(hbox_bottom), radio_aim, 0, 1, 0);

    gtk_box_pack_start(GTK_BOX(hbox_bottom), create_date_menu(), 1, 1, 0);
    return hbox_bottom;
}

void drawGui(JPK* jpk) {
    TakConfig* config = parseConfig();
    gtk_init(NULL, NULL);
    GtkWidget *window, *vbox;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    
    vbox = gtk_vbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
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

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(t->jpk, t->config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
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

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(t->jpk, t->config), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
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
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
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

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(change->jpk, change->tak), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

static void sell_entry_callback(GtkWidget* widget, gpointer data) {
    JPKChange* change = (JPKChange*)data;
    char* input = (char*)gtk_entry_get_text(GTK_ENTRY(widget));
    changeData(change->jpk, change->i, change->j, input);
    refreshSellSum(change->jpk, change->tak);
}

static void sell_addrow_callback(GtkWidget* widget, gpointer data) {
    JPKChange* ch = (JPKChange*) data;
    addSellRow(ch->jpk);

    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *root_box = widget->parent->parent->parent->parent->parent->parent->parent->parent->parent;
    gtk_widget_destroy(root_box);
    GtkWidget *vbox = gtk_vbox_new(0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(ch->jpk, ch->tak), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
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
        
        JPK* jpk = loadJPK(g_filename_to_utf8(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)), -1, NULL, NULL, NULL));
//        printf("%s\n", filename);
         
        TakConfig* config = getConfig(jpk); 
        g_print("%s\n", (char*)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        GtkWidget* menu = GTK_WIDGET(data);
        GtkWidget* root_box = menu->parent;
        GtkWidget *window = gtk_widget_get_toplevel(menu);
        
        gtk_widget_destroy(root_box);
        GtkWidget *vbox = gtk_vbox_new(0, 0);

        gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk, config), 1, 1, 0);
        gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
        gtk_container_add(GTK_CONTAINER(window), vbox);
        gtk_widget_show_all(window); 
    } else {
        g_print("Naciśnięto anuluj\n");
    }
    gtk_widget_destroy(dialog);
}
