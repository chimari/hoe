//    HDS OPE file Editor
//      stdtree.c : Standard Star List   
//                                           2018.1.24  A.Tajitsu


#include"main.h"    // 設定ヘッダ
#include"version.h"


void stddb_tree_update_item();
void std_double_cell_data_func();
static void cc_std_sptype();

gboolean flagSTD=FALSE, flag_getSTD=FALSE;

static void cancel_stddb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

#ifdef USE_WIN32
  if(hg->dwThreadID_stddb){
    PostThreadMessage(hg->dwThreadID_stddb, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_stddb, INFINITE);
    CloseHandle(hg->hThread_stddb);
    gtk_main_quit();
  }
#else
  if(stddb_pid){
    kill(stddb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(stddb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    stddb_pid=0;
  }
#endif
}

#ifndef USE_WIN32
void stddb_signal(int sig){
  pid_t child_pid=0;

  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(stddb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
}
#endif

void stddb_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  
  if(flag_getSTD) return;
  flag_getSTD=TRUE;
  
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    
    hg->dss_i=i;
    
    gtk_tree_path_free (path);
  }
  else{
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING,POPUP_TIMEOUT,
		  "Error: Please select a target in the Object List.",
		  NULL);
#else
    fprintf(stderr," Error: Please select a target in the Object List.\n");
#endif
    flag_getSTD=FALSE;
    return;
  }

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog, "delete-event", cancel_stddb, (gpointer)hg);

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  label=gtk_label_new("Searching standards in SIMBAD ...");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->std_file);
  
  hg->plabel=gtk_label_new("Searching standards in SIMBAD ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", cancel_stddb, (gpointer)hg);
  
  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
#ifndef USE_WIN32
  act.sa_handler=stddb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_stddb(hg);
  gtk_main();

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  flag_getSTD=FALSE;
}


void ver_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint fcdb_type_tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;
  
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=-1;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(VER_HOST);
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  hg->fcdb_path=g_strdup(VER_PATH);
  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FCDB_FILE_TXT,NULL);

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog, "delete-event", cancel_fcdb, (gpointer)hg);

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  label=gtk_label_new("Checking the latest version of hoe ...");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->fcdb_file);
  
  hg->plabel=gtk_label_new("Checking the latest version of hoe ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
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


void camz_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  
  if(flag_getSTD) return;
  flag_getSTD=TRUE;
  
  if(hg->std_host) g_free(hg->std_host);
  hg->std_host=g_strdup(CAMZ_HOST);
  if(hg->std_path) g_free(hg->std_path);
  hg->std_path=g_strdup(CAMZ_PATH);
  if(hg->std_file) g_free(hg->std_file);
  hg->std_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FCDB_FILE_TXT,NULL);

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog, "delete-event", cancel_stddb, (gpointer)hg);

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  label=gtk_label_new("Downloading CamZ status ...");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->std_file);
  
  hg->plabel=gtk_label_new("Downloading CamZ status ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", cancel_stddb, (gpointer)hg);
  
  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
#ifndef USE_WIN32
  act.sa_handler=stddb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_stddb(hg);
  gtk_main();

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  flag_getSTD=FALSE;
}


