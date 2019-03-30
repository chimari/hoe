//    HOE : Subaru HDS++ OPE file Editor
//        general_gui.c     GUI common for all instruments
//                                           2019.01.03  A.Tajitsu


#include "main.h"

////////////// gui_init() Create Main GUI
void gui_init(typHOE *hg){
  GtkWidget *menubar;
#ifdef USE_GTKMACINTEGRATION
  GtkosxApplication *osxapp;
#endif

  // Main Window 
  hg->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  my_signal_connect(hg->w_top, "delete-event",
		    delete_quit,(gpointer)hg);
  gtk_container_set_border_width(GTK_CONTAINER(hg->w_top),0);

  set_win_title(hg);

  hg->w_box = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->w_top), hg->w_box);




#ifdef USE_GTKMACINTEGRATION
  osxapp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
#endif
  menubar=make_menu(hg);
  gtk_box_pack_start(GTK_BOX(hg->w_box), menubar,FALSE, FALSE, 0);

#ifdef USE_GTKMACINTEGRATION
  gtk_widget_hide(menubar);
  gtkosx_application_set_menu_bar(osxapp, GTK_MENU_SHELL(menubar));
  //about_menu = gtk_item_factory_get_item(ifactory, "/Help/About");
  //prefs_menu = gtk_item_factory_get_item(ifactory, "/Configuration/Preferences...");
  //gtkosx_application_insert_app_menu_item(osxapp, about_menu, 0);
  //gtkosx_application_insert_app_menu_item(osxapp, prefs_menu, 1);
  //g_signal_connect(G_OBJECT(osxapp), "NSApplicationBlockTermination",
  //		   G_CALLBACK(osx_block_termination), mainwin);
  gtkosx_application_ready(osxapp);
#endif


  make_note(hg);
  

  gtk_widget_show_all(hg->w_top);

}


////////////// delete_quit() : Change "delete event of main window to quit from the program.
gboolean delete_quit (GtkWidget *widget, GdkEvent *event, gpointer gdata){
  do_quit(widget, gdata);
  return(TRUE);
}

gboolean delete_main_quit (GtkWidget *widget, GdkEvent *event, gpointer gdata){
  gtk_main_quit();
  return(FALSE);
}


////////////// make_note() Create TABS in Main Window
void make_note(typHOE *hg)
{
  {
    GtkWidget *table;
    GtkWidget *label;
    guint page=0;

    hg->init_flag=FALSE;
    
    hg->all_note = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->all_note), GTK_POS_TOP);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->all_note), TRUE);
    gtk_box_pack_start(GTK_BOX(hg->w_box), hg->all_note,TRUE, TRUE, 5);

    // General TAB
    GUI_GENERAL_TAB_create(hg);
    hg->page[NOTE_GENERAL]=page;

    switch(hg->inst){
    // Setup for HDS instrument
    case INST_HDS:
      HDS_SVAG_TAB_create(hg);
      page++;
      hg->page[NOTE_AG]=page;

      HDS_TAB_create(hg);
      page++;
      hg->page[NOTE_HDS]=page;
      break;

    // Setup for IRCS instrument
    case INST_IRCS:
      IRCS_TAB_create(hg);
      page++;
      hg->page[NOTE_IRCS]=page;
      
      IRCS_OH_TAB_create(hg);
      page++;
      hg->page[NOTE_OH]=page;
      break;

    case INST_HSC:
      HSC_TAB_create(hg);
      page++;
      hg->page[NOTE_HSC]=page;
      
      HSCFIL_TAB_create(hg);
      page++;
      hg->page[NOTE_HSCFIL]=page;
      break;
    }

    // Main Target TAB
    GUI_TARGET_TAB_create(hg);
    page++;
    hg->page[NOTE_OBJ]=page;

    // STDDB
    GUI_STD_TAB_create(hg);
    page++;
    hg->page[NOTE_STDDB]=page;


    // FCDB
    GUI_FCDB_TAB_create(hg);
    page++;
    hg->page[NOTE_FCDB]=page;


    // TRDB
    GUI_TRDB_TAB_create(hg);
    page++;
    hg->page[NOTE_TRDB]=page;
    

    switch(hg->inst){
    case INST_HDS:
      // Line List
      HDS_LINE_TAB_create(hg);
      page++;
      hg->page[NOTE_LINE]=page;
      
      // ETC
      HDS_ETC_TAB_create(hg);
      page++;
      hg->page[NOTE_ETC]=page;
      break;
    }


    gtk_widget_show_all(hg->all_note);
  }
  hg->init_flag=TRUE;
}



