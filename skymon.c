//    HDS OPE file Editor
//      skymon.c : Sky Monitor Using Cairo  
//                                           2008.5.8  A.Tajitsu

#include"main.h"    // 設定ヘッダ


void select_skymon_calendar();
void popup_skymon_calendar();

void close_skymon();
#ifdef USE_GTK3
gboolean draw_skymon_cb();
#else
static gboolean configure_skymon();
gboolean expose_skymon_cairo();
#endif
gboolean configure_skymon_cb();

static gint button_signal();
void my_cairo_arc_center();
void my_cairo_object();
void my_cairo_object2();
void my_cairo_object_nst();
void my_cairo_object2_nst();
void my_cairo_std();
void my_cairo_std2();
void my_cairo_moon();
void my_cairo_sun();

static void cc_skymon_mode ();
//void refresh_skymon();

static void skymon_morning();
static void skymon_evening();

static void skymon_fwd();
gint skymon_go();
static void skymon_rev();
gint skymon_back();
//void skymon_set_time_current();

gboolean update_azel2();
//void get_current_obs_time();

#ifdef USE_GTK3
GdkPixbuf *pixbuf_skymon=NULL;
#else
GdkPixmap *pixmap_skymon=NULL;
#endif

static int adj_change=0;
static int val_pre=0;

static gint cc_set_adj_time (GtkAdjustment *adj) 
{
  adj_change=(gint)gtk_adjustment_get_value(adj);
  return 0;
}


static gint time_spin_input(GtkSpinButton *spin, 
			    gdouble *new_val,
			    gpointer gdata){
  const gchar *text;
  gchar **str;
  gboolean found=FALSE;
  gint hours;
  gint minutes;
  gchar *endh;
  gchar *endm;
  typHOE *hg=(typHOE *)gdata;

  text=gtk_entry_get_text(GTK_ENTRY(spin));
  str=g_strsplit(text, ":", 2);
  
  if(g_strv_length(str)==2){
    hours=strtol(str[0], &endh, 10);
    minutes=strtol(str[1], &endm, 10);
    if(!*endh && !*endm &&
       0 <= hours && hours < 24 &&
       0 <= minutes && minutes < 60){

      hg->skymon_time=hours*60+minutes;
      hg->skymon_hour=hours;
      hg->skymon_min=minutes;
      found=TRUE;
    }
  }
  g_strfreev(str);

  if(!found){
    return GTK_INPUT_ERROR;
  }

  val_pre=0;
  return TRUE;
}


static gint time_spin_output(GtkSpinButton *spin, gpointer gdata){
  GtkAdjustment *adj;
  gchar *buf=NULL;
  gdouble hours;
  gdouble minutes;
  gint time_val;
  gint adj_val;
  typHOE *hg=(typHOE *)gdata;

  adj=gtk_spin_button_get_adjustment(spin);
  adj_val=(gint)gtk_adjustment_get_value(adj);
  hours = (gdouble)adj_val/60.0;
  minutes = (hours-floor(hours))*60.0;
  time_val=(gint)hours*60+(gint)(floor(minutes+0.5));
  if(time_val==hg->skymon_time){
    buf=g_strdup_printf("%02.0lf:%02.0lf",floor(hours),floor(minutes+0.5));
  }
  else if(adj_val!=0){
    hg->skymon_time+=adj_change-val_pre;
    val_pre=adj_change;
    if(hg->skymon_time>60*24) hg->skymon_time-=60*24;
    if(hg->skymon_time<0)     hg->skymon_time+=60*24;
    hg->skymon_hour=hg->skymon_time/60;
    hg->skymon_min=hg->skymon_time-hg->skymon_hour*60;

    buf=g_strdup_printf("%02d:%02d",hg->skymon_hour,hg->skymon_min);
  }
  if(buf){
    if(strcmp(buf, gtk_entry_get_text(GTK_ENTRY(spin))))
      gtk_entry_set_text(GTK_ENTRY(spin),buf);
    g_free(buf);
  }

  return TRUE;
}


void set_skymon_e_date(typHOE *hg){
  gchar *tmp;
  if(!flagSkymon) return;
    
  tmp=g_strdup_printf("%s %d, %d",
		      cal_month[hg->skymon_month-1],
		      hg->skymon_day,
		      hg->skymon_year);

  gtk_entry_set_text(GTK_ENTRY(hg->skymon_e_date),tmp);
  g_free(tmp);
}

void select_skymon_calendar (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  gtk_calendar_get_date(GTK_CALENDAR(widget),
			&hg->skymon_year,
			&hg->skymon_month,
			&hg->skymon_day);
  hg->skymon_month++;

  hg->fr_year =hg->skymon_year;
  hg->fr_month=hg->skymon_month;
  hg->fr_day  =hg->skymon_day;

  set_skymon_e_date(hg);
  set_fr_e_date(hg);
  set_plot_e_date(hg);

  if(flagSkymon){
    if(hg->skymon_mode==SKYMON_SET){
      calcpa2_skymon(hg);
      draw_skymon_cairo(hg->skymon_dw,hg);
    }
  }

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,hg);
  }

  gtk_main_quit();
}

