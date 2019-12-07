//    HOE : Subaru HDS++ OPE file Editor
//        hds.c        IO for HDS OPE files
//                                           2019.01.03  A.Tajitsu

#include "main.h"

//////////////////////////////////////////////////////////////////
////////////////  TABs creation for make_note()
//////////////////////////////////////////////////////////////////

///  TAB for HDS setups
void HDS_TAB_create(typHOE *hg) {
  GtkWidget *scrwin;
  GtkWidget *frame;
  GtkWidget *table, *table1;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *combo, *combo0, *fil1_combo, *fil2_combo;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  GtkWidget *button;
  gchar *tmp;
  GtkTooltip *tooltip;
  confSetup *cdata[MAX_USESETUP];
  
  hg->setup_scrwin = gtk_scrolled_window_new (NULL, NULL);
  table = gtkut_table_new(3, 6, FALSE, 0, 0, 0);
  
  gtk_container_set_border_width (GTK_CONTAINER (hg->setup_scrwin), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->setup_scrwin),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(hg->setup_scrwin),
				    GTK_CORNER_BOTTOM_LEFT);
#ifdef USE_GTK3
  gtk_container_add(GTK_CONTAINER(hg->setup_scrwin),table);
#else
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(hg->setup_scrwin),table);
#endif
  gtk_widget_set_size_request(hg->setup_scrwin, -1, 480);  
  
  
  
  // CamZ
  frame = gtkut_frame_new ("<b>Format Adjustments</b>");
  gtkut_table_attach(table, frame, 2, 3, 0, 3,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtkut_label_new ("Camera Z [&#xB5;m]  Blue");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Red");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("dCross");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);


  hg->camz_b_adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_b,
						       -500, -200, 
						       1.0, 10.0, 0);
  my_signal_connect (hg->camz_b_adj, "value_changed",
		     cc_get_adj,
		     &hg->camz_b);
  spinner =  gtk_spin_button_new (hg->camz_b_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  hg->camz_r_adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_r,
						       -500, -200, 
						       1.0, 10.0, 0);
  my_signal_connect (hg->camz_r_adj, "value_changed",
		     cc_get_adj,
		     &hg->camz_r);
  spinner =  gtk_spin_button_new (hg->camz_r_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  hg->d_cross_adj = (GtkAdjustment *)gtk_adjustment_new(hg->d_cross,
							-500, 500, 
							1.0, 10.0, 0);
  my_signal_connect (hg->d_cross_adj, "value_changed",
		     cc_get_adj,
		     &hg->d_cross);
  spinner =  gtk_spin_button_new (hg->d_cross_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock("Sync",GTK_STOCK_REFRESH);
#endif
  gtkut_table_attach(table1, button, 0, 2, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (hds_sync_camz), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Sync CamZ values to the current ones");
#endif
  
  hg->camz_label = gtkut_label_new (hg->camz_date);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, hg->camz_label, 2, 4, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  
  
  // Cross Scan Calculator
  frame = gtkut_frame_new ("<b>Cross Scan Calculator</b>");
  gtkut_table_attach(table, frame, 2, 3, 3, 5,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(3, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtkut_label_new ("Center Wavelength [&#xC5;]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wcent,
					    3200, 9500, 
					    1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->wcent);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"accessories-calculator");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_OK);
#endif
  gtkut_table_attach(table1, button, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_signal_connect(button,"pressed",
		    hds_calc_cross_scan, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Calculate Cross Scan");
#endif
  
  hg->label_wcent = gtkut_label_new (" <i>(Calculated Cross Scan)</i>");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->label_wcent, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->label_wcent, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->label_wcent), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, hg->label_wcent, 0, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  
  // Wavelength Setup
  frame = gtkut_frame_new ("<b>Wavelength Setup  : Binning (sp)x(wv) [readout] / Slit / Filter / ImR</b>");
  gtkut_table_attach(table, frame, 0, 3, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(1, 1+MAX_USESETUP, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Setup");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("CCD Binning");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new (" ");
  gtkut_table_attach(table1, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Slit Width/Length [\"]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 4, 7, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(label,"These are ignored w/Image Slicers");
#endif
  
  label = gtk_label_new (" ");
  gtkut_table_attach(table1, label, 7, 8, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Filter1/Filter2");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 8, 11, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new (" ");
  gtkut_table_attach(table1, label, 11, 12, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new("ImR");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 12, 13, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(label,"ADC setup is not descirbed in OPE. Please inform to SA if you do not use ADC.");
#endif
  
  label = gtk_label_new("IS");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 13, 14, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(label,"If you use any of ISs, Slit length/width are ignored (set to 4.0 x 60.0).");
#endif
  
  label = gtk_label_new("I2");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 14, 15, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(label,"If you use I2-Cell (IN/OUT) with this setup, please check.");
#endif
  
  {
    gint i_use, i_set, i_nonstd;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      tmp=g_strdup_printf("%d.",i_use+1);
      check = gtk_check_button_new_with_label(tmp);
      g_free(tmp);
      gtkut_table_attach(table1, check, 0, 1, i_use+1, i_use+2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->setup[i_use].use);
      my_signal_connect (check, "toggled",
			 cc_get_toggle,
			 &hg->setup[i_use].use);
      if(i_use==0){
	gtk_widget_set_sensitive(check,FALSE);
      }
      
      gtk_widget_show(check);
      
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	for(i_set=0;i_set<MAX_SETUP;i_set++){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, HDS_setups[i_set].initial,
			     1, i_set, -1);
	  if(hg->setup[i_use].setup==i_set) iter_set=iter;
	}
	for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
	  tmp=g_strdup_printf("NonStd-%d",i_nonstd+1);
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, -(i_nonstd+1), -1);
	  g_free(tmp);
	  if(hg->setup[i_use].setup==-(i_nonstd+1)) iter_set=iter;
	}
	
	combo0 = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtkut_table_attach(table1, combo0, 1, 2, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo0),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo0), renderer, "text",0,NULL);
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo0),&iter_set);
	gtk_widget_show(combo0);
      }
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	int i_bin;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	for(i_bin=0;i_bin<MAX_BINNING;i_bin++){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 
			     0, binname[i_bin],
			     1, i_bin, -1);
	  if(hg->setup[i_use].binning==i_bin) iter_set=iter;
	}
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtkut_table_attach(table1, combo, 2, 3, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->setup[i_use].binning);
      }
      
      adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->setup[i_use].slit_width/500.,
						0.2, 4.0, 
						0.05,0.10,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_slit,
			 &hg->setup[i_use].slit_width);
      spinner =  gtk_spin_button_new (adj, 0, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtkut_table_attach(table1, spinner, 4, 5, i_use+1, i_use+2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      
      label = gtk_label_new ("/");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtkut_table_attach(table1, label, 5, 6, i_use+1, i_use+2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      
      adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->setup[i_use].slit_length/500.,
						2.0, 60.0, 
						0.1,1.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_slit,
			 &hg->setup[i_use].slit_length);
      spinner =  gtk_spin_button_new (adj, 0, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtkut_table_attach(table1, spinner, 6, 7, i_use+1, i_use+2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      
      
      {
	GtkListStore *store;
	GtkTreeIter iter;	  
	GtkCellRenderer *renderer;
	int i_fil;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	for(i_fil=0;i_fil<MAX_FILTER1;i_fil++){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 
			     0, filtername1[i_fil],
			     1, i_fil,
			     -1);
	}
	
	
	fil1_combo = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (fil1_combo), 0);
	gtkut_table_attach(table1, fil1_combo, 8, 9, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	if(!hg->setup[i_use].fil1)
	  hg->setup[i_use].fil1=g_strdup(HDS_setups[StdUb].fil1);
	gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil1_combo))),
			   hg->setup[i_use].fil1);
	gtk_editable_set_editable(GTK_EDITABLE(gtk_bin_get_child(GTK_BIN(fil1_combo))),TRUE);
	
	my_entry_set_width_chars(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil1_combo))),6);
	gtk_widget_show(fil1_combo);
	my_signal_connect (gtk_bin_get_child(GTK_BIN(fil1_combo)),"changed",
			   cc_get_entry,
			   &hg->setup[i_use].fil1);
	my_signal_connect (fil1_combo,"changed",hds_cc_get_fil_combo);
      }
      
      label = gtk_label_new ("/");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtkut_table_attach(table1, label, 9, 10, i_use+1, i_use+2,
			 GTK_SHRINK,GTK_SHRINK,0,0);
      
      
      {
	GtkListStore *store;
	GtkTreeIter iter;	  
	GtkCellRenderer *renderer;
	int i_fil;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	for(i_fil=0;i_fil<MAX_FILTER2;i_fil++){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 
			     0, filtername2[i_fil],
			     1, i_fil,
			     -1);
	}
	
	fil2_combo = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (fil2_combo), 0);
	gtkut_table_attach(table1, fil2_combo, 10, 11, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	if(!hg->setup[i_use].fil2)
	  hg->setup[i_use].fil2=g_strdup(HDS_setups[StdUb].fil2);
	gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil2_combo))),
			   hg->setup[i_use].fil2);
	gtk_editable_set_editable(GTK_EDITABLE(gtk_bin_get_child(GTK_BIN(fil2_combo))),TRUE);
	
	my_entry_set_width_chars(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil2_combo))),6);
	gtk_widget_show(fil2_combo);
	my_signal_connect (gtk_bin_get_child(GTK_BIN(fil2_combo)),
			   "changed",
			   cc_get_entry,
			   &hg->setup[i_use].fil2);
	my_signal_connect (G_OBJECT(fil2_combo),"changed",
			   hds_cc_get_fil_combo);
      }
      
      
      cdata[i_use]=g_malloc0(sizeof(confSetup));
      cdata[i_use]->hg=hg;
      cdata[i_use]->i_use=i_use;
      cdata[i_use]->length_adj=adj;
      cdata[i_use]->fil1_combo=fil1_combo;
      cdata[i_use]->fil2_combo=fil2_combo;
      my_signal_connect (combo0,
			 "changed",
			 hds_cc_usesetup,
			 (gpointer)cdata[i_use]);
      
      
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "No",
			   1, IMR_NO, -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Link",
			   1, IMR_LINK, -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Zenith",
			   1, IMR_ZENITH, -1);
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtkut_table_attach(table1, combo, 12, 13, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->setup[i_use].imr);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->setup[i_use].imr);
      }
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "No",
			   1, IS_NO, -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "0\".30x5",
			   1, IS_030X5, -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "0\".45x3",
			   1, IS_045X3, -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "0\".20x3",
			   1, IS_020X3, -1);
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtkut_table_attach(table1, combo, 13, 14, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->setup[i_use].is);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->setup[i_use].is);
      }
      
      {
	check = gtk_check_button_new();
	gtkut_table_attach(table1, check, 14, 15, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->setup[i_use].i2);
	my_signal_connect (check, "toggled",
			   cc_get_toggle,
			   &hg->setup[i_use].i2);
	
	gtk_widget_show(check);
      }

    }
  }
  
  
  // Non-Standard Setup
  frame = gtkut_frame_new ("<b>Non-Standard Setup</b>");
  gtkut_table_attach(table, frame, 0, 2, 0, 5,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(1, MAX_NONSTD, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new("Color");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new("Cross Scan");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new("Echelle");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new("CamR");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table1, label, 4, 5, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  
  {
    int i;
    
    for(i=0;i<MAX_NONSTD;i++){
      tmp=g_strdup_printf("NonStd-%d",i+1);
      label = gtk_label_new (tmp);
      g_free(tmp);
      gtkut_table_attach(table1, label, 0, 1, i+1, i+2,
			 GTK_FILL,GTK_SHRINK,0,0);
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Red",
			   1, COL_RED, -1);
	if(hg->nonstd[i].col==COL_RED) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Blue",
			   1, COL_BLUE, -1);
	if(hg->nonstd[i].col==COL_BLUE) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtkut_table_attach(table1, combo, 1, 2, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->nonstd[i].col);
      }
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].cross,
						13000, 28000, 
						1.0,1.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->nonstd[i].cross);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtkut_table_attach(table1, spinner, 2, 3, i+1, i+2,
			 GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      
      
      hg->echelle_adj[i] = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].echelle,
							       -3600, 3600, 
							       60.0,60.0,0);
      my_signal_connect (hg->echelle_adj[i], "value_changed",
			 cc_get_adj,
			 &hg->nonstd[i].echelle);
      spinner =  gtk_spin_button_new (hg->echelle_adj[i], 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtkut_table_attach(table1, spinner, 3, 4, i+1, i+2,
			 GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].camr,
						-7200, 7200, 
						60.0,60.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->nonstd[i].camr);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtkut_table_attach(table1, spinner, 4, 5, i+1, i+2,
			 GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    }
  }
  
  label = gtk_label_new ("HDS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), hg->setup_scrwin, label);
}

///  TAB for SV/AG
void HDS_SVAG_TAB_create(typHOE *hg){
  GtkWidget *scrwin;
  GtkWidget *frame;
  GtkWidget *table, *table1;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *combo, *combo0;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  GtkWidget *button;
  GtkTooltip *tooltip;
  
  scrwin = gtk_scrolled_window_new (NULL, NULL);
  table = gtkut_table_new(3, 6, FALSE, 0, 0, 0);
  
  gtk_container_set_border_width (GTK_CONTAINER (scrwin), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scrwin),
				    GTK_CORNER_BOTTOM_LEFT);
#ifdef USE_GTK3
  gtk_container_add(GTK_CONTAINER(scrwin),table);
#else
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),table);
#endif
  gtk_widget_set_size_request(scrwin, -1, 480);  
  
  
  // AG
  frame = gtkut_frame_new ("<b>AG</b> (Offset Auto Guider)");
  gtkut_table_attach(table, frame, 1, 2, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 6, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  
  label = gtk_label_new ("Exptime Factor");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_factor,
					    1, 5, 1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->exptime_factor);
  
  
  label = gtk_label_new ("Brightness");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->brightness,
					    0, 10000, 1000.0, 1000.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->brightness);
  
  
  // SV
  frame = gtkut_frame_new ("<b>SV</b> (Slit Viewer)");
  gtkut_table_attach(table, frame, 0, 1, 0, 2,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 6, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  
  label = gtk_label_new ("Read Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Part",
		       1, SV_PART, -1);
    if(hg->sv_area==SV_PART) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Full",
		       1, SV_FULL, -1);
    if(hg->sv_area==SV_FULL) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->sv_area);
  }
  
  label = gtk_label_new ("Exptime [msec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_sv,
					    500, 10000, 
					    100.0, 100.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->exptime_sv);


  label = gtk_label_new ("Read Region");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_region,
					    100, 400, 
					    10.0, 10.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->sv_region);


  label = gtk_label_new ("Calc Region");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_calc,
					    10, 150, 
					    5.0, 5.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->sv_calc);
  
  
  label = gtk_label_new ("Integrate");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_integrate,
					    1, 5, 
					    10.0, 10.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->sv_integrate);
  
  label = gtk_label_new ("Acquisition Time [sec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->oh_acq,
					    30, 300, 
					    10.0, 10.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->oh_acq);
  
  
  // Slit Center on SV
  frame = gtkut_frame_new ("<b>Slit Center on SV</b>");
  gtkut_table_attach(table, frame, 1, 2, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(1, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Slit_X ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("    Slit_Y ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_slitx,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_slitx);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_slity,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_slity);
  
  label = gtk_label_new ("IS_X ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("    IS_Y ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_isx,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_isx);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_isy,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_isy);
  
  
  label = gtk_label_new ("IS3_X ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("    IS3_Y ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_is3x,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_is3x);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_is3y,
					    100, 500, 
					    0.5, 0.5, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->sv_is3y);
  
  label = gtk_label_new ("SV/AG");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
}


