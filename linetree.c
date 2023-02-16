//    HDS OPE file Editor
//      linetree.c : Line List for EFS
//                                           2010.2.22  A.Tajitsu

#include"main.h"    

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


static void linetree_add_columns();
static GtkTreeModel *linetree_create_items_model ();
void linetree_update_item();
static void cell_edited ();
void linetree_double_cell_data_func();
void linetree_zwave_cell_data_func();

enum
{
  COLUMN_LINETREE_NUMBER,
  COLUMN_LINETREE_NAME,
  COLUMN_LINETREE_WAVE,
  COLUMN_LINETREE_ZWAVE,
  NUM_LINETREE_COLUMNS
};


const Linepara line_nebula[]={
  {"[OII]  3726", 3726.03},
  {"[NeIII]3869", 3868.71},
  {"H gamma",     4340.47},
  {"HeII   4686", 4685.68}, 
  {"H beta",      4861.33},
  {"[OIII] 4959", 4958.92},
  {"[OIII] 5007", 5006.84},
  {"[NII]  5755", 5754.64},
  {"HeI    5876", 5875.67},
  {"[OI]   6300", 6300.30},
  {"[OI]   6364", 6363.78},
  {"[NII]  6548", 6548.03},
  {"H alpha",     6562.82},
  {"[NII]  6583", 6583.45},
  {"[SII]  6717", 6716.47},
  {"[SII]  6731", 6730.85},
  {"[ArIII]7136", 7135.78},
  {"[OII]  7319", 7319.40},
  {"[OII]  7330", 7329.90},
  {NULL, 0}
};

const Linepara line_star[]={
  {"Be II",       3131.07},
  {"Fe M",        3734.87},
  {"Fe L",        3820.44},
  {"Ca K",        3933.68},
  {"Ca H",        3968.49},
  {"H delta",     4101.75},
  {"H gamma",     4340.48}, 
  {"Fe d",        4383.56}, 
  {"H beta",      4861.33},
  {"Mg b4",       5167.33},
  {"Mg b2",       5172.70},
  {"Mg b1",       5183.62},
  {"Fe E",        5269.55},
  {"Na D2",       5889.97},
  {"Na D1",       5895.94},
  {"H alpha",     6562.82},
  {"Li I",        6707.80},
  {"Ca II",       8498.06},
  {"Ca II",       8542.14},
  {"Ca II",       8662.17}
};


const Linepara line_highz[]={
  {"Lyman limit", 912},
  {"Lyman beta", 1026},
  {"OVI 1034",   1034},
  {"Lyman alpha",1216},
  {"NV 1240",    1240},
  {"OIV] 1407",  1407},
  {"NIV] 1488",  1488},
  {"CIV 1549",   1549},
  {"HeII 1640",  1640},
  {"OIII] 1663", 1663},
  {"CIII] 1909", 1909},
  {"MgII 2798",  2798},
  {"Hgamma",     4340},
  {"HeII 4686",  4686},
  {"Hbeta",      4861},
  {NULL, 0},
  {NULL, 0},
  {NULL, 0},
  {NULL, 0},
  {NULL, 0}
};


void make_line_tree(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  
  if(flag_make_line_tree)  gtk_widget_destroy(hg->linetree);
  else flag_make_line_tree=TRUE;

  items_model = linetree_create_items_model (hg);

  /* create tree view */
  hg->linetree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->linetree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->linetree)),
			       GTK_SELECTION_SINGLE);
  linetree_add_columns (hg, GTK_TREE_VIEW (hg->linetree), 
			items_model);

  g_object_unref(items_model);
  
  gtk_container_add (GTK_CONTAINER (hg->sw_linetree), hg->linetree);
  
  gtk_widget_show_all(hg->linetree);
}


static void
linetree_add_columns (typHOE *hg,
		      GtkTreeView  *treeview, 
		      GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_LINETREE_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
						   renderer,
						   "text",
						   COLUMN_LINETREE_NUMBER,
						   NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_LINETREE_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_LINETREE_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Line Name",
						   renderer,
						   "text", 
						   COLUMN_LINETREE_NAME,
						   NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_LINETREE_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Wave column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_LINETREE_WAVE));
  column=gtk_tree_view_column_new_with_attributes (NULL,
					    renderer,
					    "text",
					    COLUMN_LINETREE_WAVE,
					    NULL);
  gtkut_tree_view_column_set_markup(column, "&#x3BB; [&#xC5;]");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  linetree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_LINETREE_WAVE),
					  NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_LINETREE_WAVE);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Wave column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_LINETREE_ZWAVE));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text",
						   COLUMN_LINETREE_ZWAVE,
						   NULL);
  gtkut_tree_view_column_set_markup(column, "<i>z</i> corrected &#x3BB; [&#xC5;]");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  linetree_zwave_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

}

