//    HDS OPE file Editor
//      objtree.c : Main Target List Treeview
//                                           2010.1.27  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"

//void make_obj_tree();
static void objtree_add_columns();
static GtkTreeModel *create_items_model ();
//void objtree_update_item();
void objtree_update_radec_item();
static void cell_edited ();
static void cell_toggled_check ();
static void cell_toggled ();
void objtree_int_cell_data_func();
void objtree_double_cell_data_func();
void objtree_mag_cell_data_func();
void objtree_magsrc_cell_data_func();
void objtree_rise_cell_data_func();
void objtree_set_cell_data_func();
void objtree_transit_cell_data_func();
static GtkTreeModel * create_repeat_model ();
static GtkTreeModel * create_guide_model ();
//void add_item_objtree();
//void up_item_objtree();
//void down_item_objtree();
//void remove_item_objtree();
//void wwwdb_item();
//void do_update_exp();
//void export_def ();
//void do_plot();
static void focus_objtree_item();
//void plot2_objtree_item();
//void etc_objtree_item();
static void ok_addobj();
static void addobj_simbad_query ();
static void addobj_ned_query ();
void addobj_dl();
//void addobj_dialog();
//void strchg();
//void str_replace();
//gchar *make_simbad_id();
//void update_c_label();

void get_total_basic_exp(typHOE *hg){
  gint i_list, i_use, set_num;
  gchar *tmp;
  glong total_exp=0,  total_obs=0;

  for(i_list=0;i_list<hg->i_max;i_list++){
    set_num=0;
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->obj[i_list].setup[i_use]){
	set_num++;
	total_obs+=(hg->binning[hg->setup[i_use].binning].readout
		    +hg->obj[i_list].exp)
		    *hg->obj[i_list].repeat+TIME_SETUP_FIELD;
      }
    }
    
    total_exp+=hg->obj[i_list].exp*hg->obj[i_list].repeat*set_num;
  }

  tmp=g_strdup_printf("Total Exp. = %.2lf hrs,  Estimated Obs. Time = %.2lf hrs",
		      (gdouble)total_exp/60./60.,
		      (gdouble)total_obs/60./60.);

  gtk_label_set_text(GTK_LABEL(hg->label_stat_base),tmp);
  if(tmp) g_free(tmp);

  remake_sod(hg);
}


void objtree_update_radec (gpointer gdata)
{
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    objtree_update_radec_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}



void make_obj_tree(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  GtkTreeModel *repeat_model;
  GtkTreeModel *guide_model;
  
  if(flag_make_obj_tree)  gtk_widget_destroy(hg->objtree);
  else flag_make_obj_tree=TRUE;

  items_model = create_items_model (hg);
  repeat_model = create_repeat_model ();
  guide_model = create_guide_model ();

  /* create tree view */
  hg->objtree = gtk_tree_view_new_with_model (items_model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->objtree), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->objtree)),
			       GTK_SELECTION_SINGLE);
  objtree_add_columns (hg, GTK_TREE_VIEW (hg->objtree), 
		       items_model,
		       repeat_model,
		       guide_model);

  g_object_unref(items_model);
  g_object_unref(repeat_model);
  g_object_unref(guide_model);
  
  gtk_container_add (GTK_CONTAINER (hg->sw_objtree), hg->objtree);
  
  g_signal_connect (hg->objtree, "cursor-changed",
		    G_CALLBACK (focus_objtree_item), (gpointer)hg);

  update_c_label(hg);
  gtk_widget_show_all(hg->objtree);
}


