//    HDS OPE file Editor
//      fc.c : Finding Chart
//                                           2010.3.15  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"
#include "hsc.h"

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <signal.h>


static void fc_item2();
void fc_dl ();
void fc_dl_draw_all ();
void do_fc();
void create_fc_dialog();
void close_fc();
static void cancel_fc();
#ifndef USE_WIN32
static void cancel_fc_all();
#endif
void draw_fc_obj();
gboolean draw_fc_cairo();
static gboolean expose_draw_fc();
static gboolean configure_draw_fc();
static gboolean resize_draw_fc();
static gboolean button_draw_fc();
void draw_fc_pixmap();
static void refresh_fc();
static void orbit_fc();
static void set_hsc_dith_label();
static void hsc_dith_back();
static void hsc_dith_forward();
static void cc_get_hsc_dith();
static void cc_get_fc_inst();
static void cc_get_fc_mode();
static void cc_get_fc_mode_pdf();
static void do_print_fc();
static void draw_page();

#ifndef USE_WIN32
void dss_signal();
void dssall_signal();
#endif
gboolean check_dssall();

glong get_file_size();

static void show_fc_help();
static void close_fc_help();

void create_fcdb_para_dialog();
void change_fcdb_para();
void radio_fcdb();


gboolean flag_getDSS=FALSE, flag_getFCDB=FALSE;
gboolean flagHSCDialog=FALSE;
GdkPixbuf *pixbuf_fc=NULL, *pixbuf2_fc=NULL;
gboolean  flag_dssall_finish=FALSE;
gboolean  flag_dssall_kill=FALSE;

gdouble current_yrs(typHOE *hg){
  double JD;
  struct ln_zonedate zonedate;

  zonedate.years=hg->fr_year;
  zonedate.months=hg->fr_month;
  zonedate.days=hg->fr_day;
  zonedate.hours=24;
  zonedate.minutes=0;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)hg->obs_timezone*60;
  JD = ln_get_julian_local_date(&zonedate);
  return((JD-JD2000)/365.25);
}


static void
fcdb_toggle (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->fcdb_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  if(flagFC)  draw_fc_cairo(hg->fc_dw, hg);
}


void fc_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  fc_item2(hg, FC_MODE_OBJ);
}

void fc_item_trdb (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  fc_item2(hg, FC_MODE_TRDB);
}

void fc_item_redl (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  fc_item2(hg, FC_MODE_REDL);
}

void fc_item2 (typHOE *hg, gint mode_switch)
{
  fc_dl(hg, mode_switch);

  hg->dss_arcmin_ip=hg->dss_arcmin;
  hg->fc_mode_get=hg->fc_mode;
#ifndef USE_WIN32
  if(fc_pid){
#endif
    if(pixbuf_fc)  g_object_unref(G_OBJECT(pixbuf_fc));
    pixbuf_fc = gdk_pixbuf_new_from_file(hg->dss_file, NULL);

    do_fc(hg);
#ifndef USE_WIN32
  }
#endif
  
  fcdb_clear_tree(hg,FALSE);

  if(hg->fcdb_auto) fcdb_item(NULL, (gpointer)hg);
}


void fcdb_para_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  create_fcdb_para_dialog(hg);
}

void fc_dl (typHOE *hg, gint mode_switch)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint mode;
  GtkTreeModel *model;
  GtkTreeSelection *selection;

  if(flag_getDSS) return;
  flag_getDSS=TRUE;
  
  switch(mode_switch){
  case FC_MODE_OBJ:
  case FC_MODE_TRDB:

    switch(mode_switch){
    case FC_MODE_OBJ:
      model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
      break;

    case FC_MODE_TRDB:
      model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->trdb_tree));
      break;
    }

    if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
      gint i, i_list;
      GtkTreePath *path;
    
      path = gtk_tree_model_get_path (model, &iter);
      
      switch(mode_switch){
      case FC_MODE_OBJ:
	gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	break;

      case FC_MODE_TRDB:
	gtk_tree_model_get (model, &iter, COLUMN_TRDB_NUMBER, &i, -1);
	break;
      }
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
      flag_getDSS=FALSE;
      return;
    }
    break;

  default:
    // Just use same hg->dss_i
    break;
  }

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event",cancel_fc,(gpointer)hg);
  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
  
  mode=hg->fc_mode;

  switch(mode){
  case FC_STSCI_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_ESO_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_ESO_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_ESO "\" ...");
      break;
      
  case FC_ESO_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_ESO_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_SKYVIEW_GALEXF:
    label=gtk_label_new("Retrieving GALEX (Far UV) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_GALEXN:
    label=gtk_label_new("Retrieving GALEX (Near UV) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSU:
    label=gtk_label_new("Retrieving SDSS (u-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSG:
    label=gtk_label_new("Retrieving SDSS (g-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSR:
    label=gtk_label_new("Retrieving SDSS (r-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSI:
    label=gtk_label_new("Retrieving SDSS (i-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSZ:
    label=gtk_label_new("Retrieving SDSS (z-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSJ:
    label=gtk_label_new("Retrieving 2MASS (J-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSH:
    label=gtk_label_new("Retrieving 2MASS (H-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSK:
    label=gtk_label_new("Retrieving 2MASS (K-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE34:
    label=gtk_label_new("Retrieving WISE (3.4um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE46:
    label=gtk_label_new("Retrieving WISE (4.6um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE12:
    label=gtk_label_new("Retrieving WISE (12um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE22:
    label=gtk_label_new("Retrieving WISE (22um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_AKARIN60:
    label=gtk_label_new("Retrieving AKARI N60 image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_AKARIWS:
    label=gtk_label_new("Retrieving AKARI WIDE-S image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_AKARIWL:
    label=gtk_label_new("Retrieving AKARI WIDE-L image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_AKARIN160:
    label=gtk_label_new("Retrieving AKARI N160 image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_NVSS:
    label=gtk_label_new("Retrieving NVSS (1.4GHz) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SDSS:
    label=gtk_label_new("Retrieving SDSS (DR7/color) image from \"" FC_HOST_SDSS "\" ...");
    break;
    
  case FC_SDSS13:
    label=gtk_label_new("Retrieving SDSS (DR14/color) image from \"" FC_HOST_SDSS13 "\" ...");
    break;
    
  case FC_PANCOL:
    label=gtk_label_new("Retrieving PanSTARRS-1 (color) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANG:
    label=gtk_label_new("Retrieving PanSTARRS-1 (g) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANR:
    label=gtk_label_new("Retrieving PanSTARRS-1 (r) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANI:
    label=gtk_label_new("Retrieving PanSTARRS-1 (i) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANZ:
    label=gtk_label_new("Retrieving PanSTARRS-1 (z) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANY:
    label=gtk_label_new("Retrieving PanSTARRS-1 (y) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  }
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->dss_file);
  
  hg->plabel=gtk_label_new("Retrieving image from website ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     hg->plabel,FALSE,FALSE,0);
  
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    cancel_fc, 
		    (gpointer)hg);
  
  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);

#ifndef USE_WIN32
  act.sa_handler=dss_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_dss(hg);

  gtk_main();

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) gtk_timeout_remove(timer);
  gtk_widget_destroy(dialog);
  
  flag_getDSS=FALSE;
}



void fc_dl_draw_all (typHOE *hg)
{
  GtkTreeIter iter;
  gchar tmp[128];
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  guint timer;
  gint i_list;
  gint dssall_check_timer;
  cairo_t *cr;
  cairo_surface_t *surface=NULL;
  gint total_check=0;
  gboolean get_all=FALSE;

  if(hg->i_max<=0) return;
  if(flag_getDSS) return;
  flag_getDSS=TRUE;
  

  dialog = gtk_dialog_new();
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Creating Finding Charts");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
  
  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_ESO_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSJ:
    label=gtk_label_new("Retrieving 2MASS (J-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSH:
    label=gtk_label_new("Retrieving 2MASS (H-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSK:
    label=gtk_label_new("Retrieving 2MASS (K-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SDSS:
    label=gtk_label_new("Retrieving SDSS (DR7) image from \"" FC_HOST_SDSS "\" ...");
    break;
    
  case FC_SDSS13:
    label=gtk_label_new("Retrieving SDSS (DR13) image from \"" FC_HOST_SDSS13 "\" ...");
    break;
    
  }
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);

  hg->pbar2=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar2,TRUE,TRUE,0);
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				    GTK_PROGRESS_LEFT_TO_RIGHT);
  gtk_widget_show(hg->pbar2);
  

  hg->plabel=gtk_label_new("Retrieving image from website ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     hg->plabel,TRUE,TRUE,0);

#ifndef USE_WIN32
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    cancel_fc_all, 
		    (gpointer)hg);
#endif
    
  gtk_widget_show_all(dialog);
  

  hg->fc_output=FC_OUTPUT_PDF_ALL;

  surface = cairo_pdf_surface_create(hg->filename_pdf, PLOT_HEIGHT, PLOT_HEIGHT);
  cr = cairo_create(surface); 

  cairo_set_source_rgb(cr, 1, 1, 1);

  dssall_check_timer=g_timeout_add(1000, 
				   (GSourceFunc)check_dssall,
				   (gpointer)hg);

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].check_sm) total_check++;
  }

  if(total_check==0){
    get_all=TRUE;
  }

  for(i_list=0;i_list<hg->i_max;i_list++){
    if((get_all) || (hg->obj[i_list].check_sm)){
      if((!hg->fc_all_magskip)||
	 ((hg->obj[i_list].mag>10)&&(hg->obj[i_list].mag<99))){
	hg->dss_i=i_list;
	
#ifndef USE_WIN32
	act.sa_handler=dssall_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGHSKYMON1, &act,0);
#endif
	
	hg->dss_arcmin_ip=hg->dss_arcmin;
	
	timer=g_timeout_add(100, 
			    (GSourceFunc)progress_timeout,
			    (gpointer)hg);
	
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar),"Downloading ...");
	
	get_dss(hg);
	
	gtk_main();
	gtk_timeout_remove(timer);
	
	if(flag_dssall_kill){
	  flag_dssall_kill=FALSE;
	  flag_dssall_finish=FALSE;
	  break;
	}
	else{
	  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar),"Creating PDF ...");
#ifndef USE_WIN32
	  if(fc_pid){
#endif
	    if(pixbuf_fc)  g_object_unref(G_OBJECT(pixbuf_fc));
	    pixbuf_fc = gdk_pixbuf_new_from_file(hg->dss_file, NULL);
	    
	    draw_fc_obj(hg,cr,PLOT_HEIGHT,PLOT_HEIGHT);
#ifndef USE_WIN32
	  }
#endif
	  
	  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar2),
					(get_all) ? 
					(gdouble)(hg->dss_i+1)/(gdouble)(hg->i_max) :
					(gdouble)(hg->dss_i+1)/(gdouble)(total_check));
	  sprintf(tmp,"Finished [ %d / %d ] Objects",i_list+1,
		  (get_all) ? hg->i_max : total_check);
	  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
	  
	  
	  flag_dssall_finish=FALSE;
	}
      }
    }
  }

  gtk_timeout_remove(dssall_check_timer);
  gtk_widget_destroy(dialog);

  cairo_surface_destroy(surface);
  cairo_destroy(cr);

  hg->fc_output=FC_OUTPUT_WINDOW;
  flag_getDSS=FALSE;
}

gboolean progress_timeout( gpointer data ){
  typHOE *hg=(typHOE *)data;
  glong sz;
  gchar *tmp;

  if(GTK_WIDGET_REALIZED(hg->pbar)){

    if(flag_getDSS){
      sz=get_file_size(hg->dss_file);
    }
    else{
      sz=get_file_size(hg->fcdb_file);
    }
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));

    if(sz>1024){
      sz=sz/1024;
      
      if(sz>1024){
	tmp=g_strdup_printf("Downloaded %.2f MB",(gfloat)sz/1024.);
      }
      else{
	tmp=g_strdup_printf("Downloaded %ld kB",sz);
      }
    }
    else if (sz>0){
      tmp=g_strdup_printf("Downloaded %ld bytes",sz);
    }
    else{
#ifdef USE_SSL      
      if(flag_getDSS){
	if((hg->fc_mode<FC_SEP2)||(hg->fc_mode>FC_SEP3)){
	  tmp=g_strdup_printf("Waiting for HTTP responce ...");
	}
	else{
	  tmp=g_strdup_printf("Waiting for HTTPS responce ...");
	}
      }
      else{
	switch(hg->fcdb_type){
	case FCDB_TYPE_GEMINI:
	case TRDB_TYPE_GEMINI:
	case TRDB_TYPE_FCDB_GEMINI:
	  tmp=g_strdup_printf("Waiting for HTTPS responce ...");
	  break;

	default:
	  tmp=g_strdup_printf("Waiting for HTTP responce ...");
	  break;
	}
      }
#else
      tmp=g_strdup_printf("Waiting for HTTP responce ...");
#endif
    }
    gtk_label_set_text(GTK_LABEL(hg->plabel), tmp);
    g_free(tmp);
    
    return TRUE;
  }
  else{
    //return FALSE;
    return TRUE;
  }
}

void close_hsc_dither(GtkWidget *w, GtkWidget *dialog)
{
  gtk_widget_destroy(dialog);
  flagHSCDialog=FALSE;
}


void set_hsc_dither (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *hbox, *hbox2, *frame, *spinner, *table;
  GtkAdjustment *adj;
  typHOE *hg;
  gint i;

  if(flagHSCDialog){
    return;
  }
  else{
    flagHSCDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : HSC Dithering Parameters");

  my_signal_connect(dialog,"destroy",
		    close_hsc_dither, 
		    GTK_WIDGET(dialog));

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);


  label=gtk_label_new("Dither Type");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "None",
		       1, HSC_DITH_NO, -1);
    if(hg->hsc_dithp==HSC_DITH_NO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "5-point",
		       1, HSC_DITH_5, -1);
    if(hg->hsc_dithp==HSC_DITH_5) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "N-point",
		       1, HSC_DITH_N, -1);
    if(hg->hsc_dithp==HSC_DITH_N) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_hsc_dith, (gpointer)hg);
  }

  label=gtk_label_new(" ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_BACK);
  gtk_box_pack_start(GTK_BOX(hbox),
		     button,FALSE,FALSE,0);
  my_signal_connect (button,"clicked",hsc_dith_back, (gpointer)hg);

  hg->hsc_label_dith=gtk_label_new(" 1/5 ");
  gtk_misc_set_alignment (GTK_MISC (hg->hsc_label_dith), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), hg->hsc_label_dith, FALSE, FALSE, 5);
  set_hsc_dith_label(hg);
  
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_FORWARD);
  gtk_box_pack_start(GTK_BOX(hbox),
		     button,FALSE,FALSE,0);
  my_signal_connect (button,"clicked",hsc_dith_forward, (gpointer)hg);


  frame = gtk_frame_new ("5-point parameters");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label=gtk_label_new("d_RA [arcsec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_dra,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_dra);


  label=gtk_label_new("    d_Dec [arcsec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_ddec,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_ddec);


  frame = gtk_frame_new ("N-point parameters");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(6,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label=gtk_label_new("TDITH [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_tdith,
					    0, 90,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_tdith);


  label=gtk_label_new("    RDITH [arcsec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_rdith,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_rdith);


  label=gtk_label_new("    NDITH");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 4, 5, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_ndith,
					    3, 30,
					    1.0, 1.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 5, 6, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_ndith);


  frame = gtk_frame_new ("Pointing Offset");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label=gtk_label_new("RA [arcsec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_offra,
					    -3000, 3000,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_offra);


  label=gtk_label_new("    Dec [arcsec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_offdec,
					    -3000, 3000,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_offdec);


  button=gtkut_button_new_from_stock("Redraw",GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (refresh_fc), (gpointer)hg);

  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_hsc_dither, 
		    GTK_WIDGET(dialog));


  gtk_widget_show_all(dialog);
}


void do_fc(typHOE *hg){
  if(flagFC){
    gdk_window_raise(hg->fc_main->window);
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw, hg);
    return;
  }
  else{
    flagFC=TRUE;
  }
  
  create_fc_dialog(hg);
}

void set_fc_frame_col(typHOE *hg){
  if((hg->fc_mode>FC_SEP2)&&(hg->fc_mode<FC_SEP3)){
    gtk_widget_set_sensitive(hg->fc_frame_col,TRUE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_frame_col,FALSE);
  }
}

void set_fc_frame_col_pdf(typHOE *hg){
  if((hg->fc_mode>FC_SEP2)&&(hg->fc_mode<FC_SEP3)){
    gtk_widget_set_sensitive(hg->fc_frame_col_pdf,TRUE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_frame_col_pdf,FALSE);
  }
}

void create_fc_dialog(typHOE *hg)
{
  GtkWidget *vbox, *vbox1, *hbox, *hbox1, *hbox2, *ebox, *table;
  GtkWidget *frame, *check, *label, *button, *spinner;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;

  hg->fc_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->fc_main), "HOE : Finding Chart");
  
  my_signal_connect(hg->fc_main,
		    "destroy",
		    close_fc, 
		    (gpointer)hg);

  gtk_widget_set_app_paintable(hg->fc_main, TRUE);
  
  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_main), vbox);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Image Source");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(5,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_dl), icon_dl, 
				    FALSE, NULL);

  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fc_item_redl), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Download & Redraw");
