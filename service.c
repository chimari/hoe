// service.c for Subaru Service Prorams
//             Mar 2019  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"

gboolean do_calc_service (GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  gboolean ret;
  
  hg=(typHOE *)gdata;

  if(flagService) close_service(NULL,(gpointer)hg);
  
  ret=create_calc_service_dialog(hg);

  return(ret);
}


void calc_service_sem(typHOE *hg, gint sem_year, gint sem_ab,
		      gint el_min, gint delay_min, gint svc_night){
  struct ln_zonedate zonedate;
  gdouble JD, JD_end;
  gint i_plan, i_cal=0;
  gchar *tmp;

  if(sem_ab==SEMESTER_A){
    zonedate.years=sem_year+2000;
    zonedate.months=2;
    zonedate.days=1; 
    zonedate.hours=12; 
    zonedate.minutes=0; 
    zonedate.seconds=0; 
    zonedate.gmtoff=(long)(hg->obs_timezone*60);
    JD=ln_get_julian_local_date(&zonedate);

    zonedate.years=sem_year+2000;
    zonedate.months=8;
    zonedate.days=1; 
    zonedate.hours=12; 
    zonedate.minutes=0; 
    zonedate.seconds=0; 
    zonedate.gmtoff=(long)(hg->obs_timezone*60);
    JD_end=ln_get_julian_local_date(&zonedate);
  }
  else{
    zonedate.years=sem_year+2000;
    zonedate.months=8;
    zonedate.days=1; 
    zonedate.hours=12; 
    zonedate.minutes=0; 
    zonedate.seconds=0; 
    zonedate.gmtoff=(long)(hg->obs_timezone*60);
    JD=ln_get_julian_local_date(&zonedate);

    zonedate.years=sem_year+2000+1;
    zonedate.months=2;
    zonedate.days=1; 
    zonedate.hours=12; 
    zonedate.minutes=0; 
    zonedate.seconds=0; 
    zonedate.gmtoff=(long)(hg->obs_timezone*60);
    JD_end=ln_get_julian_local_date(&zonedate);
  }

  hg->service_i_obj_all=0;
  hg->service_time_all=0;
  
  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	hg->service_i_obj_all++;
	hg->service_time_all+=hg->plan[i_plan].time;
      }
    }
  }

  if(hg->service_i_obj_all==0){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please include at least one object for your request.",
		  NULL);
  }

  if(hg->service_time_all>4*60*60){
    tmp=g_strdup_printf("Your total requested obs time is <b>%.1lf hours</b>.",
			(gdouble)hg->service_time_all/60./60.);
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  -1,
		  tmp,
		  " ",
		  "If you request a <b>service program</b>, please undestand <b>the obs time will not exceed 4 hours</b>.",
		  NULL);
    g_free(tmp);
  }

  while(JD < JD_end){
    calc_service_night(hg, i_cal, JD, el_min, delay_min, svc_night);
    JD+=1.0;
    i_cal++;
  }
  hg->service_i_max=i_cal;
}

