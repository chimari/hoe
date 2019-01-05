//    HOE : Subaru HDS++ OPE file Editor
//        io_gui.c     GUI for File I/O etc.
//                                           2019.01.03  A.Tajitsu

#include "main.h"

//////////////////////////////////////////////////////////////
///////////////  Common Functions
//////////////////////////////////////////////////////////////

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...)
{
  GtkFileFilter *filter;
  gchar *name_tmp;
  va_list args;
  gchar *pattern, *ptncat=NULL, *ptncat2=NULL;

  filter=gtk_file_filter_new();

  va_start(args, name);
  while(1){
    pattern=va_arg(args, gchar*);
    if(!pattern) break;
    gtk_file_filter_add_pattern(filter, pattern);
    if(!ptncat){
      ptncat=g_strdup(pattern);
    }
    else{
      if(ptncat2) g_free(ptncat2);
      ptncat2=g_strdup(ptncat);
      if(ptncat) g_free(ptncat);
      ptncat=g_strconcat(ptncat2,",",pattern,NULL);
    }
  }
  va_end(args);

  name_tmp=g_strconcat(name," [",ptncat,"]",NULL);
  gtk_file_filter_set_name(filter, name_tmp);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
  if(name_tmp) g_free(name_tmp);
  if(ptncat) g_free(ptncat);
  if(ptncat2) g_free(ptncat2);
}


gboolean CheckChildDialog(GtkWidget *w){
  if(flagChildDialog){
    popup_message(w, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return(TRUE);
  }
  else{
    return(FALSE);
  }
}


gboolean CheckDefDup(typHOE *hg){
  gint i_list,j_list;
  gchar *tgt=NULL, *tgt1=NULL, *tmp=NULL;
  gboolean ret=FALSE;
  gdouble d_ra, d_dec, d_ra1, d_dec1;

  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name, "TGT_");
    for(j_list=0;j_list<i_list;j_list++){
      tgt1=make_tgt(hg->obj[j_list].name, "TGT_");
      if(strcmp(tgt,tgt1)==0){
	d_ra=ra_to_deg(hg->obj[i_list].ra);
	d_dec=dec_to_deg(hg->obj[i_list].dec);
	d_ra1=ra_to_deg(hg->obj[j_list].ra);
	d_dec1=dec_to_deg(hg->obj[j_list].dec);
	if(deg_sep(d_ra,d_dec,d_ra1,d_dec1)>10.0){ // larget than 10 arcsec
	  ret=TRUE;
	}
      }
      if(tgt1) g_free(tgt1);
      if(ret) break;
    }
    if(tgt) g_free(tgt);
    if(ret) break;
  }

  if(ret){
    tgt =g_strdup_printf("%d : \"%s\"  RA=%09.2lf Dec=%+010.2lf",
      i_list+1,hg->obj[i_list].name,hg->obj[i_list].ra,hg->obj[i_list].dec);
    tgt1=g_strdup_printf("%d : \"%s\"  RA=%09.2lf Dec=%+010.2lf",
      j_list+1,hg->obj[j_list].name,hg->obj[j_list].ra,hg->obj[j_list].dec);
    popup_message(hg->w_top,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: found a duplicate target name with different coordinates.",
		  " ",
		  tgt1,
		  tgt,
		  " ", 
		  "       Please change target name.",
		  NULL);
    g_free(tgt);
    g_free(tgt1);
    return(TRUE);
  }
  return(FALSE);
}


gboolean ow_dialog (typHOE *hg, gchar *fname)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  gchar *tmp;

  dialog = gtk_dialog_new_with_buttons("HOE : Overwrite?",
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

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_CANCEL));

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

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  tmp=g_strdup_printf("The file, \"%s\", already exists.", fname);
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

  label = gtk_label_new ("Do you want to overwrite it?");
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
    gtk_widget_destroy(dialog);
    return(TRUE);
  }
  else{
    gtk_widget_destroy(dialog);
    return(FALSE);
  }
}



////////////////////////////////////////////////////////////
////////////////  Functions calling from menu callbacks
////////////////////////////////////////////////////////////



void SelectInst(typHOE *hg, gboolean destroy_flag){
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GtkWidget *rb[NUM_INST];
  GdkPixbuf *pixbuf, *pixbuf2;
  gint old_inst=hg->inst;  // = -1 for initialization
  gchar *tmp;
  gint i_inst;
  
  if(hg->inst<0){
    hg->inst=INST_HDS;
  }
  
  dialog = gtk_dialog_new_with_buttons("HOE : Select your instrument.",
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

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  96,96,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);
  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Please select an instrument for your obs.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  for(i_inst=0;i_inst<NUM_INST;i_inst++){
    tmp=g_strdup_printf("%s (%s)",inst_name_short[i_inst], inst_name_long[i_inst]);
    rb[i_inst]
      = gtk_radio_button_new_with_label_from_widget ((i_inst==0) ? NULL : GTK_RADIO_BUTTON(rb[0]),
						     tmp);
    gtk_box_pack_start(GTK_BOX(vbox), rb[i_inst], FALSE, FALSE, 0);
    my_signal_connect (rb[i_inst], "toggled", cc_radio, &hg->inst);
  }

  label = gtk_label_new (" ");
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->inst]),TRUE);

  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    if(old_inst<0){
      init_inst(hg);
    }
    else if (hg->inst!=old_inst){
      init_inst(hg);
      // Plan Initialize
      hg->i_plan_max=0;

      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-information", 
#else
		    GTK_STOCK_DIALOG_INFO,
#endif
		    POPUP_TIMEOUT*1,
		    "Your observing plan has been initialized.",
		    NULL);
    }
  }
  else{
    exit(0);
  }
  
  if(destroy_flag){
    gtk_widget_destroy(hg->all_note);
    
    flag_make_obj_tree=FALSE;
    flag_make_line_tree=FALSE;
    
    make_note(hg);
  }
}


void create_quit_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Quit Program",
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

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Do you want to quit this program?");
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
    WriteConf(hg);
    gtk_widget_destroy(dialog);
    exit(0);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
}


///////////////////////////////////////////////////////////////////
////////// Open Files
///////////////////////////////////////////////////////////////////

void action_read_list (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *) gdata;
  
  hg->list_read=OPEN_FILE_READ_LIST;
  select_list_style(hg);
}

void action_merge_list (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *) gdata;
  
  hg->list_read=OPEN_FILE_MERGE_LIST;
  select_list_style(hg);
}


void select_list_style (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GtkWidget *rb[LIST_STYLE_NUM];

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  
  flagChildDialog=TRUE;
  
  dialog = gtk_dialog_new_with_buttons("HOE : Select List Style",
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
  
  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     vbox,FALSE, FALSE, 0);

  rb[LIST_DEFAULT]
    = gtk_radio_button_new_with_label_from_widget (NULL, "Name,  RA,  Dec,  Equinox(, comment)");
  gtk_box_pack_start(GTK_BOX(vbox), rb[LIST_DEFAULT], FALSE, FALSE, 0);
  my_signal_connect (rb[LIST_DEFAULT], "toggled", cc_radio, &hg->list_style);

  rb[LIST_MAG] 
    = gtk_radio_button_new_with_label_from_widget 
    (GTK_RADIO_BUTTON(rb[LIST_DEFAULT]), "Name,  RA,  Dec,  Equinox,  Mag(, comment)");
  gtk_box_pack_start(GTK_BOX(vbox), rb[LIST_MAG], FALSE, FALSE, 0);
  my_signal_connect (rb[LIST_MAG], "toggled", cc_radio, &hg->list_style);

  gtk_widget_show_all(dialog);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->list_style]),TRUE);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    OpenFile(hg, hg->list_read);
  }
  
  flagChildDialog=FALSE;
}


void do_open_ope (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  
  flagChildDialog=TRUE;
  
  OpenFile(hg, OPEN_FILE_READ_OPE);

  flagChildDialog=FALSE;
}


void do_upload_ope (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }

  flagChildDialog=TRUE;

  OpenFile(hg, OPEN_FILE_UPLOAD_OPE);

  flagChildDialog=FALSE;
}


void do_open_hoe (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }

  flagChildDialog=TRUE;

  OpenFile(hg, OPEN_FILE_READ_HOE);

  flagChildDialog=FALSE;
}


void OpenFile(typHOE *hg, guint mode){
  GtkWidget *fdialog;
  gchar *tmp;
  gchar **tgt_file;

  switch(mode){
  case OPEN_FILE_READ_LIST:
  case OPEN_FILE_MERGE_LIST:
    tmp=g_strdup("HOE : Select an Input List File");
    tgt_file=&hg->filename_read;
    break;

  case OPEN_FILE_READ_OPE:
    tmp=g_strdup("HOE : Select an OPE File to read the Target List");
    tgt_file=&hg->filename_read;
    break;

  case OPEN_FILE_UPLOAD_OPE:
    tmp=g_strdup("HOE : Select an OPE File to read the Target List");
    tgt_file=&hg->filename_read;
    break;

  case OPEN_FILE_EDIT_OPE:
    tmp=g_strdup("HOE : Select an OPE File to edited");
    tgt_file=&hg->filename_write;
    break;

  case OPEN_FILE_READ_HOE:
    tmp=g_strdup("HOE : Select a HOE Config file");
    tgt_file=&hg->filename_hoe;
    break;

  case OPEN_FILE_READ_NST:
    tmp=g_strdup("HOE : Select Non-Sidereal Tracking File [TSC]");
    tgt_file=&hg->filename_nst;
    break;

  case OPEN_FILE_READ_JPL:
    tmp=g_strdup("HOE : Select Non-Sidereal Tracking File [JPL HORIZONS]");
    tgt_file=&hg->filename_jpl;
    break;

  case OPEN_FILE_CONV_JPL:
    tmp=g_strdup("HOE : Select Non-Sidereal Tracking File  [JPL HRIZONS]");
    tgt_file=&hg->filename_jpl;
    break;
  }
  
  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(*tgt_file,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(*tgt_file));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(*tgt_file));
  }

  switch(mode){
  case OPEN_FILE_READ_OPE:
  case OPEN_FILE_UPLOAD_OPE:
  case OPEN_FILE_EDIT_OPE:
    my_file_chooser_add_filter(fdialog,"OPE File",
			       "*." OPE_EXTENSION,NULL);
    break;

  case OPEN_FILE_READ_HOE:
    my_file_chooser_add_filter(fdialog,"HOE Config File",
			       "*." HOE_EXTENSION,NULL);
    break;
    
  case OPEN_FILE_READ_NST:
    my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			       "*." NST1_EXTENSION,
			       "*." NST2_EXTENSION,
			       NULL);
    break;

  case OPEN_FILE_READ_JPL:
  case OPEN_FILE_CONV_JPL:
    my_file_chooser_add_filter(fdialog,"JPL HORIZONS File", 
			       "*." NST1_EXTENSION,
			       "*." NST3_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    break;
    
  default:
    my_file_chooser_add_filter(fdialog,"List File", 
			       "*." LIST1_EXTENSION,
			       "*." LIST2_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    break;
  }
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);

  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    gchar *cpp, *basename0, *basename1;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strdup(dest_file);

      switch(mode){
      case OPEN_FILE_READ_LIST:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadList(hg);
	hg->i_plan_max=0;
	break;

      case OPEN_FILE_MERGE_LIST:
	MergeList(hg);
	break;

      case OPEN_FILE_READ_OPE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadListOPE(hg);
	break;

      case OPEN_FILE_UPLOAD_OPE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	UploadOPE(hg);
	break;

      case OPEN_FILE_EDIT_OPE:
	create_opedit_dialog(hg);
	break;

      case OPEN_FILE_READ_HOE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadHOE(hg, TRUE);
	break;

      case OPEN_FILE_READ_NST:
	MergeNST(hg);
	break;

      case OPEN_FILE_READ_JPL:
	MergeJPL(hg);
	break;

      case OPEN_FILE_CONV_JPL:
	SaveFile(hg, SAVE_FILE_CONV_JPL);
	break;
      }

      
      switch(mode){
      case OPEN_FILE_UPLOAD_OPE:
      case OPEN_FILE_EDIT_OPE:
	break;

      case OPEN_FILE_READ_HOE:
	if(flagSkymon){
	  refresh_skymon(hg->skymon_dw,(gpointer)hg);
	  skymon_set_and_draw(NULL, (gpointer)hg);
	}
	break;

      case OPEN_FILE_READ_NST:
      case OPEN_FILE_READ_JPL:
	//// Current Condition
	if(hg->skymon_mode==SKYMON_SET){
	  calcpa2_skymon(hg);
	}
	else{
	  calcpa2_main(hg);
	}
	
	make_obj_tree(hg);
	trdb_make_tree(hg);

	if(flagSkymon){
	  refresh_skymon(hg->skymon_dw,(gpointer)hg);
	  skymon_set_and_draw(NULL, (gpointer)hg);
	}
	break;
	
      default:
	make_obj_tree(hg);

	if(flagSkymon){
	  refresh_skymon(hg->skymon_dw,(gpointer)hg);
	  skymon_set_and_draw(NULL, (gpointer)hg);
	}
	break;
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning",
#else
		    GTK_STOCK_DIALOG_WARNING, 
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  if(mode=OPEN_FILE_READ_HOE){
    hg->skymon_year=hg->fr_year;
    hg->skymon_month=hg->fr_month;
    hg->skymon_day=hg->fr_day;
    
    if(hg->skymon_mode==SKYMON_SET){
      calc_moon_skymon(hg);
      hg->skymon_hour=23;
      hg->skymon_min=55;
      calcpa2_skymon(hg);
    }
    
    calc_rst(hg);
  }
}


//////////////////// Text target list /////////////////////
void ReadList(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  
  if((fp=fopen(hg->filename_read,"rb"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(strlen(buf)<10) break;
      tmp_char=(char *)strtok(buf,",");
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(tmp_char);
      hg->obj[i_list].name=cut_spc(tmp_char);

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      hg->obj[i_list].pm_ra=0.0;

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      hg->obj[i_list].pm_dec=0.0;
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);

      init_obj(&hg->obj[i_list], hg);

      if(hg->list_style==LIST_MAG){
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,i_list+1,"Magnitude")) break;
	hg->obj[i_list].mag=(gdouble)g_strtod(tmp_char,NULL);
      }
      
      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
      if((tmp_char=(char *)strtok(NULL,"\n"))!=NULL){
	hg->obj[i_list].note=g_strdup(tmp_char);
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }

      i_list++;
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  hg->i_max=i_list;
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}


void MergeList(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use, i_base;
  gchar *tmp_char, *tmp_name;
  gchar *buf=NULL;
  gboolean name_flag;
  
  if((fp=fopen(hg->filename_read,"r"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  i_base=hg->i_max;

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(strlen(buf)<10) break;
      
      tmp_char=(char *)strtok(buf,",");
      tmp_name=cut_spc(tmp_char);
      
      name_flag=FALSE;
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(strcmp(tmp_name,hg->obj[i_list].name)==0){
	  name_flag=TRUE;
	  break;
	}
      }

      if(!name_flag){
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"RA")) break;
	hg->obj[hg->i_max].ra=(gdouble)g_strtod(tmp_char,NULL);
	hg->obj[hg->i_max].pm_ra=0.0;
	
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"Dec")) break;
	hg->obj[hg->i_max].dec=(gdouble)g_strtod(tmp_char,NULL);
	hg->obj[hg->i_max].pm_dec=0.0;
      
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"Equinox")) break;
	hg->obj[hg->i_max].equinox=(gdouble)g_strtod(tmp_char,NULL);

	init_obj(&hg->obj[hg->i_max], hg);
	

	if(hg->list_style==LIST_MAG){
	  tmp_char=(char *)strtok(NULL,",");
	  if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"Magnitude")) break;
	  hg->obj[hg->i_max].mag=(gdouble)g_strtod(tmp_char,NULL);
	}

	if(hg->obj[hg->i_max].name) g_free(hg->obj[hg->i_max].name);
	hg->obj[hg->i_max].name=g_strdup(tmp_name);

	if(hg->obj[hg->i_max].note) g_free(hg->obj[hg->i_max].note);
	if((tmp_char=(char *)strtok(NULL,"\n"))!=NULL){
	  hg->obj[hg->i_max].note=g_strdup(tmp_char);
	  hg->obj[hg->i_max].note=cut_spc(tmp_char);
	}
	else{
	  hg->obj[hg->i_max].note=NULL;
	}
	
	hg->i_max++;
      }
      if(tmp_name) g_free(tmp_name);
    }
    if(buf) g_free(buf);
  }

  fclose(fp);

  calc_rst(hg);
}