#endif

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    GtkWidget *bar;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Red)",
		       1, FC_STSCI_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Blue)",
		       1, FC_STSCI_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Red)",
		       1, FC_STSCI_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Blue)",
		       1, FC_STSCI_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (IR)",
		       1, FC_STSCI_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP1,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS1 (Red)",
		       1, FC_ESO_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS1R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Red)",
		       1, FC_ESO_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Blue)",
		       1, FC_ESO_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2B) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (IR)",
		       1, FC_ESO_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP2, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Far UV)",
		       1, FC_SKYVIEW_GALEXF, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXF) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Near UV)",
		       1, FC_SKYVIEW_GALEXN, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXN) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Red)",
		       1, FC_SKYVIEW_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Blue)",
		       1, FC_SKYVIEW_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Red)",
		       1, FC_SKYVIEW_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Blue)",
		       1, FC_SKYVIEW_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (IR)",
		       1, FC_SKYVIEW_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2IR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (u)",
		       1, FC_SKYVIEW_SDSSU, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSU) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (g)",
		       1, FC_SKYVIEW_SDSSG, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (r)",
		       1, FC_SKYVIEW_SDSSR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (i)",
		       1, FC_SKYVIEW_SDSSI, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSI) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (z)",
		       1, FC_SKYVIEW_SDSSZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (J)",
		       1, FC_SKYVIEW_2MASSJ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (H)",
		       1, FC_SKYVIEW_2MASSH, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (K)",
		       1, FC_SKYVIEW_2MASSK, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSK) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (3.4um)",
		       1, FC_SKYVIEW_WISE34, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE34) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (4.6um)",
		       1, FC_SKYVIEW_WISE46, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE46) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (12um)",
		       1, FC_SKYVIEW_WISE12, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE12) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (22um)",
		       1, FC_SKYVIEW_WISE22, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE22) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI N60",
		       1, FC_SKYVIEW_AKARIN60, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIN60) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI WIDE-S",
		       1, FC_SKYVIEW_AKARIWS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIWS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI WIDE-L",
		       1, FC_SKYVIEW_AKARIWL, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIWL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI N160",
		       1, FC_SKYVIEW_AKARIN160, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIN160) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: NVSS (1.4GHz)",
		       1, FC_SKYVIEW_NVSS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_NVSS) iter_set=iter;
	
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP3, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR7 (color)",
		       1, FC_SDSS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR14 (color)",
		       1, FC_SDSS13, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS13) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP4, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (color)",
		       1, FC_PANCOL, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANCOL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (g)",
		       1, FC_PANG, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (r)",
		       1, FC_PANR, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (i)",
		       1, FC_PANI, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANI) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (z)",
		       1, FC_PANZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANZ) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (y)",
		       1, FC_PANY, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANY) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_mode,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Size [min]");
  gtk_table_attach (GTK_TABLE(table), frame, 2, 3, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_arcmin = (GtkAdjustment *)gtk_adjustment_new(hg->dss_arcmin,
		            DSS_ARCMIN_MIN, DSS_ARCMIN_MAX,
   			    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_arcmin, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->fc_adj_dss_arcmin, "value_changed",
		     cc_get_adj,
		     &hg->dss_arcmin);


  hg->fc_frame_col = gtk_frame_new ("Scale/Color");
  gtk_table_attach (GTK_TABLE(table), hg->fc_frame_col, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_frame_col), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_frame_col), hbox2);

  set_fc_frame_col(hg);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Linear",
		       1, FC_SCALE_LINEAR, -1);
    if(hg->dss_scale==FC_SCALE_LINEAR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Log",
		       1, FC_SCALE_LOG, -1);
    if(hg->dss_scale==FC_SCALE_LOG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Sqrt",
		       1, FC_SCALE_SQRT, -1);
    if(hg->dss_scale==FC_SCALE_SQRT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HistEq",
		       1, FC_SCALE_HISTEQ, -1);
    if(hg->dss_scale==FC_SCALE_HISTEQ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "LogLog",
		       1, FC_SCALE_LOGLOG, -1);
    if(hg->dss_scale==FC_SCALE_LOGLOG) iter_set=iter;
	
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->dss_scale);
  }

  button=gtk_check_button_new_with_label("Inverse");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_invert);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_invert);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Instrument");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);


  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (refresh_fc), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Redraw");
#endif


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "None",
		       1, FC_INST_NONE, -1);
    if(hg->fc_inst==FC_INST_NONE) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS",
		       1, FC_INST_HDS, -1);
    if(hg->fc_inst==FC_INST_HDS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (w/oImR)",
		       1, FC_INST_HDSAUTO, -1);
    if(hg->fc_inst==FC_INST_HDSAUTO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (Zenith)",
		       1, FC_INST_HDSZENITH, -1);
    if(hg->fc_inst==FC_INST_HDSZENITH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IRCS",
		       1, FC_INST_IRCS, -1);
    if(hg->fc_inst==FC_INST_IRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "COMICS",
		       1, FC_INST_COMICS, -1);
    if(hg->fc_inst==FC_INST_COMICS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FOCAS",
		       1, FC_INST_FOCAS, -1);
    if(hg->fc_inst==FC_INST_FOCAS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MOIRCS",
		       1, FC_INST_MOIRCS, -1);
    if(hg->fc_inst==FC_INST_MOIRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FMOS",
		       1, FC_INST_FMOS, -1);
    if(hg->fc_inst==FC_INST_FMOS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SupCam",
		       1, FC_INST_SPCAM, -1);
    if(hg->fc_inst==FC_INST_SPCAM) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (Det-ID)",
		       1, FC_INST_HSCDET, -1);
    if(hg->fc_inst==FC_INST_HSCDET) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (HSCA)",
		       1, FC_INST_HSCA, -1);
    if(hg->fc_inst==FC_INST_HSCA) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_inst, (gpointer)hg);
  }

  button=gtk_check_button_new_with_label("Detail");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_table_attach (GTK_TABLE(table), button, 2, 3, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_draw_slit);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_draw_slit);



  frame = gtk_frame_new ("PA [deg]");
  gtk_table_attach (GTK_TABLE(table), frame, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_pa = (GtkAdjustment *)gtk_adjustment_new(hg->dss_pa,
						       -360, 360,
						       1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_pa, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->fc_adj_dss_pa, "value_changed",
		     cc_get_adj,
		     &hg->dss_pa);


  hg->fc_button_flip=gtk_check_button_new_with_label("Flip");
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_button_flip), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),hg->fc_button_flip,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),hg->dss_flip);
  my_signal_connect(hg->fc_button_flip,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_flip);

  hg->fcdb_frame = gtk_frame_new ("SIMBAD");
  if(hg->fcdb_type==FCDB_TYPE_SIMBAD){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "SIMBAD");
  }
  else if(hg->fcdb_type==FCDB_TYPE_NED){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "NED");
  }
  else if(hg->fcdb_type==FCDB_TYPE_GSC){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "GSC 2.3");
  }
  else if(hg->fcdb_type==FCDB_TYPE_PS1){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "PanSTARRS1");
  }
  else if(hg->fcdb_type==FCDB_TYPE_SDSS){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "SDSS DR14");
  }
  else if(hg->fcdb_type==FCDB_TYPE_LAMOST){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "LAMOST DR3");
  }
  else if(hg->fcdb_type==FCDB_TYPE_USNO){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "USNO-B");
  }
  else if(hg->fcdb_type==FCDB_TYPE_GAIA){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "GAIA DR1");
  }
  else if(hg->fcdb_type==FCDB_TYPE_2MASS){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "2MASS");
  }
  else if(hg->fcdb_type==FCDB_TYPE_WISE){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "AKARI/WISE");
  }
  else if(hg->fcdb_type==FCDB_TYPE_IRC){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "AKARI/IRC");
  }
  else if(hg->fcdb_type==FCDB_TYPE_FIS){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "AKARI/FIS");
  }
  else if(hg->fcdb_type==FCDB_TYPE_SMOKA){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "SMOKA");
  }
  else if(hg->fcdb_type==FCDB_TYPE_HST){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "HST");
  }
  else if(hg->fcdb_type==FCDB_TYPE_ESO){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "ESO");
  }
  else if(hg->fcdb_type==FCDB_TYPE_GEMINI){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "Gemini");
  }
  gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fcdb_frame), 3);

  table = gtk_table_new(3,2,FALSE);
  gtk_container_add (GTK_CONTAINER (hg->fcdb_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);

  label=gtk_label_new("  ");
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 0, 1,
  		    GTK_SHRINK,GTK_FILL,0,0);

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fcdb_item), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Query");
#endif

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach (GTK_TABLE(table), vbox1, 1, 2, 0, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);

  hg->fcdb_button=gtk_check_button_new_with_label("Disp");
  gtk_container_set_border_width (GTK_CONTAINER (hg->fcdb_button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), hg->fcdb_button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
			       hg->fcdb_flag);
  my_signal_connect(hg->fcdb_button,"toggled",
		    G_CALLBACK(fcdb_toggle), 
		    (gpointer)hg);

  button=gtk_check_button_new_with_label("Auto");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->fcdb_auto);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->fcdb_auto);

  label=gtk_label_new("  ");
  gtk_table_attach (GTK_TABLE(table), label, 2, 3, 0, 1,
  		    GTK_SHRINK,GTK_FILL,0,0);

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
  my_signal_connect (button, "clicked",
		     fcdb_para_item, (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 2, 3, 1, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Search Param.");
#endif
  
  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, TRUE, TRUE, 0);

  vbox1 = gtk_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 3);



  icon = gdk_pixbuf_new_from_inline(sizeof(spline_icon), spline_icon, 
				    FALSE, NULL);
  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->orbit_flag);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (orbit_fc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Draw Non-Sidereal Orbit");
#endif

  icon = gdk_pixbuf_new_from_inline(sizeof(hsc_icon), hsc_icon, 
				    FALSE, NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (set_hsc_dither), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "HSC Dithering Parameter");
#endif


  icon = gdk_pixbuf_new_from_inline(sizeof(icon_pdf), icon_pdf, 
				    FALSE, NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_fc_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PRINT);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_print_fc), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Print out");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_INFO);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (show_fc_help), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Show Help");
#endif
  gtk_widget_grab_focus (button);

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (close_fc), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Close");
#endif

  hg->fc_mag=1;
  hg->fc_magmode=0;
  hg->fc_ptn=0;

  // Drawing Area
  ebox=gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(hbox), ebox, TRUE, TRUE, 0);
  hg->fc_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->fc_dw, hg->sz_fc, hg->sz_fc);
  gtk_container_add(GTK_CONTAINER(ebox), hg->fc_dw);
  gtk_widget_set_app_paintable(hg->fc_dw, TRUE);

  gtk_widget_set_events(hg->fc_dw, GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK);
  my_signal_connect(hg->fc_dw, 
		    "configure-event", 
		    configure_draw_fc,
		    (gpointer)hg);
  my_signal_connect(hg->fc_dw, 
		    "expose-event", 
		    expose_draw_fc,
		    (gpointer)hg);
  
  gtk_widget_set_events(ebox, GDK_SCROLL_MASK |
                      GDK_BUTTON_PRESS_MASK);

  my_signal_connect(ebox,
		    "scroll-event", 
		    resize_draw_fc,
		    (gpointer)hg);
  my_signal_connect(ebox, 
		    "button-press-event", 
		    button_draw_fc,
		    (gpointer)hg);
		    
  gtk_widget_show(hg->fc_dw);

  gtk_widget_show_all(hg->fc_main);
  
  gdk_window_raise(hg->fc_main->window);

  draw_fc_cairo(hg->fc_dw,hg);
}


void create_fc_all_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *frame, *spinner;
  GtkWidget *hbox, *hbox2, *check, *table;
  GtkAdjustment *adj;
  
  dialog = gtk_dialog_new_with_buttons("HOE : Creating Finding Charts",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 


  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  frame = gtk_frame_new ("Source");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table = gtk_table_new(5,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    GtkWidget *bar;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Red)",
		       1, FC_STSCI_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Blue)",
		       1, FC_STSCI_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Red)",
		       1, FC_STSCI_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Blue)",
		       1, FC_STSCI_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (IR)",
		       1, FC_STSCI_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP1,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS1 (Red)",
		       1, FC_ESO_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS1R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Red)",
		       1, FC_ESO_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Blue)",
		       1, FC_ESO_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2B) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (IR)",
		       1, FC_ESO_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP2, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Far UV)",
		       1, FC_SKYVIEW_GALEXF, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXF) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Near UV)",
		       1, FC_SKYVIEW_GALEXN, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXN) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Red)",
		       1, FC_SKYVIEW_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Blue)",
		       1, FC_SKYVIEW_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Red)",
		       1, FC_SKYVIEW_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Blue)",
		       1, FC_SKYVIEW_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (IR)",
		       1, FC_SKYVIEW_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2IR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (u)",
		       1, FC_SKYVIEW_SDSSU, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSU) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (g)",
		       1, FC_SKYVIEW_SDSSG, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (r)",
		       1, FC_SKYVIEW_SDSSR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (i)",
		       1, FC_SKYVIEW_SDSSI, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSI) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (z)",
		       1, FC_SKYVIEW_SDSSZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (J)",
		       1, FC_SKYVIEW_2MASSJ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (H)",
		       1, FC_SKYVIEW_2MASSH, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (K)",
		       1, FC_SKYVIEW_2MASSK, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSK) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (3.4um)",
		       1, FC_SKYVIEW_WISE34, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE34) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (4.6um)",
		       1, FC_SKYVIEW_WISE46, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE46) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (12um)",
		       1, FC_SKYVIEW_WISE12, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE12) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (22um)",
		       1, FC_SKYVIEW_WISE22, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE22) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI N60",
		       1, FC_SKYVIEW_AKARIN60, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIN60) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI WIDE-S",
		       1, FC_SKYVIEW_AKARIWS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIWS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI WIDE-L",
		       1, FC_SKYVIEW_AKARIWL, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIWL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: AKARI N160",
		       1, FC_SKYVIEW_AKARIN160, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_AKARIN160) iter_set=iter;
	
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP3, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR7 (color)",
		       1, FC_SDSS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR13 (color)",
		       1, FC_SDSS13, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS13) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP4, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (color)",
		       1, FC_PANCOL, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANCOL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (g)",
		       1, FC_PANG, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (r)",
		       1, FC_PANR, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (i)",
		       1, FC_PANI, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANI) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (z)",
		       1, FC_PANZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANZ) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (y)",
		       1, FC_PANY, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANY) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    //gtk_container_add (GTK_CONTAINER (hbox2), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_mode_pdf,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Size [\']");
  gtk_table_attach (GTK_TABLE(table), frame, 2, 3, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->dss_arcmin,
					    DSS_ARCMIN_MIN, DSS_ARCMIN_MAX,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->dss_arcmin);


  hg->fc_frame_col_pdf = gtk_frame_new ("Scale");
  gtk_table_attach (GTK_TABLE(table), hg->fc_frame_col_pdf, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_frame_col_pdf), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_frame_col_pdf), hbox2);

  set_fc_frame_col_pdf(hg);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Linear",
		       1, FC_SCALE_LINEAR, -1);
    if(hg->dss_scale==FC_SCALE_LINEAR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Log",
		       1, FC_SCALE_LOG, -1);
    if(hg->dss_scale==FC_SCALE_LOG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Sqrt",
		       1, FC_SCALE_SQRT, -1);
    if(hg->dss_scale==FC_SCALE_SQRT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HistEq",
		       1, FC_SCALE_HISTEQ, -1);
    if(hg->dss_scale==FC_SCALE_HISTEQ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "LogLog",
		       1, FC_SCALE_LOGLOG, -1);
    if(hg->dss_scale==FC_SCALE_LOGLOG) iter_set=iter;
	
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->dss_scale);
  }

  button=gtk_check_button_new_with_label("Inv");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_invert);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_invert);

  frame = gtk_frame_new ("Instrument");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "None",
		       1, FC_INST_NONE, -1);
    if(hg->fc_inst==FC_INST_NONE) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS",
		       1, FC_INST_HDS, -1);
    if(hg->fc_inst==FC_INST_HDS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (w/oImR)",
		       1, FC_INST_HDSAUTO, -1);
    if(hg->fc_inst==FC_INST_HDSAUTO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (Zenith)",
		       1, FC_INST_HDSZENITH, -1);
    if(hg->fc_inst==FC_INST_HDSZENITH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IRCS",
		       1, FC_INST_IRCS, -1);
    if(hg->fc_inst==FC_INST_IRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "COMICS",
		       1, FC_INST_COMICS, -1);
    if(hg->fc_inst==FC_INST_COMICS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FOCAS",
		       1, FC_INST_FOCAS, -1);
    if(hg->fc_inst==FC_INST_FOCAS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MOIRCS",
		       1, FC_INST_MOIRCS, -1);
    if(hg->fc_inst==FC_INST_MOIRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FMOS",
		       1, FC_INST_FMOS, -1);
    if(hg->fc_inst==FC_INST_FMOS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SupCam",
		       1, FC_INST_SPCAM, -1);
    if(hg->fc_inst==FC_INST_SPCAM) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (Det-ID)",
		       1, FC_INST_HSCDET, -1);
    if(hg->fc_inst==FC_INST_HSCDET) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (HSCA)",
		       1, FC_INST_HSCA, -1);
    if(hg->fc_inst==FC_INST_HSCA) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->fc_inst);
  }

  button=gtk_check_button_new_with_label("Detail");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_table_attach (GTK_TABLE(table), button, 2, 3, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_draw_slit);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_draw_slit);



  frame = gtk_frame_new ("PA [deg]");
  gtk_table_attach (GTK_TABLE(table), frame, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->dss_pa,
					    -360, 360,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->dss_pa);


  button=gtk_check_button_new_with_label("Flip");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_flip);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_flip);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  button=gtk_check_button_new_with_label("Skip objects w/Mag < 10 (for Service programs)");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->fc_all_magskip);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->fc_all_magskip);
  

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    fc_dl_draw_all(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}



void close_fc(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;


  gtk_widget_destroy(GTK_WIDGET(hg->fc_main));
  flagFC=FALSE;
}


