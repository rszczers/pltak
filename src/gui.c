#include <gtk/gtk.h>

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

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    GtkWidget *window, *menu_bar, *menu_item, *file_menu, *help_menu,
              *vbox, *button, *notebook, *label_tab, *label;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);

    menu_bar = gtk_menu_bar_new();
    file_menu = gtk_menu_new();
    help_menu = gtk_menu_new();

    menu_item = gtk_menu_item_new_with_label("Plik");
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

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);

    label_tab = gtk_label_new("Sprzedaże");
    label = gtk_label_new("To jest przykladowy tekst 1 ");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), label, label_tab);
    label_tab = gtk_label_new("Zakupy");
    label = gtk_label_new("To jest przykladowy tekst 2 ");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), label, label_tab);
    label_tab = gtk_label_new("Firma");
    label = gtk_label_new("To jest przykladowy tekst 3");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), label, label_tab);

    vbox = gtk_vbox_new(0, 0);
    button = gtk_button_new_with_label("Testowy przycisk");
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, 1, 1, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_set_size_request(window, 640, 480);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
