#include "main.h"
#include <string.h>
#include <stdlib.h>

#include "ar_u1.xpm"
#include "ar_u2.xpm"
#include "ar_u3.xpm"
#include "ar_d1.xpm"
#include "ar_d2.xpm"
#include "ar_d3.xpm"

static GtkWidget *window = NULL;
void tree_update_azel_item();
void tree_store_update(); 
void close_tree();
void remake_tree();



GdkPixbuf *pix_u1=NULL, 
  *pix_u2=NULL,
  *pix_u3=NULL,
  *pix_d1=NULL,
  *pix_d2=NULL,
  *pix_d3=NULL;

enum
{
  COLUMN_OBJ_NUMBER,
  COLUMN_OBJ_NAME,
  COLUMN_OBJ_AZ,
  COLUMN_OBJ_EL,
  COLUMN_OBJ_PIXBUF,
  COLUMN_OBJ_HA,
  COLUMN_OBJ_AD,
  COLUMN_OBJ_ADPA,
  COLUMN_OBJ_EXP,
  COLUMN_OBJ_REPEAT,
  COLUMN_OBJ_MAG,
  COLUMN_OBJ_RA,
  COLUMN_OBJ_DEC,
  COLUMN_OBJ_EPOCH,
  COLUMN_OBJ_PA,
  COLUMN_OBJ_GUIDE,
  //COLUMN_OBJ_SETUP
  COLUMN_OBJ_NOTE,
  NUM_OBJ_COLUMNS
};


void pos_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gdouble el;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJ_EL, &el,
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_OBJ_AZ:
    if(el>0){
      str=g_strdup_printf("%+.0lf",value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_EL:
    if(el>0){
      str=g_strdup_printf("%.0lf",el);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_HA:
    if(el>0){
      str=g_strdup_printf("%+.1lf",value);
    }
    else{
      str=NULL;;
    }
    break;

  case COLUMN_OBJ_AD:
    if(el>0){
      if((value<0)||(value>=10)){
	str=g_strdup_printf(">10");
      }
      else{
	str=g_strdup_printf("%.1lf",value);
      }
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_ADPA:
    if(el>0){
      str=g_strdup_printf("%.0lf",value);
    }
    else{
      str=NULL;
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void double_cell_data_func(GtkTreeViewColumn *col , 
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
  case COLUMN_OBJ_MAG:
    if(value>-100)
      str=g_strdup_printf("%.1lf",value);
    else
      str=NULL;
    break;

  case COLUMN_OBJ_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_OBJ_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_OBJ_EPOCH:
    str=g_strdup_printf("%7.2lf",value);
    break;

  case COLUMN_OBJ_PA:
    str=g_strdup_printf("%+.1lf",value);
    break;

  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void int_cell_data_func(GtkTreeViewColumn *col , 
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
  case COLUMN_OBJ_GUIDE:
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

  case COLUMN_OBJ_REPEAT:
    str=g_strdup_printf("x%2d",value);
    break;

  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void tree_update_azel_item(typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_list)
{
  gchar tmp[128];
  gint i;
  GtkTreePath *path;

  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJ_NUMBER,
		      i_list+1,
		      COLUMN_OBJ_NAME,
		      hg->obj[i_list].name,
		      -1);

  // Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJ_NUMBER,
		      i_list+1,
		      COLUMN_OBJ_NAME,
		      hg->obj[i_list].name,
		      -1);
  
  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_RA, hg->obj[i_list].ra, -1);
  
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_DEC, hg->obj[i_list].dec, -1);
  
  // EPOCH
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_EPOCH, hg->obj[i_list].epoch, -1);
  
  // PA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_PA, hg->obj[i_list].pa, -1);

  // Guide
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_GUIDE, hg->obj[i_list].guide, -1);

  // NOTE
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_NOTE, hg->obj[i_list].note, -1);
  
  // Az
  {
    gdouble az_tmp;
    if(hg->obj[i_list].c_el>0){
      if(hg->azel_mode==AZEL_POSI){
	if(hg->obj[i_list].c_az<-90)
	  az_tmp=hg->obj[i_list].c_az+360;
	else
	  az_tmp=hg->obj[i_list].c_az;
      }
      else if(hg->azel_mode==AZEL_NEGA){
	if(hg->obj[i_list].c_az>90)
	  az_tmp=hg->obj[i_list].c_az-360;
	else
	  az_tmp=hg->obj[i_list].c_az;
      }
      else{
	  az_tmp=hg->obj[i_list].c_az;
      }
    }
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_AZ, az_tmp, -1);
  }
  
  // El
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_EL, hg->obj[i_list].c_el, -1);
  
  // Mark
  if(hg->obj[i_list].c_el>30){
    if(hg->obj[i_list].c_ha<0){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_u1, -1);
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d1, -1);
    }
  }
  else if(hg->obj[i_list].c_el>15){
    if(hg->obj[i_list].c_ha<0){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_u2, -1);
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d2, -1);
    }
  }
  else if(hg->obj[i_list].c_el>0){
    if(hg->obj[i_list].c_ha<0){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_u3, -1);
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d3, -1);
    }
  }
  else{
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_PIXBUF, NULL, -1);
  }
  
  // HA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_HA, hg->obj[i_list].c_ha, -1);
  
  // AD
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_AD, hg->obj[i_list].c_ad, -1);
  
  // Ang
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_ADPA, hg->obj[i_list].c_pa, -1);
  
  // Exptime
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_EXP,  hg->obj[i_list].exp, -1);
  
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_REPEAT, hg->obj[i_list].repeat, -1);
  

  if(hg->flag_bunnei){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_MAG, hg->obj[i_list].mag, -1);
  }
  else{
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_MAG, -200.0, -1);
  }
  
}

