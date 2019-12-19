// hsc.c for Subaru HSC
//             Feb 2019  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"

gboolean flagHSCEditDialog=FALSE;

// GUI creation in main window
void HSC_TAB_create(typHOE *hg){
  GtkWidget *w_top;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *sw;
  GtkWidget *frame, *frame1;
  GtkWidget *table1;
  GtkWidget *hbox, *hbox1, *hbox2;
  GtkWidget *vbox;
  GtkWidget *entry;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  GtkWidget *button;
  gchar *tmp;
  GtkTooltip *tooltip;
  
  GtkWidget *combo;
  GtkListStore *store;
  GtkTreeIter iter, iter_set;	  
  GtkCellRenderer *renderer;
  gint i_band, i_dith;
  GtkTreeModel *items_model;
  
  hg->hsc_vbox = gtkut_vbox_new(FALSE,0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(hg->hsc_vbox), hbox,FALSE, FALSE, 2);

  table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), table,FALSE,FALSE,0);
    
  
  frame = gtkut_frame_new ("<b>Edit the List</b>");
  gtkut_table_attach (table, frame, 0, 1, 0, 1,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"list-add");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
  		    HSC_add_setup, 
  		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Add");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"list-remove");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    HSC_remove_setup, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Remove");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-up");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
  		    up_item_hsc_tree, 
  		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Up");
#endif
    
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    down_item_hsc_tree, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Down");
#endif


  frame = gtkut_frame_new ("<b>HSC Setup</b>");
  gtkut_table_attach (table, frame, 1, 2, 0, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(frame), 5);

  table1=gtkut_table_new(6, 2, FALSE, 5, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  label = gtk_label_new ("Filter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table1, label, 0, 1, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Dither");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table1, label, 0, 1, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
  
  for(i_band=0; i_band < NUM_HSC_FIL; i_band++){
    gtk_list_store_append(store, &iter);
    if(hsc_filter[i_band].name){
      gtk_list_store_set(store, &iter, 0, hsc_filter[i_band].name,
			 1, i_band, 2, TRUE, -1);
      if(hg->hsc_filter==i_band) iter_set=iter;
    }
    else{
      gtk_list_store_set(store, &iter, 0, NULL,
			 1, i_band, 2, FALSE, -1);
    }
  }	
      
  combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
  gtkut_table_attach (table1, combo, 1, 2, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  g_object_unref(store);
      
  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			     renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				  renderer, "text",0,NULL);
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					is_separator, NULL, NULL);	

  gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
  gtk_widget_show(combo);
  my_signal_connect (combo,"changed",cc_get_combo_box,
  		     &hg->hsc_filter);
  

  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtkut_table_attach (table1, hbox1, 2, 6, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("     Default ExpTime [sec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_exp,
					    2, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->hsc_exp);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  
  label = gtk_label_new ("        OffSet RA [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_osra,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_osra);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);
  
  label = gtk_label_new ("   Dec [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_osdec,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_osdec);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);


  store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
  
  for(i_dith=0; i_dith < NUM_HSC_DITH; i_dith++){
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, hsc_dith_name[i_dith],
		       1, i_dith, 2, TRUE, -1);
    if(hg->hsc_dith==i_dith) iter_set=iter;
  }	
      
  combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
  gtkut_table_attach (table1, combo, 1, 2, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);
  g_object_unref(store);
      
  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			     renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				  renderer, "text",0,NULL);

  gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
  gtk_widget_show(combo);
  my_signal_connect (combo,"changed",HSC_get_dith,
  		     (gpointer)hg);
  
  check = gtk_check_button_new_with_label("Auto Guide");
  gtkut_table_attach(table1, check, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->hsc_ag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->hsc_ag);

  hg->hsc_frame_5dith = gtkut_frame_new ("<b>5-shot</b>");
  gtkut_table_attach (table1, hg->hsc_frame_5dith, 3, 4, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(hg->hsc_frame_5dith), 5);

  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtk_container_add (GTK_CONTAINER (hg->hsc_frame_5dith), hbox1);

  label = gtk_label_new ("DITH_RA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dith_ra,
					    60, 600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dith_ra);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  DITH_DEC");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dith_dec,
					    60, 600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dith_dec);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);


  hg->hsc_frame_ndith = gtkut_frame_new ("<b>N-shot</b>");
  gtkut_table_attach (table1, hg->hsc_frame_ndith, 4, 5, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(hg->hsc_frame_ndith), 5);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtk_container_add (GTK_CONTAINER (hg->hsc_frame_ndith), hbox1);

  label = gtk_label_new ("NDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dith_n,
					    3, 99, 
					    1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dith_n);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  RDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dith_r,
					    120, 990, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dith_r);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  TDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dith_t,
					    0, 360, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dith_t);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);
  

  // TreeView
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hg->hsc_vbox), sw, TRUE, TRUE, 0);
  
  /* create models */
  items_model = hsc_create_items_model (hg);
  
  /* create tree view */
  hg->hsc_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->hsc_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->hsc_tree)),
			       GTK_SELECTION_SINGLE);
  hsc_add_columns (hg, GTK_TREE_VIEW (hg->hsc_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (sw), hg->hsc_tree);
  
  my_signal_connect (hg->hsc_tree, "cursor-changed",
		     G_CALLBACK (hsc_focus_item), (gpointer)hg);
  
  my_signal_connect (hg->hsc_tree, "row-activated",
		     G_CALLBACK (hsc_activate_item), (gpointer)hg);


  hsc_dith_frame_set_sensitive(hg);
  
  label = gtk_label_new ("HSC");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), hg->hsc_vbox, label);
}


void HSCFIL_TAB_create(typHOE *hg){
  GtkWidget *w_top;
  GtkWidget *label;
  GtkWidget *sw;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *button;
  gchar *tmp;
  GtkTooltip *tooltip;
  
  GtkTreeModel *items_model;
  
  vbox = gtkut_vbox_new(FALSE,0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 2);

  tmp = g_strdup_printf("List of HSC Filters : Ver. %s", hg->hsc_filter_ver);
  hg->hsc_label_filter_ver = gtk_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->hsc_label_filter_ver, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->hsc_label_filter_ver, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->hsc_label_filter_ver), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hg->hsc_label_filter_ver,FALSE,FALSE,0);
    
  label= gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (hsc_sync_filter), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Sync HSC Filter info to the latest one via network");
#endif

  // TreeView
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  
  // create models
  items_model = hscfil_create_items_model (hg);
  
  // create tree view
  hg->hscfil_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->hscfil_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->hscfil_tree)),
			       GTK_SELECTION_SINGLE);
  hscfil_add_columns (hg, GTK_TREE_VIEW (hg->hscfil_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (sw), hg->hscfil_tree);

  label = gtk_label_new ("Filter");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
}


