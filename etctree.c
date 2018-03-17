//    HDS OPE file Editor
//      etctree.c : Exposure Time Calculator Reuslts
//                                           2018.2.05  A.Tajitsu


#include"main.h"    // 設定ヘッダ
#include"version.h"

enum
{
  COLUMN_ETCTREE_NUMBER,
  COLUMN_ETCTREE_BAD,
  COLUMN_ETCTREE_ORDER,
  COLUMN_ETCTREE_PIXEL,
  COLUMN_ETCTREE_WAVE,
  COLUMN_ETCTREE_DISP,
  COLUMN_ETCTREE_FLUX,
  COLUMN_ETCTREE_PEAK,
  COLUMN_ETCTREE_SNR,
  COLUMN_ETCTREE_ISGAIN,
  COLUMN_ETCTREE_CCD,
  COLUMN_ETCTREE_LINE,
  NUM_ETCTREE_COLUMNS
};

void make_etc_tree();
static void etctree_add_columns ();
static GtkTreeModel *etc_create_items_model();
void etctree_update_item();
void etctree_bad_cell_data_func();
void etctree_sat_cell_data_func();
void etctree_pixel_cell_data_func();
void etctree_wave_cell_data_func();
void etctree_isgain_cell_data_func();
void etctree_line_cell_data_func();
void etctree_ccd_cell_data_func();
void etctree_double_cell_data_func();
//void rebuild_etc_tree();

void make_etc_tree(typHOE *hg){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  
  if(flag_make_etc_tree)  gtk_widget_destroy(hg->etc_tree);
  else flag_make_etc_tree=TRUE;

  items_model = etc_create_items_model (hg);

  /* create tree view */
  hg->etc_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->etc_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->etc_tree)),
			       GTK_SELECTION_SINGLE);
  etctree_add_columns (hg, GTK_TREE_VIEW (hg->etc_tree), 
			items_model);

  g_object_unref(items_model);
  
  gtk_container_add (GTK_CONTAINER (hg->etc_sw), hg->etc_tree);
  
  gtk_widget_show_all(hg->etc_tree);
 
}


static void
etctree_add_columns (typHOE *hg,
		      GtkTreeView  *treeview, 
		      GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Bad column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_BAD));
  column=gtk_tree_view_column_new_with_attributes ("Note",
					    renderer,
					    "text",
					    COLUMN_ETCTREE_BAD,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_bad_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Order column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_ORDER));
  column=gtk_tree_view_column_new_with_attributes ("Order",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_ORDER,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_ccd_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Pix column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_PIXEL));
  column=gtk_tree_view_column_new_with_attributes ("Pixel",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_PIXEL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_pixel_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Wave
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_WAVE));
  column=gtk_tree_view_column_new_with_attributes ("Wavelength [A]",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_WAVE,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_wave_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Dispersion
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_DISP));
  column=gtk_tree_view_column_new_with_attributes ("[A/pix]",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_DISP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_ETCTREE_DISP),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Flux
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_FLUX));
  column=gtk_tree_view_column_new_with_attributes ("Flux [uJy]",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_FLUX,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_ETCTREE_FLUX),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Peak
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_PEAK));
  column=gtk_tree_view_column_new_with_attributes ("Peak [ADU]",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_PEAK,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_sat_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // SNR
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_FLUX));
  column=gtk_tree_view_column_new_with_attributes ("S/N",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_SNR,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_ETCTREE_SNR),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // IS Gain
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_ISGAIN));
  column=gtk_tree_view_column_new_with_attributes ("Gain by IS",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_ISGAIN,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_isgain_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Lines
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ETCTREE_LINE));
  column=gtk_tree_view_column_new_with_attributes ("Lines",
						   renderer,
						   "text",
						   COLUMN_ETCTREE_LINE,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  etctree_line_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}

