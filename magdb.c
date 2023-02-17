//    HDS OPE file Editor
//      magdb.c : Magnitude Search in Online Catalogs
//                                           2018.2.7  A.Tajitsu

#include"main.h"

////////////////////// Global Args //////////////////////
extern gboolean flagChildDialog;
extern gboolean flagSkymon;
extern gboolean flagPlot;
extern gboolean flagFC;
extern gboolean flagPlan;
extern gboolean flagPAM;
extern gboolean flagService;
extern gboolean flag_getFCDB;
extern gboolean flag_getDSS;
extern gboolean flag_make_obj_tree;
extern gboolean flag_make_line_tree;
extern gboolean flag_make_etc_tree;
extern gboolean flag_nodraw;

extern int debug_flg;

#ifndef USE_WIN32
extern pid_t fc_pid;
#endif
extern pid_t fcdb_pid;
extern pid_t stddb_pid;


void ircs_magdb();
void hds_magdb();
void magdb_run();
gboolean check_magdb();

gboolean flag_magdb_kill=FALSE;
gboolean  flag_magdb_finish=FALSE;



void find_magdb(typHOE *hg)
{
  magdb_run(hg);

  hg->fcdb_i_max=0;
  hg->trdb_used=hg->fcdb_type;
}


void magdb_gaia (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_GAIA;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in GAIA",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  frame = gtkut_frame_new ("<b>Proper Motion</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Import proper motions via GAIA?");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_pm);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_pm);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_kepler (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }
 
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_KEPLER;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in Kepler Input Catalog",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->magdb_mag);

  label = gtk_label_new (" mag in Kepler magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_gsc (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_GSC;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in GSC",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_ucac (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_UCAC;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in UCAC4",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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

    for(i_fil=0;i_fil<NUM_UCAC_BAND;i_fil++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, ucac_band[i_fil],
			 1, i_fil, -1);
      if(hg->magdb_ucac_band==i_fil) iter_set=iter;
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
		       &hg->magdb_ucac_band);
  }

  check = gtk_check_button_new_with_label("Skip targets w/Mags in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}



void ircs_magdb (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *table0, *entry, 
    *spinner, *hbox, *check, *frame, *frame0;
  GtkAdjustment *adj;
  GSList *group;
  gchar *tmp;
  gint result;

  dialog = gtk_dialog_new_with_buttons(NULL,
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_IRCS_GSC:
    tmp=g_strdup("HOE : NsIR Guide Star Selection w/GSC");
    break;

  case MAGDB_TYPE_IRCS_PS1:
    tmp=g_strdup("HOE : NsIR Guide Star Selection w/PanSTARRS-1");
    break;

  case MAGDB_TYPE_IRCS_GAIA:
    tmp=g_strdup("HOE : NsIR Guide Star Selection w/GAIA");
    break;
  }
  gtk_window_set_title(GTK_WINDOW(dialog),tmp);
  g_free(tmp);
  my_signal_connect(dialog,"delete-event",delete_main_quit, NULL);


  if(hg->fcdb_type==MAGDB_TYPE_IRCS_PS1){
    frame = gtkut_frame_new ("<b>Server Parameters</b>");
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
    
    table = gtkut_table_new(3, 5, FALSE, 5, 10, 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    label = gtk_label_new ("Release");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR1 (Old, Mean only)",
			 1, FCDB_PS1_OLD, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_OLD) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR1 (MAST)",
			 1, FCDB_PS1_DR_1, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_DR_1) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR2 (MAST)",
			 1, FCDB_PS1_DR_2, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_DR_2) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
      gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
      gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
      gtkut_table_attach(table, combo, 1, 2, 0, 1,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->fcdb_ps1_dr);
    }
    
    label = gtk_label_new ("Catalog");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Mean object",
			 1, FCDB_PS1_MODE_MEAN, -1);
      if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Stacked object",
			 1, FCDB_PS1_MODE_STACK, -1);
      if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_STACK) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
      gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
      gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
      gtkut_table_attach(table, combo, 1, 2, 1, 2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->fcdb_ps1_mode);
    }
    

    label = gtk_label_new ("Minimum nDetections");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->fcdb_ps1_mindet,
					      1, 25, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtkut_table_attach(table, spinner, 1, 2, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &hg->fcdb_ps1_mindet);
  }
  
  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 5, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  frame0 = gtkut_frame_new ("<b>Tip-Tilt guide star</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtkut_table_attach(table, frame0, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  table0 = gtkut_table_new(3, 2, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame0), table0);
  
  label = gtk_label_new ("Max. Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_r_ttgs,
					    30, 90, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->ircs_magdb_r_ttgs);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  

  label = gtk_label_new ("R-band Mag. <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_mag_ttgs,
					    15, 20, 0.1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->ircs_magdb_mag_ttgs);


  frame0 = gtkut_frame_new ("<b>Natural guide star</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtkut_table_attach(table, frame0, 0, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  table0 = gtkut_table_new(3, 2, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame0), table0);
  
  label = gtk_label_new ("Max. Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_r_ngs,
					    10, 40, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->ircs_magdb_r_ngs);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  label = gtk_label_new ("R-band Mag. <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_mag_ngs,
					    10, 18, 0.1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->ircs_magdb_mag_ngs);
  
 
  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 0, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("Radius for target identification <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_r_tgt,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->ircs_magdb_r_tgt);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 0, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  if(hg->fcdb_type==MAGDB_TYPE_IRCS_PS1){
    gtk_widget_set_sensitive(hbox, FALSE);
  }
  check = gtk_check_button_new_with_label("Exclude double star");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->ircs_magdb_dse);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->ircs_magdb_dse);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_dse_r1,
					    0.0, 0.5, 0.05, 1, 0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->ircs_magdb_dse_r1);

  label = gtk_label_new ("< Dist. <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_dse_r2,
					    0.5, 5.0, 0.1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->ircs_magdb_dse_r2);

  label = gtk_label_new ("arcsec  &  dMag. <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_magdb_dse_mag,
					    0.5, 3.0, 0.1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->ircs_magdb_dse_mag);

  
  check = gtk_check_button_new_with_label("Skip targets w/GSs.");
  gtkut_table_attach(table, check, 0, 3, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->ircs_magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->ircs_magdb_skip);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
  }
}


