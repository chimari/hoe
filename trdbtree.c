//    HDS OPE file Editor
//      trdbtree.c  --- Database Query List
//   
//                                           2018.1.24  A.Tajitsu


#include"main.h"    // 設定ヘッダ
#include"version.h"

void cc_radio();
void cancel_trdb();
#ifndef USE_WIN32
void trdb_signal();
#endif
static void ok_trdb_smoka();
static void ok_trdb_hst();
static void ok_trdb_eso();
static void ok_trdb_gemini();
//void trdb_smoka();
//void trdb_hst();
//void trdb_eso();
//void trdb_gemini();
void trdb_make_tree();
static void trdb_add_columns();
static GtkTreeModel *trdb_create_items_model();
void trdb_tree_update_azel_item();
static void trdb_focus_item ();
//void trdb_simbad();
//void trdb_append_tree();
void rebuild_trdb_tree();
void trdb_run();
gboolean check_trdb();
void clear_trdb();
void make_trdb_label();

gboolean flag_trdb_kill=FALSE;
gboolean  flag_trdb_finish=FALSE;

void cc_radio(GtkWidget *button, gint *gdata)
{ 
  GSList *group=NULL;

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(group); i++){
      w = g_slist_nth_data(group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	*gdata  = g_slist_length(group) -1 - i;
	break;
      }
    }
  }
}


void cancel_trdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;
  hg=(typHOE *)gdata;

  flag_trdb_kill=TRUE;

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
void trdb_signal(int sig){
  pid_t child_pid=0;

  flag_trdb_finish=TRUE;

  do{
    int child_ret;
    child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
  
}
#endif

static void ok_trdb_smoka(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if((!hg->trdb_smoka_imag)
     &&(!hg->trdb_smoka_spec)
     &&(!hg->trdb_smoka_ipol)){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Please select at least one observation mode.",
		  NULL);
#else
      g_print ("Error: Please select at least one observation mode.\n");
#endif
  }
  else{
    gtk_main_quit();

    trdb_run(hg);

    hg->fcdb_type=hg->fcdb_type_tmp;
    hg->trdb_used=TRDB_TYPE_SMOKA;
    hg->trdb_smoka_inst_used=hg->trdb_smoka_inst;
    hg->trdb_smoka_shot_used=hg->trdb_smoka_shot;
    hg->trdb_smoka_imag_used=hg->trdb_smoka_imag;
    hg->trdb_smoka_spec_used=hg->trdb_smoka_spec;
    hg->trdb_smoka_ipol_used=hg->trdb_smoka_ipol;
    hg->trdb_arcmin_used=hg->trdb_arcmin;
    if(hg->trdb_smoka_date_used) g_free(hg->trdb_smoka_date_used);
    hg->trdb_smoka_date_used=g_strdup(hg->trdb_smoka_date);
  }
}


static void ok_trdb_hst(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  trdb_run(hg);

  hg->fcdb_type=hg->fcdb_type_tmp;
  hg->trdb_used=TRDB_TYPE_HST;
  hg->trdb_hst_mode_used  =hg->trdb_hst_mode;
  hg->trdb_hst_image_used =hg->trdb_hst_image;
  hg->trdb_hst_spec_used  =hg->trdb_hst_spec;
  hg->trdb_hst_other_used =hg->trdb_hst_other;
  hg->trdb_arcmin_used=hg->trdb_arcmin;
  if(hg->trdb_hst_date_used) g_free(hg->trdb_hst_date_used);
  hg->trdb_hst_date_used=g_strdup(hg->trdb_hst_date);
}