void copy_stacstd(typHOE *hg, const stacSTDpara *stacstd, 
		  gdouble d_ra0, gdouble d_dec0)
{
  gint i_list;
  struct ln_hms hms;
  struct ln_dms dms;

  for(i_list=0;i_list<MAX_STD;i_list++){
    if(hg->std[i_list].name) g_free(hg->std[i_list].name);
    hg->std[i_list].name=g_strdup(stacstd[i_list].name);
    if(!hg->std[i_list].name){
      hg->std_i_max=i_list;
      break;
    }
    
    hg->std[i_list].d_ra=stacstd[i_list].ra;
    hg->std[i_list].ra=deg_to_ra(hg->std[i_list].d_ra);
    
    hg->std[i_list].d_dec=stacstd[i_list].dec;
    hg->std[i_list].dec=deg_to_dec(hg->std[i_list].d_dec);
    
    hg->std[i_list].pmra=stacstd[i_list].pmra;
    hg->std[i_list].pmdec=stacstd[i_list].pmdec;
    if((fabs(hg->std[i_list].pmra)>50)||(fabs(hg->std[i_list].pmdec)>50)){
      hg->std[i_list].pm=TRUE;
    }
    else{
      hg->std[i_list].pm=FALSE;
    }

    if(hg->std[i_list].sp) g_free(hg->std[i_list].sp);
    hg->std[i_list].sp=g_strdup(stacstd[i_list].sp);
    
    hg->std[i_list].rot=stacstd[i_list].rot;
    hg->std[i_list].u=stacstd[i_list].u;
    hg->std[i_list].b=stacstd[i_list].b;
    hg->std[i_list].v=stacstd[i_list].v;
    hg->std[i_list].r=stacstd[i_list].r;
    hg->std[i_list].i=stacstd[i_list].i;
    hg->std[i_list].j=stacstd[i_list].j;
    hg->std[i_list].h=stacstd[i_list].h;
    hg->std[i_list].k=stacstd[i_list].k;
    
    hg->std[i_list].equinox=2000.00;
    hg->std[i_list].sep=deg_sep(d_ra0,d_dec0,
				hg->std[i_list].d_ra,hg->std[i_list].d_dec);
  }
}

void
stddb_toggle (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->stddb_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  if(flagSkymon) draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
}

void
stddb_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    gtk_tree_path_free (path);

    hg->std_i=i;

    object.ra=ra_to_deg(hg->obj[i].ra);
    object.dec=dec_to_deg(hg->obj[i].dec);
    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].equinox),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->stddb_mode){
    case STDDB_SSLOC:
      if(hg->std_host) g_free(hg->std_host);
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      else{
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%26",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      break;
    case STDDB_RAPID:
      if(hg->std_host) g_free(hg->std_host);
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else{
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%26",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      break;
    case STDDB_MIRSTD:
      if(hg->std_host) g_free(hg->std_host);
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	hg->std_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
     if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	  "%7c",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	  "%7c",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else{
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	  "%26",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      break;
    }

    switch(hg->stddb_mode){
    case STDDB_SSLOC:
    case STDDB_RAPID:
    case STDDB_MIRSTD:
      stddb_dl(hg);
      stddb_vo_parse(hg);
      break;
    case STDDB_ESOSTD:
      copy_stacstd(hg,esostd,object_prec.ra,object_prec.dec);
      break;
    case STDDB_IRAFSTD:
      copy_stacstd(hg,irafstd,object_prec.ra,object_prec.dec);
      break;
    case STDDB_CALSPEC:
      copy_stacstd(hg,calspec,object_prec.ra,object_prec.dec);
      break;
    case STDDB_HDSSTD:
      copy_stacstd(hg,hdsstd,object_prec.ra,object_prec.dec);
      break;
    }

    if(hg->skymon_mode==SKYMON_CUR){
      calcpa2_main(hg);
    }
    else if(hg->skymon_mode==SKYMON_SET){
      calcpa2_skymon(hg);
    }

    std_make_tree(NULL, hg);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->stddb_button),
				 TRUE);
    hg->stddb_flag=TRUE;

    // draw_skymon(hg->skymon_dw,hg, FALSE);
  }
}

void std_make_tree(GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->stddb_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->std_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    stddb_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
			   
  stddb_set_label(hg);
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),NOTE_STDDB);
}


