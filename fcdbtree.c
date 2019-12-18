//    HDS OPE file Editor
//      fcdbtree.c : DB/Finding Chart List
//                                           2018.1.24  A.Tajitsu

#include"main.h"    // 設定ヘッダ

void fcdb_item2();
void fcdb_tree_update_azel_item();
void fcdb_double_cell_data_func();
void fcdb_lamost_afgk_cell_data_func();
void fcdb_kepler_cell_data_func();
void fcdb_akari_cell_data_func();
void fcdb_smoka_cell_data_func();
void fcdb_int_cell_data_func();

void thread_cancel_fcdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg=(typHOE *)gdata;

  gtk_widget_unmap(hg->pdialog);
    
  g_cancellable_cancel(hg->pcancel);
  g_object_unref(hg->pcancel);

  hg->pabort=TRUE;
}

gboolean delete_fcdb(GtkWidget *w, GdkEvent *event, gpointer gdata){
  thread_cancel_fcdb(w, gdata);

  return(TRUE);
}

void fcdb_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gchar *tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);

  tmp=g_strdup_printf("Retrieving <b>%s</b> image from \"<b>%s</b>\" ...",
		      FC_img[hg->fc_mode], FC_host[hg->fc_mode]);
  create_pdialog(hg,
		 (flagFC) ? hg->fc_main : hg->w_top,
		 "HOE : Query to the database",
		 tmp,
		 FALSE, FALSE);
  g_free(tmp);
  my_signal_connect(hg->pdialog,"delete-event", delete_fcdb, (gpointer)hg);

  switch(hg->fcdb_type){
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_LAMOST:
  case FCDB_TYPE_KEPLER:
  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_HST:
  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_SMOKA:
  case FCDB_TYPE_WWWDB_HST:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_SMOKA:
  case TRDB_TYPE_FCDB_HST:
  case TRDB_TYPE_FCDB_ESO:
    hg->fcdb_post=TRUE;
    break;

  default:
    hg->fcdb_post=FALSE;
    break;
  }

  tmp=g_strdup_printf("Searching objects in %s ...",
		      db_name[hg->fcdb_type]);
  gtk_label_set_markup(GTK_LABEL(hg->plabel), tmp);
  g_free(tmp);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    thread_cancel_fcdb, 
		    (gpointer)hg);
  
  gtk_widget_show_all(hg->pdialog);
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
  hg->ploop=g_main_loop_new(NULL, FALSE);
  hg->pcancel=g_cancellable_new();
  hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
  g_main_loop_run(hg->ploop);
  g_thread_join(hg->pthread);
  g_main_loop_unref(hg->ploop);

  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),FALSE);
  if(timer!=-1) g_source_remove(timer);

  flag_getFCDB=FALSE;
  if(GTK_IS_WIDGET(hg->pdialog)) gtk_widget_destroy(hg->pdialog);
}

void fcdb_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  fcdb_item2(hg);
}