static void ok_trdb_eso(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  trdb_run(hg);

  hg->fcdb_type=hg->fcdb_type_tmp;
  hg->trdb_used=TRDB_TYPE_ESO;
  hg->trdb_eso_mode_used  =hg->trdb_eso_mode;
  hg->trdb_eso_image_used =hg->trdb_eso_image;
  hg->trdb_eso_spec_used  =hg->trdb_eso_spec;
  hg->trdb_eso_vlti_used =hg->trdb_eso_vlti;
  hg->trdb_eso_pola_used =hg->trdb_eso_pola;
  hg->trdb_eso_coro_used =hg->trdb_eso_coro;
  hg->trdb_eso_other_used =hg->trdb_eso_other;
  hg->trdb_eso_sam_used =hg->trdb_eso_sam;
  hg->trdb_arcmin_used=hg->trdb_arcmin;
  if(hg->trdb_eso_stdate_used) g_free(hg->trdb_eso_stdate_used);
  hg->trdb_eso_stdate_used=g_strdup(hg->trdb_eso_stdate);
  if(hg->trdb_eso_eddate_used) g_free(hg->trdb_eso_eddate_used);
  hg->trdb_eso_eddate_used=g_strdup(hg->trdb_eso_eddate);
}


static void ok_trdb_gemini(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  trdb_run(hg);

  hg->fcdb_type=hg->fcdb_type_tmp;
  hg->trdb_used=TRDB_TYPE_GEMINI;
  hg->trdb_gemini_inst_used  =hg->trdb_gemini_inst;
  hg->trdb_gemini_mode_used  =hg->trdb_gemini_mode;
  hg->trdb_arcmin_used=hg->trdb_arcmin;
  if(hg->trdb_gemini_date_used) g_free(hg->trdb_gemini_date_used);
  hg->trdb_gemini_date_used=g_strdup(hg->trdb_gemini_date);
}


void trdb_smoka (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
#else
    g_print ("Error: Please load your object list.\n");
#endif
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_SMOKA;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : SMOKA List Query");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  table = gtk_table_new(2,5,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Instrument");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_SMOKA_SUBARU;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, smoka_subaru[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_smoka_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_smoka_inst);
  }

  check = gtk_check_button_new_with_label("Shot (ONLY for Suprime-Cam & Hyper Suprime-Cam)");
  gtk_table_attach(GTK_TABLE(table), check, 0, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_shot);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_shot);

  label = gtk_label_new ("Observation Mode");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  check = gtk_check_button_new_with_label("IMAG");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_imag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_imag);

  check = gtk_check_button_new_with_label("SPEC");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_spec);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_spec);

  check = gtk_check_button_new_with_label("IPOL");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_ipol);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_ipol);


  label = gtk_label_new ("Search Radius");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_smoka_date);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_smoka_date);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    ok_trdb_smoka, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_TRDB);
}


void trdb_hst (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[3];
  GSList *group;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
#else
    g_print ("Error: Please load your object list.\n");
#endif
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_HST;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : HST archive List Query");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  table = gtk_table_new(2,5,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Imaging");
  gtk_table_attach(GTK_TABLE(table), rb[0], 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_hst_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtk_table_attach(GTK_TABLE(table), rb[1], 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_hst_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Other");
  gtk_table_attach(GTK_TABLE(table), rb[2], 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_hst_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_IMAGE;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_image[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_image==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_image);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_SPEC;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_spec[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_spec==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_spec);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_OTHER;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_other[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_other==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_other);
  }

  label = gtk_label_new ("Search Radius");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_hst_date);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_hst_date);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    ok_trdb_hst, (gpointer)hg);

  gtk_widget_show_all(dialog);

  if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_hst_mode==TRDB_HST_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_hst_mode==TRDB_HST_MODE_OTHER)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);

  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_TRDB);
}

