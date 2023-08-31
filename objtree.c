//    HDS OPE file Editor
//      objtree.c : Main Target List Treeview
//                                           2010.1.27  A.Tajitsu

#include"main.h"    
#include"objtree.h"    

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


void get_total_basic_exp(typHOE *hg){
  gint i_list, i_use, set_num;
  gchar *tmp;
  glong total_exp=0,  total_obs=0;

  switch(hg->inst){
  case INST_HDS:
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
    break;

  case INST_IRCS:
    for(i_use=0;i_use<hg->ircs_i_max;i_use++){
      for(i_list=0;i_list<hg->i_max;i_list++){
	total_obs+=ircs_get_1obj_time(hg->ircs_set[i_use],
				      hg->oh_acq,
				      ircs_oh_ao(hg,
						 hg->obj[i_list].aomode,
						 i_list));
      }
      total_exp+=(gint)(hg->ircs_set[i_use].exp
			*(gdouble)ircs_get_shot(hg->ircs_set[i_use].dith, hg->ircs_set[i_use].ssnum)
			*(gdouble)hg->i_max);
    }
    break;
    
  case INST_HSC:
    for(i_use=0;i_use<hg->hsc_i_max;i_use++){
      for(i_list=0;i_list<hg->i_max;i_list++){
	switch(hg->hsc_set[i_use].dith){
	case HSC_DITH_NO:
	  total_exp+=(gint)hg->hsc_set[i_use].exp;
	  total_obs+=((gint)hg->hsc_set[i_use].exp + HSC_TIME_READOUT)
	    +HSC_TIME_ACQ;
	  break;
	case HSC_DITH_5:
	  total_exp+=(gint)hg->hsc_set[i_use].exp*5;
	  total_obs+=((gint)hg->hsc_set[i_use].exp + HSC_TIME_READOUT)*5
	    +HSC_TIME_ACQ;
	  break;
	case HSC_DITH_N:
	  total_exp+=(gint)hg->hsc_set[i_use].exp*hg->hsc_set[i_use].dith_n;
	  total_obs+=((gint)hg->hsc_set[i_use].exp + HSC_TIME_READOUT)
	    *hg->hsc_set[i_use].dith_n
	    +HSC_TIME_ACQ;
	  break;
	}
      }
    }
    break;
    
  case INST_IRD:
    for(i_list=0;i_list<hg->i_max;i_list++){
      total_exp+=(gint)(hg->obj[i_list].dexp*(gdouble)hg->obj[i_list].repeat);
      total_obs+=hg->oh_acq+ird_oh_ao(hg, hg->obj[i_list].aomode, i_list)
	+IRD_TIME_READOUT*hg->obj[i_list].repeat;
    }
    break;
    
  }
  tmp=g_strdup_printf("Total Exp. = %.2lf hrs,  Estimated Obs. Time = %.2lf hrs",
		      (gdouble)total_exp/60./60.,
		      (gdouble)total_obs/60./60.);

  gtk_label_set_text(GTK_LABEL(hg->label_stat_base),tmp);
  if(tmp) g_free(tmp);

  remake_sod(hg);
}


void objtree_update_radec (typHOE *hg)
{
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

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
  GtkTreeModel *aomode_model;
  GtkTreeModel *kools_grism_model;
  GtkTreeModel *triccs_filter_model;
  GtkTreeModel *triccs_gain_model;
  
  if(flag_make_obj_tree)  gtk_widget_destroy(hg->objtree);
  else flag_make_obj_tree=TRUE;

  items_model = create_items_model (hg);
  repeat_model = create_repeat_model ();
  guide_model = create_guide_model ();
  aomode_model = create_aomode_model ();
  kools_grism_model = create_kools_grism_model ();
  triccs_filter_model = create_triccs_filter_model ();
  triccs_gain_model = create_triccs_gain_model ();

  /* create tree view */
  hg->objtree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->objtree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->objtree)),
			       GTK_SELECTION_SINGLE);
  objtree_add_columns (hg, GTK_TREE_VIEW (hg->objtree), 
		       items_model,
		       repeat_model,
		       guide_model,
		       aomode_model,
		       kools_grism_model,
		       triccs_filter_model,
		       triccs_gain_model);

  g_object_unref(items_model);
  g_object_unref(repeat_model);
  g_object_unref(guide_model);
  g_object_unref(aomode_model);
  g_object_unref(kools_grism_model);
  g_object_unref(triccs_filter_model);
  g_object_unref(triccs_gain_model);
  
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
		     GtkTreeModel *guide_model,
		     GtkTreeModel *aomode_model,
		     GtkTreeModel *kools_grism_model,
		     GtkTreeModel *triccs_filter_model,
		     GtkTreeModel *triccs_gain_model)
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

  switch(hg->inst){
  case INST_HSC:
    /* Std column */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_std), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_STD));
    
    column = gtk_tree_view_column_new_with_attributes ("Std",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_STD,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }
  
  switch(hg->inst){
  case INST_HDS:
  case INST_KOOLS:
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
    break;
    
  case INST_IRD:
    /* dExptime column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_DEXP));
    
    column=gtk_tree_view_column_new_with_attributes ("Exp",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_DEXP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_DEXP),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
    
  case INST_TRICCS:
    /* dExptime column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_TEXP));
    
    column=gtk_tree_view_column_new_with_attributes ("Exp",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_TEXP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_TEXP),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

  /* Frames/Exp */
  switch(hg->inst){
  case INST_TRICCS:
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_TRICCS_FRAMES));
    
    column=gtk_tree_view_column_new_with_attributes ("Fr/Exp",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_TRICCS_FRAMES,
						     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }
  
  switch(hg->inst){
  case INST_HDS:
  case INST_IRD:
  case INST_KOOLS:
  case INST_TRICCS:
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
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_REPEAT),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

 
  switch(hg->inst){
  case INST_KOOLS:
    // KOOLS Grism
    renderer = gtk_cell_renderer_combo_new ();
    g_object_set (renderer,
		  "model", kools_grism_model,
		  "text-column", COLUMN_NUMBER_TEXT,
		  "has-entry", FALSE,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
    		      G_CALLBACK (cell_edited), (gpointer)hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_KOOLS_GRISM));
    column=gtk_tree_view_column_new_with_attributes ("Grism",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_KOOLS_GRISM,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
    					    objtree_cell_data_func,
    					    GUINT_TO_POINTER(COLUMN_OBJTREE_KOOLS_GRISM),
    					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;

  case INST_TRICCS:
    // TriCCS Filter
    renderer = gtk_cell_renderer_combo_new ();
    g_object_set (renderer,
		  "model", triccs_filter_model,
		  "text-column", COLUMN_NUMBER_TEXT,
		  "has-entry", FALSE,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
    		      G_CALLBACK (cell_edited), (gpointer)hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_TRICCS_FILTER));
    column=gtk_tree_view_column_new_with_attributes ("Filter",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_TRICCS_FILTER,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
    					    objtree_cell_data_func,
    					    GUINT_TO_POINTER(COLUMN_OBJTREE_TRICCS_FILTER),
    					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // TriCCS Gain
    renderer = gtk_cell_renderer_combo_new ();
    g_object_set (renderer,
		  "model", triccs_gain_model,
		  "text-column", COLUMN_NUMBER_TEXT,
		  "has-entry", FALSE,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
    		      G_CALLBACK (cell_edited), (gpointer)hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_TRICCS_GAIN));
    column=gtk_tree_view_column_new_with_attributes ("Gain",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_TRICCS_GAIN,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
    					    objtree_cell_data_func,
    					    GUINT_TO_POINTER(COLUMN_OBJTREE_TRICCS_GAIN),
    					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

  switch(hg->inst){
  case INST_KOOLS:
  case INST_TRICCS:
    // Pointing Collection
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_seimei_pc), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_SEIMEI_PC));
    
    column = gtk_tree_view_column_new_with_attributes ("PC",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_SEIMEI_PC,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // Auto Guide
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_seimei_ag), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_SEIMEI_AG));
    
    column = gtk_tree_view_column_new_with_attributes ("AG",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_SEIMEI_AG,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }
  

  switch(hg->inst){
  case INST_KOOLS:
    // No Wait Mode
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_seimei_nw), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_SEIMEI_NW));
    
    column = gtk_tree_view_column_new_with_attributes ("NW",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_SEIMEI_NW,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

  /* Mag column */
  switch(hg->inst){
  case INST_HDS:
  case INST_IRCS:
  case INST_IRD:
  case INST_KOOLS:
  case INST_TRICCS:
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
    break;
  }

  switch(hg->inst){
  case INST_IRCS:
  case INST_IRD:
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,"editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_J));
    column=gtk_tree_view_column_new_with_attributes ("J",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_J,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_j_cell_data_func,
					    (gpointer)hg,
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_J);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,"editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_H));
    column=gtk_tree_view_column_new_with_attributes ("H",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_H,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_h_cell_data_func,
					    (gpointer)hg,
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_H);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,"editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_K));
    column=gtk_tree_view_column_new_with_attributes ("K",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_K,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_k_cell_data_func,
					    (gpointer)hg,
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_K);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

  /* SNR column */
  switch(hg->inst){
  case INST_HDS:
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_SNR));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text",
						     COLUMN_OBJTREE_SNR,
#ifdef USE_GTK3
						     "background-rgba", 
#else
						     "background-gdk", 
#endif
						     COLUMN_OBJTREE_SNR_COL,
						     NULL);
    gtkut_tree_view_column_set_markup(column, "S/N<sub>/Exp</sub>");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_SNR),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }
    

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
#ifdef USE_GTK3
						   "foreground-rgba", 