void popup_skymon_calendar (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *calendar;
  GtkAllocation *allocation=g_new(GtkAllocation, 1);
  gint root_x, root_y;
  typHOE *hg=(typHOE *)gdata;
  gtk_widget_get_allocation(widget,allocation);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_get_position(GTK_WINDOW(hg->skymon_main),&root_x,&root_y);

  my_signal_connect(dialog,"delete-event",delete_main_quit,NULL);
  gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);

  calendar=gtk_calendar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     calendar,FALSE, FALSE, 0);

  gtk_calendar_select_month(GTK_CALENDAR(calendar),
			    hg->skymon_month-1,
			    hg->skymon_year);

  gtk_calendar_select_day(GTK_CALENDAR(calendar),
			  hg->skymon_day);

  my_signal_connect(calendar,
		    "day-selected-double-click",
		    select_skymon_calendar, 
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



// Create Sky Monitor Window
void create_skymon_dialog(typHOE *hg)
{
  GtkWidget *vbox, *ebox, *entry;
  GtkWidget *hbox, *hbox1;
  GtkWidget *frame, *check, *label, *button, *spinner;
  GSList *group=NULL;
  GtkAdjustment *adj;
  GdkPixbuf *icon;
  gchar *tmp;

  hg->skymon_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), "HOE : Sky Monitor");
  
  my_signal_connect(hg->skymon_main,
		    "destroy",
		    close_skymon, 
		    (gpointer)hg);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_main), vbox);
  gtk_widget_show_all(vbox);


  // Menu
  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  hg->skymon_frame_mode = gtkut_frame_new ("<b>Mode</b>");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_mode, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_mode), 5);


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    /*
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Current",
		       1, SKYMON_CUR, -1);
    if(hg->skymon_mode==SKYMON_CUR) iter_set=iter;
    */	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Set",
		       1, SKYMON_SET, -1);
    if(hg->skymon_mode==SKYMON_SET) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Plan (Object)",
		       1, SKYMON_PLAN_OBJ, -1);
    if(hg->skymon_mode==SKYMON_PLAN_OBJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Plan (Time)",
		       1, SKYMON_PLAN_TIME, -1);
    if(hg->skymon_mode==SKYMON_PLAN_TIME) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (hg->skymon_frame_mode), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_skymon_mode,
		       (gpointer)hg);
  }

  hg->skymon_frame_date = gtkut_frame_new ("<b>Date</b>");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_date, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_date), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_date), hbox1);

  
  hg->skymon_year=hg->fr_year;
  hg->skymon_month=hg->fr_month;
  hg->skymon_day=hg->fr_day;

  hg->skymon_e_date = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_e_date,FALSE,FALSE,0);
  gtk_editable_set_editable(GTK_EDITABLE(hg->skymon_e_date),FALSE);
  my_entry_set_width_chars(GTK_ENTRY(hg->skymon_e_date),12);

  set_skymon_e_date(hg);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,TRUE,TRUE,0);
  my_signal_connect(button,"pressed",
  		    popup_skymon_calendar, 
 		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

  tmp=g_strdup_printf("<b>%s</b>", hg->obs_tzname);
  hg->skymon_frame_time = gtkut_frame_new (tmp);
  g_free(tmp);
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_time, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_time), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_time), hbox1);

  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;

  hg->skymon_adj_min = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_time,
							   0, 60*24,
							   10.0, 60.0, 0);
  spinner =  gtk_spin_button_new (hg->skymon_adj_min, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(GTK_SPIN_BUTTON(spinner)),5);
  my_signal_connect (hg->skymon_adj_min, "value-changed",
  		     cc_set_adj_time,
  		     NULL);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "output",
  		     time_spin_output,
		     (gpointer)hg);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "input",
  		     time_spin_input,
  		     (gpointer)hg);

  /*
  hg->skymon_adj_hour = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_hour,
							    -6, 30,
							    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->skymon_adj_hour, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->skymon_adj_hour, "value_changed",
		     cc_get_adj,
		     &hg->skymon_hour);

  label=gtk_label_new(":");
  gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE,FALSE,1);

  hg->skymon_adj_min = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_min,
							   0, 59,
							   1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->skymon_adj_min, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->skymon_adj_min, "value_changed",
		     cc_get_adj,
		     &hg->skymon_min);
  */

  frame = gtkut_frame_new ("<b>Action</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);


#ifdef USE_GTK3
  hg->skymon_button_set=gtkut_button_new_from_icon_name(NULL, "go-jump");
#else
  hg->skymon_button_set=gtkut_button_new_from_stock(NULL, GTK_STOCK_OK);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_set), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_set,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_set,"pressed",
		    skymon_set_and_draw, 
		    (gpointer)hg);

#ifdef USE_GTK3
  hg->skymon_button_even=gtkut_button_new_from_icon_name(NULL, "media-skip-backward");
#else
  hg->skymon_button_even=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_PREVIOUS);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_even), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_even,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_even,"pressed",
		    skymon_evening, 
		    (gpointer)hg);


#ifdef USE_GTK3
  hg->skymon_button_rev=gtkut_toggle_button_new_from_icon_name(NULL, "media-seek-backward");
#else
  hg->skymon_button_rev=gtkut_toggle_button_new_from_stock(NULL, GTK_STOCK_MEDIA_REWIND);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_rev), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_rev,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_rev,"toggled",
		    skymon_rev, 
		    (gpointer)hg);


#ifdef USE_GTK3
  hg->skymon_button_fwd=gtkut_toggle_button_new_from_icon_name(NULL, "media-seek-forward");
#else
  hg->skymon_button_fwd=gtkut_toggle_button_new_from_stock(NULL, GTK_STOCK_MEDIA_FORWARD);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_fwd), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_fwd,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_fwd,"toggled",
		    skymon_fwd, 
		    (gpointer)hg);


#ifdef USE_GTK3
  hg->skymon_button_morn=gtkut_button_new_from_icon_name(NULL, "media-skip-forward");
#else
  hg->skymon_button_morn=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_NEXT);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_morn), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_morn,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_morn,"pressed",
		    skymon_morning, 
		    (gpointer)hg);


  if(hg->skymon_mode==SKYMON_CUR){
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
  }

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL, "view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_REFRESH);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    refresh_skymon, 
		    (gpointer)hg);

  icon = gdk_pixbuf_new_from_resource ("/icons/pdf_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_skymon_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif


  /*
  hg->skymon_frame_sz = gtkut_frame_new ("Sz.");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_sz, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_sz), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_sz), hbox1);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_objsz,
					    0, 16,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->skymon_objsz);
  */
  
  // Drawing Area
  ebox=gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(vbox), ebox, TRUE, TRUE, 0);
  hg->skymon_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->skymon_dw, hg->sz_skymon, hg->sz_skymon);
  gtk_container_add(GTK_CONTAINER(ebox), hg->skymon_dw);
  gtk_widget_set_app_paintable(hg->skymon_dw, TRUE);
  gtk_widget_show(hg->skymon_dw);

#ifdef USE_GTK3
  my_signal_connect(hg->skymon_dw, 
		    "draw", 
		    draw_skymon_cb,
		    (gpointer)hg);

  my_signal_connect(hg->skymon_dw, 
		    "configure-event", 
		    configure_skymon_cb,
		    (gpointer)hg);
#else
  my_signal_connect(hg->skymon_dw, 
		    "expose-event", 
		    expose_skymon_cairo,
		    (gpointer)hg);

  my_signal_connect(hg->skymon_dw, 
		    "configure-event", 
		    configure_skymon,
		    (gpointer)hg);
#endif
  
  gtk_widget_set_events(ebox, GDK_BUTTON_PRESS_MASK);
  my_signal_connect(ebox, 
		    "button-press-event", 
		    button_signal,
		    (gpointer)hg);

  if(hg->skymon_mode==SKYMON_CUR){
    if(hg->skymon_timer<0){
      hg->skymon_timer=g_timeout_add(AZEL_INTERVAL, 
				     (GSourceFunc)update_azel2, 
				     (gpointer)hg);
    }
  }

  gtk_widget_show_all(hg->skymon_main);
  draw_skymon_cairo(hg->skymon_dw, hg);
}


