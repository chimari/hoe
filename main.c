//    HOE : Subaru HDS++ OPE file Editor
//    Input File should be ...
//            Object,  RA, DEC, EQUINOX
//         RA = hhmmss.ss
//         DEC = +/-ddmmss.s   real arguments
//   
//                                           2003.10.23  A.Tajitsu

#include"main.h"    // 設定ヘッダ

#ifndef USE_WIN32
void ChildTerm();
#endif // USE_WIN32

//void uri_clicked();
//void do_etc();

void param_init();
void make_obj_list();
//gchar* cut_spc();
//void ObjMagDB_Init();
//void init_inst();

void usage();
void get_option();

//gboolean is_number();

gboolean close_popup();
void destroy_popup();



#ifdef USE_WIN32
gchar* WindowsVersion();
#endif

//void calc_rst();
void RecalcRST();


// CSS for Gtk+3
#ifdef USE_GTK3
void css_change_col(GtkWidget *widget, gchar *color){
  GtkStyleContext *style_context;
  GtkCssProvider *provider = gtk_css_provider_new ();
  gchar tmp[64];
  style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if(gtk_minor_version>=20)  {
    g_snprintf(tmp, sizeof tmp, "button, label { color: %s; }", color);
  } else {
    g_snprintf(tmp, sizeof tmp, "GtkButton, GtkLabel { color: %s; }", color);
  }
  gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), tmp, -1, NULL);
  g_object_unref (provider);
}

void css_change_pbar_height(GtkWidget *widget, gint height){
  GtkStyleContext *style_context;
  GtkCssProvider *provider = gtk_css_provider_new ();
  gchar tmp[64];
  style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if(gtk_minor_version>=20)  {
    g_snprintf(tmp, sizeof tmp, "progress, trough { min-height: %dpx; }", height);
  } else {
    g_snprintf(tmp, sizeof tmp, "GtkProgressBar, trough { min-height: %dpx; }", height);
  }
  gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), tmp, -1, NULL);
  g_object_unref (provider);
}
#endif

gchar* fgets_new(FILE *fp){
  gint c;
  gint i=0, j=0;
  gchar *dbuf=NULL;

  do{
    i=0;
    while(!feof(fp)){
      c=fgetc(fp);
      if((c==0x00)||(c==0x0a)||(c==0x0d)) break;
      i++;
    }
  }while((i==0)&&(!feof(fp)));
  if(feof(fp)){
    if(fseek(fp,(long)(-i+1),SEEK_CUR)!=0) return(NULL);
  }
  else{
    if(fseek(fp,(long)(-i-1),SEEK_CUR)!=0) return(NULL);
  }

  if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(i+2)))==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }
  if(fread(dbuf,1, i, fp)){
    while( (c=fgetc(fp)) !=EOF){
      if((c==0x00)||(c==0x0a)||(c==0x0d))j++;
      else break;
    }
    if(c!=EOF){
      if(fseek(fp,-1L,SEEK_CUR)!=0) return(NULL);
    }
    dbuf[i]=0x00;
    //printf("%s\n",dbuf);
    return(dbuf);
  }
  else{
    return(NULL);
  }
  
}

gboolean
is_separator (GtkTreeModel *model,
	      GtkTreeIter  *iter,
	      gpointer      data)
{
  gboolean result;

  gtk_tree_model_get (model, iter, 2, &result, -1);

  return !result;  
}


#ifdef USE_WIN32
gchar* my_dirname(const gchar *file_name){
  return(g_path_get_dirname(file_name));
}


gchar* get_win_home(void){
  gchar WinPath[257]; 

  GetModuleFileName( NULL, WinPath, 256 );

  return(my_dirname(WinPath));
}

gchar* get_win_temp(void){
  gchar WinPath[257]; 
  
  GetTempPath(256, WinPath);
  if(access(WinPath,F_OK)!=0){
    GetModuleFileName( NULL, WinPath, 256 );
  }

  return(my_dirname(WinPath));
}
#endif

gchar* get_home_dir(void){
#ifdef USE_WIN32
  gchar WinPath[257]; 

  GetModuleFileName( NULL, WinPath, 256 );

  return(my_dirname(WinPath));
#else
  return(g_strdup(g_get_home_dir()));
#endif
}

#ifndef USE_WIN32
// 子プロセスの処理 
void ChildTerm(int dummy)
{
  int s;

  wait(&s);
  signal(SIGCHLD,ChildTerm);
}
#endif // USE_WIN32


void ext_play(char *exe_command)
{
#ifdef USE_WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  if(exe_command){
    ZeroMemory(&si, sizeof(si));
    si.cb=sizeof(si);

    if(CreateProcess(NULL, (LPTSTR)exe_command, NULL, NULL,
		     FALSE, NORMAL_PRIORITY_CLASS,
		     NULL, NULL, &si, &pi)){
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
    }
      
  }
  
#else
  static pid_t pid;
  gchar *cmdline;
  
  if(exe_command){
    waitpid(pid,0,WNOHANG);
    if(strcmp(exe_command,"\0")!=0){
      cmdline=g_strdup(exe_command);
      if( (pid = fork()) == 0 ){
	if(system(cmdline)==-1){
	  fprintf(stderr, "Error: cannot execute command \"%s\"!\n", exe_command);
	  _exit(-1);
	}
	_exit(-1);
	signal(SIGCHLD,ChildTerm);
      }
      g_free(cmdline);
    }
  }
#endif // USE_WIN32
}



void uri_clicked(GtkButton *button,
			gpointer data)
{
  gchar *cmdline;
  typHOE *hg=(typHOE *)data;

#ifdef USE_WIN32
  ShellExecute(NULL, 
	       "open", 
	       DEFAULT_URL,
	       NULL, 
	       NULL, 
	       SW_SHOWNORMAL);
#elif defined(USE_OSX)
  cmdline=g_strconcat("open ",DEFAULT_URL,NULL);

  if(system("open " DEFAULT_URL)==0){
    fprintf(stderr, "Error: Could not open the default www browser.");
  }
  g_free(cmdline);
#else
  cmdline=g_strconcat(hg->www_com," ",DEFAULT_URL,NULL);
  
  ext_play(cmdline);
  g_free(cmdline);
#endif
}


void do_skymon(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagSkymon){
    gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    return;
  }
  else{
    flagSkymon=TRUE;
  }
  
  create_skymon_dialog(hg);
}



