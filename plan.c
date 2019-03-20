//    HDS OPE file Editor
//      plan.c : Edit Obs Plan  
//                                           2010.1.27  A.Tajitsu

#include"main.h"    

static gint plan_cc_set_adj_time();
static gint plan_time_spin_input();
static gint plan_time_spin_output();

void hds_do_efs_for_plan();
void close_plan();
void menu_close_plan();
GtkWidget*  make_plan_menu();

static void cc_obj_list();
static void cc_setup_list();
static void cc_get_toggle_adi();
static void cc_get_plan_dith ();

static GtkTreeModel *create_plan_model();
static void plan_add_columns ();
void plan_long_cell_data_func();
void plan_cell_data_func();

void plan_make_tree();
void plan_close_tree();
void plan_remake_tree();

static void remove_item ();
static void dup_item ();
static void up_item ();
static void down_item ();

static void add_1Object_HDS ();
static void add_1Object_IRCS ();
static void add_1Object_HSC ();
static void add_Object ();
static void add_Focus ();
static void add_BIAS ();
static void add_Comp ();
static void add_Flat ();
static void add_plan_setup ();
static void add_Setup ();
static void add_plan_I2 ();
static void add_I2 ();
static void add_SetAzEl ();
static void add_Comment ();

static void menu_init_plan0();
static void menu_init_plan();
//void init_planpara();
void init_plan();
void hds_init_plan();
void ircs_init_plan();
void hsc_init_plan();

void remake_txt();
struct ln_hrz_posn get_ohrz_sod();
glong get_start_sod();

gchar * hds_make_plan_txt();
gchar * ircs_make_plan_txt();
gchar * hsc_make_plan_txt();

void plot2_plan();
void skymon2_plan();
static void focus_plan_item();

static void  view_onRowActivated();

static void go_edit_plan();
static void close_plan_edit_dialog();
static void do_edit_comment();
static void do_edit_flat();
static void do_edit_comp();
static void do_edit_setazel();
static void do_edit_bias();
static void do_edit_i2();
static void do_edit_focus();
static void do_edit_setup();
static void hds_do_edit_obj();
static void ircs_do_edit_obj();
static void hsc_do_edit_obj();


void swap_plan();

gint get_focus_time();
gint hds_obj_time();
gint ircs_obj_time();
gint hsc_obj_time();
gint comp_time();
gint flat_time();

void hsc_set_skip_stop();
void hsc_set_skip_stop_upper();
void hsc_set_skip_color();
void hsc_set_stop_color();
void cc_plan_skip_adj();
void cc_plan_stop_adj();
void set_sensitive_hsc_30();
void cc_get_hsc_dexp();


gboolean flagPlanTree;
gboolean flagPlanEditDialog=FALSE;

#ifdef USE_GTK3
GdkRGBA col_plan_setup [MAX_USESETUP]
= {
  {0.80, 0.80, 1.00, 1}, //pale2
  {1.00, 1.00, 0.80, 1}, //orange2
  {1.00, 0.80, 1.00, 1}, //purple2
  {0.80, 1.00, 0.80, 1}, //green2
  {1.00, 0.80, 0.80, 1}  //pink2
};
#else
GdkColor col_plan_setup [MAX_USESETUP]
= {
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}  //pink2
};
#endif

enum
{
  COLUMN_PLAN_TOD,
  COLUMN_PLAN_STIME,
  COLUMN_PLAN_TIME,
  COLUMN_PLAN_TXT,
  COLUMN_PLAN_FIL,
  COLUMN_PLAN_TXT_AZ,
  COLUMN_PLAN_TXT_EL,
  COLUMN_PLAN_MOON,
  COLUMN_PLAN_WEIGHT,
  COLUMN_PLAN_COL,
  COLUMN_PLAN_COLSET,
  COLUMN_PLAN_COLFG,
  COLUMN_PLAN_COLBG,
  COLUMN_PLAN_COL_AZEL,
  COLUMN_PLAN_COLSET_AZEL,
  NUM_PLAN_COLUMNS
};


static int adj_change=0;
static int val_pre=0;

static gint plan_cc_set_adj_time (GtkAdjustment *adj) 
{
  adj_change=(gint)gtk_adjustment_get_value(adj);
  return 0;
}


static gint plan_time_spin_input(GtkSpinButton *spin, 
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

      hg->plan_time=hours*60+minutes;
      hg->plan_hour=hours;
      hg->plan_min=minutes;
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


static gint plan_time_spin_output(GtkSpinButton *spin, gpointer gdata){
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
  if(time_val==hg->plan_time){
    buf=g_strdup_printf("%02.0lf:%02.0lf",floor(hours),floor(minutes+0.5));
  }
  else if(adj_val!=0){
    hg->plan_time+=adj_change-val_pre;
    val_pre=adj_change;
    if(hg->plan_time>60*24) hg->plan_time-=60*24;
    if(hg->plan_time<0)     hg->plan_time+=60*24;
    hg->plan_hour=hg->plan_time/60;
    hg->plan_min=hg->plan_time-hg->plan_hour*60;

    buf=g_strdup_printf("%02d:%02d",hg->plan_hour,hg->plan_min);
  }
  if(buf){
    if(strcmp(buf, gtk_entry_get_text(GTK_ENTRY(spin))))
      gtk_entry_set_text(GTK_ENTRY(spin),buf);
    g_free(buf);
  }

  return TRUE;
}


void hds_do_efs_for_plan (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  hg->efs_setup=hg->plan_tmp_setup;

  go_efs(hg);
}


void plan_check_consistency(typHOE *hg){
  gint i, i_plan;
  gint old_setup=-1, new_setup;
  gint old_col=-1, new_col;
  gint i_set;
  gint old_bin=-1, new_bin;
  gint colinc=0;
  gdouble old_colv=+2.106, new_colv;
  gint old_is=IS_NO, new_is;
  gboolean old_fil=TRUE, new_fil;
  // Collimator zero point : 1 filter w/oIS 2.106v
  guint now_i2=PLAN_I2_OUT;

  for(i=0;i<hg->i_plan_max;i++){
    new_setup=hg->plan[i].setup;

    switch(hg->plan[i].type){
    case PLAN_TYPE_SETUP:
      if((new_setup!=old_setup)){
	//printf("Plan-%d : Setup Change %d --> %d\n",
	//   i, old_setup, new_setup);

	// New Color
	if(hg->setup[new_setup].setup<0){ // Non-Standard
	  i_set=-hg->setup[new_setup].setup-1;
	  new_col=hg->nonstd[i_set].col;
	}
	else{
	  if(hg->setup[new_setup].setup<StdI2b){ // Standard Blue
	    new_col=COL_BLUE;
	  }
	  else{
	    new_col=COL_RED;
	  }
	}

	if(hg->plan[i].cmode!=PLAN_CMODE_SLIT){
	  if(old_col<0){ // 1st setup
	    hg->plan[i].cmode=PLAN_CMODE_1ST;
	  }
	  else if (new_col==old_col){ // Cross Scan 
	    hg->plan[i].cmode=PLAN_CMODE_EASY;
	  }
	  else{ // Color Change
	    hg->plan[i].cmode=PLAN_CMODE_FULL;
	  }
	}

	new_is=hg->setup[new_setup].is;
	new_bin=hg->setup[new_setup].binning;
	colinc=0;

	// Image Slicer
	switch(old_is){
	case IS_NO:
	  switch(new_is){
	  case IS_030X5:
	  case IS_045X3:
	    colinc+=7500;
	    break;
	    
	  case IS_020X3:
	    colinc+=11000;
	    break;
	    
	  default:
	    break;
	  }
	  break;
	  
	case IS_030X5:
	case IS_045X3:
	  switch(new_is){
	  case IS_NO:
	    colinc-=7500;
	    break;
	    
	  case IS_020X3:
	    colinc+=(11000-7500);
	    break;
	    
	  default:
	    break;
	  }
	  break;
	  
	case IS_020X3:
	  switch(new_is){
	  case IS_NO:
	    colinc-=11000;
	    break;
	    
	  case IS_030X5:
	  case IS_045X3:
	      colinc-=(11000-7500);
	      break;
	      
	  default:
	      break;
	  }
	    break;
	}

	// Filter
	if((strcmp(hg->setup[new_setup].fil1,"Free")==0)
	   &&(strcmp(hg->setup[new_setup].fil2,"Free")==0)){
	  new_fil=FALSE;
	}
	else{
	  new_fil=TRUE;
	}
	
	if(new_fil){
	  if(!old_fil){  // Insert filter (inc=-1850)
	    colinc+=-1850;
	  }
	}
	else{
	  if(old_fil){   // Remove filter (inc=+1850)
	    colinc+=1850;
	  }
	}

	switch(hg->plan[i].cmode){
	case PLAN_CMODE_FULL:
	case PLAN_CMODE_1ST:
	  hg->plan[i].time=TIME_SETUP_FULL;
	  break;

	case PLAN_CMODE_EASY:
	  hg->plan[i].time=TIME_SETUP_EASY;
	  break;

	case PLAN_CMODE_SLIT:
	  hg->plan[i].time=TIME_SETUP_SLIT;
	  break;
	}

	// Change
	hg->plan[i].colinc=colinc;
	new_colv=old_colv-(2.106-(-1.632))/11000.0*(gdouble)colinc;
	hg->plan[i].colv=new_colv;

	if(old_is!=new_is){
	  hg->plan[i].is_change=TRUE;
	  hg->plan[i].time+=TIME_SETUP_IS;
	}
	else{
	  hg->plan[i].is_change=FALSE;
	}

	if(old_bin!=new_bin){
	  hg->plan[i].bin_change=TRUE;
	  hg->plan[i].time+=TIME_SETUP_BIN;
	}
	else{
	  hg->plan[i].bin_change=FALSE;
	}

	if(colinc!=0){
	  hg->plan[i].time+=TIME_SETUP_COL;
	}


	old_setup=new_setup;
	old_col=new_col;
	old_is=new_is;
	old_bin=new_bin;
	old_fil=new_fil;
	old_colv=new_colv;
      }
      else{
	hg->plan[i].is_change=FALSE;
	hg->plan[i].bin_change=FALSE;
	hg->plan[i].colinc=0;
	hg->plan[i].colv=0;
      }
      break;

    case PLAN_TYPE_OBJ:
    case PLAN_TYPE_BIAS:
    case PLAN_TYPE_FLAT:
    case PLAN_TYPE_COMP:
      hg->plan[i].i2_pos=now_i2;
      
      if((now_i2==PLAN_I2_IN)&&(hg->plan[i].type!=PLAN_TYPE_OBJ)){
	for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
	  swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
	}
	hg->i_plan_max++;

	add_plan_I2(hg,i,PLAN_I2_OUT);
	{
	  GtkTreeIter iter;
	  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
	  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
	  remake_tod(hg, model); 
	  tree_update_plan_item(hg, model, iter, i);
	  
	  refresh_plan_plot(hg);
	}

	i--;
      }
      else{
	if(new_setup!=old_setup){
	  //printf("Plan-%d : You should insert change to Setup %d --> %d\n",
	  //      i, old_setup, new_setup);
	  
	  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
	    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
	  }
	  hg->i_plan_max++;
	  
	  add_plan_setup(hg,i,new_setup);
	  {
	    GtkTreeIter iter;
	    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
	    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
	    remake_tod(hg, model); 
	    tree_update_plan_item(hg, model, iter, i);
	    
	    refresh_plan_plot(hg);
	  }
	  
	  i--;
	}
      }
      break;

    case PLAN_TYPE_FOCUS:
      hg->plan[i].focus_is=old_is;
      hg->plan[i].i2_pos=now_i2;
      break;

    case PLAN_TYPE_I2:
      if(now_i2==hg->plan[i].i2_pos){
	hg->plan[i].backup=TRUE;
	hg->plan[i].time=0;
      }
      else{
	hg->plan[i].backup=FALSE;
	hg->plan[i].time=TIME_I2;
	now_i2=hg->plan[i].i2_pos;
      }    
      break;

    default:
      break;
    }
  }
}