gint tree_update_azel (gpointer gdata)
{
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return(0);

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;
    tree_update_azel_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}


static GtkTreeModel *
create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (COLUMN_OBJ_NOTE+1, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // az
                              G_TYPE_DOUBLE,  // el
			      GDK_TYPE_PIXBUF,	// Icon
                              G_TYPE_DOUBLE,  // HA
                              G_TYPE_DOUBLE,  // AD
                              G_TYPE_DOUBLE,  // ADPA
                              G_TYPE_INT,     // EXP
                              G_TYPE_INT,     // REPEAT
                              G_TYPE_DOUBLE,  // Mag
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
                              G_TYPE_DOUBLE,  // epoch
			      G_TYPE_DOUBLE,  // PA
			      G_TYPE_INT,     // GUIDE
			      G_TYPE_STRING);  // NOTE

  //gtk_list_store_set_column_types (GTK_LIST_STORE (model), 1, 
  //			   (GType []){ G_TYPE_STRING }); // NOTE
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (model, &iter);
    tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
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

static void
add_item (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  gint i,i_list,i_use;
  OBJpara tmp_obj;

  if(hg->i_max>=MAX_OBJECT) return;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    tmp_obj.name=g_strdup("(New Object)");
    
    tmp_obj.ra=0.0;
    tmp_obj.dec=0.0;
    tmp_obj.epoch=2000.0;
    tmp_obj.note=NULL;
  
    tmp_obj.exp=hg->def_exp;
    tmp_obj.repeat=1;
    tmp_obj.guide=hg->def_guide;
    tmp_obj.pa=0;
  
    tmp_obj.setup[0]=TRUE;
    for(i_use=1;i_use<MAX_USESETUP;i_use++){
      tmp_obj.setup[i_use]=FALSE;
    }
    
    for(i_list=hg->i_max;i_list>i;i_list--){
      hg->obj[i_list]=hg->obj[i_list-1];
    }
    
    hg->i_max++;
    
    hg->obj[i]=tmp_obj;
    
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    tree_update_azel_item(hg, model, iter, i);

    remake_tree(hg);
    //make_obj_list(hg,FALSE);
    //tree_update_azel((gpointer)hg);

    gtk_tree_path_free (path);
  }
}

static void
remove_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	
    for(i_list=i;i_list<hg->i_max;i_list++){
      hg->obj[i_list]=hg->obj[i_list+1];
    }

    hg->i_max--;
    
    remake_tree(hg);
    //make_obj_list(hg,FALSE);

    //tree_update_azel((gpointer)hg);

    gtk_tree_path_free (path);
  }
}




