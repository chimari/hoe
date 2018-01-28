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
  COLUMN_OBJ_EQUINOX,
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

  case COLUMN_OBJ_EQUINOX:
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
  
  // EQUINOX
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJ_EQUINOX, hg->obj[i_list].equinox, -1);
  
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
                              G_TYPE_DOUBLE,  // equinox
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
up_item (GtkWidget *widget, gpointer data)
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

static void
down_item (GtkWidget *widget, gpointer data)
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
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));

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
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
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
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
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

    case COLUMN_OBJ_EQUINOX:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].equinox=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].equinox, -1);
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

  /* EQUINOX column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_EQUINOX));
  column=gtk_tree_view_column_new_with_attributes ("Eq.",
					    renderer,
					    "text",
					    COLUMN_OBJ_EQUINOX,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_EQUINOX),
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

