// ircs.c for Subaru IRCS
//             Dec 2018  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"


// GUI creation in main window
void IRCS_TAB_create(typHOE *hg){
  {
    GtkWidget *w_top;
    GtkWidget *note, *im_note, *pi_note, *gr_note, *ps_note, *ec_note;
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
    gint i_band, i_slit, i_dith;
    GtkTreeModel *items_model;

    hg->ircs_vbox = gtkut_vbox_new(FALSE,0);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
    gtk_box_pack_start(GTK_BOX(hg->ircs_vbox), hbox,FALSE, FALSE, 2);

    table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), table,FALSE,FALSE,0);
    

    frame = gtk_frame_new ("Edit the List");
    gtkut_table_attach (table, frame, 0, 3, 0, 1,
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
		      IRCS_add_setup, 
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
		      IRCS_remove_setup, 
		      (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Remove");
#endif

#ifdef USE_GTK3
    button=gtkut_button_new_from_icon_name(NULL,"go-up");
#else
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
#endif
    gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      up_item_ircs_tree, 
		      (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Up");
#endif
    
#ifdef USE_GTK3
    button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
    gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      down_item_ircs_tree, 
		      (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Down");
#endif
    
    frame = gtk_frame_new ("Def. Exp.[s]");
    gtkut_table_attach (table, frame, 0, 3, 1, 3,
			GTK_SHRINK,GTK_SHRINK,0,0);
    gtk_container_set_border_width (GTK_CONTAINER(frame), 2);
    
    hbox1 = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

    gtk_container_add (GTK_CONTAINER (frame), hbox1);
    
    hg->ircs_exp_adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_exp,
							   0.006,
							   6000.0,
							   0.001,
							   1.0,
							   0);
    my_signal_connect (hg->ircs_exp_adj, "value_changed",
		       cc_get_adj_double,
		       &hg->ircs_exp);
    spinner =  gtk_spin_button_new (hg->ircs_exp_adj, 1, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
    gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

      
    note = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (note), GTK_POS_TOP);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (note), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), note,FALSE, FALSE, 2);

    my_signal_connect(note,"switch-page",
		      IRCS_get_mode, 
		      (gpointer)hg);

    // Imaging
    {
      table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);
    
      im_note = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (im_note), GTK_POS_TOP);
      gtk_notebook_set_scrollable (GTK_NOTEBOOK (im_note), FALSE);
      gtkut_table_attach (table, im_note, 0, 2, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
    
      my_signal_connect(im_note,"switch-page",
      		IRCS_get_im_mas, 
      		(gpointer)hg);
   
      // 52mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_IM52; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_im52_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_im52_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_im_band[IRCS_MAS_52]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_im52_band,
			 (gpointer)hg);

      hg->ircs_im_label[IRCS_MAS_52] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_im_label[IRCS_MAS_52],FALSE,FALSE,0);
      IRCS_set_im_label(hg, IRCS_MAS_52);

      label = gtk_label_new ("52mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (im_note), hbox1, label);
      
      
      // 20mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_IM20; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_im20_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_im20_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_im_band[IRCS_MAS_20]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_im20_band,
			 (gpointer)hg);

      hg->ircs_im_label[IRCS_MAS_20] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_im_label[IRCS_MAS_20],FALSE,FALSE,0);
      IRCS_set_im_label(hg, IRCS_MAS_20);    
      
      label = gtk_label_new ("20mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (im_note), hbox1, label);
   

      // Dithering
      frame = gtk_frame_new ("Dithering");
      gtkut_table_attach (table, frame, 2, 3, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame), 2);

      // Pattern
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);


      frame1 = gtk_frame_new ("Pattern");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	if(IRCS_dith[i_dith].f_im){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, 
			     IRCS_dith[i_dith].name,
			     1, i_dith, 2, TRUE, -1);
	  if(hg->ircs_im_dith==i_dith) iter_set=iter;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				      renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->ircs_im_dith);

      // Dithering Width
      frame1 = gtk_frame_new ("Width[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_im_dithw,
						1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_im_dithw);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      // O-S-O offsets
      frame1 = gtk_frame_new ("Offsets for O-S-O[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      label = gtk_label_new ("RA");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_im_osra,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_im_osra);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);
	
      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_im_osdec,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_im_osdec);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

	  
      label = gtk_label_new ("Imaging");
      gtk_notebook_append_page (GTK_NOTEBOOK (note), table, label);
    }


    // Pol-Imaging
    {
      table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);
    
      pi_note = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (pi_note), GTK_POS_TOP);
      gtk_notebook_set_scrollable (GTK_NOTEBOOK (pi_note), FALSE);
      gtkut_table_attach (table, pi_note, 0, 2, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
    
      my_signal_connect(pi_note,"switch-page",
      		IRCS_get_pi_mas, 
      		(gpointer)hg);
   
      // 52mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_PI52; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_pi52_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_pi52_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_pi_band[IRCS_MAS_52]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_pi52_band,
			 (gpointer)hg);

      hg->ircs_pi_label[IRCS_MAS_52] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_pi_label[IRCS_MAS_52],FALSE,FALSE,0);
      IRCS_set_pi_label(hg, IRCS_MAS_52);

      label = gtk_label_new ("52mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (pi_note), hbox1, label);
      
      
      // 20mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_PI20; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_pi20_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_pi20_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_pi_band[IRCS_MAS_20]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_pi20_band,
			 (gpointer)hg);

      hg->ircs_pi_label[IRCS_MAS_20] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_pi_label[IRCS_MAS_20],FALSE,FALSE,0);
      IRCS_set_pi_label(hg, IRCS_MAS_20);    
      
      label = gtk_label_new ("20mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (pi_note), hbox1, label);
   

      // Dithering
      frame = gtk_frame_new ("Dithering");
      gtkut_table_attach (table, frame, 2, 3, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame), 2);

      // Pattern
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);


      frame1 = gtk_frame_new ("Pattern");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	if(IRCS_dith[i_dith].f_pi){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, 
			     IRCS_dith[i_dith].name,
			     1, i_dith, 2, TRUE, -1);
	  if(hg->ircs_pi_dith==i_dith) iter_set=iter;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				      renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->ircs_pi_dith);

      // Dithering Width
      frame1 = gtk_frame_new ("Width[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_pi_dithw,
						1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_pi_dithw);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      // O-S-O Offsets
      frame1 = gtk_frame_new ("Offsets for O-S-O[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      label = gtk_label_new ("RA");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_pi_osra,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_pi_osra);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);
	
      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_pi_osdec,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_pi_osdec);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);


      label = gtk_label_new ("Pol-Imaging");
      gtk_notebook_append_page (GTK_NOTEBOOK (note), table, label);
    }

    
    // Grism
    {
      table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);

      gr_note = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gr_note), GTK_POS_TOP);
      gtk_notebook_set_scrollable (GTK_NOTEBOOK (gr_note), FALSE);
      gtkut_table_attach (table, gr_note, 0, 2, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
    
      my_signal_connect(gr_note,"switch-page",
      		IRCS_get_gr_mas, 
      		(gpointer)hg);
   
      // 52mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_GR52; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_gr52_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_gr52_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_gr_band[IRCS_MAS_52]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_gr52_band,
			 (gpointer)hg);

      hg->ircs_gr_label[IRCS_MAS_52] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_gr_label[IRCS_MAS_52],FALSE,FALSE,0);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_GR_WSLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	if(IRCS_gr_wslit[i_slit].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_gr_wslit[i_slit].name,
			     1, i_slit, 2, TRUE, -1);
	  if(hg->ircs_gr_slit[IRCS_MAS_52]==i_slit) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_slit, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_gr52_slit,
			 (gpointer)hg);
      
      hg->ircs_gr_label2[IRCS_MAS_52] = gtk_label_new ("resolution");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_gr_label2[IRCS_MAS_52],FALSE,FALSE,0);
      IRCS_set_gr_label(hg, IRCS_MAS_52);      

      label = gtk_label_new ("52mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (gr_note), hbox1, label);
      
      
      // 20mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_GR20; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_gr20_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_gr20_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_gr_band[IRCS_MAS_20]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_gr20_band,
			 (gpointer)hg);

      hg->ircs_gr_label[IRCS_MAS_20] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_gr_label[IRCS_MAS_20],FALSE,FALSE,0);
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_GR_NSLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	if(IRCS_gr_nslit[i_slit].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_gr_nslit[i_slit].name,
			     1, i_slit, 2, TRUE, -1);
	  if(hg->ircs_gr_slit[IRCS_MAS_20]==i_slit) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_slit, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_gr20_slit,
			 (gpointer)hg);
      
      hg->ircs_gr_label2[IRCS_MAS_20] = gtk_label_new ("resolution");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_gr_label2[IRCS_MAS_20],FALSE,FALSE,0);
      IRCS_set_gr_label(hg, IRCS_MAS_20);

      label = gtk_label_new ("20mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (gr_note), hbox1, label);
    

      // Dithering
      frame = gtk_frame_new ("Dithering");
      gtkut_table_attach (table, frame, 2, 3, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame), 2);

      // Pattern
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);


      frame1 = gtk_frame_new ("Pattern");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	if(IRCS_dith[i_dith].f_gr){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, 
			     IRCS_dith[i_dith].name,
			     1, i_dith, 2, TRUE, -1);
	  if(hg->ircs_gr_dith==i_dith) iter_set=iter;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				      renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->ircs_gr_dith);

      // Dithering Width
      frame1 = gtk_frame_new ("Width[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_gr_dithw,
						1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_gr_dithw);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      // O-S-O Offsets
      frame1 = gtk_frame_new ("Offsets for O-S-O[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      label = gtk_label_new ("RA");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_gr_osra,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_gr_osra);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);
	
      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_gr_osdec,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_gr_osdec);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);


      // Slit Scan
      frame1 = gtk_frame_new ("Slit Scan");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_gr_sssep,
						0.05, 5.00, 
						0.01, 0.1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_gr_sssep);
      spinner =  gtk_spin_button_new (adj, 1, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      label = gtk_label_new ("[\"]x");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_gr_ssnum,
						2, 20, 
						1, 1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_gr_ssnum);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      label = gtk_label_new ("Grism Spec.");
      gtk_notebook_append_page (GTK_NOTEBOOK (note), table, label);
    }


    // Pol-Grism
    {
      table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);

      ps_note = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (ps_note), GTK_POS_TOP);
      gtk_notebook_set_scrollable (GTK_NOTEBOOK (ps_note), FALSE);
      gtkut_table_attach (table, ps_note, 0, 2, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
    
      my_signal_connect(ps_note,"switch-page",
      		IRCS_get_ps_mas, 
      		(gpointer)hg);
   
      // 52mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_PS52; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ps52_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_ps52_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_ps_band[IRCS_MAS_52]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ps52_band,
			 (gpointer)hg);

      hg->ircs_ps_label[IRCS_MAS_52] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ps_label[IRCS_MAS_52],FALSE,FALSE,0);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_PS_WSLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ps_wslit[i_slit].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_ps_wslit[i_slit].name,
			     1, i_slit, 2, TRUE, -1);
	  if(hg->ircs_ps_slit[IRCS_MAS_52]==i_slit) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_slit, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ps52_slit,
			 (gpointer)hg);
      
      hg->ircs_ps_label2[IRCS_MAS_52] = gtk_label_new ("resolution");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ps_label2[IRCS_MAS_52],FALSE,FALSE,0);
      IRCS_set_ps_label(hg, IRCS_MAS_52);      

      label = gtk_label_new ("52mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (ps_note), hbox1, label);
      
      
      // 20mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_PS20; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ps20_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_ps20_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_ps_band[IRCS_MAS_20]==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ps20_band,
			 (gpointer)hg);

      hg->ircs_ps_label[IRCS_MAS_20] = gtk_label_new ("wavelength");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ps_label[IRCS_MAS_20],FALSE,FALSE,0);
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_PS_NSLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ps_nslit[i_slit].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_ps_nslit[i_slit].name,
			     1, i_slit, 2, TRUE, -1);
	  if(hg->ircs_ps_slit[IRCS_MAS_20]==i_slit) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_slit, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ps20_slit,
			 (gpointer)hg);
      
      hg->ircs_ps_label2[IRCS_MAS_20] = gtk_label_new ("resolution");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ps_label2[IRCS_MAS_20],FALSE,FALSE,0);
      IRCS_set_ps_label(hg, IRCS_MAS_20);

      label = gtk_label_new ("20mas");
      gtk_notebook_append_page (GTK_NOTEBOOK (ps_note), hbox1, label);
    

      // Dithering
      frame = gtk_frame_new ("Dithering");
      gtkut_table_attach (table, frame, 2, 3, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame), 2);

      // Pattern
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);


      frame1 = gtk_frame_new ("Pattern");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	if(IRCS_dith[i_dith].f_ps){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, 
			     IRCS_dith[i_dith].name,
			     1, i_dith, 2, TRUE, -1);
	  if(hg->ircs_ps_dith==i_dith) iter_set=iter;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				      renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->ircs_ps_dith);

      // Dithering Width
      frame1 = gtk_frame_new ("Width[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ps_dithw,
						1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_ps_dithw);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      // O-S-O Offsets
      frame1 = gtk_frame_new ("Offsets for O-S-O[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      label = gtk_label_new ("RA");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ps_osra,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ps_osra);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);
	
      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ps_osdec,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ps_osdec);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);


      // Slit Scan
      frame1 = gtk_frame_new ("Slit Scan");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ps_sssep,
						0.05, 5.00, 
						0.01, 0.1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_ps_sssep);
      spinner =  gtk_spin_button_new (adj, 1, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      label = gtk_label_new ("[\"]x");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ps_ssnum,
						2, 20, 
						1, 1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ps_ssnum);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				    TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      label = gtk_label_new ("Pol-Grism Spec.");
      gtk_notebook_append_page (GTK_NOTEBOOK (note), table, label);
    }

    
    // Echelle
    {
      table=gtkut_table_new(3, 3, FALSE, 3, 0, 0);

      ec_note = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (ec_note), GTK_POS_TOP);
      gtk_notebook_set_scrollable (GTK_NOTEBOOK (ec_note), FALSE);
      gtkut_table_attach (table, ec_note, 0, 2, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
    
      my_signal_connect(ec_note,"switch-page",
      		IRCS_get_ec_mas, 
      		(gpointer)hg);
   
      // 55mas
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_band=0; i_band < NUM_IRCS_ECD; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ecd_set[i_band].name){
	  gtk_list_store_set(store, &iter, 0, IRCS_ecd_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_ecd_band==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ecd_band,
			 (gpointer)hg);

      label = gtk_label_new (" Slit=");
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_EC_SLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	tmp=g_strdup_printf("%d\".%2d",
			    (gint)IRCS_ec_slit[i_slit].width,
			    (gint)((IRCS_ec_slit[i_slit].width-(gint)IRCS_ec_slit[i_slit].width)*100));
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_slit, 2, TRUE, -1);
	if(hg->ircs_ecd_slit==i_slit) iter_set=iter;
	if(tmp) g_free(tmp);
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ecd_slit,
			 (gpointer)hg);
      
      hg->ircs_ec_label[IRCS_ECD] = gtk_label_new ("x length");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ec_label[IRCS_ECD],FALSE,FALSE,0);
      IRCS_set_ec_label(hg, IRCS_ECD);
      

      label = gtk_label_new ("Defined");
      gtk_notebook_append_page (GTK_NOTEBOOK (ec_note), hbox1, label);
      
      // Manual
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      for(i_band=0; i_band < NUM_IRCS_ECM; i_band++){
	gtk_list_store_append(store, &iter);
	if(IRCS_ecm_set[i_band].name){
	  /*
	  if(i_band<IRCS_ECM_SEP1){
	    tmp=g_strdup_printf("%s (%.2lf--%.2lfum)",
				IRCS_ecm_set[i_band].name,
				IRCS_ecm_set[i_band].w1,
				IRCS_ecm_set[i_band].w2);
	  }
	  else{
	    tmp=g_strdup_printf("%s (%.3lf--%.3lfum)",
				IRCS_ecm_set[i_band].name,
				IRCS_ecm_set[i_band].w1,
				IRCS_ecm_set[i_band].w2);
	  }
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_band, 2, TRUE, -1);
	  if(tmp){
	    g_free(tmp);
	    tmp=NULL;
	    }*/
	  gtk_list_store_set(store, &iter, 0, IRCS_ecm_set[i_band].name,
			     1, i_band, 2, TRUE, -1);
	  if(hg->ircs_ecm_band==i_band) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_band, 2, FALSE, -1);
	}
      }	

      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ecm_band,
			 (gpointer)hg);
      
      label = gtk_label_new (" Ech=");
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ecm_ech,
						-15000, 15000, 
						1, 50, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ecm_ech);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
      gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);
      
      label = gtk_label_new (" XDS=");
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ecm_xds,
						-7000, 7000, 
						1, 50, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ecm_xds);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
      gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);

      label = gtk_label_new (" Slit=");
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
      
      for(i_slit=0; i_slit < NUM_IRCS_EC_SLIT; i_slit++){
	gtk_list_store_append(store, &iter);
	tmp=g_strdup_printf("%d\".%2d",
			    (gint)IRCS_ec_slit[i_slit].width,
			    (gint)((IRCS_ec_slit[i_slit].width-(gint)IRCS_ec_slit[i_slit].width)*100));
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_slit, 2, TRUE, -1);
	if(hg->ircs_ecm_slit==i_slit) iter_set=iter;
	if(tmp) g_free(tmp);
      }	
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
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
      my_signal_connect (combo,"changed",IRCS_get_ecm_slit,
			 (gpointer)hg);
      
      hg->ircs_ec_label[IRCS_ECM] = gtk_label_new ("x length");
      gtk_box_pack_start(GTK_BOX(hbox1),
			 hg->ircs_ec_label[IRCS_ECM],FALSE,FALSE,0);
      IRCS_set_ec_label(hg, IRCS_ECM);

      label = gtk_label_new ("Manual");
      gtk_notebook_append_page (GTK_NOTEBOOK (ec_note), hbox1, label);

      
      // Dithering
      frame = gtk_frame_new ("Dithering");
      gtkut_table_attach (table, frame, 2, 3, 0, 3,
			  GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame), 2);

      // Pattern
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);


      frame1 = gtk_frame_new ("Pattern");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	if(IRCS_dith[i_dith].f_ec){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, 
			     IRCS_dith[i_dith].name,
			     1, i_dith, 2, TRUE, -1);
	  if(hg->ircs_ec_dith==i_dith) iter_set=iter;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				      renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->ircs_ec_dith);

      // Dithering Width
      frame1 = gtk_frame_new ("Width[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ec_dithw,
						1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_ec_dithw);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      // O-S-O Offsets
      frame1 = gtk_frame_new ("Offsets for O-S-O[\"]");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      label = gtk_label_new ("RA");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ec_osra,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ec_osra);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);
	
      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);
	
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ec_osdec,
						-3000, 3000, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ec_osdec);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);


      // Slit Scan
      frame1 = gtk_frame_new ("Slit Scan");
      gtk_box_pack_start(GTK_BOX(hbox1), frame1,FALSE,FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER(frame1), 2);

      hbox2 = gtkut_hbox_new(FALSE,2);
      gtk_container_add (GTK_CONTAINER (frame1), hbox2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ec_sssep,
						0.05, 5.00, 
						0.01, 0.1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->ircs_ec_sssep);
      spinner =  gtk_spin_button_new (adj, 1, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      label = gtk_label_new ("[\"]x");
      gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->ircs_ec_ssnum,
						2, 20, 
						1, 1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->ircs_ec_ssnum);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				    TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE, FALSE, 0);

      
      label = gtk_label_new ("Echelle Spec.");
      gtk_notebook_append_page (GTK_NOTEBOOK (note), table, label);
    }


    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (hg->ircs_vbox), sw, TRUE, TRUE, 0);

    /* create models */
    items_model = ircs_create_items_model (hg);
      
    /* create tree view */
    hg->ircs_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->ircs_tree), TRUE);