void fcdb_item2 (typHOE *hg)
{
  gdouble ra_0, dec_0, d_ra0, d_dec0;
  gchar *mag_str, *otype_str, *url_param=NULL;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gdouble ned_arcmin;
  gint i;

  hg->fcdb_i=hg->dss_i;

  object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);

  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->fcdb_i].equinox),
		    JD2000, &object_prec);

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    switch(hg->fcdb_band){
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
			      simbad_band[hg->fcdb_band],
			      hg->fcdb_mag);
      break;
    }
    
    switch(hg->fcdb_otype){
    case FCDB_OTYPE_ALL:
      otype_str=g_strdup("%0D%0A");
      break;
    case FCDB_OTYPE_STAR:
      otype_str=g_strdup("%26maintypes%3Dstar");
      break;
    case FCDB_OTYPE_ISM:
      otype_str=g_strdup("%26maintypes%3Dism");
      break;
    case FCDB_OTYPE_PN:
      otype_str=g_strdup("%26maintypes%3DPN");
      break;
    case FCDB_OTYPE_HII:
      otype_str=g_strdup("%26maintypes%3DHII");
      break;
    case FCDB_OTYPE_GALAXY:
      otype_str=g_strdup("%26maintypes%3Dgalaxy");
      break;
    case FCDB_OTYPE_QSO:
      otype_str=g_strdup("%26maintypes%3Dqso");
      break;
    case FCDB_OTYPE_GAMMA:
      otype_str=g_strdup("%26maintypes%3Dgamma");
      break;
    case FCDB_OTYPE_X:
      otype_str=g_strdup("%26maintypes%3DX");
      break;
    case FCDB_OTYPE_IR:
      otype_str=g_strdup("%26maintypes%3DIR");
      break;
    case FCDB_OTYPE_RADIO:
      otype_str=g_strdup("%26maintypes%3Dradio");
      break;
    }
    
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

    hg->fcdb_path=g_strdup_printf(FCDB_SIMBAD_PATH_B,
				  hg->fcdb_d_ra0,
				  (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				  fabs(hg->fcdb_d_dec0),
				  (gdouble)hg->dss_arcmin,
				  (gdouble)hg->dss_arcmin,
				  mag_str,otype_str,
				  MAX_FCDB);
    g_free(mag_str);
    g_free(otype_str);
    
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    
    fcdb_dl(hg);

    fcdb_simbad_vo_parse(hg, FALSE);
    break;
    
  case FCDB_TYPE_NED:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_NED);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->dss_arcmin > hg->fcdb_ned_diam){
      ned_arcmin=(gdouble)hg->fcdb_ned_diam;
    }
    else{
      ned_arcmin=(gdouble)hg->dss_arcmin;
    }

    switch(hg->fcdb_ned_otype){
    case FCDB_NED_OTYPE_ALL:
      otype_str=g_strdup("&");
      break;
    case FCDB_NED_OTYPE_EXTRAG:
      otype_str=g_strdup("&in_objtypes1=Galaxies&in_objtypes1=GPairs&in_objtypes1=GTriples&in_objtypes1=GGroups&in_objtypes1=GClusters&in_objtypes1=QSO&in_objtypes1=QSOGroups&in_objtypes1=GravLens&in_objtypes1=AbsLineSys&in_objtypes1=EmissnLine&");
      break;
    case FCDB_NED_OTYPE_QSO:
      otype_str=g_strdup("&in_objtypes1=QSO&in_objtypes1=QSOGroups&in_objtypes1=GravLens&in_objtypes1=AbsLineSys&");
      break;
    case FCDB_NED_OTYPE_STAR:
      otype_str=g_strdup("&in_objtypes3=Star&in_objtypes3=BlueStar&in_objtypes3=RedStar&in_objtypes3=VarStar&in_objtypes3=Walfrayet&in_objtypes3=CarbonStar&in_objtypes3=WhiteDwarf&");
      break;
    case FCDB_NED_OTYPE_SN:
      otype_str=g_strdup("&in_objtypes3=Nova&in_objtypes3=Supernovae&in_objtypes3=SNR&");
      break;
    case FCDB_NED_OTYPE_PN:
      otype_str=g_strdup("&in_objtypes3=PN&");
      break;
    case FCDB_NED_OTYPE_HII:
      otype_str=g_strdup("&in_objtypes3=HIIregion&");
      break;
    }

    if(hobject_prec.dec.neg==0){
      hg->fcdb_path=g_strdup_printf(FCDB_NED_PATH,
				    hobject_prec.ra.hours,
				    hobject_prec.ra.minutes,
				    hobject_prec.ra.seconds,
				    "%2B",hobject_prec.dec.degrees,
				    hobject_prec.dec.minutes,
				    hobject_prec.dec.seconds,
				    ned_arcmin/2.,
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
				    ned_arcmin/2.,
				    otype_str);
    }
    g_free(otype_str);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ned_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_GSC:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_GSC);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_gsc_fil){
      url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->fcdb_gsc_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (hg->dss_arcmin < hg->fcdb_gsc_diam) ?
				  ((double)hg->dss_arcmin/2./60.) :
				  ((double)hg->fcdb_gsc_diam/2./60.),
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_gsc_vo_parse(hg, FALSE);

    break;


  case FCDB_TYPE_PS1:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    if(hg->fcdb_ps1_fil){
      if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
	url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->fcdb_ps1_mag);
      }
      else{
	url_param=g_strdup_printf((hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
				  "&rMeanPSFMag.lte=%d&"
				  : "&rPSFMag.lte=%d&",
				  hg->fcdb_ps1_mag);
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
				    (hg->dss_arcmin > FCDB_PS1_MAX_DIAM) ?
				    (double)FCDB_PS1_MAX_DIAM / 2. /60.:
				    (double)hg->dss_arcmin/2./60.,
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
				    (hg->dss_arcmin > FCDB_PS1_MAX_DIAM) ?
				    (double)FCDB_PS1_MAX_DIAM / 2. /60.:
				    (double)hg->dss_arcmin/2./60.,
				    hg->fcdb_ps1_mindet,
				    url_param);
    }

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ps1_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_SDSS:
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

    fcdb_dl(hg);

    fcdb_sdss_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_LAMOST:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_LAMOST_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_lamost_vo_parse(hg);

    break;

  case FCDB_TYPE_KEPLER:
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

    fcdb_dl(hg);

    fcdb_kepler_vo_parse(hg);

    break;

  case FCDB_TYPE_USNO:
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

    if(hg->fcdb_usno_fil){
      url_param=g_strdup_printf("&R2mag=%%3C%d&",hg->fcdb_usno_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_USNO_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);
    
    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_usno_vo_parse(hg);

    break;

  case FCDB_TYPE_UCAC:
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

    if(hg->fcdb_ucac_fil){
      url_param=g_strdup_printf("&rmag=%%3C%d&",hg->fcdb_ucac_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_UCAC_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);
    
    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ucac_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_GAIA:
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
    
    if(hg->fcdb_gaia_fil){
      url_param=g_strdup_printf("&Gmag=%%3C%d&",hg->fcdb_gaia_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_gaia_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_2MASS:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_2MASS);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    if(hg->fcdb_2mass_fil){
      url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->fcdb_2mass_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_2MASS_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (hg->dss_arcmin < hg->fcdb_2mass_diam) ?
				  ((double)hg->dss_arcmin/2./60.) :
				  ((double)hg->fcdb_2mass_diam/2./60.),
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_2mass_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_WISE:
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
    
    if(hg->fcdb_wise_fil){
      url_param=g_strdup_printf("&W1mag=%%3C%d&",hg->fcdb_wise_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_WISE_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_wise_vo_parse(hg);

    break;

  case FCDB_TYPE_IRC:
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
    
    hg->fcdb_path=g_strdup_printf(FCDB_IRC_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_irc_vo_parse(hg);

    break;

  case FCDB_TYPE_FIS:
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
    
    hg->fcdb_path=g_strdup_printf(FCDB_FIS_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_fis_vo_parse(hg);

    break;

  case FCDB_TYPE_SMOKA:
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

    fcdb_dl(hg);

    fcdb_smoka_txt_parse(hg);

    break;

  case FCDB_TYPE_HST:
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

    fcdb_dl(hg);

    fcdb_hst_vo_parse(hg);

    break;

  case FCDB_TYPE_ESO:
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

    fcdb_dl(hg);

    fcdb_eso_vo_parse(hg);

    break;

  case FCDB_TYPE_GEMINI:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_GEMINI);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    if(hg->fcdb_gemini_inst==GEMINI_INST_ANY){
      hg->fcdb_path=g_strdup_printf(FCDB_GEMINI_PATH,
				    hg->dss_arcmin*30,
				    "/",
				    hg->fcdb_d_ra0,
				    (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				    fabs(hg->fcdb_d_dec0));
    }
    else{
      gchar *g_inst;
      g_inst=g_strdup_printf("/%s/",gemini_inst[hg->fcdb_gemini_inst].prm);

      hg->fcdb_path=g_strdup_printf(FCDB_GEMINI_PATH,
				    hg->dss_arcmin*30,
				    g_inst,
				    hg->fcdb_d_ra0,
				    (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				    fabs(hg->fcdb_d_dec0));
      g_free(g_inst);
    }

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_JSON,NULL);

    fcdb_dl(hg);

    fcdb_gemini_json_parse(hg);

    break;
  }

  fcdb_make_tree(NULL, hg);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
			       TRUE);
  hg->fcdb_flag=TRUE;

  if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
}


void fcdb_make_tree(GtkWidget *widget, gpointer gdata){
  gint i;
  typHOE *hg;
  GtkTreeModel *model;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  flag_nodraw=TRUE;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));

  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->fcdb_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    fcdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  if(hg->fcdb_label_text) g_free(hg->fcdb_label_text);
  if(hg->fcdb_i_all>=MAX_FCDB){
    switch(hg->fcdb_type){
    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_GEMINI:
      hg->fcdb_label_text
	=g_strdup_printf("%s data around [%d] %s [%d frames (over max.)]",
			 db_name[hg->fcdb_type],
			 hg->fcdb_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;

    default:
      hg->fcdb_label_text
	=g_strdup_printf("%s Objects around [%d] %s [%d objects (over max.)]",
			 db_name[hg->fcdb_type],
			 hg->fcdb_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
    break;
    }
#ifdef USE_GTK3
    css_change_col(hg->fcdb_label,"red");
#else
    gtk_widget_modify_fg(hg->fcdb_label,GTK_STATE_NORMAL,&color_red);
#endif
  }
  else{
    switch(hg->fcdb_type){
    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_GEMINI:
      hg->fcdb_label_text
	=g_strdup_printf("%s data around [%d] %s (%d frames found)",
			 db_name[hg->fcdb_type],
			 hg->fcdb_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;

    default:
      hg->fcdb_label_text
	=g_strdup_printf("%s Objects around [%d] %s (%d objects found)",
			 db_name[hg->fcdb_type],
			 hg->fcdb_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;
    }
#ifdef USE_GTK3
    css_change_col(hg->fcdb_label,"black");
#else
    gtk_widget_modify_fg(hg->fcdb_label,GTK_STATE_NORMAL,&color_black);
#endif
  }
  gtk_label_set_text(GTK_LABEL(hg->fcdb_label), hg->fcdb_label_text);

  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), hg->page[NOTE_FCDB]);

  flag_nodraw=FALSE;
}


void
fcdb_add_columns (typHOE *hg,
		 GtkTreeView  *treeview, 
		 GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
  case FCDB_TYPE_NED:
  case FCDB_TYPE_GSC:
  case FCDB_TYPE_PS1:
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_LAMOST:
  case FCDB_TYPE_USNO:
  case FCDB_TYPE_UCAC:
  case FCDB_TYPE_GAIA:
  case FCDB_TYPE_KEPLER:
  case FCDB_TYPE_2MASS:
  case FCDB_TYPE_WISE:
  case FCDB_TYPE_IRC:
  case FCDB_TYPE_FIS:

    /* Name column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NAME));
    column=gtk_tree_view_column_new_with_attributes ("Name",
						     renderer,
						     "text", 
						     COLUMN_FCDB_NAME,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* RA column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_RA));
    column=gtk_tree_view_column_new_with_attributes ("RA",
						     renderer,
						     "text",
						     COLUMN_FCDB_RA,
						     NULL); 
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_RA),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_RA);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* Dec column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_DEC));
    column=gtk_tree_view_column_new_with_attributes ("Dec",
						     renderer,
						     "text",
						     COLUMN_FCDB_DEC,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_DEC),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DEC);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* Separation */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SEP));
    column=gtk_tree_view_column_new_with_attributes ("Dist.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* O-Type */
    if((hg->fcdb_type==FCDB_TYPE_SIMBAD)
       ||(hg->fcdb_type==FCDB_TYPE_NED)||(hg->fcdb_type==FCDB_TYPE_SDSS)){
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_OTYPE));
      column=gtk_tree_view_column_new_with_attributes ("type",
						       renderer,
						       "text",
						       COLUMN_FCDB_OTYPE,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OTYPE);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* Sp Type */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_SP));
      column=gtk_tree_view_column_new_with_attributes ("Sp.",
						       renderer,
						       "text",
						       COLUMN_FCDB_SP,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SP);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* U */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("U",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* B */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("B",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* V */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("V",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* R */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("R",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* I */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("I",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* J */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* H */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("H",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* K */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("K",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_NED){
      /* NED mag */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_NEDMAG));
      column=gtk_tree_view_column_new_with_attributes ("mag.",
						       renderer,
						       "text",
						       COLUMN_FCDB_NEDMAG,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NEDMAG);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* NED z */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_NEDZ));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_NEDZ,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "<i>z</i>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_NEDZ),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NEDZ);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      // References
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_REF));
      column=gtk_tree_view_column_new_with_attributes ("ref.",
						       renderer,
						       "text",
						       COLUMN_FCDB_REF,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_REF);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_int_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_REF),
					      NULL);
    }
    else if(hg->fcdb_type==FCDB_TYPE_GSC){
      /* U */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("U",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* B */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("B",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* V */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("V",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* R */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("R",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* I */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("I",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* J */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* H */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("H",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* K */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("Ks",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_PS1){
      // nDetections
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_REF));
      column=gtk_tree_view_column_new_with_attributes ("nDet.",
						       renderer,
						       "text",
						       COLUMN_FCDB_REF,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_REF);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_int_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_REF),
					      NULL);
      /* g */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("g",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* r */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("r",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* i */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("i",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* z */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("z",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* y */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("y",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* Ap-PSF */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("Ap-PSF (r)",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_SDSS){
      /* u */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("u",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* g */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("g",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* r */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("r",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* i */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("i",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* z */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("z",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* Redshift */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_NEDZ));
      column=gtk_tree_view_column_new_with_attributes ("Z",
						       renderer,
						       "text",
						       COLUMN_FCDB_NEDZ,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_NEDZ),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NEDZ);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_LAMOST){
      /* Teff */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "T<sub>eff</sub>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* log g */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "log <i>g</i>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* [Fe/H] */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("[Fe/H]",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* HRV */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("HRV",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Obj Type */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_OTYPE));
      column=gtk_tree_view_column_new_with_attributes ("type",
						       renderer,
						       "text",
						       COLUMN_FCDB_OTYPE,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OTYPE);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Sp Type */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_SP));
      column=gtk_tree_view_column_new_with_attributes ("Sp.",
						       renderer,
						       "text",
						       COLUMN_FCDB_SP,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SP);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_USNO){
      /* B1 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("B1",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* R1 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("R1",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* B2 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("B2",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* R2 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("R2",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* I */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("I",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_UCAC){
      /* B */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("B",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* g */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("g",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


      /* V */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("V",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      

      /* r */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("r",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      

      /* i */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("i",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      

      /* J */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      

      /* H */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("H",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      

      /* K */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("K",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);     
    }
    else if(hg->fcdb_type==FCDB_TYPE_GAIA){
      /* G */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("G",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* RP */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("RP",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* BP */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("BP",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* AG */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("AG",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* E(BP-RP) */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("E(B-R)",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Parallax */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("Plx(mas)",
						       renderer,
						       "text",
						       COLUMN_FCDB_PLX,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_PLX),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_PLX);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* e_Parallax */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("err(%)",
						       renderer,
						       "text",
						       COLUMN_FCDB_EPLX,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_EPLX),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_EPLX);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* Distance */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("D(kpc)",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* HRV */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("HRV",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Teff */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "T<sub>eff</sub>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_lamost_afgk_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_KEPLER){
      /* Kepler Mag */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("Kep",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* r Mag */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("r",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* J Mag */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Teff */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "T<sub>eff</sub>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* Log g */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "log <i>g</i>");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* [Fe/H] */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("[Fe/H]",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* E(B-V) */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("E(B-V)",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* Radius */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("Radius",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* PM */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_PLX));
      column=gtk_tree_view_column_new_with_attributes ("PM(\"/yr)",
						       renderer,
						       "text",
						       COLUMN_FCDB_PLX,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_PLX),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_PLX);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* g-r */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_EPLX));
      column=gtk_tree_view_column_new_with_attributes ("g-r",
						       renderer,
						       "text",
						       COLUMN_FCDB_EPLX,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_kepler_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_EPLX),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_EPLX);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

      /* 2MASS ID */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_OTYPE));
      column=gtk_tree_view_column_new_with_attributes ("2MASS ID",
						       renderer,
						       "text",
						       COLUMN_FCDB_OTYPE,
						       NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OTYPE);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_2MASS){
      /* J */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* H */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("H",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* K */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("K",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_WISE){
      /* J */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("J",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* H */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("H",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* K */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("K",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* W1 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "3.4&#xB5;m");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* W2 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "4.6&#xB5;m");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* W3 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes (NULL,
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "12&#xB5;m");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* W4 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("22um",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtkut_tree_view_column_set_markup(column, "22&#xB5;m");
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_double_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_IRC){
      /* S09 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("S9W",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S09 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* S18 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("L18W",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S18 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    else if(hg->fcdb_type==FCDB_TYPE_FIS){
      /* S65 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_U));
      column=gtk_tree_view_column_new_with_attributes ("N60",
						       renderer,
						       "text",
						       COLUMN_FCDB_U,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_U),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S65 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_B));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_B,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_B),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* S90 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_V));
      column=gtk_tree_view_column_new_with_attributes ("WIDE-S",
						       renderer,
						       "text",
						       COLUMN_FCDB_V,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_V),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S90 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_R));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_R,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_R),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* S140 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_I));
      column=gtk_tree_view_column_new_with_attributes ("WIDE-L",
						       renderer,
						       "text",
						       COLUMN_FCDB_I,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_I),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S140 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_J));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_J,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_J),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* S160 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_H));
      column=gtk_tree_view_column_new_with_attributes ("N160",
						       renderer,
						       "text",
						       COLUMN_FCDB_H,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_H),
					      NULL);
      gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      
      /* q_S160 */
      renderer = gtk_cell_renderer_text_new ();
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_FCDB_K));
      column=gtk_tree_view_column_new_with_attributes ("Q",
						       renderer,
						       "text",
						       COLUMN_FCDB_K,
						       NULL);
      gtk_tree_view_column_set_cell_data_func(column, renderer,
					      fcdb_akari_cell_data_func,
					      GUINT_TO_POINTER(COLUMN_FCDB_K),
					      NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
    
    break;

  case FCDB_TYPE_SMOKA:
    // Frame ID
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FID));
    column=gtk_tree_view_column_new_with_attributes ("Frame ID",
						     renderer,
						     "text",
						     COLUMN_FCDB_FID,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FID);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Name
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NAME));
    column=gtk_tree_view_column_new_with_attributes ("Name",
						     renderer,
						     "text", 
						     COLUMN_FCDB_NAME,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Date
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_DATE));
    column=gtk_tree_view_column_new_with_attributes ("Date",
						     renderer,
						     "text",
						     COLUMN_FCDB_DATE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DATE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // ExpTime
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_U));
    column=gtk_tree_view_column_new_with_attributes ("Exp.",
						     renderer,
						     "text",
						     COLUMN_FCDB_U,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_smoka_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_U),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Observer
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_OBS));
    column=gtk_tree_view_column_new_with_attributes ("Observer",
						     renderer,
						     "text",
						     COLUMN_FCDB_OBS,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OBS);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Mode
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_MODE));
    column=gtk_tree_view_column_new_with_attributes ("Mode",
						     renderer,
						     "text",
						     COLUMN_FCDB_MODE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_MODE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Type
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_TYPE));
    column=gtk_tree_view_column_new_with_attributes ("Type",
						     renderer,
						     "text",
						     COLUMN_FCDB_TYPE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Filter
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FIL));
    column=gtk_tree_view_column_new_with_attributes ("Filter",
						     renderer,
						     "text",
						     COLUMN_FCDB_FIL,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FIL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Wavelength
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_WV));
    column=gtk_tree_view_column_new_with_attributes ("Wavelength",
						     renderer,
						     "text",
						     COLUMN_FCDB_WV,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_WV);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* Separation */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SEP));
    column=gtk_tree_view_column_new_with_attributes ("Dist.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    break;


  case FCDB_TYPE_HST:
    // Frame ID
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FID));
    column=gtk_tree_view_column_new_with_attributes ("Dataset",
						     renderer,
						     "text",
						     COLUMN_FCDB_FID,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FID);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Name
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NAME));
    column=gtk_tree_view_column_new_with_attributes ("Name",
						     renderer,
						     "text", 
						     COLUMN_FCDB_NAME,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Date
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_DATE));
    column=gtk_tree_view_column_new_with_attributes ("Date",
						     renderer,
						     "text",
						     COLUMN_FCDB_DATE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DATE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // ExpTime
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_U));
    column=gtk_tree_view_column_new_with_attributes ("Exp.",
						     renderer,
						     "text",
						     COLUMN_FCDB_U,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_smoka_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_U),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Instrument
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_MODE));
    column=gtk_tree_view_column_new_with_attributes ("Inst.",
						     renderer,
						     "text",
						     COLUMN_FCDB_MODE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_MODE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Apertures
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_TYPE));
    column=gtk_tree_view_column_new_with_attributes ("Ap.",
						     renderer,
						     "text",
						     COLUMN_FCDB_TYPE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Filter
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FIL));
    column=gtk_tree_view_column_new_with_attributes ("Filter",
						     renderer,
						     "text",
						     COLUMN_FCDB_FIL,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FIL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Central Wavelength
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_V));
    column=gtk_tree_view_column_new_with_attributes ("C.Wv.",
						     renderer,
						     "text",
						     COLUMN_FCDB_V,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_smoka_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_V),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Proposal ID
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_OBS));
    column=gtk_tree_view_column_new_with_attributes ("Prop.ID",
						     renderer,
						     "text",
						     COLUMN_FCDB_OBS,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OBS);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* Separation */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SEP));
    column=gtk_tree_view_column_new_with_attributes ("Dist.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    break;

  case FCDB_TYPE_ESO:
    // Frame ID
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FID));
    column=gtk_tree_view_column_new_with_attributes ("Dataset",
						     renderer,
						     "text",
						     COLUMN_FCDB_FID,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FID);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Name
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NAME));
    column=gtk_tree_view_column_new_with_attributes ("Name",
						     renderer,
						     "text", 
						     COLUMN_FCDB_NAME,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // ExpTime
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_U));
    column=gtk_tree_view_column_new_with_attributes ("Exp.",
						     renderer,
						     "text",
						     COLUMN_FCDB_U,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_smoka_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_U),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Instrument
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_MODE));
    column=gtk_tree_view_column_new_with_attributes ("Inst.",
						     renderer,
						     "text",
						     COLUMN_FCDB_MODE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_MODE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Mode
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_TYPE));
    column=gtk_tree_view_column_new_with_attributes ("Mode",
						     renderer,
						     "text",
						     COLUMN_FCDB_TYPE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Proposal ID
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_OBS));
    column=gtk_tree_view_column_new_with_attributes ("Prop.ID",
						     renderer,
						     "text",
						     COLUMN_FCDB_OBS,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OBS);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Date
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_DATE));
    column=gtk_tree_view_column_new_with_attributes ("Release",
						     renderer,
						     "text",
						     COLUMN_FCDB_DATE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DATE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* Separation */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SEP));
    column=gtk_tree_view_column_new_with_attributes ("Dist.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    break;


  case FCDB_TYPE_GEMINI:
    // Filename
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_FID));
    column=gtk_tree_view_column_new_with_attributes ("Filename",
						     renderer,
						     "text",
						     COLUMN_FCDB_FID,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_FID);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Name
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NAME));
    column=gtk_tree_view_column_new_with_attributes ("Object",
						     renderer,
						     "text", 
						     COLUMN_FCDB_NAME,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // ExpTime
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_U));
    column=gtk_tree_view_column_new_with_attributes ("Exp.",
						     renderer,
						     "text",
						     COLUMN_FCDB_U,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_smoka_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_U),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Instrument
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_MODE));
    column=gtk_tree_view_column_new_with_attributes ("Inst.",
						     renderer,
						     "text",
						     COLUMN_FCDB_MODE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_MODE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Mode
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_TYPE));
    column=gtk_tree_view_column_new_with_attributes ("Mode",
						     renderer,
						     "text",
						     COLUMN_FCDB_TYPE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Wavelength
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_WV));
    column=gtk_tree_view_column_new_with_attributes ("Wavelength",
						     renderer,
						     "text",
						     COLUMN_FCDB_WV,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_WV);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Data Label
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_OBS));
    column=gtk_tree_view_column_new_with_attributes ("Data Label",
						     renderer,
						     "text",
						     COLUMN_FCDB_OBS,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OBS);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Date
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_DATE));
    column=gtk_tree_view_column_new_with_attributes ("UT Date",
						     renderer,
						     "text",
						     COLUMN_FCDB_DATE,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DATE);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* Separation */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SEP));
    column=gtk_tree_view_column_new_with_attributes ("Dist.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    break;
  }

}


