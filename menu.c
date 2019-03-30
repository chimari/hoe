//    HOE : Subaru HDS++ OPE file Editor
//        menu.c     GUI menu
//                                           2019.01.03  A.Tajitsu

#include "main.h"

/// Main Menu
GtkWidget *make_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;
  GdkPixbuf *pixbuf, *pixbuf2;
  gint w,h;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //File/Open List
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Open List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Open List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",action_read_list,(gpointer)hg);

  //File/Merge List
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("insert-object", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Merge List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Merge List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",action_merge_list,(gpointer)hg);


  //File/Merge List from OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Merge List from OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Merge List from OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_merge_ope,(gpointer)hg);


  //File/Import List from HOE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Merge List from Config (.hoe)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Merget List from Config (.hoe)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_merge_hoe,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //Non-Sidereal/Merge TSC
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge TSC file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge TSC file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_NST,(gpointer)hg);

    //Non-Sidereal/Merge JPL
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image,
      "Merge JPL HORIZONS file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge JPL HORIZONS file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_JPL,(gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);

    //Non-Sidereal/Conv JPL to TSC
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image,
							"Convert HORIZONS to TSC");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Convert HORIZONS to TSC");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_conv_JPL,(gpointer)hg);


    popup_button =gtk_menu_item_new_with_label ("Non-Sidereal");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //Init/Initialize Plan
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("tab-new", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Initialize Target List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Initialize Target List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_init_list,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Write Base OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write Base OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Base OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_base_ope,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

#ifdef USE_SSL
  //File/Upload OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("network-transmit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Upload OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Upload OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_upload_ope,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);
#endif

  //File/Save PROMS/Service Request File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write PROMS Target List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write PROMS Target List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_proms_txt,(gpointer)hg);

  /*
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write Service Request");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Service Request");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_service_txt,(gpointer)hg);
  */

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Load Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Load Config (.hoe)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Load Config (.hoe)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open_hoe,(gpointer)hg);


  //File/Save Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Save Config (.hoe)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save Config (.hoe)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_hoe,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Save Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view_refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Change Instrument");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Change Instrument");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_change_inst, (gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);
  

  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_quit,(gpointer)hg);



  //// Edit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("accessories-text-editor", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Edit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Edit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Edit/PLan Editor
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("format-indent-more", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Obs. Plan Editor");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INDENT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Obs. Plan Editor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_plan,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //Edit/Saved OPE File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("accessories-text-editor", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Text Editor (Saved OPE)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Saved OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_edit,(gpointer)hg);

  //Edit/Select OPE File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Text Editor (Select OPE)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Select OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_name_edit,(gpointer)hg);



  //// Tool
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("applications-engineering", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Tool");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Tool");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  //Tool/PDF Finding Charts
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/pdf_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "PDF Finding Charts");
#else
  popup_button =gtk_image_menu_item_new_with_label ("PDF Finding Charts");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_fc_pdf_all,(gpointer)hg);

  //Tool/Sky Monitor
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/sky_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, "Sky Monitor");
#else
  popup_button =gtk_image_menu_item_new_with_label ("Sky Monitor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_skymon,(gpointer)hg);
  //g_resources_unregister(resource);


  ////Database
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-web", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Database");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Database");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  // Data Archive List Query
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SMOKA");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SMOKA");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_smoka, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "HST archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("HST archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_hst, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "ESO archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("ESO archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_eso, (gpointer)hg);

    // Gemini
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, 
							"Gemini archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Gemini archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_gemini, (gpointer)hg);

    popup_button =gtk_menu_item_new_with_label ("Data Archive List Query");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // MagDB
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SIMBAD");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SIMBAD");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_simbad, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "NED");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("NED");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_ned, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "LAMOST DR4");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("LAMOST DR4");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_lamost, (gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "GSC 2.3");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("GSC 2.3");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_gsc, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "PanSTARRS-1");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("PanSTARRS-1");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_ps1, (gpointer)hg);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SDSS DR15");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SDSS DR15");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_sdss, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "GAIA DR2");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("GAIA DR2");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_gaia, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Kepler Input Catalog");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Kepler Input Catalog");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_kepler, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "2MASS");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("2MASS");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_2mass, (gpointer)hg);

    popup_button =gtk_menu_item_new_with_label ("Catalog Matching");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // Standard
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Param for Standard");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for Standard");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     create_std_para_dialog, (gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Param for DB query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for DB query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     fcdb_para_item, (gpointer)hg);


  //// HDS
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "HDS");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("HDS");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  //Tool/Echelle Format Simulator
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/efs_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "EFS: Echelle Format Simulator");
#else
  popup_button =gtk_image_menu_item_new_with_label ("EFS: Echelle Format Simulator");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",hds_do_efs_cairo,(gpointer)hg);
  
  //Tool/Exposure Time Calculator
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "ETC: Exposure Time Calculator for a selected target");
#else
  popup_button =gtk_image_menu_item_new_with_label ("ETC: Exposure Time Calculator for a selected target");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",hds_do_etc,(gpointer)hg);
  
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Calc S/N for all targets by ETC");
#else
  popup_button =gtk_image_menu_item_new_with_label ("Calc S/N for all targets by ETC");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",hds_do_etc_list,(gpointer)hg);
  
  //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Set Default Guide/PA/Exp");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Set Default Guide/PA/Exp");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate", hds_do_export_def_list,(gpointer)hg);
  
  //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Calc Exptime using Mag");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Calc Exptime using Mag");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_update_exp_list,(gpointer)hg);
  
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