/// Line list TAB
void HDS_LINE_TAB_create(typHOE *hg){
  GtkWidget *table;
  GtkWidget *hbox;
  GtkWidget *entry;
  GtkWidget *button;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *combo;
  GtkWidget *label;
  GtkWidget *check;
  
  table = gtkut_table_new(2, 2, FALSE, 0, 0, 0);
  
  hg->sw_linetree = gtk_scrolled_window_new (NULL, NULL);
  gtkut_table_attach_defaults (table, hg->sw_linetree, 0, 2, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hg->sw_linetree), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->sw_linetree),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(hg->sw_linetree),
				    GTK_CORNER_BOTTOM_LEFT);
  
  make_line_tree(hg);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Clear","edit-clear-all");
#else
  button=gtkut_button_new_from_stock("Clear",GTK_STOCK_CLEAR);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (linetree_init), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Stellar Absorption","format-justify-fill");
#else
  button=gtkut_button_new_from_stock("Stellar Absorption",GTK_STOCK_YES);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (linetree_star), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Nebular Emission","format-justify-fill");
#else
  button=gtkut_button_new_from_stock("Nebular Emission",GTK_STOCK_YES);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (linetree_nebula), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("High-z QSO","format-justify-fill");
#else
  button=gtkut_button_new_from_stock("High-z QSO",GTK_STOCK_YES);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (linetree_highz), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  
  label = gtkut_label_new ("    Redshift (<i>z</i>): ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  hg->adj_z = (GtkAdjustment *)gtk_adjustment_new(hg->etc_z,
						  -0.1, 6.0, 0.1, 0.1, 0);
  my_signal_connect (hg->adj_z, "value_changed",
		     cc_get_adj_double,
		     &hg->etc_z);
  spinner =  gtk_spin_button_new (hg->adj_z, 1, 3);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  

  label = gtk_label_new ("EFS Line List");
  gtk_widget_show(label);
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
}


void HDS_ETC_TAB_create(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *button;
  GdkPixbuf *icon;
  
  vbox = gtkut_vbox_new (FALSE, 5);
  
  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
  
  hg->etc_label= gtk_label_new (hg->etc_label_text);
  gtk_box_pack_start(GTK_BOX(hbox), hg->etc_label, TRUE, TRUE, 0);
  
  icon = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (hds_do_etc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Recalc ETC");
#endif
  
  icon = gdk_pixbuf_new_from_resource ("/icons/efs_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (hds_efs_for_etc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Display Echelle Format");
#endif
  
  
  hg->etc_sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->etc_sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->etc_sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), hg->etc_sw, TRUE, TRUE, 0);
  
  etc_append_tree(hg);
  
  label = gtk_label_new ("ETC");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
}


/////////////////////////////////////////////////////////////
//////////////////       for GUI signal connection
/////////////////////////////////////////////////////////////

void hds_calc_cross_scan(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  Crosspara cp;
  gchar *str;
  
  hg=(typHOE *)gdata;

  cp=get_cross_angle(hg->wcent, hg->d_cross);
  str=g_strdup_printf(" Color=%s,  Cross Scan=<b>%d</b>",
		     (cp.col==COL_RED) ? "<span color=\"#FF0000\"><b>Red</b></span>" : "<span color=\"#0000FF\"><b>Blue</b></span>",
		     cp.cross);
  gtk_label_set_markup(GTK_LABEL(hg->label_wcent),str);
  g_free(str);
}


void camz_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint fcdb_type_tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;
  
  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);
  
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=DBACCESS_HDSCAMZ;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(CAMZ_HOST);
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  hg->fcdb_path=g_strdup(CAMZ_PATH);
  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FCDB_FILE_TXT,NULL);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog, "delete-event", delete_fcdb, (gpointer)hg);

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  label=gtk_label_new("Downloading HDS CamZ status ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hg->plabel=gtk_label_new("Downloading HDS CamZ status ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->plabel, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->plabel, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);

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
  my_signal_connect(button,"pressed", cancel_fcdb, (gpointer)hg);
  
  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
#ifndef USE_WIN32
  act.sa_handler=fcdb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_fcdb(hg);
  gtk_main();

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  hg->fcdb_type=fcdb_type_tmp;
  flag_getFCDB=FALSE;
}


void hds_sync_camz(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  camz_dl(hg);
  camz_txt_parse(hg);
}


void hds_cc_get_fil_combo (GtkWidget *widget)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gchar *c;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 0, &c, -1);

    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))),c);
    g_free(c);
  }
}


void hds_cc_usesetup (GtkWidget *widget, gpointer gdata)
{
  gint i_set;
  gchar *p;
  confSetup *cdata;

  
  cdata = (confSetup *)gdata;

  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &i_set, -1);

    cdata->hg->setup[cdata->i_use].setup = i_set;
  }

  if (i_set>=0) {
    gtk_adjustment_set_value(GTK_ADJUSTMENT(cdata->length_adj),
			     (gdouble)HDS_setups[i_set].slit_length);
    cdata->hg->setup[cdata->i_use].slit_length = (guint)(HDS_setups[i_set].slit_length*500);
    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cdata->fil1_combo))),
		       HDS_setups[i_set].fil1);
    g_free(cdata->hg->setup[cdata->i_use].fil1);
    cdata->hg->setup[cdata->i_use].fil1=g_strdup(HDS_setups[i_set].fil1);
    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cdata->fil2_combo))),
		       HDS_setups[i_set].fil2);
    g_free(cdata->hg->setup[cdata->i_use].fil2);
    cdata->hg->setup[cdata->i_use].fil2=g_strdup(HDS_setups[i_set].fil2);
  }
}


void hds_efs_for_etc (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  hg->efs_setup=hg->etc_setup;

  go_efs(hg);
}


//////////////////// Write OPE files ///////////////////////