// Create Obs Pplan Edit Window (hg->plan_main)
void create_plan_dialog(typHOE *hg)
{
  GtkWidget *plan_tbl;
  GtkWidget *button;
  GtkWidget *plan_scroll;
  GtkWidget *plan_wbox;
  GtkWidget *hbox;
  GtkWidget *check;
  GtkWidget *combo,*label,*spinner;
  GtkWidget *entry;
  GtkAdjustment *adj;
  gchar plan_buffer[BUFFSIZE];
  gchar *fp_1, *fp_2;
  guint nchars;
  GtkTextIter start_iter, end_iter;
  GtkTextMark *end_mark;
  gchar *title_tmp;
  FILE *infile;
  GtkWidget *planbar;
  gchar tmp[64];
  gint i_use,i_list, i_dith;
  GtkTreeModel *plan_model;
  GdkPixbuf *icon;
  gint timer;
  gchar *tmp_txt;

  flagPlan=TRUE;
  gtk_widget_set_sensitive(hg->f_objtree_arud,FALSE);
  switch(hg->inst){
  case INST_HDS:
    gtk_widget_set_sensitive(hg->setup_scrwin,FALSE);
    break;

  case INST_IRCS:
    gtk_widget_set_sensitive(hg->ircs_vbox,FALSE);
    break;

  case INST_HSC:
    gtk_widget_set_sensitive(hg->hsc_vbox,FALSE);
    break;
  }

  if(hg->i_plan_max<1){
    init_plan(hg);
  }

  hg->plan_jd1=-1;
  hg->plan_jd2=-1;
  hg->plan_trace=-1;
  
  hg->plan_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  plan_wbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->plan_main), plan_wbox);

  planbar=make_plan_menu(hg);
  gtk_box_pack_start(GTK_BOX(plan_wbox), planbar,FALSE, FALSE, 0);

  title_tmp=g_strdup("HOE : Observation Plan");
  gtk_window_set_title(GTK_WINDOW(hg->plan_main), title_tmp);
  gtk_widget_realize(hg->plan_main);
  my_signal_connect(hg->plan_main,"destroy",
		    close_plan, 
		    (gpointer)hg);
  gtk_container_set_border_width (GTK_CONTAINER (hg->plan_main), 0);
  

  // Command Add
  hg->plan_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->plan_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->plan_note), TRUE);
  gtk_box_pack_start(GTK_BOX(plan_wbox), hg->plan_note,FALSE, FALSE, 2);

  // Object
  {
    {
      hg->plan_ircs_coadds=1;
      hg->plan_ircs_ndr=IRCS_DEF_NDR;
      hg->plan_obj_repeat=hg->obj[0].repeat;
    }
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Object","insert-object");
#else
    button=gtkut_button_new_from_stock("Object",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
    		      add_Object, 
    		      (gpointer)hg);

    if(hg->i_max<1){
      gtk_widget_set_sensitive(button,FALSE);
    }

    switch(hg->inst){
    case INST_HDS:
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	if(hg->obj[0].name){
	  hg->plan_obj_i=-1;
	  hg->plan_obj_exp=hg->obj[0].exp;
	  hg->plan_obj_repeat=hg->obj[0].repeat;
	  hg->plan_obj_guide=hg->obj[0].guide;
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(All Objects)",
			   1, -1, -1);
	for(i_list=0;i_list<hg->i_max;i_list++){
	  gtk_list_store_append(store, &iter);
	  sprintf(tmp,"%03d:  %s",i_list+1,hg->obj[i_list].name);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_list, -1);
	}
	
	hg->plan_obj_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),hg->plan_obj_combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_obj_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_obj_combo),
					renderer, "text",0,NULL);
	
	
	hg->e_list=0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_combo),0);
	gtk_widget_show(hg->plan_obj_combo);
	my_signal_connect (hg->plan_obj_combo,
			   "changed",
			   cc_obj_list,
			   (gpointer)hg);
      }
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SetupField -- GetObject",
			   1, PLAN_OMODE_FULL, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_FULL) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SetupField Only",
			   1, PLAN_OMODE_SET, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_SET) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GetObject Only",
			   1, PLAN_OMODE_GET, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_GET) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_obj_omode);
      }
      
      hg->plan_exp_adj = (GtkAdjustment *)gtk_adjustment_new(hg->obj[0].exp,
							     1, 9999, 1.0, 10.0, 0);
      my_signal_connect (hg->plan_exp_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_obj_exp);
      spinner =  gtk_spin_button_new (hg->plan_exp_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 4);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      label = gtk_label_new ("[s]x");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
      
      hg->plan_obj_repeat=hg->obj[0].repeat;
      hg->plan_obj_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_obj_repeat,
							     1, 50, 1.0, 1.0, 0);
      my_signal_connect (hg->plan_obj_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_obj_repeat);
      spinner =  gtk_spin_button_new (hg->plan_obj_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    

      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "No Guide",
			   1, NO_GUIDE, -1);
	if(hg->plan_obj_guide==NO_GUIDE) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "AG Guide",
			   1, AG_GUIDE, -1);
	if(hg->plan_obj_guide==AG_GUIDE) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SV Guide",
			   1, SV_GUIDE, -1);
	if(hg->plan_obj_guide==SV_GUIDE) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SV Guide (Safe)",
			   1, SVSAFE_GUIDE, -1);
	if(hg->plan_obj_guide==SVSAFE_GUIDE) iter_set=iter;
	
	hg->plan_obj_guide_combo
	  = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),hg->plan_obj_guide_combo,
			   FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_obj_guide_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_obj_guide_combo),
					renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->plan_obj_guide_combo),
				      &iter_set);
	gtk_widget_show(hg->plan_obj_guide_combo);
	my_signal_connect (hg->plan_obj_guide_combo,"changed",cc_get_combo_box,
			   &hg->plan_obj_guide);
      }
      break;

    case INST_IRCS:
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	if(hg->obj[0].name){
	  hg->plan_obj_i=-1;
	  hg->plan_obj_exp=hg->obj[0].exp;
	  hg->plan_obj_repeat=hg->obj[0].repeat;
	  hg->plan_obj_guide=hg->obj[0].guide;
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(All Objects)",
			   1, -1, -1);
	for(i_list=0;i_list<hg->i_max;i_list++){
	  gtk_list_store_append(store, &iter);
	  switch(hg->obj[i_list].aomode){
	  case AOMODE_NO:
	    tmp_txt=g_strdup_printf("%03d:  %s (w/o AO)",i_list+1,hg->obj[i_list].name);
	    break;
	  case AOMODE_NGS_S:
	    tmp_txt=g_strdup_printf("%03d:  %s (NGS:self)",i_list+1,hg->obj[i_list].name);
	    break;
	  case AOMODE_NGS_O:
	    tmp_txt=g_strdup_printf("%03d:  %s (NGS:offset)",i_list+1,hg->obj[i_list].name);
	    break;
	  case AOMODE_LGS_S:
	    tmp_txt=g_strdup_printf("%03d:  %s (LGS:self)",i_list+1,hg->obj[i_list].name);
	    break;
	  case AOMODE_LGS_O:
	    tmp_txt=g_strdup_printf("%03d:  %s (LGS:TTGS)",i_list+1,hg->obj[i_list].name);
	    break;
	  }
	  gtk_list_store_set(store, &iter, 0, tmp_txt,
			     1, i_list, -1);
	  g_free(tmp_txt);
	}
	
	hg->plan_obj_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),hg->plan_obj_combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_obj_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_obj_combo),
					renderer, "text",0,NULL);
	
	
	hg->e_list=0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_combo),0);
	gtk_widget_show(hg->plan_obj_combo);
	my_signal_connect (hg->plan_obj_combo,
			   "changed",
			   cc_obj_list,
			   (gpointer)hg);
      }
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SetupField -- GetObject",
			   1, PLAN_OMODE_FULL, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_FULL) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SetupField Only",
			   1, PLAN_OMODE_SET, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_SET) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GetObject Only",
			   1, PLAN_OMODE_GET, -1);
	if(hg->plan_obj_omode==PLAN_OMODE_GET) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_obj_omode);
      }


      hg->plan_dexp_adj = (GtkAdjustment *)gtk_adjustment_new((hg->ircs_i_max>0) ? hg->ircs_set[0].exp : 100,
							     0.006, 6000, 0.01, 1.0, 0);
      my_signal_connect (hg->plan_dexp_adj, "value_changed",
			 cc_get_adj_double,
			 &hg->plan_obj_dexp);
      spinner =  gtk_spin_button_new (hg->plan_dexp_adj, 3, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      label = gtk_label_new ("[s]  COADDS");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_ircs_coadds,
						1, 200, 1, 1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_ircs_coadds);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      label = gtk_label_new ("  NDR");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_ircs_ndr,
						1, 100, 1, 1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_ircs_ndr);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      label = gtk_label_new ("  x");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_obj_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_obj_repeat,
							     1, 50, 1.0, 1.0, 0);
      my_signal_connect (hg->plan_obj_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_obj_repeat);
      spinner =  gtk_spin_button_new (hg->plan_obj_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      hg->plan_adi=FALSE;
      hg->plan_adi_check = gtk_check_button_new_with_label("ADI");
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_adi_check,FALSE, FALSE, 0);
      my_signal_connect (hg->plan_adi_check, "toggled",
			 cc_get_toggle_adi,
			 (gpointer)hg);
      break;

    case INST_HSC:
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	hg->plan_obj_i=-1;
	hg->plan_obj_dexp=hg->hsc_set[0].exp;
	hg->plan_obj_repeat=1;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(All Objects)",
			   1, -1, -1);
	for(i_list=0;i_list<hg->i_max;i_list++){
	  gtk_list_store_append(store, &iter);
	  sprintf(tmp,"%03d:  %s",i_list+1,hg->obj[i_list].name);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_list, -1);
	}
	
	hg->plan_obj_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),hg->plan_obj_combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_obj_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_obj_combo),
					renderer, "text",0,NULL);
	
	
	hg->e_list=0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_combo),0);
	gtk_widget_show(hg->plan_obj_combo);
	my_signal_connect (hg->plan_obj_combo,
			   "changed",
			   cc_obj_list,
			   (gpointer)hg);
      }
      
      hg->plan_dexp_adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_set[0].exp,
							      2, 3600, 1.0, 10.0, 0);
      spinner =  gtk_spin_button_new (hg->plan_dexp_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 4);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

      label = gtk_label_new ("[s]x");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
      
      hg->plan_obj_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_obj_repeat,
							     1, 50, 1.0, 1.0, 0);
      my_signal_connect (hg->plan_obj_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_obj_repeat);
      spinner =  gtk_spin_button_new (hg->plan_obj_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    
      break;
    }

    if(hg->inst==INST_HSC){
      hg->plan_hbox_dz = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hg->plan_hbox_dz), 2);
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_hbox_dz,FALSE,FALSE,0);
           
      label = gtk_label_new ("  dZ (for standards)");
      gtk_box_pack_start(GTK_BOX(hg->plan_hbox_dz),label,FALSE,FALSE,0);

      hg->plan_delta_z=hg->hsc_delta_z;
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_delta_z,
						0.00, 1.00, 0.05, 0.05, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->plan_delta_z);
      spinner =  gtk_spin_button_new (adj, 2, 2);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hg->plan_hbox_dz),spinner,FALSE,FALSE,0);
      
      gtk_widget_set_sensitive(hg->plan_hbox_dz,FALSE);

      // 30sec calib
      hg->plan_hsc_30 = FALSE;
      hg->check_hsc_30 = gtk_check_button_new_with_label("30s calib");
      gtk_box_pack_start(GTK_BOX(hbox),hg->check_hsc_30,FALSE,FALSE,0);
      my_signal_connect (hg->check_hsc_30, "toggled",
			 cc_get_toggle, &hg->plan_hsc_30);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->check_hsc_30),
				   hg->plan_hsc_30);
      my_signal_connect (hg->plan_dexp_adj, "value_changed",
			 cc_get_hsc_dexp,
			 (gpointer)hg);
      set_sensitive_hsc_30(hg);
    }

    
    
    hg->plan_backup=FALSE;
    check = gtk_check_button_new_with_label("Back-Up");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->plan_backup);
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled", cc_get_toggle, &hg->plan_backup);
    
    label = gtk_label_new ("Object");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  // Focusing
  {
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Focusing","insert-object");
#else
    button=gtkut_button_new_from_stock("Focusing",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Focus, 
		      (gpointer)hg);

    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

      switch(hg->inst){
      case INST_HDS:
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "FocusSV",
			   1, PLAN_FOCUS1, -1);
	if(hg->plan_focus_mode==PLAN_FOCUS1) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "FocusAG",
			   1, PLAN_FOCUS2, -1);
	if(hg->plan_focus_mode==PLAN_FOCUS2) iter_set=iter;
	break;

      case INST_IRCS:
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "FocusOBE",
			   1, PLAN_FOCUS1, -1);
	if(hg->plan_focus_mode==PLAN_FOCUS1) iter_set=iter;

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "LGS Calibration",
			   1, PLAN_FOCUS2, -1);
	if(hg->plan_focus_mode==PLAN_FOCUS2) iter_set=iter;
	break;

      case INST_HSC:
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(Current Position)",
			   1, 0, -1);
	iter_set=iter;
	
	for(i_list=0;i_list<hg->i_max;i_list++){
	  gtk_list_store_append(store, &iter);
	  sprintf(tmp,"@ %03d:  %s",i_list+1,hg->obj[i_list].name);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_list+1, -1);
	}
	break;
      }
      
      hg->plan_focus_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_focus_combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_focus_combo),
				 renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_focus_combo),
				      renderer, "text",0,NULL);
	
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->plan_focus_combo),
				    &iter_set);
      gtk_widget_show(hg->plan_focus_combo);
      my_signal_connect (hg->plan_focus_combo,"changed",cc_get_combo_box,
			 &hg->plan_focus_mode);
    }

    if(hg->inst==INST_HSC){
      label = gtk_label_new ("  Center Z");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_focus_z=HSC_DEF_FOCUS_Z;
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_focus_z,
						3.00, 4.00, 0.05, 0.05, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->plan_focus_z);
      spinner =  gtk_spin_button_new (adj, 2, 2);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    }      


    label = gtk_label_new ("Focusing");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  // SetAzEl
  {
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("SetAzEl","insert-object");
#else
    button=gtkut_button_new_from_stock("SetAzEl",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_SetAzEl, 
		      (gpointer)hg);
    
    label = gtk_label_new ("Az ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(-90,
					      -269, 269, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan_setaz);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

    label = gtk_label_new ("    El ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(70,
					      15, 90, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan_setel);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);


    hg->plan_setazel_daytime=FALSE;
    check = gtk_check_button_new_with_label("Daytime");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &hg->plan_setazel_daytime);
   
    
    label = gtk_label_new ("SetAzEl");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }


  switch(hg->inst){
  case INST_HDS:
    // Setup
    {
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
      
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Setup","insert-object");
#else
      button=gtkut_button_new_from_stock("Setup",GTK_STOCK_ADD);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
			add_Setup, 
			(gpointer)hg);
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Full Change",
			   1, PLAN_CMODE_FULL, -1);
	if(hg->plan_setup_cmode==PLAN_CMODE_FULL) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Cross Scan",
			   1, PLAN_CMODE_EASY, -1);
	if(hg->plan_setup_cmode==PLAN_CMODE_EASY) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Slit Only",
			   1, PLAN_CMODE_SLIT, -1);
	if(hg->plan_setup_cmode==PLAN_CMODE_SLIT) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "1st Change",
			   1, PLAN_CMODE_1ST, -1);
	if(hg->plan_setup_cmode==PLAN_CMODE_1ST) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_setup_cmode);
      }

      
      hg->plan_setup_daytime=FALSE;
      check = gtk_check_button_new_with_label("Daytime");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
      my_signal_connect (check, "toggled",
			 cc_get_toggle,
			 &hg->plan_setup_daytime);
      
      label = gtk_label_new ("Setup");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
    }
    break;
      
  case INST_HSC:
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
    
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Filter Exchange","insert-object");
#else
    button=gtkut_button_new_from_stock("Filter Exchange",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Setup, 
		      (gpointer)hg);
    
    hg->plan_setup_daytime=FALSE;
    check = gtk_check_button_new_with_label("Daytime");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &hg->plan_setup_daytime);
    
    label = gtk_label_new ("Filter");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
    break;
  }
    

  // I2Cell
  switch(hg->inst){
  case INST_HDS:
    {
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
      
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("I2Cell","insert-object");
#else
      button=gtkut_button_new_from_stock("I2Cell",GTK_STOCK_ADD);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
			add_I2, 
			(gpointer)hg);
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "In",
			   1, PLAN_I2_IN, -1);
	if(hg->plan_i2_pos==PLAN_I2_IN) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Out",
			   1, PLAN_I2_OUT, -1);
	if(hg->plan_i2_pos==PLAN_I2_OUT) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_i2_pos);
      }
      
      hg->plan_i2_daytime=FALSE;
      check = gtk_check_button_new_with_label("Daytime");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
      my_signal_connect (check, "toggled",
			 cc_get_toggle,
			 &hg->plan_i2_daytime);
      
      
      label = gtk_label_new ("I2Cell");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
    }
    break;
  }


  // BIAS
  switch(hg->inst){
  case INST_HDS:
    {
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
      
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("BIAS","insert-object");
#else
      button=gtkut_button_new_from_stock("BIAS",GTK_STOCK_ADD);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
			add_BIAS, 
			(gpointer)hg);
      
      label = gtk_label_new ("  x");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_END);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_bias_repeat,
						1, 30, 1.0, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_bias_repeat);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      
      
      hg->plan_bias_daytime=FALSE;
      check = gtk_check_button_new_with_label("Daytime");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
      my_signal_connect (check, "toggled",
			 cc_get_toggle,
			 &hg->plan_bias_daytime);
      
    
      label = gtk_label_new ("BIAS");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
    }
    break;
  }
  

  // Comparison
  switch(hg->inst){
  case INST_HDS:
  case INST_IRCS:
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Comparison","insert-object");
#else
    button=gtkut_button_new_from_stock("Comparison",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Comp, 
		      (gpointer)hg);
    
    if(hg->inst == INST_IRCS){
      hg->plan_comp_mode=-1;
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	gint i_mode;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	for(i_mode=-1;i_mode<NUM_IRCS_MODE;i_mode++){
	  if(i_mode==-1){
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "for Cuerrent Setup",
			       1, i_mode, -1);
	  }
	  else{
	    switch(i_mode){
	    case IRCS_MODE_GR:
	    case IRCS_MODE_PS:
	    case IRCS_MODE_EC:
	      gtk_list_store_append(store, &iter);
	      tmp_txt=g_strdup_printf("for all %s Setups", ircs_mode_name[i_mode]);
	      gtk_list_store_set(store, &iter, 0, tmp_txt,
				 1, i_mode, -1);
	      g_free(tmp_txt);
	      break;
	    }
	  }
	}

	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo),0);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_comp_mode);
      }

      label = gtk_label_new ("  x");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_END);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
      hg->plan_comp_repeat=IRCS_COMP_REPEAT;
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_comp_repeat,
						1, 50, 1.0, 1.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_comp_repeat);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
				FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      break;
    }
    
    hg->plan_comp_daytime=FALSE;
    check = gtk_check_button_new_with_label("Daytime");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &hg->plan_comp_daytime);
   

    label = gtk_label_new ("Comparison");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  // Flat
  {
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    if(hg->inst==INST_HSC){
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Dome Flat","insert-object");
#else
      button=gtkut_button_new_from_stock("Dome Flat",GTK_STOCK_ADD);
#endif
    }
    else{
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Flat","insert-object");
#else
      button=gtkut_button_new_from_stock("Flat",GTK_STOCK_ADD);
#endif
    }
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Flat, 
		      (gpointer)hg);

    switch(hg->inst){
    case INST_HDS:
      hg->plan_flat_repeat=HDS_FLAT_REPEAT;
      break;
      
    case INST_IRCS:
      hg->plan_flat_repeat=IRCS_FLAT_REPEAT;
      hg->plan_flat_mode=-1;
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	gint i_mode;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	for(i_mode=-1;i_mode<NUM_IRCS_MODE;i_mode++){
	  gtk_list_store_append(store, &iter);
	  if(i_mode==-1){
	    gtk_list_store_set(store, &iter, 0, "for Cuerrent Setup",
			       1, i_mode, -1);
	  }
	  else{
	    tmp_txt=g_strdup_printf("for all %s Setups", ircs_mode_name[i_mode]);
	    gtk_list_store_set(store, &iter, 0, tmp_txt,
			       1, i_mode, -1);
	    g_free(tmp_txt);
	  }
	}

	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo),0);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->plan_flat_mode);
      }
      break;

    case INST_HSC:
      hg->plan_flat_repeat=HSC_FLAT_REPEAT;
      break;
    }

    label = gtk_label_new ("  x");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_flat_repeat,
					      1, 50, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->plan_flat_repeat);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);


    hg->plan_flat_daytime=FALSE;
    check = gtk_check_button_new_with_label("Daytime");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &hg->plan_flat_daytime);

    label = gtk_label_new ("Flat");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }


  // Comment
  {
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
#ifdef USE_GTK3      
    button=gtkut_button_new_from_icon_name("Comment","insert-object");
#else
    button=gtkut_button_new_from_stock("Comment",GTK_STOCK_ADD);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Comment, 
		      (gpointer)hg);

    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Text",
			 1, PLAN_COMMENT_TEXT, -1);
      if(hg->plan_comment_type==PLAN_COMMENT_TEXT) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Sunset",
			 1, PLAN_COMMENT_SUNSET, -1);
      if(hg->plan_comment_type==PLAN_COMMENT_SUNSET) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Sunrise",
			 1, PLAN_COMMENT_SUNRISE, -1);
      if(hg->plan_comment_type==PLAN_COMMENT_SUNRISE) iter_set=iter;
      

      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
	
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->plan_comment_type);
    }
    
    entry = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE, FALSE, 0);
    if(hg->plan_comment){
      gtk_entry_set_text(GTK_ENTRY(entry),
			 hg->plan_comment);
    }
    gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
    my_entry_set_width_chars(GTK_ENTRY(entry),50);
    my_signal_connect (entry,
		       "changed",
		       cc_get_entry,
		       &hg->plan_comment);

    label = gtk_label_new ("  Time[min]");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    hg->plan_comment_time=0;
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_comment_time,
					      0, 60, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->plan_comment_time);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    
    
    label = gtk_label_new ("Comment");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  

  //// Setup
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(plan_wbox), hbox,FALSE, FALSE, 2);

  switch(hg->inst){
  case INST_HDS:
    label = gtk_label_new ("   HDS Setup : ");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );
      hg->plan_tmp_setup=0;
      
      for(i_use=0;i_use<MAX_USESETUP;i_use++){
	if(hg->setup[i_use].use){
	  if(hg->setup[i_use].setup<0){
	    tmp_txt=g_strdup_printf("%d : NonStd-%d %dx%dbin",
				    i_use+1,-hg->setup[i_use].setup,
				    hg->binning[hg->setup[i_use].binning].x,
				    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{
	    tmp_txt=g_strdup_printf("Setup-%d : Std%s %dx%dbin",
				    i_use+1,HDS_setups[hg->setup[i_use].setup].initial,
				    hg->binning[hg->setup[i_use].binning].x,
				    hg->binning[hg->setup[i_use].binning].y);
	  }
	  
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 
			     0, tmp_txt,
			     1, i_use,
			     2, &color_black,
			     3, &col_plan_setup[i_use],
			     -1);
	  g_free(tmp_txt);
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan_tmp_setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->plan_tmp_setup);
    }
  
    icon = gdk_pixbuf_new_from_resource ("/icons/efs_icon.png", NULL);
    button=gtkut_button_new_from_pixbuf(NULL, icon);
    g_object_unref(icon);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (hds_do_efs_for_plan), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Display Echelle Format");
#endif
    
    check = gtk_check_button_new_with_label("Override Default Slit");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &hg->plan_tmp_or);
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan_tmp_sw/500.,
					      0.2, 4.0, 
					      0.05,0.10,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &hg->plan_tmp_sw);
    spinner =  gtk_spin_button_new (adj, 0, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    
    label = gtk_label_new ("/");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan_tmp_sl/500.,
					      2.0, 60.0, 
					      0.1,1.0,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->plan_tmp_sl);
    spinner =  gtk_spin_button_new (adj, 0, 1);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    break;

  case INST_IRCS:
    label = gtk_label_new ("   IRCS Setup : ");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );
      hg->plan_tmp_setup=0;
      
      for(i_use=0;i_use<hg->ircs_i_max;i_use++){
	tmp_txt=g_strdup_printf("%02d : %s",
				i_use+1,
				hg->ircs_set[i_use].txt);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp_txt,
			   1, i_use, 
			   2, &color_black,
			   3, &col_ircs_setup[i_use],
			   -1);
	g_free(tmp_txt);
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan_tmp_setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed", cc_setup_list,
			 (gpointer)hg);

      label = gtk_label_new (" Dither");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_dith=hg->ircs_set[0].dith;
      hg->plan_dithw=hg->ircs_set[0].dithw;
      hg->plan_osra=hg->ircs_set[0].osra;
      hg->plan_osdec=hg->ircs_set[0].osdec;
      hg->plan_sssep=hg->ircs_set[0].sssep;
      hg->plan_ssnum=hg->ircs_set[0].ssnum;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      hg->plan_tmp_setup=0;
      
      for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, IRCS_dith[i_dith].name,
			   1, i_dith, 
			   -1);
      }
      
      hg->plan_dith_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_dith_combo,FALSE, FALSE, 0);
      g_object_unref(store);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_dith_combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_dith_combo), renderer, 
				      "text",0,NULL);

      gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_dith_combo), hg->plan_dith);
      gtk_widget_show(combo);
      my_signal_connect (hg->plan_dith_combo,"changed", cc_get_plan_dith,
      			 (gpointer)hg);

      label = gtk_label_new (" width");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_dithw_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_dithw,
							       1.0, 20.0, 0.1, 1.0, 0);
      my_signal_connect (hg->plan_dithw_adj, "value_changed",
			 cc_get_adj_double,
			 &hg->plan_dithw);
      spinner =  gtk_spin_button_new (hg->plan_dithw_adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
     
      label = gtk_label_new (" dRA");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_osra_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osra,
							      -3000, 3000, 
							      1.0, 10.0, 0);
      my_signal_connect (hg->plan_osra_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_osra);
      spinner =  gtk_spin_button_new (hg->plan_osra_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      label = gtk_label_new (" dDec");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_osdec_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osdec,
							       -3000, 3000, 
							       1.0, 10.0, 0);
      my_signal_connect (hg->plan_osdec_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_osdec);
      spinner =  gtk_spin_button_new (hg->plan_osdec_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      label = gtk_label_new (" Slit Scan");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
      hg->plan_sssep_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_sssep,
							       0.05, 5.00, 
							       0.01, 0.1, 0);
      my_signal_connect (hg->plan_sssep_adj, "value_changed",
			 cc_get_adj_double,
			 &hg->plan_sssep);
      spinner =  gtk_spin_button_new (hg->plan_sssep_adj, 1, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      label = gtk_label_new ("x");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_ssnum_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_ssnum,
							       2, 20, 
							       1, 1, 0);
      my_signal_connect (hg->plan_ssnum_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_ssnum);
      spinner =  gtk_spin_button_new (hg->plan_ssnum_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    }

    break;

  case INST_HSC:
    label = gtk_label_new ("   HSC Setup : ");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );
      hg->plan_tmp_setup=0;
      hg->plan_osra=hg->hsc_set[0].osra;
      hg->plan_osdec=hg->hsc_set[0].osdec;
      hsc_set_skip_stop(hg, 0);
      
      for(i_use=0;i_use<hg->hsc_i_max;i_use++){
	tmp_txt=g_strdup_printf("%02d : %s",
				i_use+1,
				hg->hsc_set[i_use].txt);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp_txt,
			   1, i_use, 
			   2, &color_black,
			   3, &col_hsc_setup[i_use],
			   -1);
	g_free(tmp_txt);
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan_tmp_setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed", cc_setup_list,
			 (gpointer)hg);

      label = gtk_label_new (" Offset RA");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_osra_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osra,
							      -3000, 3000, 
							      1.0, 10.0, 0);
      my_signal_connect (hg->plan_osra_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_osra);
      spinner =  gtk_spin_button_new (hg->plan_osra_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      label = gtk_label_new (" Dec");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->plan_osdec_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osdec,
							       -3000, 3000, 
							       1.0, 10.0, 0);
      my_signal_connect (hg->plan_osdec_adj, "value_changed",
			 cc_get_adj,
			 &hg->plan_osdec);
      spinner =  gtk_spin_button_new (hg->plan_osdec_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      hg->plan_skip_label = gtk_label_new ("   Skip");
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_skip_label,FALSE,FALSE,0);

      hg->plan_skip_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_skip,
							       0, hg->plan_skip_upper, 
							       1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (hg->plan_skip_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      hg->plan_stop_label = gtk_label_new (" Stop");
      gtk_box_pack_start(GTK_BOX(hbox),hg->plan_stop_label,FALSE,FALSE,0);
      
      hg->plan_stop_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_stop,
							       1, hg->plan_stop_upper, 
							       1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (hg->plan_stop_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

      my_signal_connect (hg->plan_skip_adj, "value_changed",
			 cc_plan_skip_adj,
			 (gpointer)hg);
      my_signal_connect (hg->plan_stop_adj, "value_changed",
			 cc_plan_stop_adj,
			 (gpointer)hg);
      hsc_set_skip_color(hg);
      hsc_set_stop_color(hg);
    }

    break;
  }
    

  // Plan List
  plan_scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (plan_scroll),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (plan_scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (plan_wbox), plan_scroll, TRUE, TRUE, 0);
  
  gtk_widget_set_size_request(plan_scroll, -1, 400);  
  /* create models */
  plan_model = create_plan_model (hg);

  /* create tree view */
  hg->plan_tree = gtk_tree_view_new_with_model (plan_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->plan_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->plan_tree)),
			       GTK_SELECTION_SINGLE);

  calc_sun_plan(hg);
  remake_tod(hg, plan_model); 

  plan_add_columns (hg, GTK_TREE_VIEW (hg->plan_tree), plan_model);

  g_object_unref (plan_model);
  
  gtk_container_add (GTK_CONTAINER (plan_scroll), hg->plan_tree);
   
  hbox = gtkut_hbox_new(FALSE,4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start (GTK_BOX (plan_wbox), hbox, FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Plot","document-print-preview");
#else
  button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (plot2_plan), (gpointer)hg);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Plot Elevation etc.");
#endif

  icon = gdk_pixbuf_new_from_resource ("/icons/sky_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf("SkyMon", icon);
  g_object_unref(icon);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (skymon2_plan), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Sky Monitor");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("FC","starred");
#else
  button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fc_item_plan), (gpointer)hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-up");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
#endif
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    up_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Up");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    down_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Down");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"list-remove");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#endif
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    remove_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Remove");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    refresh_tree, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Check consistency + Refresh");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-copy");
#else
  button=gtkut_button_new_from_stock("Duplicate",GTK_STOCK_COPY);
#endif
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    dup_item, 
		    (gpointer)hg);

  label = gtk_label_new ("   Start @");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Evening",
		       1, PLAN_START_EVENING, -1);
    if(hg->plan_start==PLAN_START_EVENING) iter_set=iter;
      
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Specific Time",
			 1, PLAN_START_SPECIFIC, -1);
    if(hg->plan_start==PLAN_START_SPECIFIC) iter_set=iter;
      

    hg->plan_start_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),hg->plan_start_combo,FALSE, FALSE, 0);
    g_object_unref(store);
      
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_start_combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_start_combo), renderer, "text",0,NULL);

	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->plan_start_combo),&iter_set);
    gtk_widget_show(hg->plan_start_combo);
    my_signal_connect (hg->plan_start_combo,"changed",cc_get_combo_box,
		       &hg->plan_start);
  }

  if(hg->plan_hour<10) hg->plan_hour+=24;
  hg->plan_time=hg->plan_hour*60+hg->plan_min;

  hg->plan_adj_min = (GtkAdjustment *)gtk_adjustment_new(hg->plan_time,
							 0, 60*24,
							 10.0, 60.0, 0);
  spinner =  gtk_spin_button_new (hg->plan_adj_min, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(GTK_SPIN_BUTTON(spinner)),5);
  my_signal_connect (hg->plan_adj_min, "value-changed",
  		     plan_cc_set_adj_time,
  		     NULL);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "output",
  		     plan_time_spin_output,
		     (gpointer)hg);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "input",
  		     plan_time_spin_input,
  		     (gpointer)hg);

  /*
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_start_hour,
					    18, 30, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_start_hour);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);

  label = gtk_label_new (":");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_start_min,
					    0, 59, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_start_min);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  */

  
  label = gtk_label_new ("  Delay from Sunset");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_delay,
					    0, 99, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_delay);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);


  g_signal_connect (hg->plan_tree, "cursor-changed",
		    G_CALLBACK (focus_plan_item), (gpointer)hg);

  g_signal_connect(hg->plan_tree, "row-activated", 
		   G_CALLBACK (view_onRowActivated), (gpointer)hg);

  gtk_widget_show_all(hg->plan_main);

  refresh_tree(NULL, (gpointer)hg);


  gtk_main();

  g_free(title_tmp);

  flagChildDialog=FALSE;
  gdk_flush();
}


void close_plan(GtkWidget *w, gpointer gdata)
{
  typHOE *hg = (typHOE *) gdata;

  switch(hg->inst){
  case INST_HDS:
    plan_check_consistency(hg);
    break;
  }

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(hg->plan_main));

  gtk_widget_set_sensitive(hg->f_objtree_arud,TRUE);
  switch(hg->inst){
  case INST_HDS:
    gtk_widget_set_sensitive(hg->setup_scrwin,TRUE);
    break;
    
  case INST_IRCS:
    gtk_widget_set_sensitive(hg->ircs_vbox,TRUE);
    break;

  case INST_HSC:
    gtk_widget_set_sensitive(hg->hsc_vbox,TRUE);
    break;
  }
    
  flagPlan=FALSE;
}

void menu_close_plan(GtkWidget *widget,gpointer gdata)
{
  typHOE *hg = (typHOE *) gdata;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(hg->plan_main));

  gtk_widget_set_sensitive(hg->f_objtree_arud,TRUE);
  switch(hg->inst){
  case INST_HDS:
    gtk_widget_set_sensitive(hg->setup_scrwin,TRUE);
    break;

  case INST_IRCS:
    gtk_widget_set_sensitive(hg->ircs_vbox,TRUE);
    break;
  }
    
  flagPlan=FALSE;
}



static void cc_obj_list (GtkWidget *widget, gpointer gdata)
{
  gint i_use;
  typHOE *hg;
  gchar tmp[64];

  hg = (typHOE *) gdata;

  {
    GtkTreeIter iter;
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
      gint n;
      GtkTreeModel *model;
      
      model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
      gtk_tree_model_get (model, &iter, 1, &hg->e_list, -1);
      
    }
  }

  hg->plan_obj_i=hg->e_list;

  switch(hg->inst){
  case INST_HDS:
    if(hg->e_list!=-1){
      hg->plan_obj_exp=hg->obj[hg->e_list].exp;
      gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_exp_adj),(gdouble)hg->plan_obj_exp);
      
      hg->plan_obj_repeat=hg->obj[hg->e_list].repeat;
      gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_obj_adj),(gdouble)hg->plan_obj_repeat);
      
      switch(hg->obj[hg->e_list].guide){
      case NO_GUIDE:
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_guide_combo),
				 NO_GUIDE);
	hg->plan_obj_guide=NO_GUIDE;
	break;
      case AG_GUIDE:
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_guide_combo),
				 AG_GUIDE);
	hg->plan_obj_guide=AG_GUIDE;
	break;
      case SV_GUIDE:
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_guide_combo),
				 SV_GUIDE);
	hg->plan_obj_guide=SV_GUIDE;
	break;
      case SVSAFE_GUIDE:
	gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_obj_guide_combo),
				 SVSAFE_GUIDE);
	hg->plan_obj_guide=SVSAFE_GUIDE;
      }
    }
    break;

  case INST_IRCS:
    if(hg->e_list!=-1){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->plan_adi_check),
				   hg->obj[hg->e_list].adi);
    }
    break;
    
  case INST_HSC:
    if(hg->e_list!=-1){
      gtk_widget_set_sensitive(hg->plan_hbox_dz,hg->obj[hg->e_list].std);
    }
    break;
  }
}