GtkTreeModel *
fcdb_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_FCDB, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
			      G_TYPE_DOUBLE,  // Sep
			      G_TYPE_STRING,  // O_Type
			      G_TYPE_STRING,  // Sp_Type
			      G_TYPE_DOUBLE,  // U
			      G_TYPE_DOUBLE,  // B  
			      G_TYPE_DOUBLE,  // V  or g
			      G_TYPE_DOUBLE,  // R  or r
			      G_TYPE_DOUBLE,  // I  or i
			      G_TYPE_DOUBLE,  // J  or z
			      G_TYPE_DOUBLE,  // H  or y
			      G_TYPE_DOUBLE,  // K
			      G_TYPE_STRING,  // NED mag
			      G_TYPE_DOUBLE,  // NED z
			      G_TYPE_INT,     // References or ndetections
			      G_TYPE_DOUBLE,  // Parallax
			      G_TYPE_DOUBLE,  // e_Parallax
			      G_TYPE_STRING,  // Frame ID
			      G_TYPE_STRING,  // Obs Date
			      G_TYPE_STRING,  // Obs Mode
			      G_TYPE_STRING,  // Data Type
			      G_TYPE_STRING,  // Filter
			      G_TYPE_STRING,  // Wavelength
			      G_TYPE_STRING); // Observer

  for (i = 0; i < hg->fcdb_i_max; i++){
    gtk_list_store_append (model, &iter);
    fcdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void fcdb_tree_update_azel_item(typHOE *hg, 
				GtkTreeModel *model, 
				GtkTreeIter iter, 
				gint i_list)
{
  gint i;
  gdouble s_rt=-1;
  gdouble eplx_pc;

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FCDB_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FCDB_NAME,
		      hg->fcdb[i_list].name,
		      -1);

  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_RA, hg->fcdb[i_list].ra, -1);
  
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_DEC, hg->fcdb[i_list].dec, -1);

  // SEP
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_SEP, hg->fcdb[i_list].sep, -1);

  if(hg->fcdb_type==FCDB_TYPE_SIMBAD){
    // O-Type
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, -1);

    // SpType
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_SP, hg->fcdb[i_list].sp, -1);

    // UBVRIJHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_NED){
    // O-Type
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, 
		       COLUMN_FCDB_NEDMAG, hg->fcdb[i_list].nedmag, 
		       COLUMN_FCDB_NEDZ,   hg->fcdb[i_list].nedz, 
		       COLUMN_FCDB_REF, hg->fcdb[i_list].ref, 
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GSC){
    // UBVRIJHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_PS1){
    // grizy
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // g
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i, 
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // z
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // y
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // APT-PSF
		       COLUMN_FCDB_REF, hg->fcdb[i_list].ref,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_SDSS){
    // u g r i z
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // u
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // g
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // i
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // z
		       COLUMN_FCDB_NEDZ,   hg->fcdb[i_list].nedz, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, 
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_LAMOST){
    // Stellar params
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // log g
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // [Fe/H]
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // HRV
		       COLUMN_FCDB_SP, hg->fcdb[i_list].sp, // SpType
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, //Obj Type
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_USNO){
    // B1 R1 B2 R2 I2
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // B1
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // R1
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // B2
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // R2
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // I1
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_UCAC){
    // B g V r i J H K
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // B
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // g
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // V
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // i
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // J
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // H
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,  // K
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GAIA){
    if(hg->fcdb[i_list].eplx<0){
      eplx_pc=-1;
    }
    if(hg->fcdb[i_list].plx<0){
      eplx_pc=-1;
    }
    else{
      eplx_pc=hg->fcdb[i_list].eplx/hg->fcdb[i_list].plx*100;
    }

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // G
		       COLUMN_FCDB_PLX, hg->fcdb[i_list].plx,  // Parallax
		       COLUMN_FCDB_EPLX, eplx_pc,  // e_Parallax
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // BP
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // RP
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // RV
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // AG
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // Distance
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,  // E(BP-RP)
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_KEPLER){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // Kp
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // J
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // log g
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // [Fe/H]
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,  // E(B-V)
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // Radius
		       COLUMN_FCDB_PLX, hg->fcdb[i_list].plx, //PM
		       COLUMN_FCDB_EPLX, hg->fcdb[i_list].eplx, //g-r
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, //2MASS ID
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_2MASS){
    // JHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_WISE){
    // W1-4, JHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_IRC){
    // S09, S18
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_FIS){
    // S65, S90, S140
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_SMOKA){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_FIL, hg->fcdb[i_list].fil,
		       COLUMN_FCDB_WV, hg->fcdb[i_list].wv,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_HST){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_FIL, hg->fcdb[i_list].fil,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_ESO){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GEMINI){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_WV, hg->fcdb[i_list].wv,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
}