static void
up_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    if(i>0){
      tmp_obj=hg->obj[i-1];
      hg->obj[i-1]=hg->obj[i];
      hg->obj[i]=tmp_obj;

      tree_update_azel((gpointer)hg);
      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
    }
    
    //make_obj_list(hg,FALSE);

    gtk_tree_path_free (path);
  }
}


down_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    if(i<hg->i_max-1){
      tmp_obj=hg->obj[i];
      hg->obj[i]=hg->obj[i+1];
      hg->obj[i+1]=tmp_obj;

      tree_update_azel((gpointer)hg);
      gtk_tree_path_next (path);
      gtk_tree_selection_select_path(selection, path);
    }
    
    //make_obj_list(hg,FALSE);

    gtk_tree_path_free (path);
  }
}



static void
focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (model, &iter);
      //i = gtk_tree_path_get_indices (path)[0];
      gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
      i--;

      hg->obj[hg->tree_focus].check_sm=FALSE;
      hg->tree_focus=i;
      hg->obj[hg->tree_focus].check_sm=TRUE;

      gtk_tree_path_free (path);
    }

  {
    if(flagSkymon){
      draw_skymon_cairo(hg->skymon_dw,NULL,
			(gpointer)hg);
      gdk_window_raise(hg->skymon_main->window);
    }
  }
}


