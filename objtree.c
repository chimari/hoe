//    HDS OPE file Editor
//      objtree.c  --- Edit Obs Plan
//   
//                                           2010.1.27  A.Tajitsu


#include"main.h"    // 設定ヘッダ
#include"version.h"

static void objtree_add_columns();
static GtkTreeModel *create_items_model ();
void objtree_update_item();
static void cell_edited ();
static void cell_toggled_check ();
static void cell_toggled ();
void objtree_int_cell_data_func();
void objtree_double_cell_data_func();
void objtree_rst_cell_data_func();
static GtkTreeModel * create_repeat_model ();
static GtkTreeModel * create_guide_model ();
static void focus_objtree_item();


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
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_MAG),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_MAG);
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
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* EPOCH column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_EPOCH));
  column=gtk_tree_view_column_new_with_attributes ("Epoch",
					    renderer,
					    "text",
					    COLUMN_OBJTREE_EPOCH,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_EPOCH),
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
					  objtree_rst_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_RISE),
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
  column=gtk_tree_view_column_new_with_attributes ("Transit",
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_TRANSIT,
						   "foreground-gdk", 
						   COLUMN_OBJTREE_TRANSIT_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_rst_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_TRANSIT),
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
					  objtree_rst_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_SET),
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
                              G_TYPE_DOUBLE,  // Mag
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
                              G_TYPE_DOUBLE,  // epoch
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
  // Mag
  if(hg->flag_bunnei){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_MAG, 
		       hg->obj[i_list].mag, 
		       -1);
  }
  else{
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_MAG, 
		       -200.0, 
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
  // EPOCH
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_EPOCH,
		     hg->obj[i_list].epoch,
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
  ln_get_local_date(hg->obj[i_list].rise,&zonedate);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RISE_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RISE_COL,&color_gray1,
			-1);
  }
  // Transit
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_TRANSIT,
		      hg->obj[i_list].transit,
		      -1);
  ln_get_local_date(hg->obj[i_list].transit,&zonedate);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_TRANSIT_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_TRANSIT_COL,&color_gray1,
			-1);
  }
  // Set
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJTREE_SET,
		      hg->obj[i_list].set,
		      -1);
  ln_get_local_date(hg->obj[i_list].set,&zonedate);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SET_COL,&color_black,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SET_COL,&color_gray1,
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

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

        hg->obj[i].exp = atoi (new_text);
	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].exp, -1);
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
      }
      break;

    case COLUMN_OBJTREE_MAG:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].mag=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].mag, -1);
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

    case COLUMN_OBJTREE_EPOCH:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	hg->obj[i].epoch=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].epoch, -1);
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
  case COLUMN_OBJTREE_MAG:
    if(value>-100)
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

  case COLUMN_OBJTREE_EPOCH:
    str=g_strdup_printf("%7.2lf",value);
    break;
  case COLUMN_OBJTREE_PA:
    str=g_strdup_printf("%+.1lf",value);
    break;
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


void objtree_rst_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;
  struct ln_zonedate zonedate;
  gint horizon;

  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_HORIZON, &horizon,
		      index, &value,
		      -1);
  
  switch (index) {
  case COLUMN_OBJTREE_RISE:
  case COLUMN_OBJTREE_SET:
    if(horizon==0){
      ln_get_local_date (value, &zonedate);
      str=g_strdup_printf("%2d:%02d",zonedate.hours,zonedate.minutes);
    }
    else{
      str=g_strdup("---");
    }
    break;

  case COLUMN_OBJTREE_TRANSIT:
    if(horizon!=-1){
      ln_get_local_date (value, &zonedate);
      str=g_strdup_printf("%2d:%02d",zonedate.hours,zonedate.minutes);
    }
    else{
      str=g_strdup("---");
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}



void
add_item_objtree (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  gint i,i_list,i_use, i_plan;
  OBJpara tmp_obj;
  GtkTreePath *path;

  if(hg->i_max>=MAX_OBJECT) return;

  if(hg->i_max==0){
    i=hg->i_max;
  }
  else if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);
  }
  else{
    i=hg->i_max;
  }

  tmp_obj.name=g_strdup("(New Object)");
  
  tmp_obj.exp=DEF_EXP;
  tmp_obj.repeat=1;
  tmp_obj.mag=0.0;
  tmp_obj.ra=0.0;
  tmp_obj.dec=0.0;
  tmp_obj.epoch=2000.0;
  tmp_obj.pa=0.0;
  tmp_obj.guide=SV_GUIDE;
  tmp_obj.note=NULL;
  
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
  }
}


