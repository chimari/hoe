//    HOE : Subaru HDS++ OPE file Editor
//        callbacks.c     Callbacks for GUI widgets
//                                           2019.01.03  A.Tajitsu

#include "main.h"


void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void cc_get_toggle_sm (GtkWidget * widget, gboolean * gdata)
{
  confPA *cdata;

  cdata=(confPA *)gdata;

  cdata->hg->obj[cdata->i_obj].check_sm
    =gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
  {
    if(flagSkymon){
      draw_skymon_cairo(cdata->hg->skymon_dw,cdata->hg);
      gdk_window_raise(gtk_widget_get_window(cdata->hg->skymon_main));
    }
  }
}

void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=(gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_adj_slit (GtkWidget *widget, gint * gdata)
{
  *gdata=(gint)(gtk_adjustment_get_value(GTK_ADJUSTMENT(widget))*500+0.5);
}

void cc_get_adj_double (GtkWidget *widget, gdouble * gdata)
{
  *gdata=gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void cc_get_entry_int (GtkWidget *widget, gint *gdata)
{
  *gdata=(gint)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);
}

void cc_get_entry_double (GtkWidget *widget, gdouble *gdata)
{
  *gdata=(gdouble)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);
}

void cc_get_combo_box (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    *gdata=n;
  }
}


void cc_radio(GtkWidget *button, gint *gdata)
{ 
  GSList *group=NULL;

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(group); i++){
      w = g_slist_nth_data(group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	*gdata  = g_slist_length(group) -1 - i;
	break;
      }
    }
  }
}