static void cc_setup_list (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg = (typHOE *) gdata;

  {
    GtkTreeIter iter;
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
      GtkTreeModel *model;
      
      model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
      gtk_tree_model_get (model, &iter, 1, &hg->plan_tmp_setup, -1);

      switch(hg->inst){
      case INST_IRCS:
	hg->plan_obj_dexp=hg->ircs_set[hg->plan_tmp_setup].exp;
	hg->plan_dith=hg->ircs_set[hg->plan_tmp_setup].dith;
	hg->plan_dithw=hg->ircs_set[hg->plan_tmp_setup].dithw;
	hg->plan_osra=hg->ircs_set[hg->plan_tmp_setup].osra;
	hg->plan_osdec=hg->ircs_set[hg->plan_tmp_setup].osdec;
	hg->plan_sssep=hg->ircs_set[hg->plan_tmp_setup].sssep;
	hg->plan_ssnum=hg->ircs_set[hg->plan_tmp_setup].ssnum;
	
	if(flagPlanEditDialog){
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_dexp_adj), hg->plan_obj_dexp);
	  gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_e_dith_combo), hg->plan_dith);
	  
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_dithw_adj), hg->plan_dithw);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_osra_adj),  (gdouble)hg->plan_osra);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_osdec_adj), (gdouble)hg->plan_osdec);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_sssep_adj), hg->plan_sssep);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_ssnum_adj), (gdouble)hg->plan_ssnum);
	}
	else{
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_dexp_adj), hg->plan_obj_dexp);
	  gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_dith_combo), hg->plan_dith);
	  
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_dithw_adj), hg->plan_dithw);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_osra_adj),  (gdouble)hg->plan_osra);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_osdec_adj), (gdouble)hg->plan_osdec);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_sssep_adj), hg->plan_sssep);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_ssnum_adj), (gdouble)hg->plan_ssnum);
	}

	break;

      case INST_HSC:
	hg->plan_obj_dexp=hg->hsc_set[hg->plan_tmp_setup].exp;
	hg->plan_osra=hg->hsc_set[hg->plan_tmp_setup].osra;
	hg->plan_osdec=hg->hsc_set[hg->plan_tmp_setup].osdec;
	
	if(flagPlanEditDialog){
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_dexp_adj), hg->plan_obj_dexp);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_osra_adj), (gdouble)hg->plan_osra);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_osdec_adj),(gdouble)hg->plan_osdec);
	  hsc_set_skip_stop(hg, hg->plan_tmp_setup);
	  gtk_adjustment_set_upper(GTK_ADJUSTMENT(hg->plan_e_skip_adj), (gdouble)hg->plan_skip_upper);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_skip_adj), (gdouble)hg->plan_skip);
	  gtk_adjustment_set_upper(GTK_ADJUSTMENT(hg->plan_e_stop_adj), (gdouble)hg->plan_stop_upper);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_e_stop_adj), (gdouble)hg->plan_stop);
	  hsc_set_skip_color(hg);
	  hsc_set_stop_color(hg);
	}
	else{
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_dexp_adj), hg->plan_obj_dexp);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_osra_adj),  (gdouble)hg->plan_osra);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_osdec_adj), (gdouble)hg->plan_osdec);
	  hsc_set_skip_stop(hg, hg->plan_tmp_setup);
	  gtk_adjustment_set_upper(GTK_ADJUSTMENT(hg->plan_skip_adj), (gdouble)hg->plan_skip_upper);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_skip_adj), (gdouble)hg->plan_skip);
	  gtk_adjustment_set_upper(GTK_ADJUSTMENT(hg->plan_stop_adj), (gdouble)hg->plan_stop_upper);
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(hg->plan_stop_adj), (gdouble)hg->plan_stop);
	  hsc_set_skip_color(hg);
	  hsc_set_stop_color(hg);
	}

	set_sensitive_hsc_30(hg);
	break;
      }
    }
  }

}

void cc_get_toggle_adi (GtkWidget * widget, gpointer gdata)
{
  typHOE *hg;
  
  hg = (typHOE *) gdata;

  if(hg->plan_obj_i==-1){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please select an object to observe with ADI (= Angular Differencial Imaging).",
		  NULL);
    hg->plan_adi=FALSE;
  }
  else if(hg->obj[hg->plan_obj_i].i_nst>=0){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "ADI (= Angular Differencial Imaging) cannot be adopted to non-sidereal targets.",
		  NULL);
    hg->plan_adi=FALSE;
  }
  else{
    hg->plan_adi=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  }
}


static void cc_get_plan_dith (GtkWidget *widget, gpointer gdata)
{
  gint i_sel;
  gchar *mode_str=NULL, *err_str=NULL;
  gboolean ok_flag=FALSE;
  typHOE *hg;
  

  hg = (typHOE *) gdata;

  {
    GtkTreeIter iter;
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
      GtkTreeModel *model;
      
      model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
      gtk_tree_model_get (model, &iter, 1, &i_sel, -1);

      switch(hg->ircs_set[hg->plan_tmp_setup].mode){
      case IRCS_MODE_IM:
	if(IRCS_dith[i_sel].f_im) ok_flag=TRUE;
	mode_str=g_strdup("Imaging");
	break;
      case IRCS_MODE_PI:
	if(IRCS_dith[i_sel].f_pi) ok_flag=TRUE;
	mode_str=g_strdup("Imaging Polarimetry");
	break;
      case IRCS_MODE_GR:
	if(IRCS_dith[i_sel].f_gr) ok_flag=TRUE;
	mode_str=g_strdup("Grism Spectroscopy");
	break;
      case IRCS_MODE_PS:
	if(IRCS_dith[i_sel].f_ps) ok_flag=TRUE;
	mode_str=g_strdup("Grism Spectro-Polarimetry");
	break;
      case IRCS_MODE_EC:
	if(IRCS_dith[i_sel].f_ec) ok_flag=TRUE;
	mode_str=g_strdup("Echelle Spectroscopy");
	break;
      }

      if(ok_flag){
	hg->plan_dith=i_sel;
      }
      else{
	err_str=g_strdup_printf("Error: \"%s\" can not be used for \"%s\"",
				IRCS_dith[i_sel].name,
				mode_str);

	popup_message(hg->plan_main,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      err_str,
		      NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),
				 hg->ircs_set[hg->plan_tmp_setup].dith);
	
      }
    }
  }
  if(mode_str) g_free(mode_str);
  if(err_str) g_free(err_str);
}


GtkWidget *make_plan_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;
  GdkPixbuf *pixbuf, *pixbuf2;
  gint w,h;
  
  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);
  
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

  
  //File/Quit
  //File/Write OPE with Plan
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Write Plan OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan_ope,(gpointer)hg);

  //File/Write OPE with Plan
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Write Plan Text");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan Text");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan_txt,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  /*
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Write Plan YAML");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan YAML");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan_yaml,(gpointer)hg);
  */

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
  my_signal_connect (popup_button, "activate",menu_close_plan,(gpointer)hg);


  // Service
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/calendar_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  menu_item =gtkut_image_menu_item_new_with_label (image, "Service");
#else
  menu_item =gtk_image_menu_item_new_with_label ("Service");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/calendar_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Allocation Calculator");
#else
  popup_button =gtk_image_menu_item_new_with_label ("Allocation Calculator");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_calc_service,(gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write Service Request");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Service Request");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_service_txt,(gpointer)hg);


  // Init
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("tab-new", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Init");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Init");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  
  //Init/Initialize Plan
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("tab-new", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Initialize Plan");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Initialize Plan");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_init_plan,(gpointer)hg);

  //Init/Clear All
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-clear-all", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Clear All");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Clear All");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_init_plan0,(gpointer)hg);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}

static GtkTreeModel *
create_plan_model (typHOE *hg)
{
  gint i_plan = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_PLAN_COLUMNS, 
			      G_TYPE_LONG,  // sod
			      G_TYPE_INT,   // stime
			      G_TYPE_INT,   // time
			      G_TYPE_STRING, // txt
			      G_TYPE_INT,    // filter
                              G_TYPE_STRING,  // txt_az
                              G_TYPE_STRING,  // txt_el
			      G_TYPE_DOUBLE,  // moon_sep
			      G_TYPE_INT,    // weight
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
			      G_TYPE_BOOLEAN,
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //fgcolor
			      GDK_TYPE_RGBA,   //bgcolor
			      GDK_TYPE_RGBA,    //color for azel
#else
			      GDK_TYPE_COLOR,   //fgcolor
			      GDK_TYPE_COLOR,   //bgcolor
			      GDK_TYPE_COLOR,    //color for azel
#endif
			      G_TYPE_BOOLEAN);

  for (i_plan = 0; i_plan < hg->i_plan_max; i_plan++){
    gtk_list_store_append (model, &iter);
    tree_update_plan_item(hg, GTK_TREE_MODEL(model), iter, i_plan); 
  }

  return GTK_TREE_MODEL (model);
}


static void
plan_add_columns (typHOE *hg,
	     GtkTreeView  *treeview, 
	     GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  
  /* ToD column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_TOD));
  column=gtk_tree_view_column_new_with_attributes ("ToD",
						   renderer,
						   "text",
						   COLUMN_PLAN_TOD,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  plan_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PLAN_TOD),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Slew column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_STIME));
  column=gtk_tree_view_column_new_with_attributes ("Slew",
					    renderer,
					    "text",
					    COLUMN_PLAN_STIME,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  plan_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PLAN_STIME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Time column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_TIME));
  column=gtk_tree_view_column_new_with_attributes ("Time",
					    renderer,
					    "text",
					    COLUMN_PLAN_TIME,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  plan_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PLAN_TIME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Txt column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable",FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_TXT));
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(hg->plan_tree),
					       -1,
					       "Observational Tasks",
					       renderer,
					       "text", COLUMN_PLAN_TXT,
					       "weight", COLUMN_PLAN_WEIGHT,
#ifdef USE_GTK3
					       "foreground-rgba", COLUMN_PLAN_COL,
#else
					       "foreground-gdk", COLUMN_PLAN_COL,
#endif
					       "foreground-set", COLUMN_PLAN_COLSET,
#ifdef USE_GTK3
					       "foreground-rgba", COLUMN_PLAN_COLFG,
					       "background-rgba", COLUMN_PLAN_COLBG,
#else
					       "foreground-gdk", COLUMN_PLAN_COLFG,
					       "background-gdk", COLUMN_PLAN_COLBG,
#endif
					       NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_PLAN_TXT);
  //gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* HSC Filter */
  if(hg->inst==INST_HSC){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_PLAN_FIL));
    column=gtk_tree_view_column_new_with_attributes ("Filter",
						     renderer,
						     "text",
						     COLUMN_PLAN_FIL,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    plan_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_PLAN_FIL),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* AZEL column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable",FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_TXT_AZ));
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(hg->plan_tree),
					       -1,
					       "Az",
					       renderer,
					       "text", COLUMN_PLAN_TXT_AZ,
#ifdef USE_GTK3
					       "foreground-rgba", 
#else
					       "foreground-gdk", 
#endif
					       COLUMN_PLAN_COL_AZEL,
					       "foreground-set", COLUMN_PLAN_COLSET_AZEL,
					       NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable",FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_TXT_EL));
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(hg->plan_tree),
					       -1,
					       "El",
					       renderer,
					       "text", COLUMN_PLAN_TXT_EL,
#ifdef USE_GTK3
					       "foreground-rgba", 
#else
					       "foreground-gdk", 
#endif
					       COLUMN_PLAN_COL_AZEL,
					       "foreground-set", COLUMN_PLAN_COLSET_AZEL,
					       NULL);

  /* Moon Distance */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PLAN_MOON));
  column=gtk_tree_view_column_new_with_attributes ("Moon",
					    renderer,
					    "text",
					    COLUMN_PLAN_MOON,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  plan_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PLAN_MOON),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


void plan_long_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  glong value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch(index) {
  case COLUMN_PLAN_TOD:
    if(value>0){
      str=get_txt_tod(value);
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void plan_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  glong   long_value;
  gdouble double_value;
  gint    int_value;
  gchar *str=NULL;
  
  switch (index) {
  case COLUMN_PLAN_TOD:
    gtk_tree_model_get (model, iter, 
			index, &long_value,
			-1);
    break;
    
  case COLUMN_PLAN_STIME:
  case COLUMN_PLAN_TIME:
  case COLUMN_PLAN_FIL:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;

  case COLUMN_PLAN_MOON:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;   
  }

  switch (index) {
  case COLUMN_PLAN_TOD:
    if(long_value>0){
      str=get_txt_tod(long_value);
    }
    else{
      str=NULL;
    }
    break;
    
  case COLUMN_PLAN_TIME:
    if(int_value>0){
      str=g_strdup_printf("%d",(gint)int_value);
    }
    else if(int_value<0){
      str=g_strdup_printf("(%d)",(gint)(-int_value));
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_PLAN_STIME:
    if(int_value>0){
      str=g_strdup_printf("%d",int_value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_PLAN_MOON:
    if(double_value>0){
      str=g_strdup_printf("%.0lf",double_value);
    }
    else{
      str=NULL;
    }
    break;
    
  case COLUMN_PLAN_FIL:
    if(int_value>=0){
      str=g_strdup_printf("%s",hsc_filter[int_value].name);
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}

void refresh_tree (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));

  switch(hg->inst){
  case INST_HDS:
    plan_check_consistency(hg);
    break;
  }

  calc_sun_plan(hg);
  remake_tod(hg, model);
  remake_txt(hg, model);
}


static void
remove_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if(flagService) close_service(NULL,(gpointer)hg);
    
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_plan,j;
    GtkTreePath *path;
   
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	
    for(i_plan=i;i_plan<hg->i_plan_max;i_plan++){
      hg->plan[i_plan]=hg->plan[i_plan+1];
    }

    hg->i_plan_max--;
    
    remake_tod(hg, model); 
    gtk_tree_path_free (path);

    refresh_plan_plot(hg);
  }
}


static void
dup_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if(flagService) close_service(NULL,(gpointer)hg);
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_plan,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    for(i_plan=hg->i_plan_max;i_plan>i+1;i_plan--){
      swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
    }

    hg->i_plan_max++;
  
    hg->plan[i+1]=hg->plan[i];
    hg->plan[i+1].comment=g_strdup(hg->plan[i].comment);
    hg->plan[i+1].txt    =g_strdup(hg->plan[i].txt);

    if(hg->plan[i].comment) g_free(hg->plan[i].comment);
    hg->plan[i].comment=g_strdup(hg->plan[i+1].comment);
    if(hg->plan[i].txt) g_free(hg->plan[i].txt);
    hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);


    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i+1);

    remake_tod(hg, model); 
    remake_txt(hg, model);
    gtk_tree_path_free (path);

    refresh_plan_plot(hg);
  }
}


static void
up_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter1, iter2;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if(flagService) close_service(NULL,(gpointer)hg);
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter1)){
    gint i, i_plan,j;
    GtkTreePath *path1, *path2;
    
    path1 = gtk_tree_model_get_path (model, &iter1);
    path2 = gtk_tree_path_copy (path1);
    if(!gtk_tree_path_prev( path2 )){
      gtk_tree_path_free (path1);
      gtk_tree_path_free (path2);
      return;
    }
    i = gtk_tree_path_get_indices (path1)[0];
    
    gtk_tree_model_get_iter( model, &iter2, path2 );
    gtk_list_store_swap( GTK_LIST_STORE( model ), &iter1, &iter2 );

    swap_plan(&hg->plan[i],&hg->plan[i-1]);

    remake_tod(hg, model); 

    gtk_tree_path_free (path1);
    gtk_tree_path_free (path2);
  }
}


static void
down_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter1, iter2;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if(flagService) close_service(NULL,(gpointer)hg);
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter1)){
    gint i, i_plan,j;
    GtkTreePath *path1, *path2;
    
    path1 = gtk_tree_model_get_path (model, &iter1);
    path2 = gtk_tree_path_copy (path1);
    i = gtk_tree_path_get_indices (path1)[0];
    if(i==hg->i_plan_max-1){
      gtk_tree_path_free (path1);
      gtk_tree_path_free (path2);
      return;
    }
    gtk_tree_path_next( path2 );
    
    gtk_tree_model_get_iter( model, &iter2, path2 );
    gtk_list_store_swap( GTK_LIST_STORE( model ), &iter1, &iter2 );

    swap_plan(&hg->plan[i+1],&hg->plan[i]);

    remake_tod(hg, model); 

    gtk_tree_path_free (path1);
    gtk_tree_path_free (path2);
  }
}

gchar * make_plan_txt(typHOE *hg, PLANpara plan){
  gchar *ret=NULL;
  
  switch(hg->inst){
  case INST_HDS:
    ret=hds_make_plan_txt(hg, plan);
    break;

  case INST_IRCS:
    ret=ircs_make_plan_txt(hg, plan);
    break;
    
  case INST_HSC:
    ret=hsc_make_plan_txt(hg, plan);
    break;
  }

  return(ret);
}

gchar * hds_make_plan_txt(typHOE *hg, PLANpara plan){
  gchar *bu_tmp, *pa_tmp, *sv1_tmp, *sv2_tmp,
    set_tmp[64], guide_tmp[64], *is_tmp, *bin_tmp, *col_tmp, *ret_txt;
  typPlanMoon moon;
  glong sod_moon;

  
  switch(plan.type){
  case PLAN_TYPE_OBJ:
    if(plan.backup){
      bu_tmp=g_strdup("(Back Up)  ");
    }
    else if(plan.i2_pos==PLAN_I2_IN){
      bu_tmp=g_strdup("[+I2]  ");
    }
    else{
      bu_tmp=g_strdup("");
    }

    if(plan.pa_or)   pa_tmp=g_strdup_printf(" / SlitPA=%.1fdeg",plan.pa);
    else pa_tmp=g_strdup("");

    if(plan.sv_or)   sv1_tmp=g_strdup_printf(" / SV Exp=%dms",plan.sv_exp);
    else sv1_tmp=g_strdup("");

    switch(plan.sv_fil){
    case SV_FILTER_NONE:
      sv2_tmp=g_strdup("");
      break;
    case SV_FILTER_R:
      sv2_tmp=g_strdup(" / SV Filter=R-band");
      break;
    case SV_FILTER_BP530:
      sv2_tmp=g_strdup(" / SV Filter=BP530");
      break;
    case SV_FILTER_ND2:
      sv2_tmp=g_strdup(" / SV Filter=ND2");
      break;
    }

    if(hg->setup[plan.setup].setup<0){
      sprintf(set_tmp,"Setup-%d : NonStd-%d %dx%dbin",
	      plan.setup+1,
	      -hg->setup[plan.setup].setup,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    else{
      sprintf(set_tmp,"Setup-%d : Std%s %dx%dbin",
	      plan.setup+1,
	      HDS_setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }

    switch(plan.guide){
    case NO_GUIDE:
      sprintf(guide_tmp,"[No Guide]");
      break;
    case AG_GUIDE:
      sprintf(guide_tmp,"[AG Guide]");
      break;
    case SV_GUIDE:
      sprintf(guide_tmp,"[SV Guide]");
      break;
    case SVSAFE_GUIDE:
      sprintf(guide_tmp,"[SV(Safe) Guide]");
    }
    
    switch(plan.omode){
    case PLAN_OMODE_FULL:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("%s\"%s\", %dsec x%d,  %s, %.2fx%.2f slit, %s%s%s%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				plan.exp,
				plan.repeat,
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.,
				guide_tmp,
				pa_tmp, sv1_tmp,sv2_tmp);
      }
      else{
	ret_txt=g_strdup_printf("%s\"%s\", %dsec x%d,  %s, %s%s%s%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				plan.exp,
				plan.repeat,
				set_tmp,
				guide_tmp,
				pa_tmp, sv1_tmp,sv2_tmp);
      }
      break;
    case PLAN_OMODE_SET:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("%sSetupField \"%s\",  %s, %.2fx%.2f slit, %s%s%s%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.,
				guide_tmp,
				pa_tmp, sv1_tmp,sv2_tmp);
      }
      else{
	ret_txt=g_strdup_printf("%sSetupField \"%s\",  %s, %s%s%s%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				set_tmp,
				guide_tmp,
				pa_tmp, sv1_tmp,sv2_tmp);
      }
      break;
    case PLAN_OMODE_GET:
      ret_txt=g_strdup_printf("%sGetObject \"%s\", %dsec x%d",
			      bu_tmp,
			      hg->obj[plan.obj_i].name,
			      plan.exp,
			      plan.repeat);
    }
    if(bu_tmp) g_free(bu_tmp);
    if(pa_tmp) g_free(pa_tmp);
    if(sv1_tmp) g_free(sv1_tmp);
    if(sv2_tmp) g_free(sv2_tmp);
    break;

  case PLAN_TYPE_FOCUS:
    if(plan.i2_pos==PLAN_I2_IN){
      bu_tmp=g_strdup("[+I2]  ");
    }
    else{
	bu_tmp=g_strdup("");
    }
    
    switch(plan.focus_mode){
    case PLAN_FOCUS1:
	if(plan.focus_is==IS_NO){
	  ret_txt=g_strdup_printf("%sFocus SV w/Slit",bu_tmp);
	}
	else{
	  ret_txt=g_strdup_printf("%sFocus SV w/IS",bu_tmp);
	}
	break;
    default:
      if(plan.focus_is==IS_NO){
	ret_txt=g_strdup_printf("%sFocus AG w/Slit",bu_tmp);
      }
	else{
	  ret_txt=g_strdup_printf("%sFocus AG w/IS",bu_tmp);
	}
      break;
      }
    
    g_free(bu_tmp);
    break;


  case PLAN_TYPE_BIAS:
    if(hg->setup[plan.setup].setup<0){
      sprintf(set_tmp,"Setup-%d : %dx%dbin",
	      plan.setup+1,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    else{
      sprintf(set_tmp,"Setup-%d : %dx%dbin",
	      plan.setup+1,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    
    ret_txt=g_strdup_printf("BIAS x%d, %s",plan.repeat,set_tmp);
    break;

  case PLAN_TYPE_FLAT:
    if(hg->setup[plan.setup].setup<0){
      sprintf(set_tmp,"Setup-%d : NonStd-%d %dx%dbin",
	      plan.setup+1,
	      -hg->setup[plan.setup].setup,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    else{
      sprintf(set_tmp,"Setup-%d : Std%s %dx%dbin",
	      plan.setup+1,
	      HDS_setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);

    }

    if(hg->setup[plan.setup].i2){
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Flat x%d + Flat w/I2, %s, %.2fx%.2f slit",
				plan.repeat,
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Flat x%d + Flat w/I2, %s",
				plan.repeat,
				set_tmp);
      }
    }
    else{
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Flat x%d, %s, %.2fx%.2f slit",
				plan.repeat,
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Flat x%d, %s",
				plan.repeat,
				set_tmp);
      }
    }
    break;

  case PLAN_TYPE_COMP:
    if(hg->setup[plan.setup].setup<0){
      sprintf(set_tmp,"Setup-%d : NonStd-%d %dx%dbin",
	      plan.setup+1,
	      -hg->setup[plan.setup].setup,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    else{
      sprintf(set_tmp,"Setup-%d : Std%s %dx%dbin",
	      plan.setup+1,
	      HDS_setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }

    if((plan.daytime)
       && (hg->setup[plan.setup].i2)
       && (hg->setup[plan.setup].is == IS_NO)){
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Comparison, %s, %.2fx%.2f & 0.20 slit",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Comparison, %s, %.2f & 0.20 slit",set_tmp,
				(gfloat)hg->setup[plan.setup].slit_width/500.);
      }
    }
    else{
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Comparison, %s, %.2fx%.2f slit",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Comparison, %s",set_tmp);
      }
    }
    break;

  case PLAN_TYPE_SETUP:
    if(plan.is_change){
      is_tmp=g_strdup(" [IS]");
    }
    else{
      is_tmp=g_strdup("");
    }

    if(plan.bin_change){
      bin_tmp=g_strdup(" [Bin]");
    }
    else{
      bin_tmp=g_strdup("");
    }

    if(plan.cmode==PLAN_CMODE_1ST){
      col_tmp=g_strdup_printf(" [Col %+.3lfV]", plan.colv);
    }
    else if(plan.colinc!=0){
      col_tmp=g_strdup_printf(" [Col+=%+d / %+.3lfV]", plan.colinc, plan.colv);
    }
    else{
      col_tmp=g_strdup("");
    }

    if(hg->setup[plan.setup].setup<0){
      sprintf(set_tmp,"Setup-%d : NonStd-%d %dx%dbin",
	      plan.setup+1,
	      -hg->setup[plan.setup].setup,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    else{
      sprintf(set_tmp,"Setup-%d : Std%s %dx%dbin",
	      plan.setup+1,
	      HDS_setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    
    switch(plan.cmode){
    case PLAN_CMODE_FULL:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Setup Change (Full), %s, %.2fx%.2f slit%s%s%s",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.,
				is_tmp,bin_tmp,col_tmp);
      }
      else{
	ret_txt=g_strdup_printf("Setup Change (Full), %s%s%s%s",set_tmp,
				is_tmp,bin_tmp,col_tmp);
      }
      break;
    case PLAN_CMODE_EASY:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Setup Change (Cross Scan), %s, %.2fx%.2f slit%s%s%s",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.,
				is_tmp,bin_tmp,col_tmp);
      }
      else{
	ret_txt=g_strdup_printf("Setup Change (Cross Scan), %s%s%s%s",set_tmp,
				is_tmp,bin_tmp,col_tmp);
      }
      break;
    case PLAN_CMODE_SLIT:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Slit Change, %.2fx%.2f slit",
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Slit Change, %.2fx%.2f slit",
				(gfloat)hg->setup[plan.setup].slit_width/500.,
				(gfloat)hg->setup[plan.setup].slit_length/500.);
      }
      break;
    case PLAN_CMODE_1ST:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Setup Change (1st), %s, %.2fx%.2f slit%s",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.,
				col_tmp);
      }
      else{
	ret_txt=g_strdup_printf("Setup Change (1st), %s%s",set_tmp,col_tmp);
      }
      break;
    }
    g_free(is_tmp);
    g_free(bin_tmp);
    g_free(col_tmp);
    break;

  case PLAN_TYPE_I2:
    if(plan.i2_pos==PLAN_I2_IN){
      if(plan.backup){
	ret_txt=g_strdup("(( I2Cell In ))");
      }
      else{
	ret_txt=g_strdup("I2Cell In");
      }
    }
    else{
      if(plan.backup){
	ret_txt=g_strdup("(( I2Cell Out ))");
      }
      else{
	ret_txt=g_strdup("I2Cell Out");
      }
    }
  
    break;

  case PLAN_TYPE_SetAzEl:
    ret_txt=g_strdup_printf("SetAzEl Az=%+d El=%d",
	    (int)plan.setaz, (int)plan.setel);

    break;

  case PLAN_TYPE_COMMENT:
    switch(plan.comtype){
    case PLAN_COMMENT_TEXT:
      if(plan.comment){
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### %s (%dmin) ###",
				  plan.comment,
				  plan.time/60);
	else
	  ret_txt=g_strdup_printf("### %s ###", plan.comment);
      }
      else{
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### (%dmin)",plan.time/60);
	else
	  ret_txt=g_strdup("###");
      }
      break;
      
    case PLAN_COMMENT_SUNSET:
      sod_moon=(glong)hg->sun.s_set.hours*60*60
	+(glong)hg->sun.s_set.minutes*60;
      moon=calc_typPlanMoon(hg, sod_moon, -1, -1);
      ret_txt=g_strdup_printf("### SunSet %d:%02d, Twilight(18deg) %d:%02d   %d/%d/%d  Moon's Age=%.1lf ###",
			      hg->sun.s_set.hours,
			      hg->sun.s_set.minutes,
			      hg->atw18.s_set.hours,
			      hg->atw18.s_set.minutes,
			      hg->fr_month,
			      hg->fr_day,
			      hg->fr_year,
			      moon.age);
      break;

    case PLAN_COMMENT_SUNRISE:
      sod_moon=(glong)(hg->sun.s_rise.hours+24)*60*60
	+(glong)hg->sun.s_rise.minutes*60;
      moon=calc_typPlanMoon(hg, sod_moon, -1, -1);
      ret_txt=g_strdup_printf("### Twilight(18deg) %d:%02d,  SunRise %d:%02d   Moon's Age=%.1lf ###",
			      hg->atw18.s_rise.hours,
			      hg->atw18.s_rise.minutes,
			      hg->sun.s_rise.hours,
			      hg->sun.s_rise.minutes,
			      moon.age);
      break;
    }
    break;
  }

  return(ret_txt);
}