#else
						   "foreground-gdk", 
#endif
						   COLUMN_OBJTREE_RA_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_cell_data_func,
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
#ifdef USE_GTK3
						   "foreground-rgba", 
#else
						   "foreground-gdk", 
#endif
						   COLUMN_OBJTREE_DEC_COL,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  objtree_cell_data_func,
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
					  objtree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJTREE_EQUINOX),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  if(hg->inst!=INST_IRD){
    /* PA column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_PA));
    column=gtk_tree_view_column_new_with_attributes ("PA",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_PA,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_PA),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  switch(hg->inst){
  case INST_HDS:
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
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_GUIDE),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* Setup */
    {
      gint i_use;
      gchar *tmp;
      
      for(i_use=0;i_use<MAX_USESETUP;i_use++){
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled",
			  G_CALLBACK (cell_toggled), hg);
	g_object_set_data (G_OBJECT (renderer), "column", 
			   GINT_TO_POINTER (COLUMN_OBJTREE_SETUP1+i_use));
	tmp=g_strdup_printf("S%d",i_use+1);
	
	column = gtk_tree_view_column_new_with_attributes (tmp,
							   renderer,
							   "active", 
							   COLUMN_OBJTREE_SETUP1+i_use,
							   NULL);
	g_free(tmp);
	gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
      }
    }
    break;

  case INST_IRCS:
    /* GS column */
    renderer = gtk_cell_renderer_toggle_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_GS));
    
    column = gtk_tree_view_column_new_with_attributes ("GS",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_GS,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
    /* AO Mode */
    renderer = gtk_cell_renderer_combo_new ();
    g_object_set (renderer,
		  "model", aomode_model,
		  "text-column", COLUMN_NUMBER_TEXT,
		  "has-entry", FALSE,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), (gpointer)hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_AOMODE));
    column=gtk_tree_view_column_new_with_attributes ("AO-mode",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_AOMODE,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_AOMODE),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* PAM */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_PAM));
    column=gtk_tree_view_column_new_with_attributes ("PAM",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_PAM,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_PAM),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* ADI column */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_adi), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_ADI));
    
    column = gtk_tree_view_column_new_with_attributes ("ADI",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_ADI,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;

  case INST_HSC:
    /* V min */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAGV));
    column=gtk_tree_view_column_new_with_attributes ("Brightest",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAGV,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAGV),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAGV);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* Sep */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAGSEP));
    column=gtk_tree_view_column_new_with_attributes ("Sep.",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAGSEP,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAGSEP),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAGSEP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* < 7 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG6));
    column=gtk_tree_view_column_new_with_attributes ("< 7",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG6,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG6),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG6);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* 7 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG7));
    column=gtk_tree_view_column_new_with_attributes ("< 8",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG7,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG7),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG7);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* 8 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG8));
    column=gtk_tree_view_column_new_with_attributes ("< 9",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG8,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG8),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG8);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* 9 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG9));
    column=gtk_tree_view_column_new_with_attributes ("< 10",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG9,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG9),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG9);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* 10 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG10));
    column=gtk_tree_view_column_new_with_attributes ("< 11",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG10,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG10),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG10);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* 11 Mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", FALSE,
		  NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_HSC_MAG11));
    column=gtk_tree_view_column_new_with_attributes ("< 12 mag",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_HSC_MAG11,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_HSC_MAG11),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJTREE_HSC_MAG11);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;

  case INST_IRD:
    /* GS column */
    renderer = gtk_cell_renderer_toggle_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_GS));
    
    column = gtk_tree_view_column_new_with_attributes ("GS",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_GS,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  
    /* AO Mode */
    renderer = gtk_cell_renderer_combo_new ();
    g_object_set (renderer,
		  "model", aomode_model,
		  "text-column", COLUMN_NUMBER_TEXT,
		  "has-entry", FALSE,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), (gpointer)hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_AOMODE));
    column=gtk_tree_view_column_new_with_attributes ("AO-mode",
						     renderer,
						     "text",
						     COLUMN_OBJTREE_AOMODE,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    objtree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJTREE_AOMODE),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* ADI column */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (cell_toggled_adi), hg);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJTREE_ADI));
    
    column = gtk_tree_view_column_new_with_attributes ("ADI",
						       renderer,
						       "active", 
						       COLUMN_OBJTREE_ADI,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
  }

  /* Rise column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", FALSE,
                NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJTREE_RISE));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_OBJTREE_RISE,
#ifdef USE_GTK3
						   "foreground-rgba", 
#else
						   "foreground-gdk", 
#endif
						   COLUMN_OBJTREE_RISE_COL,
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Rise<sub>15</sub>");
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
#ifdef USE_GTK3
						   "foreground-rgba", 
#else
						   "foreground-gdk", 
#endif
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
#ifdef USE_GTK3
						   "foreground-rgba", 
#else
						   "foreground-gdk", 
#endif
						   COLUMN_OBJTREE_SET_COL,
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Set<sub>15</sub>");
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

static GtkTreeModel *create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_OBJTREE_COLUMNS, 
			      G_TYPE_BOOLEAN, // check
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_BOOLEAN, // Std
			      G_TYPE_INT,     // Exp
			      G_TYPE_DOUBLE,  // dExp
			      G_TYPE_DOUBLE,  // TExp
                              G_TYPE_INT,     // Repeat

			      G_TYPE_INT,     // KOOLS Grism
			      G_TYPE_INT,     // TriCCS Filter
			      G_TYPE_INT,     // TriCCS Gain
			      G_TYPE_INT,     // TriCCS frames
			      G_TYPE_BOOLEAN, // Seimei PC
			      G_TYPE_BOOLEAN, // Seimei AG
			      G_TYPE_BOOLEAN, // Seimei NW
			      
                              G_TYPE_BOOLEAN, // GS(flag)
                              G_TYPE_DOUBLE,  // Mag
                              G_TYPE_INT,     // MagSrc
                              G_TYPE_DOUBLE,  // J
                              G_TYPE_DOUBLE,  // H
                              G_TYPE_DOUBLE,  // K
                              G_TYPE_DOUBLE,  // SNR
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
                              G_TYPE_DOUBLE,  // ra
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
			      G_TYPE_DOUBLE,  // dec
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
                              G_TYPE_DOUBLE,  // equinox
			      G_TYPE_INT,     // horizon
			      G_TYPE_DOUBLE,  // rise
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
			      G_TYPE_DOUBLE,  // transit
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
			      G_TYPE_DOUBLE,  // set
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,    //color
#else
			      GDK_TYPE_COLOR,   //color
#endif
                              G_TYPE_DOUBLE,  // PA
                              G_TYPE_INT,     // Guide
                              G_TYPE_INT,     // AO-mode
                              G_TYPE_INT,     // PAM
                              G_TYPE_BOOLEAN, // ADI
                              G_TYPE_BOOLEAN, // Set1
                              G_TYPE_BOOLEAN, // Set2
                              G_TYPE_BOOLEAN, // Set3
                              G_TYPE_BOOLEAN, // Set4
                              G_TYPE_BOOLEAN, // Set5
			      G_TYPE_DOUBLE,  // hscmag v
			      G_TYPE_DOUBLE,  // hscmag sep
			      G_TYPE_INT,     // hscmag mag6
			      G_TYPE_INT,     // hscmag mag7
			      G_TYPE_INT,     // hscmag mag8
			      G_TYPE_INT,     // hscmag mag9
			      G_TYPE_INT,     // hscmag mag10
			      G_TYPE_INT,     // hscmag mag11
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
  // Std
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_STD,
		     hg->obj[i_list].std, 
		     -1);
  // Exp
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_EXP, 
		     hg->obj[i_list].exp, 
		     -1);

  // TriCCS Frames/Exp
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_TRICCS_FRAMES, 
		     hg->obj[i_list].triccs.frames, 
		     -1);
  // DExp
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_DEXP, 
		     hg->obj[i_list].dexp, 
		     -1);

  // TExp
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_TEXP, 
		     hg->obj[i_list].dexp, 
		     -1);
  // Repeat
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_REPEAT,
		     hg->obj[i_list].repeat, 
		     -1);

  // Kools Grism
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_KOOLS_GRISM,
		     hg->obj[i_list].kools.grism, 
		     -1);

  // Seimei PC
  switch(hg->inst){
  case INST_KOOLS:
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_PC,
		       hg->obj[i_list].kools.pc, 
		       -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_AG,
		       hg->obj[i_list].kools.ag, 
		       -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_NW,
		       hg->obj[i_list].kools.nw, 
		       -1);
    break;
    
  case INST_TRICCS:
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_PC,
		       hg->obj[i_list].triccs.pc, 
		       -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_SEIMEI_AG,
		       hg->obj[i_list].triccs.ag, 
		       -1);
    break;
  }
    
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

  // JHK
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_J, hg->obj[i_list].magj, 
		     COLUMN_OBJTREE_H, hg->obj[i_list].magh, 
		     COLUMN_OBJTREE_K, hg->obj[i_list].magk, 
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
  else if((fabs(hg->obj[i_list].pm_ra)>100)
	  ||(fabs(hg->obj[i_list].pm_dec)>100)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RA_COL,&color_green,
			COLUMN_OBJTREE_DEC_COL,&color_green,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RA_COL,NULL,
			COLUMN_OBJTREE_DEC_COL,NULL,
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
  ln_get_local_date(hg->obj[i_list].rise,&zonedate, hg->obs_timezone);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_RISE_COL,NULL,
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
  ln_get_local_date(hg->obj[i_list].transit,&zonedate, hg->obs_timezone);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_TRANSIT_COL,NULL,
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
  ln_get_local_date(hg->obj[i_list].set,&zonedate, hg->obs_timezone);
  obj_mod=zonedate.hours*60+zonedate.minutes;
  if((mod_set<obj_mod) || (obj_mod<mod_rise)){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_OBJTREE_SET_COL,NULL,
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

  // AO-mode
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_AOMODE,
		     hg->obj[i_list].aomode, 
		     -1);

  // PAM
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_PAM,
		     hg->obj[i_list].pam, 
		     -1);

  // ADI
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_ADI,
		     hg->obj[i_list].adi, 
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

  // HSC mag
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_OBJTREE_HSC_MAGV, hg->obj[i_list].hscmag.v, 
		     COLUMN_OBJTREE_HSC_MAGSEP, hg->obj[i_list].hscmag.sep, 
		     COLUMN_OBJTREE_HSC_MAG6, hg->obj[i_list].hscmag.mag6, 
		     COLUMN_OBJTREE_HSC_MAG7, hg->obj[i_list].hscmag.mag7, 
		     COLUMN_OBJTREE_HSC_MAG8, hg->obj[i_list].hscmag.mag8, 
		     COLUMN_OBJTREE_HSC_MAG9, hg->obj[i_list].hscmag.mag9, 
		     COLUMN_OBJTREE_HSC_MAG10,hg->obj[i_list].hscmag.mag10, 
		     COLUMN_OBJTREE_HSC_MAG11,hg->obj[i_list].hscmag.mag11, 
		     -1);
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
    else if((fabs(hg->obj[i_list].pm_ra)>100)
	    ||(fabs(hg->obj[i_list].pm_dec)>100)){
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_OBJTREE_RA_COL,&color_green,
			  COLUMN_OBJTREE_DEC_COL,&color_green,
			  -1);
    }
    else{
      gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  COLUMN_OBJTREE_RA_COL,NULL,
			  COLUMN_OBJTREE_DEC_COL,NULL,
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

    case COLUMN_OBJTREE_DEXP:
    case COLUMN_OBJTREE_TEXP:
      {
        gint i;
	gdouble old_dexp;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;
	
	old_dexp = hg->obj[i].dexp;
        hg->obj[i].dexp = atof (new_text);
	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].dexp, -1);

	if(old_dexp!=hg->obj[i].dexp){
	  hg->obj[i].snr=-1;
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter,COLUMN_OBJTREE_SNR,
			      hg->obj[i].snr, -1);
	}

	get_total_basic_exp(hg);
      }
      break;

    case COLUMN_OBJTREE_TRICCS_FRAMES:
      {
        gint i;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;
	
        hg->obj[i].triccs.frames = atoi (new_text);
	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].triccs.frames, -1);
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

	if(g_strtod(new_text,NULL)!=0){
	  if(fabs(hg->obj[i].mag-(gdouble)g_strtod(new_text,NULL))>0.1){	
	    hg->obj[i].magdb_used=0;
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
				COLUMN_OBJTREE_MAGSRC,
				hg->obj[i].magdb_used, -1);
	    hg->obj[i].mag=(gdouble)g_strtod(new_text,NULL);	
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
				hg->obj[i].mag, -1);
	    hg->obj[i].snr=-1;	
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
				COLUMN_OBJTREE_SNR,
				hg->obj[i].snr, -1);
	  }
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

    case COLUMN_OBJTREE_KOOLS_GRISM:
      {
        gint i;
        gint j;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	for(j=0;j<NUM_KOOLS_GRISM;j++){
	  if(!strcmp(new_text,kools_grism_name[j])){
	    hg->obj[i].kools.grism=j;
	  }
	}
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			    hg->obj[i].kools.grism, -1);
      }
      break;

    case COLUMN_OBJTREE_TRICCS_FILTER:
      {
        gint i;
        gint j;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	for(j=0;j<NUM_TRICCS_FILTER;j++){
	  if(!strcmp(new_text,triccs_filter_name[j])){
	    hg->obj[i].triccs.filter=j;
	  }
	}
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			    hg->obj[i].triccs.filter, -1);
      }
      break;

    case COLUMN_OBJTREE_TRICCS_GAIN:
      {
        gint i;
        gint j;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	for(j=0;j<NUM_TRICCS_GAIN;j++){
	  if(!strcmp(new_text,triccs_gain_name[j])){
	    hg->obj[i].triccs.gain=j;
	  }
	}
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			    hg->obj[i].triccs.gain, -1);
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

    case COLUMN_OBJTREE_AOMODE:
      {
        gint i;
        gint i_mode;

        gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
	i--;

	for(i_mode=0;i_mode<NUM_AOMODE;i_mode++){
	  if(!strcmp(new_text,aomode_name[i_mode])){
	    hg->obj[i].aomode=i_mode;
	    if(hg->obj[i].aomode==AOMODE_NO){
	      hg->obj[i].adi=FALSE;
	    }
	    break;
	  }
	}

	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    column, hg->obj[i].aomode,
			    COLUMN_OBJTREE_ADI, hg->obj[i].adi, -1);
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

	if(hg->obj[i].note) g_free(hg->obj[i].note);
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
cell_toggled_std (GtkCellRendererText *cell,
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

  hg->obj[i].std ^= 1;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJTREE_STD, hg->obj[i].std, -1);
  
  gtk_tree_path_free (path);
}


static void
cell_toggled_seimei_pc (GtkCellRendererText *cell,
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

  switch(hg->inst){
  case INST_KOOLS:
    hg->obj[i].kools.pc ^= 1;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_OBJTREE_SEIMEI_PC, hg->obj[i].kools.pc, -1);
    break;

  case INST_TRICCS:
    hg->obj[i].triccs.pc ^= 1;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_OBJTREE_SEIMEI_PC, hg->obj[i].triccs.pc, -1);
    break;
  }
  
  gtk_tree_path_free (path);
}


static void
cell_toggled_seimei_ag (GtkCellRendererText *cell,
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

  switch(hg->inst){
  case INST_KOOLS:
    hg->obj[i].kools.ag ^= 1;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_OBJTREE_SEIMEI_AG, hg->obj[i].kools.ag, -1);
    break;

  case INST_TRICCS:
    hg->obj[i].triccs.ag ^= 1;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_OBJTREE_SEIMEI_AG, hg->obj[i].triccs.ag, -1);
    break;
  }
  
  gtk_tree_path_free (path);
}

static void
cell_toggled_seimei_nw (GtkCellRendererText *cell,
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

  hg->obj[i].kools.nw ^= 1;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COLUMN_OBJTREE_SEIMEI_NW, hg->obj[i].kools.nw, -1);
  
  gtk_tree_path_free (path);
}


static void
cell_toggled_adi (GtkCellRendererText *cell,
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
  
  if(hg->obj[i].i_nst<0){
    if(hg->obj[i].aomode!=AOMODE_NO){
      hg->obj[i].adi ^= 1;
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "ADI (= Angular Differencial Imaging) must be used with AO (NGS or LGS).",
		    NULL);
      hg->obj[i].adi=FALSE;
    }
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "ADI (= Angular Differencial Imaging) cannot be used for non-sidereal targets.",
		  NULL);
    hg->obj[i].adi=FALSE;
  }

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJTREE_ADI, hg->obj[i].adi, -1);
  
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


void objtree_cell_data_func(GtkTreeViewColumn *col , 
			    GtkCellRenderer *renderer,
			    GtkTreeModel *model, 
			    GtkTreeIter *iter,
			    gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint int_value;
  gdouble double_value;
  gchar *str;

  switch (index) {
  case COLUMN_OBJTREE_GUIDE:
  case COLUMN_OBJTREE_AOMODE:
  case COLUMN_OBJTREE_KOOLS_GRISM:
  case COLUMN_OBJTREE_TRICCS_FILTER:
  case COLUMN_OBJTREE_TRICCS_GAIN:
  case COLUMN_OBJTREE_PAM:
  case COLUMN_OBJTREE_REPEAT:
  case COLUMN_OBJTREE_HSC_MAG6:
  case COLUMN_OBJTREE_HSC_MAG7:
  case COLUMN_OBJTREE_HSC_MAG8:
  case COLUMN_OBJTREE_HSC_MAG9:
  case COLUMN_OBJTREE_HSC_MAG10:
  case COLUMN_OBJTREE_HSC_MAG11:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;
    
  case COLUMN_OBJTREE_DEXP:
  case COLUMN_OBJTREE_TEXP:
  case COLUMN_OBJTREE_SNR:
  case COLUMN_OBJTREE_RA:
  case COLUMN_OBJTREE_DEC:
  case COLUMN_OBJTREE_EQUINOX:
  case COLUMN_OBJTREE_PA:
  case COLUMN_OBJTREE_HSC_MAGV:
  case COLUMN_OBJTREE_HSC_MAGSEP:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_OBJTREE_GUIDE:
    switch(int_value){
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

  case COLUMN_OBJTREE_AOMODE:
    str=g_strdup(aomode_name[int_value]);
    break;

  case COLUMN_OBJTREE_KOOLS_GRISM:
    str=g_strdup(kools_grism_name[int_value]);
    break;
    
  case COLUMN_OBJTREE_TRICCS_FILTER:
    str=g_strdup(triccs_filter_name[int_value]);
    break;
    
  case COLUMN_OBJTREE_TRICCS_GAIN:
    str=g_strdup(triccs_gain_name[int_value]);
    break;
    
  case COLUMN_OBJTREE_PAM:
    if(int_value<0){
      str=NULL;
    }
    else{
      str=g_strdup_printf("%d", int_value);
    }
    break;

  case COLUMN_OBJTREE_REPEAT:
    str=g_strdup_printf("x%2d",int_value);
    break;

  case COLUMN_OBJTREE_HSC_MAG6:
  case COLUMN_OBJTREE_HSC_MAG7:
  case COLUMN_OBJTREE_HSC_MAG8:
  case COLUMN_OBJTREE_HSC_MAG9:
  case COLUMN_OBJTREE_HSC_MAG10:
  case COLUMN_OBJTREE_HSC_MAG11:
    if(int_value<0){
      str=NULL;
    }
    else{
      str=g_strdup_printf("%d",int_value);
    }
    break;
    

    // double //
  case COLUMN_OBJTREE_DEXP:
    if(double_value<10){
      str=g_strdup_printf("%.1lf",double_value);
    }
    else{
      str=g_strdup_printf("%.0lf",double_value);
    }
    break;

  case COLUMN_OBJTREE_TEXP:
    str=g_strdup_printf("%.3lf",double_value);
    break;

  case COLUMN_OBJTREE_SNR:
    if(double_value>0)
      str=g_strdup_printf("%.1lf",double_value);
    else
      str=NULL;
    break;

  case COLUMN_OBJTREE_RA:
    str=g_strdup_printf("%09.2lf",double_value);
    break;

  case COLUMN_OBJTREE_DEC:
    str=g_strdup_printf("%+010.2lf",double_value);
    break;

  case COLUMN_OBJTREE_EQUINOX:
    str=g_strdup_printf("%7.2lf",double_value);
    break;

  case COLUMN_OBJTREE_PA:
    str=g_strdup_printf("%+.1lf",double_value);
    break;
    
  case COLUMN_OBJTREE_HSC_MAGV:
    if(double_value > 99){
      str=NULL;
    }
    else{
      str=g_strdup_printf("%.2lf",double_value);
    }
    break;
    
  case COLUMN_OBJTREE_HSC_MAGSEP:
    {
      gdouble sec;

      sec=double_value*3600.;

      if(double_value>99){
	str=NULL;
      }
      else if(sec<60){
	str=g_strdup_printf("%.1lf\"",sec);
      }
      else{
	str=g_strdup_printf("%d\'%02d\"",
			    (gint)(sec/60),
			    ((gint)sec%60));
      }
    }
    break;    
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void objtree_j_cell_data_func(GtkTreeViewColumn *col , 
			      GtkCellRenderer *renderer,
			      GtkTreeModel *model, 
			      GtkTreeIter *iter,
			      gpointer user_data)
{
  gchar *str=NULL;
  gdouble double_value;
  gint i, hits;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_J, &double_value,
		      -1);
  
  if(double_value>99){
    str=NULL;
  }
  else if (fabs(double_value-hg->obj[i].magdb_2mass_j)>0.01){
    str=g_strdup_printf("%.2lf*",double_value);
  }
  else{
    str=g_strdup_printf("%.2lf",double_value);
  }
  
  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}


void objtree_h_cell_data_func(GtkTreeViewColumn *col , 
			      GtkCellRenderer *renderer,
			      GtkTreeModel *model, 
			      GtkTreeIter *iter,
			      gpointer user_data)
{
  gchar *str=NULL;
  gdouble double_value;
  gint i, hits;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_H, &double_value,
		      -1);
  
  if(double_value>99){
    str=NULL;
  }
  else if (fabs(double_value-hg->obj[i].magdb_2mass_h)>0.01){
    str=g_strdup_printf("%.2lf*",double_value);
  }
  else{
    str=g_strdup_printf("%.2lf",double_value);
  }
  
  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
}


void objtree_k_cell_data_func(GtkTreeViewColumn *col , 
			      GtkCellRenderer *renderer,
			      GtkTreeModel *model, 
			      GtkTreeIter *iter,
			      gpointer user_data)
{
  gchar *str=NULL;
  gdouble double_value;
  gint i, hits;
  typHOE *hg=(typHOE *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_NUMBER, &i,
		      -1);
  i--;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJTREE_K, &double_value,
		      -1);
  
  if(double_value>99){
    str=NULL;
  }
  else if (fabs(double_value-hg->obj[i].magdb_2mass_k)>0.01){
    str=g_strdup_printf("%.2lf*",double_value);
  }
  else{
    str=g_strdup_printf("%.2lf",double_value);
  }
  
  g_object_set(renderer, "text", str, NULL);
  if(str) g_free(str);
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

    case MAGDB_TYPE_UCAC:
      hits=hg->obj[i].magdb_ucac_hits;
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

    case MAGDB_TYPE_KEPLER:
      hits=hg->obj[i].magdb_kepler_hits;
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
    ln_get_local_date (hg->obj[i].rise, &zonedate, hg->obs_timezone);
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
    ln_get_local_date (hg->obj[i].set, &zonedate, hg->obs_timezone);
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
    ln_get_local_date (hg->obj[i].transit, &zonedate, hg->obs_timezone);
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
  gchar *str;

  /* create list store */
  model = gtk_list_store_new (NUM_NUMBER_COLUMNS, 
			      G_TYPE_STRING, 
			      G_TYPE_INT);

  /* add numbers */
  for (i = 0; i < N_REPEAT; i++)
    {
      str=g_strdup_printf("x%2d",i+1);
      
      gtk_list_store_append (model, &iter);
      
      gtk_list_store_set (model, &iter,
                          COLUMN_NUMBER_TEXT, str,
                          -1);
      g_free(str);
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


static GtkTreeModel *
create_kools_grism_model (void)
{
  GtkListStore *model;
  GtkTreeIter iter;
  gint j;

  /* create list store */
  model = gtk_list_store_new (2, 
			      G_TYPE_STRING,
			      G_TYPE_INT);

  for(j=0;j<NUM_KOOLS_GRISM;j++){
    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter, 
			0, kools_grism_name[j],
			1, j,
			-1);
  }

  return GTK_TREE_MODEL (model);

}