//////////////////// OPE File /////////////////////
void ReadListOPE(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  gchar *BUF=NULL,*buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gboolean new_fmt_flag=FALSE;
  
  //hg->flag_bunnei=FALSE;

  if((fp=fopen(hg->filename_read,"rb"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"<PARAMETER_LIST>",
			     strlen("<PARAMETER_LIST>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":PARAMETER",
			     strlen(":PARAMETER"))==0){
	escape=TRUE;
	new_fmt_flag=TRUE;
      }
      g_free(buf);
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  
  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
				 strlen("</PARAMETER_LIST>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else{
	if((buf[0]!='#')){
	  if(BUF) g_free(BUF);
	  BUF=g_ascii_strup(buf,-1);
	  ok_obj=FALSE;
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	  ok_equinox=FALSE;
	
	  // OBJECT
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "OBJECT="))){
	      cpp=cp+strlen("OBJECT=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		cp++;
		ok_obj=TRUE;
		cp+=strlen("OBJECT=");
		if(cp[0]=='\"'){
		  cp+=1;
		  cp2 = strstr(cp, "\"");
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		}
		else{
		  //if(cp3) g_free(cp3);
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  if(NULL != (cp2 = strstr(cp, " ")))
		      hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else hg->obj[i_list].name=g_strdup(cp);
		}
		break;
	      }
	    }
	  }while(cp);

	    
	  // RA
	  if(ok_obj){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "RA="))){
		cpp=cp+strlen("RA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_ra=TRUE;
		  cp+=strlen("RA=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  hg->obj[i_list].ra=(gdouble)g_strtod(cp3,NULL);
		  hg->obj[i_list].pm_ra=0.0;
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  // DEC
	  if(ok_obj&&ok_ra){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "DEC="))){
		cpp=cp+strlen("DEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_dec=TRUE;
		  cp+=strlen("DEC=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  hg->obj[i_list].dec=(gdouble)g_strtod(cp3,NULL);
		  hg->obj[i_list].pm_dec=0.0;
		  break;
		}
	      }
	    }while(cp);
	  }

	  // EQUINOX
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "EQUINOX="))){
		cpp=cp+strlen("EQUINOX=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_equinox=TRUE;
		  cp+=strlen("EQUINOX=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  hg->obj[i_list].equinox=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	
	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    init_obj(&hg->obj[i_list], hg);

	    if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
	    hg->obj[i_list].note=NULL;
	  
	    i_list++;
	  }
	}
      }
      if(buf) g_free(buf);
    }

    if(escape) break;
  }


  fclose(fp);

  hg->i_max=i_list;
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}


#ifdef USE_SSL
void UploadOPE(typHOE *hg){
  gint ans=0;

  if((ans=scp_write(hg))<0){
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



///////////////////////////////////////////////////////////////////
//////////   Save Files
///////////////////////////////////////////////////////////////////

//////////   OPE save
void do_save_base_ope(GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  gint i_ret;
  gchar *tmp;

  hg=(typHOE *)gdata;
  
  if(CheckDefDup(hg)){
    return;
  }

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else if(hg->inst==INST_IRCS){
    i_ret=IRCS_check_gs(hg);
    if(i_ret>=0){
      tmp=g_strdup_printf("Guide star is not selected for [Obj-%d] %s.",
			  i_ret+1, hg->obj[i_ret].name);

      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    tmp,
		    NULL);
      g_free(tmp);
      return;
    }
  }

  flagChildDialog=TRUE;

  SaveFile(hg, SAVE_FILE_BASE_OPE);

  flagChildDialog=FALSE;
}


void do_save_plan_ope(GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(CheckDefDup(hg)){
    return;
  }

  switch(hg->inst){
  case INST_HDS:
    plan_check_consistency(hg);
    break;
  }
  
  SaveFile(hg, SAVE_FILE_PLAN_OPE);
}

//////////   HOE save
void do_save_hoe (GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }

  flagChildDialog=TRUE;

  SaveFile(hg, SAVE_FILE_HOE);

  flagChildDialog=FALSE;
}


//////////   PDF save
void do_save_plot_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_PDF_PLOT);
}

void do_save_skymon_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_PDF_SKYMON);
}
 

void do_save_efs_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_PDF_EFS);
}

void do_save_fc_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_PDF_FC);
}


void do_save_fc_pdf_all (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_PDF_FC_ALL);
}


//////////// Text files
void do_save_plan_txt (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->inst){
  case INST_HDS:
    plan_check_consistency(hg);
    break;
  }

  SaveFile(hg, SAVE_FILE_PLAN_TXT);
}

void do_save_proms_txt (GtkWidget *widget, gpointer gdata)
{
  gint i_list;
  typHOE *hg;
  enum{ERROR_NO, ERROR_MAG, ERROR_EQ};
  gint flag_exit=ERROR_NO;

  hg=(typHOE *)gdata;

  // Precheck
  if(hg->i_max==0) flag_exit=TRUE;
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(fabs(hg->obj[i_list].equinox-2000.0)>0.01) flag_exit=ERROR_EQ;
    if(fabs(hg->obj[i_list].mag)>99) flag_exit=ERROR_MAG;
  }

  switch(flag_exit){
  case ERROR_EQ:
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*3,
		  "Error: Please use Equinox J2000.0 for your target coordinates.",
		  NULL);
    return;
    break;

  case ERROR_MAG:
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*3,
		  "Error: Please set Mags for your targets.",
		  NULL);
    return;
    break;
  }

  SaveFile(hg, SAVE_FILE_PROMS_TXT);
}


void do_save_service_txt (GtkWidget *widget, gpointer gdata)
{
  gboolean flag_exit=FALSE;
  gint i_list;
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->inst){
  case INST_HDS:
    // Precheck for HDS
    if(hg->i_max==0) flag_exit=TRUE;
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(fabs(hg->obj[i_list].mag)>99) flag_exit=TRUE;
      if(fabs(hg->obj[i_list].snr)<0) flag_exit=TRUE;
    }
    
    if(flag_exit){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*3,
		    "Error: Please Set Mags for your objects,",
		    "          and Check S/N ratios using \"Update/Calc S/N by ETC\".",
		    NULL);
      return;
    }
    break;
  }

  SaveFile(hg, SAVE_FILE_SERVICE_TXT);
}


////////////////// CSV
void do_save_fcdb_csv (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  SaveFile(hg, SAVE_FILE_FCDB_CSV);
}


void do_save_trdb_csv (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->i_max<=0) return;

  SaveFile(hg, SAVE_FILE_TRDB_CSV);
}


////////////////// YAML
void do_save_plan_yaml (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  switch(hg->inst){
  case INST_HDS:
    plan_check_consistency(hg);
    break;
  }

  SaveFile(hg, SAVE_FILE_PLAN_YAML);
}


////////////////// HDS Obs Log from sumda
void do_download_log (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(!CheckInst(hg, INST_HDS)) return;

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  SaveFile(hg, SAVE_FILE_DOWNLOAD_LOG);
  
  flagChildDialog=FALSE;
}



void SaveFile(typHOE *hg, guint mode)
{
  GtkWidget *fdialog;
  gchar *tmp;
  gchar **tgt_file;
  gchar *cpp, *basename0, *basename1;

  switch(mode){
  case SAVE_FILE_BASE_OPE:
  case SAVE_FILE_PLAN_OPE:
    tmp=g_strdup("HOE : Input OPE File to be Saved");
    tgt_file=&hg->filename_write;
    break;

  case SAVE_FILE_HOE:
    tmp=g_strdup("HOE : Input HOE Config File to be Saved");
    tgt_file=&hg->filename_hoe;
    break;
    
  case SAVE_FILE_PDF_PLOT:	
  case SAVE_FILE_PDF_SKYMON:
  case SAVE_FILE_PDF_EFS:	
  case SAVE_FILE_PDF_FC:	    
  case SAVE_FILE_PDF_FC_ALL:
    tmp=g_strdup("HOE : Input PDF File to be Saved");
    tgt_file=&hg->filename_pdf;
    break;

  case SAVE_FILE_PLAN_TXT:
  case SAVE_FILE_PROMS_TXT:
  case SAVE_FILE_SERVICE_TXT:
  case SAVE_FILE_IRCS_LGS_TXT:
    tmp=g_strdup("HOE : Input Text File to be Saved");
    tgt_file=&hg->filename_txt;
    break;

  case SAVE_FILE_FCDB_CSV:
    tmp=g_strdup("HOE : CSV File to be Saved (FCDB)");
    tgt_file=&hg->filename_fcdb;
    break;

  case SAVE_FILE_TRDB_CSV:
    tmp=g_strdup("HOE : CSV File to be Saved (List Query)");
    tgt_file=&hg->filename_trdb;
    break;
    
  case SAVE_FILE_PLAN_YAML:
    tmp=g_strdup("HOE : Input YAML File to be Saved");
    tgt_file=&hg->filename_txt;
    break;

  case SAVE_FILE_CONV_JPL:
    tmp=g_strdup("HOE : Input TSC Tracking File to be saved");
    tgt_file=&hg->filename_tscconv;
    break;

  case SAVE_FILE_DOWNLOAD_LOG:
    tmp=g_strdup("HOE : Input Log File to be Saved");
    tgt_file=&hg->filename_log;
    break;
  }

  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW((flagPlan) ? hg->plan_main :hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  switch(mode){
  case SAVE_FILE_BASE_OPE:
    if(!*tgt_file){
      if(hg->filehead){
	*tgt_file=g_strconcat(hg->filehead,"." OPE_EXTENSION,NULL);
      }
    }
    break;
    
  case SAVE_FILE_PLAN_OPE:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." OPE_EXTENSION,NULL);
    }
    break;
	
  case SAVE_FILE_HOE:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." HOE_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_PDF_PLOT:	
  case SAVE_FILE_PDF_SKYMON:
  case SAVE_FILE_PDF_EFS:	
  case SAVE_FILE_PDF_FC:	    
  case SAVE_FILE_PDF_FC_ALL:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_PLAN_TXT:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." PLAN_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_PROMS_TXT:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." PROMS_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_SERVICE_TXT:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." SERVICE_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_IRCS_LGS_TXT:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." LGS_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_FCDB_CSV:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=fcdb_csv_name(hg);
    break;

  case SAVE_FILE_TRDB_CSV:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=trdb_csv_name(hg, CSV_EXTENSION);
    break;

  case SAVE_FILE_PLAN_YAML:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(hg->filehead,"." YAML_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_DOWNLOAD_LOG:
    if(*tgt_file) g_free(*tgt_file);
    if(hg->filename_write){
      *tgt_file=g_strdup_printf("%s%shdslog-%04d%02d%02d.txt",
				to_utf8(g_path_get_dirname(hg->filename_write)),
				G_DIR_SEPARATOR_S,
				hg->fr_year,hg->fr_month,hg->fr_day);
    }
    else{
      *tgt_file=g_strdup_printf("%s%shdslog-%04d%02d%02d.txt",
				hg->home_dir,
				G_DIR_SEPARATOR_S,
				hg->fr_year,hg->fr_month,hg->fr_day);
    }
    break;
  }
  
  
  if(mode==SAVE_FILE_CONV_JPL){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_jpl)));
    basename0=g_path_get_basename(hg->filename_jpl);
    cpp=(gchar *)strtok(basename0,".");
    basename1=g_strconcat(cpp,".",NST2_EXTENSION,NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(basename1));
    if(basename0) g_free(basename0);
    if(basename1) g_free(basename1);
  }
  else{
    if(access(*tgt_file,F_OK)==0){
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(*tgt_file));
      gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
					to_utf8(*tgt_file));
    }
    else if(*tgt_file){
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					   to_utf8(g_path_get_dirname(*tgt_file)));
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_basename(*tgt_file)));
    }
  }

  switch(mode){
  case SAVE_FILE_BASE_OPE:
  case SAVE_FILE_PLAN_OPE:
    my_file_chooser_add_filter(fdialog,"OPE File",
			       "*." OPE_EXTENSION,NULL);
    break;

  case SAVE_FILE_HOE:
    my_file_chooser_add_filter(fdialog,"HOE Config File",
			       "*." HOE_EXTENSION,NULL);
    break;

  case SAVE_FILE_PDF_PLOT:	
  case SAVE_FILE_PDF_SKYMON:
  case SAVE_FILE_PDF_EFS:	
  case SAVE_FILE_PDF_FC:	    
  case SAVE_FILE_PDF_FC_ALL:
    my_file_chooser_add_filter(fdialog,"PDF File",
			       "*." PDF_EXTENSION,NULL);
    break;

  case SAVE_FILE_PLAN_TXT:
    my_file_chooser_add_filter(fdialog,"Plan Text File",
			       "*" PLAN_EXTENSION,NULL);
    break;

  case SAVE_FILE_PROMS_TXT:
    my_file_chooser_add_filter(fdialog,"PROMS Text File",
			       "*" PROMS_EXTENSION,NULL);
    break;

  case SAVE_FILE_SERVICE_TXT:
    my_file_chooser_add_filter(fdialog,"Service Text File",
			       "*" SERVICE_EXTENSION,NULL);
    break;

  case SAVE_FILE_IRCS_LGS_TXT:
    my_file_chooser_add_filter(fdialog,"LGS Text File",
			       "*" LGS_EXTENSION,NULL);
    break;
    
  case SAVE_FILE_FCDB_CSV:
  case SAVE_FILE_TRDB_CSV:
    my_file_chooser_add_filter(fdialog,"CSV File",
			       "*." CSV_EXTENSION,NULL);
    break;

  case SAVE_FILE_PLAN_YAML:
    my_file_chooser_add_filter(fdialog,"YAML File",
			       "*." YAML_EXTENSION,NULL);
    break;

  case SAVE_FILE_CONV_JPL:
    my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			       "*." NST1_EXTENSION,
			       "*." NST3_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    break;

  case SAVE_FILE_DOWNLOAD_LOG:
    my_file_chooser_add_filter(fdialog,"TXT File",
			       "*." LIST3_EXTENSION,NULL);
    break;
  }

  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);
    switch(mode){
    case SAVE_FILE_BASE_OPE:
    case SAVE_FILE_PLAN_OPE:
      dest_file=check_ext(hg->w_top, dest_file,OPE_EXTENSION);
      break;

    case SAVE_FILE_HOE:
      dest_file=check_ext(hg->w_top, dest_file,HOE_EXTENSION);
      break;

    case SAVE_FILE_PDF_PLOT:
    case SAVE_FILE_PDF_SKYMON:
    case SAVE_FILE_PDF_EFS:
    case SAVE_FILE_PDF_FC:
    case SAVE_FILE_PDF_FC_ALL:
      dest_file=check_ext(hg->w_top, dest_file,PDF_EXTENSION);
      break;

    case SAVE_FILE_PLAN_TXT:
      dest_file=check_ext(hg->w_top, dest_file,PLAN_EXTENSION);
      break;

    case SAVE_FILE_PROMS_TXT:
      dest_file=check_ext(hg->w_top, dest_file,PROMS_EXTENSION);
      break;

    case SAVE_FILE_SERVICE_TXT:
      dest_file=check_ext(hg->w_top, dest_file,SERVICE_EXTENSION);
      break;

    case SAVE_FILE_IRCS_LGS_TXT:
      dest_file=check_ext(hg->w_top, dest_file,LGS_EXTENSION);
      break;

    case SAVE_FILE_FCDB_CSV:
    case SAVE_FILE_TRDB_CSV:
      dest_file=check_ext(hg->w_top, dest_file,CSV_EXTENSION);
      break;

    case SAVE_FILE_PLAN_YAML:
      dest_file=check_ext(hg->w_top, dest_file,YAML_EXTENSION);
      break;
    }

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(*tgt_file) g_free(*tgt_file);
	*tgt_file=g_strdup(dest_file);
	
	switch(mode){
	case SAVE_FILE_BASE_OPE:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  switch(hg->inst){
	  case INST_HDS:
	    HDS_WriteOPE(hg, FALSE);
	    break;
	    
	  case INST_IRCS:
	    IRCS_WriteOPE(hg, FALSE);
	    break;
	  }
	  break;

	case SAVE_FILE_PLAN_OPE:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  switch(hg->inst){
	  case INST_HDS:
	    HDS_WriteOPE(hg, TRUE);
	    break;
	    
	  case INST_IRCS:
	    IRCS_WriteOPE(hg, TRUE);
	    break;
	  }
	  break;

	case SAVE_FILE_HOE:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  WriteHOE(hg);
	  if((hg->prop_id)&&(hg->prop_pass)){
	    WritePass(hg);
	  }
	  break;

	case SAVE_FILE_PDF_PLOT:	
	  pdf_plot(hg);
	  break;
	  
	case SAVE_FILE_PDF_SKYMON:
	  pdf_skymon(hg);
	  break;

	case SAVE_FILE_PDF_EFS:
	  pdf_efs(hg);
	  break;
	  
	case SAVE_FILE_PDF_FC:	    
	  pdf_fc(hg);
	  break;
	  
	case SAVE_FILE_PDF_FC_ALL:
	  create_fc_all_dialog(hg);
	  break;

	case SAVE_FILE_PLAN_TXT:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  WritePlan(hg);
	  break;
	  
	case SAVE_FILE_PROMS_TXT:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  WritePROMS(hg);
	  break;

	case SAVE_FILE_SERVICE_TXT:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  switch(hg->inst){
	  case INST_HDS:
	    HDS_WriteService(hg);
	    break;
	    
	  case INST_IRCS:
	    IRCS_WriteService(hg);
	    break;
	  }
	  break;

	case SAVE_FILE_IRCS_LGS_TXT:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  IRCS_WriteLGS(hg);
	  break;

	case SAVE_FILE_FCDB_CSV:
	  Export_FCDB_CSV(hg);
	  break;

	case SAVE_FILE_TRDB_CSV:	
	  Export_TRDB_CSV(hg);
	  break;

	case SAVE_FILE_PLAN_YAML:
	  if(hg->filehead) g_free(hg->filehead);
	  hg->filehead=make_head(dest_file);
	  WriteYAML(hg);
	  break;

	case SAVE_FILE_CONV_JPL:
	  ConvJPL(hg);
	  break;

	case SAVE_FILE_DOWNLOAD_LOG:
	  HDS_DownloadLOG(hg);
	  break;
	}
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }

    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}