static GtkTreeModel *
etc_create_items_model (typHOE *hg)
{
  gint i_etc = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_ETCTREE_COLUMNS, 
			      G_TYPE_INT,     // number
			      G_TYPE_BOOLEAN, // Bad
			      G_TYPE_INT,     // order
			      G_TYPE_INT,     // Pix C
			      G_TYPE_DOUBLE,  // Wv C
			      G_TYPE_DOUBLE,  // Disp
			      G_TYPE_DOUBLE,  // Flux
			      G_TYPE_INT,     // Peak
			      G_TYPE_DOUBLE,  // SNR
			      G_TYPE_DOUBLE,  // IS Gain
			      G_TYPE_INT,     // CCD
			      G_TYPE_DOUBLE   // LINE
			      );  

  for (i_etc = 0; i_etc < hg->etc_i_max; i_etc++){
    gtk_list_store_append (model, &iter);
    etctree_update_item(hg, GTK_TREE_MODEL(model), iter, i_etc);
  }
  
  return GTK_TREE_MODEL (model);
}


void etctree_update_item(typHOE *hg, 
			 GtkTreeModel *model, 
			 GtkTreeIter iter, 
			 gint i_list)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_ETCTREE_NUMBER, i_list+1,
		      COLUMN_ETCTREE_BAD, hg->etc[i_list].bad,
		      COLUMN_ETCTREE_ORDER, hg->etc[i_list].order,
		      COLUMN_ETCTREE_PIXEL, hg->etc[i_list].pix_c,
		      COLUMN_ETCTREE_WAVE, hg->etc[i_list].w_c,
		      COLUMN_ETCTREE_DISP, hg->etc[i_list].disp,
		      COLUMN_ETCTREE_FLUX, hg->etc[i_list].flux,
		      COLUMN_ETCTREE_PEAK, hg->etc[i_list].peak,
		      COLUMN_ETCTREE_SNR, hg->etc[i_list].snr,
		      COLUMN_ETCTREE_ISGAIN, hg->etc[i_list].isgain,
		      COLUMN_ETCTREE_CCD, hg->etc[i_list].ccd,
		      -1);
}


void etctree_bad_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  gint i;
  typHOE *hg;
  gchar *str=NULL;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].bad){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "foreground-rgba", &color_black,
#else
		 "foreground-gdk", &color_black,
#endif
		 NULL);
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", &color_pink,
#else
		 "background-gdk", &color_pink,
#endif
		 NULL);
    str=g_strdup("Bad column");
  }
  else if(hg->etc[i].isgap){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "foreground-rgba", &color_black,
#else
		 "foreground-gdk", &color_black,
#endif
		 NULL);
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", &color_orange,
#else
		 "background-gdk", &color_orange,
#endif
		 NULL);
    str=g_strdup("Small gap");
  }
  else if(hg->etc[i].sat){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", &color_black,
#else
		 "background-gdk", &color_black,
#endif
		 NULL);
    g_object_set(renderer,
#ifdef USE_GTK3
		 "foreground-rgba", &color_white,
#else
		 "foreground-gdk", &color_white,
#endif
		 NULL);
    str=g_strdup("Saturated");
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "foreground-rgba", &color_black,
#else
		 "foreground-gdk", &color_black,
#endif
		 NULL);
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", NULL,
#else
		 "background-gdk", NULL,
#endif
		 NULL);
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void etctree_sat_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  gint i;
  typHOE *hg;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].sat){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pink,
		 NULL);
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 NULL,
		 NULL);
  }

}


