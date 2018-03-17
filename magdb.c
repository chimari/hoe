//    HDS OPE file Editor
//      magdb.c : Magnitude Search in Online Catalogs
//                                           2018.2.7  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"

void delete_magdb();
void cancel_magdb();
#ifndef USE_WIN32
void magdb_signal();
#endif
static void ok_magdb();
void magdb_run();
gboolean check_magdb();

gboolean flag_magdb_kill=FALSE;
gboolean  flag_magdb_finish=FALSE;


void delete_magdb(GtkWidget *w, GdkEvent *event, gpointer gdata)
{
  cancel_magdb(w,gdata);
}

void cancel_magdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;
  hg=(typHOE *)gdata;

  flag_magdb_kill=TRUE;

#ifdef USE_WIN32
  if(hg->dwThreadID_fcdb){
    PostThreadMessage(hg->dwThreadID_fcdb, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_fcdb, INFINITE);
    CloseHandle(hg->hThread_fcdb);
    gtk_main_quit();
  }
#else
  if(fcdb_pid){
    kill(fcdb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
    fcdb_pid=0;
  }
  else{
    gtk_main_quit();
  }
#endif
}


#ifndef USE_WIN32
void magdb_signal(int sig){
  pid_t child_pid=0;

  flag_magdb_finish=TRUE;

  do{
    int child_ret;
    child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
  
}
#endif

static void ok_magdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  magdb_run(hg);

  hg->trdb_used=hg->fcdb_type;
  hg->fcdb_type=hg->fcdb_type_tmp;
}


void magdb_gaia (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_GAIA;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Magnitude Search in GAIA");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in G");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);



#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_gsc (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_GSC;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Magnitude Search in GSC");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fil;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_fil=0;i_fil<NUM_GSC_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, gsc_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_gsc_band==i_fil) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->magdb_gsc_band);
  }

  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  trdb_make_tree(hg);
  rebuild_trdb_tree(hg);
}


void magdb_ps1 (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_PS1;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Magnitude Search in PanSTARRS");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fil;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_fil=0;i_fil<NUM_PS1_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, ps1_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_ps1_band==i_fil) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->magdb_ps1_band);
  }

  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_sdss (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_SDSS;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Magnitude Search in SDSS");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fil;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_fil=0;i_fil<NUM_SDSS_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, sdss_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_sdss_band==i_fil) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->magdb_sdss_band);
  }

  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_2mass (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_2MASS;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Magnitude Search in PanSTARRS");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fil;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_fil=0;i_fil<NUM_TWOMASS_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, twomass_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_2mass_band==i_fil) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->magdb_2mass_band);
  }

  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_simbad (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame, *vbox;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_SIMBAD;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : List Search in SIMBAD");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,6,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  label = gtk_label_new ("Search Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 1, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new (" < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_mag,
					    8, 20, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fil;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_fil=0;i_fil<NUM_FCDB_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, simbad_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_simbad_band==i_fil) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->magdb_simbad_band);
  }


  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 2, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtk_frame_new (NULL);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), frame, 0, 3, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), frame, 0, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  vbox = gtkut_vbox_new(FALSE,3);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  label = gtk_label_new ("The query picks up the brightest target within the search radius.");
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif

  label = gtk_label_new ("It picks up the nearest one if no object has a specfic magnitude.");
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif

  label = gtk_label_new ("Band = \"Nop.\" does not constrain targets\' magnitude but picks up V-band magnitude to the list.");
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif


  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), check, 0, 0, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(table), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_ned (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_NED;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : List Search in NED");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("(This query never updates Mags in the list.)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_lamost (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_LAMOST;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : List Search in LAMOST DR3");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  frame = gtk_frame_new ("Search Parameters");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,4,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_grid_attach(GTK_GRID(table), hbox, 1, 0, 1, 1);
#else
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_arcsec);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  frame = gtk_frame_new ("Mag update in the Main Target list");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

#ifdef USE_GTK3      
  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID (table), 10);
  gtk_grid_set_column_spacing (GTK_GRID (table), 5);