#endif
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->ircs_tree)),
				 GTK_SELECTION_SINGLE);
    ircs_add_columns (hg, GTK_TREE_VIEW (hg->ircs_tree), items_model);
    
    g_object_unref (items_model);
    
    gtk_container_add (GTK_CONTAINER (sw), hg->ircs_tree);
    
    my_signal_connect (hg->ircs_tree, "cursor-changed",
    		       G_CALLBACK (ircs_focus_item), (gpointer)hg);
    


    label = gtk_label_new ("IRCS");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), hg->ircs_vbox, label);
  }
} 
  

// Parameter init calling from param_init() in main.c
void IRCS_param_init(typHOE *hg){
  gint i_mas, i_set;
  
  hg->ircs_im_mas=IRCS_MAS_52;

  for(i_mas=0;i_mas<NUM_IRCS_MAS;i_mas++){
    hg->ircs_im_band[i_mas]=0;
  }
  
  hg->ircs_im_dith=IRCS_DITH_S5;
  hg->ircs_im_dithw=7.0;
  hg->ircs_im_osra=30;
  hg->ircs_im_osdec=1800;

  hg->ircs_pi_mas=IRCS_MAS_52;
  hg->ircs_pi_dith=IRCS_DITH_V3;
  hg->ircs_pi_dithw=5.0;
  hg->ircs_pi_osra=30;
  hg->ircs_pi_osdec=1800;

  hg->ircs_gr_mas=IRCS_MAS_52;
  for(i_mas=0;i_mas<NUM_IRCS_MAS;i_mas++){
    hg->ircs_gr_band[i_mas]=0;
    hg->ircs_gr_slit[i_mas]=0;
  }

  hg->ircs_gr_dith=IRCS_DITH_ABBA;
  hg->ircs_gr_dithw=4.0;
  hg->ircs_gr_osra=30;
  hg->ircs_gr_osdec=1800;
  hg->ircs_gr_sssep=0.15;
  hg->ircs_gr_ssnum=5;

  hg->ircs_ps_dith=IRCS_DITH_ABBA;
  hg->ircs_ps_dithw=4.0;
  hg->ircs_ps_osra=30;
  hg->ircs_ps_osdec=1800;
  hg->ircs_ps_sssep=0.15;
  hg->ircs_ps_ssnum=5;

  hg->ircs_ec_mas=IRCS_ECD;
  hg->ircs_ecd_band=IRCS_ECD_IZ;
  hg->ircs_ecm_band=IRCS_ECM_IZ;
  hg->ircs_ecd_slit=IRCS_EC_SLIT_N;
  hg->ircs_ecm_slit=IRCS_EC_SLIT_N;
  hg->ircs_ecm_ech=0;
  hg->ircs_ecm_xds=0;

  hg->ircs_ec_dith=IRCS_DITH_ABBA;
  hg->ircs_ec_dithw=4.0;
  hg->ircs_ec_osra=30;
  hg->ircs_ec_osdec=1800;
  hg->ircs_ec_sssep=0.15;
  hg->ircs_ec_ssnum=5;

  hg->ircs_exp=IRCS_im52_set[0].defexp;

  hg->ircs_i=0;
  hg->ircs_i_max=0;

  for(i_set=0;i_set<IRCS_MAX_SET;i_set++){
    hg->ircs_set[i_set].txt=NULL;
    hg->ircs_set[i_set].def=NULL;
    hg->ircs_set[i_set].dtxt=NULL;
    hg->ircs_set[i_set].std=FALSE;
  }

  hg->ircs_magdb_skip=TRUE;
  hg->ircs_magdb_r_tgt=IRCS_MAGDB_R_TGT_ARCSEC;
  hg->ircs_magdb_r_ngs=IRCS_MAGDB_R_NGS_ARCSEC;
  hg->ircs_magdb_r_ttgs=IRCS_MAGDB_R_TTGS_ARCSEC;
  hg->ircs_magdb_mag_ngs=IRCS_MAGDB_MAG_NGS;
  hg->ircs_magdb_mag_ttgs=IRCS_MAGDB_MAG_TTGS;
}


// get mode by TAB change
void IRCS_get_mode (GtkNotebook *im_note, 
		    GtkWidget *widget, 
		    gint i_page,
		    gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_mode=i_page;

  ircs_set_def_exp(hg);
}

void IRCS_add_setup (GtkWidget *widget, gpointer *gdata)
{
  typHOE *hg = (typHOE *)gdata;

  switch(hg->ircs_mode){
  case IRCS_MODE_IM:
    IRCS_add_im(hg);
    break;

  case IRCS_MODE_PI:
    IRCS_add_pi(hg);
    break;

  case IRCS_MODE_GR:
    IRCS_add_gr(hg);
    break;

  case IRCS_MODE_PS:
    IRCS_add_ps(hg);
    break;

  case IRCS_MODE_EC:
    IRCS_add_ec(hg);
    break;
  }

  ircs_make_tree(hg);
}

void IRCS_remove_setup (GtkWidget *widget, gpointer *gdata)
{
  typHOE *hg = (typHOE *)gdata;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->ircs_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->ircs_tree));
  gint i_plan;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_set,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_IRCS_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);
    
    
    for(i_set=i;i_set<hg->ircs_i_max;i_set++){
      ircs_swap_setup(&hg->ircs_set[i_set], &hg->ircs_set[i_set+1]);
    }

    hg->ircs_i_max--;
    
    if (gtk_tree_model_iter_nth_child(model, &iter, NULL, hg->ircs_i_max)){
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
    ircs_update_tree(hg);

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

// get IM mas by TAB change
void IRCS_get_im_mas (GtkNotebook *im_note, 
		      GtkWidget *widget, 
		      gint i_page,
		      gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_im_mas=i_page;

  ircs_set_def_exp(hg);
}

void IRCS_get_pi_mas (GtkNotebook *im_note, 
		      GtkWidget *widget, 
		      gint i_page,
		      gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_pi_mas=i_page;

  ircs_set_def_exp(hg);
}

void cc_get_switch_page (GtkNotebook *note, 
			 GtkWidget *widget, 
			 gint i_page,
			 gint *gdata)
{
  *gdata=(gint) i_page;
}


void IRCS_get_im52_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_im_band[IRCS_MAS_52]=n;
    
    IRCS_set_im_label(hg, IRCS_MAS_52);

    ircs_set_def_exp(hg);
  }
}

void IRCS_get_pi52_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_pi_band[IRCS_MAS_52]=n;
    
    IRCS_set_pi_label(hg, IRCS_MAS_52);

    ircs_set_def_exp(hg);
  }
}

void IRCS_get_im20_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_im_band[IRCS_MAS_20]=n;
    
    IRCS_set_im_label(hg, IRCS_MAS_20);

    ircs_set_def_exp(hg);
  }
}

void IRCS_get_pi20_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_pi_band[IRCS_MAS_20]=n;
    
    IRCS_set_pi_label(hg, IRCS_MAS_20);

    ircs_set_def_exp(hg);
  }
}

void IRCS_set_im_label(typHOE *hg, gint i_mas){
  gchar *tmp=NULL;

  switch(i_mas){
  case IRCS_MAS_52:
    tmp=g_strdup_printf("%.3lf--%.3lf um",
			IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].wv1,
			IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_im_label[IRCS_MAS_52]), tmp);
    break;

  case IRCS_MAS_20:
    tmp=g_strdup_printf("%.3lf--%.3lf um",
			IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].wv1,
			IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_im_label[IRCS_MAS_20]), tmp);
    break;

  }

  if(tmp) g_free(tmp);
}
			 
void IRCS_set_pi_label(typHOE *hg, gint i_mas){
  gchar *tmp=NULL;

  switch(i_mas){
  case IRCS_MAS_52:
    tmp=g_strdup_printf("%.3lf--%.3lf um",
			IRCS_pi52_set[hg->ircs_im_band[IRCS_MAS_52]].wv1,
			IRCS_pi52_set[hg->ircs_im_band[IRCS_MAS_52]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_pi_label[IRCS_MAS_52]), tmp);
    break;

  case IRCS_MAS_20:
    tmp=g_strdup_printf("%.3lf--%.3lf um",
			IRCS_im20_set[hg->ircs_pi_band[IRCS_MAS_20]].wv1,
			IRCS_im20_set[hg->ircs_pi_band[IRCS_MAS_20]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_pi_label[IRCS_MAS_20]), tmp);
    break;

  }

  if(tmp) g_free(tmp);
}
			 
void IRCS_add_im (typHOE *hg)
{
  gchar *filter=NULL, *dith=NULL;
  gint i_set;

  if(hg->ircs_i_max>=IRCS_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of IRCS setups reaches to the maximum.",
		  NULL);
    return;
  }


  i_set=hg->ircs_i_max;

  hg->ircs_set[i_set].mode=IRCS_MODE_IM;
  hg->ircs_set[i_set].mas=hg->ircs_im_mas;
  hg->ircs_set[i_set].band=hg->ircs_im_band[hg->ircs_im_mas];
  hg->ircs_set[i_set].slit=0;
  hg->ircs_set[i_set].dith=hg->ircs_im_dith;
  
  hg->ircs_set[i_set].dithw=hg->ircs_im_dithw;
  hg->ircs_set[i_set].osra=hg->ircs_im_osra;
  hg->ircs_set[i_set].osdec=hg->ircs_im_osdec;
  hg->ircs_set[i_set].sssep=0;
  hg->ircs_set[i_set].ssnum=0;

  hg->ircs_set[i_set].shot=IRCS_dith[hg->ircs_set[i_set].dith].shot;

  hg->ircs_set[i_set].ech=0;
  hg->ircs_set[i_set].xds=0;

  hg->ircs_set[i_set].spw=0;

  hg->ircs_set[i_set].exp=hg->ircs_exp;
  
  filter=ircs_get_filter_name(hg, i_set);
  
  switch(hg->ircs_im_mas){
  case IRCS_MAS_52:
    hg->ircs_set[i_set].cw1=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].cw3;
    hg->ircs_set[i_set].slw=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].slw;
    hg->ircs_set[i_set].cam=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].cam;
    break;

  case IRCS_MAS_20:
    hg->ircs_set[i_set].cw1=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].cw3;
    hg->ircs_set[i_set].slw=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].slw;
    hg->ircs_set[i_set].cam=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].cam;
    break;
  };

  hg->ircs_set[i_set].std=TRUE;
  
  hg->ircs_set[i_set].slit_x=512;
  hg->ircs_set[i_set].slit_y=512;
  

  if(hg->ircs_set[i_set].txt) g_free(hg->ircs_set[i_set].txt);
  hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);

  if(hg->ircs_set[i_set].def) g_free(hg->ircs_set[i_set].def);
  hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);

  if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
  hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					  hg->ircs_set[i_set].dithw,
					  hg->ircs_set[i_set].osra,
					  hg->ircs_set[i_set].osdec,
					  hg->ircs_set[i_set].sssep,
					  hg->ircs_set[i_set].ssnum);

  hg->ircs_i_max++;

  if(filter) g_free(filter);
}


void IRCS_add_pi (typHOE *hg)
{
  gchar *filter=NULL, *dith=NULL;
  gint i_set;

  if(hg->ircs_i_max>=IRCS_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of IRCS setups reaches to the maximum.",
		  NULL);
    return;
  }


  i_set=hg->ircs_i_max;

  hg->ircs_set[i_set].mode=IRCS_MODE_PI;
  hg->ircs_set[i_set].mas=hg->ircs_pi_mas;
  hg->ircs_set[i_set].band=hg->ircs_pi_band[hg->ircs_pi_mas];
  hg->ircs_set[i_set].slit=0;
  hg->ircs_set[i_set].dith=hg->ircs_pi_dith;
  
  hg->ircs_set[i_set].dithw=hg->ircs_pi_dithw;
  hg->ircs_set[i_set].osra=hg->ircs_pi_osra;
  hg->ircs_set[i_set].osdec=hg->ircs_pi_osdec;
  hg->ircs_set[i_set].sssep=0.150;
  hg->ircs_set[i_set].ssnum=5;

  hg->ircs_set[i_set].shot=IRCS_dith[hg->ircs_set[i_set].dith].shot;

  hg->ircs_set[i_set].ech=0;
  hg->ircs_set[i_set].xds=0;

  hg->ircs_set[i_set].spw=0;

  hg->ircs_set[i_set].exp=hg->ircs_exp;
  
  filter=ircs_get_filter_name(hg, i_set);

  switch(hg->ircs_pi_mas){
  case IRCS_MAS_52:
    hg->ircs_set[i_set].cw1=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].cw3;
    hg->ircs_set[i_set].slw=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].slw;
    hg->ircs_set[i_set].cam=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].cam;
    break;

  case IRCS_MAS_20:
    hg->ircs_set[i_set].cw1=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].cw3;
    hg->ircs_set[i_set].slw=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].slw;
    hg->ircs_set[i_set].cam=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].cam;
    break;
  };

  hg->ircs_set[i_set].std=TRUE;

  hg->ircs_set[i_set].slit_x=512;
  hg->ircs_set[i_set].slit_y=512;
  

  if(hg->ircs_set[i_set].txt) g_free(hg->ircs_set[i_set].txt);
  hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);

  if(hg->ircs_set[i_set].def) g_free(hg->ircs_set[i_set].def);
  hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);

  if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
  hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					  hg->ircs_set[i_set].dithw,
					  hg->ircs_set[i_set].osra,
					  hg->ircs_set[i_set].osdec,
					  hg->ircs_set[i_set].sssep,
					  hg->ircs_set[i_set].ssnum);

  hg->ircs_i_max++;

  if(filter) g_free(filter);
}


// Grism
void IRCS_get_gr_mas (GtkNotebook *im_note, 
		      GtkWidget *widget, 
		      gint i_page,
		      gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_gr_mas=i_page;

  ircs_set_def_exp(hg);
}

void IRCS_get_ps_mas (GtkNotebook *im_note, 
		      GtkWidget *widget, 
		      gint i_page,
		      gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_ps_mas=i_page;

  ircs_set_def_exp(hg);
}

void IRCS_get_gr52_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_gr_band[IRCS_MAS_52]=n;
    
    IRCS_set_gr_label(hg, IRCS_MAS_52);

    ircs_set_def_exp(hg);
  }
  
}

void IRCS_get_ps52_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ps_band[IRCS_MAS_52]=n;
    
    IRCS_set_ps_label(hg, IRCS_MAS_52);

    ircs_set_def_exp(hg);
  }
  
}

void IRCS_get_gr20_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_gr_band[IRCS_MAS_20]=n;
    
    IRCS_set_gr_label(hg, IRCS_MAS_20);

    ircs_set_def_exp(hg);
  }
  
}

void IRCS_get_ps20_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ps_band[IRCS_MAS_20]=n;
    
    IRCS_set_ps_label(hg, IRCS_MAS_20);

    ircs_set_def_exp(hg);
  }
  
}

void IRCS_get_gr52_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_gr_slit[IRCS_MAS_52]=n;
    
    IRCS_set_gr_label(hg, IRCS_MAS_52);
  }
  
}

void IRCS_get_ps52_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ps_slit[IRCS_MAS_52]=n;
    
    IRCS_set_ps_label(hg, IRCS_MAS_52);
  }
  
}

void IRCS_get_gr20_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_gr_slit[IRCS_MAS_20]=n;
    
    IRCS_set_gr_label(hg, IRCS_MAS_20);
  }
  
}

void IRCS_get_ps20_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ps_slit[IRCS_MAS_20]=n;
    
    IRCS_set_ps_label(hg, IRCS_MAS_20);
  }
  
}