gchar * ircs_make_plan_txt(typHOE *hg, PLANpara plan){
  gchar *bu_tmp=NULL, *pa_tmp=NULL, *sv1_tmp=NULL, *sv2_tmp=NULL,
    *set_tmp=NULL, *aomode_tmp=NULL, *is_tmp=NULL, *bin_tmp=NULL, *col_tmp=NULL,
    *dith_tmp=NULL, *exp_tmp=NULL, *ndr_tmp=NULL, *adi_tmp=NULL, *ret_txt=NULL;

  
  switch(plan.type){
  case PLAN_TYPE_OBJ:
    if(plan.backup){
      bu_tmp=g_strdup("(Back Up)  ");
    }
    else{
      bu_tmp=g_strdup("");
    }

    pa_tmp=g_strdup("");
    sv1_tmp=g_strdup("");
    sv2_tmp=g_strdup("");

    set_tmp=g_strdup_printf("Setup-%d : %s",
			    plan.setup+1,
			    hg->ircs_set[plan.setup].txt);

    switch(plan.aomode){
    case AOMODE_NO:
      aomode_tmp=g_strdup("[w/o AO]");
      break;
    case AOMODE_NGS_S:
      aomode_tmp=g_strdup("[NGS:self]");
      break;
    case AOMODE_NGS_O:
      aomode_tmp=g_strdup("[NGS:offset]");
      break;
    case AOMODE_LGS_S:
      aomode_tmp=g_strdup("[LGS:self]");
      break;
    case AOMODE_LGS_O:
      aomode_tmp=g_strdup("[LGS:TTGS]");
      break;
    }

    if(plan.adi){
      adi_tmp=g_strdup(" (ADI) ");
    }
    else{
      adi_tmp=g_strdup(" ");
    }

    if(plan.dexp>5){
      exp_tmp=g_strdup_printf("%.0lf",plan.dexp);
    }
    else{
      exp_tmp=g_strdup_printf("%.3lf",plan.dexp);
    }

    dith_tmp=ircs_plan_make_dtxt(hg,
				 plan.setup,
				 plan.dith,
				 plan.dithw,
				 plan.osra,
				 plan.osdec,
				 plan.sssep,
				 plan.ssnum);

    if(plan.ndr==IRCS_DEF_NDR){
      ndr_tmp=g_strdup(" ");
    }
    else{
      ndr_tmp=g_strdup_printf(" (NDR=%d) ",plan.ndr);
    }
    
    switch(plan.omode){
    case PLAN_OMODE_FULL:
      if(dith_tmp){
	ret_txt=g_strdup_printf("%s\"%s\"%s%s %ssec x%d%sx%d / %s / dither=%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				aomode_tmp,
				adi_tmp,
				exp_tmp,
				plan.coadds,
				ndr_tmp,
				plan.repeat,
				set_tmp,
				dith_tmp);
      }
      else{
	ret_txt=g_strdup_printf("%s\"%s\"%s%s %ssec x%d%sx%d / %s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				aomode_tmp,
				adi_tmp,
				exp_tmp,
				plan.coadds,
				ndr_tmp,
				plan.repeat,
				set_tmp);
      }
      break;
    case PLAN_OMODE_SET:
      ret_txt=g_strdup_printf("%sTarget Acquisition \"%s\" %s%s%s",
			      bu_tmp,
			      hg->obj[plan.obj_i].name,
			      aomode_tmp,
			      adi_tmp,
			      set_tmp);
      break;
    case PLAN_OMODE_GET:
      if(dith_tmp){
	ret_txt=g_strdup_printf("%sGetObject\"%s\" %s %ssec x%d%sx%d / %s / dither=%s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				aomode_tmp,
				exp_tmp,
				plan.coadds,
				ndr_tmp,
				plan.repeat,
				set_tmp,
				dith_tmp);
      }
      else{
	ret_txt=g_strdup_printf("%sGetObject\"%s\" %s %ssec x%d%sx%d / %s",
				bu_tmp,
				hg->obj[plan.obj_i].name,
				aomode_tmp,
				exp_tmp,
				plan.coadds,	
				ndr_tmp,
				plan.repeat,
				set_tmp);
      }
      break;
    }
    if(bu_tmp) g_free(bu_tmp);
    if(pa_tmp) g_free(pa_tmp);
    if(sv1_tmp) g_free(sv1_tmp);
    if(sv2_tmp) g_free(sv2_tmp);
    if(set_tmp) g_free(set_tmp);
    if(aomode_tmp) g_free(aomode_tmp);
    if(exp_tmp) g_free(exp_tmp);
    if(dith_tmp) g_free(dith_tmp);
    if(ndr_tmp) g_free(ndr_tmp);
    if(adi_tmp) g_free(adi_tmp);
    break;

  case PLAN_TYPE_FOCUS:
    switch(plan.focus_mode){
    case PLAN_FOCUS1:
      ret_txt=g_strdup_printf("Focus OBE");
    break;
    case PLAN_FOCUS2:
      ret_txt=g_strdup_printf("LGS Calibration");
    break;
    }
    break;

    
  case PLAN_TYPE_FLAT:
    if(plan.cal_mode==-1){
      set_tmp=g_strdup_printf("Setup-%d : %s",
			      plan.setup+1,
			      hg->ircs_set[plan.setup].txt);
    }
    else{
      set_tmp=g_strdup_printf("for all [%s] setups",
			      ircs_mode_name[plan.cal_mode]);
    }

    ret_txt=g_strdup_printf("Flat x%d, %s",
			    plan.repeat,
			    set_tmp);
    break;

  case PLAN_TYPE_COMP:
    if(plan.cal_mode==-1){
      set_tmp=g_strdup_printf("Setup-%d : %s",
			      plan.setup+1,
			      hg->ircs_set[plan.setup].txt);
    }
    else{
      set_tmp=g_strdup_printf("for all [%s] setups",
			      ircs_mode_name[plan.cal_mode]);
    }

    ret_txt=g_strdup_printf("Comparison x%d, %s",
			    plan.repeat,
			    set_tmp);
    break;

  case PLAN_TYPE_BIAS:
  case PLAN_TYPE_SETUP:
  case PLAN_TYPE_I2:
    ret_txt=g_strdup_printf("### (Remove this line for IRCS)");
    break;

  case PLAN_TYPE_SetAzEl:
    ret_txt=g_strdup_printf("SetAzEl Az=%+d El=%d",
	    (int)plan.setaz, (int)plan.setel);

    break;

  case PLAN_TYPE_COMMENT:
    switch(plan.comtype){
    case PLAN_COMMENT_TEXT:
      if(plan.comment){
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### %s (%dmin) ###",
				  plan.comment,
				  plan.time/60);
	else
	  ret_txt=g_strdup_printf("### %s ###", plan.comment);
      }
      else{
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### (%dmin)",plan.time/60);
	else
	  ret_txt=g_strdup("###");
      }
      break;
      
    case PLAN_COMMENT_SUNSET:
      ret_txt=g_strdup_printf("### SunSet %d:%02d, Twilight(18deg) %d:%02d   %d/%d/%d ###",
			      hg->sun.s_set.hours,
			      hg->sun.s_set.minutes,
			      hg->atw18.s_set.hours,
			      hg->atw18.s_set.minutes,
			      hg->fr_month,
			      hg->fr_day,
			      hg->fr_year);
      break;

    case PLAN_COMMENT_SUNRISE:
      ret_txt=g_strdup_printf("### Twilight(18deg) %d:%02d,  SunRise %d:%d ###",
			      hg->atw18.s_rise.hours,
			      hg->atw18.s_rise.minutes,
			      hg->sun.s_rise.hours,
			      hg->sun.s_rise.minutes);
      break;
    }
    break;
  }

  return(ret_txt);
}


gchar * hsc_make_plan_txt(typHOE *hg, PLANpara plan){
  gchar *bu_tmp=NULL, *pa_tmp=NULL,
    *set_tmp=NULL, *exp_tmp=NULL, *skip_tmp=NULL, *stop_tmp=NULL,
    *ret_txt=NULL, *calib_tmp=NULL; 

  
  switch(plan.type){
  case PLAN_TYPE_OBJ:
    if(plan.backup){
      bu_tmp=g_strdup("(Back Up)  ");
    }
    else{
      bu_tmp=g_strdup("");
    }

    if(plan.hsc_30){
      calib_tmp=g_strdup("(30s calib) ");
    }
    else{
      calib_tmp=g_strdup("");
    }

    if(plan.pa_or){
      pa_tmp=g_strdup_printf(" (PA=%.0lf) ",plan.pa);
    }
    else{
      pa_tmp=g_strdup(" ");
    }

    set_tmp=g_strdup_printf("Setup-%d : %s",
			    plan.setup+1,
			    hg->hsc_set[plan.setup].txt);

    exp_tmp=g_strdup_printf("%.0lf",plan.dexp);

    if(plan.skip!=0){
      skip_tmp=g_strdup_printf(" Skip=%d",plan.skip);
    }
    else{
      skip_tmp=g_strdup("");
    }
    
    switch(hg->hsc_set[plan.setup].dith){
    case HSC_DITH_NO:
      stop_tmp=g_strdup("");
      break;

    case HSC_DITH_5:
      if(plan.stop!=5){
	stop_tmp=g_strdup_printf(" Stop=%d",plan.stop);
      }
      else{
	stop_tmp=g_strdup("");
      }
      break;

    case HSC_DITH_N:
      if(plan.stop!=hg->hsc_set[plan.setup].dith_n){
	stop_tmp=g_strdup_printf(" Stop=%d",plan.stop);
      }
      else{
	stop_tmp=g_strdup("");
      }
      break;
    }

    ret_txt=g_strdup_printf("%s%s\"%s\"%s%ssec / %s%s%s x%d",
			    bu_tmp,
			    calib_tmp,
			    hg->obj[plan.obj_i].name,
			    pa_tmp,
			    exp_tmp,
			    set_tmp,
			    skip_tmp,
			    stop_tmp,
			    plan.repeat);
    
    if(bu_tmp) g_free(bu_tmp);
    if(calib_tmp) g_free(calib_tmp);
    if(pa_tmp) g_free(pa_tmp);
    if(exp_tmp) g_free(exp_tmp);
    if(set_tmp) g_free(set_tmp);
    if(skip_tmp) g_free(skip_tmp);
    if(stop_tmp) g_free(stop_tmp);
    break;

  case PLAN_TYPE_FOCUS:
    if(plan.focus_mode==0){
      set_tmp=g_strdup("@ current position");
    }
    else{
      set_tmp=g_strdup_printf("@ %s", hg->obj[plan.focus_mode-1].name);
    }

    ret_txt=g_strdup_printf("Focus OBE w/%s %s, z=%.2lf  + 30s exp.",
			    hsc_filter[hg->hsc_set[plan.setup].filter].name,
			    set_tmp,
			    plan.focus_z);
    if(set_tmp) g_free(set_tmp);
    break;

    
  case PLAN_TYPE_FLAT:
    set_tmp=g_strdup_printf("w/%s",
			    hsc_filter[hg->hsc_set[plan.setup].filter].name);

    ret_txt=g_strdup_printf("Dome Flat x%d, %s",
			    plan.repeat,
			    set_tmp);
    if(set_tmp) g_free(set_tmp);
    break;

  case PLAN_TYPE_SETUP:
    ret_txt=g_strdup_printf("Filter Exchange ===> %s",
			    hsc_filter[hg->hsc_set[plan.setup].filter].name);
    break;
    
  case PLAN_TYPE_BIAS:
  case PLAN_TYPE_I2:
    ret_txt=g_strdup_printf("### (Remove this line for IRCS)");
    break;

  case PLAN_TYPE_SetAzEl:
    ret_txt=g_strdup_printf("SetAzEl Az=%+d El=%d",
	    (int)plan.setaz, (int)plan.setel);

    break;

  case PLAN_TYPE_COMMENT:
    switch(plan.comtype){
    case PLAN_COMMENT_TEXT:
      if(plan.comment){
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### %s (%dmin) ###",
				  plan.comment,
				  plan.time/60);
	else
	  ret_txt=g_strdup_printf("### %s ###", plan.comment);
      }
      else{
	if(plan.time!=0)
	  ret_txt=g_strdup_printf("### (%dmin)",plan.time/60);
	else
	  ret_txt=g_strdup("###");
      }
      break;
      
    case PLAN_COMMENT_SUNSET:
      ret_txt=g_strdup_printf("### SunSet %d:%02d, Twilight(18deg) %d:%02d   %d/%d/%d ###",
			      hg->sun.s_set.hours,
			      hg->sun.s_set.minutes,
			      hg->atw18.s_set.hours,
			      hg->atw18.s_set.minutes,
			      hg->fr_month,
			      hg->fr_day,
			      hg->fr_year);
      break;

    case PLAN_COMMENT_SUNRISE:
      ret_txt=g_strdup_printf("### Twilight(18deg) %d:%02d,  SunRise %d:%d ###",
			      hg->atw18.s_rise.hours,
			      hg->atw18.s_rise.minutes,
			      hg->sun.s_rise.hours,
			      hg->sun.s_rise.minutes);
      break;
    }
    break;
  }

  return(ret_txt);
}


static void
add_1Object_HDS (typHOE *hg, gint i, gint obj_i, gint exp, gint repeat, gint guide)
{
  gint i_plan;

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_OBJ;
  hg->plan[i].slit_or=hg->plan_tmp_or;
  hg->plan[i].setup=hg->plan_tmp_setup;
  hg->plan[i].repeat=hg->plan_obj_repeat;
  hg->plan[i].obj_i=obj_i;
  hg->plan[i].exp=exp;
  
  hg->plan[i].omode=hg->plan_obj_omode;
  hg->plan[i].guide=guide;

  hg->plan[i].backup=hg->plan_backup;
  
  hg->plan[i].time=hds_obj_time(hg->plan[i],
				hg->oh_acq,
				hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout);
  
  switch(hg->plan_obj_omode){
  case PLAN_OMODE_FULL:
  case PLAN_OMODE_SET:
    if(hg->plan_tmp_or){
      hg->plan[i].slit_width=hg->plan_tmp_sw;
      hg->plan[i].slit_length=hg->plan_tmp_sl;
    }
    else{
      hg->plan[i].slit_width=200;
      hg->plan[i].slit_length=2000;
    }
    break;
  case PLAN_OMODE_GET:
    hg->plan[i].slit_width=200;
    hg->plan[i].slit_length=2000;
    break;
  }

  hg->plan[i].pa=hg->obj[hg->plan[i].obj_i].pa;
  if(hg->obj[hg->plan[i].obj_i].mag<MAG_SV2SEC){
    hg->plan[i].sv_exp=1000; 
    hg->plan[i].sv_or=TRUE;
 }
  else if(hg->obj[hg->plan[i].obj_i].mag<MAG_SV3SEC){
    hg->plan[i].sv_exp=2000;
    hg->plan[i].sv_or=TRUE;
  }
  else if(hg->obj[hg->plan[i].obj_i].mag<MAG_SV5SEC){
    hg->plan[i].sv_exp=3000;
    hg->plan[i].sv_or=TRUE;
  }
  else if(hg->obj[hg->plan[i].obj_i].mag<99){
    hg->plan[i].sv_exp=5000;
    hg->plan[i].sv_or=TRUE;
  }
  else{
    hg->plan[i].sv_exp=hg->exptime_sv;
    hg->plan[i].sv_or=FALSE;
  }
  if(hg->obj[hg->plan[i].obj_i].mag<MAG_SVFILTER2){
    hg->plan[i].sv_fil=SV_FILTER_ND2;
  }
  else if(hg->obj[hg->plan[i].obj_i].mag<MAG_SVFILTER1){
    hg->plan[i].sv_fil=SV_FILTER_R;
  }
  else{
    hg->plan[i].sv_fil=SV_FILTER_NONE;
  }
  hg->plan[i].backup=FALSE;

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);
}


static void
add_1Object_IRCS (typHOE *hg, gint i, gint obj_i)
{
  gint i_plan;

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_OBJ;
  hg->plan[i].setup=hg->plan_tmp_setup;
  hg->plan[i].repeat=hg->plan_obj_repeat;
  hg->plan[i].obj_i=obj_i;

  hg->plan[i].dexp=hg->plan_obj_dexp;
  hg->plan[i].coadds=hg->plan_ircs_coadds;
  hg->plan[i].ndr=hg->plan_ircs_ndr;

  hg->plan[i].dith=hg->plan_dith;
  hg->plan[i].dithw=hg->plan_dithw;
  hg->plan[i].osra=hg->plan_osra;
  hg->plan[i].osdec=hg->plan_osdec;
  hg->plan[i].sssep=hg->plan_sssep;
  hg->plan[i].ssnum=hg->plan_ssnum;

  hg->plan[i].shot=ircs_get_shot(hg->plan[i].dith, hg->plan[i].ssnum);
  
  hg->plan[i].omode=hg->plan_obj_omode;
  hg->plan[i].aomode=hg->obj[obj_i].aomode;
  hg->plan[i].adi=hg->obj[obj_i].adi;
 
  hg->plan[i].pa=hg->obj[hg->plan[i].obj_i].pa;

  hg->plan[i].backup=hg->plan_backup;

  hg->plan[i].time=ircs_obj_time(hg->plan[i],
				 hg->oh_acq,
				 ircs_oh_ao(hg,
					    hg->plan[i].aomode,
					    hg->plan[i].obj_i));

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);
  
}

static void
add_1Object_HSC (typHOE *hg, gint i, gint obj_i)
{
  gint i_plan;

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_OBJ;
  hg->plan[i].setup=hg->plan_tmp_setup;
  hg->plan[i].repeat=hg->plan_obj_repeat;
  hg->plan[i].obj_i=obj_i;

  hg->plan[i].dexp=hg->plan_obj_dexp;

  hg->plan[i].osra=hg->plan_osra;
  hg->plan[i].osdec=hg->plan_osdec;
  hg->plan[i].skip=hg->plan_skip;
  hg->plan[i].stop=hg->plan_stop;

  if(hg->obj[obj_i].std){
    hg->plan[i].delta_z=hg->plan_delta_z;
  }

  hg->plan[i].pa=hg->obj[hg->plan[i].obj_i].pa;

  hg->plan[i].hsc_30=hg->plan_hsc_30;
  
  hg->plan[i].backup=hg->plan_backup;

  hg->plan[i].time=hsc_obj_time(hg->plan[i],
				hg->oh_acq);

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);
  
}


static void
add_Object (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i, i_list;
  GtkTreePath *path;
  gchar *tmp;

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->inst==INST_HSC){
    if(hg->obj[hg->plan_obj_i].std){ // Standard
      if((hg->hsc_set[hg->plan_tmp_setup].dith!=HSC_DITH_NO)
	 ||(hg->hsc_set[hg->plan_tmp_setup].ag)){
	tmp=g_strdup_printf("The object \"%s\" is registered as a standard star.",
			    hg->obj[hg->plan_obj_i].name);
	popup_message(hg->plan_main, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      -1,
		      tmp,
		      "You can only use \"No dither\" mode w/o AG for standard stars.",
		      NULL);
	g_free(tmp);
	return;
      }
    }
    else if(hg->obj[i_list].i_nst>=0){ // Non-Sidereal
      if((hg->hsc_set[hg->plan_tmp_setup].dith!=HSC_DITH_NO)
	 ||(hg->hsc_set[hg->plan_tmp_setup].ag)){
	tmp=g_strdup_printf("The object \"%s\" is a non-sidereal target.",
			    hg->obj[hg->plan_obj_i].name);
	popup_message(hg->plan_main, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      -1,
		      tmp,
		      "You can only use \"No dither\" mode w/o AG for non-sidereal targets.",
		      NULL);
	g_free(tmp);
	return;
      }
    }
  }



  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
   
  if(hg->plan_obj_i==-1){
    for(i_list=0;i_list<hg->i_max;i_list++){
      switch(hg->inst){
      case INST_HDS:
	add_1Object_HDS(hg, i+i_list, i_list, 
			hg->obj[i_list].exp, 
			hg->obj[i_list].repeat,
			hg->obj[i_list].guide);
	break;

      case INST_IRCS:
	add_1Object_IRCS(hg, i+i_list, i_list);
	break;
	
      case INST_HSC:
	add_1Object_HSC(hg, i+i_list, i_list);
	break;
      }
      gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i+i_list);

      remake_tod(hg, model); 
      tree_update_plan_item(hg, model, iter, i+i_list);
    }
  }
  else{
    switch(hg->inst){
    case INST_HDS:
      add_1Object_HDS(hg, i, hg->plan_obj_i, 
		      hg->plan_obj_exp, hg->plan_obj_repeat, hg->plan_obj_guide);
      break;

    case INST_IRCS:
      add_1Object_IRCS(hg, i, hg->plan_obj_i);
      break;
      
    case INST_HSC:
      add_1Object_HSC(hg, i, hg->plan_obj_i);
      break;
    }
    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);

    remake_tod(hg, model); 
    tree_update_plan_item(hg, model, iter, i);
  }
  
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
  
}