#else
  table = gtk_table_new(3,1,FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
#endif
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new ("(This query never updates Mags in the list.)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
#endif


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
  my_signal_connect(button,"pressed",
		    ok_magdb, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
}


void magdb_run (typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gint i_list, i_band;
  GtkWidget *dialog, *vbox, *label, *button, *sep, *time_label, *stat_label,
    *hbox, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint fcdb_tree_check_timer;
  gint timer=-1;
  gchar tmp[BUFFSIZE];
  time_t start_time;
  double elapsed_sec, remaining_sec;
  gchar *url_param, *mag_str, *otype_str;
  gint hits=1;
  gboolean flag_get;
  
  if(hg->i_max<=0) return;
  if(flag_getFCDB) return;

  flag_getFCDB=TRUE;

  trdb_clear_tree(hg);

  if(hg->magdb_ow){
    for(i_list=0;i_list<hg->i_max;i_list++){
      hg->obj[i_list].mag=100;
      hg->obj[i_list].magdb_band=0;
      hg->obj[i_list].magdb_used=0;
    }
  }

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Running Catalog Matching Service");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event",delete_magdb, (gpointer)hg);
 
#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif

  switch(hg->fcdb_type){
  case MAGDB_TYPE_SIMBAD:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case MAGDB_TYPE_NED:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in NED ...");
    break;

  case MAGDB_TYPE_LAMOST:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in LAMOST ...");
    break;

  case MAGDB_TYPE_GSC:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in GSC ...");
    break;

  case MAGDB_TYPE_PS1:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in PanSTARRS ...");
    break;

  case MAGDB_TYPE_SDSS:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in SDSS ...");
    break;

  case MAGDB_TYPE_GAIA:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in GAIA ...");
    break;

  case MAGDB_TYPE_2MASS:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in 2MASS ...");
    break;

  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);

  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);

  hg->pbar2=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar2,TRUE,TRUE,0);
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				  GTK_ORIENTATION_HORIZONTAL);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				    GTK_PROGRESS_LEFT_TO_RIGHT);
#endif
  sprintf(tmp,"Searching [ 1 / %d ] Objects", hg->i_max);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
  gtk_widget_show(hg->pbar2);

  sprintf(tmp,"Estimated time left : ---");
  time_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     time_label,TRUE,TRUE,5);

