//    HDS OPE file Editor
//      plan.c : Edit Obs Plan  
//                                           2010.1.27  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"

void do_efs_for_plan();
void close_plan();
void menu_close_plan();
GtkWidget*  make_plan_menu();

static void cc_obj_list();

static GtkTreeModel *create_plan_model();
static void plan_add_columns ();
void plan_long_cell_data_func();
void plan_cell_data_func();
void tree_update_plan_item();

void plan_make_tree();
void plan_close_tree();
void plan_remake_tree();

static void refresh_tree ();
static void remove_item ();
static void dup_item ();
static void up_item ();
static void down_item ();

static void add_Object ();
static void add_FocusAG ();
static void add_BIAS ();
static void add_Comp ();
static void add_Flat ();
static void add_Setup ();
static void add_I2 ();
static void add_SetAzEl ();
static void add_Comment ();

static void menu_init_plan0();
static void menu_init_plan();
void init_plan();

void remake_txt();
void remake_tod();

void plot2_plan();
void skymon2_plan();
static void focus_plan_item();
void  refresh_plan_plot();

static void  view_onRowActivated();

static void go_edit_plan();
static void close_plan_edit_dialog();
static void do_edit_comment();static void do_edit_flat();
static void do_edit_comp();
static void do_edit_setazel();
static void do_edit_bias();
static void do_edit_i2();
static void do_edit_focus();
static void do_edit_setup();
static void do_edit_obj();

void copy_plan();
int slewtime();




GtkWidget *plan_main;


gboolean flagPlanTree;
gboolean flagPlanEditDialog=FALSE;

GdkColor col_plan_setup [MAX_USESETUP]
= {
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}  //pink2
};


enum
{
  COLUMN_PLAN_TOD,
  COLUMN_PLAN_TIME,
  COLUMN_PLAN_TXT,
  COLUMN_PLAN_TXT_AZ,
  COLUMN_PLAN_TXT_EL,
  COLUMN_PLAN_WEIGHT,
  COLUMN_PLAN_COL,
  COLUMN_PLAN_COLSET,
  COLUMN_PLAN_COLBG,
  COLUMN_PLAN_COL_AZEL,
  COLUMN_PLAN_COLSET_AZEL,
  NUM_PLAN_COLUMNS
};



void do_efs_for_plan (GtkWidget *widget, gpointer gdata)
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


// Create OPE Edit Window
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
  gint i_use,i_list;
  GtkTreeModel *plan_model;
  GdkPixbuf *icon;

  flagPlan=TRUE;
  gtk_widget_set_sensitive(hg->setup_scrwin,FALSE);

  if(hg->i_plan_max<1){
    init_plan(hg);
  }

  plan_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  plan_wbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (plan_main), plan_wbox);

  planbar=make_plan_menu(hg);
  gtk_box_pack_start(GTK_BOX(plan_wbox), planbar,FALSE, FALSE, 0);

  title_tmp=g_strconcat("HOE : Observation Plan",NULL);
  gtk_window_set_title(GTK_WINDOW(plan_main), title_tmp);
  gtk_widget_realize(plan_main);
  my_signal_connect(plan_main,"destroy",
		    close_plan, 
		    (gpointer)hg);
  gtk_container_set_border_width (GTK_CONTAINER (plan_main), 0);
  

  // Command Add
  hg->plan_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->plan_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->plan_note), TRUE);
  gtk_box_pack_start(GTK_BOX(plan_wbox), hg->plan_note,FALSE, FALSE, 2);

  // Object
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("Object",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
    		      add_Object, 
    		      (gpointer)hg);

    if(hg->i_max<1){
      gtk_widget_set_sensitive(button,FALSE);
    }
  
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
    
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

      if(hg->obj[0].name){
	//if(hg->plan_obj_name) g_free(hg->plan_obj_name);
	//if(hg->plan_obj_note) g_free(hg->plan_obj_note);
	//hg->plan_obj_name=g_strdup(hg->obj[0].name);
	//hg->plan_obj_note=g_strdup(hg->obj[0].note);
	hg->plan_obj_i=0;
	hg->plan_obj_exp=hg->obj[0].exp;
	hg->plan_obj_repeat=hg->obj[0].repeat;
	hg->plan_obj_guide=hg->obj[0].guide;
      }

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
      gtk_list_store_set(store, &iter, 0, "Setup & Get",
			 1, PLAN_OMODE_FULL, -1);
      if(hg->plan_obj_omode==PLAN_OMODE_FULL) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Setup Only",
			 1, PLAN_OMODE_SET, -1);
      if(hg->plan_obj_omode==PLAN_OMODE_SET) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Get Only",
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

    hg->e_entry = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(hbox),hg->e_entry,FALSE,FALSE,0);
    if(hg->obj[0].name){
      sprintf(tmp,"%d",hg->obj[0].exp);
      hg->plan_obj_exp=hg->obj[0].exp;
      gtk_entry_set_text(GTK_ENTRY(hg->e_entry),tmp);
    }
    gtk_entry_set_editable(GTK_ENTRY(hg->e_entry),TRUE);
    my_entry_set_width_chars(GTK_ENTRY(hg->e_entry),4);
    my_signal_connect (hg->e_entry,
		       "changed",
		       cc_get_entry_int,
		       &hg->plan_obj_exp);
  
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
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   FALSE);
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


    label = gtk_label_new ("Object");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  // FocusAG
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("Focusing",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_FocusAG, 
		      (gpointer)hg);
  
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "FocusSV",
			 1, PLAN_FOCUS_SV, -1);
      if(hg->plan_focus_mode==PLAN_FOCUS_SV) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "FocusAG",
			 1, PLAN_FOCUS_AG, -1);
      if(hg->plan_focus_mode==PLAN_FOCUS_AG) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->plan_focus_mode);
    }


    label = gtk_label_new ("Focusing");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }

  // SetAzEl
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("SetAzEl",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_SetAzEl, 
		      (gpointer)hg);
    
    label = gtk_label_new ("Az ");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(-90,
					      -269, 269, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan_setaz);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

    label = gtk_label_new ("    El ");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(70,
					      15, 90, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->plan_setel);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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

  // Setup
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    button=gtkut_button_new_from_stock("Setup",GTK_STOCK_ADD);
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

  // I2Cell
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("I2Cell",GTK_STOCK_ADD);
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


  // BIAS
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("BIAS",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_BIAS, 
		      (gpointer)hg);
    
    label = gtk_label_new ("  x");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_bias_repeat,
					      1, 30, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->plan_bias_repeat);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   FALSE);
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


  // Comparison
  {
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    button=gtkut_button_new_from_stock("Comparison",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Comp, 
		      (gpointer)hg);
    
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
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    button=gtkut_button_new_from_stock("Flat",GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    my_signal_connect(button,"pressed",
		      add_Flat, 
		      (gpointer)hg);
    
    label = gtk_label_new ("  x");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_flat_repeat,
					      1, 50, 1.0, 1.0, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->plan_flat_repeat);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			      FALSE);
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   FALSE);
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
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  
    button=gtkut_button_new_from_stock("Comment",GTK_STOCK_ADD);
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
    gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
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
    gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
    
    
    label = gtk_label_new ("Comment");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->plan_note), hbox, label);
  }


  //// Setup
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(plan_wbox), hbox,FALSE, FALSE, 2);

  label = gtk_label_new ("   HDS Setup : ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);


  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_COLOR);
    hg->plan_tmp_setup=0;
    
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
		  i_use+1,setups[hg->setup[i_use].setup].initial,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			   0, tmp,
			   1, i_use, 
			   2, &col_plan_setup[i_use],
			   -1);
      }
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
      
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, 
				    "text",0,
				    "background-gdk", 2,
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
		     G_CALLBACK (do_efs_for_plan), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Display Echelle Format");
#endif

  check = gtk_check_button_new_with_label("Override Default Slit");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->plan_tmp_or);
  
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_tmp_sw,
					    100, 2000, 
					      5.0,5.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_tmp_sw);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  label = gtk_label_new ("/");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_tmp_sl,
					    1000, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_tmp_sl);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);




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
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->plan_tree), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->plan_tree)),
			       GTK_SELECTION_SINGLE);

  calc_sun_plan(hg);
  remake_tod(hg, plan_model); 

  plan_add_columns (hg, GTK_TREE_VIEW (hg->plan_tree), plan_model);

  g_object_unref (plan_model);
  
  gtk_container_add (GTK_CONTAINER (plan_scroll), hg->plan_tree);
   

  hbox = gtk_hbox_new(FALSE,4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start (GTK_BOX (plan_wbox), hbox, FALSE, FALSE, 0);

  button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
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

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    up_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Up");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    down_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Down");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    remove_item, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Remove");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    refresh_tree, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Refresh");
#endif

  button=gtkut_button_new_from_stock("Duplicate",GTK_STOCK_COPY);
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
      

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
      
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->plan_start);
  }

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan_start_hour,
					    18, 30, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->plan_start_hour);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);

  
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);


  g_signal_connect (hg->plan_tree, "cursor-changed",
		    G_CALLBACK (focus_plan_item), (gpointer)hg);

  g_signal_connect(hg->plan_tree, "row-activated", 
		   G_CALLBACK (view_onRowActivated), (gpointer)hg);

  gtk_widget_show_all(plan_main);

  gtk_main();

  g_free(title_tmp);

  flagChildDialog=FALSE;
  gdk_flush();
}