void trdb_eso (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[7];
  GSList *group;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
#else
    g_print ("Error: Please load your object list.\n");
#endif
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_ESO;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : ESO archive List Query");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  table = gtk_table_new(2,9,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Imaging");
  gtk_table_attach(GTK_TABLE(table), rb[0], 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtk_table_attach(GTK_TABLE(table), rb[1], 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Interferometry");
  gtk_table_attach(GTK_TABLE(table), rb[2], 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[3]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Polarimetry");
  gtk_table_attach(GTK_TABLE(table), rb[3], 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[3], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[4]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Coronagraphy");
  gtk_table_attach(GTK_TABLE(table), rb[4], 0, 1, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[4], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[5]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Other");
  gtk_table_attach(GTK_TABLE(table), rb[5], 0, 1, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[5], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[6]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Sparse Aperture Mask");
  gtk_table_attach(GTK_TABLE(table), rb[6], 0, 1, 6, 7,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[6], "toggled", cc_radio, &hg->trdb_eso_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_IMAGE;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_image[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_image==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_image);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_SPEC;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_spec[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_spec==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_spec);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_VLTI;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_vlti[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_vlti==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_vlti);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_POLA;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_pola[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_pola==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_pola);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_CORO;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_coro[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_coro==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_coro);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_OTHER;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_other[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_other==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_other);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_SAM;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_sam[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_sam==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 6, 7,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_sam);
  }

  label = gtk_label_new ("Search Radius");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 7, 8,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 7, 8,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 8, 9,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("Start Date");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_eso_stdate);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),15);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_eso_stdate);

  label = gtk_label_new ("  End Date");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_eso_eddate);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),15);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_eso_eddate);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    ok_trdb_eso, (gpointer)hg);

  gtk_widget_show_all(dialog);

  if(hg->trdb_eso_mode==TRDB_ESO_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_VLTI)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_POLA)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[3]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_CORO)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[4]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_OTHER)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[5]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SAM)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[6]),TRUE);

  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_TRDB);
}


void trdb_gemini (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[3];
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;

  if(hg->i_max<=0){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Please load your object list.",
		  NULL);
#else
    g_print ("Error: Please load your object list.\n");
#endif
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg->fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_GEMINI;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Gemini archive List Query");
  my_signal_connect(dialog,"delete-event",gtk_main_quit, NULL);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  label = gtk_label_new ("Gemini Instrument");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=1;i_inst<NUM_GEMINI_INST;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, gemini_inst[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_gemini_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_gemini_inst);
  }


  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Any");
  gtk_box_pack_start(GTK_BOX(hbox), rb[0], FALSE, FALSE, 0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_gemini_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Imaging");
  gtk_box_pack_start(GTK_BOX(hbox), rb[1], FALSE, FALSE, 0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_gemini_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtk_box_pack_start(GTK_BOX(hbox), rb[2], FALSE, FALSE, 0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_gemini_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));


  label = gtk_label_new ("Search Radius");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_gemini_date);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_gemini_date);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    ok_trdb_gemini, (gpointer)hg);

  gtk_widget_show_all(dialog);

  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_ANY)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);

  gtk_main();
 
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  rebuild_trdb_tree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_TRDB);
}


void trdb_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GdkColor col_red={0,0xFFFF,0,0};
  GdkColor col_black={0,0,0,0};


  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));

  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    trdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
 } 

  make_trdb_label(hg);
  gtk_label_set_text(GTK_LABEL(hg->trdb_label), hg->trdb_label_text);
}


static void
trdb_add_columns (typHOE *hg,
		 GtkTreeView  *treeview, 
		 GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  
  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_TRDB_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
					    renderer,
					    "text",
					    COLUMN_TRDB_NUMBER,
					    NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_TRDB_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_TRDB_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_TRDB_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_TRDB_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Data column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_TRDB_DATA));
  column=gtk_tree_view_column_new_with_attributes ("Data",
						   renderer,
						   "text", 
						   COLUMN_TRDB_DATA,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_TRDB_DATA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}

static GtkTreeModel *
trdb_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_TRDB, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // OPENUM
			      G_TYPE_STRING,  // name
			      G_TYPE_STRING);  // Data

  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (model, &iter);
    trdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}

void trdb_tree_update_azel_item(typHOE *hg, 
				GtkTreeModel *model, 
				GtkTreeIter iter, 
				gint i_list)
{
  gchar tmp[12];

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_TRDB_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_TRDB_NAME,
		      hg->obj[i_list].name,
		      -1);

  // Data
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_TRDB_DATA, 
		     hg->obj[i_list].trdb_str, -1);
}



static void trdb_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->trdb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->trdb_tree));
  gint i, i_list;
  GtkTreePath *path;
  
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_TRDB_NUMBER, &i, -1);
      i--;
      hg->trdb_tree_focus=i;
      
      gtk_tree_path_free (path);
  }
}