void IRCS_set_gr_label(typHOE *hg, gint i_mas){
  gchar *tmp=NULL, *tmp2=NULL;

  switch(i_mas){
  case IRCS_MAS_52:
    tmp=g_strdup_printf("%.2lf--%.2lf um  ",
			IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].wv1,
			IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_gr_label[IRCS_MAS_52]), tmp);
    tmp2=g_strdup_printf("R=%4.0lf",
			 (gdouble)IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].r010/(IRCS_gr_wslit[hg->ircs_gr_slit[IRCS_MAS_52]].width/0.10)*(IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].wv1+IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].wv2)/2.0);
    gtk_label_set_text(GTK_LABEL(hg->ircs_gr_label2[IRCS_MAS_52]), tmp2);
    break;

  case IRCS_MAS_20:
    tmp=g_strdup_printf("%.2lf--%.2lf um  ",
			IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].wv1,
			IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_gr_label[IRCS_MAS_20]), tmp);
    tmp2=g_strdup_printf("R=%4.0lf",
			 (gdouble)IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].r010/(IRCS_gr_nslit[hg->ircs_gr_slit[IRCS_MAS_20]].width/0.10)*(IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_20]].wv1+IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_20]].wv2)/2.0);
    gtk_label_set_text(GTK_LABEL(hg->ircs_gr_label2[IRCS_MAS_20]), tmp2);
    break;
  }

  if(tmp) g_free(tmp);
  if(tmp2) g_free(tmp2);
}
			 
void IRCS_set_ps_label(typHOE *hg, gint i_mas){
  gchar *tmp=NULL, *tmp2=NULL;

  switch(i_mas){
  case IRCS_MAS_52:
    tmp=g_strdup_printf("%.2lf--%.2lf um  ",
			IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].wv1,
			IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_ps_label[IRCS_MAS_52]), tmp);
    tmp2=g_strdup_printf("R=%4.0lf",
			 (gdouble)IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].r010/(IRCS_ps_wslit[hg->ircs_ps_slit[IRCS_MAS_52]].width/0.10)*(IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].wv1+IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].wv2)/2.0);
    gtk_label_set_text(GTK_LABEL(hg->ircs_ps_label2[IRCS_MAS_52]), tmp2);
    break;

  case IRCS_MAS_20:
    tmp=g_strdup_printf("%.2lf--%.2lf um  ",
			IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].wv1,
			IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].wv2);
    gtk_label_set_text(GTK_LABEL(hg->ircs_ps_label[IRCS_MAS_20]), tmp);
    tmp2=g_strdup_printf("R=%4.0lf",
			 (gdouble)IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].r010/(IRCS_ps_nslit[hg->ircs_ps_slit[IRCS_MAS_20]].width/0.10)*(IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_20]].wv1+IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_20]].wv2)/2.0);
    gtk_label_set_text(GTK_LABEL(hg->ircs_ps_label2[IRCS_MAS_20]), tmp2);
    break;
  }

  if(tmp) g_free(tmp);
  if(tmp2) g_free(tmp2);
}
			 
void IRCS_add_gr (typHOE *hg)
{
  gchar *filter=NULL, *dith=NULL;
  gdouble slit;
  gint i_set;

  if(hg->ircs_i_max>=IRCS_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of IRCS setups reaches to the maximum.",
		  NULL);
    return;
  }


  i_set=hg->ircs_i_max;

  hg->ircs_set[i_set].mode=IRCS_MODE_GR;
  hg->ircs_set[i_set].mas=hg->ircs_gr_mas;
  hg->ircs_set[i_set].band=hg->ircs_gr_band[hg->ircs_gr_mas];
  hg->ircs_set[i_set].slit=hg->ircs_gr_slit[hg->ircs_gr_mas];
  hg->ircs_set[i_set].dith=hg->ircs_gr_dith;
  
  hg->ircs_set[i_set].dithw=hg->ircs_gr_dithw;
  hg->ircs_set[i_set].osra=hg->ircs_gr_osra;
  hg->ircs_set[i_set].osdec=hg->ircs_gr_osdec;
  hg->ircs_set[i_set].sssep=hg->ircs_gr_sssep;
  hg->ircs_set[i_set].ssnum=hg->ircs_gr_ssnum;

  switch(hg->ircs_set[i_set].dith){
  case IRCS_DITH_SS:
    hg->ircs_set[i_set].shot=hg->ircs_set[i_set].ssnum;
    break;
    
  default:
    hg->ircs_set[i_set].shot=IRCS_dith[hg->ircs_set[i_set].dith].shot;
    break;
  }

  hg->ircs_set[i_set].ech=0;
  hg->ircs_set[i_set].xds=0;

  hg->ircs_set[i_set].spw=0;

  hg->ircs_set[i_set].exp=hg->ircs_exp;

  filter=ircs_get_filter_name(hg, i_set);
  slit=ircs_get_slit_width(hg, i_set);

  switch(hg->ircs_gr_mas){
  case IRCS_MAS_52:
    hg->ircs_set[i_set].cam=IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].cam;
    hg->ircs_set[i_set].slw=IRCS_gr_wslit[hg->ircs_gr_slit[IRCS_MAS_52]].slw;
    hg->ircs_set[i_set].slit_x=IRCS_gr_wslit[hg->ircs_gr_slit[IRCS_MAS_52]].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_gr_wslit[hg->ircs_gr_slit[IRCS_MAS_52]].slit_y;
    
    hg->ircs_set[i_set].cw1=IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].cw3;
    hg->ircs_set[i_set].std=TRUE;
    break;

  case IRCS_MAS_20:
    hg->ircs_set[i_set].cam=IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].cam;
    hg->ircs_set[i_set].slw=IRCS_gr_nslit[hg->ircs_gr_slit[IRCS_MAS_20]].slw;
    hg->ircs_set[i_set].slit_x=IRCS_gr_nslit[hg->ircs_gr_slit[IRCS_MAS_20]].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_gr_nslit[hg->ircs_gr_slit[IRCS_MAS_20]].slit_y;

    hg->ircs_set[i_set].cw1=IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].cw3;
    if(hg->ircs_gr_slit[IRCS_MAS_20]<IRCS_GR_NSLIT_SEP1){
      hg->ircs_set[i_set].std=TRUE;
    }
    else{
      hg->ircs_set[i_set].std=FALSE;
    }
    break;
  };

  if(hg->ircs_set[i_set].txt) g_free(hg->ircs_set[i_set].txt);
  hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);

  if(hg->ircs_set[i_set].def) g_free(hg->ircs_set[i_set].def);
  hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);
  
  if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
  hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					  hg->ircs_set[i_set].dithw,
					  hg->ircs_set[i_set].osra,
					  hg->ircs_set[i_set].osdec,
					  hg->ircs_set[i_set].sssep,
					  hg->ircs_set[i_set].ssnum);
  
  hg->ircs_i_max++;

  if(filter) g_free(filter);
}


void IRCS_add_ps (typHOE *hg)
{
  gchar *filter=NULL, *dith=NULL;
  gdouble slit;
  gint i_set;

  if(hg->ircs_i_max>=IRCS_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of IRCS setups reaches to the maximum.",
		  NULL);
    return;
  }


  i_set=hg->ircs_i_max;

  hg->ircs_set[i_set].mode=IRCS_MODE_PS;
  hg->ircs_set[i_set].mas=hg->ircs_ps_mas;
  hg->ircs_set[i_set].band=hg->ircs_ps_band[hg->ircs_ps_mas];
  hg->ircs_set[i_set].slit=hg->ircs_ps_slit[hg->ircs_ps_mas];
  hg->ircs_set[i_set].dith=hg->ircs_ps_dith;
  
  hg->ircs_set[i_set].dithw=hg->ircs_ps_dithw;
  hg->ircs_set[i_set].osra=hg->ircs_ps_osra;
  hg->ircs_set[i_set].osdec=hg->ircs_ps_osdec;
  hg->ircs_set[i_set].sssep=hg->ircs_ps_sssep;
  hg->ircs_set[i_set].ssnum=hg->ircs_ps_ssnum;

  switch(hg->ircs_set[i_set].dith){
  case IRCS_DITH_SS:
    hg->ircs_set[i_set].shot=hg->ircs_set[i_set].ssnum;
    break;
    
  default:
    hg->ircs_set[i_set].shot=IRCS_dith[hg->ircs_set[i_set].dith].shot;
    break;
  }

  hg->ircs_set[i_set].ech=0;
  hg->ircs_set[i_set].xds=0;

  hg->ircs_set[i_set].spw=0;

  hg->ircs_set[i_set].exp=hg->ircs_exp;

  filter=ircs_get_filter_name(hg, i_set);
  slit=ircs_get_slit_width(hg, i_set);

  switch(hg->ircs_ps_mas){
  case IRCS_MAS_52:
    hg->ircs_set[i_set].cam=IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].cam;
    hg->ircs_set[i_set].slw=IRCS_ps_wslit[hg->ircs_ps_slit[IRCS_MAS_52]].slw;
    hg->ircs_set[i_set].slit_x=IRCS_ps_wslit[hg->ircs_ps_slit[IRCS_MAS_52]].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_ps_wslit[hg->ircs_ps_slit[IRCS_MAS_52]].slit_y;
    
    hg->ircs_set[i_set].cw1=IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].cw3;
    break;

  case IRCS_MAS_20:
    hg->ircs_set[i_set].cam=IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].cam;
    hg->ircs_set[i_set].slw=IRCS_ps_nslit[hg->ircs_ps_slit[IRCS_MAS_20]].slw;
    hg->ircs_set[i_set].slit_x=IRCS_ps_nslit[hg->ircs_ps_slit[IRCS_MAS_20]].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_ps_nslit[hg->ircs_ps_slit[IRCS_MAS_20]].slit_y;

    hg->ircs_set[i_set].cw1=IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].cw1;
    hg->ircs_set[i_set].cw2=IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].cw2;
    hg->ircs_set[i_set].cw3=IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].cw3;
    break;
  };

  hg->ircs_set[i_set].std=TRUE;

  if(hg->ircs_set[i_set].txt) g_free(hg->ircs_set[i_set].txt);
  hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);

  if(hg->ircs_set[i_set].def) g_free(hg->ircs_set[i_set].def);
  hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);
  
  if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
  hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					  hg->ircs_set[i_set].dithw,
					  hg->ircs_set[i_set].osra,
					  hg->ircs_set[i_set].osdec,
					  hg->ircs_set[i_set].sssep,
					  hg->ircs_set[i_set].ssnum);
  
  hg->ircs_i_max++;

  if(filter) g_free(filter);
}


// Echelle
void IRCS_get_ec_mas (GtkNotebook *im_note, 
		      GtkWidget *widget, 
		      gint i_page,
		      gpointer *gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->ircs_ec_mas=i_page;

  ircs_set_def_exp(hg);
}

void IRCS_get_ecd_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ecd_band=n;
    
    IRCS_set_ec_label(hg, IRCS_ECD);

    ircs_set_def_exp(hg);
  }
}


void IRCS_get_ecm_band(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ecm_band=n;
    
    IRCS_set_ec_label(hg, IRCS_ECM);
    
    ircs_set_def_exp(hg);
  }
  
}


void IRCS_get_ecd_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ecd_slit=n;
    
    IRCS_set_ec_label(hg, IRCS_ECD);
  }
  
}

void IRCS_get_ecm_slit(GtkWidget *widget, gpointer *gdata){
  GtkTreeIter iter;
  gchar *tmp=NULL;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->ircs_ecm_slit=n;
    
    IRCS_set_ec_label(hg, IRCS_ECM);
  }
  
}


void IRCS_set_ec_label(typHOE *hg, gint i_mas){
  gchar *tmp=NULL;
  guint slw;
  gdouble length;

  switch(i_mas){
  case IRCS_ECD:
    switch(hg->ircs_ecd_slit){
    case IRCS_EC_SLIT_N:
      slw=IRCS_ecd_set[hg->ircs_ecd_band].slw_n;
      break;
    case IRCS_EC_SLIT_M:
      slw=IRCS_ecd_set[hg->ircs_ecd_band].slw_m;
      break;
    case IRCS_EC_SLIT_W:
      slw=IRCS_ecd_set[hg->ircs_ecd_band].slw_w;
      break;
    }

    if(slw!=0){
      length=IRCS_ec_slw[slw].length;
      tmp=g_strdup_printf("x%d\".%2d", (gint)length, (gint)((length-(gint)length)*100.0));
    }
    else{
      tmp=g_strdup("!!! no def. !!!");
    }
    
    gtk_label_set_text(GTK_LABEL(hg->ircs_ec_label[IRCS_ECD]), tmp);
    break;

  case IRCS_ECM:
    switch(hg->ircs_ecm_slit){
    case IRCS_EC_SLIT_N:
      slw=IRCS_ecm_set[hg->ircs_ecm_band].slw_n;
      break;
    case IRCS_EC_SLIT_M:
      slw=IRCS_ecm_set[hg->ircs_ecm_band].slw_m;
      break;
    case IRCS_EC_SLIT_W:
      slw=IRCS_ecm_set[hg->ircs_ecm_band].slw_w;
      break;
    }

    if(slw!=0){
      length=IRCS_ec_slw[slw].length;
      tmp=g_strdup_printf("x%d\".%2d", (gint)length, (gint)((length-(gint)length)*100.0));
    }
    else{
      tmp=g_strdup("!!! no def. !!!");
    }
    
    gtk_label_set_text(GTK_LABEL(hg->ircs_ec_label[IRCS_ECM]), tmp);
    break;
  }

  if(tmp) g_free(tmp);
}
			 

void IRCS_add_ec (typHOE *hg)
{
  gchar *filter=NULL, *dith=NULL;
  gdouble slit;
  gboolean valid=TRUE;
  gint i_set, i_slit;

  if(hg->ircs_i_max>=IRCS_MAX_SET){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Number of IRCS setups reaches to the maximum.",
		  NULL);
    return;
  }

  i_set=hg->ircs_i_max;

  hg->ircs_set[i_set].mode=IRCS_MODE_EC;
  hg->ircs_set[i_set].mas=hg->ircs_ec_mas;
  hg->ircs_set[i_set].dith=hg->ircs_ec_dith;

  hg->ircs_set[i_set].cw1=0;
  hg->ircs_set[i_set].cw2=0;
  hg->ircs_set[i_set].cw3=0;
  hg->ircs_set[i_set].cam=0;
  
  hg->ircs_set[i_set].dithw=hg->ircs_ec_dithw;

  hg->ircs_set[i_set].osra=hg->ircs_ec_osra;
  hg->ircs_set[i_set].osdec=hg->ircs_ec_osdec;
  hg->ircs_set[i_set].sssep=hg->ircs_ec_sssep;
  hg->ircs_set[i_set].ssnum=hg->ircs_ec_ssnum;

  hg->ircs_set[i_set].shot=ircs_get_shot(hg->ircs_set[i_set].dith, hg->ircs_set[i_set].ssnum);

  hg->ircs_set[i_set].exp=hg->ircs_exp;
  
  switch (hg->ircs_ec_mas){
  case IRCS_ECD:
    hg->ircs_set[i_set].band=hg->ircs_ecd_band;
    hg->ircs_set[i_set].slit=hg->ircs_ecd_slit;
    hg->ircs_set[i_set].ech=IRCS_ecd_set[hg->ircs_ecd_band].ech;
    hg->ircs_set[i_set].xds=IRCS_ecd_set[hg->ircs_ecd_band].xds;
    hg->ircs_set[i_set].spw=IRCS_ecd_set[hg->ircs_ecd_band].spw;
    break;

  case IRCS_ECM:
    hg->ircs_set[i_set].band=hg->ircs_ecm_band;
    hg->ircs_set[i_set].slit=hg->ircs_ecm_slit;
    hg->ircs_set[i_set].ech=hg->ircs_ecm_ech;
    hg->ircs_set[i_set].xds=hg->ircs_ecm_xds;
    hg->ircs_set[i_set].spw=IRCS_ecm_set[hg->ircs_ecm_band].spw;
    break;
  }    

  filter=ircs_get_filter_name(hg, i_set);
  slit=ircs_get_slit_width(hg, i_set);

  switch(hg->ircs_ec_mas){
  case IRCS_ECD:
    switch(hg->ircs_ecd_slit){
    case IRCS_EC_SLIT_N:
      hg->ircs_set[i_set].slw=IRCS_ecd_set[hg->ircs_set[i_set].band].slw_n;
      break;

    case IRCS_EC_SLIT_M:
      hg->ircs_set[i_set].slw=IRCS_ecd_set[hg->ircs_set[i_set].band].slw_m;
      break;

    case IRCS_EC_SLIT_W:
      hg->ircs_set[i_set].slw=IRCS_ecd_set[hg->ircs_set[i_set].band].slw_w;
      break;
    }
    
    if(hg->ircs_set[i_set].slw==0){
      valid=FALSE;
    }
    else{
      hg->ircs_set[i_set].std=TRUE;
    }

    hg->ircs_set[i_set].slit_x=IRCS_ec_slw[hg->ircs_set[i_set].slw].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_ec_slw[hg->ircs_set[i_set].slw].slit_y;
  
    break;

  case IRCS_ECM:
    switch(hg->ircs_ecd_slit){
    case IRCS_EC_SLIT_N:
      hg->ircs_set[i_set].slw=IRCS_ecm_set[hg->ircs_set[i_set].band].slw_n;
      break;
    case IRCS_EC_SLIT_M:
      hg->ircs_set[i_set].slw=IRCS_ecm_set[hg->ircs_set[i_set].band].slw_m;
      break;
    case IRCS_EC_SLIT_W:
      hg->ircs_set[i_set].slw=IRCS_ecm_set[hg->ircs_set[i_set].band].slw_w;
      break;
    }
    
    hg->ircs_set[i_set].slit_x=IRCS_ec_slw[hg->ircs_set[i_set].slw].slit_x;
    hg->ircs_set[i_set].slit_y=IRCS_ec_slw[hg->ircs_set[i_set].slw].slit_y;

    break;
  };

  if(valid){
    if(hg->ircs_set[i_set].txt)  g_free(hg->ircs_set[i_set].txt);
    hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);

    if(hg->ircs_set[i_set].def)  g_free(hg->ircs_set[i_set].def);
    hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);


    if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
    hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					    hg->ircs_set[i_set].dithw,
					    hg->ircs_set[i_set].osra,
					    hg->ircs_set[i_set].osdec,
					    hg->ircs_set[i_set].sssep,
					    hg->ircs_set[i_set].ssnum);
  
    hg->ircs_i_max++;
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "No definition for this band and slit combination.",
		  NULL);
  }

  if(filter) g_free(filter);
}