void ircs_magdb_gsc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_IRCS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_GSC;

  ircs_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}


void ird_magdb_gsc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_IRCS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_GSC;

  ircs_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}


void ircs_magdb_ps1 (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gboolean dse_flag;

  if(!CheckInst(hg, INST_IRCS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_PS1;

  dse_flag=hg->ircs_magdb_dse;
  hg->ircs_magdb_dse=FALSE;

  ircs_magdb(hg);

  hg->ircs_magdb_dse=dse_flag;
  
  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}


void ird_magdb_ps1 (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gboolean dse_flag;

  if(!CheckInst(hg, INST_IRD)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_PS1;

  dse_flag=hg->ircs_magdb_dse;
  hg->ircs_magdb_dse=FALSE;

  ircs_magdb(hg);

  hg->ircs_magdb_dse=dse_flag;
  
  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}

void ircs_magdb_gaia (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_IRCS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_GAIA;

  ircs_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}

void ird_magdb_gaia (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_IRD)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_IRCS_GAIA;

  ircs_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}



void hds_magdb (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *table0, *entry, 
    *spinner, *hbox, *check, *frame, *frame0;
  GtkAdjustment *adj;
  GSList *group;
  gchar *tmp;
  gint result;

  dialog = gtk_dialog_new_with_buttons(NULL,
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_HDS_GSC:
    tmp=g_strdup("HOE : HDS SV Guide mode selection w/GSC");
    break;

  case MAGDB_TYPE_HDS_GAIA:
    tmp=g_strdup("HOE : HDS SV Guide mode selection w/GAIA");
    break;
  }
  gtk_window_set_title(GTK_WINDOW(dialog),tmp);
  g_free(tmp);
  my_signal_connect(dialog,"delete-event",delete_main_quit, NULL);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 5, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  frame0 = gtkut_frame_new ("<b>Target Identification</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtkut_table_attach(table, frame0, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  table0 = gtkut_table_new(3, 2, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame0), table0);
  
  label = gtk_label_new ("Radius Allowance");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hds_magdb_r_tgt,
					    1.0, 10.0, 0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->hds_magdb_r_tgt);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_HDS_GSC:
    label = gtk_label_new ("Threshold for SemiAuto <--> Safe mode in R-band");
    break;

  case MAGDB_TYPE_HDS_GAIA:
    label = gtk_label_new ("Threshold for SemiAuto <--> Safe mode in G-band");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("= ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hds_magdb_mag_tgt,
					    10.0, 16.0, 0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->hds_magdb_mag_tgt);

  label = gtk_label_new (" mag");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  

  frame0 = gtkut_frame_new ("<b>Brightest star in FOV</b>  (r &lt; 40arcsec)");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtkut_table_attach(table, frame0, 0, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  table0 = gtkut_table_new(3, 2, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame0), table0);
  
  label = gtkut_label_new ("Use Safe-mode if &#x394; mag is &lt; +");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table0, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table0, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hds_magdb_mag_fov,
					    0.0, 4.0, 0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->hds_magdb_mag_fov);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_HDS_GSC:
    label = gtk_label_new (" mag in R-band");
    break;

  case MAGDB_TYPE_HDS_GAIA:
    label = gtk_label_new (" mag in G-band");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);


  frame = gtkut_frame_new ("<b>Caution for Double Stars</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 5, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Separation");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hds_magdb_r_ds,
					    1.0, 10.0, 0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->hds_magdb_r_ds);

  label = gtk_label_new (" arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  

  label = gtkut_label_new ("&#x394; mag.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< +");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hds_magdb_mag_ds,
					    0.0, 5.0, 0.1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj_double, &hg->hds_magdb_mag_ds);
  
  switch(hg->fcdb_type){
  case MAGDB_TYPE_HDS_GSC:
    label = gtk_label_new (" in R-band");
    break;

  case MAGDB_TYPE_HDS_GAIA:
    label = gtk_label_new (" in G-band");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  check = gtk_check_button_new_with_label("Skip targets already ckecked.");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->hds_magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->hds_magdb_skip);
  
  
  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result== GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }
}



void hds_magdb_gsc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_HDS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_HDS_GSC;

  hds_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}


void hds_magdb_gaia (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(!CheckInst(hg, INST_HDS)) return;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_HDS_GAIA;

  hds_magdb(hg);

  make_obj_tree(hg);

  hg->fcdb_type=fcdb_type_tmp;
}