static void
add_Focus (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    
  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;
  
  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_FOCUS;
  hg->plan[i].focus_mode=hg->plan_focus_mode;
  hg->plan[i].focus_z=hg->plan_focus_z;

  if(hg->inst==INST_HSC){
    hg->plan[i].setup=hg->plan_tmp_setup;
    hg->plan[i].pa_or=FALSE;
    hg->plan[i].pa=(hg->plan[i].focus_mode>0)?
      hg->obj[hg->plan[i].focus_mode-1].pa : hg->def_pa;
  }

  hg->plan[i].time=get_focus_time(hg->plan[i], hg->inst);

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void
add_SetAzEl (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
   

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;
  
  init_planpara(hg, i);
  
  hg->plan[i].type=PLAN_TYPE_SetAzEl;
  hg->plan[i].setaz=hg->plan_setaz;
  hg->plan[i].setel=hg->plan_setel;
  hg->plan[i].az1=hg->plan_setaz;
  hg->plan[i].el1=hg->plan_setel;

  hg->plan[i].daytime=hg->plan_setazel_daytime;
  hg->plan[i].time=0;
  hg->plan[i].stime=slewtime(hg->plan[i-1].az1, hg->plan[i-1].el1,
			     hg->plan[i].setaz, hg->plan[i].setel,
			     hg->vel_az, hg->vel_el);

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}



static void
add_BIAS (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;
  //gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
  

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);
  
  hg->plan[i].type=PLAN_TYPE_BIAS;
  hg->plan[i].setup=hg->plan_tmp_setup;
  hg->plan[i].repeat=hg->plan_bias_repeat;
  hg->plan[i].daytime=hg->plan_bias_daytime;

  hg->plan[i].time=hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout*hg->plan_bias_repeat;

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void
add_Comp (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;
  gchar tmp[64];
  gchar *err_str=NULL;
  
  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  switch(hg->inst){
  case INST_IRCS:
    if(hg->plan_comp_mode==-1){
      if(!ircs_check_spec(hg, hg->ircs_set[hg->plan_tmp_setup].mode)){
	err_str=g_strdup_printf("Error: Comparison is not necessary for \"%s\" mode.",
				ircs_mode_name[hg->ircs_set[hg->plan_tmp_setup].mode]);
      }
    }
    else{
      if(!ircs_check_mode(hg, hg->plan_comp_mode)){
	err_str=g_strdup_printf("Error: There are no \"%s\" modes in your setups.",
				ircs_mode_name[hg->plan_comp_mode]);
      }
    }

    if(err_str){
      popup_message(hg->plan_main,
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    err_str,
		    NULL);
      g_free(err_str);
      return;
    }
    break;
  }

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);
  
  hg->plan[i].type=PLAN_TYPE_COMP;
  hg->plan[i].slit_or=hg->plan_tmp_or;

  if(hg->plan_tmp_or){
    hg->plan[i].slit_width=hg->plan_tmp_sw;
    hg->plan[i].slit_length=hg->plan_tmp_sl;
  }
  else{
    hg->plan[i].slit_width=200;
    hg->plan[i].slit_length=2000;
  }

  switch(hg->inst){
  case INST_HDS:
    hg->plan[i].repeat=1;
    hg->plan[i].cal_mode=-1;
    hg->plan[i].setup=hg->plan_tmp_setup;
    break;
  case INST_IRCS:
    hg->plan[i].repeat=hg->plan_comp_repeat;
    hg->plan[i].cal_mode=hg->plan_comp_mode;
    if(hg->plan_comp_mode==-1){
      hg->plan[i].setup=hg->plan_tmp_setup;
    }
    break;
  }

  hg->plan[i].daytime=hg->plan_comp_daytime;
  hg->plan[i].time=comp_time(hg->plan[i], hg);

  if(hg->inst==INST_HDS){
    if((hg->plan[i].daytime) 
       && (hg->setup[hg->plan_tmp_setup].i2)
       && (hg->setup[hg->plan_tmp_setup].is == IS_NO)){
      hg->plan[i].time+=TIME_SETUP_SLIT*2
	+ 20/hg->binning[hg->setup[hg->plan_tmp_setup].binning].x/hg->binning[hg->setup[hg->plan_tmp_setup].binning].y
	+ hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout;
    }
  }

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void
add_Flat (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan,i_set;
  GtkTreePath *path;
  gint nonstd_flat;
  gboolean same_rb;
  gchar *err_str=NULL;
  //gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  switch(hg->inst){
  case INST_IRCS:
    if(hg->plan_flat_mode!=-1){
      if(!ircs_check_mode(hg, hg->plan_flat_mode)){
	err_str=g_strdup_printf("Error: There are no \"%s\" modes in your setups.",
				ircs_mode_name[hg->plan_flat_mode]);
	
	popup_message(hg->plan_main,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      err_str,
		      NULL);
	if(err_str) g_free(err_str);
	return;
      }
    }
    break;
  }

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;

  init_planpara(hg, i);
  
  hg->plan[i].type=PLAN_TYPE_FLAT;
  hg->plan[i].slit_or=hg->plan_tmp_or;
  
  if(hg->plan_tmp_or){
    hg->plan[i].slit_width=hg->plan_tmp_sw;
    hg->plan[i].slit_length=hg->plan_tmp_sl;
  }
  else{
    hg->plan[i].slit_width=200;
    hg->plan[i].slit_length=2000;
  }

  hg->plan[i].repeat=hg->plan_flat_repeat;
  
  switch(hg->inst){
  case INST_HDS:
    hg->plan[i].cal_mode=-1;
    hg->plan[i].setup=hg->plan_tmp_setup;
    if(hg->plan[i].setup<0){ // NonStd
      i_set=-hg->plan[i].setup-1;
      nonstd_flat=get_nonstd_flat(hg->nonstd[i_set].col, hg->nonstd[i_set].cross); 
      same_rb=get_same_rb(nonstd_flat);
    }
    else{
      i_set=hg->setup[hg->plan[i].setup].setup;
      same_rb=get_same_rb(i_set);
    }
    break;
    
  case INST_IRCS:
    hg->plan[i].cal_mode=hg->plan_flat_mode;
    if(hg->plan_flat_mode==-1){
      hg->plan[i].setup=hg->plan_tmp_setup;
    }
    break;

  case INST_HSC:
    hg->plan[i].cal_mode=-1;
    hg->plan[i].setup=hg->plan_tmp_setup;
    break;
  }

  hg->plan[i].time=flat_time(hg->plan[i], hg);
  
  hg->plan[i].daytime=hg->plan_flat_daytime;


  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void add_plan_setup(typHOE *hg, gint i, gint setup){
  hg->plan[i].type=PLAN_TYPE_SETUP;
  hg->plan[i].setup=setup;
  hg->plan[i].daytime=hg->plan_setup_daytime;
    
  switch(hg->inst){
  case INST_HDS:
    hg->plan[i].slit_or=hg->plan_tmp_or;
    hg->plan[i].cmode=hg->plan_setup_cmode;
    
    switch(hg->plan_setup_cmode){
    case PLAN_CMODE_FULL:
      if(hg->plan_tmp_or){
	hg->plan[i].slit_width=hg->plan_tmp_sw;
	hg->plan[i].slit_length=hg->plan_tmp_sl;
      }
      else{
	hg->plan[i].slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
	hg->plan[i].slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
      }
      hg->plan[i].time=TIME_SETUP_FULL;
      break;
    case PLAN_CMODE_EASY:
      if(hg->plan_tmp_or){
	hg->plan[i].slit_width=hg->plan_tmp_sw;
	hg->plan[i].slit_length=hg->plan_tmp_sl;
      }
      else{
	hg->plan[i].slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
	hg->plan[i].slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
      }
      hg->plan[i].time=TIME_SETUP_EASY;
      break;
    case PLAN_CMODE_SLIT:
      if(hg->plan_tmp_or){
	hg->plan[i].slit_width=hg->plan_tmp_sw;
	hg->plan[i].slit_length=hg->plan_tmp_sl;
      }
      else{
	hg->plan[i].slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
	hg->plan[i].slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
      }
      hg->plan[i].time=TIME_SETUP_SLIT;
      break;
    case PLAN_CMODE_1ST:
      if(hg->plan_tmp_or){
	hg->plan[i].slit_width=hg->plan_tmp_sw;
	hg->plan[i].slit_length=hg->plan_tmp_sl;
      }
      else{
	hg->plan[i].slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
	hg->plan[i].slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
      }
      hg->plan[i].time=TIME_SETUP_FULL;
      break;
    }
    break;
    
  case INST_HSC:
    if(hg->plan[i].daytime){
      hg->plan[i].time=HSC_TIME_FILTER_DAYTIME;
    }
    else{
      hg->plan[i].time=HSC_TIME_FILTER;
    }
    break;
  }
  
  hg->plan[i].repeat=1;
  
  hg->plan[i].obj_i=-1;
  hg->plan[i].exp=0;
  
  hg->plan[i].focus_mode=PLAN_FOCUS1;
  
  hg->plan[i].omode=PLAN_OMODE_FULL;
  hg->plan[i].guide=SV_GUIDE;
  
  hg->plan[i].i2_pos=PLAN_I2_OUT;
  
  if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i].comment=NULL;
  hg->plan[i].comtype=PLAN_COMMENT_TEXT;
  
  hg->plan[i].pa_or=FALSE;
  hg->plan[i].pa=0;
  hg->plan[i].sv_or=FALSE;
  hg->plan[i].sv_exp=hg->exptime_sv;
  hg->plan[i].sv_fil=SV_FILTER_NONE;
  hg->plan[i].backup=FALSE;
  
  if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);
}

static void
add_Setup (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;
  // gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;
  
  add_plan_setup(hg,i,hg->plan_tmp_setup);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    switch(hg->inst){
    case INST_HSC:
      hg->plan[i_plan].el1=90;
      break;
    default:
      hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
      break;
    }
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void add_plan_I2(typHOE *hg, gint i, guint i2_pos){
  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_I2;
  hg->plan[i].i2_pos=i2_pos;

  hg->plan[i].daytime=hg->plan_i2_daytime;
  hg->plan[i].time=TIME_I2;

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);
}



static void
add_I2 (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    
  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;
  
  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_I2;
  hg->plan[i].i2_pos=hg->plan_i2_pos;

  hg->plan[i].daytime=hg->plan_i2_daytime;
  hg->plan[i].time=TIME_I2;

  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}


static void
add_Comment (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;
  if(flagService) close_service(NULL,(gpointer)hg);

  if(hg->i_plan_max==0){
    i=hg->i_plan_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_plan_max;
  }
    

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    swap_plan(&hg->plan[i_plan],&hg->plan[i_plan-1]);
  }

  hg->i_plan_max++;
  
  init_planpara(hg, i);

  hg->plan[i].type=PLAN_TYPE_COMMENT;
  hg->plan[i].comtype=hg->plan_comment_type;

  if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  switch(hg->plan_comment_type){
  case PLAN_COMMENT_TEXT:
    if(hg->plan_comment){
      hg->plan[i].comment=g_strdup(hg->plan_comment);
    }
    else{
      hg->plan[i].comment=NULL;
    }
    hg->plan[i].time=hg->plan_comment_time*60;
    break;
    
  case PLAN_COMMENT_SUNSET:
  case PLAN_COMMENT_SUNRISE:
    hg->plan[i].comment=NULL;
    hg->plan[i].time=0;
    break;
  }
  
  if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  hg->plan[i].txt=make_plan_txt(hg,hg->plan[i]);

  if(i_plan!=0){
    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
  }
  else{
    hg->plan[i_plan].az0=-90.;
    hg->plan[i_plan].el0=+90.;
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=+90.;
  }


  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  hg->plot_i_plan++;
  refresh_plan_plot(hg);
}




void tree_update_plan_item(typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_plan)
{

  // ToD
  if(hg->plan[i_plan].daytime){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TOD,
			hg->plan[i_plan].sod,
			-1);
  }
  
  // Slew
  if(hg->plan[i_plan].daytime){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_STIME,
			0,
			-1);
  }
  else if(hg->plan[i_plan].backup){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_STIME,
			0,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_STIME,
			hg->plan[i_plan].stime/60,
			-1);
  }

  // Time
  if(hg->plan[i_plan].daytime){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TIME,
			-hg->plan[i_plan].time/60,
			-1);
  }
  else if(hg->plan[i_plan].backup){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TIME,
			-hg->plan[i_plan].time/60,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TIME,
			hg->plan[i_plan].time/60,
			-1);
  }

  // Moon
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PLAN_MOON,
		      hg->plan[i_plan].moon.sep,
		      -1);
  
  // Task
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PLAN_TXT,
		      hg->plan[i_plan].txt,
		      -1);

  if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_WEIGHT,
			PANGO_WEIGHT_BOLD,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }

  // Moon
  if(hg->inst==INST_HSC){
    switch(hg->plan[i_plan].type){
    case PLAN_TYPE_OBJ:
    case PLAN_TYPE_FOCUS:
    case PLAN_TYPE_SETUP:
    case PLAN_TYPE_FLAT:
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_FIL,
			  hg->hsc_set[hg->plan[i_plan].setup].filter,
			  -1);
      break;
      
    default:
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_FIL,
			  -1,
			  -1);
      break;
    }
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_FIL,
			-1,
			-1);
  }
  

  // AzEl
  if((hg->plan[i_plan].type==PLAN_TYPE_OBJ)&&
     (!hg->plan[i_plan].backup)){
    if(hg->plan[i_plan].txt_az)
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_TXT_AZ,
			  hg->plan[i_plan].txt_az,
			  -1);
    if(hg->plan[i_plan].txt_el)
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_TXT_EL,
			  hg->plan[i_plan].txt_el,
			  -1);
    if((hg->plan[i_plan].el0<15)||(hg->plan[i_plan].el1<15)){
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_COL_AZEL,&color_pink,
			  COLUMN_PLAN_COLSET_AZEL,TRUE,
			  -1);
    }
    else if((hg->plan[i_plan].el0<29.9)||(hg->plan[i_plan].el1<29.9)){
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_COL_AZEL,&color_orange3,
			  COLUMN_PLAN_COLSET_AZEL,TRUE,
			  -1);
    }
    else{
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_COL_AZEL,&color_black,
			  COLUMN_PLAN_COLSET_AZEL,TRUE,
			  -1);
    }
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TXT_AZ,
			NULL,
			-1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_TXT_EL,
			NULL,
			-1);
  }
 
  switch(hg->plan[i_plan].type){
  case PLAN_TYPE_COMMENT:
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COL,&color_comment,
			COLUMN_PLAN_COLSET,TRUE,
			-1);
    break;

  case PLAN_TYPE_COMP:
  case PLAN_TYPE_FLAT:
  case PLAN_TYPE_BIAS:
  case PLAN_TYPE_SETUP:
  case PLAN_TYPE_I2:
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COL,&color_calib,
			COLUMN_PLAN_COLSET,TRUE,
			-1);
    break;

  case PLAN_TYPE_FOCUS:
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COL,&color_focus,
			COLUMN_PLAN_COLSET,TRUE,
			-1);

    break;

  default:
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COLSET,TRUE,
			-1);
  }

  if((hg->plan[i_plan].setup>=0)&&(hg->plan[i_plan].setup<MAX_USESETUP)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COLFG,
			&color_black,
			COLUMN_PLAN_COLBG,
			&col_plan_setup[hg->plan[i_plan].setup],
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COLFG,NULL,
			COLUMN_PLAN_COLBG,NULL,
			-1);
  }
}


void plan_make_tree(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  if(!flagPlanTree){
    hg=(typHOE *)gdata;

    flagPlanTree=TRUE;

     create_plan_dialog (hg);
    
  }
}


void plan_close_tree(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_widget_destroy(GTK_WIDGET(hg->plan_main));
  hg->plan_main = NULL;
  flagPlanTree=FALSE;
}


void plan_remake_tree(typHOE *hg)
{
  plan_close_tree(NULL,hg);
  plan_make_tree(NULL,hg);
}

static void menu_init_plan0(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gint i_plan;

  init_planpara(hg, 0);
  
  hg->plan[0].type=PLAN_TYPE_COMMENT;
  hg->plan[0].comment=g_strdup("========== Please Insert Your Obs Plan. ==========");
  hg->plan[0].comtype=PLAN_COMMENT_TEXT;
  hg->plan[0].txt=make_plan_txt(hg,hg->plan[0]);

  hg->i_plan_max=1;

  plan_remake_tree(hg);
}


static void menu_init_plan(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  init_plan(hg);
  plan_remake_tree(hg);
}

void init_plan(typHOE *hg){
  switch(hg->inst){
  case INST_HDS:
    hds_init_plan(hg);
    break;
    
  case INST_IRCS:
    ircs_init_plan(hg);
    break;

  case INST_HSC:
    hsc_init_plan(hg);
    break;
  }
}

// Initialize a PLANpara
//  You must set
//           .type
//           .txt
//  + alpha

void init_planpara(typHOE *hg, gint i_plan){
  //hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
  
  hg->plan[i_plan].setup=-1;
  hg->plan[i_plan].repeat=1;
  hg->plan[i_plan].slit_or=FALSE;
  hg->plan[i_plan].slit_width=200;
  hg->plan[i_plan].slit_length=2000;
  
  hg->plan[i_plan].obj_i=-1;
  hg->plan[i_plan].exp=0;
  
  hg->plan[i_plan].dexp=0;
  hg->plan[i_plan].shot=0;
  hg->plan[i_plan].coadds=IRCS_DEF_COADDS;
  hg->plan[i_plan].ndr   =IRCS_DEF_NDR;
  
  hg->plan[i_plan].dith=IRCS_DITH_NO;
  hg->plan[i_plan].dithw=4.0;
  switch(hg->inst){
  case INST_IRCS:
    hg->plan[i_plan].osra=30;
    hg->plan[i_plan].osdec=1800;
    break;
  default:
    hg->plan[i_plan].osra=0;
    hg->plan[i_plan].osdec=0;
    break;
  }
  hg->plan[i_plan].sssep=0.150;
  hg->plan[i_plan].ssnum=5;
  
  hg->plan[i_plan].skip=0;
  hg->plan[i_plan].stop=1;
  
  hg->plan[i_plan].omode=PLAN_OMODE_FULL;
  hg->plan[i_plan].guide=SV_GUIDE;
  hg->plan[i_plan].aomode=AOMODE_NO;
  hg->plan[i_plan].adi=FALSE;
  
  hg->plan[i_plan].focus_mode=PLAN_FOCUS1;
  switch(hg->inst){
  case INST_HSC:
    hg->plan[i_plan].focus_z=hg->hsc_focus_z;
      break;
      
  default:
    hg->plan[i_plan].focus_z=0;
    break;
  }
  hg->plan[i_plan].delta_z=0;
  hg->plan[i_plan].focus_is  =PLAN_I2_OUT;
  hg->plan[i_plan].cal_mode=-1;
  
  hg->plan[i_plan].cmode =PLAN_CMODE_FULL;
  hg->plan[i_plan].colinc=0;
  hg->plan[i_plan].colv  =0;
  hg->plan[i_plan].is_change =FALSE;
  hg->plan[i_plan].bin_change=FALSE;
  
  hg->plan[i_plan].i2_pos=PLAN_I2_OUT;
  
  hg->plan[i_plan].daytime=FALSE;
  
  if(hg->plan[i_plan].comment) g_free(hg->plan[i_plan].comment);
  hg->plan[i_plan].comment=NULL;
  hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
  
  hg->plan[i_plan].time=0;
  hg->plan[i_plan].stime=0;
  hg->plan[i_plan].sod=0;
  
  hg->plan[i_plan].pa_or=FALSE;
  hg->plan[i_plan].pa=0;
  hg->plan[i_plan].sv_or=FALSE;
  hg->plan[i_plan].sv_exp=hg->exptime_sv;
  hg->plan[i_plan].sv_fil=SV_FILTER_NONE;
  hg->plan[i_plan].hsc_30=FALSE;
  hg->plan[i_plan].backup=FALSE;
  
  hg->plan[i_plan].setaz=0;
  hg->plan[i_plan].setel=0;
  
  if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
  hg->plan[i_plan].txt=NULL;
  
  hg->plan[i_plan].moon=init_typPlanMoon();
}

void hds_init_plan(typHOE *hg)
{
  gchar tmp[64];
  gchar b_tmp[64];
  gint i_plan=0;

  calc_sun_plan(hg);

  if(hg->setup[hg->plan_tmp_setup].setup<0){
    sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
	    hg->plan_tmp_setup+1,
	    -hg->setup[hg->plan_tmp_setup].setup,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
    sprintf(b_tmp,"Setup-%d : %dx%dbin",
	    hg->plan_tmp_setup+1,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
  }
  else{
    sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
	    hg->plan_tmp_setup+1,
	    HDS_setups[hg->setup[hg->plan_tmp_setup].setup].initial,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
    sprintf(b_tmp,"Setup-%d : %dx%dbin",
	    hg->plan_tmp_setup+1,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
  }

  // Comment 
  {
    init_planpara(hg, i_plan);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Evening Calibration ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Setup
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_SETUP;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].cmode=PLAN_CMODE_1ST;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=TIME_SETUP_FULL;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // BIAS
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_BIAS;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=5;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=hg->binning[hg->setup[0].binning].readout*5;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Flat
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FLAT;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=10;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comparison
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMP;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=1;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=comp_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Evening Focus ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNSET;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // FocusSV
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FOCUS;
    hg->plan[i_plan].focus_mode=PLAN_FOCUS1;
    hg->plan[i_plan].daytime=FALSE;
    hg->plan[i_plan].time=get_focus_time(hg->plan[i_plan], hg->inst);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("##### <<<<<<<<< INSERT YOUR TARGETS HERE >>>>>>>>> #####");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNRISE; 
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Morning Focus ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // FocusSV
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FOCUS;
    hg->plan[i_plan].focus_mode=PLAN_FOCUS1;
    hg->plan[i_plan].time=get_focus_time(hg->plan[i_plan], hg->inst);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Morning Calibration ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comparison
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMP;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=1;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=comp_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Flat
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FLAT;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=10;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // BIAS
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_BIAS;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].repeat=5;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=hg->binning[hg->setup[0].binning].readout*5;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== End of Observation ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  hg->i_plan_max=i_plan;
}


void ircs_init_plan(typHOE *hg)
{
  gint i_plan=0, i_mode;

  calc_sun_plan(hg);

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Evening Calibration ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Flat
  for(i_mode=0;i_mode<NUM_IRCS_MODE;i_mode++){
    if(ircs_check_mode(hg, i_mode)){
      init_planpara(hg, i_plan);

      hg->plan[i_plan].type=PLAN_TYPE_FLAT;
      hg->plan[i_plan].repeat=IRCS_FLAT_REPEAT;
      hg->plan[i_plan].cal_mode=i_mode;
      hg->plan[i_plan].daytime=TRUE;
      hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
      hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

      i_plan++;
    }
  }
  
  // Comparison
  for(i_mode=0;i_mode<NUM_IRCS_MODE;i_mode++){
    switch(i_mode){
    case IRCS_MODE_GR:
    case IRCS_MODE_PS:
    case IRCS_MODE_EC:
      if(ircs_check_mode(hg, i_mode)){
	init_planpara(hg, i_plan);

	hg->plan[i_plan].type=PLAN_TYPE_COMP;
	hg->plan[i_plan].repeat=IRCS_COMP_REPEAT;
	hg->plan[i_plan].cal_mode=i_mode;
	hg->plan[i_plan].daytime=TRUE;
	hg->plan[i_plan].time=comp_time(hg->plan[i_plan], hg);
	hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

	i_plan++;
      }
      break;
    }
  }
  
  // Comment 
  {
    init_planpara(hg, i_plan);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Evening Focus ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNSET;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // SetAzEl (-90, 80)
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_SetAzEl;
    hg->plan[i_plan].setaz=-90;
    hg->plan[i_plan].setel=80;
    hg->plan[i_plan].az1=-90;
    hg->plan[i_plan].el1=80;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // FocusOBE
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FOCUS;
    hg->plan[i_plan].focus_mode=PLAN_FOCUS1;
    hg->plan[i_plan].time=get_focus_time(hg->plan[i_plan], hg->inst);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // LGS calibration
  if(ircs_check_lgs(hg)){
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FOCUS;
    hg->plan[i_plan].focus_mode=PLAN_FOCUS2;
    hg->plan[i_plan].time=get_focus_time(hg->plan[i_plan], hg->inst);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    i_plan++;
  }

  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("##### <<<<<<<<< INSERT YOUR TARGETS HERE >>>>>>>>> #####");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNRISE; 
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Morning Focus ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // FocusOBE
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FOCUS;
    hg->plan[i_plan].focus_mode=PLAN_FOCUS1;
    hg->plan[i_plan].time=get_focus_time(hg->plan[i_plan], hg->inst);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Morning Calibration ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  
  // Comparison
  for(i_mode=0;i_mode<NUM_IRCS_MODE;i_mode++){
    switch(i_mode){
    case IRCS_MODE_GR:
    case IRCS_MODE_PS:
    case IRCS_MODE_EC:
      if(ircs_check_mode(hg, i_mode)){
	init_planpara(hg, i_plan);
	
	hg->plan[i_plan].type=PLAN_TYPE_COMP;
	hg->plan[i_plan].repeat=IRCS_COMP_REPEAT;
	hg->plan[i_plan].cal_mode=i_mode;
	hg->plan[i_plan].daytime=TRUE;
	hg->plan[i_plan].time=comp_time(hg->plan[i_plan], hg);
	hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

	i_plan++;
      }
      break;
    }
  }

  // Flat
  for(i_mode=0;i_mode<NUM_IRCS_MODE;i_mode++){
    if(ircs_check_mode(hg, i_mode)){
      init_planpara(hg, i_plan);

      hg->plan[i_plan].type=PLAN_TYPE_FLAT;
      hg->plan[i_plan].repeat=IRCS_FLAT_REPEAT;
      hg->plan[i_plan].cal_mode=i_mode;
      hg->plan[i_plan].daytime=TRUE;
      hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
      hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

      i_plan++;
    }
  }
  
  // Comment 
  {
    init_planpara(hg, i_plan);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== End of Observation ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  hg->i_plan_max=i_plan;
}


void hsc_init_plan(typHOE *hg)
{
  gint i_plan=0, i_mode;
  glong sod_set, sod_rise;
  gchar *tmp;
  gint flat_sec;

  calc_sun_plan(hg);

  sod_set=(glong)hg->sun.s_set.hours*3600+(glong)hg->sun.s_set.minutes*60;
  sod_rise=(glong)hg->sun.s_rise.hours*3600+(glong)hg->sun.s_rise.minutes*60;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== Evening start up ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Take BIAS x5 [LAUNCHER/HSC] and health check");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Take a 5 sec TEST exposure [LAUNCHER/HSC], check HSC shutter sounds");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Take a 30 sec DARK frame [LAUNCHER/HSC]");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Filter Change
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_SETUP;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].cmode=PLAN_CMODE_1ST;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=TIME_SETUP_FULL;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    tmp=get_txt_tod(sod_set+HSC_TIME_FLAT_START-HSC_TIME_FLAT_LAMP);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Turn ON Dome Flat Lamp until %s", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    tmp=get_txt_tod(sod_set+HSC_TIME_FLAT_START);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Start Dome Flat at  %s", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;
  
  // Dome Flat
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FLAT;
    hg->plan[i_plan].repeat=HSC_FLAT_REPEAT;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    flat_sec=hg->plan[i_plan].time;
  }

  i_plan++;

  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNSET;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);
    
    tmp=get_txt_tod(sod_set+HSC_TIME_FLAT_START);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("========== Dome Open (%s or later) ==========", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("##### <<<<<<<<< INSERT YOUR TARGETS HERE >>>>>>>>> #####");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comtype=PLAN_COMMENT_SUNRISE; 
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    tmp=get_txt_tod(sod_rise-HSC_TIME_FLAT_START);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("========== Dome Close (until %s) ==========", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    tmp=get_txt_tod(sod_rise-HSC_TIME_FLAT_START-flat_sec);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Turn ON Dome Flat Lamp for morning flat until %s", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;

  // Dome Flat
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_FLAT;
    hg->plan[i_plan].repeat=HSC_FLAT_REPEAT;
    hg->plan[i_plan].setup=0;
    hg->plan[i_plan].daytime=TRUE;
    hg->plan[i_plan].time=flat_time(hg->plan[i_plan], hg);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    tmp=get_txt_tod(sod_rise-HSC_TIME_FLAT_START);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Finish morning flat until %s", tmp);
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    g_free(tmp);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Take BIAS x5 [LAUNCHER/HSC] and health check");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup_printf("Take a 30 sec DARK frame [LAUNCHER/HSC]");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;

  // Comment 
  {
    init_planpara(hg, i_plan);

    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== HSC Shutdown ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  
  // Comment 
  {
    init_planpara(hg, i_plan);
    
    hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
    hg->plan[i_plan].comment=g_strdup("========== End of Observation ==========");
    hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);
  }

  i_plan++;
  hg->i_plan_max=i_plan;
}



void remake_txt(typHOE *hg, GtkTreeModel *model) 
{
  gint i_plan;
  GtkTreeIter iter; 
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg,hg->plan[i_plan]);

    tree_update_plan_item(hg, model, iter, i_plan);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}

