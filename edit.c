//    hoe : Subaru HDS OPE file Editor
//      edit.c : Edit OPE file
//                                           2018.02.14  A.Tajitsu

#include"main.h"    // 設定ヘッダ

////////////////////// Global Args //////////////////////
extern gboolean flagChildDialog;
extern gboolean flagSkymon;
extern gboolean flagPlot;
extern gboolean flagFC;
extern gboolean flagPlan;
extern gboolean flagPAM;
extern gboolean flagService;
extern gboolean flag_getFCDB;
extern gboolean flag_getDSS;
extern gboolean flag_make_obj_tree;
extern gboolean flag_make_line_tree;
extern gboolean flag_make_etc_tree;
extern gboolean flag_nodraw;

extern int debug_flg;

#ifndef USE_WIN32
extern pid_t fc_pid;
#endif
extern pid_t fcdb_pid;
extern pid_t stddb_pid;


void close_shedit();
void close_opedit();
void menu_close_opedit();
void menu_close_shedit();
void save_opedit();
void do_save_shedit();

GtkWidget *make_opedit_menu();
GtkWidget *make_shedit_menu();

void add_FocusSVSequence();
void add_FocusSV();
void add_BIAS();
void add_Setup();
void add_Comp();
void add_Obj();
void add_Def();

static void cc_e_list();

void GTK2InsertText();
void GTK2InsertText_sh();

void insert();

gchar* create_filename_sh();

GtkWidget *shedit_text;
GtkWidget *opedit_main;
GtkWidget *opedit_text;
GtkTextBuffer *text_buffer;


// Create SH Edit Window
void create_shedit_dialog(typHOE *hg)
{
  GtkWidget *shedit_tbl;
  GtkWidget *button;
  GtkWidget *shedit_scroll;
  GtkWidget *shedit_wbox;
  GtkWidget *table1,*frame,*frame2,*hbox;
  GtkWidget *combo,*label,*spinner;
  GtkAdjustment *adj;
  gchar ope_buffer[BUFFSIZE];
  gchar *fp_1, *fp_2;
  guint nchars;
  GtkTextIter start_iter, end_iter;
  GtkTextMark *end_mark;
  gchar *title_tmp;
  FILE *infile;
  GtkWidget *editbar;
  gchar tmp[10];
  confEdit *cdata[MAX_USESETUP];
  gint i_use,i_list;
  gchar *filename_path=NULL, *filename_tmp=NULL;


 
  hg->shedit_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  text_buffer = gtk_text_buffer_new(NULL);


  shedit_wbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->shedit_main), shedit_wbox);

  editbar=make_shedit_menu(hg);
  gtk_box_pack_start(GTK_BOX(shedit_wbox), editbar,FALSE, FALSE, 0);
  
  gtk_widget_set_size_request (hg->shedit_main, 900,600);
  gtk_window_set_modal(GTK_WINDOW(hg->shedit_main),TRUE);

  if(hg->filename_sh){
    filename_path=g_strdup(g_path_get_dirname(hg->filename_sh));
    g_free(hg->filename_sh);
  }
  if(filename_path){
    filename_tmp=create_filename_sh(hg);
    hg->filename_sh=g_strconcat(filename_path,G_DIR_SEPARATOR_S,filename_tmp,NULL);
    g_free(filename_tmp);
    g_free(filename_path);
  }
  else{
    hg->filename_sh=create_filename_sh(hg);
  }
  
  title_tmp=g_strdup_printf("HOE : Shell Script Editor [%s]",g_path_get_basename(hg->filename_sh));
  gtk_window_set_title(GTK_WINDOW(hg->shedit_main), title_tmp);
  if(title_tmp) g_free(title_tmp);
  gtk_widget_realize(hg->shedit_main);
  my_signal_connect(hg->shedit_main,"destroy",
  		    close_shedit, 
  		    GTK_WIDGET(hg->shedit_main));
  gtk_container_set_border_width (GTK_CONTAINER (hg->shedit_main), 0);
  

  // Text Editor
  shedit_tbl = gtkut_table_new(6, 1, FALSE, 0, 0, 0);
  gtk_container_add (GTK_CONTAINER (shedit_wbox), shedit_tbl);
  
  shedit_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(shedit_scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  shedit_text = gtk_text_view_new_with_buffer (text_buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (shedit_text), TRUE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (shedit_text), TRUE);
  
  gtk_container_add(GTK_CONTAINER(shedit_scroll), shedit_text);
  
  gtkut_table_attach_defaults(shedit_tbl, shedit_scroll, 0, 5, 0, 1);
  
  infile=fopen(hg->fcdb_file,"r");
  
  if(infile){
    gtk_text_buffer_create_tag (text_buffer, "underline",
                              "underline", PANGO_UNDERLINE_SINGLE, NULL);
    gtk_text_buffer_create_tag (text_buffer, "heading",
				"weight", PANGO_WEIGHT_BOLD,
				//"size", 15 * PANGO_SCALE,
				NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com1",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com1,
#else
			       "foreground-gdk", &color_com1,
#endif
			       NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com2",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com2,
#else
			       "foreground-gdk", &color_com2,
#endif
			       NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com3",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com3,
#else
			       "foreground-gdk", &color_com3,
#endif
			       NULL);

    gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
    while(!feof(infile)){
      if(fgets(ope_buffer,BUFFSIZE-1,infile)!=NULL){
	GTK2InsertText_sh(text_buffer, &start_iter, ope_buffer);
      }
    }

    fclose(infile);
  }
    
  gtk_widget_show_all(hg->shedit_main);

  gtk_main();

  //for(i_use=0;i_use<MAX_USESETUP;i_use++){
  //  g_free(cdata[i_use]);
  //}
}