void do_edit_hsc_setup(typHOE *hg, gint i_set){
  GtkWidget *dialog, *frame, *label, *check;
  GtkWidget *hbox, *combo, *table1, *spinner, *hbox1;
  GtkAdjustment *adj;
  gchar *tmp;
  HSCpara tmp_set;
  gint dith0;
  gint i_band, i_dith;
  GtkListStore *store;
  GtkTreeIter iter, iter_set;	  
  GtkCellRenderer *renderer;

  if(flagHSCEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagHSCEditDialog=TRUE;

  tmp_set=hg->hsc_set[i_set];

  {
    dith0=hg->hsc_dith;

    hg->hsc_dith=tmp_set.dith;
  }

  dialog = gtk_dialog_new_with_buttons("HOE : HSC Setup Edit",
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
  
  
  tmp=g_strdup_printf("<b>HSC Setup-%d</b>", i_set+1);
  frame = gtkut_frame_new (tmp);
  g_free(tmp);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(frame), 5);

  table1=gtkut_table_new(6, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  label = gtk_label_new ("Filter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table1, label, 0, 1, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Dither");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table1, label, 0, 1, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
  
  for(i_band=0; i_band < NUM_HSC_FIL; i_band++){
    gtk_list_store_append(store, &iter);
    if(hsc_filter[i_band].name){
      gtk_list_store_set(store, &iter, 0, hsc_filter[i_band].name,
			 1, i_band, 2, TRUE, -1);
      if(tmp_set.filter==i_band) iter_set=iter;
    }
    else{
      gtk_list_store_set(store, &iter, 0, NULL,
			 1, i_band, 2, FALSE, -1);
    }
  }	
      
  combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
  gtkut_table_attach (table1, combo, 1, 2, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  g_object_unref(store);
      
  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			     renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				  renderer, "text",0,NULL);
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					is_separator, NULL, NULL);	

  gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
  gtk_widget_show(combo);
  my_signal_connect (combo,"changed",cc_get_combo_box,
  		     &tmp_set.filter);
  

  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtkut_table_attach (table1, hbox1, 2, 6, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("     Default ExpTime [sec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.exp,
					    2, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_set.exp);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  
  label = gtk_label_new ("        OffSet RA [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.osra,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.osra);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);
  
  label = gtk_label_new ("   Dec [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.osdec,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.osdec);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);


  store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
  
  for(i_dith=0; i_dith < NUM_HSC_DITH; i_dith++){
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, hsc_dith_name[i_dith],
		       1, i_dith, 2, TRUE, -1);
    if(hg->hsc_dith==i_dith) iter_set=iter;
  }	
      
  combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
  gtkut_table_attach (table1, combo, 1, 2, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);
  g_object_unref(store);
      
  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			     renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				  renderer, "text",0,NULL);

  gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
  gtk_widget_show(combo);
  my_signal_connect (combo,"changed",HSC_get_dith,
  		     (gpointer)hg);
  
  check = gtk_check_button_new_with_label("Auto Guide");
  gtkut_table_attach(table1, check, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_set.ag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       tmp_set.ag);

  hg->hsc_e_frame_5dith = gtkut_frame_new ("<b>5-shot</b>");
  gtkut_table_attach (table1, hg->hsc_e_frame_5dith, 3, 4, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(hg->hsc_e_frame_5dith), 5);

  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtk_container_add (GTK_CONTAINER (hg->hsc_e_frame_5dith), hbox1);

  label = gtk_label_new ("DITH_RA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.dith_ra,
					    60, 600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.dith_ra);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  DITH_DEC");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.dith_dec,
					    60, 600, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.dith_dec);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);


  hg->hsc_e_frame_ndith = gtkut_frame_new ("<b>N-shot</b>");
  gtkut_table_attach (table1, hg->hsc_e_frame_ndith, 4, 5, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER(hg->hsc_e_frame_ndith), 5);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);
  gtk_container_add (GTK_CONTAINER (hg->hsc_e_frame_ndith), hbox1);

  label = gtk_label_new ("NDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.dith_n,
					    3, 99, 
					    1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.dith_n);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  RDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.dith_r,
					    120, 990, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.dith_r);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("  TDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_set.dith_t,
					    0, 360, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_set.dith_t);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

  hsc_dith_frame_set_sensitive(hg);
  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    if(hg->hsc_set[i_set].txt) g_free(hg->hsc_set[i_set].txt);
    if(hg->hsc_set[i_set].def) g_free(hg->hsc_set[i_set].def);
    if(hg->hsc_set[i_set].dtxt) g_free(hg->hsc_set[i_set].dtxt);
    
    hg->hsc_set[i_set]=tmp_set;
    hg->hsc_set[i_set].dith=hg->hsc_dith;
      
    hg->hsc_set[i_set].txt=hsc_make_txt(hg, i_set);
    hg->hsc_set[i_set].def=hsc_make_def(hg, i_set);
    hg->hsc_set[i_set].dtxt=hsc_make_dtxt(hg, i_set);
    
    hsc_make_tree(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
  flagHSCEditDialog=FALSE;
  
  hg->hsc_dith=dith0;
}

void HSC_param_init(typHOE *hg){
  gint i_set;
  gint i_fil;

  hg->hsc_focus_z=HSC_DEF_FOCUS_Z;
  hg->hsc_delta_z=HSC_DEF_DELTA_Z;

  hg->hsc_filter=HSC_FIL_G;

  hg->hsc_dith=HSC_DITH_NO;
  hg->hsc_dith_ra=HSC_DEF_DITH_RA;
  hg->hsc_dith_dec=HSC_DEF_DITH_DEC;
  hg->hsc_dith_n=HSC_DEF_NDITH;
  hg->hsc_dith_r=HSC_DEF_RDITH;
  hg->hsc_dith_t=HSC_DEF_TDITH;
  hg->hsc_ag=FALSE;
  hg->hsc_osra=HSC_DEF_OSRA;
  hg->hsc_osdec=HSC_DEF_OSDEC;
  hg->hsc_exp=30;

  hg->hsc_i=0;
  hg->hsc_i_max=0;

  for(i_set=0;i_set<HSC_MAX_SET;i_set++){
    hg->hsc_set[i_set].txt=NULL;
    hg->hsc_set[i_set].def=NULL;
    hg->hsc_set[i_set].dtxt=NULL;
  }

  hg->hsc_magdb_arcmin=HSC_SIZE;

  hg->hsc_filter_updated=FALSE;
  hg->hsc_filter_ver=NULL;
  HSC_Init_Filter(hg);
}


GtkTreeModel * hsc_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_HSC, 
			      G_TYPE_INT,     // number
			      G_TYPE_INT,     // filter
			      G_TYPE_STRING,  // def
			      G_TYPE_STRING,  // dith
			      G_TYPE_INT,     // offset RA
			      G_TYPE_INT,     // offset Dec
			      G_TYPE_DOUBLE,  // exp
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,
			      GDK_TYPE_RGBA    //fg, bg color
#else
			      GDK_TYPE_COLOR,
			      GDK_TYPE_COLOR   //fg, bg color
#endif
			      );
  
  for (i = 0; i < hg->hsc_i_max; i++){
    gtk_list_store_append (model, &iter);
    hsc_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}

GtkTreeModel * hscfil_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_HSCFIL, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
			      G_TYPE_INT,     // ID
			      G_TYPE_DOUBLE,  // good mag
			      G_TYPE_DOUBLE,  // ag exp
			      G_TYPE_BOOLEAN, // ag flag
			      G_TYPE_INT,     // flat w
			      G_TYPE_DOUBLE,  // flat v
			      G_TYPE_DOUBLE,  // flat a
			      G_TYPE_INT,     // flat exp
			      G_TYPE_BOOLEAN, // flat flag
			      G_TYPE_DOUBLE,  // sens
			      G_TYPE_DOUBLE   // mag1e
			      );
  
  for (i = 0; i < NUM_HSC_FIL; i++){
    gtk_list_store_append (model, &iter);
    hscfil_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void hsc_tree_update_item(typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_list)
{
  // Num
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_NUMBER,
		      i_list+1,
		      -1);

  // Filter
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_FILTER,
		      hg->hsc_set[i_list].filter,
		      -1);

  // Def
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_DEF,
		      hg->hsc_set[i_list].def,
		      -1);

  // Dith
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_DITH,
		      hg->hsc_set[i_list].dtxt,
		      -1);

  // Offset
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_OSRA, hg->hsc_set[i_list].osra,
		      COLUMN_HSC_OSDEC,hg->hsc_set[i_list].osdec,
		      -1);

  // Exp
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_EXP,
		      hg->hsc_set[i_list].exp,
		      -1);

  /* BG color */
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSC_COLFG,
		      &color_black,
		      COLUMN_HSC_COLBG,
		      &col_hsc_setup[i_list],
		      -1);
}


void hscfil_tree_update_item(typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_list)
{
  // Num
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_NUMBER,
		      i_list,
		      -1);

  // Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_NAME,
		      hsc_filter[i_list].name,
		      -1);

  // Def
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_ID,
		      hsc_filter[i_list].id,
		      -1);

  // AG params
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_AGMAG, hsc_filter[i_list].good_mag,
		      COLUMN_HSCFIL_AGEXP, hsc_filter[i_list].ag_exp,
		      COLUMN_HSCFIL_AGFLG, hsc_filter[i_list].ag_flg,
		      -1);

  // FLAT params
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_FLATW, hsc_filter[i_list].flat_w,
		      COLUMN_HSCFIL_FLATV, hsc_filter[i_list].flat_v,
		      COLUMN_HSCFIL_FLATA, hsc_filter[i_list].flat_a,
		      COLUMN_HSCFIL_FLATEXP, hsc_filter[i_list].flat_exp,
		      COLUMN_HSCFIL_FLATFLG, hsc_filter[i_list].flat_flg,
		      -1);

  // snesitivity
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_HSCFIL_SENS, hsc_filter[i_list].sens,
		      COLUMN_HSCFIL_MAG1E, hsc_filter[i_list].mag1e,
		      -1);
}