void close_plan(GtkWidget *w, gpointer gdata)
{
  typHOE *hg = (typHOE *) gdata;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(plan_main));

  gtk_widget_set_sensitive(hg->f_objtree_arud,TRUE);
  gtk_widget_set_sensitive(hg->setup_scrwin,TRUE);
  flagPlan=FALSE;
}

void menu_close_plan(GtkWidget *widget,gpointer gdata)
{
  typHOE *hg = (typHOE *) gdata;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(plan_main));

  gtk_widget_set_sensitive(hg->f_objtree_arud,TRUE);
  gtk_widget_set_sensitive(hg->setup_scrwin,TRUE);
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
  //if(hg->plan_obj_name) g_free(hg->plan_obj_name);
  //hg->plan_obj_name=g_strdup(hg->obj[hg->e_list].name);
  //if(hg->plan_obj_note) g_free(hg->plan_obj_note);
  //hg->plan_obj_note=g_strdup(hg->obj[hg->e_list].note);

  sprintf(tmp,"%d",hg->obj[hg->e_list].exp);
  hg->plan_obj_exp=hg->obj[hg->e_list].exp;
  gtk_entry_set_text(GTK_ENTRY(hg->e_entry),tmp);

  sprintf(tmp,"%d",hg->obj[hg->e_list].repeat);
  hg->plan_obj_repeat=hg->obj[hg->e_list].repeat;
  //gtk_entry_set_text(GTK_ENTRY(&GTK_SPIN_BUTTON(hg->plan_obj_spinner)->entry),tmp);
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


GtkWidget *make_plan_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  //// File
#ifdef GTK_STOCK_FILE
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  
  //File/Quit
  //File/Write OPE with Plan
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan,(gpointer)hg);

  //File/Write OPE with Plan
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan Text");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan_txt,(gpointer)hg);

  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Plan YAML");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_plan_yaml,(gpointer)hg);

  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_close_plan,(gpointer)hg);


  // Init
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Init");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  
  //Init/Initialize Plan
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Initialize Plan");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",menu_init_plan,(gpointer)hg);

  //Init/Clear All
  image=gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Clear All");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
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
			      G_TYPE_LONG,
			      G_TYPE_INT,
			      G_TYPE_STRING, // txt
                              G_TYPE_STRING,  // txt_az
                              G_TYPE_STRING,  // txt_el
			      G_TYPE_INT,    // weight
			      GDK_TYPE_COLOR,   //color
			      G_TYPE_BOOLEAN,
			      GDK_TYPE_COLOR,   //bgcolor
			      GDK_TYPE_COLOR,    //color for azel
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
					  plan_long_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PLAN_TOD),
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
					       "foreground-gdk", COLUMN_PLAN_COL,
					       "foreground-set", COLUMN_PLAN_COLSET,
					       "background-gdk", COLUMN_PLAN_COLBG,
					       NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_PLAN_TXT);
  //gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

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
					       "foreground-gdk", COLUMN_PLAN_COL_AZEL,
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
					       "foreground-gdk", COLUMN_PLAN_COL_AZEL,
					       "foreground-set", COLUMN_PLAN_COLSET_AZEL,
					       NULL);

}


void plan_long_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
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
  guint64 size;
  glong value;
  gint etime;
  gchar *str;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_PLAN_TIME, &etime,
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_PLAN_TIME:
    if(etime>0){
      str=g_strdup_printf("%d",(gint)value);
    }
    else if(etime<0){
      str=g_strdup_printf("(%d)",(gint)(-value));
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}

static void
refresh_tree (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));

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

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_plan,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];


    for(i_plan=hg->i_plan_max;i_plan>i+1;i_plan--){
      hg->plan[i_plan]=hg->plan[i_plan-1];
    }

    hg->i_plan_max++;
  
    hg->plan[i+1]=hg->plan[i];

    hg->plan[i].comment=g_strdup(hg->plan[i+1].comment);
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
  PLANpara tmp_plan;

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

    tmp_plan=hg->plan[i];
    hg->plan[i]=hg->plan[i-1];
    hg->plan[i-1]=tmp_plan;

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
  PLANpara tmp_plan;

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

    tmp_plan=hg->plan[i];
    hg->plan[i]=hg->plan[i+1];
    hg->plan[i+1]=tmp_plan;

    remake_tod(hg, model); 

    gtk_tree_path_free (path1);
    gtk_tree_path_free (path2);
  }
}


gchar * make_plan_txt(typHOE *hg, PLANpara plan){
  gchar *bu_tmp, *pa_tmp, *sv1_tmp, *sv2_tmp,
    set_tmp[64], guide_tmp[64], *ret_txt;

  
  switch(plan.type){
  case PLAN_TYPE_OBJ:
    if(plan.backup)  bu_tmp=g_strdup("(Back Up)  ");
    else bu_tmp=g_strdup("");

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
	      setups[hg->setup[plan.setup].setup].initial,
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
    switch(plan.focus_mode){
    case PLAN_FOCUS_SV:
      ret_txt=g_strdup("Focus SV");
      break;
    default:
      ret_txt=g_strdup("Focus AG");
    }
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
	      setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);

    }

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
	      setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }

    if(plan.slit_or){
      ret_txt=g_strdup_printf("Comparison, %s, %.2fx%.2f slit",
			      set_tmp,
			      (gfloat)plan.slit_width/500.,
			      (gfloat)plan.slit_length/500.);
    }
    else{
      ret_txt=g_strdup_printf("Comparison, %s",set_tmp);
    }
    break;

  case PLAN_TYPE_SETUP:
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
	      setups[hg->setup[plan.setup].setup].initial,
	      hg->binning[hg->setup[plan.setup].binning].x,
	      hg->binning[hg->setup[plan.setup].binning].y);
    }
    
    switch(plan.cmode){
    case PLAN_CMODE_FULL:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Setup Change (Full), %s, %.2fx%.2f slit",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Setup Change (Full), %s",set_tmp);
      }
      break;
    case PLAN_CMODE_EASY:
      if(plan.slit_or){
	ret_txt=g_strdup_printf("Setup Change (Cross Scan), %s, %.2fx%.2f slit",
				set_tmp,
				(gfloat)plan.slit_width/500.,
				(gfloat)plan.slit_length/500.);
      }
      else{
	ret_txt=g_strdup_printf("Setup Change (Cross Scan), %s",set_tmp);
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
    }
    break;

  case PLAN_TYPE_I2:
    if(plan.i2_pos==PLAN_I2_IN){
      ret_txt=g_strdup("I2Cell In");
    }
    else{
      ret_txt=g_strdup("I2Cell Out");
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
      calc_sun_plan(hg);
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
      calc_sun_plan(hg);
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
add_Object (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  PLANpara tmp_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_OBJ;
  tmp_plan.slit_or=hg->plan_tmp_or;
  tmp_plan.setup=hg->plan_tmp_setup;
  tmp_plan.repeat=hg->plan_obj_repeat;
  tmp_plan.obj_i=hg->plan_obj_i;
  tmp_plan.exp=hg->plan_obj_exp;
  
  tmp_plan.omode=hg->plan_obj_omode;
  tmp_plan.guide=hg->plan_obj_guide;

  
  switch(hg->plan_obj_omode){
  case PLAN_OMODE_FULL:
    if(hg->plan_tmp_or){
      tmp_plan.slit_width=hg->plan_tmp_sw;
      tmp_plan.slit_length=hg->plan_tmp_sl;
    }
    else{
      tmp_plan.slit_width=200;
      tmp_plan.slit_length=2000;
    }
    //    tmp_plan.time=TIME_SETUP_FIELD
    tmp_plan.time=hg->sv_acq
      +(hg->plan_obj_exp+hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout)*hg->plan_obj_repeat;
    break;
  case PLAN_OMODE_SET:
    if(hg->plan_tmp_or){
      tmp_plan.slit_width=hg->plan_tmp_sw;
      tmp_plan.slit_length=hg->plan_tmp_sl;
    }
    else{
      tmp_plan.slit_width=200;
      tmp_plan.slit_length=2000;
    }
    //    tmp_plan.time=TIME_SETUP_FIELD;
    tmp_plan.time=hg->sv_acq;
    break;
  case PLAN_OMODE_GET:
    tmp_plan.slit_width=200;
    tmp_plan.slit_length=2000;
    tmp_plan.time=
      (hg->plan_obj_exp+hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout)*hg->plan_obj_repeat;
  }

  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;
  
  tmp_plan.daytime=FALSE;

  
  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=hg->obj[tmp_plan.obj_i].pa;
  if(hg->obj[tmp_plan.obj_i].mag<MAG_SV2SEC){
    tmp_plan.sv_exp=1000; 
    tmp_plan.sv_or=TRUE;
 }
  else if(hg->obj[tmp_plan.obj_i].mag<MAG_SV3SEC){
    tmp_plan.sv_exp=2000;
    tmp_plan.sv_or=TRUE;
  }
  else if(hg->obj[tmp_plan.obj_i].mag<MAG_SV5SEC){
    tmp_plan.sv_exp=3000;
    tmp_plan.sv_or=TRUE;
  }
  else if(hg->obj[tmp_plan.obj_i].mag<99){
    tmp_plan.sv_exp=5000;
    tmp_plan.sv_or=TRUE;
  }
  else{
    tmp_plan.sv_exp=hg->exptime_sv;
    tmp_plan.sv_or=FALSE;
  }
  if(hg->obj[tmp_plan.obj_i].mag<MAG_SVFILTER2){
    tmp_plan.sv_fil=SV_FILTER_ND2;
  }
  else if(hg->obj[tmp_plan.obj_i].mag<MAG_SVFILTER1){
    tmp_plan.sv_fil=SV_FILTER_R;
  }
  else{
    tmp_plan.sv_fil=SV_FILTER_NONE;
  }
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);

  hg->plan[i]=tmp_plan;

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  remake_tod(hg, model); 
  tree_update_plan_item(hg, model, iter, i);
    
  refresh_plan_plot(hg);
  
}