void calc_service_night(typHOE *hg, gint i_cal, gdouble JD_in,
			gint el_min, gint delay_min, gint svc_night){  
  gint i_plan;
  gint step=1;
  struct ln_lnlat_posn observer;
  struct ln_date date;
  struct ln_zonedate zonedate, zonedate0, zonedate1;
  struct ln_rst_time rst;
  struct ln_equ_posn equ, object, object_prec;
  gboolean started_flag=FALSE, obj_flag;
  gdouble JD, JD_st, JD_ed, JD0, JD1, JD_obj, JD0_calc, JD1_calc;
  my_hms hms;
  struct ln_hrz_posn hrz;
  gdouble JD_st_min=-1, JD_st_max=-1, JD_ed_min=-1, JD_ed_max=-1;
  gint i_obj_good, i_obj_good_max=0;
  gdouble az_old=-1, el_old=-1;
  gint time_obj_good, time_obj_good_max=0;
  gboolean backup[MAX_PLAN];
  
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;
  
  JD = JD_in;

  if (ln_get_solar_rst (JD, &observer, &rst) != 0){
    return;
  }

  ln_get_solar_equ_coords (rst.set, &equ);
  JD0=get_alt_adjusted_rst(rst.set,
			   equ,
			   hg,
			   FALSE)
    + (gdouble)delay_min/60./24.;

  if(rst.rise<rst.set){
    ln_get_solar_rst (JD+1, &observer, &rst);
  }

  ln_get_solar_equ_coords (rst.rise, &equ);
  JD1=get_alt_adjusted_rst(rst.rise,
			   equ,
			   hg,
			   TRUE)
    - (gdouble)delay_min/60./24.;

  switch(svc_night){
  case SVC_NIGHT_FULL:
    JD0_calc=JD0;
    JD1_calc=JD1;
    break;
    
  case SVC_NIGHT_1ST:
    JD0_calc=JD0;
    JD1_calc=(JD0+JD1)/2.;
    break;
    
  case SVC_NIGHT_2ND:
    JD0_calc=(JD0+JD1)/2.;
    JD1_calc=JD1;
    break;
  }

 
  JD_st=JD0_calc;

  JD_st_min=-1;
  JD_ed_min=-1;
  JD_st_max=-1;
  JD_ed_max=-1;

  while(JD_st < JD1_calc){
    i_obj_good=0;
    time_obj_good=0;
    JD=JD_st;
    
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      backup[i_plan]=FALSE;
      if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  object.ra=ra_to_deg(hg->obj[hg->plan[i_plan].obj_i].ra);
	  object.dec=dec_to_deg(hg->obj[hg->plan[i_plan].obj_i].dec);
	  
	  // Start
	  JD_obj=JD;
	  ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->obj[hg->plan[i_plan].obj_i].equinox),
		      JD_obj, &object_prec); 
	  ln_get_hrz_from_equ (&object_prec, &observer, JD_obj, &hrz);
	  obj_flag = (hrz.alt > (gdouble)el_min) ? TRUE : FALSE;
	  
	  // End
	  if(obj_flag){
	    JD_obj+=(gdouble)hg->plan[i_plan].time/60./60./24.;
	    if(el_old>0){
	      JD_obj+=(gdouble)slewtime(az_old, el_old,
					hrz.az, hrz.alt,
					hg->vel_az, hg->vel_el)/60./60./24.;
	    }
	  
	    ln_get_equ_prec2 (&object,
			      get_julian_day_of_epoch(hg->obj[hg->plan[i_plan].obj_i].equinox),
			      JD_obj,
			      &object_prec);
	    ln_get_hrz_from_equ (&object_prec, &observer, JD_obj, &hrz);
	    obj_flag = (hrz.alt > (gdouble)el_min) ? TRUE : FALSE;

	    if( (obj_flag) && (JD_obj < JD1_calc) ){
	      i_obj_good++;
	      time_obj_good+=hg->plan[i_plan].time;

	      az_old=hrz.az;
	      el_old=hrz.alt;
	      
	      JD_ed=JD_obj;
	      JD=JD_ed;
	    }
	    else{
	      backup[i_plan]=TRUE;
	    }
	  }
	  else{
	    backup[i_plan]=TRUE;
	  }
	}
	else{ // Not OBJ
	  JD+=(gdouble)hg->plan[i_plan].time/60./60./24.;
	}
      }
    }

    if(i_obj_good > 0){
      
      if(time_obj_good > time_obj_good_max){
	time_obj_good_max=time_obj_good;
	i_obj_good_max=i_obj_good;
	
	JD_st_min=JD_st;
	JD_ed_min=JD_ed;
	JD_st_max=JD_st;
	JD_ed_max=JD_ed;

	for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
	  hg->service_backup[i_cal][i_plan]=backup[i_plan];
	}
      }
      else if (time_obj_good == time_obj_good_max){
	JD_st_max=JD_st;
	JD_ed_max=JD_ed;
      }
    }

    JD_st+=(gdouble)step/60./24.;
  }

  hg->service_JD_in[i_cal]=JD_in;
  hg->service_JD0[i_cal]=JD0_calc;
  hg->service_JD1[i_cal]=JD1_calc;
  hg->service_JD_st_min[i_cal]=JD_st_min;
  hg->service_JD_st_max[i_cal]=JD_st_max;
  hg->service_JD_ed_min[i_cal]=JD_ed_min;
  hg->service_JD_ed_max[i_cal]=JD_ed_max;
  hg->service_moon[i_cal]=get_moon_age((JD0+JD1)/2.);
  hg->service_i_obj[i_cal]=i_obj_good_max;
  hg->service_time[i_cal]=time_obj_good_max;

  if(hg->service_alloc[i_cal]) g_free(hg->service_alloc[i_cal]);
  ln_get_date (JD0_calc, &date);
  ln_date_to_zonedate(&date,&zonedate0,(long)hg->obs_timezone*60);
  ln_get_date (JD1_calc, &date);
  ln_date_to_zonedate(&date,&zonedate1,(long)hg->obs_timezone*60);
  hg->service_alloc[i_cal]=g_strdup_printf("%d:%02d -- %d:%02d",
					   zonedate0.hours,
					   zonedate0.minutes,
					   zonedate1.hours,
					   zonedate1.minutes);
  
  if(hg->service_min[i_cal]) g_free(hg->service_min[i_cal]);
  if(hg->service_max[i_cal]) g_free(hg->service_max[i_cal]);
  if(JD_st_min>0){
    ln_get_date (JD_st_min, &date);
    ln_date_to_zonedate(&date,&zonedate0,(long)hg->obs_timezone*60);
    ln_get_date (JD_ed_min, &date);
    ln_date_to_zonedate(&date,&zonedate1,(long)hg->obs_timezone*60);
    hg->service_min[i_cal]=g_strdup_printf("%d:%02d --> %d:%02d",
					   zonedate0.hours,
					   zonedate0.minutes,
					   zonedate1.hours,
					   zonedate1.minutes);

    ln_get_date (JD_st_max, &date);
    ln_date_to_zonedate(&date,&zonedate0,(long)hg->obs_timezone*60);
    ln_get_date (JD_ed_max, &date);
    ln_date_to_zonedate(&date,&zonedate1,(long)hg->obs_timezone*60);
    hg->service_max[i_cal]=g_strdup_printf("%d:%02d --> %d:%02d",
					   zonedate0.hours,
					   zonedate0.minutes,
					   zonedate1.hours,
					   zonedate1.minutes);
  }
  else{
    hg->service_min[i_cal]=NULL;
    hg->service_max[i_cal]=NULL;
  }
}