///////////////////////////////////////////////////////////////////
//////////   Non-Sidereal Tracking
///////////////////////////////////////////////////////////////////

void do_open_NST (GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  OpenFile(hg, OPEN_FILE_READ_NST);
  
  flagChildDialog=FALSE;
}

void do_open_JPL (GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  OpenFile(hg, OPEN_FILE_READ_JPL);

  flagChildDialog=FALSE;
}

void do_conv_JPL (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->w_top)){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  OpenFile(hg, OPEN_FILE_CONV_JPL);

  flagChildDialog=FALSE;
}


gboolean MergeNST(typHOE *hg){
  FILE *fp;
  gint i,i_list=0,i_base,i_use;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gdouble date_tmp, ra_geoc, dec_geoc;
  gchar *cp, *cpp, *tmp_name, *cut_name;
  struct ln_zonedate zonedate, zonedate1;
  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return(FALSE);
  }
  

  if((fp=fopen(hg->filename_nst,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_nst,
		  NULL);
    return(FALSE);
  }

  i_list=hg->i_max;
  if(hg->i_max==0){
    hg->nst_max=0;
  }

  for(i=0;i<6;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: TSC File format might be incorrect.",
		    " ",
		    hg->filename_nst,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else{
      if(i==0){
	cpp=buf;
	cpp++;
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      if(i<5){
	if(buf) g_free(buf);
      }
    }
  }
  hg->nst[hg->nst_max].i_max=(gint)g_strtod(buf,NULL);
  if(buf) g_free(buf);
  if(hg->nst[hg->nst_max].i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: TSC File format might be incorrect.",
		  " ",
		  hg->filename_nst,
		  NULL);
    fclose(fp);
    return(FALSE);
  }

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);
  
  i=0;
  while((!feof(fp))||(i<hg->nst_max)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      sscanf(buf,"%lf %lf %lf %lf %lf",
	     &date_tmp,
	     &ra_geoc,
	     &dec_geoc,
	     &hg->nst[hg->nst_max].eph[i].geo_d,
	     &hg->nst[hg->nst_max].eph[i].equinox);
      
      hg->nst[hg->nst_max].eph[i].jd=date_to_jd(date_tmp);
      // GeoCentric --> TopoCentric
      equ_geoc.ra=ra_to_deg(ra_geoc);
      equ_geoc.dec=dec_to_deg(dec_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i].jd,
			hg->nst[hg->nst_max].eph[i].geo_d,&equ_geoc,&equ);
      hg->nst[hg->nst_max].eph[i].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i].dec=deg_to_dec(equ.dec);
      i++;
      if(buf) g_free(buf);
    }
  }
  
  if(i!=hg->nst[hg->nst_max].i_max){
    fprintf(stderr,"[MergeNST] Inconsistent Line Number in  \"%s\", %d <--> %d.", hg->filename_nst,hg->nst[hg->nst_max].i_max,i);
  }

  fclose(fp);

  if(i>0){
    init_obj(&hg->obj[i_list], hg);

    ln_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		      hg->obs_timezone/60);
    ln_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		      &zonedate1, 
		      hg->obs_timezone/60);
    if(tmp_name){
      cut_name=cut_spc(tmp_name);
      g_free(tmp_name);
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(cut_name);
      g_free(cut_name);
    }
    else{
      hg->obj[i_list].name=g_strdup("(None-Sidereal)");
    }
    hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
    hg->obj[i_list].pm_ra=0.0;
    hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
    hg->obj[i_list].pm_dec=0.0;
    hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
    if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
    hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
					 g_path_get_basename(hg->filename_nst),
					 zonedate.years,
					 zonedate.months,
					 zonedate.days,
					 zonedate.hours,
					 zonedate.minutes,
					 zonedate1.months,
					 zonedate1.days,
					 zonedate1.hours,
					 zonedate1.minutes,
					 "HST");
    if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
    hg->nst[hg->nst_max].filename=g_strdup(hg->filename_nst);
    hg->nst[hg->nst_max].type=NST_TYPE_TSC;

    hg->obj[i_list].i_nst=hg->nst_max;
    hg->obj[i_list].guide=NO_GUIDE;

    hg->i_max++;
    hg->nst_max++;
  }

  calc_rst(hg);

  return(TRUE);
}


gboolean MergeJPL(typHOE *hg){
  FILE *fp;
  gint i,i_list, i_line, i_soe=0, i_eoe=0, i_use;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_zonedate zonedate, zonedate1;
  gchar *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;

  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return(FALSE);
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }

  i_list=hg->i_max;
  if(hg->i_max==0){
    hg->nst_max=0;
  }

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->w_top, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT*2,
			"Error: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  return(FALSE);
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }


  if(i_soe>=i_eoe){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }

  hg->nst[hg->nst_max].i_max=i_eoe-i_soe-1;

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = strstr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
  }

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      zonedate.gmtoff=0;

      if(strlen(tmp1)!=4){
	// JD
	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=(gdouble)g_strtod(tmp1, NULL);
      }
      else{
	zonedate.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	zonedate.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	zonedate.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  zonedate.seconds=0.0;
	}
	else{
	  zonedate.seconds=(gdouble)g_strtod(tmp1, NULL);
	}

	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=
	  ln_get_julian_local_date(&zonedate);
      }
      g_free(tmp);


      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      ln_hequ_to_equ (&hequ, &equ_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i-i_soe-1].jd,
			hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d,
			&equ_geoc,
			&equ);
      hg->nst[hg->nst_max].eph[i-i_soe-1].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i-i_soe-1].dec=deg_to_dec(equ.dec);
      hg->nst[hg->nst_max].eph[i-i_soe-1].equinox=2000.0;

      if(buf) g_free(buf);
    }
  }
  
  fclose(fp);

  init_obj(&hg->obj[i_list], hg);

  ln_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		    hg->obs_timezone/60);
  ln_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		    &zonedate1, 
		    hg->obs_timezone/60);
  
  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
    hg->obj[i_list].name=g_strdup(cut_name);
    g_free(cut_name);
  }
  else{
    hg->obj[i_list].name=g_strdup("(None-Sidereal)");
  }
  hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
  hg->obj[i_list].pm_ra=0.0;
  hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
  hg->obj[i_list].pm_dec=0.0;
  hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
  if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
  hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
				       g_path_get_basename(hg->filename_jpl),
				       zonedate.years,
				       zonedate.months,
				       zonedate.days,
				       zonedate.hours,
				       zonedate.minutes,
				       zonedate1.months,
				       zonedate1.days,
				       zonedate1.hours,
				       zonedate1.minutes,
				       "HST");

  if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
  hg->nst[hg->nst_max].filename=g_strdup(hg->filename_jpl);
  hg->nst[hg->nst_max].type=NST_TYPE_JPL;

  hg->obj[i_list].i_nst=hg->nst_max;
  hg->obj[i_list].guide=NO_GUIDE;

  hg->i_max++;
  hg->nst_max++;

  calc_rst(hg);

  return(TRUE);
}

////////////  JPL ---> TSC format conversion
void ConvJPL(typHOE *hg){
  FILE *fp, *fp_w;
  gint i,i_list, i_line, i_soe=0, i_eoe=0, i_max;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_date date;
  char *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gdouble JD, geo_d;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;
  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return;
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  if((fp_w=fopen(hg->filename_tscconv,"wb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->w_top, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT*2,
			"Error: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  return;
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }


  if(i_soe>=i_eoe){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  i_max=i_eoe-i_soe-1;

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = strstr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
  }

  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    fprintf(fp_w,"#%s\n",cut_name);
    g_free(cut_name);
  }
  else{
    fprintf(fp_w,"#(Non-Sidereal File converted from JPL HORIZONS\n");
  }

  fprintf(fp_w,"+00.0000 +00.0000 ON%% +0.000\n");
  fprintf(fp_w,"UTC Geocentric Equatorial Mean Polar Geocentric\n");
  fprintf(fp_w,"ABS\n");
  fprintf(fp_w,"TSC\n");
  fprintf(fp_w,"%d\n",i_max);

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      if(strlen(tmp1)!=4){
	// JD
	JD=(gdouble)g_strtod(tmp1, NULL);
	ln_get_date(JD,&date);
      }
      else{
	date.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	date.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	date.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  date.seconds=0.0;
	}
	else{
	  date.seconds=(gdouble)g_strtod(tmp1, NULL);
	}
      }
      g_free(tmp);

      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      fprintf(fp_w,"%4d%02d%02d%02d%02d%06.3lf %02d%02d%06.3lf %s%02d%02d%05.2lf %13.9lf 2000.0000\n",
	      date.years,
	      date.months,
	      date.days,
	      date.hours,
	      date.minutes,
	      date.seconds,
	      hequ.ra.hours,
	      hequ.ra.minutes,
	      hequ.ra.seconds,
	      (hequ.dec.neg == 1) ? "-" : "+",
	      hequ.dec.degrees,
	      hequ.dec.minutes,
	      hequ.dec.seconds,
	      geo_d);

      if(buf) g_free(buf);
    }

  }
  
  fclose(fp);
  fclose(fp_w);
}


///////////////////////////////////////////////////////////////////
//////////   core procedure of Read/Write HOE file
///////////////////////////////////////////////////////////////////