/////////////////////////////////////////////////////////////
/////////////  TAB creations for make_note()
/////////////////////////////////////////////////////////////

/// General TAB
void GUI_GENERAL_TAB_create(typHOE *hg){
  GtkWidget *scrwin;
  GtkWidget *frame;
  GtkWidget *table, *table1;
  GtkWidget *hbox;
  GtkWidget *hbox1;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *combo, *combo0;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *button;
  gchar *tmp;
  GtkTooltip *tooltip;
  
  scrwin = gtk_scrolled_window_new (NULL, NULL);
  table=gtkut_table_new(3, 6, FALSE, 0, 0, 0);
  
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
  gtk_widget_set_size_request(scrwin, -1, 510);  
  
  
  vbox = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  
  // Header
  frame = gtkut_frame_new ("Header");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(1, 5, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table1, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
  tmp=g_strdup_printf("Date (%s in evening)", hg->obs_tzname);
  label = gtk_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),hbox1,FALSE,FALSE,0);
  
  hg->fr_e = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox1),hg->fr_e,FALSE,FALSE,0);
  gtk_editable_set_editable(GTK_EDITABLE(hg->fr_e),FALSE);
  my_entry_set_width_chars(GTK_ENTRY(hg->fr_e),12);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    popup_fr_calendar, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table1, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);

  hg->label_sun = gtk_label_new ("Solar parameters");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->label_sun, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->label_sun, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->label_sun), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->label_sun,FALSE,FALSE,0);
  
  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table1, hbox, 0, 1, 2, 3,
		     GTK_FILL,GTK_FILL,0,0);
  
  hg->label_moon = gtk_label_new ("     Moon parameters");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->label_moon, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->label_moon, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->label_moon), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->label_moon,FALSE,FALSE,0);

  set_fr_e_date(hg);
  
  
  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table1, hbox, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("ID");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_entry_set_text(GTK_ENTRY(entry),hg->prop_id);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),8);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->prop_id);
  
  label = gtk_label_new ("  Password");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),hbox1,FALSE,FALSE,0);
  
  hg->e_pass = gtk_entry_new ();
  gtk_entry_set_visibility(GTK_ENTRY(hg->e_pass), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->e_pass,FALSE,FALSE,0);
  if(hg->prop_pass)
    gtk_entry_set_text(GTK_ENTRY(hg->e_pass),hg->prop_pass);
  gtk_editable_set_editable(GTK_EDITABLE(hg->e_pass),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->e_pass),8);
  my_signal_connect (hg->e_pass,
		     "changed",
		     cc_get_entry,
		     &hg->prop_pass);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1), button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (GetPass), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  tmp = g_strconcat("Get Password from ", 
		    hg->home_dir, G_DIR_SEPARATOR_S,
		    USER_CONFFILE, NULL);
  gtk_widget_set_tooltip_text(button,tmp);
  g_free(tmp);
#endif

  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table1, hbox, 0, 1, 4, 5,
		     GTK_FILL,GTK_FILL,0,0);
  
  label = gtk_label_new ("Observer");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  if(hg->observer)
    gtk_entry_set_text(GTK_ENTRY(entry),hg->observer);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),40);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->observer);
  


  // Statistics.
  frame = gtkut_frame_new ("Base OPE");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  hbox = gtkut_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  
  hg->label_stat_base = gtk_label_new ("Total Exp. = 0.00 hrs,  Estimated Obs. Time = 0.00 hrs");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->label_stat_base, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->label_stat_base, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->label_stat_base), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hg->label_stat_base,FALSE, FALSE, 5);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE, FALSE, 5);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (UpdateTotalExp), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Recalc.");