static void
refresh_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  gint i_list;

  calcpa2_main(hg);

  if(hg->flag_bunnei) {

    for(i_list=0;i_list<hg->i_max;i_list++){

      hg->obj[i_list].exp=(gint)(pow(2.5,(hg->obj[i_list].mag - 8.0)) 
				 * hg->exp8mag);

      if(hg->flag_secz){
	hg->obj[i_list].exp=hg->obj[i_list].exp+
	  hg->obj[i_list].exp*(1/sin(hg->obj[i_list].c_el/180*3.141592) -1)
	  *hg->secz_factor;
      }

      if(hg->obj[i_list].exp<1) hg->obj[i_list].exp=1;

    }
  }

  tree_update_azel((gpointer)hg);
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
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column)
    {
    case COLUMN_OBJ_NAME:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	g_free(hg->obj[i].name);
	hg->obj[i].name=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].name, -1);
	
      }
      break;

    case COLUMN_OBJ_EXP:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

        gtk_tree_model_get (model, &iter, column, &hg->obj[i].exp, -1);

        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].exp, -1);
      }
      break;

    case COLUMN_OBJ_REPEAT:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;
        hg->obj[i].repeat = atoi (new_text+1);

        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
	                   hg->obj[i].repeat, -1);
      }
      break;

    case COLUMN_OBJ_MAG:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].mag=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].mag, -1);
      }
      break;

    case COLUMN_OBJ_RA:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].ra=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].ra, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_DEC:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].dec=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].dec, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_EPOCH:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].epoch=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].epoch, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_PA:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].pa=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].pa, -1);
      }
      break;

    case COLUMN_OBJ_GUIDE:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
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

    case COLUMN_OBJ_NOTE:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
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
add_columns (typHOE *hg,
	     GtkTreeView  *treeview, 
	     GtkTreeModel *items_model,
             GtkTreeModel *repeat_model,
             GtkTreeModel *guide_model
	     )
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("##",
					    renderer,
					    "text",
					    COLUMN_OBJ_NUMBER,
					    NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_OBJ_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Az column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_AZ));
  column=gtk_tree_view_column_new_with_attributes ("Az",
					    renderer,
					    "text",
					    COLUMN_OBJ_AZ,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_AZ),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_AZ);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* El column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_EL));
  column=gtk_tree_view_column_new_with_attributes ("El",
					    renderer,
					    "text",
					    COLUMN_OBJ_EL,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_EL),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_EL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Icon column */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_PIXBUF));
  column=gtk_tree_view_column_new_with_attributes ("",
					    renderer,
					    "pixbuf",
					    COLUMN_OBJ_PIXBUF,
					    NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_EL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* HA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_HA));
  column=gtk_tree_view_column_new_with_attributes ("HA",
					    renderer,
					    "text",
					    COLUMN_OBJ_HA,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_HA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_HA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* AD column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_AD));
  column=gtk_tree_view_column_new_with_attributes ("AD",
					    renderer,
					    "text", COLUMN_OBJ_AD,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_AD),
					  NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_AD);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* ADPA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_ADPA));
  column=gtk_tree_view_column_new_with_attributes ("Ang",
					    renderer,
					    "text", COLUMN_OBJ_ADPA,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_ADPA),
					  NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_ADPA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Exptime column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_EXP));

  column=gtk_tree_view_column_new_with_attributes ("Exp",
					    renderer,
					    "text",
					    COLUMN_OBJ_EXP,
					    NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_EXP);
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
  		     "column", GINT_TO_POINTER (COLUMN_OBJ_REPEAT));
  column=gtk_tree_view_column_new_with_attributes ("x",
						   renderer,
						   "text",
						   COLUMN_OBJ_REPEAT,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  int_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_REPEAT),
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
  		     GINT_TO_POINTER (COLUMN_OBJ_MAG));
  column=gtk_tree_view_column_new_with_attributes ("Mag",
					    renderer,
					    "text",
					    COLUMN_OBJ_MAG,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_MAG),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_MAG);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);



  /* RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_RA));
  column=gtk_tree_view_column_new_with_attributes ("RA",
					    renderer,
					    "text",
					    COLUMN_OBJ_RA,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_RA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_RA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);



  /* Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_DEC));
  column=gtk_tree_view_column_new_with_attributes ("Dec",
					    renderer,
					    "text",
					    COLUMN_OBJ_DEC,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* EPOCH column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_EPOCH));
  column=gtk_tree_view_column_new_with_attributes ("Epoch",
					    renderer,
					    "text",
					    COLUMN_OBJ_EPOCH,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_EPOCH),
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
  		     GINT_TO_POINTER (COLUMN_OBJ_PA));
  column=gtk_tree_view_column_new_with_attributes ("ImRPA",
					    renderer,
					    "text",
					    COLUMN_OBJ_PA,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_PA),
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
  		     GINT_TO_POINTER (COLUMN_OBJ_GUIDE));
  column=gtk_tree_view_column_new_with_attributes ("Guide",
					    renderer,
					    "text",
					    COLUMN_OBJ_GUIDE,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  int_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_GUIDE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Note column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_OBJ_NOTE));
  column=gtk_tree_view_column_new_with_attributes ("Note",
					    renderer,
					    "text",
					    COLUMN_OBJ_NOTE,
					    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


}

GtkWidget *
do_editable_cells (typHOE *hg)
{
  if (!window)
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *sw;
      //GtkWidget *treeview;
      GtkWidget *button;
      GtkTreeModel *items_model;
      GtkTreeModel *repeat_model;
      GtkTreeModel *guide_model;
      GtkWidget *label;
      GtkWidget *combo;
      GtkWidget *check;
      GtkWidget *entry;
      gchar tmp[12];

      /* create window, etc */
      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_screen (GTK_WINDOW (window),
                             gtk_widget_get_screen (hg->w_top));
      gtk_window_set_title (GTK_WINDOW (window), "HOE : Object List");
      gtk_container_set_border_width (GTK_CONTAINER (window), 5);
      g_signal_connect (window, "destroy",
                        G_CALLBACK (close_tree), (gpointer)hg);


      vbox = gtk_vbox_new (FALSE, 5);
      gtk_container_add (GTK_CONTAINER (window), vbox);

      hg->tree_label= gtk_label_new ("Object List");

      gtk_box_pack_start (GTK_BOX (vbox),hg->tree_label,
                          FALSE, FALSE, 0);

      sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                           GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                      GTK_POLICY_AUTOMATIC,
                                      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

      /* create models */
      items_model = create_items_model (hg);
      repeat_model = create_repeat_model ();
      guide_model = create_guide_model ();

      /* create tree view */
      hg->tree = gtk_tree_view_new_with_model (items_model);
      gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->tree), TRUE);
      gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->tree)),
                                   GTK_SELECTION_SINGLE);
      add_columns (hg, GTK_TREE_VIEW (hg->tree), items_model, 
		   repeat_model, guide_model);

      g_object_unref (guide_model);
      g_object_unref (repeat_model);
      g_object_unref (items_model);

      gtk_container_add (GTK_CONTAINER (sw), hg->tree);

      /* some buttons */
      hbox = gtk_hbox_new (TRUE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

      button=gtkut_button_new_from_stock("Add",GTK_STOCK_ADD);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (add_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      button=gtkut_button_new_from_stock("Del",GTK_STOCK_REMOVE);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (remove_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      g_signal_connect (hg->tree, "cursor-changed",
                        G_CALLBACK (focus_item), (gpointer)hg);
      
      button=gtkut_button_new_from_stock("Up",GTK_STOCK_GO_UP);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (up_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      g_signal_connect (hg->tree, "cursor-changed",
                        G_CALLBACK (focus_item), (gpointer)hg);
      
      button=gtkut_button_new_from_stock("Down",GTK_STOCK_GO_DOWN);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (down_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);


      hbox = gtk_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

      label = gtk_label_new ("AzEl");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Normal",
			   1, AZEL_NORMAL, -1);
	if(hg->azel_mode==AZEL_NORMAL) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "+270",
			   1, AZEL_POSI, -1);
	if(hg->azel_mode==AZEL_POSI) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "-270",
			   1, AZEL_NEGA, -1);
	if(hg->azel_mode==AZEL_NEGA) iter_set=iter;
	
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->azel_mode);
      }

      label = gtk_label_new ("   Exp for Mv=8");
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hg->e_exp8mag = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->e_exp8mag,FALSE,FALSE,0);
      gtk_entry_set_editable(GTK_ENTRY(hg->e_exp8mag),TRUE);
      sprintf(tmp,"%d",hg->exp8mag);
      gtk_entry_set_text(GTK_ENTRY(hg->e_exp8mag),tmp);
      my_signal_connect (hg->e_exp8mag,
			 "changed",
			 cc_get_entry_int,
			 &hg->exp8mag);
      my_entry_set_width_chars(GTK_ENTRY(hg->e_exp8mag),4);


      check = gtk_check_button_new_with_label("SecZ");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
      if(hg->flag_secz){
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
      }
      else{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),FALSE);
      }
      my_signal_connect (check, "toggled",
			 cc_get_toggle,
			 &hg->flag_secz);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      sprintf(tmp,"%4.2f",hg->secz_factor);
      gtk_entry_set_text(GTK_ENTRY(entry),tmp);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry_double,
			 &hg->secz_factor);
      my_entry_set_width_chars(GTK_ENTRY(entry),4);

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (refresh_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);


      g_signal_connect (hg->tree, "cursor-changed",
                        G_CALLBACK (focus_item), (gpointer)hg);
      
      gtk_window_set_default_size (GTK_WINDOW (window), 320, 600);
    }
  
  if (!GTK_WIDGET_VISIBLE (window))
    gtk_widget_show_all (window);
  else
    {
      gtk_widget_destroy (window);
      window = NULL;
      flagTree=FALSE;
    }

  return window;
}