static void cancel_fc(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

#ifdef USE_WIN32
  if(hg->dwThreadID_dss){
    PostThreadMessage(hg->dwThreadID_dss, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_dss, INFINITE);
    CloseHandle(hg->hThread_dss);
    gtk_main_quit();
  }
#else
  if(fc_pid){
    kill(fc_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    fc_pid=0;
  }
#endif
}

#ifndef USE_WIN32
static void cancel_fc_all(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;
  hg=(typHOE *)gdata;

  flag_dssall_kill=TRUE;

  if(fc_pid){
    kill(fc_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
    fc_pid=0;
  }
  else{
    gtk_main_quit();
  }
}
#endif


void draw_fc_obj(typHOE *hg, cairo_t *cr, gint width, gint height){
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  gint from_set, to_rise;
  int width_file, height_file;
  gfloat r_w,r_h, r;
  GdkPixbuf *pixbuf_flip=NULL;

  gdouble ra_0, dec_0;
  gchar tmp[2048];
  gfloat x_ccd, y_ccd, gap_ccd;
  struct ln_hms ra_hms;
  struct ln_dms dec_dms;


  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);



  if(pixbuf_fc){
    width_file = gdk_pixbuf_get_width(pixbuf_fc);
    height_file = gdk_pixbuf_get_height(pixbuf_fc);
    
    r_w =  (gfloat)width/(gfloat)width_file;
    r_h =  (gfloat)height/(gfloat)height_file;
    
    if(pixbuf2_fc) g_object_unref(G_OBJECT(pixbuf2_fc));
    
    if(r_w>r_h){
      r=r_h;
    }
    else{
      r=r_w;
    }
    
    if(hg->dss_flip){
      pixbuf_flip=gdk_pixbuf_flip(pixbuf_fc,TRUE);
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_flip,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
      g_object_unref(G_OBJECT(pixbuf_flip));
    }
    else{
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_fc,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
    }

      
    cairo_save (cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    cairo_translate (cr, (gdouble)width_file*r/2,
		     (gdouble)height_file*r/2);

    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_IRCS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }

      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }

    }
    cairo_translate (cr, -(gdouble)width_file*r/2,
		     -(gdouble)height_file*r/2);
    gdk_cairo_set_source_pixbuf(cr, pixbuf2_fc, 0, 0);
    
    cairo_rectangle(cr, 0,0,
		    (gint)((gdouble)width_file*r),
		    (gint)((gdouble)height_file*r));
    cairo_fill(cr);

    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    switch(hg->fc_inst){
    case FC_INST_HDS:
      if(hg->dss_draw_slit){
	cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*HDS_SLIT_MASK_ARCSEC);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*(gdouble)hg->setup[0].slit_length/2./500./60.);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*(gdouble)hg->setup[0].slit_length/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_stroke(cr);
	
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*hg->setup[0].slit_width/500.);
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*(gdouble)hg->setup[0].slit_length/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*(gdouble)hg->setup[0].slit_length/2./500./60.);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);
      }
      
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0);
      
      cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		0,M_PI*2);
      cairo_stroke(cr);
      
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_set_line_width (cr, 1.5);
      cairo_line_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_stroke(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_show_text(cr, "HDS SV FOV (1arcmin)");
      
      break;


    case FC_INST_IRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);

      sprintf(tmp,"IRCS FOV (%dx%darcsec)",(gint)IRCS_X_ARCSEC, (gint)IRCS_Y_ARCSEC);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.-5);
      cairo_show_text(cr, tmp);

      break;


    case FC_INST_FOCAS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	
      sprintf(tmp,"FOCAS FOV (%darcmin)",FOCAS_R_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    -extents.width/2,
		    -FOCAS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5);
      cairo_show_text(cr, tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_arc(cr,0,0,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, FOCAS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);

	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	cairo_text_extents (cr,"Chip 2", &extents);

	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5,
		      -sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN-5);
	cairo_show_text(cr,"Chip 2");
	
	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5,
		      sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+extents.height+5);
	cairo_show_text(cr,"Chip 1");
      }

      break;


    case FC_INST_MOIRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);

      sprintf(tmp,"MOIRCS FOV (%dx%darcmin)",(gint)MOIRCS_X_ARCMIN, (gint)MOIRCS_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-5);
      cairo_show_text(cr, tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_rectangle(cr,
			-((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
			-((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
			(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
			(gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, MOIRCS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG1X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-MOIRCS_VIG1Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG2X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+MOIRCS_VIG2Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_new_path(cr);

	cairo_reset_clip(cr);

	cairo_set_line_width(cr,1.5);
	cairo_arc(cr,0,0,
		  (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.,
		  0,M_PI*2);
	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	cairo_text_extents (cr,"Detector 2", &extents);

	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+extents.height);
	cairo_show_text(cr,"Detector 2");
	
	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_show_text(cr,"Detector 1");

	cairo_rotate (cr,-M_PI/2);
	cairo_text_extents (cr,"6 arcmin from the center", &extents);
	cairo_move_to(cr,-extents.width/2.,
		      -(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.-5);
	cairo_show_text(cr,"6 arcmin from the center");
      }


      break;


    case FC_INST_SPCAM:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);

      sprintf(tmp,"Suprime-Cam FOV (%dx%darcmin)",SPCAM_X_ARCMIN, SPCAM_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.-5);
      cairo_show_text(cr, tmp);

      if(hg->dss_draw_slit){
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, 1.5);

	x_ccd=0.20/60.*2048.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	y_ccd=0.20/60.*4096.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	gap_ccd=SPCAM_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	//2 fio
	cairo_rectangle(cr,-x_ccd/2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,-x_ccd/2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//7 clarisse
	cairo_rectangle(cr,-x_ccd/2*3.-gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//9 san
	cairo_rectangle(cr,-x_ccd/2.*3.-gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//6 chihiro
	cairo_rectangle(cr,-x_ccd/2*5.-gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//8 ponyo
	cairo_rectangle(cr,-x_ccd/2.*5.-gap_ccd*2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//2 fio
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//0 nausicca
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//3 sophie
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2,+gap_ccd/2.,
			x_ccd,y_ccd);


	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	cairo_text_extents (cr,"2. fio", &extents);

	//2 fio
	cairo_move_to(cr,-x_ccd/2.+15,-y_ccd-gap_ccd/2.+15+extents.height);
	cairo_show_text(cr,"2. fio");

	//5 satsuki
	cairo_move_to(cr,-x_ccd/2.+15,+gap_ccd/2.+y_ccd-15);
	cairo_show_text(cr,"5. satsuki");

	//7 clarisse
	cairo_move_to(cr,-x_ccd/2*3.-gap_ccd+15,-y_ccd-gap_ccd/2.+15+extents.height);
	cairo_show_text(cr,"7. clarisse");

	//9 san
	cairo_move_to(cr,-x_ccd/2.*3.-gap_ccd+15,+gap_ccd/2.+y_ccd-15);
	cairo_show_text(cr,"9. san");

	//6 chihiro
	cairo_move_to(cr,-x_ccd/2*5.-gap_ccd*2.+15,-y_ccd-gap_ccd/2.+15+extents.height);
	cairo_show_text(cr,"6. chihiro");

	//8 ponyo
	cairo_move_to(cr,-x_ccd/2.*5.-gap_ccd*2.+15,+gap_ccd/2.+y_ccd-15);
	cairo_show_text(cr,"8. ponyo");

	//1 kiki
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15,-y_ccd-gap_ccd/2.+15+extents.height);
	cairo_show_text(cr,"1. kiki");

	//4 sheeta
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15,+gap_ccd/2.+y_ccd-15);
	cairo_show_text(cr,"4. sheeta");

	//0 nausicaa
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2.+15,-y_ccd-gap_ccd/2.+15+extents.height);
	cairo_show_text(cr,"0. nausicaa");

	//3 sophie
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2+15,+gap_ccd/2.+y_ccd-15);
	cairo_show_text(cr,"3. sophie");
      }

      break;
    }

    
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    ra_0=hg->obj[hg->dss_i].ra;
    ra_hms.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(ra_hms.hours)*10000;
    ra_hms.minutes=(gint)(ra_0/100);
    ra_hms.seconds=ra_0-(gdouble)(ra_hms.minutes)*100;
    
    if(hg->obj[hg->dss_i].dec<0){
      dec_dms.neg=1;
      dec_0=-hg->obj[hg->dss_i].dec;
    }
    else{
      dec_dms.neg=0;
      dec_0=hg->obj[hg->dss_i].dec;
    }
    dec_dms.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(dec_dms.degrees)*10000;
    dec_dms.minutes=(gint)(dec_0/100);
    dec_dms.seconds=dec_0-(gfloat)(dec_dms.minutes)*100;

    
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1);
    cairo_move_to(cr,5,(gdouble)height_file*r-5);
    sprintf(tmp,"RA=%02d:%02d:%05.2lf  Dec=%s%02d:%02d:%05.2lf",
	    ra_hms.hours,ra_hms.minutes,ra_hms.seconds,
	    (dec_dms.neg) ? "-" : "+", 
	    dec_dms.degrees, dec_dms.minutes,dec_dms.seconds);
    cairo_text_extents (cr, tmp, &extents);
    cairo_show_text(cr,tmp);

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr,5,(gdouble)height_file*r-5-extents.height-5);
    cairo_show_text(cr,hg->obj[hg->dss_i].name);



    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    switch(hg->fc_mode){
    case FC_SKYVIEW_GALEXF:
      sprintf(tmp,"GALEX (Far UV)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_GALEXN:
      sprintf(tmp,"GALEX (Near UV)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1R:
    case FC_ESO_DSS1R:
    case FC_SKYVIEW_DSS1R:
      sprintf(tmp,"DSS1 (Red)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1B:
    case FC_SKYVIEW_DSS1B:
      sprintf(tmp,"DSS1 (Blue)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2R:
    case FC_ESO_DSS2R:
    case FC_SKYVIEW_DSS2R:
      sprintf(tmp,"DSS2 (Red)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2B:
    case FC_ESO_DSS2B:
    case FC_SKYVIEW_DSS2B:
      sprintf(tmp,"DSS2 (Blue)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2IR:
    case FC_ESO_DSS2IR:
    case FC_SKYVIEW_DSS2IR:
      sprintf(tmp,"DSS2 (IR)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSU:
      sprintf(tmp,"SDSS (u)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSG:
      sprintf(tmp,"SDSS (g)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSR:
      sprintf(tmp,"SDSS (r)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSI:
      sprintf(tmp,"SDSS (i)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSZ:
      sprintf(tmp,"SDSS (z)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSJ:
      sprintf(tmp,"2MASS (J)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSH:
      sprintf(tmp,"2MASS (H)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSK:
      sprintf(tmp,"2MASS (K)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE34:
      sprintf(tmp,"WISE (3.4um)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE46:
      sprintf(tmp,"WISE (4.6um)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE12:
      sprintf(tmp,"WISE (12um)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE22:
      sprintf(tmp,"WISE (22um)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIN60:
      sprintf(tmp,"AKARI N60  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIWS:
      sprintf(tmp,"AKARI WIDE-S  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIWL:
      sprintf(tmp,"AKARI WIDE-L  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIN160:
      sprintf(tmp,"AKARI N160  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_NVSS:
      sprintf(tmp,"NVSS (1.4GHz)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SDSS:
      sprintf(tmp,"SDSS DR7 (color)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_SDSS13:
      sprintf(tmp,"SDSS DR13 (color)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANCOL:
      sprintf(tmp,"PanSTARRS-1 (color)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANG:
      sprintf(tmp,"PanSTARRS-1 (g)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANR:
      sprintf(tmp,"PanSTARRS-1 (r)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANI:
      sprintf(tmp,"PanSTARRS-1 (i)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANZ:
      sprintf(tmp,"PanSTARRS-1 (z)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANY:
      sprintf(tmp,"PanSTARRS-1 (y)  %dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    default:
      sprintf(tmp,"%dx%d arcmin",
	      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
    }
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,
		  (gdouble)width_file*r-extents.width-5,
		  extents.height+5);
    cairo_show_text(cr,tmp);

    cairo_restore(cr);


    cairo_save (cr);

    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1);
    cairo_text_extents (cr, "N", &extents);

    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    cairo_translate (cr, 
		     5+(gdouble)width_file*r*0.05+extents.width*1.5,
		     5+(gdouble)width_file*r*0.05+extents.height*1.5);
    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_IRCS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }
      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      break;
    }

    
    cairo_move_to(cr,
		  -extents.width/2,
		  -(gdouble)width_file*r*0.05);
    cairo_show_text(cr,"N");
    cairo_move_to(cr,
		  -(gdouble)width_file*r*0.05-extents.width,
		  +extents.height/2);
    if(hg->dss_flip){
      cairo_show_text(cr,"W");
    }
    else{
      cairo_show_text(cr,"E");
    }

    cairo_set_line_width (cr, 1.5);
    cairo_move_to(cr,
		  0,
		  -(gdouble)width_file*r*0.05);
    cairo_line_to(cr, 0, 0);
    cairo_line_to(cr,
		  -(gdouble)width_file*r*0.05, 0);
    cairo_stroke(cr);

    if(hg->dss_flip){
      cairo_move_to(cr,0,0);
      cairo_text_extents (cr, "(flipped)", &extents);
      cairo_rel_move_to(cr,-extents.width/2.,extents.height+5);
      cairo_show_text(cr,"(flipped)");
    }

    
    cairo_restore(cr);

  }

  if(hg->fc_output!=FC_OUTPUT_WINDOW){
    cairo_show_page(cr); 
  }
}

void translate_to_center(cairo_t *cr, int width, int height, int width_file, int height_file, gfloat r, typHOE *hg)
{
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    cairo_translate (cr, (gdouble)width_file*r/2,
		     (gdouble)height_file*r/2);

    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_IRCS:
    case FC_INST_COMICS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
    case FC_INST_FMOS:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }

      break;

    case FC_INST_HDSAUTO:
      if(hg->skymon_mode==SKYMON_SET){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].s_hpa));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].c_hpa));
      }
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }
      break;

    case FC_INST_HDSZENITH:
      if(hg->skymon_mode==SKYMON_SET){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].s_pa));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].c_pa));
      }
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }
      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(270-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(270-hg->dss_pa)/180.);
      }
      break;
    }
}

void rot_pa(cairo_t *cr, typHOE *hg){
  switch(hg->fc_inst){
  case FC_INST_NONE:
  case FC_INST_HDS:
  case FC_INST_HDSAUTO:
  case FC_INST_HDSZENITH:
  case FC_INST_IRCS:
  case FC_INST_COMICS:
  case FC_INST_FOCAS:
  case FC_INST_MOIRCS:
  case FC_INST_FMOS:
    if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
    }
    break;
    
  case FC_INST_SPCAM:
    if(hg->dss_flip){
      cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
    }
    break;
  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    if(hg->dss_flip){
      cairo_rotate (cr,-M_PI*(gdouble)(270-hg->dss_pa)/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)(270-hg->dss_pa)/180.);
    }
    break;
  }
}

void translate_hsc_dith(cairo_t *cr, typHOE *hg, int width_file, gfloat r){
  gdouble dra, ddec, x0, y0, x1, y1, theta;
  
  if((hg->fc_inst==FC_INST_HSCA)||(hg->fc_inst==FC_INST_HSCDET)){
    dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_offra/60.;
    ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_offdec/60.;
    theta=M_PI*(gdouble)(270-hg->dss_pa)/180.;
    
    if(hg->dss_flip){
      cairo_translate(cr, 
		      -(dra*cos(-theta)-ddec*sin(-theta)),
		      -(dra*sin(-theta)+ddec*cos(-theta)));
    }
    else{
      cairo_translate(cr, 
		      -(dra*cos(theta)-ddec*sin(theta)),
		      -(dra*sin(theta)+ddec*cos(theta)));
    }
    

    switch(hg->hsc_dithp){
    case HSC_DITH_5:
      dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(gdouble)hg->hsc_dra/60.;
      ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(gdouble)hg->hsc_ddec/60.;
      switch(hg->hsc_dithi){
      case 1:
	x0=0;
	y0=0;
	break;

      case 2:
	x0=+ddec*2;
	y0=+dra*1;
	break;
	
      case 3:
	x0=-ddec*1;
	y0=+dra*2;
	break;
      
      case 4:
	x0=-ddec*2;
	y0=-dra*1;
	break;
	
      case 5:
	x0=+ddec*1;
	y0=-dra*2;
	break;
	
      }
      
      x1=x0*cos(-(gdouble)hg->dss_pa*M_PI/180)
	-y0*sin(-(gdouble)hg->dss_pa*M_PI/180);
      y1=x0*sin(-(gdouble)hg->dss_pa*M_PI/180)
	+y0*cos(-(gdouble)hg->dss_pa*M_PI/180);

      cairo_translate(cr, x1, y1);
      break;

    case HSC_DITH_N:
      y0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(gdouble)hg->hsc_rdith/60*
	cos(-(gdouble)hg->hsc_tdith*M_PI/180
	    -2*M_PI/(gdouble)(hg->hsc_ndith)*(gdouble)(hg->hsc_dithi-1));
      x0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(gdouble)hg->hsc_rdith/60*
	sin(-(gdouble)hg->hsc_tdith*M_PI/180
	    -2*M_PI/(gdouble)(hg->hsc_ndith)*(gdouble)(hg->hsc_dithi-1));

      x1=x0*cos(-(gdouble)hg->dss_pa*M_PI/180)
	-y0*sin(-(gdouble)hg->dss_pa*M_PI/180);
      y1=x0*sin(-(gdouble)hg->dss_pa*M_PI/180)
	+y0*cos(-(gdouble)hg->dss_pa*M_PI/180);

      cairo_translate(cr, x1, y1);
      break;
    }
  }
}


gboolean draw_fc_cairo(GtkWidget *widget, typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  GdkPixmap *pixmap_fcbk;
  gint from_set, to_rise;
  int width, height;
  int width_file, height_file;
  gfloat r_w,r_h, r;
  gint shift_x, shift_y;

  gchar *tmp;
  GdkPixbuf *pixbuf_flip=NULL;
  gfloat x_ccd, y_ccd, gap_ccd;
  gdouble scale;

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  
  if(!flagFC) return (FALSE);

  if(hg->fc_output==FC_OUTPUT_PDF){
    width= hg->sz_plot;
    height= hg->sz_plot;
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else if (hg->fc_output==FC_OUTPUT_PRINT){
    width =  (gint)gtk_print_context_get_width(hg->context);
    height =  (gint)gtk_print_context_get_height(hg->context);
#ifdef USE_WIN32
    scale=(gdouble)width/(gint)(hg->sz_plot*1.5)
      *(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);
#else
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);
#endif

    cr = gtk_print_context_get_cairo_context (hg->context);

    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else{
    width= widget->allocation.width*hg->fc_mag;
    height= widget->allocation.height*hg->fc_mag;
    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->fc_main), &width, &height);
    }
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);

    pixmap_fcbk = gdk_pixmap_new(widget->window,
				 width,
				 height,
				 -1);
  
    cr = gdk_cairo_create(pixmap_fcbk);

    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
    }
  }

  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if(!pixbuf_fc){
    gdouble l_h;

    cairo_rectangle(cr, 0,0,
		    width,
		    height);
    if(hg->fc_output==FC_OUTPUT_WINDOW){
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    }
    cairo_fill(cr);

    if(hg->fc_output==FC_OUTPUT_WINDOW){
      cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
    }
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    tmp=g_strdup("Error : Failed to load the image for the finding chart!");
    cairo_text_extents (cr,tmp, &extents);
    l_h=extents.height;
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2-l_h*1.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
 
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    tmp=g_strdup("The position might be out of the surveyed area.");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*1.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    tmp=g_strdup("or");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*3);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    tmp=g_strdup("An HTTP error might be occured in the server side.");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*4.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
  }
  else{
    width_file = gdk_pixbuf_get_width(pixbuf_fc);
    height_file = gdk_pixbuf_get_height(pixbuf_fc);

    
    r_w =  (gfloat)width/(gfloat)width_file;
    r_h =  (gfloat)height/(gfloat)height_file;
    
    if(pixbuf2_fc) g_object_unref(G_OBJECT(pixbuf2_fc));
    
    if(r_w>r_h){
      r=r_h;
    }
    else{
      r=r_w;
    }
    
    if(hg->dss_flip){
      pixbuf_flip=gdk_pixbuf_flip(pixbuf_fc,TRUE);
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_flip,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
      g_object_unref(G_OBJECT(pixbuf_flip));
    }
    else{
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_fc,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
    }

    cairo_save (cr);

    translate_to_center(cr,width,height,width_file,height_file,r,hg);

    cairo_translate (cr, -(gdouble)width_file*r/2,
		     -(gdouble)height_file*r/2);
    gdk_cairo_set_source_pixbuf(cr, pixbuf2_fc, 0, 0);
    
    cairo_rectangle(cr, 0,0,
		    (gint)((gdouble)width_file*r),
		    (gint)((gdouble)height_file*r));
    cairo_fill(cr);

    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width (cr, 1.0*scale);
      cairo_rectangle(cr, 0,0,
		      (gint)((gdouble)width_file*r),
		      (gint)((gdouble)height_file*r));
      cairo_stroke(cr);
    }

    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    switch(hg->fc_inst){
    case FC_INST_HDS:
    case FC_INST_HDSAUTO:
    case FC_INST_HDSZENITH:
      if(hg->dss_draw_slit){
	cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.3);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*HDS_SLIT_MASK_ARCSEC);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_stroke(cr);
	
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*HDS_SLIT_WIDTH/500.);
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);
      }
      
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		0,M_PI*2);
      cairo_stroke(cr);
      
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_set_line_width (cr, 1.5*scale);
      cairo_line_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_stroke(cr);
      
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_show_text(cr, "HDS SV FOV (1arcmin)");
      
      break;


    case FC_INST_IRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);

      if(hg->dss_draw_slit){
	cairo_set_line_width (cr, 1.5*scale);
	cairo_arc(cr,0,0,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*IRCS_TTGS_ARCMIN,
		  0,M_PI*2);
	cairo_stroke(cr);

	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
	tmp=g_strdup_printf("Tip-Tilt Guide Star w/LGS (%darcmin)",IRCS_TTGS_ARCMIN/2);
	cairo_text_extents (cr,tmp, &extents);
	cairo_move_to(cr,
		      -extents.width/2,
		      -IRCS_TTGS_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }

      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("IRCS FOV (%dx%darcsec)",(gint)IRCS_X_ARCSEC, (gint)IRCS_Y_ARCSEC);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;


    case FC_INST_COMICS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_X_ARCSEC/60.)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_X_ARCSEC/60.,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("COMICS FOV (%dx%darcsec)",(gint)COMICS_X_ARCSEC, (gint)COMICS_Y_ARCSEC);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;


    case FC_INST_FOCAS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      tmp=g_strdup_printf("FOCAS FOV (%darcmin)",FOCAS_R_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    -extents.width/2,
		    -FOCAS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_arc(cr,0,0,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, FOCAS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"Chip 2", &extents);

	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5*scale,
		      -sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN-5*scale);
	cairo_show_text(cr,"Chip 2");
	
	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5*scale,
		      sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+extents.height+5*scale);
	cairo_show_text(cr,"Chip 1");
      }

      break;


    case FC_INST_MOIRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("MOIRCS FOV (%dx%darcmin)",(gint)MOIRCS_X_ARCMIN, (gint)MOIRCS_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_rectangle(cr,
			-((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
			-((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
			(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
			(gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, MOIRCS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG1X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-MOIRCS_VIG1Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG2X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+MOIRCS_VIG2Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_new_path(cr);

	cairo_reset_clip(cr);

	cairo_set_line_width(cr,1.5*scale);
	cairo_arc(cr,0,0,
		  (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.,
		  0,M_PI*2);
	cairo_stroke(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"Detector 2", &extents);

	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5*scale,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+extents.height);
	cairo_show_text(cr,"Detector 2");
	
	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5*scale,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_show_text(cr,"Detector 1");

	cairo_rotate (cr,-M_PI/2);
	cairo_text_extents (cr,"6 arcmin from the center", &extents);
	cairo_move_to(cr,-extents.width/2.,
		      -(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.-5*scale);
	cairo_show_text(cr,"6 arcmin from the center");
      }


      break;


    case FC_INST_SPCAM:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("Suprime-Cam FOV (%dx%darcmin)",SPCAM_X_ARCMIN, SPCAM_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.3);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, 1.5*scale);

	x_ccd=0.20/60.*2048.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	y_ccd=0.20/60.*4096.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	gap_ccd=SPCAM_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	//2 fio
	cairo_rectangle(cr,-x_ccd/2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,-x_ccd/2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//7 clarisse
	cairo_rectangle(cr,-x_ccd/2*3.-gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//9 san
	cairo_rectangle(cr,-x_ccd/2.*3.-gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//6 chihiro
	cairo_rectangle(cr,-x_ccd/2*5.-gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//8 ponyo
	cairo_rectangle(cr,-x_ccd/2.*5.-gap_ccd*2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//2 fio
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//0 nausicca
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//3 sophie
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2,+gap_ccd/2.,
			x_ccd,y_ccd);


	cairo_stroke(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"2. fio", &extents);

	//2 fio
	cairo_move_to(cr,-x_ccd/2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"2. fio");

	//5 satsuki
	cairo_move_to(cr,-x_ccd/2.+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"5. satsuki");

	//7 clarisse
	cairo_move_to(cr,-x_ccd/2*3.-gap_ccd+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"7. clarisse");

	//9 san
	cairo_move_to(cr,-x_ccd/2.*3.-gap_ccd+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"9. san");

	//6 chihiro
	cairo_move_to(cr,-x_ccd/2*5.-gap_ccd*2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"6. chihiro");

	//8 ponyo
	cairo_move_to(cr,-x_ccd/2.*5.-gap_ccd*2.+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"8. ponyo");

	//1 kiki
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"1. kiki");

	//4 sheeta
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"4. sheeta");

	//0 nausicaa
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"0. nausicaa");

	//3 sophie
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"3. sophie");
      }

      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      translate_hsc_dith(cr, hg, width_file, r);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*HSC_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      if(!hg->dss_draw_slit){
	tmp=g_strdup_printf("HSC FOV (%darcmin)",HSC_R_ARCMIN);
	cairo_text_extents (cr,tmp, &extents);
	cairo_move_to(cr,
		      -extents.width/2,
		      -HSC_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
      else{
	gint i_chip;
	gdouble pscale;
	gdouble x_0,y_0;
	
	pscale=(1.5*60.*60./(497./0.015))/60.*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	// HSC pix scale 1.5deg = 497mm phi

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, 0.8*scale);

	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);

	// Dead chips
	{
	  gint i_dead;
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);

	  for(i_dead=0;i_dead<HSC_DEAD_ALL;i_dead++){
	  
	    y_0=(-(gdouble)hsc_dead[i_dead].crpix1*(gdouble)hsc_dead[i_dead].cd1_1/0.015-(gdouble)hsc_dead[i_dead].crpix2*(gdouble)hsc_dead[i_dead].cd1_2/0.015)*pscale;
	    x_0=(-(gdouble)hsc_dead[i_dead].crpix1*(gdouble)hsc_dead[i_dead].cd2_1/0.015-(gdouble)hsc_dead[i_dead].crpix2*(gdouble)hsc_dead[i_dead].cd2_2/0.015)*pscale;
	    if((hsc_dead[i_dead].cd1_2<0)&&(hsc_dead[i_dead].cd2_1<0)){
	      cairo_rectangle(cr, x_0-2048*pscale/4*(hsc_dead[i_dead].ch),
			      y_0-4224*pscale, 2048*pscale/4, 4224*pscale );
	    }
	    else if((hsc_dead[i_dead].cd1_2>0)&&(hsc_dead[i_dead].cd2_1>0)){
	      cairo_rectangle(cr,x_0+2048*pscale/4*(hsc_dead[i_dead].ch-1), y_0, 2048*pscale/4, 4224*pscale);
	    }
	    else if((hsc_dead[i_dead].cd1_1>0)&&(hsc_dead[i_dead].cd2_2<0)){
	      cairo_rectangle(cr,x_0-4224*pscale, y_0+2048*pscale/4*(hsc_dead[i_dead].ch-1),  4224*pscale, 2048*pscale/4);
	    }
	    else{
	      cairo_rectangle(cr,x_0, y_0-2048*pscale/4*(hsc_dead[i_dead].ch), 4224*pscale, 2048*pscale/4);
	    }
	    cairo_fill(cr);
	  }
	}

	for(i_chip=0;i_chip<HSC_CHIP_ALL;i_chip++){

	  if(hsc_param[i_chip].bees==2){
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.6, 0.0, 0.6);
	    else cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.6);
	  }
	  else if(hsc_param[i_chip].bees==0){
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.0, 0.5, 0.6);
	    else cairo_set_source_rgba(cr, 0.8, 0.4, 0.8, 0.6);
	  }
	  else{
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	    else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	  }
	  
	  cairo_set_font_size (cr, 600*pscale);
	  cairo_text_extents (cr,"000", &extents);

	  y_0=(-(gdouble)hsc_param[i_chip].crpix1*(gdouble)hsc_param[i_chip].cd1_1/0.015-(gdouble)hsc_param[i_chip].crpix2*(gdouble)hsc_param[i_chip].cd1_2/0.015)*pscale;
	  x_0=(-(gdouble)hsc_param[i_chip].crpix1*(gdouble)hsc_param[i_chip].cd2_1/0.015-(gdouble)hsc_param[i_chip].crpix2*(gdouble)hsc_param[i_chip].cd2_2/0.015)*pscale;

	  if((hsc_param[i_chip].cd1_2<0)&&(hsc_param[i_chip].cd2_1<0)){
	    cairo_rectangle(cr, x_0-2048*pscale, y_0-4224*pscale, 2048*pscale, 4224*pscale );
	    cairo_move_to(cr, x_0-2048*pscale+2044*pscale*0.05, y_0-4224*pscale+2044*pscale*0.05-extents.y_bearing);
	  }
	  else if((hsc_param[i_chip].cd1_2>0)&&(hsc_param[i_chip].cd2_1>0)){
	    cairo_rectangle(cr,x_0, y_0, 2048*pscale, 4224*pscale);
	    cairo_move_to(cr, x_0+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	  }
	  else if((hsc_param[i_chip].cd1_1>0)&&(hsc_param[i_chip].cd2_2<0)){
	    cairo_rectangle(cr,x_0-4224*pscale, y_0,  4224*pscale, 2048*pscale);
	    cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	  }
	  else{
	    cairo_rectangle(cr,x_0, y_0-2048*pscale, 4224*pscale, 2048*pscale );
	    cairo_move_to(cr, x_0+2048*pscale*0.05, y_0-2048*pscale+2048*pscale*0.05-extents.y_bearing);
	  }

	  cairo_set_font_size (cr, 600*pscale);
	  if(hg->fc_inst==FC_INST_HSCDET){
	    tmp=g_strdup_printf("%d",hsc_param[i_chip].det_id);
	  }
	  else{
	    
	    tmp=g_strdup_printf("%02d",hsc_param[i_chip].hsca);
	  }
	  cairo_show_text(cr,tmp);
	  if(tmp) g_free(tmp);

	  if(hsc_param[i_chip].hsca==35){
	    cairo_set_font_size (cr, 1600*pscale);
	    tmp=g_strdup_printf("BEES%d",hsc_param[i_chip].bees);
	    cairo_text_extents (cr,tmp, &extents);
	  
	    if(hsc_param[i_chip].bees==0){
	      cairo_move_to(cr, x_0+4224*pscale-2048*pscale*0.5-extents.width, y_0+2048*pscale*0.2-extents.y_bearing);
	    }
	    else{
	      cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.5, y_0-2048*pscale*0.2);
	    }
	    cairo_show_text(cr,tmp);
	    if(tmp) g_free(tmp);
	  }

	  cairo_stroke(cr);

	}
      }
      break;

    case FC_INST_FMOS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FMOS_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      tmp=g_strdup_printf("FMOS FOV (%darcmin)",FMOS_R_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    -extents.width/2,
		    -FMOS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;
    }

    
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    
    object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
    object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

    ln_equ_to_hequ(&object, &hobject);

    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.3, 0.45, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
    cairo_move_to(cr,5*scale,(gdouble)height_file*r-5*scale);
    tmp=g_strdup_printf("RA=%02d:%02d:%05.2lf  Dec=%s%02d:%02d:%05.2lf (%.1lf)",
			hobject.ra.hours,hobject.ra.minutes,
			hobject.ra.seconds,
			(hobject.dec.neg) ? "-" : "+", 
			hobject.dec.degrees, hobject.dec.minutes,
			hobject.dec.seconds,
			hg->obj[hg->dss_i].equinox);
    cairo_text_extents (cr, tmp, &extents);
    cairo_show_text(cr,tmp);
    if(tmp) g_free(tmp);

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr,5*scale,(gdouble)height_file*r-5*scale-extents.height-5*scale);
    cairo_show_text(cr,hg->obj[hg->dss_i].name);


    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    switch(hg->fc_mode_get){
    case FC_SKYVIEW_GALEXF:
      tmp=g_strdup_printf("GALEX (Far UV)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_GALEXN:
      tmp=g_strdup_printf("GALEX (Near UV)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1R:
    case FC_ESO_DSS1R:
    case FC_SKYVIEW_DSS1R:
      tmp=g_strdup_printf("DSS1 (Red)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1B:
    case FC_SKYVIEW_DSS1B:
      tmp=g_strdup_printf("DSS1 (Blue)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2R:
    case FC_ESO_DSS2R:
    case FC_SKYVIEW_DSS2R:
      tmp=g_strdup_printf("DSS2 (Red)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2B:
    case FC_ESO_DSS2B:
    case FC_SKYVIEW_DSS2B:
      tmp=g_strdup_printf("DSS2 (Blue)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2IR:
    case FC_ESO_DSS2IR:
    case FC_SKYVIEW_DSS2IR:
      tmp=g_strdup_printf("DSS2 (IR)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_SKYVIEW_SDSSU:
      tmp=g_strdup_printf("SDSS (u)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSG:
      tmp=g_strdup_printf("SDSS (g)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSR:
      tmp=g_strdup_printf("SDSS (r)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSI:
      tmp=g_strdup_printf("SDSS (i)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSZ:
      tmp=g_strdup_printf("SDSS (z)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSJ:
      tmp=g_strdup_printf("2MASS (J)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSH:
      tmp=g_strdup_printf("2MASS (H)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSK:
      tmp=g_strdup_printf("2MASS (K)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE34:
      tmp=g_strdup_printf("WISE (3.4um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE46:
      tmp=g_strdup_printf("WISE (4.6um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE12:
      tmp=g_strdup_printf("WISE (12um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE22:
      tmp=g_strdup_printf("WISE (22um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIN60:
      tmp=g_strdup_printf("AKARI N60  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIWS:
      tmp=g_strdup_printf("AKARI WIDE-S  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIWL:
      tmp=g_strdup_printf("AKARI WIDE-L  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_AKARIN160:
      tmp=g_strdup_printf("AKARI N160  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_NVSS:
      tmp=g_strdup_printf("NVSS (1.4GHz)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SDSS:
      tmp=g_strdup_printf("SDSS DR7 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_SDSS13:
      tmp=g_strdup_printf("SDSS DR14 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANCOL:
      tmp=g_strdup_printf("PanSTARRS-1 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANG:
      tmp=g_strdup_printf("PanSTARRS-1 (g)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANR:
      tmp=g_strdup_printf("PanSTARRS-1 (r)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANI:
      tmp=g_strdup_printf("PanSTARRS-1 (i)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANZ:
      tmp=g_strdup_printf("PanSTARRS-1 (z)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANY:
      tmp=g_strdup_printf("PanSTARRS-1 (y)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    default:
      tmp=g_strdup_printf("%dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
    }
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,
		  (gdouble)width_file*r-extents.width-5*scale,
		  extents.height+5*scale);
    cairo_show_text(cr,tmp);
    if(tmp) g_free(tmp);

    
    cairo_restore(cr);
    cairo_save (cr);

    if((hg->fc_inst==FC_INST_HSCA)||(hg->fc_inst==FC_INST_HSCDET)){
      gint i;
      double x0, y0, dra, ddec, theta;

      translate_to_center(cr,width,height,width_file,height_file,r,hg);

      // Dithering

      switch(hg->hsc_dithp){
      case HSC_DITH_NO:
	break;

      case HSC_DITH_5:
	dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	  *(gdouble)hg->hsc_dra/60.;
	ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	  *(gdouble)hg->hsc_ddec/60.;

	// 1
	if(hg->hsc_dithi==1){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	  cairo_set_line_width (cr, 2.5*scale);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	  cairo_set_line_width (cr, 1.5*scale);
	}
	cairo_move_to(cr, 0, 0);
	cairo_rel_move_to(cr, (hg->hsc_dithi==1) ? -7.5 : -5, 0);
	cairo_rel_line_to(cr, (hg->hsc_dithi==1) ? 15 : 10, 0);
	cairo_stroke(cr);
	cairo_move_to(cr, 0, 0);
	cairo_rel_move_to(cr, 0, (hg->hsc_dithi==1) ? -7.5 : -5);
	cairo_rel_line_to(cr, 0, (hg->hsc_dithi==1) ? 15: 10);
	cairo_stroke(cr);
	
	// 2
	if(hg->hsc_dithi==2){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	  cairo_set_line_width (cr, 2.5*scale);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	  cairo_set_line_width (cr, 1.5*scale);
	}
	cairo_move_to(cr, -dra*1, +ddec*2);
	cairo_rel_move_to(cr, (hg->hsc_dithi==2) ? -7.5 : -5, 0);
	cairo_rel_line_to(cr, (hg->hsc_dithi==2) ? 15 : 10, 0);
	cairo_stroke(cr);
	cairo_move_to(cr, -dra*1, +ddec*2);
	cairo_rel_move_to(cr, 0, (hg->hsc_dithi==2) ? -7.5 : -5);
	cairo_rel_line_to(cr, 0, (hg->hsc_dithi==2) ? 15: 10);
	cairo_stroke(cr);

	// 3
	if(hg->hsc_dithi==3){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	  cairo_set_line_width (cr, 2.5*scale);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	  cairo_set_line_width (cr, 1.5*scale);
	}
	cairo_move_to(cr, -dra*2, -ddec*1);
	cairo_rel_move_to(cr, (hg->hsc_dithi==3) ? -7.5 : -5, 0);
	cairo_rel_line_to(cr, (hg->hsc_dithi==3) ? 15 : 10, 0);
	cairo_stroke(cr);
	cairo_move_to(cr, -dra*2, -ddec*1);
	cairo_rel_move_to(cr, 0, (hg->hsc_dithi==3) ? -7.5 : -5);
	cairo_rel_line_to(cr, 0, (hg->hsc_dithi==3) ? 15 : 10);
	cairo_stroke(cr);

	// 4
	if(hg->hsc_dithi==4){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	  cairo_set_line_width (cr, 2.5*scale);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	  cairo_set_line_width (cr, 1.5*scale);
	}
	cairo_move_to(cr, +dra*1, -ddec*2);
	cairo_rel_move_to(cr, (hg->hsc_dithi==4) ? -7.5 : -5, 0);
	cairo_rel_line_to(cr, (hg->hsc_dithi==4) ? 15 : 10, 0);
	cairo_stroke(cr);
	cairo_move_to(cr, +dra*1, -ddec*2);
	cairo_rel_move_to(cr, 0, (hg->hsc_dithi==4) ? -7.5 : -5);
	cairo_rel_line_to(cr, 0, (hg->hsc_dithi==4) ? 15 : 10);
	cairo_stroke(cr);

	// 5
	if(hg->hsc_dithi==5){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	  cairo_set_line_width (cr, 2.5*scale);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	  cairo_set_line_width (cr, 1.5*scale);
	}
	cairo_move_to(cr, +dra*2, +ddec*1);
	cairo_rel_move_to(cr, (hg->hsc_dithi==5) ? -7.5 : -5, 0);
	cairo_rel_line_to(cr, (hg->hsc_dithi==5) ? 15 : 10, 0);
	cairo_stroke(cr);
	cairo_move_to(cr, +dra*2, +ddec*1);
	cairo_rel_move_to(cr, 0, (hg->hsc_dithi==5) ? -7.5 : -5);
	cairo_rel_line_to(cr, 0, (hg->hsc_dithi==5) ? 15 : 10);
	cairo_stroke(cr);
	break;

      case HSC_DITH_N:
	for(i=0;i<hg->hsc_ndith;i++){
	  if(hg->hsc_dithi==i+1){
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	    else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	    cairo_set_line_width (cr, 2.5*scale);
	  }
	  else{
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	    cairo_set_line_width (cr, 1.5*scale);
	  }
	  
	  y0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	    *(double)hg->hsc_rdith/60*
	    cos(-(double)hg->hsc_tdith*M_PI/180
		-2*M_PI/(double)hg->hsc_ndith*(double)i-M_PI/2);
	  x0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	    *(double)hg->hsc_rdith/60*
	    sin(-(double)hg->hsc_tdith*M_PI/180
		-2*M_PI/(double)hg->hsc_ndith*(double)i-M_PI/2);

	  cairo_move_to(cr, x0, y0);
	  cairo_rel_move_to(cr, (hg->hsc_dithi==i+1) ? -7.5 : -5, 0);
	  cairo_rel_line_to(cr, (hg->hsc_dithi==i+1) ? 15 : 10, 0);
	  cairo_stroke(cr);
	  cairo_move_to(cr, x0, y0);
	  cairo_rel_move_to(cr, 0, (hg->hsc_dithi==i+1) ? -7.5 : -5);
	  cairo_rel_line_to(cr, 0, (hg->hsc_dithi==i+1) ? 15 : 10);
	  cairo_stroke(cr);
	}
	break;
      }
    }

    cairo_restore (cr);
    cairo_save (cr);

    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
    cairo_text_extents (cr, "N", &extents);

    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    cairo_translate (cr, 
		     5+(gdouble)width_file*r*0.05+extents.width*1.5,
		     5+(gdouble)width_file*r*0.05+extents.height*1.5);
    
    rot_pa(cr, hg);

    // Position Angle
    if(hg->fc_mag==1){
      cairo_move_to(cr,
		    -extents.width/2,
		    -(gdouble)width_file*r*0.05);
      cairo_show_text(cr,"N");
      cairo_move_to(cr,
		    -(gdouble)width_file*r*0.05-extents.width,
		    +extents.height/2);
      if(hg->dss_flip){
	cairo_show_text(cr,"W");
      }
      else{
	cairo_show_text(cr,"E");
      }
      
      cairo_set_line_width (cr, 1.5*scale*hg->fc_mag);
      cairo_move_to(cr,
		    0,
		    -(gdouble)width_file*r*0.05);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr,
		    -(gdouble)width_file*r*0.05, 0);
      
      cairo_stroke(cr);
      
      if(hg->dss_flip){
	cairo_move_to(cr,0,0);
	cairo_text_extents (cr, "(flipped)", &extents);
	cairo_rel_move_to(cr,-extents.width/2.,extents.height+5*scale);
	cairo_show_text(cr,"(flipped)");
      }
    } // Position Angle
    
    cairo_restore(cr);

   // Position Angle  for mag
    if(hg->fc_mag!=1){
      gdouble wh_small;
      gdouble xsec,ysec;
      gdouble pscale;

      cairo_save (cr);

      wh_small=(gdouble)(width>height?height:width)/(gdouble)hg->fc_mag;
      pscale=(gdouble)hg->dss_arcmin_ip*60./wh_small;
      xsec=(gdouble)width*pscale/(gdouble)hg->fc_mag/(gdouble)hg->fc_mag;
      ysec=(gdouble)height*pscale/(gdouble)hg->fc_mag/(gdouble)hg->fc_mag;
	
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);

      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      if((xsec>60.) && (ysec>60.)){
	tmp=g_strdup_printf("x%d : %.2lfx%.2lf arcmin",hg->fc_mag,
			    xsec/60.,
			    ysec/60.);
      }
      else{
	tmp=g_strdup_printf("x%d : %.1lfx%.1lf arcsec",hg->fc_mag,xsec,ysec);
      }

      // Edge for magnification
      {
	shift_x=-(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag);
	shift_y=-(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag);
      
	if(shift_x>0){
	  shift_x=0;
	}
	else if((width+shift_x)<widget->allocation.width){
	  shift_x=widget->allocation.width-width;
	}

	if(shift_y>0){
	  shift_y=0;
	}
	else if((height+shift_y)<widget->allocation.height){
	  shift_y=widget->allocation.height-height;
	}
      }

      cairo_text_extents (cr, tmp, &extents);
      cairo_translate(cr,
           	      width/(gdouble)hg->fc_mag-shift_x,
		      height/(gdouble)hg->fc_mag-shift_y);
      cairo_move_to(cr,
		    -extents.width-wh_small*0.02,
		    -wh_small*0.02);
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);

      cairo_translate(cr,
		      -width/(gdouble)hg->fc_mag,
		      -height/(gdouble)hg->fc_mag);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);

      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
      cairo_text_extents (cr, "N", &extents);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      
      cairo_translate(cr,
           	      extents.height+wh_small*0.07,
		      extents.height+wh_small*0.07);

      rot_pa(cr, hg);

      cairo_move_to(cr,
		    -extents.width/2,
		    -wh_small*0.05);
      cairo_show_text(cr,"N");
      cairo_move_to(cr,
		    -wh_small*0.05-extents.width,
		    +extents.height/2);
      if(hg->dss_flip){
	cairo_show_text(cr,"W");
      }
      else{
	cairo_show_text(cr,"E");
      }
      
      cairo_set_line_width (cr, 1.5*scale);
      cairo_move_to(cr,
		    0,
		    -wh_small*0.05);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr,
		    -wh_small*0.05, 0);

      cairo_stroke(cr);
      
      if(hg->dss_flip){
	cairo_move_to(cr,0,0);
	cairo_text_extents (cr, "(flipped)", &extents);
	cairo_rel_move_to(cr,-extents.width/2.,extents.height+5*scale);
	cairo_show_text(cr,"(flipped)");
      }

      cairo_restore(cr);
    } // Position Angle
  }


  if(hg->fc_ptn==-1){
    gdouble cx, cy;
    gdouble ptx, pty, ptx0, pty0;
    gdouble rad, rad_min=1000.0, ptr;
    gint i, i_list, i_sel=-1;
    gdouble theta;

  
    cx=((gdouble)width-(gdouble)width_file*r)/2+(gdouble)width_file*r/2;
    cy=((gdouble)height-(gdouble)height_file*r)/2+(gdouble)height_file*r/2;
    if(hg->fc_mag!=1){
      cx-=-shift_x;
      cy-=-shift_y;
    }

    ptx0=((gdouble)hg->fc_ptx1-cx);
    pty0=((gdouble)hg->fc_pty1-cy);

    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_HDSAUTO:
    case FC_INST_HDSZENITH:
    case FC_INST_IRCS:
    case FC_INST_COMICS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
    case FC_INST_FMOS:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)hg->dss_pa/180.;
      }
      else{
	theta=-M_PI*(gdouble)hg->dss_pa/180.;
      }

      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)(90-hg->dss_pa)/180.;
      }
      else{
	theta=-M_PI*(gdouble)(90-hg->dss_pa)/180.;
      }
      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)(270-hg->dss_pa)/180.;
      }
      else{
	theta=-M_PI*(gdouble)(270-hg->dss_pa)/180.;
      }
      break;
    }

    ptx=ptx0*cos(theta)-pty0*sin(theta);
    pty=ptx0*sin(theta)+pty0*cos(theta);

    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if((fabs(hg->fcdb[i_list].x-ptx)<10)&&(fabs(hg->fcdb[i_list].y-pty)<10)){
	rad=(hg->fcdb[i_list].x-ptx)*(hg->fcdb[i_list].x-ptx)
	  +(hg->fcdb[i_list].y-pty)*(hg->fcdb[i_list].y-pty);
	if(rad<rad_min){
	  i_sel=i_list;
	  rad_min=rad;
	}
      }
    }
      
    if(i_sel>=0){
      hg->fcdb_tree_focus=i_sel;
      {
	GtkTreeModel *model 
	  = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
	GtkTreePath *path;
	GtkTreeIter  iter;
	
	path=gtk_tree_path_new_first();
	
	for(i=0;i<hg->fcdb_i_max;i++){
	  gtk_tree_model_get_iter (model, &iter, path);
	  gtk_tree_model_get (model, &iter, COLUMN_FCDB_NUMBER, &i_list, -1);
	  i_list--;
	  
	  if(i_list==i_sel){
	    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), 
					   NOTE_FCDB);
	    gtk_widget_grab_focus (hg->fcdb_tree);
	    gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->fcdb_tree), 
				     path, NULL, FALSE);
	    break;
	  }
	  else{
	    gtk_tree_path_next(path);
	  }
	}
	gtk_tree_path_free(path);
      }
    }

    hg->fc_ptn=0;
  }

  {
    gdouble pmx, pmy;
    gdouble yrs;

    if((hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){
      cairo_save(cr);

      translate_to_center(cr,width,height,width_file,height_file,r,hg);
      if(hg->fcdb_type==FCDB_TYPE_GAIA){
	yrs=current_yrs(hg)-15.0;
      }
      else{
	yrs=current_yrs(hg);
      }

      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	hg->fcdb[i_list].x=-(hg->fcdb[i_list].d_ra-hg->fcdb_d_ra0)*60.
	  *cos(hg->fcdb[i_list].d_dec/180.*M_PI)
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	hg->fcdb[i_list].y=-(hg->fcdb[i_list].d_dec-hg->fcdb_d_dec0)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	if(hg->dss_flip) hg->fcdb[i_list].x=-hg->fcdb[i_list].x;
      }

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_set_line_width (cr, 2*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb_tree_focus!=i_list){
	  cairo_rectangle(cr,hg->fcdb[i_list].x-6,hg->fcdb[i_list].y-6,12,12);
	  cairo_stroke(cr);
	}
      }

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.7, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
      cairo_set_line_width (cr, 4*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb_tree_focus==i_list){
	  cairo_rectangle(cr,hg->fcdb[i_list].x-8,hg->fcdb[i_list].y-8,16,16);
	  cairo_stroke(cr);
	}
      }

      // Proper Motion
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 0.2, 1.0, 0.2, 1.0);
      cairo_set_line_width (cr, 1.5*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb[i_list].pm){
	  pmx=-(hg->fcdb[i_list].d_ra-hg->fcdb_d_ra0
		+hg->fcdb[i_list].pmra/1000/60/60*yrs)*60.
	    *cos(hg->fcdb[i_list].d_dec/180.*M_PI)
	    *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	  pmy=-(hg->fcdb[i_list].d_dec-hg->fcdb_d_dec0
		+hg->fcdb[i_list].pmdec/1000/60/60*yrs)*60.
	    *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	  if(hg->dss_flip) {
	    pmx=-pmx;
	  }
	  cairo_move_to(cr,hg->fcdb[i_list].x,hg->fcdb[i_list].y);
	  cairo_line_to(cr,pmx,pmy);
	  cairo_stroke(cr);
	  cairo_arc(cr,pmx,pmy,5,0,2*M_PI);
	  cairo_fill(cr);
	}
      }

      cairo_restore(cr);
    }
  }
  
  {
    gdouble gs_x, gs_y, gs_d_ra, gs_d_dec;
    
    cairo_save(cr);
    translate_to_center(cr,width,height,width_file,height_file,r,hg);

    if(hg->obj[hg->dss_i].gs.flag){
      gs_d_ra=ra_to_deg(hg->obj[hg->dss_i].gs.ra);
      gs_d_dec=dec_to_deg(hg->obj[hg->dss_i].gs.dec);
      gs_x=-(gs_d_ra-hg->fcdb_d_ra0)*60.
	*cos(gs_d_dec/180.*M_PI)
	*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      gs_y=-(gs_d_dec-hg->fcdb_d_dec0)*60.
	*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      if(hg->dss_flip) gs_x=-gs_x;
      
      cairo_set_line_width (cr, 2*scale);
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 0.2, 1.0, 0.2, 1.0);
      cairo_arc(cr,gs_x,gs_y,15,0,2*M_PI);
      cairo_stroke(cr);
      
      cairo_select_font_face (cr, hg->fontfamily_all, 
			      CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
      tmp=g_strdup("Guide");
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    gs_x-extents.width/2,
		    gs_y-17);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    cairo_restore(cr);
  }

  {  //Non-Sidereal Orbit
    if((hg->orbit_flag)&&(hg->obj[hg->dss_i].i_nst>=0)){
      gint i, i_step=0, i_step_max=1, i_tag=3, i_tag_max=3; 
      gdouble x, y, x0, y0;
      gdouble d_ra, d_dec;
      gdouble d_step, t_step;
      struct ln_equ_posn object, object_prec;
      struct ln_zonedate zonedate;

      cairo_save(cr);

      translate_to_center(cr,width,height,width_file,height_file,r,hg);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);

      object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
      object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

      ln_get_equ_prec2 (&object, 
			get_julian_day_of_epoch(hg->obj[hg->dss_i].equinox),
			JD2000, &object_prec);

      d_ra=ra_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].ra);
      d_dec=dec_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].dec);

      x=-(d_ra-object_prec.ra)*60.
	*cos(d_dec/180.*M_PI)
	*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      y=-(d_dec-object_prec.dec)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      if(hg->dss_flip) x=-x;

      cairo_move_to(cr,x,y);
      
      for(i=1;i<hg->nst[hg->obj[hg->dss_i].i_nst].i_max;i++){
	x0=x;
	y0=y;

	d_ra=ra_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].ra);
	d_dec=dec_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].dec);

	x=-(d_ra-object_prec.ra)*60.
	  *cos(d_dec/180.*M_PI)
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	y=-(d_dec-object_prec.dec)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	if(hg->dss_flip) x=-x;

	if(i==1){
	  d_step=sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
	  if(d_step<(gdouble)width_file*r/20){
	    i_step_max=(gint)((gdouble)width_file*r/20/d_step);
	  }
	  i_step=1;
	}

	cairo_set_line_width (cr, 2.5*scale);
	cairo_line_to(cr,x,y);
	cairo_stroke(cr);

	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0*scale);

	if(i_step>0){

	  if(i_step==i_step_max){
	    cairo_set_line_width (cr, 1.5*scale);
	    if(fabs(x-x0)>fabs(y-y0)){
	      if(i_tag==i_tag_max){
		ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				  &zonedate, 
				  hg->obs_timezone/60);
		tmp=g_strdup_printf("%d/%d %d:%02d",
				    zonedate.months,
				    zonedate.days,
				    zonedate.hours,
				    zonedate.minutes);
		cairo_text_extents (cr, tmp, &extents);
		cairo_move_to(cr,x,y-10);
		cairo_rel_move_to(cr,extents.height/2, 0);
		cairo_rotate (cr,-M_PI/2);
		cairo_show_text(cr,tmp);
		cairo_rotate (cr,M_PI/2);
		if(tmp) g_free(tmp);
		i_tag=0;
	      }
	      else{
		i_tag++;
	      }
	      cairo_move_to(cr,x,y-5);
	      cairo_line_to(cr,x,y+5);
	    }
	    else{
	      if(i_tag==i_tag_max){
		ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				  &zonedate, 
				  hg->obs_timezone/60);
		tmp=g_strdup_printf("%d/%d %d:%02d",
				    zonedate.months,
				    zonedate.days,
				    zonedate.hours,
				    zonedate.minutes);
		cairo_text_extents (cr, tmp, &extents);
		cairo_move_to(cr,x+10,y);
		cairo_rel_move_to(cr,0,extents.height/2);
		cairo_show_text(cr,tmp);
		if(tmp) g_free(tmp);
		i_tag=0;
	      }
	      else{
		i_tag++;
	      }
	      cairo_move_to(cr,x-5,y);
	      cairo_line_to(cr,x+5,y);
	    }
	    cairo_stroke(cr);
	    i_step=1;
	  }
	  else{
	  }
	  i_step++;
	}
	else{
	  cairo_set_line_width (cr, 1.5*scale);
	  if(fabs(x-x0)>fabs(y-y0)){
	    if(i_tag==i_tag_max){
	      ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				&zonedate, 
				hg->obs_timezone/60);
	      tmp=g_strdup_printf("%d/%d %d:%02d",
				  zonedate.months,
				  zonedate.days,
				  zonedate.hours,
				  zonedate.minutes);
	      cairo_text_extents (cr, tmp, &extents);
	      cairo_move_to(cr,x,y-10);
	      cairo_rel_move_to(cr,extents.height/2, 0);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_show_text(cr,tmp);
	      cairo_rotate (cr,M_PI/2);
	      if(tmp) g_free(tmp);
	      i_tag=0;
	    }
	    else{
	      i_tag++;
	    }
	    cairo_move_to(cr,x,y-5);
	    cairo_line_to(cr,x,y+5);
	  }
	  else{	
	    if(i_tag==i_tag_max){
	      ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				&zonedate, 
				hg->obs_timezone/60);
	      tmp=g_strdup_printf("%d/%d %d:%02d",
				  zonedate.months,
				  zonedate.days,
				  zonedate.hours,
				  zonedate.minutes);
	      cairo_text_extents (cr, tmp, &extents);
	      cairo_move_to(cr,x+10,y);
	      cairo_rel_move_to(cr,0,extents.height/2);
	      cairo_show_text(cr,tmp);
	      if(tmp) g_free(tmp);
	      i_tag=0;
	    }
	    else{
	      i_tag++;
	    }
	    cairo_move_to(cr,x-5,y);
	    cairo_line_to(cr,x+5,y);
	  }
	  cairo_stroke(cr);
	}

	cairo_move_to(cr,x,y);
      }

      if(hg->fc_mag==1){
	cairo_restore(cr);

	cairo_save(cr);

	cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
			 (height-(gint)((gdouble)height_file*r))/2);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);

	t_step=(hg->nst[hg->obj[hg->dss_i].i_nst].eph[1].jd
		-hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].jd)
	  *24.*60.*(gdouble)i_step_max; //min
	
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
	if(t_step<5){
	  tmp=g_strdup_printf("Step=%dsec",(gint)(t_step*60+0.5));
	}
	else if(t_step<60){
	  tmp=g_strdup_printf("Step=%dmin",(gint)t_step);
	}
	else{
	  tmp=g_strdup_printf("Step=%.1lfhrs",t_step/60);
	}
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,
		      (gdouble)width_file*r-extents.width-5*scale,
		      (gdouble)height_file*r-5*scale);
	cairo_show_text(cr,tmp);
	if(tmp) g_free(tmp);
      }

     cairo_restore(cr);
    }
  }

  {  // Points and Distance
    gdouble distance;
    gdouble arad;

    if(hg->fc_ptn>=1){
      cairo_save(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.8);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.8);

      cairo_set_line_width (cr, 2*scale);

      if(hg->fc_mag!=1){
	cairo_translate(cr,
			-shift_x,
			-shift_y);
      }

      cairo_move_to(cr,hg->fc_ptx1-5,hg->fc_pty1-5);
      cairo_rel_line_to(cr,10,10);
      
      cairo_move_to(cr,hg->fc_ptx1-5,hg->fc_pty1+5);
      cairo_rel_line_to(cr,10,-10);

      cairo_stroke(cr);
    }

    if(hg->fc_ptn==2){
      cairo_move_to(cr,hg->fc_ptx2-5,hg->fc_pty2-5);
      cairo_rel_line_to(cr,10,10);
      
      cairo_move_to(cr,hg->fc_ptx2-5,hg->fc_pty2+5);
      cairo_rel_line_to(cr,10,-10);

      cairo_stroke(cr);

      cairo_set_line_width (cr, 0.8*scale);
      
      cairo_move_to(cr,hg->fc_ptx1,hg->fc_pty1);
      cairo_line_to(cr,hg->fc_ptx2,hg->fc_pty2);

      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

      distance=sqrt((gdouble)((hg->fc_ptx1-hg->fc_ptx2)
			      *(hg->fc_ptx1-hg->fc_ptx2))
		    +(gdouble)((hg->fc_pty1-hg->fc_pty2)
			       *(hg->fc_pty1-hg->fc_pty2)))
	/((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)*60.0;

      if(distance > 300){
	tmp=g_strdup_printf("%.2lf'",distance/60.0);
      }
      else{
	tmp=g_strdup_printf("%.2lf\"",distance);
      }
      cairo_text_extents (cr, tmp, &extents);

      arad=atan2((hg->fc_ptx1-hg->fc_ptx2),(hg->fc_pty1-hg->fc_pty2));
      cairo_translate(cr,
		      (hg->fc_ptx1+hg->fc_ptx2)/2,
		      (hg->fc_pty1+hg->fc_pty2)/2);
      cairo_rotate (cr,-(arad+M_PI/2));
      
      cairo_move_to(cr,-extents.width/2.,-extents.height*0.8);
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);
    }

    cairo_restore(cr);
  }
  
  if(hg->fc_output==FC_OUTPUT_PDF){
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
  }

  if(hg->fc_output!=FC_OUTPUT_PRINT){
    cairo_destroy(cr);
  }

  if(hg->fc_output==FC_OUTPUT_WINDOW){
    if(hg->pixmap_fc) g_object_unref(G_OBJECT(hg->pixmap_fc));
    hg->pixmap_fc = gdk_pixmap_new(widget->window,
				   width,
				   height,
				   -1);
    if(hg->fc_mag==1){
      gdk_draw_drawable(hg->pixmap_fc,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap_fcbk,
			0,0,0,0,
			width,
			height);
    }
    else{
      {
	gdk_draw_drawable(hg->pixmap_fc,
			  widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			  pixmap_fcbk,
			  0, 0, shift_x, shift_y,
			  width,
			  height);
      }
    }
    g_object_unref(G_OBJECT(pixmap_fcbk));

    gtk_widget_show_all(widget);
    draw_fc_pixmap(widget, hg);
    gtk_widget_queue_draw(widget);
  }

  return TRUE;

}