gchar* get_band_name(typHOE *hg, gint i){
  gchar *str;

  if(fabs(hg->obj[i].mag)>99){
    str=NULL;
  }
  else{
    switch(hg->obj[i].magdb_used){
    case MAGDB_TYPE_SIMBAD:
      if(hg->obj[i].magdb_band==0){
	str=NULL;
      }
      else{
	str=g_strdup_printf("%s",simbad_band[hg->obj[i].magdb_band]);
      }
      break;
      
    case MAGDB_TYPE_GSC:
      str=g_strdup_printf("%s",gsc_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_PS1:
      str=g_strdup_printf("PanSTARRS %s",ps1_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_SDSS:
      str=g_strdup_printf("SDSS %s",sdss_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_KEPLER:
      str=g_strdup("Kepler K");
      break;
      
    case MAGDB_TYPE_GAIA:
      str=g_strdup("GAIA G");
      break;
      
    case MAGDB_TYPE_2MASS:
      str=g_strdup_printf("2MASS %s",twomass_band[hg->obj[i].magdb_band]);
      break;
      
    default:
      str=NULL;
    }
  }

  return(str);
}

void do_etc (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *frame, *label, *button;
  GtkWidget *hbox, *combo, *entry, *table, *check;
  GtkWidget *fdialog, *spinner;
  GtkWidget *rb[ETC_SPEC_NUM], *rc[ETC_WAVE_NUM];
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg;
  gchar tmp[1024];
  gint i_list;
  gchar *str=NULL;
  gboolean skip_flag=FALSE;

  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;

  dialog = gtk_dialog_new_with_buttons("HOE : Exposure Time Calculator",
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

  frame = gtk_frame_new ("Input flux spectrum");
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
  
  if(hg->etc_mode==ETC_OBJTREE){
    str=get_band_name(hg, hg->etc_i);
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

    if(hg->etc_mode==ETC_LIST){
      label = gtk_label_new ("-band for user defined mag.   ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    }
    else{
      label = gtk_label_new ("magnitude: ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    }
  }
  
  if(hg->etc_mode!=ETC_LIST){
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_mag,
					      0.0, 22.0, 0.20, 0.20, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->etc_mag);
    spinner =  gtk_spin_button_new (adj, 1, 2);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
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
    label = gtk_label_new ("Redshift: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    spinner =  gtk_spin_button_new (hg->etc_z_adj, 1, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
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
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);


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
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
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


  frame = gtk_frame_new ("Instrument setting");
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

  {
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
	  sprintf(tmp,"Setup-%d : NonStd-%d  %dx%dbinning  %s",
		  i_use+1,
		  -hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y,
		  slit_tmp);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s  %dx%dbinning  %s",
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
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
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


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtkut_table_attach(table, hbox, 0, 1, 3, 4,
		     GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode!=ETC_LIST){
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
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
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
  }

  if(hg->etc_mode!=ETC_MENU){
    frame = gtk_frame_new ("Wavelength for S/N Display");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);

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
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    
    label = gtk_label_new ("A");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  if(hg->etc_mode==ETC_LIST){
    frame = gtk_frame_new ("Update S/N in the list");
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
    if(hg->etc_mode==ETC_LIST){
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
    }
    else{
      etc_main(hg);
    }
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void WritePass(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_col;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // Pass
  if((hg->prop_id)&&(hg->prop_pass))
    xmms_cfg_write_string(cfgfile, "Pass", hg->prop_id, hg->prop_pass);

  xmms_cfg_write_file(cfgfile, conffile);
  xmms_cfg_free(cfgfile);

  g_free(conffile);
}

void init_obj(OBJpara *obj, typHOE *hg){
  gint i_band, i_use;

  obj->check_sm=FALSE;
  obj->exp=hg->def_exp;
  obj->mag=100;
  obj->snr=-1;
  obj->sat=FALSE;
  obj->repeat=1;
  obj->guide=hg->def_guide;
  obj->aomode=hg->def_aomode;
  obj->adi=FALSE;
  obj->pa=hg->def_pa;
  obj->i_nst=-1;
  obj->std=FALSE;

  obj->gs.flag=FALSE;
  if(obj->gs.name) g_free(obj->gs.name);
  obj->gs.name=NULL;
  
  if(obj->trdb_str) g_free(obj->trdb_str);
  obj->trdb_str=NULL;
  obj->trdb_band_max=0;
  for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
    if(obj->trdb_mode[i_band]) g_free(obj->trdb_mode[i_band]);
    obj->trdb_mode[i_band]=NULL;
    if(obj->trdb_band[i_band]) g_free(obj->trdb_band[i_band]);
    obj->trdb_band[i_band]=NULL;
    obj->trdb_exp[i_band]=0;
    obj->trdb_shot[i_band]=0;
  }
  
  obj->setup[0]=TRUE;
  for(i_use=1;i_use<MAX_USESETUP;i_use++){
    obj->setup[i_use]=FALSE;
  }

  ObjMagDB_Init(obj);
}

void param_init(typHOE *hg){
  time_t t;
  struct tm *tmpt;
  int i, i_band;

  // Global Args
  flagChildDialog=FALSE;
  flagSkymon=FALSE;
  flagPlot=FALSE;
  flagFC=FALSE;
  flagPlan=FALSE;
  flag_getFCDB=FALSE;
  flag_make_obj_tree=FALSE;
  flag_make_line_tree=FALSE;
  flag_make_etc_tree=FALSE;




  // CCD Binning Mode
  const Binpara binnings[] = {
    {"1x1 [86s] ", 1, 1, 86},
    {"2x1 [60s] ", 2, 1, 60},
    {"2x2 [44s] ", 2, 2, 44},
    {"2x4 [36s] ", 2, 4, 36},
    {"4x1 [44s] ", 4, 1, 44},
    {"4x4 [33s] ", 4, 4, 33}
  };

  hg->i_max=0;
  hg->i_plan_max=0;

  hg->skymon_timer=-1;

  hg->inst=-1;
  
  t = time(NULL);
  tmpt = localtime(&t);

  hg->fr_year=tmpt->tm_year+1900;
  hg->fr_month=tmpt->tm_mon+1;
  hg->fr_day=tmpt->tm_mday;
  hg->skymon_year=hg->fr_year;
  hg->skymon_month=hg->fr_month;
  hg->skymon_day=hg->fr_day;

  hg->list_style=LIST_DEFAULT;
  
  hg->prop_id=g_strdup("o00000");
  hg->prop_pass=NULL;
  hg->observer=NULL;

#ifdef USE_WIN32
  hg->temp_dir=get_win_temp();
  hg->home_dir=get_win_home();
#else
  hg->temp_dir=g_strdup("/tmp");
  hg->home_dir=g_strdup(g_get_home_dir());
#endif

  ReadConf(hg);

  hg->fc_mode = hg->fc_mode0;

  hg->wave1=WAVE1_SUBARU;
  hg->wave0=WAVE0_SUBARU;
  hg->temp=TEMP_SUBARU;
  hg->pres=PRES_SUBARU;
  hg->dss_scale=FC_SCALE_LINEAR;

  {
    gint i_bin;

    for(i_bin=0;i_bin<MAX_BINNING;i_bin++){
      hg->binning[i_bin]=binnings[i_bin];
    }
  }

  hg->camz_b=CAMZ_B;
  hg->camz_r=CAMZ_R;
  hg->camz_date=g_strdup("(Not synced yet)");

  hg->d_cross=D_CROSS;
  hg->wcent=5500;

  {
    gint i_use;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      hg->setup[i_use].setup=0;
      hg->setup[i_use].use=FALSE;
      hg->setup[i_use].binning=0;
      hg->setup[i_use].slit_width=200;
      hg->setup[i_use].slit_length=2200;
      hg->setup[i_use].imr=IMR_NO;
      hg->setup[i_use].is=IS_NO;
      hg->setup[i_use].i2=FALSE;
    }
    hg->setup[0].use=TRUE;
  }

  hg->exptime_factor=1;
  hg->brightness=2000;
  hg->sv_area=SV_PART;
  hg->sv_integrate=1;
  hg->sv_region=200;
  hg->sv_calc=DEF_SV_CALC;
  hg->exptime_sv=DEF_SV_EXP;
  hg->sv_slitx=DEF_SV_SLITX;
  hg->sv_slity=DEF_SV_SLITY;
  hg->sv_isx=DEF_SV_ISX;
  hg->sv_isy=DEF_SV_ISY;
  hg->sv_is3x=DEF_SV_IS3X;
  hg->sv_is3y=DEF_SV_IS3Y;

  hg->oh_acq=TIME_ACQ;
  hg->oh_ngs1=IRCS_TIME_AO_NGS1;
  hg->oh_ngs2=IRCS_TIME_AO_NGS2;
  hg->oh_ngs3=IRCS_TIME_AO_NGS3;
  hg->oh_lgs=IRCS_TIME_AO_LGS;
  
  hg->def_exp=DEF_EXP;
  hg->def_guide=SV_GUIDE;
  hg->def_pa=0;
  hg->def_aomode=AOMODE_NO;

  for(i=0;i<MAX_NONSTD;i++){
    hg->nonstd[i].col=COL_RED;
    hg->nonstd[i].cross=15730;
    hg->nonstd[i].echelle=DEF_ECHELLE;
    hg->nonstd[i].camr=-3600;
  }

  for(i=0;i<MAX_LINE;i++){
    hg->line[i].name=NULL;
    hg->line[i].wave=0.;
  }

  for(i=0;i<MAX_OBJECT;i++){
    hg->obj[i].name=NULL;
    hg->obj[i].note=NULL;
    hg->obj[i].i_nst=-1;

    hg->obj[i].trdb_str=NULL;
    hg->obj[i].trdb_band_max=0;
    for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
      hg->obj[i].trdb_mode[i_band]=NULL;
      hg->obj[i].trdb_band[i_band]=NULL;
      hg->obj[i].trdb_exp[i_band]=0;
      hg->obj[i].trdb_shot[i_band]=0;
    }

    hg->obj[i].gs.flag=FALSE;
    hg->obj[i].gs.name=NULL;

    hg->obj[i].magdb_simbad_name=NULL;
    hg->obj[i].magdb_simbad_type=NULL;
    hg->obj[i].magdb_simbad_sp=NULL;

    hg->obj[i].magdb_ned_name=NULL;
    hg->obj[i].magdb_ned_type=NULL;
    hg->obj[i].magdb_ned_mag=NULL;

    hg->obj[i].magdb_lamost_name=NULL;
    hg->obj[i].magdb_lamost_type=NULL;
    hg->obj[i].magdb_lamost_sp=NULL;

    hg->obj[i].magdb_kepler_name=NULL;
    hg->obj[i].magdb_kepler_2mass=NULL;
  }

  hg->trdb_i_max=0;
  hg->trdb_disp_flag=TRUE;
  hg->trdb_smoka_inst=0;
  hg->trdb_smoka_date=g_strdup_printf("1998-01-01..%d-%02d-%02d",
				      hg->fr_year,
				      hg->fr_month,
				      hg->fr_day);
  hg->trdb_arcmin=2;
  hg->trdb_used=MAGDB_TYPE_SIMBAD;
  hg->trdb_da=TRDB_TYPE_SMOKA;
  make_trdb_label(hg);
  hg->trdb_smoka_shot  = TRUE;
  hg->trdb_smoka_shot_used  = TRUE;
  hg->trdb_smoka_imag  = TRUE;
  hg->trdb_smoka_imag_used  = TRUE;
  hg->trdb_smoka_spec  = TRUE;
  hg->trdb_smoka_spec_used  = TRUE;
  hg->trdb_smoka_ipol  = TRUE;
  hg->trdb_smoka_ipol_used  = TRUE;
  hg->trdb_hst_mode  = TRDB_HST_MODE_IMAGE;
  hg->trdb_hst_date=g_strdup_printf("1990-01-01..%d-%02d-%02d",
				    hg->fr_year,
				    hg->fr_month,
				    hg->fr_day);
  hg->trdb_eso_mode  = TRDB_ESO_MODE_IMAGE;
  hg->trdb_eso_stdate=g_strdup("1980 01 01");
  hg->trdb_eso_eddate=g_strdup_printf("%4d %02d %02d",
				      hg->fr_year,
				      hg->fr_month,
				      hg->fr_day);

  hg->trdb_gemini_inst  = GEMINI_INST_GMOS;
  hg->trdb_gemini_inst_used  = GEMINI_INST_GMOS;
  hg->trdb_gemini_mode  = TRDB_GEMINI_MODE_ANY;
  hg->trdb_gemini_date=g_strdup_printf("19980101-%d%02d%02d",
				       hg->fr_year,
				       hg->fr_month,
				       hg->fr_day);

  hg->efs_ps=g_strdup(PS_FILE);

  hg->etc_mode=ETC_MENU;
  hg->etc_filter=BAND_V;
  hg->etc_mag=17.0;
  hg->etc_z=0.0;
  hg->etc_spek=ETC_SPEC_POWERLAW;
  hg->etc_alpha=0.0;
  hg->etc_bbtemp=10000;
  hg->etc_sptype=ST_O5V;
  hg->etc_adc=ETC_ADC_IN;
  hg->etc_imr=ETC_IMR_NO;
  hg->etc_exptime=3600;
  hg->etc_seeing=0.5;
  hg->etc_label_text=g_strdup("Exposure Time Calculator");
  hg->etc_prof_text=g_strdup("Your results on ETC");
  hg->etc_wave=ETC_WAVE_CENTER;
  hg->etc_waved=5500;

  hg->filename_hoe=NULL;
  hg->filename_log=NULL;
  hg->filename_prm1=NULL;
  hg->filename_prm2=NULL;

  hg->azel_mode=AZEL_NORMAL;

  //hg->flag_bunnei=FALSE;
  hg->expmag_mag=8.0;
  hg->expmag_exp=100;

  hg->skymon_mode=SKYMON_SET;
  hg->skymon_objsz=SKYMON_DEF_OBJSZ;

  hg->dss_arcmin          =DSS_ARCMIN;
  hg->dss_pix             =DSS_PIX;

  hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
  hg->dss_path             =g_strdup(FC_PATH_SKYVIEW);
  hg->dss_src              =g_strdup(FC_SRC_SKYVIEW_DSS2R);
  hg->dss_tmp=g_strconcat(hg->temp_dir,
			  G_DIR_SEPARATOR_S,
			  FC_FILE_HTML,NULL);
  hg->dss_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FC_FILE_JPEG,NULL);
  set_fc_mode(hg);

  hg->dss_pa=0;
  hg->dss_flip=FALSE;
  hg->dss_draw_slit=TRUE;
  hg->sdss_photo=FALSE;
  hg->sdss_spec=FALSE;
  hg->fc_inst=FC_INST_HDS;

  for(i=0;i<MAX_PLAN;i++){
    hg->plan[i].txt=NULL;
    hg->plan[i].comment=NULL;
    hg->plan[i].txt_az=NULL;
    hg->plan[i].txt_el=NULL;
  }

  hg->plan_tmp_or=FALSE;
  hg->plan_tmp_sw=200;
  hg->plan_tmp_sl=2000;
  hg->plan_tmp_setup=0;

  //hg->plan_obj_i=0;
  hg->plan_obj_exp=DEF_EXP;
  hg->plan_obj_repeat=1;
  hg->plan_obj_omode=PLAN_OMODE_FULL;
  hg->plan_obj_guide=SV_GUIDE;

  hg->plan_bias_repeat=5;

  hg->plan_flat_repeat=HDS_FLAT_REPEAT;

  hg->plan_focus_mode=PLAN_FOCUS1;

  hg->plan_setup_cmode=PLAN_CMODE_FULL;

  hg->plan_i2_pos=PLAN_I2_OUT;

  hg->plan_setaz=-90.;
  hg->plan_setel=90.;

  hg->plan_start=PLAN_START_EVENING;
  hg->plan_start_hour=24;
  hg->plan_start_min=20;

  hg->plan_delay=SUNSET_OFFSET;

  hg->plan_comment=NULL;

  hg->plot_all=PLOT_ALL_SINGLE;
  hg->plot_moon=FALSE;

  hg->plot_output=PLOT_OUTPUT_WINDOW;
  hg->skymon_output=SKYMON_OUTPUT_WINDOW;
  hg->efs_output=EFS_OUTPUT_WINDOW;

  hg->hsc_dithi=1;
  hg->hsc_dithp=HSC_DITH_NO;
  hg->hsc_dra=HSC_DRA;
  hg->hsc_ddec=HSC_DDEC;
  hg->hsc_tdith=HSC_TDITH;
  hg->hsc_rdith=HSC_RDITH;
  hg->hsc_ndith=5;
  hg->hsc_offra=0;
  hg->hsc_offdec=0;

  hg->std_i_max=0;
  hg->std_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   STDDB_FILE_XML,NULL);
  hg->stddb_mode=STDDB_IRAFSTD;

  hg->std_dra   =STD_DRA;
  hg->std_ddec  =STD_DDEC;
  hg->std_vsini =STD_VSINI;
  hg->std_vmag  =STD_VMAG;
  hg->std_sptype=g_strdup(STD_SPTYPE);
  hg->std_iras12=STD_IRAS12;
  hg->std_iras25=STD_IRAS25;
  hg->std_cat   =g_strdup(STD_CAT);
  hg->std_mag1  =STD_MAG1;
  hg->std_mag2  =STD_MAG2;
  hg->std_band  =g_strdup(STD_BAND);
  hg->std_sptype2  =g_strdup(STD_SPTYPE_ALL);

  hg->fcdb_i_max=0;
  hg->fcdb_simbad=FCDB_SIMBAD_STRASBG;
  hg->fcdb_vizier=FCDB_VIZIER_NAOJ;
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);
  hg->fcdb_label_text=g_strdup("Object in Finding Chart");
  hg->fcdb_band=FCDB_BAND_NOP;
  hg->fcdb_mag=15;
  hg->fcdb_otype=FCDB_OTYPE_ALL;
  hg->fcdb_ned_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ned_otype=FCDB_NED_OTYPE_ALL;
  hg->fcdb_auto=FALSE;
  hg->fcdb_ned_ref=FALSE;
  hg->fcdb_gsc_fil=TRUE;
  hg->fcdb_gsc_mag=19;
  hg->fcdb_gsc_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ps1_fil=TRUE;
  hg->fcdb_ps1_mag=19;
  hg->fcdb_ps1_diam=FCDB_PS1_ARCMIN_MAX;
  hg->fcdb_ps1_mindet=2;
  hg->fcdb_sdss_search = FCDB_SDSS_SEARCH_IMAG;
  for(i=0;i<NUM_SDSS_BAND;i++){
    hg->fcdb_sdss_fil[i]=TRUE;
    hg->fcdb_sdss_magmin[i]=0;
    hg->fcdb_sdss_magmax[i]=20;
  }
  hg->fcdb_sdss_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_usno_fil=TRUE;
  hg->fcdb_usno_mag=19;
  hg->fcdb_usno_diam=FCDB_USNO_ARCMIN_MAX;
  hg->fcdb_gaia_fil=TRUE;
  hg->fcdb_gaia_mag=19;
  hg->fcdb_gaia_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_kepler_fil=TRUE;
  hg->fcdb_kepler_mag=19;
  hg->fcdb_2mass_fil=TRUE;
  hg->fcdb_2mass_mag=12;
  hg->fcdb_2mass_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_wise_fil=TRUE;
  hg->fcdb_wise_mag=15;
  hg->fcdb_wise_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_smoka_shot  = FALSE;
  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    hg->fcdb_smoka_subaru[i]  = TRUE;
  }
  for(i=0;i<NUM_SMOKA_KISO;i++){
    hg->fcdb_smoka_kiso[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_OAO;i++){
    hg->fcdb_smoka_oao[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_MTM;i++){
    hg->fcdb_smoka_mtm[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_KANATA;i++){
    hg->fcdb_smoka_kanata[i]  = FALSE;
  }
  for(i=0;i<NUM_HST_IMAGE;i++){
    hg->fcdb_hst_image[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_SPEC;i++){
    hg->fcdb_hst_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_OTHER;i++){
    hg->fcdb_hst_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_IMAGE;i++){
    hg->fcdb_eso_image[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SPEC;i++){
    hg->fcdb_eso_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_VLTI;i++){
    hg->fcdb_eso_vlti[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_POLA;i++){
    hg->fcdb_eso_pola[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_CORO;i++){
    hg->fcdb_eso_coro[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_OTHER;i++){
    hg->fcdb_eso_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SAM;i++){
    hg->fcdb_eso_sam[i]  = TRUE;
  }
  hg->fcdb_gemini_inst = GEMINI_INST_ANY;

  hg->sz_skymon=SKYMON_WINSIZE;
  hg->sz_plot  =  PLOT_WINSIZE;
  hg->sz_fc    =    FC_WINSIZE;

  hg->obs_timezone = TIMEZONE_SUBARU;
  //skymon_set_time_current(hg);
  hg->skymon_hour=23;
  hg->skymon_min=55;

  hg->orbit_flag=TRUE;
  hg->fcdb_flag=TRUE;

  hg->magdb_arcsec=10;
  hg->magdb_mag=18;
  hg->magdb_ow=FALSE;
  hg->magdb_pm=TRUE;
  hg->magdb_skip=FALSE;
  hg->magdb_gsc_band=GSC_BAND_V;
  hg->magdb_ps1_band=PS1_BAND_G;
  hg->magdb_sdss_band=SDSS_BAND_G;
  hg->magdb_2mass_band=TWOMASS_BAND_J;
  hg->magdb_simbad_band=FCDB_BAND_NOP;

  IRCS_param_init(hg);

  calc_moon(hg);
  calc_sun_plan(hg);

}



gchar *cut_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name, *c;
  gint  i_bak,i;

  tgt_name=g_strdup(obj_name);
  i_bak=strlen(tgt_name)-1;
  while((tgt_name[i_bak]==0x20)
	||(tgt_name[i_bak]==0x0A)
	||(tgt_name[i_bak]==0x0D)
	||(tgt_name[i_bak]==0x09)){
    tgt_name[i_bak]='\0';
    i_bak--;
  }
    
  c=tgt_name;
  i=0;
  while((tgt_name[i]==0x20)||(tgt_name[i]==0x09)){
    c++;
    i++;
  }

  ret_name=g_strdup(c);
  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}


gchar *make_tgt(gchar *obj_name, const gchar *head){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj,i_tgt;

  strcpy(tgt_name, head);
  i_tgt=strlen(tgt_name);

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(isalnum(obj_name[i_obj])){
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  return(ret_name);
}

gdouble read_radec(gchar* p){
  gchar* tmp_p;
  gdouble d_out;

  if((tmp_p=strtok(p,":"))!=NULL){
    d_out=1.0e4*(gdouble)g_strtod(tmp_p,NULL);
    if((tmp_p=strtok(NULL,":"))!=NULL){
      d_out+=1.0e2*(gdouble)g_strtod(tmp_p,NULL);
      if((tmp_p=strtok(NULL,":"))!=NULL){
	d_out+=(gdouble)g_strtod(tmp_p,NULL);
      }
    }

    return(d_out);
  }
  else{
    return((gdouble)g_strtod(p,NULL));
  }
}


void ObjMagDB_Init(OBJpara* obj){
  obj->magdb_used=0;
  obj->magdb_band=0;
  
  obj->magdb_gsc_hits=-1;
  obj->magdb_ps1_hits=-1;
  obj->magdb_sdss_hits=-1;
  obj->magdb_gaia_hits=-1;
  obj->magdb_2mass_hits=-1;
  obj->magdb_simbad_hits=-1;
  obj->magdb_ned_hits=-1;
  obj->magdb_lamost_hits=-1;
  obj->magdb_kepler_hits=-1;

  obj->magdb_gsc_sep=-1;
  obj->magdb_ps1_sep=-1;
  obj->magdb_sdss_sep=-1;
  obj->magdb_gaia_sep=-1;
  obj->magdb_2mass_sep=-1;
  obj->magdb_simbad_sep=-1;
  obj->magdb_ned_sep=-1;
  obj->magdb_lamost_sep=-1;
  obj->magdb_kepler_sep=-1;

  obj->magdb_gsc_u=100;
  obj->magdb_gsc_b=100;
  obj->magdb_gsc_v=100;
  obj->magdb_gsc_r=100;
  obj->magdb_gsc_i=100;
  obj->magdb_gsc_j=100;
  obj->magdb_gsc_h=100;
  obj->magdb_gsc_k=100;

  obj->magdb_ps1_g=100;
  obj->magdb_ps1_r=100;
  obj->magdb_ps1_i=100;
  obj->magdb_ps1_z=100;
  obj->magdb_ps1_y=100;

  obj->magdb_sdss_u=100;
  obj->magdb_sdss_g=100;
  obj->magdb_sdss_r=100;
  obj->magdb_sdss_i=100;
  obj->magdb_sdss_z=100;

  obj->magdb_gaia_g=100;
  obj->magdb_gaia_p=-1;
  obj->magdb_gaia_ep=-1;
  obj->magdb_gaia_bp=100;
  obj->magdb_gaia_rp=100;
  obj->magdb_gaia_rv=-99999;
  obj->magdb_gaia_teff=-1;
  obj->magdb_gaia_ag=100;
  obj->magdb_gaia_ebr=-1;
  obj->magdb_gaia_dist=-1;

  obj->magdb_2mass_j=100;
  obj->magdb_2mass_h=100;
  obj->magdb_2mass_k=100;

  obj->magdb_simbad_u=100;
  obj->magdb_simbad_b=100;
  obj->magdb_simbad_v=100;
  obj->magdb_simbad_r=100;
  obj->magdb_simbad_i=100;
  obj->magdb_simbad_j=100;
  obj->magdb_simbad_h=100;
  obj->magdb_simbad_k=100;
  obj->magdb_simbad_name=NULL;
  obj->magdb_simbad_type=NULL;
  obj->magdb_simbad_sp=NULL;

  obj->magdb_ned_name=NULL;
  obj->magdb_ned_type=NULL;
  obj->magdb_ned_mag=NULL;
  obj->magdb_ned_z=-100;
  obj->magdb_ned_ref=0;

  obj->magdb_lamost_name=NULL;
  obj->magdb_lamost_type=NULL;
  obj->magdb_lamost_sp=NULL;
  obj->magdb_lamost_ref=0;
  obj->magdb_lamost_teff=-1;
  obj->magdb_lamost_logg=-10;
  obj->magdb_lamost_feh=+100;
  obj->magdb_lamost_hrv=-99999;

  obj->magdb_kepler_name=NULL;
  obj->magdb_kepler_k=100;
  obj->magdb_kepler_r=100;
  obj->magdb_kepler_j=100;
  obj->magdb_kepler_teff=-1;
  obj->magdb_kepler_logg=-10;
  obj->magdb_kepler_feh=+100;
  obj->magdb_kepler_ebv=+100;
  obj->magdb_kepler_rad=+100;
  obj->magdb_kepler_pm=-10000;
  obj->magdb_kepler_gr=100;
  obj->magdb_kepler_2mass=NULL;
}


gint get_same_rb(gint i){
  gboolean same_rb;

  if((!strcmp(HDS_setups[i].f1_amp,HDS_setups[i].f2_amp))
     &&(HDS_setups[i].f1_fil1==HDS_setups[i].f2_fil1)
     &&(HDS_setups[i].f1_fil2==HDS_setups[i].f2_fil2)
     &&(HDS_setups[i].f1_fil3==HDS_setups[i].f2_fil3)
     &&(HDS_setups[i].f1_fil4==HDS_setups[i].f2_fil4)
     &&(HDS_setups[i].f1_exp==HDS_setups[i].f2_exp)){
    same_rb=TRUE;
  }
  else{
    same_rb=FALSE;
  }

  return(same_rb);
}


gint get_nonstd_flat(gint col, gint cross){
  gint j_set, nonstd_flat;

  if(col==COL_BLUE){
    for(j_set=StdUb;j_set<StdI2b;j_set++){
      if(cross<=HDS_setups[j_set].cross_scan){
	break;
      }
    }

    switch(j_set){
    case StdUb:
      nonstd_flat=StdUb;
    case StdI2b:
      nonstd_flat=StdYa;
    default:
      if((HDS_setups[j_set].cross_scan - cross)
	   < (cross - HDS_setups[j_set-1].cross_scan)){
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
  else{  // RED
    for(j_set=StdI2b;j_set<StdHa;j_set++){
      if(cross<=HDS_setups[j_set].cross_scan){
	break;
      }
    }
    switch(j_set){
    case StdI2b:
      nonstd_flat=StdI2b;
    case StdHa:
      nonstd_flat=StdNIRa;
    default:
      if((HDS_setups[j_set].cross_scan - cross)
	 < (cross - HDS_setups[j_set-1].cross_scan)){
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

  return(nonstd_flat);
}


void usage(void)
{
  g_print(" hoe : Subaru HDS++ OPE file Editor   Ver"VERSION"\n");
  g_print("  [usage] %% hoe [-i input file] [-c config_file] [-h] [-d]\n");
  g_print("     -i, --input input-file   : Set the inpout object list file\n");
  g_print("     -c, --config config-file : Load Config (.hoe) File\n");
  g_print("     -h, --help               : Print this message\n");
  g_print("     -d, --debug              : Print out debugging messages for developpers\n");

  exit(0);
}


void get_option(int argc, char **argv, typHOE *hg)
{
  int i_opt;
  int valid=1;
  gchar *cwdname=NULL;

  hg->filename_read=NULL;
  debug_flg = 0;      /* -d オプションを付けると turn on する */

  
  i_opt = 1;
  while((i_opt < argc)&&(valid==1)) {
    if((strcmp(argv[i_opt],"-i") == 0)||
	    (strcmp(argv[i_opt],"--input") == 0)){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Warning: Could not get the current working directory.");
	  }
	  hg->filename_read=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hg->filename_read=g_strdup(argv[i_opt]);
	}
	hg->filehead=make_head(hg->filename_read);
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if((strcmp(argv[i_opt],"-c") == 0)||
	    (strcmp(argv[i_opt],"--config") == 0)){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Warning: Could not get the current working directory.");
	  }
	  hg->filename_hoe=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hg->filename_hoe=g_strdup(argv[i_opt]);
	}
	hg->filehead=make_head(hg->filename_hoe);
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if ((strcmp(argv[i_opt], "-h") == 0) ||
	     (strcmp(argv[i_opt], "--help") == 0)) {
      i_opt++;
      usage();
    }
    else if ((strcmp(argv[i_opt], "-d") == 0) ||
	     (strcmp(argv[i_opt], "--debug") == 0)) {
      debug_flg=1;
      i_opt++;
    }
    else{
      fprintf(stderr, "Warning: detected invalid command line option.\n");
      usage();
    }

  }
  
}


gchar *strip_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_str=0,i;

  tgt_name=g_strdup(obj_name);
  for(i=0;i<strlen(tgt_name);i++){
    if((obj_name[i]!=0x20)
       &&(obj_name[i]!=0x0A)
       &&(obj_name[i]!=0x0D)
       &&(obj_name[i]!=0x09)){
      tgt_name[i_str]=obj_name[i];
      i_str++;
    }
  }
  tgt_name[i_str]='\0';
  
  ret_name=g_strdup(tgt_name);
  if(tgt_name) g_free(tgt_name);
  return(ret_name);
}

gchar* to_utf8(gchar *input){
  return(g_locale_to_utf8(input,-1,NULL,NULL,NULL));
}

gchar* to_locale(gchar *input){
#ifdef USE_WIN32
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
  return(g_win32_locale_filename_from_utf8(input));
#else
  return(g_locale_from_utf8(input,-1,NULL,NULL,NULL));
#endif
}

gboolean is_number(GtkWidget *parent, gchar *s, gint line, const gchar* sect){
  gchar* msg;

  if(!s){
    msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"", line, sect);
    popup_message(parent, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Input File is invalid.",
		  " ",
		  msg,
		  NULL);
 
    g_free(msg);
    return FALSE;
  }

  while(*s!='\0'){
    if(!is_num_char(*s)){
      msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"\n Irregal character code : \"%02x\"", 
			  line, sect,*s);
      popup_message(parent, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "Error: Input File is invalid.",
		    " ",
		    msg,
		    NULL);
      
      g_free(msg);
      return FALSE;
    }
    s++;
  }
  return TRUE;
}


void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  va_start(args, delay);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
  }

  my_signal_connect(dialog,"delete-event",destroy_popup, &timer);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtk_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);
  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_main();
}

void delete_disp_para(GtkWidget *w, GdkEvent *event, GtkWidget *dialog)
{
  close_disp_para(w,dialog);
}

void close_disp_para(GtkWidget *w, GtkWidget *dialog)
{
  gtk_main_quit();
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

void default_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=-1;
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}


void change_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=1;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}


gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  return(FALSE);
}

void destroy_popup(GtkWidget *w, GdkEvent *event, gint *data)
{
  g_source_remove(*data);
  gtk_main_quit();
}


void my_signal_connect(GtkWidget *widget, 
		       const gchar *detailed_signal,
		       void *func,
		       gpointer data)
{
  g_signal_connect(G_OBJECT(widget),
		   detailed_signal,
		   G_CALLBACK(func),
		   data);
}


void my_entry_set_width_chars(GtkEntry *entry, guint n){
  gtk_entry_set_width_chars(entry, n);
}


gchar* check_ext(GtkWidget *w, gchar* filename, gchar* ext){
  gint slen, elen;
  gchar *p;
  gboolean addflag=FALSE;
  gchar *tmp;

  slen=strlen(filename);
  elen=strlen(ext);
  
  if(elen>=slen){
    addflag=TRUE;
  }
  else if(filename[slen-elen-1]!='.'){
    addflag=TRUE;
  }
  else{
    p=strrchr(filename,'.');
    p++;
    if(strcmp(p,ext)!=0){
      addflag=TRUE;
    }
  }

  if(addflag){
    tmp=g_strdup(filename);
    g_free(filename);
    filename=g_strconcat(tmp,".",ext,NULL);
    g_free(tmp);

    popup_message(w, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*1,
		  "Saving to",
		  " ",
		  filename,
		  NULL);
  }

  return(filename);
}

gchar* make_head(gchar* filename){
  gchar *fname, *p=NULL;

  p=strrchr(filename,'.');
  if(p){
    fname=g_strndup(filename,strlen(filename)-strlen(p));
    return(fname);
  }
  else{
    return(filename);
  }
}

GtkWidget* gtkut_hbox_new(gboolean homogeneous, gint spacing){
  GtkWidget *w;
#ifdef USE_GTK3
  w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
#else
  w = gtk_hbox_new(homogeneous, spacing);
#endif
  return(w);
}

GtkWidget* gtkut_vbox_new(gboolean homogeneous, gint spacing){
  GtkWidget *w;
#ifdef USE_GTK3
  w = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
#else
  w = gtk_vbox_new(homogeneous, spacing);
#endif
  return(w);
}

GtkWidget* gtkut_table_new(gint col, gint row, gboolean homogeneous, gint col_spacing, gint row_spacing, gint border_width){
  GtkWidget *w;
#ifdef USE_GTK3
  w = gtk_grid_new();
  gtk_grid_set_column_spacing (GTK_GRID (w), col_spacing);
  gtk_grid_set_row_spacing (GTK_GRID (w), row_spacing);
#else
  w = gtk_table_new(col, row, homogeneous);
  gtk_table_set_col_spacings (GTK_TABLE (w), col_spacing);
  gtk_table_set_row_spacings (GTK_TABLE (w), row_spacing);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (w), border_width);
  return(w);
}

void gtkut_table_attach(GtkWidget *table,
			GtkWidget *child,
			guint left_attach,
			guint right_attach,
			guint top_attach,
			guint bottom_attach,
			GtkAttachOptions xoptions,
			GtkAttachOptions yoptions,
			guint xpadding,
			guint ypadding){
#ifdef USE_GTK3      
  gtk_grid_attach(GTK_GRID(table), child,
		  left_attach, top_attach, right_attach-left_attach, bottom_attach-top_attach);
#else
  gtk_table_attach(GTK_TABLE(table), child,
		   left_attach, right_attach, top_attach, bottom_attach,
		   xoptions, yoptions, xpadding, ypadding);
#endif
}

void gtkut_table_attach_defaults(GtkWidget *table,
				 GtkWidget *child,
				 guint left_attach,
				 guint right_attach,
				 guint top_attach,
				 guint bottom_attach){
#ifdef USE_GTK3      
  gtk_widget_set_hexpand(child,TRUE);
  gtk_widget_set_vexpand(child,TRUE);
  gtk_grid_attach(GTK_GRID(table), child,
		  left_attach, top_attach, right_attach-left_attach, bottom_attach-top_attach);
#else
  gtk_table_attach_defaults(GTK_TABLE(table), child,
			    left_attach, right_attach, top_attach, bottom_attach);
#endif
}

#ifdef USE_GTK3
GtkWidget* gtkut_image_menu_item_new_with_label(GtkWidget *icon,
						const gchar *txt){
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  GtkWidget *label = gtk_label_new (txt);
  GtkWidget *menu_item = gtk_menu_item_new ();

  gtk_container_add (GTK_CONTAINER (box), icon);
  gtk_container_add (GTK_CONTAINER (box), label);

  gtk_container_add (GTK_CONTAINER (menu_item), box);
  
  gtk_widget_show_all (menu_item);
  return(menu_item);
}
#endif


#ifdef USE_GTK3
GtkWidget* gtkut_button_new_from_icon_name(gchar *txt,
					   const gchar *stock){
#else
GtkWidget* gtkut_button_new_from_stock(gchar *txt,
				       const gchar *stock){
#endif
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_BUTTON);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_MENU);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);
  
  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}
 
#ifdef USE_GTK3
GtkWidget* gtkut_toggle_button_new_from_icon_name(gchar *txt,
						   const gchar *stock){
#else
GtkWidget* gtkut_toggle_button_new_from_stock(gchar *txt,
					      const gchar *stock){
#endif
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_BUTTON);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_MENU);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
  gtk_widget_show(button);
  return(button);
}

GtkWidget* gtkut_toggle_button_new_from_pixbuf(gchar *txt,
					      GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  gint w,h;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
   gtk_widget_show(button);
  return(button);
}


#ifdef USE_WIN32
gchar* WindowsVersion()
{
  // Get OS Info for WinXP and 2000 or later
  // for Win9x, OSVERSIONINFO should be used instead of OSVERSIONINFOEX
  OSVERSIONINFOEX osInfo;
  gchar *windowsName;
  static gchar buf[1024];

  windowsName = NULL;
  
  osInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);

  GetVersionEx ((LPOSVERSIONINFO)&osInfo);

  switch (osInfo.dwMajorVersion)
  {
  case 4:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT){
	  windowsName = g_strdup("Windows NT 4.0");
	}
	else{
	  windowsName = g_strdup("Windows 95");
	}
	break;

      case 10:
	windowsName = g_strdup("Windows 98");
	break;

      case 90:
	windowsName = g_strdup("Windows Me");
	break;
      }
    break;

  case 5:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	windowsName = g_strdup("Windows 2000");
	break;
	    
      case 1:
	windowsName = g_strdup("Windows XP");
	break;
	
      case 2:
	windowsName = g_strdup("Windows Server 2003");
	break;

      }
    break;

  case 6:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows Vista");
	else
	  windowsName = g_strdup("Windows Server 2008");
	break;
	
      case 1:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 7");
	else
	  windowsName = g_strdup("Windows Server 2008 R2");
	break;

      case 2:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8");
	else
	  windowsName = g_strdup("Windows Server 2012");
	break;

      case 3:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8.1");
	else
	  windowsName = g_strdup("Windows Server 2012 R2");
	break;
      }
    break;

  case 10:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 10");
	else
	  windowsName = g_strdup("Windows Server 2016");
	break;
      }	
    break;
  }

  if(!windowsName) windowsName = g_strdup("Windows UNKNOWN");
  
  //OutPut
  if(osInfo.wServicePackMajor!=0){
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s w/SP%d (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.wServicePackMajor,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  else{
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  g_free(windowsName);

  return(buf);
}
#endif


void calc_rst(typHOE *hg){
  gint i_list;
  gdouble JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn oequ;
  struct ln_zonedate zonedate;
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  double a0,d0;
  struct ln_zonedate rst_date;
  struct ln_rst_time rst;

  zonedate.years=hg->fr_year;
  zonedate.months=hg->fr_month;
  zonedate.days=hg->fr_day+1;
  zonedate.hours=0;
  zonedate.minutes=0;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)(hg->obs_timezone*60);
    
  JD = ln_get_julian_local_date(&zonedate);

  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;



  for (i_list=0; i_list<hg->i_max; i_list++){
    a0s=hg->obj[i_list].ra;
    ia0h=(int)(a0s/10000);
    a0s=a0s-(double)(ia0h)*10000;
    ia0m=(int)(a0s/100);
    a0s=a0s-(double)(ia0m)*100;
    
    d0s=hg->obj[i_list].dec;
    id0d=(int)(d0s/10000);
    d0s=d0s-(double)(id0d)*10000;
    id0m=(int)(d0s/100);
    d0s=d0s-(double)(id0m)*100;
	
    a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
    d0=id0d + id0m/60. + d0s/3600.;  //[deg]
	
    oequ.ra=a0*360/24;
    oequ.dec=d0;

    hg->obj[i_list].horizon
      =ln_get_object_rst_horizon (JD, &observer, &oequ, 15.0, &rst);

    hg->obj[i_list].rise=rst.rise;
    hg->obj[i_list].transit=rst.transit;
    hg->obj[i_list].set=rst.set;

  }
}

void ver_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gint major=0, minor=0, micro=0;
  gboolean update_flag=FALSE;
  gint c_major, c_minor, c_micro;
  gchar *tmp;
  

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }

  c_major=g_strtod(MAJOR_VERSION,NULL);
  c_minor=g_strtod(MINOR_VERSION,NULL);
  c_micro=g_strtod(MICRO_VERSION,NULL);
  
  while((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,",");
    
    if(strncmp(tmp_char,"MAJOR",strlen("MAJOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	major=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MINOR",strlen("MINOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	minor=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MICRO",strlen("MICRO"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	micro=g_strtod(tmp_p,NULL);
      }
    }
  }
  fclose(fp);

  unlink(hg->fcdb_file);

  if(major>c_major){
    update_flag=TRUE;
  }
  else if(major==c_major){
    if(minor>c_minor){
      update_flag=TRUE;
    }
    else if(minor==c_minor){
      if(micro>c_micro){
	update_flag=TRUE;
      }
    }
  }

  if(update_flag){
    GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

    flagChildDialog=TRUE;
  
    dialog = gtk_dialog_new_with_buttons("HOE : Download the latest version?",
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

    my_signal_connect(dialog, "destroy", NULL,NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							     GTK_RESPONSE_OK));


    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       hbox,FALSE, FALSE, 0);
    
#ifdef USE_GTK3
    pixmap=gtk_image_new_from_icon_name ("dialog-question",
					 GTK_ICON_SIZE_DIALOG);
#else
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				     GTK_ICON_SIZE_DIALOG);
#endif
    
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);
    
    vbox = gtkut_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

    tmp=g_strdup_printf("The current version : ver. %d.%d.%d",
			c_major,c_minor,c_micro);
    label = gtk_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);

    tmp=g_strdup_printf("The latest version  : ver. %d.%d.%d",
			major,minor,micro);
    label = gtk_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);
    

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("Do you go to the web page to download the latest version?");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      uri_clicked(NULL, (gpointer)hg);
    }
    gtk_widget_destroy(dialog);
    
    flagChildDialog=FALSE;
  }
  else{
    tmp=g_strdup_printf("HOE ver. %d.%d.%d is the latest version.",
			major,minor,micro);
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_OK,
#endif
		  POPUP_TIMEOUT*1,
		  tmp,
		  NULL);
    if(tmp) g_free(tmp);
  }
}