static void
objtree_add_columns (typHOE *hg,
		     GtkTreeView  *treeview, 
		     GtkTreeModel *items_model,
		     GtkTreeModel *repeat_model,
		     GtkTreeModel *guide_model
		     )
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* check column */
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled",
		    G_CALLBACK (cell_toggled_check), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_OBJTREE_CHECK));
  
  column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "active", 
						     COLUMN_OBJTREE_CHECK,
						     NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_NUMBER,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Exptime column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_EXP));

  column=gtk_tree_view_column_new_with_attributes ("Exp",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_EXP,
					    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* x times */
  renderer = gtk_cell_renderer_combo_new ();
  g_object_set (renderer,
                "model", repeat_model,
                "text-column", COLUMN_NUMBER_TEXT,
                "has-entry", FALSE,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), (gpointer)hg);
  g_object_set_data (G_OBJECT (renderer), 
  		     "column", GINT_TO_POINTER (COLUMN_OBJTREE_REPEAT));
  column=gtk_tree_view_column_new_with_attributes ("x",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_REPEAT,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_int_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_REPEAT),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Mag column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_MAG));
  column=gtk_tree_view_column_new_with_attributes ("Mag",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_MAG,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_mag_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_MAG);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* MagSrc column */
  renderer = gtk_cell_renderer_text_new ();
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_MAGSRC));
  column=gtk_tree_view_column_new_with_attributes ("Band",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_MAGSRC,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_magsrc_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* SNR column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_SNR));
  column=gtk_tree_view_column_new_with_attributes ("S/N",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_SNR,
						   "background-gdk", 
						   COLUMN_OBJTREE_SNR_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_SNR),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_RA));
  column=gtk_tree_view_column_new_with_attributes ("RA",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_RA,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_RA_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_RA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_RA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);



  /* Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_DEC));
  column=gtk_tree_view_column_new_with_attributes ("Dec",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_DEC,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_DEC_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* EQUINOX column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_EQUINOX));
  column=gtk_tree_view_column_new_with_attributes ("Equinox",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_EQUINOX,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_EQUINOX),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* PA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_PA));
  column=gtk_tree_view_column_new_with_attributes ("ImRPA",
						   renderer,
						   "text",
						   COLUMN_OBJTREE_PA,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_PA),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* GS column */
  renderer = gtk_cell_renderer_toggle_new ();
  //g_signal_connect (renderer, "toggled",
  //		    G_CALLBACK (cell_toggled_check), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
		      GINT_TO_POINTER (COLUMN_OBJTREE_GS));
  
  column = gtk_tree_view_column_new_with_attributes ("GS",
						     renderer,
						     "active", 
						     COLUMN_OBJTREE_GS,
						     NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  

  /* Guide Mode */
  renderer = gtk_cell_renderer_combo_new ();
  g_object_set (renderer,
                "model", guide_model,
                "text-column", COLUMN_NUMBER_TEXT,
                "has-entry", FALSE,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), (gpointer)hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_GUIDE));
  column=gtk_tree_view_column_new_with_attributes ("Guide",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_GUIDE,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_int_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_GUIDE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Setup */
  {
    gint i_use;
    gchar tmp_label[10];
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      renderer = gtk_cell_renderer_toggle_new ();
      g_signal_connect (renderer, "toggled",
      			G_CALLBACK (cell_toggled), hg);
      g_object_set_data (G_OBJECT (renderer), "column", 
			 GINT_TO_POINTER (COLUMN_OBJTREE_SETUP1+i_use));
      sprintf(tmp_label,"S%d",i_use+1);
      
      column = gtk_tree_view_column_new_with_attributes (tmp_label,
							 renderer,
							 "active", 
							 COLUMN_OBJTREE_SETUP1+i_use,
							 NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    }
  }

  /* Rise column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_RISE));
  column=gtk_tree_view_column_new_with_attributes ("Rise",
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_RISE,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_RISE_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_rise_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_RISE);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Transit column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_TRANSIT));
  column=gtk_tree_view_column_new_with_attributes ("Trans",
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_TRANSIT,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_TRANSIT_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_transit_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_TRANSIT);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Set column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_SET));
  column=gtk_tree_view_column_new_with_attributes ("Set",
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_SET,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_SET_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_set_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_SET);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Note column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_OBJTREE_NOTE));
  column=gtk_tree_view_column_new_with_attributes ("Note",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_NOTE,
					    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

}

static GtkTreeModel *
create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_OBJTREE_COLUMNS, 
			      G_TYPE_BOOLEAN, // check
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
			      G_TYPE_INT,     // Exp
                              G_TYPE_INT,     // Repeat
                              G_TYPE_BOOLEAN, // GS(flag)
                              G_TYPE_DOUBLE,  // Mag
                              G_TYPE_INT,     // MagSrc
                              G_TYPE_DOUBLE,  // SNR
			      GDK_TYPE_COLOR,   //color
                              G_TYPE_DOUBLE,  // ra
			      GDK_TYPE_COLOR,   //color
			      G_TYPE_DOUBLE,  // dec
			      GDK_TYPE_COLOR,   //color
                              G_TYPE_DOUBLE,  // equinox
			      G_TYPE_INT,     // horizon
			      G_TYPE_DOUBLE,  // rise
			      GDK_TYPE_COLOR,   //color
			      G_TYPE_DOUBLE,  // transit
			      GDK_TYPE_COLOR,   //color
			      G_TYPE_DOUBLE,  // set
			      GDK_TYPE_COLOR,   //color
                              G_TYPE_DOUBLE,  // PA
                              G_TYPE_INT,     // Guide
                              G_TYPE_BOOLEAN, // Set1
                              G_TYPE_BOOLEAN, // Set2
                              G_TYPE_BOOLEAN, // Set3
                              G_TYPE_BOOLEAN, // Set4
                              G_TYPE_BOOLEAN, // Set5
			      G_TYPE_STRING   // note
			      );  

  //gtk_list_store_set_column_types (GTK_LIST_STORE (model), 1, 
  //			   (GType []){ G_TYPE_STRING }); // NOTE
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (model, &iter);
    objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  get_total_basic_exp(hg);
  
  return GTK_TREE_MODEL (model);
}


void objtree_update_item(typHOE *hg, 
			 GtkTreeModel *model, 
			 GtkTreeIter iter, 
			 gint i_list)
{
  gchar tmp[128];
  gint i;
  GtkTreePath *path;
  struct ln_zonedate zonedate;
  gint mod_rise, mod_set, obj_mod;

  mod_set=hg->sun.s_set.hours*60+hg->sun.s_set.minutes;
  mod_rise=hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes;

  // Check
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_CHECK,
		     hg->obj[i_list].check_sm, 
		     -1);
  // Number
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_NUMBER,
		      i_list+1,
		      -1);

  // Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_NAME,
		      hg->obj[i_list].name,
		      -1);
  // Exp
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_EXP, 
		     hg->obj[i_list].exp, 
		     -1);
  // Repeat
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_REPEAT,
		     hg->obj[i_list].repeat, 
		     -1);
  // GS(flag)
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_GS,
		     hg->obj[i_list].gs.flag, 
		     -1);
  // Mag
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_MAG, 
		     hg->obj[i_list].mag, 
		     -1);
  // SNR
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_SNR, 
		     hg->obj[i_list].snr, 
		     -1);

  if(hg->obj[i_list].sat){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SNR_COL,&color_pink2,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SNR_COL,NULL,
			-1);
  }

  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_RA, 
		     hg->obj[i_list].ra, 
		     -1);
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_DEC, 
		     hg->obj[i_list].dec, 
		     -1);
  if(hg->obj[i_list].i_nst>=0){
    switch(hg->skymon_mode){
    case SKYMON_SET:
      if(hg->nst[hg->obj[i_list].i_nst].s_fl!=0){
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			    COLUMN_OBJTREE_RA_COL,&color_pink,
			    COLUMN_OBJTREE_DEC_COL,&color_pink,
			    -1);
      }
      else{
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			    COLUMN_OBJTREE_RA_COL,&color_pale,
			    COLUMN_OBJTREE_DEC_COL,&color_pale,
			    -1);
      }
      break;

    default:
      if(hg->nst[hg->obj[i_list].i_nst].c_fl!=0){
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			    COLUMN_OBJTREE_RA_COL,&color_pink,
			    COLUMN_OBJTREE_DEC_COL,&color_pink,
			    -1);
      }
      else{
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			    COLUMN_OBJTREE_RA_COL,&color_pale,
			    COLUMN_OBJTREE_DEC_COL,&color_pale,
			    -1);
      }
      break;
    }
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RA_COL,&color_black,
			COLUMN_OBJTREE_DEC_COL,&color_black,
			-1);
  }
  // EQUINOX
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_EQUINOX,
		     hg->obj[i_list].equinox,
		     -1);
  // HORIZON
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_HORIZON, 
		     hg->obj[i_list].horizon, 
		     -1);
  // Rise
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_RISE, 
		     hg->obj[i_list].rise, 
		     -1);
  ln_get_local_date(hg->obj[i_list].rise,&zonedate, hg->obs_timezone/60);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RISE_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RISE_COL,&color_pink,
			-1);
  }
  // Transit
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_TRANSIT,
		      hg->obj[i_list].transit,
		      -1);
  ln_get_local_date(hg->obj[i_list].transit,&zonedate, hg->obs_timezone/60);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_TRANSIT_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_TRANSIT_COL,&color_pink,
			-1);
  }
  // Set
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_SET,
		      hg->obj[i_list].set,
		      -1);
  ln_get_local_date(hg->obj[i_list].set,&zonedate, hg->obs_timezone/60);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SET_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SET_COL,&color_pink,
			-1);
  }
  // PA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_PA, 
		     hg->obj[i_list].pa, 
		     -1);
  // Guide
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_GUIDE,
		     hg->obj[i_list].guide, 
		     -1);
  // Setup
  {
    gint i_use;
   
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJTREE_SETUP1+i_use,
			 hg->obj[i_list].setup[i_use], 
			 -1);
    }
  }
  // NOTE
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_NOTE, 
		     hg->obj[i_list].note, 
		     -1);
  
}


void objtree_update_radec_item(typHOE *hg, 
			       GtkTreeModel *model, 
			       GtkTreeIter iter, 
			       gint i_list)
{
  gchar tmp[128];
  gint i;
  GtkTreePath *path;
  struct ln_zonedate zonedate;
  gint mod_rise, mod_set, obj_mod;

  if(hg->obj[i_list].i_nst>=0){
    // RA
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_RA, 
		       hg->obj[i_list].ra, 
		       -1);
    // DEC
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_DEC, 
		       hg->obj[i_list].dec, 
		       -1);
    if(hg->obj[i_list].i_nst>=0){
      switch(hg->skymon_mode){
      case SKYMON_SET:
	if(hg->nst[hg->obj[i_list].i_nst].s_fl!=0){
	  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			      COLUMN_OBJTREE_RA_COL,&color_pink,
			      COLUMN_OBJTREE_DEC_COL,&color_pink,
			      -1);
	}
	else{
	  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			      COLUMN_OBJTREE_RA_COL,&color_pale,
			      COLUMN_OBJTREE_DEC_COL,&color_pale,
			      -1);
      }
	break;
	
      default:
	if(hg->nst[hg->obj[i_list].i_nst].c_fl!=0){
	  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			      COLUMN_OBJTREE_RA_COL,&color_pink,
			      COLUMN_OBJTREE_DEC_COL,&color_pink,
			      -1);
	}
	else{
	  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			      COLUMN_OBJTREE_RA_COL,&color_pale,
			      COLUMN_OBJTREE_DEC_COL,&color_pale,
			      -1);
	}
	break;
      }
    }
    else{
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_OBJTREE_RA_COL,&color_black,
			  COLUMN_OBJTREE_DEC_COL,&color_black,
			  -1);
    }
  }
}


static void
cell_edited (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
  typHOE *hg = (typHOE *)data;
  //GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column)
    {
    case COLUMN_OBJTREE_NAME:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;
	
	g_free(hg->obj[i].name);
	hg->obj[i].name=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].name, -1);
	
      }
      break;

    case COLUMN_OBJTREE_EXP:
      {
        gint i;
	gint old_exp;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;
	
	old_exp = hg->obj[i].exp;
        hg->obj[i].exp = atoi (new_text);
	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].exp, -1);

	if(old_exp!=hg->obj[i].exp){
	  hg->obj[i].snr=-1;
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter,COLUMN_OBJTREE_SNR,
			      hg->obj[i].snr, -1);
	}

	get_total_basic_exp(hg);
      }
      break;

    case COLUMN_OBJTREE_REPEAT:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;
        hg->obj[i].repeat = atoi (new_text+1);

        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
	                   hg->obj[i].repeat, -1);
	get_total_basic_exp(hg);
      }
      break;

    case COLUMN_OBJTREE_MAG:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	if(fabs(hg->obj[i].mag-(gdouble)g_strtod(new_text,NULL))>0.1){	
	  hg->obj[i].magdb_used=0;
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
			      COLUMN_OBJTREE_MAGSRC,
			      hg->obj[i].magdb_used, -1);
	  hg->obj[i].mag=(gdouble)g_strtod(new_text,NULL);	
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			      hg->obj[i].mag, -1);
	  hg->obj[i].snr=-1;	
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJTREE_SNR,
			      hg->obj[i].snr, -1);
	}
      }
      break;

    case COLUMN_OBJTREE_RA:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].ra=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].ra, -1);
      }
      break;

    case COLUMN_OBJTREE_DEC:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].dec=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].dec, -1);
      }
      break;

    case COLUMN_OBJTREE_EQUINOX:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].equinox=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].equinox, -1);
      }
      break;

    case COLUMN_OBJTREE_PA:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].pa=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].pa, -1);
      }
      break;

    case COLUMN_OBJTREE_GUIDE:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	if(!strcmp(new_text,"No")){
	  hg->obj[i].guide=NO_GUIDE;
	}
	else if(!strcmp(new_text,"AG")){
	  hg->obj[i].guide=AG_GUIDE;
	}
	else if(!strcmp(new_text,"SV")){
	  hg->obj[i].guide=SV_GUIDE;
	}
	else{
	  hg->obj[i].guide=SVSAFE_GUIDE;
	}

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			    hg->obj[i].guide, -1);
      }
      break;

    case COLUMN_OBJTREE_NOTE:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	g_free(hg->obj[i].note);
	hg->obj[i].note=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].note, -1);
	
      }
      break;

    }


  gtk_tree_path_free (path);
}

static void
cell_toggled_check (GtkCellRendererText *cell,
		    const gchar         *path_string,
		    gpointer             data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean fixed;
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gint i;


  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
  i--;

  hg->obj[i].check_sm ^= 1;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJTREE_CHECK, hg->obj[i].check_sm, -1);
  
  gtk_tree_path_free (path);
}


static void
cell_toggled (GtkCellRendererText *cell,
             const gchar         *path_string,
             gpointer             data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean fixed;
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gint i_use=column-COLUMN_OBJTREE_SETUP1;
  gint i;


  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
  i--;

  hg->obj[i].setup[i_use] ^= 1;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJTREE_SETUP1+i_use, hg->obj[i].setup[i_use], -1);

  gtk_tree_path_free (path);
}


void objtree_int_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_OBJTREE_GUIDE:
    switch(value){
    case NO_GUIDE:
      str=g_strdup_printf("No");
      break;
    case AG_GUIDE:
      str=g_strdup_printf("AG");
      break;
    case SV_GUIDE:
      str=g_strdup_printf("SV");
      break;
    case SVSAFE_GUIDE:
      str=g_strdup_printf("SV[Safe]");
      break;
    }
    break;

  case COLUMN_OBJTREE_REPEAT:
    str=g_strdup_printf("x%2d",value);
    break;

  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}



void objtree_double_cell_data_func(GtkTreeViewColumn *col , 
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
  case COLUMN_OBJTREE_SNR:
    if(value>0)
      str=g_strdup_printf("%.1lf",value);
    else
      str=NULL;
    break;

  case COLUMN_OBJTREE_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_OBJTREE_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_OBJTREE_EQUINOX:
    str=g_strdup_printf("%7.2lf",value);
    break;
  case COLUMN_OBJTREE_PA:
    str=g_strdup_printf("%+.1lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void objtree_mag_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  gchar *str=NULL;
  gint i, hits;
  typHOE *hg=(typHOE *) user_data;
  
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  if(fabs(hg->obj[i].mag)>99){
    str=NULL;
  }
  else{
    switch(hg->obj[i].magdb_used){
    case MAGDB_TYPE_GSC:
      hits=hg->obj[i].magdb_gsc_hits;
      break;

    case MAGDB_TYPE_PS1:
      hits=hg->obj[i].magdb_ps1_hits;
      break;

    case MAGDB_TYPE_SDSS:
      hits=hg->obj[i].magdb_sdss_hits;
      break;

    case MAGDB_TYPE_GAIA:
      hits=hg->obj[i].magdb_gaia_hits;
      break;

    case MAGDB_TYPE_2MASS:
      hits=hg->obj[i].magdb_2mass_hits;
      break;

    case MAGDB_TYPE_SIMBAD:
      hits=hg->obj[i].magdb_simbad_hits;
      break;

    default:
      hits=1;
      break;
    }

    if(abs(hits)>=2){
      str=g_strdup_printf("%.1lf*",hg->obj[i].mag);
    }
    else{
      str=g_strdup_printf("%.1lf",hg->obj[i].mag);
    }
  }

  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}


void objtree_magsrc_cell_data_func(GtkTreeViewColumn *col , 
				   GtkCellRenderer *renderer,
				   GtkTreeModel *model, 
				   GtkTreeIter *iter,
				   gpointer user_data)
{
  gchar *str=NULL;
  gint i;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  str=get_band_name(hg, i);

  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}

void objtree_rise_cell_data_func(GtkTreeViewColumn *col , 
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model, 
				 GtkTreeIter *iter,
				 gpointer user_data)
{
  //const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;
  struct ln_zonedate zonedate;
  gint i;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  if(hg->obj[i].horizon==0){
    ln_get_local_date (hg->obj[i].rise, &zonedate, hg->obs_timezone/60);
    str=g_strdup_printf("%2d:%02d",zonedate.hours,zonedate.minutes);
  }
  else{
    str=g_strdup("---");
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void objtree_set_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  //const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;
  struct ln_zonedate zonedate;
  gint i;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  if(hg->obj[i].horizon!=-1){
    ln_get_local_date (hg->obj[i].transit, &zonedate, hg->obs_timezone/60);
    str=g_strdup_printf("%2d:%02d",zonedate.hours,zonedate.minutes);
  }
  else{
    str=g_strdup("---");
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void objtree_transit_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  //const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;
  struct ln_zonedate zonedate;
  gint i;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  if(hg->obj[i].horizon==0){
    ln_get_local_date (hg->obj[i].set, &zonedate, hg->obs_timezone/60);
    str=g_strdup_printf("%2d:%02d",zonedate.hours,zonedate.minutes);
  }
  else{
    str=g_strdup("---");
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

static GtkTreeModel *
create_repeat_model (void)
{
#define N_REPEAT 20
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_NUMBER_COLUMNS, 
			      G_TYPE_STRING, 
			      G_TYPE_INT);

  /* add numbers */
  for (i = 0; i < N_REPEAT; i++)
    {
      char str[4];
      
      sprintf(str,"x%2d",i+1);
      
      gtk_list_store_append (model, &iter);
      
      gtk_list_store_set (model, &iter,
                          COLUMN_NUMBER_TEXT, str,
                          -1);
    }
  
  return GTK_TREE_MODEL (model);
  
#undef N_REPEAT
}