#ifdef USE_SSL
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-downloads", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Download LOG");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Download LOG");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_download_log,(gpointer)hg);
#endif


  
  //// IRCS
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "IRCS");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("IRCS");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
    //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Set Default AO-mode/PA");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Set Default AO-mode/PA");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",ircs_do_export_def_list,(gpointer)hg);
  
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // LGS Output
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image,
							"Set Contact Info for LGSSet Default AO-mode/PA");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Set Contact Info for LGS");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",lgs_do_setup_sa,(gpointer)hg);
    

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Write LGS target list");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Write LGS target list");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",ircs_do_save_lgs_txt,(gpointer)hg);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Write LGS PRM files");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Write LGS PRM files");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate", lgs_do_create_prm,(gpointer)hg);
    
    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);
    
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/lgs_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image, "Import Collision Data (PAM)");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Import Collision Data (PAM)");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate", lgs_read_pam,(gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Export PAM to CSV for all targets");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Export PAM to CSV for all targets");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_pam_all,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("LGS");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);    
  }
 
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // Guide-Star & AO-mode
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "by GSC 2.3 (R)");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("by GSC 2.3 (R)");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       ircs_magdb_gsc, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "by PanSTARRS-1 (r)");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("by PanSTARRS-1 (r)");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       ircs_magdb_ps1, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "by GAIA (G)");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("by GAIA (G)");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       ircs_magdb_gaia, (gpointer)hg);

    popup_button =gtk_menu_item_new_with_label ("Guide-Star & AO-mode");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }


  //// HSC
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "HSC");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("HSC");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Set Default PA");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Set Default PA");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",hsc_do_export_def_list,(gpointer)hg);
  
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Check bright stars in FOV");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Check bright stars in FOV");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     hsc_magdb_simbad, (gpointer)hg);
  

  //// Info
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Info");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Check the latest ver.");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Check the latest ver.");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",CheckVer, (gpointer)hg);

  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "About");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_version, (gpointer)hg);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}



//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///////////////  Callbacks    for File I/Os  ---> io_gui.c
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
///////////////  Menu -> File
////////////////////////////////////////////////////////////


void do_init_list (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->i_max=0;
  
  make_obj_tree(hg);
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}

void do_change_inst (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg=(typHOE *) gdata;

  SelectInst(hg, TRUE);
}


void do_quit (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg=(typHOE*) gdata;

  if(!flagChildDialog){
    create_quit_dialog(hg);
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs before quitting.",
		  NULL);
  }
}



////////////////////////////////////////////////////////////
///////////////  Menu -> Edit
////////////////////////////////////////////////////////////

void do_plan(GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  gint i_ret;
  gchar *tmp;

  hg=(typHOE *)gdata;
  
  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else{
    switch(hg->inst){
    case INST_IRCS:
      if(hg->ircs_i_max==0){
	popup_message(hg->w_top,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT*2,
		      "Please set at least 1 IRCS setup to create your obs plan.",
		      NULL);
	gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), hg->page[NOTE_IRCS]);
	return;
      }
      else{
	i_ret=IRCS_check_gs(hg);
	if(i_ret>=0){
	  tmp=g_strdup_printf("Guide star is not selected for [Obj-%d] %s.",
			      i_ret+1, hg->obj[i_ret].name);
	  
	  popup_message(hg->w_top, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT*2,
			tmp,
			NULL);
	  g_free(tmp);
	  return;
	}
      }
      break;

    case INST_HSC:
      if(hg->hsc_i_max==0){
	popup_message(hg->w_top,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT*2,
		      "Please set at least 1 HSC setup to create your obs plan.",
		      NULL);
	gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), hg->page[NOTE_HSC]);
	return;
      }
      break;
      
    }
  }

  flagChildDialog=TRUE;
  create_plan_dialog(hg);
}