// Create OPE Edit Window
void create_opedit_dialog(typHOE *hg)
{
  GtkWidget *opedit_tbl;
  GtkWidget *button;
  GtkWidget *opedit_scroll;
  GtkWidget *opedit_wbox;
  GtkWidget *table1,*frame,*frame2,*hbox;
  GtkWidget *combo,*label,*spinner;
  GtkAdjustment *adj;
  gchar ope_buffer[BUFFSIZE];
  gchar *fp_1, *fp_2;
  guint nchars;
  GtkTextIter start_iter, end_iter;
  GtkTextMark *end_mark;
  gchar *title_tmp;
  FILE *infile;
  GtkWidget *editbar;
  gchar tmp[10];
  confEdit *cdata[MAX_USESETUP];
  gint i_use,i_list;


 
  opedit_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  text_buffer = gtk_text_buffer_new(NULL);


  opedit_wbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (opedit_main), opedit_wbox);

  editbar=make_opedit_menu(hg);
  gtk_box_pack_start(GTK_BOX(opedit_wbox), editbar,FALSE, FALSE, 0);
  
  gtk_widget_set_size_request (opedit_main, 900,600);
  title_tmp=g_strdup_printf("HOE : Text Editor [%s]",g_path_get_basename(hg->filename_write));
  gtk_window_set_title(GTK_WINDOW(opedit_main), title_tmp);
  if(title_tmp) g_free(title_tmp);
  gtk_widget_realize(opedit_main);
  my_signal_connect(opedit_main,"destroy",
  		    close_opedit, 
  		    GTK_WIDGET(opedit_main));
  gtk_container_set_border_width (GTK_CONTAINER (opedit_main), 0);
  

  // Text Editor
  opedit_tbl = gtkut_table_new(6, 1, FALSE, 0, 0, 0);
  gtk_container_add (GTK_CONTAINER (opedit_wbox), opedit_tbl);
  
  opedit_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(opedit_scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  opedit_text = gtk_text_view_new_with_buffer (text_buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (opedit_text), TRUE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (opedit_text), TRUE);
  
  gtk_container_add(GTK_CONTAINER(opedit_scroll), opedit_text);
  
  gtkut_table_attach_defaults(opedit_tbl, opedit_scroll, 0, 5, 0, 1);
  
  infile=fopen(hg->filename_write,"r");
  
  if(infile){
    gtk_text_buffer_create_tag (text_buffer, "underline",
				"underline", PANGO_UNDERLINE_SINGLE, NULL);
    gtk_text_buffer_create_tag (text_buffer, "heading",
				"weight", PANGO_WEIGHT_BOLD,
				//"size", 15 * PANGO_SCALE,
				NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com1",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com1,
#else
			       "foreground-gdk", &color_com1,
#endif
			       NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com2",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com2,
#else
			       "foreground-gdk", &color_com2,
#endif
			       NULL);
    gtk_text_buffer_create_tag(text_buffer, "color_com3",
#ifdef USE_GTK3
			       "foreground-rgba", &color_com3,
#else
			       "foreground-gdk", &color_com3,
#endif
			       NULL);

    gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
    while(!feof(infile)){
      if(fgets(ope_buffer,BUFFSIZE-1,infile)!=NULL){
	GTK2InsertText(text_buffer, &start_iter, ope_buffer);
      }
    }

    fclose(infile);
  }
    
  gtk_widget_show_all(opedit_main);

  gtk_main();

  //for(i_use=0;i_use<MAX_USESETUP;i_use++){
  //  g_free(cdata[i_use]);
  //}
}