void std_add_columns (typHOE *hg,
		      GtkTreeView  *treeview, 
		      GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_STD_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_STD_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_RA));
  column=gtk_tree_view_column_new_with_attributes ("RA",
						   renderer,
						   "text",
						   COLUMN_STD_RA,
						   NULL); 
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_RA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_RA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_DEC));
  column=gtk_tree_view_column_new_with_attributes ("Dec",
						   renderer,
						   "text",
						   COLUMN_STD_DEC,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Sp Type */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_SP));
  column=gtk_tree_view_column_new_with_attributes ("Sp.",
						   renderer,
						   "text",
						   COLUMN_STD_SP,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_SP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Separation */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_SEP));
  column=gtk_tree_view_column_new_with_attributes ("Dist.",
						   renderer,
						   "text",
						   COLUMN_STD_SEP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_SEP),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_SEP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* V sini */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_ROT));
  column=gtk_tree_view_column_new_with_attributes ("V sin(i)",
						   renderer,
						   "text",
						   COLUMN_STD_ROT,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_ROT),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_ROT);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* U */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_U));
  column=gtk_tree_view_column_new_with_attributes ("U",
						   renderer,
						   "text",
						   COLUMN_STD_U,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_U),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_U);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* B */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_B));
  column=gtk_tree_view_column_new_with_attributes ("B",
						   renderer,
						   "text",
						   COLUMN_STD_B,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_B),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_B);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* V */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_V));
  column=gtk_tree_view_column_new_with_attributes ("V",
						   renderer,
						   "text",
						   COLUMN_STD_V,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_V),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_V);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* R */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_R));
  column=gtk_tree_view_column_new_with_attributes ("R",
						   renderer,
						   "text",
						   COLUMN_STD_R,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_R),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_R);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* I */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_I));
  column=gtk_tree_view_column_new_with_attributes ("I",
						   renderer,
						   "text",
						   COLUMN_STD_I,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_I),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_I);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* J */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_J));
  column=gtk_tree_view_column_new_with_attributes ("J",
						   renderer,
						   "text",
						   COLUMN_STD_J,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_J),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_J);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* H */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_H));
  column=gtk_tree_view_column_new_with_attributes ("H",
						   renderer,
						   "text",
						   COLUMN_STD_H,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_H),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_H);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* K */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_K));
  column=gtk_tree_view_column_new_with_attributes ("K",
						   renderer,
						   "text",
						   COLUMN_STD_K,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_K),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_K);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


GtkTreeModel *
std_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_STD, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
			      G_TYPE_STRING,  // Sp_Type
			      G_TYPE_DOUBLE,  // Sep
			      G_TYPE_DOUBLE,  // V_sini
			      G_TYPE_DOUBLE,  // U
			      G_TYPE_DOUBLE,  // B
			      G_TYPE_DOUBLE,  // V
			      G_TYPE_DOUBLE,  // R
			      G_TYPE_DOUBLE,  // I
			      G_TYPE_DOUBLE,  // J
			      G_TYPE_DOUBLE,  // H
			      G_TYPE_DOUBLE);  // K
    /*  IRAS depricated in SIMBAD 2017-04
			      G_TYPE_STRING,  // IRAS F12
			      G_TYPE_STRING,  // IRAS F25
			      G_TYPE_STRING,  // IRAS F60
			      G_TYPE_STRING); // IRAS F100
    */

  for (i = 0; i < hg->std_i_max; i++){
    gtk_list_store_append (model, &iter);
    stddb_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void stddb_tree_update_item(typHOE *hg, 
			    GtkTreeModel *model, 
			    GtkTreeIter iter, 
			    gint i_list)
{
  gchar tmp[24];
  gint i;
  gdouble s_rt=-1;

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_STD_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_STD_NAME,
		      hg->std[i_list].name,
		      -1);

  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_RA, hg->std[i_list].ra, -1);
  
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_DEC, hg->std[i_list].dec, -1);

  // SpType
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_SP, hg->std[i_list].sp, -1);

  // SEP
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_SEP, hg->std[i_list].sep, -1);

  // Rot
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_ROT, hg->std[i_list].rot, -1);

  // UBVRIJHK
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_U, hg->std[i_list].u,
		     COLUMN_STD_B, hg->std[i_list].b,
		     COLUMN_STD_V, hg->std[i_list].v,
		     COLUMN_STD_R, hg->std[i_list].r,
		     COLUMN_STD_I, hg->std[i_list].i,
		     COLUMN_STD_J, hg->std[i_list].j,
		     COLUMN_STD_H, hg->std[i_list].h,
		     COLUMN_STD_K, hg->std[i_list].k,
		     -1);
}