#endif
  
  frame = gtkut_frame_new ("Plan OPE");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(1, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  hg->label_stat_plan = gtk_label_new ("00:00 -- 00:00 (0.00 hrs)");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->label_stat_plan, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->label_stat_plan, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->label_stat_plan), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, hg->label_stat_plan, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  frame = gtkut_frame_new ("Telescope Speed");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Azimuth [deg/s]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_az,
					    0.01, 2.00, 
					    0.01, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->vel_az);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  label = gtk_label_new ("  Elevation [deg/s]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_el,
					    0.01, 2.00, 
					    0.01, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->vel_el);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  
  vbox = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox, 1, 2, 0, 2,
		     GTK_FILL,GTK_FILL,0,0);
  
  // Environment for AD Calc.
  frame = gtkut_frame_new ("Environment for AD Calc.");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  
  // OBS Wavelength
  label = gtkut_label_new ("Observing &#x3BB; [&#xC5;]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave1,
					    2800, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->wave1);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  

  // Wavelength0
  label = gtkut_label_new ("Guiding &#x3BB; [&#xC5;]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave0,
					    2800, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->wave0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
  
  // Temperature
  label = gtkut_label_new ("  Temperature [&#xB0;C]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->temp,
					    -15, 15, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->temp);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
  
  // Pressure
  label = gtk_label_new ("  Pressure [hPa]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pres,
					    600, 650, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->pres);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtkut_table_attach(table1, spinner, 3, 4, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
  
  frame = gtkut_frame_new ("Database Access Host");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("SIMBAD");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Strasbourg (FR)",
		       1, FCDB_SIMBAD_STRASBG, 2, TRUE, -1);
    if(hg->fcdb_simbad==FCDB_SIMBAD_STRASBG) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Harvard (US)",
		       1, FCDB_SIMBAD_HARVARD, 2, TRUE, -1);
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD) iter_set=iter;
    
    
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
		       &hg->fcdb_simbad);
  }
  
  label = gtk_label_new ("   VizieR");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Strasbourg (FR)",
		       1, FCDB_VIZIER_STRASBG, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_STRASBG) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "NAOJ (JP)",
		       1, FCDB_VIZIER_NAOJ, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_NAOJ) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Harvard (US)",
		       1, FCDB_VIZIER_HARVARD, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_HARVARD) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->fcdb_vizier);
  }
  
  

#ifndef USE_WIN32
#ifndef USE_OSX
  // Web Browser
  frame = gtkut_frame_new ("Web Browser");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Command");
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  
  entry = gtk_entry_new ();
  gtkut_table_attach(table1, entry, 1, 2, 0, 1,
		     GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry),
		     hg->www_com);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->www_com);
#endif
#endif
  
  frame = gtkut_frame_new ("Font");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Base");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    button = gtk_font_button_new_with_font(hg->fontname_all);
    gtkut_table_attach(table1, button, 1, 2, 0, 1,
		       GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
    my_signal_connect(button,"font-set",ChangeFontButton_all, 
		      (gpointer)hg);
  }
  
  label = gtk_label_new ("     Object");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    button = gtk_font_button_new_with_font(hg->fontname);
    gtkut_table_attach(table1, button, 3, 4, 0, 1,
		       GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
    my_signal_connect(button,"font-set",ChangeFontButton, 
		      (gpointer)hg);
  }
  
  frame = gtkut_frame_new ("Finding Chart");
  gtk_box_pack_start (GTK_BOX (vbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Preferable Image Source");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    GtkWidget *bar;
    gint i_fc;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    for(i_fc=0;i_fc<NUM_FC;i_fc++){
      if(FC_name[i_fc]){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, FC_name[i_fc],
			   1, i_fc, 2, TRUE, -1);
	if(hg->fc_mode0==i_fc) iter_set=iter;
      }
      else{
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, NULL,
			    1, i_fc, 2, FALSE, -1);
      }
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 1, 2, 0, 1,
		       GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_mode0,
		       (gpointer)hg);
  }
  
  label = gtk_label_new ("General");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
}


/// Main target TAB
void GUI_TARGET_TAB_create(typHOE *hg){
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *hbox;
  GtkWidget *hbox1;
  GtkWidget *entry;
  GtkWidget *button;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *combo;
  GtkWidget *label;
  GdkPixbuf *pixbuf;
  
  table = gtkut_table_new(2, 2, FALSE, 0, 0, 0);
  
  hg->sw_objtree = gtk_scrolled_window_new (NULL, NULL);
  gtkut_table_attach_defaults(table, hg->sw_objtree, 0, 2, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hg->sw_objtree), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->sw_objtree),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 0, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  frame = gtkut_frame_new ("Find Object");
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (search_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Find");
#endif
  
  hg->tree_search_i=0;
  hg->tree_search_imax=0;
  
  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox1), entry,FALSE, FALSE, 0);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),12);
  my_signal_connect (entry, "changed", cc_search_text, (gpointer)hg);
  my_signal_connect (entry, "activate", search_item, (gpointer)hg);
  
  hg->tree_search_label = gtk_label_new ("     ");
  gtk_box_pack_start(GTK_BOX(hbox1),hg->tree_search_label,FALSE,FALSE,0);
  
  
  frame = gtkut_frame_new ("View");
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Plot","document-print-preview");
#else
  button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (plot2_objtree_item), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("FC","starred");