void ircs_set_def_exp(typHOE *hg){
  switch(hg->ircs_mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_im_mas){
    case IRCS_MAS_52:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].defexp,
			       IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].minexp,
			       IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].maxexp,
			       (IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_im52_set[hg->ircs_im_band[IRCS_MAS_52]].defexp;
	break;
	
    case IRCS_MAS_20:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].defexp,
			       IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].minexp,
			       IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].maxexp,
			       (IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_im20_set[hg->ircs_im_band[IRCS_MAS_20]].defexp;
      break;
    }
    break;
    
  case IRCS_MODE_PI:
    switch(hg->ircs_pi_mas){
    case IRCS_MAS_52:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].defexp,
			       IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].minexp,
			       IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].maxexp,
			       (IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_pi52_set[hg->ircs_pi_band[IRCS_MAS_52]].defexp;
	break;
	
    case IRCS_MAS_20:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].defexp,
			       IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].minexp,
			       IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].maxexp,
			       (IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_pi20_set[hg->ircs_pi_band[IRCS_MAS_20]].defexp;
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_gr_mas){
    case IRCS_MAS_52:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].defexp,
			       IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].minexp,
			       IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].maxexp,
			       (IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_gr52_set[hg->ircs_gr_band[IRCS_MAS_52]].defexp;
      break;
      
    case IRCS_MAS_20:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].defexp,
			       IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].minexp,
			       IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].maxexp,
			       (IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_gr20_set[hg->ircs_gr_band[IRCS_MAS_20]].defexp;
      break;
    }
    break;

  case IRCS_MODE_PS:
    switch(hg->ircs_ps_mas){
    case IRCS_MAS_52:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].defexp,
			       IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].minexp,
			       IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].maxexp,
			       (IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_ps52_set[hg->ircs_ps_band[IRCS_MAS_52]].defexp;
      break;
      
    case IRCS_MAS_20:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].defexp,
			       IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].minexp,
			       IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].maxexp,
			       (IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].minexp<1)?(0.001):(0.1),
			       1,
			       0);
      hg->ircs_exp=IRCS_ps20_set[hg->ircs_ps_band[IRCS_MAS_20]].defexp;
      break;
    }
    break;

  case IRCS_MODE_EC:
    switch(hg->ircs_ec_mas){
    case IRCS_ECD:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_ecd_set[hg->ircs_ecd_band].defexp,
			       IRCS_ecd_set[hg->ircs_ecd_band].minexp,
			       IRCS_ecd_set[hg->ircs_ecd_band].maxexp,
			       (IRCS_ecd_set[hg->ircs_ecd_band].maxexp<100)?(0.01):(1),
			       1,
			       0);
      hg->ircs_exp=IRCS_ecd_set[hg->ircs_ecd_band].defexp;
      break;
      
    case IRCS_ECM:
      gtk_adjustment_configure(hg->ircs_exp_adj,
			       IRCS_ecm_set[hg->ircs_ecm_band].defexp,
			       IRCS_ecm_set[hg->ircs_ecm_band].minexp,
			       IRCS_ecm_set[hg->ircs_ecm_band].maxexp,
			       (IRCS_ecm_set[hg->ircs_ecm_band].maxexp<100)?(0.01):(1),
			       1,
			       0);
      hg->ircs_exp=IRCS_ecm_set[hg->ircs_ecm_band].defexp;
      break;
    }
    break;
  }
	
}

GtkTreeModel *
ircs_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_IRCS, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
			      G_TYPE_STRING,  // def
			      G_TYPE_STRING,  // dith
			      G_TYPE_DOUBLE,  // exp
#ifdef USE_GTK3
			      GDK_TYPE_RGBA);    //bg color
#else
			      GDK_TYPE_COLOR);   //bg color
#endif

  for (i = 0; i < hg->ircs_i_max; i++){
    gtk_list_store_append (model, &iter);
    ircs_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void ircs_tree_update_item(typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_list)
{
  gchar tmp[24];
  gint i;
  gdouble s_rt=-1;

  // Num
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_NUMBER,
		      i_list+1,
		      -1);

  // Setup
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_NAME,
		      hg->ircs_set[i_list].txt,
		      -1);

  // Def
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_DEF,
		      hg->ircs_set[i_list].def,
		      -1);

  // Dith
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_DITH,
		      hg->ircs_set[i_list].dtxt,
		      -1);

  // Exp
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_EXP,
		      hg->ircs_set[i_list].exp,
		      -1);

  /* BG color */
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_IRCS_COLBG,
		      &col_ircs_setup[i_list],
		      -1);
}


void ircs_add_columns (typHOE *hg,
		       GtkTreeView  *treeview, 
		       GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Setup Number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_IRCS_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
						   renderer,
						   "text",
						   COLUMN_IRCS_NUMBER,
						   NULL); 
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_IRCS_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Setup Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_IRCS_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Setup",
						   renderer,
						   "text", 
						   COLUMN_IRCS_NAME,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_IRCS_COLBG,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_IRCS_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Def column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_IRCS_DEF));
  column=gtk_tree_view_column_new_with_attributes ("DEF",
						   renderer,
						   "text", 
						   COLUMN_IRCS_DEF,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_IRCS_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  ircs_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_IRCS_DEF),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_IRCS_DEF);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Dith column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_IRCS_DITH));
  column=gtk_tree_view_column_new_with_attributes ("Def. Dithering",
						   renderer,
						   "text", 
						   COLUMN_IRCS_DITH,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_IRCS_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  ircs_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_IRCS_DITH),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_IRCS_DITH);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Exp column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_IRCS_EXP));
  column=gtk_tree_view_column_new_with_attributes ("Def. ExpTime[s]",
						   renderer,
						   "text", 
						   COLUMN_IRCS_EXP,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_IRCS_COLBG,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  ircs_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_IRCS_EXP),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_IRCS_EXP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


void ircs_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->ircs_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->ircs_tree));

  if(gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_IRCS_NUMBER, &i, -1);
    i--;
    hg->ircs_i=i;
      
    gtk_tree_path_free (path);
  }
}


void ircs_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->ircs_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->ircs_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    ircs_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
}

void ircs_update_tree(typHOE *hg){
  int i_set;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->ircs_tree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_set=0;i_set<hg->ircs_i_max;i_set++){
    gtk_tree_model_get (model, &iter, COLUMN_IRCS_NUMBER, &i, -1);
    i--;
    ircs_tree_update_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}

void ircs_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gchar *value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_IRCS_DEF:
  case COLUMN_IRCS_DITH:
    if(!value){
      str=g_strdup_printf("---");
    }
    else{
      str=g_strdup(value);
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void ircs_double_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_IRCS_EXP:
    str=g_strdup_printf("%.3lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void ircs_swap_setup(IRCSpara *o1, IRCSpara *o2){
  IRCSpara temp;
  
  temp=*o2;
  *o2=*o1;
  *o1=temp;
}


void IRCS_WriteOPE(typHOE *hg, gboolean plan_flag){
  FILE *fp;
  int i, i_list=0, i_set, i_use, i_repeat, i_plan;
  gint to_year, to_month, to_day;
  gdouble new_ra, new_dec, new_d_ra, new_d_dec, yrs;
  gchar *tgt, *str;
  gchar *gsmode=NULL;
  gchar *gs_txt;

  if((fp=fopen(hg->filename_write,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_write);
    exit(1);
  }

  to_year=hg->fr_year;
  to_month=hg->fr_month;
  to_day=hg->fr_day;
  add_day(hg, &to_year, &to_month, &to_day, hg->nights);

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
  fprintf(fp, "*load \"ircs_mod.prm\"\n");
  fprintf(fp, "*load \"ircs_mec.prm\"\n");
  fprintf(fp, "\n");

  fprintf(fp, "DEF_AOLN=OBE_ID=AO188 OBE_MODE=LAUNCHER\n");
  fprintf(fp, "DEF_AOST=OBE_ID=AO188 OBE_MODE=AO188_SETUP\n");
  fprintf(fp, "\n");
  
  
  
  fprintf(fp, "\n");
  fprintf(fp, "###### LIST of OBJECTS ######\n");
  fprintf(fp, "#### Main Targets\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name, "TGT_");

    if(gsmode){
      g_free(gsmode);
    }
    switch(hg->obj[i_list].aomode){
    case AOMODE_NO:
      gsmode=g_strdup(" ");
      break;

    case AOMODE_NGS_S:
    case AOMODE_NGS_O:
      gsmode=g_strdup(" GSMODE=\"NGS\"");
      break;
      
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      gsmode=g_strdup(" GSMODE=\"LGS\"");
      break;
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

	fprintf(fp, "PM%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f%s\n# ",
		tgt, hg->obj[i_list].name, 
		new_ra,  new_dec, 
		hg->obj[i_list].equinox,
		gsmode);
      }
      fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f%s\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, 
	      hg->obj[i_list].equinox,
	      gsmode);
    }
    else{
      fprintf(fp, "# (Non-sidereal)\n# %s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f%s\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox,
	      gsmode);
    }
    g_free(tgt);
  }

  if(gsmode) g_free(gsmode);

  fprintf(fp, "\n");
  fprintf(fp, "#### Offset Natural Guide Stars\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    if((hg->obj[i_list].gs.flag)&&(hg->obj[i_list].aomode==AOMODE_NGS_O)){
      tgt=make_tgt(hg->obj[i_list].name, "NGS_");
      fprintf(fp, "## !!! Offset NGS for \"%s\" (Object Name is set just for GetObject)\n",
	      hg->obj[i_list].name);
      gs_txt=get_gs_txt(hg->obj[i_list].gs);
      fprintf(fp, "## %s\n", gs_txt);
      g_free(gs_txt);
      fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f GSMODE=\"NGS\"\n",
	      tgt,
	      hg->obj[i_list].name, 
	      hg->obj[i_list].gs.ra,  hg->obj[i_list].gs.dec, 
	      hg->obj[i_list].gs.equinox);

      g_free(tgt);
    }
  }  

  fprintf(fp, "\n");
  fprintf(fp, "#### Tip-Tilt Guide Stars\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    if((hg->obj[i_list].gs.flag)&&(hg->obj[i_list].aomode==AOMODE_LGS_O)){
      gs_txt=get_gs_txt(hg->obj[i_list].gs);
      fprintf(fp, "## %s\n", gs_txt);
      g_free(gs_txt);
      tgt=make_tgt(hg->obj[i_list].name, "TTGS_");
      fprintf(fp, "%s=OBJECT=\"TT GS for %s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f GSMODE=\"LGS\"\n",
	      tgt,
	      hg->obj[i_list].name, 
	      hg->obj[i_list].gs.ra,  hg->obj[i_list].gs.dec, 
	      hg->obj[i_list].gs.equinox);

      g_free(tgt);
    }
  }  

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "###### IRCS Setup Definition ######\n");
  for(i_set=0; i_set<hg->ircs_i_max; i_set++){
    fprintf(fp, "## Setup-%02d: %s\n",i_set+1,hg->ircs_set[i_set].txt);
    if(hg->ircs_set[i_set].std){
      fprintf(fp, "# (This setup is defined in  \"ircs_mec.prm\".)\n");
      switch(hg->ircs_set[i_set].mode){
      case IRCS_MODE_IM:
	fprintf(fp, "# DEF_%s=CW1=%d CW2=%d CW3=%d SLW=%d CAMFOC=%d SCALE=%s\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].cw1,
		hg->ircs_set[i_set].cw2,
		hg->ircs_set[i_set].cw3,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].cam,
		(hg->ircs_set[i_set].mas==IRCS_MAS_52) ? "L" : "H");
	break;

      case IRCS_MODE_GR:
	fprintf(fp, "# DEF_%s=CW1=%d CW2=%d CW3=%d SLW=%d CAMFOC=%d SCALE=%s\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].cw1,
		hg->ircs_set[i_set].cw2,
		hg->ircs_set[i_set].cw3,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].cam,
		(hg->ircs_set[i_set].mas==IRCS_MAS_52) ? "L" : "H");
	break;

      case IRCS_MODE_EC:
	fprintf(fp, "# DEF_%s=SLW=%d SPW=%d ECH=%d XDS=%d\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].spw,
		hg->ircs_set[i_set].ech,
		hg->ircs_set[i_set].xds);
	break;
      }
    }
    else{
      fprintf(fp, "# !!! User Mode !!!\n");
      switch(hg->ircs_set[i_set].mode){
      case IRCS_MODE_IM:
	fprintf(fp, "DEF_%s=CW1=%d CW2=%d CW3=%d SLW=%d CAMFOC=%d SCALE=%s\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].cw1,
		hg->ircs_set[i_set].cw2,
		hg->ircs_set[i_set].cw3,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].cam,
		(hg->ircs_set[i_set].mas==IRCS_MAS_52) ? "L" : "H");
	break;

      case IRCS_MODE_GR:
	fprintf(fp, "DEF_%s=CW1=%d CW2=%d CW3=%d SLW=%d CAMFOC=%d SCALE=%s\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].cw1,
		hg->ircs_set[i_set].cw2,
		hg->ircs_set[i_set].cw3,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].cam,
		(hg->ircs_set[i_set].mas==IRCS_MAS_52) ? "L" : "H");
	break;

      case IRCS_MODE_EC:
	fprintf(fp, "DEF_%s=SLW=%d SPW=%d ECH=%d XDS=%d\n",
		hg->ircs_set[i_set].def,
		hg->ircs_set[i_set].slw,
		hg->ircs_set[i_set].spw,
		hg->ircs_set[i_set].ech,
		hg->ircs_set[i_set].xds);
	break;
      }
    }
    fprintf(fp, "\n");
  }
  
  fprintf(fp, "</PARAMETER_LIST>\n");

  fprintf(fp, "\n");


  fprintf(fp, "<COMMAND>\n");
  fprintf(fp, "###==== Setup ====###\n");
  fprintf(fp, "        CheckOBE $DEF_IRST\n");
  fprintf(fp, "        BootQDAS $DEF_IRST\n");
  fprintf(fp, "        BootVGW  $DEF_CMNT\n");
  fprintf(fp, "###==== Choose a bright star  ====###\n");
  fprintf(fp, "        MoveToStar $DEF_CMNV\n");
  fprintf(fp, "###==== Focusing/IRCS ====###\n");
  fprintf(fp, "        FocusOBE_WO_AG $DEF_IRST $DEF_IMKP EXPTIME=0.5 COADDS=5 Z=!TSCL.Z DELTAZ=0.07\n");
  fprintf(fp, "\n");
  fprintf(fp, "###==== LGS Calibration (required only for LGS) ====###\n");
  fprintf(fp, "        AO188_M2CNT $DEF_AOST MODE=LGS INS=IRCS IMR=DEFAULT LGSH=DEFAULT MODECHANGE=YES\n");
  fprintf(fp, "        AO188_LGSCOL $DEF_AOST EXPTIME=10 MODE=MANUAL\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  
  fprintf(fp, "#################### Command for Observation ####################\n");

  /////////////////// for Plan OPE /////////////////
  if(plan_flag){
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "#################### %02d/%02d/%4d Obs Sequence ####################\n",
	    hg->fr_month,hg->fr_day,hg->fr_year);
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "#################################################################\n");
    fprintf(fp, "\n");
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_OBJ:
	IRCS_WriteOPE_OBJ_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_SetAzEl:
	fprintf(fp, "### SetAzEl  Az=%d El=%d  via Launcher ###\n\n",(int)hg->plan[i_plan].az1,(int)hg->plan[i_plan].el1);
	break;
	
      case PLAN_TYPE_FOCUS:
	IRCS_WriteOPE_FOCUS_plan(fp,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_FLAT:
	IRCS_WriteOPE_FLAT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_COMP:
	IRCS_WriteOPE_COMP_plan(fp,hg,hg->plan[i_plan]);
	break;
      }
    
    }
    fprintf(fp, "##===== ShutDown =====##\n");
    fprintf(fp, "ShutDownQDAS $DEF_IRST\n");
    fprintf(fp, "ShutDownVGW  $DEF_CMNT\n");
    fprintf(fp, "ShutDownOBE  $DEF_IRST\n");
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

  for(i_set=0; i_set<hg->ircs_i_max; i_set++){
    fprintf(fp, "\n");
    for(i=0;i<strlen("##### ")*2+strlen("Setup-XX: ")+strlen(hg->ircs_set[i_set].txt);i++){
      fprintf(fp, "#");
    }
    fprintf(fp, "\n");
    fprintf(fp, "##### Setup-%02d: %s #####\n",i_set+1, hg->ircs_set[i_set].txt);
    for(i=0;i<strlen("##### ")*2+strlen("Setup-XX: ")+strlen(hg->ircs_set[i_set].txt);i++){
      fprintf(fp, "#");
    }
    fprintf(fp, "\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      IRCS_WriteOPE_obj(fp, hg, i_list, i_set);
      fprintf(fp, "\n");
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");
  fprintf(fp, "##===== ShutDown =====##\n");
  fprintf(fp, "ShutDownQDAS $DEF_IRST\n");
  fprintf(fp, "ShutDownVGW  $DEF_CMNT\n");
  fprintf(fp, "ShutDownOBE  $DEF_IRST\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "</Command>\n");


  fclose(fp);
}


void IRCS_WriteOPE_obj(FILE*fp, typHOE *hg, gint i_list, gint i_set){
  gchar *tgt=NULL, *ttgs=NULL, *ngs=NULL;
  gboolean flag_nst=FALSE;
  gchar *mode_letter=NULL, *dith_letter=NULL;
  gchar *slew_to=NULL, *pa_str=NULL, *dith=NULL, *ao_sfx=NULL,
    *ss_pat=NULL, *ss_tmp1=NULL, *ss_tmp2=NULL, *tmode=NULL;
  gdouble d_ra, d_dec, d_x, d_y;
  gdouble ss_pos;
  gint i_scan;
  gchar *gs_txt;
  gboolean ao_ow=FALSE;
  gboolean l_flag=FALSE;


  mode_letter=g_strdup(ircs_mode_initial[hg->ircs_set[i_set].mode]);

  dith_letter=g_strdup(IRCS_dith[hg->ircs_set[i_set].dith].def);

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    pa_str=g_strdup_printf("FIELD_PA=%.2lf",hg->obj[i_list].pa);
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      switch(hg->ircs_set[i_set].band){
      case IRCS_IM52_LP:
      case IRCS_IM52_LPN:
      case IRCS_IM52_H2O:
      case IRCS_IM52_H3P:
      case IRCS_IM52_BRA:
      case IRCS_IM52_H2ON:
      case IRCS_IM52_BRACN:
      case IRCS_IM52_BRAN:
	l_flag=TRUE;
	break;
      }
      break;
      
    case IRCS_MAS_20:
      switch(hg->ircs_set[i_set].band){
      case IRCS_IM20_LP:
      case IRCS_IM20_LPN:
      case IRCS_IM20_H2O:
      case IRCS_IM20_BRAC:
      case IRCS_IM20_BRA:
      case IRCS_IM20_H2ON:
      case IRCS_IM20_BRACN:
      case IRCS_IM20_BRAN:
	l_flag=TRUE;
	break;
      }
      break;
    }
    break;

  case IRCS_MODE_PI:
    pa_str=g_strdup_printf("FIELD_PA=%.2lf",hg->obj[i_list].pa);
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      switch(hg->ircs_set[i_set].band){
      case IRCS_PI52_LP:
      case IRCS_PI52_LPN:
	l_flag=TRUE;
	break;
      }
      break;
      
    case IRCS_MAS_20:
      switch(hg->ircs_set[i_set].band){
      case IRCS_PI20_LP:
      case IRCS_PI20_LPN:
	l_flag=TRUE;
	break;
      }
      break;
    }
    break;
    
  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
  case IRCS_MODE_EC:
    pa_str=g_strdup_printf("SLIT_PA=%.2lf",hg->obj[i_list].pa);
    break;
  }
  
  
  if(hg->obj[i_list].i_nst>=0){ // Non-Sidereal
    tgt=g_strdup_printf("OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
			hg->obj[i_list].name,
			g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
    tmode=g_strdup("TMODE=\"NON-SID\"");
    flag_nst=TRUE;
  }
  else {
    tgt=make_tgt(hg->obj[i_list].name, "$TGT_");
    if(hg->obj[i_list].aomode==AOMODE_LGS_O){
      ttgs=make_tgt(hg->obj[i_list].name, "$TTGS_");
    }
    else if(hg->obj[i_list].aomode==AOMODE_NGS_O){
      ngs=make_tgt(hg->obj[i_list].name, "$NGS_");
    }

    if(hg->obj[i_list].adi){
      tmode=g_strdup("TMODE=ADI");
    }
    else{
      tmode=g_strdup("TMODE=SID");
    }
  }

	       
  // Comment line
  fprintf(fp, "## Object-%d : \"%s\"  ",i_list+1, hg->obj[i_list].name);
  switch(hg->obj[i_list].aomode){
  case AOMODE_NO:
    fprintf(fp, "(w/o AO)\n");
    slew_to=g_strdup(tgt);
    break;
    
  case AOMODE_NGS_S:
    fprintf(fp, "(NGS by target)\n");
    slew_to=g_strdup(tgt);
    break;
    
  case AOMODE_NGS_O:
    fprintf(fp, "(NGS by offset star)\n");
    gs_txt=get_gs_txt(hg->obj[i_list].gs);
    fprintf(fp, "# Slew to offset NGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ngs);
    break;
    
  case AOMODE_LGS_S:
    fprintf(fp, "(LGS : TT Guide Star = Target)\n");
    slew_to=g_strdup(tgt);
    break;
    
  case AOMODE_LGS_O:
    fprintf(fp, "(LGS : Offset TTGS)\n"); 
    gs_txt=get_gs_txt(hg->obj[i_list].gs);
    fprintf(fp, "# Slew to offset TTGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ttgs);
    break;
  }

  if(l_flag){
    fprintf(fp, "SetClock $DEF_IRST SLWCNT=1\n");
  }

  // SetupField
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
  case IRCS_MODE_PI:
  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
    fprintf(fp, "SetupField $DEF_%sSTA $DEF_IMK %s %s %s\n",
	    mode_letter, slew_to, pa_str, tmode);
    break;
  case IRCS_MODE_EC:
    fprintf(fp, "SetupField $DEF_%sSTA $DEF_%s $DEF_IMK %s %s %s\n",
	    mode_letter, hg->ircs_set[i_set].def, slew_to, pa_str, tmode);
    break;
  }

  // Offset for Guide Star
  switch(hg->obj[i_list].aomode){
  case AOMODE_LGS_O:
    fprintf(fp, "AO188_OFFSET_RADEC $DEF_AOLN %s %s\n",
	    tgt, pa_str);
    break;

  case AOMODE_NGS_O:
    d_ra=(ra_to_deg(hg->obj[i_list].ra)-ra_to_deg(hg->obj[i_list].gs.ra))*3600.0;
    d_dec=(dec_to_deg(hg->obj[i_list].dec)-dec_to_deg(hg->obj[i_list].gs.dec))*3600.0;
    fprintf(fp, "# Add Offset to the target from the offset NGS (dRA, dDec)=(%.2lf,%.2lf)\n",
	    d_ra, d_dec);
    break;
  }

  // CheckField
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
  case IRCS_MODE_PI:
  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
    fprintf(fp, "CheckField $DEF_%sSTA $DEF_IMK EXPTIME=10\n",
	    mode_letter);
    break;
  case IRCS_MODE_EC:
    fprintf(fp, "CheckField $DEF_%sSTA $DEF_%s EXPTIME=10\n",
	    mode_letter, hg->ircs_set[i_set].def);
    break;
  }

  fprintf(fp, "# Move Target using OBCP --> (%.1lf, %.1lf) \n",
	  hg->ircs_set[i_set].slit_x, hg->ircs_set[i_set].slit_y);

  // GetObject
  switch(hg->obj[i_list].aomode){
  case AOMODE_NO:
    switch(hg->ircs_set[i_set].mode){
    case IRCS_MODE_IM:
      if(!IRCS_dith[hg->ircs_set[i_set].dith].n_im) ao_ow=TRUE;
      break;
    case IRCS_MODE_PI:
      if(!IRCS_dith[hg->ircs_set[i_set].dith].n_pi) ao_ow=TRUE;
      break;
    case IRCS_MODE_GR:
      if(!IRCS_dith[hg->ircs_set[i_set].dith].n_gr) ao_ow=TRUE;
      break;
    case IRCS_MODE_PS:
      if(!IRCS_dith[hg->ircs_set[i_set].dith].n_ps) ao_ow=TRUE;
      break;
    case IRCS_MODE_EC:
      if(!IRCS_dith[hg->ircs_set[i_set].dith].n_ec) ao_ow=TRUE;
      break;
    }

    if(ao_ow){
      fprintf(fp, "# !!! DEF_%s%sN does not exist!   Use DEF_%s%sA here!!  !!!#\n",
	      mode_letter,
	      dith_letter,
	      mode_letter,
	      dith_letter);
      if(flag_nst){
	ao_sfx=g_strdup("A COORD=FILE MODE=AOP");
      }
      else{
	ao_sfx=g_strdup("A MODE=AOP");
      }
    }
    else{
      if(flag_nst){
	ao_sfx=g_strdup("N COORD=FILE");
      }
      else{
	ao_sfx=g_strdup("N");
      }
    }
    break;

  default:
    if(flag_nst){
      ao_sfx=g_strdup("A COORD=FILE MODE=AOP");
    }
    else{
      ao_sfx=g_strdup("A MODE=AOP");
    }
    break;
  }

  // Dithering description
  switch(hg->ircs_set[i_set].dith){
  case IRCS_DITH_S5:
  case IRCS_DITH_S9:
  case IRCS_DITH_D5:
  case IRCS_DITH_D9:
  case IRCS_DITH_V3:
  case IRCS_DITH_V5:
  case IRCS_DITH_ABBA:
    dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf DITH=%.1lf",
			 mode_letter,
			 dith_letter,
			 ao_sfx,
			 hg->ircs_set[i_set].exp,
			 hg->ircs_set[i_set].dithw);
    break;
    
  case IRCS_DITH_OSO: 
  case IRCS_DITH_OSSO: 
    dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf DITH=%.1lf RA_OFFSET=%d DEC_OFFSET=%d",
			 mode_letter,
			 dith_letter,
			 ao_sfx,
			 hg->ircs_set[i_set].exp,
			 hg->ircs_set[i_set].dithw,
			 hg->ircs_set[i_set].osra,
			 hg->ircs_set[i_set].osdec);
    
  case IRCS_DITH_SS:
    ss_pos=-hg->ircs_set[i_set].sssep*((gdouble)(hg->ircs_set[i_set].ssnum-1))/2.0;
    ss_tmp1=g_strdup_printf("%.2lf %.3lf 1", ss_pos, hg->ircs_set[i_set].exp);
    for(i_scan=1;i_scan<hg->ircs_set[i_set].ssnum;i_scan++){
      ss_pos+=hg->ircs_set[i_set].sssep;
      ss_tmp2=g_strdup_printf("%s %.2lf %.3lf 1", ss_tmp1, ss_pos, hg->ircs_set[i_set].exp);
      g_free(ss_tmp1);
      ss_tmp1=g_strdup(ss_tmp2);
      g_free(ss_tmp2);
    }
    
    dith=g_strdup_printf("%sSS%s SCAN_PAT=[%s] EXPTIME_SV=0.5",
			 mode_letter,
			 ao_sfx,
			 ss_tmp1);
    g_free(ss_tmp1);
    break;
    
  default:
    dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf ",
			 mode_letter,
			 dith_letter,
			 ao_sfx,
			 hg->ircs_set[i_set].exp);
    break;
  }
  
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
  case IRCS_MODE_PI:
    if(flag_nst){
      fprintf(fp, "GetObject $DEF_%s PIXELSCALE=%dMAS COADDS=1 NDUMMYREAD=0\n",
	      dith,
	      IRCS_mas[hg->ircs_set[i_set].mas]);
    }
    else{
      fprintf(fp, "GetObject $DEF_%s PIXELSCALE=%dMAS COADDS=1 NDUMMYREAD=0\n",
	      dith,
	      IRCS_mas[hg->ircs_set[i_set].mas]);
    }
    break;

  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
  case IRCS_MODE_EC:
    if(flag_nst){
      fprintf(fp, "GetObject $DEF_%s PIXELSCALE=52MAS COADDS=1 NDUMMYREAD=0 %s\n",
	      dith,
	      (hg->obj[i_list].aomode==AOMODE_NO) ? pa_str : " ");
    }
    else{
      fprintf(fp, "GetObject $DEF_%s PIXELSCALE=52MAS COADDS=1 NDUMMYREAD=0 %s\n",
	      dith,
	      (hg->obj[i_list].aomode==AOMODE_NO) ? pa_str : " ");
    }
    break;
  }
  
  if(l_flag){
    fprintf(fp, "SetClock $DEF_IRST SLWCNT=1\n");
  }

  if(tgt) g_free(tgt);
  if(ttgs) g_free(ttgs);
  if(ngs) g_free(ngs);
  if(mode_letter) g_free(mode_letter);
  if(dith_letter) g_free(dith_letter);
  if(slew_to) g_free(slew_to);
  if(pa_str) g_free(pa_str);
  if(ao_sfx) g_free(ao_sfx);
  if(dith) g_free(dith);
  if(ss_pat) g_free(ss_pat);
  if(tmode) g_free(tmode);
}