void std_double_cell_data_func(GtkTreeViewColumn *col , 
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
  case COLUMN_STD_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_STD_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_STD_SEP:
    str=g_strdup_printf("%.1lf",value);
    break;

  case COLUMN_STD_ROT:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%4.0lf",value);
    break;
  case COLUMN_STD_U:
  case COLUMN_STD_B:
  case COLUMN_STD_V:
  case COLUMN_STD_R:
  case COLUMN_STD_I:
  case COLUMN_STD_J:
  case COLUMN_STD_H:
  case COLUMN_STD_K:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void stddb_set_label(typHOE *hg)
{
  if(hg->stddb_label_text) g_free(hg->stddb_label_text);

  if(hg->i_max==0){
    hg->stddb_label_text=g_strdup("Standard List");
    hg->stddb_label_text=g_strdup("Standard List");
    gtk_label_set_text(GTK_LABEL(hg->stddb_label), hg->stddb_label_text);
  }
  else{
    switch(hg->stddb_mode){
    case STDDB_SSLOC:
      if(strcmp(hg->std_cat,"FS")==0){
	hg->stddb_label_text
	  =g_strdup_printf("UKIRT Faint Standard for [%d] %s (%d objects found)",
			   hg->std_i+1,
			   hg->obj[hg->std_i].name,hg->std_i_max);
      }
      else if(strcmp(hg->std_cat,"HIP")==0){
	hg->stddb_label_text
	  =g_strdup_printf("Standard (HIPPARCOS Catalog) for [%d] %s (%d objects found)",
			   hg->std_i+1,
			   hg->obj[hg->std_i].name,hg->std_i_max);
      }
      else if(strcmp(hg->std_cat,"SAO")==0){
	hg->stddb_label_text
	  =g_strdup_printf("Standard (SAO Catalog) for [%d] %s (%d objects found)",
			   hg->std_i+1,
			   hg->obj[hg->std_i].name,hg->std_i_max);
      }
      break;
    case STDDB_RAPID:
      hg->stddb_label_text
	=g_strdup_printf("Rapid rotator for [%d] %s (%d objects found)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
    case STDDB_MIRSTD:
      hg->stddb_label_text
	=g_strdup_printf("Mid-IR standard for [%d] %s (%d objects found)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
    case STDDB_ESOSTD:
      hg->stddb_label_text
	=g_strdup_printf("ESO Optical and UV Spectrophotometric Standard for [%d] %s (all %d objects)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
    case STDDB_IRAFSTD:
      hg->stddb_label_text
	=g_strdup_printf("IRAF Standard in spec16/50 for [%d] %s (all %d objects)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
    case STDDB_CALSPEC:
      hg->stddb_label_text
	=g_strdup_printf("HST CALSPEC Standard for [%d] %s (all %d objects)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
      
    case STDDB_HDSSTD:
      hg->stddb_label_text
	=g_strdup_printf("HDS Efficiency Measument Standard for [%d] %s (all %d objects)",
			 hg->std_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
      break;
    }
    gtk_label_set_text(GTK_LABEL(hg->stddb_label), hg->stddb_label_text);
  }
}


void std_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->stddb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->stddb_tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;
      
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_STD_NUMBER, &i, -1);
      i--;
      hg->stddb_tree_focus=i;
      
      gtk_tree_path_free (path);
      
      if(flagSkymon) draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
    }
}


void std_simbad (GtkWidget *widget, gpointer data)
{
  gchar *tmp;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  gchar *tgt;

  if((hg->stddb_tree_focus>=0)&&(hg->stddb_tree_focus<hg->std_i_max)){
    tgt=make_simbad_id(hg->std[hg->stddb_tree_focus].name);

    tmp=g_strdup_printf(STD_SIMBAD_URL,tgt);
    
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
    g_free(tgt);
    g_free(tmp);
#endif
  }
}

void add_item_std(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;
  OBJpara tmp_obj;
  gint i, i_list, i_use;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path;

  hg=(typHOE *)gdata;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  if(hg->i_max>=MAX_OBJECT) return;
  if((hg->stddb_tree_focus<0)||(hg->stddb_tree_focus>=hg->std_i_max)) return;

  tmp_obj.name=g_strdup(hg->std[hg->stddb_tree_focus].name);

  switch(hg->stddb_mode){
  case STDDB_SSLOC:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("Standard for %s, V=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("Standard for %s, R=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup_printf("Standard for %s",
				   hg->obj[hg->std_i].name);
    break;

  case STDDB_RAPID:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("Rapid Rotator for %s, V=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("Rapid Rotator for %s, R=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup_printf("Rapid Rotator for %s",
				   hg->obj[hg->std_i].name);
    break;

  case STDDB_MIRSTD:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("MIR standard for %s, V=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("MIR standard for %s, R=%.2lf %s",
				   hg->obj[hg->std_i].name,
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup_printf("MIR standard for %s",
				   hg->obj[hg->std_i].name);
    break;

  case STDDB_ESOSTD:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("ESO Opt/UV Standard, V=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("ESO Opt/UV Standard, R=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup("ESO Opt/UV Standard");
    break;

  case STDDB_IRAFSTD:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("IRAF spec16/50 Standard, V=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("IRAF spec16/50 Standard, R=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup("IRAF spec16/50 Standard");
    break;

  case STDDB_CALSPEC:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("HST CALSPEC Standard, V=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("HST CALSPEC Standard, R=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup("HST CALSPEC Standard");
    break;

  case STDDB_HDSSTD:
    if(hg->std[hg->stddb_tree_focus].v<99)
      tmp_obj.note=g_strdup_printf("Standard for HDS-efficiency, V=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].v,
				   hg->std[hg->stddb_tree_focus].sp);
    else if(hg->std[hg->stddb_tree_focus].r<99)
      tmp_obj.note=g_strdup_printf("Standard for HDS-efficiency, R=%.2lf %s",
				   hg->std[hg->stddb_tree_focus].r,
				   hg->std[hg->stddb_tree_focus].sp);
    else
      tmp_obj.note=g_strdup("Standard for HDS-efficiency");
    break;
  }
  
  if(hg->std[hg->stddb_tree_focus].pm){ // Proper Motion
    yrs=current_yrs(hg);
    new_d_ra=hg->std[hg->stddb_tree_focus].d_ra+
      hg->std[hg->stddb_tree_focus].pmra/1000/60/60*yrs;
    new_d_dec=hg->std[hg->stddb_tree_focus].d_dec+
      hg->std[hg->stddb_tree_focus].pmdec/1000/60/60*yrs;
    
    new_ra=deg_to_ra(new_d_ra);
    new_dec=deg_to_dec(new_d_dec);
    
    tmp_obj.ra=new_ra;
    tmp_obj.dec=new_dec;
    tmp_obj.equinox=2000.0;
  }
  else{  // No Proper Motion
    tmp_obj.ra=hg->std[hg->stddb_tree_focus].ra;
    tmp_obj.dec=hg->std[hg->stddb_tree_focus].dec;
    tmp_obj.equinox=hg->std[hg->stddb_tree_focus].equinox;
  }

  tmp_obj.i_nst=-1;
  tmp_obj.exp=30;
  tmp_obj.repeat=1;
  tmp_obj.guide=SV_GUIDE;
  tmp_obj.pa=0;

  tmp_obj.setup[0]=TRUE;
  for(i_use=1;i_use<MAX_USESETUP;i_use++){
    tmp_obj.setup[i_use]=FALSE;
  }

  hg->obj[hg->i_max]=tmp_obj;
  hg->i_max++;

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, hg->i_max-1);
  objtree_update_item(hg, GTK_TREE_MODEL(model), iter, hg->i_max-1);
  
  calc_rst(hg);
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),NOTE_OBJ);

  gtk_widget_grab_focus (hg->objtree);
  path=gtk_tree_path_new_first();
  for(i=0;i<hg->i_max-1;i++){
    gtk_tree_path_next(path);
  }

  gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->objtree), 
			   path, NULL, FALSE);
  gtk_tree_path_free (path);
}


static void cc_std_sptype (GtkWidget *widget, gchar **gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gchar *n;
    GtkTreeModel *model;
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);
  
    g_free(*gdata);
    *gdata=g_strdup(n);
  }
}

void create_std_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale, *frame, *hbox, *check,
    *spinner;
  gint tmp_dra, tmp_ddec, tmp_vsini, tmp_vmag, tmp_iras12, tmp_iras25;
  gint tmp_mag1, tmp_mag2;
  gchar *tmp_sptype, *tmp_cat, *tmp_band, *tmp_sptype2;
  GtkAdjustment *adj;
  confProp *cdata;
  typHOE *hg;
  gint i;
  GSList *group=NULL;
 

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  cdata=g_malloc0(sizeof(confProp));
  cdata->mode=0;

  tmp_dra   =hg->std_dra;
  tmp_ddec  =hg->std_ddec;
  tmp_vsini =hg->std_vsini;
  tmp_vmag  =hg->std_vmag;
  tmp_sptype=g_strdup(hg->std_sptype);
  tmp_iras12=hg->std_iras12;
  tmp_iras25=hg->std_iras25;
  tmp_cat   =g_strdup(hg->std_cat);
  tmp_mag1  =hg->std_mag1;
  tmp_mag2  =hg->std_mag2;
  tmp_band  =g_strdup(hg->std_band);
  tmp_sptype2=g_strdup(hg->std_sptype2);

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Change Parameters for Searching Stndards");
  my_signal_connect(dialog,"delete-event",close_disp_para,GTK_WIDGET(dialog));

  frame = gtk_frame_new ("Sky Area");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // delta_RA
  label = gtk_label_new ("dRA [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_dra,
					    5.0, 50.0, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_dra);

  // delta_Dec
  label = gtk_label_new ("        dDec [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_ddec,
					    5, 20, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_ddec);

  frame = gtk_frame_new ("Standard Star Locator");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // Catalog
  label = gtk_label_new ("Catalog");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "UKIRT Faint Standards", 1, "FS",
		       2, 0, -1);
    if(strcmp(hg->std_cat,"FS")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HIPPARCOS", 1, "HIP",
		       2, 1, -1);
    if(strcmp(hg->std_cat,"HIP")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SAO Catalog",1, "SAO",
		       2, 2, -1);
    if(strcmp(hg->std_cat,"SAO")==0) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_cat);
  }

  label = gtk_label_new ("Magnitude");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 4, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_mag1,
					    5, 15, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_mag1);

  label = gtk_label_new ("<");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "B", 1, "Bmag",
		       2, 0, -1);
    if(strcmp(hg->std_band,"Bmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "V", 1, "Vmag",
		       2, 1, -1);
    if(strcmp(hg->std_band,"Vmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "R", 1, "Rmag",
		       2, 2, -1);
    if(strcmp(hg->std_band,"Rmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "I", 1, "Imag",
		       2, 3, -1);
    if(strcmp(hg->std_band,"Imag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "J", 1, "Jmag",
		       2, 4, -1);
    if(strcmp(hg->std_band,"Jmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H", 1, "Hmag",
		       2, 5, -1);
    if(strcmp(hg->std_band,"Hmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "K", 1, "Kmag",
		       2, 6, -1);
    if(strcmp(hg->std_band,"Kmag")==0) iter_set=iter;
	
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_band);
  }

  label = gtk_label_new ("<");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_mag2,
					    5, 15, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_mag2);


  label = gtk_label_new ("Spectral Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All", 1, STD_SPTYPE_ALL,
		       2, 0, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_ALL)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "O", 1, STD_SPTYPE_O,
		       2, 1, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_O)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "B", 1, STD_SPTYPE_B,
		       2, 2, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_B)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "A", 1, STD_SPTYPE_A,
		       2, 3, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_A)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "F", 1, STD_SPTYPE_F,
		       2, 4, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_F)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "G", 1, STD_SPTYPE_G,
		       2, 5, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_G)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "K", 1, STD_SPTYPE_K,
		       2, 6, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_K)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "M", 1, STD_SPTYPE_M,
		       2, 7, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_M)==0) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_sptype2);
  }


  frame = gtk_frame_new ("Rapid Rotators for High Dispersion Spectroscopy");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // V_sini
  label = gtk_label_new ("V_sin(i) [km/s]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vsini,
					    50, 300, 
					    10,10,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vsini);

  // Vmag
  label = gtk_label_new ("     V mag  <");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vmag,
					    5, 12, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vmag);

  
  label = gtk_label_new ("      Spectral Type");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< B5", 1, "B5",
		       2, 0, -1);
    if(strcmp(hg->std_sptype,"B5")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< A0", 1, "A0",
		       2, 1, -1);
    if(strcmp(hg->std_sptype,"A0")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< A5",1, "A5",
		       2, 2, -1);
    if(strcmp(hg->std_sptype,"A5")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< F0", 1, "F0",
		       2, 3, -1);
    if(strcmp(hg->std_sptype,"F0")==0) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_sptype);
  }

  frame = gtk_frame_new ("Mid-IR Standard for COMICS");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // IRAS 12um
  label = gtk_label_new ("IRAS F(12um) [Jy]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras12,
					    3, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras12);

  // IRAS 25um
  label = gtk_label_new ("     F(25um) [Jy]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras25,
					    5, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras25);




  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_disp_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
      hg->std_dra   =tmp_dra;
      hg->std_ddec  =tmp_ddec;
      hg->std_vsini =tmp_vsini;
      hg->std_vmag  =tmp_vmag;
      if(hg->std_sptype) g_free(hg->std_sptype);
      hg->std_sptype=g_strdup(tmp_sptype);
      hg->std_iras12=tmp_iras12;
      hg->std_iras25=tmp_iras25;
      if(hg->std_cat) g_free(hg->std_cat);
      hg->std_cat   =g_strdup(tmp_cat);
      if(tmp_mag1>tmp_mag2){
	hg->std_mag1  =tmp_mag2;
	hg->std_mag2  =tmp_mag1;
      }
      else{
	hg->std_mag1  =tmp_mag1;
	hg->std_mag2  =tmp_mag2;
      }
      if(hg->std_band) g_free(hg->std_band);
      hg->std_band  =g_strdup(tmp_band);
      if(hg->std_sptype2) g_free(hg->std_sptype2);
      hg->std_sptype2  =g_strdup(tmp_sptype2);
    }
    else{
      hg->std_dra   =STD_DRA;
      hg->std_ddec  =STD_DDEC;
      hg->std_vsini =STD_VSINI;
      hg->std_vmag  =STD_VMAG;
      if(hg->std_sptype) g_free(hg->std_sptype);
      hg->std_sptype=g_strdup(STD_SPTYPE);
      hg->std_iras12=STD_IRAS12;
      hg->std_iras25=STD_IRAS25;
      if(hg->std_cat) g_free(hg->std_cat);
      hg->std_cat   =g_strdup(STD_CAT);
      hg->std_mag1  =STD_MAG1;
      hg->std_mag2  =STD_MAG2;
      if(hg->std_band) g_free(hg->std_band);
      hg->std_band  =g_strdup(STD_BAND);
      if(hg->std_sptype2) g_free(hg->std_sptype2);
      hg->std_sptype2  =g_strdup(STD_SPTYPE_ALL);
    }
  }

  flagChildDialog=FALSE;
  g_free(tmp_sptype);
  g_free(tmp_cat);
  g_free(tmp_band);
  g_free(tmp_sptype2);
  g_free(cdata);
}