void WriteHOE(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *filename;
  gchar tmp[64],f_tmp[64], bname[128];
  int i_nonstd, i_set, i_list, i_line, i_plan, i_band;


  //filename = g_strconcat(g_get_home_dir(), "/save.hoe", NULL);
  filename = g_strdup(hg->filename_hoe);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // General 
  xmms_cfg_write_string(cfgfile, "General", "prog_ver",VERSION);
  xmms_cfg_write_string(cfgfile, "General", "major_ver",MAJOR_VERSION);
  xmms_cfg_write_string(cfgfile, "General", "minor_ver",MINOR_VERSION);
  xmms_cfg_write_string(cfgfile, "General", "micro_ver",MICRO_VERSION);
  if(hg->filename_write) xmms_cfg_write_string(cfgfile, "General", "OPE", hg->filename_write);
  if(hg->filename_read)  xmms_cfg_write_string(cfgfile, "General", "List",hg->filename_read);
  //xmms_cfg_write_boolean(cfgfile, "General", "PSFlag",hg->flag_bunnei);
  //xmms_cfg_write_boolean(cfgfile, "General", "SecZFlag",hg->flag_secz);
  //xmms_cfg_write_double(cfgfile, "General", "SecZFactor",hg->secz_factor);

  // Header
  xmms_cfg_write_int(cfgfile, "Header", "FromYear",hg->fr_year);
  xmms_cfg_write_int(cfgfile, "Header", "FromMonth",hg->fr_month);
  xmms_cfg_write_int(cfgfile, "Header", "FromDay",hg->fr_day);
  xmms_cfg_write_string(cfgfile, "Header", "ID",hg->prop_id);
  //if(hg->prop_pass)
  //  xmms_cfg_write_string(cfgfile, "Header", "Pass",hg->prop_pass);
  xmms_cfg_remove_key(cfgfile, "Header", "Pass");
  if(hg->observer)
    xmms_cfg_write_string(cfgfile, "Header", "Observer",hg->observer);


  // Default Parameter
  xmms_cfg_write_int(cfgfile, "DefPara", "Guide",hg->def_guide);
  xmms_cfg_write_double2(cfgfile, "DefPara", "PA",hg->def_pa, "%+6.2f");
  xmms_cfg_write_int(cfgfile, "DefPara", "ExpTime",(gint)hg->def_exp);



  // AD Calc.
  xmms_cfg_write_int(cfgfile, "ADC", "Wave1",(gint)hg->wave1);
  xmms_cfg_write_int(cfgfile, "ADC", "Wave0",(gint)hg->wave0);
  xmms_cfg_write_int(cfgfile, "ADC", "Pres",(gint)hg->pres);
  xmms_cfg_write_int(cfgfile, "ADC", "Temp",(gint)hg->temp);

  // Instrument
  xmms_cfg_write_int(cfgfile, "Inst", "Inst",  hg->inst);

  // AG
  xmms_cfg_write_int(cfgfile, "AG", "ExptimeFactor",(gint)hg->exptime_factor);
  xmms_cfg_write_int(cfgfile, "AG", "Brightness",(gint)hg->brightness);

  
  // SV
  xmms_cfg_write_int(cfgfile, "SV", "Area",(gint)hg->sv_area);
  xmms_cfg_write_int(cfgfile, "SV", "Integrate",(gint)hg->sv_integrate);
  xmms_cfg_write_int(cfgfile, "SV", "Region",(gint)hg->sv_region);
  xmms_cfg_write_int(cfgfile, "SV", "Calc",(gint)hg->sv_calc);
  xmms_cfg_write_int(cfgfile, "SV", "Exptime",(gint)hg->exptime_sv);
  xmms_cfg_write_double2(cfgfile, "SV", "SlitX",hg->sv_slitx, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "SlitY",hg->sv_slity, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "ISX",hg->sv_isx, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "ISY",hg->sv_isy, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "IS3X",hg->sv_is3x, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "IS3Y",hg->sv_is3y, "%5.1f");

  // Overhead
  xmms_cfg_write_int(cfgfile, "Overhead", "Acq", (gint)hg->oh_acq);
  xmms_cfg_write_int(cfgfile, "Duration", "NGS1",(gint)hg->oh_ngs1);
  xmms_cfg_write_int(cfgfile, "Duration", "NGS2",(gint)hg->oh_ngs2);
  xmms_cfg_write_int(cfgfile, "Duration", "NGS3",(gint)hg->oh_ngs3);
  xmms_cfg_write_int(cfgfile, "Duration", "LGS", (gint)hg->oh_lgs);

  // CameraZ
  xmms_cfg_write_int(cfgfile, "CameraZ", "Blue",(gint)hg->camz_b);
  xmms_cfg_write_int(cfgfile, "CameraZ", "Red",(gint)hg->camz_r);

  // Cross
  xmms_cfg_write_int(cfgfile, "Cross", "dCross",(gint)hg->d_cross);

  // HDS NonStd
  for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
    sprintf(tmp,"NonStd-%d",i_nonstd+1);
    xmms_cfg_write_int(cfgfile, tmp, "Color",(gint)hg->nonstd[i_nonstd].col);
    xmms_cfg_write_int(cfgfile, tmp, "Cross",(gint)hg->nonstd[i_nonstd].cross);
    xmms_cfg_write_int(cfgfile, tmp, "Echelle",hg->nonstd[i_nonstd].echelle);
    xmms_cfg_write_int(cfgfile, tmp, "CamRot",hg->nonstd[i_nonstd].camr);
  }

  // HDS Setup
  for(i_set=0;i_set<MAX_USESETUP;i_set++){
    sprintf(tmp,"SetUp-%d",i_set+1);
    xmms_cfg_write_int(cfgfile, tmp, "Setup",hg->setup[i_set].setup);
    xmms_cfg_write_boolean(cfgfile, tmp, "Use",hg->setup[i_set].use);
    xmms_cfg_write_int(cfgfile, tmp, "Binning",(gint)hg->setup[i_set].binning);
    xmms_cfg_write_int(cfgfile, tmp, "SlitWidth",(gint)hg->setup[i_set].slit_width);
    xmms_cfg_write_int(cfgfile, tmp, "SlitLength",(gint)hg->setup[i_set].slit_length);
    if(hg->setup[i_set].fil1)
      xmms_cfg_write_string(cfgfile, tmp, "Filter1",hg->setup[i_set].fil1);
    if(hg->setup[i_set].fil2)
      xmms_cfg_write_string(cfgfile, tmp, "Filter2",hg->setup[i_set].fil2);
    xmms_cfg_write_int(cfgfile, tmp, "ImR",hg->setup[i_set].imr);
    xmms_cfg_write_int(cfgfile, tmp, "IS",hg->setup[i_set].is);
    xmms_cfg_write_boolean(cfgfile, tmp, "I2",hg->setup[i_set].i2);
  }
  

  // IRCS Setup
  xmms_cfg_write_int(cfgfile, "IRCS", "Max",  hg->ircs_i_max);
  for(i_set=0;i_set<hg->ircs_i_max;i_set++){
    sprintf(tmp,"IRCS_SetUp-%02d",i_set+1);
    xmms_cfg_write_int(cfgfile, tmp, "Mode",  hg->ircs_set[i_set].mode);
    xmms_cfg_write_int(cfgfile, tmp, "Mas",   hg->ircs_set[i_set].mas);
    xmms_cfg_write_int(cfgfile, tmp, "Band",  hg->ircs_set[i_set].band);
    xmms_cfg_write_int(cfgfile, tmp, "Slit",  hg->ircs_set[i_set].slit);
    xmms_cfg_write_int(cfgfile, tmp, "Dith",  hg->ircs_set[i_set].dith);

    xmms_cfg_write_double2(cfgfile, tmp, "DithW",   hg->ircs_set[i_set].dithw,"%.2f");
    xmms_cfg_write_int(cfgfile, tmp, "OSRA",  hg->ircs_set[i_set].osra);
    xmms_cfg_write_int(cfgfile, tmp, "OSDec", hg->ircs_set[i_set].osdec);
    xmms_cfg_write_double2(cfgfile, tmp, "SSsep",   hg->ircs_set[i_set].sssep,"%.2f");
    xmms_cfg_write_int(cfgfile, tmp, "SSnum", hg->ircs_set[i_set].ssnum);
    xmms_cfg_write_int(cfgfile, tmp, "Shot", hg->ircs_set[i_set].shot);

    xmms_cfg_write_int(cfgfile, tmp, "Ech",    hg->ircs_set[i_set].ech);
    xmms_cfg_write_int(cfgfile, tmp, "XDS",    hg->ircs_set[i_set].xds);

    xmms_cfg_write_int(cfgfile, tmp, "CW1",    hg->ircs_set[i_set].cw1);
    xmms_cfg_write_int(cfgfile, tmp, "CW2",    hg->ircs_set[i_set].cw2);
    xmms_cfg_write_int(cfgfile, tmp, "CW3",    hg->ircs_set[i_set].cw3);
    xmms_cfg_write_int(cfgfile, tmp, "SLW",    hg->ircs_set[i_set].slw);
    xmms_cfg_write_int(cfgfile, tmp, "SPW",    hg->ircs_set[i_set].spw);
    xmms_cfg_write_int(cfgfile, tmp, "Cam",    hg->ircs_set[i_set].cam);

    xmms_cfg_write_double2(cfgfile, tmp, "SlitX",   hg->ircs_set[i_set].slit_x,"%.1f");
    xmms_cfg_write_double2(cfgfile, tmp, "SlitY",   hg->ircs_set[i_set].slit_y,"%.1f");
    
    xmms_cfg_write_boolean(cfgfile, tmp, "Std",  hg->ircs_set[i_set].std);

    xmms_cfg_write_double2(cfgfile, tmp, "Exp",   hg->ircs_set[i_set].exp,"%.3f");
  }
  

  // Object List
  for(i_list=0;i_list<hg->i_max;i_list++){
    sprintf(tmp,"Obj-%d",i_list+1);
    xmms_cfg_write_string(cfgfile, tmp, "Name",hg->obj[i_list].name); 
    xmms_cfg_write_boolean(cfgfile,tmp, "Std",hg->obj[i_list].std);
    xmms_cfg_write_int(cfgfile, tmp, "ExpTime",hg->obj[i_list].exp);
    xmms_cfg_write_int(cfgfile, tmp, "Repeat",hg->obj[i_list].repeat);
    xmms_cfg_write_double2(cfgfile, tmp, "RA",hg->obj[i_list].ra,"%9.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "PM_RA",hg->obj[i_list].pm_ra,"%+.4f");
    xmms_cfg_write_double2(cfgfile, tmp, "Dec",hg->obj[i_list].dec,"%+10.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "PM_Dec",hg->obj[i_list].pm_dec,"%+.4f");
    xmms_cfg_write_double2(cfgfile, tmp, "Epoch",hg->obj[i_list].equinox,"%7.2f");
    if(hg->obj[i_list].i_nst>=0){
      xmms_cfg_write_string(cfgfile, tmp, "NST_File",hg->nst[hg->obj[i_list].i_nst].filename); 
      xmms_cfg_write_int(cfgfile, tmp, "NST_Type",hg->nst[hg->obj[i_list].i_nst].type); 
    }
    else{
      xmms_cfg_remove_key(cfgfile,tmp, "NST_File");
      xmms_cfg_remove_key(cfgfile,tmp, "NST_Type");
    }
    xmms_cfg_write_double2(cfgfile, tmp, "Mag",hg->obj[i_list].mag,"%4.1f");
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_Used",hg->obj[i_list].magdb_used);
    xmms_cfg_write_string(cfgfile, tmp, "MagDB_UsedName",(gchar *)db_name[hg->obj[i_list].magdb_used]);
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_Band",hg->obj[i_list].magdb_band);
    xmms_cfg_write_double2(cfgfile, tmp, "PA",hg->obj[i_list].pa,"%+7.2f");
    xmms_cfg_write_int(cfgfile, tmp, "Guide",hg->obj[i_list].guide);
    xmms_cfg_write_int(cfgfile, tmp, "AOmode",hg->obj[i_list].aomode);
    xmms_cfg_write_boolean(cfgfile, tmp, "ADI",hg->obj[i_list].adi);
    if(hg->obj[i_list].note) xmms_cfg_write_string(cfgfile, tmp, "Note",hg->obj[i_list].note);
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_write_boolean(cfgfile, tmp, f_tmp,hg->obj[i_list].setup[i_set]);
    }
    if(hg->obj[i_list].gs.flag){
      xmms_cfg_write_string(cfgfile, tmp, "GS_Name",hg->obj[i_list].gs.name); 
      xmms_cfg_write_double2(cfgfile, tmp, "GS_RA",hg->obj[i_list].gs.ra,"%9.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Dec",hg->obj[i_list].gs.dec,"%+10.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Epoch",hg->obj[i_list].gs.equinox,"%7.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Sep",hg->obj[i_list].gs.sep,"%.6f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Mag",hg->obj[i_list].gs.mag,"%.2f");
      xmms_cfg_write_int(cfgfile, tmp, "GS_Src",hg->obj[i_list].gs.src);
    }
    else{
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Name");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_RA");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Dec");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Epoch");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Sep");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Mag");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Src");
    }
    
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_SIMBAD_Hits",hg->obj[i_list].magdb_simbad_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_Sep",hg->obj[i_list].magdb_simbad_sep,"%.6lf");
    if(hg->obj[i_list].magdb_simbad_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Name",hg->obj[i_list].magdb_simbad_name);
    if(hg->obj[i_list].magdb_simbad_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Type",hg->obj[i_list].magdb_simbad_type);
    if(hg->obj[i_list].magdb_simbad_sp)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Sp",hg->obj[i_list].magdb_simbad_sp);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_U",hg->obj[i_list].magdb_simbad_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_B",hg->obj[i_list].magdb_simbad_b,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_V",hg->obj[i_list].magdb_simbad_v,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_R",hg->obj[i_list].magdb_simbad_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_I",hg->obj[i_list].magdb_simbad_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_J",hg->obj[i_list].magdb_simbad_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_H",hg->obj[i_list].magdb_simbad_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_K",hg->obj[i_list].magdb_simbad_k,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_NED_Hits",   hg->obj[i_list].magdb_ned_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_NED_Sep",hg->obj[i_list].magdb_ned_sep,"%.6lf");
    if(hg->obj[i_list].magdb_ned_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Name",hg->obj[i_list].magdb_ned_name);
    if(hg->obj[i_list].magdb_ned_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Type",hg->obj[i_list].magdb_ned_type);
    if(hg->obj[i_list].magdb_ned_mag)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Mag", hg->obj[i_list].magdb_ned_mag);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_NED_Z",  hg->obj[i_list].magdb_ned_z,"%.6lf");
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_NED_Ref",   hg->obj[i_list].magdb_ned_ref);

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_LAMOST_Hits",   hg->obj[i_list].magdb_lamost_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_Sep",hg->obj[i_list].magdb_lamost_sep,"%.6lf");
    if(hg->obj[i_list].magdb_lamost_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Name",hg->obj[i_list].magdb_lamost_name);
    if(hg->obj[i_list].magdb_lamost_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Type",hg->obj[i_list].magdb_lamost_type);
    if(hg->obj[i_list].magdb_lamost_sp)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Sp", hg->obj[i_list].magdb_lamost_sp);
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_LAMOST_Ref",   hg->obj[i_list].magdb_lamost_ref);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_TEFF",  hg->obj[i_list].magdb_lamost_teff,"%.0lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_LOGG",  hg->obj[i_list].magdb_lamost_logg,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_FEH",  hg->obj[i_list].magdb_lamost_feh,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_HRV",  hg->obj[i_list].magdb_lamost_hrv,"%.1lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_Kepler_Hits",   hg->obj[i_list].magdb_kepler_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_Sep",hg->obj[i_list].magdb_kepler_sep,"%.6lf");
    if(hg->obj[i_list].magdb_kepler_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_Kepler_Name",hg->obj[i_list].magdb_kepler_name);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_K",   hg->obj[i_list].magdb_kepler_k,"%.3lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_R",   hg->obj[i_list].magdb_kepler_r,"%.3lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_J",   hg->obj[i_list].magdb_kepler_j,"%.3lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_kepler_TEFF",  hg->obj[i_list].magdb_kepler_teff,"%.0lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_LOGG",  hg->obj[i_list].magdb_kepler_logg,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_FEH",  hg->obj[i_list].magdb_kepler_feh,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_EBV",  hg->obj[i_list].magdb_kepler_ebv,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_RAD",  hg->obj[i_list].magdb_kepler_rad,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_PM",  hg->obj[i_list].magdb_kepler_pm,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_Kepler_GR",  hg->obj[i_list].magdb_kepler_gr,"%.4lf");
    if(hg->obj[i_list].magdb_kepler_2mass)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_Kepler_2MASS",hg->obj[i_list].magdb_kepler_2mass);

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_GSC_Hits",hg->obj[i_list].magdb_gsc_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_Sep",hg->obj[i_list].magdb_gsc_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_U",hg->obj[i_list].magdb_gsc_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_B",hg->obj[i_list].magdb_gsc_b,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_V",hg->obj[i_list].magdb_gsc_v,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_R",hg->obj[i_list].magdb_gsc_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_I",hg->obj[i_list].magdb_gsc_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_J",hg->obj[i_list].magdb_gsc_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_H",hg->obj[i_list].magdb_gsc_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_K",hg->obj[i_list].magdb_gsc_k,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_PS1_Hits",hg->obj[i_list].magdb_ps1_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Sep",hg->obj[i_list].magdb_ps1_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_G",hg->obj[i_list].magdb_ps1_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_R",hg->obj[i_list].magdb_ps1_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_I",hg->obj[i_list].magdb_ps1_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Z",hg->obj[i_list].magdb_ps1_z,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Y",hg->obj[i_list].magdb_ps1_y,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_SDSS_Hits",hg->obj[i_list].magdb_sdss_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_Sep",hg->obj[i_list].magdb_sdss_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_U",hg->obj[i_list].magdb_sdss_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_G",hg->obj[i_list].magdb_sdss_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_R",hg->obj[i_list].magdb_sdss_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_I",hg->obj[i_list].magdb_sdss_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_Z",hg->obj[i_list].magdb_sdss_z,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_GAIA_Hits",hg->obj[i_list].magdb_gaia_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_Sep",hg->obj[i_list].magdb_gaia_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_G",hg->obj[i_list].magdb_gaia_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_P",hg->obj[i_list].magdb_gaia_p,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_EP",hg->obj[i_list].magdb_gaia_ep,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_RP",hg->obj[i_list].magdb_gaia_rp,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_BP",hg->obj[i_list].magdb_gaia_bp,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_RV",hg->obj[i_list].magdb_gaia_rv,"%.1lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_TEFF",  hg->obj[i_list].magdb_gaia_teff,"%.0lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_AG",hg->obj[i_list].magdb_gaia_ag,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_EBR",hg->obj[i_list].magdb_gaia_ebr,"%.4lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_DIST",hg->obj[i_list].magdb_gaia_dist,"%.4lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_2MASS_Hits",hg->obj[i_list].magdb_2mass_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_Sep",hg->obj[i_list].magdb_2mass_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_J",hg->obj[i_list].magdb_2mass_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_H",hg->obj[i_list].magdb_2mass_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_K",hg->obj[i_list].magdb_2mass_k,"%.2lf");

    xmms_cfg_write_int (cfgfile, tmp, "BandMax",  
			hg->obj[i_list].trdb_band_max);

    // Band
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      sprintf(bname, "Object-%d_Band-%d",i_list,i_band);
      if(hg->obj[i_list].trdb_mode[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Mode", 
			       hg->obj[i_list].trdb_mode[i_band]);
      if(hg->obj[i_list].trdb_band[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Band", 
			       hg->obj[i_list].trdb_band[i_band]);
      xmms_cfg_write_double2(cfgfile, bname, "Exp",
			     hg->obj[i_list].trdb_exp[i_band], "%.2f");
      xmms_cfg_write_int(cfgfile, bname, "Shot",
			 hg->obj[i_list].trdb_shot[i_band]);
    }
  }
  for(i_list=hg->i_max;i_list<MAX_OBJECT;i_list++){
    sprintf(tmp,"Obj-%d",i_list+1);
    xmms_cfg_remove_key(cfgfile,tmp, "Name");
    xmms_cfg_remove_key(cfgfile,tmp, "Std");
    xmms_cfg_remove_key(cfgfile,tmp, "TGT");
    xmms_cfg_remove_key(cfgfile,tmp, "ExpTime");
    xmms_cfg_remove_key(cfgfile,tmp, "Repeat");
    xmms_cfg_remove_key(cfgfile,tmp, "RA");
    xmms_cfg_remove_key(cfgfile,tmp, "Dec");
    xmms_cfg_remove_key(cfgfile,tmp, "Epoch");
    xmms_cfg_remove_key(cfgfile,tmp, "PA");  
    xmms_cfg_remove_key(cfgfile,tmp, "Mag");  
    xmms_cfg_remove_key(cfgfile,tmp, "MagDB_Used");  
    xmms_cfg_remove_key(cfgfile,tmp, "MagDB_UsedName");  
    xmms_cfg_remove_key(cfgfile,tmp, "MagDB_Band");  
    xmms_cfg_remove_key(cfgfile,tmp, "Guide");
    xmms_cfg_remove_key(cfgfile,tmp, "AOmode");
    xmms_cfg_remove_key(cfgfile,tmp, "ADI");
    xmms_cfg_remove_key(cfgfile,tmp, "Note");
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_remove_key(cfgfile,tmp, f_tmp);
    }
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Name");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_RA");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Dec");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Epoch");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Sep");
  }


  // TRDB
  xmms_cfg_write_int(cfgfile, "TRDB", "Mode", hg->trdb_da);
  xmms_cfg_write_int(cfgfile, "TRDB", "Arcmin", hg->trdb_arcmin_used);

  // SMOKA
  xmms_cfg_write_int(cfgfile, "SMOKA", "Inst", hg->trdb_smoka_inst_used);
  if(hg->trdb_smoka_date_used)
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date_used);
  else
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Shot", hg->trdb_smoka_shot_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Imag", hg->trdb_smoka_imag_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Spec", hg->trdb_smoka_spec_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Ipol", hg->trdb_smoka_ipol_used);

  // HST
  xmms_cfg_write_int(cfgfile, "HST", "Mode", hg->trdb_hst_mode_used);
  if(hg->trdb_hst_date_used)
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date_used);
  else
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date);
  xmms_cfg_write_int(cfgfile, "HST", "Image", hg->trdb_hst_image_used);
  xmms_cfg_write_int(cfgfile, "HST", "Spec", hg->trdb_hst_spec_used);
  xmms_cfg_write_int(cfgfile, "HST", "Other", hg->trdb_hst_other_used);

  // ESO
  xmms_cfg_write_int(cfgfile, "ESO", "Mode", hg->trdb_eso_mode_used);
  if(hg->trdb_eso_stdate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate);
  if(hg->trdb_eso_eddate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate);
  xmms_cfg_write_int(cfgfile, "ESO", "Image", hg->trdb_eso_image_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Spec", hg->trdb_eso_spec_used);
  xmms_cfg_write_int(cfgfile, "ESO", "VLTI", hg->trdb_eso_vlti_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Pola", hg->trdb_eso_pola_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Coro", hg->trdb_eso_coro_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Other", hg->trdb_eso_other_used);
  xmms_cfg_write_int(cfgfile, "ESO", "SAM", hg->trdb_eso_sam_used);

  // Gemini
  xmms_cfg_write_int(cfgfile, "Gemini", "Inst", hg->trdb_gemini_inst_used);
  xmms_cfg_write_int(cfgfile, "Gemini", "Mode", hg->trdb_gemini_mode_used);
  if(hg->trdb_gemini_date_used)
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date_used);
  else
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date);


  // Line List
  for(i_line=0;i_line<MAX_LINE;i_line++){
    if(hg->line[i_line].name){
      sprintf(tmp,"Line-%d",i_line+1);
      xmms_cfg_write_string(cfgfile, tmp, "Name",hg->line[i_line].name);
      xmms_cfg_write_double2(cfgfile, tmp, "Wave",hg->line[i_line].wave,"%8.2f");
    }
  }

  // Obs Plan
  xmms_cfg_write_int(cfgfile, "Plan", "Start", hg->plan_start);
  xmms_cfg_write_int(cfgfile, "Plan", "StartHour", hg->plan_start_hour);
  xmms_cfg_write_int(cfgfile, "Plan", "StartMin", hg->plan_start_min);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    sprintf(tmp,"Plan-%d",i_plan+1);
    xmms_cfg_write_int(cfgfile, tmp, "Type",hg->plan[i_plan].type);

    xmms_cfg_write_int(cfgfile, tmp, "Setup",hg->plan[i_plan].setup);
    xmms_cfg_write_int(cfgfile, tmp, "Repeat",hg->plan[i_plan].repeat);
    xmms_cfg_write_boolean(cfgfile, tmp, "Slit_or", hg->plan[i_plan].slit_or);
    xmms_cfg_write_int(cfgfile, tmp, "Slit_Width",hg->plan[i_plan].slit_width);
    xmms_cfg_write_int(cfgfile, tmp, "Slit_Length",hg->plan[i_plan].slit_length);

    xmms_cfg_write_int(cfgfile, tmp, "ObjI",hg->plan[i_plan].obj_i);
    xmms_cfg_write_int(cfgfile, tmp, "Exp",hg->plan[i_plan].exp);

    xmms_cfg_write_double2(cfgfile, tmp, "dExp", hg->plan[i_plan].dexp,"%.3f");
    xmms_cfg_write_int(cfgfile, tmp, "Shot",hg->plan[i_plan].shot);
    xmms_cfg_write_int(cfgfile, tmp, "CoAdds",hg->plan[i_plan].coadds);
    xmms_cfg_write_int(cfgfile, tmp, "NDR",hg->plan[i_plan].ndr);

    xmms_cfg_write_int(cfgfile, tmp, "Dith",hg->plan[i_plan].dith);
    xmms_cfg_write_double2(cfgfile, tmp, "DithW", hg->plan[i_plan].dithw,"%.3f");
    xmms_cfg_write_int(cfgfile, tmp, "OSRA",hg->plan[i_plan].osra);
    xmms_cfg_write_int(cfgfile, tmp, "OSDec",hg->plan[i_plan].osdec);
    xmms_cfg_write_double2(cfgfile, tmp, "SSsep", hg->plan[i_plan].sssep,"%.3f");
    xmms_cfg_write_int(cfgfile, tmp, "SSnum",hg->plan[i_plan].ssnum);
    
    xmms_cfg_write_int(cfgfile, tmp, "Omode",hg->plan[i_plan].omode);
    xmms_cfg_write_int(cfgfile, tmp, "Guide",hg->plan[i_plan].guide);
    xmms_cfg_write_int(cfgfile, tmp, "AOmode",hg->plan[i_plan].aomode);
    xmms_cfg_write_boolean(cfgfile, tmp, "ADI", hg->plan[i_plan].adi);
    
    xmms_cfg_write_int(cfgfile, tmp, "FocusMode",hg->plan[i_plan].focus_mode);
    xmms_cfg_write_int(cfgfile, tmp, "CalMode",hg->plan[i_plan].cal_mode);

    xmms_cfg_write_int(cfgfile, tmp, "Cmode",hg->plan[i_plan].cmode);

    xmms_cfg_write_int(cfgfile, tmp, "I2_pos", hg->plan[i_plan].i2_pos);

    xmms_cfg_write_double2(cfgfile, tmp, "SetAz", hg->plan[i_plan].setaz,"%6.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "SetEl", hg->plan[i_plan].setel,"%6.2f");

    xmms_cfg_write_boolean(cfgfile, tmp, "Daytime", hg->plan[i_plan].daytime);

    if(hg->plan[i_plan].comment)
      xmms_cfg_write_string(cfgfile, tmp, "Comment",hg->plan[i_plan].comment);
    xmms_cfg_write_int(cfgfile, tmp, "Comtype",hg->plan[i_plan].comtype); 
    
    xmms_cfg_write_int(cfgfile, tmp, "Time",hg->plan[i_plan].time); 

    xmms_cfg_write_boolean(cfgfile, tmp, "PA_or", hg->plan[i_plan].pa_or);
    xmms_cfg_write_double2(cfgfile, tmp, "PA", hg->plan[i_plan].pa,"%6.2f");
    xmms_cfg_write_boolean(cfgfile, tmp, "SV_or", hg->plan[i_plan].sv_or);
    xmms_cfg_write_int(cfgfile, tmp, "SV_exp", hg->plan[i_plan].sv_exp);
    xmms_cfg_write_int(cfgfile, tmp, "SV_fil", hg->plan[i_plan].sv_fil);
    xmms_cfg_write_boolean(cfgfile, tmp, "BackUp", hg->plan[i_plan].backup);
  }

  for(i_plan=hg->i_plan_max;i_plan<MAX_PLAN;i_plan++){
    sprintf(tmp,"Plan-%d",i_plan+1);
    xmms_cfg_remove_key(cfgfile, tmp, "Type");

    xmms_cfg_remove_key(cfgfile, tmp, "Setup");
    xmms_cfg_remove_key(cfgfile, tmp, "Repeat");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_or");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_Width");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_Length");

    xmms_cfg_remove_key(cfgfile, tmp, "ObjI");
    xmms_cfg_remove_key(cfgfile, tmp, "Exp");

    xmms_cfg_remove_key(cfgfile, tmp, "dExp");
    xmms_cfg_remove_key(cfgfile, tmp, "Shot");
    xmms_cfg_remove_key(cfgfile, tmp, "CoAdds");
    xmms_cfg_remove_key(cfgfile, tmp, "NDR");

    xmms_cfg_remove_key(cfgfile, tmp, "Dith");
    xmms_cfg_remove_key(cfgfile, tmp, "DithW");
    xmms_cfg_remove_key(cfgfile, tmp, "OSRA");
    xmms_cfg_remove_key(cfgfile, tmp, "OSDec");
    xmms_cfg_remove_key(cfgfile, tmp, "SSsep");
    xmms_cfg_remove_key(cfgfile, tmp, "SSnum");
    
    xmms_cfg_remove_key(cfgfile, tmp, "Omode");
    xmms_cfg_remove_key(cfgfile, tmp, "Guide");
    xmms_cfg_remove_key(cfgfile, tmp, "AOmode");
    xmms_cfg_remove_key(cfgfile, tmp, "ADI");
    
    xmms_cfg_remove_key(cfgfile, tmp, "FocusMode");
    xmms_cfg_remove_key(cfgfile, tmp, "CalMode");

    xmms_cfg_remove_key(cfgfile, tmp, "Cmode");

    xmms_cfg_remove_key(cfgfile, tmp, "I2_pos");

    xmms_cfg_remove_key(cfgfile, tmp, "SetAz");
    xmms_cfg_remove_key(cfgfile, tmp, "SetEl");

    xmms_cfg_remove_key(cfgfile, tmp, "Daytime");

    xmms_cfg_remove_key(cfgfile, tmp, "Comment");
    xmms_cfg_remove_key(cfgfile, tmp, "Comtype");
    
    xmms_cfg_remove_key(cfgfile, tmp, "Time"); 

    xmms_cfg_remove_key(cfgfile, tmp, "PA_or"); 
    xmms_cfg_remove_key(cfgfile, tmp, "PA"); 
    xmms_cfg_remove_key(cfgfile, tmp, "SV_or"); 
    xmms_cfg_remove_key(cfgfile, tmp, "Sv_exp"); 
    xmms_cfg_remove_key(cfgfile, tmp, "SV_fil"); 
    xmms_cfg_remove_key(cfgfile, tmp, "BackUp"); 
  }


  xmms_cfg_write_file(cfgfile, filename);
  xmms_cfg_free(cfgfile);

  set_win_title(hg);
}