static gint button_signal(GtkWidget *widget, 
		   GdkEventButton *event, 
		   gpointer userdata){
  typHOE *hg;
  gint x,y;
  gint i_list, i_sel=-1, i;
  gdouble sep=10.0, r_min=1000.0, r;
  

  hg=(typHOE *)userdata;

  if ( event->button==1 ) {
    switch(hg->skymon_mode){
    case SKYMON_SET:
    case SKYMON_CUR:
    
#ifdef USE_GTK3
      gdk_window_get_device_position(gtk_widget_get_window(widget),
				     event->device, &x,&y,NULL);
#else
      gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif
      
      if((x-hg->win_cx)*(x-hg->win_cx)+(y-hg->win_cy)*(y-hg->win_cy)<
	 (hg->win_r*hg->win_r)){
	for(i_list=0;i_list<hg->i_max;i_list++){
	  if((hg->obj[i_list].x>0)&&(hg->obj[i_list].y>0)){
	    if((fabs(hg->obj[i_list].x-x)<sep)
	       &&(fabs(hg->obj[i_list].y-y)<sep)){
	      r=(hg->obj[i_list].x-x)*(hg->obj[i_list].x-x)
		+(hg->obj[i_list].y-y)*(hg->obj[i_list].y-y);
	      if(r<r_min){
		i_sel=i_list;
		r_min=r;
	      }
	    }
	  }
	}
	
	if(i_sel>=0){
	  hg->plot_i=i_sel;
	  draw_skymon_cairo(hg->skymon_dw,hg);
	  refresh_plot(NULL, (gpointer)hg);
	  if(flagPlan){
	    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_combo),
				     i_sel+1);
	    if(hg->inst==INST_HSC){
	      gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_focus_combo),
				       i_sel+1);
	    }
	  }
	  
	  {
	    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
	    GtkTreePath *path;
	    GtkTreeIter  iter;
	    
	    path=gtk_tree_path_new_first();
	    
	    for(i=0;i<hg->i_max;i++){
	      gtk_tree_model_get_iter (model, &iter, path);
	      gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i_list, -1);
	      i_list--;
	      
	      if(i_list==i_sel){
		gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note),
					       hg->page[NOTE_OBJ]);
		gtk_widget_grab_focus (hg->objtree);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->objtree), path, NULL, FALSE);
		break;
	      }
	      else{
		gtk_tree_path_next(path);
	      }
	    }
	    gtk_tree_path_free(path);
	  }

	}
      }
      break;

    default:
      break;
    }
  }
  
  return FALSE;
}


void close_skymon(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->skymon_timer>0){
    g_source_remove(hg->skymon_timer);
    hg->skymon_timer=-1;
  }

  gtk_widget_destroy(GTK_WIDGET(w));
  flagSkymon=FALSE;
}

#ifdef USE_GTK3
gboolean draw_skymon_cb(GtkWidget *widget,
			cairo_t *cr, 
			gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixbuf_skymon) draw_skymon_cairo(widget,hg);
  gdk_cairo_set_source_pixbuf(cr, pixbuf_skymon, 0, 0);
  cairo_paint(cr);
  return(TRUE);
}

gboolean configure_skymon_cb(GtkWidget *widget,
			     GdkEventConfigure *event, 
			     gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  draw_skymon_cairo(widget,hg);
  return(TRUE);
}
#else
static gboolean configure_skymon (GtkWidget *widget, 
			   GdkEventConfigure *event, 
			   gpointer data)
{
  typHOE *hg = (typHOE *)data;
  if(!pixmap_skymon) return(TRUE);

  draw_skymon_cairo(widget, hg);
  //draw_skymon_pixmap(widget, hg);
  return(TRUE);
}

gboolean expose_skymon_cairo(GtkWidget *widget,
			     GdkEventExpose *event, 
			     gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixmap_skymon) draw_skymon_cairo(widget,hg);
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    GtkStyle *style=gtk_widget_get_style(widget);
    gtk_widget_get_allocation(widget,allocation);

    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_skymon,
		      0,0,0,0,
		      allocation->width,
		      allocation->height);
    g_free(allocation);
  }
  return (TRUE);
}
#endif

gboolean draw_skymon_cairo(GtkWidget *widget, typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  gdouble e_h;
  double x,y;
  gint i_list;
  gint from_set, to_rise;
  int width, height;

  if(!flagSkymon) return (FALSE);

  switch(hg->skymon_output){
  case SKYMON_OUTPUT_PDF:
    width= SKYMON_WIDTH;
    height= SKYMON_HEIGHT;

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

    cairo_set_source_rgb(cr, 1, 1, 1);
    break;

  default:
    {
      GtkAllocation *allocation=g_new(GtkAllocation, 1);
      gtk_widget_get_allocation(widget,allocation);

      width= allocation->width;
      height= allocation->height;
      g_free(allocation);
    }

    hg->win_cx=(gdouble)width/2.0;
    hg->win_cy=(gdouble)height/2.0;
    if(width < height){
      hg->win_r=hg->win_cx*0.9;
    }
    else{
      hg->win_r=hg->win_cy*0.9;
    }

    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
    }

#ifdef USE_GTK3
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);

    cr = cairo_create(surface);
#else
    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				   width,
				   height,
				   -1);
  
    cr = gdk_cairo_create(pixmap_skymon);