gint IRCS_check_gs(typHOE *hg){
  gint i_list;
  gint ret=-1;

  for(i_list=0; i_list<hg->i_max; i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_NGS_O:
    case AOMODE_LGS_O:
      if(!hg->obj[i_list].gs.flag){
	ret=i_list;
      }
      break;
    }

    if(ret!=-1) break;
  }

  return(ret);
}


gchar* get_gs_txt(GSpara gs){
  gchar *ret, *src_txt;

  switch(gs.src){
  case FCDB_TYPE_SIMBAD:
    src_txt=g_strdup("SIMBAD V-band");
    break;
  case FCDB_TYPE_GSC:
  case MAGDB_TYPE_GSC:
  case MAGDB_TYPE_IRCS_GSC:
    src_txt=g_strdup("GSC R-band");
    break;
  case FCDB_TYPE_PS1:
    src_txt=g_strdup("PanSTARRS-1 r-band");
    break;
  case FCDB_TYPE_SDSS:
    src_txt=g_strdup("SDSS r-band");
    break;
  case FCDB_TYPE_USNO:
    src_txt=g_strdup("USNO R1-band");
    break;
  case FCDB_TYPE_GAIA:
    src_txt=g_strdup("GAIA G-band");
    break;
  case FCDB_TYPE_KEPLER:
    src_txt=g_strdup("Kepler Kp-band");
    break;
  case FCDB_TYPE_2MASS:
    src_txt=g_strdup("2MASS J-band");
    break;
  default:
    src_txt=g_strdup("(unknown band)");
    break;
  }

  if(gs.mag>90){
    ret=g_strdup_printf("[%s] %s=(unknown mag) / sparation=%.0lfarcsec",
			gs.name, src_txt, gs.sep*60.*60.);
  }
  else{
    ret=g_strdup_printf("[%s] %s=%.1lfmag / sparation=%.0lfarcsec",
			gs.name, src_txt,gs.mag, gs.sep*60.*60.);
  }

  g_free(src_txt);
  return(ret);
}


gchar* get_tgt_mag_txt(typHOE *hg, gint i){
  gchar *ret=NULL, *src=NULL;
  
  if(hg->obj[i].mag>99){
    ret=g_strdup("AO Guide Star = Target : Object magnitude is UNKNOWN");
    return(ret);
  }

  src=get_band_name(hg, i);

  ret=g_strdup_printf("AO Guide Star = Target : %s=%.1lfmag\n",
		      src, hg->obj[i].mag);

  g_free(src);
  return(ret);
}


gchar* ircs_get_filter_name(typHOE *hg, gint i_set){
  gchar *ret=NULL;

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup(IRCS_im52_set[hg->ircs_set[i_set].band].name);
      break;
    case IRCS_MAS_20:
      ret=g_strdup(IRCS_im20_set[hg->ircs_set[i_set].band].name);
      break;
    }
    break;

  case IRCS_MODE_PI:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup(IRCS_pi52_set[hg->ircs_set[i_set].band].name);
      break;
    case IRCS_MAS_20:
      ret=g_strdup(IRCS_pi20_set[hg->ircs_set[i_set].band].name);
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup(IRCS_gr52_set[hg->ircs_set[i_set].band].name);
      break;
    case IRCS_MAS_20:
      ret=g_strdup(IRCS_gr20_set[hg->ircs_set[i_set].band].name);
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup(IRCS_ps52_set[hg->ircs_set[i_set].band].name);
      break;
    case IRCS_MAS_20:
      ret=g_strdup(IRCS_ps20_set[hg->ircs_set[i_set].band].name);
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      ret=g_strdup(IRCS_ecd_set[hg->ircs_set[i_set].band].name);
      break;
    case IRCS_ECM:
      ret=g_strdup(IRCS_ecm_set[hg->ircs_set[i_set].band].name);
      break;
    }
    break;
  }
    
  return(ret);
}


gdouble ircs_get_slit_width(typHOE *hg, gint i_set){
  gdouble ret=0;

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=IRCS_gr_wslit[hg->ircs_set[i_set].slit].width;
      break;
    case IRCS_MAS_20:
      ret=IRCS_gr_nslit[hg->ircs_set[i_set].slit].width;
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=IRCS_ps_wslit[hg->ircs_set[i_set].slit].width;
      break;
    case IRCS_MAS_20:
      ret=IRCS_ps_nslit[hg->ircs_set[i_set].slit].width;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    ret=IRCS_ec_slit[hg->ircs_set[i_set].slit].width;
    break;
  }
    
  return(ret);
}


gchar* ircs_make_setup_txt(typHOE *hg, gint i_set){
  gchar *ret=NULL;
  gchar *filter=NULL;
  gdouble slit;

  filter=ircs_get_filter_name(hg, i_set);
  slit=ircs_get_slit_width(hg, i_set);

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    ret=g_strdup_printf("[Imaging] %dmas filter=\"%s\"",
			IRCS_mas[hg->ircs_set[i_set].mas],
			filter);
    break;
    
  case IRCS_MODE_PI:
    ret=g_strdup_printf("[Pol-Imaging] %dmas filter=\"%s\"",
			IRCS_mas[hg->ircs_set[i_set].mas],
			filter);
    break;

  case IRCS_MODE_GR:
    ret=g_strdup_printf("[Grism] %dmas filter=\"%s\" slit=%.2lf\"",
			IRCS_mas[hg->ircs_set[i_set].mas],
			filter,
			slit);
    break;
    
  case IRCS_MODE_PS:
    ret=g_strdup_printf("[Pol-Grism] %dmas filter=\"%s\" slit=%.2lf\"",
			IRCS_mas[hg->ircs_set[i_set].mas],
			filter,
			slit);
    break;

  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      ret=g_strdup_printf("[Echelle] config=\"%s\" slit=%.2lf\"",
			  filter,
			  slit);
      break;
      
    case IRCS_ECM:
      ret=g_strdup_printf("[Echelle_M] filter=\"%s\" ech=%d xds=%d slit=%.2lf",
			  filter,
			  hg->ircs_set[i_set].ech,
			  hg->ircs_set[i_set].xds,
			  slit);
      break;
    }
    break;
  }

  if(filter) g_free(filter);
  return(ret);
}