static 
gboolean configure_draw_fc (GtkWidget *widget, 
			    GdkEventConfigure *event, 
			    gpointer data)
{
  if(!flagFC) return(TRUE);

  typHOE *hg = (typHOE *)data;
  if(!hg->pixmap_fc) return(TRUE);

  draw_fc_cairo(hg->fc_dw,hg);
  draw_fc_pixmap(widget,hg);

  return(TRUE);
}

static
gboolean expose_draw_fc (GtkWidget *widget, 
			 GdkEventExpose *event, 
			 gpointer data)
{
  if(!flagFC) return(TRUE);
  if(event->count!=0) return(TRUE);

  typHOE *hg = (typHOE *)data;
  if(!hg->pixmap_fc) return(TRUE);

  draw_fc_pixmap(hg->fc_dw,hg);

  return(TRUE);
}

gboolean resize_draw_fc(GtkWidget *widget, 
			GdkEventScroll *event, 
			gpointer userdata){
  typHOE *hg;
  GdkScrollDirection direction;
  gint x,y;
  gint magx0, magy0, mag0;
  gint width, height;

  direction = event->direction;
  hg=(typHOE *)userdata;

  if(flagFC){
    if(event->state & GDK_SHIFT_MASK){
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-5));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+5));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,hg);
    }
    else if(event->state & GDK_CONTROL_MASK){
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-1));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+1));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,hg);
    }
    else if(event->state & GDK_MOD1_MASK){
      width= widget->allocation.width;
      height=widget->allocation.height;

      x=width/2;
      y=height/2;
	  
      mag0=hg->fc_mag;
      magx0=hg->fc_magx;
      magy0=hg->fc_magy;
      
      if(direction & GDK_SCROLL_DOWN){
	hg->fc_mag--;
	hg->fc_ptn=0;
      }
      else{
	hg->fc_mag++;
	hg->fc_ptn=0;
      }
      
      if(hg->fc_mag<1){
	hg->fc_mag=1;
	hg->fc_magmode=0;
      }
      else if(hg->fc_mag>FC_MAX_MAG){
	hg->fc_mag=FC_MAX_MAG;
      }
      else{
	if(mag0==1){
	  hg->fc_magmode=0;
	}
	else if(hg->fc_magmode==0){
	  if((magx0!=x)||(magy0!=y)){
	    hg->fc_magmode=1;
	  }
	}
	
	if(hg->fc_magmode==0){
	  hg->fc_magx=x;
	  hg->fc_magy=y;
	}
	else{
	  hg->fc_magx=magx0+(x-width/2)/mag0;
	  hg->fc_magy=magy0+(y-height/2)/mag0;
	}
	gtk_drawing_area_size (GTK_DRAWING_AREA(hg->fc_dw),
			       hg->fc_dw->allocation.width*hg->fc_mag,
			       hg->fc_dw->allocation.height*hg->fc_mag);
      }
    }
    else{
      gdk_window_get_pointer(widget->window,&x,&y,NULL);
      
      mag0=hg->fc_mag;
      magx0=hg->fc_magx;
      magy0=hg->fc_magy;
      
      if(direction & GDK_SCROLL_DOWN){
	hg->fc_mag--;
	hg->fc_ptn=0;
      }
      else{
	hg->fc_mag++;
	hg->fc_ptn=0;
      }
      
      if(hg->fc_mag<1){
	hg->fc_mag=1;
	hg->fc_magmode=0;
      }
      else if(hg->fc_mag>FC_MAX_MAG){
	hg->fc_mag=FC_MAX_MAG;
      }
      else{
	if(mag0==1){
	  hg->fc_magmode=0;
	}
	else if(hg->fc_magmode==0){
	  if((magx0!=x)||(magy0!=y)){
	    hg->fc_magmode=1;
	  }
	}
	
	if(hg->fc_magmode==0){
	  hg->fc_magx=x;
	  hg->fc_magy=y;
	}
	else{
	  width= widget->allocation.width;
	  height=widget->allocation.height;
	  
	  hg->fc_magx=magx0+(x-width/2)/mag0;
	  hg->fc_magy=magy0+(y-height/2)/mag0;
	}
	gtk_drawing_area_size (GTK_DRAWING_AREA(hg->fc_dw),
			       hg->fc_dw->allocation.width*hg->fc_mag,
			       hg->fc_dw->allocation.height*hg->fc_mag);
      }
    }
  }

  return(TRUE);
}
  