#endif
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
    break;
  }

  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  /* draw a circle */
  

  //El =0
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  my_cairo_arc_center (cr, width, height, 0.0); 
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 0.0); 
  //cairo_fill(cr);
  cairo_stroke(cr);

  //El =15
  my_cairo_arc_center (cr, width, height, 15.0); 
  cairo_set_line_width(cr,1.0);
  //cairo_fill(cr);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);
  
  //El =30
  my_cairo_arc_center (cr, width, height, 30.0); 
  //cairo_fill(cr);
  cairo_stroke(cr);
  
  //El =60
  my_cairo_arc_center (cr, width, height, 60.0); 
  //cairo_fill(cr);
  cairo_stroke(cr);
  
  // ZENITH
  my_cairo_arc_center (cr, width, height, 89.0); 
  cairo_fill(cr);

  // N-S
  cairo_move_to ( cr, width/2,
		  (width<height ? height/2-width/2 * 0.9 : height*0.05) ); 
  cairo_line_to ( cr, width/2,
		  (width<height ? height/2+width/2 * 0.9 : height*0.95) ); 
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);
  
  // W-E
  cairo_move_to ( cr, 
		  (width<height ? width*0.05 : width/2-height/2*0.9),
		   height/2);
  cairo_line_to ( cr, 
		  (width<height ? width*0.95 : width/2+height/2*0.9),
		   height/2);
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);

  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);

  // N
  cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
  cairo_text_extents (cr, "N", &extents);
  x = 0-(extents.width/2 + extents.x_bearing);
  y = 0;
  x += width/2; 
  y += (width<height ? height/2-width/2 * 0.9 : height*0.05) -2; 
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "N");

  // S
  cairo_text_extents (cr, "S", &extents);
  x = 0-(extents.width/2 + extents.x_bearing);
  y = 0+extents.height;
  x += width/2; 
  y += (width<height ? height/2+width/2 * 0.9 : height*0.95)+2; 
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "S");

  // E
  cairo_text_extents (cr, "E", &extents);
  x = 0-extents.width;
  y = 0-(extents.height/2 + extents.y_bearing);
  x += (width<height ? width*0.05 : width/2-height/2*0.9) -2;
  y += height/2;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "E");

  // W
  cairo_text_extents (cr, "W", &extents);
  x = 0;
  y = 0-(extents.height/2 + extents.y_bearing);
  x += (width<height ? width*0.95 : width/2+height/2*0.9) +2;
  y += height/2;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "W");

  // Date 
  {
    gchar *tmp;
    time_t t;
    struct tm *tmpt;
    int year, month, day, hour, min;
    double sec;
    struct ln_zonedate zonedate;
    struct ln_date date;
    gdouble base_height,w_rise,w_digit;
    cairo_text_extents_t ext_s;

    switch(hg->skymon_mode){
    case SKYMON_SET:
      year=hg->skymon_year;
      month=hg->skymon_month;
      day=hg->skymon_day;
      
      hour=hg->skymon_hour;
      min=hg->skymon_min;
      sec=0;

      break;

    case SKYMON_CUR:
      get_current_obs_time(hg,&year, &month, &day, &hour, &min, &sec);
      break;

    case SKYMON_PLAN_OBJ:
    case SKYMON_PLAN_TIME:
      year=hg->fr_year;
      month=hg->fr_month;
      day=hg->fr_day;

      break;
    }
    
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    tmp=g_strdup_printf("%s %d, %d",cal_month[month-1],day,year);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_text_extents (cr, tmp, &extents);
    e_h=extents.height;
    cairo_move_to(cr,5,+e_h+5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    if((hg->skymon_mode==SKYMON_SET)||(hg->skymon_mode==SKYMON_CUR)){
      tmp=g_strdup_printf("%s=%02d:%02d",hg->obs_tzname,hour,min);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_move_to(cr,10,+e_h*2+10);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
      
      zonedate.years=year;
      zonedate.months=month;
      zonedate.days=day;
      zonedate.hours=hour;
      zonedate.minutes=min;
      zonedate.seconds=sec;
      zonedate.gmtoff=(long)hg->obs_timezone*60;
      
      ln_zonedate_to_date(&zonedate, &date);
      tmp=g_strdup_printf("UT =%02d:%02d",
			  date.hours,date.minutes);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_move_to(cr,10,+e_h*3+15);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
      
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
      if(date.days!=day){
	if(hg->obs_timezone>0){
	  cairo_show_text(cr, " [-1day]");
	}
	else{
	  cairo_show_text(cr, " [+1day]");
	}
      }
      
      if(hg->skymon_mode==SKYMON_SET){
	tmp=g_strdup_printf("LST=%02d:%02d",hg->skymon_lst_hour,hg->skymon_lst_min);
      }
      else{
	tmp=g_strdup_printf("LST=%02d:%02d",hg->lst_hour,hg->lst_min);
      }
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_move_to(cr,10,+e_h*4+20);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      glong plan_stime, plan_etime;
      
      plan_stime=hg->plan[hg->plot_i_plan].sod;
      plan_etime=hg->plan[hg->plot_i_plan].sod+hg->plan[hg->plot_i_plan].time+hg->plan[hg->plot_i_plan].stime;

      if(plan_stime!=0){
	if(plan_stime!=plan_etime){
	  tmp=g_strdup_printf("%s=%02ld:%02ld -- %02ld:%02ld",
			      hg->obs_tzname,
			      plan_stime /60/60,
			      (plan_stime %(60*60))/60,
			      plan_etime /60/60,
			      (plan_etime %(60*60))/60);
	  
	}
	else{
	  tmp=g_strdup_printf("%s=%02ld:%02ld",
			      hg->obs_tzname,
			      plan_stime /60/60,
			      (plan_stime %(60*60))/60);
	}
	
	if(hg->plan[hg->plot_i_plan].backup){
	  cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 1.0);
	}
	else{
	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	}
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	cairo_move_to(cr,10,+e_h*2+10);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
	
	if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
	  if(hg->plan[hg->plot_i_plan].backup){
	    tmp=g_strdup_printf("   (Backup) %s",
				hg->obj[hg->plan[hg->plot_i_plan].obj_i].name);
	    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
	    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);
	  }
	  else{
	    tmp=g_strdup_printf("   \" %s \"",
				hg->obj[hg->plan[hg->plot_i_plan].obj_i].name);
	    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	  }
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	  cairo_move_to(cr,10,+e_h*3+15);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);

	}
      }
    }

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    
    base_height=e_h*5+35;
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);

    cairo_text_extents (cr, "99:99", &ext_s);

    cairo_move_to(cr,5,base_height);
    cairo_show_text(cr, "Set");

    cairo_move_to(cr,5,base_height+ext_s.height+4);
    cairo_show_text(cr, "Rise");

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
    cairo_move_to(cr,5+ext_s.width,base_height-ext_s.height-2);
    cairo_show_text(cr, "Sun");
    cairo_move_to(cr,5+ext_s.width*2+5,base_height-ext_s.height-2);
    cairo_show_text(cr, "Tw12");
    cairo_move_to(cr,5+ext_s.width*3+10,base_height-ext_s.height-2);
    cairo_show_text(cr, "Tw18");

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.c_set.hours,hg->sun.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.c_set.hours,hg->atw12.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.c_set.hours,hg->atw18.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.s_set.hours,hg->sun.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.s_set.hours,hg->atw12.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.s_set.hours,hg->atw18.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }

    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.c_rise.hours,hg->sun.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.c_rise.hours,hg->atw12.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.c_rise.hours,hg->atw18.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.s_rise.hours,hg->sun.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.s_rise.hours,hg->atw12.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.s_rise.hours,hg->atw18.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }


    // Moon
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, 12.0);

    //if((hg->skymon_mode==SKYMON_SET)||(hg->skymon_mode==SKYMON_CUR)){
      
      if(hg->skymon_mode==SKYMON_CUR){
	tmp=g_strdup_printf("RA=%02d:%02d:%04.1lf Dec=%+03d:%02d:%04.1lf",
			    hg->moon.c_ra.hours,hg->moon.c_ra.minutes,hg->moon.c_ra.seconds,
			    hg->moon.c_dec.neg==1 ? 
			    -hg->moon.c_dec.degrees : hg->moon.c_dec.degrees,
			    hg->moon.c_dec.minutes,hg->moon.c_dec.seconds);
      }
      else{
	tmp=g_strdup_printf("RA=%02d:%02d:%04.1lf Dec=%+03d:%02d:%04.1lf",
			    hg->moon.s_ra.hours,hg->moon.s_ra.minutes,hg->moon.s_ra.seconds,
			    hg->moon.s_dec.neg==1 ? 
			    -hg->moon.s_dec.degrees : hg->moon.s_dec.degrees,
			    hg->moon.s_dec.minutes,hg->moon.s_dec.seconds);
      }
      cairo_move_to(cr,10,height-extents.height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
      //}

    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("Illum=%4.1f%%",hg->moon.c_disk*100);
    }
    else{
      tmp=g_strdup_printf("Illum=%4.1f%%",hg->moon.s_disk*100);
    }
    cairo_move_to(cr,10,height-extents.height*2-5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
   
    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("Age=%.1lf", hg->moon.c_age);
    }
    else{
      tmp=g_strdup_printf("Age=%.1lf", hg->moon.s_age); 
    }
    cairo_move_to(cr,10,height-e_h*3-10);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("Set=%02d:%02d",
			  hg->moon.c_set.hours,hg->moon.c_set.minutes);
    }
    else{
      tmp=g_strdup_printf("Set=%02d:%02d",
			  hg->moon.s_set.hours,hg->moon.s_set.minutes);
    }
    cairo_move_to(cr,10,height-extents.height*4-15);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
      
    if(hg->skymon_mode==SKYMON_CUR){
      tmp=g_strdup_printf("Rise=%02d:%02d",
			  hg->moon.c_rise.hours,hg->moon.c_rise.minutes);
    }
    else{
      tmp=g_strdup_printf("Rise=%02d:%02d",
			  hg->moon.s_rise.hours,hg->moon.s_rise.minutes);
    }
    cairo_move_to(cr,10,height-extents.height*5-20);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    cairo_move_to(cr,5,height-extents.height*6-25);
    cairo_show_text(cr, "Moon");
    
    
    if(hg->skymon_mode==SKYMON_SET){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_set_font_size (cr, 12.0);
      cairo_text_extents (cr, "!!! Set Mode !!!", &extents);
      cairo_move_to(cr,width-extents.width-10,extents.height+4);
      cairo_show_text(cr, "!!! Set Mode !!!");
    }
    else if ((hg->skymon_mode==SKYMON_PLAN_OBJ)||(hg->skymon_mode==SKYMON_PLAN_TIME)){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_set_font_size (cr, 12.0);
      cairo_text_extents (cr, "Observing Plan", &extents);
      cairo_move_to(cr,width-extents.width-10,extents.height+4);
      cairo_show_text(cr, "Observing Plan");
    }
  
      
    if(hg->skymon_mode==SKYMON_SET){
      from_set=(hour>=24 ? 
		(hour-24)*60+min-hg->sun.s_set.hours*60-hg->sun.s_set.minutes :
		hour*60+min-hg->sun.s_set.hours*60-hg->sun.s_set.minutes);
      to_rise=(hour>=24 ? 
	       hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes-(hour-24)*60-min :
	       hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes-(hour)*60-min);
      }
    else if(hg->skymon_mode==SKYMON_CUR){
      from_set=(hour>=24 ? 
		(hour-24)*60+min-hg->sun.c_set.hours*60-hg->sun.c_set.minutes :
		hour*60+min-hg->sun.c_set.hours*60-hg->sun.c_set.minutes);
      to_rise=(hour>=24 ? 
	       hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes-(hour-24)*60-min :
	       hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes-(hour)*60-min);
    }
    
    if((hg->skymon_mode==SKYMON_SET)||(hg->skymon_mode==SKYMON_CUR)){
      if((from_set<0)&&(to_rise<0)){ 
	cairo_text_extents_t extents2;
	
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 0.7, 0.7, 1.0, 1.0);
	cairo_set_font_size (cr, 80.0);
	cairo_text_extents (cr, "Daytime", &extents);
	x = width / 2 -extents.width/2;
	y = height /2 -(extents.height/2 + extents.y_bearing);
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, "Daytime");
	
	cairo_set_font_size (cr, 15.0);
	cairo_text_extents (cr, "Have a good sleep...", &extents2);
	x = width / 2 +extents.width/2 -extents2.width;
	y = height /2 + (extents.height/2 ) + (extents2.height) +5;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, "Have a good sleep...");
	
	
	
      }
      else if((from_set>0)&&(from_set<60)){
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 0.8, 0.6, 1.0, 1.0);
	cairo_set_font_size (cr, 12.0);
	tmp=g_strdup_printf("%02dmin after SunSet",from_set);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,width-extents.width-10,extents.height*2+4+5);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
      else if((to_rise>0)&&(to_rise<60)){
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 0.8, 0.6, 1.0, 1.0);
	cairo_set_font_size (cr, 12.0);
	tmp=g_strdup_printf("%02dmin before SunRise",to_rise);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,width-extents.width-10,extents.height*2+4+5);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
    }
  }
  

  // Moon
  if(hg->skymon_mode==SKYMON_SET){
    my_cairo_moon(cr,width,height,
		  hg->moon.s_az,hg->moon.s_el,hg->moon.s_disk);
    my_cairo_sun(cr,width,height,
		 hg->sun.s_az,hg->sun.s_el);
  }
  else if(hg->skymon_mode==SKYMON_CUR){
    my_cairo_moon(cr,width,height,
		  hg->moon.c_az,hg->moon.c_el,hg->moon.c_disk);
    my_cairo_sun(cr,width,height,
		 hg->sun.c_az,hg->sun.c_el);
  }
    

  // for Plan orbit
  if(flagPlan){
    gdouble r, el_r;
    gdouble d_jd;
    struct ln_lnlat_posn observer;
    struct ln_hrz_posn ohrz0, ohrz1;
    struct ln_equ_posn oequ;
    gdouble JD_plan;

    JD_plan=hg->plan_jd1;
    
    if(hg->skymon_mode!=SKYMON_CUR){
      if( (hg->plan_jd1>0) && (hg->plot_i==hg->plan_trace)){
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
	cairo_set_line_width(cr, 4.0);
	
	observer.lat = hg->obs_latitude;
	observer.lng = hg->obs_longitude;
  
	oequ.ra=ra_to_deg(hg->obj[hg->plot_i].ra);
	oequ.dec=dec_to_deg(hg->obj[hg->plot_i].dec);

	d_jd=(hg->plan_jd2-hg->plan_jd1)/100.0;

	r= width<height ? width/2*0.9 : height/2*0.9;
	
	ln_get_hrz_from_equ (&oequ, &observer, JD_plan, &ohrz0);

	el_r = r * (90. - ohrz0.alt)/90.;
	x = width/2. + el_r*cos(M_PI/180.*(90-ohrz0.az));
	y = height/2. + el_r*sin(M_PI/180.*(90-ohrz0.az));

	cairo_move_to(cr, x, y);
	
	JD_plan+=d_jd;
	do{
	  ln_get_hrz_from_equ (&oequ, &observer, JD_plan, &ohrz1);

	  el_r = r * (90. - ohrz1.alt)/90.;
	  x = width/2. + el_r*cos(M_PI/180.*(90-ohrz1.az));
	  y = height/2. + el_r*sin(M_PI/180.*(90-ohrz1.az));

	  if( (ohrz0.alt>0) && (ohrz1.alt>0) ){
	    cairo_line_to(cr, x, y);
	    cairo_stroke(cr);
	  }
	  cairo_move_to(cr, x, y);

	  ohrz0=ohrz1;
	  JD_plan+=d_jd;
	}while(JD_plan < hg->plan_jd2);
      }
    }
  }
  
  // Object
  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  switch(hg->skymon_mode){
  case SKYMON_SET:
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].s_el>0){
	if((hg->obj[i_list].i_nst>=0)
	   &&(hg->nst[hg->obj[i_list].i_nst].s_fl!=0)){
	  my_cairo_object_nst(cr,hg,i_list,width,height,SKYMON_SET);
	}
	else{
	  my_cairo_object(cr,hg,i_list,width,height,SKYMON_SET);
	}
      }
      else{
	hg->obj[i_list].x=-1;
	hg->obj[i_list].y=-1;
      }
    }
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].s_el>0){
	if((hg->obj[i_list].i_nst>=0)
	   &&(hg->nst[hg->obj[i_list].i_nst].s_fl!=0)){
	  my_cairo_object2_nst(cr,hg,i_list,width,height,SKYMON_SET);
	}
	else{
	  my_cairo_object2(cr,hg,i_list,width,height,SKYMON_SET);
	}
      }
    }
    if(hg->std_i==hg->plot_i){
      for(i_list=0;i_list<hg->std_i_max;i_list++){
	if(hg->std[i_list].s_el>0){
	  if(hg->stddb_tree_focus!=i_list){
	    my_cairo_std(cr,width,height,
			 hg->std[i_list].s_az,hg->std[i_list].s_el,
			 &hg->std[i_list].x,&hg->std[i_list].y);
	  }
	}
	else{
	  hg->std[i_list].x=-1;
	  hg->std[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->std_i_max;i_list++){
	if(hg->std[i_list].s_el>0){
	  if(hg->stddb_tree_focus==i_list){
	    my_cairo_std2(cr,width,height,
			  hg->std[i_list].s_az,hg->std[i_list].s_el,
			  &hg->std[i_list].x,&hg->std[i_list].y);
	  }
	}
	else{
	  hg->std[i_list].x=-1;
	  hg->std[i_list].y=-1;
	}
      }
    }
    break;
 
  case SKYMON_CUR:
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].c_el>0){
	  if((hg->obj[i_list].i_nst>=0)
	     &&(hg->nst[hg->obj[i_list].i_nst].c_fl!=0)){
	    my_cairo_object_nst(cr,hg,i_list,width,height,SKYMON_CUR);
	  }
	  else{
	    my_cairo_object(cr,hg,i_list,width,height,SKYMON_CUR);
	  }
      }
      else{
	hg->obj[i_list].x=-1;
	hg->obj[i_list].y=-1;
      }
    }
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].c_el>0){
	if((hg->obj[i_list].i_nst>=0)
	   &&(hg->nst[hg->obj[i_list].i_nst].c_fl!=0)){
	  my_cairo_object2_nst(cr,hg,i_list,width,height,SKYMON_CUR);
	}
	else{
	  my_cairo_object2(cr,hg,i_list,width,height,SKYMON_CUR);
	}
      }
    }
    if(hg->std_i==hg->plot_i){
      for(i_list=0;i_list<hg->std_i_max;i_list++){
	if(hg->std[i_list].c_el>0){
	  if(hg->stddb_tree_focus!=i_list){
	    my_cairo_std(cr,width,height,
			 hg->std[i_list].c_az,hg->std[i_list].c_el,
			 &hg->std[i_list].x,&hg->std[i_list].y);
	  }
	}
	else{
	  hg->std[i_list].x=-1;
	  hg->std[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->std_i_max;i_list++){
	if(hg->std[i_list].c_el>0){
	  if(hg->stddb_tree_focus==i_list){
	    my_cairo_std2(cr,width,height,
			  hg->std[i_list].c_az,hg->std[i_list].c_el,
			  &hg->std[i_list].x,&hg->std[i_list].y);
	  }
	}
	else{
	  hg->std[i_list].x=-1;
	  hg->std[i_list].y=-1;
	}
      }
    }  
    break;

  case SKYMON_PLAN_OBJ:
  case SKYMON_PLAN_TIME:
    {
      gint i_pp;
      gdouble r, el_r;
      gdouble x, y;
      cairo_text_extents_t extents;
      gdouble el_r0;
      gdouble x0, y0, x1, x_old,y_old;
      gchar *str;

      
      r= width<height ? width/2*0.9 : height/2*0.9;

      // Moon
      if(hg->i_pp_moon_max>0){
	el_r = r * (90. - hg->moon.p_el[0])/90.;
	
	x_old = width/2. + el_r*cos(M_PI/180.*(90-hg->moon.p_az[0]));
	y_old = height/2. + el_r*sin(M_PI/180.*(90-hg->moon.p_az[0]));
      }
      
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 0.5);
      cairo_set_line_width(cr,15.);
      
      for(i_pp=1;i_pp<hg->i_pp_moon_max;i_pp++){
	cairo_move_to(cr,x_old,y_old);
	
	el_r = r * (90. - hg->moon.p_el[i_pp])/90.;
	
	x = width/2. + el_r*cos(M_PI/180.*(90-hg->moon.p_az[i_pp]));
	y = height/2. + el_r*sin(M_PI/180.*(90-hg->moon.p_az[i_pp]));
	if((hg->moon.p_el[i_pp]>0)&&(hg->moon.p_el[i_pp-1]>0)){
	  cairo_line_to(cr,x,y);
	  cairo_stroke(cr);
	}
	
	x_old=x;
	y_old=y;
	
      }
	
      //if((hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ)&&
      //	 (!hg->plan[hg->plot_i_plan].backup)){
	my_cairo_moon(cr,width,height,
		      hg->moon.s_az,hg->moon.s_el,hg->moon.s_disk);
	my_cairo_sun(cr,width,height,
		     hg->sun.s_az,hg->sun.s_el);
	//}

      if(hg->i_pp_max>0){
	el_r = r * (90. - hg->pp[0].el)/90.;
	
	x = width/2. + el_r*cos(M_PI/180.*(90-hg->pp[0].az));
	y = height/2. + el_r*sin(M_PI/180.*(90-hg->pp[0].az));
      }
      
      for(i_pp=0;i_pp<hg->i_pp_max;i_pp++){
	cairo_move_to(cr,x,y);
	
	el_r = r * (90. - hg->pp[i_pp].el)/90.;
	
	x = width/2. + el_r*cos(M_PI/180.*(90-hg->pp[i_pp].az));
	y = height/2. + el_r*sin(M_PI/180.*(90-hg->pp[i_pp].az));

	if(hg->pp[i_pp].start){
	  cairo_set_source_rgba(cr, 0.1, 0.4, 0.4, 0.5);
	  cairo_set_line_width(cr,1.0);
	  
	}
	else if(hg->pp[i_pp].i_plan==hg->plot_i_plan){
	  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	  cairo_set_line_width(cr,4.0);
	}
	else{
	  cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	  cairo_set_line_width(cr,3.0);
	}
	
	cairo_line_to(cr,x,y);
	cairo_stroke(cr);
	
	if(hg->pp[i_pp].start){
	  
	  cairo_move_to(cr,x,y);
	  
	  el_r0 = r * (90. - 15.)/90.;
	  
	  x0 = width/2. + el_r0*cos(M_PI/180.*(90-hg->pp[i_pp].az));
	  y0 = height/2. + el_r0*sin(M_PI/180.*(90-hg->pp[i_pp].az));

	  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.5);
	  cairo_set_line_width(cr,1.0);
	  cairo_line_to(cr,x0,y0);
	  
	  cairo_stroke(cr);

	  cairo_save (cr);
	  cairo_translate(cr,width/2,height/2);
	  x1 = (width<height ? width*0.95*75./90. : width/2+height/2*0.75)-width/2;
	  
	  cairo_rotate(cr,(-hg->pp[i_pp].az+90)*M_PI/180.);

	  if(hg->pp[i_pp].i_plan==hg->plot_i_plan){
	    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*1.2);
	    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	  }
	  else{
	    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz);
	    cairo_set_source_rgba(cr, 0.4, 0.1, 0.1, 1.0);
	  }

	  if(hg->skymon_mode==SKYMON_PLAN_OBJ){
	    if(hg->obj[hg->plan[hg->pp[i_pp].i_plan].obj_i].name)
	    cairo_text_extents (cr, hg->obj[hg->plan[hg->pp[i_pp].i_plan].obj_i].name, &extents);
	  }
	  else{
	    str=get_txt_tod(hg->plan[hg->pp[i_pp].i_plan].sod);
	    cairo_text_extents (cr, str, &extents);
	  }
	  cairo_move_to(cr,
			x1,
			-(extents.height/2 + extents.y_bearing));
	  
	  if(hg->skymon_mode==SKYMON_PLAN_OBJ){
	    if(hg->obj[hg->plan[hg->pp[i_pp].i_plan].obj_i].name)
	    cairo_show_text(cr, hg->obj[hg->plan[hg->pp[i_pp].i_plan].obj_i].name);
	  }
	  else{
	    cairo_show_text(cr, str);
	    if(str) g_free(str);
	  }
	  cairo_restore (cr);
	}
      }

    }

    break;
  }


  

  switch(hg->skymon_output){
  case SKYMON_OUTPUT_PDF:
    cairo_show_page(cr); 
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    break;

  case SKYMON_OUTPUT_WINDOW:
    cairo_destroy(cr);
#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    {
      GtkStyle *style=gtk_widget_get_style(widget);
      gdk_draw_drawable(gtk_widget_get_window(widget),
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymon,
			0,0,0,0,
			width,
			height);
    }