#else
  button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fc_item), (gpointer)hg);
  
  if(hg->inst==INST_HDS){
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
    button=gtkut_button_new_from_pixbuf("ETC", pixbuf);
    g_object_unref(G_OBJECT(pixbuf));
    g_signal_connect (button, "clicked",
		      G_CALLBACK (etc_objtree_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);
  }
  
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/pm_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf("PM", pixbuf);
  g_object_unref(G_OBJECT(pixbuf));
  g_signal_connect (button, "clicked",
		    G_CALLBACK (pm_objtree_item), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);
  
  if(hg->inst==INST_IRCS){
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/lgs_icon.png", NULL);
    button=gtkut_button_new_from_pixbuf("LGS", pixbuf);
    g_object_unref(G_OBJECT(pixbuf));
    g_signal_connect (button, "clicked",
    		    G_CALLBACK (pam_objtree_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);
  }

  hg->f_objtree_arud = gtkut_frame_new ("Edit the List");
  gtk_box_pack_start (GTK_BOX (hbox), hg->f_objtree_arud, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->f_objtree_arud), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (hg->f_objtree_arud), hbox1);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"list-add");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    addobj_dialog, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Add");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"list-remove");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    remove_item_objtree, 
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
		    up_item_objtree, 
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
		    down_item_objtree, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Down");
#endif
  
  frame = gtkut_frame_new ("Web Browsing");
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    GdkPixbuf *icon;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
			       G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SIMBAD",
		       1, WWWDB_SIMBAD, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_SIMBAD) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "NED",
		       1, WWWDB_NED, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_NED) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS (DR15)",
		       1, WWWDB_SDSS_DRNOW, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_SDSS_DRNOW) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MAST",
		       1, WWWDB_MAST, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_MAST) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MAST Portal",
		       1, WWWDB_MASTP, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_MASTP) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "KECK archive",
		       1, WWWDB_KECK, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_KECK) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "GEMINI archive",
		       1, WWWDB_GEMINI, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_GEMINI) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IRSA",
		       1, WWWDB_IRSA, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_IRSA) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Spitzer",
		       1, WWWDB_SPITZER, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_SPITZER) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "CASSIS",
		       1, WWWDB_CASSIS, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_CASSIS) iter_set=iter;
    
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, WWWDB_SEP1,2, FALSE, 
			-1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Standard Locator",
		       1, WWWDB_SSLOC, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_SSLOC) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
		       1, WWWDB_RAPID, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_RAPID) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
		       1, WWWDB_MIRSTD, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_MIRSTD) iter_set=iter;
    
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, WWWDB_SEP2,2, FALSE, 
			-1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SMOKA",
		       1, WWWDB_SMOKA, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_SMOKA) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HST Archive",
		       1, WWWDB_HST, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_HST) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO Archive",
		       1, WWWDB_ESO, 2, TRUE, -1);
    if(hg->wwwdb_mode==WWWDB_ESO) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->wwwdb_mode);
    
#ifdef USE_OSX
    icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
    icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
    if(strcmp(hg->www_com,"firefox")==0){
      icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
    }
    else{
      icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
    }