static void
add_FocusAG (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  PLANpara tmp_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    

  tmp_plan.type=PLAN_TYPE_FOCUS;
  //tmp_plan.txt=g_strdup("Focus AG");

  tmp_plan.focus_mode=hg->plan_focus_mode;
  
  tmp_plan.setup=0;
  tmp_plan.repeat=0;
  tmp_plan.slit_or=FALSE;
  tmp_plan.slit_width=0;
  tmp_plan.slit_length=0;

  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;

  tmp_plan.daytime=FALSE;
  tmp_plan.time=TIME_FOCUS_AG;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;
  
  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
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
  PLANpara tmp_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_SetAzEl;
  tmp_plan.setaz=hg->plan_setaz;
  tmp_plan.setel=hg->plan_setel;
  tmp_plan.az1=hg->plan_setaz;
  tmp_plan.el1=hg->plan_setel;

  tmp_plan.setup=-1;
  tmp_plan.repeat=1;
  tmp_plan.slit_or=FALSE;
  tmp_plan.slit_width=0;
  tmp_plan.slit_length=0;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  
  tmp_plan.daytime=hg->plan_setazel_daytime;
  tmp_plan.time=slewtime(hg->plan[i-1].az1, hg->plan[i-1].el1,
			 tmp_plan.setaz, tmp_plan.setel);
  printf("slewtime %d\n",tmp_plan.time);

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  hg->plan[i]=tmp_plan;

  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
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
  PLANpara tmp_plan;
  GtkTreePath *path;
  //gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;

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
  

  tmp_plan.type=PLAN_TYPE_BIAS;
  //tmp_plan.txt=g_strdup_printf("BIAS x%d, %s",hg->plan_bias_repeat,tmp);
  
  tmp_plan.setup=hg->plan_tmp_setup;
  tmp_plan.repeat=hg->plan_bias_repeat;
  tmp_plan.slit_or=FALSE;
  tmp_plan.slit_width=0;
  tmp_plan.slit_length=0;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;

  tmp_plan.daytime=hg->plan_bias_daytime;
  tmp_plan.time=hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout*hg->plan_bias_repeat;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;
  
  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
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
  PLANpara tmp_plan;
  GtkTreePath *path;
  gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_COMP;
  tmp_plan.slit_or=hg->plan_tmp_or;

  if(hg->plan_tmp_or){
    tmp_plan.slit_width=hg->plan_tmp_sw;
    tmp_plan.slit_length=hg->plan_tmp_sl;
  }
  else{
    tmp_plan.slit_width=200;
    tmp_plan.slit_length=2000;
  }

  tmp_plan.setup=hg->plan_tmp_setup;
  tmp_plan.repeat=1;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;
  
  tmp_plan.daytime=hg->plan_comp_daytime;
  tmp_plan.time=TIME_COMP
    + 20/hg->binning[hg->setup[hg->plan_tmp_setup].binning].x/hg->binning[hg->setup[hg->plan_tmp_setup].binning].y
    + hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
  refresh_plan_plot(hg);
}


static void
add_Flat (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  PLANpara tmp_plan;
  GtkTreePath *path;
  //gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_FLAT;
  tmp_plan.slit_or=hg->plan_tmp_or;

  
  if(hg->plan_tmp_or){
    tmp_plan.slit_width=hg->plan_tmp_sw;
    tmp_plan.slit_length=hg->plan_tmp_sl;
  }
  else{
    tmp_plan.slit_width=200;
    tmp_plan.slit_length=2000;
  }

  tmp_plan.setup=hg->plan_tmp_setup;
  tmp_plan.repeat=hg->plan_flat_repeat;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;
  
  tmp_plan.daytime=hg->plan_flat_daytime;
  tmp_plan.time=TIME_FLAT
    + (16/hg->binning[hg->setup[hg->plan_tmp_setup].binning].x/hg->binning[hg->setup[hg->plan_tmp_setup].binning].y
       + hg->binning[hg->setup[hg->plan_tmp_setup].binning].readout)
    * hg->plan_flat_repeat *2;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
  refresh_plan_plot(hg);
}


static void
add_Setup (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  PLANpara tmp_plan;
  GtkTreePath *path;
  // gchar tmp[64];

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_SETUP;
  tmp_plan.setup=hg->plan_tmp_setup;
  tmp_plan.slit_or=hg->plan_tmp_or;
  tmp_plan.cmode=hg->plan_setup_cmode;

  switch(hg->plan_setup_cmode){
  case PLAN_CMODE_FULL:
    if(hg->plan_tmp_or){
      tmp_plan.slit_width=hg->plan_tmp_sw;
      tmp_plan.slit_length=hg->plan_tmp_sl;
    }
    else{
      tmp_plan.slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
      tmp_plan.slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
    }
    tmp_plan.time=TIME_SETUP_FULL;
    break;
  case PLAN_CMODE_EASY:
    if(hg->plan_tmp_or){
      tmp_plan.slit_width=hg->plan_tmp_sw;
      tmp_plan.slit_length=hg->plan_tmp_sl;
    }
    else{
      tmp_plan.slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
      tmp_plan.slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
    }
    tmp_plan.time=TIME_SETUP_EASY;
    break;
  case PLAN_CMODE_SLIT:
    if(hg->plan_tmp_or){
      tmp_plan.slit_width=hg->plan_tmp_sw;
      tmp_plan.slit_length=hg->plan_tmp_sl;
    }
    else{
      tmp_plan.slit_width=hg->setup[hg->plan_tmp_setup].slit_width;
      tmp_plan.slit_length=hg->setup[hg->plan_tmp_setup].slit_length;
    }
    tmp_plan.time=TIME_SETUP_SLIT;
    break;
  }

  tmp_plan.repeat=1;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.i2_pos=PLAN_I2_IN;
  
  tmp_plan.daytime=hg->plan_setup_daytime;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
  refresh_plan_plot(hg);
}