gboolean create_calc_service_dialog (typHOE *hg){
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox0, *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkAdjustment *adj;
  gint iyear, month, iday, hour, min, sec;
  gint sem_year, sem_ab=SEMESTER_A, svc_night=SVC_NIGHT_FULL;
  gint el_min;
  gint delay_min;
  gboolean ret=FALSE;
  
  dialog = gtk_dialog_new_with_buttons("HOE : Input Semester for Service Obs Time Calculation",
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

  hbox0 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox0), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox0,FALSE, FALSE, 0);
  
  frame = gtkut_frame_new ("<b>Semester</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(hbox0),frame,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add(GTK_CONTAINER(frame), hbox);


  label = gtk_label_new ("S");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  get_current_obs_time(hg, &iyear, &month, &iday, &hour, &min, &sec);
  sem_year=iyear-2000;
  
  adj = (GtkAdjustment *)gtk_adjustment_new(sem_year,
					    0, 99, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &sem_year);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  
  // A or B
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "A",
		       1, SEMESTER_A, -1);
    if(sem_ab==SEMESTER_A) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "B",
		       1, SEMESTER_B, -1);
    if(sem_ab==SEMESTER_B) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &sem_ab);
  }

  
  frame = gtkut_frame_new ("<b>Night allocation</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(hbox0),frame,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add(GTK_CONTAINER(frame), hbox);

  // Full / 1st /2nd
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Full Night",
		       1, SVC_NIGHT_FULL, -1);
    if(svc_night==SVC_NIGHT_FULL) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "1st Half",
		       1, SVC_NIGHT_1ST, -1);
    if(svc_night==SVC_NIGHT_1ST) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "2nd Half",
		       1, SVC_NIGHT_2ND, -1);
    if(svc_night==SVC_NIGHT_2ND) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &svc_night);
  }
  
  label = gtk_label_new ("  Delay from Sunset [min]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  delay_min=hg->plan_delay;
  adj = (GtkAdjustment *)gtk_adjustment_new(delay_min,
					    0, 99, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &delay_min);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);

  
  frame = gtkut_frame_new ("<b>Obs Condition</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(hbox0),frame,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add(GTK_CONTAINER(frame), hbox);

  label = gtk_label_new ("Acceptable Telescope El.[deg] >");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  el_min=30;
  adj = (GtkAdjustment *)gtk_adjustment_new(el_min,
					    15, 60, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &el_min);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    calc_service_sem(hg, sem_year, sem_ab, el_min, delay_min, svc_night);
    ret=create_service_dialog(hg, sem_year, sem_ab, el_min, delay_min, svc_night);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  return(ret);
}