static GtkTreeModel *
create_triccs_filter_model (void)
{
  GtkListStore *model;
  GtkTreeIter iter;
  gint j;

  /* create list store */
  model = gtk_list_store_new (2, 
			      G_TYPE_STRING,
			      G_TYPE_INT);

  for(j=0;j<NUM_TRICCS_FILTER;j++){
    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter, 
			0, triccs_filter_name[j],
			1, j,
			-1);
  }

  return GTK_TREE_MODEL (model);

}


static GtkTreeModel *
create_triccs_gain_model (void)
{
  GtkListStore *model;
  GtkTreeIter iter;
  gint j;

  /* create list store */
  model = gtk_list_store_new (2, 
			      G_TYPE_STRING,
			      G_TYPE_INT);

  for(j=0;j<NUM_TRICCS_GAIN;j++){
    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter, 
			0, triccs_gain_name[j],
			1, j,
			-1);
  }

  return GTK_TREE_MODEL (model);

}


static GtkTreeModel *
create_aomode_model (void)
{
  GtkListStore *model;
  GtkTreeIter iter;
  gint i_mode;

  /* create list store */
  model = gtk_list_store_new (2, 
			      G_TYPE_STRING,
			      G_TYPE_INT);

  for(i_mode=0;i_mode<NUM_AOMODE;i_mode++){
    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter, 
			0, aomode_name[i_mode],
			1, i_mode,
			-1);
  }    

  return GTK_TREE_MODEL (model);
}