#endif
    break;

  default:
    cairo_destroy(cr);
    break;
  }

  return TRUE;
}

void my_cairo_arc_center(cairo_t *cr, gint w, gint h, gdouble r){
  cairo_arc(cr, 
	    w / 2, h / 2, 
	    (w < h ? w : h) / 2 * ((90. - r)/100.) , 
	    0, 2 * M_PI);
}

// Normal
void my_cairo_object(cairo_t *cr, typHOE *hg, gint i, 
		     gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(i==hg->plot_i) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_fill(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz);
    cairo_text_extents (cr, hg->obj[i].name, &extents);

    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-5);
    cairo_show_text(cr, hg->obj[i].name);
  }

}

// High-ligted
void my_cairo_object2(cairo_t *cr, typHOE *hg, gint i, 
		      gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(i!=hg->plot_i) return;
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_arc(cr, x, y, 14, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
  cairo_arc(cr, x, y, 12, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_arc(cr, x, y, 7, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
  cairo_arc(cr, x, y, 5, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*1.8);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    cairo_text_path(cr, hg->obj[i].name);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, 6);
    cairo_stroke(cr);
    
    cairo_new_path(cr);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_show_text(cr, hg->obj[i].name);
  }
}

void my_cairo_object_nst(cairo_t *cr, typHOE *hg, gint i,
			 gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(i==hg->plot_i) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;
  
    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;
  
    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
  cairo_set_line_width(cr, 2);
  
  cairo_move_to(cr, x-3, y-3);
  cairo_line_to(cr, x+3, y+3);
  cairo_stroke(cr);
  
  cairo_move_to(cr, x-3, y+3);
  cairo_line_to(cr, x+3, y-3);
  cairo_stroke(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    
    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-5);
    cairo_show_text(cr, hg->obj[i].name);
  }

}