static void
add_I2 (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  gint i,i_plan;
  PLANpara tmp_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_I2;
  tmp_plan.i2_pos=hg->plan_i2_pos;

  /*
  if(hg->plan_i2_pos==PLAN_I2_IN){
    tmp_plan.txt=g_strdup("I2Cell In");
  }
  else{
    tmp_plan.txt=g_strdup("I2Cell Out");
  }
  */
  
  tmp_plan.setup=-1;
  tmp_plan.repeat=1;
  tmp_plan.slit_or=FALSE;
  tmp_plan.slit_width=0;
  tmp_plan.slit_length=0;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  
  tmp_plan.daytime=hg->plan_i2_daytime;
  tmp_plan.time=TIME_I2;

  tmp_plan.comment=NULL;
  tmp_plan.comtype=PLAN_COMMENT_TEXT;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  //if(hg->plan[i].txt) g_free(hg->plan[i].txt);
  //if(hg->plan[i].comment) g_free(hg->plan[i].comment);
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
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
  PLANpara tmp_plan;
  GtkTreePath *path;

  if(hg->i_plan_max>=MAX_PLAN) return;

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
    
  tmp_plan.type=PLAN_TYPE_COMMENT;
  tmp_plan.comtype=hg->plan_comment_type;

  switch(hg->plan_comment_type){
  case PLAN_COMMENT_TEXT:
    if(hg->plan_comment){
      tmp_plan.comment=g_strdup(hg->plan_comment);
    }
    else{
      tmp_plan.comment=NULL;
    }
    tmp_plan.time=hg->plan_comment_time*60;
    break;
    
  case PLAN_COMMENT_SUNSET:
  case PLAN_COMMENT_SUNRISE:
    tmp_plan.comment=NULL;
    tmp_plan.time=0;
    break;
  }
  
  tmp_plan.setup=-1;
  tmp_plan.repeat=1;
  tmp_plan.slit_or=FALSE;
  tmp_plan.slit_width=0;
  tmp_plan.slit_length=0;
  
  tmp_plan.obj_i=0;
  tmp_plan.exp=0;
  
  tmp_plan.omode=PLAN_OMODE_FULL;
  tmp_plan.guide=SV_GUIDE;
  
  tmp_plan.focus_mode=PLAN_FOCUS_SV;

  tmp_plan.cmode=PLAN_CMODE_FULL;
  tmp_plan.i2_pos=PLAN_I2_IN;

  tmp_plan.daytime=FALSE;

  tmp_plan.pa_or=FALSE;
  tmp_plan.pa=0;
  tmp_plan.sv_or=FALSE;
  tmp_plan.sv_exp=hg->exptime_sv;
  tmp_plan.sv_fil=SV_FILTER_NONE;
  tmp_plan.backup=FALSE;

  tmp_plan.txt=make_plan_txt(hg,tmp_plan);

  for(i_plan=hg->i_plan_max;i_plan>i;i_plan--){
    hg->plan[i_plan]=hg->plan[i_plan-1];
  }

  hg->i_plan_max++;
  
  hg->plan[i]=tmp_plan;

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
  tree_update_plan_item(hg, model, iter, i);
  
  remake_tod(hg, model); 
    
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
    if((hg->plan[i_plan].el0<0)||(hg->plan[i_plan].el1<0)){
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_COL_AZEL,&color_gray2,
			  COLUMN_PLAN_COLSET_AZEL,TRUE,
			  -1);
    }
    else if((hg->plan[i_plan].el0<15)||(hg->plan[i_plan].el1<15)){
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_PLAN_COL_AZEL,&color_gray1,
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
			COLUMN_PLAN_COL,&color_black,
			COLUMN_PLAN_COLSET,TRUE,
			-1);
  }

  if((hg->plan[i_plan].setup>=0)&&(hg->plan[i_plan].setup<MAX_USESETUP)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PLAN_COLBG,
			&col_plan_setup[hg->plan[i_plan].setup],
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
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

  gtk_widget_destroy(GTK_WIDGET(plan_main));
  plan_main = NULL;
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


  hg->plan[0].type=PLAN_TYPE_COMMENT;
  
  hg->plan[0].setup=-1;
  hg->plan[0].repeat=0;
  hg->plan[0].slit_or=FALSE;
  hg->plan[0].slit_width=0;
  hg->plan[0].slit_length=0;
  
  hg->plan[0].obj_i=0;;
  hg->plan[0].exp=0;
  
  hg->plan[0].omode=PLAN_OMODE_FULL;
  hg->plan[0].guide=SV_GUIDE;
  
  hg->plan[0].focus_mode=PLAN_FOCUS_SV;

  hg->plan[0].cmode=PLAN_CMODE_FULL;
  hg->plan[0].i2_pos=PLAN_I2_IN;

  hg->plan[0].daytime=FALSE;
  hg->plan[0].time=0;
  
  if(hg->plan[0].comment) g_free(hg->plan[0].comment);
  hg->plan[0].comment=g_strdup("========== Please Insert Your Obs Plan. ==========");
  hg->plan[0].comtype=PLAN_COMMENT_TEXT;

  hg->plan[0].pa_or=FALSE;
  hg->plan[0].pa=0;
  hg->plan[0].sv_or=FALSE;
  hg->plan[0].sv_exp=hg->exptime_sv;
  hg->plan[0].sv_fil=SV_FILTER_NONE;
  hg->plan[0].backup=FALSE;

  if(hg->plan[0].txt) g_free(hg->plan[0].txt);
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

void init_plan(typHOE *hg)
{
  gchar tmp[64];
  gchar b_tmp[64];

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
	    setups[hg->setup[hg->plan_tmp_setup].setup].initial,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
    sprintf(b_tmp,"Setup-%d : %dx%dbin",
	    hg->plan_tmp_setup+1,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].x,
	    hg->binning[hg->setup[hg->plan_tmp_setup].binning].y);
  }

  // Comment 
  {
    hg->plan[0].type=PLAN_TYPE_COMMENT;
    
    hg->plan[0].setup=-1;
    hg->plan[0].repeat=1;
    hg->plan[0].slit_or=FALSE;
    hg->plan[0].slit_width=200;
    hg->plan[0].slit_length=2000;
    
    hg->plan[0].obj_i=0;;
    hg->plan[0].exp=0;
    
    hg->plan[0].omode=PLAN_OMODE_FULL;
    hg->plan[0].guide=SV_GUIDE;
    
    hg->plan[0].focus_mode=PLAN_FOCUS_SV;
  
    hg->plan[0].cmode=PLAN_CMODE_FULL;
    hg->plan[0].i2_pos=PLAN_I2_IN;

    hg->plan[0].daytime=FALSE;
    hg->plan[0].time=0;
    
    if(hg->plan[0].comment) g_free(hg->plan[0].comment);
    hg->plan[0].comment=g_strdup("========== Evening Calibration ==========");

    hg->plan[0].comtype=PLAN_COMMENT_TEXT;

    hg->plan[0].pa_or=FALSE;
    hg->plan[0].pa=0;
    hg->plan[0].sv_or=FALSE;
    hg->plan[0].sv_exp=hg->exptime_sv;
    hg->plan[0].sv_fil=SV_FILTER_NONE;
    hg->plan[0].backup=FALSE;

    if(hg->plan[0].txt) g_free(hg->plan[0].txt);
    hg->plan[0].txt=make_plan_txt(hg,hg->plan[0]);
  }

  // Setup
  {
    hg->plan[1].type=PLAN_TYPE_SETUP;
    
    hg->plan[1].setup=0;
    hg->plan[1].repeat=1;
    hg->plan[1].slit_or=FALSE;
    hg->plan[1].slit_width=200;
    hg->plan[1].slit_length=2000;
    
    hg->plan[1].obj_i=0;;
    hg->plan[1].exp=0;
    
    hg->plan[1].omode=PLAN_OMODE_FULL;
    hg->plan[1].guide=SV_GUIDE;
    
    hg->plan[1].focus_mode=PLAN_FOCUS_SV;
  
    hg->plan[1].cmode=PLAN_CMODE_FULL;
    hg->plan[1].i2_pos=PLAN_I2_IN;
    
    hg->plan[1].daytime=TRUE;
    hg->plan[1].time=TIME_SETUP_FULL;

    if(hg->plan[1].comment) g_free(hg->plan[1].comment);
    hg->plan[1].comment=NULL;
    hg->plan[1].comtype=PLAN_COMMENT_TEXT;

    hg->plan[1].pa_or=FALSE;
    hg->plan[1].pa=0;
    hg->plan[1].sv_or=FALSE;
    hg->plan[1].sv_exp=hg->exptime_sv;
    hg->plan[1].sv_fil=SV_FILTER_NONE;
    hg->plan[1].backup=FALSE;

    if(hg->plan[1].txt) g_free(hg->plan[1].txt);
    hg->plan[1].txt=make_plan_txt(hg,hg->plan[1]);
  }

  // BIAS
  {
    hg->plan[2].type=PLAN_TYPE_BIAS;

    hg->plan[2].setup=0;
    hg->plan[2].repeat=5;
    hg->plan[2].slit_or=FALSE;
    hg->plan[2].slit_width=200;
    hg->plan[2].slit_length=2000;
    
    hg->plan[2].obj_i=0;;
    hg->plan[2].exp=0;
    
    hg->plan[2].omode=PLAN_OMODE_FULL;
    hg->plan[2].guide=SV_GUIDE;
    
    hg->plan[2].focus_mode=PLAN_FOCUS_SV;

    hg->plan[2].cmode=PLAN_CMODE_FULL;
    hg->plan[2].i2_pos=PLAN_I2_IN;
    
    hg->plan[2].daytime=TRUE;
    hg->plan[2].time=hg->binning[hg->setup[0].binning].readout*5;

    if(hg->plan[2].comment) g_free(hg->plan[2].comment);
    hg->plan[2].comment=NULL;
    hg->plan[2].comtype=PLAN_COMMENT_TEXT;

    hg->plan[2].pa_or=FALSE;
    hg->plan[2].pa=0;
    hg->plan[2].sv_or=FALSE;
    hg->plan[2].sv_exp=hg->exptime_sv;
    hg->plan[2].sv_fil=SV_FILTER_NONE;
    hg->plan[2].backup=FALSE;

    if(hg->plan[2].txt) g_free(hg->plan[2].txt);
    hg->plan[2].txt=make_plan_txt(hg,hg->plan[2]);
  }

  // Flat
  {
    hg->plan[3].type=PLAN_TYPE_FLAT;
    
    hg->plan[3].setup=0;
    hg->plan[3].repeat=10;
    hg->plan[3].slit_or=FALSE;
    hg->plan[3].slit_width=200;
    hg->plan[3].slit_length=2000;
    
    hg->plan[3].obj_i=0;;
    hg->plan[3].exp=0;
    
    hg->plan[3].omode=PLAN_OMODE_FULL;
    hg->plan[3].guide=SV_GUIDE;
    
    hg->plan[3].focus_mode=PLAN_FOCUS_SV;

    hg->plan[3].cmode=PLAN_CMODE_FULL;
    hg->plan[3].i2_pos=PLAN_I2_IN;
    
    hg->plan[3].daytime=TRUE;
    hg->plan[3].time=TIME_FLAT+
      (16/hg->binning[hg->setup[0].binning].x/hg->binning[hg->setup[0].binning].y + hg->binning[hg->setup[0].binning].readout)*10*2;

    if(hg->plan[3].comment) g_free(hg->plan[3].comment);
    hg->plan[3].comment=NULL;
    hg->plan[3].comtype=PLAN_COMMENT_TEXT;

    hg->plan[3].pa_or=FALSE;
    hg->plan[3].pa=0;
    hg->plan[3].sv_or=FALSE;
    hg->plan[3].sv_exp=hg->exptime_sv;
    hg->plan[3].sv_fil=SV_FILTER_NONE;
    hg->plan[3].backup=FALSE;

    if(hg->plan[3].txt) g_free(hg->plan[3].txt);
    hg->plan[3].txt=make_plan_txt(hg,hg->plan[3]);
  }

  // Comparison
  {
    hg->plan[4].type=PLAN_TYPE_COMP;
    
    hg->plan[4].setup=0;
    hg->plan[4].repeat=1;
    hg->plan[4].slit_or=FALSE;
    hg->plan[4].slit_width=200;
    hg->plan[4].slit_length=2000;
    
    hg->plan[4].obj_i=0;;
    hg->plan[4].exp=0;
    
    hg->plan[4].omode=PLAN_OMODE_FULL;
    hg->plan[4].guide=SV_GUIDE;
    
    hg->plan[4].focus_mode=PLAN_FOCUS_SV;

    hg->plan[4].cmode=PLAN_CMODE_FULL;
    hg->plan[4].i2_pos=PLAN_I2_IN;
    
    hg->plan[4].daytime=TRUE;
    hg->plan[4].time=TIME_COMP+
      20/hg->binning[hg->setup[0].binning].x/hg->binning[hg->setup[0].binning].y + hg->binning[hg->setup[0].binning].readout;

    if(hg->plan[4].comment) g_free(hg->plan[4].comment);
    hg->plan[4].comment=NULL;
    hg->plan[4].comtype=PLAN_COMMENT_TEXT;

    hg->plan[4].pa_or=FALSE;
    hg->plan[4].pa=0;
    hg->plan[4].sv_or=FALSE;
    hg->plan[4].sv_exp=hg->exptime_sv;
    hg->plan[4].sv_fil=SV_FILTER_NONE;
    hg->plan[4].backup=FALSE;

    if(hg->plan[4].txt) g_free(hg->plan[4].txt);
    hg->plan[4].txt=make_plan_txt(hg,hg->plan[4]);
  }

  // Comment 
  {
    hg->plan[5].type=PLAN_TYPE_COMMENT;
    
    hg->plan[5].setup=-1;
    hg->plan[5].repeat=1;
    hg->plan[5].slit_or=FALSE;
    hg->plan[5].slit_width=200;
    hg->plan[5].slit_length=2000;
    
    hg->plan[5].obj_i=0;;
    hg->plan[5].exp=0;
    
    hg->plan[5].omode=PLAN_OMODE_FULL;
    hg->plan[5].guide=SV_GUIDE;
    
    hg->plan[5].focus_mode=PLAN_FOCUS_SV;

    hg->plan[5].cmode=PLAN_CMODE_FULL;
    hg->plan[5].i2_pos=PLAN_I2_IN;
    
    hg->plan[5].daytime=FALSE;
    hg->plan[5].time=0;

    if(hg->plan[5].comment) g_free(hg->plan[5].comment);
    hg->plan[5].comment=g_strdup("========== Evening Focus ==========");
    hg->plan[5].comtype=PLAN_COMMENT_TEXT;

    hg->plan[5].pa_or=FALSE;
    hg->plan[5].pa=0;
    hg->plan[5].sv_or=FALSE;
    hg->plan[5].sv_exp=hg->exptime_sv;
    hg->plan[5].sv_fil=SV_FILTER_NONE;
    hg->plan[5].backup=FALSE;

    if(hg->plan[5].txt) g_free(hg->plan[5].txt);
    hg->plan[5].txt=make_plan_txt(hg,hg->plan[5]);
  }

  // Comment 
  {
    hg->plan[6].type=PLAN_TYPE_COMMENT;
    
    hg->plan[6].setup=-1;
    hg->plan[6].repeat=1;
    hg->plan[6].slit_or=FALSE;
    hg->plan[6].slit_width=200;
    hg->plan[6].slit_length=2000;
    
    hg->plan[6].obj_i=0;;
    hg->plan[6].exp=0;
    
    hg->plan[6].omode=PLAN_OMODE_FULL;
    hg->plan[6].guide=SV_GUIDE;
    
    hg->plan[6].focus_mode=PLAN_FOCUS_SV;

    hg->plan[6].cmode=PLAN_CMODE_FULL;
    hg->plan[6].i2_pos=PLAN_I2_IN;
    
    hg->plan[6].daytime=FALSE;
    hg->plan[6].time=0;

    if(hg->plan[6].comment) g_free(hg->plan[6].comment);
    hg->plan[6].comment=NULL;
    hg->plan[6].comtype=PLAN_COMMENT_SUNSET;

    hg->plan[6].pa_or=FALSE;
    hg->plan[6].pa=0;
    hg->plan[6].sv_or=FALSE;
    hg->plan[6].sv_exp=hg->exptime_sv;
    hg->plan[6].sv_fil=SV_FILTER_NONE;
    hg->plan[6].backup=FALSE;

    if(hg->plan[6].txt) g_free(hg->plan[6].txt);
    hg->plan[6].txt=make_plan_txt(hg,hg->plan[6]);
  }

  // FocusAG
  {
    hg->plan[7].type=PLAN_TYPE_FOCUS;
    
    hg->plan[7].setup=-1;
    hg->plan[7].repeat=1;
    hg->plan[7].slit_or=FALSE;
    hg->plan[7].slit_width=200;
    hg->plan[7].slit_length=2000;
    
    hg->plan[7].obj_i=0;;
    hg->plan[7].exp=0;
    
    hg->plan[7].omode=PLAN_OMODE_FULL;
    hg->plan[7].guide=SV_GUIDE;
    
    hg->plan[7].focus_mode=PLAN_FOCUS_SV;

    hg->plan[7].cmode=PLAN_CMODE_FULL;
    hg->plan[7].i2_pos=PLAN_I2_IN;
    
    hg->plan[7].daytime=FALSE;
    hg->plan[7].time=TIME_FOCUS_AG;

    if(hg->plan[7].comment) g_free(hg->plan[7].comment);
    hg->plan[7].comment=NULL;

    hg->plan[7].comtype=PLAN_COMMENT_TEXT;

    hg->plan[7].pa_or=FALSE;
    hg->plan[7].pa=0;
    hg->plan[7].sv_or=FALSE;
    hg->plan[7].sv_exp=hg->exptime_sv;
    hg->plan[7].sv_fil=SV_FILTER_NONE;
    hg->plan[7].backup=FALSE;

    if(hg->plan[7].txt) g_free(hg->plan[7].txt);
    hg->plan[7].txt=make_plan_txt(hg,hg->plan[7]);
  }

  // Comment 
  {
    hg->plan[8].type=PLAN_TYPE_COMMENT;
    
    hg->plan[8].setup=-1;
    hg->plan[8].repeat=1;
    hg->plan[8].slit_or=FALSE;
    hg->plan[8].slit_width=200;
    hg->plan[8].slit_length=2000;
    
    hg->plan[8].obj_i=0;;
    hg->plan[8].exp=0;
    
    hg->plan[8].omode=PLAN_OMODE_FULL;
    hg->plan[8].guide=SV_GUIDE;
    
    hg->plan[8].focus_mode=PLAN_FOCUS_SV;

    hg->plan[8].cmode=PLAN_CMODE_FULL;
    hg->plan[8].i2_pos=PLAN_I2_IN;
    
    hg->plan[8].daytime=FALSE;
    hg->plan[8].time=0;

    if(hg->plan[8].comment) g_free(hg->plan[8].comment);
    hg->plan[8].comment=g_strdup("##### <<<<<<<<< INSERT YOUR TARGETS HERE >>>>>>>>> #####");
    hg->plan[8].comtype=PLAN_COMMENT_TEXT;

    hg->plan[8].pa_or=FALSE;
    hg->plan[8].pa=0;
    hg->plan[8].sv_or=FALSE;
    hg->plan[8].sv_exp=hg->exptime_sv;
    hg->plan[8].sv_fil=SV_FILTER_NONE;
    hg->plan[8].backup=FALSE;

    if(hg->plan[8].txt) g_free(hg->plan[8].txt);
    hg->plan[8].txt=make_plan_txt(hg,hg->plan[8]);
  }

  // Comment 
  {
    hg->plan[9].type=PLAN_TYPE_COMMENT;
    
    hg->plan[9].setup=-1;
    hg->plan[9].repeat=1;
    hg->plan[9].slit_or=FALSE;
    hg->plan[9].slit_width=200;
    hg->plan[9].slit_length=2000;
    
    hg->plan[9].obj_i=0;;
    hg->plan[9].exp=0;
    
    hg->plan[9].omode=PLAN_OMODE_FULL;
    hg->plan[9].guide=SV_GUIDE;
    
    hg->plan[9].focus_mode=PLAN_FOCUS_SV;

    hg->plan[9].cmode=PLAN_CMODE_FULL;
    hg->plan[9].i2_pos=PLAN_I2_IN;
    
    hg->plan[9].daytime=FALSE;
    hg->plan[9].time=0;

    if(hg->plan[9].comment) g_free(hg->plan[9].comment);
    hg->plan[9].comment=NULL;
    hg->plan[9].comtype=PLAN_COMMENT_SUNRISE; 

    hg->plan[9].pa_or=FALSE;
    hg->plan[9].pa=0;
    hg->plan[9].sv_or=FALSE;
    hg->plan[9].sv_exp=hg->exptime_sv;
    hg->plan[9].sv_fil=SV_FILTER_NONE;
    hg->plan[9].backup=FALSE;

    if(hg->plan[9].txt) g_free(hg->plan[9].txt);
    hg->plan[9].txt=make_plan_txt(hg,hg->plan[9]);
 }

  // Comment 
  {
    hg->plan[10].type=PLAN_TYPE_COMMENT;
    
    hg->plan[10].setup=-1;
    hg->plan[10].repeat=1;
    hg->plan[10].slit_or=FALSE;
    hg->plan[10].slit_width=200;
    hg->plan[10].slit_length=2000;
    
    hg->plan[10].obj_i=0;;
    hg->plan[10].exp=0;
    
    hg->plan[10].omode=PLAN_OMODE_FULL;
    hg->plan[10].guide=SV_GUIDE;
    
    hg->plan[10].focus_mode=PLAN_FOCUS_SV;

    hg->plan[10].cmode=PLAN_CMODE_FULL;
    hg->plan[10].i2_pos=PLAN_I2_IN;
    
    hg->plan[10].daytime=FALSE;
    hg->plan[10].time=0;

    if(hg->plan[10].comment) g_free(hg->plan[10].comment);
    hg->plan[10].comment=g_strdup("========== Morning Focus ==========");
    hg->plan[10].comtype=PLAN_COMMENT_TEXT;

    hg->plan[10].pa_or=FALSE;
    hg->plan[10].pa=0;
    hg->plan[10].sv_or=FALSE;
    hg->plan[10].sv_exp=hg->exptime_sv;
    hg->plan[10].sv_fil=SV_FILTER_NONE;
    hg->plan[10].backup=FALSE;

    if(hg->plan[10].txt) g_free(hg->plan[10].txt);
    hg->plan[10].txt=make_plan_txt(hg,hg->plan[10]);
  }

  // FocusAG
  {
    hg->plan[11].type=PLAN_TYPE_FOCUS;
    
    hg->plan[11].setup=-1;
    hg->plan[11].repeat=1;
    hg->plan[11].slit_or=FALSE;
    hg->plan[11].slit_width=200;
    hg->plan[11].slit_length=2000;
    
    hg->plan[11].obj_i=0;;
    hg->plan[11].exp=0;
    
    hg->plan[11].omode=PLAN_OMODE_FULL;
    hg->plan[11].guide=SV_GUIDE;
    
    hg->plan[11].focus_mode=PLAN_FOCUS_SV;

    hg->plan[11].cmode=PLAN_CMODE_FULL;
    hg->plan[11].i2_pos=PLAN_I2_IN;
    
    hg->plan[11].daytime=FALSE;
    hg->plan[11].time=TIME_FOCUS_AG;

    if(hg->plan[11].comment) g_free(hg->plan[11].comment);
    hg->plan[11].comment=NULL;
    hg->plan[11].comtype=PLAN_COMMENT_TEXT;

    hg->plan[11].pa_or=FALSE;
    hg->plan[11].pa=0;
    hg->plan[11].sv_or=FALSE;
    hg->plan[11].sv_exp=hg->exptime_sv;
    hg->plan[11].sv_fil=SV_FILTER_NONE;
    hg->plan[11].backup=FALSE;

    if(hg->plan[11].txt) g_free(hg->plan[11].txt);
    hg->plan[11].txt=make_plan_txt(hg,hg->plan[11]);
  }

  // Comment 
  {
    hg->plan[12].type=PLAN_TYPE_COMMENT;
    
    hg->plan[12].setup=-1;
    hg->plan[12].repeat=1;
    hg->plan[12].slit_or=FALSE;
    hg->plan[12].slit_width=200;
    hg->plan[12].slit_length=2000;
    
    hg->plan[12].obj_i=0;;
    hg->plan[12].exp=0;
    
    hg->plan[12].omode=PLAN_OMODE_FULL;
    hg->plan[12].guide=SV_GUIDE;
    
    hg->plan[12].focus_mode=PLAN_FOCUS_SV;

    hg->plan[12].cmode=PLAN_CMODE_FULL;
    hg->plan[12].i2_pos=PLAN_I2_IN;
    
    hg->plan[12].daytime=FALSE;
    hg->plan[12].time=0;

    if(hg->plan[12].comment) g_free(hg->plan[12].comment);
    hg->plan[12].comment=g_strdup("========== Morning Calibration ==========");
    hg->plan[12].comtype=PLAN_COMMENT_TEXT;

    hg->plan[12].pa_or=FALSE;
    hg->plan[12].pa=0;
    hg->plan[12].sv_or=FALSE;
    hg->plan[12].sv_exp=hg->exptime_sv;
    hg->plan[12].sv_fil=SV_FILTER_NONE;
    hg->plan[12].backup=FALSE;

    if(hg->plan[12].txt) g_free(hg->plan[12].txt);
    hg->plan[12].txt=make_plan_txt(hg,hg->plan[12]);
  }

  // Comparison
  {
    hg->plan[13].type=PLAN_TYPE_COMP;
    
    hg->plan[13].setup=0;
    hg->plan[13].repeat=1;
    hg->plan[13].slit_or=FALSE;
    hg->plan[13].slit_width=200;
    hg->plan[13].slit_length=2000;
    
    hg->plan[13].obj_i=0;;
    hg->plan[13].exp=0;
    
    hg->plan[13].omode=PLAN_OMODE_FULL;
    hg->plan[13].guide=SV_GUIDE;
    
    hg->plan[13].focus_mode=PLAN_FOCUS_SV;

    hg->plan[13].cmode=PLAN_CMODE_FULL;
    hg->plan[13].i2_pos=PLAN_I2_IN;
    
    hg->plan[13].daytime=TRUE;
    hg->plan[13].time=TIME_COMP+
      20/hg->binning[hg->setup[0].binning].x/hg->binning[hg->setup[0].binning].y + hg->binning[hg->setup[0].binning].readout;

    if(hg->plan[13].comment) g_free(hg->plan[13].comment);
    hg->plan[13].comment=NULL;
    hg->plan[13].comtype=PLAN_COMMENT_TEXT;

    hg->plan[13].pa_or=FALSE;
    hg->plan[13].pa=0;
    hg->plan[13].sv_or=FALSE;
    hg->plan[13].sv_exp=hg->exptime_sv;
    hg->plan[13].sv_fil=SV_FILTER_NONE;
    hg->plan[13].backup=FALSE;

    if(hg->plan[13].txt) g_free(hg->plan[13].txt);
    hg->plan[13].txt=make_plan_txt(hg,hg->plan[13]);
  }

  // Flat
  {
    hg->plan[14].type=PLAN_TYPE_FLAT;
    
    hg->plan[14].setup=0;
    hg->plan[14].repeat=10;
    hg->plan[14].slit_or=FALSE;
    hg->plan[14].slit_width=200;
    hg->plan[14].slit_length=2000;
    
    hg->plan[14].obj_i=0;;
    hg->plan[14].exp=0;
    
    hg->plan[14].omode=PLAN_OMODE_FULL;
    hg->plan[14].guide=SV_GUIDE;
    
    hg->plan[14].focus_mode=PLAN_FOCUS_SV;

    hg->plan[14].cmode=PLAN_CMODE_FULL;
    hg->plan[14].i2_pos=PLAN_I2_IN;
    
    hg->plan[14].daytime=TRUE;
    hg->plan[14].time=TIME_FLAT+
      (16/hg->binning[hg->setup[0].binning].x/hg->binning[hg->setup[0].binning].y + hg->binning[hg->setup[0].binning].readout)*10*2;

    if(hg->plan[14].comment) g_free(hg->plan[14].comment);
    hg->plan[14].comment=NULL;
    hg->plan[14].comtype=PLAN_COMMENT_TEXT;

    hg->plan[14].pa_or=FALSE;
    hg->plan[14].pa=0;
    hg->plan[14].sv_or=FALSE;
    hg->plan[14].sv_exp=hg->exptime_sv;
    hg->plan[14].sv_fil=SV_FILTER_NONE;
    hg->plan[14].backup=FALSE;

    if(hg->plan[14].txt) g_free(hg->plan[14].txt);
    hg->plan[14].txt=make_plan_txt(hg,hg->plan[14]);
  }

  // BIAS
  {
    hg->plan[15].type=PLAN_TYPE_BIAS;
    
    hg->plan[15].setup=0;
    hg->plan[15].repeat=5;
    hg->plan[15].slit_or=FALSE;
    hg->plan[15].slit_width=200;
    hg->plan[15].slit_length=2000;
    
    hg->plan[15].obj_i=0;;
    hg->plan[15].exp=0;
    
    hg->plan[15].omode=PLAN_OMODE_FULL;
    hg->plan[15].guide=SV_GUIDE;
    
    hg->plan[15].focus_mode=PLAN_FOCUS_SV;

    hg->plan[15].cmode=PLAN_CMODE_FULL;
    hg->plan[15].i2_pos=PLAN_I2_IN;
    
    hg->plan[15].daytime=TRUE;
    hg->plan[15].time=hg->binning[hg->setup[0].binning].readout*5;

    if(hg->plan[15].comment) g_free(hg->plan[15].comment);
    hg->plan[15].comment=NULL;
    hg->plan[15].comtype=PLAN_COMMENT_TEXT;

    hg->plan[15].pa_or=FALSE;
    hg->plan[15].pa=0;
    hg->plan[15].sv_or=FALSE;
    hg->plan[15].sv_exp=hg->exptime_sv;
    hg->plan[15].sv_fil=SV_FILTER_NONE;
    hg->plan[15].backup=FALSE;

    if(hg->plan[15].txt) g_free(hg->plan[15].txt);
    hg->plan[15].txt=make_plan_txt(hg,hg->plan[15]);
  }

  // Comment 
  {
    hg->plan[16].type=PLAN_TYPE_COMMENT;
    
    hg->plan[16].setup=-1;
    hg->plan[16].repeat=1;
    hg->plan[16].slit_or=FALSE;
    hg->plan[16].slit_width=200;
    hg->plan[16].slit_length=2000;
    
    hg->plan[16].obj_i=0;;
    hg->plan[16].exp=0;
    
    hg->plan[16].omode=PLAN_OMODE_FULL;
    hg->plan[16].guide=SV_GUIDE;
    
    hg->plan[16].focus_mode=PLAN_FOCUS_SV;

    hg->plan[16].cmode=PLAN_CMODE_FULL;
    hg->plan[16].i2_pos=PLAN_I2_IN;
    
    hg->plan[16].daytime=FALSE;
    hg->plan[16].time=0;

    if(hg->plan[16].comment) g_free(hg->plan[16].comment);
    hg->plan[16].comment=g_strdup("========== End of Observation ==========");
    hg->plan[16].comtype=PLAN_COMMENT_TEXT;

    hg->plan[16].pa_or=FALSE;
    hg->plan[16].pa=0;
    hg->plan[16].sv_or=FALSE;
    hg->plan[16].sv_exp=hg->exptime_sv;
    hg->plan[16].sv_fil=SV_FILTER_NONE;
    hg->plan[16].backup=FALSE;

    if(hg->plan[16].txt) g_free(hg->plan[16].txt);
    hg->plan[16].txt=make_plan_txt(hg,hg->plan[16]);
  }

  hg->i_plan_max=17;
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