void
add_item_objtree (typHOE *hg)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  gint i,i_list,i_use, i_plan, i_band;
  GtkTreePath *path;

  if(hg->i_max>=MAX_OBJECT) return;

  i=hg->i_max;

  init_obj(&hg->obj[i], hg);

  if(hg->obj[i].name) g_free(hg->obj[i].name);
  hg->obj[i].name=g_strdup(hg->addobj_name);
  hg->obj[i].ra=hg->addobj_ra;
  hg->obj[i].dec=hg->addobj_dec;
  hg->obj[i].pm_ra=hg->addobj_pm_ra;
  hg->obj[i].pm_dec=hg->addobj_pm_dec;
  hg->obj[i].equinox=2000.0;

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    hg->obj[i].magdb_simbad_hits=1;
    hg->obj[i].magdb_simbad_sep=0;

    hg->obj[i].magdb_simbad_u=hg->addobj_u;
    hg->obj[i].magdb_simbad_b=hg->addobj_b;
    hg->obj[i].magdb_simbad_v=hg->addobj_v;
    hg->obj[i].magdb_simbad_r=hg->addobj_r;
    hg->obj[i].magdb_simbad_i=hg->addobj_i;
    hg->obj[i].magdb_simbad_j=hg->addobj_j;
    hg->obj[i].magdb_simbad_h=hg->addobj_h;
    hg->obj[i].magdb_simbad_k=hg->addobj_k;

    if(hg->obj[i].magdb_simbad_r<99){
      hg->obj[i].mag=hg->obj[i].magdb_simbad_r;
      hg->obj[i].magdb_used=MAGDB_TYPE_SIMBAD;
      hg->obj[i].magdb_band=FCDB_BAND_R;
    }
    else if(hg->obj[i].magdb_simbad_v<99){
      hg->obj[i].mag=hg->obj[i].magdb_simbad_v;
      hg->obj[i].magdb_used=MAGDB_TYPE_SIMBAD;
      hg->obj[i].magdb_band=FCDB_BAND_V;
    }

    hg->obj[i].magj=hg->addobj_j;
    hg->obj[i].magh=hg->addobj_h;
    hg->obj[i].magk=hg->addobj_k;
    
    if(hg->obj[i].magdb_simbad_name)
      g_free(hg->obj[i].magdb_simbad_name);
    hg->obj[i].magdb_simbad_name=g_strdup(hg->addobj_voname);

    if(hg->obj[i].magdb_simbad_sp)
      g_free(hg->obj[i].magdb_simbad_sp);
    hg->obj[i].magdb_simbad_sp=g_strdup(hg->addobj_vosp);
    
    if(hg->obj[i].magdb_simbad_type)
      g_free(hg->obj[i].magdb_simbad_type);
    hg->obj[i].magdb_simbad_type=g_strdup(hg->addobj_votype);
    break;
    
  case FCDB_TYPE_NED:
    hg->obj[i].magdb_simbad_hits=1;
    hg->obj[i].magdb_simbad_sep=0;
    break;
  }
  
  if(hg->obj[i].note) g_free(hg->obj[i].note);
  if(hg->addobj_votype){
    if(hg->addobj_magsp)
      hg->obj[i].note=g_strdup_printf("%s, %s",hg->addobj_votype,hg->addobj_magsp);
    else
      hg->obj[i].note=g_strdup_printf("%s, mag=unknown",hg->addobj_votype);
  }
  else{
    hg->obj[i].note=NULL;
  }
  
  hg->i_max++;
  
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


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if(i>0){
      swap_obj(&hg->obj[i-1], &hg->obj[i]);
      
      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
      recalc_rst(hg);
      objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i-1);
      objtree_update_item(hg, GTK_TREE_MODEL(model), iter, i);

      for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  if(hg->plan[i_plan].obj_i==i){
	    hg->plan[i_plan].obj_i=i-1;
	  }
	  else if(hg->plan[i_plan].obj_i==i-1){
	    hg->plan[i_plan].obj_i=i;
	  }
	}
      }
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


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list, i_plan;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    if(i<hg->i_max-1){
      swap_obj(&hg->obj[i], &hg->obj[i+1]);

      gtk_tree_path_next (path);
      gtk_tree_selection_select_path(selection, path);

      for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
	if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	  if(hg->plan[i_plan].obj_i==i){
	    hg->plan[i_plan].obj_i=i+1;
	  }
	  else if(hg->plan[i_plan].obj_i==i+1){
	    hg->plan[i_plan].obj_i=i;
	  }
	}
      }
    }
    
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
    gint i, i_list,j, i_plan, i_band;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    gtk_tree_path_free (path);

    for(i_list=i;i_list<hg->i_max-1;i_list++){
      swap_obj(&hg->obj[i_list], &hg->obj[i_list+1]);
    }

    hg->i_max--;
    
    if (gtk_tree_model_iter_nth_child(model, &iter, NULL, hg->i_max)){
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

      if(hg->obj[hg->i_max].name) g_free(hg->obj[hg->i_max].name);
      hg->obj[hg->i_max].name=NULL;
      if(hg->obj[hg->i_max].note) g_free(hg->obj[hg->i_max].note);
      hg->obj[hg->i_max].note=NULL;
      if(hg->obj[hg->i_max].gs.name) g_free(hg->obj[hg->i_max].gs.name);
      hg->obj[hg->i_max].gs.name=NULL;
      if(hg->obj[hg->i_max].trdb_str) g_free(hg->obj[hg->i_max].trdb_str);
      hg->obj[hg->i_max].trdb_str=NULL;
      for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
	if(hg->obj[hg->i_max].trdb_mode[i_band]) 
	  g_free(hg->obj[hg->i_max].trdb_mode[i_band]);
	hg->obj[hg->i_max].trdb_mode[i_band]=NULL;
	if(hg->obj[hg->i_max].trdb_band[i_band])
	  g_free(hg->obj[hg->i_max].trdb_band[i_band]);
	hg->obj[hg->i_max].trdb_band[i_band]=NULL;
      }
    }

    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	if(hg->plan[i_plan].obj_i==i){
	  init_planpara(hg, i_plan);
	  hg->plan[i_plan].type=PLAN_TYPE_COMMENT;
	  hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;
	  hg->plan[i_plan].txt=g_strdup("### (The object was removed from the list.) ###");
	  hg->plan[i_plan].comment=g_strdup(" (The object was removed from the list.) ");
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
  gchar *simbad_host;


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
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      tmp=g_strdup_printf(SIMBAD_URL,
			  simbad_host,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      g_free(simbad_host);
      break;

    case WWWDB_NED:
      tmp=g_strdup_printf(NED_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_TRANSIENT:
      tmp=g_strdup_printf(TRANSIENT_URL,
			  ln_hms_to_deg(&hobject_prec.ra),
			  ln_dms_to_deg(&hobject_prec.dec));
      break;

    case WWWDB_DR8:
      tmp=g_strdup_printf(DR8_URL,
			  hobject_prec.ra.hours,hobject_prec.ra.minutes,
			  hobject_prec.ra.seconds,
			  (hobject_prec.dec.neg) ? "-" : "+", 
			  hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			  hobject_prec.dec.seconds);
      break;

    case WWWDB_SDSS_DRNOW:
      tmp=g_strdup_printf(SDSS_DRNOW_URL,
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
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(SSLOC_URL,
			    simbad_host,
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
			    simbad_host,
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
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%26",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
			    hg->std_sptype2,MAX_STD);
      }
      g_free(simbad_host);
      break;
    case WWWDB_RAPID:
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(RAPID_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	tmp=g_strdup_printf(RAPID_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else{
	tmp=g_strdup_printf(RAPID_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%26",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      g_free(simbad_host);
      break;
    case WWWDB_MIRSTD:
      if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
      }
      else{
	simbad_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
      }
     if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	tmp=g_strdup_printf(MIRSTD_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	tmp=g_strdup_printf(MIRSTD_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%7c",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else{
	tmp=g_strdup_printf(MIRSTD_URL,
			    simbad_host,
			    ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
			    "%26",
			    ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
			    ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
			    ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
			    hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      g_free(simbad_host);
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




void hds_export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].guide=hg->def_guide;
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


void ircs_export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].aomode=hg->def_aomode;
    hg->obj[i_list].pa=hg->def_pa;
  }
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_AOMODE, hg->obj[i_list].aomode, 
		       COLUMN_OBJTREE_PA, hg->obj[i_list].pa, 
		       -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}

void ird_export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].aomode=hg->def_aomode;
  }
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_AOMODE, hg->obj[i_list].aomode, 
		       -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

}


void hsc_export_def (typHOE *hg)
{
  int i_list;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));

  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].pa=hg->def_pa;
  }
  
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;

  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJTREE_PA, hg->obj[i_list].pa, 
		       -1);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
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
      hg->pam_slot_i=-1;

      gtk_tree_path_free (path);
    }
  
  if(flagPlot){
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,hg);
  }
  
  if(flagPAM){
    pam_update_dialog(hg);
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


void pm_objtree_item (GtkWidget *widget, gpointer data)
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

    hg->pm_i=i;

    pm_dialog(hg);

    gtk_tree_path_free (path);

    update_objtree(hg);
  }
}