void CheckVer(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  ver_dl(hg);
  ver_txt_parse(hg);
}

void init_inst(typHOE *hg){
  switch(hg->inst){
  case INST_HDS:    
    hg->def_pa=HDS_DEF_PA;
    hg->fc_inst=FC_INST_HDS;
    hg->fcdb_type=FCDB_TYPE_SIMBAD;
    hg->dss_arcmin=HDS_SIZE;
    hg->oh_acq=TIME_ACQ;
    break;
  case INST_IRCS:
    hg->def_pa=IRCS_DEF_PA;
    hg->fc_inst=FC_INST_IRCS;
    hg->fcdb_type=FCDB_TYPE_GSC;
    hg->dss_arcmin=IRCS_SIZE;
    hg->oh_acq=IRCS_TIME_ACQ;
    break;
  }

  if(flagFC){
    gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
			     (gdouble)hg->dss_pa);
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)hg->dss_arcmin);
  }
}

 
gboolean CheckInst(typHOE *hg, gint target_inst){
  gchar *tmp;
  
  if(hg->inst==target_inst){
    return(TRUE);
  }
  else{
    tmp=g_strdup_printf("   %s : %s .\n",
			inst_name_short[target_inst],
			inst_name_long[target_inst]);
    
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*1,
		  "This function is available only for ",
		  " ",
		  tmp,
		  NULL);
    g_free(tmp);

    return(FALSE);
  }
}
 