void trdb_simbad (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gchar *tmp;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->trdb_tree));

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gint fcdb_type_old;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_TRDB_NUMBER, &i, -1);
    i--;

    object.ra=ra_to_deg(hg->obj[i].ra);
    object.dec=dec_to_deg(hg->obj[i].dec);

    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].equinox),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->trdb_used){
    case TRDB_TYPE_SMOKA:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=TRDB_TYPE_WWWDB_SMOKA;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_SMOKA "/");


#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;

    case TRDB_TYPE_HST:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=TRDB_TYPE_WWWDB_HST;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_HST);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_HST_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_HST "/");

#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;

    case TRDB_TYPE_ESO:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=FCDB_TYPE_WWWDB_ESO;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_ESO);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_ESO_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_ESO "/");

#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;

    case TRDB_TYPE_GEMINI:
      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      {
	gchar *g_inst;
	gchar *g_mode;

	g_inst=g_strdup_printf("/%s/",gemini_inst[hg->trdb_gemini_inst].prm);
	switch(hg->trdb_gemini_mode){
	case TRDB_GEMINI_MODE_ANY:
	  g_mode=g_strdup("/");
	  break;

	case TRDB_GEMINI_MODE_IMAGE:
	  g_mode=g_strdup("/imaging/");
	  break;

	case TRDB_GEMINI_MODE_SPEC:
	  g_mode=g_strdup("/spectrosocpy/");
	  break;
	}

	tmp=g_strdup_printf(TRDB_GEMINI_URL,
			    hg->trdb_arcmin_used*60,
			    g_inst,
			    hg->fcdb_d_ra0,	
			    hg->trdb_gemini_date,
			    g_mode,
			    (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
			    fabs(hg->fcdb_d_dec0));
	g_free(g_inst);
	g_free(g_mode);
      }

      break;
    }

#ifndef USE_WIN32
    if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
  }
#endif

#ifdef USE_WIN32
    ShellExecute(NULL, 
		 "open", 
		 tmp,
		 NULL, 
		 NULL, 
		 SW_SHOWNORMAL);
#elif defined(USE_OSX)
    if(system(tmp)==0){
      fprintf(stderr, "Error: Could not open the default www browser.");
    }
#else
    cmdline=g_strconcat(hg->www_com," ",tmp,NULL);
    
    ext_play(cmdline);
    g_free(cmdline);
#endif
    if(tmp) g_free(tmp);
    
    gtk_tree_path_free (path);
  }
}


void trdb_append_tree(typHOE *hg){
  GtkTreeModel *items_model;

  /* create models */
  items_model = trdb_create_items_model (hg);
  
  /* create tree view */
  hg->trdb_tree = gtk_tree_view_new_with_model (items_model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->trdb_tree), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->trdb_tree)),
			       GTK_SELECTION_SINGLE);
  trdb_add_columns (hg, GTK_TREE_VIEW (hg->trdb_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (hg->trdb_sw), hg->trdb_tree);

  my_signal_connect (hg->trdb_tree, "cursor-changed",
		     G_CALLBACK (trdb_focus_item), (gpointer)hg);
}    

void rebuild_trdb_tree(typHOE *hg)
{
  gtk_widget_destroy(GTK_WIDGET(hg->trdb_tree));

  hg->trdb_i_max=0;

  trdb_append_tree(hg);
  gtk_widget_show(hg->trdb_tree);
}