struct ln_hrz_posn get_ohrz_sod(gdouble ra, gdouble dec,
				gdouble lat, gdouble lng,
				struct ln_zonedate zonedate,
				gdouble az_old){
  struct ln_lnlat_posn observer;
  struct ln_hrz_posn ohrz;
  struct ln_equ_posn oequ;
  gdouble JD_hst;

  observer.lat = lat;
  observer.lng = lng;
  
  JD_hst = ln_get_julian_local_date(&zonedate);
  
  oequ.ra=ra_to_deg(ra);
  oequ.dec=dec_to_deg(dec);
  
  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);

  if((ohrz.az-az_old)>180){
    if(ohrz.az-360>-270){
      ohrz.az-=360;
    }
  }
  else if((ohrz.az-az_old)<-180){
    if(ohrz.az+360<270){
      ohrz.az+=360;
    }
  }

  if(ohrz.az>270) ohrz.az-=360;
  if(ohrz.az<-270) ohrz.az+=360;

  return(ohrz);
}

void remake_tod(typHOE *hg, GtkTreeModel *model) 
{
  gint i_plan;
  glong sod, sod_start;
  GtkTreeIter iter; 
  struct ln_zonedate zonedate;
  gdouble JD_hst;
  struct ln_hrz_posn ohrz;
  gchar *tod_start, *tod_end, *tmp;
  glong total_exp=0;


  zonedate.years=hg->fr_year;
  zonedate.months=hg->fr_month;
  zonedate.days=hg->fr_day;
  zonedate.gmtoff=(long)(hg->obs_timezone*60);

  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
    

  if(hg->plan_start==PLAN_START_EVENING){
    sod=hg->sun.s_set.hours*60*60 + hg->sun.s_set.minutes*60
      + hg->plan_delay*60;
  }
  else{
    if(hg->plan_hour<10){
      sod=hg->plan_time*60+24*60*60;
    }
    else{
      sod=hg->plan_time*60;
    }
  }
 
  sod_start=sod;
  tod_start=get_txt_tod(sod_start);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=90.;
    hg->plan[i_plan].txt_az=NULL;
    hg->plan[i_plan].txt_el=NULL;

    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
      switch(hg->inst){
      case INST_HDS:
	total_exp+=hg->plan[i_plan].exp*hg->plan[i_plan].repeat;
	break;
	
      case INST_IRCS:
	total_exp+=(gint)(hg->plan[i_plan].dexp
			  *(gdouble)hg->plan[i_plan].shot
			  *(gdouble)hg->plan[i_plan].coadds
			  *(gdouble)hg->plan[i_plan].repeat);
	break;
	
      case INST_HSC:
	total_exp+=(gint)hg->plan[i_plan].dexp
	  *(hg->plan[i_plan].stop-hg->plan[i_plan].skip)
	  *(gdouble)hg->plan[i_plan].repeat;
	break;
      }
    }

    // Not Backup or DayTime
    if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
      if((hg->plan[i_plan].time+hg->plan[i_plan].stime)>0){

	// At beginning of the plan (az0, el0)
	hg->plan[i_plan].sod=sod;

	if((hg->plan[i_plan].type==PLAN_TYPE_OBJ)
	   || ((hg->inst==INST_HSC)
	       && (hg->plan[i_plan].type==PLAN_TYPE_FOCUS)
	       && (hg->plan[i_plan].focus_mode!=0))){
	  zonedate.hours=sod/60/60;
	  zonedate.minutes=(sod-zonedate.hours*60*60)/60;
	  zonedate.seconds=0;

	  switch(hg->plan[i_plan].type){
	  case PLAN_TYPE_OBJ:
	    ohrz=get_ohrz_sod(hg->obj[hg->plan[i_plan].obj_i].ra,
			      hg->obj[hg->plan[i_plan].obj_i].dec,
			      hg->obs_latitude, hg->obs_longitude,
			      zonedate,
			      (i_plan==0) ? -90 : hg->plan[i_plan-1].az1);
	    break;
	    
	  case PLAN_TYPE_FOCUS:
	    ohrz=get_ohrz_sod(hg->obj[hg->plan[i_plan].focus_mode-1].ra,
			      hg->obj[hg->plan[i_plan].focus_mode-1].dec,
			      hg->obs_latitude, hg->obs_longitude,
			      zonedate,
			      (i_plan==0) ? -90 : hg->plan[i_plan-1].az1);
	    break;
	  }

	  hg->plan[i_plan].az0=ohrz.az;
	  hg->plan[i_plan].el0=ohrz.alt;

	  switch(hg->inst){
	  case INST_HDS:
	    hg->plan[i_plan].time
	      =hds_obj_time(hg->plan[i_plan],
			    hg->oh_acq,
			    hg->binning[hg->setup[hg->plan[i_plan].setup].binning].readout);
	    switch(hg->plan[i_plan].omode){
	    case PLAN_OMODE_FULL:
	    case PLAN_OMODE_SET:
	      if(i_plan!=0){
		// !HERE
		hg->plan[i_plan].stime=
		  slewtime(hg->plan[i_plan-1].az1, hg->plan[i_plan-1].el1,
			   hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			   hg->vel_az, hg->vel_el);
	      }
	      else{
		hg->plan[i_plan].stime=
		  slewtime(-90, 90,hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			   hg->vel_az, hg->vel_el);
	      }
	      break;
	    }
	    break;
	  
	  case INST_IRCS:
	    hg->plan[i_plan].time=ircs_obj_time(hg->plan[i_plan],
						hg->oh_acq,
						ircs_oh_ao(hg,
							   hg->plan[i_plan].aomode,
							   hg->plan[i_plan].obj_i));
	    switch(hg->plan[i_plan].omode){
	    case PLAN_OMODE_FULL:
	    case PLAN_OMODE_SET:
	      if(i_plan!=0){
		hg->plan[i_plan].stime=
		  slewtime(hg->plan[i_plan-1].az1, hg->plan[i_plan-1].el1,
			   hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			   hg->vel_az, hg->vel_el);
	      }
	      else{
		hg->plan[i_plan].stime=
		  slewtime(-90, 90,hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			   hg->vel_az, hg->vel_el);
	      }
	      break;
	    }
	    break;

	  case INST_HSC:
	    switch(hg->plan[i_plan].type){
	    case PLAN_TYPE_OBJ:
	      hg->plan[i_plan].time=hsc_obj_time(hg->plan[i_plan],
						 hg->oh_acq);
	      break;
	      
	    case PLAN_TYPE_FOCUS:
	      hg->plan[i_plan].time=HSC_TIME_FOCUS;
	      break;
	    }
	      
	    hg->plan[i_plan].stime=
	      slewtime(hg->plan[i_plan-1].az1, hg->plan[i_plan-1].el1,
		       hg->plan[i_plan].az0,hg->plan[i_plan].el0,
		       hg->vel_az, hg->vel_el);
	    break;
	  }

	  hg->plan[i_plan].moon=calc_typPlanMoon(hg, hg->plan[i_plan].sod,
						 hg->plan[i_plan].az0,
						 hg->plan[i_plan].el0);
	}
	else if(hg->plan[i_plan].type==PLAN_TYPE_SetAzEl){
	  if(i_plan!=0){
	    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
	    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
	  }
	  else{
	    hg->plan[i_plan].az0=-90.;
	    hg->plan[i_plan].el0=+90.;
	  }
	  hg->plan[i_plan].az1=hg->plan[i_plan].setaz;
	  hg->plan[i_plan].el1=hg->plan[i_plan].setel;
	  hg->plan[i_plan].time=0;
	  hg->plan[i_plan].stime=
	    slewtime(hg->plan[i_plan].az0,hg->plan[i_plan].el0,
		     hg->plan[i_plan].az1,hg->plan[i_plan].el1,
		     hg->vel_az, hg->vel_el);
	}
	else{
	  if(i_plan!=0){
	    hg->plan[i_plan].az0=hg->plan[i_plan-1].az0;
	    hg->plan[i_plan].el0=hg->plan[i_plan-1].el0;
	  }
	  else{
	    hg->plan[i_plan].az0=-90.;
	    hg->plan[i_plan].el0=+90.;
	    hg->plan[i_plan].az1=-90.;
	    hg->plan[i_plan].el1=+90.;
	  }
	}
	

	// At end of the plan (az1, el1)
	sod+=(glong)(hg->plan[i_plan].time+hg->plan[i_plan].stime);

	if((hg->plan[i_plan].type==PLAN_TYPE_OBJ)
	   || ((hg->inst==INST_HSC)
	       && (hg->plan[i_plan].type==PLAN_TYPE_FOCUS)
	       && (hg->plan[i_plan].focus_mode!=0))){
	  zonedate.hours=sod/60/60;
	  zonedate.minutes=(sod-zonedate.hours*60*60)/60;
	  zonedate.seconds=0;

	  switch(hg->plan[i_plan].type){
	  case PLAN_TYPE_OBJ:
	    ohrz=get_ohrz_sod(hg->obj[hg->plan[i_plan].obj_i].ra,
			      hg->obj[hg->plan[i_plan].obj_i].dec,
			      hg->obs_latitude, hg->obs_longitude,
			      zonedate,
			      hg->plan[i_plan].az0);
	    break;
	    
	  case PLAN_TYPE_FOCUS:
	    ohrz=get_ohrz_sod(hg->obj[hg->plan[i_plan].focus_mode-1].ra,
			      hg->obj[hg->plan[i_plan].focus_mode-1].dec,
			      hg->obs_latitude, hg->obs_longitude,
			      zonedate,
			      hg->plan[i_plan].az0);
	    break;
	  }

	  hg->plan[i_plan].az1=ohrz.az;
	  hg->plan[i_plan].el1=ohrz.alt;

	  if(hg->plan[i_plan].txt_az) g_free(hg->plan[i_plan].txt_az);
	  if(hg->plan[i_plan].txt_el) g_free(hg->plan[i_plan].txt_el);
	  hg->plan[i_plan].txt_az
	    =g_strdup_printf("%+4.0lf --> %+4.0lf",
			     hg->plan[i_plan].az0,
			     hg->plan[i_plan].az1);
	  hg->plan[i_plan].txt_el
	    =g_strdup_printf("%2.0lf --> %2.0lf",
			     hg->plan[i_plan].el0,
			     hg->plan[i_plan].el1);
	}
	else if(hg->plan[i_plan].type==PLAN_TYPE_SetAzEl){
	  if(i_plan!=0){
	    hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
	    hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
	  }
	  else{
	    hg->plan[i_plan].az0=-90.;
	    hg->plan[i_plan].el0=90.;
	  }
	  hg->plan[i_plan].az1=hg->plan[i_plan].setaz;
	  hg->plan[i_plan].el1=hg->plan[i_plan].setel;
	  hg->plan[i_plan].txt_az
	    =g_strdup_printf("%+4.0lf --> %+4.0lf",
			     hg->plan[i_plan].az0,
			     hg->plan[i_plan].az1);
	  hg->plan[i_plan].txt_el
	    =g_strdup_printf("%2.0lf --> %2.0lf",
			     hg->plan[i_plan].el0,
			     hg->plan[i_plan].el1);
	}
	else{
	  if(i_plan!=0){
	    hg->plan[i_plan].az0=hg->plan[i_plan-1].az0;
	    hg->plan[i_plan].el0=hg->plan[i_plan-1].el0;
	    hg->plan[i_plan].az1=hg->plan[i_plan-1].az1;
	    hg->plan[i_plan].el1=hg->plan[i_plan-1].el1;
	  }
	  else{
	    hg->plan[i_plan].az0=-90.;
	    hg->plan[i_plan].el0=+90.;
	    hg->plan[i_plan].az1=-90.;
	    hg->plan[i_plan].el1=+90.;
	  }
	  if(hg->plan[i_plan].txt_az) g_free(hg->plan[i_plan].txt_az);
	  if(hg->plan[i_plan].txt_el) g_free(hg->plan[i_plan].txt_el);
	}

      }
      else{
	hg->plan[i_plan].sod=0;
      }
    }
    else if(hg->plan[i_plan].type==PLAN_TYPE_SetAzEl){
      if(i_plan!=0){
	hg->plan[i_plan].az0=hg->plan[i_plan-1].az1;
	hg->plan[i_plan].el0=hg->plan[i_plan-1].el1;
      }
      else{
	hg->plan[i_plan].az0=-90.;
	hg->plan[i_plan].el0=90.;
      }
      hg->plan[i_plan].az1=hg->plan[i_plan].setaz;
      hg->plan[i_plan].el1=hg->plan[i_plan].setel;
      hg->plan[i_plan].txt_az
	=g_strdup_printf("%+4.0lf --> %+4.0lf",
			 hg->plan[i_plan].az0,
			 hg->plan[i_plan].az1);
      hg->plan[i_plan].txt_el
	=g_strdup_printf("%2.0lf --> %2.0lf",
			 hg->plan[i_plan].el0,
			 hg->plan[i_plan].el1);
    }
    else{
      hg->plan[i_plan].sod=0;
    }
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_PLAN_TOD, hg->plan[i_plan].sod, -1);
    if(hg->plan[i_plan].txt_az)
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_PLAN_TXT_AZ, hg->plan[i_plan].txt_az, -1);
    if(hg->plan[i_plan].txt_az)
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_PLAN_TXT_EL, hg->plan[i_plan].txt_el, -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;    
  }
  
  tod_end=get_txt_tod(sod);
  if(sod-sod_start!=0){
    tmp=g_strdup_printf("%s -- %s (%.2lf hrs),  Open Shutter Rate = %.1lf%%",
			tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.,
			(gdouble)total_exp/(gdouble)(sod-sod_start)*100);
  }
  else{
    tmp=g_strdup_printf("%s -- %s (%.2lf hrs)",
			tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.);
  }
  gtk_label_set_text(GTK_LABEL(hg->label_stat_plan),tmp);
  if(tod_start) g_free(tod_start);
  if(tod_end) g_free(tod_end);
  if(tmp) g_free(tmp);
}


glong get_start_sod(typHOE *hg){
  glong sod;
  
  if(hg->plan_start==PLAN_START_EVENING){
    sod=hg->sun.s_set.hours*60*60 + hg->sun.s_set.minutes*60
      + SUNSET_OFFSET*60;
  }
  else{
    if(hg->plan_hour<10){
      sod=hg->plan_time*60+24*60*60;
    }
    else{
      sod=hg->plan_time*60;
    }
  }

  return(sod);
}

void remake_sod(typHOE *hg) 
{
  gint i_plan;
  glong sod, sod_start;
  gchar *tod_start, *tod_end, *tmp;
  glong total_exp=0;

  sod=get_start_sod(hg);
  
  sod_start=sod;
  tod_start=get_txt_tod(sod_start);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
      switch(hg->inst){
      case INST_HDS:
	total_exp+=hg->plan[i_plan].exp*hg->plan[i_plan].repeat;
	break;
	
      case INST_IRCS:
	total_exp+=(gint)(hg->plan[i_plan].dexp
			  *(gdouble)hg->plan[i_plan].shot
			  *(gdouble)hg->plan[i_plan].coadds
			  *(gdouble)hg->plan[i_plan].repeat);
	break;
	
      case INST_HSC:
	total_exp+=(gint)hg->plan[i_plan].dexp
	  *(hg->plan[i_plan].stop-hg->plan[i_plan].skip)
	  *(gdouble)hg->plan[i_plan].repeat;
	break;
      }
    }

    if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
      if(hg->plan[i_plan].time>0){
	hg->plan[i_plan].sod=sod;
	sod+=(glong)(hg->plan[i_plan].time+hg->plan[i_plan].stime);
      }
      else{
	hg->plan[i_plan].sod=0;
      }
    }
    else{
      hg->plan[i_plan].sod=0;
    }
  }

  tod_end=get_txt_tod(sod);
  if(sod-sod_start!=0){
    tmp=g_strdup_printf("%s -- %s (%.2lf hrs),  Open Shutter Rate = %.1lf%%",
			tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.,
			(gdouble)total_exp/(gdouble)(sod-sod_start)*100);
  }
  else{
    tmp=g_strdup_printf("%s -- %s (%.2lf hrs)",
			tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.);
  }
  gtk_label_set_text(GTK_LABEL(hg->label_stat_plan),tmp);
  if(tod_start) g_free(tod_start);
  if(tod_end) g_free(tod_end);
  if(tmp) g_free(tmp);
}

gchar *get_txt_tod(glong sod){
  gint hour,min;
  gchar *ret;
  
  hour=sod/60/60;
  min=(sod-hour*60*60)/60;
  
  if(hour>=24) hour-=24;

  ret=g_strdup_printf("%d:%02d",hour,min);

  return(ret);
}


void plot2_plan (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gboolean valid=FALSE;
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    hg->plot_i_plan=i;
    

    gtk_tree_path_free (path);

    if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
      hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
      //hg->plot_target=PLOT_PLAN;
      do_plot(widget,(gpointer)hg);
      valid=TRUE;
    }
  }

  if(!valid){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please select an \"Object\" line in your plan.",
		  NULL);
  }
}

void skymon2_plan (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gboolean valid=FALSE;
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    hg->plot_i_plan=i;

    gtk_tree_path_free (path);

    if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
      hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
    }
    else if ((hg->inst==INST_HSC) &&
	     (hg->plan[hg->plot_i_plan].type==PLAN_TYPE_FOCUS)){
      if(hg->plan[hg->plot_i_plan].focus_mode!=0){
	hg->plot_i=hg->plan[hg->plot_i_plan].focus_mode-1;
      }
    }

    do_skymon(widget,(gpointer)hg);
    refresh_plan_plot(hg);
    valid=TRUE;
  }

  if(!valid){
    popup_message(hg->plan_main,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please select a line in your plan.",
		  NULL);
  }
}


static void
focus_plan_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];
    
    hg->plot_i_plan=i;
    if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
      hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
    }
    else if((hg->inst==INST_HSC) &&
	    (hg->plan[hg->plot_i_plan].type==PLAN_TYPE_FOCUS)){
      if(hg->plan[hg->plot_i_plan].focus_mode!=0){
	hg->plot_i=hg->plan[hg->plot_i_plan].focus_mode-1;
      }
    }
    
    gtk_tree_path_free (path);
  }

  refresh_plan_plot(hg);
}

void refresh_plan_plot(typHOE *hg){
  gint i_plan;
  glong end_sod;
  struct ln_zonedate zonedate;
  
  if(hg->plan[hg->plot_i_plan].sod>0){
    hg->skymon_year=hg->fr_year;
    hg->skymon_month=hg->fr_month;
    hg->skymon_day=hg->fr_day;
    hg->skymon_hour=hg->plan[hg->plot_i_plan].sod/60./60.;
    hg->skymon_min=(hg->plan[hg->plot_i_plan].sod-hg->skymon_hour*60.*60.)/60.;

    if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
      zonedate.years=hg->fr_year;
      zonedate.months=hg->fr_month;
      zonedate.days=hg->fr_day;
      zonedate.hours=0;
      zonedate.minutes=0;
      zonedate.seconds=0;
      zonedate.gmtoff=(long)(hg->obs_timezone*60);

      hg->plan_jd1 = ln_get_julian_local_date(&zonedate)
	+ (gdouble)hg->plan[hg->plot_i_plan].sod /24./60./60.;
      hg->plan_jd2 = hg->plan_jd1
	+ (gdouble)(hg->plan[hg->plot_i_plan].time+hg->plan[hg->plot_i_plan].stime) /24./60./60.; 
      hg->plan_trace=hg->plan[hg->plot_i_plan].obj_i;
    }
    else{
      hg->plan_jd1=-1;
      hg->plan_jd2=-1;
      hg->plan_trace=-1;
    }
  }
  else{
    // Plot the end of previous target
    for(i_plan=hg->plot_i_plan-1;i_plan>=0;i_plan--){
      if(hg->plan[i_plan].sod>0){
	break;
      }
    }

    if(i_plan<0){
      end_sod=get_start_sod(hg);
      hg->plot_i=-1;
      
      hg->plan_jd1=-1;
      hg->plan_jd2=-1;
      hg->plan_trace=-1;
    }
    else{
      end_sod=hg->plan[i_plan].sod+hg->plan[i_plan].time+hg->plan[i_plan].stime;
      hg->plot_i=hg->plan[i_plan].obj_i;
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	zonedate.years=hg->fr_year;
	zonedate.months=hg->fr_month;
	zonedate.days=hg->fr_day;
	zonedate.hours=0;
	zonedate.minutes=0;
	zonedate.seconds=0;
	zonedate.gmtoff=(long)(hg->obs_timezone*60);

	hg->plan_jd1 = ln_get_julian_local_date(&zonedate)
	  + (gdouble)hg->plan[i_plan].sod /24./60./60.;
	hg->plan_jd2 = hg->plan_jd1
	  + (gdouble)(hg->plan[i_plan].time+hg->plan[i_plan].stime) /24./60./60.; 
	hg->plan_trace=hg->plan[i_plan].obj_i;
      }
      else{
	hg->plan_jd1=-1;
	hg->plan_jd2=-1;
	hg->plan_trace=-1;
      }
    }
    
    hg->skymon_year=hg->fr_year;
    hg->skymon_month=hg->fr_month;
    hg->skymon_day=hg->fr_day;
    hg->skymon_hour=end_sod/60./60.;
    hg->skymon_min=(end_sod-hg->skymon_hour*60.*60.)/60.;
  }

  refresh_plot(NULL, (gpointer)hg);

  if(flagSkymon){
    switch(hg->skymon_mode){
    case SKYMON_PLAN_OBJ:
    case SKYMON_PLAN_TIME:
      if((hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ)&&
	 (!hg->plan[hg->plot_i_plan].backup)){
	calc_moon_skymon(hg);
      }

      draw_skymon_cairo(hg->skymon_dw,hg);
      gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
      break;

    case SKYMON_SET:
      skymon_set_and_draw(NULL, (gpointer)hg);
      break;
    }
  }
}