void etc_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  gboolean ret=TRUE;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    hg->etc_i=i;
    hg->etc_exptime=hg->obj[i].exp;
    
    if(hg->obj[i].mag>99){
      ret=hds_svcmag(hg, ETC_OBJTREE);
    }

    if(ret){
      hg->etc_mode=ETC_OBJTREE;
      hds_do_etc(NULL, (gpointer)hg);
      hg->etc_mode=ETC_MENU;
    }

    gtk_tree_path_free (path);
  }
}



void sh_objtree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->objtree));
  gboolean ret=TRUE;
  gint i_tmp;


  if(!do_seimei_sh_queue_dialog(hg)){
    return;
  }
  

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;

    hg->sh_i=i;

    gtk_tree_path_free (path);

    i_tmp=hg->fcdb_type;
  
    
    switch(hg->inst){
    case INST_KOOLS:
      hg->sh_type=MAGDB_TYPE_SEIMEI_KOOLS;
      hg->fcdb_type=hg->sh_type;
      sh_dl(hg);
      break;
      
    case INST_TRICCS:
      hg->sh_type=MAGDB_TYPE_SEIMEI_TRICCS;
      hg->fcdb_type=hg->sh_type;
      sh_dl(hg);
      break;
    }
    
    hg->fcdb_type=i_tmp;

    create_shedit_dialog(hg);
  }
}