void close_service(GtkWidget *w, gpointer gdata)
{
  typHOE *hg=(typHOE *)gdata;
  
  gtk_widget_destroy(hg->service_main);
  flagService=FALSE;
}


gboolean create_service_dialog(typHOE *hg, gint sem_year, gint sem_ab,
			       gint el_min, gint delay_min, gint svc_night){
  GtkWidget *button;
  GtkWidget *sw;
  GtkWidget *hbox, *vbox, *frame, *label;
  gchar *tmp;
  gboolean ret=FALSE;
  GtkTreeModel *items_model;
  
  if(hg->service_i_max<=0) return(FALSE);
  if(flagService) return(FALSE);

  flagService=TRUE;

  hg->service_main = gtk_dialog_new();
  //gtk_window_set_transient_for(GTK_WINDOW(hg->service_main),
  //			       GTK_WINDOW(hg->plan_main));
  gtk_container_set_border_width(GTK_CONTAINER(hg->service_main),5);
  gtk_window_set_title(GTK_WINDOW(hg->service_main),
		       "HOE : Calendar for Service Program");
  my_signal_connect(hg->service_main,"destroy",
		    close_service, 
		    (gpointer)hg);
  
  vbox = gtkut_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->service_main))),
		     vbox,TRUE, TRUE, 0);

  tmp=g_strdup_printf("Semester : S%02d%s", sem_year,
		      (sem_ab==SEMESTER_A) ? "A" : "B");
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  switch(svc_night){
  case SVC_NIGHT_FULL:
    tmp=g_strdup("Night allocation : Full night");
    break;
    
  case SVC_NIGHT_1ST:
    tmp=g_strdup("Night allocation : 1st half");
    break;
    
  case SVC_NIGHT_2ND:
    tmp=g_strdup("Night allocation : 2nd half");
    break;
  }
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  tmp=g_strdup_printf("Delay from Sunset : %d min", delay_min);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  tmp=g_strdup_printf("Acceptable Telescope Elevation : <b>> %d deg</b>", el_min);
  label = gtkut_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);
  
  tmp=g_strdup_printf("Number of Objects : <b>%d</b>", hg->service_i_obj_all);
  label = gtkut_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  if(hg->service_time_all<60*60){
    tmp=g_strdup_printf("Total Obs. Time   : <b>%02d min</b>",
			hg->service_time_all/60);
  }
  else{
    tmp=g_strdup_printf("Total Obs. Time   : <b>%.1lf hour</b>",
			(gdouble)hg->service_time_all/(60*60));
  }
  label = gtkut_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  label = gtkut_label_new ("        <i>Allocation</i> = The ratio of the allocable obs time to the total request");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);

  label = gtkut_label_new ("        <i>Score</i> = The flexibility for time allocation within the night time");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  
  
  // TreeView
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_widget_set_size_request(sw, -1, 500);  

  // create models
  items_model = service_create_items_model (hg);
  
  // create tree view
  hg->service_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->service_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->service_tree)),
			       GTK_SELECTION_SINGLE);
  service_add_columns (hg, GTK_TREE_VIEW (hg->service_tree), items_model);
  
  g_object_unref (items_model);
  
  //g_signal_connect (hg->pam_tree, "cursor-changed",
  //		    G_CALLBACK (focus_service_tree_item), (gpointer)hg);
  
  gtk_container_add (GTK_CONTAINER (sw), hg->service_tree);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Set Date", "go-jump");