static GtkTreeModel *
create_guide_model (void)
{
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (2, 
			      G_TYPE_STRING,
			      G_TYPE_INT);

  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 
		      0, "No",
		      1, NO_GUIDE,
		      -1);
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 
		      0, "AG", 
		      1, AG_GUIDE,
		      -1);
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 
		      0, "SV",  
		      1, SV_GUIDE,
		      -1);
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 
		      0, "SV[Safe]",
		      1, SVSAFE_GUIDE,
		      -1);

  return GTK_TREE_MODEL (model);

}




void
add_item_objtree (typHOE *hg)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  gint i,i_list,i_use, i_plan;
  OBJpara tmp_obj;
  GtkTreePath *path;

  if(hg->i_max>=MAX_OBJECT) return;

  i=hg->i_max;

  tmp_obj.name=g_strdup(hg->addobj_name);
  
  tmp_obj.check_sm=FALSE;
  tmp_obj.exp=DEF_EXP;
  tmp_obj.mag=100;
  tmp_obj.snr=-1;
  tmp_obj.sat=FALSE;
  ObjMagDB_Init(&tmp_obj);
  tmp_obj.repeat=1;
  tmp_obj.guide=SV_GUIDE;
  tmp_obj.pa=0;
  tmp_obj.i_nst=-1;
  tmp_obj.gs.flag=FALSE;
  tmp_obj.gs.name=NULL;

  tmp_obj.ra=hg->addobj_ra;
  tmp_obj.dec=hg->addobj_dec;
  tmp_obj.equinox=2000.0;
  if(hg->addobj_votype){
    if(hg->addobj_magsp)
      tmp_obj.note=g_strdup_printf("%s, %s",hg->addobj_votype,hg->addobj_magsp);
    else
      tmp_obj.note=g_strdup_printf("%s, mag=unknown",hg->addobj_votype);
  }
  else{
    tmp_obj.note=g_strdup("(added via dialog)");
  }
  
  tmp_obj.setup[0]=TRUE;
  for(i_use=1;i_use<MAX_USESETUP;i_use++){
    tmp_obj.setup[i_use]=FALSE;
  }
  
  for(i_list=hg->i_max;i_list>i;i_list--){
    hg->obj[i_list]=hg->obj[i_list-1];
  }
  
  hg->i_max++;
  
  hg->obj[i]=tmp_obj;
  
  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
  objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i);


  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_NUMBER, i_list+1, -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
      if(hg->plan[i_plan].obj_i>=i){
	hg->plan[i_plan].obj_i++;
      }
    }
  }
  
  get_total_basic_exp(hg);
  recalc_rst(hg);
}


