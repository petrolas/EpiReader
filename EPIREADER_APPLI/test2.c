#include <stdlib.h>
#include <gtk/gtk.h>
 
void OnQuitter(GtkWidget* widget, gpointer data);
void OnAbout(GtkWidget* widget, gpointer data);
void on_activate_entry(GtkWidget *pEntry, gpointer data);
void on_copier_button(GtkWidget *pButton, gpointer data);

int main(int argc, char **argv)
{
    GtkWidget *pWindow;
    GtkWidget *pVBox;
    GtkWidget *pMenuBar;
    GtkWidget *pMenu;
    GtkWidget *pMenuItem;
    GtkWidget *pLabel;
    GtkWidget *pEntry;
    GtkWidget *pButton;

    //pour l'image
    GtkWidget *pImage;
    GtkWidget *pQuitImage;
    GtkWidget *pQuitBtn;
 
    gtk_init(&argc, &argv);
 
    //_________________________________________________________________

    //CREATION DE LA FENETRE

    //_________________________________________________________________


    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "QRcode");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 1000, 750);
    g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    pVBox = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);


    //_________________________________________________________________

    //MISE EN PLACE DE LA BARRE EN HAUT
    
    //_________________________________________________________________


    /* Création de la GtkVBox */
    
 
    /**** Création du menu ****/
 
    /* ÉTAPE 1 */
    pMenuBar = gtk_menu_bar_new();
    /** Premier sous-menu **/
    /* ÉTAPE 2 */
    pMenu = gtk_menu_new();
    /* ÉTAPE 3 */
    gtk_box_pack_start(GTK_BOX(pVBox), pMenuBar, FALSE, FALSE, 0);
    pMenuItem = gtk_menu_item_new_with_label("charger votre QRcode");
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


    //_________________________________________________________________

    //  MISE EN PLACE DE LA BARRE POUR TAPER

    //_________________________________________________________________

    /* Creation du GtkEntry */
    pEntry = gtk_entry_new();
    /* Insertion du GtkEntry dans la GtkVBox */
    gtk_box_pack_start(GTK_BOX(pVBox), pEntry, FALSE, FALSE, 20);
 
    pButton = gtk_button_new_with_label("créer le QRcode");
    gtk_box_pack_start(GTK_BOX(pVBox), pButton, FALSE, FALSE, 10);
 
    pLabel = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, FALSE, 0);
 
    /* Connexion du signal "activate" du GtkEntry */
    g_signal_connect(G_OBJECT(pEntry), "activate", G_CALLBACK(on_activate_entry), (GtkWidget*) pLabel);
 
    /* Connexion du signal "clicked" du GtkButton */
    /* La donnée supplémentaire est la GtkVBox pVBox */
    g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(on_copier_button), (GtkWidget*) pVBox);


    //_________________________________________________________________

    //AFFICHAGE DE NOTRE LOGO

    //_________________________________________________________________

    pImage = gtk_image_new_from_file("qr-code.png");
    gtk_box_pack_start(GTK_BOX(pVBox), pImage, FALSE, FALSE, 0);

 
    //pQuitBtn = gtk_button_new();
    //gtk_box_pack_start(GTK_BOX(pVBox), pQuitBtn, TRUE, FALSE, 5);
    //g_signal_connect(G_OBJECT(pQuitBtn), "clicked", G_CALLBACK(gtk_main_quit), NULL);
 
    /* Chargement d'une image à partir d'un GtkStockItem */
    //pQuitImage = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_LARGE_TOOLBAR);
    //gtk_container_add(GTK_CONTAINER(pQuitBtn), pQuitImage);


    //_________________________________________________________________


 
    /* Ajout du menu a la fenêtre */
    gtk_box_pack_start(GTK_BOX(pVBox), pMenuBar, FALSE, FALSE, 0);
 
    gtk_widget_show_all(pWindow);
 
    gtk_main();
 
    return EXIT_SUCCESS;
}
 
//initialisation du bouton quitter
//affichage de texte plus laisse le choix de quitter ou non
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
 
//initialisation du bouton "a propos" avec le texte a afficher
//
void OnAbout(GtkWidget* widget, gpointer data)
{
    GtkWidget *pAbout;
 
    pAbout = gtk_message_dialog_new(GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "lecteur/créateur de QRcode\n"
        "alpha");
 
    gtk_dialog_run(GTK_DIALOG(pAbout));
 
    gtk_widget_destroy(pAbout);
}

//_________________________________________________________________

// SERT A AFFICHER LE TEXTE (DONC PROBABLEMENT INUTILE)

//_________________________________________________________________


/* Fonction callback execute lors du signal "activate" */
void on_activate_entry(GtkWidget *pEntry, gpointer data)
{
    const gchar *sText;
 
    /* Recuperation du texte contenu dans le GtkEntry */
    sText = gtk_entry_get_text(GTK_ENTRY(pEntry));
 
    /* Modification du texte contenu dans le GtkLabel */
    gtk_label_set_text(GTK_LABEL((GtkWidget*)data), sText);
}
 
/* Fonction callback executee lors du signal "clicked" */
void on_copier_button(GtkWidget *pButton, gpointer data)
{
    GtkWidget *pTempEntry;
    GtkWidget *pTempLabel;
    GList *pList;
    const gchar *sText;
 
    /* Récupération de la liste des éléments que contient la GtkVBox */
    pList = gtk_container_get_children(GTK_CONTAINER((GtkWidget*)data));
 
    /* Le premier élément est le GtkEntry */
    pTempEntry = GTK_WIDGET(pList->data);
 
    /* Passage à l élément suivant : le GtkButton */
    pList = g_list_next(pList);
 
    /* Passage à l élément suivant : le GtkLabel */
    pList = g_list_next(pList);
 
    /* Cet élément est le GtkLabel */
    pTempLabel = GTK_WIDGET(pList->data);
 
    /* Recuperation du texte contenu dans le GtkEntry */
    sText = gtk_entry_get_text(GTK_ENTRY(pTempEntry));
 
    /* Modification du texte contenu dans le GtkLabel */
    gtk_label_set_text(GTK_LABEL(pTempLabel), sText);
 
    /* Libération de la mémoire utilisée par la liste */
    //g_list_free(pList);
    
}