#ifdef USE_GTK3
  sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  sep = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     sep,FALSE,TRUE,5);

  sprintf(tmp,"%s : hit ---", hg->obj[0].name);
  stat_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     stat_label,TRUE,TRUE,5);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_SIMBAD:
    hg->plabel=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case MAGDB_TYPE_NED:
    hg->plabel=gtk_label_new("Searching objects in NED ...");
    break;

  case MAGDB_TYPE_LAMOST:
    hg->plabel=gtk_label_new("Searching objects in LAMOST ...");
    break;

  case MAGDB_TYPE_GSC:
    hg->plabel=gtk_label_new("Searching objects in GSC ...");
    break;

  case MAGDB_TYPE_PS1:
    hg->plabel=gtk_label_new("Searching objects in PanSTARRS ...");
    break;

  case MAGDB_TYPE_SDSS:
    hg->plabel=gtk_label_new("Searching objects in SDSS ...");
    break;

  case MAGDB_TYPE_GAIA:
    hg->plabel=gtk_label_new("Searching objects in GAIA ...");
    break;

  case MAGDB_TYPE_2MASS:
    hg->plabel=gtk_label_new("Searching objects in 2MASS ...");
    break;

  }
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->plabel, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->plabel, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,TRUE,TRUE,0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",cancel_magdb,(gpointer)hg);


  gtk_widget_show_all(dialog);

  start_time=time(NULL);

  fcdb_tree_check_timer=g_timeout_add(1000, 
				      (GSourceFunc)check_magdb,
				      (gpointer)hg);

  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->fcdb_type){
    case MAGDB_TYPE_SIMBAD:
      hits=hg->obj[i_list].magdb_simbad_hits;
      break;
	
    case MAGDB_TYPE_NED:
      hits=hg->obj[i_list].magdb_ned_hits;
      break;
      
    case MAGDB_TYPE_LAMOST:
      hits=hg->obj[i_list].magdb_lamost_hits;
      break;
      
    case MAGDB_TYPE_GSC:
      hits=hg->obj[i_list].magdb_gsc_hits;
      break;
	
    case MAGDB_TYPE_PS1:
      hits=hg->obj[i_list].magdb_ps1_hits;
      break;
      
    case MAGDB_TYPE_SDSS:
      hits=hg->obj[i_list].magdb_sdss_hits;
      break;
	
    case MAGDB_TYPE_GAIA:
      hits=hg->obj[i_list].magdb_gaia_hits;
      break;
	
    case MAGDB_TYPE_2MASS:
      hits=hg->obj[i_list].magdb_2mass_hits;
      break;
    }

    flag_get=FALSE;
    switch(hg->fcdb_type){
    case MAGDB_TYPE_GSC:
    case MAGDB_TYPE_PS1:
    case MAGDB_TYPE_SDSS:
    case MAGDB_TYPE_GAIA:
    case MAGDB_TYPE_2MASS:
      if ((!hg->magdb_skip)||
	  ((hits<0)&&(hg->obj[i_list].mag>99))) flag_get=TRUE;
      break;

    default:
      if(hits<0) flag_get=TRUE;
      break;
    }

    if(flag_get){
      hg->fcdb_i=i_list;
      
      object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
      object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);
      
      ln_get_equ_prec2 (&object, 
			get_julian_day_of_epoch(hg->obj[hg->fcdb_i].equinox),
			JD2000, &object_prec);
      
      switch(hg->fcdb_type){
      case MAGDB_TYPE_GSC:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_GSC);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->magdb_mag);
	
	hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      (double)hg->magdb_arcsec/60./60.,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_PS1:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_PS1);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
    
	if(hg->fcdb_ps1_fil){
	  url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->magdb_mag);
	}
	else{
	  url_param=g_strdup("&");
	}
    
	hg->fcdb_path=g_strdup_printf(FCDB_PS1_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      (gdouble)hg->magdb_arcsec/60./60.,
				      2,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	
	break;
	
      case MAGDB_TYPE_SDSS:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_SDSS);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	hg->fcdb_path=g_strdup(FCDB_SDSS_PATH);
	

	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;

	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;
	
      case MAGDB_TYPE_GAIA:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_GAIA);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&%%3CGmag%%3E=%%3C%d&",hg->magdb_mag);
	
	hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      hg->magdb_arcsec,
				      hg->magdb_arcsec,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;
	
      case MAGDB_TYPE_2MASS:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_2MASS);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	if(hg->fcdb_2mass_fil){
	  url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->magdb_mag);
	}
	else{
	  url_param=g_strdup("&");
	}
    
	hg->fcdb_path=g_strdup_printf(FCDB_2MASS_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      (gdouble)hg->magdb_arcsec/60./60.,
				      url_param);

	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_SIMBAD:
	switch(hg->magdb_simbad_band){
	case FCDB_BAND_NOP:
	  mag_str=g_strdup("%0D%0A");
	  break;
	case FCDB_BAND_U:
	case FCDB_BAND_B:
	case FCDB_BAND_V:
	case FCDB_BAND_R:
	case FCDB_BAND_I:
	case FCDB_BAND_J:
	case FCDB_BAND_H:
	case FCDB_BAND_K:
	  mag_str=g_strdup_printf("%%26%smag<%d",
				  simbad_band[hg->magdb_simbad_band],
				  hg->magdb_mag);
	  break;
	}
    
	otype_str=g_strdup("%0D%0A");
    
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	  hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
	}
	else{
	  hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
	}
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	if(hg->fcdb_d_dec0>0){
	  hg->fcdb_path=g_strdup_printf(FCDB_PATH,hg->fcdb_d_ra0,
					"%2B",hg->fcdb_d_dec0,
					(gdouble)hg->magdb_arcsec/60.,
					(gdouble)hg->magdb_arcsec/60.,
					mag_str,otype_str,
					MAX_FCDB);
	}
	else{
	  hg->fcdb_path=g_strdup_printf(FCDB_PATH,hg->fcdb_d_ra0,
					"%2D",-hg->fcdb_d_dec0,
					(gdouble)hg->magdb_arcsec/60.,
					(gdouble)hg->magdb_arcsec/60.,
					mag_str,otype_str,
					MAX_FCDB);
	}
	g_free(mag_str);
	g_free(otype_str);
	
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_NED:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_NED);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;

	otype_str=g_strdup("&");

	if(hobject_prec.dec.neg==0){
	  hg->fcdb_path=g_strdup_printf(FCDB_NED_PATH,
					hobject_prec.ra.hours,
					hobject_prec.ra.minutes,
					hobject_prec.ra.seconds,
					"%2B",hobject_prec.dec.degrees,
					hobject_prec.dec.minutes,
					hobject_prec.dec.seconds,
					hg->magdb_arcsec/60.,
					otype_str);
	}
	else{
	  hg->fcdb_path=g_strdup_printf(FCDB_NED_PATH,
					hobject_prec.ra.hours,
					hobject_prec.ra.minutes,
					hobject_prec.ra.seconds,
					"%2D",hobject_prec.dec.degrees,
					hobject_prec.dec.minutes,
					hobject_prec.dec.seconds,
					hg->magdb_arcsec/60.,
					otype_str);
	}
	g_free(otype_str);

	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_LAMOST:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST);
	
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	hg->fcdb_path=g_strdup(FCDB_LAMOST_PATH);

	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	break;

      default:
	break;
      }
      