void up_item_objtree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if(i>0){
      tmp_obj=hg->obj[i-1];
      hg->obj[i-1]=hg->obj[i];
      hg->obj[i]=tmp_obj;

      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
      recalc_rst(hg);
      objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i-1);
      objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
    }
    
    //make_obj_list(hg,FALSE);

    get_total_basic_exp(hg);
    gtk_tree_path_free (path);
  }
}

void down_item_objtree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if(i<hg->i_max-1){
      tmp_obj=hg->obj[i];
      hg->obj[i]=hg->obj[i+1];
      hg->obj[i+1]=tmp_obj;

      gtk_tree_path_next (path);
      gtk_tree_selection_select_path(selection, path);
    }
    
    //make_obj_list(hg,FALSE);

    gtk_tree_path_free (path);
  }
  recalc_rst(hg);
}


void remove_item_objtree (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list,j, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	
    for(i_list=i;i_list<hg->i_max;i_list++){
      hg->obj[i_list]=hg->obj[i_list+1];
    }

    hg->i_max--;
    
    if(!gtk_tree_model_get_iter_first(model, &iter)) return;
    
    for(i_list=0;i_list<hg->i_max;i_list++){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJTREE_NUMBER, i_list+1, -1);
      if(!gtk_tree_model_iter_next(model, &iter)) break;
    }

    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	if(hg->plan[i_plan].obj_i==i){
	  hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
	  hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
	  hg->plan[i_plan].txt=g_strdup("### (The object was removed from the list.) ###");
	  hg->plan[i_plan].comment=g_strdup(" (The object was removed from the list.) ");
	  hg->plan[i_plan].time=0;
	  hg->plan[i_plan].setup=0;
	  hg->plan[i_plan].repeat=1;
	  hg->plan[i_plan].slit_or=FALSE;
	  hg->plan[i_plan].slit_width=0;
	  hg->plan[i_plan].slit_length=0;
	  
	  hg->plan[i_plan].obj_i=0;
	  hg->plan[i_plan].exp=0;
	  
	  hg->plan[i_plan].omode=PLAN_OMODE_FULL;
	  hg->plan[i_plan].guide=SV_GUIDE;
	  
	  hg->plan[i_plan].cmode=PLAN_CMODE_FULL;
	  hg->plan[i_plan].i2_pos=PLAN_I2_IN;
	  
	  hg->plan[i_plan].daytime=FALSE;
	}
	else if (hg->plan[i_plan].obj_i>i){
	  hg->plan[i_plan].obj_i--;
	}
      }
    }

    recalc_rst(hg);
  }
}