void etctree_pixel_cell_data_func(GtkTreeViewColumn *col , 
				  GtkCellRenderer *renderer,
				  GtkTreeModel *model, 
				  GtkTreeIter *iter,
				  gpointer user_data)
{
  gint i;
  typHOE *hg;
  gchar *str=NULL;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].ccd==1){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pale2,
		 NULL);
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pink2,
		 NULL);
  }

  str=g_strdup_printf("%d -- %d -- %d",
		      hg->etc[i].pix_s,
		      hg->etc[i].pix_c,
		      hg->etc[i].pix_e);		      

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void etctree_wave_cell_data_func(GtkTreeViewColumn *col , 
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model, 
				 GtkTreeIter *iter,
				 gpointer user_data)
{
  gint i;
  typHOE *hg;
  gchar *str=NULL;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].ccd==1){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pale2,
		 NULL);
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pink2,
		 NULL);
  }

  str=g_strdup_printf("%.3lf -- %.3lf -- %.3lf",
		      hg->etc[i].w_s,
		      hg->etc[i].w_c,
		      hg->etc[i].w_e);		      

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void etctree_isgain_cell_data_func(GtkTreeViewColumn *col , 
				   GtkCellRenderer *renderer,
				   GtkTreeModel *model, 
				   GtkTreeIter *iter,
				   gpointer user_data)
{
  gint i;
  typHOE *hg;
  gchar *str=NULL;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].isgap){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_orange,
		 NULL);
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 NULL,
		 NULL);
  }

  if(hg->etc[i].isgain>0){
    if(hg->etc[i].snr_gain>0){
      str=g_strdup_printf("x%.2lf  S/N+=%.0lf",
			  hg->etc[i].isgain,
			  hg->etc[i].snr_gain);
    }
    else{
      str=g_strdup_printf("x%.2lf  S/N-=%.0lf",
			  hg->etc[i].isgain,
			  -hg->etc[i].snr_gain);
    }
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void etctree_line_cell_data_func(GtkTreeViewColumn *col , 
				   GtkCellRenderer *renderer,
				   GtkTreeModel *model, 
				   GtkTreeIter *iter,
				   gpointer user_data)
{
  gint i, i_ln;
  typHOE *hg;
  gchar *str=NULL, *tmp=NULL;
  gdouble wv;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  for(i_ln=0;i_ln<MAX_LINE;i_ln++){
    if(hg->line[i_ln].name){
      wv=hg->line[i_ln].wave*(1+hg->etc_z);
      if((hg->etc[i].w_s < wv) && (wv < hg->etc[i].w_e)){
	if(str){
	  tmp=g_strdup(str);
	  g_free(str);
	  str=g_strdup_printf("%s, %s (%.0lf)", tmp, hg->line[i_ln].name, wv);
	  g_free(tmp);
	}
	else{
	  str=g_strdup_printf("%s (%.0lf)", hg->line[i_ln].name, wv);
	}
      }
    }
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void etctree_ccd_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  gint i;
  typHOE *hg;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_ETCTREE_NUMBER, &i, -1);
  i--;

  if(hg->etc[i].ccd==1){
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pale2,
		 NULL);
  }
  else{
    g_object_set(renderer,
#ifdef USE_GTK3
		 "background-rgba", 
#else
		 "background-gdk", 
#endif
		 &color_pink2,
		 NULL);
  }
}


void etctree_double_cell_data_func(GtkTreeViewColumn *col , 
				    GtkCellRenderer *renderer,
				    GtkTreeModel *model, 
				    GtkTreeIter *iter,
				    gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str=NULL;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_ETCTREE_DISP:
    if(value>0)
      str=g_strdup_printf("%.3lf",value);
    else
      str=NULL;
    break;

  case COLUMN_ETCTREE_FLUX:
    if(value>0)
      str=g_strdup_printf("%.2lf",value);
    else
      str=NULL;
    break;

  case COLUMN_ETCTREE_SNR:
    if(value>0)
      str=g_strdup_printf("%.0lf",value);
    else
      str=NULL;
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void rebuild_etc_tree(typHOE *hg)
{
  gtk_widget_destroy(GTK_WIDGET(hg->etc_tree));

  etc_append_tree(hg);
  gtk_widget_show(hg->etc_tree);

  if(hg->etc_label_text) g_free(hg->etc_label_text);
  hg->etc_label_text
    =g_strdup_printf("ETC : Setup-%d, %s=%.2lfmag, ExpTime=%ds, Seeing=%.2lf\"",
		     hg->etc_setup+1,
		     etc_filters[hg->etc_filter],
		     hg->etc_mag,
		     hg->etc_exptime,
		     hg->etc_seeing);
  gtk_label_set_text(GTK_LABEL(hg->etc_label), hg->etc_label_text);

  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->all_note), NOTE_ETC);
}

void etc_append_tree(typHOE *hg){
  GtkTreeModel *items_model;

  /* create models */
  items_model = etc_create_items_model (hg);
  
  /* create tree view */
  hg->etc_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->etc_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->etc_tree)),
			       GTK_SELECTION_SINGLE);
  etctree_add_columns (hg, GTK_TREE_VIEW (hg->etc_tree), items_model);
  
  g_object_unref (items_model);
  
  gtk_container_add (GTK_CONTAINER (hg->etc_sw), hg->etc_tree);
}    