#ifndef USE_WIN32
      act.sa_handler=magdb_signal;
      sigemptyset(&act.sa_mask);
      act.sa_flags=0;
      if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
	fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
      
      timer=g_timeout_add(100, 
			  (GSourceFunc)progress_timeout,
			  (gpointer)hg);
      
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar),
				"Downloading ...");
      
      unlink(hg->fcdb_file);
      
      get_fcdb(hg);
      gtk_main();
      g_source_remove(timer);
      
      if(flag_magdb_kill){
	flag_magdb_kill=FALSE;
	flag_magdb_finish=FALSE;
	break;
      }
      else{
	switch(hg->fcdb_type){
	case MAGDB_TYPE_SIMBAD:
	  fcdb_simbad_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_simbad_hits;
	  break;

	case MAGDB_TYPE_NED:
	  fcdb_ned_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_ned_hits;
	  break;

	case MAGDB_TYPE_LAMOST:
	  fcdb_lamost_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_lamost_hits;
	  break;

	case MAGDB_TYPE_GSC:
	  fcdb_gsc_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_gsc_hits;
	  break;
	  
	case MAGDB_TYPE_PS1:
	  fcdb_ps1_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_ps1_hits;
	  break;
	  
	case MAGDB_TYPE_SDSS:
	  fcdb_sdss_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_sdss_hits;
	  break;
	  
	case MAGDB_TYPE_GAIA:
	  fcdb_gaia_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_gaia_hits;
	  break;

	case MAGDB_TYPE_2MASS:
	  fcdb_2mass_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_2mass_hits;
	  break;
	}
	
	elapsed_sec=difftime(time(NULL),start_time);
	remaining_sec=elapsed_sec/(double)(i_list+1)
	  *(double)(hg->i_max-(i_list+1));
	
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar2),
				      (gdouble)(i_list+1)/(gdouble)(hg->i_max));
	sprintf(tmp,"Finished [ %d / %d ] Objects",i_list+1, hg->i_max);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
	
	if(hits>0){
#ifdef USE_GTK3
	  gtk_widget_override_color(stat_label,GTK_STATE_FLAG_NORMAL,&color_red);
#else
	  gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&color_red);
#endif
	}
	else{
#ifdef USE_GTK3
	  gtk_widget_override_color(stat_label,GTK_STATE_FLAG_NORMAL,&color_black);
#else
	  gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&color_black);