void ReadHOE(typHOE *hg, gboolean destroy_flag)
{
  ConfigFile *cfgfile;
  gchar tmp[64], f_tmp[64], bname[64];
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf, *tmp_p=NULL;
  gboolean b_buf;
  gint i_nonstd,i_set,i_list,i_line,i_plan,i_band, fcdb_type_tmp, i_dbname;
  gint major_ver=0,minor_ver=0,micro_ver=0;

  cfgfile = xmms_cfg_open_file(hg->filename_hoe);

  hg->nst_max=0;
  
  if (cfgfile) {
    
    // General 
    if(hg->filename_write) g_free(hg->filename_write);
    hg->filename_write=
      (xmms_cfg_read_string(cfgfile, "General", "OPE",  &c_buf))? c_buf : NULL;
    if(hg->filename_read) g_free(hg->filename_read);
    hg->filename_read =
      (xmms_cfg_read_string(cfgfile, "General", "List", &c_buf))? c_buf : NULL;
    if(xmms_cfg_read_string(cfgfile, "General", "prog_ver", &c_buf)){
      if((tmp_p=strtok(c_buf,"."))!=NULL){
	major_ver=(gint)g_strtod(tmp_p,NULL);
	if((tmp_p=strtok(NULL,"."))!=NULL){
	  minor_ver=(gint)g_strtod(tmp_p,NULL);
	  if((tmp_p=strtok(NULL,"."))!=NULL){
	    micro_ver=(gint)g_strtod(tmp_p,NULL);
	  }
	}
      }
    }

    // Header
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromYear", &i_buf)){
      hg->fr_year =i_buf;
      hg->skymon_year =i_buf;
    }
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromMonth",&i_buf)){
      hg->fr_month=i_buf;
      hg->skymon_month=i_buf;
    }
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromDay",  &i_buf)){
      hg->fr_day  =i_buf;
      hg->skymon_day  =i_buf;
    }
    if(hg->prop_id) g_free(hg->prop_id);
    hg->prop_id =
      (xmms_cfg_read_string(cfgfile, "Header", "ID",       &c_buf)) ? c_buf : NULL;
    if(hg->prop_pass) g_free(hg->prop_pass);
    hg->prop_pass =
      (xmms_cfg_read_string(cfgfile, "Header", "Pass",       &c_buf))? c_buf : NULL;
    if(hg->observer) g_free(hg->observer);
    hg->observer=
      (xmms_cfg_read_string(cfgfile, "Header", "Observer", &c_buf)) ? c_buf : NULL;

    // Default Parameter
    if(xmms_cfg_read_int  (cfgfile, "DefPara", "Guide",  &i_buf)) hg->def_guide=i_buf;
    if(xmms_cfg_read_double(cfgfile, "DefPara", "PA",     &f_buf)) hg->def_pa   =f_buf;
    if(xmms_cfg_read_int  (cfgfile, "DefPara", "ExpTime",&i_buf)) hg->def_exp  =i_buf;


    // AD Calc.
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave1",  &i_buf)) hg->wave1=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave0",  &i_buf)) hg->wave0=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Pres",   &i_buf)) hg->pres =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Temp",   &i_buf)) hg->temp =i_buf;

    // Instrument
    if(xmms_cfg_read_int    (cfgfile, "Inst", "Inst",     &i_buf)) hg->inst=i_buf;
    else hg->inst=INST_HDS;
    init_inst(hg);

    // AG
    if(xmms_cfg_read_int  (cfgfile, "AG", "ExptimeFactor",  &i_buf)) hg->exptime_factor=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "AG", "Brightness",     &i_buf)) hg->brightness    =i_buf;

  
    // SV
    if(xmms_cfg_read_int  (cfgfile, "SV", "Area",       &i_buf)) hg->sv_area     =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Integrate",  &i_buf)) hg->sv_integrate=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Region",     &i_buf)) hg->sv_region   =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Calc",       &i_buf)) hg->sv_calc     =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Exptime",    &i_buf)) hg->exptime_sv  =i_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "SlitX",      &f_buf)) hg->sv_slitx    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "SlitY",      &f_buf)) hg->sv_slity    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "ISX",      &f_buf)) hg->sv_isx    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "ISY",      &f_buf)) hg->sv_isy    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "IS3X",      &f_buf)) hg->sv_is3x    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "IS3Y",      &f_buf)) hg->sv_is3y    =f_buf;

    // Overhead
    if(xmms_cfg_read_int  (cfgfile, "Overhead", "Acq",  &i_buf)) hg->oh_acq=i_buf;
    else{
      switch(hg->inst){
      case INST_HDS:
	hg->oh_acq=TIME_ACQ;
	break;
      case INST_IRCS:
	hg->oh_acq=IRCS_TIME_ACQ;
	break;
      }
    }
    if(xmms_cfg_read_int  (cfgfile, "Overhead", "NGS1", &i_buf)) hg->oh_ngs1=i_buf;
    else hg->oh_ngs1=IRCS_TIME_AO_NGS1;
    if(xmms_cfg_read_int  (cfgfile, "Overhead", "NGS2", &i_buf)) hg->oh_ngs2=i_buf;
    else hg->oh_ngs2=IRCS_TIME_AO_NGS2;
    if(xmms_cfg_read_int  (cfgfile, "Overhead", "NGS3", &i_buf)) hg->oh_ngs3=i_buf;
    else hg->oh_ngs3=IRCS_TIME_AO_NGS3;
    if(xmms_cfg_read_int  (cfgfile, "Overhead", "LGS",  &i_buf)) hg->oh_lgs=i_buf;
    else hg->oh_lgs=IRCS_TIME_AO_LGS;

    // CameraZ
    if(xmms_cfg_read_int  (cfgfile, "CameraZ", "Blue",   &i_buf)) hg->camz_b=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "CameraZ", "Red",    &i_buf)) hg->camz_r=i_buf;
    
    //Cross
    if(xmms_cfg_read_int  (cfgfile, "Cross", "dCross",    &i_buf)) hg->d_cross=i_buf;


    // HDS NonStd
    for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
      sprintf(tmp,"NonStd-%d",i_nonstd+1);
      if(xmms_cfg_read_int  (cfgfile, tmp, "Color",   &i_buf)) hg->nonstd[i_nonstd].col    =i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "Cross",   &i_buf)) hg->nonstd[i_nonstd].cross  =i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "Echelle", &i_buf)) hg->nonstd[i_nonstd].echelle=i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "CamRot",  &i_buf)) hg->nonstd[i_nonstd].camr   =i_buf;
    }

    
    // HDS Setup
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(tmp,"SetUp-%d",i_set+1);
      if(xmms_cfg_read_int    (cfgfile, tmp, "Setup",     &i_buf)) hg->setup[i_set].setup      =i_buf;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "Use",       &b_buf)) hg->setup[i_set].use        =b_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Binning",   &i_buf)) hg->setup[i_set].binning    =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SlitWidth" ,&i_buf)) hg->setup[i_set].slit_width =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SlitLength",&i_buf)) hg->setup[i_set].slit_length=i_buf;
      if(hg->setup[i_set].fil1) g_free(hg->setup[i_set].fil1);
      hg->setup[i_set].fil1=
	(xmms_cfg_read_string (cfgfile, tmp, "Filter1",   &c_buf)) ? c_buf : NULL;
      if(hg->setup[i_set].fil2) g_free(hg->setup[i_set].fil2);
      hg->setup[i_set].fil2=
	(xmms_cfg_read_string (cfgfile, tmp, "Filter2",   &c_buf)) ? c_buf : NULL;
      if(xmms_cfg_read_int    (cfgfile, tmp, "ImR",       &i_buf)) hg->setup[i_set].imr        =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "IS",       &i_buf)) hg->setup[i_set].is        =i_buf;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "I2",       &b_buf)) hg->setup[i_set].i2        =b_buf;
    }

    // IRCS Setup
    if(xmms_cfg_read_int    (cfgfile, "IRCS", "Max",     &i_buf)) hg->ircs_i_max=i_buf;
    else hg->ircs_i_max=0;
    for(i_set=0;i_set<hg->ircs_i_max;i_set++){
      sprintf(tmp,"IRCS_SetUp-%02d",i_set+1);
      if(xmms_cfg_read_int    (cfgfile, tmp, "Mode", &i_buf)) hg->ircs_set[i_set].mode=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Mas",  &i_buf)) hg->ircs_set[i_set].mas =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Band", &i_buf)) hg->ircs_set[i_set].band=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Slit", &i_buf)) hg->ircs_set[i_set].slit=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Dith", &i_buf)) hg->ircs_set[i_set].dith=i_buf;
 
      if(xmms_cfg_read_double (cfgfile, tmp, "DithW",  &f_buf)) hg->ircs_set[i_set].dithw=f_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "OSRA",   &i_buf)) hg->ircs_set[i_set].osra=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "OSDec",  &i_buf)) hg->ircs_set[i_set].osdec=i_buf;
      if(xmms_cfg_read_double (cfgfile, tmp, "SSsep",  &f_buf)) hg->ircs_set[i_set].sssep=f_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SSnum",  &i_buf)) hg->ircs_set[i_set].ssnum=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Shot",   &i_buf)) hg->ircs_set[i_set].shot=i_buf;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Ech",  &i_buf)) hg->ircs_set[i_set].ech=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "XDS",  &i_buf)) hg->ircs_set[i_set].xds=i_buf;

      if(xmms_cfg_read_int    (cfgfile, tmp, "CW1",  &i_buf)) hg->ircs_set[i_set].cw1=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "CW2",  &i_buf)) hg->ircs_set[i_set].cw2=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "CW3",  &i_buf)) hg->ircs_set[i_set].cw3=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SLW",  &i_buf)) hg->ircs_set[i_set].slw=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SPW",  &i_buf)) hg->ircs_set[i_set].spw=i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Cam",  &i_buf)) hg->ircs_set[i_set].cam=i_buf;

      if(xmms_cfg_read_double (cfgfile, tmp, "SlitX",  &f_buf)) hg->ircs_set[i_set].slit_x=f_buf;
      if(xmms_cfg_read_double (cfgfile, tmp, "SlitY",  &f_buf)) hg->ircs_set[i_set].slit_y=f_buf;

      if(hg->ircs_set[i_set].txt) g_free(hg->ircs_set[i_set].txt);
      hg->ircs_set[i_set].txt=ircs_make_setup_txt(hg, i_set);
      if(hg->ircs_set[i_set].def) g_free(hg->ircs_set[i_set].def);
      hg->ircs_set[i_set].def=ircs_make_def(hg, i_set);
      if(hg->ircs_set[i_set].dtxt) g_free(hg->ircs_set[i_set].dtxt);
      hg->ircs_set[i_set].dtxt=ircs_make_dtxt(hg->ircs_set[i_set].dith,
					      hg->ircs_set[i_set].dithw,
					      hg->ircs_set[i_set].osra,
					      hg->ircs_set[i_set].osdec,
					      hg->ircs_set[i_set].sssep,
					      hg->ircs_set[i_set].ssnum);

      if(xmms_cfg_read_boolean(cfgfile, tmp, "Std",  &b_buf)) hg->ircs_set[i_set].std=b_buf;

      if(xmms_cfg_read_double (cfgfile, tmp, "Exp",  &f_buf)) hg->ircs_set[i_set].exp=f_buf;
    }
    
    // Object List
    for(i_list=0;i_list<MAX_OBJECT;i_list++){
      sprintf(tmp,"Obj-%d",i_list+1);
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      if(xmms_cfg_read_string (cfgfile, tmp, "Name",   &c_buf)){
	hg->obj[i_list].name  =c_buf;
      }
      else{
	hg->obj[i_list].name  =NULL;
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_boolean(cfgfile, tmp, "Std",  &b_buf)) hg->obj[i_list].std=b_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "ExpTime",&i_buf)) hg->obj[i_list].exp   =i_buf;
      else{
	hg->obj[i_list].exp=DEF_EXP;
      }
      if(xmms_cfg_read_int    (cfgfile, tmp, "Repeat",&i_buf))  hg->obj[i_list].repeat=i_buf;
      else{
	hg->obj[i_list].repeat=1;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "RA",     &f_buf)) hg->obj[i_list].ra    =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "PM_RA",     &f_buf)) hg->obj[i_list].pm_ra    =f_buf;
      else{
	hg->obj[i_list].pm_ra=0.0;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "Dec",    &f_buf)) hg->obj[i_list].dec   =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "PM_Dec",     &f_buf)) hg->obj[i_list].pm_dec    =f_buf;
      else{
	hg->obj[i_list].pm_dec=0.0;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "Epoch",  &f_buf)) hg->obj[i_list].equinox =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      ObjMagDB_Init(&hg->obj[i_list]);
      if(xmms_cfg_read_double  (cfgfile, tmp, "Mag",  &f_buf)){
	hg->obj[i_list].mag =f_buf;
	if(xmms_cfg_read_string  (cfgfile, tmp, "MagDB_UsedName",  &c_buf)){
	  hg->obj[i_list].magdb_used=0;
	  hg->obj[i_list].magdb_band=0;
	  for(i_dbname=MAGDB_TYPE_SIMBAD;i_dbname<NUM_DB_ALL;i_dbname++){
	    if(strcmp(db_name[i_dbname], c_buf) == 0){
	      hg->obj[i_list].magdb_used=i_dbname;
	      // printf("Hit Name=%s \n",c_buf);
	      if(xmms_cfg_read_int  (cfgfile, tmp, "MagDB_Band",  &i_buf)){
		hg->obj[i_list].magdb_band =i_buf;
	      }
	      else{	
		hg->obj[i_list].magdb_band=0;
	      }
	      break;
	    }
	  }
	}
	else if(xmms_cfg_read_int  (cfgfile, tmp, "MagDB_Used",  &i_buf)){
	  hg->obj[i_list].magdb_used =i_buf;
	  // printf("Hit Num=%d \n",i_buf);
	  if((major_ver<=3)&&(minor_ver<7)){
	    if(hg->obj[i_list].magdb_used<MAGDB_TYPE_KEPLER){
	      hg->obj[i_list].magdb_used++;
	    }
	  }

	  if(xmms_cfg_read_int  (cfgfile, tmp, "MagDB_Band",  &i_buf)){
	    hg->obj[i_list].magdb_band =i_buf;
	  }
	  else{	
	    hg->obj[i_list].magdb_band=0;
	  }
	}
	else{
	  hg->obj[i_list].magdb_used=0;
	  hg->obj[i_list].magdb_band=0;
	}
      }
      else{
	hg->obj[i_list].mag=100;
	hg->obj[i_list].magdb_used=0;
	hg->obj[i_list].magdb_band=0;
      }

      hg->obj[i_list].snr=-1;
      hg->obj[i_list].sat=FALSE;

      // MagDB SIMBAD
      hg->obj[i_list].magdb_simbad_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_SIMBAD_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_simbad_hits>0){
	hg->obj[i_list].magdb_simbad_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_Sep",  &f_buf)) ? f_buf : -1;
	if(hg->obj[i_list].magdb_simbad_name) g_free(hg->obj[i_list].magdb_simbad_name);
	hg->obj[i_list].magdb_simbad_name=
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Name",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_simbad_type) g_free(hg->obj[i_list].magdb_simbad_type);	
	hg->obj[i_list].magdb_simbad_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Type",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_simbad_sp) g_free(hg->obj[i_list].magdb_simbad_sp);
	hg->obj[i_list].magdb_simbad_sp=
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Sp",  &c_buf))? c_buf : NULL;
	hg->obj[i_list].magdb_simbad_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_b =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_B",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_v =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_V",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_K",  &f_buf)) ? f_buf : 100;
      }

      // MagDB NED
      hg->obj[i_list].magdb_ned_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_NED_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_ned_hits>0){
	hg->obj[i_list].magdb_ned_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_NED_Sep",  &f_buf)) ? f_buf : -1;
	if(hg->obj[i_list].magdb_ned_name) g_free(hg->obj[i_list].magdb_ned_name);
	hg->obj[i_list].magdb_ned_name =						  
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Name",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_ned_type) g_free(hg->obj[i_list].magdb_ned_type);
	hg->obj[i_list].magdb_ned_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Type",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_ned_mag) g_free(hg->obj[i_list].magdb_ned_mag);
	hg->obj[i_list].magdb_ned_mag =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Mag",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_ned_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_NED_Z",  &f_buf)) ? f_buf : -100;
	hg->obj[i_list].magdb_ned_ref =
	  (xmms_cfg_read_int(cfgfile, tmp, "MagDB_NED_REF",  &i_buf)) ? i_buf : 0;
      }

      // MagDB LAMOST
      hg->obj[i_list].magdb_lamost_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_LAMOST_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_lamost_hits>0){
	hg->obj[i_list].magdb_lamost_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_Sep",  &f_buf)) ? f_buf : -1;
	if(hg->obj[i_list].magdb_lamost_name) g_free(hg->obj[i_list].magdb_lamost_name);
	hg->obj[i_list].magdb_lamost_name =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Name",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_lamost_type) g_free(hg->obj[i_list].magdb_lamost_type);
	hg->obj[i_list].magdb_lamost_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Type",  &c_buf)) ? c_buf : NULL;
	if(hg->obj[i_list].magdb_lamost_sp) g_free(hg->obj[i_list].magdb_lamost_sp);
	hg->obj[i_list].magdb_lamost_sp =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Sp",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_lamost_ref =
	  (xmms_cfg_read_int(cfgfile, tmp, "MagDB_LAMOST_REF",  &i_buf)) ? i_buf : 0;
	hg->obj[i_list].magdb_lamost_teff =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_TEFF",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_lamost_logg =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_LOGG",  &f_buf)) ? f_buf : -10;
	hg->obj[i_list].magdb_lamost_feh =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_FEH",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_lamost_hrv =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_HRV",  &f_buf)) ? f_buf : -99999;
      }

      // MagDB Kepler
      hg->obj[i_list].magdb_kepler_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_Kepler_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_kepler_hits>0){
	hg->obj[i_list].magdb_kepler_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_Sep",  &f_buf)) ? f_buf : -1;
	if(hg->obj[i_list].magdb_kepler_name) g_free(hg->obj[i_list].magdb_kepler_name);
	hg->obj[i_list].magdb_kepler_name =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_Kepler_Name",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_kepler_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_K",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_kepler_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_kepler_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_kepler_teff =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_TEFF",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_kepler_logg =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_LOGG",  &f_buf)) ? f_buf : -10;
	hg->obj[i_list].magdb_kepler_feh =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_FEH",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_kepler_ebv =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_EBV",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_kepler_rad =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_RAD",  &f_buf)) ? f_buf : -100;
	hg->obj[i_list].magdb_kepler_pm =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_PM",  &f_buf)) ? f_buf : -10000;
	hg->obj[i_list].magdb_kepler_gr =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_Kepler_GR",  &f_buf)) ? f_buf : 100;
	if(hg->obj[i_list].magdb_kepler_2mass) g_free(hg->obj[i_list].magdb_kepler_2mass);
	hg->obj[i_list].magdb_kepler_2mass =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_Kepler_2MASS",  &c_buf)) ? c_buf : NULL;
      }

      // MagDB GSC
      hg->obj[i_list].magdb_gsc_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_GSC_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_gsc_hits>0){
	hg->obj[i_list].magdb_gsc_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gsc_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_b =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_B",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_v =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_V",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_K",  &f_buf)) ? f_buf : 100;
      }

      // MagDB PS1
      hg->obj[i_list].magdb_ps1_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_PS1_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_ps1_hits>0){
	hg->obj[i_list].magdb_ps1_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_ps1_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Z",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_y =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Y",  &f_buf)) ? f_buf : 100;
      }

      // MagDB SDSS
      hg->obj[i_list].magdb_sdss_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_SDSS_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_sdss_hits>0){
	hg->obj[i_list].magdb_sdss_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_sdss_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_Z",  &f_buf)) ? f_buf : 100;
      }

      // MagDB GAIA
      hg->obj[i_list].magdb_gaia_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_GAIA_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_gaia_hits>0){
	hg->obj[i_list].magdb_gaia_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gaia_p =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_P",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_ep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_EP",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_rp =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_RP",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gaia_bp =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_BP",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gaia_rv =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_RV",  &f_buf)) ? f_buf : -99999;
	hg->obj[i_list].magdb_gaia_teff =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_TEFF",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_ag =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_AG",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gaia_ebr =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_EBR",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_dist =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_DIST",  &f_buf)) ? f_buf : -1;
      }

      // MagDB 2MASS
      hg->obj[i_list].magdb_2mass_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_2MASS_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_2mass_hits>0){
	hg->obj[i_list].magdb_2mass_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_2mass_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_2mass_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_2mass_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_K",  &f_buf)) ? f_buf : 100;
      }

      if(xmms_cfg_read_double  (cfgfile, tmp, "PA",     &f_buf)) hg->obj[i_list].pa    =f_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Guide",  &i_buf)) hg->obj[i_list].guide =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "AOmode",  &i_buf)) hg->obj[i_list].aomode =i_buf;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "ADI",  &b_buf)) hg->obj[i_list].adi=b_buf;
      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
      hg->obj[i_list].note=
	(xmms_cfg_read_string (cfgfile, tmp, "Note",   &c_buf)) ? c_buf : NULL;

      // NST
      if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
      if(xmms_cfg_read_string  (cfgfile, tmp, "NST_File",  &c_buf)){
	hg->nst[hg->nst_max].filename =c_buf;

	if(xmms_cfg_read_int  (cfgfile, tmp, "NST_Type",  &i_buf)){
	  hg->nst[hg->nst_max].type =i_buf;
	}
	else{
	  hg->nst[hg->nst_max].type =NST_TYPE_TSC;
	}
	
	hg->i_max=i_list;
	{
	  gboolean ret;
	  gchar *basename, *dirname;
	  
	  switch(hg->nst[hg->nst_max].type){
	  case NST_TYPE_TSC:
	    if(hg->filename_nst) g_free(hg->filename_nst);
	    hg->filename_nst=g_strdup(hg->nst[hg->nst_max].filename);
	    ret=MergeNST(hg);

	    if(!ret){
	      dirname=g_path_get_dirname(hg->filename_hoe);
	      basename=g_path_get_basename(hg->filename_nst);
	      if(hg->filename_nst) g_free(hg->filename_nst);
	      hg->filename_nst=g_strconcat(to_utf8(dirname),
					   G_DIR_SEPARATOR_S,
					   to_utf8(basename),
					   NULL);
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "dialog-information", 
#else
			    GTK_STOCK_DIALOG_INFO,
#endif
			    POPUP_TIMEOUT*1,
			    "Retrying to Load",
			    " ",
			    hg->filename_nst,
			    NULL);
	      ret=MergeNST(hg);

	      if(dirname) g_free(dirname);
	      if(basename) g_free(basename);
	    }
	    if(ret){
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "network-transmit", 
#else
			    GTK_STOCK_OK, 
#endif
			    POPUP_TIMEOUT*1,
			    "Succeeded to Load Non-Sidereal Tracking File",
			    " ",
			    hg->filename_nst,
			    NULL);
	    }
	    break;

	  case NST_TYPE_JPL:
	    if(hg->filename_jpl) g_free(hg->filename_jpl);
	    hg->filename_jpl=g_strdup(hg->nst[hg->nst_max].filename);
	    ret=MergeJPL(hg);

	    if(!ret){
	      dirname=g_path_get_dirname(hg->filename_hoe);
	      basename=g_path_get_basename(hg->filename_jpl);
	      if(hg->filename_jpl) g_free(hg->filename_jpl);
	      hg->filename_jpl=g_strconcat(to_utf8(dirname),
					   G_DIR_SEPARATOR_S,
					   to_utf8(basename),
					   NULL);
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "dialog-information", 
#else
			    GTK_STOCK_DIALOG_INFO, 
#endif
			    POPUP_TIMEOUT*1,
			    "Retrying to Load",
			    " ",
			    hg->filename_jpl,
			    NULL);
	      ret=MergeJPL(hg);

	      if(dirname) g_free(dirname);
	      if(basename) g_free(basename);
	    }
	    if(ret){
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "document-open", 
#else
			    GTK_STOCK_OK, 
#endif
			    POPUP_TIMEOUT*1,
			    "Succeeded to Load Non-Sidereal Tracking File",
			    " ",
			    hg->filename_jpl,
			    NULL);
	      }
	    break;
	  }

	  if(ret){
	    if(hg->skymon_mode==SKYMON_SET){
	      calcpa2_skymon(hg);
	    }
	    else{
	      calcpa2_main(hg);
	    }
	  }
	  else{
	    hg->obj[i_list].i_nst=-1;
	  }
	}
      }
      else{
	hg->nst[hg->nst_max].filename=NULL;
	hg->obj[i_list].i_nst=-1;
      }
      
      for(i_set=0;i_set<MAX_USESETUP;i_set++){
	sprintf(f_tmp,"SetUp-%d",i_set+1);
	if(xmms_cfg_read_boolean(cfgfile, tmp, f_tmp,  &b_buf)) hg->obj[i_list].setup[i_set]=b_buf;
      }

      if(hg->obj[i_list].gs.name) g_free(hg->obj[i_list].gs.name);
      if(xmms_cfg_read_string (cfgfile, tmp, "GS_Name", &c_buf)){
	hg->obj[i_list].gs.name=c_buf;
	hg->obj[i_list].gs.flag=TRUE;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_RA",   &f_buf)) hg->obj[i_list].gs.ra=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Dec",   &f_buf)) hg->obj[i_list].gs.dec=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Epoch",   &f_buf)) hg->obj[i_list].gs.equinox=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Sep",   &f_buf)) hg->obj[i_list].gs.sep=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Mag",   &f_buf)) hg->obj[i_list].gs.mag=f_buf;
	if(xmms_cfg_read_int  (cfgfile, tmp, "GS_Src",  &i_buf)) hg->obj[i_list].gs.src=i_buf;
      }
      else{
	hg->obj[i_list].gs.name=NULL;
	hg->obj[i_list].gs.flag=FALSE;
      }
      
      if(xmms_cfg_read_int  (cfgfile, tmp, "BandMax",  &i_buf))
	hg->obj[i_list].trdb_band_max=i_buf;
      else
	hg->obj[i_list].trdb_band_max=0;
	
      // Band
      for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
	sprintf(bname, "Object-%d_Band-%d",i_list,i_band);

	if(hg->obj[i_list].trdb_mode[i_band]) 
	  g_free(hg->obj[i_list].trdb_mode[i_band]);
	if(hg->obj[i_list].trdb_mode[i_band]) g_free(hg->obj[i_list].trdb_mode[i_band]);
	hg->obj[i_list].trdb_mode[i_band]=
	  (xmms_cfg_read_string  (cfgfile, bname, "Mode",  &c_buf)) ? c_buf : NULL;

	if(hg->obj[i_list].trdb_band[i_band]) 
	  g_free(hg->obj[i_list].trdb_band[i_band]);
	if(hg->obj[i_list].trdb_band[i_band]) g_free(hg->obj[i_list].trdb_band[i_band]);
	hg->obj[i_list].trdb_band[i_band]=
	  (xmms_cfg_read_string  (cfgfile, bname, "Band",  &c_buf)) ? c_buf : NULL;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;
	
	if(xmms_cfg_read_int  (cfgfile, bname, "Shot",  &i_buf))
	  hg->obj[i_list].trdb_shot[i_band]=i_buf;
	else
	  hg->obj[i_list].trdb_shot[i_band]=0;
      }
      make_band_str(hg, i_list, hg->trdb_da);
    }

    // TRDB
    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Mode",  &i_buf))
      hg->trdb_da=i_buf;
    else
      hg->trdb_da=TRDB_TYPE_SMOKA;

    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Arcmin",  &i_buf))
      hg->trdb_arcmin_used=i_buf;
    else
      hg->trdb_arcmin_used=2;
    hg->trdb_arcmin=hg->trdb_arcmin_used;

    // SMOKA
    if(xmms_cfg_read_int  (cfgfile, "SMOKA", "Inst",  &i_buf))
      hg->trdb_smoka_inst_used=i_buf;
    else
      hg->trdb_smoka_inst_used=0;
    hg->trdb_smoka_inst=hg->trdb_smoka_inst_used;

    if(hg->trdb_smoka_date_used) g_free(hg->trdb_smoka_date_used);
    if(xmms_cfg_read_string(cfgfile, "SMOKA", "Date", &c_buf)) 
      hg->trdb_smoka_date_used =c_buf;
    else
      hg->trdb_smoka_date_used=g_strdup_printf("1998-01-01..%d-%02d-%02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_smoka_date) g_free(hg->trdb_smoka_date);
    hg->trdb_smoka_date=g_strdup(hg->trdb_smoka_date_used);

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Shot", &b_buf))
      hg->trdb_smoka_shot_used =b_buf;
    else
      hg->trdb_smoka_shot_used =TRUE;
    hg->trdb_smoka_shot=hg->trdb_smoka_shot_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Imag", &b_buf))
      hg->trdb_smoka_imag_used =b_buf;
    else
      hg->trdb_smoka_imag_used =TRUE;
    hg->trdb_smoka_imag=hg->trdb_smoka_imag_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Spec", &b_buf))
      hg->trdb_smoka_spec_used =b_buf;
    else
      hg->trdb_smoka_spec_used =TRUE;
    hg->trdb_smoka_spec=hg->trdb_smoka_spec_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Ipol", &b_buf))
      hg->trdb_smoka_ipol_used =b_buf;
    else
      hg->trdb_smoka_ipol_used =TRUE;
    hg->trdb_smoka_ipol=hg->trdb_smoka_ipol_used;

    // HST
    if(xmms_cfg_read_int  (cfgfile, "HST", "Mode",  &i_buf))
      hg->trdb_hst_mode_used=i_buf;
    else
      hg->trdb_hst_mode_used=0;
    hg->trdb_hst_mode=hg->trdb_hst_mode_used;

    if(hg->trdb_hst_date_used) g_free(hg->trdb_hst_date_used);
    if(xmms_cfg_read_string(cfgfile, "HST", "Date", &c_buf)) 
      hg->trdb_hst_date_used =c_buf;
    else
      hg->trdb_hst_date_used=g_strdup_printf("1990-01-01..%d-%02d-%02d",
					     hg->skymon_year,
					     hg->skymon_month,
					     hg->skymon_day);
    if(hg->trdb_hst_date) g_free(hg->trdb_hst_date);
    hg->trdb_hst_date=g_strdup(hg->trdb_hst_date_used);

    if(xmms_cfg_read_int  (cfgfile, "HST", "Image",  &i_buf))
      hg->trdb_hst_image_used=i_buf;
    else
      hg->trdb_hst_image_used=0;
    hg->trdb_hst_image=hg->trdb_hst_image_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Spec",  &i_buf))
      hg->trdb_hst_spec_used=i_buf;
    else
      hg->trdb_hst_spec_used=0;
    hg->trdb_hst_spec=hg->trdb_hst_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Other",  &i_buf))
      hg->trdb_hst_other_used=i_buf;
    else
      hg->trdb_hst_other_used=0;
    hg->trdb_hst_other=hg->trdb_hst_other_used;

    // ESO
    if(xmms_cfg_read_int  (cfgfile, "ESO", "Mode",  &i_buf))
      hg->trdb_eso_mode_used=i_buf;
    else
      hg->trdb_eso_mode_used=0;
    hg->trdb_eso_mode=hg->trdb_eso_mode_used;

    if(hg->trdb_eso_stdate_used) g_free(hg->trdb_eso_stdate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "StDate", &c_buf)) 
      hg->trdb_eso_stdate_used =c_buf;
    else
      hg->trdb_eso_stdate_used=g_strdup("1980 01 01");
    if(hg->trdb_eso_stdate) g_free(hg->trdb_eso_stdate);
    hg->trdb_eso_stdate=g_strdup(hg->trdb_eso_stdate_used);

    if(hg->trdb_eso_eddate_used) g_free(hg->trdb_eso_eddate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "EdDate", &c_buf)) 
      hg->trdb_eso_eddate_used =c_buf;
    else
      hg->trdb_eso_eddate_used=g_strdup_printf("%4d %02d %02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_eso_eddate) g_free(hg->trdb_eso_eddate);
    hg->trdb_eso_eddate=g_strdup(hg->trdb_eso_eddate_used);

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Image",  &i_buf))
      hg->trdb_eso_image_used=i_buf;
    else
      hg->trdb_eso_image_used=0;
    hg->trdb_eso_image=hg->trdb_eso_image_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Spec",  &i_buf))
      hg->trdb_eso_spec_used=i_buf;
    else
      hg->trdb_eso_spec_used=0;
    hg->trdb_eso_spec=hg->trdb_eso_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "VLTI",  &i_buf))
      hg->trdb_eso_vlti_used=i_buf;
    else
      hg->trdb_eso_vlti_used=0;
    hg->trdb_eso_vlti=hg->trdb_eso_vlti_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Pola",  &i_buf))
      hg->trdb_eso_pola_used=i_buf;
    else
      hg->trdb_eso_pola_used=0;
    hg->trdb_eso_pola=hg->trdb_eso_pola_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Coro",  &i_buf))
      hg->trdb_eso_coro_used=i_buf;
    else
      hg->trdb_eso_coro_used=0;
    hg->trdb_eso_coro=hg->trdb_eso_coro_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Other",  &i_buf))
      hg->trdb_eso_other_used=i_buf;
    else
      hg->trdb_eso_other_used=0;
    hg->trdb_eso_other=hg->trdb_eso_other_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "SAM",  &i_buf))
      hg->trdb_eso_sam_used=i_buf;
    else
      hg->trdb_eso_sam_used=0;
    hg->trdb_eso_sam=hg->trdb_eso_sam_used;

    // Gemini
    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Inst",  &i_buf))
      hg->trdb_gemini_inst_used=i_buf;
    else
      hg->trdb_gemini_inst_used=GEMINI_INST_GMOS;
    hg->trdb_gemini_inst=hg->trdb_gemini_inst_used;

    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Mode",  &i_buf))
      hg->trdb_gemini_mode_used=i_buf;
    else
      hg->trdb_gemini_mode_used=0;
    hg->trdb_gemini_mode=hg->trdb_gemini_mode_used;

    if(hg->trdb_gemini_date_used) g_free(hg->trdb_gemini_date_used);
    if(xmms_cfg_read_string(cfgfile, "Gemini", "Date", &c_buf)) 
      hg->trdb_gemini_date_used =c_buf;
    else
      hg->trdb_gemini_date_used=g_strdup_printf("19980101-%4d%02d%02d",
						hg->skymon_year,
						hg->skymon_month,
						hg->skymon_day);
    if(hg->trdb_gemini_date) g_free(hg->trdb_gemini_date);
    hg->trdb_gemini_date=g_strdup(hg->trdb_gemini_date_used);


    // Line List
    for(i_line=0;i_line<MAX_LINE;i_line++){
      sprintf(tmp,"Line-%d",i_line+1);
      if(hg->line[i_line].name) g_free(hg->line[i_line].name);
      hg->line[i_line].name=
	(xmms_cfg_read_string (cfgfile, tmp, "Name",   &c_buf))? c_buf : NULL;
      if(xmms_cfg_read_double    (cfgfile, tmp, "Wave",   &f_buf)) hg->line[i_line].wave=f_buf;
    }


    // Obs Plan
    if(xmms_cfg_read_int(cfgfile, "Plan","Start",&i_buf)) hg->plan_start =i_buf;
    if(xmms_cfg_read_int(cfgfile, "Plan","StartHour",&i_buf)) hg->plan_start_hour =i_buf;
    if(xmms_cfg_read_int(cfgfile, "Plan","StartMin",&i_buf)) hg->plan_start_min =i_buf;

    for(i_plan=0;i_plan<MAX_PLAN;i_plan++){
      sprintf(tmp,"Plan-%d",i_plan+1);
      if(xmms_cfg_read_int    (cfgfile, tmp, "Type",  &i_buf)) hg->plan[i_plan].type =i_buf;
      else{
	hg->i_plan_max=i_plan;
	break;
      }
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
      case PLAN_TYPE_FOCUS:
      case PLAN_TYPE_I2:
      case PLAN_TYPE_SetAzEl:
	hg->plan[i_plan].setup=-1;
	break;

      default:
	if(xmms_cfg_read_int    (cfgfile, tmp, "Setup", &i_buf)) hg->plan[i_plan].setup  =i_buf;
	else hg->plan[i_plan].setup=-1;
	break;
      }
      if(xmms_cfg_read_int    (cfgfile, tmp, "Repeat",&i_buf)) hg->plan[i_plan].repeat =i_buf;
      else hg->plan[i_plan].repeat=1;
      
      if(xmms_cfg_read_boolean(cfgfile, tmp, "Slit_or",&b_buf))hg->plan[i_plan].slit_or=b_buf;
      else hg->plan[i_plan].slit_or=FALSE;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Slit_Width",&i_buf)) hg->plan[i_plan].slit_width =i_buf;
      else hg->plan[i_plan].slit_width=200;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Slit_Length",&i_buf)) hg->plan[i_plan].slit_length =i_buf;
      else hg->plan[i_plan].slit_length=2000;

      if(xmms_cfg_read_int    (cfgfile, tmp, "ObjI",     &i_buf)) hg->plan[i_plan].obj_i     =i_buf;
      else hg->plan[i_plan].exp=DEF_EXP;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Exp",     &i_buf)) hg->plan[i_plan].exp     =i_buf;
      else hg->plan[i_plan].exp=DEF_EXP;

      if(xmms_cfg_read_double    (cfgfile, tmp, "dExp",   &f_buf)) hg->plan[i_plan].dexp=f_buf;
      else hg->plan[i_plan].dexp=0;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Shot",     &i_buf)) hg->plan[i_plan].shot     =i_buf;
      else hg->plan[i_plan].shot=0;
      if(xmms_cfg_read_int    (cfgfile, tmp, "CoAdds",     &i_buf)) hg->plan[i_plan].coadds     =i_buf;
      else hg->plan[i_plan].coadds=IRCS_DEF_COADDS;
      if(xmms_cfg_read_int    (cfgfile, tmp, "NDR",     &i_buf)) hg->plan[i_plan].ndr     =i_buf;
      else hg->plan[i_plan].ndr=IRCS_DEF_NDR;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Dith",     &i_buf)) hg->plan[i_plan].dith     =i_buf;
      else hg->plan[i_plan].dith=IRCS_DITH_NO;
      if(xmms_cfg_read_double    (cfgfile, tmp, "DithW",   &f_buf)) hg->plan[i_plan].dithw=f_buf;
      else hg->plan[i_plan].dithw=4.0;
      if(xmms_cfg_read_int    (cfgfile, tmp, "OSRA",     &i_buf)) hg->plan[i_plan].osra     =i_buf;
      else hg->plan[i_plan].osra=30;
      if(xmms_cfg_read_int    (cfgfile, tmp, "OSDec",     &i_buf)) hg->plan[i_plan].osdec     =i_buf;
      else hg->plan[i_plan].osdec=1800;
      if(xmms_cfg_read_double    (cfgfile, tmp, "SSsep",   &f_buf)) hg->plan[i_plan].sssep=f_buf;
      else hg->plan[i_plan].sssep=0.150;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SSnum",     &i_buf)) hg->plan[i_plan].ssnum     =i_buf;
      else hg->plan[i_plan].ssnum=5;
      
      if(xmms_cfg_read_int    (cfgfile, tmp, "Omode",   &i_buf)) hg->plan[i_plan].omode   =i_buf;
      else hg->plan[i_plan].omode=0;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Guide",   &i_buf)) hg->plan[i_plan].guide   =i_buf;
      else hg->plan[i_plan].guide=SV_GUIDE;
      if(xmms_cfg_read_int    (cfgfile, tmp, "AOmode",   &i_buf)) hg->plan[i_plan].aomode   =i_buf;
      else hg->plan[i_plan].aomode=AOMODE_NO;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "ADI", &b_buf)) hg->plan[i_plan].adi =b_buf;
      else hg->plan[i_plan].adi=FALSE;
      
      if(xmms_cfg_read_int    (cfgfile, tmp, "FocusMode",   &i_buf)) hg->plan[i_plan].focus_mode   =i_buf;
      else hg->plan[i_plan].focus_mode=0;

      if(xmms_cfg_read_int    (cfgfile, tmp, "CalMode",   &i_buf)) hg->plan[i_plan].cal_mode   =i_buf;
      else hg->plan[i_plan].cal_mode=-1;
      
      if(xmms_cfg_read_int    (cfgfile, tmp, "Cmode",   &i_buf)) hg->plan[i_plan].cmode   =i_buf;
      else hg->plan[i_plan].cmode=0;

      if(hg->plan[i_plan].type==PLAN_TYPE_I2){
	if(xmms_cfg_read_int(cfgfile, tmp, "I2_pos",  &i_buf)) hg->plan[i_plan].i2_pos  =i_buf;
	else hg->plan[i_plan].i2_pos=PLAN_I2_OUT;
      }
      else{
	hg->plan[i_plan].i2_pos=PLAN_I2_OUT;
      }

      if(xmms_cfg_read_double    (cfgfile, tmp, "SetAz",   &f_buf)) hg->plan[i_plan].setaz=f_buf;
      else hg->plan[i_plan].setaz=-90.;

      if(xmms_cfg_read_double    (cfgfile, tmp, "SetEl",   &f_buf)) hg->plan[i_plan].setel=f_buf;
      else hg->plan[i_plan].setel=90.;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "Daytime", &b_buf)) hg->plan[i_plan].daytime =b_buf;
      else hg->plan[i_plan].daytime=FALSE;

      if(hg->plan[i_plan].comment) g_free(hg->plan[i_plan].comment);
      hg->plan[i_plan].comment=
	(xmms_cfg_read_string (cfgfile, tmp, "Comment", &c_buf)) ? c_buf : NULL;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Comtype", &i_buf)) hg->plan[i_plan].comtype   =i_buf;
      else hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Time",    &i_buf)) hg->plan[i_plan].time    =i_buf;
      else hg->plan[i_plan].time=0;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "PA_or", &b_buf)) hg->plan[i_plan].pa_or =b_buf;
      else hg->plan[i_plan].pa_or=FALSE;

      if(xmms_cfg_read_double    (cfgfile, tmp, "PA",   &f_buf)) hg->plan[i_plan].pa=f_buf;
      else hg->plan[i_plan].pa=0;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "SV_or", &b_buf)) hg->plan[i_plan].sv_or =b_buf;
      else hg->plan[i_plan].sv_or=FALSE;

      if(xmms_cfg_read_int    (cfgfile, tmp, "SV_exp", &i_buf)) hg->plan[i_plan].sv_exp   =i_buf;
      else hg->plan[i_plan].sv_exp=hg->exptime_sv;

      if(xmms_cfg_read_int    (cfgfile, tmp, "SV_fil", &i_buf)) hg->plan[i_plan].sv_fil   =i_buf;
      else hg->plan[i_plan].sv_fil=SV_FILTER_NONE;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "BackUp", &b_buf)) hg->plan[i_plan].backup =b_buf;
      else hg->plan[i_plan].backup=FALSE;

      hg->plan[i_plan].focus_is=FALSE;
      hg->plan[i_plan].is_change=FALSE;
      hg->plan[i_plan].colinc=0;
      hg->plan[i_plan].bin_change=FALSE;

      if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
      hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
    }

    xmms_cfg_free(cfgfile);
  }

  if(!hg->prop_pass){
    ReadPass(hg);
  }

  calc_rst(hg);
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=hg->trdb_da;
  if(destroy_flag){
    trdb_make_tree(hg);
    rebuild_trdb_tree(hg);
  }
  hg->fcdb_type=fcdb_type_tmp;

  if(destroy_flag){
    gtk_widget_destroy(hg->all_note);

    flag_make_obj_tree=FALSE;
    flag_make_line_tree=FALSE;

    make_note(hg);
  }

  if(destroy_flag){
    set_win_title(hg);
  }
}