static gboolean button_draw_fc(GtkWidget *widget, 
			GdkEventButton *event, 
			gpointer userdata){
  typHOE *hg;
  gint x,y, width, height;

  hg=(typHOE *)userdata;

  if(flagFC){
    gdk_window_get_pointer(widget->window,&x,&y,NULL);

    if((event->button==1)&&(hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){
      hg->fc_ptn=-1;
      hg->fc_ptx1=x;
      hg->fc_pty1=y;
    }
    else if(event->button==2){
      width= widget->allocation.width;
      height=widget->allocation.height;
	  
      hg->fc_magx+=(x-width/2)/hg->fc_mag;
      hg->fc_magy+=(y-height/2)/hg->fc_mag;
    }
    else{
      if(hg->fc_ptn==2){
	hg->fc_ptn=0;
      }
      else if(hg->fc_ptn==0){
	hg->fc_ptn=1;
	hg->fc_ptx1=x;
	hg->fc_pty1=y;
      }
      else if(hg->fc_ptn==1){
	hg->fc_ptn=2;
	hg->fc_ptx2=x;
	hg->fc_pty2=y;
      }
    }

    draw_fc_cairo(hg->fc_dw,hg);
  }

  return(TRUE);
}


void draw_fc_pixmap(GtkWidget *widget, typHOE *hg){
  gdk_window_set_back_pixmap(widget->window,
			     hg->pixmap_fc,
			     FALSE);
      
  gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		    hg->pixmap_fc,
		    0,0,0,0,
		    hg->fc_dw->allocation.width,
		    hg->fc_dw->allocation.height);
}