void make_tree(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  if(!flagTree){
    hg=(typHOE *)gdata;

    flagTree=TRUE;

    if(!pix_u1)  pix_u1 = gdk_pixbuf_new_from_xpm_data(ar_u1_xpm);
    if(!pix_u2)  pix_u2 = gdk_pixbuf_new_from_xpm_data(ar_u2_xpm);
    if(!pix_u3)  pix_u3 = gdk_pixbuf_new_from_xpm_data(ar_u3_xpm);
    if(!pix_d1)  pix_d1 = gdk_pixbuf_new_from_xpm_data(ar_d1_xpm);
    if(!pix_d2)  pix_d2 = gdk_pixbuf_new_from_xpm_data(ar_d2_xpm);
    if(!pix_d3)  pix_d3 = gdk_pixbuf_new_from_xpm_data(ar_d3_xpm);

    do_editable_cells (hg);
    
  }

  /*
  g_timeout_add(AZEL_INTERVAL, 
		(GSourceFunc)tree_update_azel, 
		(gpointer)hg);
  */
}


void close_tree(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_widget_destroy(GTK_WIDGET(window));
  window = NULL;
  flagTree=FALSE;
}

void remake_tree(typHOE *hg)
{
  close_tree(NULL,hg);
  make_tree(NULL,hg);
}