void HDS_WriteOPE(typHOE *hg, gboolean plan_flag){
  FILE *fp;
  int i_list=0, i_set, i_use, i_repeat, i_plan;
  gint to_year, to_month, to_day;
  gdouble new_ra, new_dec, new_d_ra, new_d_dec, yrs;
  gchar *tgt, *str;

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
  fprintf(fp, "OBE_ID=HDS\n");
  fprintf(fp, "</HEADER>\n");


  fprintf(fp, "\n");


  fprintf(fp, "<PARAMETER_LIST>\n");
  fprintf(fp, "DEF_SPEC=OBE_ID=HDS OBE_MODE=SPEC\n");
  fprintf(fp, "DEF_PROTO=OBE_ID=HDS OBE_MODE=PROTO\n");
  fprintf(fp, "DEF_COMMON=OBE_ID=COMMON OBE_MODE=TOOL\n");
  
  fprintf(fp, "\n");
  
  fprintf(fp, "###### M2-Z POSITION for I2-Cell ######\n");
  fprintf(fp, "# I2-Cell In  :  SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
  fprintf(fp, "# I2-Cell Out :  SETI2 $DEF_SPEC I2_POSITION=\"OUT\" $I2_Z\n");
  if(hg->setup[0].is==IS_NO){
    fprintf(fp, "## w/Slit\nI2_Z=Z_IN=0.15 Z_OUT=0.0\n");
    fprintf(fp, "## w/IS\n##  I2_Z=Z_IN=0.65 Z_OUT=0.5\n");
  }
  else{
    fprintf(fp, "## w/Slit\n##  I2_Z=Z_IN=0.15 Z_OUT=0.0\n");
    fprintf(fp, "## w/IS\nI2_Z=Z_IN=0.65 Z_OUT=0.5\n");
  }
  
  fprintf(fp, "\n");
  fprintf(fp, "###### SLIT CENTER POSITION ######\n");
  fprintf(fp, "## Slit\n");
  fprintf(fp, "SV_X=SV_SLIT_X=%5.1f\n",hg->sv_slitx);
  fprintf(fp, "SV_Y=SV_SLIT_Y=%5.1f CALC_MODE=SLIT SV_COUNT=(-100)\n",hg->sv_slity);
  fprintf(fp, "## Image Slicer #1 and #2\n");
  fprintf(fp, "SV_IS_X=SV_SLIT_X=%5.1f\n",hg->sv_isx);
  fprintf(fp, "SV_IS_Y=SV_SLIT_Y=%5.1f CALC_MODE=CTR SV_COUNT=(-160)\n",hg->sv_isy);
  fprintf(fp, "## Image Slicer #3\n");
  fprintf(fp, "SV_IS3_X=SV_SLIT_X=%5.1f\n",hg->sv_is3x);
  fprintf(fp, "SV_IS3_Y=SV_SLIT_Y=%5.1f CALC_MODE=CTR SV_COUNT=(-160)\n",hg->sv_is3y);
  
  fprintf(fp, "\n");
  fprintf(fp, "###### CAMERA Z POSITION ######\n");
  fprintf(fp, "## See the most recent values in http://hds.skr.jp/wiki/index.php?%%B8%%BD%%BA%%DF%%A4%%CECamZ%%C3%%CD\n");
  fprintf(fp, "CAMZ_B=CAM_Z=%d\n",hg->camz_b);
  fprintf(fp, "CAMZ_R=CAM_Z=%d\n",hg->camz_r);

  fprintf(fp, "\n");
  fprintf(fp, "###### GUIDE PARAMETER only for bad weather ######\n");
  fprintf(fp, "G_PARA=Exptime_Factor=%d Brightness=%d SVIntegrate=%d\n",
	  hg->exptime_factor,
	  hg->brightness,
	  hg->sv_integrate);
  
  fprintf(fp, "\n");
  fprintf(fp, "###### LIST of OBJECTS ######\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name, "TGT_");
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
  fprintf(fp, "# Focusing/ShowImage\n");
  fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
  fprintf(fp, "FocusSV $DEF_COMMON\n");
  fprintf(fp, "FocusAGSequence $DEF_PROTO\n");
  fprintf(fp, "FocusAG $DEF_PROTO\n");
  fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
  fprintf(fp, "MoveToStar $DEF_COMMON\n");
  fprintf(fp, "ShowImage $DEF_PROTO\n");
  fprintf(fp, "ShowImage $DEF_PROTO Display_Frame=HDSA00000000\n");
  fprintf(fp, "\n");

  fprintf(fp, "\n");
  fprintf(fp, "## Setup (If changed the number of filters, MOVE Collimator from UI)\n");
  fprintf(fp, "##  Use \"hds_col\" in Pane-9 of OBCP tmux.\n");
  fprintf(fp, "\n");


  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;
	fprintf(fp, "# Setup-%d  :  NonStd-%d   ", i_use+1, i_set+1);

	switch(hg->setup[i_use].is){
	case IS_030X5:
	  fprintf(fp, "### w/Image Slicer #1 (0.30x5) ###\n");
	  break;
	case IS_045X3:
	  fprintf(fp, "### w/Image Slicer #2 (0.45x3) ###\n");
	  break;
	case IS_020X3:
	  fprintf(fp, "### w/Image Slicer #3 (0.20x3) ###\n");
	  break;
	default:
	  fprintf(fp, "### w/Normal Slit ###\n");
	  break;
	}

	fprintf(fp, "## w/R<-->B Color Change\n");
	
	if(hg->nonstd[i_set].col==COL_BLUE){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=Blue CROSS_SCAN=%d COLLIMATOR=Blue $CAMZ_B",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	  if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	    fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	  }
	  if(hg->nonstd[i_set].camr!=-3600){
	    fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	  }
	  fprintf(fp, "\n## w/o Color Change\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=Red CROSS_SCAN=%d COLLIMATOR=Red $CAMZ_R",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	  if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	    fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	  }
	  if(hg->nonstd[i_set].camr!=-3600){
	    fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	  }
	  fprintf(fp, "\n## w/o Color Change\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);

	  fprintf(fp, "## !!! If necessary, Move Collimator via UI Maintenance -> Collimator tab !!!\n");
	  fprintf(fp, "##   Use \"hds_col\" in Pane-9 of OBCP tmux.\n");
	  switch(hg->setup[i_use].is){
	  case IS_NO:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=+2.106V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=+1.441V  !!!\n");
	    }
	    break;
	  case IS_030X5:
	  case IS_045X3:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=-0.401V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=-1.062V  !!!\n");
	    }
	    break;
	  case IS_020X3:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=-1.632V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=-2.292V  !!!\n");
	    }
	    break;
	  }
	  fprintf(fp,"## !!!  Calculate INC values referring the previous setup and values in http://hds.skr.jp/wiki/index.php?%%C0%%DF%%C4%%EA%%C1%%E1%%B8%%AB%%C9%%BD#c5068d22\n");
	  
	}
	fprintf(fp, "\n");
      }
      else{ //Std
	i_set=hg->setup[i_use].setup;
	
	fprintf(fp, "# Setup-%d  :  Std%s   ", i_use+1, HDS_setups[i_set].initial);
	switch(hg->setup[i_use].is){
	case IS_030X5:
	  fprintf(fp, "### w/Image Slicer #1 (0.30x5) ###\n");
	  break;
	case IS_045X3:
	  fprintf(fp, "### w/Image Slicer #2 (0.45x3) ###\n");
	  break;
	case IS_020X3:
	  fprintf(fp, "### w/Image Slicer #3 (0.20x3) ###\n");
	  break;
	default:
	  fprintf(fp, "### w/Normal Slit ###\n");
	  break;
	}

	fprintf(fp, "## w/R<--->B Color Change\n");
	
	if(i_set<StdI2b){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_B\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  HDS_setups[i_set].cross,HDS_setups[i_set].initial,HDS_setups[i_set].col);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_R\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  HDS_setups[i_set].cross,HDS_setups[i_set].initial,HDS_setups[i_set].col);
	}
	fprintf(fp, "## w/o Color Change\n");
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=Std%s\n",
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		HDS_setups[i_set].initial);

	fprintf(fp, "## !!! If necessary, Move Collimator via UI Maintenance -> Collimator tab !!!\n");
	fprintf(fp, "##   Use \"hds_col\" in Pane-9 of OBCP tmux.\n");
	switch(hg->setup[i_use].is){
	case IS_NO:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=+2.106V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=+1.441V  !!!\n");
	  }
	  break;
	case IS_030X5:
	case IS_045X3:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=-0.401V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=-1.062V  !!!\n");
	  }
	  break;
	case IS_020X3:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=-1.632V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=-2.292V  !!!\n");
	  }
	  break;
	}
	fprintf(fp,"## !!!  Calculate INC values referring the previous setup and values in http://hds.skr.jp/wiki/index.php?%%C0%%DF%%C4%%EA%%C1%%E1%%B8%%AB%%C9%%BD#c5068d22\n");

      }
      fprintf(fp, "\n");
	
    }
  }
  

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "#################### %d/%d/%d Obs Sequence ####################\n",
	  hg->fr_month,hg->fr_day,hg->fr_year);


  if(plan_flag){
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_OBJ:
	WriteOPE_OBJ_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_SetAzEl:
	fprintf(fp, "### SetAzEl  Az=%d El=%d  via Launcher ###\n\n",(int)hg->plan[i_plan].az1,(int)hg->plan[i_plan].el1);
	break;

      case PLAN_TYPE_FOCUS:
	WriteOPE_FOCUS_plan(fp,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_BIAS:
	WriteOPE_BIAS_plan(fp,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_FLAT:
	WriteOPE_FLAT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_COMP:
	WriteOPE_COMP_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_SETUP:
	WriteOPE_SetUp_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_I2:
	if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
	fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
	if(hg->plan[i_plan].i2_pos==PLAN_I2_IN){
	  fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n\n\n");
	}
	else{
	  fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n\n\n");
	}
	break;
      }
    }

  }
  else{
    fprintf(fp, "###### Evening CAL #####\n\n");
    
    WriteOPE_BIAS(fp);
    
    fprintf(fp, "### [LAUNCHER/Telescope] Telescope Setup [NsOpt] #####\n\n");
    fprintf(fp, "\n");
    
    WriteOPE_FLAT(fp,hg);
    
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL LampOff #####\n\n");
    fprintf(fp, "\n");
    
    WriteOPE_COMP(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT Retract #####\n\n");
    
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Evening FocusSV #####\n");
    //fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
    //fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
    fprintf(fp, "# [Launcher/HDS] FocusSVSequence \n");
    fprintf(fp, "# [Launcher/HDS] Set Seeing\n\n\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### (Please Edit Targets) #####\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### MidNight FocusSV #####\n");
    fprintf(fp, "# [Launcher/HDS] FocusSVSequence \n");
    fprintf(fp, "# [Launcher/HDS] Set Seeing\n\n\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### (Please Edit Targets) #####\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Morning FocusSV #####\n");
    fprintf(fp, "# [Launcher/HDS] FocusSVSequence \n");
    fprintf(fp, "# [Launcher/HDS] Set Seeing\n\n\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Morning CAL #####\n\n");
    fprintf(fp, "### Shutdown VGW\n");
    fprintf(fp, "ShutdownVGW $DEF_COMMON\n");
    fprintf(fp, "\n");
    WriteOPE_COMP(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT LampOff #####\n\n");
    
    fprintf(fp, "\n");
    
    WriteOPE_FLAT(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL Returct #####\n\n");
    
    fprintf(fp, "\n");

    WriteOPE_BIAS(fp);

  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "#################### Reserved Targets ####################\n");
  fprintf(fp, "### Guide = NO  : No Guide\n");
  fprintf(fp, "### Guide = AG  : AG Guide\n");
  fprintf(fp, "### Guide = SV  : SV Guide\n");
  fprintf(fp, "\n");


  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup < 0){ //Non-Std
	fprintf(fp, "\n\n########  NonStd-%d %dx%dbinning  #######\n\n",
		-hg->setup[i_use].setup,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
      }
      else{  //Std
	fprintf(fp, "\n\n########  Std%s %dx%dbinning  #######\n",
		HDS_setups[hg->setup[i_use].setup].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
      }
    
      for(i_list=0;i_list<hg->i_max;i_list++){
	/// Comment for Object Name ///
	if(hg->obj[i_list].setup[i_use]){
	  fprintf(fp, "# [%d] %s   ",
		  i_list+1,
		  hg->obj[i_list].name);
	  
	  if(fabs(hg->obj[i_list].mag)<99){
	    str=get_band_name(hg, i_list);
	    fprintf(fp, " ,   %4.1lfmag (%s)",
		    hg->obj[i_list].mag, (str)?(str):"---");
	    if(str) g_free(str);
	  }
	  else{
	    fprintf(fp, " ,");
	  }

	  if(hg->obj[i_list].note){
	    fprintf(fp, "    %s\n",
		    hg->obj[i_list].note);
	  }
	  else{
	    fprintf(fp, "\n");
	  }


	  /// Comment for Setup ///
	  if(hg->setup[i_use].setup < 0){ //Non-Std
	    fprintf(fp, "#    NonStd-%d %dx%dbinning /",
		    -hg->setup[i_use].setup,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{  //Std
	    fprintf(fp, "#    Std%s %dx%dbinning /",
		    HDS_setups[hg->setup[i_use].setup].initial,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	    fprintf(fp," No Guide /");
	    break;
	  case AG_GUIDE:
	    fprintf(fp," AG Guide /");
	    break;
	  case SV_GUIDE:
	    fprintf(fp," SV Guide /");
	    break;
	  case SVSAFE_GUIDE:
	    fprintf(fp,"SV[Safe] Guide /");
	    break;
	  }

	  switch(hg->setup[i_use].imr){
	  case IMR_NO:
	    fprintf(fp," ImR=No\n");
	    break;
	  case IMR_LINK:
	    fprintf(fp," ImR=Link\n");
	    break;
	  case IMR_ZENITH:
	    fprintf(fp," ImR=Zenith\n");
	    break;
	  }   



	  /// SetupField ///
	  if(hg->obj[i_list].mag<MAG_SVFILTER2){  // Filter=ND2
	    fprintf(fp,"SetupField SV_Filter01=3");
	  }
	  else if(hg->obj[i_list].mag<MAG_SVFILTER1){  //Filter=R
	    fprintf(fp,"SetupField SV_Filter01=1");
	  }
	  else{  //Filter=No
	    fprintf(fp,"SetupField");// Filter=No
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	    fprintf(fp," Guide=No");
	    break;
	  case AG_GUIDE:
	    fprintf(fp," Guide=AG");
	    break;
	  case SV_GUIDE:
	    fprintf(fp," Guide=SV Mode=SemiAuto");
	    break;
	  case SVSAFE_GUIDE:
	    fprintf(fp," Guide=SV Mode=Safe");
	    break;
	  }

	  if(hg->obj[i_list].i_nst<0){
	    tgt=make_tgt(hg->obj[i_list].name, "TGT_");
	    if((fabs(hg->obj[i_list].pm_ra)>100)
	       ||(fabs(hg->obj[i_list].pm_dec)>100)){
	      fprintf(fp, " $DEF_PROTO $PM%s", tgt);
	    }
	    else{
	      fprintf(fp, " $DEF_PROTO $%s", tgt);
	    }
	    g_free(tgt);
	  }
	  else{
	    fprintf(fp, 
		    " $DEF_PROTO OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
		    hg->obj[i_list].name,
		    g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	  case AG_GUIDE:
	    fprintf(fp, " SVRegion=%d",
		    hg->sv_region);
	    break;
	  case SV_GUIDE:
	  case SVSAFE_GUIDE:
	    fprintf(fp," ReadRegion=%d CalcRegion=%d",
		    hg->sv_region,
		    hg->sv_calc);
	    break;
	  }

	  fprintf(fp, " Exptime_SV=%d",
		  hg->exptime_sv);
	  
	  switch(hg->setup[i_use].imr){
	  case IMR_NO:
	    fprintf(fp," IMGROT_FLAG=0");
	    break;
	  case IMR_LINK:
	    fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f",
		    hg->obj[i_list].pa);
	    break;
	  case IMR_ZENITH:
	    fprintf(fp," IMGROT_FLAG=1 IMGROT_MODE=ZENITH");
	    break;
	  }   
	  
	  if(hg->setup[i_use].is!=IS_NO){
	    fprintf(fp," IS_FLAG=1");
	    if(hg->setup[i_use].is==IS_020X3){
	      fprintf(fp, " $SV_IS3_X $SV_IS3_Y $G_PARA");
	    }
	    else{
	      fprintf(fp, " $SV_IS_X $SV_IS_Y $G_PARA");
	    }
	  }
	  else{
	    fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		    hg->setup[i_use].slit_length,
		    hg->setup[i_use].slit_width);
	    fprintf(fp, " $SV_X $SV_Y $G_PARA");
	  }


	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	  case AG_GUIDE:
	    fprintf(fp,"\n");
	    break;
	  case SV_GUIDE:
	  case SVSAFE_GUIDE:
	    if(hg->sv_area==SV_FULL){
	      fprintf(fp, " ReadArea=Full\n");
	    }
	    else{
	      fprintf(fp, " ReadArea=Part\n");
	    }
	    break;
	  }

	  
	  for(i_repeat=0;i_repeat<hg->obj[i_list].repeat;i_repeat++){
	    fprintf(fp,"GetObject");
	    if(hg->setup[i_use].is!=IS_NO){
	      fprintf(fp," IS_FLAG=1");
	      if(hg->setup[i_use].is==IS_020X3){
		fprintf(fp," IS_Z_OFFSET=-0.40");
	      }
	    }
	    if(hg->obj[i_list].i_nst<0){
	      tgt=make_tgt(hg->obj[i_list].name, "TGT_");
	      if((fabs(hg->obj[i_list].pm_ra)>100)
		 ||(fabs(hg->obj[i_list].pm_dec)>100)){
		fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $PM%s\n",
			hg->obj[i_list].exp, hg->sv_integrate, tgt);
	      }
	      else{
		fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
			hg->obj[i_list].exp, hg->sv_integrate, tgt);
	      }
	      g_free(tgt);
	    }
	    else{
	      fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
		      hg->obj[i_list].exp, hg->sv_integrate, 
		      hg->obj[i_list].name);
	    }
	  }

	  if(hg->setup[i_use].i2){
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    for(i_repeat=0;i_repeat<hg->obj[i_list].repeat;i_repeat++){
	      fprintf(fp,"GetObject");
	      if(hg->setup[i_use].is!=IS_NO){
		fprintf(fp," IS_FLAG=1");
		if(hg->setup[i_use].is==IS_020X3){
		  fprintf(fp," IS_Z_OFFSET=-0.40");
		}
	      }
	      if(hg->obj[i_list].i_nst<0){
		tgt=make_tgt(hg->obj[i_list].name, "TGT_");
		if((fabs(hg->obj[i_list].pm_ra)>100)
		   ||(fabs(hg->obj[i_list].pm_dec)>100)){
		  fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $PM%s\n",
			  hg->obj[i_list].exp, hg->sv_integrate, tgt);
		}
		else{
		  fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
			  hg->obj[i_list].exp, hg->sv_integrate, tgt);
		}
		g_free(tgt);
	      }
	      else{
		fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
			hg->obj[i_list].exp, hg->sv_integrate, 
			hg->obj[i_list].name);
	      }
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  fprintf(fp, "\n");
	}
      }
    }
  }


  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "##########   Calibration   ###########\n");
  fprintf(fp, "\n");

  WriteOPE_BIAS(fp);


  fprintf(fp, "##### COMPARISON #####\n"); 
  fprintf(fp, "\n");

  WriteOPE_COMP(fp,hg);

  //fprintf(fp, "SHUTDOWNCOMPARISON_HCTLAMP $DEF_PROTO OBJECT=HCT F_SELECT=NS_OPT\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HCT OBE_ID=COMMON OBE_MODE=TOOL OBJECT=HCT F_SELECT=NS_OPT\n");
  
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "##### FLAT #####\n");
  fprintf(fp, "\n");

  WriteOPE_FLAT(fp,hg);

  fprintf(fp, "\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HALLAMP $DEF_PROTO OBJECT=HAL F_SELECT=NS_OPT\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HAL  OBE_ID=COMMON OBE_MODE=TOOL OBJECT=HAL F_SELECT=NS_OPT\n");

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "</Command>\n");


  fclose(fp);

  
}


