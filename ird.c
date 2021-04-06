// ird.c for Subaru IRD
//             Mar 2020  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"

// TAB for Overheads
void IRD_OH_TAB_create(typHOE *hg){
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
  

  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  label = gtk_label_new ("SetupField + CheckField");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  hg->ird_adj_oh_acq = (GtkAdjustment *)gtk_adjustment_new(hg->oh_acq,
							   0, 900, 
							   1, 10, 0);
  my_signal_connect (hg->ird_adj_oh_acq, "value_changed",
		     cc_get_adj,
		     &hg->oh_acq);
  spinner =  gtk_spin_button_new (hg->ird_adj_oh_acq, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  frame = gtkut_frame_new ("<b>AO tuning</b> [sec]");
  gtkut_table_attach(table, frame, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  

  table1 = gtkut_table_new(1, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  frame1 = gtk_frame_new ("Natural Guide Star");
  gtkut_table_attach(table1, frame1, 0, 1, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table2 = gtkut_table_new(2, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame1), table2);

  tmp=g_strdup_printf(" < %d.0 mag", IRD_NGS_MAG1);
  label = gtk_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  hg->ird_adj_oh_ngs1 = (GtkAdjustment *)gtk_adjustment_new(hg->oh_ngs1,
							    120, 1200, 
							    1, 10, 0);
  my_signal_connect (hg->ird_adj_oh_ngs1, "value_changed",
		     cc_get_adj,
		     &hg->oh_ngs1);
  spinner =  gtk_spin_button_new (hg->ird_adj_oh_ngs1, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtkut_table_attach(table2, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  tmp=g_strdup_printf(" < %d.0 mag", IRD_NGS_MAG2);
  label = gtk_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  hg->ird_adj_oh_ngs2 = (GtkAdjustment *)gtk_adjustment_new(hg->oh_ngs2,
							    120, 1200, 
							    1, 10, 0);
  my_signal_connect (hg->ird_adj_oh_ngs2, "value_changed",
		     cc_get_adj,
		     &hg->oh_ngs2);
  spinner =  gtk_spin_button_new (hg->ird_adj_oh_ngs2, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtkut_table_attach(table2, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  tmp=g_strdup_printf(" %d.0 mag < ", IRD_NGS_MAG2);
  label = gtk_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  hg->ird_adj_oh_ngs3 = (GtkAdjustment *)gtk_adjustment_new(hg->oh_ngs3,
							    120, 1200, 
							    1, 10, 0);
  my_signal_connect (hg->ird_adj_oh_ngs3, "value_changed",
		     cc_get_adj,
		     &hg->oh_ngs3);
  spinner =  gtk_spin_button_new (hg->ird_adj_oh_ngs3, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtkut_table_attach(table2, spinner, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  frame1 = gtk_frame_new ("Laser Guide Star");
  gtkut_table_attach(table1, frame1, 0, 1, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table2 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame1), table2);

  label = gtk_label_new ("for All LGS Targets");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  hg->ird_adj_oh_lgs = (GtkAdjustment *)gtk_adjustment_new(hg->oh_lgs,
							   120, 2000, 
							   1, 10, 0);
  my_signal_connect (hg->ird_adj_oh_lgs, "value_changed",
		     cc_get_adj,
		     &hg->oh_lgs);
  spinner =  gtk_spin_button_new (hg->ird_adj_oh_lgs, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtkut_table_attach(table2, spinner, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  
  frame = gtkut_frame_new ("<b>Sync Overheads</b>");
  gtkut_table_attach(table, frame, 0, 1, 2, 3,
		     GTK_FILL,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
#else
  button=gtkut_button_new_from_stock("Sync",GTK_STOCK_REFRESH);
#endif
  gtkut_table_attach(table1, button, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (ird_sync_overhead), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Sync Recommended Overheads");
#endif
  
  hg->ird_label_overhead_ver = gtkut_label_new (hg->ird_overhead_ver);
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->ird_label_overhead_ver, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->ird_label_overhead_ver, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->ird_label_overhead_ver), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, hg->ird_label_overhead_ver, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);


  label = gtk_label_new ("Overheads");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
}


// Parameter init calling from param_init() in main.c
void IRD_param_init(typHOE *hg){
  hg->ird_overhead_ver=g_strdup("<i>(Not synced yet)</i>");
}


void IRD_WriteOPE(typHOE *hg, gboolean plan_flag){
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
  fprintf(fp, "*load \"ird_obs.prm\"\n");
  fprintf(fp, "*load \"ird_common.prm\"\n");
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
      //if((fabs(hg->obj[i_list].pm_ra)>100)
      // ||(fabs(hg->obj[i_list].pm_dec)>100)){
      yrs=current_yrs(hg);
      new_d_ra=ra_to_deg(hg->obj[i_list].ra)+
	hg->obj[i_list].pm_ra/1000/60/60*yrs;
      new_d_dec=dec_to_deg(hg->obj[i_list].dec)+
	hg->obj[i_list].pm_dec/1000/60/60*yrs;
      
      new_ra=deg_to_ra(new_d_ra);
      new_dec=deg_to_dec(new_d_dec);
      
      //fprintf(fp, "PM%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n# ",
      //      tgt, hg->obj[i_list].name, 
      //      new_ra,  new_dec, 
      //      hg->obj[i_list].equinox);
      //}
      fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f PMRA=%.4lf PMDEC=%.4lf\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, 
	      hg->obj[i_list].equinox,
	      hg->obj[i_list].pm_ra/1000.,  hg->obj[i_list].pm_dec/1000.); 
    }
    else{
      fprintf(fp, "# (Non-sidereal)\n# %s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox);
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

  fprintf(fp, "################# Telescope Offset #################\n");
  fprintf(fp, "# [target in N]\n");
  fprintf(fp, "DELTA_N1=DELTA_RA=-12 DELTA_DEC=-10\n");
  fprintf(fp, "DELTA_N2=DELTA_RA=0   DELTA_DEC=5\n");
  fprintf(fp, "# [target in NE]\n");
  fprintf(fp, "DELTA_NE1=DELTA_RA=3  DELTA_DEC=5\n");
  fprintf(fp, "DELTA_NE2=DELTA_RA=-9 DELTA_DEC=-5\n");
  fprintf(fp, "# [target in NW]\n");
  fprintf(fp, "DELTA_NW=DELTA_RA=0  DELTA_DEC=-5\n");
  fprintf(fp, "# [target in S +/-10 degree]\n");
  fprintf(fp, "DELTA_S=DELTA_RA=3  DELTA_DEC=5\n");
  fprintf(fp, "# [target in SE]\n");
  fprintf(fp, "DELTA_SE=DELTA_RA=0  DELTA_DEC=6\n");
  fprintf(fp, "# [target in E +/-10 degree]\n");
  fprintf(fp, "DELTA_E=DELTA_RA=0  DELTA_DEC=5\n");
  fprintf(fp, "# [target in SW]\n");
  fprintf(fp, "DELTA_SW=DELTA_RA=0  DELTA_DEC=-5\n");
  fprintf(fp, "####################################################\n");

  fprintf(fp, "\n");

 
  fprintf(fp, "</PARAMETER_LIST>\n");

  fprintf(fp, "\n");


  fprintf(fp, "<COMMAND>\n");
  fprintf(fp, "###==== LGS Calibration (required only for LGS) ====###\n");
  fprintf(fp, "        AO188_M2CNT $DEF_AOST MODE=LGS INS=AO IMR=DEFAULT LGSH=DEFAULT MODECHANGE=YES\n");
  fprintf(fp, "        AO188_LGSCOL $DEF_AOST EXPTIME=5 MODE=MANUAL BIN=4\n");
  fprintf(fp, "        AO188_LGSCOL $DEF_AOST EXPTIME=5 MODE=MANUAL BIN=4\n");
  fprintf(fp, "        #        AO188_M2CNT_QUICK $DEF_AOST MODE=LGS INS=AO SETMODE=YES\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");


  fprintf(fp, "###==== Proposal Change ====###\n");
  fprintf(fp, "## w/ dialog\n");
  fprintf(fp, "CHANGEPROP OBE_ID=COMMON OBE_MODE=TOOL PROPID=\"%s\" OBSERVER=\"%s\"\n",
	  (hg->prop_id) ? hg->prop_id : "oXXXXX",
	  (hg->observer) ? hg->observer : "Observer Name");
  fprintf(fp, "## w/o dialog\n");
  fprintf(fp, "Exec OBS CHANGEPROP PROPID=\"%s\" OBSERVER=\"%s\"\n",
	  (hg->prop_id) ? hg->prop_id : "oXXXXX",
	  (hg->observer) ? hg->observer : "Observer Name");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  /// FIM dark sequence
  fprintf(fp, "##==== FIM Dark Acquistion ====##\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=0.1 FNUM=10\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=1.0 FNUM=10\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=5.0 FNUM=10\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=10.0 FNUM=10\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=30.0 FNUM=5\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=60.0 FNUM=3\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=180.0 FNUM=3\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=300.0 FNUM=3\n");
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
	IRD_WriteOPE_OBJ_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_SetAzEl:
	fprintf(fp, "### SetAzEl  Az=%d El=%d  via Launcher ###\n\n",(int)hg->plan[i_plan].az1,(int)hg->plan[i_plan].el1);
	break;
	
      case PLAN_TYPE_FOCUS:
	IRD_WriteOPE_FOCUS_plan(fp,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_FLAT:
	IRD_WriteOPE_FLAT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_COMP:
	IRD_WriteOPE_COMP_plan(fp,hg,hg->plan[i_plan]);
	break;
	
      case PLAN_TYPE_DARK:
	IRD_WriteOPE_DARK_plan(fp,hg,hg->plan[i_plan]);
	break;
      }
    
    }
    fprintf(fp, "##===== FIM Power Supply (NEVER EXEC DURING OBSERVATION!!) =====##\n");
    fprintf(fp, "# Power ON\n");
    fprintf(fp, "EXEC AO188 FIMPWR DEV=MOTOR CMD=ON\n");
    fprintf(fp, "EXEC AO188 FIMPWR DEV=CAMERA CMD=ON\n");
    fprintf(fp, "EXEC AO188 FIMPWR DEV=VENT CMD=ON\n");
    fprintf(fp, "EXEC AO188 FIMPWR DEV=SHUTTER CMD=ON\n");
    fprintf(fp, "\n");
    fprintf(fp, "# Power OFF\n");
    fprintf(fp, "# EXEC AO188 FIMPWR DEV=MOTOR CMD=OFF\n");
    fprintf(fp, "# EXEC AO188 FIMPWR DEV=CAMERA CMD=OFF\n");
    fprintf(fp, "# EXEC AO188 FIMPWR DEV=VENT CMD=OFF\n");
    fprintf(fp, "# EXEC AO188 FIMPWR DEV=SHUTTER CMD=OFF\n");
    fprintf(fp, "##################################################################\n");
    fprintf(fp, "##################################################################\n");
    fprintf(fp, "##################################################################\n");
    fprintf(fp, "##################################################################\n");
  }
  /////////////////// for Plan OPE ///////////////// until here

  
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "#################### Reserved Targets ####################\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    IRD_WriteOPE_OBJ(fp, hg, i_list);
    fprintf(fp, "\n");
    fprintf(fp, "\n");
  }
    
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "##===== FIM Power Supply (NEVER EXEC DURING OBSERVATION!!) =====##\n");
  fprintf(fp, "# Power ON\n");
  fprintf(fp, "EXEC AO188 FIMPWR DEV=MOTOR CMD=ON\n");
  fprintf(fp, "EXEC AO188 FIMPWR DEV=CAMERA CMD=ON\n");
  fprintf(fp, "EXEC AO188 FIMPWR DEV=VENT CMD=ON\n");
  fprintf(fp, "EXEC AO188 FIMPWR DEV=SHUTTER CMD=ON\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Power OFF\n");
  fprintf(fp, "# EXEC AO188 FIMPWR DEV=MOTOR CMD=OFF\n");
  fprintf(fp, "# EXEC AO188 FIMPWR DEV=CAMERA CMD=OFF\n");
  fprintf(fp, "# EXEC AO188 FIMPWR DEV=VENT CMD=OFF\n");
  fprintf(fp, "# EXEC AO188 FIMPWR DEV=SHUTTER CMD=OFF\n");
  fprintf(fp, "##################################################################\n");
  fprintf(fp, "##################################################################\n");
  fprintf(fp, "##################################################################\n");
  fprintf(fp, "##################################################################\n");
  fprintf(fp, "</Command>\n");


  fclose(fp);
}


void IRD_WriteOPE_OBJ(FILE*fp, typHOE *hg, gint i_list){
  gchar *tgt=NULL, *ttgs=NULL, *ngs=NULL;
  gboolean flag_nst=FALSE;
  gchar *slew_to=NULL, *tmode=NULL;
  gdouble d_ra, d_dec, d_x, d_y;
  gdouble ss_pos;
  gint i, i10;
  gchar *gs_txt, *gs_mode;
  gboolean ao_ow=FALSE;
  gboolean l_flag=FALSE;
  gint pf;

  
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
      tmode=g_strdup("TRACMODE=ADI");
    }
    else{
      tmode=g_strdup("TRACMODE=SIDEREAL");
    }
  }
  
	       
  // Comment line
  fprintf(fp, "##################################################\n");
  fprintf(fp, "### Object-%d : \"%s\"  ",i_list+1, hg->obj[i_list].name);
  switch(hg->obj[i_list].aomode){
  case AOMODE_NO:
    fprintf(fp, "(w/o AO)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup(" ");
    break;
    
  case AOMODE_NGS_S:
    fprintf(fp, "(NGS by target)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup("$NGS");
    break;
    
  case AOMODE_NGS_O:
    fprintf(fp, "(NGS by offset star)\n");
    gs_txt=get_gs_txt(hg->obj[i_list].gs);
    fprintf(fp, "# Slew to offset NGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ngs);
    gs_mode=g_strdup("$NGS");
    break;
    
  case AOMODE_LGS_S:
    fprintf(fp, "(LGS : TT Guide Star = Target)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup("$LGS");
    break;
    
  case AOMODE_LGS_O:
    fprintf(fp, "(LGS : Offset TTGS)\n"); 
    gs_txt=get_gs_txt(hg->obj[i_list].gs);
    fprintf(fp, "# Slew to offset TTGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ttgs);
    gs_mode=g_strdup("$NGS");
    break;
  }
  fprintf(fp, "##################################################\n");

  if(hg->obj[i_list].adi){
    // SetupField2
    fprintf(fp, "SetupField2 $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    fprintf(fp, "\n");
  
    // SetupField
    fprintf(fp, "# SetupField $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    // MoveTelescope
    fprintf(fp, "## DELTA_N1=(-12,-10) N2=(0,5) NE1=(3,5) NE2=(-9,-5) NW=(0,5) ##\n");
    fprintf(fp, "##       S1=(3,5) SE=(0,6) E=(0,5) SW=(0,-5)                  ##\n");
    if(dec_to_deg(hg->obj[i_list].dec)>hg->obs_latitude){ // North
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_N1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=-12 DELTA_DEC=-10\n");
    }
    else{ // South
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_S1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=3 DELTA_DEC=5\n");
    }
  }
  else{
    // SetupField
    fprintf(fp, "SetupField $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    // MoveTelescope
    fprintf(fp, "## DELTA_N1=(-12,-10) N2=(0,5) NE1=(3,5) NE2=(-9,-5) NW=(0,5) ##\n");
    fprintf(fp, "##       S1=(3,5) SE=(0,6) E=(0,5) SW=(0,-5)                  ##\n");
    if(dec_to_deg(hg->obj[i_list].dec)>hg->obs_latitude){ // North
      fprintf(fp, "MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_N1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=-12 DELTA_DEC=-10\n");
    }
    else{ // South
      fprintf(fp, "MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_S1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=3 DELTA_DEC=5\n");
    }
  }
  
  // Offset for Guide Star
  switch(hg->obj[i_list].aomode){
  case AOMODE_LGS_O:
    fprintf(fp, "\nAO188_OFFSET_RADEC $DEF_AOLN %s\n", tgt);
    break;

  case AOMODE_NGS_O:
    d_ra=(ra_to_deg(hg->obj[i_list].ra)-ra_to_deg(hg->obj[i_list].gs.ra))*3600.0;
    d_dec=(dec_to_deg(hg->obj[i_list].dec)-dec_to_deg(hg->obj[i_list].gs.dec))*3600.0;
    fprintf(fp, "\n# Add Offset to the target from the offset NGS (dRA, dDec)=(%.2lf,%.2lf)\n",
	    d_ra, d_dec);
    break;
  }

  fprintf(fp, "\n### Tip-Tilt Initialize\n");
  fprintf(fp, "TTINIT $SK_ROUTINE\n");
  
  fprintf(fp, "\n### Set AO position\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=5.0\n");
  fprintf(fp, "SETUPSTARPOS $SK_ROUTINE\n");
  fprintf(fp, "SETUPAOP $SK_ROUTINE\n");

  fprintf(fp, "\n### comb PF\n");
  pf=get_pf(hg->obj[i_list].dexp);
  fprintf(fp, "EXEC IRD COMBSHARP MODE=quick POWER=%d\n",pf);
  
  fprintf(fp, "\n### Move star to fiber\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=1.0\n");
  fprintf(fp, "SETUPSTARPOS $SK_ROUTINE\n");
  fprintf(fp, "IRD_AOMOVE $SK_ROUTINE $MMF_STAR\n");

  fprintf(fp, "\n### FIM TT correction\n");
  fprintf(fp, "FIMTT $SK_ROUTINE EXPTIME=5.0 FNUM=3\n");
  
  //fprintf(fp, "\n### comb PF/init PF\n");
  //fprintf(fp, "EXEC IRD COMBSHARP MODE=quick POWER=%d\n", pf);
  //fprintf(fp, "EXEC IRD COMBSHARP MODE=initial POWER=%d\n", pf);

  if(hg->obj[i_list].dexp<10){
    fprintf(fp,  "\n### Exposure  \"%s\"  %dx%.1lfs\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].repeat,hg->obj[i_list].dexp);
    if(hg->obj[i_list].adi){
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "GETOBJECT_AG $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES FIMEXP=0 INTERVAL=0\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "# GETOBJECT $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
    }
    else{
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
    }
  }
  else{
    fprintf(fp,  "\n### Exposure  \"%s\"  %dx%.0lfs\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].repeat,hg->obj[i_list].dexp);
    if(hg->obj[i_list].adi){
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "GETOBJECT_AG $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES FIMEXP=0 INTERVAL=0\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "# GETOBJECT $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
    }
    else{
      i10=0;
      for(i=0;i<hg->obj[i_list].repeat;i++){
	fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		hg->obj[i_list].dexp);
      }
      i10++;
      if( i10 == 10 ){
	fprintf(fp, "\n");
	i10=0;
      }
    }
  }

  fprintf(fp, "\n");

  if(tgt) g_free(tgt);
  if(ttgs) g_free(ttgs);
  if(ngs) g_free(ngs);
  if(slew_to) g_free(slew_to);
  if(tmode) g_free(tmode);
}


void IRD_WriteOPE_OBJ_plan(FILE*fp, typHOE *hg, PLANpara plan){
  gchar *tgt=NULL, *ttgs=NULL, *ngs=NULL;
  gboolean flag_nst=FALSE;
  gchar *slew_to=NULL, *tmode=NULL;
  gdouble d_ra, d_dec, d_x, d_y;
  gdouble ss_pos;
  gint i;
  gchar *gs_txt, *gs_mode;
  gboolean ao_ow=FALSE;
  gboolean l_flag=FALSE;
  gint pf;

  if (plan.backup) fprintf(fp, "## *** BackUp ***\n");
  else if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
    
  if(hg->obj[plan.obj_i].i_nst>=0){ // Non-Sidereal
    tgt=g_strdup_printf("OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
			hg->obj[plan.obj_i].name,
			g_path_get_basename(hg->nst[hg->obj[plan.obj_i].i_nst].filename));
    tmode=g_strdup("TMODE=\"NON-SID\"");
    flag_nst=TRUE;
  }
  else {
    tgt=make_tgt(hg->obj[plan.obj_i].name, "$TGT_");
    if(hg->obj[plan.obj_i].aomode==AOMODE_LGS_O){
      ttgs=make_tgt(hg->obj[plan.obj_i].name, "$TTGS_");
    }
    else if(hg->obj[plan.obj_i].aomode==AOMODE_NGS_O){
      ngs=make_tgt(hg->obj[plan.obj_i].name, "$NGS_");
    }

    if(hg->obj[plan.obj_i].adi){
      tmode=g_strdup("TRACMODE=ADI");
    }
    else{
      tmode=g_strdup("TRACMODE=SIDEREAL");
    }
  }
  
	       
  // Comment line
  fprintf(fp, "##################################################\n");
  fprintf(fp, "### Object-%d : \"%s\"  ",plan.obj_i+1, hg->obj[plan.obj_i].name);
  switch(hg->obj[plan.obj_i].aomode){
  case AOMODE_NO:
    fprintf(fp, "(w/o AO)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup(" ");
    break;
    
  case AOMODE_NGS_S:
    fprintf(fp, "(NGS by target)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup("$NGS");
    break;
    
  case AOMODE_NGS_O:
    fprintf(fp, "(NGS by offset star)\n");
    gs_txt=get_gs_txt(hg->obj[plan.obj_i].gs);
    fprintf(fp, "# Slew to offset NGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ngs);
    gs_mode=g_strdup("$NGS");
    break;
    
  case AOMODE_LGS_S:
    fprintf(fp, "(LGS : TT Guide Star = Target)\n");
    slew_to=g_strdup(tgt);
    gs_mode=g_strdup("$LGS");
    break;
    
  case AOMODE_LGS_O:
    fprintf(fp, "(LGS : Offset TTGS)\n"); 
    gs_txt=get_gs_txt(hg->obj[plan.obj_i].gs);
    fprintf(fp, "# Slew to offset TTGS %s\n",gs_txt);
    g_free(gs_txt);
    slew_to=g_strdup(ttgs);
    gs_mode=g_strdup("$NGS");
    break;
  }
  fprintf(fp, "##################################################\n");

  if(hg->obj[plan.obj_i].adi){
    // SetupField2
    fprintf(fp, "SetupField2 $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    fprintf(fp, "\n");
    // SetupField
    fprintf(fp, "# SetupField $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    // MoveTelescope
    fprintf(fp, "## DELTA_N1=(-12,-10) N2=(0,5) NE1=(3,5) NE2=(-9,-5) NW=(0,5) ##\n");
    fprintf(fp, "##       S1=(3,5) SE=(0,6) E=(0,5) SW=(0,-5)                  ##\n");
    if(dec_to_deg(hg->obj[plan.obj_i].dec)>hg->obs_latitude){ // North
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_N1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=-12 DELTA_DEC=-10\n");
    }
    else{ // South
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_S1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=3 DELTA_DEC=5\n");
    }
  }
  else{
    // SetupField
    fprintf(fp, "SetupField $SK_ROUTINE $ADC %s %s %s\n",
	    slew_to, gs_mode, tmode);
    // MoveTelescope
    fprintf(fp, "## DELTA_N1=(-12,-10) N2=(0,5) NE1=(3,5) NE2=(-9,-5) NW=(0,5) ##\n");
    fprintf(fp, "##       S1=(3,5) SE=(0,6) E=(0,5) SW=(0,-5)                  ##\n");
    if(dec_to_deg(hg->obj[plan.obj_i].dec)>hg->obs_latitude){ // North
      fprintf(fp, "MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_N1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=-12 DELTA_DEC=-10\n");
    }
    else{ // South
      fprintf(fp, "MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE $DELTA_S1\n");
      fprintf(fp, "# MOVETELESCOPE OBE_ID=COMMON OBE_MODE=LAUNCHER OFFSET_MODE=RELATIVE DELTA_RA=3 DELTA_DEC=5\n");
    }
  }
  // Offset for Guide Star
  switch(hg->obj[plan.obj_i].aomode){
  case AOMODE_LGS_O:
    fprintf(fp, "\nAO188_OFFSET_RADEC $DEF_AOLN %s\n", tgt);
    break;

  case AOMODE_NGS_O:
    d_ra=(ra_to_deg(hg->obj[plan.obj_i].ra)-ra_to_deg(hg->obj[plan.obj_i].gs.ra))*3600.0;
    d_dec=(dec_to_deg(hg->obj[plan.obj_i].dec)-dec_to_deg(hg->obj[plan.obj_i].gs.dec))*3600.0;
    fprintf(fp, "\n# Add Offset to the target from the offset NGS (dRA, dDec)=(%.2lf,%.2lf)\n",
	    d_ra, d_dec);
    break;
  }

  fprintf(fp, "\n### Tip-Tilt Initialize\n");
  fprintf(fp, "TTINIT $SK_ROUTINE\n");
  
  fprintf(fp, "\n### Set AO position\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=5.0\n");
  fprintf(fp, "SETUPSTARPOS $SK_ROUTINE\n");
  fprintf(fp, "SETUPAOP $SK_ROUTINE\n");

  fprintf(fp, "\n### comb PF\n");
  pf=get_pf(plan.dexp);
  fprintf(fp, "EXEC IRD COMBSHARP MODE=quick POWER=%d\n",pf);
  
  fprintf(fp, "\n### Move star to fiber\n");
  fprintf(fp, "TAKEFIMIMG $SK_ROUTINE EXPTIME=1.0\n");
  fprintf(fp, "SETUPSTARPOS $SK_ROUTINE\n");
  fprintf(fp, "IRD_AOMOVE $SK_ROUTINE $MMF_STAR\n");

  fprintf(fp, "\n### FIM TT correction\n");
  fprintf(fp, "FIMTT $SK_ROUTINE EXPTIME=5.0 FNUM=3\n");
  
  //fprintf(fp, "\n### comb PF/init PF\n");
  //fprintf(fp, "EXEC IRD COMBSHARP MODE=quick POWER=%d\n", pf);
  //fprintf(fp, "EXEC IRD COMBSHARP MODE=initial POWER=%d\n", pf);

  if(plan.dexp<10){
    fprintf(fp,  "\n### Exposure  \"%s\"  %dx%.1lfs\n",
	    hg->obj[plan.obj_i].name,
	    plan.repeat,plan.dexp);
    if(hg->obj[plan.obj_i].adi){
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "GETOBJECT_AG $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES FIMEXP=0 INTERVAL=0\n",
		plan.dexp);
      }
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "# GETOBJECT $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		plan.dexp);
      }
    }
    else{
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		plan.dexp);
      }
    }
  }
  else{
    fprintf(fp,  "\n### Exposure  \"%s\"  %dx%.0lfs\n",
	    hg->obj[plan.obj_i].name,
	    plan.repeat,plan.dexp);
    if(hg->obj[plan.obj_i].adi){
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "GETOBJECT_AG $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES FIMEXP=0 INTERVAL=0\n",
		plan.dexp);
      }
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "# GETOBJECT $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		plan.dexp);
      }
    }
    else{
      for(i=0;i<plan.repeat;i++){
	fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=YES\n",
		plan.dexp);
      }
    }
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");

  if(tgt) g_free(tgt);
  if(ttgs) g_free(ttgs);
  if(ngs) g_free(ngs);
  if(slew_to) g_free(slew_to);
  if(tmode) g_free(tmode);
}


void IRD_WriteOPE_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set, i, len;
  gint exp10;
  gint i10;
  gchar *cmode=NULL;
  
  switch(plan.cal_mode){
  case IRD_COMP_STAR_COMB:
    fprintf(fp, "###### Change FIM setting ######\n");
    fprintf(fp, "EXEC IRD COMBSHARP MODE=quick POWER=-62\n");
    fprintf(fp, "FIMMIRROR $SK_ROUTINE ACTION=OPEMIRROR POSITION=1\n");
    fprintf(fp, "\n");
    break;
  case IRD_COMP_COMB_THAR:
    fprintf(fp, "###### Change FIM setting ######\n");
    fprintf(fp, "FIMMIRROR $SK_ROUTINE ACTION=OPEMIRROR POSITION=0\n");
    fprintf(fp, "\n");
    break;
  }

  if(!plan.daytime){
    if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  }
  len=strlen("######  %s  #####")-2+strlen(plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "######  %s  #####\n", plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");

  switch(plan.cal_mode){
  case IRD_COMP_STAR_COMB:
    fprintf(fp, "EXEC IRD COMBSHUT OPE1=CLOSE OPE2=open\n");
    break;
  case IRD_COMP_STAR_THAR:
    fprintf(fp, "EXEC IRD HCL_THAR OPE=ON MODE=POWER\n");
    fprintf(fp, "== BREAK ==\n");
    fprintf(fp, "# You can wait in the comb room until ThAr lamp is turned ON.\n");
    fprintf(fp, "EXEC IRD HCL_THAR OPE=12 MODE=FW\n");
    break;
  case IRD_COMP_COMB_THAR:
    fprintf(fp,"EXEC IRD HCL_THAR OPE=8 MODE=FW\n");
    break;
  }
  
  exp10=(gint)(ird_comp_exp[plan.cal_mode]*10.0);

  i10=0;
  for(i=0; i<ird_comp_repeat[plan.cal_mode]; i++){
    if(exp10%10==0){
      fprintf(fp, "GETOBJECT $SK_ROUTINE OBJ=%s EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=COMPARISON ARCHIVE=yes\n",
	      ird_comp_obj[plan.cal_mode],
	      ird_comp_exp[plan.cal_mode]);
    }
    else{
      fprintf(fp, "GETOBJECT $SK_ROUTINE OBJ=%s EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=COMPARISON ARCHIVE=yes\n",
	      ird_comp_obj[plan.cal_mode],
	      ird_comp_exp[plan.cal_mode]);
    }
    i10++;
    if(i10==10){
	fprintf(fp, "\n");
	i10=0;
    }
  }

  switch(plan.cal_mode){
  case IRD_COMP_STAR_COMB:
    fprintf(fp,"EXEC IRD COMBSHUT OPE1=CLOSE OPE2=close\n");
    break;
  case IRD_COMP_COMB_THAR:
    fprintf(fp,"EXEC IRD HCL_THAR OPE=OFF MODE=POWER\n");
    fprintf(fp,"EXEC IRD HCL_THAR OPE=1 MODE=FW\n");
    fprintf(fp,"# You can wait in the comb room until ThAr lamp is turned OFF.\n");
    break;
  }
  
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void IRD_WriteOPE_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set, i, len, i10;
  gint exp10;
  gint nd;  // = Voltage for dome flats
  gdouble amp;
  gint coadds=1;
  gchar *cmode;

  
  switch(plan.cal_mode){
  case IRD_FLAT_COMB:
    fprintf(fp, "###### Change FIM setting ######\n");
    fprintf(fp, "FIMMIRROR $SK_ROUTINE ACTION=OPEMIRROR POSITION=1\n");
    fprintf(fp, "\n");
    break;
  case IRD_FLAT_STAR:
    fprintf(fp, "###### Change FIM setting ######\n");
    fprintf(fp, "FIMMIRROR $SK_ROUTINE ACTION=OPEMIRROR POSITION=0\n");
    fprintf(fp, "\n");
    break;
  }
  
  if(!plan.daytime){
    if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  }
  len=strlen("######  %s  #####")-2+strlen(plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");
  fprintf(fp, "######  %s  #####\n", plan.txt);
  for(i=0;i<len;i++){
    fprintf(fp, "#");
  }
  fprintf(fp, "\n");

  switch(plan.cal_mode){
  case IRD_FLAT_COMB:
    fprintf(fp, "# ==> Insert NsIR CAL probe (from TWS)\n");
    fprintf(fp, "# Turn ON HAL Lamp (from TWS)\n");
    break;
  case IRD_FLAT_STAR:
    fprintf(fp, "# Set ND1 filter (from TWS)\n");
    break;
  }
  
  exp10=(gint)(ird_flat_exp[plan.cal_mode]*10.0);

  i10=0;
  for(i=0; i<ird_flat_repeat[plan.cal_mode]; i++){
    if(exp10%10==0){
      fprintf(fp, "GETOBJECT $SK_ROUTINE OBJ=%s EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=FLAT ARCHIVE=yes\n",
	      ird_flat_obj[plan.cal_mode],
	      ird_flat_exp[plan.cal_mode]);
    }
    else{
      fprintf(fp, "GETOBJECT $SK_ROUTINE OBJ=%s EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=FLAT ARCHIVE=yes\n",
	      ird_flat_obj[plan.cal_mode],
	      ird_flat_exp[plan.cal_mode]);
    }
    i10++;
    if(i10==10){
	fprintf(fp, "\n");
	i10=0;
    }
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void IRD_WriteOPE_DARK_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set, i, len;
  gint exp10;
  gint i10;
  gchar *cmode=NULL;
  

  if(!plan.daytime){
    if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  }
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

  exp10=(gint)(plan.dexp*10.0);

  i10=0;
  for(i=0; i<plan.repeat; i++){
    if(exp10%10==0){
      fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.0lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=dark\n",
	      plan.dexp);
    }
    else{
      fprintf(fp, "GETOBJECT $SK_ROUTINE EXPTIME=%.1lf NCOADD=1 CIMAGE=NO NFOWLER=1 FIMAGE=NO DATA_TYPE=dark\n",
	      plan.dexp);
    }
    i10++;
    if(i10==10){
	fprintf(fp, "\n");
	i10=0;
    }
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void IRD_WriteOPE_FOCUS_plan(FILE *fp, PLANpara plan){
  gdouble z=0.0;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  switch(plan.focus_mode){
  case PLAN_FOCUS1:
    break;

  case PLAN_FOCUS2:
    fprintf(fp, "# [Launcher/AO188(Common)] [ImR] (Free)\n");
    fprintf(fp, "# [Launcher/AO188(Common)] [ImR] (Slew) (Angle=90)\n");
    fprintf(fp, "# Centering of LGS\n");
    fprintf(fp, "AO188_M2CNT $DEF_AOST MODE=LGS INS=AO IMR=DEFAULT LGSH=DEFAULT MODECHANGE=YES\n");
    fprintf(fp, "# LGS Collimation\n");
    fprintf(fp, "AO188_LGSCOL $DEF_AOST EXPTIME=10 MODE=MANUAL\n");
    fprintf(fp, "# [Launcher/AO188(LGS)] [Set LGS Height] (CURRENT)\n");
  }
  fprintf(fp,"\n");
}


gint get_pf(gdouble expt){
  gdouble t;
  gint ret;

  t=(gdouble)expt/60.;
  
  ret=(gint)(-0.0012*t*t*t + 0.0741*t*t -1.7463*t -48.17 -4 +0.5);

  return(ret);
}


// Overheads for AO
gint ird_oh_ao(typHOE *hg, gint aomode, gint obj_i){
  gdouble gs_mag;
  gint oh_ao;
  
  switch(aomode){
  case AOMODE_NO:
    oh_ao=0;
    break;
    
  case AOMODE_LGS_S:
  case AOMODE_LGS_O:
    oh_ao=hg->oh_lgs;
    break;

  case AOMODE_NGS_O:
    gs_mag=hg->obj[obj_i].gs.mag;
    
    if(gs_mag<IRD_NGS_MAG1){
      oh_ao=hg->oh_ngs1;
    }
    else if(gs_mag<IRD_NGS_MAG2){
      oh_ao=hg->oh_ngs2;
    }
    else{
      oh_ao=hg->oh_ngs3;
    }
    break;

  case AOMODE_NGS_S:
    gs_mag=hg->obj[obj_i].mag;
    
    if(gs_mag<IRD_NGS_MAG1){
      oh_ao=hg->oh_ngs1;
    }
    else if(gs_mag<IRD_NGS_MAG2){
      oh_ao=hg->oh_ngs2;
    }
    else{
      oh_ao=hg->oh_ngs3;
    }
    break;
  }

  return(oh_ao);
}


void ird_do_export_def_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gdouble tmp_pa;
  gint tmp_aomode;
  
  
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_IRD)) return;

  tmp_aomode=hg->def_aomode;

  dialog = gtk_dialog_new_with_buttons("HOE : Set Default AO mode",
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
  
  // AO_MODE
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_mode;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_mode=0;i_mode<NUM_AOMODE;i_mode++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, aomode_name[i_mode],
			 1, i_mode, -1);
      if(hg->def_aomode==i_mode) iter_set=iter;
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
		       &tmp_aomode);
  }



  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    hg->def_aomode=tmp_aomode;
    ird_export_def(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}



void IRD_Read_Overhead(typHOE *hg)
{
  ConfigFile *cfgfile;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_fil;
  gchar *ini_file;
  gboolean fail_flag=FALSE;
  gchar *tmp=NULL;

  ini_file=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       IRD_SET_FILE,NULL);
  

  cfgfile = xmms_cfg_open_file(ini_file);

  // Basically this function never overwrite parameters when
  // it fails to find it in the reading ini file.
  if (cfgfile) {
    // General 
    if(hg->ird_overhead_ver) g_free(hg->ird_overhead_ver);
    tmp=
      (xmms_cfg_read_string(cfgfile, "General", "ver",  &c_buf))? c_buf : NULL;
    hg->ird_overhead_ver=g_strdup_printf("<b>%s</b>",tmp);
    if(tmp) g_free(tmp);

    if(hg->ird_overhead_ver){
      if(xmms_cfg_read_int(cfgfile, "Overhead", "Acq",  &i_buf)){
	hg->oh_acq=i_buf;
      }
      if(xmms_cfg_read_int(cfgfile, "Overhead", "NGS1",  &i_buf)){
	hg->oh_ngs1=i_buf;
      }
      if(xmms_cfg_read_int(cfgfile, "Overhead", "NGS2",  &i_buf)){
	hg->oh_ngs2=i_buf;
      }
      if(xmms_cfg_read_int(cfgfile, "Overhead", "NGS3",  &i_buf)){
	hg->oh_ngs3=i_buf;
      }
      if(xmms_cfg_read_int(cfgfile, "Overhead", "LGS",  &i_buf)){
	hg->oh_lgs=i_buf;
      }
    }
    else{
      fail_flag=TRUE;
      hg->ird_overhead_ver=g_strdup("<i>(Not synced yet.)</i>");
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
		  "Failed to load IRD overheads information via network.",
		  "",
		  "     https://" IRD_SET_HOST IRD_SET_PATH,
		  "",
		  "Please try to sync manually later.",
		  NULL);
  }
  else{
    tmp=g_strdup_printf("IRD overheads (Ver. %s) has been loaded",
			hg->ird_overhead_ver);
			
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


void ird_sync_overhead(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar *tmp;
  hg=(typHOE *)gdata;

  ird_overhead_dl(hg);
  IRD_Read_Overhead(hg);

  tmp = g_strdup_printf(" %s", hg->ird_overhead_ver);
  if(GTK_IS_LABEL(hg->ird_label_overhead_ver))
    gtk_label_set_markup(GTK_LABEL(hg->ird_label_overhead_ver), tmp);
  g_free(tmp);
  
  if(GTK_IS_ADJUSTMENT(hg->ird_adj_oh_acq))  gtk_adjustment_set_value(hg->ird_adj_oh_acq, (gdouble)hg->oh_acq);
  if(GTK_IS_ADJUSTMENT(hg->ird_adj_oh_ngs1)) gtk_adjustment_set_value(hg->ird_adj_oh_ngs1, (gdouble)hg->oh_ngs1);
  if(GTK_IS_ADJUSTMENT(hg->ird_adj_oh_ngs2)) gtk_adjustment_set_value(hg->ird_adj_oh_ngs2, (gdouble)hg->oh_ngs2);
  if(GTK_IS_ADJUSTMENT(hg->ird_adj_oh_ngs3)) gtk_adjustment_set_value(hg->ird_adj_oh_ngs3, (gdouble)hg->oh_ngs3);
  if(GTK_IS_ADJUSTMENT(hg->ird_adj_oh_lgs))  gtk_adjustment_set_value(hg->ird_adj_oh_lgs, (gdouble)hg->oh_lgs);
}


void ird_overhead_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gint fcdb_type_tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;
   
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=DBACCESS_IRDSET;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(IRD_SET_HOST);
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  hg->fcdb_path=g_strdup(IRD_SET_PATH);
  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   IRD_SET_FILE,NULL);

  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);

  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Downloading IRD overhead",
		 "Downloading IRD overhead parameters ...",
		 FALSE, FALSE);
  my_signal_connect(hg->pdialog, "delete-event", delete_fcdb, (gpointer)hg);

  gtk_label_set_markup(GTK_LABEL(hg->plabel),
		       "Downloading IRD overhead parameters ...");
 
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