void pam_objtree_item (GtkWidget *widget, gpointer data)
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

    if((hg->lgs_pam_i_max>0) && (hg->obj[i].pam>=0)){
      hg->plot_i=i;
      create_pam_dialog(hg);
    }

    gtk_tree_path_free (path);
  }
}


static void ok_addobj(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if((fabs(hg->addobj_ra)<0.01)&&(fabs(hg->addobj_dec)<0.01)){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please input RA and Dec of the target",
		  NULL);
  }
  else{
    gtk_main_quit();
    add_item_objtree(hg);
  }
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

static void addobj_transient_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;
  gint i_tmp;

  i_tmp=hg->fcdb_type;
  
  hg->addobj_type=ADDOBJ_TYPE_TRANSIENT;
  hg->fcdb_type=hg->addobj_type;
  addobj_dl(hg);
  
  hg->fcdb_type=i_tmp;
}

void addobj_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gchar *tgt;
  gchar *tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);
  

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    tgt=make_simbad_id(hg->addobj_name);
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_SIMBAD_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
    }
    else{
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
    }
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    break;

  case FCDB_TYPE_NED:
    tgt=make_simbad_id(hg->addobj_name);
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_NED_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_NED);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    break;

  default: // Transient
    tgt=strip_spc(hg->addobj_name);
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_TRANSIENT_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(ADDOBJ_TRANSIENT_HOST);
    break;
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    break;
  }
  g_free(tgt);

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    tmp=g_strdup("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    tmp=g_strdup("Searching objects in NED ...");
    break;

  default: // Transient
    tmp=g_strdup("Searching objects in Transient Name Server ...");
    break;
  }
  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Query by name to the database",
		 tmp,
		 FALSE, FALSE);
  g_free(tmp);
  my_signal_connect(hg->pdialog,"delete-event", delete_fcdb, (gpointer)hg);

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in NED ...");
    break;

  default: // Transient
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in Transient Name Server ...");
    break;
  }
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),
			       button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    thread_cancel_fcdb, 
		    (gpointer)hg);
  
  gtk_widget_show_all(hg->pdialog);
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),TRUE);

  hg->ploop=g_main_loop_new(NULL, FALSE);
  hg->pcancel=g_cancellable_new();
  hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
  g_main_loop_run(hg->ploop);
  //g_thread_join(hg->pthread);
  g_main_loop_unref(hg->ploop);
  hg->ploop=NULL;

  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  gtk_widget_destroy(hg->pdialog);

  flag_getFCDB=FALSE;

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
  case FCDB_TYPE_NED:
    addobj_vo_parse(hg);
    break;

  default:
    addobj_transient_txt_parse(hg);
    break;
  }

  if(hg->addobj_voname){
    tmp=g_strdup_printf("%09.2lf",hg->addobj_ra);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+010.2lf",hg->addobj_dec);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+.2lf",hg->addobj_pm_ra);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_ra),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+.2lf",hg->addobj_pm_dec);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_dec),tmp);
    g_free(tmp);

    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"<b>%s</b>\" (<i>%s</i>) in SIMBAD.",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"<b>%s</b>\" (<i>%s</i>) in NED.",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;

    default: // Transient
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"<b>%s</b>\" (<i>%s</i>) in Transient Name Server.",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;
    }
    gtk_label_set_markup(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  else{
    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input \"%s\" is not found in SIMBAD.</span>",
			  hg->addobj_name); 
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input \"%s\" is not found in NED.</span>",
			  hg->addobj_name); 
      break;
      
    default:
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input \"%s\" is not found in Transient Name Server.</span>",
			  hg->addobj_name); 
      break;
    }
    gtk_label_set_markup(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  
}

static void pm_simbad_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->pm_type=FCDB_TYPE_SIMBAD;
  pm_dl(hg);
}

static void pm_gaia_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->pm_type=FCDB_TYPE_GAIA;
  pm_dl(hg);
}