#endif
    button=gtkut_button_new_from_pixbuf(NULL, icon);
    g_object_unref(icon);
    gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (wwwdb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Open Web Browser");
#endif
  }     
  
  frame = gtkut_frame_new ("Standard Stars");
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
			       G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Standard Locator",
		       1, STDDB_SSLOC, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_SSLOC) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
		       1, STDDB_RAPID, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_RAPID) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
		       1, STDDB_MIRSTD, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_MIRSTD) iter_set=iter;
    
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, WWWDB_SEP1,2, FALSE, 
			-1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO Opt/UV Standard",
		       1, STDDB_ESOSTD, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_ESOSTD) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IRAF 1D-std (spec16/50)",
		       1, STDDB_IRAFSTD, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_IRAFSTD) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HST CALSPEC",
		       1, STDDB_CALSPEC, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_CALSPEC) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS efficiency",
		       1, STDDB_HDSSTD, 2, TRUE, -1);
    if(hg->stddb_mode==STDDB_HDSSTD) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->stddb_mode);
  }
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (stddb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Search");
#endif
  
  hg->mode_frame = gtkut_frame_new ("Current");
  gtk_box_pack_start (GTK_BOX (hbox), hg->mode_frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->mode_frame), 2);
  
  hbox1 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_container_add (GTK_CONTAINER (hg->mode_frame), hbox1);
  
  hg->mode_label = gtk_label_new ("XX-XX-XX XX:XX HST");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->mode_label, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->mode_label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->mode_label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),hg->mode_label,FALSE, FALSE, 0);
  
  /*
    #ifdef USE_GTK3
    button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
    #else
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
    #endif
    gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
    G_CALLBACK (RecalcRST), (gpointer)hg);
    #ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Refresh Rise & Set Time (El=15)");
    #endif
  */
      
  make_obj_tree(hg);
  
  label = gtk_label_new ("Main Target");
  gtk_widget_show(label);
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
}


/// Standard TAB
void GUI_STD_TAB_create(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *sw;
  GtkTreeModel *items_model;
  GdkPixbuf *icon;
  
  vbox = gtkut_vbox_new (FALSE, 5);
  
  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
  
  
#ifdef USE_GTK3
  hg->stddb_button=gtkut_toggle_button_new_from_icon_name(NULL,"video-display");
#else
  hg->stddb_button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_APPLY);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->stddb_button), 0);
  gtk_box_pack_start(GTK_BOX(hbox),hg->stddb_button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->stddb_button),
			       hg->stddb_flag);
  my_signal_connect(hg->stddb_button,"toggled",
		    G_CALLBACK(stddb_toggle), 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->stddb_button,
			      "Display Standards in SkyMon");
#endif
  
  hg->stddb_label= gtk_label_new (hg->stddb_label_text);
  gtk_box_pack_start(GTK_BOX(hbox), hg->stddb_label, TRUE, TRUE, 0);
  
  stddb_set_label(hg);
  
#ifdef USE_GTK3    
  button=gtkut_button_new_from_icon_name(NULL,"emblem-system");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (create_std_para_dialog), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Config for Standard Search");
#endif
  
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  
  /* create models */
  items_model = std_create_items_model (hg);
  
  /* create tree view */
  hg->stddb_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->stddb_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->stddb_tree)),
			       GTK_SELECTION_SINGLE);
  std_add_columns (hg, GTK_TREE_VIEW (hg->stddb_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (sw), hg->stddb_tree);
  
  my_signal_connect (hg->stddb_tree, "cursor-changed",
		     G_CALLBACK (std_focus_item), (gpointer)hg);
  
  /* some buttons */
  hbox = gtkut_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  
#ifdef USE_OSX
  icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
  icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
  if(strcmp(hg->www_com,"firefox")==0){
    icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
  }
  else{
    icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
  }
#endif
  button=gtkut_button_new_from_pixbuf("Browse", icon);
  g_object_unref(icon);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (std_simbad), (gpointer)hg);
  
#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("Standard","list-add");
#else
  button=gtkut_button_new_from_stock("Standard",GTK_STOCK_ADD);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (add_item_std), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Add to the Main Target List as a Standard Star");
#endif
  
  label= gtk_label_new ("    ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("OPE Def.","accessories-text-editor");
#else
  button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     make_std_tgt, (gpointer)hg);
  
  hg->std_tgt = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->std_tgt,TRUE, TRUE, 0);
  gtk_editable_set_editable(GTK_EDITABLE(hg->std_tgt),FALSE);
  my_entry_set_width_chars(GTK_ENTRY(hg->std_tgt),50);

#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name(NULL,"edit-copy");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (clip_copy), (gpointer)hg->std_tgt);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Copy to clipboard");
#endif
  label = gtk_label_new ("Standard");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
}


/// FCDB TAB
void GUI_FCDB_TAB_create(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *button;
  GdkPixbuf *icon;
  
  vbox = gtkut_vbox_new (FALSE, 5);
  
  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-save");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_fcdb_csv), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif
  
  hg->fcdb_label= gtk_label_new (hg->fcdb_label_text);
  gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_label, TRUE, TRUE, 0);
  
#ifdef USE_GTK3    
  button=gtkut_button_new_from_icon_name(NULL,"emblem-system");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fcdb_para_item), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Search Param.");