void remake_tod(typHOE *hg, GtkTreeModel *model) 
{
  gint i_plan;
  glong sod, sod_start;
  GtkTreeIter iter; 
  struct ln_equ_posn oequ;
  struct ln_lnlat_posn observer;
  struct ln_zonedate zonedate;
  gdouble JD_hst;
  struct ln_hrz_posn ohrz;
  gchar *tod_start, *tod_end, *tmp;

  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;

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
    sod=hg->plan_start_hour*60*60 + hg->plan_start_min*60;
  }
 
  sod_start=sod;
  tod_start=get_txt_tod(sod_start);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    hg->plan[i_plan].az1=-90.;
    hg->plan[i_plan].el1=90.;
    hg->plan[i_plan].txt_az=NULL;
    hg->plan[i_plan].txt_el=NULL;

    if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
      if(hg->plan[i_plan].time>0){
	hg->plan[i_plan].sod=sod;

	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  double a0s;
	  int ia0h,ia0m;
	  double d0s;
	  int id0d,id0m;
	  double a0,d0;

	  zonedate.hours=sod/60/60;
	  zonedate.minutes=(sod-zonedate.hours*60*60)/60;
	  zonedate.seconds=0;

	  JD_hst = ln_get_julian_local_date(&zonedate);
	  
	  a0s=hg->obj[hg->plan[i_plan].obj_i].ra;
	  ia0h=(int)(a0s/10000);
	  a0s=a0s-(double)(ia0h)*10000;
	  ia0m=(int)(a0s/100);
	  a0s=a0s-(double)(ia0m)*100;
	
	  //d0s=hg->obj[hg->plot_i].dec;
	  d0s=hg->obj[hg->plan[i_plan].obj_i].dec;
	  id0d=(int)(d0s/10000);
	  d0s=d0s-(double)(id0d)*10000;
	  id0m=(int)(d0s/100);
	  d0s=d0s-(double)(id0m)*100;
	
	  a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
	  d0=id0d + id0m/60. + d0s/3600.;  //[deg]

	  oequ.ra=a0*360/24;
	  oequ.dec=d0;


	  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	  if(i_plan!=0){
	    if((ohrz.az-hg->plan[i_plan-1].az1)>180){
	      if(ohrz.az-360>-270){
		ohrz.az-=360;
	      }
	    }
	    else if((ohrz.az-hg->plan[i_plan-1].az1)<-180){
	      if(ohrz.az+360<270){
		ohrz.az+=360;
	      }
	    }
	  }
	  if(ohrz.az>270) ohrz.az-=360;
	  if(ohrz.az<-270) ohrz.az+=360;

	  hg->plan[i_plan].az0=ohrz.az;
	  hg->plan[i_plan].el0=ohrz.alt;
	  switch(hg->plan[i_plan].omode){
	  case PLAN_OMODE_FULL:
	    if(i_plan!=0){
	      hg->plan[i_plan].time=hg->sv_acq
		+slewtime(hg->plan[i_plan-1].az1, hg->plan[i_plan-1].el1,
			  hg->plan[i_plan].az0,hg->plan[i_plan].el0)
		+(hg->plan[i_plan].exp+hg->binning[hg->setup[hg->plan[i_plan].setup].binning].readout)*hg->plan[i_plan].repeat;
	    }
	    else{
	      hg->plan[i_plan].time=hg->sv_acq
		+slewtime(-90, 90,hg->plan[i_plan].az0,hg->plan[i_plan].el0)
		+(hg->plan[i_plan].exp+hg->binning[hg->setup[hg->plan[i_plan].setup].binning].readout)*hg->plan[i_plan].repeat;
	    }
	    break;
	  case PLAN_OMODE_SET:
	    if(i_plan!=0){
	      hg->plan[i_plan].time=hg->sv_acq
		+slewtime(hg->plan[i_plan-1].az1, hg->plan[i_plan-1].el1,
			  hg->plan[i_plan].az0,hg->plan[i_plan].el0);
	    }
	    else{
	      hg->plan[i_plan].time=hg->sv_acq
		+slewtime(-90, 90,hg->plan[i_plan].az0,hg->plan[i_plan].el0);
	    }
	    break;
	  }
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
	  hg->plan[i_plan].time=
	    slewtime(hg->plan[i_plan].az0,hg->plan[i_plan].el0,
		     hg->plan[i_plan].az1,hg->plan[i_plan].el1);
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
	

	sod+=(glong)hg->plan[i_plan].time;

	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  zonedate.hours=sod/60/60;
	  zonedate.minutes=(sod-zonedate.hours*60*60)/60;
	  zonedate.seconds=0;

	  JD_hst = ln_get_julian_local_date(&zonedate);

	  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	  if(i_plan!=0){
	    if((ohrz.az-hg->plan[i_plan].az0)>180){
	      if(ohrz.az-360>-270){
		ohrz.az-=360;
	      }
	    }
	    else if((ohrz.az-hg->plan[i_plan].az0)<-180){
	      if(ohrz.az+360<270){
		ohrz.az+=360;
	      }
	    }
	  }
	  if(ohrz.az>270) ohrz.az-=360;
	  if(ohrz.az<-270) ohrz.az+=360;

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
  tmp=g_strdup_printf("%s -- %s (%.2lf hrs)",
		      tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.);
  gtk_label_set_text(GTK_LABEL(hg->label_stat_plan),tmp);
  if(tod_start) g_free(tod_start);
  if(tod_end) g_free(tod_end);
  if(tmp) g_free(tmp);
}