static void refresh_fc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  if(flagFC){
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw, hg);
  }
}

static void orbit_fc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  if(flagFC){
    hg->orbit_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw, hg);
  }
}


void set_fc_mode (typHOE *hg)
{
  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_STSCI);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file             =g_strconcat(hg->temp_dir,
					  G_DIR_SEPARATOR_S,
					  FC_FILE_GIF,NULL);
    
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_STSCI);
    
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_STSCI_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1R);
      break;
    case FC_STSCI_DSS1B:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1B);
      break;
    case FC_STSCI_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2R);
      break;
    case FC_STSCI_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2B);
      break;
    case FC_STSCI_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2IR);
      break;
    }
    break;
    
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_ESO);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_ESO);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file             =g_strconcat(hg->temp_dir,
					  G_DIR_SEPARATOR_S,
					  FC_FILE_GIF,NULL);
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS1R);
      break;
    case FC_ESO_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2R);
      break;
    case FC_ESO_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2B);
      break;
    case FC_ESO_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2IR);
      break;
    }
    break;
    
  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SKYVIEW);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_SKYVIEW_GALEXF:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXF);
      break;
    case FC_SKYVIEW_GALEXN:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXN);
      break;
    case FC_SKYVIEW_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1R);
      break;
    case FC_SKYVIEW_DSS1B:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1B);
      break;
    case FC_SKYVIEW_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2R);
      break;
    case FC_SKYVIEW_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2B);
      break;
    case FC_SKYVIEW_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2IR);
      break;
    case FC_SKYVIEW_SDSSU:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSU);
      break;
    case FC_SKYVIEW_SDSSG:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSG);
      break;
    case FC_SKYVIEW_SDSSR:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSR);
      break;
    case FC_SKYVIEW_SDSSI:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSI);
      break;
    case FC_SKYVIEW_SDSSZ:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSZ);
      break;
    case FC_SKYVIEW_2MASSJ:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSJ);
      break;
    case FC_SKYVIEW_2MASSH:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSH);
      break;
    case FC_SKYVIEW_2MASSK:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSK);
      break;
    case FC_SKYVIEW_WISE34:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE34);
      break;
    case FC_SKYVIEW_WISE46:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE46);
      break;
    case FC_SKYVIEW_WISE12:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE12);
      break;
    case FC_SKYVIEW_WISE22:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE22);
      break;
    case FC_SKYVIEW_AKARIN60:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN60);
      break;
    case FC_SKYVIEW_AKARIWS:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWS);
      break;
    case FC_SKYVIEW_AKARIWL:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWL);
      break;
    case FC_SKYVIEW_AKARIN160:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN160);
      break;
    case FC_SKYVIEW_NVSS:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_NVSS);
      break;
    }
    break;

  case FC_SDSS:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SDSS);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SDSS);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;
    
  case FC_SDSS13:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SDSS13);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SDSS13);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;

  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if(hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
    }
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_PANCOL);
    if(hg->dss_path) g_free(hg->dss_path);
    switch(hg->fc_mode){
    case FC_PANCOL:
      hg->dss_path             =g_strdup(FC_PATH_PANCOL);
      break;

    case FC_PANG:
      hg->dss_path             =g_strdup(FC_PATH_PANG);
      break;
 
    case FC_PANR:
      hg->dss_path             =g_strdup(FC_PATH_PANR);
      break;
 
    case FC_PANI:
      hg->dss_path             =g_strdup(FC_PATH_PANI);
      break;
 
    case FC_PANZ:
      hg->dss_path             =g_strdup(FC_PATH_PANZ);
      break;
 
    case FC_PANY:
      hg->dss_path             =g_strdup(FC_PATH_PANY);
      break;
    }
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;

  }
}

void set_hsc_dith_label (typHOE *hg){
  gchar *tmp;

  switch(hg->hsc_dithp){
  case HSC_DITH_NO:
    tmp=g_strdup_printf(" %d/1 ",hg->hsc_dithi);
    break;

  case HSC_DITH_5:
    tmp=g_strdup_printf(" %d/5 ",hg->hsc_dithi);
    break;

  case HSC_DITH_N:
    tmp=g_strdup_printf(" %d/%d ",hg->hsc_dithi,hg->hsc_ndith);
    break;
  }

  gtk_label_set_text(GTK_LABEL(hg->hsc_label_dith),tmp);

  if(tmp) g_free(tmp);
}

static void hsc_dith_back (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->hsc_dithp){
  case HSC_DITH_NO:
    return;
    break;

  case HSC_DITH_5:
  case HSC_DITH_N:
    if(hg->hsc_dithi>1){
      hg->hsc_dithi--;
      set_hsc_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;
  }
}

static void hsc_dith_forward (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->hsc_dithp){
  case HSC_DITH_NO:
    return;
    break;

  case HSC_DITH_5:
    if(hg->hsc_dithi<5){
      hg->hsc_dithi++;
      set_hsc_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;

  case HSC_DITH_N:
    if(hg->hsc_dithi<hg->hsc_ndith){
      hg->hsc_dithi++;
      set_hsc_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;
  }
}

static void cc_get_hsc_dith (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->hsc_dithp=n;
    hg->hsc_dithi=1;

    set_hsc_dith_label(hg);

    if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
  }
}

static void cc_get_fc_inst (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_inst=n;
  }

  switch(hg->fc_inst){
  case FC_INST_HDS:
  case FC_INST_HDSAUTO:
  case FC_INST_HDSZENITH:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(HDS_SIZE));
    break;

  case FC_INST_IRCS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(IRCS_SIZE));
    break;

  case FC_INST_COMICS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(COMICS_SIZE));
    break;

  case FC_INST_FOCAS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(FOCAS_SIZE));
    break;

  case FC_INST_MOIRCS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(MOIRCS_SIZE));
    break;

  case FC_INST_FMOS:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(FMOS_SIZE));
      break;
    }
    break;
			     
  case FC_INST_SPCAM:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(SPCAM_SIZE));
      break;
    }
    break;
			     
  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(HSC_SIZE));
      break;
    }
    break;

  default:
    break;
  }

  if(hg->fc_inst==FC_INST_HDSAUTO){
    hg->dss_flip=FALSE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),
				 hg->dss_flip);
    gtk_widget_set_sensitive(hg->fc_button_flip,FALSE);
  }
  else if(hg->fc_inst==FC_INST_HDSZENITH){
    hg->dss_flip=TRUE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),
				 hg->dss_flip);
    gtk_widget_set_sensitive(hg->fc_button_flip,FALSE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_button_flip,TRUE);
  }
}

static void cc_get_fc_mode (GtkWidget *widget,  gpointer gdata)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_mode=n;

    set_fc_frame_col(hg);
    set_fc_mode(hg);
  }
}

static void cc_get_fc_mode_pdf (GtkWidget *widget,  gpointer gdata)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_mode=n;

    set_fc_frame_col_pdf(hg);
    set_fc_mode(hg);
  }
}