void sh_dl(typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gchar *tmp;
  gchar *url_param, *mag_str, *otype_str;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);

  hg->fcdb_i=hg->sh_i;
  hg->fcdb_post=TRUE;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(MAGDB_HOST_SEIMEI_SH);
  
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  switch(hg->inst){
  case INST_KOOLS:
    hg->fcdb_path=g_strdup(MAGDB_PATH_SEIMEI_SH_KOOLS);
    break;

  case INST_TRICCS:
    hg->fcdb_path=g_strdup(MAGDB_PATH_SEIMEI_SH_TRICCS);
    break;
  }
    

  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_TXT,NULL);
  
  tmp=g_strdup("Retrieving Shell Script via www ...");

  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Retrieving Shell Script via www",
		 tmp,
		 FALSE, FALSE);
  
  g_free(tmp);
  my_signal_connect(hg->pdialog,"delete-event", delete_fcdb, (gpointer)hg);

  gtk_label_set_markup(GTK_LABEL(hg->plabel),
		       "Retrieving Shell-Script via WWW ...");
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    thread_cancel_fcdb, 
		    (gpointer)hg);
  
  gtk_widget_show_all(hg->pdialog);
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),TRUE);

  hg->ploop=g_main_loop_new(NULL, FALSE);
  hg->pcancel=g_cancellable_new();
  hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
  g_main_loop_run(hg->ploop);
  //g_thread_join(hg->pthread);
  g_main_loop_unref(hg->ploop);
  hg->ploop=NULL;

  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  gtk_widget_destroy(hg->pdialog);

  flag_getFCDB=FALSE;
}


void pm_dl(typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  GtkTreeIter iter;
  GtkWidget *button;
  gint timer=-1;
  gchar *tmp;
  gchar *url_param, *mag_str, *otype_str;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  if(access(hg->fcdb_file, F_OK)==0) unlink(hg->fcdb_file);

  hg->fcdb_i=hg->pm_i;

  object.ra=ra_to_deg(hg->obj[hg->pm_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->pm_i].dec);
  
  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->pm_i].equinox),
		    JD2000, &object_prec);
  
  switch(hg->pm_type){
  case FCDB_TYPE_SIMBAD:
    switch(hg->magdb_simbad_band){
    case FCDB_BAND_NOP:
      mag_str=g_strdup("%0D%0A");
      break;
    case FCDB_BAND_U:
    case FCDB_BAND_B:
    case FCDB_BAND_V:
    case FCDB_BAND_R:
    case FCDB_BAND_I:
    case FCDB_BAND_J:
    case FCDB_BAND_H:
    case FCDB_BAND_K:
      mag_str=g_strdup_printf("%%26%smag<%d",
			      simbad_band[hg->magdb_simbad_band],
			      hg->magdb_mag);
      break;
    }
    
    otype_str=g_strdup("%0D%0A");
    
    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
    }
    else{
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
    }

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(FCDB_SIMBAD_PATH_R,
				  hg->fcdb_d_ra0,
				  (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				  fabs(hg->fcdb_d_dec0),
				  (gdouble)hg->magdb_arcsec/60.,
				  mag_str,otype_str,
				  MAX_FCDB);
    g_free(mag_str);
    g_free(otype_str);
    break;

  case FCDB_TYPE_GAIA:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    
    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    url_param=g_strdup_printf("&Gmag=%%3C%d&",hg->magdb_mag);

    switch(hg->gaia_dr){
    case GAIA_DR2:
      hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_R,
				    hg->fcdb_d_ra0,
				    hg->fcdb_d_dec0,
				    hg->magdb_arcsec,
				    url_param);
      break;
    case GAIA_EDR3:
      hg->fcdb_path=g_strdup_printf(FCDB_GAIA_E3_PATH_R,
				    hg->fcdb_d_ra0,
				    hg->fcdb_d_dec0,
				    hg->magdb_arcsec,
				    url_param);
      break;
    case GAIA_DR3:
      hg->fcdb_path=g_strdup_printf(FCDB_GAIA_DR3_PATH_R,
				    hg->fcdb_d_ra0,
				    hg->fcdb_d_dec0,
				    hg->magdb_arcsec,
				    url_param);
      break;
    }
      
    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    break;
  }

  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);


  switch(hg->pm_type){
  case FCDB_TYPE_SIMBAD:
    tmp=g_strdup("Searching objects in SIMBAD ...");
    break;
    
  case FCDB_TYPE_GAIA:
    tmp=g_strdup("Searching objects in GAIA DR2 ...");
    break;
  }
  create_pdialog(hg,
		 hg->w_top,
		 "HOE : Query by coordinate to the database",
		 tmp,
		 FALSE, FALSE);
  g_free(tmp);
  my_signal_connect(hg->pdialog,"delete-event", delete_fcdb, (gpointer)hg);

  switch(hg->pm_type){
  case FCDB_TYPE_SIMBAD:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_GAIA:
    gtk_label_set_markup(GTK_LABEL(hg->plabel),
			 "Searching objects in GAIA DR2 ...");
    break;
  }
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pdialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    thread_cancel_fcdb, 
		    (gpointer)hg);
  
  gtk_widget_show_all(hg->pdialog);
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),TRUE);

  hg->ploop=g_main_loop_new(NULL, FALSE);
  hg->pcancel=g_cancellable_new();
  hg->pthread=g_thread_new("hoe_fcdb", thread_get_fcdb, (gpointer)hg);
  g_main_loop_run(hg->ploop);
  //g_thread_join(hg->pthread);
  g_main_loop_unref(hg->ploop);
  hg->ploop=NULL;

  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  gtk_widget_destroy(hg->pdialog);

  flag_getFCDB=FALSE;

  switch(hg->pm_type){
  case FCDB_TYPE_SIMBAD:
    fcdb_simbad_vo_parse(hg, TRUE);

    if(hg->obj[hg->pm_i].magdb_simbad_name){
      tmp=g_strdup_printf("%+.2lf",hg->obj[hg->pm_i].pm_ra);
      gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_ra),tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("%+.2lf",hg->obj[hg->pm_i].pm_dec);
      gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_dec),tmp);
      g_free(tmp);

      tmp=g_strdup_printf("Your input coordinate matches with \"<b>%s</b>\" (<i>%s</i>) in SIMBAD.",
			  hg->obj[hg->pm_i].magdb_simbad_name, 
			  hg->obj[hg->pm_i].magdb_simbad_type);
    }
    else{
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input does not match any objects in SIMBAD.</span>");
    }
    break;

  case FCDB_TYPE_GAIA:
    fcdb_gaia_vo_parse(hg, TRUE);
    
    if(hg->obj[hg->pm_i].magdb_gaia_g<99){
      tmp=g_strdup_printf("%+.2lf",hg->obj[hg->pm_i].pm_ra);
      gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_ra),tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("%+.2lf",hg->obj[hg->pm_i].pm_dec);
      gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_dec),tmp);
      g_free(tmp);

      tmp=g_strdup_printf("Your input coordinate matches with a <i>G=%.2lf mag</i> star in GAIA DR2.",
			  hg->obj[hg->pm_i].magdb_gaia_g); 
    }
    else{
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input does not match any objects in GAIA.</span>");
    }
    break;
  }
  gtk_label_set_markup(GTK_LABEL(hg->pm_label),tmp);
  g_free(tmp);

  rebuild_trdb_tree(hg);
}


gchar*  pm_get_new_radec(typHOE *hg){
  gchar *tmp=NULL;
  gdouble new_ra, new_dec, new_d_ra, new_d_dec, yrs;
  
  yrs=current_yrs(hg);
  new_d_ra=ra_to_deg(hg->obj[hg->pm_i].ra)+
    hg->obj[hg->pm_i].pm_ra/1000/60/60*yrs;
  new_d_dec=dec_to_deg(hg->obj[hg->pm_i].dec)+
    hg->obj[hg->pm_i].pm_dec/1000/60/60*yrs;
  
  new_ra=deg_to_ra(new_d_ra);
  new_dec=deg_to_dec(new_d_dec);
  
  if((fabs(hg->obj[hg->pm_i].pm_ra)>100)
     ||(fabs(hg->obj[hg->pm_i].pm_dec)>100)){
    tmp=g_strdup_printf("  <b>Current :</b>  <span color=\"#FF0000\">RA = %09.2lf   Dec = %+010.2lf</span>  (%.2lf)",
			new_ra,
			new_dec,
			2000.0);
  }
  else{
    tmp=g_strdup_printf("  <b>Current :</b>  RA = %09.2lf   Dec = %+010.2lf  (%.2lf)",
			new_ra,
			new_dec,
			2000.0);
  }

  return(tmp);
}

