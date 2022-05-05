#include <stdlib.h>
#include <gtk/gtk.h>
 
void OnQuitter(GtkWidget* widget, gpointer data);
void OnAbout(GtkWidget* widget, gpointer data);
 
int main(int argc, char **argv)
{
    GtkWidget *pWindow;
    GtkWidget *pVBox;
    GtkWidget *pMenuBar;
    GtkWidget *pMenu;
    GtkWidget *pMenuItem;
 
    gtk_init(&argc, &argv);
 
    /* Création de la fenêtre */
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "QRcode");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 320, 200);
    g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
    /* Création de la GtkVBox */
    pVBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);
 
    /**** Création du menu ****/
 
    /* ÉTAPE 1 */
    pMenuBar = gtk_menu_bar_new();
    /** Premier sous-menu **/
    /* ÉTAPE 2 */
    pMenu = gtk_menu_new();
    /* ÉTAPE 3 */
    p
 
    pMenuItem = gtk_menu_item_new_with_label("Ouvrir");
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
 
    pMenuItem = gtk_menu_item_new_with_label("Quitter");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(OnQuitter),(GtkWidget*) pWindow);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    /* ÉTAPE 4 */
    pMenuItem = gtk_menu_item_new_with_label("Fichier");
    /* ÉTAPE 5 */
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    /* ÉTAPE 6 */
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);
 
    /** Second sous-menu **/
    /* ÉTAPE 2 */
    pMenu = gtk_menu_new();
    /* ÉTAPE 3 */
    pMenuItem = gtk_menu_item_new_with_label("À propos de...");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(OnAbout),(GtkWidget*) pWindow);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    /* ÉTAPE 4 */
    pMenuItem = gtk_menu_item_new_with_label("?");
    /* ÉTAPE 5 */
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    /* ÉTAPE 6 */
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);
 
    /* Ajout du menu a la fenêtre */
    gtk_box_pack_start(GTK_BOX(pVBox), pMenuBar, FALSE, FALSE, 0);
 
    gtk_widget_show_all(pWindow);
 
    gtk_main();
 
    return EXIT_SUCCESS;
}
 
void OnQuitter(GtkWidget* widget, gpointer data)
{
    GtkWidget *pQuestion;
 
    pQuestion = gtk_message_dialog_new(GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Voulez vous vraiment\n"
        "quitter le programme?");
 
    switch(gtk_dialog_run(GTK_DIALOG(pQuestion)))
    {
        case GTK_RESPONSE_YES:
            gtk_main_quit();
            break;
        case GTK_RESPONSE_NONE:
        case GTK_RESPONSE_NO:
            gtk_widget_destroy(pQuestion);
            break;
    }
}
 
void OnAbout(GtkWidget* widget, gpointer data)
{
    GtkWidget *pAbout;
 
    pAbout = gtk_message_dialog_new(GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "projet réalisé par\n"
        "EPIREADER");
 
    gtk_dialog_run(GTK_DIALOG(pAbout));
 
    gtk_widget_destroy(pAbout);
}
