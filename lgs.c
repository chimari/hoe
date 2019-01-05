// lgs.c for Subaru LGS PRM file creation
//             Jan 2019  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"

void lgs_do_create_prm(GtkWidget *widget, gpointer gdata){
  GtkWidget *fdialog;
  typHOE *hg;
  gchar *fname;
  gchar *dest_file;
  FILE *fp;
  gint i_list, i_list_max;
  struct ln_zonedate zonedate_n, zonedate1, zonedate2;
  struct ln_date date_n, date_n0, date0, date1, date2;
  gint dur, dur_hours, dur_minutes;
  gdouble JD_n, JD_n0, JD0, JD1, JD2;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;

  hg=(typHOE *)gdata;

  if(!Check_LGS_SA(hg)) return;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select a Folder to create LGS PRM files",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 

  if(hg->filename_txt){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_txt)));
  }

  if (gtk_dialog_run(GTK_DIALOG(fdialog)) != GTK_RESPONSE_ACCEPT) {
    gtk_widget_destroy(fdialog);
    return;
  }
  
  fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
  gtk_widget_destroy(fdialog);

  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////  UT calc etc.
  /////////////////////////////////////////////////////////////////////////////////////////////////
  i_list=0;
  while(LGS_AzEl[i_list].name){
    i_list++;
  }
  i_list_max=i_list;

  get_current_obs_time(hg, &zonedate_n.years, &zonedate_n.months, &zonedate_n.days,
		       &zonedate_n.hours, &zonedate_n.minutes, &zonedate_n.seconds);
  zonedate_n.gmtoff=(long)hg->obs_timezone*60;
  ln_zonedate_to_date(&zonedate_n, &date_n);

  date_n0.years=date_n.years;
  date_n0.months=1;
  date_n0.days=1;
  date_n0.hours=0;
  date_n0.minutes=0;
  date_n0.seconds=0;
  

  calc_sun_plan(hg); 

  zonedate1.years=hg->fr_year;
  zonedate1.months=hg->fr_month;
  zonedate1.days=hg->fr_day;
  zonedate1.hours=hg->sun.s_set.hours;
  zonedate1.minutes=hg->sun.s_set.minutes;
  zonedate1.seconds=0.0;
  zonedate1.gmtoff=(long)hg->obs_timezone*60;
  ln_zonedate_to_date(&zonedate1, &date1);
  
  zonedate2.years=hg->fr_year;
  zonedate2.months=hg->fr_month;
  zonedate2.days=hg->fr_day;
  add_day(hg, &zonedate2.years, &zonedate2.months, &zonedate2.days, +1);
  zonedate2.hours=hg->sun.s_rise.hours;
  zonedate2.minutes=hg->sun.s_rise.minutes;
  zonedate2.seconds=0.0;
  zonedate2.gmtoff=(long)hg->obs_timezone*60;
  ln_zonedate_to_date(&zonedate2, &date2);

  date0.years=date1.years;
  date0.months=1;
  date0.days=1;
  date0.hours=0;
  date0.minutes=0;
  date0.seconds=0;

  JD_n=ln_get_julian_day(&date_n);
  JD_n0=ln_get_julian_day(&date_n0);
  JD0=ln_get_julian_day(&date0);
  JD1=ln_get_julian_day(&date1);
  JD2=ln_get_julian_day(&date2);

  dur=((zonedate2.hours+24)*60 + zonedate2.minutes) - ((zonedate1.hours)*60 + zonedate1.minutes);
  dur_hours=dur/60;
  dur_minutes=dur%60;
  /////////////////////////////////////////////////////////////////////////////////////////////////
    
  dest_file=to_locale(fname);

  if(hg->filename_prm1) g_free(hg->filename_prm1);
  hg->filename_prm1=g_strdup_printf("%s" G_DIR_SEPARATOR_S LGS_FNAME_BASE "%02d%s%d_For_JDAY%d_AZEL.txt",
				    to_utf8(dest_file),
				    date_n.days, cal_month[date_n.months-1], date_n.years,
				    (gint)(JD1-JD0));

  if(hg->filename_prm2) g_free(hg->filename_prm2);
  hg->filename_prm2=g_strdup_printf("%s" G_DIR_SEPARATOR_S LGS_FNAME_BASE "%02d%s%d_For_JDAY%d_RADEC.txt",
				    to_utf8(dest_file),
				    date_n.days, cal_month[date_n.months-1], date_n.years,
				    (gint)(JD1-JD0));

  g_free(fname);
  g_free(dest_file);
  
  if((fp=fopen(hg->filename_prm1,"wb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_prm1,
		  NULL);
    return;
  }


  fprintf(fp, "Classification:               Unclassified\n");
  fprintf(fp, "File Name:                    %s\n",
	  g_path_get_basename(hg->filename_prm1));
  fprintf(fp, "Message Purpose:              Request for Predictive Avoidance Support\n");
  fprintf(fp, "Message Date/Time (UTC):      %d %s %d (%d) %02d:%02d:%02d\n",
	  date_n.years, cal_month[date_n.months-1], date_n.days,
	  (gint)(JD_n-JD_n0), date_n.hours, date_n.minutes, (gint)date_n.seconds);
  fprintf(fp, "Type Windows Requested:       Open\n");
  fprintf(fp, "Point of Contact:             %s\n", hg->lgs_sa_name);
  fprintf(fp, "                              (Voice) (%03d) %03d %04d\n",
	  hg->lgs_sa_phone1, hg->lgs_sa_phone2, hg->lgs_sa_phone3);
  fprintf(fp, "                              (Fax) (808) 934 5099\n");
  fprintf(fp, "                              (E-mail) %s\n", hg->lgs_sa_email);
  fprintf(fp, "Emergency Phone # at Operations Site: (808) 935 5861\n");
  fprintf(fp, "Remarks:                      Email approval message to %s / Number of Targets: %d\n",
	  hg->lgs_sa_email, i_list_max);
  
  fprintf(fp, "MISSION INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  fprintf(fp, "Owner/Operator:             SUBARU\n");
  fprintf(fp, "Mission Name/Number:        " LGS_NAME "\n");
  fprintf(fp, "Target Type:                Fixed Azimuth/Elevation\n");
  fprintf(fp, "Location:                   Subaru Observatory\n");
  fprintf(fp, "Start Date/Time (UTC):      %d %s %d (%d) %02d:%02d:00\n",
	  date1.years, cal_month[date1.months-1], date1.days,
	  (gint)(JD1-JD0), date1.hours, date1.minutes);
  fprintf(fp, "End Date/Time (UTC):        %d %s %d (%d) %02d:%02d:00\n",
	  date2.years, cal_month[date2.months-1], date2.days,
	  (gint)(JD1-JD0), date2.hours, date2.minutes);
  fprintf(fp, "Duration (HH:MM:SS):        %02d:%02d:00\n",
	  dur_hours, dur_minutes);
  fprintf(fp, "\n");
  

  fprintf(fp, "LASER INFORMATION\n");
  fprintf(fp, "------------------------------------------------\n");
  fprintf(fp, "Laser:                      " LGS_NAME "\n");
  fprintf(fp, "\n");
  
  fprintf(fp, "SOURCE INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  fprintf(fp, "Method:                     Fixed Point\n");
  fprintf(fp, "Latitude:                   19.82550 degrees N\n");
  fprintf(fp, "Longitude:                  155.47602 degrees W\n");
  fprintf(fp, "Altitude:                   4.16409 km\n");
  fprintf(fp, "\n");
  
  fprintf(fp, "TARGET INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  
  i_list=0;
  for(i_list=0; i_list<i_list_max; i_list++){
    fprintf(fp, "Method:                     Fixed Azimuth/Elevation\n");
    fprintf(fp, "Azimuth:                    %.4lf\n", LGS_AzEl[i_list].az);
    fprintf(fp, "Elevation:                  %.4lf\n", LGS_AzEl[i_list].el);
    fprintf(fp, "\n");
  }

  fprintf(fp, "END OF FILE\n");
  
  fclose(fp);


  
  i_list_max=0;
  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      i_list_max++;
      if(hg->obj[i_list].gs.flag){
	i_list_max++;
      }
      break;
    }
  }

  if((fp=fopen(hg->filename_prm2,"wb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_prm2,
		  NULL);
    return;
  }

  
  fprintf(fp, "Classification:               Unclassified\n");
  fprintf(fp, "File Name:                    %s\n",
	  g_path_get_basename(hg->filename_prm2));
  fprintf(fp, "Message Purpose:              Request for Predictive Avoidance Support\n");
  fprintf(fp, "Message Date/Time (UTC):      %d %s %d (%d) %02d:%02d:%02d\n",
	  date_n.years, cal_month[date_n.months-1], date_n.days,
	  (gint)(JD_n-JD_n0), date_n.hours, date_n.minutes, (gint)date_n.seconds);
  fprintf(fp, "Type Windows Requested:       Open\n");
  fprintf(fp, "Point of Contact:             %s\n", hg->lgs_sa_name);
  fprintf(fp, "                              (Voice) (%03d) %03d %04d\n",
	  hg->lgs_sa_phone1, hg->lgs_sa_phone2, hg->lgs_sa_phone3);
  fprintf(fp, "                              (Fax) (808) 934 5099\n");
  fprintf(fp, "                              (E-mail) %s\n", hg->lgs_sa_email);
  fprintf(fp, "Emergency Phone # at Operations Site: (808) 935 5861\n");
  fprintf(fp, "Remarks:                      Email approval message to %s / Number of Targets: %d\n",
	  hg->lgs_sa_email, i_list_max);
  
  fprintf(fp, "MISSION INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  fprintf(fp, "Owner/Operator:             SUBARU\n");
  fprintf(fp, "Mission Name/Number:        " LGS_NAME "\n");
  fprintf(fp, "Target Type:                Right Ascension and Declination\n");
  fprintf(fp, "Location:                   Subaru Observatory\n");
  fprintf(fp, "Start Date/Time (UTC):      %d %s %d (%d) %02d:%02d:00\n",
	  date1.years, cal_month[date1.months-1], date1.days,
	  (gint)(JD1-JD0), date1.hours, date1.minutes);
  fprintf(fp, "End Date/Time (UTC):        %d %s %d (%d) %02d:%02d:00\n",
	  date2.years, cal_month[date2.months-1], date2.days,
	  (gint)(JD1-JD0), date2.hours, date2.minutes);
  fprintf(fp, "Duration (HH:MM:SS):        %02d:%02d:00\n",
	  dur_hours, dur_minutes);
  fprintf(fp, "\n");
  

  fprintf(fp, "LASER INFORMATION\n");
  fprintf(fp, "------------------------------------------------\n");
  fprintf(fp, "Laser:                      " LGS_NAME "\n");
  fprintf(fp, "\n");
  
  fprintf(fp, "SOURCE INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  fprintf(fp, "Method:                     Fixed Point\n");
  fprintf(fp, "Latitude:                   19.82550 degrees N\n");
  fprintf(fp, "Longitude:                  155.47602 degrees W\n");
  fprintf(fp, "Altitude:                   4.16409 km\n");
  fprintf(fp, "\n");
  
  fprintf(fp, "TARGET INFORMATION\n");
  fprintf(fp, "----------------------------\n");
  
  for(i_list=0; i_list<hg->i_max; i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      object.ra=ra_to_deg(hg->obj[i_list].ra);
      object.dec=dec_to_deg(hg->obj[i_list].dec);
      
      ln_get_equ_prec2 (&object, 
			get_julian_day_of_epoch(hg->obj[i_list].equinox),
			JD2000, &object_prec);
      
      fprintf(fp, "Method:                     Right Ascension and Declination\n");
      fprintf(fp, "Catalog Date:               J2000\n");
      fprintf(fp, "Right Ascension:            %.4lf\n", object_prec.ra);
      fprintf(fp, "Declination:                %+.4lf\n", object_prec.dec);
      fprintf(fp, "\n");
      break;
    }
  }

  for(i_list=0; i_list<hg->i_max; i_list++){
    switch(hg->obj[i_list].aomode){
    case AOMODE_LGS_S:
    case AOMODE_LGS_O:
      if(hg->obj[i_list].gs.flag){
	object.ra=ra_to_deg(hg->obj[i_list].gs.ra);
	object.dec=dec_to_deg(hg->obj[i_list].gs.dec);
	
	ln_get_equ_prec2 (&object, 
			  get_julian_day_of_epoch(hg->obj[i_list].equinox),
			  JD2000, &object_prec);
	
	fprintf(fp, "Method:                     Right Ascension and Declination\n");
	fprintf(fp, "Catalog Date:               J2000\n");
	fprintf(fp, "Right Ascension:            %.4lf\n", object_prec.ra);
	fprintf(fp, "Declination:                %+.4lf\n", object_prec.dec);
	fprintf(fp, "\n");
      }
      break;
    }
  }
  
  fprintf(fp, "END OF FILE\n");

  fclose(fp);

  popup_message(hg->w_top, 
#ifdef USE_GTK3
		"dialog-information", 
#else
		GTK_STOCK_DIALOG_INFO,
#endif
		POPUP_TIMEOUT*3,
		"Succeeded to create PRM files for LGS .",
		  " ",
		hg->filename_prm1,
		hg->filename_prm2,
		NULL);
}


void lgs_do_setup_sa (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_IRCS)) return;

  dialog = gtk_dialog_new_with_buttons("HOE : Set SA contact information for LGS request",
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

  frame = gtk_frame_new ("Contact information : LGS Support Astronomer");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtkut_table_new(2, 3, FALSE, 0, 0, 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  label = gtk_label_new ("Name");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL, GTK_FILL, 0, 0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 0, 1,
		     GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);
  if(hg->lgs_sa_name)
    gtk_entry_set_text(GTK_ENTRY(entry),hg->lgs_sa_name);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),40);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->lgs_sa_name);
  
  label = gtk_label_new ("e-mail");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL, GTK_FILL, 0, 0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);
  if(hg->lgs_sa_email)
    gtk_entry_set_text(GTK_ENTRY(entry),hg->lgs_sa_email);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),40);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->lgs_sa_email);
  
  label = gtk_label_new ("Phone number");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL, GTK_FILL, 0, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(table, hbox, 1, 2, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->lgs_sa_phone1,
					    0, 999, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->lgs_sa_phone1);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 3);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("-");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->lgs_sa_phone2,
					    0, 999, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->lgs_sa_phone2);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 3);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("-");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->lgs_sa_phone3,
					    0, 9999, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->lgs_sa_phone3);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    Check_LGS_SA(hg);

    WriteConf(hg);
  }
}


gboolean Check_LGS_SA(typHOE *hg){
  gboolean ret=TRUE;
  if(!hg->lgs_sa_name){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Warning: Please set LGS Support Astronomer\'s name .",
		  NULL);

    ret=FALSE;
  }
  
  if(!hg->lgs_sa_email){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Warning: Please set LGS Support Astronomer\'s e-mail address .",
		  NULL);
    
    ret=FALSE;
  }

  if(hg->lgs_sa_phone1*hg->lgs_sa_phone2*hg->lgs_sa_phone3==0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Warning: Please set LGS Support Astronomer\'s valid phone number .",
		  NULL);

    ret=FALSE;
  }

  return(ret);
}