static GtkTreeModel *
linetree_create_items_model (typHOE *hg)
{
  gint i_ln = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_LINETREE_COLUMNS, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
			      G_TYPE_DOUBLE,  // Wavelength
			      G_TYPE_DOUBLE   // Z Wavelength
			      );  

  //gtk_list_store_set_column_types (GTK_LIST_STORE (model), 1, 
  //			   (GType []){ G_TYPE_STRING }); // NOTE
  for (i_ln = 0; i_ln < MAX_LINE; i_ln++){
    gtk_list_store_append (model, &iter);
    linetree_update_item(hg, GTK_TREE_MODEL(model), iter, i_ln);
  }
  
  return GTK_TREE_MODEL (model);
}


void linetree_update_item(typHOE *hg, 
			 GtkTreeModel *model, 
			 GtkTreeIter iter, 
			 gint i_list)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_LINETREE_NUMBER, i_list+1,
		      COLUMN_LINETREE_NAME,  hg->line[i_list].name,
		      COLUMN_LINETREE_WAVE, hg->line[i_list].wave, 
		      COLUMN_LINETREE_ZWAVE, hg->line[i_list].wave, 
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
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->linetree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column)
    {
    case COLUMN_LINETREE_NAME:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        if(old_text) g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_LINETREE_NUMBER, &i, -1);
	i--;
	
	if(hg->line[i].name) g_free(hg->line[i].name);
	hg->line[i].name=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->line[i].name, -1);
	
      }
      break;

    case COLUMN_LINETREE_WAVE:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_LINETREE_NUMBER, &i, -1);
	i--;

	hg->line[i].wave=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->line[i].wave, -1);
      }
      break;

    }


  gtk_tree_path_free (path);
}


void linetree_zwave_cell_data_func(GtkTreeViewColumn *col , 
				   GtkCellRenderer *renderer,
				   GtkTreeModel *model, 
				   GtkTreeIter *iter,
				   gpointer user_data)
{
  gint i;
  typHOE *hg;
  gchar *str=NULL;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_LINETREE_NUMBER, &i, -1);
  i--;

  if(hg->line[i].name){
    str=g_strdup_printf("%.2lf",hg->line[i].wave*(1+hg->etc_z));
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void linetree_double_cell_data_func(GtkTreeViewColumn *col , 
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
  case COLUMN_LINETREE_WAVE:
    if(value>0)
      str=g_strdup_printf("%.2lf",value);
    else
      str=NULL;
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}



void linetree_init(GtkWidget *button, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->linetree));
  int i_ln;
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_ln=0;i_ln<MAX_LINE;i_ln++){
    if(hg->line[i_ln].name) g_free(hg->line[i_ln].name);
    hg->line[i_ln].name=g_strdup("");
    hg->line[i_ln].wave=0;

    linetree_update_item(hg, model, iter, i_ln); 
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}

void linetree_nebula(GtkWidget *button, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->linetree));
  int i_ln;
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_ln=0;i_ln<MAX_LINE;i_ln++){
    if(hg->line[i_ln].name) g_free(hg->line[i_ln].name);
    hg->line[i_ln].name=g_strdup(line_nebula[i_ln].name);
    hg->line[i_ln].wave=line_nebula[i_ln].wave;

    linetree_update_item(hg, model, iter, i_ln); 
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}

void linetree_star(GtkWidget *button, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->linetree));
  int i_ln;
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_ln=0;i_ln<MAX_LINE;i_ln++){
    if(hg->line[i_ln].name) g_free(hg->line[i_ln].name);
    hg->line[i_ln].name=g_strdup(line_star[i_ln].name);
    hg->line[i_ln].wave=line_star[i_ln].wave;

    linetree_update_item(hg, model, iter, i_ln); 
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}

void linetree_highz(GtkWidget *button, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->linetree));
  int i_ln;
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_ln=0;i_ln<MAX_LINE;i_ln++){
    if(hg->line[i_ln].name) g_free(hg->line[i_ln].name);
    hg->line[i_ln].name=g_strdup(line_highz[i_ln].name);
    hg->line[i_ln].wave=line_highz[i_ln].wave;

    linetree_update_item(hg, model, iter, i_ln); 
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}