static void view_onRowActivated (GtkTreeView        *treeview,
				 GtkTreePath        *path,
				 GtkTreeViewColumn  *col,
				 gpointer            data){
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gint i_plan;
  
  model = gtk_tree_view_get_model(treeview);
  
  if (gtk_tree_model_get_iter(model, &iter, path)){
    
    i_plan = gtk_tree_path_get_indices (path)[0];
    
    switch(hg->inst){
    case INST_HDS:
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	do_edit_comment(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FLAT:
	do_edit_flat(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_COMP:
	do_edit_comp(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_BIAS:
	do_edit_bias(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_I2:
	do_edit_i2(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FOCUS:
	do_edit_focus(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_SETUP:
	do_edit_setup(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_SetAzEl:
	do_edit_setazel(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_OBJ:
	hds_do_edit_obj(hg,model,iter,i_plan);
	break;
      }
      break;

    case INST_IRCS:
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	do_edit_comment(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FLAT:
	do_edit_flat(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_COMP:
	do_edit_comp(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FOCUS:
	do_edit_focus(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_SetAzEl:
	do_edit_setazel(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_OBJ:
	ircs_do_edit_obj(hg,model,iter,i_plan);
	break;
      }
      break;

    case INST_HSC:
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	do_edit_comment(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FLAT:
	do_edit_flat(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_FOCUS:
	do_edit_focus(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_SetAzEl:
	do_edit_setazel(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_SETUP:
	do_edit_setup(hg,model,iter,i_plan);
	break;
      case PLAN_TYPE_OBJ:
	hsc_do_edit_obj(hg,model,iter,i_plan);
	break;
      }
      break;
    }
    
    remake_tod(hg, model);
    tree_update_plan_item(hg, model, iter, i_plan);
    //gtk_tree_row_reference_new(model, path);    
  }
}



static void do_edit_comment (typHOE *hg, 
			     GtkTreeModel *model, 
			     GtkTreeIter iter, 
			     gint i_plan)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Comment)",
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

  sprintf(tmp,"[Plan #%d]  Comment :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Text",
		       1, PLAN_COMMENT_TEXT, -1);
    if(hg->plan[i_plan].comtype==PLAN_COMMENT_TEXT) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Sunset",
		       1, PLAN_COMMENT_SUNSET, -1);
    if(hg->plan[i_plan].comtype==PLAN_COMMENT_SUNSET) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Sunrise",
		       1, PLAN_COMMENT_SUNRISE, -1);
    if(hg->plan[i_plan].comtype==PLAN_COMMENT_SUNRISE) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.comtype);
  }
  

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE, FALSE, 0);
  if(hg->plan[i_plan].comment){
    gtk_entry_set_text(GTK_ENTRY(entry),
		       hg->plan[i_plan].comment);
  }
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),80);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &tmp_plan.comment);
  
  label = gtk_label_new ("  Time[min]");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  tmp_time=hg->plan[i_plan].time/60.;
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].time/60.,
					    0, 60, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_time);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  
  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    hg->plan[i_plan]=tmp_plan;
    hg->plan[i_plan].time=tmp_time*60;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}


static void do_edit_flat (typHOE *hg, 
		      GtkTreeModel *model, 
		      GtkTreeIter iter, 
		      gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;
  gchar *err_str=NULL;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Flat)",
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

  sprintf(tmp,"[Plan #%d]  Flat :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  

  switch(hg->inst){
  case INST_IRCS:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gint i_mode;
      gchar *tmp_txt=NULL;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_mode=-1;i_mode<NUM_IRCS_MODE;i_mode++){
	if(i_mode==-1){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, "for a Specified Setup =",
			     1, i_mode, -1);
	  if(tmp_plan.cal_mode==i_mode) iter_set=iter;
	}
	else{
	  switch(i_mode){
	  case IRCS_MODE_GR:
	  case IRCS_MODE_PS:
	  case IRCS_MODE_EC:
	    gtk_list_store_append(store, &iter);
	    tmp_txt=g_strdup_printf("for all %s Setups", ircs_mode_name[i_mode]);
	    gtk_list_store_set(store, &iter, 0, tmp_txt,
			       1, i_mode, -1);
	    if(tmp_plan.cal_mode==i_mode) iter_set=iter;
	    g_free(tmp_txt);
	    break;
	  }
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
			 &tmp_plan.cal_mode);
    }
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gchar *tmp_txt=NULL;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );

      for(i_use=0;i_use<hg->ircs_i_max;i_use++){
	tmp_txt=g_strdup_printf("%02d : %s",
				i_use+1,
				hg->ircs_set[i_use].txt);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp_txt,
			   1, i_use, 
			   2, &color_black,
			   3, &col_ircs_setup[i_use],
			   -1);
	g_free(tmp_txt);
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);

      if(tmp_plan.cal_mode!=-1){
	tmp_plan.setup=hg->plan_tmp_setup;
      }
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }
  }

  label = gtk_label_new ("  x");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].repeat,
					    1, 50, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  switch(hg->inst){
  case INST_HDS:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_use=0;i_use<MAX_USESETUP;i_use++){
	if(hg->setup[i_use].use){
	  if(hg->setup[i_use].setup<0){
	    sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
		    i_use+1,
		    -hg->setup[i_use].setup,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{
	    sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
		    i_use+1,
		    HDS_setups[hg->setup[i_use].setup].initial,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_use, -1);
	}
	else if (hg->plan[i_plan].setup==i_use){
	  tmp_plan.setup=0;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }
    
    check = gtk_check_button_new_with_label("Override Default Slit");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->plan[i_plan].slit_or);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_plan.slit_or);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_width/500.,
					      0.2, 4.0, 
					      0.05,0.10,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_width);
    spinner =  gtk_spin_button_new (adj, 0, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    
    label = gtk_label_new ("/");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_length/500.,
					      2.0, 60.0, 
					      0.1,1.0,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_length);
    spinner =  gtk_spin_button_new (adj, 0, 1);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    break;
  }

  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    switch(hg->inst){
    case INST_IRCS:
      if(tmp_plan.cal_mode!=-1){
	if(!ircs_check_mode(hg, tmp_plan.cal_mode)){
	  err_str=g_strdup_printf("Error: There are no \"%s\" modes in your setups.",
	  			  ircs_mode_name[tmp_plan.cal_mode]);
	}
	tmp_plan.setup=-1;
      }
	

      if(err_str){
	popup_message(hg->plan_main,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      err_str,
		      NULL);
	g_free(err_str);
	
	flagPlanEditDialog=FALSE;
	return;
      }
      break;
    }

    tmp_plan.time=flat_time(tmp_plan, hg);

    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}




static void do_edit_comp (typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;
  gchar *err_str=NULL;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Comparison)",
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

  sprintf(tmp,"[Plan #%d]  Comparison :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  switch(hg->inst){
  case INST_IRCS:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gint i_mode;
      gchar *tmp_txt=NULL;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_mode=-1;i_mode<NUM_IRCS_MODE;i_mode++){
	if(i_mode==-1){
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, "for a Specified Setup =",
			     1, i_mode, -1);
	  if(tmp_plan.cal_mode==i_mode) iter_set=iter;
	}
	else{
	  switch(i_mode){
	  case IRCS_MODE_GR:
	  case IRCS_MODE_PS:
	  case IRCS_MODE_EC:
	    gtk_list_store_append(store, &iter);
	    tmp_txt=g_strdup_printf("for all %s Setups", ircs_mode_name[i_mode]);
	    gtk_list_store_set(store, &iter, 0, tmp_txt,
			       1, i_mode, -1);
	    if(tmp_plan.cal_mode==i_mode) iter_set=iter;
	    g_free(tmp_txt);
	    break;
	  }
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
			 &tmp_plan.cal_mode);
    }
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gchar *tmp_txt=NULL;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );

      for(i_use=0;i_use<hg->ircs_i_max;i_use++){
	tmp_txt=g_strdup_printf("%02d : %s",
				i_use+1,
				hg->ircs_set[i_use].txt);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp_txt,
			   1, i_use, 
			   2, &color_black,
			   3, &col_ircs_setup[i_use],
			   -1);
	g_free(tmp_txt);
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);

      if(tmp_plan.cal_mode!=-1){
	tmp_plan.setup=hg->plan_tmp_setup;
      }
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }

    label = gtk_label_new ("  x");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.repeat,
					      1, 50, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &tmp_plan.repeat);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    break;

  case INST_HDS:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_use=0;i_use<MAX_USESETUP;i_use++){
	if(hg->setup[i_use].use){
	  if(hg->setup[i_use].setup<0){
	    sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
		    i_use+1,-hg->setup[i_use].setup,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{
	    sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
		    i_use+1,HDS_setups[hg->setup[i_use].setup].initial,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_use, -1);
	  
	}
	else if (hg->plan[i_plan].setup==i_use){
	  tmp_plan.setup=0;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }

    check = gtk_check_button_new_with_label("Override Default Slit");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->plan[i_plan].slit_or);
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_plan.slit_or);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_width/500.,
					      0.2, 4.0, 
					      0.05,0.10,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_width);
    spinner =  gtk_spin_button_new (adj, 0, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

    label = gtk_label_new ("/");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_length/500.,
					      2.0, 60.0, 
					      0.1,1.0,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_length);
    spinner =  gtk_spin_button_new (adj, 0, 1);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    break;
  }


  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    switch(hg->inst){
    case INST_IRCS:
      if(tmp_plan.cal_mode!=-1){
	if(!ircs_check_mode(hg, tmp_plan.cal_mode)){
	  err_str=g_strdup_printf("Error: There are no \"%s\" modes in your setups.",
	  			  ircs_mode_name[tmp_plan.cal_mode]);
	}
	tmp_plan.setup=-1;
      }
      else{
	if(!ircs_check_spec(hg, hg->ircs_set[tmp_plan.setup].mode)){
	  err_str=g_strdup_printf("Error: Comparison is not necessary for \"%s\" mode.",
				  ircs_mode_name[hg->ircs_set[tmp_plan.setup].mode]);
	}
      }

      if(err_str){
	popup_message(hg->plan_main,
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      err_str,
		      NULL);
	g_free(err_str);
	
	flagPlanEditDialog=FALSE;
	return;
      }
      break;
    }

    tmp_plan.time=comp_time(tmp_plan, hg);
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}




static void do_edit_bias (typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Bias)",
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

  sprintf(tmp,"[Plan #%d]  Bias :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  

  label = gtk_label_new ("  x");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].repeat,
					    1, 30, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);


  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : %dx%dbin",
		  i_use+1,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	else{
	  sprintf(tmp,"Setup-%d : %dx%dbin",
		  i_use+1,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	
      }
      else if (hg->plan[i_plan].setup==i_use){
	tmp_plan.setup=0;
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.setup);
  }
  

  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);
   
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.time=hg->binning[hg->setup[tmp_plan.setup].binning].readout*tmp_plan.repeat;
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}


static void do_edit_setazel (typHOE *hg, 
			     GtkTreeModel *model, 
			     GtkTreeIter iter, 
			     gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (SetAzEl)",
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

  sprintf(tmp,"[Plan #%d]  SetAzEl :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  

  label = gtk_label_new ("Az ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].setaz,
					    -269, 269, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.setaz);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  label = gtk_label_new ("    El ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].setel,
					    15, 90, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.setel);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);


  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);
   
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.az1=tmp_plan.setaz;
    tmp_plan.el1=tmp_plan.setel;
    tmp_plan.time=0;
    tmp_plan.stime=slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,
			    hg->plan[i_plan].setaz,hg->plan[i_plan].setel,
			    hg->vel_az, hg->vel_el);
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}




static void do_edit_i2 (typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (I2 Cell)",
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

  sprintf(tmp,"[Plan #%d]  I2 Cell :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "In",
		       1, PLAN_I2_IN, -1);
    if(hg->plan[i_plan].i2_pos==PLAN_I2_IN) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Out",
		       1, PLAN_I2_OUT, -1);
    if(hg->plan[i_plan].i2_pos==PLAN_I2_OUT) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.i2_pos);
  }

  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);
   
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.time=TIME_I2;
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}



static void do_edit_focus (typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;
  gint i_list;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Focusing)",
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

  sprintf(tmp,"[Plan #%d]  Focusing :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  if(hg->inst==INST_HSC){
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gchar *tmp_txt=NULL;
    
    store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
			       GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
			       GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
			       );
    
    for(i_use=0;i_use<hg->hsc_i_max;i_use++){
      tmp_txt=g_strdup_printf("Setup-%02d : %s",
			      i_use+1,
			      hsc_filter[hg->hsc_set[i_use].filter].name);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp_txt,
			 1, i_use, 
			 2, &color_black,
			 3, &col_hsc_setup[i_use],
			 -1);
      g_free(tmp_txt);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				    "text",0,
#ifdef USE_GTK3				    
				    "foreground-rgba", 2,
				    "background-rgba", 3,
#else
				    "foreground-gdk", 2,
				    "background-gdk", 3,
#endif
				    NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),
			     tmp_plan.setup);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.setup);
  }
  
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    switch(hg->inst){
    case INST_HDS:
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "FocusSV",
			 1, PLAN_FOCUS1, -1);
      if(hg->plan[i_plan].focus_mode==PLAN_FOCUS1) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "FocusAG",
			 1, PLAN_FOCUS2, -1);
      if(hg->plan[i_plan].focus_mode==PLAN_FOCUS2) iter_set=iter;
      break;

    case INST_IRCS:
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "FocusOBE",
			 1, PLAN_FOCUS1, -1);
      if(hg->plan[i_plan].focus_mode==PLAN_FOCUS1) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "LGS Calibration",
			 1, PLAN_FOCUS2, -1);
      if(hg->plan[i_plan].focus_mode==PLAN_FOCUS2) iter_set=iter;
      break;

    case INST_HSC:
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "(current position)",
			 1, 0, -1);
      if(hg->plan[i_plan].focus_mode==0) iter_set=iter;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	gtk_list_store_append(store, &iter);
	sprintf(tmp,"@ %03d:  %s",i_list+1,hg->obj[i_list].name);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_list+1, -1);
	if(hg->plan[i_plan].focus_mode==i_list+1) iter_set=iter;
      }
      break;
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
		       &tmp_plan.focus_mode);
  }

  if(hg->inst==INST_HSC){
    label = gtk_label_new ("  Center Z");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].focus_z,
					      3.00, 4.00, 0.05, 0.05, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &tmp_plan.focus_z);
    spinner =  gtk_spin_button_new (adj, 2, 2);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

    check = gtk_check_button_new_with_label("Override Default PA");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->plan[i_plan].pa_or);
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_plan.pa_or);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].pa,
					      -360.0, 360.0, 0.1, 0.1, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &tmp_plan.pa);
    spinner =  gtk_spin_button_new (adj, 1, 1);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  }
  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.time=get_focus_time(tmp_plan, hg->inst);
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}


static void do_edit_setup (typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_use;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Setup)",
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

  switch(hg->inst){
  case INST_HDS:
    sprintf(tmp,"[Plan #%d]  Setup :", i_plan);
    break;

  case INST_HSC:
    sprintf(tmp,"[Plan #%d]  Filter Exchange :", i_plan);
    break;
  }
    
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  switch(hg->inst){
  case INST_HDS:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_use=0;i_use<MAX_USESETUP;i_use++){
	if(hg->setup[i_use].use){
	  if(hg->setup[i_use].setup<0){
	    sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
		    i_use+1,-hg->setup[i_use].setup,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{
	    sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
		    i_use+1,
		    HDS_setups[hg->setup[i_use].setup].initial,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	}
	  
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter, 0, tmp,
			     1, i_use, -1);
	}
	else if (hg->plan[i_plan].setup==i_use){
	  tmp_plan.setup=0;
	}
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Full Change",
			 1, PLAN_CMODE_FULL, -1);
      if(hg->plan[i_plan].cmode==PLAN_CMODE_FULL) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Cross Scan",
			 1, PLAN_CMODE_EASY, -1);
      if(hg->plan[i_plan].cmode==PLAN_CMODE_EASY) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Slit Only",
			 1, PLAN_CMODE_SLIT, -1);
      if(hg->plan[i_plan].cmode==PLAN_CMODE_SLIT) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "1st Change",
			 1, PLAN_CMODE_1ST, -1);
      if(hg->plan[i_plan].cmode==PLAN_CMODE_1ST) iter_set=iter;
      
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.cmode);
    }
    
    
    check = gtk_check_button_new_with_label("Override Default Slit");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->plan[i_plan].slit_or);
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_plan.slit_or);


    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_width/500.,
					      0.2, 4.0, 
					      0.05,0.10,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_width);
    spinner =  gtk_spin_button_new (adj, 0, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

    label = gtk_label_new ("/");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_length/500.,
					      2.0, 60.0, 
					      0.1,1.0,0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_slit,
		       &tmp_plan.slit_length);
    spinner =  gtk_spin_button_new (adj, 0, 1);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			      TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

    break;

  case INST_HSC:
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gchar *tmp_txt=NULL;
      
      store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
				 GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
				 );
      
      for(i_use=0;i_use<hg->hsc_i_max;i_use++){
	tmp_txt=g_strdup_printf("Setup-%02d : %s",
				i_use+1,
				hsc_filter[hg->hsc_set[i_use].filter].name);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp_txt,
			   1, i_use, 
			   2, &color_black,
			   3, &col_hsc_setup[i_use],
			 -1);
	g_free(tmp_txt);
      }
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				      "text",0,
#ifdef USE_GTK3				    
				      "foreground-rgba", 2,
				      "background-rgba", 3,
#else
				      "foreground-gdk", 2,
				      "background-gdk", 3,
#endif
				      NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),
			       tmp_plan.setup);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_plan.setup);
    }
    break;
  }
    
  check = gtk_check_button_new_with_label("Daytime");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].daytime);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.daytime);
  

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    if(!tmp_plan.slit_or){
      tmp_plan.slit_width=hg->setup[tmp_plan.setup].slit_width;
      tmp_plan.slit_length=hg->setup[tmp_plan.setup].slit_length;
    }

    switch(hg->inst){
    case INST_HDS:
      switch(tmp_plan.cmode){
      case PLAN_CMODE_FULL:
      case PLAN_CMODE_1ST:
	tmp_plan.time=TIME_SETUP_FULL;
	break;
      case PLAN_CMODE_EASY:
	tmp_plan.time=TIME_SETUP_EASY;
	break;
      case PLAN_CMODE_SLIT:
	tmp_plan.time=TIME_SETUP_SLIT;
	break;
      }
      break;

    case INST_HSC:
      if(tmp_plan.daytime){
	tmp_plan.time=HSC_TIME_FILTER_DAYTIME;
      }
      else{
	tmp_plan.time=HSC_TIME_FILTER;
      }
      break;
    }
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}



static void hds_do_edit_obj (typHOE *hg, 
			     GtkTreeModel *model, 
			     GtkTreeIter iter, 
			     gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_list,i_use;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Object)",
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

  sprintf(tmp,"[Plan #%d]  Object :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_list=0;i_list<hg->i_max;i_list++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hg->obj[i_list].name,
			 1, i_list, -1);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan[i_plan].obj_i);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.obj_i);
  }
   
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SetupField & GetObject",
		       1, PLAN_OMODE_FULL, -1);
    if(hg->plan[i_plan].omode==PLAN_OMODE_FULL) iter_set=iter;
      
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SetupField Only",
		       1, PLAN_OMODE_SET, -1);
    if(hg->plan[i_plan].omode==PLAN_OMODE_SET) iter_set=iter;
      
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "GetObject Only",
		       1, PLAN_OMODE_GET, -1);
    if(hg->plan[i_plan].omode==PLAN_OMODE_GET) iter_set=iter;
      

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
      
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.omode);
  }

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.exp,
					    1, 9999, 1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.exp);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  label = gtk_label_new ("[s]x");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].repeat,
					    1, 50, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
		  i_use+1,-hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
		  i_use+1,HDS_setups[hg->setup[i_use].setup].initial,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	
      }
      else if (hg->plan[i_plan].setup==i_use){
	tmp_plan.setup=0;
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),tmp_plan.setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.setup);
  }

  check = gtk_check_button_new_with_label("Override Default Slit");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].slit_or);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.slit_or);


  adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_width/500.,
					    0.2, 4.0, 
					    0.05,0.10,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_slit,
		     &tmp_plan.slit_width);
  spinner =  gtk_spin_button_new (adj, 0, 3);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
  label = gtk_label_new ("/");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new((gdouble)hg->plan[i_plan].slit_length/500.,
					    2.0, 60.0, 
					    0.1,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_slit,
		     &tmp_plan.slit_length);
  spinner =  gtk_spin_button_new (adj, 0, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);


  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No Guide",
		       1, NO_GUIDE, -1);
    if(hg->plan[i_plan].guide==NO_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "AG Guide",
		       1, AG_GUIDE, -1);
    if(hg->plan[i_plan].guide==AG_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Guide",
		       1, SV_GUIDE, -1);
    if(hg->plan[i_plan].guide==SV_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Guide (Safe)",
		       1, SVSAFE_GUIDE, -1);
    if(hg->plan[i_plan].guide==SVSAFE_GUIDE) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,
		       FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				    renderer, "text",0,NULL);
    

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),
				  &iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.guide);
  }


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  label = gtk_label_new ("                   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE, TRUE, 0);

  check = gtk_check_button_new_with_label("Override Default PA");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].pa_or);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.pa_or);


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].pa,
					    -360.0, 360.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.pa);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  
  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Override Default SV Exp.");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].sv_or);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.sv_or);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].sv_exp,
					    100, 100000, 
					    100.0, 100.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.sv_exp);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Filter : None",
		       1, SV_FILTER_NONE, -1);
    if(hg->plan[i_plan].sv_fil==SV_FILTER_NONE) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Filter : R",
		       1, SV_FILTER_R, -1);
    if(hg->plan[i_plan].sv_fil==SV_FILTER_R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Filter : BP530",
		       1, SV_FILTER_BP530, -1);
    if(hg->plan[i_plan].sv_fil==SV_FILTER_BP530) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Filter : ND2",
		       1, SV_FILTER_ND2, -1);
    if(hg->plan[i_plan].sv_fil==SV_FILTER_ND2) iter_set=iter;

    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,
		       FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),
				    renderer, "text",0,NULL);
    

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),
				  &iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.sv_fil);
  }

  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  check = gtk_check_button_new_with_label("Back-Up Target?");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].backup);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.backup);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    

    tmp_plan.time=hds_obj_time(tmp_plan,
			       hg->oh_acq,
			       hg->binning[hg->setup[tmp_plan.setup].binning].readout);
    
    switch(tmp_plan.omode){
    case PLAN_OMODE_FULL:
    case PLAN_OMODE_SET:
      tmp_plan.stime=slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,
			      hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			      hg->vel_az, hg->vel_el);
      break;
      
    default:
      tmp_plan.stime=0;
      break;
    }
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}