void WriteYAML(typHOE *hg){
  FILE *fp;
  int i_list=0, i_set, i_use, i_repeat, i_plan, j_set;
  gint to_year, to_month, to_day, nonstd_flat;
  gchar *tgt;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
  }

  to_year=hg->fr_year;
  to_month=hg->fr_month;
  to_day=hg->fr_day;
  add_day(hg, &to_year, &to_month, &to_day, 1);

  fprintf(fp, "#####   Subaru Gen2 Intermediate Format File (YAML)\n");
  fprintf(fp, "#####   created by HOE : HDS OPE file Editor,  version "VERSION"\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // General Info
  fprintf(fp, "###########################\n");
  fprintf(fp, "### General Information ###\n");
  fprintf(fp, "###########################\n");
  fprintf(fp, "observation:\n");
  fprintf(fp, "   prop-id  : %s\n",hg->prop_id);
  fprintf(fp, "   start    : %4d-%02d-%02d 16:59 -10\n",hg->fr_year,hg->fr_month,hg->fr_day);
  fprintf(fp, "   end      : %4d-%02d-%02d  8:25 -10\n",to_year,to_month,to_day);
  if(hg->observer)
    fprintf(fp, "   observers: [%s ]\n",hg->observer);
  else
    fprintf(fp, "   observers: [ ]\n");
  fprintf(fp, "   operators: [ ]\n");
  fprintf(fp, "   ss       : [ ]\n");
  fprintf(fp, "   comments : \"       \"\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // Target Definition
  fprintf(fp, "#############################\n");
  fprintf(fp, "### Target Specifications ###\n");
  fprintf(fp, "#############################\n");
  fprintf(fp, "targets:\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name, "TGT_");
    fprintf(fp, "   - &%s\n",tgt);
    g_free(tgt);
    fprintf(fp, "      name   : \"%s\"\n", hg->obj[i_list].name);
    fprintf(fp, "      ra     : s%09.2f\n", hg->obj[i_list].ra);
    fprintf(fp, "      dec    : s%+010.2f\n", hg->obj[i_list].dec);
    fprintf(fp, "      equinox: %7.2f\n", hg->obj[i_list].equinox);
    if(hg->obj[i_list].note)
      fprintf(fp, "      note   : \"%s\"\n", hg->obj[i_list].note);
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");


  // Telescope Specifications
  fprintf(fp, "################################\n");
  fprintf(fp, "### Telescope Specifications ###\n");
  fprintf(fp, "################################\n");
  fprintf(fp, "telescopes:\n");

  // No Guide
  fprintf(fp, "   - &t_no\n");
  fprintf(fp, "      foci           : ns_opt\n");
  fprintf(fp, "      imr            : none\n");
  fprintf(fp, "      guiding        : no\n");
  if(hg->sv_area==SV_FULL){
    fprintf(fp, "      sv_area        : full\n");
  }
  else{
    fprintf(fp, "      sv_area        : part\n");
  }
  fprintf(fp, "      sv_exp         : %d\n",hg->exptime_sv);
  fprintf(fp, "      sv_read_region : %d\n",hg->sv_region);
  fprintf(fp, "      sv_filter      : 0\n");
  fprintf(fp, "      sv_integrate   : %d\n",hg->sv_integrate);
  fprintf(fp, "      sv_acq         : %d\n",hg->oh_acq);
  fprintf(fp, "\n");
  
  // AG Guide
  fprintf(fp, "   - &t_ag\n");
  fprintf(fp, "      <<             : *t_no\n");
  fprintf(fp, "      guiding        : ag\n");
  fprintf(fp, "      exptime_factor : %d\n",hg->exptime_factor);
  fprintf(fp, "      brightness     : %d\n",hg->brightness);
  fprintf(fp, "\n");

  // SV Guide [Safe]
  fprintf(fp, "   - &t_sv_safe\n");
  fprintf(fp, "      <<             : *t_no\n");
  fprintf(fp, "      guiding        : sv\n");
  fprintf(fp, "      sv_mode        : safe\n");
  fprintf(fp, "      sv_calc_region : %d\n",hg->sv_calc);
  fprintf(fp, "      sv_slit_x      : %.1f\n",hg->sv_slitx);
  fprintf(fp, "      sv_slit_y      : %.1f\n",hg->sv_slity);
  fprintf(fp, "      sv_is_x        : %.1f\n",hg->sv_isx);
  fprintf(fp, "      sv_is_y        : %.1f\n",hg->sv_isy);
  fprintf(fp, "      sv_is3_x       : %.1f\n",hg->sv_is3x);
  fprintf(fp, "      sv_is3_y       : %.1f\n",hg->sv_is3y);
  fprintf(fp, "\n");

  // SV Guide [SemiAuto]
  fprintf(fp, "   - &t_sv_semiauto\n");
  fprintf(fp, "      <<             : *t_sv_safe\n");
  fprintf(fp, "      sv_mode        : semiauto\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // CAL Specifications
  fprintf(fp, "##########################\n");
  fprintf(fp, "### CAL Specifications ###\n");
  fprintf(fp, "##########################\n");
  fprintf(fp, "cal:\n");
  for(j_set=StdUb;j_set<=StdHa;j_set++){
    fprintf(fp, "  # Std%s\n", HDS_setups[j_set].initial);
    fprintf(fp, "   - &cal_std_%s\n", HDS_setups[j_set].initial);
    fprintf(fp, "      f1_amp     : %s    # Flat for CCD1\n", HDS_setups[j_set].f1_amp);
    fprintf(fp, "      f1_fil1    : %d\n", HDS_setups[j_set].f1_fil1);
    fprintf(fp, "      f1_fil2    : %d\n", HDS_setups[j_set].f1_fil2);
    fprintf(fp, "      f1_fil3    : %d\n", HDS_setups[j_set].f1_fil3);
    fprintf(fp, "      f1_fil4    : %d\n", HDS_setups[j_set].f1_fil4);
    fprintf(fp, "      f1_exp     : %d\n", HDS_setups[j_set].f1_exp);
    fprintf(fp, "      f2_amp     : %s    # Flat for CCD2\n", HDS_setups[j_set].f2_amp);
    fprintf(fp, "      f2_fil1    : %d\n", HDS_setups[j_set].f2_fil1);
    fprintf(fp, "      f2_fil2    : %d\n", HDS_setups[j_set].f2_fil2);
    fprintf(fp, "      f2_fil3    : %d\n", HDS_setups[j_set].f2_fil3);
    fprintf(fp, "      f2_fil4    : %d\n", HDS_setups[j_set].f2_fil4);
    fprintf(fp, "      f2_exp     : %d\n", HDS_setups[j_set].f2_exp);
    fprintf(fp, "      c_amp      : 15    # Comparison\n");
    fprintf(fp, "      c_fil1     : 1\n");
    fprintf(fp, "      c_fil2     : 1\n");
    fprintf(fp, "      c_fil3     : 1\n");
    fprintf(fp, "      c_fil4     : 1\n");
    fprintf(fp, "      c_exp      : 20\n");
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");


  // HDS Specifications
  fprintf(fp, "#################################\n");
  fprintf(fp, "### Instrument Specifications ###\n");
  fprintf(fp, "#################################\n");
  fprintf(fp, "instruments:\n");
  fprintf(fp, "   - &hds_basic\n");
  fprintf(fp, "      obcp        : hds\n");
  fprintf(fp, "      camz_b      : %d\n",hg->camz_b);
  fprintf(fp, "      camz_r      : %d\n",hg->camz_r);
  fprintf(fp, "      d_cross     : %d\n",hg->d_cross);
  fprintf(fp, "      exptime     : 1\n");
  fprintf(fp, "      repeat      : 1\n");
  fprintf(fp, "\n");

  // HDS Standard
  fprintf(fp, "## HDS Standard Setups\n");
  
  for(j_set=StdUb;j_set<=StdHa;j_set++){
    fprintf(fp, "  # Std%s\n", HDS_setups[j_set].initial);
    fprintf(fp, "   - &hds_std_%s\n", HDS_setups[j_set].initial);
    fprintf(fp, "      collimator : %s\n", HDS_setups[j_set].col);
    fprintf(fp, "      cross      : %s\n", HDS_setups[j_set].cross);
    fprintf(fp, "      slit_length: %.0lf\n", HDS_setups[j_set].slit_length);
    fprintf(fp, "      filter1    : %s\n", HDS_setups[j_set].fil1);
    fprintf(fp, "      filter2    : %s\n", HDS_setups[j_set].fil2);
    fprintf(fp, "      echelle    : %d\n", DEF_ECHELLE);
    fprintf(fp, "      cross_scan : %.0lf\n", HDS_setups[j_set].cross_scan);
    fprintf(fp, "      camera_rorate: -3600\n");
    fprintf(fp, "\n");
  }
  

  fprintf(fp, "\n");

  fprintf(fp, "## HDS Setups for this observation\n");
  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;
	fprintf(fp, "# NonStd-%d %dx%d\n", i_set+1,    
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "   - &hds_nonstd%d_%dx%d\n", i_set+1,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	if(hg->nonstd[i_set].col==COL_BLUE){
	  fprintf(fp, "      collimator       : Blue\n");
	  fprintf(fp, "      cross            : Blue\n");
	}
	else{
	  fprintf(fp, "      collimator       : Red\n");
	  fprintf(fp, "      cross            : Red\n");
	}
	fprintf(fp, "      slit_width     : %d\n", hg->setup[i_use].slit_width);
	fprintf(fp, "      slit_length    : %d\n", hg->setup[i_use].slit_length);
	fprintf(fp, "      filter1        : %s\n", hg->setup[i_use].fil1);
	fprintf(fp, "      filter2        : %s\n", hg->setup[i_use].fil2);
	fprintf(fp, "      echelle        : %d\n", hg->nonstd[i_set].echelle);
	fprintf(fp, "      cross_scan     : %d\n", hg->nonstd[i_set].cross);
	fprintf(fp, "      camera_rorate  : %d\n", hg->nonstd[i_set].camr);
	fprintf(fp, "      xbin           : %d\n", hg->binning[hg->setup[i_use].binning].x);
	fprintf(fp, "      ybin           : %d\n", hg->binning[hg->setup[i_use].binning].y);
	
	if(hg->nonstd[i_set].col==COL_BLUE){
	  for(j_set=StdUb;j_set<StdI2b;j_set++){
	    if(hg->nonstd[i_set].cross<=HDS_setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdUb:
	    nonstd_flat=StdUb;
	  case StdI2b:
	    nonstd_flat=StdYa;
	  default:
	    if((HDS_setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - HDS_setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    else {
	      if(j_set==StdUb){
		nonstd_flat=j_set;
	      }
	      else{
		nonstd_flat=j_set-1;
	      }
	    }
	    
	  }
	}
	else{
	  for(j_set=StdI2b;j_set<StdHa;j_set++){
	    if(hg->nonstd[i_set].cross<=HDS_setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdI2b:
	    nonstd_flat=StdI2b;
	  case StdHa:
	    nonstd_flat=StdNIRa;
	  default:
	    if((HDS_setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - HDS_setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    if(j_set==StdI2b){
	      nonstd_flat=j_set;
	    }
	    else{
	      nonstd_flat=j_set-1;
	    }
	    
	  }
	}

	fprintf(fp, "      cal : { <<: *cal_std_%s", HDS_setups[nonstd_flat].initial);
	if(hg->binning[hg->setup[i_use].binning].x*hg->binning[hg->setup[i_use].binning].y==1){
	  fprintf(fp, "}\n");
	}
	else{
	  gint f1_exp, f2_exp, c_exp;
	  f1_exp=HDS_setups[nonstd_flat].f1_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f1_exp==0) f1_exp=1;

	  f2_exp=HDS_setups[nonstd_flat].f2_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f2_exp==0) f2_exp=1;

	  c_exp=20/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(c_exp==0) c_exp=1;

	  fprintf(fp, ", f1_exp: %d, f2_exp: %d, fc_exp: %d}\n"
		  ,f1_exp,f2_exp,c_exp);
	}

	fprintf(fp, "\n");

      }
      else{ //Std
	i_set=hg->setup[i_use].setup;
	fprintf(fp, "# Std%s %dx%d\n", HDS_setups[i_set].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "   - &hds_std%s_%dx%d\n", HDS_setups[i_set].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "      << : *hds_std_%s\n", HDS_setups[i_set].initial);
	fprintf(fp, "      slit_width     : %d\n", hg->setup[i_use].slit_width);
	fprintf(fp, "      slit_length    : %d\n", hg->setup[i_use].slit_length);
	fprintf(fp, "      filter1        : %s\n", hg->setup[i_use].fil1);
	fprintf(fp, "      filter2        : %s\n", hg->setup[i_use].fil2);
	fprintf(fp, "      xbin           : %d\n", hg->binning[hg->setup[i_use].binning].x);
	fprintf(fp, "      ybin           : %d\n", hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "      cal : { <<: *cal_std_%s", HDS_setups[i_set].initial);
	if(hg->binning[hg->setup[i_use].binning].x*hg->binning[hg->setup[i_use].binning].y==1){
	  fprintf(fp, "}\n");
	}
	else{
	  gint f1_exp, f2_exp, c_exp;
	  f1_exp=HDS_setups[i_set].f1_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f1_exp==0) f1_exp=1;

	  f2_exp=HDS_setups[i_set].f2_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f2_exp==0) f2_exp=1;

	  c_exp=20/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(c_exp==0) c_exp=1;

	  fprintf(fp, ", f1_exp: %d, f2_exp: %d, c_exp: %d}\n"
		  ,f1_exp,f2_exp, c_exp);
	}
      }
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n");


  // Activities
  fprintf(fp, "################################\n");
  fprintf(fp, "### Acitivity Specifications ###\n");
  fprintf(fp, "################################\n");
  fprintf(fp, "activities:\n");

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    switch(hg->plan[i_plan].type){
    case PLAN_TYPE_COMMENT:
      WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_OBJ:
      WriteYAML_OBJ_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_FOCUS:
      if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
      fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
      switch(hg->plan[i_plan].focus_mode){
      case PLAN_FOCUS1:
	fprintf(fp, "   - type   : focussv\n");
	break;
      default:
	fprintf(fp, "   - type   : focusag\n");
      }
      break;
      
    case PLAN_TYPE_BIAS:
      WriteYAML_BIAS_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_FLAT:
      WriteYAML_FLAT_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_COMP:
      WriteYAML_COMP_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_SETUP:
      WriteYAML_SetUp_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_I2:
      if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
      fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
      fprintf(fp, "   - type   : i2\n");
      if(hg->plan[i_plan].i2_pos==PLAN_I2_IN){
	fprintf(fp, "     i2_position : in\n");
      }
      else{
	fprintf(fp, "     i2_position : out\n");
      }
      break;
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "##############################\n");
  fprintf(fp, "##########   END   ###########\n");
  fprintf(fp, "##############################\n");
  fprintf(fp, "\n");


  fclose(fp);
  
}


void HDS_WriteService(typHOE *hg){
  FILE *fp;
  gint i, i_list, len, name_len=0, band_len=0, i_set, i_fc=0, num_fc=0;
  gchar *band_str, *form_str=NULL, *imr_str, *setup_str;
  gdouble r;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  // Precheck
  for(i_list=0;i_list<hg->i_max;i_list++){
    len=strlen(hg->obj[i_list].name);
    if(len>name_len) name_len=len;
    
    band_str=get_band_name(hg, i_list);
    len=strlen(band_str);
    if(len>band_len) band_len=len;
    if(band_str) g_free(band_str);
  }

  {
    fprintf (fp, "1-5.  Target List\n");
    fprintf(fp, "    (1)Name    (2)RA      Dec     Equinox   (3)Magnitude  (4)ExpTime         total S/N     (5)Slit PA\n");
    form_str=g_strdup_printf("    %%%ds,  %%09.2lf, %%+010.2lf, %%7.2lf,  %%%ds=%%5.2lf,    %%2d x %%4dsec = %%5dsec,  %%4.0lf /pixel,   %%s\n",
			     name_len,band_len);

    for(i_list=0;i_list<hg->i_max;i_list++){
      band_str=get_band_name(hg, i_list);
      if(hg->etc_imr==ETC_IMR_NO){
	imr_str=g_strdup("(Any)");
      }
      else if(hg->setup[hg->etc_setup].imr==IMR_ZENITH){
	imr_str=g_strdup("Parallactic");
      }
      else{
	imr_str=g_strdup_printf("%.2lfdeg",hg->obj[i_list].pa);
      }
      
      fprintf(fp, form_str,
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox,
	      band_str,
	      hg->obj[i_list].mag,
	      hg->obj[i_list].repeat,
	      hg->obj[i_list].exp,
	      hg->obj[i_list].exp*hg->obj[i_list].repeat,
	      (gdouble)hg->obj[i_list].snr*sqrt((gdouble)hg->obj[i_list].repeat),
	      imr_str);
      if(band_str) g_free(band_str);
      if(imr_str) g_free(imr_str);
    }

    if(form_str) g_free(form_str);

    fprintf(fp,"\n");
    if(hg->etc_wave==ETC_WAVE_CENTER){
      fprintf(fp,"          *** Total S/N is estimated in %.2lf seeing at Center ***\n", hg->etc_seeing);
    }
    else{
      fprintf(fp,"          *** Total S/N is estimated in %.2lf seeing at %dA ***\n", hg->etc_seeing, hg->etc_waved);
    }

    fprintf(fp,"\n");
    fprintf(fp,"\n");

    fprintf (fp, "6.  Requirement for ADC (Yes/No)\n");
    if(hg->etc_adc==ETC_ADC_IN){
      fprintf (fp, "      Yes\n");
    }
    else{
      fprintf (fp, "      No\n");
    }

    fprintf (fp, "7.  Image Rotator Mode (Red/Blue/None)\n");
    if(hg->etc_imr==ETC_IMR_RED){
      fprintf (fp, "      Red\n");
    }
    else if(hg->etc_imr==ETC_IMR_BLUE){
      fprintf (fp, "      Blue\n");
    }
    else{
      fprintf (fp, "      None\n");
    }

    fprintf (fp, "8.  Wavelength Setup\n");
    if(hg->setup[hg->etc_setup].setup<0){
      i_set=-hg->setup[hg->etc_setup].setup-1;
      if(hg->nonstd[i_set].col==COL_BLUE){
	setup_str=g_strdup_printf("      Non-Std (Blue/Cross Scan=%d\"/Camera Rot=%d\"/Echelle=%d\")\n",
				  hg->nonstd[i_set].cross,
				  hg->nonstd[i_set].camr,
				  hg->nonstd[i_set].echelle);
      }
      else{
	setup_str=g_strdup_printf("      Non-Std (Red/Cross Scan=%d\"/Camera Rot=%d\"/Echelle=%d\")\n",
				  hg->nonstd[i_set].cross,
				  hg->nonstd[i_set].camr,
				  hg->nonstd[i_set].echelle);
      }
      fprintf(fp,"%s", setup_str);
      if(setup_str) g_free(setup_str);
    }
    else{
      fprintf(fp, "      Std%s\n", HDS_setups[hg->setup[hg->etc_setup].setup].initial);
    }

    fprintf (fp, "9.  Wavelength Resolution\n");
    switch(hg->setup[hg->etc_setup].is){
    case IS_030X5:
      fprintf (fp, "      R=110000 (Image Slicer #1 : 0.30\"x5)\n");
      break;

    case IS_045X3:
      fprintf (fp, "      R=80000 (Image Slicer #2 : 0.45\"x3)\n");
      break;

    case IS_020X3:
      fprintf (fp, "      R=160000 (Image Slicer #3 : 0.20\"x3)\n");
      break;

    case IS_NO:
      r=36000/((gdouble)hg->setup[hg->etc_setup].slit_width/500.);
      fprintf (fp, "      R=%.0lf (%.2lf\" slit width)\n",
	       (r>160000)?16000:r,
	       (gdouble)hg->setup[hg->etc_setup].slit_width/500.);
      break;
    }
	     
    fprintf (fp, "10. CCD Binning\n");
    fprintf (fp, "      %dx%d\n",
	     hg->binning[hg->setup[hg->etc_setup].binning].x,
	     hg->binning[hg->setup[hg->etc_setup].binning].y);

    fprintf (fp, "11. Requested seeing size\n");
    fprintf (fp, "      Any\n");
    
    fprintf (fp, "12. Are photometric conditions necessary? (Yes/No)\n");
    fprintf (fp, "      No\n");

    fprintf(fp,"\n");
    
    fprintf (fp, "13. Priority\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      fprintf(fp, "     %d. %s\n",i_list+1, hg->obj[i_list].name);
    }

    fprintf (fp, "14. Special requests\n");
    if(hg->setup[hg->etc_setup].is){
      fprintf (fp, "      I2 Cell will be employed for this program.\n");
    }
    else{
      fprintf (fp, "      None\n");
    }

    fprintf(fp,"\n");

    fprintf (fp, "A. Finding Charts\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].mag>10){
	i_fc++;
      }
    }
    num_fc=i_fc;
    i_fc=0;
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].mag>10){
	i_fc++;
	fprintf(fp, "     %d. %s,  Yes,  %d of %d,  %d\'x%d\',  North is up\n",
		i_list+1, hg->obj[i_list].name,
		i_fc, num_fc, hg->dss_arcmin, hg->dss_arcmin);
      }
      else{
	fprintf(fp, "     %d. %s,  No\n",i_list+1, hg->obj[i_list].name);
      }
    }

    fclose(fp);
  }
}


void WritePROMS(typHOE *hg){
  FILE *fp;
  gint i_list;
  gchar *band_str;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  // Precheck
  for(i_list=0;i_list<hg->i_max;i_list++){
    band_str=get_band_name(hg, i_list);
    fprintf(fp,"%s\t%09.2lf\t%+09.1lf\t%.2lf(%s)\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    hg->obj[i_list].mag,
	    band_str);
    if(band_str) g_free(band_str);
  }

  fclose(fp);
}


void WritePlan(typHOE *hg){
  FILE *fp;
  int i_plan;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  fprintf (fp,     " HST   [min]  Tasks\n");
  fprintf (fp, "================================================================\n");

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].sod>0){
      fprintf (fp, " %5s [%3d]  %s\n", 
	       get_txt_tod(hg->plan[i_plan].sod), 
	       ( (hg->plan[i_plan].time+hg->plan[i_plan].stime>0) ?
		 (hg->plan[i_plan].time+hg->plan[i_plan].stime)/60 : -hg->plan[i_plan].time/60),
	       hg->plan[i_plan].txt);
    }
    else if (hg->plan[i_plan].time!=0){
      fprintf (fp, "       [%3d]  %s\n", 
	       ( ((hg->plan[i_plan].time+hg->plan[i_plan].stime)>0) ?
		 (hg->plan[i_plan].time+hg->plan[i_plan].stime)/60 : -hg->plan[i_plan].time/60),
	       hg->plan[i_plan].txt);
    }
    else{
      if(hg->plan[i_plan].type==PLAN_TYPE_COMMENT){
	fprintf (fp, "\n              %s\n\n", hg->plan[i_plan].txt);
      }
      else{
	fprintf (fp, "              %s\n", hg->plan[i_plan].txt);
      }
    }
  }

  fclose(fp);

  
}