#else
  button=gtkut_button_new_from_stock("Set Date", GTK_STOCK_OK);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->service_main),button,GTK_RESPONSE_OK);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CLOSE);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->service_main),button,GTK_RESPONSE_CANCEL);

  gtk_widget_show_all(hg->service_main);

  while (gtk_dialog_run(GTK_DIALOG(hg->service_main)) != GTK_RESPONSE_CANCEL) {
    service_set_date(hg);
    ret=TRUE;
  }
  gtk_widget_destroy(hg->service_main);
  flagService=FALSE;
  return(ret);
}  
  

GtkTreeModel * service_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_SVC, 
			      G_TYPE_INT,     // number
			      G_TYPE_DOUBLE,  // Day (JD)
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //fg color
#else
			      GDK_TYPE_COLOR, //fg color
#endif
			      G_TYPE_STRING,  // Alloc
			      G_TYPE_DOUBLE,  // Moon
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,
			      GDK_TYPE_RGBA,  //fg, bg color
#else
			      GDK_TYPE_COLOR,
			      GDK_TYPE_COLOR, //fg, bg color
#endif
			      G_TYPE_STRING,  // Min
			      G_TYPE_STRING,  // Max
			      G_TYPE_INT,     // Obj
			      G_TYPE_DOUBLE,   // Time
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //bg color
#else
			      GDK_TYPE_COLOR, //bg color
#endif
			      G_TYPE_DOUBLE   // Score
			      );
  
  for (i = 0; i < hg->service_i_max; i++){
    gtk_list_store_append (model, &iter);
    service_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void service_tree_update_item(typHOE *hg, 
			      GtkTreeModel *model, 
			      GtkTreeIter iter, 
			      gint i_list)
{
  gdouble JD_local, dur_s;
#ifdef USE_GTK3
  GdkRGBA color_moon;
#else
  GdkColor color_moon;
#endif
  gdouble ratio;
  gdouble JD_hst;
  struct ln_date date;
  gint day_of_week;
  gdouble score;

  JD_hst=hg->service_JD_in[i_list]+(gdouble)hg->obs_timezone/60.0/24.0;
  if(hg->service_time_all>0){
    ratio=(gdouble)hg->service_time[i_list]/(gdouble)hg->service_time_all*100;
  }
  else{
    ratio=-1;
  }

  if(ratio>0){
    score=(hg->service_JD_ed_max[i_list]-hg->service_JD_st_min[i_list])
      /(hg->service_JD1[i_list]-hg->service_JD0[i_list])*100;
  }
  else{
    score=-1;
  }
  
  // Num
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_SVC_NUMBER,
		      i_list,
		      -1);

  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_SVC_DAY,    JD_hst,
		      COLUMN_SVC_ALLOC,  hg->service_alloc[i_list],
		      COLUMN_SVC_MOON,   hg->service_moon[i_list],
		      COLUMN_SVC_MIN,    hg->service_min[i_list],
		      COLUMN_SVC_MAX,    hg->service_max[i_list],
		      COLUMN_SVC_OBJ,    hg->service_i_obj[i_list],
		      COLUMN_SVC_TIME,   ratio,
		      COLUMN_SVC_SCORE,  score,
		      -1);

  ln_get_date (JD_hst, &date);
  day_of_week=subZeller(date.years, date.months, date.days);

  if(day_of_week==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLFG_DAY,
			&color_red,
			-1);
  }
  else if(day_of_week==6){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLFG_DAY,
			&color_blue,
			-1);
  }
  