void  wwwdb_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gchar *tmp;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gint fcdb_type_old;
  gchar *c=NULL, *cp, *cpp;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);

    object.ra=ra_to_deg(hg->obj[i].ra);
    object.dec=dec_to_deg(hg->obj[i].dec);

    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].equinox),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->wwwdb_mode){
    case WWWDB_SIMBAD:
      tmp=g_strdup_printf(SIMBAD_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_NED:
      tmp=g_strdup_printf(NED_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_DR8:
      tmp=g_strdup_printf(DR8_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_DR14:
      tmp=g_strdup_printf(DR14_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_MAST:
      tmp=g_strdup_printf(MAST_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  (hobject_prec.dec.neg) ? "%2D" : "%2B", 
			  fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_MASTP:
      tmp=g_strdup_printf(MASTP_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  (hobject_prec.dec.neg) ? "%2D" : "%2B", 
			  fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_KECK:
      tmp=g_strdup_printf(KECK_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  ln_dms_to_deg(&hobject_prec.dec));
      break;

    case WWWDB_GEMINI:
      tmp=g_strdup_printf(GEMINI_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  ln_dms_to_deg(&hobject_prec.dec));
      break;

    case WWWDB_IRSA:
      tmp=g_strdup_printf(IRSA_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_SPITZER:
      tmp=g_strdup_printf(SPITZER_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  ln_dms_to_deg(&hobject_prec.dec));
      break;
      
    case WWWDB_CASSIS:
      tmp=g_strdup_printf(CASSIS_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  ln_dms_to_deg(&hobject_prec.dec));
      break; 
    case WWWDB_SSLOC:
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(SSLOC_URL,
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
	tmp=g_strdup_printf(SSLOC_URL,
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
	tmp=g_strdup_printf(SSLOC_URL,
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
    case WWWDB_RAPID:
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(RAPID_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	tmp=g_strdup_printf(RAPID_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else{
	tmp=g_strdup_printf(RAPID_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%26",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      break;
    case WWWDB_MIRSTD:
     if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(MIRSTD_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	tmp=g_strdup_printf(MIRSTD_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else{
	tmp=g_strdup_printf(MIRSTD_URL,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%26",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      break;

    case WWWDB_SMOKA:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=FCDB_TYPE_WWWDB_SMOKA;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_SMOKA "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_SMOKA "/");


#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;

    case WWWDB_HST:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=FCDB_TYPE_WWWDB_HST;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_HST);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_HST_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_HST "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_HST "/");

#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;

    case WWWDB_ESO:
      fcdb_type_old=hg->fcdb_type;
      hg->fcdb_type=FCDB_TYPE_WWWDB_ESO;

      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_ESO);

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_ESO_PATH);

      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_HTML,NULL);

      hg->fcdb_d_ra0=ln_hms_to_deg(&hobject_prec.ra);
      hg->fcdb_d_dec0=ln_dms_to_deg(&hobject_prec.dec);

      fcdb_dl(hg);
      hg->fcdb_type=fcdb_type_old;
      str_replace(hg->fcdb_file,
		  "href=\"/",
		  "href=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "HREF=\"/",
		  "HREF=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "src=\"/",
		  "src=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "SRC=\"/",
		  "SRC=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "action=\"/",
		  "action=\"http://" FCDB_HOST_ESO "/");
      str_replace(hg->fcdb_file,
		  "ACTION=\"/",
		  "ACTION=\"http://" FCDB_HOST_ESO "/");

#ifdef USE_WIN32      
      tmp=g_strdup(hg->fcdb_file);
#elif defined(USE_OSX)
      tmp=g_strconcat("open ", hg->fcdb_file, NULL);
#else
      tmp=g_strconcat("\"",hg->fcdb_file,"\"",NULL);
#endif
      break;
    }

#ifndef USE_WIN32
    if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
  }
#endif

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
#endif
    if(tmp) g_free(tmp); 
  }
}


void do_update_exp(typHOE *hg){
  int i_list;
  gchar tmp[64];
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  for(i_list=0;i_list<hg->i_max;i_list++){

    if(fabs(hg->obj[i_list].mag)<99){
      hg->obj[i_list].exp=(gint)(pow(2.5119,
				     (hg->obj[i_list].mag - hg->expmag_mag)) 
				 * hg->expmag_exp);
      if(hg->obj[i_list].exp<1) hg->obj[i_list].exp=1;
    }
    if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  }
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(fabs(hg->obj[i_list].mag)<99){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJTREE_EXP, hg->obj[i_list].exp, -1);
    }
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}




void export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->def_guide){
    case NO_GUIDE:
      hg->obj[i_list].guide=NO_GUIDE;
      break;
    case AG_GUIDE:
      hg->obj[i_list].guide=AG_GUIDE;
      break;
    case SV_GUIDE:
      hg->obj[i_list].guide=SV_GUIDE;
      break;
    case SVSAFE_GUIDE:
      hg->obj[i_list].guide=SVSAFE_GUIDE;
      break;
    }

    hg->obj[i_list].pa=hg->def_pa;
    hg->obj[i_list].exp=hg->def_exp;
  }
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_GUIDE, hg->obj[i_list].guide, 
		       COLUMN_OBJTREE_PA, hg->obj[i_list].pa, 
		       COLUMN_OBJTREE_EXP, hg->obj[i_list].exp, 
		       -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}


void do_plot(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagPlot){
    gdk_window_raise(gtk_widget_get_window(hg->plot_main));
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,hg);
  }
  else{
    create_plot_dialog(hg);
  }
  
}



static void
focus_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (model, &iter);
      //i = gtk_tree_path_get_indices (path)[0];
      gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
      i--;
      hg->plot_i=i;
      

      gtk_tree_path_free (path);
    }
  
  if(flagPlot){
    //hg->plot_target=PLOT_OBJTREE;
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,hg);
  }

  if(flagSkymon){
    switch(hg->skymon_mode){
    case SKYMON_CUR:
      draw_skymon_cairo(hg->skymon_dw,hg);
      break;

    case SKYMON_SET:
      skymon_set_and_draw(NULL, (gpointer)hg);
      break;
    }
  }
}


void plot2_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    hg->plot_i=i;

    //hg->plot_target=PLOT_OBJTREE;
    do_plot(widget,(gpointer)hg);

    gtk_tree_path_free (path);
  }
}