void WriteOPE_BIAS(FILE *fp){
  fprintf(fp, "### BIAS\n");
  fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
  fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS NFILES=4\n");

  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void WriteOPE_BIAS_plan(FILE *fp, PLANpara plan){
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(plan.repeat>1){
    fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
    fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS NFILES=%d\n",plan.repeat-1);
  }
  else{
    fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
  }
  
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void WriteYAML_BIAS_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : bias\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", HDS_setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  fprintf(fp, ", exptime:0, repeat: %d}\n",plan.repeat);
}


void WriteOPE_SetUp_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(plan.cmode==PLAN_CMODE_1ST){
    fprintf(fp, "### 1st setup change for the night ###\n");
  }

  switch(hg->setup[plan.setup].is){
  case IS_030X5:
    fprintf(fp, "### w/Image Slicer #1 (0.30x5) ###\n");
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=30000 SLIT_WIDTH=2000\n");
    break;
  case IS_045X3:
    fprintf(fp, "### w/Image Slicer #2 (0.45x3) ###\n");
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=30000 SLIT_WIDTH=2000\n");
    break;
  case IS_020X3:
    fprintf(fp, "### w/Image Slicer #3 (0.20x3) ###\n");
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=30000 SLIT_WIDTH=2000\n");
    break;
  default:
    if(plan.slit_or){
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	      plan.slit_length,plan.slit_width);
    }
    else{
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	      hg->setup[plan.setup].slit_length,
	      hg->setup[plan.setup].slit_width);
    }
    break;
  }




  if(hg->setup[plan.setup].setup<0){ // NonStd
    if(plan.cmode==PLAN_CMODE_1ST){
      fprintf(fp, "# Color Change, if you need. (Please check the current setup)\n");
    }

    i_set=-hg->setup[plan.setup].setup-1;
    if((plan.cmode==PLAN_CMODE_FULL)||(plan.cmode==PLAN_CMODE_1ST)){
      if(hg->nonstd[i_set].col==COL_BLUE){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Blue CROSS_SCAN=%d COLLIMATOR=Blue $CAMZ_B",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		hg->nonstd[i_set].cross);
	if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	  fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	}
	if(hg->nonstd[i_set].camr!=-3600){
	  fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	}
      }
      else{
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Red CROSS_SCAN=%d COLLIMATOR=Red $CAMZ_R",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		hg->nonstd[i_set].cross);
	if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	  fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	}
	if(hg->nonstd[i_set].camr!=-3600){
	  fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	}
      }
    }

    if(plan.cmode==PLAN_CMODE_1ST){
      fprintf(fp, "\n# w/o Color Change. (Please check the current setup)\n");
    }

    if((plan.cmode==PLAN_CMODE_EASY)||(plan.cmode==PLAN_CMODE_1ST)){
      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d",
	      hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
	      hg->nonstd[i_set].cross);
    }
    fprintf(fp, "\n");
  }
  else{ //Std
    if(plan.cmode==PLAN_CMODE_1ST){
      fprintf(fp, "# Color Change, if you need. (Please check the current setup)\n");
    }
    
    i_set=hg->setup[plan.setup].setup;
    if((plan.cmode==PLAN_CMODE_FULL)||(plan.cmode==PLAN_CMODE_1ST)){
      if(i_set<StdI2b){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_B\n",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		HDS_setups[i_set].cross,HDS_setups[i_set].initial,HDS_setups[i_set].col);
      }
      else{
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_R\n",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		HDS_setups[i_set].cross,HDS_setups[i_set].initial,HDS_setups[i_set].col);
      }
    }

    if(plan.cmode==PLAN_CMODE_1ST){
      fprintf(fp, "# w/o Color Change. (Please check the current setup)\n");
    }
    
    if((plan.cmode==PLAN_CMODE_EASY)||(plan.cmode==PLAN_CMODE_1ST)){
      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS_SCAN=Std%s\n",
	      hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
	      HDS_setups[i_set].initial);
    }
  }

  
  if(plan.cmode==PLAN_CMODE_1ST){
    fprintf(fp, "\n### Check Collimator Position\n");
    fprintf(fp, "# Please check collimator position in UI  ===> %+.3lf(+-0.05)V\n",
	    plan.colv);
    fprintf(fp, "#   Use \"hds_col\" in Pane-9 of OBCP tmux.\n");
  }
  else{
    if(plan.colinc!=0){
      fprintf(fp, "\n### Change Collimator Position\n");
      fprintf(fp, "# Please change collimator position value in UI : inc=%+d  ===> %+.3lf(+-0.05)V\n",
	      plan.colinc, plan.colv);
      fprintf(fp, "#   Use \"hds_col\" in Pane-9 of OBCP tmux.\n");
    }
    
    if(plan.is_change){
      switch(hg->setup[plan.setup].is){
      case IS_030X5:
	fprintf(fp, "\n### Attach Image Slicer #1 (0.30x5) in NsOpt (~10min)\n");
	break;
	
      case IS_045X3:
	fprintf(fp, "\n### Attach Image Slicer #2 (0.45x3) in NsOpt (~10min)\n");
	break;
	
      case IS_020X3:
	fprintf(fp, "\n### Attach Image Slicer #3 (0.20x3) in NsOpt (~10min)\n");
	break;

      default:
	fprintf(fp, "\n### Detach Image Slicer in NsOpt (~10min)\n");
	break;
      }
      
      if(plan.bin_change){
	fprintf(fp, "\n### Change CCD binning --> %dx%d\n",
		hg->binning[hg->setup[plan.setup].binning].x,
		hg->binning[hg->setup[plan.setup].binning].y);
      }
    }
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");

}

void WriteYAML_SetUp_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : setup\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", HDS_setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }

  if(plan.cmode==PLAN_CMODE_FULL){
    fprintf(fp, ", change: full}\n");
  }
  else if(plan.cmode==PLAN_CMODE_EASY){
    fprintf(fp, ", change: cross}\n");
  }
  else{
    fprintf(fp, ", change: slit}\n");
  }

}


void WriteOPE_COMP(FILE *fp, typHOE *hg){
  gint i_use;

  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      {
	gint i_bin;
	i_bin=hg->setup[i_use].binning;
	if(hg->setup[i_use].setup < 0){ // NonStd
	  fprintf(fp, "### COMPARISON  for NonStd-%d  %dx%dBINNING\n", 
		  -hg->setup[i_use].setup,hg->binning[i_bin].x, hg->binning[i_bin].y);
	}
	else{ // Std
	  fprintf(fp, "### COMPARISON  for Std%s  %dx%dBINNING\n", 
		  HDS_setups[hg->setup[i_use].setup].initial,hg->binning[i_bin].x, hg->binning[i_bin].y);
	}
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_width : 2000);
	fprintf(fp, "# Turn ON HCT1 from TWS :  15mA  Filter=1-1-1-1\n");
	fprintf(fp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
		20/hg->binning[i_bin].x/hg->binning[i_bin].y);
	fprintf(fp, "\n");
      }
    }
  }
}


void WriteOPE_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  if(plan.slit_or){
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	    plan.slit_length,
	    plan.slit_width);
  }
  else{
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000,
	    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_width : 2000);
  }
  
  fprintf(fp, "# Turn ON HCT1 from TWS :  15mA  Filter=1-1-1-1\n");
  fprintf(fp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
	  20/hg->binning[hg->setup[plan.setup].binning].x/hg->binning[hg->setup[plan.setup].binning].y);
  if((plan.daytime) 
     && (hg->setup[plan.setup].i2) 
     && (hg->setup[plan.setup].is == IS_NO)){
    fprintf(fp, "# w/ 0.2 arcsec slit\n");
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
    fprintf(fp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
	    20/hg->binning[hg->setup[plan.setup].binning].x/hg->binning[hg->setup[plan.setup].binning].y);
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	    hg->setup[plan.setup].slit_length,
	    hg->setup[plan.setup].slit_width);
  }
  fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT Retract/LampOff #####\n\n\n");
  
}


void WriteYAML_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  fprintf(fp, "   - type   : comp\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", HDS_setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d}\n",
	    plan.slit_width, plan.slit_length);
  }
  else{
    fprintf(fp,"}\n");
  }

}

void WriteOPE_FOCUS_plan(FILE *fp, PLANpara plan){
  gdouble z=0.0;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  switch(plan.focus_mode){
  case PLAN_FOCUS1:
    switch(plan.focus_is){
    case IS_030X5:
    case IS_045X3:
      z+=0.45;
      break;
      
    case IS_020X3:
      z+=0.60;
      break;
      
    default:
      break;
    }
    
    if(plan.i2_pos==PLAN_I2_IN){
      z+=0.15;
    }

    fprintf(fp, "# [Launcher/Telescope2] Move Telescope Focus  z=%+.2lf\n",z);
  
    fprintf(fp, "# [Launcher/HDS] FocusSVSequence ");
    if(plan.focus_is==IS_NO){
      fprintf(fp, "  (set Slit Length ===> \"30\")\n");
      fprintf(fp, "# [Launcher/HDS] Set Seeing for Slit Guide\n");
    }
    else{
      fprintf(fp, "  (set Slit Length ===> \"-1\")\n");
    }
    break;
  default:
    fprintf(fp, "# [Launcher/HDS] FocusAGSequence \n");
  }
  fprintf(fp,"\n");
}


void WriteOPE_COMMENT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  switch(plan.comtype){
  case PLAN_COMMENT_TEXT:
    if(plan.comment){
      fprintf(fp, "###### %s #####\n\n", plan.comment);
    }
    else{
      fprintf(fp, "######\n\n");
    }
    break;

  case PLAN_COMMENT_SUNSET:
  case PLAN_COMMENT_SUNRISE:
    fprintf(fp, "%s\n\n", plan.txt);
    break;
  }
}