/*
void cc_get_fc_mode (GtkWidget *widget,  gpointer gdata)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_mode=n;

    switch(hg->fc_mode){
    case FC_STSCI_DSS1R:
    case FC_STSCI_DSS1B:
    case FC_STSCI_DSS2R:
    case FC_STSCI_DSS2B:
    case FC_STSCI_DSS2IR:
      if(hg->dss_host) g_free(hg->dss_host);
      hg->dss_host             =g_strdup(FC_HOST_STSCI);
      if(hg->dss_file) g_free(hg->dss_file);
      hg->dss_file             =g_strdup(FC_FILE_GIF);
      if(hg->dss_path) g_free(hg->dss_path);
      hg->dss_path             =g_strdup(FC_PATH_STSCI);

      if(hg->dss_src) g_free(hg->dss_src);
      switch(hg->fc_mode){
      case FC_STSCI_DSS1R:
	hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1R);
	break;
      case FC_STSCI_DSS1B:
	hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1B);
	break;
      case FC_STSCI_DSS2R:
	hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2R);
	break;
      case FC_STSCI_DSS2B:
	hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2B);
	break;
      case FC_STSCI_DSS2IR:
	hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2IR);
	break;
      }
      break;

    case FC_ESO_DSS2R:
      if(hg->dss_host) g_free(hg->dss_host);
      hg->dss_host             =g_strdup(FC_HOST_ESO);
      if(hg->dss_path) g_free(hg->dss_path);
      hg->dss_path             =g_strdup(FC_PATH_ESO);
      if(hg->dss_file) g_free(hg->dss_file);
      hg->dss_file             =g_strdup(FC_FILE_GIF);
      if(hg->dss_tmp) g_free(hg->dss_tmp);
      hg->dss_tmp             =g_strdup(FC_FILE_HTML);
      if(hg->dss_src) g_free(hg->dss_src);
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2R);
      break;

    case FC_SKYVIEW_DSS1R:
    case FC_SKYVIEW_DSS1B:
    case FC_SKYVIEW_DSS2R:
    case FC_SKYVIEW_DSS2B:
    case FC_SKYVIEW_DSS2IR:
    case FC_SKYVIEW_2MASSJ:
    case FC_SKYVIEW_2MASSH:
    case FC_SKYVIEW_2MASSK:
      if(hg->dss_host) g_free(hg->dss_host);
      hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
      if(hg->dss_path) g_free(hg->dss_path);
      hg->dss_path             =g_strdup(FC_PATH_SKYVIEW);
      if(hg->dss_file) g_free(hg->dss_file);
      hg->dss_file             =g_strdup(FC_FILE_JPEG);
      if(hg->dss_tmp) g_free(hg->dss_tmp);
      hg->dss_tmp             =g_strdup(FC_FILE_HTML);
      if(hg->dss_src) g_free(hg->dss_src);
      switch(hg->fc_mode){
      case FC_SKYVIEW_DSS1R:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1R);
	break;
      case FC_SKYVIEW_DSS1B:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1B);
	break;
      case FC_SKYVIEW_DSS2R:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2R);
	break;
      case FC_SKYVIEW_DSS2B:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2B);
	break;
      case FC_SKYVIEW_DSS2IR:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2IR);
	break;
      case FC_SKYVIEW_2MASSJ:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSJ);
	break;
      case FC_SKYVIEW_2MASSH:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSH);
	break;
      case FC_SKYVIEW_2MASSK:
	hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSK);
	break;
      }
      break;

    case FC_SDSS:
      if(hg->dss_host) g_free(hg->dss_host);
      hg->dss_host             =g_strdup(FC_HOST_SDSS);
      if(hg->dss_path) g_free(hg->dss_path);
      hg->dss_path             =g_strdup(FC_PATH_SDSS);
      if(hg->dss_file) g_free(hg->dss_file);
      hg->dss_file             =g_strdup(FC_FILE_JPEG);
      break; 

    case FC_SDSS13:
      if(hg->dss_host) g_free(hg->dss_host);
      hg->dss_host             =g_strdup(FC_HOST_SDSS13);
      if(hg->dss_path) g_free(hg->dss_path);
      hg->dss_path             =g_strdup(FC_PATH_SDSS13);
      if(hg->dss_file) g_free(hg->dss_file);
      hg->dss_file             =g_strdup(FC_FILE_JPEG);
    break;
   }
  }
}
*/

void pdf_fc (typHOE *hg)
{
  hg->fc_output=FC_OUTPUT_PDF;

  if(flagFC){
    draw_fc_cairo(hg->fc_dw, hg);
  }

  hg->fc_output=FC_OUTPUT_WINDOW;
}

static void do_print_fc (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  GtkPrintOperation *op; 
  GtkPrintOperationResult res; 

  hg=(typHOE *)gdata;

  op = gtk_print_operation_new ();

  gtk_print_operation_set_n_pages (op, 1); 
  my_signal_connect (op, "draw_page", G_CALLBACK (draw_page), (gpointer)hg); 
  res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				 NULL,NULL);

  g_object_unref(G_OBJECT(op));
}


static void draw_page (GtkPrintOperation *operation, 
		       GtkPrintContext *context,
		       gint page_nr, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->fc_output=FC_OUTPUT_PRINT;
  hg->context=context;
  if(flagFC){
    draw_fc_cairo(hg->fc_dw, hg);
  }

  hg->fc_output=FC_OUTPUT_WINDOW;
  hg->context=NULL;
} 


#ifndef USE_WIN32
void dss_signal(int sig){
  pid_t child_pid=0;
  
  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
  
}


void dssall_signal(int sig){
  pid_t child_pid=0;

  flag_dssall_finish=TRUE;

  do{
    int child_ret;
    child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
  
}

#endif

gboolean check_dssall (gpointer gdata){
  if(flag_dssall_finish){
    flag_dssall_finish=FALSE;
      gtk_main_quit();
  }
  return(TRUE);
}

glong get_file_size(gchar *fname)
{
  FILE *fp;
  long sz;

  fp = fopen( fname, "rb" );
  if( fp == NULL ){
    return -1;
  }

  fseek( fp, 0, SEEK_END );
  sz = ftell( fp );

  fclose( fp );
  return sz;
}


static void show_fc_help (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *table;
  GdkPixbuf *icon, *pixbuf;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Help for Finding Chart");

  my_signal_connect(dialog,"destroy",
		    close_fc_help, 
		    GTK_WIDGET(dialog));
  
  table = gtk_table_new(2,11,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_dl), icon_dl, 
				    FALSE, NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon,16,16,GDK_INTERP_BILINEAR);

  pixmap = gtk_image_new_from_pixbuf(pixbuf);
  g_object_unref(icon);
  g_object_unref(pixbuf);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 0, 1,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Download new image and redraw w/instrument");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 0, 1,
		    GTK_FILL,GTK_SHRINK,0,0);
  

  pixmap=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Redraw selected instrument and PA");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 1, 2,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 2, 3,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  Query objects in the finding chart via online database (SIMBAD/NED)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 2, 3,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 3, 4,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Change parameters for database query");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 3, 4,
		    GTK_FILL,GTK_SHRINK,0,0);


  label = gtk_label_new ("<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 4, 5,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view around cursor (upto x5)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 4, 5,
		    GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_OSX
  label = gtk_label_new ("<option>+<wheel-scroll>");
#else  
  label = gtk_label_new ("<alt>+<wheel-scroll>");
#endif
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 5, 6,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view w/o moving the center (upto x5)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 5, 6,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<ctrl>+<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 6, 7,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/1 deg step)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 6, 7,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<shift>+<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 7, 8,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/5 deg step)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 7, 8,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<left-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 8, 9,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Focus on the identified object");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 8, 9,
		    GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("<middle-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 9, 10,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Move the clicked point to the center");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 9, 10,
		    GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("<right-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 10, 11,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Measure the distance between 2-points (The 3rd click to clear)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 10, 11,
		    GTK_FILL,GTK_SHRINK,0,0);
  

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);


  label = gtk_label_new ("Please use SkyView or SDSS for large FOV (> 60\') to save the traffic.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("ESO and STSci cannot change their pixel scale.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("Because the maximum pixel sizes for SkyView (1000pix) and SDSS (2000pix) are limited,");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("the downloaded FC image for large FOV (> 13\' for SDSS) should be degraded from the original.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);



  button=gtkut_button_new_from_stock("OK",GTK_STOCK_OK);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_fc_help, 
		    GTK_WIDGET(dialog));

  gtk_widget_show_all(dialog);
}

static void close_fc_help(GtkWidget *w, GtkWidget *dialog)
{
  gtk_widget_destroy(dialog);
}