gchar* ircs_make_def(typHOE *hg, gint i_set){
  gchar *ret=NULL;

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup_printf("IM%s",
			  IRCS_im52_set[hg->ircs_set[i_set].band].def);
      break;
    case IRCS_MAS_20:
      ret=g_strdup_printf("IM%s",
			  IRCS_im20_set[hg->ircs_set[i_set].band].def);
      break;
    }
    break;

  case IRCS_MODE_PI:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup_printf("PI%s",
			  IRCS_pi52_set[hg->ircs_set[i_set].band].def);
      break;
    case IRCS_MAS_20:
      ret=g_strdup_printf("PI%s",
			  IRCS_pi20_set[hg->ircs_set[i_set].band].def);
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      if(hg->ircs_set[i_set].slit<IRCS_GR_WSLIT_SEP1){
	ret=g_strdup_printf("GR%s_N",
			    IRCS_gr52_set[hg->ircs_set[i_set].band].def);
      }
      else{
	ret=g_strdup_printf("GR%s",
			    IRCS_gr52_set[hg->ircs_set[i_set].band].def);
      }
      break;
    case IRCS_MAS_20:
      if(hg->ircs_set[i_set].slit<IRCS_GR_NSLIT_SEP1){
	ret=g_strdup_printf("GR%s",
			    IRCS_gr20_set[hg->ircs_set[i_set].band].def);
      }
      else{
	ret=g_strdup_printf("GR_USER%02d",i_set+1);
      }
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      ret=g_strdup_printf("PS%s",
			  IRCS_ps52_set[hg->ircs_set[i_set].band].def);
      break;
    case IRCS_MAS_20:
      ret=g_strdup_printf("PS%s",
			  IRCS_ps20_set[hg->ircs_set[i_set].band].def);
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      switch(hg->ircs_set[i_set].slit){
      case IRCS_EC_SLIT_N:
	if(hg->ircs_set[i_set].slw==0){
	  ret=NULL;
	}
	else{
	ret=g_strdup_printf("EC%s_N",
			    IRCS_ecd_set[hg->ircs_set[i_set].band].def);
	}
	break;
	
      case IRCS_EC_SLIT_M:
	if(hg->ircs_set[i_set].slw==0){
	  ret=NULL;
	}
	else{
	  ret=g_strdup_printf("EC%s_M",
			      IRCS_ecd_set[hg->ircs_set[i_set].band].def);
	}
	break;

      case IRCS_EC_SLIT_W:
	if(hg->ircs_set[i_set].slw==0){
	  ret=NULL;
	}
	else{
	  ret=g_strdup_printf("EC%s_W",
			      IRCS_ecd_set[hg->ircs_set[i_set].band].def);
	}
	break;
      }
      break;

    case IRCS_ECM:
      ret=g_strdup_printf("EC_USER%02d",i_set+1);
      break;
    }
    break;
  }
    
  return(ret);
}


void IRCS_WriteLGS(typHOE *hg){
  FILE *fp;
  gint i_list;
  gdouble d_ra, d_dec;
  struct ln_hms hms;
  struct ln_dms dms;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  fprintf(fp,"# LGS Target List\n");
  fprintf(fp,"# RA, Dec, and target name must be separated by tab (not space) .\n");
  fprintf(fp,"#   HH:MM:SS.SSS\t[+/-]DD:MM:SS.SS\tTarget name and additional\n");
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      d_ra=ra_to_deg(hg->obj[i_list].ra);
      d_dec=dec_to_deg(hg->obj[i_list].dec);

      ln_deg_to_hms(d_ra, &hms);
      ln_deg_to_dms(d_dec, &dms);
      
      fprintf(fp,"%02d:%02d:%06.3lf\t%s%02d:%02d:%05.2lf\t%s\n",
	      hms.hours, hms.minutes, hms.seconds,
	      (dms.neg)?"-":"+",
	      dms.degrees, dms.minutes, dms.seconds,
	      hg->obj[i_list].name);

      if((hg->obj[i_list].aomode==AOMODE_LGS_O)&&(hg->obj[i_list].gs.flag)){
	d_ra=ra_to_deg(hg->obj[i_list].gs.ra);
	d_dec=dec_to_deg(hg->obj[i_list].gs.dec);
	
	ln_deg_to_hms(d_ra, &hms);
	ln_deg_to_dms(d_dec, &dms);
	
	fprintf(fp,"%02d:%02d:%06.3lf\t%s%02d:%02d:%05.2lf\t%s TT Guide Star\n",
		hms.hours, hms.minutes, hms.seconds,
		(dms.neg)?"-":"+",
		dms.degrees, dms.minutes, dms.seconds,
		hg->obj[i_list].name);
      }
      break;
    }
  }

  fclose(fp);
}


void IRCS_WriteService(typHOE *hg){
  FILE *fp;
  gint i_list, i_plan;
  gdouble d_ra, d_dec;
  struct ln_hms hms;
  struct ln_dms dms;
  gint oplan[MAX_PLAN], i_oplan=0, i_oplan_max, j_oplan;
  gboolean sv_out[MAX_PLAN];
  gint obj_i, mode, mas, obj_i0, mode0, mas0;
  gchar *fil=NULL, *exp=NULL, *coadds=NULL, *slit=NULL, *dith=NULL, *dithw=NULL;
  gchar *fil_tmp=NULL, *exp_tmp=NULL, *coadds_tmp=NULL, *slit_tmp=NULL,
    *dith_tmp=NULL, *dithw_tmp=NULL;
  gchar *tmp;
  gdouble j_mag, h_mag, k_mag;
  GtkWidget *dialog, *label, *hbox, *spinner;
  GtkAdjustment *adj;

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
      oplan[i_oplan]=i_plan;
      sv_out[i_oplan]=FALSE;
      i_oplan++;
    }
  }

  i_oplan_max=i_oplan;

  if(i_oplan_max==0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "There are no plans to take object frames.",
		  NULL);
    return;
  }

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  
  for(i_oplan=0;i_oplan<i_oplan_max;i_oplan++){
    if(!sv_out[i_oplan]){
      obj_i=hg->plan[oplan[i_oplan]].obj_i;
      mode=hg->ircs_set[hg->plan[oplan[i_oplan]].setup].mode;
      mas=hg->ircs_set[hg->plan[oplan[i_oplan]].setup].mas;
      sv_out[i_plan]=TRUE;

      fil=ircs_get_filter_name(hg, hg->plan[oplan[i_oplan]].setup);
      exp=g_strdup_printf("%.3lf", hg->plan[oplan[i_oplan]].dexp);
      coadds=g_strdup_printf("%d", hg->plan[oplan[i_oplan]].coadds);
      slit=g_strdup_printf("%.2lf", ircs_get_slit_width(hg, hg->plan[oplan[i_oplan]].setup));
      dith=g_strdup_printf("%s x%d",
			   IRCS_dith[hg->plan[oplan[i_oplan]].dith].name,
			   hg->plan[oplan[i_oplan]].repeat);
      dithw=g_strdup_printf("%.3lf", hg->plan[oplan[i_oplan]].dithw);
      
      for(j_oplan=i_oplan+1;j_oplan<i_oplan_max;j_oplan++){
	obj_i0=hg->plan[oplan[j_oplan]].obj_i;
	mode0=hg->ircs_set[hg->plan[oplan[j_oplan]].setup].mode;
	mas0=hg->ircs_set[hg->plan[oplan[j_oplan]].setup].mas;

	if((obj_i==obj_i0)&&(mas==mas0)&&(mode==mode0)){
	  sv_out[j_oplan]=TRUE;

	  fil_tmp=g_strdup(fil);
	  exp_tmp=g_strdup(exp);
	  coadds_tmp=g_strdup(coadds);
	  slit_tmp=g_strdup(slit);
	  dith_tmp=g_strdup(dith);
	  dithw_tmp=g_strdup(dithw);

	  if(fil) g_free(fil);
	  if(exp) g_free(exp);
	  if(coadds) g_free(coadds);
	  if(slit) g_free(slit);
	  if(dith) g_free(dith);
	  if(dithw) g_free(dithw);

	  fil=g_strdup_printf("%s, %s",
			      fil_tmp,
			      ircs_get_filter_name(hg, hg->plan[oplan[j_oplan]].setup));
	  exp=g_strdup_printf("%s, %.3lf",
			      exp_tmp,
			      hg->plan[oplan[j_oplan]].dexp);
	  coadds=g_strdup_printf("%s, %d",
				 coadds_tmp,
				 hg->plan[oplan[j_oplan]].coadds);
	  slit=g_strdup_printf("%s, %.2lf",
			       slit_tmp,
			       ircs_get_slit_width(hg, hg->plan[oplan[j_oplan]].setup));
	  dith=g_strdup_printf("%s, %s x%d",
			       dith_tmp,
			       IRCS_dith[hg->plan[oplan[j_oplan]].dith].name,
			       hg->plan[oplan[j_oplan]].repeat);
	  dithw=g_strdup_printf("%s, %.3lf",
				dithw_tmp,
				hg->plan[oplan[i_oplan]].dithw);

	  if(fil_tmp) g_free(fil_tmp);
	  if(exp_tmp) g_free(exp_tmp);
	  if(coadds_tmp) g_free(coadds_tmp);
	  if(slit_tmp) g_free(slit_tmp);
	  if(dith_tmp) g_free(dith_tmp);
	  if(dithw_tmp) g_free(dithw_tmp);
	}
      }

      fprintf(fp, "1. Object/field name :\n");
      fprintf(fp, "   %s\n", hg->obj[hg->plan[oplan[i_oplan]].obj_i].name);
      fprintf(fp, "2. Right ascension of the target field in format HHMMSS.SSS\n");
      fprintf(fp, "   %09.2lf\n", hg->obj[hg->plan[oplan[i_oplan]].obj_i].ra);
      fprintf(fp, "3. Declination of the target field in format (+/-)DDMMSS.SS\n");
      fprintf(fp, "   %+010.2lf\n", hg->obj[hg->plan[oplan[i_oplan]].obj_i].dec);
      fprintf(fp, "4. Equinox of position in format YYYY.Y\n");
      fprintf(fp, "   %.1lf\n", hg->obj[hg->plan[oplan[i_oplan]].obj_i].equinox);
      fprintf(fp, "5. Are photometric conditions necessary? (Yes/No)\n");
      fprintf(fp, "   Yes\n");     
      fprintf(fp, "6. Observation mode (Imaging/Grism/Echelle) %s\n", ircs_mode_name[mode]);
      switch(hg->obj[hg->plan[oplan[i_oplan]].obj_i].aomode){
      case AOMODE_NO:
	fprintf(fp, "  i. For imaging, do you need AO188? (Yes/No) No\n");
	fprintf(fp, "  ii. NGS, or LGS? --\n");
	break;
	
      case AOMODE_NGS_S:
      case AOMODE_NGS_O:
	fprintf(fp, "  i. For imaging, do you need AO188? (Yes/No) Yes\n");
	fprintf(fp, "  ii. NGS, or LGS? NGS\n");
	break;

      case AOMODE_LGS_S:
      case AOMODE_LGS_O:
	fprintf(fp, "  i. For imaging, do you need AO188? (Yes/No) Yes\n");
	fprintf(fp, "  ii. NGS, or LGS? LGS\n");
	break;
      }
      fprintf(fp, "7. Magnitude/range of magnitudes in a specified NIR band.\n");
      j_mag=hg->obj[hg->plan[oplan[i_oplan]].obj_i].magdb_2mass_j;
      h_mag=hg->obj[hg->plan[oplan[i_oplan]].obj_i].magdb_2mass_h;
      k_mag=hg->obj[hg->plan[oplan[i_oplan]].obj_i].magdb_2mass_k;
      while((j_mag>99)&&(h_mag>99)&&(k_mag>99)){	
	dialog = gtk_dialog_new_with_buttons("HOE : J, H, K-magnitudes for your target",
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

	label = gtk_label_new ("There are no 2MASS magnitudes information for");
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			   label,FALSE, FALSE, 0);

	tmp=g_strdup_printf("   Object-%d : %s",
			    hg->plan[oplan[i_oplan]].obj_i+1,
			    hg->obj[hg->plan[oplan[i_oplan]].obj_i].name);
	label = gtk_label_new (tmp);
	g_free(tmp);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			   label,FALSE, FALSE, 0);

	label = gtk_label_new ("Please input J, H, K-magnitudes for your target.");
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			   label,FALSE, FALSE, 0);

	hbox = gtkut_hbox_new(FALSE,2);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			   hbox,FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

	label = gtk_label_new ("J");
	gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

	adj = (GtkAdjustment *)gtk_adjustment_new(j_mag, 0.0, 100.0,
						  0.1, 1.0, 0);
	my_signal_connect (adj, "value_changed",
			   cc_get_adj_double,
			   &j_mag);
	spinner =  gtk_spin_button_new (adj, 1, 1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				  TRUE);
	my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

	label = gtk_label_new ("  H");
	gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

	adj = (GtkAdjustment *)gtk_adjustment_new(h_mag, 0.0, 100.0,
						  0.1, 1.0, 0);
	my_signal_connect (adj, "value_changed",
			   cc_get_adj_double,
			   &h_mag);
	spinner =  gtk_spin_button_new (adj, 1, 1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				  TRUE);
	my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

	label = gtk_label_new ("  K");
	gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

	adj = (GtkAdjustment *)gtk_adjustment_new(k_mag, 0.0, 100.0,
						  0.1, 1.0, 0);
	my_signal_connect (adj, "value_changed",
			   cc_get_adj_double,
			   &k_mag);
	spinner =  gtk_spin_button_new (adj, 1, 1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				  TRUE);
	my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

	gtk_widget_show_all(dialog);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
	  gtk_widget_destroy(dialog);
	}
      }
      if(j_mag<99){
	fprintf(fp, "   J=%.1lf", j_mag);
      }
      else{
	fprintf(fp, "   J=---");
      }
      if(h_mag<99){
	fprintf(fp, ", H=%.1lf", h_mag);
      }
      else{
	fprintf(fp, ", H=---");
      }
      if(k_mag<99){
	fprintf(fp, ", K=%.1lf\n", k_mag);
      }
      else{
	fprintf(fp, ", K=---\n");
      }
      fprintf(fp, "8. What is the maximum acceptable seeing size (in arcseconds) in the specified NIR band? In case of using AO188, please provide the maximum acceptable FWHM of the point-spread function after the AO correction\n.");
      fprintf(fp, "   0.2 arcseconds\n");
      fprintf(fp, "9. Please describe the AO guide stars as follows.\n");
      fprintf(fp, "  i. R (or, V if not available) magnitude of your AO guide star.\n");
      fprintf(fp, "  ii. Separation between the target and the AO guide star\n");
      switch(hg->obj[hg->plan[oplan[i_oplan]].obj_i].aomode){
      case AOMODE_NO:
	fprintf(fp, "     ---\n");
	break;
	
      case AOMODE_NGS_S:
      case AOMODE_LGS_S:
	tmp=get_tgt_mag_txt(hg, hg->plan[oplan[i_oplan]].obj_i);
	fprintf(fp, "     %s / Separation = 0arcsec\n", tmp);
	g_free(tmp);
	break;

      case AOMODE_NGS_O:
      case AOMODE_LGS_O:
	tmp=get_gs_txt(hg->obj[hg->plan[oplan[i_oplan]].obj_i].gs);
	fprintf(fp, "     %s\n", tmp);
	g_free(tmp);
	break;
      }
      fprintf(fp, "  iii. Whether the AO guide star is a point source or not. If you use an extended object or a star associated with nebulosity, you must describe a FWHM of its spatial distribution or contrast between the star and nebulousity. \n");
      fprintf(fp, "     point source\n");
      fprintf(fp, "10. Filter(s)/Band configuration for observation. If more than one filter is to be used, separate them with commas.\n");
      fprintf(fp, "     %s\n", fil);
      fprintf(fp, "11. Pixel scale of camera detector array for imaging, or Grism spectroscopy (20MAS/52MAS).\n");
      if(mode==IRCS_MODE_EC){
	fprintf(fp, "      N/A\n");
      }
      else{
	fprintf(fp, "      %dmas\n", IRCS_mas[mas]);
      }
      fprintf(fp, "12. Slit width for Grism/Echelle spectroscopy. A value must be given for each band configuration specified in item 10, separated by commas.\n");
      switch(mode){
      case IRCS_MODE_IM:
      case IRCS_MODE_PI:
	fprintf(fp, "      N/A\n");
	break;

      case IRCS_MODE_GR:
      case IRCS_MODE_PS:
      case IRCS_MODE_EC:
	fprintf(fp, "      %s\n", slit);
	break;
      }
      fprintf(fp, "13. Field position angle (PA) for imaging, or slit PA for Grism/Echelle spectroscopy. \n");
      fprintf(fp, "    %.1lf degree\n", hg->plan[oplan[i_oplan]].pa);
      fprintf(fp, "14. Single frame exposure time in seconds.\n");
      fprintf(fp, "     %s\n", exp);
      fprintf(fp, "15. Number of coadds at each dithering position.\n");
      fprintf(fp, "     %s\n", coadds);
      fprintf(fp, "16. Dither patterns.\n");
      fprintf(fp, "     %s\n", dith);
      fprintf(fp, "17. Dithering size in arcseconds (\"DITH\" in the above figures).\n");
      fprintf(fp, "     %s\n", dithw);
      fprintf(fp, "18. Type of target. Use \"OBJ\" for objects, \"STD\" for standards.\n");
      fprintf(fp, "    %s\n", (hg->obj[hg->plan[oplan[i_oplan]].obj_i].std) ? "STD" : "OBJ");
      fprintf(fp, "19. Name of the target, to be written in the OBJECT field of the image FITS header.\n");
      fprintf(fp, "   %s\n", hg->obj[hg->plan[oplan[i_oplan]].obj_i].name);
      fprintf(fp, "\n\n");

      
      if(fil) g_free(fil);
      if(exp) g_free(exp);
      if(coadds) g_free(coadds);
      if(slit) g_free(slit);
      if(dith) g_free(dith);
      if(dithw) g_free(dithw);
    }
  }

  fclose(fp);
}