void do_edit(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(!hg->filename_write){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "No OPE files have been saved yet.",
		  NULL);
    return;
  }

  create_opedit_dialog(hg);
}


////////////////////////////////////////////////////////////
///////////////  Menu -> Tool
////////////////////////////////////////////////////////////

void do_skymon(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagSkymon){
    gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    return;
  }
  else{
    flagSkymon=TRUE;
  }
  
  create_skymon_dialog(hg);
}




////////////////////////////////////////////////////////////
///////////////  Menu -> Update
////////////////////////////////////////////////////////////

void do_update_exp_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;

  dialog = gtk_dialog_new_with_buttons("HOE : Update Exptime using Mag",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  frame = gtkut_frame_new ("Update Exptime in the list (shot noise limit)");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(1, 2, FALSE, 0, 0, 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Set Mag for each target using \"Database/Magnitude List Query\" at first.");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("ExpTime for ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->expmag_mag,
					    4.0, 18.0,
					    0.1, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->expmag_mag);
  
  label = gtk_label_new ("mag = ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->expmag_exp,
					    1, 3600,
					    1.0, 10.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->expmag_exp);
  
  label = gtk_label_new ("[s]");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    do_update_exp(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


////////////////////////////////////////////////////////////
///////////////  Menu -> About
////////////////////////////////////////////////////////////

void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GdkPixbuf *pixbuf, *pixbuf2;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  typHOE *hg=(typHOE *) gdata;
  gint result;
  gchar *tempdir=NULL, *conffile=NULL;

  dialog = gtk_dialog_new_with_buttons("HOE : About This Program",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  96,96,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(label), "<span size=\"larger\"><b>HOE : Subaru HDS++ OPE file Editor</b></span>   version "VERSION);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#if HAVE_SYS_UTSNAME_H
  uname(&utsbuf);
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s %s (%s)",
	     utsbuf.sysname, utsbuf.release, utsbuf.machine);
#elif defined(USE_WIN32)
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s",
	     WindowsVersion());
#else
  g_snprintf(buf, sizeof(buf),
	     "Operating System: unknown UNIX");
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#ifdef USE_OSX
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : OpenSSL=%s, GtkMacIntegration=%s", 
#ifdef USE_SSL
	     "ON",
#else
             "OFF",
#endif
#ifdef USE_GTKMACINTEGRATION
	     "ON"
#else
	     "OFF"
#endif
	     );
#else
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : OpenSSL=%s", 
#ifdef USE_SSL
	     "ON"
#else
             "OFF"
#endif
	     );
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  // Config file @ home dir
  conffile = g_strconcat("                General Config : ",
			 hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);
  
  label = gtk_label_new (conffile);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  g_free(conffile);

  // temp dir
  tempdir = g_strconcat("                Temp Directory : ",
			 hg->temp_dir, G_DIR_SEPARATOR_S,
			 NULL);
  label = gtk_label_new (tempdir);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  g_free(tempdir);
  
  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

 
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&#xA9; 2003-2019  Akito Tajitsu");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label=gtk_label_new(NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&lt;<i>tajitsu@naoj.org</i>&gt;");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  button = gtk_button_new_with_label(" "DEFAULT_URL" ");
  gtk_box_pack_start(GTK_BOX(vbox), 
		     button, TRUE, FALSE, 0);
  gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
  my_signal_connect(button,"clicked",uri_clicked, (gpointer)hg);
#ifdef USE_GTK3
  css_change_col(gtk_bin_get_child(GTK_BIN(button)),"blue");
#else
  gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(button)),
		       GTK_STATE_NORMAL,&color_blue);