void fcdb_double_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_FCDB_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_FCDB_SEP:
    {
      gdouble sec;

      sec=value*3600.;
	
      if(sec<60){
	str=g_strdup_printf("%.1lf\"",sec);
      }
      else{
	str=g_strdup_printf("%d\'%02d\"",
			    (gint)(sec/60),
			    ((gint)sec%60));
      }
    }
    break;

  case COLUMN_FCDB_U:
  case COLUMN_FCDB_B:
  case COLUMN_FCDB_V:
  case COLUMN_FCDB_R:
  case COLUMN_FCDB_I:
  case COLUMN_FCDB_J:
  case COLUMN_FCDB_H:
  case COLUMN_FCDB_K:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;

  case COLUMN_FCDB_NEDZ:
    if(value<-99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.6lf",value);
    break;

  case COLUMN_FCDB_PLX:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;

  case COLUMN_FCDB_EPLX:
    if(value<0) str=g_strdup_printf("---");
    else if(value>100) str=g_strdup_printf(">100");
    else str=g_strdup_printf("%.1lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void fcdb_lamost_afgk_cell_data_func(GtkTreeViewColumn *col , 
				     GtkCellRenderer *renderer,
				     GtkTreeModel *model, 
				     GtkTreeIter *iter,
				     gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_U:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.0lf",value);
    break;

  case COLUMN_FCDB_B:
    if(value<-9) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;

  case COLUMN_FCDB_V:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%+5.2lf",value);
    break;

  case COLUMN_FCDB_R:
  case COLUMN_FCDB_I:
    if(value<-99990) str=g_strdup_printf("---");
    else str=g_strdup_printf("%+5.1lf",value);
    break;

  case COLUMN_FCDB_H:
    if(value<0) str=g_strdup_printf("---");
    else if(value<0.1) str=g_strdup_printf("%.3lf",value);
    else str=g_strdup_printf("%.2lf",value);
    break;

  case COLUMN_FCDB_K:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void fcdb_kepler_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_V:
  case COLUMN_FCDB_R:
  case COLUMN_FCDB_J:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;

  case COLUMN_FCDB_U:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.0lf",value);
    break;

  case COLUMN_FCDB_H:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;

  case COLUMN_FCDB_B:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%+5.2lf",value);
    break;

  case COLUMN_FCDB_K:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;

  case COLUMN_FCDB_I:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;

  case COLUMN_FCDB_PLX:
    if(value<-9999) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;

  case COLUMN_FCDB_EPLX:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.3lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void fcdb_akari_cell_data_func(GtkTreeViewColumn *col , 
			       GtkCellRenderer *renderer,
			       GtkTreeModel *model, 
			       GtkTreeIter *iter,
			       gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_U:  //S09  S65
  case COLUMN_FCDB_V:  //S18  S90
  case COLUMN_FCDB_I:  //S140  
  case COLUMN_FCDB_H:  //S160
    if(value<-99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;

  case COLUMN_FCDB_B:
  case COLUMN_FCDB_R:
  case COLUMN_FCDB_J:
  case COLUMN_FCDB_K:
    str=g_strdup_printf("%d",(gint)value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void fcdb_smoka_cell_data_func(GtkTreeViewColumn *col , 
			       GtkCellRenderer *renderer,
			       GtkTreeModel *model, 
			       GtkTreeIter *iter,
			       gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_U:
  case COLUMN_FCDB_V:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.2lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void fcdb_int_cell_data_func(GtkTreeViewColumn *col , 
			     GtkCellRenderer *renderer,
			     GtkTreeModel *model, 
			     GtkTreeIter *iter,
			     gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_REF:
    if(value==0)
      str=NULL;
    else
      str=g_strdup_printf("%d",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void fcdb_clear_tree(typHOE *hg, gboolean force_flag){
  GtkTreeModel *model;

  flag_nodraw=TRUE;
  if(GTK_IS_TREE_VIEW(hg->fcdb_tree)){
    if((force_flag)||(hg->dss_i!=hg->fcdb_i)){
      model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
      
      gtk_list_store_clear (GTK_LIST_STORE(model));
      hg->fcdb_i_max=0;
    }
  }
  flag_nodraw=FALSE;
}


static void fcdb_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->fcdb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->fcdb_tree));
  
  
  if ((hg->fc_ptn!=-1)&&
      (gtk_tree_selection_get_selected (selection, NULL, &iter)) &&
      (!flag_nodraw))
    {
      gint i;
      GtkTreePath *path;
      
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_FCDB_NUMBER, &i, -1);
      i--;
      hg->fcdb_tree_focus=i;
      
      gtk_tree_path_free (path);
      
      if(flagFC){
	draw_fc_cairo(hg->fc_dw,hg);
      }
    }
}

void rebuild_fcdb_tree(typHOE *hg)
{
  gtk_widget_destroy(GTK_WIDGET(hg->fcdb_tree));

  hg->fcdb_i_max=0;

  flag_nodraw=TRUE;

  fcdb_append_tree(hg);
  gtk_widget_show(hg->fcdb_tree);
  flag_nodraw=FALSE;
}

void fcdb_append_tree(typHOE *hg){
  GtkTreeModel *items_model;

  /* create models */
  items_model = fcdb_create_items_model (hg);
  
  /* create tree view */
  hg->fcdb_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->fcdb_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->fcdb_tree)),
			       GTK_SELECTION_SINGLE);
  fcdb_add_columns (hg, GTK_TREE_VIEW (hg->fcdb_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (hg->fcdb_sw), hg->fcdb_tree);

  my_signal_connect (hg->fcdb_tree, "cursor-changed",
		     G_CALLBACK (fcdb_focus_item), (gpointer)hg);
}    


void
fcdb_simbad (GtkWidget *widget, gpointer data)
{
  gchar *tmp=NULL;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  gchar *tgt;
  gchar *cp, *cpp;
  gchar *shot_name;
  gchar *inst_name;
  gchar *simbad_host;

  if((hg->fcdb_tree_focus>=0)&&(hg->fcdb_tree_focus<hg->fcdb_i_max)){
    switch(hg->fcdb_type){
    case FCDB_TYPE_LAMOST:
      tmp=g_strdup_printf(FCDB_LAMOST_URL,
			  hg->fcdb[hg->fcdb_tree_focus].ref);
      break;

    case FCDB_TYPE_SMOKA:
      if(strncmp(hg->fcdb[hg->fcdb_tree_focus].fid,
		 "HSC",strlen("HSC"))==0){
	if((cp = strstr(hg->fcdb[hg->fcdb_tree_focus].fid, "XX")) != NULL){
	  // Shot Mode
	  shot_name=g_strdup(hg->fcdb[hg->fcdb_tree_focus].fid);
	  strchg(shot_name, "XX", "*");
	  inst_name=g_strdup("HSC");

	  tmp=g_strdup_printf(FCDB_SMOKA_SHOT_URL,
			      shot_name,
			      inst_name);
	  if(shot_name) g_free(shot_name);
	  if(inst_name) g_free(inst_name);
	}
	else{
	  // Frame Mode
	  tmp=g_strdup_printf(FCDB_SMOKA_URL,
			      hg->fcdb[hg->fcdb_tree_focus].fid,
			      hg->fcdb[hg->fcdb_tree_focus].date,
			      hg->fcdb_tree_focus);
	}
      }
      else if(strncmp(hg->fcdb[hg->fcdb_tree_focus].fid,
		      "SUP",strlen("SUP"))==0){
	if((cp = strstr(hg->fcdb[hg->fcdb_tree_focus].fid, "X")) != NULL){
	  // Shot Mode
	  shot_name=g_strdup(hg->fcdb[hg->fcdb_tree_focus].fid);
	  strchg(shot_name, "X", "*");
	  inst_name=g_strdup("SUP");
	  
	  tmp=g_strdup_printf(FCDB_SMOKA_SHOT_URL,
			      shot_name,
			      inst_name);
	  if(shot_name) g_free(shot_name);
	  if(inst_name) g_free(inst_name);
	}
	else{
	  tmp=g_strdup_printf(FCDB_SMOKA_URL,
			      hg->fcdb[hg->fcdb_tree_focus].fid,
			      hg->fcdb[hg->fcdb_tree_focus].date,
			      hg->fcdb_tree_focus);
	}
      }
      else{
	tmp=g_strdup_printf(FCDB_SMOKA_URL,
			    hg->fcdb[hg->fcdb_tree_focus].fid,
			    hg->fcdb[hg->fcdb_tree_focus].date,
			    hg->fcdb_tree_focus);
      }
      break;

    case FCDB_TYPE_HST:
      tmp=g_strdup_printf(FCDB_HST_URL,
			  hg->fcdb[hg->fcdb_tree_focus].fid);
      break;

    case FCDB_TYPE_ESO:
      tmp=g_strdup_printf(FCDB_ESO_URL,
			  hg->fcdb[hg->fcdb_tree_focus].fid);
      break;

    case FCDB_TYPE_GEMINI:
      {
	gchar *c;
	gint i,i_minus=0;

	for(i=0;i<strlen(hg->fcdb[hg->fcdb_tree_focus].obs);i++){
	  if(hg->fcdb[hg->fcdb_tree_focus].obs[i]=='-') i_minus++;
	  if(i_minus==4) break;
	}

	if(i==strlen(hg->fcdb[hg->fcdb_tree_focus].obs)){
	  c=g_strdup(hg->fcdb[hg->fcdb_tree_focus].obs);
	}
	else{
	  c=g_strndup(hg->fcdb[hg->fcdb_tree_focus].obs,i);
	}

	tmp=g_strdup_printf(FCDB_GEMINI_URL, c);
	g_free(c);
      }
      break;

    case FCDB_TYPE_SIMBAD:
    case FCDB_TYPE_NED:
    case FCDB_TYPE_SDSS:
      tgt=make_simbad_id(hg->fcdb[hg->fcdb_tree_focus].name);
      switch(hg->fcdb_type){
      case FCDB_TYPE_SIMBAD:
	if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	  simbad_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
	}
	else{
	  simbad_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
	}
	tmp=g_strdup_printf(STD_SIMBAD_URL,simbad_host,tgt);
	g_free(simbad_host);
	break;
	
      case FCDB_TYPE_NED:
	tmp=g_strdup_printf(FCDB_NED_URL,tgt);
	break;
	
      case FCDB_TYPE_SDSS:
	tmp=g_strdup_printf(FCDB_SDSS_URL,tgt);
	break;
      }
      g_free(tgt);
      break;

    default:
      break;
    }

    if(tmp){

      printf("%s\n",tmp);
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
      g_free(tmp);
#endif
    }
  }
}


void add_item_fcdb(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;
  gint i, i_list, i_use, i_band;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *path;

  hg=(typHOE *)gdata;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  if(hg->i_max>=MAX_OBJECT) return;
  if((hg->fcdb_tree_focus<0)||(hg->fcdb_tree_focus>=hg->fcdb_i_max)) return;

  i=hg->i_max;
  init_obj(&hg->obj[i], hg);

  if(hg->obj[i].name) g_free(hg->obj[i].name);
  if(hg->obj[i].note) g_free(hg->obj[i].note);

  switch(hg->fcdb_type){
  case FCDB_TYPE_GSC:
  case FCDB_TYPE_PS1:
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_USNO:
  case FCDB_TYPE_UCAC:
    hg->obj[i].name=g_strconcat(hg->obj[hg->fcdb_i].name," TTGS",NULL);
    hg->obj[i].note=g_strconcat("added via FC (",hg->obj[hg->fcdb_i].name,")",NULL);
    break;
    
  case FCDB_TYPE_LAMOST:
  case FCDB_TYPE_GAIA:
  case FCDB_TYPE_KEPLER:
  case FCDB_TYPE_2MASS:
  case FCDB_TYPE_WISE:
  case FCDB_TYPE_IRC:
  case FCDB_TYPE_FIS:
  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_HST:
  case FCDB_TYPE_ESO:
  case FCDB_TYPE_GEMINI:
  default:
    hg->obj[i].name=g_strdup(hg->fcdb[hg->fcdb_tree_focus].name);
    hg->obj[i].note=g_strconcat("added via FC (",hg->obj[hg->fcdb_i].name,")",NULL);
    break;
  }
  
  if(hg->fcdb[hg->fcdb_tree_focus].pm){ // Proper Motion
    yrs=current_yrs(hg);
    new_d_ra=hg->fcdb[hg->fcdb_tree_focus].d_ra+
      hg->fcdb[hg->fcdb_tree_focus].pmra/1000/60/60*yrs;
    new_d_dec=hg->fcdb[hg->fcdb_tree_focus].d_dec+
      hg->fcdb[hg->fcdb_tree_focus].pmdec/1000/60/60*yrs;

    new_ra=deg_to_ra(new_d_ra);
    new_dec=deg_to_dec(new_d_dec);
    
    hg->obj[i].ra=new_ra;
    hg->obj[i].dec=new_dec;
    hg->obj[i].equinox=2000.0;
  }
  else{  // No Proper Motion
    hg->obj[i].ra=hg->fcdb[hg->fcdb_tree_focus].ra;
    hg->obj[i].dec=hg->fcdb[hg->fcdb_tree_focus].dec;
    hg->obj[i].equinox=hg->fcdb[hg->fcdb_tree_focus].equinox;
  }

  hg->obj[i].exp=hg->def_exp;
  hg->obj[i].guide=hg->def_guide;
  hg->obj[i].pa=hg->def_pa;

  hg->i_max++;
  
  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, hg->i_max-1);
  objtree_update_item(hg, GTK_TREE_MODEL(model), iter, hg->i_max-1);
  
  calc_rst(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),hg->page[NOTE_OBJ]);

  gtk_widget_grab_focus (hg->objtree);
  path=gtk_tree_path_new_first();
  for(i=0;i<hg->i_max-1;i++){
    gtk_tree_path_next(path);
  }

  gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->objtree), 
			   path, NULL, FALSE);
  gtk_tree_path_free (path);
  
  //trdb_make_tree(hg);
}