gchar* ircs_plan_make_dtxt(typHOE *hg, gint i_set, gint dith, gdouble dithw,
			   gint osra, gint osdec, gdouble sssep, gint ssnum){
  gchar *ret=NULL;
  gboolean f_diff=FALSE;

  if(hg->ircs_set[i_set].dith!=dith){
    f_diff=TRUE;
  }
  else{
    switch(dith){
    case IRCS_DITH_S5:
    case IRCS_DITH_S9:
    case IRCS_DITH_D5:
    case IRCS_DITH_D9:
    case IRCS_DITH_V3:
    case IRCS_DITH_V5:
    case IRCS_DITH_ABBA:
      if(fabs(hg->ircs_set[i_set].dithw-dithw)>0.001){
	f_diff=TRUE;
      }
      break;

    case IRCS_DITH_OSO:
    case IRCS_DITH_OSSO:
      if((hg->ircs_set[i_set].osra!=osra)||(hg->ircs_set[i_set].osdec!=osdec)){
	f_diff=TRUE;
      }
      break;

    case IRCS_DITH_SS:
      if((fabs(hg->ircs_set[i_set].sssep-sssep)>0.001)||(hg->ircs_set[i_set].ssnum!=ssnum)){
	f_diff=TRUE;
      }
      break;
    }
  }

  if(f_diff){
    ret=ircs_make_dtxt(dith, dithw, osra, osdec, sssep, ssnum);
  }

  return(ret);
}

gchar* ircs_make_dtxt(gint dith, gdouble dithw, gint osra, gint osdec, gdouble sssep, gint ssnum){
  gchar *ret;

  switch(dith){
  case IRCS_DITH_S5:
  case IRCS_DITH_S9:
  case IRCS_DITH_D5:
  case IRCS_DITH_D9:
  case IRCS_DITH_V3:
  case IRCS_DITH_V5:
  case IRCS_DITH_ABBA:
    ret=g_strdup_printf("[%s] width=%.1lf",
			IRCS_dith[dith].name,
			dithw);
    break;

  case IRCS_DITH_OSO:
  case IRCS_DITH_OSSO:
    ret=g_strdup_printf("[%s] width=%.1lf dRA=%d dDec=%d",
			IRCS_dith[dith].name,
			dithw, osra, osdec);
    break;

  case IRCS_DITH_SS:
    ret=g_strdup_printf("[%s] %.2lfx%d",
			IRCS_dith[dith].name,
			sssep, ssnum);
    break;

  default:
    ret=g_strdup_printf("[%s]",
			IRCS_dith[dith].name);
    break;
  }

  return(ret);
}


gboolean ircs_check_spec(typHOE *hg){
  gboolean ret=FALSE;
  gint i_set;

  for(i_set=0;i_set<hg->ircs_i_max;i_set++){
    switch(hg->ircs_set[i_set].mode){
    case IRCS_MODE_GR:
    case IRCS_MODE_PS:
    case IRCS_MODE_EC:
      ret=TRUE;
      break;
    }
  }

  return(ret);
}

gboolean ircs_check_mode(typHOE *hg, gint mode){
  gboolean ret=FALSE;
  gint i_set;

  for(i_set=0;i_set<hg->ircs_i_max;i_set++){
    if(hg->ircs_set[i_set].mode==mode){
      ret=TRUE;
    }
  }

  return(ret);
}


gboolean ircs_check_lgs(typHOE *hg){
  gboolean ret=FALSE;
  gint i_list;

  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      ret=TRUE;
      break;
    }
  }

  return(ret);
}


gint ircs_get_shot(gint dith, gint ssnum){
  gint ret;
  
  switch(dith){
  case IRCS_DITH_SS:
    ret=ssnum;
    break;
    
  default:
    ret=IRCS_dith[dith].shot;
    break;
  }

  return(ret);
}


gdouble ircs_get_flat_exp(typHOE *hg, gint i_set){
  gdouble exp;
  
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      exp   =IRCS_im52_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    case IRCS_MAS_20:
      exp   =IRCS_im20_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    }
    break;
    
  case IRCS_MODE_PI:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      exp   =IRCS_pi52_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    case IRCS_MAS_20:
      exp   =IRCS_pi20_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      if(IRCS_gr_wslit[hg->ircs_set[i_set].slit].slw==4){
	exp   =IRCS_gr52_set[hg->ircs_set[i_set].band].n_flat_exp;
      }
      else{
	exp   =IRCS_gr52_set[hg->ircs_set[i_set].band].w_flat_exp;
      }
      break;
    case IRCS_MAS_20:
      if(IRCS_gr_nslit[hg->ircs_set[i_set].slit].slw==4){
	exp   =IRCS_gr20_set[hg->ircs_set[i_set].band].n_flat_exp;
      }
      else{
	exp   =IRCS_gr20_set[hg->ircs_set[i_set].band].w_flat_exp;
      }
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      exp   =IRCS_ps52_set[hg->ircs_set[i_set].band].n_flat_exp;
      break;
    case IRCS_MAS_20:
      exp   =IRCS_ps20_set[hg->ircs_set[i_set].band].n_flat_exp;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      exp   =IRCS_ecd_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    case IRCS_ECM:
      exp   =IRCS_ecm_set[hg->ircs_set[i_set].band].flat_exp;
      break;
    }
    break;
  }

  return(exp);
}


gdouble ircs_get_flat_amp(typHOE *hg, gint i_set){
  gdouble amp;
  
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      amp   =IRCS_im52_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    case IRCS_MAS_20:
      amp   =IRCS_im20_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    }
    break;
    
  case IRCS_MODE_PI:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      amp   =IRCS_pi52_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    case IRCS_MAS_20:
      amp   =IRCS_pi20_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      if(IRCS_gr_wslit[hg->ircs_set[i_set].slit].slw==4){
	amp   =IRCS_gr52_set[hg->ircs_set[i_set].band].n_flat_amp;
      }
      else{
	amp   =IRCS_gr52_set[hg->ircs_set[i_set].band].w_flat_amp;
      }
      break;
    case IRCS_MAS_20:
      if(IRCS_gr_nslit[hg->ircs_set[i_set].slit].slw==4){
	amp   =IRCS_gr20_set[hg->ircs_set[i_set].band].n_flat_amp;
      }
      else{
	amp   =IRCS_gr20_set[hg->ircs_set[i_set].band].w_flat_amp;
      }
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      amp   =IRCS_ps52_set[hg->ircs_set[i_set].band].n_flat_amp;
      break;
    case IRCS_MAS_20:
      amp   =IRCS_ps20_set[hg->ircs_set[i_set].band].n_flat_amp;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      amp   =IRCS_ecd_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    case IRCS_ECM:
      amp   =IRCS_ecm_set[hg->ircs_set[i_set].band].flat_amp;
      break;
    }
    break;
  }

  return(amp);
}


gint ircs_get_flat_nd(typHOE *hg, gint i_set){
  gint nd;
  
  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_IM:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      nd   =IRCS_im52_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    case IRCS_MAS_20:
      nd   =IRCS_im20_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    }
    break;
    
  case IRCS_MODE_PI:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      nd   =IRCS_pi52_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    case IRCS_MAS_20:
      nd   =IRCS_pi20_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    }
    break;
    
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      if(IRCS_gr_wslit[hg->ircs_set[i_set].slit].slw==4){
	nd   =IRCS_gr52_set[hg->ircs_set[i_set].band].n_flat_nd;
      }
      else{
	nd   =IRCS_gr52_set[hg->ircs_set[i_set].band].w_flat_nd;
      }
      break;
    case IRCS_MAS_20:
      if(IRCS_gr_nslit[hg->ircs_set[i_set].slit].slw==4){
	nd   =IRCS_gr20_set[hg->ircs_set[i_set].band].n_flat_nd;
      }
      else{
	nd   =IRCS_gr20_set[hg->ircs_set[i_set].band].w_flat_nd;
      }
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      nd   =IRCS_ps52_set[hg->ircs_set[i_set].band].n_flat_nd;
      break;
    case IRCS_MAS_20:
      nd   =IRCS_ps20_set[hg->ircs_set[i_set].band].n_flat_nd;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      nd   =IRCS_ecd_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    case IRCS_ECM:
      nd   =IRCS_ecm_set[hg->ircs_set[i_set].band].flat_nd;
      break;
    }
    break;
  }

  return(nd);
}

 
gdouble ircs_get_comp_exp(typHOE *hg, gint i_set){
  gdouble exp;

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      exp   =IRCS_gr52_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    case IRCS_MAS_20:
      exp   =IRCS_gr20_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      exp   =IRCS_ps52_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    case IRCS_MAS_20:
      exp   =IRCS_ps20_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      exp   =IRCS_ecd_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    case IRCS_ECM:
      exp   =IRCS_ecm_set[hg->ircs_set[i_set].band].comp_exp;
      break;
    }
    break;
  }

  return(exp);
}