void RecalcRST(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  recalc_rst(hg);
}

void update_objtree(typHOE *hg){
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    objtree_update_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}

void recalc_rst(typHOE *hg){
  calc_moon(hg);
  calc_sun_plan(hg);
  calc_rst(hg);

  update_objtree(hg);
}


GtkWidget* gtkut_button_new_from_pixbuf(gchar *txt,
				       GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  gint w,h;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);
  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}



int main(int argc, char* argv[]){
  typHOE *hg;
#ifdef USE_WIN32
  WSADATA wsaData;
  int nErrorStatus;
#else
  GdkPixbuf *icon;
#endif

  hg=g_malloc0(sizeof(typHOE));

  hg->init_flag=FALSE;

  setlocale(LC_ALL,"");

#ifndef USE_GTK3
  gtk_set_locale();
#endif

  gtk_init(&argc, &argv);

  param_init(hg);

  get_option(argc, argv, hg);

  // Gdk-Pixbufで使用
#if !GTK_CHECK_VERSION(2,21,8)
  gdk_rgb_init();
#endif

#ifndef USE_WIN32  
  icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  gtk_window_set_default_icon(icon);
#endif

#ifdef USE_WIN32   // Initialize Winsock2
    nErrorStatus = WSAStartup(MAKEWORD(2,0), &wsaData);
    if(atexit((void (*)(void))(WSACleanup))){
      fprintf(stderr, "WSACleanup() : Failed\n");
      exit(-1);
    }
    if (nErrorStatus!=0) {
      fprintf(stderr, "WSAStartup() : Failed\n");
      exit(-1);
    }
#endif

#ifdef USE_WIN32
#ifdef USE_GTK3
  usleep(1e6);
#endif
#endif
  
  // Check latest ver via network
  CheckVer(NULL, (gpointer)hg);

  if(hg->filename_hoe){
    ReadHOE(hg, FALSE);
  }

  // Instrument selection
  if(hg->inst<0){
    SelectInst(hg, FALSE);
  }

  // main GUI start up
  gui_init(hg);

  // Time initialize (23:55 in fr_date)
  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
  hg->skymon_adj_min = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_time,
							   0, 60*24,
							   10.0, 60.0, 0);
  
  hg->init_flag=TRUE;

  if((hg->filename_read)&&(!hg->filename_hoe)){
    ReadList(hg);
  }
  ////make_obj_list(hg,TRUE);
  make_obj_tree(hg);

  gtk_main();
}