static void ircs_do_edit_obj (typHOE *hg, 
			      GtkTreeModel *model, 
			      GtkTreeIter iter, 
			      gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gchar *tmp_txt;
  gint i_list,i_use, i_dith;
  gint setup0, dith0, osra0, osdec0, ssnum0;
  gdouble dexp0, dithw0, sssep0;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  {
    setup0=hg->plan_tmp_setup;
    dexp0=hg->plan_obj_dexp;
    dith0=hg->plan_dith;

    dithw0=hg->plan_dithw;
    osra0=hg->plan_osra;
    osdec0=hg->plan_osdec;
    sssep0=hg->plan_sssep;
    ssnum0=hg->plan_ssnum;

    hg->plan_tmp_setup=tmp_plan.setup;
    hg->plan_obj_dexp=tmp_plan.dexp;
    hg->plan_dith=tmp_plan.dith;

    hg->plan_obj_dexp=tmp_plan.dexp;
    hg->plan_dithw=tmp_plan.dithw;
    hg->plan_osra=tmp_plan.osra;
    hg->plan_osdec=tmp_plan.osdec;
    hg->plan_sssep=tmp_plan.sssep;
    hg->plan_ssnum=tmp_plan.ssnum;
  }
  
  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Object)",
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

  sprintf(tmp,"[Plan #%d]  Object :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);


  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_list=0;i_list<hg->i_max;i_list++){
      gtk_list_store_append(store, &iter);
      switch(hg->obj[i_list].aomode){
      case AOMODE_NO:
	tmp_txt=g_strdup_printf("%03d:  %s (w/o AO)",i_list+1,hg->obj[i_list].name);
	break;
      case AOMODE_NGS_S:
	tmp_txt=g_strdup_printf("%03d:  %s (NGS:self)",i_list+1,hg->obj[i_list].name);
	break;
      case AOMODE_NGS_O:
	tmp_txt=g_strdup_printf("%03d:  %s (NGS:offset)",i_list+1,hg->obj[i_list].name);
	break;
      case AOMODE_LGS_S:
	tmp_txt=g_strdup_printf("%03d:  %s (LGS:self)",i_list+1,hg->obj[i_list].name);
	break;
      case AOMODE_LGS_O:
	tmp_txt=g_strdup_printf("%03d:  %s (LGS:TTGS)",i_list+1,hg->obj[i_list].name);
	break;
      }
      gtk_list_store_set(store, &iter, 0, tmp_txt,
			 1, i_list, -1);
      g_free(tmp_txt);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo),renderer, "text",0,NULL);
	
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), hg->plan[i_plan].obj_i);
    gtk_widget_show(hg->plan_obj_combo);
    my_signal_connect (combo, "changed", cc_get_combo_box, &tmp_plan.obj_i);
  }
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SetupField -- GetObject",
		       1, PLAN_OMODE_FULL, -1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SetupField Only",
		       1, PLAN_OMODE_SET, -1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "GetObject Only",
		       1, PLAN_OMODE_GET, -1);
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), tmp_plan.omode);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box, &tmp_plan.omode);
  }
  
  hg->plan_e_dexp_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_obj_dexp,
							    0.006, 6000, 0.01, 1.0, 0);
  my_signal_connect (hg->plan_e_dexp_adj, "value_changed",
		     cc_get_adj_double,
		     &hg->plan_obj_dexp);
  spinner =  gtk_spin_button_new (hg->plan_e_dexp_adj, 3, 3);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  label = gtk_label_new ("[s]  COADDS");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.coadds,
					    1, 200, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.coadds);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  
  label = gtk_label_new ("  NDR");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.ndr,
					    1, 100, 1, 1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.ndr);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  
  label = gtk_label_new ("  x");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.repeat,
					    1, 50, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new (" IRCS Setup : ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gchar *tmp_txt=NULL;
    
    store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
			       GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
			       GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
			       );
    
    for(i_use=0;i_use<hg->ircs_i_max;i_use++){
      tmp_txt=g_strdup_printf("%02d : %s",
			      i_use+1,
			      hg->ircs_set[i_use].txt);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp_txt,
			 1, i_use, 
			 2, &color_black,
			 3, &col_ircs_setup[i_use],
			 -1);
      g_free(tmp_txt);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				    "text",0,
#ifdef USE_GTK3				    
				    "foreground-rgba", 2,
				    "background-rgba", 3,
#else
				    "foreground-gdk", 2,
				    "background-gdk", 3,
#endif
				    NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan_tmp_setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_setup_list,
		       (gpointer)hg);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new (" Dither");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
    for(i_dith=0;i_dith<NUM_IRCS_DITH;i_dith++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, IRCS_dith[i_dith].name,
			 1, i_dith, 
			 -1);
    }
      
    hg->plan_e_dith_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),hg->plan_e_dith_combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->plan_e_dith_combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->plan_e_dith_combo), renderer, 
				    "text",0,NULL);
  
    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->plan_e_dith_combo), tmp_plan.dith);
    gtk_widget_show(combo);
    my_signal_connect (hg->plan_e_dith_combo,"changed", cc_get_plan_dith,
		       (gpointer)hg);
  }
  
  label = gtk_label_new (" width");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  hg->plan_e_dithw_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_dithw,
							     1.0, 20.0, 0.1, 1.0, 0);
  my_signal_connect (hg->plan_e_dithw_adj, "value_changed",
		     cc_get_adj_double,
		     &hg->plan_dithw);
  spinner =  gtk_spin_button_new (hg->plan_e_dithw_adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
     
  label = gtk_label_new (" dRA");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hg->plan_e_osra_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osra,
							    -3000, 3000, 
							    1.0, 10.0, 0);
  my_signal_connect (hg->plan_e_osra_adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_osra);
  spinner =  gtk_spin_button_new (hg->plan_e_osra_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new (" dDec");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hg->plan_e_osdec_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osdec,
							     -3000, 3000, 
							     1.0, 10.0, 0);
  my_signal_connect (hg->plan_e_osdec_adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_osdec);
  spinner =  gtk_spin_button_new (hg->plan_e_osdec_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  
  label = gtk_label_new (" Slit Scan");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
      
  hg->plan_e_sssep_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_sssep,
							     0.05, 5.00, 
							     0.01, 0.1, 0);
  my_signal_connect (hg->plan_e_sssep_adj, "value_changed",
		     cc_get_adj_double,
		     &hg->plan_sssep);
  spinner =  gtk_spin_button_new (hg->plan_e_sssep_adj, 1, 3);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("x");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hg->plan_e_ssnum_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_ssnum,
							     2, 20, 
							     1, 1, 0);
  my_signal_connect (hg->plan_e_ssnum_adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_ssnum);
  spinner =  gtk_spin_button_new (hg->plan_e_ssnum_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE, TRUE, 0);

  check = gtk_check_button_new_with_label("ADI");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       tmp_plan.adi);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.adi);

  check = gtk_check_button_new_with_label("Override Default PA");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       tmp_plan.pa_or);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.pa_or);


  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.pa,
					    -360.0, 360.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.pa);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  
  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  check = gtk_check_button_new_with_label("Back-Up Target?");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].backup);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.backup);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);

    tmp_plan.setup=hg->plan_tmp_setup;
    tmp_plan.dexp=hg->plan_obj_dexp;

    tmp_plan.dith=hg->plan_dith;
    tmp_plan.dithw=hg->plan_dithw;
    tmp_plan.osra=hg->plan_osra;
    tmp_plan.osdec=hg->plan_osdec;
    tmp_plan.sssep=hg->plan_sssep;
    tmp_plan.ssnum=hg->plan_ssnum;

    if(hg->obj[tmp_plan.obj_i].i_nst>=0){
      tmp_plan.adi=FALSE;
    }
    else if(hg->obj[tmp_plan.obj_i].aomode==AOMODE_NO){
      tmp_plan.adi=FALSE;
    }
    
    tmp_plan.time=ircs_obj_time(tmp_plan,
				hg->oh_acq,
				ircs_oh_ao(hg,
					   tmp_plan.aomode,
					   tmp_plan.obj_i));
    
    switch(tmp_plan.omode){
    case PLAN_OMODE_FULL:
    case PLAN_OMODE_SET:
      tmp_plan.stime=slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,
			      hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			      hg->vel_az, hg->vel_el);
      break;

    default:
      tmp_plan.stime=0;
    }
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagPlanEditDialog=FALSE;

  hg->plan_tmp_setup=setup0;
  hg->plan_obj_dexp=dexp0;
  hg->plan_dith=dith0;

  hg->plan_dithw=dithw0;
  hg->plan_osra=osra0;
  hg->plan_osdec=osdec0;
  hg->plan_sssep=sssep0;
  hg->plan_ssnum=ssnum0;
}



static void hsc_do_edit_obj (typHOE *hg, 
			     GtkTreeModel *model, 
			     GtkTreeIter iter, 
			     gint i_plan)
{
  GtkWidget *dialog, *label, *button, *check;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *spinner;
  GtkAdjustment *adj;
  gchar *tmp_comment;
  PLANpara tmp_plan;
  gint tmp_time;
  gchar tmp[64];
  gint i_list,i_use;
  gint setup0, osra0, osdec0, skip0, stop0, skip_upper0, stop_upper0;
  gdouble dexp0;
  gboolean hsc_300;

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  {
    // These are changed by Setup Combo
    setup0=hg->plan_tmp_setup;
    dexp0=hg->plan_obj_dexp;
    osra0 =hg->plan_osra;
    osdec0=hg->plan_osdec;
    skip0 =hg->plan_skip;
    stop0 =hg->plan_stop;
    skip_upper0 =hg->plan_skip_upper;
    stop_upper0 =hg->plan_stop_upper;
    hsc_300=hg->plan_hsc_30;
    

    hg->plan_tmp_setup=tmp_plan.setup;
    hg->plan_obj_dexp=tmp_plan.dexp;
    hg->plan_osra=tmp_plan.osra;
    hg->plan_osdec=tmp_plan.osdec;
    hg->plan_skip=tmp_plan.skip;
    hg->plan_stop=tmp_plan.stop;
    hg->plan_hsc_30=tmp_plan.hsc_30;
  }

  
  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Object)",
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

  sprintf(tmp,"[Plan #%d]  Object :", i_plan);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_list=0;i_list<hg->i_max;i_list++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hg->obj[i_list].name,
			 1, i_list, -1);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->plan[i_plan].obj_i);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_plan.obj_i);
  }
   


  hg->plan_e_dexp_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_obj_dexp,
							    2, 3600, 1.0, 10.0, 0);
  spinner =  gtk_spin_button_new (hg->plan_e_dexp_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry), 4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  label = gtk_label_new ("[s]x");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_plan.repeat,
					    1, 50, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    
  {  
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gchar *tmp_txt=NULL;
    
    store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, 
#ifdef USE_GTK3
			       GDK_TYPE_RGBA, GDK_TYPE_RGBA
#else
			       GDK_TYPE_COLOR, GDK_TYPE_COLOR
#endif
			       );
    
    for(i_use=0;i_use<hg->hsc_i_max;i_use++){
      tmp_txt=g_strdup_printf("Setup-%02d : %s",
			      i_use+1,
			      hg->hsc_set[i_use].txt);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp_txt,
			 1, i_use, 
			 2, &color_black,
			 3, &col_hsc_setup[i_use],
			 -1);
      g_free(tmp_txt);
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				    "text",0,
#ifdef USE_GTK3				    
				    "foreground-rgba", 2,
				    "background-rgba", 3,
#else
				    "foreground-gdk", 2,
				    "background-gdk", 3,
#endif
				    NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),
			     hg->plan_tmp_setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_setup_list,
		       (gpointer)hg);
  }

  hsc_set_skip_stop_upper(hg, hg->plan_tmp_setup);
    
  hg->plan_e_skip_label = gtk_label_new ("   Skip");
  gtk_box_pack_start(GTK_BOX(hbox),hg->plan_e_skip_label,FALSE,FALSE,0);

  hg->plan_e_skip_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_skip,
							    0, hg->plan_skip_upper, 
							    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->plan_e_skip_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  hg->plan_e_stop_label = gtk_label_new (" Stop");
  gtk_box_pack_start(GTK_BOX(hbox),hg->plan_e_stop_label,FALSE,FALSE,0);
      
  hg->plan_e_stop_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_stop,
							    1, hg->plan_stop_upper, 
							    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->plan_e_stop_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  my_signal_connect (hg->plan_e_skip_adj, "value_changed",
		     cc_plan_skip_adj,
		     (gpointer)hg);
  my_signal_connect (hg->plan_e_stop_adj, "value_changed",
		     cc_plan_stop_adj,
		     (gpointer)hg);
  hsc_set_skip_color(hg);
  hsc_set_stop_color(hg);

  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  label = gtk_label_new ("                   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE, TRUE, 0);

  hg->plan_e_check_hsc_30 = gtk_check_button_new_with_label("30s calib");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->plan_e_check_hsc_30),
			       hg->plan[i_plan].hsc_30);
  gtk_box_pack_start(GTK_BOX(hbox),hg->plan_e_check_hsc_30,FALSE, FALSE, 0);
  my_signal_connect (hg->plan_e_check_hsc_30, "toggled",
		     cc_get_toggle,
		     &tmp_plan.hsc_30);
  my_signal_connect (hg->plan_e_dexp_adj, "value_changed",
		     cc_get_hsc_dexp,
		     (gpointer)hg);
  set_sensitive_hsc_30(hg);

  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  check = gtk_check_button_new_with_label("Override Default PA");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].pa_or);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.pa_or);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].pa,
					    -360.0, 360.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.pa);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  
  label = gtk_label_new ("   Offset RA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  hg->plan_e_osra_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osra,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (hg->plan_e_osra_adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_osra);
  spinner =  gtk_spin_button_new (hg->plan_e_osra_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

  label = gtk_label_new (" Dec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  hg->plan_e_osdec_adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_osdec,
					    -3600, 3600, 
					    1.0, 10.0, 0);
  my_signal_connect (hg->plan_e_osdec_adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_osdec);
  spinner =  gtk_spin_button_new (hg->plan_e_osdec_adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			    TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  
 
  label = gtk_label_new ("   ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  check = gtk_check_button_new_with_label("Back-Up Target?");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->plan[i_plan].backup);
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_plan.backup);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.setup=hg->plan_tmp_setup;
    tmp_plan.dexp=hg->plan_obj_dexp;
    tmp_plan.osra=hg->plan_osra;
    tmp_plan.osdec=hg->plan_osdec;
    tmp_plan.skip=hg->plan_skip;
    tmp_plan.stop=hg->plan_stop;
    tmp_plan.hsc_30=hg->plan_hsc_30;
    
    tmp_plan.time=hsc_obj_time(tmp_plan,
			       hg->oh_acq);
    
    tmp_plan.stime=slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,
			    hg->plan[i_plan].az0,hg->plan[i_plan].el0,
			    hg->vel_az, hg->vel_el);
    
    hg->plan[i_plan]=tmp_plan;
    if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;

  hg->plan_tmp_setup=setup0;
  hg->plan_obj_dexp=dexp0;
  hg->plan_osra =osra0;
  hg->plan_osdec=osdec0;
  hg->plan_skip =skip0;
  hg->plan_stop =stop0;
  hg->plan_skip_upper=skip_upper0;
  hg->plan_stop_upper=stop_upper0;
  hg->plan_hsc_30=hsc_300;
}




int slewtime(gdouble az0, gdouble el0, gdouble az1, gdouble el1,
	     gdouble vaz, gdouble vel){
  gdouble daz, del;

  daz=fabs(az0-az1);
  
  del=fabs(el0-el1);

  if(daz>del){
    return((int)(daz/vaz));
  }
  else{
    return((int)(del/vel));
  }
}

void swap_plan(PLANpara *o1, PLANpara *o2){
  PLANpara temp;
  
  temp=*o2;
  *o2=*o1;
  *o1=temp;
}


gint get_focus_time(PLANpara plan, gint inst){
  gint ret;
  
  switch(inst){
  case INST_HDS:
    switch(plan.focus_mode){
    case PLAN_FOCUS1:
      ret=TIME_FOCUS_SV;
      break;
    case PLAN_FOCUS2:
      ret=TIME_FOCUS_AG;
      break;
    }
    break;
    
  case INST_IRCS:
    switch(plan.focus_mode){
    case PLAN_FOCUS1:
      ret=IRCS_TIME_FOCUS_OBE;
      break;
    case PLAN_FOCUS2:
      ret=IRCS_TIME_FOCUS_LGS;
      break;
    }
    break;

  case INST_HSC:
    ret=HSC_TIME_FOCUS;
    break;
  }

  return(ret);
}


gint hds_obj_time(PLANpara plan, gint oh_acq, gint readout){
  gint ret_time;

  switch(plan.omode){
  case PLAN_OMODE_FULL:
    ret_time=oh_acq+(plan.exp+readout)*plan.repeat;
    break;
      
  case PLAN_OMODE_SET:
    ret_time=oh_acq;
    break;

  case PLAN_OMODE_GET:
    ret_time=(plan.exp+readout)*plan.repeat;
    break;
  }
  return(ret_time);
}

gint ircs_obj_time(PLANpara plan, gint oh_acq, gint oh_ao){
  gint ret_time;

  switch(plan.omode){
  case PLAN_OMODE_FULL:
    ret_time
      =(gint)(oh_acq+oh_ao
	      +(plan.dexp*plan.coadds+IRCS_TIME_READOUT_NORMAL*plan.ndr+IRCS_TIME_FITS)
	      *(gdouble)plan.shot*(gdouble)plan.repeat);
    break;
    
  case PLAN_OMODE_SET:
    ret_time=oh_acq+oh_ao;
    break;
    
  case PLAN_OMODE_GET:
    ret_time=
      (gint)((plan.dexp*plan.coadds+IRCS_TIME_READOUT_NORMAL*plan.ndr+IRCS_TIME_FITS)
	     *(gdouble)plan.shot*(gdouble)plan.repeat);
    break;
  }

  return(ret_time);
}


gint hsc_obj_time(PLANpara plan, gint oh_acq){
  gint ret_time;

  ret_time=oh_acq
    +((gint)plan.dexp+HSC_TIME_READOUT)*(plan.stop-plan.skip)*plan.repeat;

  return(ret_time);
}


gint comp_time(PLANpara plan, typHOE *hg){
  gint ret=0;
  gint i_set;
  gint rep;
  gint coadds;
  gdouble exp;

  switch(hg->inst){
  case INST_HDS:
    ret=TIME_COMP
      + 20/hg->binning[hg->setup[plan.setup].binning].x/hg->binning[hg->setup[plan.setup].binning].y
      + hg->binning[hg->setup[plan.setup].binning].readout;
    break;

  case INST_IRCS:
    if(plan.cal_mode==-1){
      exp   =ircs_get_comp_exp   (hg, plan.setup);
      coadds=ircs_get_comp_coadds(hg, plan.setup);

      ret+=(exp+IRCS_TIME_READOUT_NORMAL)*(gdouble)coadds*(gdouble)plan.repeat;
    }
    else{
      ret=TIME_COMP;
      
      for(i_set=0;i_set<hg->ircs_i_max;i_set++){
	if(plan.cal_mode==hg->ircs_set[i_set].mode){

	  exp   =ircs_get_comp_exp   (hg, i_set);
	  coadds=ircs_get_comp_coadds(hg, i_set);
	  
	  ret+=(exp+IRCS_TIME_READOUT_NORMAL)*(gdouble)coadds*(gdouble)plan.repeat;
	}
      }
    }
    break;
  }

  return(ret);
}


gint flat_time(PLANpara plan, typHOE *hg){
  gint ret=0;
  gint i_set;
  gint rep;
  gint coadds=1;
  gdouble exp;

  switch(hg->inst){
  case INST_HDS:
    ret=TIME_FLAT
      + (16/hg->binning[hg->setup[plan.setup].binning].x/hg->binning[hg->setup[plan.setup].binning].y
	 + hg->binning[hg->setup[plan.setup].binning].readout)
      * plan.repeat;
    break;

  case INST_IRCS:
    ret=TIME_FLAT;
    
    if(plan.cal_mode==-1){
      exp=ircs_get_flat_exp(hg, plan.setup);
      coadds=1;
      
      ret+=(exp+IRCS_TIME_READOUT_NORMAL)*(gdouble)coadds*(gdouble)plan.repeat;
    }
    else{
      for(i_set=0;i_set<hg->ircs_i_max;i_set++){
	if(plan.cal_mode==hg->ircs_set[i_set].mode){

	  exp=ircs_get_flat_exp(hg, i_set);
	  coadds=1;
	  
	  ret+=(exp+IRCS_TIME_READOUT_NORMAL)*(gdouble)coadds*(gdouble)plan.repeat;
	}
      }
    }
    break;

  case INST_HSC:
    ret=HSC_TIME_FLAT_LAMP+(HSC_TIME_READOUT + hsc_filter[hg->hsc_set[plan.setup].filter].flat_exp)
      * hg->plan_flat_repeat;
    break;
  }

  return(ret);
}


void hsc_set_skip_stop(typHOE *hg, gint i_set){
  switch(hg->hsc_set[i_set].dith){
  case HSC_DITH_NO:
    hg->plan_skip=0;
    hg->plan_skip_upper=0;
    hg->plan_stop=1;
    hg->plan_stop_upper=1;
    break;
    
  case HSC_DITH_5:
    hg->plan_skip=0;
    hg->plan_skip_upper=4;
    hg->plan_stop=5;
    hg->plan_stop_upper=5;
    break;
    
  case HSC_DITH_N:
    hg->plan_skip=0;
    hg->plan_skip_upper=hg->hsc_set[i_set].dith_n-1;
    hg->plan_stop=      hg->hsc_set[i_set].dith_n;
    hg->plan_stop_upper=hg->hsc_set[i_set].dith_n;
    break;
  }
}

void hsc_set_skip_stop_upper(typHOE *hg, gint i_set){
  switch(hg->hsc_set[i_set].dith){
  case HSC_DITH_NO:
    hg->plan_skip_upper=0;
    hg->plan_stop_upper=1;
    break;
    
  case HSC_DITH_5:
    hg->plan_skip_upper=4;
    hg->plan_stop_upper=5;
    break;
    
  case HSC_DITH_N:
    hg->plan_skip_upper=hg->hsc_set[i_set].dith_n-1;
    hg->plan_stop_upper=hg->hsc_set[i_set].dith_n;
    break;
  }
}

void hsc_set_skip_color(typHOE *hg){
  GtkWidget *w;

  if(flagPlanEditDialog){
    w=hg->plan_e_skip_label;
  }
  else{
    w=hg->plan_skip_label;
  }
  
  if(hg->plan_skip==0){
#ifdef USE_GTK3
    css_change_col(w,"gray");
#else
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL,&color_gray2);
#endif
  }
  else{
#ifdef USE_GTK3
    css_change_col(w, "pink");
#else
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL,&color_pink);
#endif
  }
}


void hsc_set_stop_color(typHOE *hg){
  GtkWidget *w;

  if(flagPlanEditDialog){
    w=hg->plan_e_stop_label;
  }
  else{
    w=hg->plan_stop_label;
  }

  if(hg->plan_stop==hg->plan_stop_upper){
#ifdef USE_GTK3
    css_change_col(w, "gray");
#else
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL,&color_gray2);
#endif
  }
  else{
#ifdef USE_GTK3
    css_change_col(w, "pink");
#else
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL,&color_pink);
#endif
  }
}


void cc_plan_skip_adj (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  hg->plan_skip=(gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
  hsc_set_skip_color(hg);
  gtk_adjustment_set_lower(hg->plan_stop_adj, (gdouble)(hg->plan_skip+1));
}


void cc_plan_stop_adj (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  hg->plan_stop=(gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
  hsc_set_stop_color(hg);
  gtk_adjustment_set_upper(hg->plan_skip_adj, (gdouble)(hg->plan_stop-1));
}


void set_sensitive_hsc_30(typHOE *hg){
  gint i_set;
  GtkWidget *w;

  i_set=hg->plan_tmp_setup;
  
  if(flagPlanEditDialog){
    w=hg->plan_e_check_hsc_30;
  }
  else{
    w=hg->check_hsc_30;
  }
  
  if(flagPlan){
    if((hg->hsc_set[i_set].dith==HSC_DITH_NO)
       &&(!hg->hsc_set[i_set].ag)
       &&(fabs(hg->plan_obj_dexp-30)<0.01)){

      gtk_widget_set_sensitive(w, TRUE);
    }
    else{
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
				   FALSE);
      gtk_widget_set_sensitive(w, FALSE);
    }
  }
}


void cc_get_hsc_dexp (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  hg->plan_obj_dexp=gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
  set_sensitive_hsc_30(hg);
}