void WriteOPE_OBJ_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_repeat, i_set;
  gchar *tgt;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(hg->obj[plan.obj_i].note)   fprintf(fp, "# %s\n", hg->obj[plan.obj_i].note);
  if(plan.omode!=PLAN_OMODE_GET){
    /// SetupField ///
    fprintf(fp,"SetupField");
    
    switch(plan.guide){
    case NO_GUIDE:
      fprintf(fp," Guide=No");
      break;
    case AG_GUIDE:
      fprintf(fp," Guide=AG");
      break;
    case SV_GUIDE:
      fprintf(fp," Guide=SV Mode=SemiAuto");
      break;
    case SVSAFE_GUIDE:
      fprintf(fp," Guide=SV Mode=Safe");
      break;
    }

    if(hg->obj[plan.obj_i].i_nst<0){
      tgt=make_tgt(hg->obj[plan.obj_i].name, "TGT_");
      if((fabs(hg->obj[plan.obj_i].pm_ra)>100)
	 ||(fabs(hg->obj[plan.obj_i].pm_dec)>100)){
	fprintf(fp, " $DEF_PROTO $PM%s", tgt);
      }
      else{
	fprintf(fp, " $DEF_PROTO $%s", tgt);
      }
      g_free(tgt);
    }
    else{
      fprintf(fp, 
	      " $DEF_PROTO OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
	      hg->obj[plan.obj_i].name,
	      g_path_get_basename(hg->nst[hg->obj[plan.obj_i].i_nst].filename));
    }
    
    switch(plan.guide){
    case NO_GUIDE:
    case AG_GUIDE:
      fprintf(fp, " SVRegion=%d",
	      hg->sv_region);
      break;
    case SV_GUIDE:
    case SVSAFE_GUIDE:
      fprintf(fp," ReadRegion=%d CalcRegion=%d",
	      hg->sv_region,
	      hg->sv_calc);
      break;
    }
    
    if(plan.sv_or)
      fprintf(fp, " Exptime_SV=%d", plan.sv_exp);
    else
      fprintf(fp, " Exptime_SV=%d", hg->exptime_sv);
    
    if(plan.sv_fil!=SV_FILTER_NONE)
      fprintf(fp, " SV_FILTER01=%d", plan.sv_fil);
    
    switch(hg->setup[plan.setup].imr){
    case IMR_NO:
      fprintf(fp," IMGROT_FLAG=0");
      break;
    case IMR_LINK:
      if(plan.pa_or)
	fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f", plan.pa);
      else
	fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f", hg->obj[plan.obj_i].pa);
      break;
    case IMR_ZENITH:
      fprintf(fp," IMGROT_FLAG=1 IMGROT_MODE=ZENITH");
      break;
    }   

    if(hg->setup[plan.setup].is!=IS_NO){
      fprintf(fp," IS_FLAG=1");
      if(hg->setup[plan.setup].is==IS_020X3){
	fprintf(fp, " $SV_IS3_X $SV_IS3_Y $G_PARA");
      }
      else{
	fprintf(fp, " $SV_IS_X $SV_IS_Y $G_PARA");
      }
    }
    else{
      if(plan.slit_or){
	fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		plan.slit_length,
		plan.slit_width);
      }
      else{
	fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		hg->setup[plan.setup].slit_length,
		hg->setup[plan.setup].slit_width);
      }
      fprintf(fp, " $SV_X $SV_Y $G_PARA");
    }
    
    
    
    switch(plan.guide){
    case NO_GUIDE:
    case AG_GUIDE:
      fprintf(fp,"\n");
      break;
    case SV_GUIDE:
    case SVSAFE_GUIDE:
      if(hg->sv_area==SV_FULL){
	fprintf(fp, " ReadArea=Full\n");
      }
      else{
	fprintf(fp, " ReadArea=Part\n");
      }
      break;
    }
  }

	  
  if(plan.omode!=PLAN_OMODE_SET){
    for(i_repeat=0;i_repeat<plan.repeat;i_repeat++){
      fprintf(fp,"GetObject");
      if(hg->setup[plan.setup].is!=IS_NO){
	fprintf(fp," IS_FLAG=1");
	if(hg->setup[plan.setup].is==IS_020X3){
	  fprintf(fp," IS_Z_OFFSET=-0.40");
	}
      }
      if(hg->obj[plan.obj_i].i_nst<0){
	tgt=make_tgt(hg->obj[plan.obj_i].name, "TGT_");
	if((fabs(hg->obj[plan.obj_i].pm_ra)>100)
	   ||(fabs(hg->obj[plan.obj_i].pm_dec)>100)){
	  fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $PM%s\n",
		  plan.exp, hg->sv_integrate, tgt);
	}
	else{
	  fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
		  plan.exp, hg->sv_integrate, tgt);
	}
	g_free(tgt);
      }
      else{
	fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
		plan.exp, hg->sv_integrate, hg->obj[plan.obj_i].name);
      }
    }
  }
  fprintf(fp, "\n");
}


void WriteYAML_OBJ_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_repeat, i_set;
  gchar *tgt;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(hg->obj[plan.obj_i].note)   fprintf(fp, "# %s\n", hg->obj[plan.obj_i].note);
  fprintf(fp, "   - type   : obj\n");

  // Obj
  tgt=make_tgt(hg->obj[plan.obj_i].name, "TGT_");
  fprintf(fp, "     obj    : *%s\n", tgt);
  g_free(tgt);

  // Instrument
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", HDS_setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }

  fprintf(fp,", exptime: %d, repeat: %d}\n", plan.exp, plan.repeat);
  

  // Telescope
  switch(plan.guide){
  case NO_GUIDE:
    fprintf(fp, "     tel: { <<: *t_no");
    break;
  case AG_GUIDE:
    fprintf(fp, "     tel: { <<: *t_ag");
    break;
  case SV_GUIDE:
    fprintf(fp, "     tel: { <<: *t_sv_semiauto");
    break;
  case SVSAFE_GUIDE:
    fprintf(fp, "     tel: { <<: *t_sv_safe");
    break;
  }

  if(plan.sv_or)
    fprintf(fp, ", sv_exp: %d", plan.sv_exp);

  if(plan.sv_fil!=SV_FILTER_NONE)
    fprintf(fp, ", sv_filter: %d", plan.sv_fil);

  switch(hg->setup[plan.setup].imr){
  case IMR_LINK:
    if(plan.pa_or)
      fprintf(fp,", imr: link, pa:%.1f}\n", plan.pa);
    else
      fprintf(fp,", imr: link, pa:%.1f}\n", hg->obj[plan.obj_i].pa);
    break;
  case IMR_ZENITH:
    fprintf(fp,", imr: zenith}\n");
    break;
  default:
    fprintf(fp,"}\n");
  }   
}


void WriteOPE_FLAT(FILE *fp, typHOE *hg){
  gint i_set,j_set,i_use,i_bin;
  int nonstd_flat;
  gint sl, sw;
  gboolean same_rb, is_flag, i2_flag;
  gdouble fl_factor;

  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){

      if(hg->setup[i_use].is == IS_NO){ // Slit or IS
	is_flag=FALSE;
	sw = 200;
	sl = hg->setup[i_use].slit_length;
	fl_factor=1.0;
      }
      else{
	is_flag=TRUE;
	sw = 2000;
	sl = 30000;
	if(hg->setup[i_use].is == IS_020X3){
	  fl_factor=IS_FLAT_FACTOR*2.0;
	}
	else{
	  fl_factor=IS_FLAT_FACTOR;
	}
      }
      i2_flag=hg->setup[i_use].i2;
      i_bin=hg->setup[i_use].binning;

      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;

	nonstd_flat=get_nonstd_flat(hg->nonstd[i_set].col, hg->nonstd[i_set].cross); 

	same_rb=get_same_rb(nonstd_flat);
	
	if(same_rb){  //Same Setup for Blue & Red
	  fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
		  i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  if(!is_flag){
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d \n\n", sl, sw);
	  }
	  fprintf(fp, "# CCD1 and 2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		  i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[nonstd_flat].f1_amp,  
		  HDS_setups[nonstd_flat].f1_fil1, HDS_setups[nonstd_flat].f1_fil2,
		  HDS_setups[nonstd_flat].f1_fil3, HDS_setups[nonstd_flat].f1_fil4);
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  
	  if(i2_flag){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  
	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",sl);
	  fprintf(fp, "\n");
	  fprintf(fp, "\n");
	}
	else{  // NonStd and Different Red/Blue Flat
	  fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
		  i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  if(!is_flag){
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n\n", sl, sw);
	  }
	  
	  // RED FLAT
	  fprintf(fp, "# CCD1 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		  i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[nonstd_flat].f1_amp,  
		  HDS_setups[nonstd_flat].f1_fil1, HDS_setups[nonstd_flat].f1_fil2,
		  HDS_setups[nonstd_flat].f1_fil3, HDS_setups[nonstd_flat].f1_fil4);
	  
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  if((i2_flag)&&(nonstd_flat<StdI2a)){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  
	  fprintf(fp, "\n");
	  
	  // BLUE FLAT
	  fprintf(fp, "# CCD2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		  i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[nonstd_flat].f2_amp,  
		  HDS_setups[nonstd_flat].f2_fil1, HDS_setups[nonstd_flat].f2_fil2,
		  HDS_setups[nonstd_flat].f2_fil3, HDS_setups[nonstd_flat].f2_fil4);
	  if((nonstd_flat==StdUa)||(nonstd_flat==StdUb)){
	    fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	  }
	  
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  
	  if((i2_flag)&&(nonstd_flat>=StdI2a)){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  
	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",sl);
	  
	  fprintf(fp, "\n");
	  fprintf(fp, "\n");
	}
      }
      else{ //Std Setup
	i_set=hg->setup[i_use].setup;

	same_rb=get_same_rb(i_set);

	if(same_rb){  //Same setup for Blue and Red
	  fprintf(fp, "## FLAT  %s %dx%d\n",
		  HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  if(!is_flag){ // Slit
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n\n", sl ,sw);
	  }
	  fprintf(fp, "# CCD1 and 2 Flat for %s (%dx%dbinning)\n",
		  HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[i_set].f1_amp,  
		  HDS_setups[i_set].f1_fil1, HDS_setups[i_set].f1_fil2,
		  HDS_setups[i_set].f1_fil3, HDS_setups[i_set].f1_fil4);
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  if(i2_flag){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }

	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",sl);
	  
	  fprintf(fp, "\n");
	  fprintf(fp, "\n");
	}
	else{  // Std : Different Setup for Blue & Red
	  fprintf(fp, "## FLAT  %s %dx%d\n",
		  HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  if(!is_flag){
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n\n", sl ,sw);
	  }
	  
	  // RED
	  fprintf(fp, "# CCD1 Flat for %s (%dx%dbinning)\n",
		  HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[i_set].f1_amp,  
		  HDS_setups[i_set].f1_fil1, HDS_setups[i_set].f1_fil2,
		  HDS_setups[i_set].f1_fil3, HDS_setups[i_set].f1_fil4);
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  if((i2_flag) && (i_set<StdI2a)){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  
	  fprintf(fp, "\n");
	  
	  // BLUE
	  fprintf(fp, "# CCD2 Flat for %s (%dx%dbinning)\n",
		  HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	  fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
		  HDS_setups[i_set].f2_amp,  
		  HDS_setups[i_set].f2_fil1, HDS_setups[i_set].f2_fil2,
		  HDS_setups[i_set].f2_fil3, HDS_setups[i_set].f2_fil4);
	  if((i_set==StdUa)||(i_set==StdUb)){
	    fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	  }
	  
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  if((i2_flag) && (i_set>=StdI2a)){
	    fprintf(fp, "\n");
	    fprintf(fp, "# Flat w/I2\n");
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    if(!is_flag){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }

	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",sl);
	  
	  if((i_set==StdUa)||(i_set==StdUb)){
	    fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=Free\n");
	  }
	  fprintf(fp, "\n");
	  fprintf(fp, "\n");
	}
      }
    }
  }
}


void WriteOPE_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set,j_set,i_bin;
  int nonstd_flat;
  gboolean is_flag,i2_flag,same_rb;
  gdouble fl_factor;
  gint sl,sw;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  if(hg->setup[plan.setup].is == IS_NO){ // Slit or IS
    is_flag=FALSE;
    sw = 200;
    if(plan.slit_or){
      sl = plan.slit_length;
    }
    else{
      sl = hg->setup[plan.setup].slit_length;
    }
    fl_factor=1.0;
  }
  else{
    is_flag=TRUE;
    sw = 2000;
    sl = 30000;
    if(hg->setup[plan.setup].is == IS_020X3){
      fl_factor=IS_FLAT_FACTOR*2.0;
    }
    else{
      fl_factor=IS_FLAT_FACTOR;
    }
  }
  i2_flag=hg->setup[plan.setup].i2;
  i_bin=hg->setup[plan.setup].binning;

  if(!is_flag){
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n\n",sl,sw);
  }
   
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;

    nonstd_flat=get_nonstd_flat(hg->nonstd[i_set].col, hg->nonstd[i_set].cross); 
    
    same_rb=get_same_rb(nonstd_flat);
    
    if(same_rb){  // NonStd : Same Setup for Blue & Red
      fprintf(fp, "# CCD1 and 2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
	      i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[nonstd_flat].f1_amp,  
	      HDS_setups[nonstd_flat].f1_fil1, HDS_setups[nonstd_flat].f1_fil2,
	      HDS_setups[nonstd_flat].f1_fil3, HDS_setups[nonstd_flat].f1_fil4);
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat>0){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if(i2_flag){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      fprintf(fp, "#  for order trace\n");
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",sl);
    }
    else{  // NonStd : Different Red/Blue Flat
      // RED
      fprintf(fp, "# CCD1 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
	      i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[nonstd_flat].f1_amp,  
	      HDS_setups[nonstd_flat].f1_fil1, HDS_setups[nonstd_flat].f1_fil2,
	      HDS_setups[nonstd_flat].f1_fil3, HDS_setups[nonstd_flat].f1_fil4);
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat>0){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if((i2_flag) && (nonstd_flat<StdI2a)){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      
      // BLUE
      fprintf(fp, "# CCD2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
	      i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,HDS_setups[nonstd_flat].initial);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[nonstd_flat].f2_amp,  
	      HDS_setups[nonstd_flat].f2_fil1, HDS_setups[nonstd_flat].f2_fil2,
	      HDS_setups[nonstd_flat].f2_fil3, HDS_setups[nonstd_flat].f2_fil4);
      if((nonstd_flat==StdUa)||(nonstd_flat==StdUb)){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
      }
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat>0){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if((i2_flag) && (nonstd_flat>=StdI2a)){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		(guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      fprintf(fp, "#  for order trace\n");
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",sl);
    }
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;

    same_rb=get_same_rb(i_set);

    if(same_rb){ // Std : Same setup for Blue and Red
      fprintf(fp, "# CCD1 and 2 Flat for %s (%dx%dbinning)\n",
	      HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[i_set].f1_amp,  
	      HDS_setups[i_set].f1_fil1, HDS_setups[i_set].f1_fil2,
	      HDS_setups[i_set].f1_fil3, HDS_setups[i_set].f1_fil4);
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if(i2_flag){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      fprintf(fp, "#  for order trace\n");
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",sl);
    }
    else{  // Std : Different Setup for Blue & Red
      // RED
      fprintf(fp, "# CCD1 Flat for %s (%dx%dbinning)\n",
	      HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[i_set].f1_amp,  
	      HDS_setups[i_set].f1_fil1, HDS_setups[i_set].f1_fil2,
	      HDS_setups[i_set].f1_fil3, HDS_setups[i_set].f1_fil4);
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat>0){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if((i2_flag) && (i_set<StdI2a)){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      
      // BLUE
      fprintf(fp, "# CCD2 Flat for %s (%dx%dbinning)\n",
	      HDS_setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
      fprintf(fp, "# Turn ON HAL(other) from TWS : %sA  Filter=%d-%d-%d-%d\n",
	      HDS_setups[i_set].f2_amp,  
	      HDS_setups[i_set].f2_fil1, HDS_setups[i_set].f2_fil2,
	      HDS_setups[i_set].f2_fil3, HDS_setups[i_set].f2_fil4);
      if((i_set==StdUa)||(i_set==StdUb)){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
      }
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      if(plan.repeat>0){
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
      }
      
      if((i2_flag) && (i_set>=StdI2a)){
	fprintf(fp, "\n");
	fprintf(fp, "# Flat w/I2\n");
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		(guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	if(!is_flag){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=100\n");
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		  (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y*2.));
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200\n");
	}
	fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
      }
      
      fprintf(fp, "\n");
      fprintf(fp, "#  for order trace\n");
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
	      (guint)(fl_factor*(gdouble)HDS_setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",sl);
      
      if((i_set==StdUa)||(i_set==StdUb)){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=Free\n");
      }
    }
  }
  fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL Retract/LampOff #####\n\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

}


void WriteYAML_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set,j_set;
  int nonstd_flat;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : flat\n");
  
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", HDS_setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }
  else{
    fprintf(fp,", slit_width: 200");
  }
  
  fprintf(fp,", repeat: %d}\n", plan.repeat);
  
}


#ifdef USE_SSL
void HDS_DownloadLOG(typHOE *hg){
  gint ans=0;

  if((ans=scp_get(hg))<0){
    switch(ans){
    case -1:
      break;
    
    case -2:
      break;
    
    case -3:
      break;

    default:
      break;
    }
  }
}  
#endif

gboolean hds_svcmag (typHOE *hg, gint mode)
{
  GtkWidget *dialog, *frame, *label, *spinner, *button, *hbox, *bar;
  GtkAdjustment *adj;
  GSList *group;
  gchar *tmp;


  if(hg->obj[hg->etc_i].mag>99){
    tmp=g_strdup_printf("Magnitude of \"<b>%s</b>\" is not found in your Main Target list.",
			hg->obj[hg->etc_i].name);
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  "Please input or search magnitude of this target.",
		  NULL);
    g_free(tmp);
  }

  hg->svcmag_type=-1;

  dialog = gtk_dialog_new_with_buttons("HOE : Search Target\'s Magnitude",
				       GTK_WINDOW(hg->plan_main),
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
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);

  if(mode==ETC_SERVICE){
    tmp=g_strdup_printf("<b>[Plan #%d]</b>", hg->etc_i_plan);
    label = gtkut_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);
  }
  
  tmp=g_strdup_printf("<b> Target-%d : %s</b>", hg->etc_i+1, hg->obj[hg->etc_i].name);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  if(hg->obj[hg->etc_i].note){
    tmp=g_strdup_printf("    %s", hg->obj[hg->etc_i].note);
    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);
  }

  label = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  label = gtk_label_new (" mag = ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  hg->svcmag_adj = (GtkAdjustment *)gtk_adjustment_new(hg->obj[hg->etc_i].mag,
						       -1.0,
						       100,
						       0.01, 0.1, 0);
  my_signal_connect (hg->svcmag_adj, "value_changed",
		     cc_get_adj_double,
		     &hg->obj[hg->etc_i].mag);
  spinner =  gtk_spin_button_new (hg->svcmag_adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("SIMBAD V", "edit-find");
#else
  button=gtkut_button_new_from_stock("SIMBAD V", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", svcmag_simbad_query, (gpointer)hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("PanSTARRS g", "edit-find");
#else
  button=gtkut_button_new_from_stock("PanSTARRS g", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", svcmag_ps1_query, (gpointer)hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("GAIA G", "edit-find");
#else
  button=gtkut_button_new_from_stock("GAIA G", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", svcmag_gaia_query, (gpointer)hg);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  hg->svcmag_label = gtk_label_new ("(Input or Search trget\'s magnitude)");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->svcmag_label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (hg->svcmag_label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(hg->svcmag_label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->svcmag_label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->svcmag_label,TRUE, TRUE, 0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  label = gtk_label_new ("    Search radius ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->magdb_arcsec,
					    3,
					    60,
					    1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->magdb_arcsec);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("arcsec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    switch(hg->svcmag_type){
    case MAGDB_TYPE_SIMBAD:
      if(fabs(hg->obj[hg->etc_i].mag-hg->obj[hg->etc_i].magdb_simbad_v) > 0.01){
	hg->obj[hg->etc_i].magdb_used=0;
      }
      break;
      
    case MAGDB_TYPE_PS1:
      if(fabs(hg->obj[hg->etc_i].mag-hg->obj[hg->etc_i].magdb_ps1_g) > 0.01){
	hg->obj[hg->etc_i].magdb_used=0;
      }
      break;
      
    case MAGDB_TYPE_GAIA:
      if(fabs(hg->obj[hg->etc_i].mag-hg->obj[hg->etc_i].magdb_gaia_g) > 0.01){
	hg->obj[hg->etc_i].magdb_used=0;
      }
      break;

    default:
      hg->obj[hg->etc_i].magdb_used=0;
      break;
    }
    
    if(hg->obj[hg->etc_i].mag>99){
      popup_message((mode==ETC_SERVICE) ? hg->plan_main : hg->w_top,
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "Effective magnitude is not found for this target.",
		    NULL);
      return(FALSE);
    }
    else{
      return(TRUE);
    }
  }
  else{
    gtk_widget_destroy(dialog);
    return(FALSE);
  }
}


gint hds_select_etc_am (typHOE *hg, gboolean auto_flag)
{
  GtkWidget *dialog, *frame, *label;
  GtkWidget *rb[NUM_HDS_ETC_AM];
  GSList *group;
  gint ret=HDS_ETC_AUTO;
  gchar *tmp;

  dialog = gtk_dialog_new_with_buttons("HOE : Automatic / Manual?",
				       GTK_WINDOW(hg->plan_main),
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
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  
  tmp=g_strdup_printf("<b>[Plan #%d]</b>", hg->etc_i_plan);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  tmp=g_strdup_printf("<b> Target-%d : %s</b>", hg->etc_i+1, hg->obj[hg->etc_i].name);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  if(hg->obj[hg->etc_i].note){
    tmp=g_strdup_printf("    %s", hg->obj[hg->etc_i].note);
    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);
  }

  label = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("Please calculate S/N for your plan using ETC.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("If your obs is not continuum based, just input acceptable S/N manually.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  rb[HDS_ETC_AUTO] 
    = gtk_radio_button_new_with_label_from_widget (NULL, "Automatic S/N calculation for continuum based observation.");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     rb[HDS_ETC_AUTO],FALSE, FALSE, 0);
  my_signal_connect (rb[HDS_ETC_AUTO], "toggled", cc_radio, &ret);

  rb[HDS_ETC_MANUAL] 
    = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(rb[HDS_ETC_AUTO]),
						   "Manual S/N input for emission line observation etc.");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     rb[HDS_ETC_MANUAL],FALSE, FALSE, 0);
  my_signal_connect (rb[HDS_ETC_MANUAL], "toggled", cc_radio, &ret);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[HDS_ETC_AUTO]));
  
  gtk_widget_show_all(dialog);
    
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return(ret);
  }
  else{
    gtk_widget_destroy(dialog);
    return(-1);
  }
}


gboolean hds_input_tsnr (typHOE *hg, gboolean auto_flag)
{
  GtkWidget *dialog, *frame, *label;
  GtkWidget *hbox, *entry, *table;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  GSList *group;
  gchar *tmp;
  gint i_list;
  gchar *str=NULL;
  gboolean skip_flag=FALSE;
  gdouble old_seeing, old_z;

  dialog = gtk_dialog_new_with_buttons("HOE : Input Target S/N",
				       GTK_WINDOW(hg->plan_main),
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
  
  tmp=g_strdup_printf("<b>[Plan #%d]</b>", hg->etc_i_plan);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  tmp=g_strdup_printf("<b> Target-%d : %s</b>", hg->etc_i+1, hg->obj[hg->etc_i].name);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  if(hg->obj[hg->etc_i].note){
    tmp=g_strdup_printf("    %s", hg->obj[hg->etc_i].note);
    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);
  }

  label = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  if(auto_flag){
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       hbox,FALSE, FALSE, 0);

    tmp=g_strdup_printf("   Expected (ideal) S/N = %.0lf",
			hg->plan[hg->etc_i_plan].snr*sqrt(hg->plan[hg->etc_i_plan].repeat));
    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),
		       label,FALSE, FALSE, 0);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       hbox,FALSE, FALSE, 0);
    
    label = gtk_label_new ("   Input acceptable S/N = ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),
		       label,FALSE, FALSE, 0);

    if(hg->plan[hg->etc_i_plan].tsnr<0){
      hg->plan[hg->etc_i_plan].tsnr=0.7*hg->plan[hg->etc_i_plan].snr
	*sqrt(hg->plan[hg->etc_i_plan].repeat);
    }
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[hg->etc_i_plan].tsnr,
					      3.0,
					      hg->plan[hg->etc_i_plan].snr*sqrt(hg->plan[hg->etc_i_plan].repeat),
					      1, 10, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan[hg->etc_i_plan].tsnr);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  }
  else{
    table = gtkut_table_new(4, 2, FALSE, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       table,FALSE, FALSE, 0);

    label = gtk_label_new ("   Input acceptable S/N = ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 0, 1, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);

    if(hg->plan[hg->etc_i_plan].tsnr<0){
      hg->plan[hg->etc_i_plan].tsnr=10;
    }
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[hg->etc_i_plan].tsnr,
					      3.0,
					      999.0,
					      1, 10, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan[hg->etc_i_plan].tsnr);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);

    label = gtk_label_new ("   at ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 2, 3, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
    
    entry = gtk_entry_new ();
    gtkut_table_attach(table, entry, 3, 4, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
    gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
    my_entry_set_width_chars(GTK_ENTRY(entry),20);
    my_signal_connect (entry,
		       "changed",
		       cc_get_entry,
		       &hg->plan[hg->etc_i_plan].wavec);
    if(hg->plan[hg->etc_i_plan].wavec){
      gtk_entry_set_text(GTK_ENTRY(entry),hg->plan[hg->etc_i_plan].wavec);
    }
    
    label = gtk_label_new ("(ex. 5500A, Halpha ...)");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 3, 4, 1, 2,
		       GTK_FILL,GTK_FILL,0,0);
    
  }
    
  gtk_widget_show_all(dialog);
    
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return(TRUE);
  }
  else{
    gtk_widget_destroy(dialog);
    hg->plan[hg->etc_i_plan].tsnr=-1;
    return(FALSE);
  }
}