void remake_sod(typHOE *hg) 
{
  gint i_plan;
  glong sod, sod_start;
  gchar *tod_start, *tod_end, *tmp;
  
  if(hg->plan_start==PLAN_START_EVENING){
    sod=hg->sun.s_set.hours*60*60 + hg->sun.s_set.minutes*60
      + SUNSET_OFFSET*60;
  }
  else{
    sod=hg->plan_start_hour*60*60 + hg->plan_start_min*60;
  }
  
  sod_start=sod;
  tod_start=get_txt_tod(sod_start);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if((!hg->plan[i_plan].daytime)&&(!hg->plan[i_plan].backup)){
      if(hg->plan[i_plan].time>0){
	hg->plan[i_plan].sod=sod;
	sod+=(glong)hg->plan[i_plan].time;
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
  tmp=g_strdup_printf("%s -- %s (%.2lf hrs)",
		      tod_start,tod_end, (gdouble)(sod-sod_start)/60./60.);
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
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    hg->plot_i_plan=i;
    

    gtk_tree_path_free (path);
  }

  if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
    hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
    //hg->plot_target=PLOT_PLAN;
    do_plot(widget,(gpointer)hg);
  }

}

void skymon2_plan (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));
  
  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    
    path = gtk_tree_model_get_path (model, &iter);
    i = gtk_tree_path_get_indices (path)[0];

    hg->plot_i_plan=i;
    

    gtk_tree_path_free (path);
  }

  if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
    hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
    do_skymon(widget,(gpointer)hg);
    refresh_plan_plot(hg);
  }

}