void close_opedit(GtkWidget *w, GtkWidget *dialog)
{
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  flagChildDialog=FALSE;
}

void close_shedit(GtkWidget *w, GtkWidget *dialog)
{
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  flagChildDialog=FALSE;
}


void menu_close_opedit(GtkWidget *widget,gpointer gdata)
{
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(opedit_main));

  flagChildDialog=FALSE;
}

void menu_close_shedit(GtkWidget *widget,gpointer gdata)
{
  typHOE *hg=(typHOE *)gdata;
  
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(hg->shedit_main));

  flagChildDialog=FALSE;
}


void save_opedit(GtkWidget *widget, gpointer gdata)
{
  FILE *outfile;
  gchar *ope_buffer;
  gint nchars;
  typHOE *hg;
  GtkTextIter start_iter, end_iter;

  hg=(typHOE *)gdata;

  outfile = fopen(hg->filename_write, "w");

  if(outfile){
    gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
    gtk_text_buffer_get_end_iter(text_buffer, &end_iter);
    ope_buffer=gtk_text_buffer_get_text(text_buffer,
					&start_iter,
					&end_iter,
					TRUE);
    nchars = fwrite(ope_buffer,sizeof(gchar),strlen(ope_buffer),outfile);
    
    fclose(outfile);

    if(nchars!=strlen(ope_buffer)){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_write);
    }
    
    g_free(ope_buffer);
  }
  else{
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_write);
  }
 
}


void do_save_shedit(GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  hoe_SaveFile(hg, SAVE_FILE_SH);
}  


void Save_sh(typHOE *hg){
  FILE *outfile;
  gchar *ope_buffer;
  gint nchars;
  GtkTextIter start_iter, end_iter;

  outfile = fopen(hg->filename_sh, "w");

  if(outfile){
    gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
    gtk_text_buffer_get_end_iter(text_buffer, &end_iter);
    ope_buffer=gtk_text_buffer_get_text(text_buffer,
					&start_iter,
					&end_iter,
					TRUE);
    nchars = fwrite(ope_buffer,sizeof(gchar),strlen(ope_buffer),outfile);
    
    fclose(outfile);

    if(nchars!=strlen(ope_buffer)){
      popup_message(hg->shedit_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR,
#endif
		    POPUP_TIMEOUT*1,
		    "Failed to Save the file",
		    " ",
		    hg->filename_sh,
		    NULL);
    }
    
    g_free(ope_buffer);

    if((chmod(hg->filename_sh,(S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH ))) != 0){
      popup_message(hg->shedit_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR,
#endif
		    POPUP_TIMEOUT*1,
		    "Failed to Chmod the file",
		    " ",
		    hg->filename_sh,
		    NULL);
    }
  }
  else{
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_sh);
  }
 
}


GtkWidget *make_opedit_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //File/Save
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Save");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",save_opedit,(gpointer)hg);

  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_close_opedit,NULL);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}


GtkWidget *make_shedit_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //File/Save
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Save");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_shedit,(gpointer)hg);

  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_close_shedit, (gpointer)hg);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}