gint ircs_get_comp_coadds(typHOE *hg, gint i_set){
  gint coadds;

  switch(hg->ircs_set[i_set].mode){
  case IRCS_MODE_GR:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      coadds=IRCS_gr52_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    case IRCS_MAS_20:
      coadds=IRCS_gr20_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    }
    break;
    
  case IRCS_MODE_PS:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_MAS_52:
      coadds=IRCS_ps52_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    case IRCS_MAS_20:
      coadds=IRCS_ps20_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    }
    break;
    
  case IRCS_MODE_EC:
    switch(hg->ircs_set[i_set].mas){
    case IRCS_ECD:
      coadds=IRCS_ecd_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    case IRCS_ECM:
      coadds=IRCS_ecm_set[hg->ircs_set[i_set].band].comp_coadds;
      break;
    }
    break;
  }

  return(coadds);
}



 
void IRCS_WriteOPE_FOCUS_plan(FILE *fp, PLANpara plan){
  gdouble z=0.0;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  switch(plan.focus_mode){
  case PLAN_FOCUS1:
    fprintf(fp, "# [Launcher/Telescope2] Move Telescope Focus  z=0.0\n");
    fprintf(fp, "FocusOBE_WO_AG $DEF_IRST $DEF_IMKP EXPTIME=0.5 COADDS=5 Z=!TSCL.Z DELTAZ=0.07\n");
    break;

  case PLAN_FOCUS2:
    fprintf(fp, "# [Launcher/AO188(Common)] [ImR] (Free)\n");
    fprintf(fp, "# [Launcher/AO188(Common)] [ImR] (Slew) (Angle=90)\n");
    fprintf(fp, "# Centering of LGS\n");
    fprintf(fp, "AO188_M2CNT $DEF_AOST MODE=LGS INS=IRCS IMR=DEFAULT LGSH=DEFAULT MODECHANGE=YES\n");
    fprintf(fp, "# LGS Collimation\n");
    fprintf(fp, "AO188_LGSCOL $DEF_AOST EXPTIME=10 MODE=MANUAL\n");
    fprintf(fp, "# [Launcher/AO188(LGS)] [Set LGS Height] (CURRENT)\n");
  }
  fprintf(fp,"\n");
}

 
void IRCS_WriteOPE_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set, i, len;
  gdouble exp;
  gint coadds;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  len=strlen("###### %s #####")-2+strlen(plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "# ==> Insert NsIR CAL probe (may be from TWS)\n");
  fprintf(fp, "SETUPCAL $DEF_IRST\n");
  fprintf(fp, "# Turn ON Rare Gas Lamp (may be from TWS)\n");
  fprintf(fp, "SETUPAR $DEF_IRST\n");
  if(plan.cal_mode==-1){
    exp   = ircs_get_comp_exp   (hg, plan.setup);
    coadds= ircs_get_comp_coadds(hg, plan.setup);

    fprintf(fp, "\n");
    fprintf(fp, "# Comparison : ARC Lamp ON\n");
    fprintf(fp, "CHECKCOMPARISON $DEF_%sSTN $DEF_%s EXPTIME=%.2lf\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp);
    fprintf(fp, "GETCOMPARISON $DEF_%sSTN OBJECT=\"%s_ARC_ON\" EXPTIME=%.2lf NDR=16 COADDS=%d REPEATS=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp,
	    coadds,
	    plan.repeat);
    fprintf(fp, "\n");
    fprintf(fp, "# Turn OFF Rare Gas Lamp (may be from TWS)\n");
    fprintf(fp, "SHUTDOWNAR $DEF_IRST\n");
    fprintf(fp, "# Comparison : ARC Lamp OFF\n");
    fprintf(fp, "CHECKCOMPARISON $DEF_%sSTN $DEF_%s EXPTIME=%.2lf\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp);
    fprintf(fp, "GETCOMPARISON $DEF_%sSTN OBJECT=\"%s_ARC_OFF\" EXPTIME=%.2lf NDR=16 COADDS=%d REPEATS=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp,
	    coadds,
	    plan.repeat);
    fprintf(fp, "\n");
  }
  else{
    for(i_set=0;i_set<hg->ircs_i_max;i_set++){
      if(plan.cal_mode==hg->ircs_set[i_set].mode){
	exp   = ircs_get_comp_exp   (hg, i_set);
	coadds= ircs_get_comp_coadds(hg, i_set);
	
	len=strlen("### for Setup-%02d %s ###")-4+strlen(hg->ircs_set[i_set].txt);
	for(i=0;i<len;i++){
	  fprintf(fp, "#");
	}
	fprintf(fp, "\n");
	fprintf(fp, "### for Setup-%02d %s ###\n", i_set+1, hg->ircs_set[i_set].txt);
	for(i=0;i<len;i++){
	  fprintf(fp, "#");
	}
	fprintf(fp, "\n");
	fprintf(fp, "# Comparison : ARC Lamp ON\n");
	fprintf(fp, "CHECKCOMPARISON $DEF_%sSTN $DEF_%s EXPTIME=%.2lf\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp);
	fprintf(fp, "GETCOMPARISON $DEF_%sSTN OBJECT=\"%s_ARC_ON\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp,
		plan.repeat);
	fprintf(fp, "\n");
	fprintf(fp, "# Turn OFF Rare Gas Lamp (may be from TWS)\n");
	fprintf(fp, "SHUTDOWNAR $DEF_IRST\n");
	fprintf(fp, "# Comparison : ARC Lamp OFF\n");
	fprintf(fp, "CHECKCOMPARISON $DEF_%sSTN $DEF_%s EXPTIME=%.2lf\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp);
	fprintf(fp, "GETCOMPARISON $DEF_%sSTN OBJECT=\"%s_ARC_OFF\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp,
		plan.repeat);
	fprintf(fp, "\n");
      }
    }
  }
  fprintf(fp, "# <== Retract and Shutdown CAL probe (may be from TWS)\n");
  fprintf(fp, "SHUTDOWNCAL $DEF_IRST\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void IRCS_WriteOPE_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set, i, len;
  gdouble exp;
  gint nd;
  gdouble amp;
  gint coadds=1;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  len=strlen("###### %s #####")-2+strlen(plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "# ==> Insert NsIR CAL probe (may be from TWS)\n");
  fprintf(fp, "SETUPCAL $DEF_IRST\n");
  if(plan.cal_mode==-1){
    exp = ircs_get_flat_exp(hg, plan.setup);
    amp = ircs_get_flat_amp(hg, plan.setup);
    nd  = ircs_get_flat_nd (hg, plan.setup);

    fprintf(fp, "# Turn ON HAL Lamp (may be from TWS)\n");
    fprintf(fp, "SETUPHAL $DEF_IRST AMP=%.3lf CALND=%d\n", amp, nd);
    fprintf(fp, "\n");
    fprintf(fp, "# Flat : HAL Lamp ON\n");
    fprintf(fp, "CHECKFLAT $DEF_%sSTN $DEF_%s EXPTIME=%.2lf AMP=%.3lf CALND=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp, amp, nd);
    fprintf(fp, "GETFLAT $DEF_%sSTN OBJECT=\"%s_ON\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp,
	    plan.repeat);
    fprintf(fp, "\n");
    fprintf(fp, "# Turn OFF HAL Lamp (may be from TWS)\n");
    fprintf(fp, "SHUTDOWNHAL $DEF_IRST\n");
    fprintf(fp, "# Flat : HAL Lamp OFF\n");
    fprintf(fp, "CHECKFLAT $DEF_%sSTN $DEF_%s EXPTIME=%.2lf CALND=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp, nd);
    fprintf(fp, "GETFLAT $DEF_%sSTN OBJECT=\"%s_OFF\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
	    ircs_mode_initial[hg->ircs_set[plan.setup].mode],
	    hg->ircs_set[plan.setup].def,
	    exp,
	    plan.repeat);
    fprintf(fp, "\n");
  }
  else{
    for(i_set=0;i_set<hg->ircs_i_max;i_set++){
      if(plan.cal_mode==hg->ircs_set[i_set].mode){
	exp = ircs_get_flat_exp(hg, i_set);
	amp = ircs_get_flat_amp(hg, i_set);
	nd  = ircs_get_flat_nd (hg, i_set);

	len=strlen("### for Setup-%02d %s ###")-4+strlen(hg->ircs_set[i_set].txt);
	for(i=0;i<len;i++){
	  fprintf(fp, "#");
	}
	fprintf(fp, "\n");
	fprintf(fp, "### for Setup-%02d %s ###\n", i_set+1, hg->ircs_set[i_set].txt);
	for(i=0;i<len;i++){
	  fprintf(fp, "#");
	}
	fprintf(fp, "\n");
	fprintf(fp, "# Turn ON HAL Lamp (may be from TWS)\n");
	fprintf(fp, "SETUPHAL $DEF_IRST AMP=%.3lf CALND=%d\n", amp, nd);
	fprintf(fp, "\n");
	fprintf(fp, "# Flat : HAL Lamp ON\n");
	fprintf(fp, "CHECKFLAT $DEF_%sSTN $DEF_%s EXPTIME=%.2lf AMP=%.3lf CALND=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp, amp, nd);
	fprintf(fp, "GETFLAT $DEF_%sSTN OBJECT=\"%s_ON\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp,
		plan.repeat);
	fprintf(fp, "\n");
	fprintf(fp, "# Turn OFF HAL Lamp (may be from TWS)\n");
	fprintf(fp, "SHUTDOWNHAL $DEF_IRST\n");
	fprintf(fp, "# Flat : HAL Lamp OFF\n");
	fprintf(fp, "CHECKFLAT $DEF_%sSTN $DEF_%s EXPTIME=%.2lf CALND=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp, nd);
	fprintf(fp, "GETFLAT $DEF_%sSTN OBJECT=\"%s_OFF\" EXPTIME=%.2lf NDR=16 COADDS=1 REPEATS=%d\n",
		ircs_mode_initial[hg->ircs_set[i_set].mode],
		hg->ircs_set[i_set].def,
		exp,
		plan.repeat);
	fprintf(fp, "\n");
      }
    }
  }
  fprintf(fp, "# <== Retract and Shutdown CAL probe (may be from TWS)\n");
  fprintf(fp, "SHUTDOWNCAL $DEF_IRST\n");

  fprintf(fp, "\n");
  fprintf(fp, "\n");
}
 

void IRCS_WriteOPE_OBJ_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_repeat, i_set;
  gchar *tgt=NULL, *ttgs=NULL, *ngs=NULL;
  gboolean flag_nst=FALSE;
  gchar *mode_letter=NULL, *dith_letter=NULL;
  gchar *slew_to=NULL, *pa_str=NULL, *dith=NULL, *ao_sfx=NULL,
    *ss_pat=NULL, *ss_tmp1=NULL, *ss_tmp2=NULL, *tmode=NULL;
  gdouble d_ra, d_dec, d_x, d_y;
  gdouble ss_pos;
  gint i_scan;
  gchar *gs_txt;
  gboolean ao_ow=FALSE;
  gchar *head[5], *dtxt=NULL, *tmp=NULL;
  gint len[5], len_max=0, i, j;
  gboolean l_flag=FALSE;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");

  for(i=0;i<5;i++){
    head[i]=NULL;
    len[i]=0;
  }

  // Comment line
  switch(plan.aomode){
  case AOMODE_NGS_S:
    tmp=g_strdup("(NGS by target)");
    break;
    
  case AOMODE_NGS_O:
    tmp=g_strdup("(NGS by offset star)");
    break;
    
  case AOMODE_LGS_S:
    tmp=g_strdup("(LGS : TT Guide Star = Target)");
    break;
    
  case AOMODE_LGS_O:
    tmp=g_strdup("(LGS : Offset TTGS)"); 
    break;

  default:
    tmp=g_strdup("(w/o AO)");
    break;
  }

  switch(hg->ircs_set[plan.setup].mode){
  case IRCS_MODE_IM:
    pa_str=g_strdup_printf("FIELD_PA=%.2lf",plan.pa);
    switch(hg->ircs_set[plan.setup].mas){
    case IRCS_MAS_52:
      switch(hg->ircs_set[plan.setup].band){
      case IRCS_IM52_LP:
      case IRCS_IM52_LPN:
      case IRCS_IM52_H2O:
      case IRCS_IM52_H3P:
      case IRCS_IM52_BRA:
      case IRCS_IM52_H2ON:
      case IRCS_IM52_BRACN:
      case IRCS_IM52_BRAN:
	l_flag=TRUE;
	break;
      }
      break;
      
    case IRCS_MAS_20:
      switch(hg->ircs_set[plan.setup].band){
      case IRCS_IM20_LP:
      case IRCS_IM20_LPN:
      case IRCS_IM20_H2O:
      case IRCS_IM20_BRAC:
      case IRCS_IM20_BRA:
      case IRCS_IM20_H2ON:
      case IRCS_IM20_BRACN:
      case IRCS_IM20_BRAN:
	l_flag=TRUE;
	break;
      }
      break;
    }
    break;

  case IRCS_MODE_PI:
    pa_str=g_strdup_printf("FIELD_PA=%.2lf",plan.pa);
    switch(hg->ircs_set[plan.setup].mas){
    case IRCS_MAS_52:
      switch(hg->ircs_set[plan.setup].band){
      case IRCS_PI52_LP:
      case IRCS_PI52_LPN:
	l_flag=TRUE;
	break;
      }
      break;
      
    case IRCS_MAS_20:
      switch(hg->ircs_set[plan.setup].band){
      case IRCS_PI20_LP:
      case IRCS_PI20_LPN:
	l_flag=TRUE;
	break;
      }
      break;
    }
    break;
    
  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
  case IRCS_MODE_EC:
    pa_str=g_strdup_printf("SLIT_PA=%.2lf",plan.pa);
    break;
  }


  switch(plan.omode){
  case PLAN_OMODE_GET:
    head[0]=g_strdup_printf("# GetObject for Object-%d : \"%s\"  %s  ",
			    plan.obj_i+1,
			    hg->obj[plan.obj_i].name,
			    tmp);
    break;
  case PLAN_OMODE_SET:
    head[0]=g_strdup_printf("# SetupField for Object-%d : \"%s\"  %s  ",
			    plan.obj_i+1,
			    hg->obj[plan.obj_i].name,
			    tmp);
    break;
  default:
    head[0]=g_strdup_printf("# SetupField--GetObject for Object-%d : \"%s\"  %s  ",
			    plan.obj_i+1,
			    hg->obj[plan.obj_i].name,
			    tmp);
    break;
  }
  if(tmp) g_free(tmp);

  if(hg->obj[plan.obj_i].note){
    head[1]=g_strdup_printf("#      %s  ", hg->obj[plan.obj_i].note);
  }

  head[2]=g_strdup_printf("#  Setup-%02d : %s  ",
			  plan.setup+1,
			  hg->ircs_set[plan.setup].txt);
  
  dtxt=ircs_make_dtxt(plan.dith, plan.dithw, plan.osra, plan.osdec, plan.sssep, plan.ssnum);
  if(dtxt){
    head[3]=g_strdup_printf("#  Dithering : %s  ", dtxt);
    g_free(dtxt);
  }

  if(plan.omode!=PLAN_OMODE_SET){
    head[4]=g_strdup_printf("#    %.3lf[s]  COADDS=%d NDR=%d x%d  %s ",
			    plan.dexp,
			    plan.coadds,
			    plan.ndr,
			    plan.repeat,
			    pa_str);
  }
  
  for(i=0;i<5;i++){
    if(head[i]) len[i]=strlen(head[i]);
    if(len[i]>len_max) len_max=len[i];
  }

  for(i=0;i<len_max+1;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  for(i=0;i<5;i++){
    if(head[i]){
      fprintf(fp, "%s", head[i]);
      g_free(head[i]);

      for(j=len[i];j<len_max;j++){
	fprintf(fp, " ");
      }
      fprintf(fp, "#\n");
    }
  }
  for(i=0;i<len_max+1;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  
  
  mode_letter=g_strdup(ircs_mode_initial[hg->ircs_set[plan.setup].mode]);
  dith_letter=g_strdup(IRCS_dith[plan.dith].def);


  ///// prep for SetupField /////
  if(hg->obj[plan.obj_i].i_nst>=0){ // Non-Sidereal
    tgt=g_strdup_printf("OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
			hg->obj[plan.obj_i].name,
			g_path_get_basename(hg->nst[hg->obj[plan.obj_i].i_nst].filename));
    flag_nst=TRUE;
    tmode=g_strdup("TMODE=\"NON-SID\"");
  }
  else{
    tgt=make_tgt(hg->obj[plan.obj_i].name, "$TGT_");
    if(hg->obj[plan.obj_i].aomode==AOMODE_LGS_O){
      ttgs=make_tgt(hg->obj[plan.obj_i].name, "$TTGS_");
    }
    else if(hg->obj[plan.obj_i].aomode==AOMODE_NGS_O){
      ngs=make_tgt(hg->obj[plan.obj_i].name, "$NGS_");
    }

    if(plan.adi){
      tmode=g_strdup("TMODE=ADI");
    }
    else{
      tmode=g_strdup("TMODE=SID");
    }
  }

  switch(hg->obj[plan.obj_i].aomode){
  case AOMODE_NGS_S:
    slew_to=g_strdup(tgt);
    break;
    
  case AOMODE_NGS_O:
    gs_txt=get_gs_txt(hg->obj[plan.obj_i].gs);
    fprintf(fp, "# Slew to offset NGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ngs);
    break;
    
  case AOMODE_LGS_S:
    slew_to=g_strdup(tgt);
    break;
    
  case AOMODE_LGS_O:
    gs_txt=get_gs_txt(hg->obj[plan.obj_i].gs);
    fprintf(fp, "# Slew to offset TTGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ttgs);
    break;

  default:
    slew_to=g_strdup(tgt);
    break;
  }
  
  
  ////// Commands from HERE!!
  if(l_flag){
    fprintf(fp, "SetClock $DEF_IRST SLWCNT=1\n");
  }
  
  if(plan.omode!=PLAN_OMODE_GET){
    //// SetupField ////
    switch(hg->ircs_set[plan.setup].mode){
    case IRCS_MODE_IM:
    case IRCS_MODE_PI:
    case IRCS_MODE_GR:
    case IRCS_MODE_PS:
      fprintf(fp, "SetupField $DEF_%sSTA $DEF_IMK %s %s %s\n",
	      mode_letter, slew_to, pa_str, tmode);
      break;
    case IRCS_MODE_EC:
      fprintf(fp, "SetupField $DEF_%sSTA $DEF_%s $DEF_IMK %s %s %s\n",
	      mode_letter, hg->ircs_set[plan.setup].def, slew_to, pa_str, tmode);
      break;
    }

    // Offset for Guide Star
    switch(hg->obj[plan.obj_i].aomode){
    case AOMODE_LGS_O:
      fprintf(fp, "AO188_OFFSET_RADEC $DEF_AOLN %s %s\n",
	      tgt, pa_str);
      break;
    
    case AOMODE_NGS_O:
      d_ra=(ra_to_deg(hg->obj[plan.obj_i].ra)-ra_to_deg(hg->obj[plan.obj_i].gs.ra))*3600.0;
      d_dec=(dec_to_deg(hg->obj[plan.obj_i].dec)-dec_to_deg(hg->obj[plan.obj_i].gs.dec))*3600.0;
      fprintf(fp, "# Add Offset to the target from the offset NGS (dRA, dDec)=(%.2lf,%.2lf)\n",
	      d_ra, d_dec);
      break;
    }
  }

  // CheckField
  switch(hg->ircs_set[plan.setup].mode){
  case IRCS_MODE_IM:
  case IRCS_MODE_PI:
  case IRCS_MODE_GR:
  case IRCS_MODE_PS:
    fprintf(fp, "CheckField $DEF_%sSTA $DEF_IMK EXPTIME=10\n",
	    mode_letter);
    break;
  case IRCS_MODE_EC:
    fprintf(fp, "CheckField $DEF_%sSTA $DEF_%s EXPTIME=10\n",
	    mode_letter, hg->ircs_set[plan.setup].def);
    break;
  }
  
  fprintf(fp, "# Move Target using OBCP --> (%.1lf, %.1lf) \n",
	  hg->ircs_set[plan.setup].slit_x, hg->ircs_set[plan.setup].slit_y);


  if(plan.omode!=PLAN_OMODE_SET){
    //// GetObject ////
    switch(plan.aomode){
    case AOMODE_NO:
      switch(hg->ircs_set[plan.setup].mode){
      case IRCS_MODE_IM:
	if(!IRCS_dith[plan.dith].n_im) ao_ow=TRUE;
	break;
      case IRCS_MODE_PI:
	if(!IRCS_dith[plan.dith].n_pi) ao_ow=TRUE;
	break;
      case IRCS_MODE_GR:
	if(!IRCS_dith[plan.dith].n_gr) ao_ow=TRUE;
	break;
      case IRCS_MODE_PS:
	if(!IRCS_dith[plan.dith].n_ps) ao_ow=TRUE;
	break;
      case IRCS_MODE_EC:
	if(!IRCS_dith[plan.dith].n_ec) ao_ow=TRUE;
	break;
      }

      if(ao_ow){
	fprintf(fp, "# !!! DEF_%s%sN does not exist!   Use DEF_%s%sA here!!  !!!#\n",
		mode_letter,
		dith_letter,
		mode_letter,
		dith_letter);
	if(flag_nst){
	  ao_sfx=g_strdup("A COORD=FILE MODE=AOP TMODE=\"NON-SID\"");
	}
	else{
	  ao_sfx=g_strdup("A MODE=AOP");
	}
      }
      else{
	if(flag_nst){
	  ao_sfx=g_strdup("N COORD=FILE");
	}
	else{
	  ao_sfx=g_strdup("N");
	}
      }
      break;

    default:
      if(flag_nst){
	ao_sfx=g_strdup("A COORD=FILE MODE=AOP TMODE=\"NON-SID\"");
      }
      else{
	ao_sfx=g_strdup("A MODE=AOP");
      }
      break;
    }

    // Dithering description
    switch(plan.dith){
    case IRCS_DITH_S5:
    case IRCS_DITH_S9:
    case IRCS_DITH_D5:
    case IRCS_DITH_D9:
    case IRCS_DITH_V3:
    case IRCS_DITH_V5:
    case IRCS_DITH_ABBA:
      dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf DITH=%.1lf",
			   mode_letter,
			   dith_letter,
			   ao_sfx,
			   plan.dexp,
			   plan.dithw);
      break;
    
    case IRCS_DITH_OSO: 
    case IRCS_DITH_OSSO: 
      dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf DITH=%.1lf RA_OFFSET=%d DEC_OFFSET=%d",
			   mode_letter,
			   dith_letter,
			   ao_sfx,
			   plan.dexp,
			   plan.dithw,
			   plan.osra,
			   plan.osdec);
    
    case IRCS_DITH_SS:
      ss_pos=-plan.sssep*((gdouble)(plan.ssnum-1))/2.0;
      ss_tmp1=g_strdup_printf("%.2lf %.3lf 1", ss_pos, plan.dexp);
      for(i_scan=1;i_scan<plan.ssnum;i_scan++){
	ss_pos+=plan.sssep;
	ss_tmp2=g_strdup_printf("%s %.2lf %.3lf 1", ss_tmp1, ss_pos, plan.dexp);
	g_free(ss_tmp1);
	ss_tmp1=g_strdup(ss_tmp2);
	g_free(ss_tmp2);
      }
    
      dith=g_strdup_printf("%sSS%s SCAN_PAT=[%s] EXPTIME_SV=0.5",
			   mode_letter,
			   ao_sfx,
			   ss_tmp1);
      g_free(ss_tmp1);
      break;
      
    default:
      dith=g_strdup_printf("%s%s%s EXPTIME=%.3lf ",
			   mode_letter,
			   dith_letter,
			   ao_sfx,
			   plan.dexp);
      break;
    }

    for(i_repeat=0;i_repeat<plan.repeat;i_repeat++){
      switch(hg->ircs_set[plan.setup].mode){
      case IRCS_MODE_IM:
      case IRCS_MODE_PI:
	if(flag_nst){
	  fprintf(fp, "GetObject $DEF_%s PIXELSCALE=%dMAS COADDS=%d NDR=%d NDUMMYREAD=0\n",
		  dith,
		  IRCS_mas[hg->ircs_set[plan.setup].mas],
		  plan.coadds, plan.ndr);
	}
	else{
	  fprintf(fp, "GetObject $DEF_%s PIXELSCALE=%dMAS COADDS=%d NDR=%d NDUMMYREAD=0\n",
		  dith,
		  IRCS_mas[hg->ircs_set[plan.setup].mas],
		  plan.coadds, plan.ndr);
	}
	break;
	
      case IRCS_MODE_GR:
      case IRCS_MODE_PS:
      case IRCS_MODE_EC:
	if(flag_nst){
	  fprintf(fp, "GetObject $DEF_%s PIXELSCALE=52MAS COADDS=%d NDR=%d NDUMMYREAD=0 %s\n",
		  dith, plan.coadds, plan.ndr,
		  (plan.aomode==AOMODE_NO) ? pa_str : " ");
	}
	else{
	  fprintf(fp, "GetObject $DEF_%s PIXELSCALE=52MAS COADDS=%d NDR=%d NDUMMYREAD=0 %s\n",
		  dith, plan.coadds, plan.ndr,
		  (plan.aomode==AOMODE_NO) ? pa_str : " ");
	}
	break;
      }
    }
  }
    
  if(l_flag){
    fprintf(fp, "SetClock $DEF_IRST SLWCNT=16\n");
  }
  
  if(tgt) g_free(tgt);
  if(ttgs) g_free(ttgs);
  if(ngs) g_free(ngs);
  if(mode_letter) g_free(mode_letter);
  if(dith_letter) g_free(dith_letter);
  if(slew_to) g_free(slew_to);
  if(pa_str) g_free(pa_str);
  if(ao_sfx) g_free(ao_sfx);
  if(dith) g_free(dith);
  if(ss_pat) g_free(ss_pat);
  if(tmode) g_free(tmode);

  fprintf(fp, "\n");
}


gint ircs_get_1obj_time(OBJpara obj, IRCSpara set){
  gint ret_time;
  
  switch(obj.aomode){
  case AOMODE_NO:
    ret_time
      =(gint)(IRCS_TIME_ACQ
	      +(set.exp+IRCS_TIME_READOUT_NORMAL*IRCS_DEF_NDR+IRCS_TIME_FITS)
	      *(gdouble)ircs_get_shot(set.dith, set.ssnum));
    break;
    
  default:
    ret_time
      =(gint)(IRCS_TIME_ACQ+IRCS_TIME_AO_ACQ
	      +(set.exp+IRCS_TIME_READOUT_NORMAL*IRCS_DEF_NDR+IRCS_TIME_FITS)
	      *(gdouble)ircs_get_shot(set.dith, set.ssnum));
    break;
  }

  return(ret_time);
}


void up_item_ircs_tree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->ircs_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->ircs_tree));


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_IRCS_NUMBER, &i, -1);
    i--;

    if(i>0){
      ircs_swap_setup(&hg->ircs_set[i-1], &hg->ircs_set[i]);
      
      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
      ircs_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i-1);
      ircs_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
      
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
    ircs_update_tree(hg);
  }
}


void down_item_ircs_tree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->ircs_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->ircs_tree));


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_IRCS_NUMBER, &i, -1);
    i--;

    if(i<hg->ircs_i_max-1){
      ircs_swap_setup(&hg->ircs_set[i], &hg->ircs_set[i+1]);
      
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
    ircs_update_tree(hg);
  }
}