static void
focus_plan_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->plan_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->plan_tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (model, &iter);
      i = gtk_tree_path_get_indices (path)[0];

      hg->plot_i_plan=i;
      if(hg->plan[hg->plot_i_plan].type==PLAN_TYPE_OBJ){
	hg->plot_i=hg->plan[hg->plot_i_plan].obj_i;
    }

      gtk_tree_path_free (path);
    }


  refresh_plan_plot(hg);
  
}

void refresh_plan_plot(typHOE *hg){
  if(hg->plan[hg->plot_i_plan].sod>0){
    hg->skymon_year=hg->fr_year;
    hg->skymon_month=hg->fr_month;
    hg->skymon_day=hg->fr_day;
    hg->skymon_hour=hg->plan[hg->plot_i_plan].sod/60./60.;
    hg->skymon_min=(hg->plan[hg->plot_i_plan].sod-hg->skymon_hour*60.*60.)/60.;
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

      draw_skymon_cairo(hg->skymon_dw,NULL,(gpointer)hg);
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
      do_edit_obj(hg,model,iter,i_plan);
      break;
    }
    
    tree_update_plan_item(hg, model, iter, i_plan);
    remake_tod(hg, model);
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Comment :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
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
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  
  
  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    hg->plan[i_plan]=tmp_plan;
    hg->plan[i_plan].time=tmp_time*60;
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

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Flat)",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Flat :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  


  label = gtk_label_new ("  x");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].repeat,
					    1, 50, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
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
		  i_use+1,
		  -hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
		  i_use+1,
		  setups[hg->setup[i_use].setup].initial,
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
  

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_width,
					    100, 2000, 
					    5.0,5.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_width);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  label = gtk_label_new ("/");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_length,
					    1000, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_length);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  

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

    tmp_plan.time=TIME_FLAT
      + (16/hg->binning[hg->setup[tmp_plan.setup].binning].x/hg->binning[hg->setup[tmp_plan.setup].binning].y
	 + hg->binning[hg->setup[tmp_plan.setup].binning].readout)
      * tmp_plan.repeat *2;

    hg->plan[i_plan]=tmp_plan;
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

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Comparison)",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Comparison :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
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
		  i_use+1,setups[hg->setup[i_use].setup].initial,
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


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_width,
					    100, 2000, 
					    5.0,5.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_width);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

  label = gtk_label_new ("/");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_length,
					    1000, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_length);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


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
    
    tmp_plan.time=TIME_COMP
      + 20/hg->binning[hg->setup[tmp_plan.setup].binning].x/hg->binning[hg->setup[tmp_plan.setup].binning].y
      + hg->binning[hg->setup[tmp_plan.setup].binning].readout;
    
    hg->plan[i_plan]=tmp_plan;
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Bias :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  

  label = gtk_label_new ("  x");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].repeat,
					    1, 30, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.repeat);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  SetAzEl :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  

  label = gtk_label_new ("Az ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].setaz,
					    -269, 269, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.setaz);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

  label = gtk_label_new ("    El ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
    
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].setel,
					    15, 90, 1.0, 1.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_plan.setel);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
			    FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
    tmp_plan.time=slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,
			   hg->plan[i_plan].setaz,hg->plan[i_plan].setel);
    
    hg->plan[i_plan]=tmp_plan;
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  I2 Cell :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
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

  if(flagPlanEditDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  flagPlanEditDialog=TRUE;

  tmp_plan=hg->plan[i_plan];

  dialog = gtk_dialog_new_with_buttons("HOE : Plan Edit (Focusing)",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Focusing :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FocusSV",
		       1, PLAN_FOCUS_SV, -1);
    if(hg->plan[i_plan].focus_mode==PLAN_FOCUS_SV) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FocusAG",
		       1, PLAN_FOCUS_AG, -1);
    if(hg->plan[i_plan].focus_mode==PLAN_FOCUS_AG) iter_set=iter;
    
    
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

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    
    tmp_plan.time=TIME_FOCUS_AG;
    
    hg->plan[i_plan]=tmp_plan;
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Setup :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  
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
		  setups[hg->setup[i_use].setup].initial,
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


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_width,
					    100, 2000, 
					    5.0,5.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_width);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

  label = gtk_label_new ("/");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_length,
					    1000, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_length);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  

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

    switch(tmp_plan.cmode){
    case PLAN_CMODE_FULL:
      tmp_plan.time=TIME_SETUP_FULL;
      break;
    case PLAN_CMODE_EASY:
      tmp_plan.time=TIME_SETUP_EASY;
      break;
    case PLAN_CMODE_SLIT:
      tmp_plan.time=TIME_SETUP_SLIT;
      break;
    }
    
    hg->plan[i_plan]=tmp_plan;
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}