void add_item_gs(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;
  gint i, i_list, i_use;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
  case FCDB_TYPE_NED:
  case FCDB_TYPE_GSC:
  case FCDB_TYPE_PS1:
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_USNO:
  case FCDB_TYPE_UCAC:
  case FCDB_TYPE_GAIA:
  case FCDB_TYPE_KEPLER:
  case FCDB_TYPE_2MASS:
  
    hg->obj[hg->fcdb_i].gs.flag=TRUE;
    if(hg->obj[hg->fcdb_i].gs.name) g_free(hg->obj[hg->fcdb_i].gs.name);
    hg->obj[hg->fcdb_i].gs.name=g_strdup(hg->fcdb[hg->fcdb_tree_focus].name);
    hg->obj[hg->fcdb_i].gs.ra=hg->fcdb[hg->fcdb_tree_focus].ra;
    hg->obj[hg->fcdb_i].gs.dec=hg->fcdb[hg->fcdb_tree_focus].dec;
    hg->obj[hg->fcdb_i].gs.equinox=hg->fcdb[hg->fcdb_tree_focus].equinox;
    hg->obj[hg->fcdb_i].gs.sep=hg->fcdb[hg->fcdb_tree_focus].sep;
    
    hg->obj[hg->fcdb_i].gs.src=hg->fcdb_type;

    switch(hg->fcdb_type){
    case FCDB_TYPE_SIMBAD:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].v;
      break;
    case FCDB_TYPE_GSC:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].r;
      break;
    case FCDB_TYPE_PS1:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].r;
      break;
    case FCDB_TYPE_SDSS:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].r;
      break;
    case FCDB_TYPE_USNO:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].r;
      break;
    case FCDB_TYPE_UCAC:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].r;
      break;
    case FCDB_TYPE_GAIA:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].v;
      break;
    case FCDB_TYPE_KEPLER:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].v;
      break;
    case FCDB_TYPE_2MASS:
      hg->obj[hg->fcdb_i].gs.mag=hg->fcdb[hg->fcdb_tree_focus].j;
      break;
    }
    
    make_obj_tree(hg);
    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),hg->page[NOTE_OBJ]);

    if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    break;

  default:
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "This catalog cannot be used for guide star slection.",
		  NULL);
    break;
  }    
}