void create_fcdb_para_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox, 
    *spinner, *combo, *table, *check, *rb[16], 
    *table1, *hbox1, *vbox1;
  GtkAdjustment *adj;
  gint tmp_band, tmp_mag, tmp_otype, tmp_ned_otype, tmp_ned_diam, 
    tmp_gsc_mag, tmp_gsc_diam, tmp_ps1_mag, tmp_ps1_diam, tmp_ps1_mindet, 
    tmp_sdss_search,
    tmp_sdss_magmax[NUM_SDSS_BAND], tmp_sdss_magmin[NUM_SDSS_BAND], 
    tmp_sdss_diam, tmp_usno_mag, tmp_usno_diam,
    tmp_gaia_mag, tmp_gaia_diam, tmp_2mass_mag, tmp_2mass_diam,
    tmp_wise_mag, tmp_wise_diam;
  gboolean tmp_ned_ref, tmp_gsc_fil, tmp_ps1_fil, tmp_usno_fil,
    tmp_sdss_fil[NUM_SDSS_BAND], 
    tmp_gaia_fil, tmp_2mass_fil, tmp_wise_fil,
    tmp_smoka_shot,
    tmp_smoka_subaru[NUM_SMOKA_SUBARU],
    tmp_smoka_kiso[NUM_SMOKA_KISO],
    tmp_smoka_oao[NUM_SMOKA_OAO],
    tmp_smoka_mtm[NUM_SMOKA_MTM],
    tmp_smoka_kanata[NUM_SMOKA_KANATA],
    tmp_hst_image[NUM_HST_IMAGE],
    tmp_hst_spec[NUM_HST_SPEC],
    tmp_hst_other[NUM_HST_OTHER],
    tmp_eso_image[NUM_ESO_IMAGE],
    tmp_eso_spec[NUM_ESO_SPEC],
    tmp_eso_vlti[NUM_ESO_VLTI],
    tmp_eso_pola[NUM_ESO_POLA],
    tmp_eso_coro[NUM_ESO_CORO],
    tmp_eso_other[NUM_ESO_OTHER],
    tmp_eso_sam[NUM_ESO_SAM],
    tmp_gemini_inst;
  confPropFCDB *cdata;
  gboolean rebuild_flag=FALSE;
  gint i;
  gchar tmp[BUFFSIZE];

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  cdata=g_malloc0(sizeof(confPropFCDB));
  cdata->mode=0;

  tmp_band=hg->fcdb_band;
  tmp_mag=hg->fcdb_mag;
  tmp_otype=hg->fcdb_otype;
  tmp_ned_diam=hg->fcdb_ned_diam;
  tmp_ned_otype=hg->fcdb_ned_otype;
  tmp_ned_ref=hg->fcdb_ned_ref;
  tmp_gsc_fil=hg->fcdb_gsc_fil;
  tmp_gsc_mag=hg->fcdb_gsc_mag;
  tmp_gsc_diam=hg->fcdb_gsc_diam;
  tmp_ps1_fil=hg->fcdb_ps1_fil;
  tmp_ps1_mag=hg->fcdb_ps1_mag;
  tmp_ps1_diam=hg->fcdb_ps1_diam;
  tmp_ps1_mindet=hg->fcdb_ps1_mindet;
  tmp_sdss_search=hg->fcdb_sdss_search;
  for(i=0;i<NUM_SDSS_BAND;i++){
    tmp_sdss_fil[i]=hg->fcdb_sdss_fil[i];
    tmp_sdss_magmax[i]=hg->fcdb_sdss_magmax[i];
    tmp_sdss_magmin[i]=hg->fcdb_sdss_magmin[i];
  }
  tmp_sdss_diam=hg->fcdb_sdss_diam;
  tmp_usno_fil=hg->fcdb_usno_fil;
  tmp_usno_mag=hg->fcdb_usno_mag;
  tmp_usno_diam=hg->fcdb_usno_diam;
  tmp_gaia_fil=hg->fcdb_gaia_fil;
  tmp_gaia_mag=hg->fcdb_gaia_mag;
  tmp_gaia_diam=hg->fcdb_gaia_diam;
  tmp_2mass_fil=hg->fcdb_2mass_fil;
  tmp_2mass_mag=hg->fcdb_2mass_mag;
  tmp_2mass_diam=hg->fcdb_2mass_diam;
  tmp_wise_fil=hg->fcdb_wise_fil;
  tmp_wise_mag=hg->fcdb_wise_mag;
  tmp_wise_diam=hg->fcdb_wise_diam;
  tmp_smoka_shot=hg->fcdb_smoka_shot;
  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    tmp_smoka_subaru[i]=hg->fcdb_smoka_subaru[i];
  }
  for(i=0;i<NUM_SMOKA_KISO;i++){
    tmp_smoka_kiso[i]=hg->fcdb_smoka_kiso[i];
  }
  for(i=0;i<NUM_SMOKA_OAO;i++){
    tmp_smoka_oao[i]=hg->fcdb_smoka_oao[i];
  }
  for(i=0;i<NUM_SMOKA_MTM;i++){
    tmp_smoka_mtm[i]=hg->fcdb_smoka_mtm[i];
  }
  for(i=0;i<NUM_SMOKA_KANATA;i++){
    tmp_smoka_kanata[i]=hg->fcdb_smoka_kanata[i];
  }
  for(i=0;i<NUM_HST_IMAGE;i++){
    tmp_hst_image[i]=hg->fcdb_hst_image[i];
  }
  for(i=0;i<NUM_HST_SPEC;i++){
    tmp_hst_spec[i]=hg->fcdb_hst_spec[i];
  }
  for(i=0;i<NUM_HST_OTHER;i++){
    tmp_hst_other[i]=hg->fcdb_hst_other[i];
  }
  for(i=0;i<NUM_ESO_IMAGE;i++){
    tmp_eso_image[i]=hg->fcdb_eso_image[i];
  }
  for(i=0;i<NUM_ESO_SPEC;i++){
    tmp_eso_spec[i]=hg->fcdb_eso_spec[i];
  }
  for(i=0;i<NUM_ESO_VLTI;i++){
    tmp_eso_vlti[i]=hg->fcdb_eso_vlti[i];
  }
  for(i=0;i<NUM_ESO_POLA;i++){
    tmp_eso_pola[i]=hg->fcdb_eso_pola[i];
  }
  for(i=0;i<NUM_ESO_CORO;i++){
    tmp_eso_coro[i]=hg->fcdb_eso_coro[i];
  }
  for(i=0;i<NUM_ESO_OTHER;i++){
    tmp_eso_other[i]=hg->fcdb_eso_other[i];
  }
  for(i=0;i<NUM_ESO_SAM;i++){
    tmp_eso_sam[i]=hg->fcdb_eso_sam[i];
  }
  tmp_gemini_inst =hg->fcdb_gemini_inst;

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Change Parameters for database query");

  my_signal_connect(dialog,"delete-event",close_disp_para,GTK_WIDGET(dialog));

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox1,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 0);

  frame = gtk_frame_new ("Database");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[0] = gtk_radio_button_new_with_label_from_widget (NULL, "SIMBAD");
  gtk_box_pack_start(GTK_BOX(hbox), rb[0], FALSE, FALSE, 0);
  my_signal_connect (rb[0], "toggled", radio_fcdb, (gpointer)hg);

  rb[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "NED");
  gtk_box_pack_start(GTK_BOX(hbox), rb[1], FALSE, FALSE, 0);
  gtk_widget_show (rb[1]);
  my_signal_connect (rb[1], "toggled", radio_fcdb, (gpointer)hg);

  frame = gtk_frame_new ("Optical");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  rb[2] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "GSC");
  gtk_box_pack_start(GTK_BOX(hbox), rb[2], FALSE, FALSE, 0);
  gtk_widget_show (rb[2]);
  my_signal_connect (rb[2], "toggled", radio_fcdb, (gpointer)hg);

  rb[3] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "PanSTARRS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[3], FALSE, FALSE, 0);
  gtk_widget_show (rb[3]);
  my_signal_connect (rb[3], "toggled", radio_fcdb, (gpointer)hg);

  rb[4] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "SDSS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[4], FALSE, FALSE, 0);
  gtk_widget_show (rb[4]);
  my_signal_connect (rb[4], "toggled", radio_fcdb, (gpointer)hg);

  rb[5] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "LAMOST");
  gtk_box_pack_start(GTK_BOX(hbox), rb[5], FALSE, FALSE, 0);
  gtk_widget_show (rb[5]);
  my_signal_connect (rb[5], "toggled", radio_fcdb, (gpointer)hg);

  rb[6] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "USNO-B");
  gtk_box_pack_start(GTK_BOX(hbox), rb[6], FALSE, FALSE, 0);
  gtk_widget_show (rb[6]);
  my_signal_connect (rb[6], "toggled", radio_fcdb, (gpointer)hg);

  rb[7] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "GAIA");
  gtk_box_pack_start(GTK_BOX(hbox), rb[7], FALSE, FALSE, 0);
  gtk_widget_show (rb[7]);
  my_signal_connect (rb[7], "toggled", radio_fcdb, (gpointer)hg);

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox1,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 0);

  frame = gtk_frame_new ("Infrared");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[8] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "2MASS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[8], FALSE, FALSE, 0);
  gtk_widget_show (rb[8]);
  my_signal_connect (rb[8], "toggled", radio_fcdb, (gpointer)hg);

  rb[9] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "WISE");
  gtk_box_pack_start(GTK_BOX(hbox), rb[9], FALSE, FALSE, 0);
  gtk_widget_show (rb[9]);
  my_signal_connect (rb[9], "toggled", radio_fcdb, (gpointer)hg);

  rb[10] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "AKARI/IRC");
  gtk_box_pack_start(GTK_BOX(hbox), rb[10], FALSE, FALSE, 0);
  gtk_widget_show (rb[10]);
  my_signal_connect (rb[10], "toggled", radio_fcdb, (gpointer)hg);

  rb[11] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "AKARI/FIS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[11], FALSE, FALSE, 0);
  gtk_widget_show (rb[11]);
  my_signal_connect (rb[11], "toggled", radio_fcdb, (gpointer)hg);

  frame = gtk_frame_new ("Data Archive");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  rb[12] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "SMOKA");
  gtk_box_pack_start(GTK_BOX(hbox), rb[12], FALSE, FALSE, 0);
  gtk_widget_show (rb[12]);
  my_signal_connect (rb[12], "toggled", radio_fcdb, (gpointer)hg);

  rb[13] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "HST");
  gtk_box_pack_start(GTK_BOX(hbox), rb[13], FALSE, FALSE, 0);
  gtk_widget_show (rb[13]);
  my_signal_connect (rb[13], "toggled", radio_fcdb, (gpointer)hg);

  rb[14] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "ESO");
  gtk_box_pack_start(GTK_BOX(hbox), rb[14], FALSE, FALSE, 0);
  gtk_widget_show (rb[14]);
  my_signal_connect (rb[14], "toggled", radio_fcdb, (gpointer)hg);

  rb[15] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "Gemini");
  gtk_box_pack_start(GTK_BOX(hbox), rb[15], FALSE, FALSE, 0);
  gtk_widget_show (rb[15]);
  my_signal_connect (rb[15], "toggled", radio_fcdb, (gpointer)hg);

  cdata->fcdb_group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));
  cdata->fcdb_type=hg->fcdb_type;

  frame = gtk_frame_new ("Query parameters");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hg->query_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->query_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->query_note), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), hg->query_note);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("SIMBAD");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Magnitude");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "(Nop.)",
		       1, FCDB_BAND_NOP, -1);
    if(hg->fcdb_band==FCDB_BAND_NOP) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  U  ",
		       1, FCDB_BAND_U, -1);
    if(hg->fcdb_band==FCDB_BAND_U) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  B  ",
		       1, FCDB_BAND_B, -1);
    if(hg->fcdb_band==FCDB_BAND_B) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  V  ",
		       1, FCDB_BAND_V, -1);
    if(hg->fcdb_band==FCDB_BAND_V) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  R  ",
		       1, FCDB_BAND_R, -1);
    if(hg->fcdb_band==FCDB_BAND_R) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  I  ",
		       1, FCDB_BAND_I, -1);
    if(hg->fcdb_band==FCDB_BAND_I) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  J  ",
		       1, FCDB_BAND_J, -1);
    if(hg->fcdb_band==FCDB_BAND_J) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  H  ",
		       1, FCDB_BAND_H, -1);
    if(hg->fcdb_band==FCDB_BAND_H) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  K  ",
		       1, FCDB_BAND_K, -1);
    if(hg->fcdb_band==FCDB_BAND_K) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_band);
  }

  label = gtk_label_new (" < ");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_mag,
					    8, 25, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_mag);


  label = gtk_label_new ("Object Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Types",
		       1, FCDB_OTYPE_ALL, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_ALL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Star",
		       1, FCDB_OTYPE_STAR, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_STAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ISM",
		       1, FCDB_OTYPE_ISM, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_ISM) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Planetary Nebula",
		       1, FCDB_OTYPE_PN, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_PN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H II region",
		       1, FCDB_OTYPE_HII, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_HII) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Galaxy",
		       1, FCDB_OTYPE_GALAXY, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_GALAXY) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "QSO",
		       1, FCDB_OTYPE_QSO, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_QSO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "gamma-ray source",
		       1, FCDB_OTYPE_GAMMA, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_GAMMA) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "X-ray source",
		       1, FCDB_OTYPE_X, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_X) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IR source",
		       1, FCDB_OTYPE_IR, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_IR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Radio source",
		       1, FCDB_OTYPE_RADIO, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_RADIO) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_otype);
  }


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("NED");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ned_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ned_diam);

  label = gtk_label_new ("[arcmin]");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("Object Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Types",
		       1, FCDB_NED_OTYPE_ALL, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_ALL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Extragalactic Object",
		       1, FCDB_NED_OTYPE_EXTRAG, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_EXTRAG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "QSO",
		       1, FCDB_NED_OTYPE_QSO, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_QSO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Star",
		       1, FCDB_NED_OTYPE_STAR, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_STAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Nova / Super Nova",
		       1, FCDB_NED_OTYPE_SN, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_SN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Planetary nebula",
		       1, FCDB_NED_OTYPE_PN, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_PN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H II region",
		       1, FCDB_NED_OTYPE_HII, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_HII) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_ned_otype);
  }

  check = gtk_check_button_new_with_label("Only objects w/references");
  gtk_table_attach(GTK_TABLE(table), check, 0, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ned_ref);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ned_ref);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("GSC 2.3");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_diam);

  label = gtk_label_new ("[arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gsc_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gsc_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("PanSTARRS-1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_diam,
					    1, FCDB_PS1_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_diam);

  label = gtk_label_new ("[arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ps1_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ps1_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("r < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mag);

  label = gtk_label_new ("Minimum nDetections");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mindet,
					    1, 25, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 2, 3,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mindet);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("SDSS DR14");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(3,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 3, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_diam);

  label = gtk_label_new ("[arcmin]"); 
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Imaging Query",
		       1, FCDB_NED_OTYPE_ALL, -1);
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Spectro Query",
		       1, FCDB_NED_OTYPE_EXTRAG, -1);
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_SPEC) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_sdss_search);
  }

  
  frame = gtk_frame_new ("Mag. filter");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  table1= gtk_table_new(6,NUM_SDSS_BAND,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  for(i=0;i<NUM_SDSS_BAND;i++){
    check = gtk_check_button_new_with_label(NULL);
    gtk_table_attach(GTK_TABLE(table1), check, 0, 1, i, i+1,
		     GTK_FILL,GTK_SHRINK,0,0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_sdss_fil[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_sdss_fil[i]);

    adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_magmin[i],
					      0, 30, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   FALSE);
    gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, i, i+1,
		     GTK_FILL,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_magmin[i]);


    label = gtk_label_new ("< "); 
    gtk_table_attach(GTK_TABLE(table1), label, 2, 3, i, i+1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new (sdss_band[i]); 
    gtk_table_attach(GTK_TABLE(table1), label, 3, 4, i, i+1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new (" <"); 
    gtk_table_attach(GTK_TABLE(table1), label, 4, 5, i, i+1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_magmax[i],
					      0, 30, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   FALSE);
    gtk_table_attach(GTK_TABLE(table1), spinner, 5, 6, i, i+1,
		     GTK_FILL,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_magmax[i]);
  }


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("LAMOST DR3");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(3,6,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter = Finding Chart Diameter");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("USNO-B");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_usno_diam,
					    1, FCDB_USNO_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_usno_diam);

  label = gtk_label_new ("[arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_usno_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_usno_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_usno_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_usno_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("GAIA DR1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Area ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gaia_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gaia_diam);

  label = gtk_label_new ("[arcmin x arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gaia_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gaia_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("G (0.33 - 1.0um) < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gaia_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gaia_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("2MASS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_diam);

  label = gtk_label_new ("[arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_2mass_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_2mass_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("H < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_mag,
					    8, 16, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("WISE");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,6,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Area ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 3, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_wise_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_wise_diam);

  label = gtk_label_new ("[arcmin x arcmin]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_wise_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_wise_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 3, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("W1 < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_wise_mag,
					    8, 18, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_wise_mag);

  label = gtk_label_new ("W1 [mag] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("3.35um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("2.75-3.87um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W2 [mag] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("4.6um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("3.96-5.34um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W3 [mag] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("11.6um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("7.44-17.3um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W4 [mag] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("22.1um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("19.5-27.9um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("AKARI/IRC");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,4,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("S9W [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("6.7-11.6um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("9.4\"x9.4\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("L18W [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("13.9-25.6um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("10.4\"x9.4\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("AKARI/FIS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(3,6,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("N60 [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("50-80um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("26.8\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("WIDE-S [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("60-110um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("26.8\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("WIDE-L [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("110-180um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("44.2\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 4, 5,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("N160 [Jy] : ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("140-180um");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("44.2\"/pix");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 5, 6,
		   GTK_FILL,GTK_SHRINK,0,0);

  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("SMOKA");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(4,5, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Shot (Suprime-Cam, Hyper Suprime-Cam, and KWFC ONLY)");
  gtk_table_attach(GTK_TABLE(table), check, 0, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_smoka_shot);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_smoka_shot);

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 0, 1, 2, 5,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Subaru");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    check = gtk_check_button_new_with_label(smoka_subaru[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_subaru[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_subaru[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 1, 2, 2, 4,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Kiso");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_KISO;i++){
    check = gtk_check_button_new_with_label(smoka_kiso[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_kiso[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_kiso[i]);
  }

  frame = gtk_frame_new ("OAO");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_OAO;i++){
    check = gtk_check_button_new_with_label(smoka_oao[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_oao[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_oao[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 2, 3, 2, 3,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("MITSuME");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_MTM;i++){
    check = gtk_check_button_new_with_label(smoka_mtm[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_mtm[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_mtm[i]);
  }

  frame = gtk_frame_new ("Hiroshima");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_KANATA;i++){
    check = gtk_check_button_new_with_label(smoka_kanata[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_kanata[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_kanata[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 3, 4, 2, 3,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("HST");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(4,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);


  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 0, 1, 1, 3,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Imaging");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_IMAGE;i++){
    check = gtk_check_button_new_with_label(hst_image[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_image[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_image[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 1, 2, 1, 3,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Spectroscopy");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_SPEC;i++){
    check = gtk_check_button_new_with_label(hst_spec[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_spec[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_spec[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 2, 3, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Other");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_OTHER;i++){
    check = gtk_check_button_new_with_label(hst_other[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_other[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_other[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 3, 4, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("ESO");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(4,6,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 0, 1, 1, 3,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Imaging");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_IMAGE;i++){
    check = gtk_check_button_new_with_label(eso_image[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_image[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_image[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 1, 2, 1, 5,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Spectroscopy");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_SPEC;i++){
    check = gtk_check_button_new_with_label(eso_spec[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_spec[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_spec[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 2, 3, 1, 4,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Interferometry");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_VLTI;i++){
    check = gtk_check_button_new_with_label(eso_vlti[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_vlti[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_vlti[i]);
  }

  frame = gtk_frame_new ("Polarimetry");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_POLA;i++){
    check = gtk_check_button_new_with_label(eso_pola[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_pola[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_pola[i]);
  }

  frame = gtk_frame_new ("Coronagraphy");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_CORO;i++){
    check = gtk_check_button_new_with_label(eso_coro[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_coro[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_coro[i]);
  }

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), vbox1, 3, 4, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtk_frame_new ("Other");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_OTHER;i++){
    check = gtk_check_button_new_with_label(eso_other[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_other[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_other[i]);
  }

  frame = gtk_frame_new ("SAM");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_SAM;i++){
    check = gtk_check_button_new_with_label(eso_sam[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_sam[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_sam[i]);
  }

  
  // Gemini
  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("Gemini");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(4,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Radius = Finding Chart Radius");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Instrument");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_GEMINI_INST;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, gemini_inst[i_inst].name,
			 1, i_inst, -1);
      if(hg->fcdb_gemini_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table), combo, 1, 3, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_gemini_inst);
  }

  


  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_fcdb_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);

  if(hg->fcdb_type<=FCDB_TYPE_GEMINI)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->fcdb_type]),TRUE);

  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
      hg->fcdb_band  = tmp_band;
      hg->fcdb_mag   = tmp_mag;
      hg->fcdb_otype = tmp_otype;
      hg->fcdb_ned_diam = tmp_ned_diam;
      hg->fcdb_ned_otype = tmp_ned_otype;
      if(hg->fcdb_type!=cdata->fcdb_type) rebuild_flag=TRUE;
      hg->fcdb_type  = cdata->fcdb_type;
      hg->fcdb_ned_ref  = tmp_ned_ref;
      hg->fcdb_gsc_fil  = tmp_gsc_fil;
      hg->fcdb_gsc_mag  = tmp_gsc_mag;
      hg->fcdb_gsc_diam  = tmp_gsc_diam;
      hg->fcdb_ps1_fil  = tmp_ps1_fil;
      hg->fcdb_ps1_mag  = tmp_ps1_mag;
      hg->fcdb_ps1_diam  = tmp_ps1_diam;
      hg->fcdb_ps1_mindet  = tmp_ps1_mindet;
      hg->fcdb_sdss_search = tmp_sdss_search;
      for(i=0;i<NUM_SDSS_BAND;i++){
	hg->fcdb_sdss_fil[i]  = tmp_sdss_fil[i];
	hg->fcdb_sdss_magmax[i]  = tmp_sdss_magmax[i];
	hg->fcdb_sdss_magmax[i]  = tmp_sdss_magmax[i];
      }
      hg->fcdb_sdss_diam  = tmp_sdss_diam;
      hg->fcdb_usno_fil  = tmp_usno_fil;
      hg->fcdb_usno_mag  = tmp_usno_mag;
      hg->fcdb_usno_diam  = tmp_usno_diam;
      hg->fcdb_gaia_fil  = tmp_gaia_fil;
      hg->fcdb_gaia_mag  = tmp_gaia_mag;
      hg->fcdb_gaia_diam  = tmp_gaia_diam;
      hg->fcdb_2mass_fil  = tmp_2mass_fil;
      hg->fcdb_2mass_mag  = tmp_2mass_mag;
      hg->fcdb_2mass_diam  = tmp_2mass_diam;
      hg->fcdb_wise_fil  = tmp_wise_fil;
      hg->fcdb_wise_mag  = tmp_wise_mag;
      hg->fcdb_wise_diam  = tmp_wise_diam;
      hg->fcdb_smoka_shot  = tmp_smoka_shot;
      for(i=0;i<NUM_SMOKA_SUBARU;i++){
	hg->fcdb_smoka_subaru[i]  = tmp_smoka_subaru[i];
      }
      for(i=0;i<NUM_SMOKA_KISO;i++){
	hg->fcdb_smoka_kiso[i]  = tmp_smoka_kiso[i];
      }
      for(i=0;i<NUM_SMOKA_OAO;i++){
	hg->fcdb_smoka_oao[i]  = tmp_smoka_oao[i];
      }
      for(i=0;i<NUM_SMOKA_MTM;i++){
	hg->fcdb_smoka_mtm[i]  = tmp_smoka_mtm[i];
      }
      for(i=0;i<NUM_SMOKA_KANATA;i++){
	hg->fcdb_smoka_kanata[i]  = tmp_smoka_kanata[i];
      }
      for(i=0;i<NUM_HST_IMAGE;i++){
	hg->fcdb_hst_image[i]  = tmp_hst_image[i];
      }
      for(i=0;i<NUM_HST_SPEC;i++){
	hg->fcdb_hst_spec[i]  = tmp_hst_spec[i];
      }
      for(i=0;i<NUM_HST_OTHER;i++){
	hg->fcdb_hst_other[i]  = tmp_hst_other[i];
      }
      for(i=0;i<NUM_ESO_IMAGE;i++){
	hg->fcdb_eso_image[i]  = tmp_eso_image[i];
      }
      for(i=0;i<NUM_ESO_SPEC;i++){
	hg->fcdb_eso_spec[i]  = tmp_eso_spec[i];
      }
      for(i=0;i<NUM_ESO_VLTI;i++){
	hg->fcdb_eso_vlti[i]  = tmp_eso_vlti[i];
      }
      for(i=0;i<NUM_ESO_POLA;i++){
	hg->fcdb_eso_pola[i]  = tmp_eso_pola[i];
      }
      for(i=0;i<NUM_ESO_CORO;i++){
	hg->fcdb_eso_coro[i]  = tmp_eso_coro[i];
      }
      for(i=0;i<NUM_ESO_OTHER;i++){
	hg->fcdb_eso_other[i]  = tmp_eso_other[i];
      }
      for(i=0;i<NUM_ESO_SAM;i++){
	hg->fcdb_eso_sam[i]  = tmp_eso_sam[i];
      }
      hg->fcdb_gemini_inst = tmp_gemini_inst;
    }
    else{
      hg->fcdb_band  = FCDB_BAND_NOP;
      hg->fcdb_mag   = 15;
      hg->fcdb_otype = FCDB_OTYPE_ALL;
      hg->fcdb_ned_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_ned_otype = FCDB_NED_OTYPE_ALL;
      if(hg->fcdb_type!=FCDB_TYPE_SIMBAD) rebuild_flag=TRUE;
      hg->fcdb_type  = FCDB_TYPE_SIMBAD;
      hg->fcdb_ned_ref = FALSE;
      hg->fcdb_gsc_fil = TRUE;
      hg->fcdb_gsc_mag = 19;
      hg->fcdb_gsc_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_ps1_fil = TRUE;
      hg->fcdb_ps1_mag = 19;
      hg->fcdb_ps1_diam = FCDB_PS1_ARCMIN_MAX;
      hg->fcdb_ps1_mindet = 2;
      hg->fcdb_sdss_search = FCDB_SDSS_SEARCH_IMAG;
      for(i=0;i<NUM_SDSS_BAND;i++){
	hg->fcdb_sdss_fil[i] = TRUE;
	hg->fcdb_sdss_magmin[i] = 0;
	hg->fcdb_sdss_magmax[i] = 20;
      }
      hg->fcdb_sdss_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_usno_fil = TRUE;
      hg->fcdb_usno_mag = 19;
      hg->fcdb_usno_diam = FCDB_USNO_ARCMIN_MAX;
      hg->fcdb_gaia_fil = TRUE;
      hg->fcdb_gaia_mag = 19;
      hg->fcdb_gaia_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_2mass_fil = TRUE;
      hg->fcdb_2mass_mag = 12;
      hg->fcdb_2mass_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_wise_fil = TRUE;
      hg->fcdb_wise_mag = 15;
      hg->fcdb_wise_diam = FCDB_ARCMIN_MAX;
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
      hg->fcdb_gemini_inst=GEMINI_INST_ANY;
    }

    if(flagFC){
      if(hg->fcdb_type==FCDB_TYPE_SIMBAD)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SIMBAD");
      else if(hg->fcdb_type==FCDB_TYPE_NED)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"NED");
      else if(hg->fcdb_type==FCDB_TYPE_GSC)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"GSC 2.3");
      else if(hg->fcdb_type==FCDB_TYPE_PS1)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"PanSTARRS-1");
      else if(hg->fcdb_type==FCDB_TYPE_SDSS)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SDSS DR14");
      else if(hg->fcdb_type==FCDB_TYPE_LAMOST)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"LAMOST DR3");
      else if(hg->fcdb_type==FCDB_TYPE_USNO)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"USNO-B");
      else if(hg->fcdb_type==FCDB_TYPE_GAIA)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"GAIA DR1");
      else if(hg->fcdb_type==FCDB_TYPE_2MASS)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"2MASS");
      else if(hg->fcdb_type==FCDB_TYPE_WISE)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"WISE");
      else if(hg->fcdb_type==FCDB_TYPE_IRC)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"AKARI/IRC");
      else if(hg->fcdb_type==FCDB_TYPE_FIS)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"AKARI/FIS");
      else if(hg->fcdb_type==FCDB_TYPE_SMOKA)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SMOKA");
      else if(hg->fcdb_type==FCDB_TYPE_HST)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"HST archive");
      else if(hg->fcdb_type==FCDB_TYPE_ESO)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"ESO archive");
      else if(hg->fcdb_type==FCDB_TYPE_GEMINI)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"Gemini archive");
    }

    if(rebuild_flag) rebuild_fcdb_tree(hg);
  }

  flagChildDialog=FALSE;
  g_free(cdata);
}


void change_fcdb_para(GtkWidget *w, gpointer gdata)
{ 
  confPropFCDB *cdata;

  cdata=(confPropFCDB *)gdata;

  cdata->mode=1;

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(cdata->fcdb_group); i++){
      w = g_slist_nth_data(cdata->fcdb_group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	cdata->fcdb_type  = g_slist_length(cdata->fcdb_group) -1 - i;
	break;
      }
    }
  }
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}


void radio_fcdb(GtkWidget *button, gpointer gdata)
{ 
  typHOE *hg;
  confPropFCDB *cdata;
  GSList *group=NULL;

  hg=(typHOE *)gdata;

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(group); i++){
      w = g_slist_nth_data(group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->query_note),g_slist_length(group)-1-i);
	break;
      }
    }
  }
}