void add_FocusSVSequence(GtkWidget *widget, gpointer gdata){
  insert("### FocusSV\n");
  insert("FocusSVSequence $DEF_COMMON\n");
  insert("SetStarSize $DEF_PROTO SEEING=0.60\n");
  insert("\n");
}

void add_FocusSV(GtkWidget *widget, gpointer gdata){
  insert("### FocusSV\n");
  insert("FocusSV $DEF_COMMON\n");
  insert("SetStarSize $DEF_PROTO SEEING=0.60\n");
  insert("\n");
}

void add_BIAS(GtkWidget *widget, gpointer gdata){
  insert("### BIAS\n");
  insert("GetBias $DEF_SPEC OBJECT=BIAS\n");
  insert("\n");
}


void add_Setup(GtkWidget *widget, gpointer gdata){
  guint i_use,i_set;
  gchar tmp[BUFFSIZE];
  confEdit *cdata;

  cdata = (confEdit *)gdata;
  
  i_use=cdata->i_use;

  if(cdata->hg->setup[i_use].use){
    if(cdata->hg->setup[i_use].setup<0){ // NonStd
      i_set=-cdata->hg->setup[i_use].setup-1;
      sprintf(tmp, "### Change Setup  NonStd-%d\n", i_set+1);
      insert(tmp);
      sprintf(tmp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
	      cdata->hg->setup[i_use].slit_length);
      insert(tmp);
      if(cdata->hg->nonstd[i_set].col==COL_BLUE){
	sprintf(tmp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Blue CROSS_SCAN=%d COLLIMATOR=Blue $CAMZ_B",
		cdata->hg->setup[i_use].fil1,cdata->hg->setup[i_use].fil2,
		cdata->hg->nonstd[i_set].cross);
	insert(tmp);
	if(cdata->hg->nonstd[i_set].echelle!=1440){
	  sprintf(tmp, " ECHELLE=%d", cdata->hg->nonstd[i_set].echelle);
	  insert(tmp);
	}
	if(cdata->hg->nonstd[i_set].camr!=-3600){
	  sprintf(tmp, " CAMERA_ROTATE=%d", cdata->hg->nonstd[i_set].camr);
	  insert(tmp);
	}
      }
      else{
	sprintf(tmp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Red CROSS_SCAN=%d COLLIMATOR=Red $CAMZ_R",
		cdata->hg->setup[i_use].fil1,cdata->hg->setup[i_use].fil2,
		cdata->hg->nonstd[i_set].cross);
	insert(tmp);
	if(cdata->hg->nonstd[i_set].echelle!=1440){
	  sprintf(tmp, " ECHELLE=%d", cdata->hg->nonstd[i_set].echelle);
	  insert(tmp);
	}
	if(cdata->hg->nonstd[i_set].camr!=-3600){
	  sprintf(tmp, " CAMERA_ROTATE=%d", cdata->hg->nonstd[i_set].camr);
	  insert(tmp);
	}
      }
    }
    else{ //Std
      i_set=cdata->hg->setup[i_use].setup;
      
      sprintf(tmp, "### Change Setup  Std%s\n", HDS_setups[i_set].initial);
      insert(tmp);
      
      sprintf(tmp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
	      cdata->hg->setup[i_use].slit_length);
      insert(tmp);
      if(i_set<StdI2b){
	sprintf(tmp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_B\n",
		cdata->hg->setup[i_use].fil1,
		cdata->hg->setup[i_use].fil2,
		HDS_setups[i_set].cross,
		HDS_setups[i_set].initial,
		HDS_setups[i_set].col);
	insert(tmp);
      }
      else{
	sprintf(tmp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_R\n",
		cdata->hg->setup[i_use].fil1,
		cdata->hg->setup[i_use].fil2,
		HDS_setups[i_set].cross,
		HDS_setups[i_set].initial,
		HDS_setups[i_set].col);
	insert(tmp);
      }
    }
    insert("\n");
  }
}