void dss_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gdouble ra_0, dec_0;
  gchar tmp[2048];
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  struct ln_hms ra_hms;
  struct ln_dms dec_dms;
  

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if((int)hg->obj[i].epoch!=2000){
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: Object Epoch should be J2000",
		    " ",
		    "       for DSS Quick View.",
		    NULL);
#else
      fprintf(stderr, "Error: Object Epoch should be J2000 for DSS Quick View.");
#endif
      
    return;
    }
    
    ra_0=hg->obj[i].ra;
    ra_hms.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(ra_hms.hours)*10000;
    ra_hms.minutes=(gint)(ra_0/100);
    ra_hms.seconds=ra_0-(gdouble)(ra_hms.minutes)*100;
    
    if(hg->obj[i].dec<0){
      dec_dms.neg=1;
      dec_0=-hg->obj[i].dec;
    }
    else{
      dec_dms.neg=0;
      dec_0=hg->obj[i].dec;
    }
    dec_dms.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(dec_dms.degrees)*10000;
    dec_dms.minutes=(gint)(dec_0/100);
    dec_dms.seconds=dec_0-(gfloat)(dec_dms.minutes)*100;


    sprintf(tmp,DSS_URL,
	    ra_hms.hours,ra_hms.minutes,ra_hms.seconds,
	    (dec_dms.neg) ? "-" : "+", 
	    dec_dms.degrees, dec_dms.minutes,dec_dms.seconds);

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
    
    gtk_tree_path_free (path);
  }
}


void simbad_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gdouble ra_0, dec_0;
  gchar tmp[2048];
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  struct ln_hms ra_hms;
  struct ln_dms dec_dms;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if((int)hg->obj[i].epoch!=2000){
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: Object Epoch should be J2000",
		    " ",
		    "       for SIMBAD Query View.",
		  NULL);
#else
      fprintf(stderr, "Error: Object Epoch should be J2000 for SIMBAD Query View.");
#endif

      return;
    }
  
    ra_0=hg->obj[i].ra;
    ra_hms.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(ra_hms.hours)*10000;
    ra_hms.minutes=(gint)(ra_0/100);
    ra_hms.seconds=ra_0-(gdouble)(ra_hms.minutes)*100;
    
    if(hg->obj[i].dec<0){
      dec_dms.neg=1;
      dec_0=-hg->obj[i].dec;
    }
    else{
      dec_dms.neg=0;
      dec_0=hg->obj[i].dec;
    }
    dec_dms.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(dec_dms.degrees)*10000;
    dec_dms.minutes=(gint)(dec_0/100);
    dec_dms.seconds=dec_0-(gfloat)(dec_dms.minutes)*100;
    
    sprintf(tmp,SIMBAD_URL,
	    ra_hms.hours,ra_hms.minutes,ra_hms.seconds,
	    (dec_dms.neg) ? "-" : "+", 
	  dec_dms.degrees, dec_dms.minutes,dec_dms.seconds);
    
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
    
    gtk_tree_path_free (path);
  }
}



void do_update_exp(GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;
  int i_list;
  gchar tmp[64];
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  if(!hg->flag_bunnei)  return;

  for(i_list=0;i_list<hg->i_max;i_list++){

    hg->obj[i_list].exp=(gint)(pow(2.5,(hg->obj[i_list].mag - 8.0)) 
      * hg->exp8mag);

    if(hg->flag_secz){
      hg->obj[i_list].exp=hg->obj[i_list].exp+
	hg->obj[i_list].exp*(1/sin(hg->obj[i_list].c_el/180*3.141592) -1)
	*hg->secz_factor;
    }

    if(hg->obj[i_list].exp<1) hg->obj[i_list].exp=1;

    if(!gtk_tree_model_get_iter_first(model, &iter)) return;
    
    //sprintf(tmp,"%d",hg->obj[i_list].exp);
    //gtk_entry_set_text(GTK_ENTRY(hg->obj[i_list].exp_entry),tmp);

  }
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_EXP, hg->obj[i_list].exp, -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}




void export_def (GtkWidget *widget, gpointer gdata)
{
  int i_list;
  typHOE *hg=(typHOE *)gdata;
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
    gdk_window_raise(hg->plot_main->window);
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
    return;
  }
  else{
    flagPlot=TRUE;
  }
  
  create_plot_dialog(hg);
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
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
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