void magdb_ps1 (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_PS1;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in PanSTARRS",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  {
    frame = gtkut_frame_new ("<b>Server Parameters</b>");
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
    
    table = gtkut_table_new(3, 5, FALSE, 5, 10, 5);
    gtk_container_add (GTK_CONTAINER (frame), table);
    label = gtk_label_new ("Release");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR1 (Old, Mean only)",
			 1, FCDB_PS1_OLD, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_OLD) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR1 (MAST)",
			 1, FCDB_PS1_DR_1, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_DR_1) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "DR2 (MAST)",
			 1, FCDB_PS1_DR_2, -1);
      if(hg->fcdb_ps1_dr==FCDB_PS1_DR_2) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
      gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
      gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
      gtkut_table_attach(table, combo, 1, 2, 0, 1,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->fcdb_ps1_dr);
    }
    
    label = gtk_label_new ("Catalog");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Mean object",
			 1, FCDB_PS1_MODE_MEAN, -1);
      if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Stacked object",
			 1, FCDB_PS1_MODE_STACK, -1);
      if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_STACK) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
      gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
      gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
      gtkut_table_attach(table, combo, 1, 2, 1, 2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->fcdb_ps1_mode);
    }
    

    label = gtk_label_new ("Minimum nDetections");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->fcdb_ps1_mindet,
					      1, 25, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtkut_table_attach(table, spinner, 1, 2, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &hg->fcdb_ps1_mindet);
  }

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_sdss (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_SDSS;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in SDSS",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_2mass (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_2MASS;

  dialog = gtk_dialog_new_with_buttons("HOE : Magnitude Search in 2MASS",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_simbad (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame, *vbox;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_SIMBAD;

  dialog = gtk_dialog_new_with_buttons("HOE : List Search in SIMBAD",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 6, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
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
  gtkut_table_attach(table, check, 0, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_skip);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_skip);

  frame = gtkut_frame_new (NULL);
  gtkut_table_attach(table, frame, 0, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
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


  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Overwrite existing Mag in the Main Target List.");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);


  frame = gtkut_frame_new ("<b>Proper Motion</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  check = gtk_check_button_new_with_label("Import proper motions via SIMBAD?");
  gtkut_table_attach(table, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_pm);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_pm);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void hsc_magdb_simbad (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame, *vbox;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;
  
  if(!CheckInst(hg, INST_HSC)) return;
  
  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_HSC_SIMBAD;
  fprintf(stderr, "ehe\n");fflush(stderr);

  dialog = gtk_dialog_new_with_buttons("HOE : Check bright stars in FOV via SIMBAD",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 6, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_magdb_arcmin,
					    90, 150, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->hsc_magdb_arcmin);

  label = gtk_label_new (" arcmin");
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
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("V < 12 mag");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  frame = gtkut_frame_new ("<b>Data update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new(3, 6, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  
  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  check = gtk_check_button_new_with_label("Overwrite existing values in the Main Target List.");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->magdb_ow);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->magdb_ow);

  gtk_widget_show_all(dialog);
  fprintf(stderr, "ehe1\n");fflush(stderr);

  result=gtk_dialog_run(GTK_DIALOG(dialog));
    fprintf(stderr, "ehe2\n");fflush(stderr);

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  fprintf(stderr, "ehe3\n");fflush(stderr);
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
  }
    fprintf(stderr, "ehe4\n");fflush(stderr);

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_ned (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_NED;

  dialog = gtk_dialog_new_with_buttons("HOE : List Search in NED",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 4, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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


  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("(This query never updates Mags in the list.)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_lamost (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *frame;
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;
  gint result;

  if(hg->i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=MAGDB_TYPE_LAMOST;

  dialog = gtk_dialog_new_with_buttons("HOE : List Search in LAMOST",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Find",GTK_RESPONSE_APPLY,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_FIND,GTK_RESPONSE_APPLY,
#endif
				       NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  frame = gtkut_frame_new ("<b>Data Release</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(1, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR5",
		       1, FCDB_LAMOST_DR5, -1);
    if(hg->fcdb_lamost_dr==FCDB_LAMOST_DR5) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR7 (Low Resolution)",
		       1, FCDB_LAMOST_DR7, -1);
    if(hg->fcdb_lamost_dr==FCDB_LAMOST_DR7) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR7 (Medium Resolution)",
		       1, FCDB_LAMOST_DR7M, -1);
    if(hg->fcdb_lamost_dr==FCDB_LAMOST_DR7M) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR8 (Low Resolution)",
		       1, FCDB_LAMOST_DR8, -1);
    if(hg->fcdb_lamost_dr==FCDB_LAMOST_DR8) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR8 (Medium Resolution)",
		       1, FCDB_LAMOST_DR8M, -1);
    if(hg->fcdb_lamost_dr==FCDB_LAMOST_DR8M) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
    gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
    gtkut_table_attach(table, combo, 0, 1, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->fcdb_lamost_dr);
  }
  

  frame = gtkut_frame_new ("<b>Search Parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 2, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3, 60, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
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


  frame = gtkut_frame_new ("<b>Mag update in the Main Target list</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(3, 1, FALSE, 5, 10, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("(This query never updates Mags in the list.)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  gtk_widget_show_all(dialog);

  result=gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  
  if (result == GTK_RESPONSE_APPLY) {
    find_magdb(hg);
    rebuild_trdb_tree(hg);
  }

  hg->fcdb_type=fcdb_type_tmp;
}


void magdb_run (typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gint i_list, i_band;
  GtkWidget *button, *hbox;
  gint fcdb_tree_check_timer;
  gint timer=-1;
  gchar *tmp;
  time_t start_time;
  double elapsed_sec, remaining_sec;
  gchar *url_param, *mag_str, *otype_str;
  gint hits=1;
  gboolean flag_get;
  gdouble yrs, new_d_ra, new_d_dec;
  
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

  tmp=g_strdup_printf("Searching objects in %s ...",
		      db_name[hg->fcdb_type]);
  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Running Catalog Matching Service",
		 tmp,
		 TRUE, TRUE);
  g_free(tmp);
  my_signal_connect(hg->pdialog,"delete-event",delete_fcdb, (gpointer)hg);

  gtk_label_set_markup(GTK_LABEL(hg->plabel),
		       "Retrieving image from website ...");

  switch(hg->fcdb_type){
  case MAGDB_TYPE_LAMOST:
  case MAGDB_TYPE_SDSS:
  case MAGDB_TYPE_KEPLER:
    hg->fcdb_post=TRUE;
    break;

  default:
    hg->fcdb_post=FALSE;
    break;
  }

  tmp=g_strdup_printf("Searching [ 1 / %d ] Objects", hg->i_max);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
  g_free(tmp);

  tmp=g_strdup("Estimated time left : ---");
  gtk_label_set_markup(GTK_LABEL(hg->plabel2), tmp);
  g_free(tmp);

  tmp=g_strdup_printf("%s : hit ---", hg->obj[0].name);
  gtk_label_set_markup(GTK_LABEL(hg->plabel3), tmp);
  g_free(tmp);

  switch(hg->fcdb_type){
  case MAGDB_TYPE_SIMBAD:
  case MAGDB_TYPE_HSC_SIMBAD:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in SIMBAD ...");
    break;

  case MAGDB_TYPE_NED:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in NED ...");
    break;

  case MAGDB_TYPE_LAMOST:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in LAMOST ...");
    break;

  case MAGDB_TYPE_GSC:
  case MAGDB_TYPE_IRCS_GSC:
  case MAGDB_TYPE_HDS_GSC:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in GSC ...");
    break;

  case MAGDB_TYPE_UCAC:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in UCAC4 ...");
    break;
    
  case MAGDB_TYPE_PS1:
  case MAGDB_TYPE_IRCS_PS1:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in PanSTARRS ...");
    break;

  case MAGDB_TYPE_SDSS:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in SDSS ...");
    break;

  case MAGDB_TYPE_GAIA:
  case MAGDB_TYPE_IRCS_GAIA:
  case MAGDB_TYPE_HDS_GAIA:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in GAIA ...");
    break;

  case MAGDB_TYPE_KEPLER:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in Kepler Input Catalog ...");
    break;

  case MAGDB_TYPE_2MASS:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in 2MASS ...");
    break;

  default:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in database ...");
    break;
  }

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",thread_cancel_fcdb,(gpointer)hg);


  gtk_widget_show_all(hg->pdialog);

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
	
    case MAGDB_TYPE_UCAC:
      hits=hg->obj[i_list].magdb_ucac_hits;
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
	
    case MAGDB_TYPE_KEPLER:
      hits=hg->obj[i_list].magdb_kepler_hits;
      break;
	
    case MAGDB_TYPE_2MASS:
      hits=hg->obj[i_list].magdb_2mass_hits;
      break;
    }

    flag_get=FALSE;
    switch(hg->fcdb_type){
    case MAGDB_TYPE_GSC:
    case MAGDB_TYPE_UCAC:
    case MAGDB_TYPE_PS1:
    case MAGDB_TYPE_SDSS:
    case MAGDB_TYPE_GAIA:
    case MAGDB_TYPE_KEPLER:
    case MAGDB_TYPE_2MASS:
      if ((!hg->magdb_skip)||
	  ((hits<0)&&(hg->obj[i_list].mag>99))) flag_get=TRUE;
      break;

    case MAGDB_TYPE_IRCS_GSC:
    case MAGDB_TYPE_IRCS_PS1:
    case MAGDB_TYPE_IRCS_GAIA:
      if(hg->obj[i_list].i_nst>=0){
	hg->obj[i_list].gs.flag=FALSE;
	hg->obj[i_list].aomode=AOMODE_NGS_S;
	flag_get=FALSE;
      }
      else if(hg->ircs_magdb_skip){
	if(hg->obj[i_list].gs.flag){
	  flag_get=FALSE;
	}
	else{
	  flag_get=TRUE;
	}
      }
      else{
	flag_get=TRUE;
      }

      /*
      if((fabs(hg->obj[hg->fcdb_i].pm_ra)>100)
	 ||(fabs(hg->obj[hg->fcdb_i].pm_dec)>100)){
	yrs=current_yrs(hg);
	new_d_ra=ra_to_deg(hg->obj[hg->fcdb_i].ra)+
	  hg->obj[hg->fcdb_i].pm_ra/1000/60/60*yrs;
	new_d_dec=dec_to_deg(hg->obj[i_list].dec)+
	  hg->obj[hg->fcdb_i].pm_dec/1000/60/60*yrs;

	object.ra=new_d_ra;
	object.dec=new_d_dec;
      }
      else{
	object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
	object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);
	}*/
      break;
      
    case MAGDB_TYPE_HSC_SIMBAD:
      if((hg->magdb_ow) || (hg->obj[i_list].hscmag.v>99)){
	flag_get=TRUE;
      }
      break;
      
    case MAGDB_TYPE_HDS_GSC:
    case MAGDB_TYPE_HDS_GAIA:
      if(hg->obj[i_list].i_nst>=0){
	hg->obj[i_list].guide=SVSAFE_GUIDE;
	flag_get=FALSE;
      }
      else{
	if(hg->hds_magdb_skip){
	  if(hg->obj[i_list].sv_checked){
	    flag_get=FALSE;
	  }
	  else{
	    flag_get=TRUE;
	  }
	}
	else{
	  flag_get=TRUE;
	}
      }
      break;
      
    default:
      if(hits<0) flag_get=TRUE;
      break;
    }

    if(flag_get){
      hg->fcdb_i=i_list;

      if((fabs(hg->obj[hg->fcdb_i].pm_ra)>100)
	 ||(fabs(hg->obj[hg->fcdb_i].pm_dec)>100)){
	yrs=current_yrs(hg);
	new_d_ra=ra_to_deg(hg->obj[hg->fcdb_i].ra)+
	  hg->obj[hg->fcdb_i].pm_ra/1000/60/60*yrs;
	new_d_dec=dec_to_deg(hg->obj[i_list].dec)+
	  hg->obj[hg->fcdb_i].pm_dec/1000/60/60*yrs;

	object.ra=new_d_ra;
	object.dec=new_d_dec;
      }
      else{
	object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
	object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);
      }
      
      
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
	
	url_param=g_strdup_printf("&MAGRANGE=0,20&");
	
	hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      HDS_MAGDB_R_ARCSEC/60./60.,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_IRCS_GSC:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_GSC);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&MAGRANGE=0,%.1lf&",hg->ircs_magdb_mag_ttgs);
	
	hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      (gdouble)hg->ircs_magdb_r_ttgs/60./60.,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_HDS_GSC:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_GSC);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&MAGRANGE=0,20.0&");
	
	hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      HDS_MAGDB_R_ARCSEC/60./60.,
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
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
    
	if(hg->fcdb_ps1_fil){
	  if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
	    url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->magdb_mag);
	  }
	  else{
	    url_param=g_strdup_printf((hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
				      "&rMeanPSFMag.lte=%d&"
				      : "&rPSFMag.lte=%d&",
				      hg->magdb_mag);
	  }
	}
	else{
	  url_param=g_strdup("&");
	}
    
	if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
	  hg->fcdb_host=g_strdup(FCDB_HOST_PS1OLD);
	  hg->fcdb_path=g_strdup_printf(FCDB_PS1OLD_PATH,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gdouble)hg->magdb_arcsec/60./60.,
					hg->fcdb_ps1_mindet,
					url_param);
	}
	else{
	  hg->fcdb_host=g_strdup(FCDB_HOST_PS1);
	  hg->fcdb_path=g_strdup_printf(FCDB_PS1_PATH,
					(hg->fcdb_ps1_dr==FCDB_PS1_DR_2) ?
					"dr2" : "dr1",
					(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
					"mean" : "stack",
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gdouble)hg->magdb_arcsec/60./60.,
					hg->fcdb_ps1_mindet,
					url_param);
	}
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	
	break;
	
      case MAGDB_TYPE_IRCS_PS1:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
	  hg->fcdb_host=g_strdup(FCDB_HOST_PS1OLD);
	  url_param=g_strdup_printf("&MAGRANGE=0,%lf&",hg->ircs_magdb_mag_ttgs);
	  hg->fcdb_path=g_strdup_printf(FCDB_PS1OLD_PATH,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gdouble)hg->ircs_magdb_r_ttgs/60./60.,
					hg->fcdb_ps1_mindet,
					url_param);
	}
	else{
	  hg->fcdb_host=g_strdup(FCDB_HOST_PS1);
	  url_param=g_strdup_printf((hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
				    "&rMeanPSFMag.lte=%.2lf&"
				    : "&rPSFMag.lte=%.2lf&",
				    hg->ircs_magdb_mag_ttgs);
	  hg->fcdb_path=g_strdup_printf(FCDB_PS1_PATH,
					(hg->fcdb_ps1_dr==FCDB_PS1_DR_2) ?
					"dr2" : "dr1",
					(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
					"mean" : "stack",
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gdouble)hg->ircs_magdb_r_ttgs/60./60.,
					hg->fcdb_ps1_mindet,
					url_param);
	}
    
	
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
	
      case MAGDB_TYPE_UCAC:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	switch(hg->fcdb_vizier){
	case FCDB_VIZIER_STRASBG:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
	  break;
	case FCDB_VIZIER_NAOJ:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
	  break;
	default:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
	  break;
	}
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&rmag=%%3C%d&",hg->magdb_mag);
	
	hg->fcdb_path=g_strdup_printf(FCDB_UCAC_PATH_R,
				      hg->fcdb_d_ra0,
				      hg->fcdb_d_dec0,
				      hg->magdb_arcsec,
				      url_param);
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;
	
      case MAGDB_TYPE_GAIA:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	switch(hg->fcdb_vizier){
	case FCDB_VIZIER_STRASBG:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
	  break;
	case FCDB_VIZIER_NAOJ:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
	  break;
	default:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
	  break;
	}
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&Gmag=%%3C%d&",hg->magdb_mag);
	
	switch(hg->gaia_dr){
	case GAIA_DR2:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->magdb_arcsec,
					url_param);
	  break;
	case GAIA_EDR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_E3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->magdb_arcsec,
					url_param);
	  break;
	case GAIA_DR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_DR3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->magdb_arcsec,
					url_param);
	  break;
	}
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;
	
      case MAGDB_TYPE_IRCS_GAIA:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	switch(hg->fcdb_vizier){
	case FCDB_VIZIER_STRASBG:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
	  break;
	case FCDB_VIZIER_NAOJ:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
	  break;
	default:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
	  break;
	}
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&Gmag=%%3C%.1lf&",hg->ircs_magdb_mag_ttgs);
	
	switch(hg->gaia_dr){
	case GAIA_DR2:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->ircs_magdb_r_ttgs,
					url_param);
	  break;
	case GAIA_EDR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_E3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->ircs_magdb_r_ttgs,
					url_param);
	  break;
	case GAIA_DR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_DR3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					hg->ircs_magdb_r_ttgs,
					url_param);
	  break;
	}
	
	if(url_param) g_free(url_param);
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	break;
	
      case MAGDB_TYPE_HDS_GAIA:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	switch(hg->fcdb_vizier){
	case FCDB_VIZIER_STRASBG:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
	  break;
	case FCDB_VIZIER_NAOJ:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
	  break;
	default:
	  hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
	  break;
	}
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	
	url_param=g_strdup_printf("&Gmag=%%3C20.0&");
	
	switch(hg->gaia_dr){
	case GAIA_DR2:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gint)HDS_MAGDB_R_ARCSEC,
					url_param);
	  break;
	case GAIA_EDR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_E3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gint)HDS_MAGDB_R_ARCSEC,
					url_param);
	  break;
	case GAIA_DR3:
	  hg->fcdb_path=g_strdup_printf(FCDB_GAIA_DR3_PATH_R,
					hg->fcdb_d_ra0,
					hg->fcdb_d_dec0,
					(gint)HDS_MAGDB_R_ARCSEC,
					url_param);
	  break;
	}
	
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
	
	hg->fcdb_path=g_strdup_printf(FCDB_SIMBAD_PATH_R,
				      hg->fcdb_d_ra0,
				      (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				      fabs(hg->fcdb_d_dec0),
				      (gdouble)hg->magdb_arcsec/60.,
				      mag_str,otype_str,
				      MAX_FCDB);
	g_free(mag_str);
	g_free(otype_str);
	
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
	break;

      case MAGDB_TYPE_HSC_SIMBAD:
	mag_str=g_strdup_printf("%%26Vmag<12");
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
	
	hg->fcdb_path=g_strdup_printf(FCDB_SIMBAD_PATH_R,
				      hg->fcdb_d_ra0,
				      (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				      fabs(hg->fcdb_d_dec0),
				      (gdouble)hg->hsc_magdb_arcmin/2.,
				      mag_str,otype_str,
				      MAX_FCDB);
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
	switch(hg->fcdb_lamost_dr){
	case FCDB_LAMOST_DR5:
	  hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST_DR5);
	  break;

	case FCDB_LAMOST_DR7:
	case FCDB_LAMOST_DR7M:
	  hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST_DR7);
	  break;
	  
	case FCDB_LAMOST_DR8:
	case FCDB_LAMOST_DR8M:
	  hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST_DR8);
	  break;
	}
	
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	switch(hg->fcdb_lamost_dr){
	case FCDB_LAMOST_DR5:
	  hg->fcdb_path=g_strdup(FCDB_LAMOST_DR5_PATH);
	  break;
	  
	case FCDB_LAMOST_DR7:
	  hg->fcdb_path=g_strdup(FCDB_LAMOST_DR7_PATH);
	  break;
	  
	case FCDB_LAMOST_DR7M:
	  hg->fcdb_path=g_strdup(FCDB_LAMOST_DR7_MED_PATH);
	  break;
	  
	case FCDB_LAMOST_DR8:
	  hg->fcdb_path=g_strdup(FCDB_LAMOST_DR8_PATH);
	  break;
	  
	case FCDB_LAMOST_DR8M:
	  hg->fcdb_path=g_strdup(FCDB_LAMOST_DR8_MED_PATH);
	  break;
	}
	
	if(hg->fcdb_file) g_free(hg->fcdb_file);
	hg->fcdb_file=g_strconcat(hg->temp_dir,
				  G_DIR_SEPARATOR_S,
				  FCDB_FILE_XML,NULL);
	
	hg->fcdb_d_ra0=object_prec.ra;
	hg->fcdb_d_dec0=object_prec.dec;
	break;

      case MAGDB_TYPE_KEPLER:
	ln_equ_to_hequ (&object_prec, &hobject_prec);
	if(hg->fcdb_host) g_free(hg->fcdb_host);
	hg->fcdb_host=g_strdup(FCDB_HOST_KEPLER);
	
	if(hg->fcdb_path) g_free(hg->fcdb_path);
	hg->fcdb_path=g_strdup(FCDB_KEPLER_PATH);

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

      timer=g_timeout_add(100, 
			  (GSourceFunc)progress_timeout,
			  (gpointer)hg);
      
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar),
				"Downloading ...");
      
      unlink(hg->fcdb_file);
      
      hg->ploop=g_main_loop_new(NULL, FALSE);
      hg->pcancel=g_cancellable_new();
      hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
      g_main_loop_run(hg->ploop);
      //g_thread_join(hg->pthread);
      g_main_loop_unref(hg->ploop);
      hg->ploop=NULL;

      if(hg->pabort) flag_magdb_kill=TRUE;
      
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
	  
	case MAGDB_TYPE_UCAC:
	  fcdb_ucac_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_ucac_hits;
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

	case MAGDB_TYPE_KEPLER:
	  fcdb_kepler_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_kepler_hits;
	  break;

	case MAGDB_TYPE_2MASS:
	  fcdb_2mass_vo_parse(hg, TRUE);
	  hits=hg->obj[i_list].magdb_2mass_hits;
	  break;

	case MAGDB_TYPE_IRCS_GSC:
	  fcdb_ircs_gsc_vo_parse(hg);
	  break;

	case MAGDB_TYPE_IRCS_PS1:
	  fcdb_ircs_ps1_vo_parse(hg);
	  break;

	case MAGDB_TYPE_IRCS_GAIA:
	  fcdb_ircs_gaia_vo_parse(hg);
	  break;

	case MAGDB_TYPE_HSC_SIMBAD:
	  fcdb_hsc_simbad_vo_parse(hg);
	  hits=hg->obj[i_list].hscmag.hits;
	  break;
	  
	case MAGDB_TYPE_HDS_GSC:
	  fcdb_hds_gsc_vo_parse(hg);
	  break;

	case MAGDB_TYPE_HDS_GAIA:
	  fcdb_hds_gaia_vo_parse(hg);
	  break;
	}
	
	elapsed_sec=difftime(time(NULL),start_time);
	remaining_sec=elapsed_sec/(double)(i_list+1)
	  *(double)(hg->i_max-(i_list+1));
	
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar2),
				      (gdouble)(i_list+1)/(gdouble)(hg->i_max));
	tmp=g_strdup_printf("Finished [ %d / %d ] Objects",
			    i_list+1, hg->i_max);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
	g_free(tmp);
	
	if(hits>0){
#ifdef USE_GTK3
	  css_change_col(hg->plabel3,"red");
#else
	  gtk_widget_modify_fg(hg->plabel3,GTK_STATE_NORMAL,&color_red);
#endif
	}
	else{
#ifdef USE_GTK3
	  css_change_col(hg->plabel3,"black");
#else
	  gtk_widget_modify_fg(hg->plabel3,GTK_STATE_NORMAL,&color_black);
#endif
	}
	switch(hg->fcdb_type){
	case MAGDB_TYPE_IRCS_GSC:
	case MAGDB_TYPE_IRCS_PS1:
	case MAGDB_TYPE_IRCS_GAIA:
	  switch(hg->obj[i_list].aomode){
	  case AOMODE_NO:
	    tmp=g_strdup_printf("%s : Failed to find a guide star",
				hg->obj[i_list].name);
	    hg->obj[i_list].adi=FALSE;
	    break;
	  case AOMODE_NGS_S:
	    tmp=g_strdup_printf("%s : Found Target = NGS",
				hg->obj[i_list].name); 
	    break;
	  case AOMODE_NGS_O:
	    tmp=g_strdup_printf("%s : Found an offset NGS",
				hg->obj[i_list].name); 
	    break;
	  case AOMODE_LGS_S:
	    tmp=g_strdup_printf("%s : Found Target = Tip-Tilt guide star (LGS)",
				hg->obj[i_list].name); 
	    break;
	  case AOMODE_LGS_O:
	    tmp=g_strdup_printf("%s : Found an offset Tip-Tilt guide star (LGS)",
				hg->obj[i_list].name); 
	    break;

	  default:
	    tmp=g_strdup_printf("%s : Cannot use SV Guide",
				hg->obj[i_list].name); 
	    break;
	  }
	  break;

	case MAGDB_TYPE_HDS_GSC:
	case MAGDB_TYPE_HDS_GAIA:
	  switch(hg->obj[i_list].guide){
	  case SV_GUIDE:
	    tmp=g_strdup_printf("%s : Use SV (SemiAuto) Guide",
				hg->obj[i_list].name);
	    hg->obj[i_list].adi=FALSE;
	    break;
	  case SVSAFE_GUIDE:
	    tmp=g_strdup_printf("%s : Use SV (Safe) Guide",
				hg->obj[i_list].name); 
	    break;
	  }
	  break;
	  
	default:
	  tmp=g_strdup_printf("%s : hit %d-objects",
			      hg->obj[i_list].name, 
			      hits);
	  break;
	}
	gtk_label_set_text(GTK_LABEL(hg->plabel3),tmp);
	g_free(tmp);

	if(remaining_sec>3600){
	  tmp=g_strdup_printf("Estimated time left : %dhrs and %dmin", 
			      (int)(remaining_sec)/3600,
			      ((int)remaining_sec%3600)/60);
	}
	else if(remaining_sec>60){
	  tmp=g_strdup_printf("Estimated time left : %dmin and %dsec", 
			      (int)(remaining_sec)/60,(int)remaining_sec%60);
	}
	else{
	  tmp=g_strdup_printf("Estimated time left : %.0lfsec", 
			      remaining_sec);
	}
	gtk_label_set_text(GTK_LABEL(hg->plabel2),tmp);
	g_free(tmp);
	
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
      case MAGDB_TYPE_HDS_GSC:
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

      case MAGDB_TYPE_UCAC:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    switch(hg->magdb_ucac_band){
	    case UCAC_BAND_B:
	      mag=hg->obj[i_list].magdb_ucac_b;
	      break;
	    case UCAC_BAND_G:
	      mag=hg->obj[i_list].magdb_ucac_g;
	      break;
	    case UCAC_BAND_V:
	      mag=hg->obj[i_list].magdb_ucac_v;
	      break;
	    case UCAC_BAND_R:
	      mag=hg->obj[i_list].magdb_ucac_r;
	      break;
	    case UCAC_BAND_I:
	      mag=hg->obj[i_list].magdb_ucac_i;
	      break;
	    case UCAC_BAND_J:
	      mag=hg->obj[i_list].magdb_ucac_j;
	      break;
	    case UCAC_BAND_H:
	      mag=hg->obj[i_list].magdb_ucac_h;
	      break;
	    case UCAC_BAND_K:
	      mag=hg->obj[i_list].magdb_ucac_k;
	      break;
	    }

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_UCAC;
	      hg->obj[i_list].magdb_band=hg->magdb_ucac_band;
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
      case MAGDB_TYPE_HDS_GAIA:
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

      case MAGDB_TYPE_KEPLER:
	if((hg->magdb_ow)||
	   (fabs(hg->obj[i_list].mag)>99)){
	  if(hits>0){
	    mag=hg->obj[i_list].magdb_kepler_k;

	    if(mag<99){
	      hg->obj[i_list].mag=mag;
	      hg->obj[i_list].magdb_used=MAGDB_TYPE_KEPLER;
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


  if(GTK_IS_WIDGET(hg->pdialog)) gtk_widget_destroy(hg->pdialog);

  make_trdb_label(hg);
  gtk_label_set_text(GTK_LABEL(hg->trdb_label), hg->trdb_label_text);
  update_objtree(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), hg->page[NOTE_OBJ]);

  flag_getFCDB=FALSE;
}

gboolean check_magdb (gpointer gdata){
  if(flag_magdb_finish){
    flag_magdb_finish=FALSE;
      gtk_main_quit();
  }
  return(TRUE);
}


void magdb_mag_copy(typHOE *hg, gint i, gint i_mag,
		    gint magdb_type, gboolean mag_flag){

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
  case MAGDB_TYPE_SIMBAD:
    if(mag_flag){
      hg->obj[i].magdb_simbad_u=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_simbad_b=hg->fcdb[i_mag].b;
      hg->obj[i].magdb_simbad_v=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_simbad_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_simbad_i=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_simbad_j=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_simbad_h=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_simbad_k=hg->fcdb[i_mag].k;

      hg->obj[i].magj=hg->fcdb[i_mag].j;
      hg->obj[i].magh=hg->fcdb[i_mag].h;
      hg->obj[i].magk=hg->fcdb[i_mag].k;
    }
    else{
      hg->obj[i].magdb_simbad_u=100;
      hg->obj[i].magdb_simbad_b=100;
      hg->obj[i].magdb_simbad_v=100;
      hg->obj[i].magdb_simbad_r=100;
      hg->obj[i].magdb_simbad_i=100;
      hg->obj[i].magdb_simbad_j=100;
      hg->obj[i].magdb_simbad_h=100;
      hg->obj[i].magdb_simbad_k=100;
    }
    break;
    
  case FCDB_TYPE_GSC:
  case MAGDB_TYPE_GSC:
    if(mag_flag){
      hg->obj[i].magdb_gsc_u=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_gsc_b=hg->fcdb[i_mag].b;
      hg->obj[i].magdb_gsc_v=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_gsc_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_gsc_i=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_gsc_j=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_gsc_h=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_gsc_k=hg->fcdb[i_mag].k;

      hg->obj[i].magj=hg->fcdb[i_mag].j;
      hg->obj[i].magh=hg->fcdb[i_mag].h;
      hg->obj[i].magk=hg->fcdb[i_mag].k;
    }
    else{
      hg->obj[i].magdb_gsc_u=100;
      hg->obj[i].magdb_gsc_b=100;
      hg->obj[i].magdb_gsc_v=100;
      hg->obj[i].magdb_gsc_r=100;
      hg->obj[i].magdb_gsc_i=100;
      hg->obj[i].magdb_gsc_j=100;
      hg->obj[i].magdb_gsc_h=100;
      hg->obj[i].magdb_gsc_k=100;
    }
    break;
    
  case FCDB_TYPE_UCAC:
  case MAGDB_TYPE_UCAC:
    if(mag_flag){
      hg->obj[i].magdb_ucac_b=hg->fcdb[i_mag].b;
      hg->obj[i].magdb_ucac_g=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_ucac_v=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_ucac_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_ucac_i=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_ucac_j=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_ucac_h=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_ucac_k=hg->fcdb[i_mag].k;

      hg->obj[i].magj=hg->fcdb[i_mag].j;
      hg->obj[i].magh=hg->fcdb[i_mag].h;
      hg->obj[i].magk=hg->fcdb[i_mag].k;
    }
    else{
      hg->obj[i].magdb_ucac_b=100;
      hg->obj[i].magdb_ucac_g=100;
      hg->obj[i].magdb_ucac_v=100;
      hg->obj[i].magdb_ucac_r=100;
      hg->obj[i].magdb_ucac_i=100;
      hg->obj[i].magdb_ucac_j=100;
      hg->obj[i].magdb_ucac_h=100;
      hg->obj[i].magdb_ucac_k=100;
    }
    break;
    
  case FCDB_TYPE_PS1:
  case MAGDB_TYPE_PS1:
    if(mag_flag){
      hg->obj[i].magdb_ps1_g=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_ps1_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_ps1_i=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_ps1_z=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_ps1_y=hg->fcdb[i_mag].h;
       }
    else{
      hg->obj[i].magdb_ps1_g=100;
      hg->obj[i].magdb_ps1_r=100;
      hg->obj[i].magdb_ps1_i=100;
      hg->obj[i].magdb_ps1_z=100;
      hg->obj[i].magdb_ps1_y=100;
    }
    break;
    
  case FCDB_TYPE_SDSS:
  case MAGDB_TYPE_SDSS:
    if(mag_flag){
      hg->obj[i].magdb_sdss_u=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_sdss_g=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_sdss_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_sdss_i=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_sdss_z=hg->fcdb[i_mag].j;
    }
    else{
      hg->obj[i].magdb_sdss_u=100;
      hg->obj[i].magdb_sdss_g=100;
      hg->obj[i].magdb_sdss_r=100;
      hg->obj[i].magdb_sdss_i=100;
      hg->obj[i].magdb_sdss_z=100;
    }
    break;
    
  case FCDB_TYPE_GAIA:
  case MAGDB_TYPE_GAIA:
    if(mag_flag){
      hg->obj[i].magdb_gaia_g=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_gaia_p=hg->fcdb[i_mag].plx;
      hg->obj[i].magdb_gaia_ep=hg->fcdb[i_mag].eplx;
      hg->obj[i].magdb_gaia_bp=hg->fcdb[i_mag].b;
      hg->obj[i].magdb_gaia_rp=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_gaia_rv=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_gaia_teff=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_gaia_ag=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_gaia_dist=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_gaia_ebr=hg->fcdb[i_mag].k;
     }
    else{
      hg->obj[i].magdb_gaia_g=100;
      hg->obj[i].magdb_gaia_p=-1;
      hg->obj[i].magdb_gaia_ep=-1;
      hg->obj[i].magdb_gaia_bp=100;
      hg->obj[i].magdb_gaia_rp=100;
      hg->obj[i].magdb_gaia_rv=-99999;
      hg->obj[i].magdb_gaia_teff=-1;
      hg->obj[i].magdb_gaia_ag=100;
      hg->obj[i].magdb_gaia_ebr=-1;
      hg->obj[i].magdb_gaia_dist=-1;
    }
    break;
    
  case FCDB_TYPE_KEPLER:
  case MAGDB_TYPE_KEPLER:
    if(mag_flag){
      hg->obj[i].magdb_kepler_k=hg->fcdb[i_mag].v;
      hg->obj[i].magdb_kepler_r=hg->fcdb[i_mag].r;
      hg->obj[i].magdb_kepler_j=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_kepler_teff=hg->fcdb[i_mag].u;
      hg->obj[i].magdb_kepler_logg=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_kepler_feh=hg->fcdb[i_mag].b;
      hg->obj[i].magdb_kepler_ebv=hg->fcdb[i_mag].k;
      hg->obj[i].magdb_kepler_rad=hg->fcdb[i_mag].i;
      hg->obj[i].magdb_kepler_pm=hg->fcdb[i_mag].plx;
      hg->obj[i].magdb_kepler_gr=hg->fcdb[i_mag].eplx;
    }
    else{
      hg->obj[i].magdb_kepler_r=100;
      hg->obj[i].magdb_kepler_j=100;
      hg->obj[i].magdb_kepler_teff=-1;
      hg->obj[i].magdb_kepler_logg=-10;
      hg->obj[i].magdb_kepler_feh=100;
      hg->obj[i].magdb_kepler_ebv=100;
      hg->obj[i].magdb_kepler_rad=-100;
      hg->obj[i].magdb_kepler_pm=-10000;
      hg->obj[i].magdb_kepler_gr=100;
    }
    break;
    
  case FCDB_TYPE_2MASS:
  case MAGDB_TYPE_2MASS:
    if(mag_flag){
      hg->obj[i].magdb_2mass_j=hg->fcdb[i_mag].j;
      hg->obj[i].magdb_2mass_h=hg->fcdb[i_mag].h;
      hg->obj[i].magdb_2mass_k=hg->fcdb[i_mag].k;

      hg->obj[i].magj=hg->fcdb[i_mag].j;
      hg->obj[i].magh=hg->fcdb[i_mag].h;
      hg->obj[i].magk=hg->fcdb[i_mag].k;
    }
    else{
      hg->obj[i].magdb_2mass_j=100;
      hg->obj[i].magdb_2mass_h=100;
      hg->obj[i].magdb_2mass_k=100;
    }
    break;
    
  }
}