void etc_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    hg->etc_i=i;

    hg->etc_exptime=hg->obj[i].exp;
    if(fabs(hg->obj[i].mag)<99){
      hg->etc_mag=hg->obj[i].mag;
    }

    hg->etc_mode=ETC_OBJTREE;
    do_etc(NULL, (gpointer)hg);
    hg->etc_mode=ETC_MENU;

    gtk_tree_path_free (path);
  }
}


static void ok_addobj(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  add_item_objtree(hg);
}

static void addobj_simbad_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->addobj_type=FCDB_TYPE_SIMBAD;
  addobj_dl(hg);
}

static void addobj_ned_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->addobj_type=FCDB_TYPE_NED;
  addobj_dl(hg);
}

void addobj_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gchar *tgt;
  gchar *tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  tgt=make_simbad_id(hg->addobj_name);

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_SIMBAD_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
    }
    else{
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
    }
    break;

  case FCDB_TYPE_NED:
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_NED_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_NED);
    break;
  }
  g_free(tgt);

  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Query to the database");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event", delete_fcdb, (gpointer)hg);
  
#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    label=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    label=gtk_label_new("Searching objects in NED ...");
    break;
  }

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->fcdb_file);
  
  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    hg->plabel=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    hg->plabel=gtk_label_new("Searching objects in NED ...");
    break;
  }
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    cancel_fcdb, 
		    (gpointer)hg);
  
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
  gtk_widget_destroy(dialog);

  flag_getFCDB=FALSE;

  
  addobj_vo_parse(hg);

  if(hg->addobj_voname){
    tmp=g_strdup_printf("%09.2lf",hg->addobj_ra);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+010.2lf",hg->addobj_dec);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec),tmp);
    g_free(tmp);

    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"%s\" (%s) in SIMBAD",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"%s\" (%s) in NED",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;
    }
    gtk_label_set_text(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  else{
    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is not found in SIMBAD",
			  hg->addobj_name); 
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is not found in NED",
			  hg->addobj_name); 
      break;
    }
    gtk_label_set_text(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  
}