#endif
  
  hg->fcdb_sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->fcdb_sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->fcdb_sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), hg->fcdb_sw, TRUE, TRUE, 0);
  
  fcdb_append_tree(hg);
  
  /* some buttons */
  hbox = gtkut_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
  case FCDB_TYPE_NED:
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_LAMOST:
  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_HST:
  case FCDB_TYPE_ESO:
  case FCDB_TYPE_GEMINI:
#ifdef USE_OSX
    icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
    icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
    if(strcmp(hg->www_com,"firefox")==0){
      icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
    }
    else{
      icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
    }
#endif
    button=gtkut_button_new_from_pixbuf("Browse", icon);
    g_object_unref(icon);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (fcdb_simbad), (gpointer)hg);
    
    break;
    
  default:
    break;
  }
  
#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("Main target","list-add");
#else
  button=gtkut_button_new_from_stock("Main target",GTK_STOCK_ADD);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (add_item_fcdb), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Add to the Main Target List");
#endif
  
  if(hg->inst==INST_IRCS){
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Guide Star","list-add");
#else
    button=gtkut_button_new_from_stock("Guide Star",GTK_STOCK_ADD);
#endif
    my_signal_connect (button, "clicked",
		       G_CALLBACK (add_item_gs), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Add as a Guide Star");
#endif
  }
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("DB / Main Target","go-next");
#else
  button=gtkut_button_new_from_stock("DB / Main Target",GTK_STOCK_GO_FORWARD);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fcdb_to_trdb), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Add/Replace the Object to DB / Main Target TAB");
#endif
  
  label= gtk_label_new ("    ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("OPE Def.","accessories-text-editor");
#else
  button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     make_fcdb_tgt, (gpointer)hg);
  
  hg->fcdb_tgt = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->fcdb_tgt,TRUE, TRUE, 0);
  gtk_editable_set_editable(GTK_EDITABLE(hg->fcdb_tgt),FALSE);
  my_entry_set_width_chars(GTK_ENTRY(hg->fcdb_tgt),50);
  
#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name(NULL,"edit-copy");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (clip_copy), (gpointer)hg->fcdb_tgt);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Copy to clipboard");
#endif
  label = gtk_label_new ("DB / Finding Chart");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
}


void GUI_TRDB_TAB_create(typHOE *hg)
      {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *label;
      GtkWidget *button;
      GtkWidget *entry;
      GdkPixbuf *icon;

      vbox = gtkut_vbox_new (FALSE, 5);
      
      hbox = gtkut_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"document-save");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (do_save_trdb_csv), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_search_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Find Object");
#endif
      
      hg->trdb_search_i=0;
      hg->trdb_search_imax=0;
      
      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox), entry,FALSE, FALSE, 0);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),10);
      my_signal_connect (entry, "changed", trdb_cc_search_text, (gpointer)hg);
      my_signal_connect (entry, "activate", trdb_search_item, (gpointer)hg);
      
      hg->trdb_search_label = gtk_label_new ("     ");
      gtk_box_pack_start(GTK_BOX(hbox),hg->trdb_search_label,FALSE,FALSE,0);

      hg->trdb_label= gtk_label_new (hg->trdb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->trdb_label, TRUE, TRUE, 0);
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	GtkWidget *combo;
	gint i_inst;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Data Archive",
			   1, TRDB_TYPE_SMOKA, -1);
	if((hg->trdb_used==TRDB_TYPE_SMOKA)||
	   (hg->trdb_used==TRDB_TYPE_HST)||
	   (hg->trdb_used==TRDB_TYPE_ESO)||
	   (hg->trdb_used==TRDB_TYPE_GEMINI)) iter_set=iter;

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SIMBAD",
			   1, MAGDB_TYPE_SIMBAD, -1);
	if(hg->trdb_used==MAGDB_TYPE_SIMBAD) iter_set=iter;
	iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "NED",
			   1, MAGDB_TYPE_NED, -1);
	if(hg->trdb_used==MAGDB_TYPE_NED) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "LAMOST",
			   1, MAGDB_TYPE_LAMOST, -1);
	if(hg->trdb_used==MAGDB_TYPE_LAMOST) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GSC",
			   1, MAGDB_TYPE_GSC, -1);
	if(hg->trdb_used==MAGDB_TYPE_GSC) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "PanSTARRS1",
			   1, MAGDB_TYPE_PS1, -1);
	if(hg->trdb_used==MAGDB_TYPE_PS1) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SDSS",
			   1, MAGDB_TYPE_SDSS, -1);
	if(hg->trdb_used==MAGDB_TYPE_SDSS) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GAIA",
			   1, MAGDB_TYPE_GAIA, -1);
	if(hg->trdb_used==MAGDB_TYPE_GAIA) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Kepler",
			   1, MAGDB_TYPE_KEPLER, -1);
	if(hg->trdb_used==MAGDB_TYPE_KEPLER) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "2MASS",
			   1, MAGDB_TYPE_2MASS, -1);
	if(hg->trdb_used==MAGDB_TYPE_2MASS) iter_set=iter;
	
	hg->trdb_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox), hg->trdb_combo, FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->trdb_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->trdb_combo), 
					renderer, "text",0,NULL);
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->trdb_combo),&iter_set);
	gtk_widget_show(hg->trdb_combo);
	my_signal_connect (hg->trdb_combo,"changed",cc_get_combo_box_trdb,
			   (gpointer)hg);
      }
      
     
      hg->trdb_sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->trdb_sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->trdb_sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), hg->trdb_sw, TRUE, TRUE, 0);

      trdb_append_tree(hg);

      /* some buttons */
      hbox = gtkut_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Show Detail","go-previous");
