#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "jpk.h"

static void create_sell_col_filter(GtkWidget* widget, JPK* jpk) {
    GtkWidget* check_sell;
    JPKColumns* title = jpk->colNames;
    while(strcmp(title->title, "NrKontrahenta") != 0)
         title = title->next;
    while(strcmp(title->title, "LiczbaWierszySprzedazy") != 0) {
        check_sell = gtk_check_button_new_with_label(title->title);
        gtk_box_pack_start(GTK_BOX(widget), check_sell, 0, 0, 0);
        title = title->next;
    }
}

static void create_pur_col_filter(GtkWidget* widget, JPK* jpk) {
    GtkWidget* check_purchase;
    JPKColumns* title = jpk->colNames;
    while(strcmp(title->title, "NrDostawcy") != 0)
         title = title->next;
    while(strcmp(title->title, "LiczbaWierszyZakupow") != 0) {
        check_purchase = gtk_check_button_new_with_label(title->title);
        gtk_box_pack_start(GTK_BOX(widget), check_purchase, 0, 0, 0);
        title = title->next;
    }
}

static GtkWidget* spreadsheet(JPK* data) {
    GtkWidget* table_sell = gtk_table_new(10, 10, FALSE);
    GtkWidget *entry, *button;
    gtk_table_set_homogeneous(GTK_TABLE(table_sell), FALSE);
    char* buffer = (char*)malloc(64);
    for (int i = 0; i < 10; i++) {
        gtk_table_attach_defaults (GTK_TABLE(table_sell), gtk_label_new("Tytuł kolumny"),
                i, i+1, 0, 1);
        for (int j = 1; j < 10; j++) {
            if (i == 0) {
                button = gtk_button_new_with_label("Usuń");
                gtk_table_attach_defaults (GTK_TABLE(table_sell), button,
                        i, i+1, j, j+1);
            } else {
                sprintf (buffer, "(%d,%d)", i, j);
                entry = gtk_entry_new ();
                gtk_entry_set_text (GTK_ENTRY(entry), buffer);
                gtk_table_attach_defaults (GTK_TABLE(table_sell), entry,
                        i, i+1, j, j+1);
            }
        }
    }
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
    g_signal_connect(menu_item, "activate", G_CALLBACK(open_dialog), NULL);

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

static void create_sell_notebook(GtkWidget *notebook, JPK* jpk) {
    GtkWidget* label_tab = gtk_label_new("Sprzedaże");
    GtkWidget* hbox_sell = gtk_hbox_new(0, 0);
    GtkWidget* vbox_spread = gtk_vbox_new(0, 0);
    GtkWidget* vbox_col_sell = gtk_vbox_new(0, 0);
    create_sell_col_filter(vbox_col_sell, jpk);
    GtkWidget* scroll_col_sell = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_col_sell),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_col_sell), vbox_col_sell);
    gtk_widget_set_size_request(scroll_col_sell, 180, 480);
    GtkWidget* scroll_sell = gtk_scrolled_window_new (NULL, NULL);
    // Pseudoarkusz
    GtkWidget* table_sell = spreadsheet(NULL);

    gtk_table_set_row_spacings(GTK_TABLE(table_sell), 1);
    gtk_table_set_col_spacings(GTK_TABLE(table_sell), 1);
    GtkWidget *button_add_row = gtk_button_new_with_label("Dodaj wiersz");
    GtkWidget *hbox_space = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), table_sell, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), button_add_row, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox_spread), hbox_space, 1, 1, 0);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_sell), vbox_spread);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll_sell),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(hbox_sell), scroll_sell, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_sell), scroll_col_sell, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_sell, label_tab);
}

static void create_purchase_notebook(GtkWidget *notebook, JPK* jpk) {
    GtkWidget *label_tab = gtk_label_new("Zakupy");
    GtkWidget* label = gtk_label_new("To jest przykladowy tekst 2 ");
    GtkWidget* hbox_purchase = gtk_hbox_new(0, 0);
    GtkWidget* vbox_col_pur = gtk_vbox_new(0, 0);
    GtkWidget* scroll_col_pur = gtk_scrolled_window_new (NULL, NULL);
    create_pur_col_filter(vbox_col_pur, jpk);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_col_pur),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scroll_col_pur), vbox_col_pur);
    gtk_widget_set_size_request(scroll_col_pur, 180, 480);
    gtk_box_pack_start(GTK_BOX(hbox_purchase), label, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox_purchase), scroll_col_pur, 0, 0, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_purchase, label_tab);
}

static void create_profile_notebook(GtkWidget *notebook) {
    GtkWidget *hbox_profile = gtk_hbox_new(0, 0);
    GtkWidget *label_tab = gtk_label_new("Firma");
    GtkWidget *label_profile = gtk_label_new("To jest przykladowy tekst 3");
    gtk_box_pack_start(GTK_BOX(hbox_profile), label_profile, 1, 1, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox_profile, label_tab);
}

static GtkWidget* create_notebooks(JPK* jpk) {
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    create_sell_notebook(notebook, jpk);
    create_purchase_notebook(notebook, jpk);
    create_profile_notebook(notebook);
    return notebook;
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
    return hbox_bottom;
}

void drawGui(JPK* jpk) {
//    gtk_init(&argc, &argv);
    gtk_init(NULL, NULL);
    GtkWidget *window, *vbox;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_menu_bar(), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_notebooks(jpk), 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_box_bottom(), 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_set_size_request(window, 800, 600);
    gtk_widget_show_all(window);
    gtk_main();
}