void my_cairo_object2_nst(cairo_t *cr, typHOE *hg, gint i,
			  gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(i!=hg->plot_i) return;
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_arc(cr, x, y, 14, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
  cairo_arc(cr, x, y, 12, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_arc(cr, x, y, 7, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
  cairo_arc(cr, x, y, 5, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_new_path(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*1.8);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    cairo_text_path(cr, hg->obj[i].name);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 6);
    cairo_stroke(cr);
    
    cairo_new_path(cr);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_show_text(cr, hg->obj[i].name);
  }
}

void my_cairo_std(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble *objx, gdouble *objy){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  *objx=x;
  *objy=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1, 1, 1, 0.75);
  cairo_arc(cr, x, y, 5, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 1, 1, 1, 1.0);
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 0.9, 0.25, 0.9, 1.0);
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_set_line_width (cr, 1.5);
  cairo_stroke(cr);

  cairo_new_path(cr);
}


void my_cairo_std2(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble *objx, gdouble *objy){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  *objx=x;
  *objy=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
  cairo_arc(cr, x, y, 7, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-4, y-8);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-8, y-4);

  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+4, y-8);
  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+8, y-4);

  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+4, y+8);
  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+8, y+4);

  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-4, y+8);
  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-8, y+4);
  cairo_set_line_width (cr, 5.5);
  cairo_stroke (cr);
  
  cairo_set_source_rgba(cr, 1.0, 0.25, 0.25, 1.0);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-4, y-8);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-8, y-4);

  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+4, y-8);
  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+8, y-4);

  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+4, y+8);
  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+8, y+4);

  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-4, y+8);
  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-8, y+4);
  cairo_set_line_width (cr, 2.5);
  cairo_stroke (cr);


  cairo_set_source_rgba(cr, 1.0, 0.1, 0.1, 1.0);
  cairo_arc(cr, x, y, 4.5, 0, 2*M_PI);
  cairo_fill(cr);
  
  cairo_new_path(cr);
}