gboolean hds_do_etc (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *frame, *label, *button;
  GtkWidget *hbox, *combo, *entry, *table, *check;
  GtkWidget *fdialog, *spinner;
  GtkWidget *rb[ETC_SPEC_NUM], *rc[ETC_WAVE_NUM];
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg;
  gchar *tmp;
  gint i_list;
  gchar *str=NULL;
  gboolean skip_flag=FALSE;
  gdouble old_seeing, old_z;

  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return(FALSE);

  old_seeing=hg->etc_seeing;
  old_z=hg->etc_z;

  dialog = gtk_dialog_new_with_buttons("HOE : Exposure Time Calculator",
				       GTK_WINDOW((hg->etc_mode==ETC_SERVICE)
						  ? hg->plan_main :hg->w_top),
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
  if(hg->etc_mode==ETC_OBJTREE){
    if(hg->obj[hg->etc_i].etc_done){ // not 1st calc.
      hg->etc_filter   =hg->obj[hg->etc_i].etc_filter;
      hg->etc_z        =hg->obj[hg->etc_i].etc_z;
      hg->etc_spek     =hg->obj[hg->etc_i].etc_spek;
      hg->etc_alpha    =hg->obj[hg->etc_i].etc_alpha; 
      hg->etc_bbtemp   =hg->obj[hg->etc_i].etc_bbtemp;
      hg->etc_sptype   =hg->obj[hg->etc_i].etc_sptype;
      
      hg->etc_adc      =hg->obj[hg->etc_i].etc_adc;
      hg->etc_imr      =hg->obj[hg->etc_i].etc_imr;
      
      hg->etc_wave     =hg->obj[hg->etc_i].etc_wave;
      hg->etc_waved    =hg->obj[hg->etc_i].etc_waved;
      //hg->etc_seeing   =hg->obj[hg->etc_i].etc_seeing;
    }
    if(hg->obj[hg->etc_i].mag<99){
      hg->etc_mag       =hg->obj[hg->etc_i].mag;
    }
  }
  
  if(hg->etc_mode==ETC_SERVICE){
    hg->etc_setup    =hg->plan[hg->etc_i_plan].setup;

    if(hg->obj[hg->etc_i].etc_done){ // not 1st calc.
      hg->etc_filter   =hg->obj[hg->etc_i].etc_filter;
      hg->etc_z        =hg->obj[hg->etc_i].etc_z;
      hg->etc_spek     =hg->obj[hg->etc_i].etc_spek;
      hg->etc_alpha    =hg->obj[hg->etc_i].etc_alpha; 
      hg->etc_bbtemp   =hg->obj[hg->etc_i].etc_bbtemp;
      hg->etc_sptype   =hg->obj[hg->etc_i].etc_sptype;
      
      hg->etc_adc      =hg->obj[hg->etc_i].etc_adc;
      hg->etc_imr      =hg->obj[hg->etc_i].etc_imr;
	
      hg->etc_wave     =hg->obj[hg->etc_i].etc_wave;
      hg->etc_waved    =hg->obj[hg->etc_i].etc_waved;
      //hg->etc_seeing   =hg->obj[hg->etc_i].etc_seeing;
    }
    else{                                 // for 1st calc using general params
      switch(hg->setup[hg->etc_setup].imr){
      case IMR_NO:
	hg->etc_adc      =ETC_ADC_IN;
	hg->etc_imr      =ETC_IMR_NO;
	break;
	
      case IMR_LINK:
	hg->etc_adc      =ETC_ADC_IN;
	hg->etc_imr      =ETC_IMR_RED;
	break;
	
      case IMR_ZENITH:
	hg->etc_adc      =ETC_ADC_OUT;
	hg->etc_imr      =ETC_IMR_BLUE;
	break;
      }
    }
    
    if(hg->obj[hg->etc_i].mag<99){
      hg->etc_mag       =hg->obj[hg->etc_i].mag;
    }
    hg->etc_exptime   =hg->plan[hg->etc_i_plan].exp;
    
    
    tmp=g_strdup_printf("<b>[Plan #%d]</b>", hg->etc_i_plan);
    label = gtkut_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);

    tmp=g_strdup_printf("<b> Target-%d : %s</b>", hg->etc_i+1, hg->obj[hg->etc_i].name);
    label = gtkut_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       label,FALSE, FALSE, 0);

    if(hg->obj[hg->etc_i].note){
      tmp=g_strdup_printf("    %s", hg->obj[hg->etc_i].note);
      label = gtk_label_new (tmp);
      g_free(tmp);
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 label,FALSE, FALSE, 0);
    }
  }

  
  frame = gtkut_frame_new ("<b>Input flux spectrum</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(2, 3, FALSE, 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode==ETC_LIST){
    label = gtk_label_new ("Assume ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  switch(hg->etc_mode){
  case ETC_OBJTREE:
  case ETC_SERVICE:
    str=get_band_name(hg, hg->etc_i);
    break;
  }
    
  if(str){
    label = gtk_label_new (str);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    g_free(str);

    label = gtk_label_new (" magnitude: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  else{
    // BAND
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gint i_band;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_band=0;i_band<BAND_NUM;i_band++){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, etc_filters[i_band],
			   1, i_band, -1);
	if(hg->etc_filter==i_band) iter_set=iter;
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->etc_filter);
    }

    switch(hg->etc_mode){
    case ETC_LIST:
      label = gtk_label_new ("-band for user defined mag.   ");
      break;

    default:
      label = gtk_label_new ("magnitude: ");
      break;
    }
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  
  switch(hg->etc_mode){
  case ETC_LIST:
    break;
    
  default:
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_mag,
					      0.0, 22.0, 0.20, 0.20, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->etc_mag);
    spinner =  gtk_spin_button_new (adj, 1, 2);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

    label = gtk_label_new ("   ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode==ETC_LIST){
    label = gtk_label_new ("All redshifts are assumed to be Zero.");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  else{
    label = gtkut_label_new ("Redshift (<i>z</i>): ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_z,
					      -0.1, 6.0, 0.1, 0.1, 0);
    spinner =  gtk_spin_button_new (adj, 1, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_POWERLAW] 
    = gtk_radio_button_new_with_label_from_widget (NULL, "Power law");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_POWERLAW], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_POWERLAW], "toggled", cc_radio, &hg->etc_spek);

  label = gtk_label_new ("   Spectral index: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_alpha,
					    -3.0, 3.0, 0.01, 0.01, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->etc_alpha);
  spinner =  gtk_spin_button_new (adj, 1, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtkut_label_new ("(S<sub>&#x3BD;</sub> &#x223C; &#x3BD;<sup>-&#x3B1;</sup>)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_BLACKBODY] 
    = gtk_radio_button_new_with_label_from_widget 
    (GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]), "Blackbody");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_BLACKBODY], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_BLACKBODY], "toggled", cc_radio, &hg->etc_spek);

  label = gtk_label_new ("   Temperature: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_bbtemp,
					    3000, 200000, 100, 100, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->etc_bbtemp);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("K");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 1, 2, 2, 3,
		   GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_TEMPLATE] 
    = gtk_radio_button_new_with_label_from_widget 
    (GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]), "Template");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_TEMPLATE], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_TEMPLATE], "toggled", cc_radio, &hg->etc_spek);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]));

  // Template
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_st;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_st=0;i_st<ST_NUM;i_st++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, etc_st_name[i_st],
			 1, i_st, -1);
      if(hg->etc_sptype==i_st) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_sptype);
  }


  frame = gtkut_frame_new ("<b>Instrument setting</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(1, 5, FALSE, 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Setup:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  if(hg->etc_mode==ETC_SERVICE){
    gchar *slit_tmp;
    
    switch(hg->setup[hg->etc_setup].is){
    case IS_NO:
      slit_tmp=g_strdup_printf("Normal Slit (Width=%.2lf\")",
			       (gdouble)hg->setup[hg->etc_setup].slit_width/500.);
      break;
      
    case IS_030X5:
      slit_tmp=g_strdup("IS#1 0\".3x5");
      break;
      
    case IS_045X3:
      slit_tmp=g_strdup("IS#2 0\".45x3");
      break;
      
    case IS_020X3:
      slit_tmp=g_strdup("IS#3 0\".2x3");
      break;
    }
    if(hg->setup[hg->etc_setup].setup<0){
      tmp=g_strdup_printf("Setup-%d : NonStd-%d  %dx%dbinning  %s",
			  hg->etc_setup+1,
			  -hg->setup[hg->etc_setup].setup,
			  hg->binning[hg->setup[hg->etc_setup].binning].x,
			  hg->binning[hg->setup[hg->etc_setup].binning].y,
			  slit_tmp);
    }
    else{
      tmp=g_strdup_printf("Setup-%d : Std%s  %dx%dbinning  %s",
			  hg->etc_setup+1,
			  HDS_setups[hg->setup[hg->etc_setup].setup].initial,
			  hg->binning[hg->setup[hg->etc_setup].binning].x,
			  hg->binning[hg->setup[hg->etc_setup].binning].y,
			  slit_tmp);
    }

    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  else{
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gchar *slit_tmp;
    int i_use;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    if(!hg->setup[hg->etc_setup].use) hg->etc_setup=0;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	switch(hg->setup[i_use].is){
	case IS_NO:
	  slit_tmp=g_strdup_printf("Normal Slit (Width=%.2lf\")",
				   (gdouble)hg->setup[i_use].slit_width/500.);
	  break;
	  
	case IS_030X5:
	    slit_tmp=g_strdup("IS#1 0\".3x5");
	    break;
	    
	case IS_045X3:
	  slit_tmp=g_strdup("IS#2 0\".45x3");
	  break;
	  
	case IS_020X3:
	  slit_tmp=g_strdup("IS#3 0\".2x3");
	  break;
	}
	if(hg->setup[i_use].setup<0){
	  tmp=g_strdup_printf("Setup-%d : NonStd-%d  %dx%dbinning  %s",
			      i_use+1,
			      -hg->setup[i_use].setup,
			      hg->binning[hg->setup[i_use].binning].x,
			      hg->binning[hg->setup[i_use].binning].y,
			      slit_tmp);
	}
	else{
	  tmp=g_strdup_printf("Setup-%d : Std%s  %dx%dbinning  %s",
			      i_use+1,
			      HDS_setups[hg->setup[i_use].setup].initial,
			      hg->binning[hg->setup[i_use].binning].x,
			      hg->binning[hg->setup[i_use].binning].y,
			      slit_tmp);
	}

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	if(hg->etc_setup==i_use) iter_set=iter;
	g_free(tmp);
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_setup);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Pre-Slit Optics:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  // ADC
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ADC In",
		       1, ETC_ADC_IN, -1);
    if(hg->etc_adc==ETC_ADC_IN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ADC Out",
		       1, ETC_ADC_OUT, -1);
    if(hg->etc_adc==ETC_ADC_OUT) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_adc);
  }

  // ImR
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No Image Rotator",
		       1, ETC_IMR_NO, -1);
    if(hg->etc_imr==ETC_IMR_NO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Blue Image Rotator",
		       1, ETC_IMR_BLUE, -1);
    if(hg->etc_imr==ETC_IMR_BLUE) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Red Image Rotator",
		       1, ETC_IMR_RED, -1);
    if(hg->etc_imr==ETC_IMR_RED) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_imr);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 2, 3,
		     GTK_FILL,GTK_FILL,0,0);

  switch(hg->etc_mode){
  case ETC_LIST:
    break;

  case ETC_SERVICE:
    tmp=g_strdup_printf("Exposure Time: %d x %d [sec]",
			hg->plan[hg->etc_i_plan].repeat,
			hg->etc_exptime);
    label = gtk_label_new (tmp);
    g_free(tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    break;
    
  default:
    label = gtk_label_new ("Exposure Time: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_exptime,
					      1, 7200, 1, 1, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->etc_exptime);
    spinner =  gtk_spin_button_new (adj, 1, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

    label = gtk_label_new ("seconds");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

    break;
  }

  frame = gtkut_frame_new ("<b>Obs Condition</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(1, 5, FALSE, 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Seeing: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_seeing,
					    0.3, 3.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->etc_seeing);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("arcsecond");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  
  if(hg->etc_mode!=ETC_MENU){
    frame = gtkut_frame_new ("<b>Wavelength for S/N Display</b>");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame, FALSE, FALSE, 0);

    table = gtkut_table_new(1, 2, FALSE, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(frame), table);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
    
    rc[ETC_WAVE_CENTER] = gtk_radio_button_new_with_label_from_widget 
      (NULL, "The 1st order of Red CCD");
    gtk_box_pack_start(GTK_BOX(hbox), rc[ETC_WAVE_CENTER], FALSE, FALSE, 0);
    my_signal_connect (rc[ETC_WAVE_CENTER], "toggled", cc_radio, &hg->etc_wave);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtkut_table_attach(table, hbox, 0, 1, 1, 2,
		       GTK_FILL,GTK_FILL,0,0);
    
    rc[ETC_WAVE_SPEC] = gtk_radio_button_new_with_label_from_widget 
      (GTK_RADIO_BUTTON(rc[ETC_WAVE_CENTER]), 
       "The order including the specified wavelength : ");
    gtk_box_pack_start(GTK_BOX(hbox), rc[ETC_WAVE_SPEC], FALSE, FALSE, 0);
    my_signal_connect (rc[ETC_WAVE_SPEC], "toggled", cc_radio, &hg->etc_wave);
    
    group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rc[ETC_WAVE_CENTER]));
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_waved,
					      3200, 9900, 1, 100, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->etc_waved);
    spinner =  gtk_spin_button_new (adj, 1, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    
    label = gtkut_label_new ("&#xC5;");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  if(hg->etc_mode==ETC_LIST){
    frame = gtkut_frame_new ("<b>Update S/N in the list</b>");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);

    table = gtkut_table_new(1, 2, FALSE, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(frame), table);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtkut_table_attach(table, hbox, 0, 1, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
    
    check = gtk_check_button_new_with_label("Skip if the object already has S/N ratio.");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 skip_flag);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &skip_flag);
  }
  
  gtk_widget_show_all(dialog);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->etc_spek]),TRUE);

  if(hg->etc_mode!=ETC_MENU){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rc[hg->etc_wave]),TRUE);
  }

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    switch(hg->etc_mode){
    case ETC_LIST:
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(skip_flag){
	  if(hg->obj[i_list].snr<0)
	    hg->obj[i_list].snr=etc_obj(hg, i_list);
	}
	else{
	  hg->obj[i_list].snr=etc_obj(hg, i_list);
	}
      }
      update_objtree(hg);
      break;

    case ETC_SERVICE:
      //hg->obj[hg->etc_i].mag       =hg->etc_mag;
      hg->obj[hg->etc_i].etc_filter=hg->etc_filter;
      hg->obj[hg->etc_i].etc_z     =hg->etc_z;
      hg->obj[hg->etc_i].etc_spek  =hg->etc_spek;
      hg->obj[hg->etc_i].etc_alpha =hg->etc_alpha;
      hg->obj[hg->etc_i].etc_bbtemp=hg->etc_bbtemp;
      hg->obj[hg->etc_i].etc_sptype=hg->etc_sptype;
      hg->obj[hg->etc_i].etc_adc   =hg->etc_adc;
      hg->obj[hg->etc_i].etc_imr   =hg->etc_imr;
      hg->obj[hg->etc_i].etc_wave  =hg->etc_wave;
      hg->obj[hg->etc_i].etc_waved =hg->etc_waved;
      hg->obj[hg->etc_i].etc_seeing=hg->etc_seeing;
      hg->obj[hg->etc_i].etc_done=TRUE;
      if(flagPlan){
	gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->adj_seeing),
				 (gdouble)hg->etc_seeing);
      }
      gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->adj_z),
			       (gdouble)hg->etc_z);
      
      hg->plan[hg->etc_i_plan].snr=etc_obj(hg, -1);
      if(hg->plan[hg->etc_i_plan].wavec) g_free(hg->plan[hg->etc_i_plan].wavec);
      switch(hg->etc_wave){
      case ETC_WAVE_CENTER:
	hg->plan[hg->etc_i_plan].wavec=g_strdup("Center");
	break;

      default:
	hg->plan[hg->etc_i_plan].wavec=g_strdup_printf("%dA",hg->etc_waved);
	break;
      }
      break;

    case ETC_OBJTREE:
      etc_main(hg);

      //hg->obj[hg->etc_i].mag       =hg->etc_mag;
      hg->obj[hg->etc_i].etc_filter=hg->etc_filter;
      hg->obj[hg->etc_i].etc_z     =hg->etc_z;
      hg->obj[hg->etc_i].etc_spek  =hg->etc_spek;
      hg->obj[hg->etc_i].etc_alpha =hg->etc_alpha;
      hg->obj[hg->etc_i].etc_bbtemp=hg->etc_bbtemp;
      hg->obj[hg->etc_i].etc_sptype=hg->etc_sptype;
      hg->obj[hg->etc_i].etc_adc   =hg->etc_adc;
      hg->obj[hg->etc_i].etc_imr   =hg->etc_imr;
      hg->obj[hg->etc_i].etc_wave  =hg->etc_wave;
      hg->obj[hg->etc_i].etc_waved =hg->etc_waved;
      hg->obj[hg->etc_i].etc_seeing=hg->etc_seeing;
      hg->obj[hg->etc_i].etc_done=TRUE;
      break;
      
    default:
      etc_main(hg);
      break;
    }
    
    return(TRUE);
  }
  else{
    gtk_widget_destroy(dialog);
    hg->etc_seeing=old_seeing;
    hg->etc_z=old_z;

    return(FALSE);
  }
}