void trdb_run (typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gint i_list, i_band;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *sep, *time_label, *stat_label,
    *hbox;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint fcdb_tree_check_timer;
  gint timer=-1;
  gchar tmp[BUFFSIZE];
  GdkColor col_red={0,0xFFFF,0,0};
  GdkColor col_black={0,0,0,0};
  time_t start_time;
  double elapsed_sec, remaining_sec;
  
  if(hg->i_max<=0) return;
  if(flag_getFCDB) return;

  clear_trdb(hg);

  flag_getFCDB=TRUE;

  for(i_list=0;i_list<hg->i_max;i_list++){
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      if(hg->obj[i_list].trdb_band[i_band]) 
	g_free(hg->obj[i_list].trdb_band[i_band]);
      hg->obj[i_list].trdb_band[i_band]=NULL;
      if(hg->obj[i_list].trdb_mode[i_band]) 
	g_free(hg->obj[i_list].trdb_mode[i_band]);
      hg->obj[i_list].trdb_mode[i_band]=NULL;
      hg->obj[i_list].trdb_exp[i_band]=0;
      hg->obj[i_list].trdb_shot[i_band]=0;
    }
    if(hg->obj[i_list].trdb_str) g_free(hg->obj[i_list].trdb_str);
    hg->obj[i_list].trdb_str=NULL;
    hg->obj[i_list].trdb_band_max=0;
  }

  dialog = gtk_dialog_new();
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Running List Query");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event",cancel_trdb, (gpointer)hg);
 
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);

  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in SMOKA ...");
    break;
    
  case TRDB_TYPE_HST:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in HST archive ...");
    break;
    
  case TRDB_TYPE_ESO:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case TRDB_TYPE_GEMINI:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);

  hg->pbar2=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar2,TRUE,TRUE,0);
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				    GTK_PROGRESS_LEFT_TO_RIGHT);
  sprintf(tmp,"Searching [ 1 / %d ] Objects", hg->i_max);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
  gtk_widget_show(hg->pbar2);

  sprintf(tmp,"Estimated time left : ---");
  time_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     time_label,TRUE,TRUE,5);

  sep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     sep,FALSE,TRUE,5);

  sprintf(tmp,"%s : hit ---", hg->obj[0].name);
  stat_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     stat_label,TRUE,TRUE,5);

  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:
    hg->plabel=gtk_label_new("Searching objects in SMOKA ...");
    break;
    
  case TRDB_TYPE_HST:
    hg->plabel=gtk_label_new("Searching objects in HST archive ...");
    break;
    
  case TRDB_TYPE_ESO:
    hg->plabel=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case TRDB_TYPE_GEMINI:
    hg->plabel=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);

  hbox = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     hbox,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(hbox),hg->plabel,FALSE,TRUE,0);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,0);
  my_signal_connect(button,"pressed",cancel_trdb,(gpointer)hg);


  gtk_widget_show_all(dialog);

  start_time=time(NULL);

  fcdb_tree_check_timer=g_timeout_add(1000, 
				      (GSourceFunc)check_trdb,
				      (gpointer)hg);

  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->fcdb_i=i_list;

    object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
    object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);

    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[hg->fcdb_i].equinox),
		      JD2000, &object_prec);
  
    switch(hg->fcdb_type){
    case TRDB_TYPE_SMOKA:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_TXT,NULL);

      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;

      break;

    case TRDB_TYPE_HST:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_HST);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_HST_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;
      
      break;
      
    case TRDB_TYPE_ESO:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_ESO);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_ESO_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;
      break;

    case TRDB_TYPE_GEMINI:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_GEMINI);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      {
	gchar *g_inst;
	gchar *g_mode;

	g_inst=g_strdup_printf("/%s/",gemini_inst[hg->trdb_gemini_inst].prm);
	switch(hg->trdb_gemini_mode){
	case TRDB_GEMINI_MODE_ANY:
	  g_mode=g_strdup("/");
	  break;

	case TRDB_GEMINI_MODE_IMAGE:
	  g_mode=g_strdup("/imaging/");
	  break;

	case TRDB_GEMINI_MODE_SPEC:
	  g_mode=g_strdup("/spectrosocpy/");
	  break;
	}

	hg->fcdb_path=g_strdup_printf(TRDB_GEMINI_PATH,
				      hg->trdb_arcmin*60,
				      g_inst,
				      hg->fcdb_d_ra0,	
				      hg->trdb_gemini_date,
				      g_mode,
				      (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				      fabs(hg->fcdb_d_dec0));
	g_free(g_inst);
	g_free(g_mode);
      }
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_JSON,NULL);
      
      break;

    default:
      break;
    }