///////////////////////////////////////////////////////////////////
//////////   core procedure of Read/Write Conf (HOME$/.hoe) file
///////////////////////////////////////////////////////////////////

void WriteConf(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_col;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // Version
  xmms_cfg_write_string(cfgfile, "Version", "Major", MAJOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Minor", MINOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Micro", MICRO_VERSION);
  
  // SIMBAD
  xmms_cfg_write_int(cfgfile, "Database", "SIMBAD", hg->fcdb_simbad);
  xmms_cfg_write_int(cfgfile, "Database", "VizieR", hg->fcdb_vizier);

  // Font
  xmms_cfg_write_string(cfgfile, "Font", "Name", hg->fontname);
  xmms_cfg_write_string(cfgfile, "Font", "All", hg->fontname_all);

  // PC 
  if(hg->www_com) 
    xmms_cfg_write_string(cfgfile, "PC", "Browser", hg->www_com);

  // FC-mode
  xmms_cfg_write_int(cfgfile, "FC", "Mode0", hg->fc_mode0);

  // LGS-SA
  if(hg->lgs_sa_name)
    xmms_cfg_write_string(cfgfile, "LGS_SA", "Name", hg->lgs_sa_name);
  if(hg->lgs_sa_email)
    xmms_cfg_write_string(cfgfile, "LGS_SA", "Mail", hg->lgs_sa_email);
  if(hg->lgs_sa_phone1!=0)
    xmms_cfg_write_int(cfgfile, "LGS_SA", "Phone1", hg->lgs_sa_phone1);
  if(hg->lgs_sa_phone2!=0)
    xmms_cfg_write_int(cfgfile, "LGS_SA", "Phone2", hg->lgs_sa_phone2);
  if(hg->lgs_sa_phone3!=0)
    xmms_cfg_write_int(cfgfile, "LGS_SA", "Phone3", hg->lgs_sa_phone3);
  
  xmms_cfg_write_file(cfgfile, conffile);
  xmms_cfg_free(cfgfile);

  g_free(conffile);
}