void hsc_add_columns (typHOE *hg,
		      GtkTreeView  *treeview, 
		      GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Setup Number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_HSC_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
						   renderer,
						   "text",
						   COLUMN_HSC_NUMBER,
						   NULL); 
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Setup Filter column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_FILTER));
  column=gtk_tree_view_column_new_with_attributes ("Filter",
						   renderer,
						   "text", 
						   COLUMN_HSC_FILTER,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_FILTER),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_FILTER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Def column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_DEF));
  column=gtk_tree_view_column_new_with_attributes ("DEF",
						   renderer,
						   "text", 
						   COLUMN_HSC_DEF,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_DEF),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_DEF);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Dith column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_DITH));
  column=gtk_tree_view_column_new_with_attributes ("Dithering Parameters",
						   renderer,
						   "text", 
						   COLUMN_HSC_DITH,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_DITH),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_DITH);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Offset RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_OSRA));
  column=gtk_tree_view_column_new_with_attributes ("Offset RA",
						   renderer,
						   "text", 
						   COLUMN_HSC_OSRA,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_OSRA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_OSRA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Offset Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_OSDEC));
  column=gtk_tree_view_column_new_with_attributes ("Offset Dec",
						   renderer,
						   "text", 
						   COLUMN_HSC_OSDEC,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_OSDEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_OSDEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
  /* Exp column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSC_EXP));
  column=gtk_tree_view_column_new_with_attributes ("Def. ExpTime [s]",
						   renderer,
						   "text", 
						   COLUMN_HSC_EXP,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_HSC_COLFG,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_HSC_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hsc_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSC_EXP),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_HSC_EXP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


void hscfil_add_columns (typHOE *hg,
			 GtkTreeView  *treeview, 
			 GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  // Name
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_NAME,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_NAME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // ID
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_ID));
  column=gtk_tree_view_column_new_with_attributes ("ID#",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_ID,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_ID),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Good Mag
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_AGMAG));
  column=gtk_tree_view_column_new_with_attributes ("Good Mag",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_AGMAG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_AGMAG),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // AG Exp
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_AGEXP));
  column=gtk_tree_view_column_new_with_attributes ("AG Exp",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_AGEXP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_AGEXP),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // AG flg
  renderer = gtk_cell_renderer_toggle_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_AGFLG));
  column=gtk_tree_view_column_new_with_attributes ("confirmed?",
						   renderer,
						   "active", 
						   COLUMN_HSCFIL_AGFLG,
						   NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flat W
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_FLATW));
  column=gtk_tree_view_column_new_with_attributes ("Flat Lamp",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_FLATW,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_FLATW),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flat V
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_FLATV));
  column=gtk_tree_view_column_new_with_attributes ("Vol",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_FLATV,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_FLATV),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flat A
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_FLATA));
  column=gtk_tree_view_column_new_with_attributes ("Amp",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_FLATA,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_FLATA),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flat Exp
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_FLATEXP));
  column=gtk_tree_view_column_new_with_attributes ("Exp",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_FLATEXP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_FLATEXP),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flat flg
  renderer = gtk_cell_renderer_toggle_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_FLATFLG));
  column=gtk_tree_view_column_new_with_attributes ("confirmed?",
						   renderer,
						   "active", 
						   COLUMN_HSCFIL_FLATFLG,
						   NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Sensitivity
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_SENS));
  column=gtk_tree_view_column_new_with_attributes ("Sensitivity",
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_SENS,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_SENS),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Mag 1e-
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_HSCFIL_MAG1E));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_HSCFIL_MAG1E,
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Mag for 1e<sup>-</sup>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  hscfil_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_HSCFIL_MAG1E),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

}


void hsc_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->hsc_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->hsc_tree));

  if(gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_HSC_NUMBER, &i, -1);
    i--;
    hg->hsc_i=i;
      
    gtk_tree_path_free (path);
  }
}


void hsc_activate_item (GtkTreeView        *treeview,
			GtkTreePath        *path,
			GtkTreeViewColumn  *col,
			gpointer            data){
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gint i_set;
  
  model = gtk_tree_view_get_model(treeview);
  
  if (gtk_tree_model_get_iter(model, &iter, path)){
    
    i_set = gtk_tree_path_get_indices (path)[0];
    do_edit_hsc_setup(hg, i_set);
  }
}


void hsc_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hsc_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->hsc_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    hsc_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
}


void hscfil_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hscfil_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < NUM_HSC_FIL; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    hscfil_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
}


void hsc_update_tree(typHOE *hg){
  int i_set;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hsc_tree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_set=0;i_set<hg->hsc_i_max;i_set++){
    gtk_tree_model_get (model, &iter, COLUMN_HSC_NUMBER, &i, -1);
    i--;
    hsc_tree_update_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}


void hsc_cell_data_func(GtkTreeViewColumn *col , 
			GtkCellRenderer *renderer,
			GtkTreeModel *model, 
			GtkTreeIter *iter,
			gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  gchar *str_value;
  gint  int_value;
  gdouble  double_value;
  gchar *str=NULL;

  switch (index) {
  case COLUMN_HSC_DEF:
  case COLUMN_HSC_DITH:
    gtk_tree_model_get (model, iter, 
			index, &str_value,
			-1);
    break;

  case COLUMN_HSC_EXP:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;

  case COLUMN_HSC_FILTER:
  case COLUMN_HSC_OSRA:
  case COLUMN_HSC_OSDEC:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_HSC_DEF:
  case COLUMN_HSC_DITH:
    if(!str_value){
      str=g_strdup_printf("---");
    }
    else{
      str=g_strdup(str_value);
    }
    break;

  case COLUMN_HSC_EXP:
    str=g_strdup_printf("%.0lf", double_value);
    break;

  case COLUMN_HSC_OSRA:
  case COLUMN_HSC_OSDEC:
    str=g_strdup_printf("%+d", int_value);
    break;
    
  case COLUMN_HSC_FILTER:
    str=g_strdup(hsc_filter[int_value].name);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void hscfil_cell_data_func(GtkTreeViewColumn *col , 
			   GtkCellRenderer *renderer,
			   GtkTreeModel *model, 
			   GtkTreeIter *iter,
			   gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  gchar *str_value;
  gint  int_value;
  gdouble  double_value;
  gchar *str=NULL;

  switch (index) {
  case COLUMN_HSCFIL_NAME:
    gtk_tree_model_get (model, iter, 
			index, &str_value,
			-1);
    break;

  case COLUMN_HSCFIL_AGMAG:
  case COLUMN_HSCFIL_AGEXP:
  case COLUMN_HSCFIL_FLATV:
  case COLUMN_HSCFIL_FLATA:
  case COLUMN_HSCFIL_SENS:
  case COLUMN_HSCFIL_MAG1E:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;

  case COLUMN_HSCFIL_ID:
  case COLUMN_HSCFIL_FLATEXP:
  case COLUMN_HSCFIL_FLATW:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_HSCFIL_NAME:
    if(!str_value){
      str=g_strdup_printf("-----");
    }
    else{
      str=g_strdup(str_value);
    }
    break;

  case COLUMN_HSCFIL_AGMAG:
  case COLUMN_HSCFIL_AGEXP:
  case COLUMN_HSCFIL_FLATV:
    if(double_value>0){
      str=g_strdup_printf("%.1lf", double_value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_HSCFIL_SENS:
  case COLUMN_HSCFIL_MAG1E:
    if((double_value>0) && (double_value<99)){
      str=g_strdup_printf("%.1lf", double_value);
    }
    else{
      str=NULL;
    }
    break;
    
  case COLUMN_HSCFIL_FLATA:
    if(double_value>0){
      str=g_strdup_printf("%.2lf", double_value);
    }
    else{
      str=NULL;
    }
    break;
    
  case COLUMN_HSCFIL_ID:
  case COLUMN_HSCFIL_FLATEXP:
    if(int_value>0){
      str=g_strdup_printf("%d", int_value);
    }
    else{
      str=NULL;
    }
    break;
    
  case COLUMN_HSCFIL_FLATW:
    if(int_value>0){
      str=g_strdup_printf("4x%dW", int_value);
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void hsc_swap_setup(HSCpara *o1, HSCpara *o2){
  HSCpara temp;
  
  temp=*o2;
  *o2=*o1;
  *o1=temp;
}

void hsc_dith_frame_set_sensitive(typHOE *hg){
  switch(hg->hsc_dith){
  case HSC_DITH_NO:
    if(flagHSCEditDialog){
      gtk_widget_set_sensitive(hg->hsc_e_frame_5dith,FALSE);
      gtk_widget_set_sensitive(hg->hsc_e_frame_ndith,FALSE);
    }
    else{
      gtk_widget_set_sensitive(hg->hsc_frame_5dith,FALSE);
      gtk_widget_set_sensitive(hg->hsc_frame_ndith,FALSE);
    }
    break;
    
  case HSC_DITH_5:
    if(flagHSCEditDialog){
      gtk_widget_set_sensitive(hg->hsc_e_frame_5dith,TRUE);
      gtk_widget_set_sensitive(hg->hsc_e_frame_ndith,FALSE);
    }
    else{
      gtk_widget_set_sensitive(hg->hsc_frame_5dith,TRUE);
      gtk_widget_set_sensitive(hg->hsc_frame_ndith,FALSE);
    }
    break;

  case HSC_DITH_N:
    if(flagHSCEditDialog){
      gtk_widget_set_sensitive(hg->hsc_e_frame_5dith,FALSE);
      gtk_widget_set_sensitive(hg->hsc_e_frame_ndith,TRUE);
    }
    else{
      gtk_widget_set_sensitive(hg->hsc_frame_5dith,FALSE);
      gtk_widget_set_sensitive(hg->hsc_frame_ndith,TRUE);
    }
    break;
  }
}


void HSC_get_dith(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->hsc_dith=n;

    hsc_dith_frame_set_sensitive(hg);
  }
}


void HSC_add_setup (GtkWidget *widget, gpointer *gdata)
{
  typHOE *hg = (typHOE *)gdata;
  gint i_set;
  gboolean ag_flag;

  if(hg->hsc_i_max>=HSC_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of HSC setups reaches to the maximum.",
		  NULL);
    return;
  }

  if((hg->hsc_ag) && (hg->hsc_exp<=60)){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "You cannot use AG w/ExpTime &lt; 60 sec.",
		  NULL);
    return;
  }

  ag_flag=hg->hsc_ag;
  if((!ag_flag) && (hg->hsc_exp>=240)){
    if(popup_dialog(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    "For ExpTime > 240 sec, we recommend you to use AG.",
		    "Do you want to use AG?.",
		    NULL)){
      ag_flag=TRUE;
    }
  }

  i_set=hg->hsc_i_max;

  hg->hsc_set[i_set].filter=hg->hsc_filter;
  hg->hsc_set[i_set].exp   =hg->hsc_exp;
  hg->hsc_set[i_set].osra  =hg->hsc_osra;
  hg->hsc_set[i_set].osdec =hg->hsc_osdec;

  hg->hsc_set[i_set].dith     =hg->hsc_dith;
  hg->hsc_set[i_set].dith_ra  =hg->hsc_dith_ra;
  hg->hsc_set[i_set].dith_dec =hg->hsc_dith_dec;
  hg->hsc_set[i_set].dith_n   =hg->hsc_dith_n;
  hg->hsc_set[i_set].dith_r   =hg->hsc_dith_r; 
  hg->hsc_set[i_set].dith_t   =hg->hsc_dith_t;
  hg->hsc_set[i_set].ag       =ag_flag;
   
  if(hg->hsc_set[i_set].txt) g_free(hg->hsc_set[i_set].txt);
  hg->hsc_set[i_set].txt=hsc_make_txt(hg, i_set);

  if(hg->hsc_set[i_set].def) g_free(hg->hsc_set[i_set].def);
  hg->hsc_set[i_set].def=hsc_make_def(hg, i_set);

  if(hg->hsc_set[i_set].dtxt) g_free(hg->hsc_set[i_set].dtxt);
  hg->hsc_set[i_set].dtxt=hsc_make_dtxt(hg, i_set);

  
  hg->hsc_i_max++;

  hsc_make_tree(hg);
}


void HSC_remove_setup (GtkWidget *widget, gpointer *gdata)
{
  typHOE *hg = (typHOE *)gdata;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hsc_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->hsc_tree));
  gint i_plan;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_set,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_HSC_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);
    
    
    for(i_set=i;i_set<hg->hsc_i_max;i_set++){
      hsc_swap_setup(&hg->hsc_set[i_set], &hg->hsc_set[i_set+1]);
    }

    hg->hsc_i_max--;
    
    if (gtk_tree_model_iter_nth_child(model, &iter, NULL, hg->hsc_i_max)){
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
    hsc_update_tree(hg);

    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	if(hg->plan[i_plan].setup==i){
	  init_planpara(hg, i_plan);
	  
	  hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
	  hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
	  hg->plan[i_plan].txt=g_strdup("### (The setup was removed from the list.) ###");
	  hg->plan[i_plan].comment=g_strdup(" (The setup was removed from the list.) ");
	  hg->plan[i_plan].time=0;
	}
	else if (hg->plan[i_plan].setup>i){
	  hg->plan[i_plan].setup--;
	}
      }
    }
    
  }
}


void up_item_hsc_tree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hsc_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->hsc_tree));


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_HSC_NUMBER, &i, -1);
    i--;

    if(i>0){
      hsc_swap_setup(&hg->hsc_set[i-1], &hg->hsc_set[i]);
      
      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
      hsc_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i-1);
      hsc_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
      
      for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
	if(hg->plan[i_plan].setup==i){
	  hg->plan[i_plan].setup=i-1;
	}
	else if(hg->plan[i_plan].setup==i-1){
	  hg->plan[i_plan].setup=i;
	}
      }
    }
    gtk_tree_path_free (path);
    hsc_update_tree(hg);
  }
}


void down_item_hsc_tree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->hsc_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->hsc_tree));


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_HSC_NUMBER, &i, -1);
    i--;

    if(i<hg->hsc_i_max-1){
      hsc_swap_setup(&hg->hsc_set[i], &hg->hsc_set[i+1]);
      
      gtk_tree_path_next (path);
      gtk_tree_selection_select_path(selection, path);
      
      for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
	if(hg->plan[i_plan].setup==i){
	  hg->plan[i_plan].setup=i+1;
	}
	else if(hg->plan[i_plan].setup==i+1){
	  hg->plan[i_plan].setup=i;
	}
      }
    }
    gtk_tree_path_free (path);
    hsc_update_tree(hg);
  }
}


gchar* hsc_make_txt(typHOE *hg, gint i_set){
  gchar *ret=NULL;

  switch(hg->hsc_set[i_set].dith){
  case HSC_DITH_NO:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup_printf("%s [1-shot w/AG]",
			  hsc_filter[hg->hsc_set[i_set].filter].name);
    }
    else{
      ret=g_strdup_printf("%s [1-shot w/o AG]",
			  hsc_filter[hg->hsc_set[i_set].filter].name);
    }
    break;

  case HSC_DITH_5:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup_printf("%s [5-shot w/AG, DITH_RA=%d DITH_DEC=%d]",
			  hsc_filter[hg->hsc_set[i_set].filter].name,
			  hg->hsc_set[i_set].dith_ra,
			  hg->hsc_set[i_set].dith_dec);
    }
    else{
      ret=g_strdup_printf("%s [5-shot w/o AG, DITH_RA=%d DITH_DEC=%d]",
			  hsc_filter[hg->hsc_set[i_set].filter].name,
			  hg->hsc_set[i_set].dith_ra,
			  hg->hsc_set[i_set].dith_dec);
    }
    break;
    
  case HSC_DITH_N:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup_printf("%s [N(=%d)-shot w/AG, RDITH=%d TDITH=%d]",
			  hsc_filter[hg->hsc_set[i_set].filter].name,
			  hg->hsc_set[i_set].dith_n,
			  hg->hsc_set[i_set].dith_r,
			  hg->hsc_set[i_set].dith_t);
    }
    else{
      ret=g_strdup_printf("%s [N(=%d)-shot w/o AG, RDITH=%d TDITH=%d]",
			  hsc_filter[hg->hsc_set[i_set].filter].name,
			  hg->hsc_set[i_set].dith_n,
			  hg->hsc_set[i_set].dith_r,
			  hg->hsc_set[i_set].dith_t);
    }
    break;
  }

  return(ret);
}


gchar* hsc_make_def(typHOE *hg, gint i_set){
  gchar *ret=NULL;

  switch(hg->hsc_set[i_set].dith){
  case HSC_DITH_NO:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup("IMAGE_VGW");
    }
    else{
      ret=g_strdup("IMAGE");
    }
    break;

  case HSC_DITH_5:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup("IMAGE5_VGW");
    }
    else{
      ret=g_strdup("IMAGE5");
    }
    break;
    
  case HSC_DITH_N:
    if(hg->hsc_set[i_set].ag){
      ret=g_strdup("IMAGEN_VGW");
    }
    else{
      ret=g_strdup("IMAGEN");
    }
    break;
  }

  return(ret);
}


gchar* hsc_make_dtxt(typHOE *hg, gint i_set){
  gchar *ret=NULL;

  switch(hg->hsc_set[i_set].dith){
  case HSC_DITH_5:
    ret=g_strdup_printf("DITH_RA=%d DITH_DEC=%d",
			hg->hsc_set[i_set].dith_ra,
			hg->hsc_set[i_set].dith_dec);
    break;

  case HSC_DITH_N:
    ret=g_strdup_printf("NDITH=%d RDITH=%d TDITH=%d",
			hg->hsc_set[i_set].dith_n,
			hg->hsc_set[i_set].dith_r,
			hg->hsc_set[i_set].dith_t);
    break;
  }

  return(ret);
}


void HSC_WriteOPE(typHOE *hg, gboolean plan_flag){
  FILE *fp;
  int i, i_list=0, i_set, i_use, i_repeat, i_plan, i_fil;
  gint to_year, to_month, to_day;
  gdouble new_ra, new_dec, new_d_ra, new_d_dec, yrs;
  gchar *tgt, *str;
  gchar *gsmode=NULL;
  gchar *gs_txt;
  gboolean fil_flag[NUM_HSC_FIL];

  if((fp=fopen(hg->filename_write,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_write);
    exit(1);
  }

  to_year=hg->fr_year;
  to_month=hg->fr_month;
  to_day=hg->fr_day;
  add_day(hg, &to_year, &to_month, &to_day, 1);

  fprintf(fp, "<HEADER>\n");
  if(hg->observer) fprintf(fp, "######  Observer : %s ######\n",hg->observer);
  fprintf(fp, "OBSERVATION_PERIOD=%4d-%02d-%02d-17:00:00 - %4d-%02d-%02d-08:00:00\n",
	  hg->fr_year,hg->fr_month,hg->fr_day,
	  to_year,to_month,to_day);
  fprintf(fp, "PROPOSALID=%s\n",hg->prop_id);
  fprintf(fp, "Observation_File_Name=%s\n",g_path_get_basename(hg->filename_write));
  fprintf(fp, "Observation_File_Type=OPE\n");
  fprintf(fp, "</HEADER>\n");


  fprintf(fp, "\n");


  fprintf(fp, "<PARAMETER_LIST>\n");
  fprintf(fp, "DEF_CMNTOOL=OBE_ID=COMMON OBE_MODE=TOOL\n");
  fprintf(fp, "DEF_TOOLS=OBE_ID=HSC OBE_MODE=TOOLS\n");
  fprintf(fp, "DEF_IMAGE=OBE_ID=HSC OBE_MODE=IMAG\n");
  fprintf(fp, "DEF_IMAGE_VGW=OBE_ID=HSC OBE_MODE=IMAG_VGW\n");
  fprintf(fp, "DEF_IMAGE5=OBE_ID=HSC OBE_MODE=IMAG_5\n");
  fprintf(fp, "DEF_IMAGE5_VGW=OBE_ID=HSC OBE_MODE=IMAG_5_VGW\n");
  fprintf(fp, "DEF_IMAGEN=OBE_ID=HSC OBE_MODE=IMAG_N\n");
  fprintf(fp, "DEF_IMAGEN_VGW=OBE_ID=HSC OBE_MODE=IMAG_N_VGW\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "###### LIST of OBJECTS ######\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].std){
      tgt=make_tgt(hg->obj[i_list].name, "STD_");
    }
    else{
      tgt=make_tgt(hg->obj[i_list].name, "TGT_");
    }
    if(hg->obj[i_list].i_nst<0){
      if((fabs(hg->obj[i_list].pm_ra)>100)
	 ||(fabs(hg->obj[i_list].pm_dec)>100)){
	yrs=current_yrs(hg);
	new_d_ra=ra_to_deg(hg->obj[i_list].ra)+
	  hg->obj[i_list].pm_ra/1000/60/60*yrs;
	new_d_dec=dec_to_deg(hg->obj[i_list].dec)+
	  hg->obj[i_list].pm_dec/1000/60/60*yrs;

	new_ra=deg_to_ra(new_d_ra);
	new_dec=deg_to_dec(new_d_dec);

	fprintf(fp, "PM%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n# ",
		tgt, hg->obj[i_list].name, 
		new_ra,  new_dec, 
		hg->obj[i_list].equinox);
      }
      fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, 
	      hg->obj[i_list].equinox);
    }
    else{
      fprintf(fp, "# %s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, hg->obj[i_list].equinox);
    }
    g_free(tgt);
  }
  
  fprintf(fp, "\n");
  fprintf(fp, "</PARAMETER_LIST>\n");
  fprintf(fp, "\n");


  fprintf(fp, "<COMMAND>\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "########################################################################\n");
  fprintf(fp, "# Commands for taking bias and dark.\n");
  fprintf(fp, "#\n");
  fprintf(fp, "# You can specify the number of bias/dark you want to take using\n");
  fprintf(fp, "# the parameter \"NUMBER\".\n");
  fprintf(fp, "#\n");
  fprintf(fp, "########################################################################\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "#BIAS\n");
  fprintf(fp, "GetBias $DEF_IMAGE NUMBER=10\n");
  fprintf(fp, "\n");
  fprintf(fp, "#DARK\n");
  fprintf(fp, "GetDark $DEF_IMAGE EXPTIME=300 NUMBER=5\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "########################################################################\n");
  fprintf(fp, "# Filter Change Command\n");
  fprintf(fp, "#\n");
  fprintf(fp, "# Names of available filters:\n");
  fprintf(fp, "#\n");
  fprintf(fp, "# HSC-g, HSC-r2, HSC-i2, HSC-z, HSC-Y,\n");
  fprintf(fp, "# NB0387, NB0468, NB0515, NB0527, NB0656,\n");
  fprintf(fp, "# NB0718, NB0816, NB0921, NB0926, NB0973\n");
  fprintf(fp, "########################################################################\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
    fil_flag[i_fil]=FALSE;
  }
  for(i_set=0;i_set<hg->hsc_i_max;i_set++){
    fil_flag[hg->hsc_set[i_set].filter]=TRUE;
  }
  for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
    if(fil_flag[i_fil]){
      fprintf(fp, "# %s  (Filter ID=%d)\n",
	      hsc_filter[i_fil].name, hsc_filter[i_fil].id);
      fprintf(fp, "FilterChange1 $DEF_TOOLS FILTER=\"%s\"\n",
	      hsc_filter[i_fil].name);
      fprintf(fp, "FilterChange2 $DEF_TOOLS FILTER=\"%s\" MIRROR=CLOSE\n",
	      hsc_filter[i_fil].name);
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "#################### Command for Observation ####################\n");

  /////////////////// for Plan OPE /////////////////
  if(plan_flag){
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "###\n");
    fprintf(fp, "###               %02d/%02d/%4d Obs Sequence\n",
	    hg->fr_month,hg->fr_day,hg->fr_year);
    fprintf(fp, "###                  SunSet %d:%02d, Tw18Set %d:%02d, Tw18Rise %d:%02d, SunRise %d:%02d\n",
	    hg->sun.s_set.hours,
	    hg->sun.s_set.minutes,
	    hg->atw18.s_set.hours,
	    hg->atw18.s_set.minutes,
	    hg->atw18.s_rise.hours,
	    hg->atw18.s_rise.minutes,
	    hg->sun.s_rise.hours,
	    hg->sun.s_rise.minutes);
    fprintf(fp, "###                  Age of the moon : %.1lf days\n",hg->fr_moon);
    fprintf(fp, "###\n");
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "\n");
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_OBJ:
	HSC_WriteOPE_OBJ_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_SetAzEl:
	fprintf(fp, "### SetAzEl  Az=%d El=%d  via Launcher ###\n\n",
		(int)hg->plan[i_plan].az1,(int)hg->plan[i_plan].el1);
	break;
	
      case PLAN_TYPE_FOCUS:
	HSC_WriteOPE_FOCUS_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_FLAT:
	HSC_WriteOPE_FLAT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_SETUP:
	HSC_WriteOPE_SETUP_plan(fp,hg,hg->plan[i_plan]);
	break;
      }
    
    }
    fprintf(fp, "\n");
    fprintf(fp, "##########################################################\n");
    fprintf(fp, "##########################################################\n");
    fprintf(fp, "##########################################################\n");
    fprintf(fp, "##########################################################\n");
    fprintf(fp, "##########################################################\n");
  }
  /////////////////// for Plan OPE ///////////////// until here
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "#################### Reserved Targets ####################\n");

  for(i_set=0; i_set<hg->hsc_i_max; i_set++){
    fprintf(fp, "\n");
    for(i=0;i<strlen("##### ")*2+strlen("Setup-XX: ")+strlen(hg->hsc_set[i_set].txt);i++){
      fprintf(fp, "#");
    }
    fprintf(fp, "\n");
    fprintf(fp, "##### Setup-%02d: %s #####\n",i_set+1, hg->hsc_set[i_set].txt);
    for(i=0;i<strlen("##### ")*2+strlen("Setup-XX: ")+strlen(hg->hsc_set[i_set].txt);i++){
      fprintf(fp, "#");
    }
    fprintf(fp, "\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      HSC_WriteOPE_obj(fp, hg, i_list, i_set);
      fprintf(fp, "\n");
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");

  
  fprintf(fp, "########################################################################\n");
  fprintf(fp, "# Dome Flat\n");
  fprintf(fp, "#\n");
  fprintf(fp, "# Please issue SetupDomeFlat command with SETUP=SETUP when you want to\n");
  fprintf(fp, "# turn on the light. If the light is on, use SETUP=CHANGE to change\n");
  fprintf(fp, "# the voltage and ampair. Appropriate combinations of VOLT, AMP and EXPTIME\n");
  fprintf(fp, "########################################################################\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
    if(fil_flag[i_fil]){
      fprintf(fp, "# %s  (Filter ID=%d)\n",
	      hsc_filter[i_fil].name, hsc_filter[i_fil].id);
      if(!hsc_filter[hg->hsc_set[i_set].filter].flat_flg){
	fprintf(fp, "# !!! These Dome Flat parameters are temporal. Be careful!!!\n");
      }
      fprintf(fp, "SetupDomeFlat $DEF_TOOLS SETUP=SETUP Z=3.75 INSROT_POS=0 LAMP=4X%dW VOLT=%.2lf AMP=%.2lf\n",
	      hsc_filter[i_fil].flat_w,
	      hsc_filter[i_fil].flat_v,
	      hsc_filter[i_fil].flat_a);
      fprintf(fp, "# Wait 15 minutes for flat lamp stability. Check Insrot = 0.0 deg, Z = 3.75 mm. \n");
      fprintf(fp, "GetDomeFlat $DEF_IMAGE EXPTIME=%d Filter=\"%s\"\n",
	      hsc_filter[i_fil].flat_exp,
	      hsc_filter[i_fil].name);
      fprintf(fp, "GetDomeFlat $DEF_IMAGE EXPTIME=%d Filter=\"%s\"  NUMBER=9\n",
	      hsc_filter[i_fil].flat_exp,
	      hsc_filter[i_fil].name);
      fprintf(fp, "\n");
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "</Command>\n");


  fclose(fp);
}


void HSC_WriteOPE_obj(FILE*fp, typHOE *hg, gint i_list, gint i_set){
  gchar *tgt=NULL, *ag_str=NULL, *dith_str=NULL;
  gboolean flag_nst=FALSE, flag_std=FALSE;
  gint i_fil, i_dith;

  i_fil=hg->hsc_set[i_set].filter;
  i_dith=hg->hsc_set[i_set].dith;

  // Check for Non-Sidereal
  if(hg->obj[i_list].i_nst>=0){ 
    if((i_dith==HSC_DITH_NO) && (!hg->hsc_set[i_set].ag)){
      flag_nst=TRUE;
      tgt=g_strdup_printf("OBJECT=\"%s\" FILE=\"08 %s\"",
			  hg->obj[i_list].name,
			  g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
    }
    else{
      return;
    }
  }

  // Check for Standard
  if(hg->obj[i_list].std){ 
    if((i_dith==HSC_DITH_NO) && (!hg->hsc_set[i_set].ag)){
      flag_std=TRUE;
      tgt=make_tgt(hg->obj[i_list].name, "$STD_");
    }
    else{
      return;
    }
  }
  
  // Normal target
  if(!tgt){
    tgt=make_tgt(hg->obj[i_list].name, "$TGT_");
  }



  fprintf(fp, "## Object-%d : \"%s\"\n",i_list+1, hg->obj[i_list].name);
  if(hg->obj[i_list].note){
    fprintf(fp, "#      %s\n", hg->obj[i_list].note);
  }
  
  // AG
  if(hg->hsc_set[i_set].ag){
    if(!hsc_filter[i_fil].ag_flg){
      fprintf(fp, "# !!! These AG parameters are temporal. Be careful!!!\n");
    }
    ag_str=g_strdup_printf("GOODMAG=%.1lf AG_EXP=%.1lf SELECT_MODE=SEMIAUTO",
			   hsc_filter[i_fil].good_mag,
			   hsc_filter[i_fil].ag_exp);
  }
  else{
    ag_str=g_strdup("");
  }

  // Dithering parameters
  switch(i_dith){
  case HSC_DITH_NO:
    dith_str=g_strdup("");
    break;
    
  case HSC_DITH_5:
    dith_str=g_strdup_printf("DITH_RA=%d DITH_DEC=%d",
			     hg->hsc_set[i_set].dith_ra,
			     hg->hsc_set[i_set].dith_dec);
    break;
    
  case HSC_DITH_N:
    dith_str=g_strdup_printf("NDITH=%d RDITH=%d TDITH=%d",
			     hg->hsc_set[i_set].dith_n,
			     hg->hsc_set[i_set].dith_r,
			     hg->hsc_set[i_set].dith_t);
    break;
  }

  if(flag_nst) { // Non-Sidereal
    // FocusOBE
    fprintf(fp, "SetupField $DEF_%s %s OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hg->hsc_set[i_set].def,
	    tgt,
	    hg->hsc_set[i_set].osra,
	    hg->hsc_set[i_set].osdec,
	    hsc_filter[i_fil].name,
	    hg->obj[i_list].pa);
	    
    fprintf(fp, "FocusOBE $DEF_IMAGE OBJECT=\"FOCUS TEST\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 EXPTIME=10 Z=%.2lf DELTA_Z=0.05 DELTA_DEC=5 Filter=\"%s\"\n",
	    hg->hsc_focus_z,
	    hsc_filter[i_fil].name);
    fprintf(fp, "\n");

    // SetupField
    fprintf(fp, "SetupField $DEF_%s %s OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hg->hsc_set[i_set].def,
	    tgt,
	    hg->hsc_set[i_set].osra,
	    hg->hsc_set[i_set].osdec,
	    hsc_filter[i_fil].name,
	    hg->obj[i_list].pa);

    // GetObject
    fprintf(fp, "GetObject  $DEF_%s %s EXPTIME=%.0lf OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hg->hsc_set[i_set].def,
	    tgt,
	    hg->hsc_set[i_set].exp,
	    hg->hsc_set[i_set].osra,
	    hg->hsc_set[i_set].osdec,
	    hsc_filter[i_fil].name,
	    hg->obj[i_list].pa);
  }
 else{ // All sidereal targets
    // FocusOBE
    fprintf(fp, "FocusOBE $DEF_IMAGE %s EXPTIME=10 Z=%.2lf DELTA_Z=0.05 DELTA_DEC=5 Filter=\"%s\" INSROT_PA=%.2lf\n",
	    tgt,
	    hg->hsc_focus_z,
	    hsc_filter[i_fil].name,
	    hg->obj[i_list].pa);
    fprintf(fp, "\n");
    
    // SetupField
    fprintf(fp, "SetupField $DEF_%s %s OFFSET_RA=%d OFFSET_DEC=%d %s %s Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hg->hsc_set[i_set].def,
	    tgt,
	    hg->hsc_set[i_set].osra,
	    hg->hsc_set[i_set].osdec,
	    dith_str,
	    ag_str,
	    hsc_filter[i_fil].name,
	    hg->obj[i_list].pa);
    
    // GetObject
    if(flag_std){
      fprintf(fp, "GetStandard  $DEF_%s %s EXPTIME=%.0lf  DELTA_Z=%.2lf OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	      hg->hsc_set[i_set].def,
	      tgt,
	      hg->hsc_set[i_set].exp,
	      hg->hsc_delta_z,
	      hg->hsc_set[i_set].osra,
	      hg->hsc_set[i_set].osdec,
	      hsc_filter[i_fil].name,
	      hg->obj[i_list].pa);
    }
    else{
      fprintf(fp, "GetObject  $DEF_%s %s EXPTIME=%.0lf OFFSET_RA=%d OFFSET_DEC=%d %s %s Filter=\"%s\" INSROT_PA=%.2lf\n",
	      hg->hsc_set[i_set].def,
	      tgt,
	      hg->hsc_set[i_set].exp,
	      hg->hsc_set[i_set].osra,
	      hg->hsc_set[i_set].osdec,
	      dith_str,
	      ag_str,
	      hsc_filter[i_fil].name,
	      hg->obj[i_list].pa);
    }
  }

  if(tgt) g_free(tgt);
  if(ag_str) g_free(ag_str);
  if(dith_str) g_free(dith_str);
}


void HSC_WriteOPE_OBJ_plan(FILE*fp, typHOE *hg,  PLANpara plan){
  gchar *tgt=NULL, *ag_str=NULL, *dith_str=NULL, *skip_str=NULL, *stop_str=NULL;
  gboolean flag_nst=FALSE, flag_std=FALSE;
  gint i_set, i_list, i_fil, i_dith, i_repeat;
  gchar *tmp;

  i_set=plan.setup;
  i_list=plan.obj_i;
  i_fil=hg->hsc_set[i_set].filter;
  i_dith=hg->hsc_set[i_set].dith;

  // Check for Non-Sidereal
  if(hg->obj[i_list].i_nst>=0){ 
    if((i_dith==HSC_DITH_NO) && (!hg->hsc_set[i_set].ag)){
      flag_nst=TRUE;
      tgt=g_strdup_printf("OBJECT=\"%s\" FILE=\"08 %s\"",
			  hg->obj[i_list].name,
			  g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
    }
    else{
      tmp=g_strdup_printf("The object \"%s\" is a non-sidereal target.",
			  hg->obj[i_list].name);
      popup_message(hg->plan_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    -1,
		    tmp,
		    "You can only use \"No dither\" mode w/o AG for non-sidereal targets.",
		    NULL);
      g_free(tmp);
      return;
    }
  }

  // Check for Standard
  if(hg->obj[i_list].std){ 
    if((i_dith==HSC_DITH_NO) && (!hg->hsc_set[i_set].ag)){
      flag_std=TRUE;
      tgt=make_tgt(hg->obj[i_list].name, "$STD_");
    }
    else{
      tmp=g_strdup_printf("The object \"%s\" is registered as a standard star.",
			  hg->obj[i_list].name);
      popup_message(hg->plan_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    -1,
		    tmp,
		    "You can only use \"No dither\" mode w/o AG for standard stars.",
		    NULL);
      g_free(tmp);
      return;
    }
  }

  // Normal target
  if(!tgt){
    tgt=make_tgt(hg->obj[i_list].name, "$TGT_");
  }

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");

  fprintf(fp, "## Object-%d : \"%s\"\n",i_list+1, hg->obj[i_list].name);
  if(hg->obj[i_list].note){
    fprintf(fp, "#      %s\n", hg->obj[i_list].note);
  }
  
  // AG
  if(hg->hsc_set[i_set].ag){
    if(!hsc_filter[i_fil].ag_flg){
      fprintf(fp, "# !!! These AG parameters are temporal. Be careful!!!\n");
    }
    ag_str=g_strdup_printf("GOODMAG=%.1lf AG_EXP=%.1lf SELECT_MODE=SEMIAUTO",
			   hsc_filter[i_fil].good_mag,
			   hsc_filter[i_fil].ag_exp);
  }
  else{
    ag_str=g_strdup("");
  }

  // Dithering parameters
  switch(i_dith){
  case HSC_DITH_NO:
    dith_str=g_strdup("");
    break;
    
  case HSC_DITH_5:
    dith_str=g_strdup_printf("DITH_RA=%d DITH_DEC=%d",
			     hg->hsc_set[i_set].dith_ra,
			     hg->hsc_set[i_set].dith_dec);
    break;
    
  case HSC_DITH_N:
    dith_str=g_strdup_printf("NDITH=%d RDITH=%d TDITH=%d",
			     hg->hsc_set[i_set].dith_n,
			     hg->hsc_set[i_set].dith_r,
			     hg->hsc_set[i_set].dith_t);
    break;
  }

  // Skip
  switch(i_dith){
  case HSC_DITH_NO:
    skip_str=g_strdup("");
    break;
    
  case HSC_DITH_5:
  case HSC_DITH_N:
    if(plan.skip>0){
      skip_str=g_strdup_printf("SKIP=%d",plan.skip);
    }
    else{
      skip_str=g_strdup("");
    }
    break;
  }

  // Stop
  switch(i_dith){
  case HSC_DITH_NO:
    stop_str=g_strdup("");
    break;
    
  case HSC_DITH_5:
    if(plan.stop<5){
      stop_str=g_strdup_printf("STOP=%d",plan.stop);
    }
    else{
      stop_str=g_strdup("");
    }
    break;

  case HSC_DITH_N:
    if(plan.stop<hg->hsc_set[i_set].dith_n){
      stop_str=g_strdup_printf("STOP=%d",plan.stop);
    }
    else{
      stop_str=g_strdup("");
    }
    break;
  }
  
  if(flag_nst) { // Non-Sidereal
    // SetupField
    fprintf(fp, "SetupField $DEF_%s %s OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hg->hsc_set[i_set].def,
	    tgt,
	    plan.osra,
	    plan.osdec,
	    hsc_filter[i_fil].name,
	    (plan.pa_or) ? plan.pa : hg->obj[plan.obj_i].pa);

    for(i_repeat=0;i_repeat<plan.repeat;i_repeat++){
    // GetObject
      fprintf(fp, "GetObject  $DEF_%s %s EXPTIME=%.0lf OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	      hg->hsc_set[i_set].def,
	      tgt,
	      plan.dexp,
	      plan.osra,
	      plan.osdec,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[plan.obj_i].pa);
    }
  }
  else{ // All sidereal targets
    for(i_repeat=0;i_repeat<plan.repeat;i_repeat++){
      if((i_dith!=HSC_DITH_NO) || (i_repeat==0)){
	// SetupField
	fprintf(fp, "SetupField $DEF_%s %s OFFSET_RA=%d OFFSET_DEC=%d %s %s Filter=\"%s\" INSROT_PA=%.2lf %s %s\n",
		hg->hsc_set[i_set].def,
		tgt,
		plan.osra,
		plan.osdec,
		dith_str,
		ag_str,
		hsc_filter[i_fil].name,
		(plan.pa_or) ? plan.pa : hg->obj[plan.obj_i].pa,
		skip_str,
		stop_str);
      }
      
      // GetObject
      if(flag_std){ // Standard
	fprintf(fp, "GetStandard  $DEF_%s %s EXPTIME=%.0lf  DELTA_Z=%.2lf OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
		hg->hsc_set[i_set].def,
		tgt,
		plan.dexp,
		plan.delta_z,
		plan.osra,
		plan.osdec,
		hsc_filter[i_fil].name,
		(plan.pa_or) ? plan.pa : hg->obj[plan.obj_i].pa);
      }
      else{
	fprintf(fp, "GetObject  $DEF_%s %s EXPTIME=%.0lf OFFSET_RA=%d OFFSET_DEC=%d %s %s Filter=\"%s\" INSROT_PA=%.2lf%s%s %s\n",
		hg->hsc_set[i_set].def,
		tgt,
		plan.dexp,
		plan.osra,
		plan.osdec,
		dith_str,
		ag_str,
		hsc_filter[i_fil].name,
		(plan.pa_or) ? plan.pa : hg->obj[plan.obj_i].pa,
		(plan.hsc_30) ? " PURPOSE=\"CALIB_PHOTOM\" " : " ",
		skip_str,
		stop_str);
      }
    }
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");


  if(tgt) g_free(tgt);
  if(ag_str) g_free(ag_str);
  if(dith_str) g_free(dith_str);
  if(skip_str) g_free(skip_str);
  if(stop_str) g_free(stop_str);
}


void HSC_WriteOPE_FOCUS_plan(FILE*fp, typHOE *hg,  PLANpara plan){
  gchar *tgt=NULL;
  gboolean flag_nst=FALSE;
  gint i_set, i_list, i_fil;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  i_set=plan.setup;
  i_list=plan.focus_mode-1;
  i_fil=hg->hsc_set[i_set].filter;
  
  if(i_list>=0){
    // Check for Non-Sidereal
    if(hg->obj[i_list].i_nst>=0){ 
      flag_nst=TRUE;
      tgt=g_strdup_printf("OBJECT=\"%s\" FILE=\"08 %s\"",
			  hg->obj[i_list].name,
			  g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
    }
    else{
      // Check for Standard
      if(hg->obj[i_list].std){ 
	tgt=make_tgt(hg->obj[i_list].name, "$STD_");
      }
      else{
	tgt=make_tgt(hg->obj[i_list].name, "$TGT_");
      }
    }
  }

  if(i_list<0){ // at current position
    fprintf(fp, "FocusOBE $DEF_IMAGE OBJECT=\"FOCUS TEST\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 EXPTIME=10 Z=%.2lf DELTA_Z=0.05 DELTA_DEC=5 Filter=\"%s\" INSROT_PA=%.2lf\n",
	    plan.focus_z,
	    hsc_filter[i_fil].name,
	    (plan.pa_or) ? plan.pa : hg->def_pa);

    fprintf(fp, "\n");
    fprintf(fp, "# a 30 sec exposure for PURPOSE=\"CALIB_PHOTOM\"\n");
    fprintf(fp, "SetupField $DEF_IMAGE OBJECT=\"CALIB_PHOTOM\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 Filter=\"%s\" INSROT_PA=%.2lf\n",
	    hsc_filter[i_fil].name,
	    (plan.pa_or) ? plan.pa : hg->def_pa);
    fprintf(fp, "GetObject  $DEF_IMAGE  OBJECT=\"CALIB_PHOTOM\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 EXPTIME=30 Filter=\"%s\" INSROT_PA=%.2lf PURPOSE=\"CALIB_PHOTOM\"\n",
	    hsc_filter[i_fil].name,
	    (plan.pa_or) ? plan.pa : hg->def_pa);
  }
  else{
    if(flag_nst) { // Non-Sidereal
      // SetupField
      fprintf(fp, "SetupField $DEF_IMAGE %s OFFSET_RA=%d OFFSET_DEC=%d Filter=\"%s\" INSROT_PA=%.2lf\n",
	      tgt,
	      plan.osra,
	      plan.osdec,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
      fprintf(fp, "FocusOBE $DEF_IMAGE OBJECT=\"FOCUS TEST\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 EXPTIME=10 Z=%.2lf DELTA_Z=0.05 DELTA_DEC=5 Filter=\"%s\" INSROT_PA=%.2lf\n",
	      plan.focus_z,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);

      fprintf(fp, "\n");
      fprintf(fp, "# a 30 sec exposure for PURPOSE=\"CALIB_PHOTOM\"\n");
      fprintf(fp, "SetupField $DEF_IMAGE OBJECT=\"CALIB_PHOTOM\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 Filter=\"%s\" INSROT_PA=%.2lf\n",
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
      fprintf(fp, "GetObject  $DEF_IMAGE  OBJECT=\"CALIB_PHOTOM\" RA=!STATS.RA DEC=!STATS.DEC EQUINOX=2000.0 EXPTIME=30 Filter=\"%s\" INSROT_PA=%.2lf PURPOSE=\"CALIB_PHOTOM\"\n",
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
    }
    else{ // All sidereal targets
      fprintf(fp, "FocusOBE $DEF_IMAGE %s EXPTIME=10 Z=%.2lf DELTA_Z=0.05 DELTA_DEC=5 Filter=\"%s\" INSROT_PA=%.2lf\n",
	      tgt,
	      hg->hsc_focus_z,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
      
      fprintf(fp, "\n");
      fprintf(fp, "# a 30 sec exposure for PURPOSE=\"CALIB_PHOTOM\"\n");
      fprintf(fp, "SetupField $DEF_IMAGE %s Filter=\"%s\" INSROT_PA=%.2lf\n",
	      tgt,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
      fprintf(fp, "GetObject  $DEF_IMAGE %s EXPTIME=30 Filter=\"%s\" INSROT_PA=%.2lf PURPOSE=\"CALIB_PHOTOM\"\n",
	      tgt,
	      hsc_filter[i_fil].name,
	      (plan.pa_or) ? plan.pa : hg->obj[i_list].pa);
    }
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");

  if(tgt) g_free(tgt);
}


void HSC_WriteOPE_FLAT_plan(FILE*fp, typHOE *hg,  PLANpara plan){
  gint i_set, i_fil;
 
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  i_set=plan.setup;
  i_fil=hg->hsc_set[i_set].filter;

  if(!hsc_filter[i_fil].flat_flg){
    fprintf(fp, "# !!! These Dome Flat parameters are temporal. Be careful!!!\n");
  }
  fprintf(fp, "SetupDomeFlat $DEF_TOOLS SETUP=SETUP Z=3.75 INSROT_POS=0 LAMP=4X%dW VOLT=%.2lf AMP=%.2lf\n",
	  hsc_filter[i_fil].flat_w,
	  hsc_filter[i_fil].flat_v,
	  hsc_filter[i_fil].flat_a);
  fprintf(fp, "# Wait 15 minutes for flat lamp stability. Check Insrot = 0.0 deg, Z = 3.75 mm. \n");
  fprintf(fp, "GetDomeFlat $DEF_IMAGE EXPTIME=%d Filter=\"%s\"\n",
	  hsc_filter[i_fil].flat_exp,
	  hsc_filter[i_fil].name);
  fprintf(fp, "GetDomeFlat $DEF_IMAGE EXPTIME=%d Filter=\"%s\"  NUMBER=9\n",
	  hsc_filter[i_fil].flat_exp,
	  hsc_filter[i_fil].name);
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}

void HSC_WriteOPE_SETUP_plan(FILE*fp, typHOE *hg,  PLANpara plan){
  gint i_set, i_fil;
 
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  i_set=plan.setup;
  i_fil=hg->hsc_set[i_set].filter;

  fprintf(fp, "FilterChange1 $DEF_TOOLS FILTER=\"%s\"\n",
	  hsc_filter[i_fil].name);
  if(plan.sod>0){
    fprintf(fp, "# Rotate insrot angle [LAUNCHER/Telescope2] Free / 0.0 deg.\n");
    fprintf(fp, "# Close dome shutter and top screen\n");
  }
  if(plan.sod>0){
    fprintf(fp, "FilterChange2 $DEF_TOOLS FILTER=\"%s\" MIRROR=OPEN\n",
	    hsc_filter[i_fil].name);
  }
  else{
    fprintf(fp, "FilterChange2 $DEF_TOOLS FILTER=\"%s\" MIRROR=CLOSE\n",
	    hsc_filter[i_fil].name);
  }
  fprintf(fp, "# Check Filter ID (=%d) for \"%s\" in hkdump.\n",
	  hsc_filter[i_fil].id, hsc_filter[i_fil].name);
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


gint hsc_filter_get_from_id(gint fil_id){
  gint i_fil;

  for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
    if(hsc_filter[i_fil].id==fil_id){
      return(i_fil);
    }
  }

  return(-1);
}


// from menu.c
void hsc_do_export_def_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  gdouble tmp_focus_z, tmp_delta_z, tmp_pa;
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HSC)) return;

  tmp_pa=hg->def_pa;
  tmp_focus_z=hg->hsc_focus_z;
  tmp_delta_z=hg->hsc_delta_z;

  dialog = gtk_dialog_new_with_buttons("HOE : Set Default PA",
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

  frame = gtkut_frame_new ("<b>Set Default Parameters to the list</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(1, 2, FALSE, 0, 0, 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  

  label = gtk_label_new ("  PA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_pa,
					    -360.0, 360.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_pa);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  

  label = gtk_label_new ("  N-up=-90, W-up=0 (on zview)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  frame = gtkut_frame_new ("<b>Focusing parametes</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(1, 2, FALSE, 0, 0, 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  

  label = gtk_label_new ("  Center Z");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_focus_z,
					    3.0, 4.0, 0.05, 0.05, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_focus_z);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("    dZ for GetStandard");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_delta_z,
					    0.0, 1.0, 0.05, 0.05, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_delta_z);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);


  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    hg->def_pa=tmp_pa;
    hg->hsc_focus_z=tmp_focus_z;
    hg->hsc_delta_z=tmp_delta_z;
    hsc_export_def(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void HSC_Init_Filter(typHOE *hg){
  gint i_fil;

  if(hg->hsc_filter_ver) g_free(hg->hsc_filter_ver);
  hg->hsc_filter_ver=g_strdup("(Not synced yet. Please load manually -->)");

  for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
    hsc_filter[i_fil]=hsc_filter_stock[i_fil];
    hsc_filter[i_fil].name=g_strdup(hsc_filter_stock[i_fil].name);
  }
}

void HSC_Read_Filter(typHOE *hg)
{
  ConfigFile *cfgfile;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_fil;
  gchar *ini_file;
  gboolean fail_flag=FALSE;
  gchar *tmp;

  ini_file=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       HSC_FILTER_FILE,NULL);
  

  cfgfile = xmms_cfg_open_file(ini_file);

  // Basically this function never overwrite parameters when
  // it fails to find it in the reading ini file.
  if (cfgfile) {
    // General 
    if(hg->hsc_filter_ver) g_free(hg->hsc_filter_ver);
    hg->hsc_filter_ver=
      (xmms_cfg_read_string(cfgfile, "General", "ver",  &c_buf))? c_buf : NULL;

    if(hg->hsc_filter_ver){
      // Current Best Focus
      if(xmms_cfg_read_double(cfgfile, "General", "FocusZ",  &f_buf)){
	hg->hsc_focus_z=f_buf;
      }
      
      // Each filters
      for(i_fil=0;i_fil<NUM_HSC_FIL;i_fil++){
	if(hsc_filter_stock[i_fil].name){
	  // AG params
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "AG_mag",     &f_buf))
	    hsc_filter[i_fil].good_mag   =f_buf;
	  
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "AG_exp",     &f_buf))
	    hsc_filter[i_fil].ag_exp   =f_buf;
	  
	  if(xmms_cfg_read_boolean(cfgfile,
				   hsc_filter_stock[i_fil].name,
				   "AG_flg",     &b_buf))
	    hsc_filter[i_fil].ag_flg   =b_buf;
	  
	  // Dome Flat params
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "FLAT_v",     &f_buf))
	    hsc_filter[i_fil].flat_v   =f_buf;
	  
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "FLAT_a",     &f_buf))
	    hsc_filter[i_fil].flat_a   =f_buf;
	  
	  if(xmms_cfg_read_int(cfgfile,
			       hsc_filter_stock[i_fil].name,
			       "FLAT_exp",     &i_buf))
	    hsc_filter[i_fil].flat_exp   =i_buf;
	  
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "FLAT_w",     &f_buf))
	    hsc_filter[i_fil].flat_w   =f_buf;
	  
	  if(xmms_cfg_read_boolean(cfgfile,
				   hsc_filter_stock[i_fil].name,
				   "FLAT_flg",     &b_buf))
	    hsc_filter[i_fil].flat_flg   =b_buf;
	  
	  // Sensitivity params
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "SENS",     &f_buf))
	    hsc_filter[i_fil].sens   =f_buf;
	  
	  if(xmms_cfg_read_double(cfgfile,
				  hsc_filter_stock[i_fil].name,
				  "MAG1e",     &f_buf))
	    hsc_filter[i_fil].mag1e   =f_buf;
	}
      }
      
      hg->hsc_filter_updated=TRUE;
    }
    else{
      fail_flag=TRUE;
      hg->hsc_filter_ver=g_strdup("(Not synced yet. Please load manually -->)");
    }

    xmms_cfg_free(cfgfile);
  }
  else{
    fail_flag=TRUE;
  }
  
  if(fail_flag){  
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "Failed to load HSC filter information via network.",
		  "",
		  "     https://" HSC_FILTER_HOST HSC_FILTER_PATH,
		  "",
		  "Please try to sync manually later.",
		  NULL);
  }
  else{
    tmp=g_strdup_printf("List of HSC Filter (Ver. %s) has been loaded",
			hg->hsc_filter_ver);
			
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT,
		  tmp,
		  NULL);
    g_free(tmp);
  }
  
  g_free(ini_file);
}


void hsc_sync_filter(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar *tmp;
  hg=(typHOE *)gdata;
  
  hsc_fil_dl(hg);
  HSC_Read_Filter(hg);

  tmp = g_strdup_printf("List of HSC Filters : Ver. %s", hg->hsc_filter_ver);
  gtk_label_set_text(GTK_LABEL(hg->hsc_label_filter_ver), tmp);
  g_free(tmp);
  
  hscfil_make_tree(hg);
}


void hsc_fil_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gint fcdb_type_tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;
  
  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);
  
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=DBACCESS_HSCFIL;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(HSC_FILTER_HOST);
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  hg->fcdb_path=g_strdup(HSC_FILTER_PATH);
  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   HSC_FILTER_FILE,NULL);

  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Downloading HSC setup",
		 "Downloading HSC filter parameters ...",
		 FALSE, FALSE);
  my_signal_connect(hg->pdialog, "delete-event", delete_fcdb, (gpointer)hg);

  gtk_label_set_markup(GTK_LABEL(hg->plabel),
		       "Downloading HSC filter parameters ...");
 
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed", thread_cancel_fcdb, (gpointer)hg);
    
  gtk_widget_show_all(hg->pdialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
    
  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),TRUE);
  
  hg->ploop=g_main_loop_new(NULL, FALSE);
  hg->pcancel=g_cancellable_new();
  hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
  g_main_loop_run(hg->ploop);
  //g_thread_join(hg->pthread);
  g_main_loop_unref(hg->ploop);
  hg->ploop=NULL;

  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  if(GTK_IS_WIDGET(hg->pdialog)) gtk_widget_destroy(hg->pdialog);

  hg->fcdb_type=fcdb_type_tmp;
  flag_getFCDB=FALSE;
}