#ifdef USE_GTK3
  color_moon.red=(1-fabs(29.5/2-hg->service_moon[i_list])/(29.5/2.));
  color_moon.green=(1-fabs(29.5/2-hg->service_moon[i_list])/(29.5/2.));
  color_moon.blue=0;
  color_moon.alpha=1;
#else
  color_moon.pixel=0;
  color_moon.red=(1-fabs(29.5/2-hg->service_moon[i_list])/(29.5/2.))*0xFFFF;
  color_moon.green=(1-fabs(29.5/2-hg->service_moon[i_list])/(29.5/2.))*0xFFFF;
  color_moon.blue=0;
#endif
  
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_SVC_COLFG_MOON,
		      (fabs(29.5/2-hg->service_moon[i_list]) > 29.5/4.) ? &color_white : &color_black,
		      COLUMN_SVC_COLBG_MOON,
		      &color_moon,
		      -1);

  if(ratio>99){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLBG_TIME,
			&color_lblue,
			-1);
  }
  else if(ratio>50){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLBG_TIME,
			&color_lgreen,
			-1);
  }
  else if(ratio>20){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLBG_TIME,
			&color_lorange,
			-1);
  }
  else if(ratio>1){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_SVC_COLBG_TIME,
			&color_lred,
			-1);
  }
}