void add_Comp(GtkWidget *widget, gpointer gdata){
  guint i_use;
  gchar tmp[BUFFSIZE];
  confEdit *cdata;

  cdata = (confEdit *)gdata;
  
  i_use=cdata->i_use;

  if(cdata->hg->setup[i_use].use){
    {
      gint i_bin;
      i_bin=cdata->hg->setup[i_use].binning;
      if(cdata->hg->setup[i_use].setup < 0){ // NonStd
	sprintf(tmp, "### COMPARISON  for NonStd-%d  %dx%dBINNING\n", 
		-cdata->hg->setup[i_use].setup,cdata->hg->binning[i_bin].x, cdata->hg->binning[i_bin].y);
	insert(tmp);
      }
      else{ // Std
	sprintf(tmp, "### COMPARISON  for Std%s  %dx%dBINNING\n", 
		HDS_setups[cdata->hg->setup[i_use].setup].initial,cdata->hg->binning[i_bin].x, cdata->hg->binning[i_bin].y);
	insert(tmp);
      }
      sprintf(tmp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n"
	      ,cdata->hg->setup[i_use].slit_length,cdata->hg->setup[i_use].slit_width);
      insert(tmp);
      sprintf(tmp, "SETUPCOMPARISON_HCT $DEF_PROTO OBJECT=HCT LAMP=HCT1 AMP=15.0 FILTER01=1 FILTER02=1 FILTER03=1 FILTER04=1 F_SELECT=NS_OPT\n");
      insert(tmp);
      sprintf(tmp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
	      20/cdata->hg->binning[i_bin].x/cdata->hg->binning[i_bin].y);
      insert(tmp);
      insert("### [LAUNCHER/HDS] ShutdownComparison HCT LampOff during ReadOut #####\n");
    }
    insert("\n");
  }
}