#else
      button=gtkut_button_new_from_stock("Show Detail",GTK_STOCK_GO_BACK);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_dbtab), (gpointer)hg);

#ifdef USE_OSX
      icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
      icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
      if(strcmp(hg->www_com,"firefox")==0){
	icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
      }
      else{
	icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
      }
#endif
      button=gtkut_button_new_from_pixbuf("Browse", icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_simbad), (gpointer)hg);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("FC","starred");
#else
      button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (fc_item_trdb), (gpointer)hg);

      label = gtk_label_new ("DB / Main Target");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      }

//////////////////////////////////////////////////////////
/////////   Callbacks
//////////////////////////////////////////////////////////
void ChangeFontButton(GtkWidget *w, gpointer gdata)
{ 
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  if(hg->fontname) g_free(hg->fontname);
  hg->fontname
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));

  get_font_family_size(hg);
}

void ChangeFontButton_all(GtkWidget *w, gpointer gdata)
{ 
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  if(hg->fontname_all) g_free(hg->fontname_all);
  hg->fontname_all
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));

  get_font_family_size(hg);
}

void get_font_family_size(typHOE *hg)
{
  PangoFontDescription *fontdc;
      
  fontdc=pango_font_description_from_string(hg->fontname);

  if(hg->fontfamily) g_free(hg->fontfamily);
  hg->fontfamily
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_objsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);

  fontdc=pango_font_description_from_string(hg->fontname_all);

  if(hg->fontfamily_all) g_free(hg->fontfamily_all);
  hg->fontfamily_all
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_allsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);
}


void UpdateTotalExp(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  get_total_basic_exp(hg);
}


void popup_fr_calendar (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *calendar;
  GtkAllocation *allocation=g_new(GtkAllocation, 1);
  gint root_x, root_y;
  typHOE *hg=(typHOE *)gdata;
  gtk_widget_get_allocation(widget,allocation);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_get_position(GTK_WINDOW(hg->w_top),&root_x,&root_y);

  my_signal_connect(dialog,"delete-event",delete_main_quit,NULL);
  gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);

  calendar=gtk_calendar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     calendar,FALSE, FALSE, 0);

  gtk_calendar_select_month(GTK_CALENDAR(calendar),
			    hg->fr_month-1,
			    hg->fr_year);

  gtk_calendar_select_day(GTK_CALENDAR(calendar),
			  hg->fr_day);

  my_signal_connect(calendar,
		    "day-selected-double-click",
		    select_fr_calendar, 
		    (gpointer)hg);

  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_window_move(GTK_WINDOW(dialog),
		  root_x+allocation->x,
		  root_y+allocation->y);
  g_free(allocation);
  gtk_widget_show_all(dialog);
  gtk_main();
  gtk_widget_destroy(dialog);
}

void select_fr_calendar (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;
  gint i_list;

  gtk_calendar_get_date(GTK_CALENDAR(widget),
			&hg->fr_year,
			&hg->fr_month,
			&hg->fr_day);
  hg->fr_month++;

  hg->skymon_year =hg->fr_year;
  hg->skymon_month=hg->fr_month;
  hg->skymon_day  =hg->fr_day;
  hg->skymon_hour =23;
  hg->skymon_min  =55;

  set_fr_e_date(hg);

  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].pam=-1;
  }
  hg->lgs_pam_i_max=0;
  update_objtree(hg);
  
  gtk_main_quit();
}