void addobj_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox,
    *spinner, *table, *entry, *bar;
  GtkAdjustment *adj;
  typHOE *hg;
  GSList *fcdb_group=NULL; 
  gboolean rebuild_flag=FALSE;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  hg->addobj_ra=0;
  hg->addobj_dec=0;
  if(hg->addobj_votype) g_free(hg->addobj_votype);
  hg->addobj_votype=NULL;
  if(hg->addobj_magsp) g_free(hg->addobj_magsp);
  hg->addobj_magsp=NULL;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Add Object");
  my_signal_connect(dialog,"delete-event", gtk_main_quit, NULL);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  
  label = gtk_label_new ("Object Name");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE, FALSE, 0);
  my_signal_connect (entry, "changed", cc_get_entry, &hg->addobj_name);
  gtk_entry_set_text(GTK_ENTRY(entry), "(New Object)");
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),30);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("SIMBAD", "edit-search");
#else
  button=gtkut_button_new_from_stock("SIMBAD", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_simbad_query, (gpointer)hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("NED", "edit-search");
#else
  button=gtkut_button_new_from_stock("NED", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_ned_query, (gpointer)hg);

  bar = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  hg->addobj_label = gtk_label_new ("Input Object Name to be added & resolve its coordinate in the database.");
  gtk_misc_set_alignment (GTK_MISC (hg->addobj_label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_label,FALSE, FALSE, 0);


  bar = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);
 
  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new ("             RA(2000)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_ra = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_ra,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra), "000000.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_ra),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_ra),12);
  my_signal_connect (hg->addobj_entry_ra, "changed", 
		     cc_get_entry_double, &hg->addobj_ra);
  
  label = gtk_label_new ("    Dec(2000)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_dec = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_dec,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec), "000000.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_dec),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_dec),12);
  my_signal_connect (hg->addobj_entry_dec, "changed", 
		     cc_get_entry_double, &hg->addobj_dec);
  

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", gtk_main_quit, NULL);