void add_Obj(GtkWidget *widget, gpointer gdata){
  guint i_use,i_list,i_times;
  gchar tmp[BUFFSIZE];
  confEdit *cdata;
  gchar *tgt;

  cdata = (confEdit *)gdata;
  
  i_use=cdata->i_use;
  i_list=cdata->hg->e_list;
  
  switch(cdata->hg->obj[i_list].guide){
  case NO_GUIDE:
    if(cdata->hg->obj[i_list].note){
      sprintf(tmp, "# %s   No Guide,   %s\n",cdata->hg->obj[i_list].name,
	      cdata->hg->obj[i_list].note);
      insert(tmp);
    }
    else{
      sprintf(tmp, "# %s   No Guide\n",cdata->hg->obj[i_list].name);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].setup < 0){ //Non-Std
      sprintf(tmp, "#    NonStd-%d %dx%dbinning\n",
	      -cdata->hg->setup[i_use].setup,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    else{  //Std
      sprintf(tmp, "#    Std%s %dx%dbinning\n",
	      HDS_setups[cdata->hg->setup[i_use].setup].initial,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].imr){
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=NO $DEF_PROTO $%s SVRegion=%d Exptime_SV=%d IMGROT_FLAG=1 Slit_PA=%.1f Slit_Length=%d Slit_Width=%d SVIntegrate=%d $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->exptime_sv,
	      cdata->hg->obj[i_list].pa, 
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    else{
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=NO $DEF_PROTO $%s SVRegion=%d Exptime_SV=%d IMGROT_FLAG=0 Slit_Length=%d Slit_Width=%d SVIntegrate=%d $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->exptime_sv,
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    break;
  case AG_GUIDE:
    if(cdata->hg->obj[i_list].note){
      sprintf(tmp, "# %s   AG Guide,   %s\n",cdata->hg->obj[i_list].name,
	      cdata->hg->obj[i_list].note);
      insert(tmp);
    }
    else{
      sprintf(tmp, "# %s   AG Guide\n",cdata->hg->obj[i_list].name);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].setup < 0){ //Non-Std
      sprintf(tmp, "#    NonStd-%d %dx%dbinning\n",
	      -cdata->hg->setup[i_use].setup,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    else{  //Std
      sprintf(tmp, "#    Std%s %dx%dbinning\n",
	      HDS_setups[cdata->hg->setup[i_use].setup].initial,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].imr){
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=AG $DEF_PROTO $%s SVRegion=%d Exptime_SV=%d IMGROT_FLAG=1 Slit_PA=%.1f Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->exptime_sv,
	      cdata->hg->obj[i_list].pa, 
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    else{
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=AG $DEF_PROTO $%s SVRegion=%d Exptime_SV=%d IMGROT_FLAG=0 Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->exptime_sv,
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    break;
  case SV_GUIDE:
    if(cdata->hg->obj[i_list].note){
      sprintf(tmp, "# %s   SV Guide,   %s\n",cdata->hg->obj[i_list].name,
	      cdata->hg->obj[i_list].note);
      insert(tmp);
    }
    else{
      sprintf(tmp, "# %s   SV Guide\n",cdata->hg->obj[i_list].name);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].setup < 0){ //Non-Std
      sprintf(tmp, "#    NonStd-%d %dx%dbinning\n",
	      -cdata->hg->setup[i_use].setup,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    else{  //Std
      sprintf(tmp, "#    Std%s %dx%dbinning\n",
	      HDS_setups[cdata->hg->setup[i_use].setup].initial,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].imr){
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=SV $DEF_PROTO $%s ReadRegion=%d CalcRegion=%d Exptime_SV=%d IMGROT_FLAG=1 Slit_PA=%.1f Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d ReadArea=Part Mode=SemiAuto $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->sv_calc,cdata->hg->exptime_sv,
	      cdata->hg->obj[i_list].pa, 
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    else{
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=SV $DEF_PROTO $%s ReadRegion=%d CalcRegion=%d Exptime_SV=%d IMGROT_FLAG=0 Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d ReadArea=Part Mode=SemiAuto $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->sv_calc,cdata->hg->exptime_sv,
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    break;
  case SVSAFE_GUIDE:
    if(cdata->hg->obj[i_list].note){
      sprintf(tmp, "# %s   SV[Safe] Guide,   %s\n",cdata->hg->obj[i_list].name,
	      cdata->hg->obj[i_list].note);
      insert(tmp);
    }
    else{
      sprintf(tmp, "# %s   SV[Safe] Guide\n",cdata->hg->obj[i_list].name);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].setup < 0){ //Non-Std
      sprintf(tmp, "#    NonStd-%d %dx%dbinning\n",
	      -cdata->hg->setup[i_use].setup,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    else{  //Std
      sprintf(tmp, "#    Std%s %dx%dbinning\n",
	      HDS_setups[cdata->hg->setup[i_use].setup].initial,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].x,
	      cdata->hg->binning[cdata->hg->setup[i_use].binning].y);
      insert(tmp);
    }
    if(cdata->hg->setup[i_use].imr){
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=SV $DEF_PROTO $%s ReadRegion=%d CalcRegion=%d Exptime_SV=%d IMGROT_FLAG=1 Slit_PA=%.1f Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d ReadArea=Part Mode=Safe $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->sv_calc,cdata->hg->exptime_sv,
	      cdata->hg->obj[i_list].pa, 
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    else{
      tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
      sprintf(tmp, "SetupField Guide=SV $DEF_PROTO $%s ReadRegion=%d CalcRegion=%d Exptime_SV=%d IMGROT_FLAG=0 Slit_Length=%d Slit_Width=%d Exptime_Factor=%d Brightness=%d SVIntegrate=%d ReadArea=Part Mode=Safe $SV_X $SV_Y\n",
	      tgt, 
	      cdata->hg->sv_region,cdata->hg->sv_calc,cdata->hg->exptime_sv,
	      cdata->hg->setup[i_use].slit_length,
	      cdata->hg->setup[i_use].slit_width,
	      cdata->hg->exptime_factor,cdata->hg->brightness,cdata->hg->sv_integrate);
      g_free(tgt);
      insert(tmp);
    }
    break;
  }

  tgt=make_tgt(cdata->hg->obj[i_list].name, "TGT_");
  sprintf(tmp, "GetObject $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
	  cdata->hg->e_exp, cdata->hg->sv_integrate, tgt);
  g_free(tgt);
  for(i_times=0;i_times<cdata->hg->e_times;i_times++){
    insert(tmp);
  }
  insert("\n");
  
}


void add_Def(GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  guint i_list;
  gchar tmp[BUFFSIZE];
  gchar *tgt;

  hg=(typHOE *)gdata;
  i_list=hg->e_list;

  if(hg->obj[i_list].name){
    tgt=make_tgt(hg->obj[i_list].name, "TGT_");
    sprintf(tmp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	    tgt, hg->obj[i_list].name, 
	    hg->obj[i_list].ra,  hg->obj[i_list].dec, hg->obj[i_list].equinox);
    g_free(tgt);
    insert(tmp);
  }
}


void insert(gchar *insert_text){
  GtkTextIter iter;

  gtk_text_buffer_insert_at_cursor(text_buffer,
  	   insert_text,
  	   strlen(insert_text));
}

void GTK2InsertText(GtkTextBuffer *buffer,
		    GtkTextIter   *iter,
                    const gchar   *text){
  if(strncmp(text,"###",3)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com3",
					      NULL);
  }
  else if(strncmp(text,"##",2)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com2",
					      NULL);
  }
  else if(strncmp(text,"#",1)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com1",
					      NULL);
  }
  else{
    gtk_text_buffer_insert (buffer, iter, text, -1);
  }
}

void GTK2InsertText_sh(GtkTextBuffer *buffer,
		       GtkTextIter   *iter,
		       const gchar   *text){
  if(strncmp(text,"<",1)==0){
  }
  else if(strncmp(text,"###skipping",11)==0){
  }
  else if(strncmp(text,"###",3)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com3",
					      NULL);
  }
  else if(strncmp(text,"##",2)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com2",
					      NULL);
  }
  else if(strncmp(text,"#",1)==0){
    gtk_text_buffer_insert_with_tags_by_name (buffer, iter,
					      text, -1,
					      "color_com1",
					      NULL);
  }
  else{
    gtk_text_buffer_insert (buffer, iter, text, -1);
  }
}


gchar* create_filename_sh(typHOE *hg){
  gchar *ret=NULL, *tmp_name=NULL;
  
  tmp_name=repl_spc(hg->obj[hg->sh_i].name);

  if(tmp_name){
    switch(hg->inst){
    case INST_KOOLS:
      switch(hg->fcdb_type){
      case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
	ret=g_strdup_printf("%s%04d_%s__%dx%d__KOOLS_%s.sh",
			    (hg->plan_queue) ? "Q-Obj" : "Obj",
			    hg->sh_i+1,
			    tmp_name,
			    hg->obj[hg->sh_i].exp,
			    hg->obj[hg->sh_i].repeat,
			    kools_grism_name[hg->obj[hg->sh_i].kools.grism]);
	break;
      default:
	ret=g_strdup_printf("%s%04d_%s__%dx%d__KOOLS_%s.sh",
			    (hg->def_kools_queue) ? "Q-Obj" : "Obj",
			    hg->sh_i+1,
			    tmp_name,
			    hg->obj[hg->sh_i].exp,
			    hg->obj[hg->sh_i].repeat,
			    kools_grism_name[hg->obj[hg->sh_i].kools.grism]);
	break;
      }	
      break;
      
    case INST_TRICCS:
      switch(hg->fcdb_type){
      case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
	ret=g_strdup_printf("%s%04d_%s__%.4lfx%d__TriCCS.sh",
			    (hg->plan_queue) ? "Q-Obj" : "Obj",
			    hg->sh_i+1,
			    tmp_name,
			    hg->obj[hg->sh_i].dexp,
			    hg->obj[hg->sh_i].repeat);
	break;
      default:
	ret=g_strdup_printf("%s%04d_%s__%.4lfx%d__TriCCS.sh",
			    (hg->def_kools_queue) ? "Q-Obj" : "Obj",
			    hg->sh_i+1,
			    tmp_name,
			    hg->obj[hg->sh_i].dexp,
			    hg->obj[hg->sh_i].repeat);
	break;
      }
      break;
    }
    
    return(ret);
  }
  else{
    return(NULL);
  }
}