void service_add_columns (typHOE *hg,
			  GtkTreeView  *treeview, 
			  GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  gchar *tmp;

  // Day
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_DAY));
  tmp=g_strdup_printf("Date (%s)", hg->obs_tzname);
  column=gtk_tree_view_column_new_with_attributes (tmp,
						   renderer,
						   "text", 
						   COLUMN_SVC_DAY,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_SVC_COLFG_DAY,
						   NULL);
  g_free(tmp);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  service_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_SVC_DAY),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Alloc
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_ALLOC));
  column=gtk_tree_view_column_new_with_attributes ("Duration",
						   renderer,
						   "text", 
						   COLUMN_SVC_ALLOC,
						   NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Moon
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_MOON));
  column=gtk_tree_view_column_new_with_attributes ("Moon Age",
						   renderer,
						   "text", 
						   COLUMN_SVC_MOON,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_SVC_COLFG_MOON,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_SVC_COLBG_MOON,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  service_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_SVC_MOON),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
  // Min
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_MIN));
  column=gtk_tree_view_column_new_with_attributes ("Earliest",
						   renderer,
						   "text", 
						   COLUMN_SVC_MIN,
						   NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
  // Max
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_MAX));
  column=gtk_tree_view_column_new_with_attributes ("Latest",
						   renderer,
						   "text", 
						   COLUMN_SVC_MAX,
						   NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
  // Obj
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_OBJ));
  tmp=g_strdup_printf("Obj / %d", hg->service_i_obj_all);
  column=gtk_tree_view_column_new_with_attributes (tmp,
						   renderer,
						   "text", 
						   COLUMN_SVC_OBJ,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  service_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_SVC_OBJ),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Time
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_TIME));
  column=gtk_tree_view_column_new_with_attributes ("Allocation",
						   renderer,
						   "text", 
						   COLUMN_SVC_TIME,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_SVC_COLBG_TIME,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  service_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_SVC_TIME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Score
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_SVC_SCORE));
  column=gtk_tree_view_column_new_with_attributes ("Score",
						   renderer,
						   "text", 
						   COLUMN_SVC_SCORE,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  service_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_SVC_SCORE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


void service_cell_data_func(GtkTreeViewColumn *col , 
			    GtkCellRenderer *renderer,
			    GtkTreeModel *model, 
			    GtkTreeIter *iter,
			    gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  gint  int_value;
  gdouble  double_value;
  struct ln_date date;
  gchar *str=NULL;

  switch (index) {
  case COLUMN_SVC_DAY:
  case COLUMN_SVC_MOON:
  case COLUMN_SVC_TIME:
  case COLUMN_SVC_SCORE:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;

  case COLUMN_SVC_OBJ:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_SVC_DAY:
    if(double_value<0){
      str=NULL;
    }
    else{
      ln_get_date (double_value, &date);
      str=g_strdup_printf("%s %02d, %04d (%s)",
			  cal_month[date.months-1],
			  date.days,
			  date.years,
			  day_name[subZeller(date.years, date.months, date.days)]);
    }
    break;
    
  case COLUMN_SVC_MOON:
    str=g_strdup_printf("%.1lf", double_value);
    break;

  case COLUMN_SVC_TIME:
    if(double_value<0.01){
      str=NULL;
    }
    else{
      str=g_strdup_printf("%.0lf%%", double_value);
    }
    break;
    
  case COLUMN_SVC_SCORE:
    if(double_value<0){
      str=NULL;
    }
    else{
      str=g_strdup_printf("%.0lf", double_value);
    }
    break;
    
  case COLUMN_SVC_OBJ:
    if(int_value>0){
      str=g_strdup_printf("%d", int_value);
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void service_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->service_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));

  for (i = 0; i < hg->service_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    service_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
}

int subZeller( int y, int m, int d )
{
    if( m < 3 ) {
        y--; m += 12;
    }
    return ( y + y/4 - y/100 + y/400 + ( 13*m + 8 )/5 + d )%7;
}


void service_set_date (typHOE *hg){
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->service_tree));
  GtkTreeModel *pmodel;
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->service_tree));
  gint i_cal,i_plan;
  GtkTreePath *path;
  struct ln_date date;
  struct ln_zonedate zonedate;
  gdouble JD;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i_cal = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);

    ln_get_date (hg->service_JD_in[i_cal], &date);
    ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*60);
    hg->fr_year = zonedate.years;
    hg->fr_month= zonedate.months;
    hg->fr_day  = zonedate.days;

    set_fr_e_date(hg);
    
    if(hg->service_JD_st_min[i_cal]>0){
      ln_get_date (hg->service_JD_st_min[i_cal], &date);
      ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*60);
      hg->plan_hour=zonedate.hours;
      hg->plan_min =zonedate.minutes;
      hg->plan_time=hg->plan_hour*60+hg->plan_min;
      hg->plan_start=PLAN_START_SPECIFIC;
      if(flagPlan){
	gtk_adjustment_set_value(hg->plan_adj_min, (gdouble)hg->plan_time);
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_start_combo),
				 hg->plan_start);
      }
    }
    else{
      ln_get_date (hg->service_JD0[i_cal], &date);
      ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*60);
    }
    
    hg->skymon_year = zonedate.years;
    hg->skymon_month= zonedate.months;
    hg->skymon_day  = zonedate.days;
    hg->skymon_hour = zonedate.hours;
    hg->skymon_min  = zonedate.minutes;
    
    if(flagSkymon){
      set_skymon_e_date(hg);
    }

    for(i_plan=0; i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	hg->plan[i_plan].backup=hg->service_backup[i_cal][i_plan];
	if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
	hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
      }
    }

    if(flagPlan){
      pmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
      remake_tod(hg, pmodel);
      
      if(!gtk_tree_model_get_iter_first(pmodel, &iter)) return;
    
      for(i_plan=0; i_plan<hg->i_plan_max;i_plan++){
	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  tree_update_plan_item(hg, pmodel, iter, i_plan);
	}
	if(!gtk_tree_model_iter_next(pmodel, &iter)) break;
      }
      remake_tod(hg, pmodel);
      refresh_plan_plot(hg);
    }
  }
  else{
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please select an obs date in the calender.",
		  NULL);
  }
}