void hds_do_etc_list (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;
  
  hg->etc_mode=ETC_LIST;
  hds_do_etc(widget,(gpointer)hg);
  hg->etc_mode=ETC_MENU;
}

void hds_do_etc_plan (GtkWidget *widget, gpointer gdata)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gboolean valid=FALSE;
  gboolean ret;
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    hg->etc_i_plan=i;
    hg->etc_i=hg->plan[i].obj_i;
    
    gtk_tree_path_free (path);

    if(hg->plan[hg->etc_i_plan].type==PLAN_TYPE_OBJ){
      if(hg->obj[hg->etc_i].mag>99){
	ret=hds_svcmag(hg, ETC_SERVICE);
	if(!ret) return;
      }
      hg->etc_mode=ETC_SERVICE;
      hds_do_etc(widget,(gpointer)hg);
      hg->etc_mode=ETC_MENU;
      tree_update_plan_item(hg, model, iter, hg->etc_i_plan);
      valid=TRUE;
    }
  }

  if(!valid){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please select an \"Object\" line in your plan.",
		  NULL);
  }
}


void hds_recalc_etc_plan (GtkWidget *widget, gpointer gdata){
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  gint i_plan;
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  hg->etc_mode=ETC_SERVICE;

  for(i_plan=0; i_plan<hg->i_plan_max; i_plan++){
    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
      if(hg->plan[i_plan].snr > 0){ // ONLY not 1st calc.
	hg->etc_i_plan = i_plan;
	hg->etc_i = hg->plan[hg->etc_i_plan].obj_i;
	hg->etc_setup    =hg->plan[hg->etc_i_plan].setup;
	
        hg->etc_mag      =hg->obj[hg->etc_i].mag;
	hg->etc_filter   =hg->obj[hg->etc_i].etc_filter;
     
	hg->etc_z        =hg->obj[hg->etc_i].etc_z;
	hg->etc_spek     =hg->obj[hg->etc_i].etc_spek;
	hg->etc_alpha    =hg->obj[hg->etc_i].etc_alpha; 
	hg->etc_bbtemp   =hg->obj[hg->etc_i].etc_bbtemp;
	hg->etc_sptype   =hg->obj[hg->etc_i].etc_sptype;
	
	hg->etc_adc      =hg->obj[hg->etc_i].etc_adc;
	hg->etc_imr      =hg->obj[hg->etc_i].etc_imr;
	
	hg->etc_wave     =hg->obj[hg->etc_i].etc_wave;
	hg->etc_waved    =hg->obj[hg->etc_i].etc_waved;
	//hg->etc_seeing   =hg->obj[hg->etc_i].etc_seeing;

	hg->plan[hg->etc_i_plan].snr=etc_obj(hg, -1);
      }
    }
    tree_update_plan_item(hg, model, iter, i_plan);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
  
  hg->etc_mode=ETC_MENU;
}



void hds_do_export_def_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  gdouble tmp_pa;
  gint tmp_exp, tmp_guide;
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;

  tmp_pa=hg->def_pa;
  tmp_exp=hg->def_exp;
  tmp_guide=hg->def_guide;

  dialog = gtk_dialog_new_with_buttons("HOE : Set Default Guide mode, PA, & Exptime",
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
  
  // GUIDE_MODE
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No Guide",
		       1, NO_GUIDE, -1);
    if(hg->def_guide==NO_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "AG Guide",
		       1, AG_GUIDE, -1);
    if(hg->def_guide==AG_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Guide",
		       1, SV_GUIDE, -1);
    if(hg->def_guide==SV_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV [Safe]",
		       1, SVSAFE_GUIDE, -1);
    if(hg->def_guide==SVSAFE_GUIDE) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_guide);
  }



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
  


  label = gtk_label_new ("  Exptime");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_exp,
					    1.0, 3600.0, 1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_exp);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    hg->def_pa=tmp_pa;
    hg->def_exp=tmp_exp;
    hg->def_guide=tmp_guide;
    hds_export_def(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void hds_do_efs_cairo (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;

  dialog = gtk_dialog_new_with_buttons("HOE : Echelle Format Simulator",
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

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("EFS : Echelle Format Simulator");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new ("Setup:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    if(!hg->setup[hg->efs_setup].use) hg->efs_setup=0;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : NonStd-%d  %dx%dbinning",
		  i_use+1,
		  -hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s  %dx%dbinning",
		  i_use+1,
		  HDS_setups[hg->setup[i_use].setup].initial,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	if(hg->efs_setup==i_use) iter_set=iter;
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->efs_setup);
  }


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    go_efs(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