void ReadConf(typHOE *hg)
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
  
  if (cfgfile) {
    if(xmms_cfg_read_int(cfgfile, "Database", "SIMBAD", &i_buf)) 
      hg->fcdb_simbad =i_buf;
    else
      hg->fcdb_simbad=FCDB_SIMBAD_STRASBG;

    if(xmms_cfg_read_int(cfgfile, "Database", "VizieR", &i_buf)) 
      hg->fcdb_vizier =i_buf;
    else
      hg->fcdb_vizier=FCDB_VIZIER_NAOJ;

    if(xmms_cfg_read_string(cfgfile, "Font", "Name", &c_buf)) 
      hg->fontname =c_buf;
    else
      hg->fontname=g_strdup(SKYMON_FONT);

    if(xmms_cfg_read_string(cfgfile, "Font", "All", &c_buf)) 
      hg->fontname_all=c_buf;
    else
      hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);

    // PC 
    if(xmms_cfg_read_string(cfgfile, "PC", "Browser", &c_buf)) 
      hg->www_com =c_buf;
    else
      hg->www_com=g_strdup(WWW_BROWSER);

    // FC mode0
    if(xmms_cfg_read_int(cfgfile, "FC", "Mode0", &i_buf)) 
      hg->fc_mode0 =i_buf;
    else
      hg->fc_mode0 = FC_SKYVIEW_DSS2R;

    // Tel Speed
    if(xmms_cfg_read_double(cfgfile, "Observatory", "VelAz", &f_buf)) 
      hg->vel_az =f_buf;
    else
      hg->vel_az = VEL_AZ_SUBARU;

    if(xmms_cfg_read_double(cfgfile, "Observatory", "VelEl", &f_buf)) 
      hg->vel_el =f_buf;
    else
      hg->vel_el = VEL_EL_SUBARU;

    // LGS SA
    if(xmms_cfg_read_string(cfgfile, "LGS_SA", "Name", &c_buf)) 
      hg->lgs_sa_name =c_buf;
    else
      hg->lgs_sa_name=NULL;
    if(xmms_cfg_read_string(cfgfile, "LGS_SA", "Mail", &c_buf)) 
      hg->lgs_sa_email =c_buf;
    else
      hg->lgs_sa_email=NULL;
    if(xmms_cfg_read_int(cfgfile, "LGS_SA", "Phone1", &i_buf)) 
      hg->lgs_sa_phone1 =i_buf;
    else
      hg->lgs_sa_phone1 =808;
    if(xmms_cfg_read_int(cfgfile, "LGS_SA", "Phone2", &i_buf)) 
      hg->lgs_sa_phone2 =i_buf;
    else
      hg->lgs_sa_phone2 =934;
    if(xmms_cfg_read_int(cfgfile, "LGS_SA", "Phone3", &i_buf)) 
      hg->lgs_sa_phone3 =i_buf;
    else
      hg->lgs_sa_phone3 =0;
    
    xmms_cfg_free(cfgfile);
  }
  else{
    hg->www_com=g_strdup(WWW_BROWSER);
    hg->fontname=g_strdup(SKYMON_FONT);
    hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);
    hg->fc_mode0 = FC_SKYVIEW_DSS2R;
    hg->vel_az = VEL_AZ_SUBARU;
    hg->vel_el = VEL_EL_SUBARU;
    
    hg->lgs_sa_name=NULL;
    hg->lgs_sa_email=NULL;
    hg->lgs_sa_phone1 =808;
    hg->lgs_sa_phone2 =934;
    hg->lgs_sa_phone3 =0;
  }
}