void make_fcdb_tgt(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar *tmp, *tgt;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;

  hg=(typHOE *)gdata;


  if((hg->fcdb_tree_focus>=0)&&(hg->fcdb_tree_focus<hg->fcdb_i_max)){
    switch(hg->fcdb_type){
    case FCDB_TYPE_GSC:
    case FCDB_TYPE_PS1:
    case FCDB_TYPE_SDSS:
    case FCDB_TYPE_USNO:
    case FCDB_TYPE_UCAC:
      tmp=make_tgt(hg->obj[hg->fcdb_i].name, "TGT_");
      tgt=g_strconcat(tmp,"_TT",NULL);
      if(tmp) g_free(tmp);
      break;

    case FCDB_TYPE_LAMOST:
    case FCDB_TYPE_GAIA:
    case FCDB_TYPE_KEPLER:
    case FCDB_TYPE_2MASS:
    case FCDB_TYPE_WISE:
    case FCDB_TYPE_IRC:
    case FCDB_TYPE_FIS:
    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_GEMINI:
    default:
      tgt=make_tgt(hg->fcdb[hg->fcdb_tree_focus].name, "TGT_");
      break;
    }

    if(hg->fcdb[hg->fcdb_tree_focus].pm){
      yrs=current_yrs(hg);
      new_d_ra=hg->fcdb[hg->fcdb_tree_focus].d_ra+
	hg->fcdb[hg->fcdb_tree_focus].pmra/1000/60/60*yrs;
      new_d_dec=hg->fcdb[hg->fcdb_tree_focus].d_dec+
	hg->fcdb[hg->fcdb_tree_focus].pmdec/1000/60/60*yrs;

      new_ra=deg_to_ra(new_d_ra);
      new_dec=deg_to_dec(new_d_dec);
    
      switch(hg->fcdb_type){
      case FCDB_TYPE_GSC:
      case FCDB_TYPE_PS1:
      case FCDB_TYPE_SDSS:
      case FCDB_TYPE_USNO:
      case FCDB_TYPE_UCAC:
	tmp=g_strdup_printf("PM%s=OBJECT=\"%s TTGS\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			    tgt,hg->obj[hg->fcdb_i].name,
			    new_ra,new_dec,2000.00);
	break;

      case FCDB_TYPE_LAMOST:
      case FCDB_TYPE_GAIA:
      case FCDB_TYPE_KEPLER:
      case FCDB_TYPE_2MASS:
      case FCDB_TYPE_WISE:
      case FCDB_TYPE_IRC:
      case FCDB_TYPE_FIS:
      case FCDB_TYPE_SMOKA:
      case FCDB_TYPE_HST:
      case FCDB_TYPE_ESO:
      case FCDB_TYPE_GEMINI:
      default:
	tmp=g_strdup_printf("PM%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			    tgt,hg->fcdb[hg->fcdb_tree_focus].name,
			    new_ra,new_dec,2000.00);
	break;
      }
    }
    else{
      switch(hg->fcdb_type){
      case FCDB_TYPE_GSC:
      case FCDB_TYPE_PS1:
      case FCDB_TYPE_SDSS:
      case FCDB_TYPE_USNO:
      case FCDB_TYPE_UCAC:
	tmp=g_strdup_printf("%s=OBJECT=\"%s TTGS\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			    tgt,hg->obj[hg->fcdb_i].name,
			    hg->fcdb[hg->fcdb_tree_focus].ra,
			    hg->fcdb[hg->fcdb_tree_focus].dec,
			    hg->fcdb[hg->fcdb_tree_focus].equinox);
	break;
	
      case FCDB_TYPE_LAMOST:
      case FCDB_TYPE_GAIA:
      case FCDB_TYPE_KEPLER:
      case FCDB_TYPE_2MASS:
      case FCDB_TYPE_WISE:
      case FCDB_TYPE_IRC:
      case FCDB_TYPE_FIS:
      case FCDB_TYPE_SMOKA:
      case FCDB_TYPE_HST:
      case FCDB_TYPE_ESO:
      case FCDB_TYPE_GEMINI:
      default:
	tmp=g_strdup_printf("%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			    tgt,hg->fcdb[hg->fcdb_tree_focus].name,
			    hg->fcdb[hg->fcdb_tree_focus].ra,
			    hg->fcdb[hg->fcdb_tree_focus].dec,
			    hg->fcdb[hg->fcdb_tree_focus].equinox);
	break;
      }
    }
    g_free(tgt);
    gtk_entry_set_text(GTK_ENTRY(hg->fcdb_tgt),tmp);
    if(tmp) g_free(tmp);
  }
}