#ifndef USE_WIN32
    act.sa_handler=trdb_signal;
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
    gtk_timeout_remove(timer);

    if(flag_trdb_kill){
      flag_trdb_kill=FALSE;
      flag_trdb_finish=FALSE;
      break;
    }
    else{
      switch(hg->fcdb_type){
      case TRDB_TYPE_SMOKA:
	trdb_smoka_txt_parse(hg);
	break;

      case TRDB_TYPE_HST:
	trdb_hst_vo_parse(hg);
	break;

      case TRDB_TYPE_ESO:
	trdb_eso_vo_parse(hg);
	break;

      case TRDB_TYPE_GEMINI:
	trdb_gemini_json_parse(hg);
	break;
      }

      elapsed_sec=difftime(time(NULL),start_time);
      remaining_sec=elapsed_sec/(double)(i_list+1)
	*(double)(hg->i_max-(i_list+1));
      
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar2),
				    (gdouble)(i_list+1)/(gdouble)(hg->i_max));
      sprintf(tmp,"Finished [ %d / %d ] Objects",i_list+1, hg->i_max);
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);

      if(hg->obj[i_list].trdb_band_max>0){
	gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&col_red);
      }
      else{
	gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&col_black);
      }
      sprintf(tmp,"%s : hit %d-bands", hg->obj[i_list].name, 
	      hg->obj[i_list].trdb_band_max);
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
      
      flag_trdb_finish=FALSE;
    }
  }

  gtk_timeout_remove(fcdb_tree_check_timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  make_trdb_label(hg);
  trdb_make_tree(hg);

  rebuild_trdb_tree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_TRDB);

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
  gtk_list_store_clear (GTK_LIST_STORE(model));
  hg->fcdb_i_max=0;

  if((hg->skymon_mode==SKYMON_CUR) || (hg->skymon_mode==SKYMON_SET))
    draw_skymon_cairo(hg->skymon_dw,hg, TRUE);

  flag_getFCDB=FALSE;
}

gboolean check_trdb (gpointer gdata){
  if(flag_trdb_finish){
    flag_trdb_finish=FALSE;
      gtk_main_quit();
  }
  return(TRUE);
}


void clear_trdb(typHOE *hg){
  gint i_list, i_band;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      if(hg->obj[i_list].trdb_band[i_band]){
	g_free(hg->obj[i_list].trdb_band[i_band]);
	hg->obj[i_list].trdb_band[i_band]=NULL;
      }
      if(hg->obj[i_list].trdb_mode[i_band]){
	g_free(hg->obj[i_list].trdb_mode[i_band]);
	hg->obj[i_list].trdb_mode[i_band]=NULL;
      }
      hg->obj[i_list].trdb_exp[i_band]=0;
      hg->obj[i_list].trdb_shot[i_band]=0;
    }
    if(hg->obj[i_list].trdb_str){
      g_free(hg->obj[i_list].trdb_str);
      hg->obj[i_list].trdb_str=NULL;
    }
    hg->obj[i_list].trdb_band_max=0;
  }

  hg->trdb_i_max=0;
}