#endif
	}
	sprintf(tmp,"%s : hit %d-objects", hg->obj[i_list].name, 
		hits);
	gtk_label_set_text(GTK_LABEL(stat_label),tmp);

	if(remaining_sec>3600){
	  sprintf(tmp,"Estimated time left : %dhrs and %dmin", 
		  (int)(remaining_sec)/3600,
		  ((int)remaining_sec%3600)/60);
	}
	else if(remaining_sec>60){
	  sprintf(tmp,"Estimated time left : %dmin and %dsec", 
		  (int)(remaining_sec)/60,(int)remaining_sec%60);
	}
	else{
	  sprintf(tmp,"Estimated time left : %.0lfsec", 
		  remaining_sec);
	}
	gtk_label_set_text(GTK_LABEL(time_label),tmp);
	
	flag_magdb_finish=FALSE;
      }
    }
    else{ 
      gdouble mag=100;

      // flag_get=FALSE
      switch(hg->fcdb_type){
      case MAGDB_TYPE_SIMBAD:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_simbad_band){
	    case FCDB_BAND_NOP:
		mag=hg->obj[i_list].magdb_simbad_v;
	      break;
	    case FCDB_BAND_U:
		mag=hg->obj[i_list].magdb_simbad_u;
	      break;
	    case FCDB_BAND_B:
		mag=hg->obj[i_list].magdb_simbad_b;
	      break;
	    case FCDB_BAND_V:
		mag=hg->obj[i_list].magdb_simbad_v;
	      break;
	    case FCDB_BAND_R:
		mag=hg->obj[i_list].magdb_simbad_r;
	      break;
	    case FCDB_BAND_I:
		mag=hg->obj[i_list].magdb_simbad_i;
	      break;
	    case FCDB_BAND_J:
		mag=hg->obj[i_list].magdb_simbad_j;
	      break;
	    case FCDB_BAND_H:
		mag=hg->obj[i_list].magdb_simbad_h;
	      break;
	    case FCDB_BAND_K:
		mag=hg->obj[i_list].magdb_simbad_k;
	      break;
	    }
	    
	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_SIMBAD;
	      if(hg->magdb_simbad_band==FCDB_BAND_NOP)
		hg->obj[i_list].magdb_band=FCDB_BAND_V;
	      else
		hg->obj[i_list].magdb_band=hg->magdb_simbad_band;
	    }
	  }
	}
	break;

      case MAGDB_TYPE_GSC:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_gsc_band){
	    case GSC_BAND_U:
		mag=hg->obj[i_list].magdb_gsc_u;
	      break;
	    case GSC_BAND_B:
		mag=hg->obj[i_list].magdb_gsc_b;
	      break;
	    case GSC_BAND_V:
		mag=hg->obj[i_list].magdb_gsc_v;
	      break;
	    case GSC_BAND_R:
		mag=hg->obj[i_list].magdb_gsc_r;
	      break;
	    case GSC_BAND_I:
		mag=hg->obj[i_list].magdb_gsc_i;
	      break;
	    case GSC_BAND_J:
		mag=hg->obj[i_list].magdb_gsc_j;
	      break;
	    case GSC_BAND_H:
		mag=hg->obj[i_list].magdb_gsc_h;
	      break;
	    case GSC_BAND_K:
		mag=hg->obj[i_list].magdb_gsc_k;
	      break;
	    }

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_GSC;
	      hg->obj[i_list].magdb_band=hg->magdb_gsc_band;
	    }
	  }
	}
	break;

      case MAGDB_TYPE_PS1:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_ps1_band){
	    case PS1_BAND_G:
		mag=hg->obj[i_list].magdb_ps1_g;
	      break;
	    case PS1_BAND_R:
		mag=hg->obj[i_list].magdb_ps1_r;
	      break;
	    case PS1_BAND_I:
		mag=hg->obj[i_list].magdb_ps1_i;
	      break;
	    case PS1_BAND_Z:
		mag=hg->obj[i_list].magdb_ps1_z;
	      break;
	    case PS1_BAND_Y:
		mag=hg->obj[i_list].magdb_ps1_y;
	      break;
	    }

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_PS1;
	      hg->obj[i_list].magdb_band=hg->magdb_ps1_band;
	    }
	  }
	}
	break;

      case MAGDB_TYPE_SDSS:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_sdss_band){
	    case SDSS_BAND_U:
		mag=hg->obj[i_list].magdb_sdss_u;
	      break;
	    case SDSS_BAND_G:
		mag=hg->obj[i_list].magdb_sdss_g;
	      break;
	    case SDSS_BAND_R:
		mag=hg->obj[i_list].magdb_sdss_r;
	      break;
	    case SDSS_BAND_I:
		mag=hg->obj[i_list].magdb_sdss_i;
	      break;
	    case SDSS_BAND_Z:
		mag=hg->obj[i_list].magdb_sdss_z;
	      break;
	    }

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_SDSS;
	      hg->obj[i_list].magdb_band=hg->magdb_sdss_band;
	    }
	  }
	}
	break;

      case MAGDB_TYPE_GAIA:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    mag=hg->obj[i_list].magdb_gaia_g;

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_GAIA;
	      hg->obj[i_list].magdb_band=0;
	    }
	  }
	}
	break;

      case MAGDB_TYPE_2MASS:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_2mass_band){
	    case TWOMASS_BAND_J:
		mag=hg->obj[i_list].magdb_2mass_j;
	      break;
	    case TWOMASS_BAND_H:
		mag=hg->obj[i_list].magdb_2mass_h;
	      break;
	    case TWOMASS_BAND_K:
		mag=hg->obj[i_list].magdb_2mass_k;
	      break;
	    }

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_2MASS;
	      hg->obj[i_list].magdb_band=hg->magdb_2mass_band;
	    }
	  }
	}
	break;
      }
    }
  }


  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  make_trdb_label(hg);
  gtk_label_set_text(GTK_LABEL(hg->trdb_label), hg->trdb_label_text);
  update_objtree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_OBJ);

  flag_getFCDB=FALSE;
}

gboolean check_magdb (gpointer gdata){
  if(flag_magdb_finish){
    flag_magdb_finish=FALSE;
      gtk_main_quit();
  }
  return(TRUE);
}