static void do_edit_obj (typHOE *hg, 
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
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  sprintf(tmp,"[Plan #%d]  Object :", i_plan);
  label = gtk_label_new (tmp);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
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
    gtk_list_store_set(store, &iter, 0, "Setup & Get",
		       1, PLAN_OMODE_FULL, -1);
    if(hg->plan[i_plan].omode==PLAN_OMODE_FULL) iter_set=iter;
      
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Setup Only",
		       1, PLAN_OMODE_SET, -1);
    if(hg->plan[i_plan].omode==PLAN_OMODE_SET) iter_set=iter;
      
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Get Only",
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

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  sprintf(tmp,"%d",hg->plan[i_plan].exp);
  gtk_entry_set_text(GTK_ENTRY(entry),tmp);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),4);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry_int,
		     &tmp_plan.exp);
  
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
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
		  i_use+1,setups[hg->setup[i_use].setup].initial,
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


  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_width,
					    100, 2000, 
					    5.0,5.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_width);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  
  label = gtk_label_new ("/");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->plan[i_plan].slit_length,
					      1000, 30000, 
					    100.0,100.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_plan.slit_length);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


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


  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  
  label = gtk_label_new ("                   ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  
  label = gtk_label_new ("   ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
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
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
    

    switch(tmp_plan.omode){
    case PLAN_OMODE_FULL:
      //      tmp_plan.time=TIME_SETUP_FIELD
      //	+(tmp_plan.exp+hg->binning[hg->setup[tmp_plan.setup].binning].readout)*tmp_plan.repeat;
      tmp_plan.time=hg->sv_acq+slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,hg->plan[i_plan].az0,hg->plan[i_plan].el0)
	+(tmp_plan.exp+hg->binning[hg->setup[tmp_plan.setup].binning].readout)*tmp_plan.repeat;
      break;
    case PLAN_OMODE_SET:
      //      tmp_plan.time=TIME_SETUP_FIELD;
      tmp_plan.time=hg->sv_acq+slewtime(hg->plan[i_plan-1].az1,hg->plan[i_plan-1].el1,hg->plan[i_plan].az0,hg->plan[i_plan].el0);
    break;
    case PLAN_OMODE_GET:
      tmp_plan.time=(tmp_plan.exp+hg->binning[hg->setup[tmp_plan.setup].binning].readout)*tmp_plan.repeat;
    }
    
    hg->plan[i_plan]=tmp_plan;
    hg->plan[i_plan].txt=make_plan_txt(hg, hg->plan[i_plan]);
  }
  else{
    gtk_widget_destroy(dialog);
  }
  
  flagPlanEditDialog=FALSE;
}






void copy_plan(PLANpara tmp_plan, PLANpara plan){
  plan.type=tmp_plan.type;
  plan.txt=g_strdup(tmp_plan.txt);

  plan.setup=tmp_plan.setup;
  plan.repeat=tmp_plan.repeat;
  plan.slit_or=tmp_plan.slit_or;
  plan.slit_width=tmp_plan.slit_width;
  plan.slit_length=tmp_plan.slit_length;

  plan.obj_i=tmp_plan.obj_i;
  plan.exp=tmp_plan.exp;

  plan.omode=tmp_plan.omode;
  plan.guide=tmp_plan.guide;
  
  plan.cmode=tmp_plan.cmode;
  
  plan.i2_pos=tmp_plan.i2_pos;

  plan.daytime=tmp_plan.daytime;

  plan.comment=g_strdup(tmp_plan.comment);

  plan.comtype=tmp_plan.comtype;
  plan.time=tmp_plan.time;
  plan.sod=tmp_plan.sod;
}

int slewtime(gdouble az0, gdouble el0, gdouble az1, gdouble el1){
  gdouble daz, del;

  daz=fabs(az0-az1);
  del=fabs(el0-el1);

  if(daz>del){
    return((int)daz*2);
  }
  else{
    return((int)del*2);
  }
}