void trdb_dbtab (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->trdb_tree));

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gint fcdb_type_old;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_TRDB_NUMBER, &i, -1);
    i--;

    object.ra=ra_to_deg(hg->obj[i].ra);
    object.dec=dec_to_deg(hg->obj[i].dec);

    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].equinox),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->trdb_used){
    case TRDB_TYPE_SMOKA:
      if(hg->fcdb_type!=FCDB_TYPE_SMOKA){
	hg->fcdb_type=FCDB_TYPE_SMOKA;
	rebuild_fcdb_tree(hg);
      }
      hg->fcdb_type=TRDB_TYPE_FCDB_SMOKA;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_TXT,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      fcdb_smoka_txt_parse(hg);

      hg->fcdb_type=FCDB_TYPE_SMOKA;
      if(flagFC) gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SMOKA");
      break;

    case TRDB_TYPE_HST:
      if(hg->fcdb_type!=FCDB_TYPE_HST){
	hg->fcdb_type=FCDB_TYPE_HST;
	rebuild_fcdb_tree(hg);
      }
      hg->fcdb_type=TRDB_TYPE_FCDB_HST;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_HST);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_HST_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      fcdb_hst_vo_parse(hg);

      hg->fcdb_type=FCDB_TYPE_HST;
      if(flagFC) gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"HST archive");
      break;

    case TRDB_TYPE_ESO:
      if(hg->fcdb_type!=FCDB_TYPE_ESO){
	hg->fcdb_type=FCDB_TYPE_ESO;
	rebuild_fcdb_tree(hg);
      }
      hg->fcdb_type=TRDB_TYPE_FCDB_ESO;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_ESO);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_ESO_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      fcdb_eso_vo_parse(hg);

      hg->fcdb_type=FCDB_TYPE_ESO;
      if(flagFC) gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"ESO archive");
      break;

    case TRDB_TYPE_GEMINI:
      if(hg->fcdb_type!=FCDB_TYPE_GEMINI){
	hg->fcdb_type=FCDB_TYPE_GEMINI;
	rebuild_fcdb_tree(hg);
      }
      hg->fcdb_type=TRDB_TYPE_FCDB_GEMINI;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_GEMINI);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      {
	gchar *g_inst;
	gchar *g_mode;

	g_inst=g_strdup_printf("/%s/",gemini_inst[hg->trdb_gemini_inst_used].prm);
	switch(hg->trdb_gemini_mode_used){
	case TRDB_GEMINI_MODE_ANY:
	  g_mode=g_strdup("/");
	  break;

	case TRDB_GEMINI_MODE_IMAGE:
	  g_mode=g_strdup("/imaging/");
	  break;

	case TRDB_GEMINI_MODE_SPEC:
	  g_mode=g_strdup("/spectrosocpy/");
	  break;
	}

	hg->fcdb_path=g_strdup_printf(TRDB_GEMINI_PATH,
				      hg->trdb_arcmin_used*60,
				      g_inst,
				      hg->fcdb_d_ra0,
				      hg->trdb_gemini_date_used,
				      g_mode,
				      (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				      fabs(hg->fcdb_d_dec0));
	g_free(g_inst);
	g_free(g_mode);
      }

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_JSON,NULL);

      fcdb_dl(hg);
      fcdb_gemini_json_parse(hg);

      hg->fcdb_type=FCDB_TYPE_GEMINI;
      if(flagFC) gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"Gemini archive");
      break;
    }

    gtk_tree_path_free (path);
    fcdb_make_tree(NULL, hg);

    if(flagFC) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
					    TRUE);
    hg->fcdb_flag=TRUE;
    if(flagFC)  draw_fc_cairo(hg->fc_dw, hg);
  }
}

void make_trdb_label(typHOE *hg){
  if(hg->trdb_label_text) g_free(hg->trdb_label_text);

  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:    
    hg->trdb_label_text
      =g_strdup_printf("SMOKA List Query (%s) [%d/%d hits]", 
		       smoka_subaru[hg->trdb_smoka_inst].name,
		       hg->trdb_i_max, hg->i_max);
    break;

  case TRDB_TYPE_HST:
    switch(hg->trdb_hst_mode){
    case TRDB_HST_MODE_IMAGE:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Imaging : %s) [%d/%d hits]", 
			 hst_image[hg->trdb_hst_image].name,
			 hg->trdb_i_max, hg->i_max);
      break;

    case TRDB_HST_MODE_SPEC:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Spectroscopy : %s) [%d/%d hits]", 
			 hst_spec[hg->trdb_hst_spec].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_HST_MODE_OTHER:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Other : %s) [%d/%d hits]", 
			 hst_other[hg->trdb_hst_other].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    }
    break;

  case TRDB_TYPE_ESO:
    switch(hg->trdb_eso_mode){
    case TRDB_ESO_MODE_IMAGE:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Imaging : %s) [%d/%d hits]", 
			 eso_image[hg->trdb_eso_image].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_SPEC:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Spectroscopy : %s) [%d/%d hits]", 
			 eso_spec[hg->trdb_eso_spec].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_VLTI:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Interferometry : %s) [%d/%d hits]", 
			 eso_vlti[hg->trdb_eso_vlti].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_POLA:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Polarimetry : %s) [%d/%d hits]", 
			 eso_pola[hg->trdb_eso_pola].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_CORO:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Coronagraphy : %s) [%d/%d hits]", 
			 eso_coro[hg->trdb_eso_coro].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_OTHER:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Other : %s) [%d/%d hits]", 
			 eso_other[hg->trdb_eso_other].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_SAM:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Sparse Aperture Mask : %s) [%d/%d hits]", 
			 eso_sam[hg->trdb_eso_sam].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    }
    break;

  case TRDB_TYPE_GEMINI:
    hg->trdb_label_text
      =g_strdup_printf("Gemini archive List Query (%s) [%d/%d hits]", 
		       gemini_inst[hg->trdb_gemini_inst].name,
		       hg->trdb_i_max, hg->i_max);
    break;

  default:
    hg->trdb_label_text
      =g_strdup("hskymon List Query"); 
    break;
  }
}