#endif

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     scrolledwin, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scrolledwin, 400, 250);
  
  text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 6);
  gtk_container_add(GTK_CONTAINER(scrolledwin), text);
  
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program (HOE) accesses to the following astronomical online database services via WWW. The author of the program (AT) acknowledges with thanks to all of them.\n\n"

			 "[SIMBAD]\n"
			 "    http://simbad.u-strasbg.fr/\n"
			 "    http://simbad.harvard.edu/\n\n"

			 "[The NASA/IPAC Extragalactic Database (NED)]\n"
			 "    http://ned.ipac.caltech.edu/\n\n"
			 
			 "[SkyView by NASA]\n"
			 "    https://skyview.gsfc.nasa.gov/\n\n"

			 "[SLOAN DIGITAL SKY SURVEY : SkyServer]\n"
			 "    http://skyserver.sdss.org/\n\n"

			 "[The Mikulski Archive for Space Telescopes (MAST)]\n"
			 "    http://archive.stsci.edu/\n\n"

			 "[Gemini Observatory Archive Search]\n"
			 "    https://archive.gemini.edu/\n\n"

			 "[NASA/IPAC Infrared Science Archive (IRSA)]\n"
			 "    http://irsa.ipac.caltech.edu\n\n"

			 "[The Combined Atlas of Sources with Spitzer IRS Spectra (CASSIS)]\n"
			 "    http://cassis.sirtf.com/\n\n"

			 "[Large Sky Area Multi-Object Fiber Spectroscoic Telescope (LAMOST)]\n"
			 "    http://www.lamost.org/\n\n"

			 "[The Subaru-Mitaka-Okayama-Kiso-Archive (SMOKA)]\n"
			 "    http://smoka.nao.ac.jp/\n\n"

			 "[The ESO Science Archive Facility]\n"
			 "    http://archive.eso.org/\n\n"

			 "[Keck Observatory Archive]\n"
			 "    https://koa.ipac.caltech.edu/\n\n"

			 "[Pan-STARRS1 data archive]\n"
			 "    https://panstarrs.stsci.edu/\n\n"

			 "[The VizieR catalogue access tool, CDS, Strasbourg, France]\n"
			 "    http://vizier.u-strasbg.fr/\n\n",
			 -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "\n======================================================\n\n"
			 , -1);

  
  gtk_text_buffer_insert(buffer, &iter,
			 "This program is free software; you can redistribute it and/or modify "
			 "it under the terms of the GNU General Public License as published by "
			 "the Free Software Foundation; either version 3, or (at your option) "
			 "any later version.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program is distributed in the hope that it will be useful, "
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 "See the GNU General Public License for more details.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "You should have received a copy of the GNU General Public License "
			 "along with this program.  If not, see <http://www.gnu.org/licenses/>.", -1);

  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_place_cursor(buffer, &iter);

  gtk_widget_show_all(dialog);

  result= gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}



////////////////////////////////////////////////////////////
//////////// Other callbacks
////////////////////////////////////////////////////////////

void do_plot(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagPlot){
    gdk_window_raise(gtk_widget_get_window(hg->plot_main));
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,hg);
  }
  else{
    create_plot_dialog(hg);
  }
  
}




////////////////////////////////////////////////////////////
////////////////  Functions calling from menu callbacks
////////////////////////////////////////////////////////////

void SelectInst(typHOE *hg, gboolean destroy_flag){
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GtkWidget *rb[NUM_INST];
  GdkPixbuf *pixbuf, *pixbuf2;
  gint old_inst=hg->inst;  // = -1 for initialization
  gchar *tmp;
  gint i_inst;
  
  if(hg->inst<0){
    hg->inst=INST_HDS;
  }
  
  dialog = gtk_dialog_new_with_buttons("HOE : Select your instrument.",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  96,96,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);
  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Please select an instrument for your obs.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  for(i_inst=0;i_inst<NUM_INST;i_inst++){
    tmp=g_strdup_printf("%s (%s)",inst_name_short[i_inst], inst_name_long[i_inst]);
    rb[i_inst]
      = gtk_radio_button_new_with_label_from_widget ((i_inst==0) ? NULL : GTK_RADIO_BUTTON(rb[0]),
						     tmp);
    gtk_box_pack_start(GTK_BOX(vbox), rb[i_inst], FALSE, FALSE, 0);
    my_signal_connect (rb[i_inst], "toggled", cc_radio, &hg->inst);
  }

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->inst]),TRUE);

  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    if(old_inst<0){
      init_inst(hg);
    }
    else if (hg->inst!=old_inst){
      init_inst(hg);
      // Plan Initialize
      hg->i_plan_max=0;

      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-information", 
#else
		    GTK_STOCK_DIALOG_INFO,
#endif
		    POPUP_TIMEOUT*1,
		    "Your observing plan has been initialized.",
		    NULL);
    }
  }
  else{
    exit(0);
  }
  
  if(destroy_flag){
    gtk_widget_destroy(hg->all_note);
    
    flag_make_obj_tree=FALSE;
    flag_make_line_tree=FALSE;
    
    make_note(hg);
    set_win_title(hg);
  }
}


gboolean CheckInst(typHOE *hg, gint target_inst){
  gchar *tmp;
  
  if(hg->inst==target_inst){
    return(TRUE);
  }
  else{
    tmp=g_strdup_printf("   %s : %s .\n",
			inst_name_short[target_inst],
			inst_name_long[target_inst]);
    
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*1,
		  "This function is available only for ",
		  " ",
		  tmp,
		  NULL);
    g_free(tmp);

    return(FALSE);
  }
}


void create_quit_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Quit Program",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-question",
				   GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Do you want to quit this program?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    WriteConf(hg);
    gtk_widget_destroy(dialog);
    exit(0);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
}


