// seimei.c for Seimei Telescope
//             Dec 2021  A. Tajitsu (Subaru Telescope Okayama Brnch Office, NAOJ)

#include "main.h"

void init_obj_seimei(OBJpara* obj, typHOE *hg){
  obj->kools.grism=hg->def_kools_grism;
  obj->kools.pc=hg->def_kools_pc;
  obj->kools.ag=hg->def_kools_ag;
  obj->kools.nw=hg->def_kools_nw;
  
  obj->triccs.filter=TRICCS_FILTER_GRI;
  obj->triccs.gain  =TRICCS_GAIN_AUTO;
  obj->triccs.frames=1;
  obj->triccs.pc=FALSE;
  obj->triccs.ag=TRUE;
}


void kools_export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].exp=hg->def_exp;
    hg->obj[i_list].repeat=hg->def_repeat;
    hg->obj[i_list].kools.grism=hg->def_kools_grism;
    hg->obj[i_list].kools.pc=hg->def_kools_pc;
    hg->obj[i_list].kools.ag=hg->def_kools_ag;
    hg->obj[i_list].kools.nw=hg->def_kools_nw;
  }
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_EXP, hg->obj[i_list].exp, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_REPEAT, hg->obj[i_list].repeat, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_KOOLS_GRISM, hg->obj[i_list].kools.grism, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_PC, hg->obj[i_list].kools.pc, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_AG, hg->obj[i_list].kools.ag, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_NW, hg->obj[i_list].kools.nw, 
		       -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}


void kools_do_export_def_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gint tmp_exp, tmp_repeat;
  gint tmp_kools_grism;
  gboolean  tmp_kools_sh, tmp_kools_pc, tmp_kools_ag, tmp_kools_nw,
    tmp_kools_queue;
  GSList *sh_group=NULL, *pc_group=NULL, *ag_group=NULL, *nw_group=NULL,
    *queue_group=NULL;
  
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_KOOLS)) return;

  tmp_exp=hg->def_exp;
  tmp_repeat=hg->def_repeat;
  tmp_kools_grism=hg->def_kools_grism;
  tmp_kools_sh=hg->def_kools_sh;
  tmp_kools_pc=hg->def_kools_pc;
  tmp_kools_ag=hg->def_kools_ag;
  tmp_kools_nw=hg->def_kools_nw;
  tmp_kools_queue=hg->def_kools_queue;

  dialog = gtk_dialog_new_with_buttons("HOE : Set Default Obs. Parametes",
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

  table = gtkut_table_new(2, 5, FALSE, 0, 0, 5);
  gtk_container_add(GTK_CONTAINER(frame), table);


  // Exptime
  label = gtk_label_new ("Exposure Time");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_exp,
					    1.0, 3600.0, 1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_exp);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("sec x");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_repeat,
					    1, 20, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_repeat);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  
  // Grism
  label = gtkut_label_new ("Grism");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
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
    gint i_mode;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_mode=0;i_mode<NUM_KOOLS_GRISM;i_mode++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, kools_grism_name[i_mode],
			 1, i_mode, -1);
      if(hg->def_kools_grism==i_mode) iter_set=iter;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_kools_grism);
  }


  // SH
  label = gtkut_label_new ("M1 Alignment (SH)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 2, 3,
		     GTK_FILL,GTK_FILL,0,0);

  button = gtk_radio_button_new_with_label (sh_group, "Yes");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  sh_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->def_kools_sh);
  my_signal_connect (button, "toggled", cc_get_toggle, &tmp_kools_sh);

  button = gtk_radio_button_new_with_label (sh_group, "No");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->def_kools_sh);

  // PC
  label = gtkut_label_new ("Pointing Correction");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 3, 4,
		     GTK_FILL,GTK_FILL,0,0);

  button = gtk_radio_button_new_with_label (pc_group, "Yes");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  pc_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->def_kools_pc);
  my_signal_connect (button, "toggled", cc_get_toggle, &tmp_kools_pc);

  button = gtk_radio_button_new_with_label (pc_group, "No");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->def_kools_pc);

  // AG
  label = gtkut_label_new ("Auto Guide");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 4, 5,
		     GTK_FILL,GTK_FILL,0,0);

  button = gtk_radio_button_new_with_label (ag_group, "Yes");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  ag_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->def_kools_ag);
  my_signal_connect (button, "toggled", cc_get_toggle, &tmp_kools_ag);

  button = gtk_radio_button_new_with_label (ag_group, "No");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->def_kools_ag);

  // NW
  label = gtkut_label_new ("No Wipe Mode");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 5, 6,
		     GTK_FILL,GTK_FILL,0,0);

  button = gtk_radio_button_new_with_label (nw_group, "On");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  nw_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->def_kools_nw);
  my_signal_connect (button, "toggled", cc_get_toggle, &tmp_kools_nw);

  button = gtk_radio_button_new_with_label (nw_group, "Off");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->def_kools_nw);

  // Queue
  label = gtkut_label_new ("Output Format");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 6, 7,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 6, 7,
		     GTK_FILL,GTK_FILL,0,0);

  button = gtk_radio_button_new_with_label (queue_group, "Queue file");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  queue_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->def_kools_queue);
  my_signal_connect (button, "toggled", cc_get_toggle, &tmp_kools_queue);

  button = gtk_radio_button_new_with_label (queue_group, "Shell script");
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->def_kools_queue);

  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    hg->def_exp=tmp_exp;
    hg->def_repeat=tmp_repeat;
    hg->def_kools_grism=tmp_kools_grism;
    hg->def_kools_sh=tmp_kools_sh;
    hg->def_kools_pc=tmp_kools_pc;
    hg->def_kools_ag=tmp_kools_ag;
    hg->def_kools_nw=tmp_kools_nw;
    hg->def_kools_queue=tmp_kools_queue;
    kools_export_def(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


// TAB for Overheads
void KOOLS_OH_TAB_create(typHOE *hg){
  GtkWidget *scrwin;
  GtkWidget *frame, *frame1;
  GtkWidget *table, *table1, *table2;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *label, *button;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  gchar *tmp;
  
  scrwin = gtk_scrolled_window_new (NULL, NULL);
  table = gtkut_table_new(1, 2, FALSE, 0, 0, 0);

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
  

  frame = gtkut_frame_new ("<b>Target Acquisition</b> [sec]");
  gtkut_table_attach(table, frame, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  

  table1 = gtkut_table_new(2, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  label = gtk_label_new ("Base Overhead Time");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->oh_acq,
					    10, 300, 
					    1, 10, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->oh_acq);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);


  label = gtk_label_new ("Pointing Correction");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->oh_ngs1,
					    10, 300, 
					    1, 10, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->oh_ngs1);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);


    label = gtk_label_new ("Auto Guiding");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->oh_ngs2,
					    10, 300, 
					    1, 10, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->oh_ngs2);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtkut_table_attach(table1, spinner, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Overheads");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
}