void make_std_tgt(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar *tmp, *tgt;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;

  hg=(typHOE *)gdata;


  if((hg->stddb_tree_focus>=0)&&(hg->stddb_tree_focus<hg->std_i_max)){
    tgt=make_tgt(hg->std[hg->stddb_tree_focus].name);
    if(hg->std[hg->stddb_tree_focus].pm){
      yrs=current_yrs(hg);
      new_d_ra=hg->std[hg->stddb_tree_focus].d_ra+
	hg->std[hg->stddb_tree_focus].pmra/1000/60/60*yrs;
      new_d_dec=hg->std[hg->stddb_tree_focus].d_dec+
	hg->std[hg->stddb_tree_focus].pmdec/1000/60/60*yrs;

      new_ra=deg_to_ra(new_d_ra);
      new_dec=deg_to_dec(new_d_dec);
    
      tmp=g_strdup_printf("PM%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			  tgt,hg->std[hg->stddb_tree_focus].name,
			  new_ra,new_dec,2000.00);
    }
    else{
      tmp=g_strdup_printf("%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
			  tgt,hg->std[hg->stddb_tree_focus].name,
			  hg->std[hg->stddb_tree_focus].ra,hg->std[hg->stddb_tree_focus].dec,
			  hg->std[hg->stddb_tree_focus].equinox);
    }
    g_free(tgt);
    gtk_entry_set_text(GTK_ENTRY(hg->std_tgt),tmp);
    if(tmp) g_free(tmp);
  }
}