void set_fr_e_date(typHOE *hg){
  gchar *tmp;
  typPlanMoon moon;
    
  tmp=g_strdup_printf("%s %d, %d",
		      cal_month[hg->fr_month-1],
		      hg->fr_day,
		      hg->fr_year);

  gtk_entry_set_text(GTK_ENTRY(hg->fr_e),tmp);
  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
  if(flagSkymon)
    gtk_entry_set_text(GTK_ENTRY(hg->skymon_e_date),tmp);
  g_free(tmp);

  if(hg->init_flag){
    calcpa2_skymon(hg);
    gtk_adjustment_set_value(hg->skymon_adj_min, (gdouble)hg->skymon_time);
    recalc_rst(hg);
    if(flagSkymon) draw_skymon_cairo(hg->skymon_dw,hg);
  }

  moon=calc_typPlanMoon(hg, 24*60*60, -1, -1);
  hg->fr_moon=moon.age;

  tmp=g_strdup_printf("   Moon : %.1lf days", hg->fr_moon);
  gtk_label_set_text(GTK_LABEL(hg->label_moon),tmp);
  g_free(tmp);

  calc_sun_plan(hg);
  tmp=g_strdup_printf("   Sun : Set %d:%02d, Tw<sub>18</sub> %d:%02d ---- Tw<sub>18</sub>  %d:%02d, Rise %d:%02d",
		      hg->sun.s_set.hours,
		      hg->sun.s_set.minutes,
		      hg->atw18.s_set.hours,
		      hg->atw18.s_set.minutes,
		      hg->atw18.s_rise.hours,
		      hg->atw18.s_rise.minutes,
		      hg->sun.s_rise.hours,
		      hg->sun.s_rise.minutes);
  gtk_label_set_markup(GTK_LABEL(hg->label_sun),tmp);
  g_free(tmp);

  if(flagPlan){
    refresh_tree (NULL, (gpointer)hg);
  }
}


void ReadPass(typHOE *hg)
{
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_col;
  gint major_ver,minor_ver,micro_ver;
  gboolean flag_prec;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  
  if(hg->prop_pass) g_free(hg->prop_pass);

  if (cfgfile) {
    if(hg->prop_id){
      if(xmms_cfg_read_string(cfgfile, "Pass", hg->prop_id, &c_buf)) 
	hg->prop_pass =c_buf;
      else
	hg->prop_pass=NULL;
    }

    xmms_cfg_free(cfgfile);
  }
  else{
    hg->prop_pass=NULL;
  }
}

void GetPass(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  if(hg->prop_id){
    ReadPass(hg);
    if(hg->prop_pass){
      gtk_entry_set_text(GTK_ENTRY(hg->e_pass), hg->prop_pass);
    }
  }
}


void clip_copy(GtkWidget *widget, gpointer gdata){
  GtkWidget *entry;
  GtkClipboard* clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  const gchar *c;

  entry=(GtkWidget *)gdata;

  c = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_clipboard_set_text (clipboard, c, strlen(c));
}


void cc_get_combo_box_trdb (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg=(typHOE *)gdata;
  gint fcdb_tmp;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    fcdb_tmp=hg->fcdb_type;
    if(n==TRDB_TYPE_SMOKA){
      n=hg->trdb_da;
    }
    hg->trdb_used=n;
    hg->fcdb_type=n;
    trdb_make_tree(hg);
    rebuild_trdb_tree(hg);
    hg->fcdb_type=fcdb_tmp;
  }
}


void set_win_title(typHOE *hg){
  gchar *win_title=NULL, *path_name=NULL;

  if(hg->filename_hoe){
    path_name=g_path_get_basename(hg->filename_hoe);
    win_title=g_strdup_printf("HOE : Subaru HDS++ OPE file Editor for %s [%s]",
			      inst_name_short[hg->inst],
			      path_name);
  }
  else{
    win_title=g_strdup_printf("HOE : Subaru HDS++ OPE file Editor for %s",
			      inst_name_short[hg->inst]);
  }
  gtk_window_set_title(GTK_WINDOW(hg->w_top), win_title);
  if(win_title) g_free(win_title);
  if(path_name) g_free(path_name);
}