void trdb_cc_search_text (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->trdb_search_text) g_free(hg->trdb_search_text);
  hg->trdb_search_text=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));

  hg->trdb_search_i=0;
  hg->trdb_search_imax=0;

  gtk_label_set_text(GTK_LABEL(hg->trdb_search_label),"      ");
}


void trdb_search_item (GtkWidget *widget, gpointer data)
{
  gint i;
  gchar *label_text;
  typHOE *hg = (typHOE *)data;
  gchar *up_text1, *up_text2, *up_obj1, *up_obj2;

  if(!hg->trdb_search_text) return;

  if(strlen(hg->trdb_search_text)<1){
    hg->trdb_search_imax=0;
    hg->trdb_search_i=0;

    gtk_label_set_text(GTK_LABEL(hg->trdb_search_label),"      ");
    return;
  }

  if(hg->trdb_search_imax==0){
    up_text1=g_ascii_strup(hg->trdb_search_text, -1);
    up_text2=strip_spc(up_text1);
    g_free(up_text1);
    for(i=0; i<hg->i_max; i++){
      up_obj1=g_ascii_strup(hg->obj[i].name, -1);
      up_obj2=strip_spc(up_obj1);
      g_free(up_obj1);
      if(g_strstr_len(up_obj2, -1, up_text2)!=NULL){
	hg->trdb_search_iobj[hg->trdb_search_imax]=i;
	hg->trdb_search_imax++;
      }
      else if(hg->obj[i].note){
	g_free(up_obj2);
	up_obj1=g_ascii_strup(hg->obj[i].note, -1);
	up_obj2=strip_spc(up_obj1);
	g_free(up_obj1);
	if(g_strstr_len(up_obj2, -1, up_text2)!=NULL){
	  hg->trdb_search_iobj[hg->trdb_search_imax]=i;
	  hg->trdb_search_imax++;
	}
      }
      g_free(up_obj2);
    }
    g_free(up_text2);
  }
  else{
    hg->trdb_search_i++;
    if(hg->trdb_search_i>=hg->trdb_search_imax) hg->trdb_search_i=0;
  }

  {
    if(hg->trdb_search_imax!=0){
      label_text=g_strdup_printf("%d/%d   ",
				 hg->trdb_search_i+1,
				 hg->trdb_search_imax);

      {
	gint i_list;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
	GtkTreePath *path;
	GtkTreeIter  iter;

	path=gtk_tree_path_new_first();
	
	for(i=0;i<hg->i_max;i++){
	  gtk_tree_model_get_iter (model, &iter, path);
	  gtk_tree_model_get (model, &iter, COLUMN_TRDB_NUMBER, &i_list, -1);
	  i_list--;

	  if(i_list==hg->trdb_search_iobj[hg->trdb_search_i]){
	    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),NOTE_TRDB);
	    gtk_widget_grab_focus (hg->trdb_tree);
	    gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->trdb_tree), path, NULL, FALSE);
	    break;
	  }
	  else{
	    gtk_tree_path_next(path);
	  }
	}
	gtk_tree_path_free(path);
      }
    }
    else{
      label_text=g_strdup_printf("%d/%d   ",
				 hg->trdb_search_i,
				 hg->trdb_search_imax);
    }
    gtk_label_set_text(GTK_LABEL(hg->trdb_search_label),label_text);
    g_free(label_text);
  }
}