gchar *fcdb_csv_name (typHOE *hg){
  gchar *fname;
  gchar *oname;

  oname=cut_spc(hg->obj[hg->fcdb_i].name);
		
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    fname=g_strconcat("FCDB_", oname, "_by_SIMBAD." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_NED:
    fname=g_strconcat("FCDB_", oname, "_by_NED." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GSC:
    fname=g_strconcat("FCDB_", oname, "_by_GSC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_PS1:
    fname=g_strconcat("FCDB_", oname, "_by_PanSTARRS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SDSS:
    fname=g_strconcat("FCDB_", oname, "_by_SDSS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_LAMOST:
    fname=g_strconcat("FCDB_", oname, "_by_LAMOST." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_USNO:
    fname=g_strconcat("FCDB_", oname, "_by_USNO." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_UCAC:
    fname=g_strconcat("FCDB_", oname, "_by_UCAC4." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GAIA:
    fname=g_strconcat("FCDB_", oname, "_by_GAIA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_KEPLER:
    fname=g_strconcat("FCDB_", oname, "_by_Kepler." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_2MASS:
    fname=g_strconcat("FCDB_", oname, "_by_2MASS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_WISE:
    fname=g_strconcat("FCDB_", oname, "_by_WISE." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_IRC:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_IRC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_FIS:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_FIS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SMOKA:
    fname=g_strconcat("FCDB_", oname, "_by_SMOKA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_HST:
    fname=g_strconcat("FCDB_", oname, "_by_HSTarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_ESO:
    fname=g_strconcat("FCDB_", oname, "_by_ESOarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GEMINI:
    fname=g_strconcat("FCDB_", oname, "_by_GEMINIarchive." CSV_EXTENSION,NULL);
    break;

  default:
    fname=g_strconcat("FCDB_", oname, "_by_hskymon." CSV_EXTENSION,NULL);
    break;
  }

  if(oname) g_free(oname);

  return(fname);
}