void my_cairo_moon(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble s_disk){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(el<=0) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 0.7, 0.7, 0.0, 1.0);
  cairo_arc(cr, x, y, 11, 0, 2*M_PI);
  cairo_fill(cr);

  if(s_disk>=1){
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc(cr, x, y, 10, 0, 2*M_PI);
    cairo_fill(cr);
  }
  else if(s_disk>0.0){
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc(cr, x, y, 10, -M_PI/2, M_PI/2);
    cairo_fill(cr);
    
    if(s_disk>0.5){
      cairo_save (cr);
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
      cairo_translate (cr, x, y);
      cairo_scale (cr, (s_disk-0.5)*2.*10., 10);
      cairo_arc (cr, 0.0, 0.0, 1., 0, 2*M_PI);
      cairo_fill(cr);
      cairo_restore (cr);
    }
    else if(s_disk<0.5){
      cairo_save (cr);
      cairo_set_source_rgba(cr, 0.7, 0.7, 0.0, 1.0);
      cairo_translate (cr, x, y);
      cairo_scale (cr, (0.5-s_disk)*2.*10., 10);
      cairo_arc (cr, 0.0, 0.0, 1., 0, 2*M_PI);
      cairo_fill(cr);
      cairo_restore (cr);
    }

  }

}

void my_cairo_sun(cairo_t *cr, gint w, gint h, gdouble az, gdouble el){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(el<=0) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.2);
  cairo_arc(cr, x, y, 16, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.3);
  cairo_arc(cr, x, y, 13, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.5);
  cairo_arc(cr, x, y, 11, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.8);
  cairo_arc(cr, x, y, 10, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 1.0);
  cairo_arc(cr, x, y, 9, 0, 2*M_PI);
  cairo_fill(cr);

}

static void cc_skymon_mode (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->skymon_mode=n;
  }

  switch(hg->skymon_mode){
  case SKYMON_SET:
    gtk_widget_set_sensitive(hg->skymon_frame_date,TRUE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    
    if(flagSkymon){
      calcpa2_skymon(hg);

      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }
    break;

  case SKYMON_CUR:
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    
    update_azel2((gpointer)hg);
    if(flagSkymon){
      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    if(hg->skymon_timer<0){
      hg->skymon_timer=g_timeout_add(AZEL_INTERVAL, 
				     (GSourceFunc)update_azel2, 
				     (gpointer)hg);
    }

    break;

  case SKYMON_PLAN_OBJ:
  case SKYMON_PLAN_TIME:
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);

    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }

    remake_sod(hg);
    if(flagSkymon){
      calcpa2_plan(hg);

      if((hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ)&&
	 (!hg->plan[hg->plot_i_plan].backup)){
	hg->skymon_year=hg->fr_year;
	hg->skymon_month=hg->fr_month;
	hg->skymon_day=hg->fr_day;
	hg->skymon_hour=hg->plan[hg->plot_i_plan].sod/60./60.;
	hg->skymon_min=(hg->plan[hg->plot_i_plan].sod-hg->skymon_hour*60.*60.)/60.;
	calc_moon_skymon(hg);
      }

      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    break;
  }
}