void cc_get_entry_pm_ra (GtkWidget *widget, gdouble *gdata)
{
  typHOE *hg=(typHOE *)gdata;
  gchar *tmp;
  
  hg->obj[hg->pm_i].pm_ra
    =(gdouble)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);

  tmp=pm_get_new_radec(hg);
  gtk_label_set_markup(GTK_LABEL(hg->pm_label_radec), tmp);
  g_free(tmp);
}

void cc_get_entry_pm_dec (GtkWidget *widget, gdouble *gdata)
{
  typHOE *hg=(typHOE *)gdata;
  gchar *tmp;
  
  hg->obj[hg->pm_i].pm_dec
    =(gdouble)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);

  tmp=pm_get_new_radec(hg);
  gtk_label_set_markup(GTK_LABEL(hg->pm_label_radec), tmp);
  g_free(tmp);
}

void pm_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox,
    *spinner, *table, *entry, *bar;
  GtkAdjustment *adj;
  gchar *tmp=NULL;
  GSList *fcdb_group=NULL; 
  gdouble tmp_ra, tmp_dec;
  gboolean rebuild_flag=FALSE;
  GtkWidget *label_pm_radec;

  tmp_ra=hg->obj[hg->pm_i].pm_ra;
  tmp_dec=hg->obj[hg->pm_i].pm_dec;
 
  dialog = gtk_dialog_new_with_buttons("HOE : Edit Proper Motion",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_Clear",GTK_RESPONSE_APPLY,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_CLEAR,GTK_RESPONSE_APPLY,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_CANCEL));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  //my_signal_connect(dialog,"delete-event", delete_main_quit, NULL);

  tmp=g_strdup_printf("Object-%d = \"<b>%s</b>\"",
		      hg->pm_i+1,
		      hg->obj[hg->pm_i].name);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 5);

  tmp=g_strdup_printf("  <b>Input :</b>  RA = %09.2lf   Dec = %+010.2lf  (%.2lf)",
		      hg->obj[hg->pm_i].ra,
		      hg->obj[hg->pm_i].dec,
		      hg->obj[hg->pm_i].equinox);
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 5);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new ("Proper Motion (mas/yr) :  RA ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->pm_entry_pm_ra = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->pm_entry_pm_ra,FALSE, FALSE, 0);
  tmp=g_strdup_printf("%.2lf", hg->obj[hg->pm_i].pm_ra);
  gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_ra), tmp);
  g_free(tmp);
  gtk_editable_set_editable(GTK_EDITABLE(hg->pm_entry_pm_ra),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->pm_entry_pm_ra),10);
  my_signal_connect (hg->pm_entry_pm_ra, "changed", cc_get_entry_pm_ra, 
		     (gpointer)hg);


  label = gtk_label_new ("    Dec ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->pm_entry_pm_dec = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->pm_entry_pm_dec,FALSE, FALSE, 0);
  tmp=g_strdup_printf("%.2lf", hg->obj[hg->pm_i].pm_dec);
  gtk_entry_set_text(GTK_ENTRY(hg->pm_entry_pm_dec), tmp);
  g_free(tmp);
  gtk_editable_set_editable(GTK_EDITABLE(hg->pm_entry_pm_dec),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->pm_entry_pm_dec),10);
  my_signal_connect (hg->pm_entry_pm_dec, "changed", cc_get_entry_pm_dec, 
		     (gpointer)hg);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("SIMBAD", "edit-find");
#else
  button=gtkut_button_new_from_stock("SIMBAD", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", pm_simbad_query, (gpointer)hg);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("GAIA", "edit-find");
#else
  button=gtkut_button_new_from_stock("GAIA", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", pm_gaia_query, (gpointer)hg);

  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  hg->pm_label = gtkut_label_new ("Check the proper motion using input coordinate via SIMBAD / GAIA DR2.");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->pm_label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (hg->pm_label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->pm_label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->pm_label,FALSE, FALSE, 0);


#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  tmp=pm_get_new_radec(hg);
  hg->pm_label_radec = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->pm_label_radec, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->pm_label_radec, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->pm_label_radec), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->pm_label_radec,FALSE, FALSE, 5);

  gtk_widget_show_all(dialog);

  switch (gtk_dialog_run(GTK_DIALOG(dialog))){
  case GTK_RESPONSE_OK:
    break;

  case GTK_RESPONSE_APPLY:
    // Clear
    hg->obj[hg->pm_i].pm_ra=0.0;
    hg->obj[hg->pm_i].pm_dec=0.0;
    break;

  default:
    hg->obj[hg->pm_i].pm_ra=tmp_ra;
    hg->obj[hg->pm_i].pm_dec=tmp_dec;
    break;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
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
  hg->addobj_pm_ra=0;
  hg->addobj_pm_dec=0;
  if(hg->addobj_votype) g_free(hg->addobj_votype);
  hg->addobj_votype=NULL;
  if(hg->addobj_magsp) g_free(hg->addobj_magsp);
  hg->addobj_magsp=NULL;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Add Object");
  my_signal_connect(dialog,"delete-event", delete_main_quit, NULL);


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  
  label = gtk_label_new ("Object Name");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE, FALSE, 0);
  my_signal_connect (entry, "changed", cc_get_entry, &hg->addobj_name);
  gtk_entry_set_text(GTK_ENTRY(entry), "(New Object)");
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),30);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("SIMBAD", "edit-find");
#else
  button=gtkut_button_new_from_stock("SIMBAD", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_simbad_query, (gpointer)hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("NED", "edit-find");
#else
  button=gtkut_button_new_from_stock("NED", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_ned_query, (gpointer)hg);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Transient", "edit-find");
#else
  button=gtkut_button_new_from_stock("Transient", GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_transient_query, (gpointer)hg);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  hg->addobj_label = gtkut_label_new ("Input Object Name to be added &amp; resolve its coordinate in the database.");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->addobj_label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (hg->addobj_label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->addobj_label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_label,FALSE, FALSE, 0);


#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);
 
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtkut_label_new ("             RA<sub>2000</sub>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_ra = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_ra,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra), "000000.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_ra),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_ra),12);
  my_signal_connect (hg->addobj_entry_ra, "changed", 
		     cc_get_entry_double, &hg->addobj_ra);
  
  label = gtkut_label_new ("    Dec<sub>2000</sub>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_dec = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_dec,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec), "000000.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_dec),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_dec),12);
  my_signal_connect (hg->addobj_entry_dec, "changed", 
		     cc_get_entry_double, &hg->addobj_dec);
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new ("              Proper Motion (mas/yr) : RA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_pm_ra = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_pm_ra,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_ra), "0.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_pm_ra),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_pm_ra),10);
  my_signal_connect (hg->addobj_entry_pm_ra, "changed", 
		     cc_get_entry_double, &hg->addobj_pm_ra);
  
  label = gtk_label_new ("    Dec");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_pm_dec = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_pm_dec,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_dec), "0.00");
  gtk_editable_set_editable(GTK_EDITABLE(hg->addobj_entry_pm_dec),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_pm_dec),10);
  my_signal_connect (hg->addobj_entry_pm_dec, "changed", 
		     cc_get_entry_double, &hg->addobj_pm_dec);
  

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed", gtk_main_quit, NULL);

#ifdef USE_GTK3      
  button=gtkut_button_new_from_icon_name("Add Object","list-add");
#else
  button=gtkut_button_new_from_stock("Add Object",GTK_STOCK_ADD);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);
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
    gtkut_frame_set_label(GTK_FRAME(hg->mode_frame), "<b>Current</b>");
    tmp=g_strdup_printf("%02d/%02d/%04d %02d:%02d %s",
			hg->fr_month,hg->fr_day,hg->fr_year,
			24,0,"HST");
  }
  else{
    gtkut_frame_set_label(GTK_FRAME(hg->mode_frame), "<b>Set</b>");
    tmp=g_strdup_printf("%02d/%02d/%04d %02d:%02d %s",
			hg->skymon_month,hg->skymon_day,hg->skymon_year,
			hg->skymon_hour,hg->skymon_min,hg->obs_tzname);
  }
  gtk_label_set_text(GTK_LABEL(hg->mode_label),tmp);
  if(tmp) g_free(tmp);
}

void swap_obj(OBJpara *o1, OBJpara *o2){
  OBJpara temp;
  
  temp=*o2;
  *o2=*o1;
  *o1=temp;
}