#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("Add Object","list-add");
#else
  button=gtkut_button_new_from_stock("Add Object",GTK_STOCK_ADD);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
  		    ok_addobj, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void strchg(gchar *buf, const gchar *str1, const gchar *str2)
{
  gchar tmp[BUFFSIZE+1];
  gchar *p;

  while ((p = strstr(buf, str1)) != NULL) {
    *p = '\0'; 
    p += strlen(str1);	
    strcpy(tmp, p);
    strcat(buf, str2);
    strcat(buf, tmp);
  }
}


void str_replace(gchar *in_file, const gchar *str1, const gchar *str2){
  gchar buf[BUFFSIZE +1];
  FILE *fp_r, *fp_w;
  gchar *out_file;

  fp_r=fopen(in_file,"r");
  out_file=g_strconcat(in_file,"_tmp",NULL);
  fp_w=fopen(out_file,"w");

  while(!feof(fp_r)){
    if((fgets(buf,BUFFSIZE,fp_r))==NULL){
      break;
    }
    else{
      strchg(buf,str1,str2);
      fprintf(fp_w,"%s",buf);
    }
  }

  fclose(fp_r);
  fclose(fp_w);

  unlink(in_file);
  rename(out_file,in_file);

  if(out_file) g_free(out_file);
}

gchar *make_simbad_id(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_obj, i_tgt;

  if((tgt_name=(gchar *)g_malloc(sizeof(gchar)*(strlen(obj_name)*3+1)))
     ==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }

  i_tgt=0;

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(obj_name[i_obj]==0x20){
      tgt_name[i_tgt]='%';
      i_tgt++;
      tgt_name[i_tgt]='2';
      i_tgt++;
      tgt_name[i_tgt]='0';
      i_tgt++;
    }
    else if(obj_name[i_obj]==0x2b){
      tgt_name[i_tgt]='%';
      i_tgt++;
      tgt_name[i_tgt]='2';
      i_tgt++;
      tgt_name[i_tgt]='b';
      i_tgt++;
    }    
    else{
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);
  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}

void cc_search_text (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->tree_search_text) g_free(hg->tree_search_text);
  hg->tree_search_text=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));

  hg->tree_search_i=0;
  hg->tree_search_imax=0;

  gtk_label_set_text(GTK_LABEL(hg->tree_search_label),"      ");
}

void search_item (GtkWidget *widget, gpointer data)
{
  gint i;
  gchar *label_text;
  typHOE *hg = (typHOE *)data;
  gchar *up_text1, *up_text2, *up_obj1, *up_obj2;

  if(!hg->tree_search_text) return;

  if(strlen(hg->tree_search_text)<1){
    hg->tree_search_imax=0;
    hg->tree_search_i=0;

    gtk_label_set_text(GTK_LABEL(hg->tree_search_label),"      ");
    return;
  }

  if(hg->tree_search_imax==0){
    up_text1=g_ascii_strup(hg->tree_search_text, -1);
    up_text2=strip_spc(up_text1);
    g_free(up_text1);
    for(i=0; i<hg->i_max; i++){
      up_obj1=g_ascii_strup(hg->obj[i].name, -1);
      up_obj2=strip_spc(up_obj1);
      g_free(up_obj1);
      if(g_strstr_len(up_obj2, -1, up_text2)!=NULL){
	hg->tree_search_iobj[hg->tree_search_imax]=i;
	hg->tree_search_imax++;
      }
      else if(hg->obj[i].note){
	g_free(up_obj2);
	up_obj1=g_ascii_strup(hg->obj[i].note, -1);
	up_obj2=strip_spc(up_obj1);
	g_free(up_obj1);
	if(g_strstr_len(up_obj2, -1, up_text2)!=NULL){
	  hg->tree_search_iobj[hg->tree_search_imax]=i;
	  hg->tree_search_imax++;
	}
      }
      g_free(up_obj2);
    }
    g_free(up_text2);
  }
  else{
    hg->tree_search_i++;
    if(hg->tree_search_i>=hg->tree_search_imax) hg->tree_search_i=0;
  }

  {
    if(hg->tree_search_imax!=0){
      label_text=g_strdup_printf("%d/%d   ",
				 hg->tree_search_i+1,
				 hg->tree_search_imax);

      {
	gint i_list;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
	GtkTreePath *path;
	GtkTreeIter  iter;

	path=gtk_tree_path_new_first();
	
	for(i=0;i<hg->i_max;i++){
	  gtk_tree_model_get_iter (model, &iter, path);
	  gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i_list, -1);
	  i_list--;

	  if(i_list==hg->tree_search_iobj[hg->tree_search_i]){
	    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_OBJ);
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
    else{
      label_text=g_strdup_printf("%d/%d   ",
				 hg->tree_search_i,
				 hg->tree_search_imax);
    }
    gtk_label_set_text(GTK_LABEL(hg->tree_search_label),label_text);
    g_free(label_text);
  }
}

void update_c_label (typHOE *hg){
  gchar *tmp;

  if(hg->skymon_mode==SKYMON_CUR){
    gtk_frame_set_label(GTK_FRAME(hg->mode_frame), "Current");
    tmp=g_strdup_printf("%02d/%02d/%04d %02d:%02d %s",
			hg->fr_month,hg->fr_day,hg->fr_year,
			24,0,"HST");
  }
  else{
    gtk_frame_set_label(GTK_FRAME(hg->mode_frame), "Set");
    tmp=g_strdup_printf("%02d/%02d/%04d %02d:%02d %s",
			hg->skymon_month,hg->skymon_day,hg->skymon_year,
			hg->skymon_hour,hg->skymon_min,"HST");
  }
  gtk_label_set_text(GTK_LABEL(hg->mode_label),tmp);
  if(tmp) g_free(tmp);
}