void refresh_skymon(GtkWidget *w, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  hg->skymon_output=SKYMON_OUTPUT_WINDOW;

  switch(hg->skymon_mode){
  case SKYMON_SET:
    gtk_widget_set_sensitive(hg->skymon_frame_date,TRUE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    
    if(flagSkymon){
      calcpa2_skymon(hg);

      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    break;

  case SKYMON_CUR:
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    
    update_azel2((gpointer)hg);
    if(flagSkymon){
      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }
    break;

  case SKYMON_PLAN_OBJ:
  case SKYMON_PLAN_TIME:
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);

    remake_sod(hg);
    if(flagSkymon){
      calcpa2_plan(hg);

      //if((hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ)&&
      //	 (!hg->plan[hg->plot_i_plan].backup)){
	hg->skymon_year=hg->fr_year;
	hg->skymon_month=hg->fr_month;
	hg->skymon_day=hg->fr_day;
	hg->skymon_hour=hg->plan[hg->plot_i_plan].sod/60./60.;
	hg->skymon_min=(hg->plan[hg->plot_i_plan].sod-hg->skymon_hour*60.*60.)/60.;
	calc_moon_skymon(hg);
	//}

      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    break;
  }
}

void skymon_set_date(typHOE *hg){
  if(hg->skymon_hour>=24){
    hg->skymon_hour-=24;
    add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, +1);
  }
  
  set_skymon_e_date(hg);
  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
  gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
}

void skymon_set_and_draw (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  
  if(flagSkymon){
    if(hg->skymon_mode==SKYMON_CUR){
      skymon_set_time_current(hg);    
    }
    else{
      calcpa2_skymon(hg);
      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    }

    skymon_set_date(hg);
  }
}

static void skymon_morning (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;
  gchar tmp[6];

  hg=(typHOE *)gdata;

  
  if(flagSkymon){
    if(hg->skymon_mode==SKYMON_SET){

      if(hg->skymon_hour>10){
	add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, +1);
	
	set_skymon_e_date(hg);
      }
      hg->skymon_hour=hg->sun.s_rise.hours;
      hg->skymon_min=hg->sun.s_rise.minutes-SUNRISE_OFFSET;
      if(hg->skymon_min<0){
	hg->skymon_min+=60;
	hg->skymon_hour-=1;
      }
      hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
      
      gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
    
      calcpa2_skymon(hg);
      draw_skymon_cairo(hg->skymon_dw,hg);
    }
  }
}


static void skymon_evening (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;
  gchar tmp[6];

  hg=(typHOE *)gdata;

  
  if(flagSkymon){
    if(hg->skymon_mode==SKYMON_SET){
      if(hg->skymon_hour<10){
	add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, -1);
	
	set_skymon_e_date(hg);
      }
      hg->skymon_hour=hg->sun.s_set.hours;
      hg->skymon_min=hg->sun.s_set.minutes+SUNSET_OFFSET;
      if(hg->skymon_min>=60){
	hg->skymon_min-=60;
	hg->skymon_hour+=1;
      }
      hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
      
      gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
      
      calcpa2_skymon(hg);
      draw_skymon_cairo(hg->skymon_dw,hg);
    }
  }
}


static void skymon_fwd (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
    gtk_widget_set_sensitive(hg->skymon_frame_mode,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_set,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    if(hg->skymon_timer<0)
      hg->skymon_timer=g_timeout_add(SKYMON_INTERVAL, 
				     (GSourceFunc)skymon_go, 
				     (gpointer)hg);
  }
  else{
    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }
  
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
  }

}

gint skymon_go(typHOE *hg){
  gchar tmp[4];

  hg->skymon_min+=5;
  if(hg->skymon_min>=60){
    hg->skymon_min-=60;
    hg->skymon_hour+=1;
  }
  if(hg->skymon_hour>=24){
    hg->skymon_hour-=24;
    add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, +1);

    set_skymon_e_date(hg);
  }

  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
  gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);

  if((hg->skymon_hour==7)||(hg->skymon_hour==7+24)){
    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_fwd),FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    return FALSE;
  }
  
  if(flagSkymon){
    calcpa2_skymon(hg);
    draw_skymon_cairo(hg->skymon_dw,hg);
  }

  return TRUE;

}


static void skymon_rev (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){

    calcpa2_skymon(hg);
    
    gtk_widget_set_sensitive(hg->skymon_frame_mode,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_set,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    if(hg->skymon_timer<0)
      hg->skymon_timer=g_timeout_add(SKYMON_INTERVAL, 
				     (GSourceFunc)skymon_back, 
				     (gpointer)hg);
  }
  else{
    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }
  
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
  }

}


gint skymon_back(typHOE *hg){
  gchar tmp[4];

  hg->skymon_min-=5;
  if(hg->skymon_min<0){
    hg->skymon_min+=60;
    hg->skymon_hour-=1;
  }
  if(hg->skymon_hour<0){
    hg->skymon_hour+=24;
    add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, -1);

    set_skymon_e_date(hg);
  }

  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
  gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);

  if((hg->skymon_hour==18)||(hg->skymon_hour==18-24)){
    if(hg->skymon_timer>0){
      g_source_remove(hg->skymon_timer);
      hg->skymon_timer=-1;
    }
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_rev),FALSE);
    return FALSE;
  }
  
  if(flagSkymon){
    calcpa2_skymon(hg);
    draw_skymon_cairo(hg->skymon_dw,hg);
  }

  return TRUE;

}


void skymon_set_time_current(typHOE *hg){
  int year, month, day, hour, min;
  double sec;
  
  get_current_obs_time(hg,&year, &month, &day, &hour, &min, &sec);
  
  hg->skymon_year=year;
  hg->skymon_month=month;
  hg->skymon_day=day;
    
  hg->skymon_hour=hour;
  hg->skymon_min=min;
}


gboolean update_azel2 (gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  calcpa2_main(hg);

  if(flagSkymon){  // Automatic update for current time
    if(hg->skymon_mode==SKYMON_CUR)
      draw_skymon_cairo(hg->skymon_dw,hg);
  }

  //if(flagTree){
  // tree_update_azel((gpointer)hg);
  // }
  return(TRUE);
}

void pdf_skymon (typHOE *hg)
{
  hg->skymon_output=SKYMON_OUTPUT_PDF;

  if(flagSkymon){
    draw_skymon_cairo(hg->skymon_dw,hg);
  }

  hg->skymon_output=SKYMON_OUTPUT_WINDOW;
}


void get_current_obs_time(typHOE *hg, int *year, int *month, int *day, 
			  int *hour, int *min, gdouble *sec)
{
  struct ln_date date;
  struct ln_zonedate zonedate;
  
  /* get sys date (UT) */
  ln_get_date_from_sys (&date);

  /* UT -> obs time */
  ln_date_to_zonedate(&date, &zonedate, (long)(hg->obs_timezone*60));

  *year=zonedate.years;
  *month=zonedate.months;
  *day=zonedate.days;

  *hour=zonedate.hours;
  *min=zonedate.minutes;
  *sec=zonedate.seconds;
}
