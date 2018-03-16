//    HDS OPE file Editor
//    Input File should be ...
//            Object,  RA, DEC, EQUINOX
//         RA = hhmmss.ss
//         DEC = +/-ddmmss.s   real arguments
//   
//                                           2003.10.23  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"
#include"configfile.h"

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef USE_WIN32
#include <winsock2.h>
#endif

#include<locale.h>

#ifndef USE_WIN32
void ChildTerm();
#endif // USE_WIN32

void gui_init();
void set_fr_e_date();
void select_fr_calendar();
void popup_fr_calendar();
void make_note();
GtkWidget *make_menu();

static void cc_get_toggle_sm();
static void cc_usesetup();
void cc_get_combo_box_trdb();
void cc_get_fil_combo();

void delete_quit();
void do_quit();
void do_open();
void do_open2();
void do_open_ope();
void do_open_NST();
void do_open_JPL();
void do_conv_JPL();
void do_upload();
void do_download_log();
void do_merge();
void do_save();
void do_save_fc_pdf_all();
void do_save_hoe();
void do_save_FCDB_List();
void do_save_service_txt();
void do_save_proms_txt();
gchar* repl_nonalnum(gchar * obj_name, const gchar c_repl);
gchar* trdb_file_name();
void do_save_TRDB_CSV();
void do_read_hoe();
gboolean ow_dialog();
void create_quit_dialog();
//void uri_clicked();
void show_version();
void do_edit();
void do_plan();
void do_name_edit();
void do_efs_cairo();
void do_efs_for_etc();
//void do_etc();
void do_etc_list();
void do_update_exp_list();
void do_export_def_list();

void param_init();
void make_obj_list();
gchar* cut_spc();
void ChangeFontButton();
void ChangeFontButton_all();
//void ObjMagDB_Init();
void ReadList();
void ReadList2();
void UploadOPE();
void DownloadLOG();
void ReadListOPE();
void MergeList();
gboolean MergeNST();
gboolean MergeJPL();
void ConvJPL();
gboolean CheckDefDup();
void WriteOPE();
void WriteYAML();
void WritePlan();
void WriteService();
void WritePROMS();
void WriteOPE_BIAS();
void WriteOPE_COMP();
void WriteOPE_FLAT();
void WriteOPE_BIAS_plan();
void WriteYAML_BIAS_plan();
void WriteOPE_COMP_plan();
void WriteYAML_COMP_plan();
void WriteOPE_COMMENT_plan();
void WriteOPE_FLAT_plan();
void WriteYAML_FLAT_plan();
void WriteOPE_OBJ_plan();
void WriteYAML_OBJ_plan();
void WriteOPE_SetUp_plan();
void WriteYAML_SetUp_plan();

void usage();
void get_option();

void WriteHOE();
void ReadHOE();

void WriteConf();
void ReadConf();

gboolean is_number();

void clip_copy();
gchar* to_utf8();
gchar* to_locale();
gboolean close_popup();
static void destroy_popup();

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...);

gchar* make_head();

#ifdef USE_WIN32
gchar* WindowsVersion();
#endif

//void calc_rst();
void UpdateTotalExp();
void CheckVer();
void SyncCamZ();
void RecalcRST();
void CalcCrossScan();

gchar* fgets_new(FILE *fp){
  gint c;
  gint i=0, j=0;
  gchar *dbuf=NULL;

  do{
    i=0;
    while(!feof(fp)){
      c=fgetc(fp);
      if((c==0x00)||(c==0x0a)||(c==0x0d)) break;
      i++;
    }
  }while((i==0)&&(!feof(fp)));
  if(feof(fp)){
    if(fseek(fp,(long)(-i+1),SEEK_CUR)!=0) return(NULL);
  }
  else{
    if(fseek(fp,(long)(-i-1),SEEK_CUR)!=0) return(NULL);
  }

  if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(i+2)))==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }
  if(fread(dbuf,1, i, fp)){
    while( (c=fgetc(fp)) !=EOF){
      if((c==0x00)||(c==0x0a)||(c==0x0d))j++;
      else break;
    }
    if(c!=EOF){
      if(fseek(fp,-1L,SEEK_CUR)!=0) return(NULL);
    }
    dbuf[i]=0x00;
    //printf("%s\n",dbuf);
    return(dbuf);
  }
  else{
    return(NULL);
  }
  
}

gboolean
is_separator (GtkTreeModel *model,
	      GtkTreeIter  *iter,
	      gpointer      data)
{
  gboolean result;

  gtk_tree_model_get (model, iter, 2, &result, -1);

  return !result;  
}


#ifdef USE_WIN32
gchar* my_dirname(const gchar *file_name){
  return(g_path_get_dirname(file_name));
}


gchar* get_win_home(void){
  gchar WinPath[257]; 

  GetModuleFileName( NULL, WinPath, 256 );

  return(my_dirname(WinPath));
}

gchar* get_win_temp(void){
  gchar WinPath[257]; 
  
  GetTempPath(256, WinPath);
  if(access(WinPath,F_OK)!=0){
    GetModuleFileName( NULL, WinPath, 256 );
  }

  return(my_dirname(WinPath));
}
#endif

gchar* get_home_dir(void){
#ifdef USE_WIN32
  gchar WinPath[257]; 

  GetModuleFileName( NULL, WinPath, 256 );

  return(my_dirname(WinPath));
#else
  return(g_strdup(g_get_home_dir()));
#endif
}

#ifndef USE_WIN32
// 子プロセスの処理 
void ChildTerm(int dummy)
{
  int s;

  wait(&s);
  signal(SIGCHLD,ChildTerm);
}
#endif // USE_WIN32


void gui_init(typHOE *hg){
  GtkWidget *menubar;
#ifdef USE_GTKMACINTEGRATION
  GtkosxApplication *osxapp;
#endif

  // Main Window 
  hg->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  my_signal_connect(hg->w_top, "delete-event",
		    delete_quit,(gpointer)hg);
  gtk_container_set_border_width(GTK_CONTAINER(hg->w_top),0);
  gtk_window_set_title(GTK_WINDOW(hg->w_top),"HOE : HDS OPE file Editor");

  hg->w_box = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->w_top), hg->w_box);




#ifdef USE_GTKMACINTEGRATION
  osxapp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
#endif
  menubar=make_menu(hg);
  gtk_box_pack_start(GTK_BOX(hg->w_box), menubar,FALSE, FALSE, 0);

#ifdef USE_GTKMACINTEGRATION
  gtk_widget_hide(menubar);
  gtkosx_application_set_menu_bar(osxapp, GTK_MENU_SHELL(menubar));
  //about_menu = gtk_item_factory_get_item(ifactory, "/Help/About");
  //prefs_menu = gtk_item_factory_get_item(ifactory, "/Configuration/Preferences...");
  //gtkosx_application_insert_app_menu_item(osxapp, about_menu, 0);
  //gtkosx_application_insert_app_menu_item(osxapp, prefs_menu, 1);
  //g_signal_connect(G_OBJECT(osxapp), "NSApplicationBlockTermination",
  //		   G_CALLBACK(osx_block_termination), mainwin);
  gtkosx_application_ready(osxapp);
#endif


  make_note(hg);
  

  gtk_widget_show_all(hg->w_top);

}


void set_fr_e_date(typHOE *hg){
  gchar *tmp;
    
  tmp=g_strdup_printf("%s %d, %d",
		      cal_month[hg->fr_month-1],
		      hg->fr_day,
		      hg->fr_year);

  gtk_entry_set_text(GTK_ENTRY(hg->fr_e),tmp);
  g_free(tmp);
}

void select_fr_calendar (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  gtk_calendar_get_date(GTK_CALENDAR(widget),
			&hg->fr_year,
			&hg->fr_month,
			&hg->fr_day);
  hg->fr_month++;

  set_fr_e_date(hg);

  gtk_main_quit();
}

void popup_fr_calendar (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *calendar;
  GtkAllocation *allocation=g_new(GtkAllocation, 1);
  gint root_x, root_y;
  typHOE *hg=(typHOE *)gdata;
  gtk_widget_get_allocation(widget,allocation);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->w_top));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_get_position(GTK_WINDOW(hg->w_top),&root_x,&root_y);

  my_signal_connect(dialog,"delete-event",gtk_main_quit,NULL);
  gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);

  calendar=gtk_calendar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     calendar,FALSE, FALSE, 0);

  gtk_calendar_select_month(GTK_CALENDAR(calendar),
			    hg->fr_month-1,
			    hg->fr_year);

  gtk_calendar_select_day(GTK_CALENDAR(calendar),
			  hg->fr_day);

  my_signal_connect(calendar,
		    "day-selected-double-click",
		    select_fr_calendar, 
		    (gpointer)hg);

  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_window_move(GTK_WINDOW(dialog),
		  root_x+allocation->x,
		  root_y+allocation->y);
  g_free(allocation);
  gtk_widget_show_all(dialog);
  gtk_main();
  gtk_widget_destroy(dialog);
}


void make_note(typHOE *hg)
{
  {
    GtkWidget *table;
    GtkWidget *label;

    hg->all_note = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->all_note), GTK_POS_TOP);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->all_note), TRUE);
    gtk_box_pack_start(GTK_BOX(hg->w_box), hg->all_note,TRUE, TRUE, 5);

  
    {
      GtkWidget *scrwin;
      GtkWidget *frame;
      GtkWidget *table1;
      GtkWidget *hbox;
      GtkWidget *hbox1;
      GtkWidget *vbox;
      GtkWidget *entry;
      GtkWidget *combo, *combo0;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *check;
      GtkWidget *button;
      gchar tmp[64];
      GtkTooltip *tooltip;
      confSetup *cdata[MAX_USESETUP];

      scrwin = gtk_scrolled_window_new (NULL, NULL);
      table = gtk_table_new (3, 6, FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (scrwin), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);
      gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scrwin),
					GTK_CORNER_BOTTOM_LEFT);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),table);
      gtk_widget_set_size_request(scrwin, -1, 510);  
      
      
      // Header
      frame = gtk_frame_new ("Header");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(1,3,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      hbox = gtkut_hbox_new(FALSE,5);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);

      label = gtk_label_new ("Date");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      hbox1 = gtkut_hbox_new(FALSE,0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_box_pack_start(GTK_BOX(hbox),hbox1,FALSE,FALSE,0);

      hg->fr_e = gtk_entry_new();
      gtk_box_pack_start(GTK_BOX(hbox1),hg->fr_e,FALSE,FALSE,0);
      gtk_editable_set_editable(GTK_EDITABLE(hg->fr_e),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->fr_e),12);
      
      set_fr_e_date(hg);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
			popup_fr_calendar, 
			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

      label = gtk_label_new ("  x");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->nights,
						1, 10,
						1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->nights);

      label = gtk_label_new ("nights");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);


      hbox = gtkut_hbox_new(FALSE,5);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("ID");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      gtk_entry_set_text(GTK_ENTRY(entry),hg->prop_id);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),8);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->prop_id);

      label = gtk_label_new ("  Pass");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      if(hg->prop_pass)
	gtk_entry_set_text(GTK_ENTRY(entry),hg->prop_pass);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),8);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->prop_pass);
      
      label = gtk_label_new ("   OCS");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SOSs",
			   1, OCS_SOSS, -1);
	if(hg->ocs==OCS_SOSS) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Gen2",
			   1, OCS_GEN2, -1);
	if(hg->ocs==OCS_GEN2) iter_set=iter;
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->ocs);
      }

      hbox = gtkut_hbox_new(FALSE,5);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 2, 3,
		       GTK_FILL,GTK_FILL,0,0);

      label = gtk_label_new ("Observer");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      if(hg->observer)
	gtk_entry_set_text(GTK_ENTRY(entry),hg->observer);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),40);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->observer);



      // Statistics.
      frame = gtk_frame_new ("Base OPE");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 1, 2,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hbox = gtkut_hbox_new (FALSE, 5);
      gtk_container_add (GTK_CONTAINER (frame), hbox);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

      hg->label_stat_base = gtk_label_new ("Total Exp. = 0.00 hrs,  Estimated Obs. Time = 0.00 hrs");
#ifdef USE_GTK3
      gtk_widget_set_halign (hg->label_stat_base, 0.0);
      gtk_widget_set_valign (hg->label_stat_base, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (hg->label_stat_base), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox), hg->label_stat_base,FALSE, FALSE, 5);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
      gtk_box_pack_start(GTK_BOX(hbox), button,FALSE, FALSE, 5);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (UpdateTotalExp), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Recalc.");
#endif

      frame = gtk_frame_new ("Plan OPE");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 2, 3,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hbox = gtkut_hbox_new (FALSE, 5);
      gtk_container_add (GTK_CONTAINER (frame), hbox);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

      hg->label_stat_plan = gtk_label_new ("00:00 -- 00:00 (0.00 hrs)");
#ifdef USE_GTK3
      gtk_widget_set_halign (hg->label_stat_plan, 0.0);
      gtk_widget_set_valign (hg->label_stat_plan, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (hg->label_stat_plan), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox), hg->label_stat_plan,FALSE, FALSE, 5);


      // Environment for AD Calc.
      frame = gtk_frame_new ("Environment for AD Calc.");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 3, 4,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(4,4,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      // OBS Wavelength
      label = gtk_label_new ("Obs WL [A]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave1,
						2800, 30000, 
						100.0,100.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->wave1);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Wavelength0
      label = gtk_label_new ("Guide WL [A]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave0,
						2800, 30000, 
						100.0,100.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->wave0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Temperature
      label = gtk_label_new ("  Temperature [C]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->temp,
						-15, 15, 
						1.0,1.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->temp);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Pressure
      label = gtk_label_new ("  Pressure [hPa]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      adj = (GtkAdjustment *)gtk_adjustment_new(hg->pres,
						600, 650, 
						1.0,1.0,0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->pres);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


#ifndef USE_WIN32
#ifndef USE_OSX
      // Environment for AD Calc.
      frame = gtk_frame_new ("Web Browser");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 4, 5,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(2,1,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

      label = gtk_label_new ("Command");
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      
      entry = gtk_entry_new ();
      gtk_table_attach(GTK_TABLE(table1), entry, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
      gtk_entry_set_text(GTK_ENTRY(entry),
			 hg->www_com);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->www_com);
#endif
#endif

      frame = gtk_frame_new ("Font");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 
#ifdef USE_OSX
		       4, 5,
#elif defined(USE_WIN32)
		       4, 5,
#else
		       5, 6,
#endif
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      
      hbox = gtkut_hbox_new(FALSE,5);
      gtk_container_add (GTK_CONTAINER (frame), hbox);
      
      label = gtk_label_new ("Base");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 5);
      
      {
	button = gtk_font_button_new_with_font(hg->fontname_all);
	gtk_box_pack_start(GTK_BOX(hbox), button,TRUE, TRUE, 2);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
	my_signal_connect(button,"font-set",ChangeFontButton_all, 
			  (gpointer)hg);
      }
      
      label = gtk_label_new ("     Object");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 5);
      
      {
	button = gtk_font_button_new_with_font(hg->fontname);
	gtk_box_pack_start(GTK_BOX(hbox), button,TRUE, TRUE, 2);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
	my_signal_connect(button,"font-set",ChangeFontButton, 
			  (gpointer)hg);
      }
      
      label = gtk_label_new ("General");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
    }

    {
      GtkWidget *scrwin;
      GtkWidget *frame;
      GtkWidget *table1;
      GtkWidget *hbox;
      GtkWidget *vbox;
      GtkWidget *entry;
      GtkWidget *combo, *combo0;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *check;
      GtkWidget *button;
      gchar tmp[64];
      GtkTooltip *tooltip;
      confSetup *cdata[MAX_USESETUP];

      scrwin = gtk_scrolled_window_new (NULL, NULL);
      table = gtk_table_new (3, 6, FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (scrwin), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);
      gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scrwin),
					GTK_CORNER_BOTTOM_LEFT);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),table);
      gtk_widget_set_size_request(scrwin, -1, 480);  
      

      // AG
      frame = gtk_frame_new ("AG");
      gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 1, 2,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(2,6,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      label = gtk_label_new ("Exptime Factor");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_factor,
						1, 5, 1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->exptime_factor);


      label = gtk_label_new ("Brightness");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->brightness,
						0, 10000, 1000.0, 1000.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->brightness);


      // SV
      frame = gtk_frame_new ("SV");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 2,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(2,6,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      label = gtk_label_new ("Read Area");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Part",
			   1, SV_PART, -1);
	if(hg->sv_area==SV_PART) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Full",
			   1, SV_FULL, -1);
	if(hg->sv_area==SV_FULL) iter_set=iter;
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_table_attach(GTK_TABLE(table1), combo, 1, 2, 0, 1,
			 GTK_FILL,GTK_SHRINK,0,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->sv_area);
      }

      label = gtk_label_new ("Exptime[msec]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_sv,
						100, 100000, 
						100.0, 100.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->exptime_sv);


      label = gtk_label_new ("Read Region");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_region,
						100, 400, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_region);


      label = gtk_label_new ("Calc Region");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 3, 4,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_calc,
						10, 150, 
						5.0, 5.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 3, 4,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_calc);


      label = gtk_label_new ("Integrate");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 4, 5,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_integrate,
						1, 5, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 4, 5,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_integrate);

      label = gtk_label_new ("Acquisition Time[s]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 5, 6,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_acq,
						30, 300, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 5, 6,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_acq);


      // Slit Center on SV
      frame = gtk_frame_new ("Slit Center on SV");
      gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(1,1,FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      
      label = gtk_label_new ("Slit_X ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    Slit_Y ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_slitx,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_slitx);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_slity,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_slity);

      label = gtk_label_new ("IS_X ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    IS_Y ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_isx,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_isx);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_isy,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_isy);


      label = gtk_label_new ("IS3_X ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    IS3_Y ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_is3x,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_is3x);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_is3y,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_is3y);

      label = gtk_label_new ("SV/AG");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
    }

    // 全体の設定
    {
      GtkWidget *scrwin;
      GtkWidget *frame;
      GtkWidget *table1;
      GtkWidget *hbox;
      GtkWidget *vbox;
      GtkWidget *entry;
      GtkWidget *combo, *combo0, *fil1_combo, *fil2_combo;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *check;
      GtkWidget *button;
      gchar tmp[64];
      GtkTooltip *tooltip;
      confSetup *cdata[MAX_USESETUP];

      hg->setup_scrwin = gtk_scrolled_window_new (NULL, NULL);
      table = gtk_table_new (3, 6, FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (hg->setup_scrwin), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->setup_scrwin),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);
      gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(hg->setup_scrwin),
					GTK_CORNER_BOTTOM_LEFT);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(hg->setup_scrwin),table);
      gtk_widget_set_size_request(hg->setup_scrwin, -1, 480);  
      
      

      // CamZ
      frame = gtk_frame_new ("Format Adjustments");
      gtk_table_attach(GTK_TABLE(table), frame, 2, 3, 0, 3,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(4,2,FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      
      label = gtk_label_new ("Camera Z [um]  Blue");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("Red");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("dCross");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      hg->camz_b_adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_b,
						-500, -200, 
						1.0, 10.0, 0);
      my_signal_connect (hg->camz_b_adj, "value_changed",
			 cc_get_adj,
			 &hg->camz_b);
      spinner =  gtk_spin_button_new (hg->camz_b_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

      hg->camz_r_adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_r,
						-500, -200, 
						1.0, 10.0, 0);
      my_signal_connect (hg->camz_r_adj, "value_changed",
			 cc_get_adj,
			 &hg->camz_r);
      spinner =  gtk_spin_button_new (hg->camz_r_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

      hg->d_cross_adj = (GtkAdjustment *)gtk_adjustment_new(hg->d_cross,
						-500, 500, 
						1.0, 10.0, 0);
      my_signal_connect (hg->d_cross_adj, "value_changed",
			 cc_get_adj,
			 &hg->d_cross);
      spinner =  gtk_spin_button_new (hg->d_cross_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
      button=gtkut_button_new_from_stock("Sync",GTK_STOCK_REFRESH);
#endif
      gtk_table_attach(GTK_TABLE(table1), button, 0, 2, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (SyncCamZ), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Sync CamZ values to the current ones");
#endif

      hg->camz_label = gtk_label_new (hg->camz_date);
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 1.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), hg->camz_label, 2, 4, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);


      // Cross Scan Calculator
      frame = gtk_frame_new ("Cross Scan Calculator");
      gtk_table_attach(GTK_TABLE(table), frame, 2, 3, 3, 5,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(3,2,FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
      gtk_container_add (GTK_CONTAINER (frame), table1);

      label = gtk_label_new ("Center Wavelength [A]");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.0);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->wcent,
						3200, 9500, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->wcent);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"accessories-calculator");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_OK);
#endif
      gtk_table_attach(GTK_TABLE(table1), button, 2, 3, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_signal_connect(button,"pressed",
      			CalcCrossScan, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Calculate Cross Scan");
#endif

      hg->label_wcent = gtk_label_new (" Calculated Cross Scan");
#ifdef USE_GTK3
      gtk_widget_set_halign (hg->label_wcent, 0.0);
      gtk_widget_set_valign (hg->label_wcent, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (hg->label_wcent), 0.0, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), hg->label_wcent, 0, 3, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      // Wavelength Setup
      frame = gtk_frame_new ("Wavelength Setup  : Binning (sp)x(wv) [readout] / Slit / Filter / ImR");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 3, 5, 6,
		       GTK_FILL,GTK_SHRINK,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(1,1+MAX_USESETUP,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      label = gtk_label_new ("Setup");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 0, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("CCD Binning");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("Slit Width/Length");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 4, 7, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 7, 8, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("Filter1/Filter2");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 8, 11, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 11, 12, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("ImR");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 12, 13, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      label = gtk_label_new("IS");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 13, 14, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      label = gtk_label_new("I2");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 14, 15, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      {
	gint i_use, i_set, i_nonstd;

	for(i_use=0;i_use<MAX_USESETUP;i_use++){
	  sprintf(tmp,"%d.",i_use+1);
	  check = gtk_check_button_new_with_label(tmp);
	  gtk_table_attach(GTK_TABLE(table1), check, 0, 1, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->setup[i_use].use);
	  my_signal_connect (check, "toggled",
			     cc_get_toggle,
			     &hg->setup[i_use].use);

	  gtk_widget_show(check);
	  

	  {
	    GtkListStore *store;
	    GtkTreeIter iter, iter_set;	  
	    GtkCellRenderer *renderer;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    for(i_set=0;i_set<MAX_SETUP;i_set++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 0, setups[i_set].initial,
				 1, i_set, -1);
	      if(hg->setup[i_use].setup==i_set) iter_set=iter;
	    }
	    for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
	      sprintf(tmp,"NonStd-%d",i_nonstd+1);
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 0, tmp,
				 1, -(i_nonstd+1), -1);
	      if(hg->setup[i_use].setup==-(i_nonstd+1)) iter_set=iter;
	    }
	    
	    combo0 = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	    gtk_table_attach(GTK_TABLE(table1), combo0, 1, 2, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    renderer = gtk_cell_renderer_text_new();
	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo0),renderer, TRUE);
	    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo0), renderer, "text",0,NULL);
	    
	    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo0),&iter_set);
	    gtk_widget_show(combo0);
	  }

	  {
	    GtkListStore *store;
	    GtkTreeIter iter, iter_set;	  
	    GtkCellRenderer *renderer;
	    int i_bin;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    for(i_bin=0;i_bin<MAX_BINNING;i_bin++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 
				 0, binname[i_bin],
				 1, i_bin, -1);
	      if(hg->setup[i_use].binning==i_bin) iter_set=iter;
	    }
	    
	    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	    gtk_table_attach(GTK_TABLE(table1), combo, 2, 3, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    renderer = gtk_cell_renderer_text_new();
	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	    
	    
	    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	    gtk_widget_show(combo);
	    my_signal_connect (combo,"changed",cc_get_combo_box,
			       &hg->setup[i_use].binning);
	  }

	  adj = (GtkAdjustment *)gtk_adjustment_new(hg->setup[i_use].slit_width,
						    100, 2000, 
						    5.0,5.0,0);
	  my_signal_connect (adj, "value_changed",
			     cc_get_adj,
			     &hg->setup[i_use].slit_width);
	  spinner =  gtk_spin_button_new (adj, 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 4, 5, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

	  label = gtk_label_new ("/");
#ifdef USE_GTK3
	  gtk_widget_set_halign (label, 0.5);
	  gtk_widget_set_valign (label, 0.5);
#else
	  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
	  gtk_table_attach(GTK_TABLE(table1), label, 5, 6, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);

	  adj = (GtkAdjustment *)gtk_adjustment_new(hg->setup[i_use].slit_length,
						    1000, 30000, 
						    100.0,100.0,0);
	  my_signal_connect (adj, "value_changed",
			     cc_get_adj,
			     &hg->setup[i_use].slit_length);
	  spinner =  gtk_spin_button_new (adj, 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 6, 7, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


	  {
	    GtkListStore *store;
	    GtkTreeIter iter;	  
	    GtkCellRenderer *renderer;
	    int i_fil;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    for(i_fil=0;i_fil<MAX_FILTER1;i_fil++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 
				 0, filtername1[i_fil],
				 1, i_fil,
				 -1);
	    }


	    fil1_combo = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (fil1_combo), 0);
	    gtk_table_attach(GTK_TABLE(table1), fil1_combo, 8, 9, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    if(!hg->setup[i_use].fil1)
	      hg->setup[i_use].fil1=g_strdup(setups[StdUb].fil1);
	    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil1_combo))),
			       hg->setup[i_use].fil1);
	    gtk_editable_set_editable(GTK_EDITABLE(gtk_bin_get_child(GTK_BIN(fil1_combo))),TRUE);
	    
	    my_entry_set_width_chars(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil1_combo))),6);
	    gtk_widget_show(fil1_combo);
	    my_signal_connect (gtk_bin_get_child(GTK_BIN(fil1_combo)),"changed",
			       cc_get_entry,
			       &hg->setup[i_use].fil1);
	    my_signal_connect (fil1_combo,"changed",cc_get_fil_combo);
	  }

	  label = gtk_label_new ("/");
#ifdef USE_GTK3
	  gtk_widget_set_halign (label, 0.5);
	  gtk_widget_set_valign (label, 0.5);
#else
	  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
	  gtk_table_attach(GTK_TABLE(table1), label, 9, 10, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);


	  {
	    GtkListStore *store;
	    GtkTreeIter iter;	  
	    GtkCellRenderer *renderer;
	    int i_fil;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    	    
	    for(i_fil=0;i_fil<MAX_FILTER2;i_fil++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 
				 0, filtername2[i_fil],
				 1, i_fil,
				 -1);
	    }

	    fil2_combo = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (fil2_combo), 0);
	    gtk_table_attach(GTK_TABLE(table1), fil2_combo, 10, 11, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	   
	    if(!hg->setup[i_use].fil2)
	      hg->setup[i_use].fil2=g_strdup(setups[StdUb].fil2);
	    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil2_combo))),
			       hg->setup[i_use].fil2);
	    gtk_editable_set_editable(GTK_EDITABLE(gtk_bin_get_child(GTK_BIN(fil2_combo))),TRUE);
	    
	    my_entry_set_width_chars(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fil2_combo))),6);
	    gtk_widget_show(fil2_combo);
	    my_signal_connect (gtk_bin_get_child(GTK_BIN(fil2_combo)),
			       "changed",
			       cc_get_entry,
			       &hg->setup[i_use].fil2);
	    my_signal_connect (G_OBJECT(fil2_combo),"changed",
	    		       cc_get_fil_combo);
	  }


	  cdata[i_use]=g_malloc0(sizeof(confSetup));
	  cdata[i_use]->hg=hg;
	  cdata[i_use]->i_use=i_use;
	  cdata[i_use]->length_entry=spinner;
	  cdata[i_use]->fil1_combo=fil1_combo;
	  cdata[i_use]->fil2_combo=fil2_combo;
	  my_signal_connect (combo0,
			     "changed",
			     cc_usesetup,
			     (gpointer)cdata[i_use]);

	  

	  {
	    GtkListStore *store;
	    GtkTreeIter iter, iter_set;	  
	    GtkCellRenderer *renderer;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "No",
			       1, IMR_NO, -1);

	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "Link",
			       1, IMR_LINK, -1);

	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "Zenith",
			       1, IMR_ZENITH, -1);
	    
	    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	    gtk_table_attach(GTK_TABLE(table1), combo, 12, 13, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    renderer = gtk_cell_renderer_text_new();
	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	    
	    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->setup[i_use].imr);
	    gtk_widget_show(combo);
	    my_signal_connect (combo,"changed",cc_get_combo_box,
			       &hg->setup[i_use].imr);
	  }

	  {
	    GtkListStore *store;
	    GtkTreeIter iter, iter_set;	  
	    GtkCellRenderer *renderer;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "No",
			       1, IS_NO, -1);

	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "0\".30x5",
			       1, IS_030X5, -1);

	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "0\".45x3",
			       1, IS_045X3, -1);
	    
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "0\".20x3",
			       1, IS_020X3, -1);
	    
	    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	    gtk_table_attach(GTK_TABLE(table1), combo, 13, 14, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    renderer = gtk_cell_renderer_text_new();
	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	    
	    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->setup[i_use].is);
	    gtk_widget_show(combo);
	    my_signal_connect (combo,"changed",cc_get_combo_box,
	    	       &hg->setup[i_use].is);
	  }

	  {
	    check = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE(table1), check, 14, 15, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->setup[i_use].i2);
	    my_signal_connect (check, "toggled",
			       cc_get_toggle,
			     &hg->setup[i_use].i2);
	    
	    gtk_widget_show(check);
	  }
	}
      }
      
      
      // Non-Standard Setup
      frame = gtk_frame_new ("Non-Standard Setup");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 0, 5,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(1,MAX_NONSTD,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 10);


      label = gtk_label_new("Color");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("Cross Scan");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("Echelle");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("CamR");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_table_attach(GTK_TABLE(table1), label, 4, 5, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);


      {
	int i;
	
	for(i=0;i<MAX_NONSTD;i++){
	  sprintf(tmp,"NonStd-%d",i+1);
	  label = gtk_label_new (tmp);
	  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);

	  {
	    GtkListStore *store;
	    GtkTreeIter iter, iter_set;	  
	    GtkCellRenderer *renderer;
	    
	    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	    
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "Red",
			       1, COL_RED, -1);
	    if(hg->nonstd[i].col==COL_RED) iter_set=iter;

	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, &iter, 0, "Blue",
			       1, COL_BLUE, -1);
	    if(hg->nonstd[i].col==COL_BLUE) iter_set=iter;
	    
	    
	    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	    gtk_table_attach(GTK_TABLE(table1), combo, 1, 2, i+1, i+2,
			     GTK_FILL,GTK_SHRINK,0,0);
	    g_object_unref(store);
	    
	    renderer = gtk_cell_renderer_text_new();
	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	    
	    
	    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	    gtk_widget_show(combo);
	    my_signal_connect (combo,"changed",cc_get_combo_box,
			       &hg->nonstd[i].col);
	  }
	  
	  adj = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].cross,
						    13000, 28000, 
						    1.0,1.0,0);
	  my_signal_connect (adj, "value_changed",
			     cc_get_adj,
			     &hg->nonstd[i].cross);
	  spinner =  gtk_spin_button_new (adj, 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 2, 3, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	  //gtk_signal_connect (GTK_OBJECT (&GTK_SPIN_BUTTON(spinner)->entry),
	  //		      "value_changed",
	  //		      GTK_SIGNAL_FUNC (cc_get_entry_int),
	  //		      &hg->nonstd[i].cross);
	  
	  
	  hg->echelle_adj[i] = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].echelle,
						    -3600, 3600, 
						    60.0,60.0,0);
	  my_signal_connect (hg->echelle_adj[i], "value_changed",
			     cc_get_adj,
			     &hg->nonstd[i].echelle);
	  spinner =  gtk_spin_button_new (hg->echelle_adj[i], 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	  //gtk_signal_connect (GTK_OBJECT (&GTK_SPIN_BUTTON(spinner)->entry),
	  //		      "value_changed",
	  //		      GTK_SIGNAL_FUNC (cc_get_entry_int),
	  //		      &hg->nonstd[i].echelle);
	  
	  
	  adj = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].camr,
						    -7200, 7200, 
						    60.0,60.0,0);
	  my_signal_connect (adj, "value_changed",
			     cc_get_adj,
			     &hg->nonstd[i].camr);
	  spinner =  gtk_spin_button_new (adj, 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 4, 5, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	}
      }

      label = gtk_label_new ("HDS");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), hg->setup_scrwin, label);
    }


    // 天体リスト
    {
      GtkWidget *frame;
      GtkWidget *hbox;
      GtkWidget *hbox1;
      GtkWidget *entry;
      GtkWidget *button;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *combo;
      GtkWidget *label;
      GtkWidget *check;
      GdkPixbuf *pixbuf;
      gchar tmp[12];

      table = gtk_table_new (2, 2, FALSE);

      
      hg->sw_objtree = gtk_scrolled_window_new (NULL, NULL);
      gtk_table_attach_defaults (GTK_TABLE(table), hg->sw_objtree, 0, 2, 0, 1);
      gtk_container_set_border_width (GTK_CONTAINER (hg->sw_objtree), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->sw_objtree),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);

      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      frame = gtk_frame_new ("Find Object");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (search_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Find");
#endif
      
      hg->tree_search_i=0;
      hg->tree_search_imax=0;

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox1), entry,FALSE, FALSE, 0);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),12);
      my_signal_connect (entry, "changed", cc_search_text, (gpointer)hg);
      my_signal_connect (entry, "activate", search_item, (gpointer)hg);
      
      hg->tree_search_label = gtk_label_new ("     ");
      gtk_box_pack_start(GTK_BOX(hbox1),hg->tree_search_label,FALSE,FALSE,0);
      
      
      frame = gtk_frame_new ("View");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Plot","document-print-preview");
#else
      button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
#endif
      g_signal_connect (button, "clicked",
                        G_CALLBACK (plot2_objtree_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("FC","starred");
#else
      button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (fc_item), (gpointer)hg);

      pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
      button=gtkut_button_new_from_pixbuf("ETC", pixbuf);
      g_object_unref(G_OBJECT(pixbuf));
      g_signal_connect (button, "clicked",
                        G_CALLBACK (etc_objtree_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);


      hg->f_objtree_arud = gtk_frame_new ("Edit the List");
      gtk_box_pack_start (GTK_BOX (hbox), hg->f_objtree_arud, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hg->f_objtree_arud), 2);
	
      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (hg->f_objtree_arud), hbox1);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"list-add");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			addobj_dialog, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Add");
#endif

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"list-remove");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			remove_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Remove");
#endif

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"go-up");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			up_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Up");
#endif

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			down_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Down");
#endif

      frame = gtk_frame_new ("Web Browsing");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	GdkPixbuf *icon;
	
	store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
				   G_TYPE_BOOLEAN);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SIMBAD",
			   1, WWWDB_SIMBAD, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_SIMBAD) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "NED",
			   1, WWWDB_NED, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_NED) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SDSS (DR14)",
			   1, WWWDB_DR14, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_DR14) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "MAST",
			   1, WWWDB_MAST, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_MAST) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "MAST Portal",
			   1, WWWDB_MASTP, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_MASTP) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "KECK archive",
			   1, WWWDB_KECK, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_KECK) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GEMINI archive",
			   1, WWWDB_GEMINI, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_GEMINI) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "IRSA",
			   1, WWWDB_IRSA, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_IRSA) iter_set=iter;
      
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Spitzer",
			   1, WWWDB_SPITZER, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_SPITZER) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "CASSIS",
			   1, WWWDB_CASSIS, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_CASSIS) iter_set=iter;
      
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, NULL,
			    1, WWWDB_SEP1,2, FALSE, 
			    -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Standard Locator",
			   1, WWWDB_SSLOC, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_SSLOC) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
			   1, WWWDB_RAPID, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_RAPID) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
			   1, WWWDB_MIRSTD, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_MIRSTD) iter_set=iter;
	
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, NULL,
			    1, WWWDB_SEP2,2, FALSE, 
			    -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SMOKA",
			   1, WWWDB_SMOKA, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_SMOKA) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "HST Archive",
			   1, WWWDB_HST, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_HST) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "ESO Archive",
			   1, WWWDB_ESO, 2, TRUE, -1);
	if(hg->wwwdb_mode==WWWDB_ESO) iter_set=iter;
      
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
	g_object_unref(store);
      
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					      is_separator, NULL, NULL);	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->wwwdb_mode);
	
#ifdef USE_OSX
	icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
	icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
	if(strcmp(hg->www_com,"firefox")==0){
	  icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
	}
	else{
	  icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
	}
#endif
	button=gtkut_button_new_from_pixbuf(NULL, icon);
	g_object_unref(icon);
	gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
	my_signal_connect (button, "clicked",
			   G_CALLBACK (wwwdb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
	gtk_widget_set_tooltip_text(button,"Open Web Browser");
#endif
      }     

      frame = gtk_frame_new ("Standard Stars");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	
	store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
				   G_TYPE_BOOLEAN);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Standard Locator",
			   1, STDDB_SSLOC, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_SSLOC) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
			   1, STDDB_RAPID, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_RAPID) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
			   1, STDDB_MIRSTD, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_MIRSTD) iter_set=iter;
	
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, NULL,
			    1, WWWDB_SEP1,2, FALSE, 
			    -1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "ESO Opt/UV Standard",
			   1, STDDB_ESOSTD, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_ESOSTD) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "IRAF 1D-std (spec16/50)",
			   1, STDDB_IRAFSTD, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_IRAFSTD) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "HST CALSPEC",
			   1, STDDB_CALSPEC, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_CALSPEC) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "HDS efficiency",
			   1, STDDB_HDSSTD, 2, TRUE, -1);
	if(hg->stddb_mode==STDDB_HDSSTD) iter_set=iter;
	
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					      is_separator, NULL, NULL);	
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->stddb_mode);
      }
    
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (stddb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Search");
#endif
      
      hg->mode_frame = gtk_frame_new ("Current");
      gtk_box_pack_start (GTK_BOX (hbox), hg->mode_frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hg->mode_frame), 2);

      hbox1 = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (hg->mode_frame), hbox1);

      hg->mode_label = gtk_label_new ("XX-XX-XX XX:XX HST");
#ifdef USE_GTK3
      gtk_widget_set_halign (hg->mode_label, 0.0);
      gtk_widget_set_valign (hg->mode_label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (hg->mode_label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),hg->mode_label,FALSE, FALSE, 0);

      label = gtk_label_new ("Main Target");
      gtk_widget_show(label);
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (RecalcRST), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Refresh Rise & Set Time (El=15)");
#endif

      make_obj_tree(hg);
    }

    // STDDB
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *button;
      GtkWidget *sw;
      GtkTreeModel *items_model;
      GdkPixbuf *icon;

      vbox = gtkut_vbox_new (FALSE, 5);
      label = gtk_label_new ("Standard");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtkut_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
    

#ifdef USE_GTK3
      hg->stddb_button=gtkut_toggle_button_new_from_icon_name(NULL,"video-display");
#else
      hg->stddb_button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_APPLY);
#endif
      gtk_container_set_border_width (GTK_CONTAINER (hg->stddb_button), 0);
      gtk_box_pack_start(GTK_BOX(hbox),hg->stddb_button,FALSE,FALSE,0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->stddb_button),
				   hg->stddb_flag);
      my_signal_connect(hg->stddb_button,"toggled",
			G_CALLBACK(stddb_toggle), 
			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(hg->stddb_button,
				  "Display Standards in SkyMon");
#endif

      hg->stddb_label= gtk_label_new (hg->stddb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->stddb_label, TRUE, TRUE, 0);
    
      stddb_set_label(hg);

#ifdef USE_GTK3    
      button=gtkut_button_new_from_icon_name(NULL,"emblem-system");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#endif
      my_signal_connect (button, "clicked",
      		 G_CALLBACK (create_std_para_dialog), (gpointer)hg);
      gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Config for Standard Search");
#endif
      
      sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
      
      /* create models */
      items_model = std_create_items_model (hg);
      
      /* create tree view */
      hg->stddb_tree = gtk_tree_view_new_with_model (items_model);
      gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->stddb_tree), TRUE);
      gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->stddb_tree)),
				   GTK_SELECTION_SINGLE);
      std_add_columns (hg, GTK_TREE_VIEW (hg->stddb_tree), items_model);
      
      g_object_unref (items_model);
      
      gtk_container_add (GTK_CONTAINER (sw), hg->stddb_tree);
      
      my_signal_connect (hg->stddb_tree, "cursor-changed",
			 G_CALLBACK (std_focus_item), (gpointer)hg);
      
      /* some buttons */
      hbox = gtkut_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
#ifdef USE_OSX
      icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
      icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
      if(strcmp(hg->www_com,"firefox")==0){
	icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
      }
      else{
	icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
      }
#endif
      button=gtkut_button_new_from_pixbuf("Browse", icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (std_simbad), (gpointer)hg);

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Main Target","list-add");
#else
      button=gtkut_button_new_from_stock("Main Target",GTK_STOCK_ADD);
#endif
      my_signal_connect (button, "clicked",
			 G_CALLBACK (add_item_std), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Add to the Main Target List");
#endif

      label= gtk_label_new ("    ");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("OPE Def.","accessories-text-editor");
#else
      button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 make_std_tgt, (gpointer)hg);
      
      hg->std_tgt = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->std_tgt,TRUE, TRUE, 0);
      gtk_editable_set_editable(GTK_EDITABLE(hg->std_tgt),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->std_tgt),50);

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name(NULL,"edit-copy");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (clip_copy), (gpointer)hg->std_tgt);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Copy to clipboard");
#endif
    }


    // FCDB
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *button;
      GdkPixbuf *icon;

      vbox = gtkut_vbox_new (FALSE, 5);
      label = gtk_label_new ("DB / Finding Chart");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtkut_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"document-save");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (do_save_FCDB_List), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif
      
      hg->fcdb_label= gtk_label_new (hg->fcdb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_label, TRUE, TRUE, 0);
      
#ifdef USE_GTK3    
      button=gtkut_button_new_from_icon_name(NULL,"emblem-system");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#endif
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (fcdb_para_item), (gpointer)hg);
      gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Search Param.");
#endif
      
      hg->fcdb_sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->fcdb_sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->fcdb_sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), hg->fcdb_sw, TRUE, TRUE, 0);

      fcdb_append_tree(hg);

      /* some buttons */
      hbox = gtkut_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
      switch(hg->fcdb_type){
      case FCDB_TYPE_SIMBAD:
      case FCDB_TYPE_NED:
      case FCDB_TYPE_SDSS:
      case FCDB_TYPE_LAMOST:
      case FCDB_TYPE_SMOKA:
      case FCDB_TYPE_HST:
      case FCDB_TYPE_ESO:
      case FCDB_TYPE_GEMINI:
#ifdef USE_OSX
	icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
	icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
	if(strcmp(hg->www_com,"firefox")==0){
	  icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
	}
	else{
	  icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
	}
#endif
	button=gtkut_button_new_from_pixbuf("Browse", icon);
	g_object_unref(icon);
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
	my_signal_connect (button, "clicked",
				   G_CALLBACK (fcdb_simbad), (gpointer)hg);
	
	break;

      default:
	break;
      }

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Main target","list-add");
#else
      button=gtkut_button_new_from_stock("Main target",GTK_STOCK_ADD);
#endif
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (add_item_fcdb), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Add to the Main Target List");
#endif

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("Guide Star","list-add");
#else
      button=gtkut_button_new_from_stock("Guide Star",GTK_STOCK_ADD);
#endif
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (add_item_gs), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Add as a Guide Star");
#endif
      
      label= gtk_label_new ("    ");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
      
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name("OPE Def.","accessories-text-editor");
#else
      button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 make_fcdb_tgt, (gpointer)hg);
      
      hg->fcdb_tgt = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->fcdb_tgt,TRUE, TRUE, 0);
      gtk_editable_set_editable(GTK_EDITABLE(hg->fcdb_tgt),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->fcdb_tgt),50);
      
#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name(NULL,"edit-copy");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (clip_copy), (gpointer)hg->fcdb_tgt);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Copy to clipboard");
#endif
    }


    // TRDB
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *button;
      GtkWidget *entry;
      GdkPixbuf *icon;

      vbox = gtkut_vbox_new (FALSE, 5);
      label = gtk_label_new ("DB / Main Target");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtkut_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"document-save");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (do_save_TRDB_CSV), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_search_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Find Object");
#endif
      
      hg->trdb_search_i=0;
      hg->trdb_search_imax=0;
      
      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox), entry,FALSE, FALSE, 0);
      gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),10);
      my_signal_connect (entry, "changed", trdb_cc_search_text, (gpointer)hg);
      my_signal_connect (entry, "activate", trdb_search_item, (gpointer)hg);
      
      hg->trdb_search_label = gtk_label_new ("     ");
      gtk_box_pack_start(GTK_BOX(hbox),hg->trdb_search_label,FALSE,FALSE,0);

      hg->trdb_label= gtk_label_new (hg->trdb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->trdb_label, TRUE, TRUE, 0);
      
      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
	GtkWidget *combo;
	gint i_inst;
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "Data Archive",
			   1, TRDB_TYPE_SMOKA, -1);
	if((hg->trdb_used==TRDB_TYPE_SMOKA)||
	   (hg->trdb_used==TRDB_TYPE_HST)||
	   (hg->trdb_used==TRDB_TYPE_ESO)||
	   (hg->trdb_used==TRDB_TYPE_GEMINI)) iter_set=iter;

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SIMBAD",
			   1, MAGDB_TYPE_SIMBAD, -1);
	if(hg->trdb_used==MAGDB_TYPE_SIMBAD) iter_set=iter;
	iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "NED",
			   1, MAGDB_TYPE_NED, -1);
	if(hg->trdb_used==MAGDB_TYPE_NED) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "LAMOST",
			   1, MAGDB_TYPE_LAMOST, -1);
	if(hg->trdb_used==MAGDB_TYPE_LAMOST) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GSC",
			   1, MAGDB_TYPE_GSC, -1);
	if(hg->trdb_used==MAGDB_TYPE_GSC) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "PanSTARRS1",
			   1, MAGDB_TYPE_PS1, -1);
	if(hg->trdb_used==MAGDB_TYPE_PS1) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "SDSS",
			   1, MAGDB_TYPE_SDSS, -1);
	if(hg->trdb_used==MAGDB_TYPE_SDSS) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "GAIA",
			   1, MAGDB_TYPE_GAIA, -1);
	if(hg->trdb_used==MAGDB_TYPE_GAIA) iter_set=iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "2MASS",
			   1, MAGDB_TYPE_2MASS, -1);
	if(hg->trdb_used==MAGDB_TYPE_2MASS) iter_set=iter;
	
	hg->trdb_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(hbox), hg->trdb_combo, FALSE, FALSE, 0);
	g_object_unref(store);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->trdb_combo),
				   renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->trdb_combo), 
					renderer, "text",0,NULL);
	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->trdb_combo),&iter_set);
	gtk_widget_show(hg->trdb_combo);
	my_signal_connect (hg->trdb_combo,"changed",cc_get_combo_box_trdb,
			   (gpointer)hg);
      }
      
     
      hg->trdb_sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->trdb_sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->trdb_sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), hg->trdb_sw, TRUE, TRUE, 0);

      trdb_append_tree(hg);

      /* some buttons */
      hbox = gtkut_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Show Detail","go-previous");
#else
      button=gtkut_button_new_from_stock("Show Detail",GTK_STOCK_GO_BACK);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_dbtab), (gpointer)hg);

#ifdef USE_OSX
      icon = gdk_pixbuf_new_from_resource ("/icons/safari_icon.png", NULL);
#elif defined(USE_WIN32)
      icon = gdk_pixbuf_new_from_resource ("/icons/ie_icon.png", NULL);
#else
      if(strcmp(hg->www_com,"firefox")==0){
	icon = gdk_pixbuf_new_from_resource ("/icons/firefox_icon.png", NULL);
      }
      else{
	icon = gdk_pixbuf_new_from_resource ("/icons/chrome_icon.png", NULL);
      }
#endif
      button=gtkut_button_new_from_pixbuf("Browse", icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_simbad), (gpointer)hg);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("FC","starred");
#else
      button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (fc_item_trdb), (gpointer)hg);
    }

    // ラインリスト
    {
      GtkWidget *hbox;
      GtkWidget *entry;
      GtkWidget *button;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *combo;
      GtkWidget *label;
      GtkWidget *check;
      gchar tmp[12];

      table = gtk_table_new (2, 2, FALSE);

      
      hg->sw_linetree = gtk_scrolled_window_new (NULL, NULL);
      gtk_table_attach_defaults (GTK_TABLE(table), hg->sw_linetree, 0, 2, 0, 1);
      gtk_container_set_border_width (GTK_CONTAINER (hg->sw_linetree), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->sw_linetree),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);
      gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(hg->sw_linetree),
					GTK_CORNER_BOTTOM_LEFT);

      make_line_tree(hg);

      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Clear","edit-clear-all");
#else
      button=gtkut_button_new_from_stock("Clear",GTK_STOCK_CLEAR);
#endif
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_init), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Stellar Absorption","format-justify-fill");
#else
      button=gtkut_button_new_from_stock("Stellar Absorption",GTK_STOCK_YES);
#endif
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_star), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("Nebular Emission","format-justify-fill");
#else
      button=gtkut_button_new_from_stock("Nebular Emission",GTK_STOCK_YES);
#endif
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_nebula), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("High-z QSO","format-justify-fill");
#else
      button=gtkut_button_new_from_stock("High-z QSO",GTK_STOCK_YES);
#endif
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_highz), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

      label = gtk_label_new ("    Redshift: ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

      hg->etc_z_adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_z,
							  -0.1, 6.0, 0.1, 0.1, 0);
      my_signal_connect (hg->etc_z_adj, "value_changed",
			 cc_get_adj_double,
			 &hg->etc_z);
      spinner =  gtk_spin_button_new (hg->etc_z_adj, 1, 3);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);


      label = gtk_label_new ("EFS Line List");
      gtk_widget_show(label);
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
      
    }


    // ETC
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *button;
      GdkPixbuf *icon;

      vbox = gtkut_vbox_new (FALSE, 5);
      label = gtk_label_new ("ETC");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtkut_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
      hg->etc_label= gtk_label_new (hg->etc_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->etc_label, TRUE, TRUE, 0);

      icon = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
      button=gtkut_button_new_from_pixbuf(NULL, icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (do_etc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Recalc ETC");
#endif

      icon = gdk_pixbuf_new_from_resource ("/icons/efs_icon.png", NULL);
      button=gtkut_button_new_from_pixbuf(NULL, icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (do_efs_for_etc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Display Echelle Format");
#endif

      
      hg->etc_sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->etc_sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->etc_sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), hg->etc_sw, TRUE, TRUE, 0);

      etc_append_tree(hg);
    }


    gtk_widget_show_all(hg->all_note);
  }
}


GtkWidget *make_menu(typHOE *hg){
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
  
  //File/Open List
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Open List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Open List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open,(gpointer)hg);

  //File/Merge List
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("insert-object", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Merge List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Merge List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_merge,(gpointer)hg);


  //File/Import List from OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Import List from OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Import List from OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open_ope,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //Non-Sidereal/Merge TSC
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge TSC file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge TSC file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_NST,(gpointer)hg);

    //Non-Sidereal/Merge JPL
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image,
      "Merge JPL HORIZONS file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge JPL HORIZONS file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_JPL,(gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);

    //Non-Sidereal/Conv JPL to TSC
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image,
							"Convert HORIZONS to TSC");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Convert HORIZONS to TSC");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_conv_JPL,(gpointer)hg);


    popup_button =gtk_menu_item_new_with_label ("Non-Sidereal");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Write Base OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write Base OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write Base OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

#ifdef USE_SSL
  //File/Upload OPE
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("network-transmit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Upload OPE");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Upload OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_upload,(gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-downloads", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Download LOG");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Download LOG");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_download_log,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);
#endif

  //File/Save PROMS/Service Request File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Write PROMS Target List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write PROMS Target List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_proms_txt,(gpointer)hg);

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

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Load Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Load Config");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Load Config");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_read_hoe,(gpointer)hg);


  //File/Save Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Save Config");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save Config");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_hoe,(gpointer)hg);


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_quit,(gpointer)hg);



  //// Edit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("accessories-text-editor", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Edit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Edit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Edit/PLan Editor
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("format-indent-more", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Obs. Plan Editor");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INDENT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Obs. Plan Editor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_plan,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //Edit/Saved OPE File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("accessories-text-editor", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Text Editor (Saved OPE)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Saved OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_edit,(gpointer)hg);

  //Edit/Select OPE File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Text Editor (Select OPE)");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Select OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_name_edit,(gpointer)hg);



  //// Tool
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("applications-engineering", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Tool");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Tool");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Tool/Echelle Format Simulator
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/efs_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "EFS: Echelle Format Simulator");
#else
  popup_button =gtk_image_menu_item_new_with_label ("EFS: Echelle Format Simulator");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_efs_cairo,(gpointer)hg);

  //Tool/Exposure Time Calculator
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "ETC: Exposure Time Calculator");
#else
  popup_button =gtk_image_menu_item_new_with_label ("ETC: Exposure Time Calculator");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_etc,(gpointer)hg);

  //Tool/PDF Finding Charts
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/pdf_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "PDF Finding Charts");
#else
  popup_button =gtk_image_menu_item_new_with_label ("PDF Finding Charts");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_fc_pdf_all,(gpointer)hg);

  //Tool/Sky Monitor
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/sky_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, "Sky Monitor");
#else
  popup_button =gtk_image_menu_item_new_with_label ("Sky Monitor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_skymon,(gpointer)hg);
  //g_resources_unregister(resource);


  //// Update
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Update");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Update");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/etc_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Calc S/N by ETC");
#else
  popup_button =gtk_image_menu_item_new_with_label ("Calc S/N by ETC");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_etc_list,(gpointer)hg);

  //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Set Default Guide/PA/Exp");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Set Default Guide/PA/Exp");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_export_def_list,(gpointer)hg);

  //Update/Exptime
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Exptime using Mag");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Exptime using Mag");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_update_exp_list,(gpointer)hg);


  ////Database
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-web", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Database");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Database");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  // Data Archive List Query
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SMOKA");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SMOKA");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_smoka, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "HST archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("HST archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_hst, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "ESO archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("ESO archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_eso, (gpointer)hg);

    // Gemini
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, 
							"Gemini archive");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Gemini archive");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       trdb_gemini, (gpointer)hg);

    popup_button =gtk_menu_item_new_with_label ("Data Archive List Query");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // MagDB
  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SIMBAD");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SIMBAD");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_simbad, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "NED");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("NED");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_ned, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "LAMOST DR3");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("LAMOST DR3");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_lamost, (gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "GSC 2.3");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("GSC 2.3");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_gsc, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "PanSTARRS-1");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("PanSTARRS-1");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_ps1, (gpointer)hg);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "SDSS DR14");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("SDSS DR14");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_sdss, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "GAIA DR1");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("GAIA DR1");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_gaia, (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "2MASS");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("2MASS");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",
		       magdb_2mass, (gpointer)hg);

    popup_button =gtk_menu_item_new_with_label ("Catalog Matching");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // Standard
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, 
						      "Param for Standard");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for Standard");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     create_std_para_dialog, (gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Param for DB query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for DB query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     fcdb_para_item, (gpointer)hg);

  //// Info
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Info");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Check the latest ver.");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Check the latest ver.");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",CheckVer, (gpointer)hg);

  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "About");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_version, (gpointer)hg);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}


void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void cc_get_toggle_sm (GtkWidget * widget, gboolean * gdata)
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



static void cc_usesetup (GtkWidget *widget, gpointer gdata)
{
  gint i_set;
  gchar *p,tmp[10];
  confSetup *cdata;

  
  cdata = (confSetup *)gdata;

  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &i_set, -1);

    cdata->hg->setup[cdata->i_use].setup = i_set;
  }

  if (i_set>=0) {
    sprintf(tmp,"%d",(guint)(setups[i_set].slit_length*500));
    gtk_entry_set_text(GTK_ENTRY(&GTK_SPIN_BUTTON(cdata->length_entry)->entry),tmp);
    cdata->hg->setup[cdata->i_use].slit_length = setups[i_set].slit_length*500;
    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cdata->fil1_combo))),
		       setups[i_set].fil1);
    g_free(cdata->hg->setup[cdata->i_use].fil1);
    cdata->hg->setup[cdata->i_use].fil1=g_strdup(setups[i_set].fil1);
    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cdata->fil2_combo))),
		       setups[i_set].fil2);
    g_free(cdata->hg->setup[cdata->i_use].fil2);
    cdata->hg->setup[cdata->i_use].fil2=g_strdup(setups[i_set].fil2);
  }
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

void cc_get_fil_combo (GtkWidget *widget)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gchar *c;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 0, &c, -1);

    gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))),c);
    g_free(c);
  }
}

void cc_get_combo_box_trdb (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg=(typHOE *)gdata;
  gint fcdb_tmp;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    fcdb_tmp=hg->fcdb_type;
    if(n==TRDB_TYPE_SMOKA){
      n=hg->trdb_da;
    }
    hg->trdb_used=n;
    hg->fcdb_type=n;
    trdb_make_tree(hg);
    rebuild_trdb_tree(hg);
    hg->fcdb_type=fcdb_tmp;
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


void ext_play(char *exe_command)
{
#ifdef USE_WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  if(exe_command){
    ZeroMemory(&si, sizeof(si));
    si.cb=sizeof(si);

    if(CreateProcess(NULL, (LPTSTR)exe_command, NULL, NULL,
		     FALSE, NORMAL_PRIORITY_CLASS,
		     NULL, NULL, &si, &pi)){
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
    }
      
  }
  
#else
  static pid_t pid;
  gchar *cmdline;
  
  if(exe_command){
    waitpid(pid,0,WNOHANG);
    if(strcmp(exe_command,"\0")!=0){
      cmdline=g_strdup(exe_command);
      if( (pid = fork()) == 0 ){
	if(system(cmdline)==-1){
	  fprintf(stderr, "Error: cannot execute command \"%s\"!\n", exe_command);
	  _exit(-1);
	}
	_exit(-1);
	signal(SIGCHLD,ChildTerm);
      }
      g_free(cmdline);
    }
  }
#endif // USE_WIN32
}


gboolean ow_dialog (typHOE *hg, gchar *fname)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  gchar *tmp;

  dialog = gtk_dialog_new_with_buttons("HOE : Overwrite?",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_CANCEL));

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-question",
				   GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  tmp=g_strdup_printf("The file, \"%s\", already exists.", fname);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  if(tmp) g_free(tmp);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Do you want to overwrite it?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return(TRUE);
  }
  else{
    gtk_widget_destroy(dialog);
    return(FALSE);
  }
}


void create_quit_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Quit Program",
				       GTK_WINDOW(hg->w_top),
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
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-question",
				   GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Do you want to quit this program?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    WriteConf(hg);
    gtk_widget_destroy(dialog);
    exit(0);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
}

void delete_quit (GtkWidget *widget, GdkEvent *event, gpointer gdata){
  do_quit(widget, gdata);
}

void do_quit (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg=(typHOE*) gdata;

  if(!flagChildDialog){
    create_quit_dialog(hg);
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs before quitting.",
		  NULL);
  }
}

void do_open (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_read,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_read));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_read));
  }

  my_file_chooser_add_filter(fdialog,"List File", 
			     "*." LIST1_EXTENSION,
			     "*." LIST2_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_read) g_free(hg->filename_read);
      hg->filename_read=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadList(hg);
      ////make_obj_list(hg,TRUE);
      hg->i_plan_max=0;
      make_obj_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning",
#else
		    GTK_STOCK_DIALOG_WARNING, 
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE; 
}


void do_open2 (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning",
#else
		  GTK_STOCK_DIALOG_WARNING, 
#endif
		  POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_read,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_read));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_read));
  }

  my_file_chooser_add_filter(fdialog,"List File", 
			     "*." LIST1_EXTENSION,
			     "*." LIST2_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_read) g_free(hg->filename_read);
      hg->filename_read=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadList2(hg);
      ////make_obj_list(hg,TRUE);
      hg->i_plan_max=0;
      make_obj_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}

void do_open_ope (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_read,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_read));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_read));
  }

  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_read) g_free(hg->filename_read);
      hg->filename_read=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadListOPE(hg);
      ////make_obj_list(hg,TRUE);
      make_obj_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void do_open_NST (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Non-Sidereal Tracking File [TSC]",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_nst,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_nst));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_nst));
  }

  my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			     "*." NST1_EXTENSION,
			     "*." NST2_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_nst) g_free(hg->filename_nst);
      hg->filename_nst=g_strdup(dest_file);
      MergeNST(hg);

      //// Current Condition
      if(hg->skymon_mode==SKYMON_SET){
	calcpa2_skymon(hg);
      }
      else{
	calcpa2_main(hg);
      }

      make_obj_tree(hg);
      trdb_make_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
}


void do_open_JPL (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Non-Sidereal Tracking File  [JPL HRIZONS]",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_jpl,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_jpl));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_jpl));
  }

  my_file_chooser_add_filter(fdialog,"JPL HORIZONS File", 
			     "*." NST1_EXTENSION,
			     "*." NST3_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_jpl) g_free(hg->filename_jpl);
      hg->filename_jpl=g_strdup(dest_file);
      MergeJPL(hg);

      //// Current Condition
      if(hg->skymon_mode==SKYMON_SET){
	calcpa2_skymon(hg);
      }
      else{
	calcpa2_main(hg);
      }

      make_obj_tree(hg);
      trdb_make_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
      }
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
}


void do_conv_JPL (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  GtkWidget *fdialog_w;
  typHOE *hg;
  char *fname, *fname_w;
  gchar *dest_file, *dest_file_w;
  gboolean ret=TRUE;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select Non-Sidereal Tracking File  [JPL HRIZONS]",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_jpl,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_jpl));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_jpl));
  }

  my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			     "*." NST1_EXTENSION,
			     "*." NST3_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    gchar *cpp, *basename0, *basename1;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_jpl) g_free(hg->filename_jpl);
      hg->filename_jpl=g_strdup(dest_file);
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
      return;
    }
     
    if(fname) g_free(fname);
    if(dest_file) g_free(dest_file);
    
    fdialog_w = gtk_file_chooser_dialog_new("Sky Monitor : Input TSC Tracking File to be saved",
					    GTK_WINDOW(hg->w_top),
					    GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					    "_Cancel",GTK_RESPONSE_CANCEL,
					    "_Save", GTK_RESPONSE_ACCEPT,
#else
					    GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					    NULL);
    
    gtk_dialog_set_default_response(GTK_DIALOG(fdialog_w), GTK_RESPONSE_ACCEPT); 
    
    my_file_chooser_add_filter(fdialog_w,"TSC Tracking File", 
			       "*." NST1_EXTENSION,
			       "*." NST3_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    
    my_file_chooser_add_filter(fdialog_w,"All File","*",NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog_w), 
					 to_utf8(g_path_get_dirname(hg->filename_jpl)));
    
    basename0=g_path_get_basename(hg->filename_jpl);
    cpp=(gchar *)strtok(basename0,".");
    basename1=g_strconcat(cpp,".",NST2_EXTENSION,NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog_w), 
				       to_utf8(basename1));
    if(basename0) g_free(basename0);
    if(basename1) g_free(basename1);
    
    gtk_widget_show_all(fdialog_w);
    
    
    if (gtk_dialog_run(GTK_DIALOG(fdialog_w)) == GTK_RESPONSE_ACCEPT) {
      fname_w = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog_w)));
      gtk_widget_destroy(fdialog_w);
      dest_file_w=to_locale(fname_w);
      
      if(access(dest_file_w,F_OK)==0){
	ret=ow_dialog(hg, dest_file_w);
      }

      if(ret){
	if(hg->filename_tscconv) g_free(hg->filename_tscconv);
	hg->filename_tscconv=g_strdup(dest_file_w);
	ConvJPL(hg);
      }
      if(fname_w) g_free(fname_w);
      if(dest_file_w) g_free(dest_file_w);
    }
    else {
      gtk_widget_destroy(fdialog_w);
    }
  }
  else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
}


void do_upload (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File to be Uploaded",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_write,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_write));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_write));
  }

  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_read) g_free(hg->filename_read);
      hg->filename_read=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      UploadOPE(hg);
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void do_download_log (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }


  fdialog = gtk_file_chooser_dialog_new("HOE : Input Log File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 

  if(hg->filename_log) g_free(hg->filename_log);
  if(hg->filename_write){
    hg->filename_log=g_strdup_printf("%s%shdslog-%04d%02d%02d.txt",
				     to_utf8(g_path_get_dirname(hg->filename_write)),
				     G_DIR_SEPARATOR_S,
				     hg->fr_year,hg->fr_month,hg->fr_day);
  }
  else{
    hg->filename_log=g_strdup_printf("%s%shdslog-%04d%02d%02d.txt",
				     hg->home_dir,
				     G_DIR_SEPARATOR_S,
				     hg->fr_year,hg->fr_month,hg->fr_day);
  }

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_dirname(hg->filename_log)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_log)));



    my_file_chooser_add_filter(fdialog,"TXT File","*." LIST3_EXTENSION,NULL);
    my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_log) g_free(hg->filename_log);
	hg->filename_log=g_strdup(dest_file);
	DownloadLOG(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void do_merge (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_read,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_read));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_read));
  }

  my_file_chooser_add_filter(fdialog,"List File", 
			     "*." LIST1_EXTENSION,
			     "*." LIST2_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_read) g_free(hg->filename_read);
      hg->filename_read=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      MergeList(hg);
      ////make_obj_list(hg,TRUE);
      make_obj_tree(hg);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
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
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}

void do_save (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckDefDup(hg)) return;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Input OPE File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(!hg->filename_write){
    if(hg->filehead){
      hg->filename_write=g_strconcat(hg->filehead,"." OPE_EXTENSION,NULL);
    }
  }

  if(access(hg->filename_write,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_write));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_write));
  }
  else if(hg->filename_write){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_write)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_write)));
  }


  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_write) g_free(hg->filename_write);
	hg->filename_write=g_strdup(dest_file);
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	WriteOPE(hg, FALSE);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }

    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void do_save_plan (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckDefDup(hg)) return;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input OPE File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_write) g_free(hg->filename_write);
    hg->filename_write=g_strconcat(hg->filehead,"." OPE_EXTENSION,NULL);
  }

  if(access(hg->filename_write,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_write));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_write));
  }
  else if(hg->filename_write){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_write)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_write)));
  }


  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_write) g_free(hg->filename_write);
	hg->filename_write=g_strdup(dest_file);
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	WriteOPE(hg, TRUE);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}

void do_save_plan_txt (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input Text File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_txt) g_free(hg->filename_txt);
    hg->filename_txt=g_strconcat(hg->filehead,PLAN_EXTENSION,NULL);
  }

  if(access(hg->filename_txt,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_txt));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_txt));
  }
  else if(hg->filename_txt){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_txt)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_txt)));
  }


  my_file_chooser_add_filter(fdialog,"Plan Text File","*" PLAN_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	if(hg->filename_txt) g_free(hg->filename_txt);
	hg->filename_txt=g_strdup(dest_file);
	WritePlan(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_service_txt (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  gboolean flag_exit=FALSE;
  gint i_list;
  typHOE *hg;

  hg=(typHOE *)gdata;

  // Precheck
  if(hg->i_max==0) flag_exit=TRUE;
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(fabs(hg->obj[i_list].mag)>99) flag_exit=TRUE;
    if(fabs(hg->obj[i_list].snr)<0) flag_exit=TRUE;
  }

  if(flag_exit){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*3,
		  "Error: Please Set Mags for your objects,",
		  "          and Check S/N ratios using \"Update/Calc S/N by ETC\".",
		  NULL);
    return;
  }


  fdialog = gtk_file_chooser_dialog_new("HOE : Input Text File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_txt) g_free(hg->filename_txt);
    hg->filename_txt=g_strconcat(hg->filehead,SERVICE_EXTENSION,NULL);
  }

  if(access(hg->filename_txt,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_txt));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_txt));
  }
  else if(hg->filename_txt){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_txt)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_txt)));
  }


  my_file_chooser_add_filter(fdialog,"Service Text File","*" SERVICE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
      
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	if(hg->filename_txt) g_free(hg->filename_txt);
	hg->filename_txt=g_strdup(dest_file);
	WriteService(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_proms_txt (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  gint i_list;
  typHOE *hg;
  enum{ERROR_NO, ERROR_MAG, ERROR_EQ};
  gint flag_exit=ERROR_NO;

  hg=(typHOE *)gdata;

  // Precheck
  if(hg->i_max==0) flag_exit=TRUE;
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(fabs(hg->obj[i_list].equinox-2000.0)>0.01) flag_exit=ERROR_EQ;
    if(fabs(hg->obj[i_list].mag)>99) flag_exit=ERROR_MAG;
  }

  switch(flag_exit){
  case ERROR_EQ:
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*3,
		  "Error: Please use Equinox J2000.0 for your target coordinates.",
		  NULL);
    return;
    break;

  case ERROR_MAG:
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*3,
		  "Error: Please set Mags for your targets.",
		  NULL);
    return;
    break;
  }


  fdialog = gtk_file_chooser_dialog_new("HOE : Input Text File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_txt) g_free(hg->filename_txt);
    hg->filename_txt=g_strconcat(hg->filehead,PROMS_EXTENSION,NULL);
  }

  if(access(hg->filename_txt,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_txt));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_txt));
  }
  else if(hg->filename_txt){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_txt)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_txt)));
  }


  my_file_chooser_add_filter(fdialog,"PROMS Text File","*" PROMS_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	if(hg->filename_txt) g_free(hg->filename_txt);
	hg->filename_txt=g_strdup(dest_file);
	WritePROMS(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_plan_yaml (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input YAML File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_txt) g_free(hg->filename_txt);
    hg->filename_txt=g_strconcat(hg->filehead,"." YAML_EXTENSION,NULL);
  }

  if(access(hg->filename_txt,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_txt));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_txt));
  }
  else if(hg->filename_txt){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_txt)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_txt)));
  }


  my_file_chooser_add_filter(fdialog,"YAML File","*." YAML_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	if(hg->filename_txt) g_free(hg->filename_txt);
	hg->filename_txt=g_strdup(dest_file);
	WriteYAML(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_pdf) g_free(hg->filename_pdf);
    hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
  }

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_pdf) g_free(hg->filename_pdf);
	hg->filename_pdf=g_strdup(dest_file);
	
	pdf_plot(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}

void do_save_skymon_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_pdf) g_free(hg->filename_pdf);
    hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
  }

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_pdf) g_free(hg->filename_pdf);
	hg->filename_pdf=g_strdup(dest_file);
	
	pdf_skymon(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_efs_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_pdf) g_free(hg->filename_pdf);
    hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
  }

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);
  
  gtk_widget_show_all(fdialog);
  
  
  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);
    
    dest_file=to_locale(fname);
    
    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_pdf) g_free(hg->filename_pdf);
	hg->filename_pdf=g_strdup(dest_file);
	
	pdf_efs(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_fc_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_pdf) g_free(hg->filename_pdf);
    hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
  }

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_pdf) g_free(hg->filename_pdf);
	hg->filename_pdf=g_strdup(dest_file);
	
	pdf_fc(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_fc_pdf_all (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_pdf) g_free(hg->filename_pdf);
    hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
  }

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_pdf) g_free(hg->filename_pdf);
	hg->filename_pdf=g_strdup(dest_file);
	
	create_fc_all_dialog(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void do_save_hoe (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Input HOE File to be Saved",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filehead){
    if(hg->filename_hoe) g_free(hg->filename_hoe);
    hg->filename_hoe=g_strconcat(hg->filehead,"." HOE_EXTENSION,NULL);
  }

  if(access(hg->filename_hoe,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_hoe));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_hoe));
  }
  else if(hg->filename_hoe){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_hoe)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_hoe)));
  }


  my_file_chooser_add_filter(fdialog,"HOE Config File","*." HOE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_hoe) g_free(hg->filename_hoe);
	hg->filename_hoe=g_strdup(dest_file);
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	WriteHOE(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


gchar *fcdb_csv_name (typHOE *hg){
  gchar *fname;
  gchar *oname;

  oname=cut_spc(hg->obj[hg->fcdb_i].name);
		
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    fname=g_strconcat("FCDB_", oname, "_by_SIMBAD." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_NED:
    fname=g_strconcat("FCDB_", oname, "_by_NED." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GSC:
    fname=g_strconcat("FCDB_", oname, "_by_GSC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_PS1:
    fname=g_strconcat("FCDB_", oname, "_by_PanSTARRS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SDSS:
    fname=g_strconcat("FCDB_", oname, "_by_SDSS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_LAMOST:
    fname=g_strconcat("FCDB_", oname, "_by_LAMOST." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_USNO:
    fname=g_strconcat("FCDB_", oname, "_by_USNO." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GAIA:
    fname=g_strconcat("FCDB_", oname, "_by_GAIA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_2MASS:
    fname=g_strconcat("FCDB_", oname, "_by_2MASS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_WISE:
    fname=g_strconcat("FCDB_", oname, "_by_WISE." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_IRC:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_IRC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_FIS:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_FIS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SMOKA:
    fname=g_strconcat("FCDB_", oname, "_by_SMOKA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_HST:
    fname=g_strconcat("FCDB_", oname, "_by_HSTarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_ESO:
    fname=g_strconcat("FCDB_", oname, "_by_ESOarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GEMINI:
    fname=g_strconcat("FCDB_", oname, "_by_GEMINIarchive." CSV_EXTENSION,NULL);
    break;

  default:
    fname=g_strconcat("FCDB_", oname, "_by_hskymon." CSV_EXTENSION,NULL);
    break;
  }

  if(oname) g_free(oname);

  return(fname);
}


void do_save_FCDB_List (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;


  fdialog = gtk_file_chooser_dialog_new("HOE : CSV File to be Saved (FCDB)",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filename_fcdb) g_free(hg->filename_fcdb);
  hg->filename_fcdb=fcdb_csv_name(hg);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_dirname(hg->filename_fcdb)));
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				     to_utf8(g_path_get_basename(hg->filename_fcdb)));

  my_file_chooser_add_filter(fdialog,"CSV File","*." CSV_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_fcdb) g_free(hg->filename_fcdb);
	hg->filename_fcdb=g_strdup(dest_file);
	
	Export_FCDB_List(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT*2,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }
}


gchar* repl_nonalnum(gchar * obj_name, const gchar c_repl){
  gchar *tgt_name, *ret_name;
  gint  i_obj;

  if((tgt_name=(gchar *)g_malloc(sizeof(gchar)*(strlen(obj_name)+1)))
     ==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(!isalnum(obj_name[i_obj])){
      tgt_name[i_obj]=c_repl;
    }
    else{
      tgt_name[i_obj]=obj_name[i_obj];
    }
  }

  tgt_name[i_obj]='\0';
  ret_name=g_strdup(tgt_name);

  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}


gchar* trdb_file_name (typHOE *hg, const gchar *ext){
  gchar *fname=NULL;
  gchar *iname=NULL;

  switch(hg->trdb_used){
  case TRDB_TYPE_SMOKA:
    iname=repl_nonalnum(smoka_subaru[hg->trdb_smoka_inst_used].name,0x5F);
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_query_list_by_Subaru_",
		      iname,
		      ".",
		      ext,
		      NULL);
    break;

  case TRDB_TYPE_HST:
    switch(hg->trdb_hst_mode_used){
    case TRDB_HST_MODE_IMAGE:
      iname=repl_nonalnum(hst_image[hg->trdb_hst_image_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Imag.",
			ext,
			NULL);
      break;

    case TRDB_HST_MODE_SPEC:
      iname=repl_nonalnum(hst_spec[hg->trdb_hst_spec_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Spec.",
			ext,
			NULL);
      break;

    case TRDB_HST_MODE_OTHER:
      iname=repl_nonalnum(hst_other[hg->trdb_hst_other_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Other.",
			ext,
			NULL);
      break;
    }
    break;

  case TRDB_TYPE_ESO:
    switch(hg->trdb_eso_mode_used){
    case TRDB_ESO_MODE_IMAGE:
      iname=repl_nonalnum(eso_image[hg->trdb_eso_image_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Imag.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_SPEC:
      iname=repl_nonalnum(eso_spec[hg->trdb_eso_spec_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Spec.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_VLTI:
      iname=repl_nonalnum(eso_vlti[hg->trdb_eso_vlti_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_IF.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_POLA:
      iname=repl_nonalnum(eso_pola[hg->trdb_eso_pola_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Pola.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_CORO:
      iname=repl_nonalnum(eso_coro[hg->trdb_eso_coro_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Coro.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_OTHER:
      iname=repl_nonalnum(eso_other[hg->trdb_eso_other_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Other.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_SAM:
      iname=repl_nonalnum(eso_sam[hg->trdb_eso_sam_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_SAM.",
			ext,
			NULL);
      break;
    }
    break;
  case TRDB_TYPE_GEMINI:
    iname=repl_nonalnum(gemini_inst[hg->trdb_gemini_inst_used].name,0x5F);
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_query_list_by_Gemini_",
		      iname,
		      ".",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_SIMBAD:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_SIMBAD_matching_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_NED:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_NED_matching_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_LAMOST:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_LAMOST_matching_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_GSC:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_GSC_mag_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_PS1:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_PanSTARRS1_mag_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_SDSS:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_SDSS_mag_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_GAIA:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_GAIA_mag_list.",
		      ext,
		      NULL);
    break;

  case MAGDB_TYPE_2MASS:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_2MASS_mag_list.",
		      ext,
		      NULL);
    break;

  default:
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_DB_output.",
		      ext,
		      NULL);
  }

  if(iname) g_free(iname);

  return(fname);
}


void do_save_TRDB_CSV (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->i_max<=0) return;

  fdialog = gtk_file_chooser_dialog_new("HOE : CSV file to be Saved (List Query)",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filename_trdb) g_free(hg->filename_trdb);
  hg->filename_trdb=trdb_file_name(hg, CSV_EXTENSION);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_dirname(hg->filename_trdb)));
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				     to_utf8(g_path_get_basename(hg->filename_trdb)));

  my_file_chooser_add_filter(fdialog,"CSV File","*." CSV_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      ret=ow_dialog(hg, dest_file);
    }

    if(ret){
      if((fp_test=fopen(dest_file,"w"))!=NULL){
	fclose(fp_test);
	
	if(hg->filename_trdb) g_free(hg->filename_trdb);
	hg->filename_trdb=g_strdup(dest_file);
	
	Export_TRDB_CSV(hg);
      }
      else{
	popup_message(hg->w_top, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT*2,
		      "Error: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }
}


void do_read_hoe (GtkWidget *widget,gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Select HOE Config File",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filename_hoe){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_hoe));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_hoe));
  }

  my_file_chooser_add_filter(fdialog,"HOE Config File","*." HOE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_hoe) g_free(hg->filename_hoe);
      hg->filename_hoe=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadHOE(hg,TRUE);

      if(flagSkymon){
	refresh_skymon(hg->skymon_dw,(gpointer)hg);
	skymon_set_and_draw(NULL, (gpointer)hg);
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
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  hg->skymon_year=hg->fr_year;
  hg->skymon_month=hg->fr_month;
  hg->skymon_day=hg->fr_day;
  hg->skymon_hour=18;
  hg->skymon_min=0;

  if(hg->skymon_mode==SKYMON_SET){
    calc_moon_skymon(hg);
    hg->skymon_hour=hg->atw18.s_set.hours;
    hg->skymon_min=hg->atw18.s_set.minutes;
    calcpa2_skymon(hg);
  }

  flagChildDialog=FALSE;
  
  calc_rst(hg);
}


void uri_clicked(GtkButton *button,
			gpointer data)
{
  gchar *cmdline;
  typHOE *hg=(typHOE *)data;

#ifdef USE_WIN32
  ShellExecute(NULL, 
	       "open", 
	       DEFAULT_URL,
	       NULL, 
	       NULL, 
	       SW_SHOWNORMAL);
#elif defined(USE_OSX)
  cmdline=g_strconcat("open ",DEFAULT_URL,NULL);

  if(system("open " DEFAULT_URL)==0){
    fprintf(stderr, "Error: Could not open the default www browser.");
  }
  g_free(cmdline);
#else
  cmdline=g_strconcat(hg->www_com," ",DEFAULT_URL,NULL);
  
  ext_play(cmdline);
  g_free(cmdline);
#endif
}


void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GdkPixbuf *pixbuf, *pixbuf2;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  typHOE *hg=(typHOE *) gdata;

  dialog = gtk_dialog_new_with_buttons("HOE : About This Program",
				       GTK_WINDOW(hg->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/hoe_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  48,48,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("HOE : Subaru HDS OPE file Editor,  version "VERSION);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#if HAVE_SYS_UTSNAME_H
  uname(&utsbuf);
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s %s (%s)",
	     utsbuf.sysname, utsbuf.release, utsbuf.machine);
#elif defined(USE_WIN32)
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s",
	     WindowsVersion());
#else
  g_snprintf(buf, sizeof(buf),
	     "Operating System: unknown UNIX");
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#ifdef USE_OSX
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : OpenSSL=%s, GtkMacIntegration=%s", 
#ifdef USE_SSL
	     "ON",
#else
             "OFF",
#endif
#ifdef USE_GTKMACINTEGRATION
	     "ON"
#else
	     "OFF"
#endif
	     );
#else
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : OpenSSL=%s", 
#ifdef USE_SSL
	     "ON"
#else
             "OFF"
#endif
	     );
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  
  label = gtk_label_new ("Copyright(C) 2003-18 Akito Tajitsu");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("<tajitsu@naoj.org>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  button = gtk_button_new_with_label(" "DEFAULT_URL" ");
  gtk_box_pack_start(GTK_BOX(vbox), 
		     button, TRUE, FALSE, 0);
  gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
  my_signal_connect(button,"clicked",uri_clicked, (gpointer)hg);
  gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(button)),
		       GTK_STATE_NORMAL,&color_blue);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     scrolledwin, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scrolledwin, 400, 250);
  
  text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 6);
  gtk_container_add(GTK_CONTAINER(scrolledwin), text);
  
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program (HOE) accesses to the following astronomical online database services via WWW. The author of the program (AT) acknowledge with thanks to all of them.\n\n"

			 "[SIMBAD]\n"
			 "    http://simbad.u-strasbg.fr/\n"
			 "    http://simbad.harvard.edu/\n\n"

			 "[The NASA/IPAC Extragalactic Database (NED)]\n"
			 "    http://ned.ipac.caltech.edu/\n\n"
			 
			 "[SkyView by NASA]\n"
			 "    https://skyview.gsfc.nasa.gov/\n\n"

			 "[SLOAN DIGITAL SKY SURVEY : SkyServer]\n"
			 "    http://skyserver.sdss.org/\n\n"

			 "[The Mikulski Archive for Space Telescopes (MAST)]\n"
			 "    http://archive.stsci.edu/\n\n"

			 "[Gemini Observatory Archive Search]\n"
			 "    https://archive.gemini.edu/\n\n"

			 "[NASA/IPAC Infrared Science Archive (IRSA)]\n"
			 "    http://irsa.ipac.caltech.edu\n\n"

			 "[The Combined Atlas of Sources with Spitzer IRS Spectra (CASSIS)]\n"
			 "    http://cassis.sirtf.com/\n\n"

			 "[Large Sky Area Multi-Object Fiber Spectroscoic Telescope (LAMOST)]\n"
			 "    http://www.lamost.org/\n\n"

			 "[The Subaru-Mitaka-Okayama-Kiso-Archive (SMOKA)]\n"
			 "    http://smoka.nao.ac.jp/\n\n"

			 "[The ESO Science Archive Facility]\n"
			 "    http://archive.eso.org/\n\n"

			 "[Keck Observatory Archive]\n"
			 "    https://koa.ipac.caltech.edu/\n\n"

			 "[Pan-STARRS1 data archive]\n"
			 "    https://panstarrs.stsci.edu/\n\n"

			 "[The VizieR catalogue access tool, CDS, Strasbourg, France]\n"
			 "    http://vizier.u-strasbg.fr/\n\n",
			 -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "\n======================================================\n\n"
			 , -1);

  
  gtk_text_buffer_insert(buffer, &iter,
			 "This program is free software; you can redistribute it and/or modify "
			 "it under the terms of the GNU General Public License as published by "
			 "the Free Software Foundation; either version 3, or (at your option) "
			 "any later version.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program is distributed in the hope that it will be useful, "
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 "See the GNU General Public License for more details.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "You should have received a copy of the GNU General Public License "
			 "along with this program.  If not, see <http://www.gnu.org/licenses/>.", -1);

  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_place_cursor(buffer, &iter);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
  }
}


void do_edit(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(!hg->filename_write){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "No OPE files have been saved yet.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }
  create_opedit_dialog(hg);
}


void do_plan(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  gtk_widget_set_sensitive(hg->f_objtree_arud,FALSE);
  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }
  create_plan_dialog(hg);
}


void do_skymon(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagSkymon){
    gdk_window_raise(gtk_widget_get_window(hg->skymon_main));
    return;
  }
  else{
    flagSkymon=TRUE;
  }
  
  create_skymon_dialog(hg);
}

void do_name_edit (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagChildDialog){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File to be Edited",
					GTK_WINDOW(hg->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filename_write){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_write));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_write));
  }

  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_write) g_free(hg->filename_write);
      hg->filename_write=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      create_opedit_dialog(hg);
    }
    else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}



void do_efs_cairo (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  dialog = gtk_dialog_new_with_buttons("HOE : Echelle Format Simulator",
				       GTK_WINDOW(hg->w_top),
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
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("EFS : Echelle Format Simulator");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new ("Setup:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 1.0);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    if(!hg->setup[hg->efs_setup].use) hg->efs_setup=0;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : NonStd-%d  %dx%dbinning",
		  i_use+1,
		  -hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s  %dx%dbinning",
		  i_use+1,
		  setups[hg->setup[i_use].setup].initial,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	if(hg->efs_setup==i_use) iter_set=iter;
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
		       &hg->efs_setup);
  }


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    go_efs(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void do_efs_for_etc (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *combo, *entry;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  hg->efs_setup=hg->etc_setup;

  go_efs(hg);
}

gchar* get_band_name(typHOE *hg, gint i){
  gchar *str=NULL;

  if(fabs(hg->obj[i].mag)>99){
    str=NULL;
  }
  else{
    switch(hg->obj[i].magdb_used){
    case MAGDB_TYPE_SIMBAD:
      str=g_strdup_printf("%s",simbad_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_GSC:
      str=g_strdup_printf("%s",gsc_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_PS1:
      str=g_strdup_printf("PanSTARRS %s",ps1_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_SDSS:
      str=g_strdup_printf("SDSS %s",sdss_band[hg->obj[i].magdb_band]);
      break;
      
    case MAGDB_TYPE_GAIA:
      str=g_strdup("GAIA G");
      break;
      
    case MAGDB_TYPE_2MASS:
      str=g_strdup_printf("2MASS %s",twomass_band[hg->obj[i].magdb_band]);
      break;
      
    default:
      str=NULL;
    }
  }

  return(str);
}

void do_etc (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *frame, *label, *button;
  GtkWidget *hbox, *combo, *entry, *table, *check;
  GtkWidget *fdialog, *spinner;
  GtkWidget *rb[ETC_SPEC_NUM], *rc[ETC_WAVE_NUM];
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg;
  gchar tmp[1024];
  gint i_list;
  gchar *str=NULL;
  gboolean skip_flag=TRUE;

  hg=(typHOE *)gdata;

  dialog = gtk_dialog_new_with_buttons("HOE : Exposure Time Calculator",
				       GTK_WINDOW(hg->w_top),
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
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  frame = gtk_frame_new ("Input flux spectrum");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtk_table_new (2, 3, FALSE);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		   GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode==ETC_LIST){
    label = gtk_label_new ("Assume ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  
  if(hg->etc_mode==ETC_OBJTREE){
    str=get_band_name(hg, hg->etc_i);
  }

  if(str){
    label = gtk_label_new (str);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    g_free(str);

    label = gtk_label_new (" magnitude: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  else{
    // BAND
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gint i_band;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      for(i_band=0;i_band<BAND_NUM;i_band++){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, etc_filters[i_band],
			   1, i_band, -1);
	if(hg->etc_filter==i_band) iter_set=iter;
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
			 &hg->etc_filter);
    }

    if(hg->etc_mode==ETC_LIST){
      label = gtk_label_new ("-band for user defined mag.   ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    }
    else{
      label = gtk_label_new ("magnitude: ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, 0.5);
      gtk_widget_set_valign (label, 0.5);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    }
  }
  
  if(hg->etc_mode!=ETC_LIST){
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_mag,
					      0.0, 22.0, 0.20, 0.20, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj_double,
		       &hg->etc_mag);
    spinner =  gtk_spin_button_new (adj, 1, 2);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

    label = gtk_label_new ("   ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
		   GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode==ETC_LIST){
    label = gtk_label_new ("All redshifts are assumed to be Zero.");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }
  else{
    label = gtk_label_new ("Redshift: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    spinner =  gtk_spin_button_new (hg->etc_z_adj, 1, 3);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  }


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_POWERLAW] 
    = gtk_radio_button_new_with_label_from_widget (NULL, "Power law");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_POWERLAW], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_POWERLAW], "toggled", cc_radio, &hg->etc_spek);

  label = gtk_label_new ("   Spectral index: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_alpha,
					    -3.0, 3.0, 0.01, 0.01, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->etc_alpha);
  spinner =  gtk_spin_button_new (adj, 1, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_BLACKBODY] 
    = gtk_radio_button_new_with_label_from_widget 
    (GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]), "Blackbody");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_BLACKBODY], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_BLACKBODY], "toggled", cc_radio, &hg->etc_spek);

  label = gtk_label_new ("   Temperature: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_bbtemp,
					    3000, 200000, 100, 100, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->etc_bbtemp);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),7);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("K");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
		   GTK_FILL,GTK_FILL,0,0);

  rb[ETC_SPEC_TEMPLATE] 
    = gtk_radio_button_new_with_label_from_widget 
    (GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]), "Template");
  gtk_box_pack_start(GTK_BOX(hbox), rb[ETC_SPEC_TEMPLATE], FALSE, FALSE, 0);
  my_signal_connect (rb[ETC_SPEC_TEMPLATE], "toggled", cc_radio, &hg->etc_spek);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[ETC_SPEC_POWERLAW]));

  // Template
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_st;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_st=0;i_st<ST_NUM;i_st++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, etc_st_name[i_st],
			 1, i_st, -1);
      if(hg->etc_sptype==i_st) iter_set=iter;
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
		       &hg->etc_sptype);
  }


  frame = gtk_frame_new ("Instrument setting");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtk_table_new (1, 5, FALSE);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		   GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Setup:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 1.0);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gchar *slit_tmp;
    int i_use;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    if(!hg->setup[hg->etc_setup].use) hg->etc_setup=0;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	switch(hg->setup[i_use].is){
	case IS_NO:
	  slit_tmp=g_strdup_printf("Normal Slit (Width=%.2lf\")",
				   (gdouble)hg->setup[i_use].slit_width/500.);
	  break;
	  
	case IS_030X5:
	    slit_tmp=g_strdup("IS#1 0\".3x5");
	    break;
	    
	case IS_045X3:
	  slit_tmp=g_strdup("IS#2 0\".45x3");
	  break;
	  
	case IS_020X3:
	  slit_tmp=g_strdup("IS#3 0\".2x3");
	  break;
	}
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : NonStd-%d  %dx%dbinning  %s",
		  i_use+1,
		  -hg->setup[i_use].setup,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y,
		  slit_tmp);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s  %dx%dbinning  %s",
		  i_use+1,
		  setups[hg->setup[i_use].setup].initial,
		  hg->binning[hg->setup[i_use].binning].x,
		  hg->binning[hg->setup[i_use].binning].y,
		  slit_tmp);
	}

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);
	if(hg->etc_setup==i_use) iter_set=iter;
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
		       &hg->etc_setup);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
		   GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Pre-Slit Optics:");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 1.0);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  // ADC
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ADC In",
		       1, ETC_ADC_IN, -1);
    if(hg->etc_adc==ETC_ADC_IN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ADC Out",
		       1, ETC_ADC_OUT, -1);
    if(hg->etc_adc==ETC_ADC_OUT) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_adc);
  }

  // ImR
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No Image Rotator",
		       1, ETC_IMR_NO, -1);
    if(hg->etc_imr==ETC_IMR_NO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Blue Image Rotator",
		       1, ETC_IMR_BLUE, -1);
    if(hg->etc_imr==ETC_IMR_BLUE) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Red Image Rotator",
		       1, ETC_IMR_RED, -1);
    if(hg->etc_imr==ETC_IMR_RED) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->etc_imr);
  }


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 2, 3,
		   GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Seeing: ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_seeing,
					    0.3, 3.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->etc_seeing);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  label = gtk_label_new ("arcsecond");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.5);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 3, 4,
		   GTK_FILL,GTK_FILL,0,0);

  if(hg->etc_mode!=ETC_LIST){
    label = gtk_label_new ("Exposure Time: ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_exptime,
					      1, 7200, 1, 1, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->etc_exptime);
    spinner =  gtk_spin_button_new (adj, 1, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

    label = gtk_label_new ("seconds");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  if(hg->etc_mode!=ETC_MENU){
    frame = gtk_frame_new ("Wavelength for S/N Display");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);
    
    table = gtk_table_new (1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
    
    rc[ETC_WAVE_CENTER] = gtk_radio_button_new_with_label_from_widget 
      (NULL, "The 1st order of Red CCD");
    gtk_box_pack_start(GTK_BOX(hbox), rc[ETC_WAVE_CENTER], FALSE, FALSE, 0);
    my_signal_connect (rc[ETC_WAVE_CENTER], "toggled", cc_radio, &hg->etc_wave);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_FILL,0,0);
    
    rc[ETC_WAVE_SPEC] = gtk_radio_button_new_with_label_from_widget 
      (GTK_RADIO_BUTTON(rc[ETC_WAVE_CENTER]), 
       "The order including the specified wavelength : ");
    gtk_box_pack_start(GTK_BOX(hbox), rc[ETC_WAVE_SPEC], FALSE, FALSE, 0);
    my_signal_connect (rc[ETC_WAVE_SPEC], "toggled", cc_radio, &hg->etc_wave);
    
    group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rc[ETC_WAVE_CENTER]));
    
    adj = (GtkAdjustment *)gtk_adjustment_new(hg->etc_waved,
					      3200, 9900, 1, 100, 0);
    my_signal_connect (adj, "value_changed",
		       cc_get_adj,
		       &hg->etc_waved);
    spinner =  gtk_spin_button_new (adj, 1, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
    gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			   TRUE);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
    gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
    
    label = gtk_label_new ("A");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  }

  if(hg->etc_mode==ETC_LIST){
    frame = gtk_frame_new ("Update S/N in the list");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       frame,FALSE, FALSE, 0);
    
    table = gtk_table_new (1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);
    
    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_FILL,0,0);
    
    check = gtk_check_button_new_with_label("Skip if the object already has S/N ratio.");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &skip_flag);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 skip_flag);
  }
  
  gtk_widget_show_all(dialog);

  if(hg->etc_spek==ETC_SPEC_POWERLAW)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[ETC_SPEC_POWERLAW]),TRUE);
  if(hg->etc_spek==ETC_SPEC_BLACKBODY)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[ETC_SPEC_BLACKBODY]),TRUE);
  if(hg->etc_spek==ETC_SPEC_TEMPLATE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[ETC_SPEC_TEMPLATE]),TRUE);

  if(hg->etc_mode!=ETC_MENU){
    if(hg->etc_wave==ETC_WAVE_CENTER)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rc[ETC_WAVE_CENTER]),TRUE);
    if(hg->etc_wave==ETC_WAVE_SPEC)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rc[ETC_WAVE_SPEC]),TRUE);
  }

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    if(hg->etc_mode==ETC_LIST){
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(skip_flag){
	  if(hg->obj[i_list].snr<0)
	    hg->obj[i_list].snr=etc_obj(hg, i_list);
	}
	else{
	  hg->obj[i_list].snr=etc_obj(hg, i_list);
	}
      }
      update_objtree(hg);
    }
    else{
      etc_main(hg);
    }
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void do_etc_list (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  hg->etc_mode=ETC_LIST;
  do_etc(widget,(gpointer)hg);
  hg->etc_mode=ETC_MENU;
}


void do_update_exp_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  dialog = gtk_dialog_new_with_buttons("HOE : Update Exptime using Mag",
				       GTK_WINDOW(hg->w_top),
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
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  frame = gtk_frame_new ("Update Exptime in the list (shot noise limit)");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  table = gtk_table_new (1, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		   GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("Set Mag for each target using \"Database/Magnitude List Query\" at first.");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
		   GTK_FILL,GTK_FILL,0,0);

  label = gtk_label_new ("ExpTime for ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->expmag_mag,
					    4.0, 18.0,
					    0.1, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->expmag_mag);
  
  label = gtk_label_new ("mag = ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->expmag_exp,
					    1, 3600,
					    1.0, 10.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->expmag_exp);
  
  label = gtk_label_new ("[s]");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    do_update_exp(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void do_export_def_list (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check, *table, *frame, *combo, *spinner;
  GtkWidget *fdialog;
  GtkAdjustment *adj;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  hg=(typHOE *)gdata;

  dialog = gtk_dialog_new_with_buttons("HOE : Set Default Guide mode, PA, & Exptime",
				       GTK_WINDOW(hg->w_top),
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
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  frame = gtk_frame_new ("Set Default Parameters to the list");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  
  table = gtk_table_new (1, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
		   GTK_FILL,GTK_FILL,0,0);
  
  // GUIDE_MODE
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No Guide",
		       1, NO_GUIDE, -1);
    if(hg->def_guide==NO_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "AG Guide",
		       1, AG_GUIDE, -1);
    if(hg->def_guide==AG_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV Guide",
		       1, SV_GUIDE, -1);
    if(hg->def_guide==SV_GUIDE) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SV [Safe]",
		       1, SVSAFE_GUIDE, -1);
    if(hg->def_guide==SVSAFE_GUIDE) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->def_guide);
  }



  label = gtk_label_new ("  PA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.0);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_pa,
					    -360.0, 360.0, 0.1, 0.1, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &hg->def_pa);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);
  


  label = gtk_label_new ("  Exptime");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, 0.0);
  gtk_widget_set_valign (label, 0.5);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_exp,
					    1.0, 3600.0, 1.0, 10.0, 0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->def_exp);
  spinner =  gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    export_def(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }
}


void get_font_family_size(typHOE *hg)
{
  PangoFontDescription *fontdc;
      
  fontdc=pango_font_description_from_string(hg->fontname);

  if(hg->fontfamily) g_free(hg->fontfamily);
  hg->fontfamily
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_objsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);

  fontdc=pango_font_description_from_string(hg->fontname_all);

  if(hg->fontfamily_all) g_free(hg->fontfamily_all);
  hg->fontfamily_all
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_allsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);
}

void WriteConf(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_col;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // Version
  xmms_cfg_write_string(cfgfile, "Version", "Major", MAJOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Minor", MINOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Micro", MICRO_VERSION);
  
  // Font
  xmms_cfg_write_string(cfgfile, "Font", "Name", hg->fontname);
  xmms_cfg_write_string(cfgfile, "Font", "All", hg->fontname_all);

  // PC 
  if(hg->www_com) 
    xmms_cfg_write_string(cfgfile, "PC", "Browser", hg->www_com);

  xmms_cfg_write_file(cfgfile, conffile);
  xmms_cfg_free(cfgfile);

  g_free(conffile);
}


void ReadConf(typHOE *hg)
{
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_col;
  gint major_ver,minor_ver,micro_ver;
  gboolean flag_prec;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  
  if (cfgfile) {
    if(xmms_cfg_read_string(cfgfile, "Font", "Name", &c_buf)) 
      hg->fontname =c_buf;
    else
      hg->fontname=g_strdup(SKYMON_FONT);

    if(xmms_cfg_read_string(cfgfile, "Font", "All", &c_buf)) 
      hg->fontname_all=c_buf;
    else
      hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);

    // PC 
    if(xmms_cfg_read_string(cfgfile, "PC", "Browser", &c_buf)) 
      hg->www_com =c_buf;
    else
      hg->www_com=g_strdup(WWW_BROWSER);

    xmms_cfg_free(cfgfile);
  }
  else{
    hg->www_com=g_strdup(WWW_BROWSER);
    hg->fontname=g_strdup(SKYMON_FONT);
    hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);
  }
}


void param_init(typHOE *hg){
  time_t t;
  struct tm *tmpt;
  int i, i_band;

  // Global Args
  flagChildDialog=FALSE;
  flagSkymon=FALSE;
  flagPlot=FALSE;
  flagFC=FALSE;
  flagPlan=FALSE;
  flag_getFCDB=FALSE;
  flag_make_obj_tree=FALSE;
  flag_make_line_tree=FALSE;
  flag_make_etc_tree=FALSE;




  // CCD Binning Mode
  const Binpara binnings[] = {
    {"1x1 [86s] ", 1, 1, 86},
    {"2x1 [60s] ", 2, 1, 60},
    {"2x2 [44s] ", 2, 2, 44},
    {"2x4 [36s] ", 2, 4, 36},
    {"4x1 [44s] ", 4, 1, 44},
    {"4x4 [33s] ", 4, 4, 33}
  };

  hg->i_max=0;
  hg->i_plan_max=0;

  hg->skymon_timer=-1;

  t = time(NULL);
  tmpt = localtime(&t);

  hg->fr_year=tmpt->tm_year+1900;
  hg->fr_month=tmpt->tm_mon+1;
  hg->fr_day=tmpt->tm_mday;
  hg->nights=1;
  
  hg->prop_id=g_strdup("o00000");
  hg->prop_pass=NULL;
  hg->observer=NULL;

#ifdef USE_WIN32
  hg->temp_dir=get_win_temp();
  hg->home_dir=get_win_home();
#else
  hg->temp_dir=g_strdup("/tmp");
  hg->home_dir=g_strdup(g_get_home_dir());
#endif

  ReadConf(hg);

  hg->ocs=OCS_GEN2;
  hg->wave1=WAVE1_SUBARU;
  hg->wave0=WAVE0_SUBARU;
  hg->temp=TEMP_SUBARU;
  hg->pres=PRES_SUBARU;
  hg->dss_scale=FC_SCALE_LINEAR;

  {
    gint i_bin;

    for(i_bin=0;i_bin<MAX_BINNING;i_bin++){
      hg->binning[i_bin]=binnings[i_bin];
    }
  }

  hg->camz_b=CAMZ_B;
  hg->camz_r=CAMZ_R;
  hg->camz_date=g_strdup("(Not synced yet)");

  hg->d_cross=D_CROSS;
  hg->wcent=5500;

  {
    gint i_use;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      hg->setup[i_use].setup=0;
      hg->setup[i_use].use=FALSE;
      hg->setup[i_use].binning=0;
      hg->setup[i_use].slit_width=200;
      hg->setup[i_use].slit_length=2200;
      hg->setup[i_use].imr=IMR_NO;
      hg->setup[i_use].is=IS_NO;
      hg->setup[i_use].i2=FALSE;
    }
    hg->setup[0].use=TRUE;
  }

  hg->exptime_factor=1;
  hg->brightness=2000;
  hg->sv_area=SV_PART;
  hg->sv_integrate=1;
  hg->sv_acq=TIME_ACQ;
  hg->sv_region=200;
  hg->sv_calc=DEF_SV_CALC;
  hg->exptime_sv=DEF_SV_EXP;
  hg->sv_slitx=DEF_SV_SLITX;
  hg->sv_slity=DEF_SV_SLITY;
  hg->sv_isx=DEF_SV_ISX;
  hg->sv_isy=DEF_SV_ISY;
  hg->sv_is3x=DEF_SV_IS3X;
  hg->sv_is3y=DEF_SV_IS3Y;

  hg->def_exp=DEF_EXP;
  hg->def_guide=SV_GUIDE;
  hg->def_pa=0;

  for(i=0;i<MAX_NONSTD;i++){
    hg->nonstd[i].col=COL_RED;
    hg->nonstd[i].cross=15730;
    hg->nonstd[i].echelle=DEF_ECHELLE;
    hg->nonstd[i].camr=-3600;
  }

  for(i=0;i<MAX_LINE;i++){
    hg->line[i].name=NULL;
    hg->line[i].wave=0.;
  }

  for(i=0;i<MAX_OBJECT;i++){
    hg->obj[i].name=NULL;
    hg->obj[i].i_nst=-1;

    hg->obj[i].trdb_str=NULL;
    hg->obj[i].trdb_band_max=0;
    for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
      hg->obj[i].trdb_mode[i_band]=NULL;
      hg->obj[i].trdb_band[i_band]=NULL;
      hg->obj[i].trdb_exp[i_band]=0;
      hg->obj[i].trdb_shot[i_band]=0;
    }

    hg->obj[i].gs.flag=FALSE;
    hg->obj[i].gs.name=NULL;

    hg->obj[i].magdb_simbad_name=NULL;
    hg->obj[i].magdb_simbad_type=NULL;
    hg->obj[i].magdb_simbad_sp=NULL;

    hg->obj[i].magdb_ned_name=NULL;
    hg->obj[i].magdb_ned_type=NULL;
    hg->obj[i].magdb_ned_mag=NULL;

    hg->obj[i].magdb_lamost_name=NULL;
    hg->obj[i].magdb_lamost_type=NULL;
    hg->obj[i].magdb_lamost_sp=NULL;
  }

  hg->trdb_i_max=0;
  hg->trdb_disp_flag=TRUE;
  hg->trdb_smoka_inst=0;
  hg->trdb_smoka_date=g_strdup_printf("1998-01-01..%d-%02d-%02d",
				      hg->fr_year,
				      hg->fr_month,
				      hg->fr_day);
  hg->trdb_arcmin=2;
  hg->trdb_used=MAGDB_TYPE_SIMBAD;
  hg->trdb_da=TRDB_TYPE_SMOKA;
  make_trdb_label(hg);
  hg->trdb_smoka_shot  = TRUE;
  hg->trdb_smoka_shot_used  = TRUE;
  hg->trdb_smoka_imag  = TRUE;
  hg->trdb_smoka_imag_used  = TRUE;
  hg->trdb_smoka_spec  = TRUE;
  hg->trdb_smoka_spec_used  = TRUE;
  hg->trdb_smoka_ipol  = TRUE;
  hg->trdb_smoka_ipol_used  = TRUE;
  hg->trdb_hst_mode  = TRDB_HST_MODE_IMAGE;
  hg->trdb_hst_date=g_strdup_printf("1990-01-01..%d-%02d-%02d",
				    hg->fr_year,
				    hg->fr_month,
				    hg->fr_day);
  hg->trdb_eso_mode  = TRDB_ESO_MODE_IMAGE;
  hg->trdb_eso_stdate=g_strdup("1980 01 01");
  hg->trdb_eso_eddate=g_strdup_printf("%4d %02d %02d",
				      hg->fr_year,
				      hg->fr_month,
				      hg->fr_day);

  hg->trdb_gemini_inst  = GEMINI_INST_GMOS;
  hg->trdb_gemini_mode  = TRDB_GEMINI_MODE_ANY;
  hg->trdb_gemini_date=g_strdup_printf("19980101-%d%02d%02d",
				       hg->fr_year,
				       hg->fr_month,
				       hg->fr_day);

  hg->efs_ps=g_strdup(PS_FILE);

  hg->etc_mode=ETC_MENU;
  hg->etc_filter=BAND_V;
  hg->etc_mag=17.0;
  hg->etc_z=0.0;
  hg->etc_spek=ETC_SPEC_POWERLAW;
  hg->etc_alpha=0.0;
  hg->etc_bbtemp=10000;
  hg->etc_sptype=ST_O5V;
  hg->etc_adc=ETC_ADC_IN;
  hg->etc_imr=ETC_IMR_NO;
  hg->etc_exptime=3600;
  hg->etc_seeing=0.5;
  hg->etc_label_text=g_strdup("Exposure Time Calculator");
  hg->etc_prof_text=g_strdup("Your results on ETC");
  hg->etc_wave=ETC_WAVE_CENTER;
  hg->etc_waved=5500;

  hg->filename_hoe=NULL;
  hg->filename_log=NULL;

  hg->azel_mode=AZEL_NORMAL;

  //hg->flag_bunnei=FALSE;
  hg->expmag_mag=8.0;
  hg->expmag_exp=100;

  hg->skymon_mode=SKYMON_SET;
  hg->skymon_objsz=SKYMON_DEF_OBJSZ;

  hg->dss_arcmin          =DSS_ARCMIN;
  hg->dss_pix             =DSS_PIX;

  hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
  hg->dss_path             =g_strdup(FC_PATH_SKYVIEW);
  hg->dss_src              =g_strdup(FC_SRC_SKYVIEW_DSS2R);
  hg->dss_tmp=g_strconcat(hg->temp_dir,
			  G_DIR_SEPARATOR_S,
			  FC_FILE_HTML,NULL);
  hg->dss_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FC_FILE_JPEG,NULL);
  hg->fc_mode              =FC_SKYVIEW_DSS2R;
  hg->dss_pa=0;
  hg->dss_flip=FALSE;
  hg->dss_draw_slit=TRUE;
  hg->sdss_photo=FALSE;
  hg->sdss_spec=FALSE;
  hg->fc_inst=FC_INST_HDS;

  hg->plan_tmp_or=FALSE;
  hg->plan_tmp_sw=200;
  hg->plan_tmp_sl=2000;
  hg->plan_tmp_setup=0;

  hg->plan_obj_i=0;
  hg->plan_obj_exp=DEF_EXP;
  hg->plan_obj_repeat=1;
  hg->plan_obj_omode=PLAN_OMODE_FULL;
  hg->plan_obj_guide=SV_GUIDE;

  hg->plan_bias_repeat=5;

  hg->plan_flat_repeat=10;

  hg->plan_focus_mode=PLAN_FOCUS_SV;

  hg->plan_setup_cmode=PLAN_CMODE_FULL;

  hg->plan_i2_pos=PLAN_I2_IN;

  hg->plan_setaz=-90.;
  hg->plan_setel=90.;

  hg->plan_start=PLAN_START_EVENING;
  hg->plan_start_hour=24;
  hg->plan_start_min=20;

  hg->plan_delay=SUNSET_OFFSET;

  hg->plan_comment=NULL;

  hg->plot_all=PLOT_ALL_SINGLE;
  hg->plot_moon=FALSE;

  hg->plot_output=PLOT_OUTPUT_WINDOW;
  hg->skymon_output=SKYMON_OUTPUT_WINDOW;
  hg->efs_output=EFS_OUTPUT_WINDOW;

  hg->hsc_dithi=1;
  hg->hsc_dithp=HSC_DITH_NO;
  hg->hsc_dra=HSC_DRA;
  hg->hsc_ddec=HSC_DDEC;
  hg->hsc_tdith=HSC_TDITH;
  hg->hsc_rdith=HSC_RDITH;
  hg->hsc_ndith=5;
  hg->hsc_offra=0;
  hg->hsc_offdec=0;

  hg->std_i_max=0;
  hg->std_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   STDDB_FILE_XML,NULL);
  hg->stddb_mode=STDDB_IRAFSTD;

  hg->std_dra   =STD_DRA;
  hg->std_ddec  =STD_DDEC;
  hg->std_vsini =STD_VSINI;
  hg->std_vmag  =STD_VMAG;
  hg->std_sptype=g_strdup(STD_SPTYPE);
  hg->std_iras12=STD_IRAS12;
  hg->std_iras25=STD_IRAS25;
  hg->std_cat   =g_strdup(STD_CAT);
  hg->std_mag1  =STD_MAG1;
  hg->std_mag2  =STD_MAG2;
  hg->std_band  =g_strdup(STD_BAND);
  hg->std_sptype2  =g_strdup(STD_SPTYPE_ALL);

  hg->fcdb_i_max=0;
  hg->fcdb_simbad=FCDB_SIMBAD_STRASBG;
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);
  hg->fcdb_label_text=g_strdup("Object in Finding Chart");
  hg->fcdb_band=FCDB_BAND_NOP;
  hg->fcdb_mag=15;
  hg->fcdb_otype=FCDB_OTYPE_ALL;
  hg->fcdb_ned_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ned_otype=FCDB_NED_OTYPE_ALL;
  hg->fcdb_auto=FALSE;
  hg->fcdb_ned_ref=FALSE;
  hg->fcdb_gsc_fil=TRUE;
  hg->fcdb_gsc_mag=19;
  hg->fcdb_gsc_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ps1_fil=TRUE;
  hg->fcdb_ps1_mag=19;
  hg->fcdb_ps1_diam=FCDB_PS1_ARCMIN_MAX;
  hg->fcdb_ps1_mindet=2;
  hg->fcdb_sdss_search = FCDB_SDSS_SEARCH_IMAG;
  for(i=0;i<NUM_SDSS_BAND;i++){
    hg->fcdb_sdss_fil[i]=TRUE;
    hg->fcdb_sdss_magmin[i]=0;
    hg->fcdb_sdss_magmax[i]=20;
  }
  hg->fcdb_sdss_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_usno_fil=TRUE;
  hg->fcdb_usno_mag=19;
  hg->fcdb_usno_diam=FCDB_USNO_ARCMIN_MAX;
  hg->fcdb_gaia_fil=TRUE;
  hg->fcdb_gaia_mag=19;
  hg->fcdb_gaia_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_2mass_fil=TRUE;
  hg->fcdb_2mass_mag=12;
  hg->fcdb_2mass_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_wise_fil=TRUE;
  hg->fcdb_wise_mag=15;
  hg->fcdb_wise_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_smoka_shot  = FALSE;
  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    hg->fcdb_smoka_subaru[i]  = TRUE;
  }
  for(i=0;i<NUM_SMOKA_KISO;i++){
    hg->fcdb_smoka_kiso[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_OAO;i++){
    hg->fcdb_smoka_oao[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_MTM;i++){
    hg->fcdb_smoka_mtm[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_KANATA;i++){
    hg->fcdb_smoka_kanata[i]  = FALSE;
  }
  for(i=0;i<NUM_HST_IMAGE;i++){
    hg->fcdb_hst_image[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_SPEC;i++){
    hg->fcdb_hst_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_OTHER;i++){
    hg->fcdb_hst_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_IMAGE;i++){
    hg->fcdb_eso_image[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SPEC;i++){
    hg->fcdb_eso_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_VLTI;i++){
    hg->fcdb_eso_vlti[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_POLA;i++){
    hg->fcdb_eso_pola[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_CORO;i++){
    hg->fcdb_eso_coro[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_OTHER;i++){
    hg->fcdb_eso_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SAM;i++){
    hg->fcdb_eso_sam[i]  = TRUE;
  }
  hg->fcdb_gemini_inst = GEMINI_INST_ANY;

  hg->sz_skymon=SKYMON_WINSIZE;
  hg->sz_plot  =  PLOT_WINSIZE;
  hg->sz_fc    =    FC_WINSIZE;

  hg->obs_timezone = TIMEZONE_SUBARU;
  skymon_set_time_current(hg);

  hg->orbit_flag=TRUE;
  hg->fcdb_flag=TRUE;

  hg->magdb_arcsec=10;
  hg->magdb_mag=18;
  hg->magdb_ow=FALSE;
  hg->magdb_skip=TRUE;
  hg->magdb_gsc_band=GSC_BAND_V;
  hg->magdb_ps1_band=PS1_BAND_G;
  hg->magdb_sdss_band=SDSS_BAND_G;
  hg->magdb_2mass_band=TWOMASS_BAND_J;
  hg->magdb_simbad_band=FCDB_BAND_NOP;

  calc_moon(hg);
  calc_sun_plan(hg);

}



gchar *cut_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name, *c;
  gint  i_bak,i;

  tgt_name=g_strdup(obj_name);
  i_bak=strlen(tgt_name)-1;
  while((tgt_name[i_bak]==0x20)
	||(tgt_name[i_bak]==0x0A)
	||(tgt_name[i_bak]==0x0D)
	||(tgt_name[i_bak]==0x09)){
    tgt_name[i_bak]='\0';
    i_bak--;
  }
    
  c=tgt_name;
  i=0;
  while((tgt_name[i]==0x20)||(tgt_name[i]==0x09)){
    c++;
    i++;
  }

  ret_name=g_strdup(c);
  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}


gchar *make_tgt(gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj,i_tgt;

  strcpy(tgt_name,"TGT_");
  i_tgt=strlen(tgt_name);

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(isalnum(obj_name[i_obj])){
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  //tgt_name[i_tgt]=(char)NULL;
  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  return(ret_name);
}

gdouble read_radec(gchar* p){
  gchar* tmp_p;
  gdouble d_out;

  if((tmp_p=strtok(p,":"))!=NULL){
    d_out=1.0e4*(gdouble)g_strtod(tmp_p,NULL);
    if((tmp_p=strtok(NULL,":"))!=NULL){
      d_out+=1.0e2*(gdouble)g_strtod(tmp_p,NULL);
      if((tmp_p=strtok(NULL,":"))!=NULL){
	d_out+=(gdouble)g_strtod(tmp_p,NULL);
      }
    }

    return(d_out);
  }
  else{
    return((gdouble)g_strtod(p,NULL));
  }
}


void ChangeFontButton(GtkWidget *w, gpointer gdata)
{ 
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  if(hg->fontname) g_free(hg->fontname);
  hg->fontname
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));

  get_font_family_size(hg);
}

void ChangeFontButton_all(GtkWidget *w, gpointer gdata)
{ 
  typHOE *hg;
  hg=(typHOE *)gdata;
  
  if(hg->fontname_all) g_free(hg->fontname_all);
  hg->fontname_all
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));

  get_font_family_size(hg);
}

void ObjMagDB_Init(OBJpara* obj){
  obj->magdb_used=0;
  obj->magdb_band=0;
  
  obj->magdb_gsc_hits=-1;
  obj->magdb_ps1_hits=-1;
  obj->magdb_sdss_hits=-1;
  obj->magdb_gaia_hits=-1;
  obj->magdb_2mass_hits=-1;
  obj->magdb_simbad_hits=-1;
  obj->magdb_ned_hits=-1;
  obj->magdb_lamost_hits=-1;

  obj->magdb_gsc_sep=-1;
  obj->magdb_ps1_sep=-1;
  obj->magdb_sdss_sep=-1;
  obj->magdb_gaia_sep=-1;
  obj->magdb_2mass_sep=-1;
  obj->magdb_simbad_sep=-1;
  obj->magdb_ned_sep=-1;
  obj->magdb_lamost_sep=-1;

  obj->magdb_gsc_u=100;
  obj->magdb_gsc_b=100;
  obj->magdb_gsc_v=100;
  obj->magdb_gsc_r=100;
  obj->magdb_gsc_i=100;
  obj->magdb_gsc_j=100;
  obj->magdb_gsc_h=100;
  obj->magdb_gsc_k=100;

  obj->magdb_ps1_g=100;
  obj->magdb_ps1_r=100;
  obj->magdb_ps1_i=100;
  obj->magdb_ps1_z=100;
  obj->magdb_ps1_y=100;

  obj->magdb_sdss_u=100;
  obj->magdb_sdss_g=100;
  obj->magdb_sdss_r=100;
  obj->magdb_sdss_i=100;
  obj->magdb_sdss_z=100;

  obj->magdb_gaia_g=100;
  obj->magdb_gaia_p=-1;

  obj->magdb_2mass_j=100;
  obj->magdb_2mass_h=100;
  obj->magdb_2mass_k=100;

  obj->magdb_simbad_u=100;
  obj->magdb_simbad_b=100;
  obj->magdb_simbad_v=100;
  obj->magdb_simbad_r=100;
  obj->magdb_simbad_i=100;
  obj->magdb_simbad_j=100;
  obj->magdb_simbad_h=100;
  obj->magdb_simbad_k=100;
  obj->magdb_simbad_name=NULL;
  obj->magdb_simbad_type=NULL;
  obj->magdb_simbad_sp=NULL;

  obj->magdb_ned_name=NULL;
  obj->magdb_ned_type=NULL;
  obj->magdb_ned_mag=NULL;
  obj->magdb_ned_z=-100;
  obj->magdb_ned_ref=0;

  obj->magdb_lamost_name=NULL;
  obj->magdb_lamost_type=NULL;
  obj->magdb_lamost_sp=NULL;
  obj->magdb_lamost_ref=0;
  obj->magdb_lamost_teff=-1;
  obj->magdb_lamost_logg=-10;
  obj->magdb_lamost_feh=+100;
  obj->magdb_lamost_hrv=-99999;
}

void ReadList(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  
  //hg->flag_bunnei=FALSE;

  if((fp=fopen(hg->filename_read,"rb"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(strlen(buf)<10) break;
      tmp_char=(char *)strtok(buf,",");
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(tmp_char);
      hg->obj[i_list].name=cut_spc(tmp_char);

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      //hg->obj[i_list].ra=read_radec(tmp_char);

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      //hg->obj[i_list].dec=read_radec(tmp_char);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      
      if((tmp_char=(char *)strtok(NULL,"\n"))!=NULL){
	hg->obj[i_list].note=g_strdup(tmp_char);
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }

      hg->obj[i_list].exp=DEF_EXP;
      hg->obj[i_list].mag=100;
      hg->obj[i_list].snr=-1;
      hg->obj[i_list].sat=FALSE;
      ObjMagDB_Init(&hg->obj[i_list]);
      hg->obj[i_list].repeat=1;
      hg->obj[i_list].guide=SV_GUIDE;
      hg->obj[i_list].pa=0;
      hg->obj[i_list].i_nst=-1;
      hg->obj[i_list].gs.flag=FALSE;
      if(hg->obj[i_list].gs.name){
	g_free(hg->obj[i_list].gs.name);
	hg->obj[i_list].gs.name=NULL;
      }
      
      hg->obj[i_list].setup[0]=TRUE;
      for(i_use=1;i_use<MAX_USESETUP;i_use++){
	hg->obj[i_list].setup[i_use]=FALSE;
      }
      
      i_list++;
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  hg->i_max=i_list;
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}

void ReadList2(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  
  if((fp=fopen(hg->filename_read,"rb"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  //hg->flag_bunnei=TRUE;

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(strlen(buf)<10) break;
      tmp_char=(char *)strtok(buf,",");
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(tmp_char);
      hg->obj[i_list].name=cut_spc(tmp_char);

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(hg->w_top, tmp_char,i_list+1,"Magnitude")) break;
      hg->obj[i_list].mag=(gdouble)g_strtod(tmp_char,NULL);
      
      if((tmp_char=(char *)strtok(NULL,"\n"))!=NULL){
	hg->obj[i_list].note=g_strdup(tmp_char);
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }

      hg->obj[i_list].exp=DEF_EXP;
      hg->obj[i_list].snr=-1;
      hg->obj[i_list].sat=FALSE;
      ObjMagDB_Init(&hg->obj[i_list]);
      hg->obj[i_list].repeat=1;
      hg->obj[i_list].guide=SV_GUIDE;
      hg->obj[i_list].pa=0;
      hg->obj[i_list].i_nst=-1;
      hg->obj[i_list].gs.flag=FALSE;
      if(hg->obj[i_list].gs.name){
	g_free(hg->obj[i_list].gs.name);
	hg->obj[i_list].gs.name=NULL;
      }
      
      hg->obj[i_list].setup[0]=TRUE;
      for(i_use=1;i_use<MAX_USESETUP;i_use++){
	hg->obj[i_list].setup[i_use]=FALSE;
      }
      
      i_list++;
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  hg->i_max=i_list;
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}

#ifdef USE_SSL
void UploadOPE(typHOE *hg){
  gint ans=0;

  if((ans=scp_write(hg))<0){
    switch(ans){
    case -1:
      break;
    
    case -2:
      break;
    
    case -3:
      break;

    default:
      break;
    }
  }
} 

void DownloadLOG(typHOE *hg){
  gint ans=0;

  if((ans=scp_get(hg))<0){
    switch(ans){
    case -1:
      break;
    
    case -2:
      break;
    
    case -3:
      break;

    default:
      break;
    }
  }
}  
#endif

void ReadListOPE(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  gchar *BUF=NULL,*buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gboolean new_fmt_flag=FALSE;
  
  //hg->flag_bunnei=FALSE;

  if((fp=fopen(hg->filename_read,"rb"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"<PARAMETER_LIST>",
			     strlen("<PARAMETER_LIST>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":PARAMETER",
			     strlen(":PARAMETER"))==0){
	escape=TRUE;
	new_fmt_flag=TRUE;
      }
      g_free(buf);
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  
  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
				 strlen("</PARAMETER_LIST>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else{
	if((buf[0]!='#')){
	  if(BUF) g_free(BUF);
	  BUF=g_ascii_strup(buf,-1);
	  ok_obj=FALSE;
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	  ok_equinox=FALSE;
	
	  // OBJECT
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "OBJECT="))){
	      cpp=cp+strlen("OBJECT=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		cp++;
		ok_obj=TRUE;
		cp+=strlen("OBJECT=");
		if(cp[0]=='\"'){
		  cp+=1;
		  cp2 = strstr(cp, "\"");
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		}
		else{
		  //if(cp3) g_free(cp3);
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  if(NULL != (cp2 = strstr(cp, " ")))
		      hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else hg->obj[i_list].name=g_strdup(cp);
		}
		break;
	      }
	    }
	  }while(cp);

	    
	  // RA
	  if(ok_obj){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "RA="))){
		cpp=cp+strlen("RA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_ra=TRUE;
		  cp+=strlen("RA=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  hg->obj[i_list].ra=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  // DEC
	  if(ok_obj&&ok_ra){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "DEC="))){
		cpp=cp+strlen("DEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_dec=TRUE;
		  cp+=strlen("DEC=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  hg->obj[i_list].dec=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }

	  // EQUINOX
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "EQUINOX="))){
		cpp=cp+strlen("EQUINOX=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) ){
		  cp++;
		  ok_equinox=TRUE;
		  cp+=strlen("EQUINOX=");
		  if(cp3) g_free(cp3);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    cp3=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else cp3=g_strdup(cp);
		  hg->obj[i_list].equinox=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	
	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    hg->obj[i_list].note=NULL;
	  
	    hg->obj[i_list].exp=DEF_EXP;
	    hg->obj[i_list].mag=100;
	    hg->obj[i_list].snr=-1;
	    hg->obj[i_list].sat=FALSE;
	    ObjMagDB_Init(&hg->obj[i_list]);
	    hg->obj[i_list].repeat=1;
	    hg->obj[i_list].guide=SV_GUIDE;
	    hg->obj[i_list].pa=0;
	    hg->obj[i_list].i_nst=-1;
	    hg->obj[i_list].gs.flag=FALSE;
	    if(hg->obj[i_list].gs.name){
	      g_free(hg->obj[i_list].gs.name);
	      hg->obj[i_list].gs.name=NULL;
	    }
	  
	    hg->obj[i_list].setup[0]=TRUE;
	    for(i_use=1;i_use<MAX_USESETUP;i_use++){
	      hg->obj[i_list].setup[i_use]=FALSE;
	    }
	    i_list++;
	  }
	}
      }
      if(buf) g_free(buf);
    }

    if(escape) break;
  }


  fclose(fp);

  hg->i_max=i_list;
  fcdb_clear_tree(hg,TRUE);
  trdb_clear_tree(hg);

  calc_rst(hg);
}


void MergeList(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use, i_base;
  gchar *tmp_char;
  gchar *buf=NULL;
  OBJpara tmp_obj;
  gboolean name_flag;
  
  if((fp=fopen(hg->filename_read,"r"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_read);
    exit(1);
  }

  i_base=hg->i_max;

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(strlen(buf)<10) break;
      
      tmp_char=(char *)strtok(buf,",");
      tmp_obj.name=g_strdup(tmp_char);
      tmp_obj.name=cut_spc(tmp_char);
      
      name_flag=FALSE;
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(strcmp(tmp_obj.name,hg->obj[i_list].name)==0){
	  name_flag=TRUE;
	  break;
	}
      }

      if(!name_flag){
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"RA")) break;
	tmp_obj.ra=(gdouble)g_strtod(tmp_char,NULL);
	
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"Dec")) break;
	tmp_obj.dec=(gdouble)g_strtod(tmp_char,NULL);
      
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(hg->w_top, tmp_char,hg->i_max-i_base+1,"Equinox")) break;
	tmp_obj.equinox=(gdouble)g_strtod(tmp_char,NULL);
	
	if((tmp_char=(char *)strtok(NULL,"\n"))!=NULL){
	  tmp_obj.note=g_strdup(tmp_char);
	  tmp_obj.note=cut_spc(tmp_char);
	}
	else{
	  tmp_obj.note=NULL;
	}
	
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
	
	tmp_obj.setup[0]=TRUE;
	for(i_use=1;i_use<MAX_USESETUP;i_use++){
	  tmp_obj.setup[i_use]=FALSE;
	}
	
	hg->obj[hg->i_max]=tmp_obj;
	hg->i_max++;
      }
    }
    if(buf) g_free(buf);
  }

  fclose(fp);

  calc_rst(hg);
}


gboolean MergeNST(typHOE *hg){
  FILE *fp;
  gint i,i_list=0,i_base,i_use;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gdouble date_tmp, ra_geoc, dec_geoc;
  gchar *cp, *cpp, *tmp_name, *cut_name;
  struct ln_zonedate zonedate, zonedate1;
  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return(FALSE);
  }
  

  if((fp=fopen(hg->filename_nst,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_nst,
		  NULL);
    return(FALSE);
  }

  i_list=hg->i_max;
  if(hg->i_max==0){
    hg->nst_max=0;
  }

  for(i=0;i<6;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: TSC File format might be incorrect.",
		    " ",
		    hg->filename_nst,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else{
      if(i==0){
	cpp=buf;
	cpp++;
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      if(i<5){
	if(buf) g_free(buf);
      }
    }
  }
  hg->nst[hg->nst_max].i_max=(gint)g_strtod(buf,NULL);
  if(buf) g_free(buf);
  if(hg->nst[hg->nst_max].i_max<=0){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: TSC File format might be incorrect.",
		  " ",
		  hg->filename_nst,
		  NULL);
    fclose(fp);
    return(FALSE);
  }

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);
  
  i=0;
  while((!feof(fp))||(i<hg->nst_max)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      sscanf(buf,"%lf %lf %lf %lf %lf",
	     &date_tmp,
	     &ra_geoc,
	     &dec_geoc,
	     &hg->nst[hg->nst_max].eph[i].geo_d,
	     &hg->nst[hg->nst_max].eph[i].equinox);
      
      hg->nst[hg->nst_max].eph[i].jd=date_to_jd(date_tmp);
      // GeoCentric --> TopoCentric
      equ_geoc.ra=ra_to_deg(ra_geoc);
      equ_geoc.dec=dec_to_deg(dec_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i].jd,
			hg->nst[hg->nst_max].eph[i].geo_d,&equ_geoc,&equ);
      hg->nst[hg->nst_max].eph[i].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i].dec=deg_to_dec(equ.dec);
      i++;
      if(buf) g_free(buf);
    }
  }
  
  if(i!=hg->nst[hg->nst_max].i_max){
    fprintf(stderr,"[MergeNST] Inconsistent Line Number in  \"%s\", %d <--> %d.", hg->filename_nst,hg->nst[hg->nst_max].i_max,i);
  }

  fclose(fp);

  if(i>0){
    ln_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		      hg->obs_timezone/60);
    ln_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		      &zonedate1, 
		      hg->obs_timezone/60);
    if(tmp_name){
      cut_name=cut_spc(tmp_name);
      g_free(tmp_name);
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(cut_name);
      g_free(cut_name);
    }
    else{
      hg->obj[i_list].name=g_strdup("(None-Sidereal)");
    }
    hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
    hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
    hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
    if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
    hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
					 g_path_get_basename(hg->filename_nst),
					 zonedate.years,
					 zonedate.months,
					 zonedate.days,
					 zonedate.hours,
					 zonedate.minutes,
					 zonedate1.months,
					 zonedate1.days,
					 zonedate1.hours,
					 zonedate1.minutes,
					 "HST");
    if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
    hg->nst[hg->nst_max].filename=g_strdup(hg->filename_nst);
    hg->nst[hg->nst_max].type=NST_TYPE_TSC;

    hg->obj[i_list].check_sm=FALSE;
    hg->obj[i_list].i_nst=hg->nst_max;
    hg->obj[i_list].exp=DEF_EXP;
    hg->obj[i_list].mag=100;
    hg->obj[i_list].snr=-1;
    hg->obj[i_list].sat=FALSE;
    ObjMagDB_Init(&hg->obj[i_list]);
    hg->obj[i_list].repeat=1;
    hg->obj[i_list].guide=NO_GUIDE;
    hg->obj[i_list].pa=0;
    hg->obj[i_list].gs.flag=FALSE;
    if(hg->obj[i_list].gs.name){
      g_free(hg->obj[i_list].gs.name);
      hg->obj[i_list].gs.name=NULL;
    }

    hg->obj[i_list].setup[0]=TRUE;
    for(i_use=1;i_use<MAX_USESETUP;i_use++){
      hg->obj[i_list].setup[i_use]=FALSE;
    }

    hg->i_max++;
    hg->nst_max++;
  }

  calc_rst(hg);

  return(TRUE);
}


gboolean MergeJPL(typHOE *hg){
  FILE *fp;
  gint i,i_list, i_line, i_soe=0, i_eoe=0, i_use;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_zonedate zonedate, zonedate1;
  gchar *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;

  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return(FALSE);
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }

  i_list=hg->i_max;
  if(hg->i_max==0){
    hg->nst_max=0;
  }

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->w_top, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT*2,
			"Error: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  return(FALSE);
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }


  if(i_soe>=i_eoe){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return(FALSE);
  }

  hg->nst[hg->nst_max].i_max=i_eoe-i_soe-1;

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = strstr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
  }

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      zonedate.gmtoff=0;

      if(strlen(tmp1)!=4){
	// JD
	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=(gdouble)g_strtod(tmp1, NULL);
      }
      else{
	zonedate.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	zonedate.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	zonedate.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  zonedate.seconds=0.0;
	}
	else{
	  zonedate.seconds=(gdouble)g_strtod(tmp1, NULL);
	}

	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=
	  ln_get_julian_local_date(&zonedate);
      }
      g_free(tmp);


      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      ln_hequ_to_equ (&hequ, &equ_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i-i_soe-1].jd,
			hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d,
			&equ_geoc,
			&equ);
      hg->nst[hg->nst_max].eph[i-i_soe-1].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i-i_soe-1].dec=deg_to_dec(equ.dec);
      hg->nst[hg->nst_max].eph[i-i_soe-1].equinox=2000.0;

      if(buf) g_free(buf);
    }
  }
  
  fclose(fp);

  ln_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		    hg->obs_timezone/60);
  ln_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		    &zonedate1, 
		    hg->obs_timezone/60);
  
  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
    hg->obj[i_list].name=g_strdup(cut_name);
    g_free(cut_name);
  }
  else{
    hg->obj[i_list].name=g_strdup("(None-Sidereal)");
  }
  hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
  hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
  hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
  hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
				       g_path_get_basename(hg->filename_jpl),
				       zonedate.years,
				       zonedate.months,
				       zonedate.days,
				       zonedate.hours,
				       zonedate.minutes,
				       zonedate1.months,
				       zonedate1.days,
				       zonedate1.hours,
				       zonedate1.minutes,
				       "HST");

  if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
  hg->nst[hg->nst_max].filename=g_strdup(hg->filename_jpl);
  hg->nst[hg->nst_max].type=NST_TYPE_JPL;

  hg->obj[i_list].check_sm=FALSE;
  hg->obj[i_list].i_nst=hg->nst_max;
  hg->obj[i_list].exp=DEF_EXP;
  hg->obj[i_list].mag=100;
  hg->obj[i_list].snr=-1;
  hg->obj[i_list].sat=FALSE;
  ObjMagDB_Init(&hg->obj[i_list]);
  hg->obj[i_list].repeat=1;
  hg->obj[i_list].guide=NO_GUIDE;
  hg->obj[i_list].pa=0;
  hg->obj[i_list].gs.flag=FALSE;
  if(hg->obj[i_list].gs.name){
    g_free(hg->obj[i_list].gs.name);
    hg->obj[i_list].gs.name=NULL;
  }

  hg->obj[i_list].setup[0]=TRUE;
  for(i_use=1;i_use<MAX_USESETUP;i_use++){
    hg->obj[i_list].setup[i_use]=FALSE;
  }

  hg->i_max++;
  hg->nst_max++;

  calc_rst(hg);

  return(TRUE);
}


void ConvJPL(typHOE *hg){
  FILE *fp, *fp_w;
  gint i,i_list, i_line, i_soe=0, i_eoe=0, i_max;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_date date;
  char *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gdouble JD, geo_d;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;
  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Warning: Object Number exceeds the limit.",
		  NULL);
    return;
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  if((fp_w=fopen(hg->filename_tscconv,"wb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->w_top, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT*2,
			"Error: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  return;
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }


  if(i_soe>=i_eoe){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  i_max=i_eoe-i_soe-1;

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = strstr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = strstr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
  }

  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    fprintf(fp_w,"#%s\n",cut_name);
    g_free(cut_name);
  }
  else{
    fprintf(fp_w,"#(Non-Sidereal File converted from JPL HORIZONS\n");
  }

  fprintf(fp_w,"+00.0000 +00.0000 ON%% +0.000\n");
  fprintf(fp_w,"UTC Geocentric Equatorial Mean Polar Geocentric\n");
  fprintf(fp_w,"ABS\n");
  fprintf(fp_w,"TSC\n");
  fprintf(fp_w,"%d\n",i_max);

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      if(strlen(tmp1)!=4){
	// JD
	JD=(gdouble)g_strtod(tmp1, NULL);
	ln_get_date(JD,&date);
      }
      else{
	date.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	date.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	date.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  date.seconds=0.0;
	}
	else{
	  date.seconds=(gdouble)g_strtod(tmp1, NULL);
	}
      }
      g_free(tmp);

      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      fprintf(fp_w,"%4d%02d%02d%02d%02d%06.3lf %02d%02d%06.3lf %s%02d%02d%05.2lf %13.9lf 2000.0000\n",
	      date.years,
	      date.months,
	      date.days,
	      date.hours,
	      date.minutes,
	      date.seconds,
	      hequ.ra.hours,
	      hequ.ra.minutes,
	      hequ.ra.seconds,
	      (hequ.dec.neg == 1) ? "-" : "+",
	      hequ.dec.degrees,
	      hequ.dec.minutes,
	      hequ.dec.seconds,
	      geo_d);

      if(buf) g_free(buf);
    }

  }
  
  fclose(fp);
  fclose(fp_w);
}

gboolean CheckDefDup(typHOE *hg){
  gint i_list,j_list;
  gchar *tgt=NULL, *tgt1=NULL, *tmp=NULL;
  gboolean ret=FALSE;
  gdouble d_ra, d_dec, d_ra1, d_dec1;

  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name);
    for(j_list=0;j_list<i_list;j_list++){
      tgt1=make_tgt(hg->obj[j_list].name);
      if(strcmp(tgt,tgt1)==0){
	d_ra=ra_to_deg(hg->obj[i_list].ra);
	d_dec=dec_to_deg(hg->obj[i_list].dec);
	d_ra1=ra_to_deg(hg->obj[j_list].ra);
	d_dec1=dec_to_deg(hg->obj[j_list].dec);
	if(deg_sep(d_ra,d_dec,d_ra1,d_dec1)>10.0){ // larget than 10 arcsec
	  ret=TRUE;
	}
      }
      if(tgt1) g_free(tgt1);
      if(ret) break;
    }
    if(tgt) g_free(tgt);
    if(ret) break;
  }

  if(ret){
    tgt =g_strdup_printf("%d : \"%s\"  RA=%09.2lf Dec=%+010.2lf",
      i_list+1,hg->obj[i_list].name,hg->obj[i_list].ra,hg->obj[i_list].dec);
    tgt1=g_strdup_printf("%d : \"%s\"  RA=%09.2lf Dec=%+010.2lf",
      j_list+1,hg->obj[j_list].name,hg->obj[j_list].ra,hg->obj[j_list].dec);
    popup_message(hg->w_top,
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: found a duplicate target name with different coordinates.",
		  " ",
		  tgt1,
		  tgt,
		  " ", 
		  "       Please change target name.",
		  NULL);
    g_free(tgt);
    g_free(tgt1);
    return(TRUE);
  }
  return(FALSE);
}

void WriteOPE(typHOE *hg, gboolean plan_flag){
  FILE *fp;
  int i_list=0, i_set, i_use, i_repeat, i_plan;
  gint to_year, to_month, to_day;
  gchar *tgt, *str;

  if((fp=fopen(hg->filename_write,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_write);
    exit(1);
  }

  to_year=hg->fr_year;
  to_month=hg->fr_month;
  to_day=hg->fr_day;
  add_day(hg, &to_year, &to_month, &to_day, hg->nights);

  fprintf(fp, "<HEADER>\n");
  if(hg->observer) fprintf(fp, "######  Observer : %s ######\n",hg->observer);
  fprintf(fp, "OBSERVATION_PERIOD=%4d-%02d-%02d-17:00:00 - %4d-%02d-%02d-08:00:00\n",
	  hg->fr_year,hg->fr_month,hg->fr_day,
	  to_year,to_month,to_day);
  fprintf(fp, "PROPOSALID=%s\n",hg->prop_id);
  fprintf(fp, "OBE_ID=HDS\n");
  fprintf(fp, "</HEADER>\n");


  fprintf(fp, "\n");


  fprintf(fp, "<PARAMETER_LIST>\n");
  fprintf(fp, "DEF_SPEC=OBE_ID=HDS OBE_MODE=SPEC\n");
  fprintf(fp, "DEF_PROTO=OBE_ID=HDS OBE_MODE=PROTO\n");
  fprintf(fp, "DEF_COMMON=OBE_ID=COMMON OBE_MODE=TOOL\n");
  
  fprintf(fp, "\n");
  
  fprintf(fp, "###### M2-Z POSITION for I2-Cell ######\n");
  fprintf(fp, "# I2-Cell In  :  SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
  fprintf(fp, "# I2-Cell Out :  SETI2 $DEF_SPEC I2_POSITION=\"OUT\" $I2_Z\n");
  if(hg->setup[0].is==IS_NO){
    fprintf(fp, "## w/Slit\nI2_Z=Z_IN=0.15 Z_OUT=0.0\n");
    fprintf(fp, "## w/IS\n##  I2_Z=Z_IN=0.65 Z_OUT=0.5\n");
  }
  else{
    fprintf(fp, "## w/Slit\n##  I2_Z=Z_IN=0.15 Z_OUT=0.0\n");
    fprintf(fp, "## w/IS\nI2_Z=Z_IN=0.65 Z_OUT=0.5\n");
  }
  
  fprintf(fp, "\n");
  fprintf(fp, "###### SLIT CENTER POSITION ######\n");
  fprintf(fp, "## Slit\n");
  fprintf(fp, "SV_X=SV_SLIT_X=%5.1f\n",hg->sv_slitx);
  fprintf(fp, "SV_Y=SV_SLIT_Y=%5.1f CALC_MODE=SLIT SV_COUNT=(-100)\n",hg->sv_slity);
  fprintf(fp, "## Image Slicer #1 and #2\n");
  fprintf(fp, "SV_IS_X=SV_SLIT_X=%5.1f\n",hg->sv_isx);
  fprintf(fp, "SV_IS_Y=SV_SLIT_Y=%5.1f CALC_MODE=CTR SV_COUNT=(-160)\n",hg->sv_isy);
  fprintf(fp, "## Image Slicer #3\n");
  fprintf(fp, "SV_IS3_X=SV_SLIT_X=%5.1f\n",hg->sv_is3x);
  fprintf(fp, "SV_IS3_Y=SV_SLIT_Y=%5.1f CALC_MODE=CTR SV_COUNT=(-160)\n",hg->sv_is3y);
  
  fprintf(fp, "\n");
  fprintf(fp, "###### CAMERA Z POSITION ######\n");
  fprintf(fp, "## See the most recent values in http://hds.skr.jp/wiki/index.php?%%B8%%BD%%BA%%DF%%A4%%CECamZ%%C3%%CD\n");
  fprintf(fp, "CAMZ_B=CAM_Z=%d\n",hg->camz_b);
  fprintf(fp, "CAMZ_R=CAM_Z=%d\n",hg->camz_r);

  fprintf(fp, "\n");
  fprintf(fp, "###### GUIDE PARAMETER only for bad weather ######\n");
  fprintf(fp, "G_PARA=Exptime_Factor=%d Brightness=%d SVIntegrate=%d\n",
	  hg->exptime_factor,
	  hg->brightness,
	  hg->sv_integrate);
  
  fprintf(fp, "\n");
  fprintf(fp, "###### LIST of OBJECTS ######\n");

  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name);
    if(hg->obj[i_list].i_nst<0){
      fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, hg->obj[i_list].equinox);
    }
    else{
      fprintf(fp, "# %s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	      tgt, hg->obj[i_list].name, 
	      hg->obj[i_list].ra,  hg->obj[i_list].dec, hg->obj[i_list].equinox);
    }
    g_free(tgt);
  }

  fprintf(fp, "\n");
  fprintf(fp, "</PARAMETER_LIST>\n");

  fprintf(fp, "\n");


  fprintf(fp, "<COMMAND>\n");
  fprintf(fp, "\n");
  if(hg->ocs!=OCS_GEN2){
    fprintf(fp, "# QDAS/VGW\n");
    fprintf(fp, "BootQDAS $DEF_PROTO\n");
    fprintf(fp, "BootVGW $DEF_COMMON\n");
    fprintf(fp, "ShutdownQDAS $DEF_PROTO\n");
    fprintf(fp, "ShutdownVGW $DEF_COMMON\n");
  }
  fprintf(fp, "\n");
  fprintf(fp, "# Focusing/ShowImage\n");
  fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
  fprintf(fp, "FocusSV $DEF_COMMON\n");
  fprintf(fp, "FocusAGSequence $DEF_PROTO\n");
  fprintf(fp, "FocusAG $DEF_PROTO\n");
  fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
  fprintf(fp, "MoveToStar $DEF_COMMON\n");
  fprintf(fp, "ShowImage $DEF_PROTO\n");
  fprintf(fp, "ShowImage $DEF_PROTO Display_Frame=HDSA00000000\n");
  fprintf(fp, "\n");

  fprintf(fp, "\n");
  fprintf(fp, "## Setup (If changed the number of filters, MOVE Collimator from UI)\n");
  fprintf(fp, "\n");


  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;
	fprintf(fp, "# Setup-%d  :  NonStd-%d\n", i_use+1, i_set+1);
	if(hg->nonstd[i_set].col==COL_BLUE){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=Blue CROSS_SCAN=%d COLLIMATOR=Blue $CAMZ_B",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	  if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	    fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	  }
	  if(hg->nonstd[i_set].camr!=-3600){
	    fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	  }
	  fprintf(fp, "\n## w/o Color Change\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=Red CROSS_SCAN=%d COLLIMATOR=Red $CAMZ_R",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);
	  if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	    fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	  }
	  if(hg->nonstd[i_set].camr!=-3600){
	    fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	  }
	  fprintf(fp, "\n## w/o Color Change\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  hg->nonstd[i_set].cross);

	  fprintf(fp, "## !!! If necessary, Move Collimator via UI Maintenance -> Collimator tab !!!\n");
	  switch(hg->setup[i_use].is){
	  case IS_NO:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=+2.106V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=+1.441V  !!!\n");
	    }
	    break;
	  case IS_030X5:
	  case IS_045X3:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=-0.401V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=-1.062V  !!!\n");
	    }
	    break;
	  case IS_020X3:
	    if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	       (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	      fprintf(fp, "## !!!  voltage=-1.632V  !!!\n");
	    }
	    else{
	      fprintf(fp, "## !!!  voltage=-2.292V  !!!\n");
	    }
	    break;
	  }
	  fprintf(fp,"## !!!  Calculate INC values referring the previous setup and values in http://hds.skr.jp/wiki/index.php?%%C0%%DF%%C4%%EA%%C1%%E1%%B8%%AB%%C9%%BD#c5068d22\n");
	  
	}
	fprintf(fp, "\n");
      }
      else{ //Std
	i_set=hg->setup[i_use].setup;
	
	fprintf(fp, "# Setup-%d  :  Std%s\n", i_use+1, setups[i_set].initial);
	if(i_set<StdI2b){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_B\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  setups[i_set].cross,setups[i_set].initial,setups[i_set].col);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_R\n",
		  (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		  hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		  setups[i_set].cross,setups[i_set].initial,setups[i_set].col);
	}
	fprintf(fp, "## w/o Color Change\n");
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d FILTER_1=%s FILTER_2=%s CROSS_SCAN=Std%s\n",
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		hg->setup[i_use].fil1,hg->setup[i_use].fil2,
		setups[i_set].initial);

	fprintf(fp, "## !!! If necessary, Move Collimator via UI Maintenance -> Collimator tab !!!\n");
	switch(hg->setup[i_use].is){
	case IS_NO:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=+2.106V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=+1.441V  !!!\n");
	  }
	  break;
	case IS_030X5:
	case IS_045X3:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=-0.401V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=-1.062V  !!!\n");
	  }
	  break;
	case IS_020X3:
	  if((strcmp(hg->setup[i_use].fil1,"Free")!=0)||
	     (strcmp(hg->setup[i_use].fil2,"Free")!=0)){
	    fprintf(fp, "## !!!  voltage=-1.632V  !!!\n");
	  }
	  else{
	    fprintf(fp, "## !!!  voltage=-2.292V  !!!\n");
	  }
	  break;
	}
	fprintf(fp,"## !!!  Calculate INC values referring the previous setup and values in http://hds.skr.jp/wiki/index.php?%%C0%%DF%%C4%%EA%%C1%%E1%%B8%%AB%%C9%%BD#c5068d22\n");

      }
      fprintf(fp, "\n");
	
    }
  }
  

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "#################### %d/%d/%d Obs Sequence ####################\n",
	  hg->fr_month,hg->fr_day,hg->fr_year);

  if(hg->ocs!=OCS_GEN2){
    fprintf(fp, "### QDAS/VGW\n");
    fprintf(fp, "BootQDAS $DEF_PROTO\n");
    fprintf(fp, "BootVGW $DEF_COMMON\n");
    fprintf(fp, "\n");
  }

  if(plan_flag){
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
	WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_OBJ:
	WriteOPE_OBJ_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_SetAzEl:
	fprintf(fp, "### SetAzEl  Az=%d El=%d  via Launcher ###\n\n",(int)hg->plan[i_plan].az1,(int)hg->plan[i_plan].el1);
	break;

      case PLAN_TYPE_FOCUS:
	if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
	fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
	switch(hg->plan[i_plan].focus_mode){
	case PLAN_FOCUS_SV:
	  fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
	  fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n\n\n");
	  break;
	default:
	  fprintf(fp, "FocusAGSequence $DEF_PROTO\n");
	  fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n\n\n");
	}
	break;

      case PLAN_TYPE_BIAS:
	WriteOPE_BIAS_plan(fp,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_FLAT:
	WriteOPE_FLAT_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_COMP:
	WriteOPE_COMP_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_SETUP:
	WriteOPE_SetUp_plan(fp,hg,hg->plan[i_plan]);
	break;

      case PLAN_TYPE_I2:
	if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
	fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
	if(hg->plan[i_plan].i2_pos==PLAN_I2_IN){
	  fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n\n\n");
	}
	else{
	  fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n\n\n");
	}
	break;
      }
    }

    if(hg->ocs!=OCS_GEN2){
      fprintf(fp, "### QDAS\n");
      fprintf(fp, "ShutdownQDAS $DEF_PROTO\n");
    }
  }
  else{
    fprintf(fp, "###### Evening CAL #####\n\n");
    
    WriteOPE_BIAS(fp);
    
    fprintf(fp, "### [LAUNCHER/Telescope] Telescope Setup [NsOpt] #####\n\n");
    fprintf(fp, "\n");
    
    WriteOPE_FLAT(fp,hg);
    
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL LampOff #####\n\n");
    fprintf(fp, "\n");
    
    WriteOPE_COMP(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT Retract #####\n\n");
    
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Evening FocusSV #####\n");
    fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
    fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### (Please Edit Targets) #####\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### MidNight FocusSV #####\n");
    fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
    fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### (Please Edit Targets) #####\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Morning FocusSV #####\n");
    fprintf(fp, "FocusSVSequence $DEF_COMMON\n");
    fprintf(fp, "SetStarSize $DEF_PROTO SEEING=0.60\n");
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "###### Morning CAL #####\n\n");
    fprintf(fp, "### Shutdown VGW\n");
    fprintf(fp, "ShutdownVGW $DEF_COMMON\n");
    fprintf(fp, "\n");
    WriteOPE_COMP(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT LampOff #####\n\n");
    
    fprintf(fp, "\n");
    
    WriteOPE_FLAT(fp,hg);
    fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL Returct #####\n\n");
    
    fprintf(fp, "\n");

    WriteOPE_BIAS(fp);

    if(hg->ocs!=OCS_GEN2){
      fprintf(fp, "### QDAS\n");
      fprintf(fp, "ShutdownQDAS $DEF_PROTO\n");
    }
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "#################### Reserved Targets ####################\n");
  fprintf(fp, "### Guide = NO  : No Guide\n");
  fprintf(fp, "### Guide = AG  : AG Guide\n");
  fprintf(fp, "### Guide = SV  : SV Guide\n");
  fprintf(fp, "\n");


  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup < 0){ //Non-Std
	fprintf(fp, "\n\n########  NonStd-%d %dx%dbinning  #######\n\n",
		-hg->setup[i_use].setup,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
      }
      else{  //Std
	fprintf(fp, "\n\n########  Std%s %dx%dbinning  #######\n",
		setups[hg->setup[i_use].setup].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
      }
    
      for(i_list=0;i_list<hg->i_max;i_list++){
	/// Comment for Object Name ///
	if(hg->obj[i_list].setup[i_use]){
	  fprintf(fp, "# [%d] %s   ",
		  i_list+1,
		  hg->obj[i_list].name);
	  
	  if(fabs(hg->obj[i_list].mag)<99){
	    str=get_band_name(hg, i_list);
	    fprintf(fp, " ,   %4.1lfmag (%s)",
		    hg->obj[i_list].mag, (str)?(str):"---");
	    if(str) g_free(str);
	  }
	  else{
	    fprintf(fp, " ,");
	  }

	  if(hg->obj[i_list].note){
	    fprintf(fp, "    %s\n",
		    hg->obj[i_list].note);
	  }
	  else{
	    fprintf(fp, "\n");
	  }


	  /// Comment for Setup ///
	  if(hg->setup[i_use].setup < 0){ //Non-Std
	    fprintf(fp, "#    NonStd-%d %dx%dbinning /",
		    -hg->setup[i_use].setup,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }
	  else{  //Std
	    fprintf(fp, "#    Std%s %dx%dbinning /",
		    setups[hg->setup[i_use].setup].initial,
		    hg->binning[hg->setup[i_use].binning].x,
		    hg->binning[hg->setup[i_use].binning].y);
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	    fprintf(fp," No Guide /");
	    break;
	  case AG_GUIDE:
	    fprintf(fp," AG Guide /");
	    break;
	  case SV_GUIDE:
	    fprintf(fp," SV Guide /");
	    break;
	  case SVSAFE_GUIDE:
	    fprintf(fp,"SV[Safe] Guide /");
	    break;
	  }

	  switch(hg->setup[i_use].imr){
	  case IMR_NO:
	    fprintf(fp," ImR=No\n");
	    break;
	  case IMR_LINK:
	    fprintf(fp," ImR=Link\n");
	    break;
	  case IMR_ZENITH:
	    fprintf(fp," ImR=Zenith\n");
	    break;
	  }   



	  /// SetupField ///
	  if(hg->obj[i_list].mag<MAG_SVFILTER2){  // Filter=ND2
	    fprintf(fp,"SetupField SV_Filter01=3");
	  }
	  else if(hg->obj[i_list].mag<MAG_SVFILTER1){  //Filter=R
	    fprintf(fp,"SetupField SV_Filter01=1");
	  }
	  else{  //Filter=No
	    fprintf(fp,"SetupField");// Filter=No
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	    fprintf(fp," Guide=No");
	    break;
	  case AG_GUIDE:
	    fprintf(fp," Guide=AG");
	    break;
	  case SV_GUIDE:
	    fprintf(fp," Guide=SV Mode=SemiAuto");
	    break;
	  case SVSAFE_GUIDE:
	    fprintf(fp," Guide=SV Mode=Safe");
	    break;
	  }

	  if(hg->obj[i_list].i_nst<0){
	    tgt=make_tgt(hg->obj[i_list].name);
	    fprintf(fp, " $DEF_PROTO $%s", tgt);
	    g_free(tgt);
	  }
	  else{
	    fprintf(fp, 
		    " $DEF_PROTO OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
		    hg->obj[i_list].name,
		    g_path_get_basename(hg->nst[hg->obj[i_list].i_nst].filename));
	  }

	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	  case AG_GUIDE:
	    fprintf(fp, " SVRegion=%d",
		    hg->sv_region);
	    break;
	  case SV_GUIDE:
	  case SVSAFE_GUIDE:
	    fprintf(fp," ReadRegion=%d CalcRegion=%d",
		    hg->sv_region,
		    hg->sv_calc);
	    break;
	  }

	  fprintf(fp, " Exptime_SV=%d",
		  hg->exptime_sv);
	  
	  switch(hg->setup[i_use].imr){
	  case IMR_NO:
	    fprintf(fp," IMGROT_FLAG=0");
	    break;
	  case IMR_LINK:
	    fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f",
		    hg->obj[i_list].pa);
	    break;
	  case IMR_ZENITH:
	    fprintf(fp," IMGROT_FLAG=1 IMGROT_MODE=ZENITH");
	    break;
	  }   
	  
	  if(hg->setup[i_use].is!=IS_NO){
	    fprintf(fp," IS_FLAG=1");
	    if(hg->setup[i_use].is==IS_020X3){
	      fprintf(fp, " $SV_IS3_X $SV_IS3_Y $G_PARA");
	    }
	    else{
	      fprintf(fp, " $SV_IS_X $SV_IS_Y $G_PARA");
	    }
	  }
	  else{
	    fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		    hg->setup[i_use].slit_length,
		    hg->setup[i_use].slit_width);
	    fprintf(fp, " $SV_X $SV_Y $G_PARA");
	  }


	  switch(hg->obj[i_list].guide){
	  case NO_GUIDE:
	  case AG_GUIDE:
	    fprintf(fp,"\n");
	    break;
	  case SV_GUIDE:
	  case SVSAFE_GUIDE:
	    if(hg->sv_area==SV_FULL){
	      fprintf(fp, " ReadArea=Full\n");
	    }
	    else{
	      fprintf(fp, " ReadArea=Part\n");
	    }
	    break;
	  }

	  
	  for(i_repeat=0;i_repeat<hg->obj[i_list].repeat;i_repeat++){
	    fprintf(fp,"GetObject");
	    if(hg->setup[i_use].is!=IS_NO){
	      fprintf(fp," IS_FLAG=1");
	      if(hg->setup[i_use].is==IS_020X3){
		fprintf(fp," IS_Z_OFFSET=-0.40");
	      }
	    }
	    if(hg->obj[i_list].i_nst<0){
	      tgt=make_tgt(hg->obj[i_list].name);
	      fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
		      hg->obj[i_list].exp, hg->sv_integrate, tgt);
	      g_free(tgt);
	    }
	    else{
	      fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
		      hg->obj[i_list].exp, hg->sv_integrate, 
		      hg->obj[i_list].name);
	    }
	  }

	  if(hg->setup[i_use].i2){
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    for(i_repeat=0;i_repeat<hg->obj[i_list].repeat;i_repeat++){
	      fprintf(fp,"GetObject");
	      if(hg->setup[i_use].is!=IS_NO){
		fprintf(fp," IS_FLAG=1");
		if(hg->setup[i_use].is==IS_020X3){
		  fprintf(fp," IS_Z_OFFSET=-0.40");
		}
	      }
	      if(hg->obj[i_list].i_nst<0){
		tgt=make_tgt(hg->obj[i_list].name);
		fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
			hg->obj[i_list].exp, hg->sv_integrate, tgt);
		g_free(tgt);
	      }
	      else{
		fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
			hg->obj[i_list].exp, hg->sv_integrate, 
			hg->obj[i_list].name);
	      }
	    }
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"OUT\"  $I2_Z\n");
	  }
	  fprintf(fp, "\n");
	}
      }
    }
  }


  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "##########   Calibration   ###########\n");
  fprintf(fp, "\n");

  WriteOPE_BIAS(fp);


  fprintf(fp, "##### COMPARISON #####\n"); 
  fprintf(fp, "\n");

  WriteOPE_COMP(fp,hg);

  //fprintf(fp, "SHUTDOWNCOMPARISON_HCTLAMP $DEF_PROTO OBJECT=HCT F_SELECT=NS_OPT\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HCT OBE_ID=COMMON OBE_MODE=TOOL OBJECT=HCT F_SELECT=NS_OPT\n");
  
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  fprintf(fp, "##### FLAT #####\n");
  fprintf(fp, "\n");

  WriteOPE_FLAT(fp,hg);

  fprintf(fp, "\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HALLAMP $DEF_PROTO OBJECT=HAL F_SELECT=NS_OPT\n");
  //fprintf(fp, "SHUTDOWNCOMPARISON_HAL  OBE_ID=COMMON OBE_MODE=TOOL OBJECT=HAL F_SELECT=NS_OPT\n");

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "</Command>\n");


  fclose(fp);

  
}


void WriteYAML(typHOE *hg){
  FILE *fp;
  int i_list=0, i_set, i_use, i_repeat, i_plan, j_set;
  gint to_year, to_month, to_day, nonstd_flat;
  gchar *tgt;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
  }

  to_year=hg->fr_year;
  to_month=hg->fr_month;
  to_day=hg->fr_day;
  add_day(hg, &to_year, &to_month, &to_day, hg->nights);

  fprintf(fp, "#####   Subaru Gen2 Intermediate Format File (YAML)\n");
  fprintf(fp, "#####   created by HOE : HDS OPE file Editor,  version "VERSION"\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // General Info
  fprintf(fp, "###########################\n");
  fprintf(fp, "### General Information ###\n");
  fprintf(fp, "###########################\n");
  fprintf(fp, "observation:\n");
  fprintf(fp, "   prop-id  : %s\n",hg->prop_id);
  fprintf(fp, "   start    : %4d-%02d-%02d 16:59 -10\n",hg->fr_year,hg->fr_month,hg->fr_day);
  fprintf(fp, "   end      : %4d-%02d-%02d  8:25 -10\n",to_year,to_month,to_day);
  if(hg->observer)
    fprintf(fp, "   observers: [%s ]\n",hg->observer);
  else
    fprintf(fp, "   observers: [ ]\n");
  fprintf(fp, "   operators: [ ]\n");
  fprintf(fp, "   ss       : [ ]\n");
  fprintf(fp, "   comments : \"       \"\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // Target Definition
  fprintf(fp, "#############################\n");
  fprintf(fp, "### Target Specifications ###\n");
  fprintf(fp, "#############################\n");
  fprintf(fp, "targets:\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    tgt=make_tgt(hg->obj[i_list].name);
    fprintf(fp, "   - &%s\n",tgt);
    g_free(tgt);
    fprintf(fp, "      name   : \"%s\"\n", hg->obj[i_list].name);
    fprintf(fp, "      ra     : s%09.2f\n", hg->obj[i_list].ra);
    fprintf(fp, "      dec    : s%+010.2f\n", hg->obj[i_list].dec);
    fprintf(fp, "      equinox: %7.2f\n", hg->obj[i_list].equinox);
    if(hg->obj[i_list].note)
      fprintf(fp, "      note   : \"%s\"\n", hg->obj[i_list].note);
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");


  // Telescope Specifications
  fprintf(fp, "################################\n");
  fprintf(fp, "### Telescope Specifications ###\n");
  fprintf(fp, "################################\n");
  fprintf(fp, "telescopes:\n");

  // No Guide
  fprintf(fp, "   - &t_no\n");
  fprintf(fp, "      foci           : ns_opt\n");
  fprintf(fp, "      imr            : none\n");
  fprintf(fp, "      guiding        : no\n");
  if(hg->sv_area==SV_FULL){
    fprintf(fp, "      sv_area        : full\n");
  }
  else{
    fprintf(fp, "      sv_area        : part\n");
  }
  fprintf(fp, "      sv_exp         : %d\n",hg->exptime_sv);
  fprintf(fp, "      sv_read_region : %d\n",hg->sv_region);
  fprintf(fp, "      sv_filter      : 0\n");
  fprintf(fp, "      sv_integrate   : %d\n",hg->sv_integrate);
  fprintf(fp, "      sv_acq         : %d\n",hg->sv_acq);
  fprintf(fp, "\n");
  
  // AG Guide
  fprintf(fp, "   - &t_ag\n");
  fprintf(fp, "      <<             : *t_no\n");
  fprintf(fp, "      guiding        : ag\n");
  fprintf(fp, "      exptime_factor : %d\n",hg->exptime_factor);
  fprintf(fp, "      brightness     : %d\n",hg->brightness);
  fprintf(fp, "\n");

  // SV Guide [Safe]
  fprintf(fp, "   - &t_sv_safe\n");
  fprintf(fp, "      <<             : *t_no\n");
  fprintf(fp, "      guiding        : sv\n");
  fprintf(fp, "      sv_mode        : safe\n");
  fprintf(fp, "      sv_calc_region : %d\n",hg->sv_calc);
  fprintf(fp, "      sv_slit_x      : %.1f\n",hg->sv_slitx);
  fprintf(fp, "      sv_slit_y      : %.1f\n",hg->sv_slity);
  fprintf(fp, "      sv_is_x        : %.1f\n",hg->sv_isx);
  fprintf(fp, "      sv_is_y        : %.1f\n",hg->sv_isy);
  fprintf(fp, "      sv_is3_x       : %.1f\n",hg->sv_is3x);
  fprintf(fp, "      sv_is3_y       : %.1f\n",hg->sv_is3y);
  fprintf(fp, "\n");

  // SV Guide [SemiAuto]
  fprintf(fp, "   - &t_sv_semiauto\n");
  fprintf(fp, "      <<             : *t_sv_safe\n");
  fprintf(fp, "      sv_mode        : semiauto\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

  // CAL Specifications
  fprintf(fp, "##########################\n");
  fprintf(fp, "### CAL Specifications ###\n");
  fprintf(fp, "##########################\n");
  fprintf(fp, "cal:\n");
  for(j_set=StdUb;j_set<=StdHa;j_set++){
    fprintf(fp, "  # Std%s\n", setups[j_set].initial);
    fprintf(fp, "   - &cal_std_%s\n", setups[j_set].initial);
    fprintf(fp, "      f1_amp     : %s    # Flat for CCD1\n", setups[j_set].f1_amp);
    fprintf(fp, "      f1_fil1    : %d\n", setups[j_set].f1_fil1);
    fprintf(fp, "      f1_fil2    : %d\n", setups[j_set].f1_fil2);
    fprintf(fp, "      f1_fil3    : %d\n", setups[j_set].f1_fil3);
    fprintf(fp, "      f1_fil4    : %d\n", setups[j_set].f1_fil4);
    fprintf(fp, "      f1_exp     : %d\n", setups[j_set].f1_exp);
    fprintf(fp, "      f2_amp     : %s    # Flat for CCD2\n", setups[j_set].f2_amp);
    fprintf(fp, "      f2_fil1    : %d\n", setups[j_set].f2_fil1);
    fprintf(fp, "      f2_fil2    : %d\n", setups[j_set].f2_fil2);
    fprintf(fp, "      f2_fil3    : %d\n", setups[j_set].f2_fil3);
    fprintf(fp, "      f2_fil4    : %d\n", setups[j_set].f2_fil4);
    fprintf(fp, "      f2_exp     : %d\n", setups[j_set].f2_exp);
    fprintf(fp, "      c_amp      : 15    # Comparison\n");
    fprintf(fp, "      c_fil1     : 1\n");
    fprintf(fp, "      c_fil2     : 1\n");
    fprintf(fp, "      c_fil3     : 1\n");
    fprintf(fp, "      c_fil4     : 1\n");
    fprintf(fp, "      c_exp      : 20\n");
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");


  // HDS Specifications
  fprintf(fp, "#################################\n");
  fprintf(fp, "### Instrument Specifications ###\n");
  fprintf(fp, "#################################\n");
  fprintf(fp, "instruments:\n");
  fprintf(fp, "   - &hds_basic\n");
  fprintf(fp, "      obcp        : hds\n");
  fprintf(fp, "      camz_b      : %d\n",hg->camz_b);
  fprintf(fp, "      camz_r      : %d\n",hg->camz_r);
  fprintf(fp, "      d_cross     : %d\n",hg->d_cross);
  fprintf(fp, "      exptime     : 1\n");
  fprintf(fp, "      repeat      : 1\n");
  fprintf(fp, "\n");

  // HDS Standard
  fprintf(fp, "## HDS Standard Setups\n");
  
  for(j_set=StdUb;j_set<=StdHa;j_set++){
    fprintf(fp, "  # Std%s\n", setups[j_set].initial);
    fprintf(fp, "   - &hds_std_%s\n", setups[j_set].initial);
    fprintf(fp, "      collimator : %s\n", setups[j_set].col);
    fprintf(fp, "      cross      : %s\n", setups[j_set].cross);
    fprintf(fp, "      slit_length: %.0lf\n", setups[j_set].slit_length);
    fprintf(fp, "      filter1    : %s\n", setups[j_set].fil1);
    fprintf(fp, "      filter2    : %s\n", setups[j_set].fil2);
    fprintf(fp, "      echelle    : %d\n", DEF_ECHELLE);
    fprintf(fp, "      cross_scan : %.0lf\n", setups[j_set].cross_scan);
    fprintf(fp, "      camera_rorate: -3600\n");
    fprintf(fp, "\n");
  }
  

  fprintf(fp, "\n");

  fprintf(fp, "## HDS Setups for this observation\n");
  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;
	fprintf(fp, "# NonStd-%d %dx%d\n", i_set+1,    
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "   - &hds_nonstd%d_%dx%d\n", i_set+1,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	if(hg->nonstd[i_set].col==COL_BLUE){
	  fprintf(fp, "      collimator       : Blue\n");
	  fprintf(fp, "      cross            : Blue\n");
	}
	else{
	  fprintf(fp, "      collimator       : Red\n");
	  fprintf(fp, "      cross            : Red\n");
	}
	fprintf(fp, "      slit_width     : %d\n", hg->setup[i_use].slit_width);
	fprintf(fp, "      slit_length    : %d\n", hg->setup[i_use].slit_length);
	fprintf(fp, "      filter1        : %s\n", hg->setup[i_use].fil1);
	fprintf(fp, "      filter2        : %s\n", hg->setup[i_use].fil2);
	fprintf(fp, "      echelle        : %d\n", hg->nonstd[i_set].echelle);
	fprintf(fp, "      cross_scan     : %d\n", hg->nonstd[i_set].cross);
	fprintf(fp, "      camera_rorate  : %d\n", hg->nonstd[i_set].camr);
	fprintf(fp, "      xbin           : %d\n", hg->binning[hg->setup[i_use].binning].x);
	fprintf(fp, "      ybin           : %d\n", hg->binning[hg->setup[i_use].binning].y);
	
	if(hg->nonstd[i_set].col==COL_BLUE){
	  for(j_set=StdUb;j_set<StdI2b;j_set++){
	    if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdUb:
	    nonstd_flat=StdUb;
	  case StdI2b:
	    nonstd_flat=StdYa;
	  default:
	    if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    else {
	      if(j_set==StdUb){
		nonstd_flat=j_set;
	      }
	      else{
		nonstd_flat=j_set-1;
	      }
	    }
	    
	  }
	}
	else{
	  for(j_set=StdI2b;j_set<StdHa;j_set++){
	    if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdI2b:
	    nonstd_flat=StdI2b;
	  case StdHa:
	    nonstd_flat=StdNIRa;
	  default:
	    if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    if(j_set==StdI2b){
	      nonstd_flat=j_set;
	    }
	    else{
	      nonstd_flat=j_set-1;
	    }
	    
	  }
	}

	fprintf(fp, "      cal : { <<: *cal_std_%s", setups[nonstd_flat].initial);
	if(hg->binning[hg->setup[i_use].binning].x*hg->binning[hg->setup[i_use].binning].y==1){
	  fprintf(fp, "}\n");
	}
	else{
	  gint f1_exp, f2_exp, c_exp;
	  f1_exp=setups[nonstd_flat].f1_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f1_exp==0) f1_exp=1;

	  f2_exp=setups[nonstd_flat].f2_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f2_exp==0) f2_exp=1;

	  c_exp=20/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(c_exp==0) c_exp=1;

	  fprintf(fp, ", f1_exp: %d, f2_exp: %d, fc_exp: %d}\n"
		  ,f1_exp,f2_exp,c_exp);
	}

	fprintf(fp, "\n");

      }
      else{ //Std
	i_set=hg->setup[i_use].setup;
	fprintf(fp, "# Std%s %dx%d\n", setups[i_set].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "   - &hds_std%s_%dx%d\n", setups[i_set].initial,
		hg->binning[hg->setup[i_use].binning].x,
		hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "      << : *hds_std_%s\n", setups[i_set].initial);
	fprintf(fp, "      slit_width     : %d\n", hg->setup[i_use].slit_width);
	fprintf(fp, "      slit_length    : %d\n", hg->setup[i_use].slit_length);
	fprintf(fp, "      filter1        : %s\n", hg->setup[i_use].fil1);
	fprintf(fp, "      filter2        : %s\n", hg->setup[i_use].fil2);
	fprintf(fp, "      xbin           : %d\n", hg->binning[hg->setup[i_use].binning].x);
	fprintf(fp, "      ybin           : %d\n", hg->binning[hg->setup[i_use].binning].y);
	fprintf(fp, "      cal : { <<: *cal_std_%s", setups[i_set].initial);
	if(hg->binning[hg->setup[i_use].binning].x*hg->binning[hg->setup[i_use].binning].y==1){
	  fprintf(fp, "}\n");
	}
	else{
	  gint f1_exp, f2_exp, c_exp;
	  f1_exp=setups[i_set].f1_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f1_exp==0) f1_exp=1;

	  f2_exp=setups[i_set].f2_exp/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(f2_exp==0) f2_exp=1;

	  c_exp=20/hg->binning[hg->setup[i_use].binning].x
	    /hg->binning[hg->setup[i_use].binning].y;
	  if(c_exp==0) c_exp=1;

	  fprintf(fp, ", f1_exp: %d, f2_exp: %d, c_exp: %d}\n"
		  ,f1_exp,f2_exp, c_exp);
	}
      }
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n");


  // Activities
  fprintf(fp, "################################\n");
  fprintf(fp, "### Acitivity Specifications ###\n");
  fprintf(fp, "################################\n");
  fprintf(fp, "activities:\n");

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    switch(hg->plan[i_plan].type){
    case PLAN_TYPE_COMMENT:
      WriteOPE_COMMENT_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_OBJ:
      WriteYAML_OBJ_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_FOCUS:
      if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
      fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
      switch(hg->plan[i_plan].focus_mode){
      case PLAN_FOCUS_SV:
	fprintf(fp, "   - type   : focussv\n");
	break;
      default:
	fprintf(fp, "   - type   : focusag\n");
      }
      break;
      
    case PLAN_TYPE_BIAS:
      WriteYAML_BIAS_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_FLAT:
      WriteYAML_FLAT_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_COMP:
      WriteYAML_COMP_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_SETUP:
      WriteYAML_SetUp_plan(fp,hg,hg->plan[i_plan]);
      break;
      
    case PLAN_TYPE_I2:
      if(hg->plan[i_plan].sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(hg->plan[i_plan].sod));
      fprintf(fp, "###### %s #####\n", hg->plan[i_plan].txt);
      fprintf(fp, "   - type   : i2\n");
      if(hg->plan[i_plan].i2_pos==PLAN_I2_IN){
	fprintf(fp, "     i2_position : in\n");
      }
      else{
	fprintf(fp, "     i2_position : out\n");
      }
      break;
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  fprintf(fp, "##############################\n");
  fprintf(fp, "##########   END   ###########\n");
  fprintf(fp, "##############################\n");
  fprintf(fp, "\n");


  fclose(fp);
  
}


void WriteService(typHOE *hg){
  FILE *fp;
  gint i, i_list, len, name_len=0, band_len=0, i_set, i_fc=0, num_fc=0;
  gchar *band_str, *form_str=NULL, *imr_str, *setup_str;
  gdouble r;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  // Precheck
  for(i_list=0;i_list<hg->i_max;i_list++){
    len=strlen(hg->obj[i_list].name);
    if(len>name_len) name_len=len;
    
    band_str=get_band_name(hg, i_list);
    len=strlen(band_str);
    if(len>band_len) band_len=len;
    if(band_str) g_free(band_str);
  }

  {
    fprintf (fp, "1-5.  Target List\n");
    fprintf(fp, "    (1)Name    (2)RA      Dec     Equinox   (3)Magnitude  (4)ExpTime         total S/N     (5)Slit PA\n");
    form_str=g_strdup_printf("    %%%ds,  %%09.2lf, %%+010.2lf, %%7.2lf,  %%%ds=%%5.2lf,    %%2d x %%4dsec = %%5dsec,  %%4.0lf /pixel,   %%s\n",
			     name_len,band_len);

    for(i_list=0;i_list<hg->i_max;i_list++){
      band_str=get_band_name(hg, i_list);
      if(hg->etc_imr==ETC_IMR_NO){
	imr_str=g_strdup("(Any)");
      }
      else if(hg->setup[hg->etc_setup].imr==IMR_ZENITH){
	imr_str=g_strdup("Parallactic");
      }
      else{
	imr_str=g_strdup_printf("%.2lfdeg",hg->obj[i_list].pa);
      }
      
      fprintf(fp, form_str,
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox,
	      band_str,
	      hg->obj[i_list].mag,
	      hg->obj[i_list].repeat,
	      hg->obj[i_list].exp,
	      hg->obj[i_list].exp*hg->obj[i_list].repeat,
	      (gdouble)hg->obj[i_list].snr*sqrt((gdouble)hg->obj[i_list].repeat),
	      imr_str);
      if(band_str) g_free(band_str);
      if(imr_str) g_free(imr_str);
    }

    if(form_str) g_free(form_str);

    fprintf(fp,"\n");
    if(hg->etc_wave==ETC_WAVE_CENTER){
      fprintf(fp,"          *** Total S/N is estimated in %.2lf seeing at Center ***\n", hg->etc_seeing);
    }
    else{
      fprintf(fp,"          *** Total S/N is estimated in %.2lf seeing at %dA ***\n", hg->etc_seeing, hg->etc_waved);
    }

    fprintf(fp,"\n");
    fprintf(fp,"\n");

    fprintf (fp, "6.  Requirement for ADC (Yes/No)\n");
    if(hg->etc_adc==ETC_ADC_IN){
      fprintf (fp, "      Yes\n");
    }
    else{
      fprintf (fp, "      No\n");
    }

    fprintf (fp, "7.  Image Rotator Mode (Red/Blue/None)\n");
    if(hg->etc_imr==ETC_IMR_RED){
      fprintf (fp, "      Red\n");
    }
    else if(hg->etc_imr==ETC_IMR_BLUE){
      fprintf (fp, "      Blue\n");
    }
    else{
      fprintf (fp, "      None\n");
    }

    fprintf (fp, "8.  Wavelength Setup\n");
    if(hg->setup[hg->etc_setup].setup<0){
      i_set=-hg->setup[hg->etc_setup].setup-1;
      if(hg->nonstd[i_set].col==COL_BLUE){
	setup_str=g_strdup_printf("      Non-Std (Blue/Cross Scan=%d\"/Camera Rot=%d\"/Echelle=%d\")\n",
				  hg->nonstd[i_set].cross,
				  hg->nonstd[i_set].camr,
				  hg->nonstd[i_set].echelle);
      }
      else{
	setup_str=g_strdup_printf("      Non-Std (Red/Cross Scan=%d\"/Camera Rot=%d\"/Echelle=%d\")\n",
				  hg->nonstd[i_set].cross,
				  hg->nonstd[i_set].camr,
				  hg->nonstd[i_set].echelle);
      }
      fprintf(fp,"%s", setup_str);
      if(setup_str) g_free(setup_str);
    }
    else{
      fprintf(fp, "      Std%s\n", setups[i_set].initial);
    }

    fprintf (fp, "9.  Wavelength Resolution\n");
    switch(hg->setup[hg->etc_setup].is){
    case IS_030X5:
      fprintf (fp, "      R=110000 (Image Slicer #1 : 0.30\"x5)\n");
      break;

    case IS_045X3:
      fprintf (fp, "      R=80000 (Image Slicer #2 : 0.45\"x3)\n");
      break;

    case IS_020X3:
      fprintf (fp, "      R=160000 (Image Slicer #3 : 0.20\"x3)\n");
      break;

    case IS_NO:
      r=36000/((gdouble)hg->setup[hg->etc_setup].slit_width/500.);
      fprintf (fp, "      R=%.0lf (%.2lf\" slit width)\n",
	       (r>160000)?16000:r,
	       (gdouble)hg->setup[hg->etc_setup].slit_width/500.);
      break;
    }
	     
    fprintf (fp, "10. CCD Binning\n");
    fprintf (fp, "      %dx%d\n",
	     hg->binning[hg->setup[hg->etc_setup].binning].x,
	     hg->binning[hg->setup[hg->etc_setup].binning].y);

    fprintf (fp, "11. Requested seeing size\n");
    fprintf (fp, "      Any\n");
    
    fprintf (fp, "12. Are photometric conditions necessary? (Yes/No)\n");
    fprintf (fp, "      No\n");

    fprintf(fp,"\n");
    
    fprintf (fp, "13. Priority\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      fprintf(fp, "     %d. %s\n",i_list+1, hg->obj[i_list].name);
    }

    fprintf (fp, "14. Special requests\n");
    if(hg->setup[hg->etc_setup].is){
      fprintf (fp, "      I2 Cell will be employed for this program.\n");
    }
    else{
      fprintf (fp, "      None\n");
    }

    fprintf(fp,"\n");

    fprintf (fp, "A. Finding Charts\n");
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].mag>10){
	i_fc++;
      }
    }
    num_fc=i_fc;
    i_fc=0;
    for(i_list=0;i_list<hg->i_max;i_list++){
      if(hg->obj[i_list].mag>10){
	i_fc++;
	fprintf(fp, "     %d. %s,  Yes,  %d of %d,  %d\'x%d\',  North is up\n",
		i_list+1, hg->obj[i_list].name,
		i_fc, num_fc, hg->dss_arcmin, hg->dss_arcmin);
      }
      else{
	fprintf(fp, "     %d. %s,  No\n",i_list+1, hg->obj[i_list].name);
      }
    }

    fclose(fp);
  }
}


void WritePROMS(typHOE *hg){
  FILE *fp;
  gint i_list;
  gchar *band_str;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  // Precheck
  for(i_list=0;i_list<hg->i_max;i_list++){
    band_str=get_band_name(hg, i_list);
    fprintf(fp,"%s\t%09.2lf\t%+09.1lf\t%.2lf(%s)\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    hg->obj[i_list].mag,
	    band_str);
    if(band_str) g_free(band_str);
  }

  fclose(fp);
}


void WritePlan(typHOE *hg){
  FILE *fp;
  int i_plan;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    return;
  }

  fprintf (fp,     " HST   [min]  Tasks\n");
  fprintf (fp, "================================================================\n");

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].sod>0){
      fprintf (fp, " %5s [%3d]  %s\n", 
	       get_txt_tod(hg->plan[i_plan].sod), 
	       ( (hg->plan[i_plan].time>0) ? hg->plan[i_plan].time/60 : -hg->plan[i_plan].time/60),
	       hg->plan[i_plan].txt);
    }
    else if (hg->plan[i_plan].time!=0){
      fprintf (fp, "       [%3d]  %s\n", 
	       ( (hg->plan[i_plan].time>0) ? hg->plan[i_plan].time/60 : -hg->plan[i_plan].time/60),
	       hg->plan[i_plan].txt);
    }
    else{
      if(hg->plan[i_plan].type==PLAN_TYPE_COMMENT){
	fprintf (fp, "\n              %s\n\n", hg->plan[i_plan].txt);
      }
      else{
	fprintf (fp, "              %s\n", hg->plan[i_plan].txt);
      }
    }
  }

  fclose(fp);

  
}


void WriteOPE_BIAS(FILE *fp){
  fprintf(fp, "### BIAS\n");
  fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
  fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS NFILES=4\n");

  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


 void WriteOPE_BIAS_plan(FILE *fp, PLANpara plan){
   if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
   fprintf(fp, "###### %s #####\n", plan.txt);
   if(plan.repeat>1){
     fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
     fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS NFILES=%d\n",plan.repeat-1);
   }
   else{
     fprintf(fp, "GetBias $DEF_SPEC OBJECT=BIAS\n");
   }

  fprintf(fp, "\n");
  fprintf(fp, "\n");
}


void WriteYAML_BIAS_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : bias\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  fprintf(fp, ", exptime:0, repeat: %d}\n",plan.repeat);
}


void WriteOPE_SetUp_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  if(plan.slit_or){
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
	    plan.slit_length);
  }
  else{
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
	    hg->setup[plan.setup].slit_length);
  }

  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    //fprintf(fp, "# NonStd-%d\n", i_set+1);
    if(plan.cmode==PLAN_CMODE_FULL){
      if(hg->nonstd[i_set].col==COL_BLUE){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Blue CROSS_SCAN=%d COLLIMATOR=Blue $CAMZ_B",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		hg->nonstd[i_set].cross);
	if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	  fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	}
	if(hg->nonstd[i_set].camr!=-3600){
	  fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	}
      }
      else{
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=Red CROSS_SCAN=%d COLLIMATOR=Red $CAMZ_R",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		hg->nonstd[i_set].cross);
	if(hg->nonstd[i_set].echelle!=DEF_ECHELLE){
	  fprintf(fp, " ECHELLE=%d", hg->nonstd[i_set].echelle);
	}
	if(hg->nonstd[i_set].camr!=-3600){
	  fprintf(fp, " CAMERA_ROTATE=%d", hg->nonstd[i_set].camr);
	}
      }
    }
    else if(plan.cmode==PLAN_CMODE_EASY){
      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS_SCAN=%d",
	      hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
	      hg->nonstd[i_set].cross);
    }
    fprintf(fp, "\n");
  }
  else{ //Std
    i_set=hg->setup[plan.setup].setup;
	
    //fprintf(fp, "# Std%s\n", setups[i_set].initial);
    if(plan.cmode==PLAN_CMODE_FULL){
      if(i_set<StdI2b){
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_B\n",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		setups[i_set].cross,setups[i_set].initial,setups[i_set].col);
      }
      else{
	fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS=%s CROSS_SCAN=Std%s COLLIMATOR=%s $CAMZ_R\n",
		hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
		setups[i_set].cross,setups[i_set].initial,setups[i_set].col);
      }
    }
    else if(plan.cmode==PLAN_CMODE_EASY){
      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=%s FILTER_2=%s CROSS_SCAN=Std%s\n",
	      hg->setup[plan.setup].fil1,hg->setup[plan.setup].fil2,
	      setups[i_set].initial);
    }
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");

}

void WriteYAML_SetUp_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : setup\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }

  if(plan.cmode==PLAN_CMODE_FULL){
    fprintf(fp, ", change: full}\n");
  }
  else if(plan.cmode==PLAN_CMODE_EASY){
    fprintf(fp, ", change: cross}\n");
  }
  else{
    fprintf(fp, ", change: slit}\n");
  }

}


void WriteOPE_COMP(FILE *fp, typHOE *hg){
  gint i_use;

  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      {
	gint i_bin;
	i_bin=hg->setup[i_use].binning;
	if(hg->setup[i_use].setup < 0){ // NonStd
	  fprintf(fp, "### COMPARISON  for NonStd-%d  %dx%dBINNING\n", 
		  -hg->setup[i_use].setup,hg->binning[i_bin].x, hg->binning[i_bin].y);
	}
	else{ // Std
	  fprintf(fp, "### COMPARISON  for Std%s  %dx%dBINNING\n", 
		  setups[hg->setup[i_use].setup].initial,hg->binning[i_bin].x, hg->binning[i_bin].y);
	}
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000,
		(hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_width : 2000);
	fprintf(fp, "SETUPCOMPARISON_HCT $DEF_PROTO OBJECT=HCT LAMP=HCT1 AMP=15.0 FILTER01=1 FILTER02=1 FILTER03=1 FILTER04=1 F_SELECT=NS_OPT\n");
	fprintf(fp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
		20/hg->binning[i_bin].x/hg->binning[i_bin].y);
	fprintf(fp, "\n");
      }
    }
  }
}


void WriteOPE_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  if(plan.slit_or){
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	    plan.slit_length,
	    plan.slit_width);
  }
  else{
    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d SLIT_WIDTH=%d\n",
	    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000,
	    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_width : 2000);
  }
  
  fprintf(fp, "SETUPCOMPARISON_HCT $DEF_PROTO OBJECT=HCT LAMP=HCT1 AMP=15.0 FILTER01=1 FILTER02=1 FILTER03=1 FILTER04=1 F_SELECT=NS_OPT\n");
  fprintf(fp, "GetComparison $DEF_SPEC OBJECT=Comparison Exptime=%d\n",
	  20/hg->binning[hg->setup[plan.setup].binning].x/hg->binning[hg->setup[plan.setup].binning].y);
  fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HCT Retract/LampOff #####\n\n\n");
  
}


void WriteYAML_COMP_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  
  fprintf(fp, "   - type   : comp\n");
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d}\n",
	    plan.slit_width, plan.slit_length);
  }
  else{
    fprintf(fp,"}\n");
  }

}


void WriteOPE_COMMENT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  switch(plan.comtype){
  case PLAN_COMMENT_TEXT:
    if(plan.comment){
      fprintf(fp, "###### %s #####\n\n", plan.comment);
    }
    else{
      fprintf(fp, "######\n\n");
    }
    break;

  case PLAN_COMMENT_SUNSET:
  case PLAN_COMMENT_SUNRISE:
    fprintf(fp, "%s\n\n", plan.txt);
    break;
  }
}


void WriteOPE_OBJ_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_repeat, i_set;
  gchar *tgt;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(hg->obj[plan.obj_i].note)   fprintf(fp, "# %s\n", hg->obj[plan.obj_i].note);
  if(plan.omode!=PLAN_OMODE_GET){
    /// SetupField ///
    fprintf(fp,"SetupField");
    
    switch(plan.guide){
    case NO_GUIDE:
      fprintf(fp," Guide=No");
      break;
    case AG_GUIDE:
      fprintf(fp," Guide=AG");
      break;
    case SV_GUIDE:
      fprintf(fp," Guide=SV Mode=SemiAuto");
      break;
    case SVSAFE_GUIDE:
      fprintf(fp," Guide=SV Mode=Safe");
      break;
    }

    if(hg->obj[plan.obj_i].i_nst<0){
      tgt=make_tgt(hg->obj[plan.obj_i].name);
      fprintf(fp, " $DEF_PROTO $%s", tgt);
      g_free(tgt);
    }
    else{
      fprintf(fp, 
	      " $DEF_PROTO OBJECT=\"%s\" COORD=FILE Target=\"08 %s\"",
	      hg->obj[plan.obj_i].name,
	      g_path_get_basename(hg->nst[hg->obj[plan.obj_i].i_nst].filename));
    }
    
    switch(plan.guide){
    case NO_GUIDE:
    case AG_GUIDE:
      fprintf(fp, " SVRegion=%d",
	      hg->sv_region);
      break;
    case SV_GUIDE:
    case SVSAFE_GUIDE:
      fprintf(fp," ReadRegion=%d CalcRegion=%d",
	      hg->sv_region,
	      hg->sv_calc);
      break;
    }
    
    if(plan.sv_or)
      fprintf(fp, " Exptime_SV=%d", plan.sv_exp);
    else
      fprintf(fp, " Exptime_SV=%d", hg->exptime_sv);
    
    if(plan.sv_fil!=SV_FILTER_NONE)
      fprintf(fp, " SV_FILTER01=%d", plan.sv_fil);
    
    switch(hg->setup[plan.setup].imr){
    case IMR_NO:
      fprintf(fp," IMGROT_FLAG=0");
      break;
    case IMR_LINK:
      if(plan.pa_or)
	fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f", plan.pa);
      else
	fprintf(fp," IMGROT_FLAG=1 SLIT_PA=%.1f", hg->obj[plan.obj_i].pa);
      break;
    case IMR_ZENITH:
      fprintf(fp," IMGROT_FLAG=1 IMGROT_MODE=ZENITH");
      break;
    }   

    if(hg->setup[plan.setup].is!=IS_NO){
      fprintf(fp," IS_FLAG=1");
      if(hg->setup[plan.setup].is==IS_020X3){
	fprintf(fp, " $SV_IS3_X $SV_IS3_Y $G_PARA");
      }
      else{
	fprintf(fp, " $SV_IS_X $SV_IS_Y $G_PARA");
      }
    }
    else{
      if(plan.slit_or){
	fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		plan.slit_length,
		plan.slit_width);
      }
      else{
	fprintf(fp, " Slit_Length=%d Slit_Width=%d",
		hg->setup[plan.setup].slit_length,
		hg->setup[plan.setup].slit_width);
      }
      fprintf(fp, " $SV_X $SV_Y $G_PARA");
    }
    
    
    
    switch(plan.guide){
    case NO_GUIDE:
    case AG_GUIDE:
      fprintf(fp,"\n");
      break;
    case SV_GUIDE:
    case SVSAFE_GUIDE:
      if(hg->sv_area==SV_FULL){
	fprintf(fp, " ReadArea=Full\n");
      }
      else{
	fprintf(fp, " ReadArea=Part\n");
      }
      break;
    }
  }

	  
  if(plan.omode!=PLAN_OMODE_SET){
    for(i_repeat=0;i_repeat<plan.repeat;i_repeat++){
      fprintf(fp,"GetObject");
      if(hg->setup[plan.setup].is!=IS_NO){
	fprintf(fp," IS_FLAG=1");
	if(hg->setup[plan.setup].is==IS_020X3){
	  fprintf(fp," IS_Z_OFFSET=-0.40");
	}
      }
      if(hg->obj[plan.obj_i].i_nst<0){
	tgt=make_tgt(hg->obj[plan.obj_i].name);
	fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
		plan.exp, hg->sv_integrate, tgt);
	g_free(tgt);
      }
      else{
	fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d OBJECT=\"%s\"\n",
		plan.exp, hg->sv_integrate, hg->obj[plan.obj_i].name);
      }
    }
  }
  fprintf(fp, "\n");
}


void WriteYAML_OBJ_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_repeat, i_set;
  gchar *tgt;
  
  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  else if (plan.backup) fprintf(fp, "## *** BackUp ***\n");
  fprintf(fp, "###### %s #####\n", plan.txt);
  if(hg->obj[plan.obj_i].note)   fprintf(fp, "# %s\n", hg->obj[plan.obj_i].note);
  fprintf(fp, "   - type   : obj\n");

  // Obj
  tgt=make_tgt(hg->obj[plan.obj_i].name);
  fprintf(fp, "     obj    : *%s\n", tgt);
  g_free(tgt);

  // Instrument
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }

  fprintf(fp,", exptime: %d, repeat: %d}\n", plan.exp, plan.repeat);
  

  // Telescope
  switch(plan.guide){
  case NO_GUIDE:
    fprintf(fp, "     tel: { <<: *t_no");
    break;
  case AG_GUIDE:
    fprintf(fp, "     tel: { <<: *t_ag");
    break;
  case SV_GUIDE:
    fprintf(fp, "     tel: { <<: *t_sv_semiauto");
    break;
  case SVSAFE_GUIDE:
    fprintf(fp, "     tel: { <<: *t_sv_safe");
    break;
  }

  if(plan.sv_or)
    fprintf(fp, ", sv_exp: %d", plan.sv_exp);

  if(plan.sv_fil!=SV_FILTER_NONE)
    fprintf(fp, ", sv_filter: %d", plan.sv_fil);

  switch(hg->setup[plan.setup].imr){
  case IMR_LINK:
    if(plan.pa_or)
      fprintf(fp,", imr: link, pa:%.1f}\n", plan.pa);
    else
      fprintf(fp,", imr: link, pa:%.1f}\n", hg->obj[plan.obj_i].pa);
    break;
  case IMR_ZENITH:
    fprintf(fp,", imr: zenith}\n");
    break;
  default:
    fprintf(fp,"}\n");
  }   
}



void WriteOPE_FLAT(FILE *fp, typHOE *hg){
  gint i_set,j_set,i_use;
  int nonstd_flat;

  for(i_use=0;i_use<MAX_USESETUP;i_use++){
    if(hg->setup[i_use].use){
      if(hg->setup[i_use].setup<0){ // NonStd
	i_set=-hg->setup[i_use].setup-1;

	if(hg->nonstd[i_set].col==COL_BLUE){
	  for(j_set=StdUb;j_set<StdI2b;j_set++){
	    if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdUb:
	    nonstd_flat=StdUb;
	  case StdI2b:
	    nonstd_flat=StdYa;
	  default:
	    if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    else {
	      if(j_set==StdUb){
		nonstd_flat=j_set;
	      }
	      else{
		nonstd_flat=j_set-1;
	      }
	    }
	    
	  }
	}
	else{
	  for(j_set=StdI2b;j_set<StdHa;j_set++){
	    if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	      break;
	    }
	  }
	  switch(j_set){
	  case StdI2b:
	    nonstd_flat=StdI2b;
	  case StdHa:
	    nonstd_flat=StdNIRa;
	  default:
	    if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	       < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	      nonstd_flat=j_set;
	    }
	    if(j_set==StdI2b){
	      nonstd_flat=j_set;
	    }
	    else{
	      nonstd_flat=j_set-1;
	    }
	    
	  }
	}
      
	if((!strcmp(setups[nonstd_flat].f1_amp,setups[nonstd_flat].f2_amp))
	   &&(setups[nonstd_flat].f1_fil1==setups[nonstd_flat].f2_fil1)
	   &&(setups[nonstd_flat].f1_fil2==setups[nonstd_flat].f2_fil2)
	   &&(setups[nonstd_flat].f1_fil3==setups[nonstd_flat].f2_fil3)
	   &&(setups[nonstd_flat].f1_fil4==setups[nonstd_flat].f2_fil4)
	   &&(setups[nonstd_flat].f1_exp==setups[nonstd_flat].f2_exp)){  //Same Setup for Blue & Red

	  {
	    int i_bin;
	    i_bin=hg->setup[i_use].binning;
	    
	    fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
		    i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=%d SLIT_LENGTH=%d\n\n",
		      (hg->setup[i_use].is == IS_NO) ? 200 : 2000,
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    fprintf(fp, "# CCD1 and 2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		    i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[nonstd_flat].f1_amp,  setups[nonstd_flat].f1_fil1, setups[nonstd_flat].f1_fil2,
		    setups[nonstd_flat].f1_fil3, setups[nonstd_flat].f1_fil4);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "\n");
	    fprintf(fp, "#  for order trace\n");
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
		    (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    fprintf(fp, "\n");
	    fprintf(fp, "\n");
	  }
	}
	else{
	  
	  {
	    int i_bin;
	    i_bin=hg->setup[i_use].binning;

	    fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
		    i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=%d SLIT_LENGTH=%d\n\n",
		      (hg->setup[i_use].is == IS_NO) ? 200 : 2000,
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    fprintf(fp, "# CCD1 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		    i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[nonstd_flat].f1_amp,  setups[nonstd_flat].f1_fil1, setups[nonstd_flat].f1_fil2,
		    setups[nonstd_flat].f1_fil3, setups[nonstd_flat].f1_fil4);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "\n");
	    
	    fprintf(fp, "# CCD2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		    i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[nonstd_flat].f2_amp,  setups[nonstd_flat].f2_fil1, setups[nonstd_flat].f2_fil2,
		    setups[nonstd_flat].f2_fil3, setups[nonstd_flat].f2_fil4);
	    if((nonstd_flat==StdUa)||(nonstd_flat==StdUb)){
	      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	      
	    }
	    if(hg->ocs==OCS_GEN2){
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
			(guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      fprintf(fp, "\n");
	      fprintf(fp, "#  for order trace\n");
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    else{
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	    }
	    fprintf(fp, "\n");
	    fprintf(fp, "\n");
	  }
	}
      }
      else{ //Std
	i_set=hg->setup[i_use].setup;

	if((!strcmp(setups[i_set].f1_amp,setups[i_set].f2_amp))
	   &&(setups[i_set].f1_fil1==setups[i_set].f2_fil1)
	   &&(setups[i_set].f1_fil2==setups[i_set].f2_fil2)
	   &&(setups[i_set].f1_fil3==setups[i_set].f2_fil3)
	   &&(setups[i_set].f1_fil4==setups[i_set].f2_fil4)
	   &&(setups[i_set].f1_exp==setups[i_set].f2_exp)){  //Same setup for Blue and Red
	
	  {
	    int i_bin;
	    i_bin=hg->setup[i_use].binning;

	    fprintf(fp, "## FLAT  %s %dx%d\n",
		    setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=%d SLIT_LENGTH=%d\n\n",
		      (hg->setup[i_use].is == IS_NO) ? 200 : 2000,
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    fprintf(fp, "# CCD1 and 2 Flat for %s (%dx%dbinning)\n",
		    setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[i_set].f1_amp,  setups[i_set].f1_fil1, setups[i_set].f1_fil2,
		    setups[i_set].f1_fil3, setups[i_set].f1_fil4);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "\n");
	    fprintf(fp, "#  for order trace\n");
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
		    (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    
	    fprintf(fp, "\n");
	    fprintf(fp, "\n");
	  }
	}
	else{  //Different Setup for Blue & Red
	  
	  {
	    int i_bin;
	    i_bin=hg->setup[i_use].binning;
	    
	    fprintf(fp, "## FLAT  %s %dx%d\n",
		    setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=%d SLIT_LENGTH=%d\n\n",
		      (hg->setup[i_use].is == IS_NO) ? 200 : 2000,
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    fprintf(fp, "# CCD1 Flat for %s (%dx%dbinning)\n",
		    setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[i_set].f1_amp,  setups[i_set].f1_fil1, setups[i_set].f1_fil2,
		    setups[i_set].f1_fil3, setups[i_set].f1_fil4);
	    if(hg->setup[i_use].is==IS_NO){
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    else{
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4\n",
		      (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    }
	    fprintf(fp, "\n");
	      
	    fprintf(fp, "# CCD2 Flat for %s (%dx%dbinning)\n",
		    setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	    fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		    setups[i_set].f2_amp,  setups[i_set].f2_fil1, setups[i_set].f2_fil2,
		    setups[i_set].f2_fil3, setups[i_set].f2_fil4);
	    if((i_set==StdUa)||(i_set==StdUb)){
	      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	    }
	    if(hg->ocs==OCS_GEN2){
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
			(guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      fprintf(fp, "\n");
	      fprintf(fp, "#  for order trace\n");
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n",
		      (hg->setup[i_use].is == IS_NO) ? hg->setup[i_use].slit_length : 30000);
	    }
	    else{
	      if(hg->setup[i_use].is==IS_NO){
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	      else{
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
		fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=4 Display_Frame=!STATOBS.HDS.C2\n",
			(guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	      }
	    }
	    if((i_set==StdUa)||(i_set==StdUb)){
	      fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=Free\n");
	    }
	    fprintf(fp, "\n");
	    fprintf(fp, "\n");
	  }
	}
      }
    }
  }
}


void WriteOPE_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set,j_set;
  int nonstd_flat;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);

  if(hg->setup[plan.setup].is==IS_NO){
    if(plan.slit_or){
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=%d SLIT_LENGTH=%d\n\n",
	      plan.slit_width, plan.slit_length);
    }
    else{
      fprintf(fp, "SetupOBE $DEF_SPEC SLIT_WIDTH=200 SLIT_LENGTH=%d\n\n",
	      hg->setup[plan.setup].slit_length);
    }
  }
   

  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    
    if(hg->nonstd[i_set].col==COL_BLUE){
      for(j_set=StdUb;j_set<StdI2b;j_set++){
	    if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	      break;
	    }
      }
      switch(j_set){
      case StdUb:
	nonstd_flat=StdUb;
      case StdI2b:
	nonstd_flat=StdYa;
      default:
	if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	   < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	  nonstd_flat=j_set;
	}
	else {
	  if(j_set==StdUb){
	    nonstd_flat=j_set;
	  }
	  else{
	    nonstd_flat=j_set-1;
	  }
	}
	
      }
    }
    else{
      for(j_set=StdI2b;j_set<StdHa;j_set++){
	if(hg->nonstd[i_set].cross<=setups[j_set].cross_scan){
	  break;
	    }
	  }
      switch(j_set){
      case StdI2b:
	nonstd_flat=StdI2b;
      case StdHa:
	nonstd_flat=StdNIRa;
      default:
	if((setups[j_set].cross_scan - hg->nonstd[i_set].cross)
	   < (hg->nonstd[i_set].cross - setups[j_set-1].cross_scan)){
	  nonstd_flat=j_set;
	}
	if(j_set==StdI2b){
	  nonstd_flat=j_set;
	}
	else{
	  nonstd_flat=j_set-1;
	}
	
      }
    }
    
    if((!strcmp(setups[nonstd_flat].f1_amp,setups[nonstd_flat].f2_amp))
       &&(setups[nonstd_flat].f1_fil1==setups[nonstd_flat].f2_fil1)
       &&(setups[nonstd_flat].f1_fil2==setups[nonstd_flat].f2_fil2)
       &&(setups[nonstd_flat].f1_fil3==setups[nonstd_flat].f2_fil3)
       &&(setups[nonstd_flat].f1_fil4==setups[nonstd_flat].f2_fil4)
       &&(setups[nonstd_flat].f1_exp==setups[nonstd_flat].f2_exp)){  //Same Setup for Blue & Red
      
      {
	int i_bin;
	i_bin=hg->setup[plan.setup].binning;
	
	//fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
	//	i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "# CCD1 and 2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[nonstd_flat].f1_amp,  setups[nonstd_flat].f1_fil1, setups[nonstd_flat].f1_fil2,
		setups[nonstd_flat].f1_fil3, setups[nonstd_flat].f1_fil4);
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	fprintf(fp, "\n");
	fprintf(fp, "#  for order trace\n");
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	}
	if(plan.slit_or){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		  plan.slit_length);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		  (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000);
	}
      }
    }
    else{
	  
      {
	int i_bin;
	i_bin=hg->setup[plan.setup].binning;
	
	//fprintf(fp, "## FLAT  NonStd-%d %dx%d\n",
	//i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "# CCD1 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[nonstd_flat].f1_amp,  setups[nonstd_flat].f1_fil1, setups[nonstd_flat].f1_fil2,
		setups[nonstd_flat].f1_fil3, setups[nonstd_flat].f1_fil4);
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	fprintf(fp, "\n");
	    
	fprintf(fp, "# CCD2 Flat for NonStd-%d (%dx%dbinning)    Using Setup for Std%s\n",
		i_set+1,hg->binning[i_bin].x,hg->binning[i_bin].y,setups[nonstd_flat].initial);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[nonstd_flat].f2_amp,  setups[nonstd_flat].f2_fil1, setups[nonstd_flat].f2_fil2,
		setups[nonstd_flat].f2_fil3, setups[nonstd_flat].f2_fil4);
	if((nonstd_flat==StdUa)||(nonstd_flat==StdUb)){
	  fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	  
	}
	if(hg->ocs==OCS_GEN2){
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		    (guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  }
	  if(plan.slit_or){
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		    plan.slit_length);
	  }
	  else{
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000);
	  }
	}
	else{
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[nonstd_flat].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	}
      }
    }
  }
  else{ //Std
    i_set=hg->setup[plan.setup].setup;

    if((!strcmp(setups[i_set].f1_amp,setups[i_set].f2_amp))
       &&(setups[i_set].f1_fil1==setups[i_set].f2_fil1)
       &&(setups[i_set].f1_fil2==setups[i_set].f2_fil2)
       &&(setups[i_set].f1_fil3==setups[i_set].f2_fil3)
       &&(setups[i_set].f1_fil4==setups[i_set].f2_fil4)
       &&(setups[i_set].f1_exp==setups[i_set].f2_exp)){  //Same setup for Blue and Red
      
      {
	int i_bin;
	i_bin=hg->setup[plan.setup].binning;

	//fprintf(fp, "## FLAT  %s %dx%d\n",
	//	setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "# CCD1 and 2 Flat for %s (%dx%dbinning)\n",
		setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[i_set].f1_amp,  setups[i_set].f1_fil1, setups[i_set].f1_fil2,
		setups[i_set].f1_fil3, setups[i_set].f1_fil4);
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		plan.repeat-1);
	}
	fprintf(fp, "\n");
	fprintf(fp, "#  for order trace\n");
	fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	}
	if(plan.slit_or){
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		  plan.slit_length);
	}
	else{
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		  (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000);
	}
      }
    }
    else{  //Different Setup for Blue & Red
	  
      {
	int i_bin;
	i_bin=hg->setup[plan.setup].binning;
	
	//fprintf(fp, "## FLAT  %s %dx%d\n",
	//	setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "# CCD1 Flat for %s (%dx%dbinning)\n",
		setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[i_set].f1_amp,  setups[i_set].f1_fil1, setups[i_set].f1_fil2,
		setups[i_set].f1_fil3, setups[i_set].f1_fil4);
	if(hg->setup[plan.setup].is==IS_NO){
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	else{
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d\n",
		  (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f1_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		  plan.repeat-1);
	}
	fprintf(fp, "\n");
	      
	fprintf(fp, "# CCD2 Flat for %s (%dx%dbinning)\n",
		setups[i_set].initial,hg->binning[i_bin].x,hg->binning[i_bin].y);
	fprintf(fp, "SETUPCOMPARISON_HAL $DEF_PROTO OBJECT=HAL LAMP=HAL1 AMP=%s FILTER01=%d FILTER02=%d FILTER03=%d FILTER04=%d F_SELECT=NS_OPT\n",
		setups[i_set].f2_amp,  setups[i_set].f2_fil1, setups[i_set].f2_fil2,
		setups[i_set].f2_fil3, setups[i_set].f2_fil4);
	if((i_set==StdUa)||(i_set==StdUb)){
	  fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=U340\n");
	}
	if(hg->ocs==OCS_GEN2){
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		    (guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  fprintf(fp, "\n");
	  fprintf(fp, "#  for order trace\n");
	  fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=1000\n");
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_CCD=2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	  }
	  if(plan.slit_or){
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		    plan.slit_length);
	  }
	  else{
	    fprintf(fp, "SetupOBE $DEF_SPEC SLIT_LENGTH=%d\n\n",
		    (hg->setup[plan.setup].is == IS_NO) ? hg->setup[plan.setup].slit_length : 30000);
	  }
	}
	else{
	  if(hg->setup[plan.setup].is==IS_NO){
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	  else{
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y));
	    fprintf(fp, "GetOBEFlat $DEF_SPEC OBJECT=Flat Exptime=%d NFILES=%d Display_Frame=!STATOBS.HDS.C2\n",
		    (guint)(IS_FLAT_FACTOR*(gdouble)setups[i_set].f2_exp/hg->binning[i_bin].x/hg->binning[i_bin].y),
		    plan.repeat-1);
	  }
	}
	if((i_set==StdUa)||(i_set==StdUb)){
	  fprintf(fp, "SetupOBE $DEF_SPEC FILTER_1=Free\n");
	}
      }
    }
  }
  fprintf(fp, "### [LAUNCHER/HDS] ShutdownComparison HAL Retract/LampOff #####\n\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");

}


void WriteYAML_FLAT_plan(FILE *fp, typHOE *hg, PLANpara plan){
  gint i_set,j_set;
  int nonstd_flat;

  if(plan.sod>0)  fprintf(fp, "## [%s]\n", get_txt_tod(plan.sod));
  fprintf(fp, "###### %s #####\n", plan.txt);
  fprintf(fp, "   - type   : flat\n");
  
  if(hg->setup[plan.setup].setup<0){ // NonStd
    i_set=-hg->setup[plan.setup].setup-1;
    fprintf(fp, "     ins: { <<: *hds_nonstd%d_%dx%d", i_set+1,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }
  else{ // Std
    i_set=hg->setup[plan.setup].setup;
    fprintf(fp, "     ins: { <<: *hds_std%s_%dx%d", setups[i_set].initial,
	    hg->binning[hg->setup[plan.setup].binning].x,
	    hg->binning[hg->setup[plan.setup].binning].y);
  }

  if(plan.slit_or){
    fprintf(fp, ", slit_width: %d, slit_length: %d",
	    plan.slit_width, plan.slit_length);
  }
  else{
    fprintf(fp,", slit_width: 200");
  }
  
  fprintf(fp,", repeat: %d}\n", plan.repeat);
  
}




void usage(void)
{
  g_print(" hoe : HDS OPE file Editor   Ver"VERSION"\n");
  g_print("  [usage] %% hoe [-i input file] [-h]\n");
  g_print("     -h, --help               : Print this message\n");
  g_print("     -l, --large              : for large display fonts\n");
  g_print("     -i, --input input-file   : Set the inpout object list file\n");
  g_print("     -c, --config config-file : Load Config File\n");

  exit(0);
}


void get_option(int argc, char **argv, typHOE *hg)
{
  int i_opt;
  int valid=1;
  gchar *cwdname=NULL;

  hg->filename_read=NULL;
  debug_flg = 0;      /* -d オプションを付けると turn on する */

  
  i_opt = 1;
  while((i_opt < argc)&&(valid==1)) {
    if((strcmp(argv[i_opt],"-i") == 0)||
	    (strcmp(argv[i_opt],"--input") == 0)){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Worning: Could not get the current working directory.");
	  }
	  hg->filename_read=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hg->filename_read=g_strdup(argv[i_opt]);
	}
	hg->filehead=make_head(hg->filename_read);
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if((strcmp(argv[i_opt],"-c") == 0)||
	    (strcmp(argv[i_opt],"--config") == 0)){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Worning: Could not get the current working directory.");
	  }
	  hg->filename_hoe=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hg->filename_hoe=g_strdup(argv[i_opt]);
	}
	hg->filehead=make_head(hg->filename_hoe);
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if ((strcmp(argv[i_opt], "-h") == 0) ||
	     (strcmp(argv[i_opt], "--help") == 0)) {
      i_opt++;
      usage();
    }
    else if ((strcmp(argv[i_opt], "-d") == 0) ||
	     (strcmp(argv[i_opt], "--debug") == 0)) {
      debug_flg=1;
      i_opt++;
    }

  }
  
}


void WriteHOE(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *filename;
  gchar tmp[64],f_tmp[64], bname[128];
  int i_nonstd, i_set, i_list, i_line, i_plan, i_band;


  //filename = g_strconcat(g_get_home_dir(), "/save.hoe", NULL);
  filename = g_strdup(hg->filename_hoe);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // General 
  xmms_cfg_write_string(cfgfile, "General", "prog_ver",VERSION);
  if(hg->filename_write) xmms_cfg_write_string(cfgfile, "General", "OPE", hg->filename_write);
  if(hg->filename_read)  xmms_cfg_write_string(cfgfile, "General", "List",hg->filename_read);
  //xmms_cfg_write_boolean(cfgfile, "General", "PSFlag",hg->flag_bunnei);
  //xmms_cfg_write_boolean(cfgfile, "General", "SecZFlag",hg->flag_secz);
  //xmms_cfg_write_double(cfgfile, "General", "SecZFactor",hg->secz_factor);

  // Header
  xmms_cfg_write_int(cfgfile, "Header", "FromYear",hg->fr_year);
  xmms_cfg_write_int(cfgfile, "Header", "FromMonth",hg->fr_month);
  xmms_cfg_write_int(cfgfile, "Header", "FromDay",hg->fr_day);
  xmms_cfg_write_int(cfgfile, "Header", "Nights",hg->nights);
  xmms_cfg_write_string(cfgfile, "Header", "ID",hg->prop_id);
  if(hg->prop_pass)
    xmms_cfg_write_string(cfgfile, "Header", "Pass",hg->prop_pass);
  if(hg->observer)
    xmms_cfg_write_string(cfgfile, "Header", "Observer",hg->observer);
  xmms_cfg_write_int(cfgfile, "Header", "OCS",hg->ocs);


  // Default Parameter
  xmms_cfg_write_int(cfgfile, "DefPara", "Guide",hg->def_guide);
  xmms_cfg_write_double2(cfgfile, "DefPara", "PA",hg->def_pa, "%+6.2f");
  xmms_cfg_write_int(cfgfile, "DefPara", "ExpTime",(gint)hg->def_exp);


  // AD Calc.
  xmms_cfg_write_int(cfgfile, "ADC", "Wave1",(gint)hg->wave1);
  xmms_cfg_write_int(cfgfile, "ADC", "Wave0",(gint)hg->wave0);
  xmms_cfg_write_int(cfgfile, "ADC", "Pres",(gint)hg->pres);
  xmms_cfg_write_int(cfgfile, "ADC", "Temp",(gint)hg->temp);

  // AG
  xmms_cfg_write_int(cfgfile, "AG", "ExptimeFactor",(gint)hg->exptime_factor);
  xmms_cfg_write_int(cfgfile, "AG", "Brightness",(gint)hg->brightness);

  
  // SV
  xmms_cfg_write_int(cfgfile, "SV", "Area",(gint)hg->sv_area);
  xmms_cfg_write_int(cfgfile, "SV", "Integrate",(gint)hg->sv_integrate);
  xmms_cfg_write_int(cfgfile, "SV", "Acq",(gint)hg->sv_acq);
  xmms_cfg_write_int(cfgfile, "SV", "Region",(gint)hg->sv_region);
  xmms_cfg_write_int(cfgfile, "SV", "Calc",(gint)hg->sv_calc);
  xmms_cfg_write_int(cfgfile, "SV", "Exptime",(gint)hg->exptime_sv);
  xmms_cfg_write_double2(cfgfile, "SV", "SlitX",hg->sv_slitx, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "SlitY",hg->sv_slity, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "ISX",hg->sv_isx, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "ISY",hg->sv_isy, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "IS3X",hg->sv_is3x, "%5.1f");
  xmms_cfg_write_double2(cfgfile, "SV", "IS3Y",hg->sv_is3y, "%5.1f");


  // CameraZ
  xmms_cfg_write_int(cfgfile, "CameraZ", "Blue",(gint)hg->camz_b);
  xmms_cfg_write_int(cfgfile, "CameraZ", "Red",(gint)hg->camz_r);

  // Cross
  xmms_cfg_write_int(cfgfile, "Cross", "dCross",(gint)hg->d_cross);

  // NonStd
  for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
    sprintf(tmp,"NonStd-%d",i_nonstd+1);
    xmms_cfg_write_int(cfgfile, tmp, "Color",(gint)hg->nonstd[i_nonstd].col);
    xmms_cfg_write_int(cfgfile, tmp, "Cross",(gint)hg->nonstd[i_nonstd].cross);
    xmms_cfg_write_int(cfgfile, tmp, "Echelle",hg->nonstd[i_nonstd].echelle);
    xmms_cfg_write_int(cfgfile, tmp, "CamRot",hg->nonstd[i_nonstd].camr);
  }

  // Setup
  for(i_set=0;i_set<MAX_USESETUP;i_set++){
    sprintf(tmp,"SetUp-%d",i_set+1);
    xmms_cfg_write_int(cfgfile, tmp, "Setup",hg->setup[i_set].setup);
    xmms_cfg_write_boolean(cfgfile, tmp, "Use",hg->setup[i_set].use);
    xmms_cfg_write_int(cfgfile, tmp, "Binning",(gint)hg->setup[i_set].binning);
    xmms_cfg_write_int(cfgfile, tmp, "SlitWidth",(gint)hg->setup[i_set].slit_width);
    xmms_cfg_write_int(cfgfile, tmp, "SlitLength",(gint)hg->setup[i_set].slit_length);
    xmms_cfg_write_string(cfgfile, tmp, "Filter1",hg->setup[i_set].fil1);
    xmms_cfg_write_string(cfgfile, tmp, "Filter2",hg->setup[i_set].fil2);
    xmms_cfg_write_int(cfgfile, tmp, "ImR",hg->setup[i_set].imr);
    xmms_cfg_write_int(cfgfile, tmp, "IS",hg->setup[i_set].is);
    xmms_cfg_write_boolean(cfgfile, tmp, "I2",hg->setup[i_set].i2);
  }

  // Object List
  for(i_list=0;i_list<hg->i_max;i_list++){
    sprintf(tmp,"Obj-%d",i_list+1);
    xmms_cfg_write_string(cfgfile, tmp, "Name",hg->obj[i_list].name); 
    xmms_cfg_write_int(cfgfile, tmp, "ExpTime",hg->obj[i_list].exp);
    xmms_cfg_write_int(cfgfile, tmp, "Repeat",hg->obj[i_list].repeat);
    xmms_cfg_write_double2(cfgfile, tmp, "RA",hg->obj[i_list].ra,"%9.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "Dec",hg->obj[i_list].dec,"%+10.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "Epoch",hg->obj[i_list].equinox,"%7.2f");
    if(hg->obj[i_list].i_nst>=0){
      xmms_cfg_write_string(cfgfile, tmp, "NST_File",hg->nst[hg->obj[i_list].i_nst].filename); 
      xmms_cfg_write_int(cfgfile, tmp, "NST_Type",hg->nst[hg->obj[i_list].i_nst].type); 
    }
    else{
      xmms_cfg_remove_key(cfgfile,tmp, "NST_File");
      xmms_cfg_remove_key(cfgfile,tmp, "NST_Type");
    }
    xmms_cfg_write_double2(cfgfile, tmp, "Mag",hg->obj[i_list].mag,"%4.1f");
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_Used",hg->obj[i_list].magdb_used);
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_Band",hg->obj[i_list].magdb_band);
    xmms_cfg_write_double2(cfgfile, tmp, "PA",hg->obj[i_list].pa,"%+7.2f");
    xmms_cfg_write_int(cfgfile, tmp, "Guide",hg->obj[i_list].guide);
    if(hg->obj[i_list].note) xmms_cfg_write_string(cfgfile, tmp, "Note",hg->obj[i_list].note);
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_write_boolean(cfgfile, tmp, f_tmp,hg->obj[i_list].setup[i_set]);
    }
    if(hg->obj[i_list].gs.flag){
      xmms_cfg_write_string(cfgfile, tmp, "GS_Name",hg->obj[i_list].gs.name); 
      xmms_cfg_write_double2(cfgfile, tmp, "GS_RA",hg->obj[i_list].gs.ra,"%9.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Dec",hg->obj[i_list].gs.dec,"%+10.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Epoch",hg->obj[i_list].gs.equinox,"%7.2f");
      xmms_cfg_write_double2(cfgfile, tmp, "GS_Sep",hg->obj[i_list].gs.sep,"%.2f");
    }
    else{
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Name");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_RA");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Dec");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Epoch");
      xmms_cfg_remove_key(cfgfile,tmp, "GS_Sep");
    }
    
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_SIMBAD_Hits",hg->obj[i_list].magdb_simbad_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_Sep",hg->obj[i_list].magdb_simbad_sep,"%.6lf");
    if(hg->obj[i_list].magdb_simbad_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Name",hg->obj[i_list].magdb_simbad_name);
    if(hg->obj[i_list].magdb_simbad_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Type",hg->obj[i_list].magdb_simbad_type);
    if(hg->obj[i_list].magdb_simbad_sp)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_SIMBAD_Sp",hg->obj[i_list].magdb_simbad_sp);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_U",hg->obj[i_list].magdb_simbad_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_B",hg->obj[i_list].magdb_simbad_b,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_V",hg->obj[i_list].magdb_simbad_v,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_R",hg->obj[i_list].magdb_simbad_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_I",hg->obj[i_list].magdb_simbad_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_J",hg->obj[i_list].magdb_simbad_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_H",hg->obj[i_list].magdb_simbad_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SIMBAD_K",hg->obj[i_list].magdb_simbad_k,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_NED_Hits",   hg->obj[i_list].magdb_ned_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_NED_Sep",hg->obj[i_list].magdb_ned_sep,"%.6lf");
    if(hg->obj[i_list].magdb_ned_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Name",hg->obj[i_list].magdb_ned_name);
    if(hg->obj[i_list].magdb_ned_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Type",hg->obj[i_list].magdb_ned_type);
    if(hg->obj[i_list].magdb_ned_mag)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_NED_Mag", hg->obj[i_list].magdb_ned_mag);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_NED_Z",  hg->obj[i_list].magdb_ned_z,"%.6lf");
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_NED_Ref",   hg->obj[i_list].magdb_ned_ref);

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_LAMOST_Hits",   hg->obj[i_list].magdb_lamost_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_Sep",hg->obj[i_list].magdb_lamost_sep,"%.6lf");
    if(hg->obj[i_list].magdb_lamost_name)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Name",hg->obj[i_list].magdb_lamost_name);
    if(hg->obj[i_list].magdb_lamost_type)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Type",hg->obj[i_list].magdb_lamost_type);
    if(hg->obj[i_list].magdb_lamost_sp)
      xmms_cfg_write_string(cfgfile, tmp, "MagDB_LAMOST_Sp", hg->obj[i_list].magdb_lamost_sp);
    xmms_cfg_write_int(cfgfile, tmp, "MagDB_LAMOST_Ref",   hg->obj[i_list].magdb_lamost_ref);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_TEFF",  hg->obj[i_list].magdb_lamost_teff,"%.0lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_LOGG",  hg->obj[i_list].magdb_lamost_logg,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_FEH",  hg->obj[i_list].magdb_lamost_feh,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_LAMOST_HRV",  hg->obj[i_list].magdb_lamost_hrv,"%.1lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_GSC_Hits",hg->obj[i_list].magdb_gsc_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_Sep",hg->obj[i_list].magdb_gsc_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_U",hg->obj[i_list].magdb_gsc_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_B",hg->obj[i_list].magdb_gsc_b,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_V",hg->obj[i_list].magdb_gsc_v,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_R",hg->obj[i_list].magdb_gsc_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_I",hg->obj[i_list].magdb_gsc_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_J",hg->obj[i_list].magdb_gsc_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_H",hg->obj[i_list].magdb_gsc_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GSC_K",hg->obj[i_list].magdb_gsc_k,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_PS1_Hits",hg->obj[i_list].magdb_ps1_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Sep",hg->obj[i_list].magdb_ps1_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_G",hg->obj[i_list].magdb_ps1_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_R",hg->obj[i_list].magdb_ps1_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_I",hg->obj[i_list].magdb_ps1_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Z",hg->obj[i_list].magdb_ps1_z,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_PS1_Y",hg->obj[i_list].magdb_ps1_y,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_SDSS_Hits",hg->obj[i_list].magdb_sdss_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_Sep",hg->obj[i_list].magdb_sdss_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_U",hg->obj[i_list].magdb_sdss_u,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_G",hg->obj[i_list].magdb_sdss_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_R",hg->obj[i_list].magdb_sdss_r,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_I",hg->obj[i_list].magdb_sdss_i,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_SDSS_Z",hg->obj[i_list].magdb_sdss_z,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_GAIA_Hits",hg->obj[i_list].magdb_gaia_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_Sep",hg->obj[i_list].magdb_gaia_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_G",hg->obj[i_list].magdb_gaia_g,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_GAIA_P",hg->obj[i_list].magdb_gaia_p,"%.2lf");

    xmms_cfg_write_int(cfgfile, tmp, "MagDB_2MASS_Hits",hg->obj[i_list].magdb_2mass_hits);
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_Sep",hg->obj[i_list].magdb_2mass_sep,"%.6lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_J",hg->obj[i_list].magdb_2mass_j,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_H",hg->obj[i_list].magdb_2mass_h,"%.2lf");
    xmms_cfg_write_double2(cfgfile, tmp, "MagDB_2MASS_K",hg->obj[i_list].magdb_2mass_k,"%.2lf");

    xmms_cfg_write_int (cfgfile, tmp, "BandMax",  
			hg->obj[i_list].trdb_band_max);

    // Band
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      sprintf(bname, "Object-%d_Band-%d",i_list,i_band);
      if(hg->obj[i_list].trdb_mode[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Mode", 
			       hg->obj[i_list].trdb_mode[i_band]);
      if(hg->obj[i_list].trdb_band[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Band", 
			       hg->obj[i_list].trdb_band[i_band]);
      xmms_cfg_write_double2(cfgfile, bname, "Exp",
			     hg->obj[i_list].trdb_exp[i_band], "%.2f");
      xmms_cfg_write_int(cfgfile, bname, "Shot",
			 hg->obj[i_list].trdb_shot[i_band]);
    }
  }
  for(i_list=hg->i_max;i_list<MAX_OBJECT;i_list++){
    sprintf(tmp,"Obj-%d",i_list+1);
    xmms_cfg_remove_key(cfgfile,tmp, "Name");
    xmms_cfg_remove_key(cfgfile,tmp, "TGT");
    xmms_cfg_remove_key(cfgfile,tmp, "ExpTime");
    xmms_cfg_remove_key(cfgfile,tmp, "Repeat");
    xmms_cfg_remove_key(cfgfile,tmp, "RA");
    xmms_cfg_remove_key(cfgfile,tmp, "Dec");
    xmms_cfg_remove_key(cfgfile,tmp, "Epoch");
    xmms_cfg_remove_key(cfgfile,tmp, "PA");  
    xmms_cfg_remove_key(cfgfile,tmp, "Mag");  
    xmms_cfg_remove_key(cfgfile,tmp, "MagDB_Used");  
    xmms_cfg_remove_key(cfgfile,tmp, "MagDB_Band");  
    xmms_cfg_remove_key(cfgfile,tmp, "Guide");
    xmms_cfg_remove_key(cfgfile,tmp, "Note");
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_remove_key(cfgfile,tmp, f_tmp);
    }
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Name");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_RA");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Dec");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Epoch");
    xmms_cfg_remove_key(cfgfile,tmp, "GS_Sep");
  }


  // TRDB
  xmms_cfg_write_int(cfgfile, "TRDB", "Mode", hg->trdb_da);
  xmms_cfg_write_int(cfgfile, "TRDB", "Arcmin", hg->trdb_arcmin_used);

  // SMOKA
  xmms_cfg_write_int(cfgfile, "SMOKA", "Inst", hg->trdb_smoka_inst_used);
  if(hg->trdb_smoka_date_used)
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date_used);
  else
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Shot", hg->trdb_smoka_shot_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Imag", hg->trdb_smoka_imag_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Spec", hg->trdb_smoka_spec_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Ipol", hg->trdb_smoka_ipol_used);

  // HST
  xmms_cfg_write_int(cfgfile, "HST", "Mode", hg->trdb_hst_mode_used);
  if(hg->trdb_hst_date_used)
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date_used);
  else
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date);
  xmms_cfg_write_int(cfgfile, "HST", "Image", hg->trdb_hst_image_used);
  xmms_cfg_write_int(cfgfile, "HST", "Spec", hg->trdb_hst_spec_used);
  xmms_cfg_write_int(cfgfile, "HST", "Other", hg->trdb_hst_other_used);

  // ESO
  xmms_cfg_write_int(cfgfile, "ESO", "Mode", hg->trdb_eso_mode_used);
  if(hg->trdb_eso_stdate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate);
  if(hg->trdb_eso_eddate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate);
  xmms_cfg_write_int(cfgfile, "ESO", "Image", hg->trdb_eso_image_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Spec", hg->trdb_eso_spec_used);
  xmms_cfg_write_int(cfgfile, "ESO", "VLTI", hg->trdb_eso_vlti_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Pola", hg->trdb_eso_pola_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Coro", hg->trdb_eso_coro_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Other", hg->trdb_eso_other_used);
  xmms_cfg_write_int(cfgfile, "ESO", "SAM", hg->trdb_eso_sam_used);

  // Gemini
  xmms_cfg_write_int(cfgfile, "Gemini", "Inst", hg->trdb_gemini_inst_used);
  xmms_cfg_write_int(cfgfile, "Gemini", "Mode", hg->trdb_gemini_mode_used);
  if(hg->trdb_gemini_date_used)
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date_used);
  else
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date);


  // Line List
  for(i_line=0;i_line<MAX_LINE;i_line++){
    if(hg->line[i_line].name){
      sprintf(tmp,"Line-%d",i_line+1);
      xmms_cfg_write_string(cfgfile, tmp, "Name",hg->line[i_line].name);
      xmms_cfg_write_double2(cfgfile, tmp, "Wave",hg->line[i_line].wave,"%8.2f");
    }
  }

  // Obs Plan
  xmms_cfg_write_int(cfgfile, "Plan", "Start", hg->plan_start);
  xmms_cfg_write_int(cfgfile, "Plan", "StartHour", hg->plan_start_hour);
  xmms_cfg_write_int(cfgfile, "Plan", "StartMin", hg->plan_start_min);

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    sprintf(tmp,"Plan-%d",i_plan+1);
    xmms_cfg_write_int(cfgfile, tmp, "Type",hg->plan[i_plan].type);

    xmms_cfg_write_int(cfgfile, tmp, "Setup",hg->plan[i_plan].setup);
    xmms_cfg_write_int(cfgfile, tmp, "Repeat",hg->plan[i_plan].repeat);
    xmms_cfg_write_boolean(cfgfile, tmp, "Slit_or", hg->plan[i_plan].slit_or);
    xmms_cfg_write_int(cfgfile, tmp, "Slit_Width",hg->plan[i_plan].slit_width);
    xmms_cfg_write_int(cfgfile, tmp, "Slit_Length",hg->plan[i_plan].slit_length);

    xmms_cfg_write_int(cfgfile, tmp, "ObjI",hg->plan[i_plan].obj_i);
    xmms_cfg_write_int(cfgfile, tmp, "Exp",hg->plan[i_plan].exp);

    xmms_cfg_write_int(cfgfile, tmp, "Omode",hg->plan[i_plan].omode);
    xmms_cfg_write_int(cfgfile, tmp, "Guide",hg->plan[i_plan].guide);
    
    xmms_cfg_write_int(cfgfile, tmp, "FocusMode",hg->plan[i_plan].focus_mode);

    xmms_cfg_write_int(cfgfile, tmp, "Cmode",hg->plan[i_plan].cmode);

    xmms_cfg_write_int(cfgfile, tmp, "I2_pos", hg->plan[i_plan].i2_pos);

    xmms_cfg_write_double2(cfgfile, tmp, "SetAz", hg->plan[i_plan].setaz,"%6.2f");
    xmms_cfg_write_double2(cfgfile, tmp, "SetEl", hg->plan[i_plan].setel,"%6.2f");

    xmms_cfg_write_boolean(cfgfile, tmp, "Daytime", hg->plan[i_plan].daytime);

    if(hg->plan[i_plan].comment)
      xmms_cfg_write_string(cfgfile, tmp, "Comment",hg->plan[i_plan].comment);
    xmms_cfg_write_int(cfgfile, tmp, "Comtype",hg->plan[i_plan].comtype); 
    
    xmms_cfg_write_int(cfgfile, tmp, "Time",hg->plan[i_plan].time); 

    xmms_cfg_write_boolean(cfgfile, tmp, "PA_or", hg->plan[i_plan].pa_or);
    xmms_cfg_write_double2(cfgfile, tmp, "PA", hg->plan[i_plan].pa,"%6.2f");
    xmms_cfg_write_boolean(cfgfile, tmp, "SV_or", hg->plan[i_plan].sv_or);
    xmms_cfg_write_int(cfgfile, tmp, "SV_exp", hg->plan[i_plan].sv_exp);
    xmms_cfg_write_int(cfgfile, tmp, "SV_fil", hg->plan[i_plan].sv_fil);
    xmms_cfg_write_boolean(cfgfile, tmp, "BackUp", hg->plan[i_plan].backup);
  }

  for(i_plan=hg->i_plan_max;i_plan<MAX_PLAN;i_plan++){
    sprintf(tmp,"Plan-%d",i_plan+1);
    xmms_cfg_remove_key(cfgfile, tmp, "Type");

    xmms_cfg_remove_key(cfgfile, tmp, "Setup");
    xmms_cfg_remove_key(cfgfile, tmp, "Repeat");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_or");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_Width");
    xmms_cfg_remove_key(cfgfile, tmp, "Slit_Length");

    xmms_cfg_remove_key(cfgfile, tmp, "ObjI");
    xmms_cfg_remove_key(cfgfile, tmp, "Exp");

    xmms_cfg_remove_key(cfgfile, tmp, "Omode");
    xmms_cfg_remove_key(cfgfile, tmp, "Guide");
    
    xmms_cfg_remove_key(cfgfile, tmp, "FocusMode");

    xmms_cfg_remove_key(cfgfile, tmp, "Cmode");

    xmms_cfg_remove_key(cfgfile, tmp, "I2_pos");

    xmms_cfg_remove_key(cfgfile, tmp, "SetAz");
    xmms_cfg_remove_key(cfgfile, tmp, "SetEl");

    xmms_cfg_remove_key(cfgfile, tmp, "Daytime");

    xmms_cfg_remove_key(cfgfile, tmp, "Comment");
    xmms_cfg_remove_key(cfgfile, tmp, "Comtype");
    
    xmms_cfg_remove_key(cfgfile, tmp, "Time"); 

    xmms_cfg_remove_key(cfgfile, tmp, "PA_or"); 
    xmms_cfg_remove_key(cfgfile, tmp, "PA"); 
    xmms_cfg_remove_key(cfgfile, tmp, "SV_or"); 
    xmms_cfg_remove_key(cfgfile, tmp, "Sv_exp"); 
    xmms_cfg_remove_key(cfgfile, tmp, "SV_fil"); 
    xmms_cfg_remove_key(cfgfile, tmp, "BackUp"); 
  }


  xmms_cfg_write_file(cfgfile, filename);
  xmms_cfg_free(cfgfile);

}

void ReadHOE(typHOE *hg, gboolean destroy_flag)
{
  ConfigFile *cfgfile;
  gchar tmp[64], f_tmp[64], bname[64];
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_nonstd,i_set,i_list,i_line,i_plan,i_band, fcdb_type_tmp;

  cfgfile = xmms_cfg_open_file(hg->filename_hoe);

  hg->nst_max=0;
  
  if (cfgfile) {
    
    // General 
    if(xmms_cfg_read_string(cfgfile, "General", "OPE",  &c_buf)) hg->filename_write=c_buf;
    if(xmms_cfg_read_string(cfgfile, "General", "List", &c_buf)) hg->filename_read =c_buf;
    //if(xmms_cfg_read_boolean(cfgfile, "General", "PSFlag", &b_buf)) hg->flag_bunnei =b_buf;
    //else hg->flag_bunnei = FALSE;
    //if(xmms_cfg_read_boolean(cfgfile, "General", "SecZFlag", &b_buf)) hg->flag_secz =b_buf;
    //else hg->flag_secz = FALSE;
    //if(xmms_cfg_read_double(cfgfile, "General", "SecZFactor", &f_buf)) hg->secz_factor =f_buf;

    // Header
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromYear", &i_buf)) hg->fr_year =i_buf;
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromMonth",&i_buf)) hg->fr_month=i_buf;
    if(xmms_cfg_read_int   (cfgfile, "Header", "FromDay",  &i_buf)) hg->fr_day  =i_buf;
    if(xmms_cfg_read_int   (cfgfile, "Header", "Nights",   &i_buf)) hg->nights =i_buf;
    else
      hg->nights=1;
    if(xmms_cfg_read_string(cfgfile, "Header", "ID",       &c_buf)) hg->prop_id =c_buf;
    if(xmms_cfg_read_string(cfgfile, "Header", "Pass",       &c_buf)) hg->prop_pass =c_buf;
    if(xmms_cfg_read_string(cfgfile, "Header", "Observer",       &c_buf)) hg->observer =c_buf;
    if(xmms_cfg_read_int   (cfgfile, "Header", "OCS",       &i_buf)) hg->ocs=i_buf;


    // Default Parameter
    if(xmms_cfg_read_int  (cfgfile, "DefPara", "Guide",  &i_buf)) hg->def_guide=i_buf;
    if(xmms_cfg_read_double(cfgfile, "DefPara", "PA",     &f_buf)) hg->def_pa   =f_buf;
    if(xmms_cfg_read_int  (cfgfile, "DefPara", "ExpTime",&i_buf)) hg->def_exp  =i_buf;


    // AD Calc.
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave1",  &i_buf)) hg->wave1=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave0",  &i_buf)) hg->wave0=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Pres",   &i_buf)) hg->pres =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Temp",   &i_buf)) hg->temp =i_buf;


    // AG
    if(xmms_cfg_read_int  (cfgfile, "AG", "ExptimeFactor",  &i_buf)) hg->exptime_factor=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "AG", "Brightness",     &i_buf)) hg->brightness    =i_buf;

  
    // SV
    if(xmms_cfg_read_int  (cfgfile, "SV", "Area",       &i_buf)) hg->sv_area     =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Integrate",  &i_buf)) hg->sv_integrate=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Acq",  &i_buf)) hg->sv_acq=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Region",     &i_buf)) hg->sv_region   =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Calc",       &i_buf)) hg->sv_calc     =i_buf;
    if(xmms_cfg_read_int  (cfgfile, "SV", "Exptime",    &i_buf)) hg->exptime_sv  =i_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "SlitX",      &f_buf)) hg->sv_slitx    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "SlitY",      &f_buf)) hg->sv_slity    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "ISX",      &f_buf)) hg->sv_isx    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "ISY",      &f_buf)) hg->sv_isy    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "IS3X",      &f_buf)) hg->sv_is3x    =f_buf;
    if(xmms_cfg_read_double(cfgfile, "SV", "IS3Y",      &f_buf)) hg->sv_is3y    =f_buf;


    // CameraZ
    if(xmms_cfg_read_int  (cfgfile, "CameraZ", "Blue",   &i_buf)) hg->camz_b=i_buf;
    if(xmms_cfg_read_int  (cfgfile, "CameraZ", "Red",    &i_buf)) hg->camz_r=i_buf;
    
    //Cross
    if(xmms_cfg_read_int  (cfgfile, "Cross", "dCross",    &i_buf)) hg->d_cross=i_buf;


    // NonStd
    for(i_nonstd=0;i_nonstd<MAX_NONSTD;i_nonstd++){
      sprintf(tmp,"NonStd-%d",i_nonstd+1);
      if(xmms_cfg_read_int  (cfgfile, tmp, "Color",   &i_buf)) hg->nonstd[i_nonstd].col    =i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "Cross",   &i_buf)) hg->nonstd[i_nonstd].cross  =i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "Echelle", &i_buf)) hg->nonstd[i_nonstd].echelle=i_buf;
      if(xmms_cfg_read_int  (cfgfile, tmp, "CamRot",  &i_buf)) hg->nonstd[i_nonstd].camr   =i_buf;
    }

    
    // Setup
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(tmp,"SetUp-%d",i_set+1);
      if(xmms_cfg_read_int    (cfgfile, tmp, "Setup",     &i_buf)) hg->setup[i_set].setup      =i_buf;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "Use",       &b_buf)) hg->setup[i_set].use        =b_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Binning",   &i_buf)) hg->setup[i_set].binning    =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SlitWidth" ,&i_buf)) hg->setup[i_set].slit_width =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "SlitLength",&i_buf)) hg->setup[i_set].slit_length=i_buf;
      if(xmms_cfg_read_string (cfgfile, tmp, "Filter1",   &c_buf)) hg->setup[i_set].fil1       =c_buf;
      if(xmms_cfg_read_string (cfgfile, tmp, "Filter2",   &c_buf)) hg->setup[i_set].fil2       =c_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "ImR",       &i_buf)) hg->setup[i_set].imr        =i_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "IS",       &i_buf)) hg->setup[i_set].is        =i_buf;
      if(xmms_cfg_read_boolean(cfgfile, tmp, "I2",       &b_buf)) hg->setup[i_set].i2        =b_buf;
    }

    // Object List
    for(i_list=0;i_list<MAX_OBJECT;i_list++){
      sprintf(tmp,"Obj-%d",i_list+1);
      if(xmms_cfg_read_string (cfgfile, tmp, "Name",   &c_buf)) hg->obj[i_list].name  =c_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_int    (cfgfile, tmp, "ExpTime",&i_buf)) hg->obj[i_list].exp   =i_buf;
      else{
	hg->obj[i_list].exp=DEF_EXP;
      }
      if(xmms_cfg_read_int    (cfgfile, tmp, "Repeat",&i_buf))  hg->obj[i_list].repeat=i_buf;
      else{
	hg->obj[i_list].repeat=1;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "RA",     &f_buf)) hg->obj[i_list].ra    =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "Dec",    &f_buf)) hg->obj[i_list].dec   =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "Epoch",  &f_buf)) hg->obj[i_list].equinox =f_buf;
      else{
	hg->i_max=i_list;
	break;
      }
      ObjMagDB_Init(&hg->obj[i_list]);
      if(xmms_cfg_read_double  (cfgfile, tmp, "Mag",  &f_buf)){
	hg->obj[i_list].mag =f_buf;
	if(xmms_cfg_read_int  (cfgfile, tmp, "MagDB_Used",  &i_buf)){
	  hg->obj[i_list].magdb_used =i_buf;

	  if(xmms_cfg_read_int  (cfgfile, tmp, "MagDB_Band",  &i_buf)){
	    hg->obj[i_list].magdb_band =i_buf;
	  }
	  else{	
	    hg->obj[i_list].magdb_band=0;
	  }
	}
	else{
	  hg->obj[i_list].magdb_used=0;
	  hg->obj[i_list].magdb_band=0;
	}
      }
      else{
	hg->obj[i_list].mag=100;
	hg->obj[i_list].magdb_used=0;
	hg->obj[i_list].magdb_band=0;
      }

      hg->obj[i_list].snr=-1;
      hg->obj[i_list].sat=FALSE;

      // MagDB SIMBAD
      hg->obj[i_list].magdb_simbad_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_SIMBAD_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_simbad_hits>0){
	hg->obj[i_list].magdb_simbad_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_simbad_name =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Name",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_simbad_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Type",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_simbad_sp =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_SIMBAD_Sp",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_simbad_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_b =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_B",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_v =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_V",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_simbad_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SIMBAD_K",  &f_buf)) ? f_buf : 100;
      }

      // MagDB NED
      hg->obj[i_list].magdb_ned_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_NED_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_ned_hits>0){
	hg->obj[i_list].magdb_ned_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_NED_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_ned_name =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Name",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_ned_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Type",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_ned_mag =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_NED_Mag",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_ned_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_NED_Z",  &f_buf)) ? f_buf : -100;
	hg->obj[i_list].magdb_ned_ref =
	  (xmms_cfg_read_int(cfgfile, tmp, "MagDB_NED_REF",  &i_buf)) ? i_buf : 0;
      }

      // MagDB LAMOST
      hg->obj[i_list].magdb_lamost_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_LAMOST_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_lamost_hits>0){
	hg->obj[i_list].magdb_lamost_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_lamost_name =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Name",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_lamost_type =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Type",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_lamost_sp =
	  (xmms_cfg_read_string(cfgfile, tmp, "MagDB_LAMOST_Sp",  &c_buf)) ? c_buf : NULL;
	hg->obj[i_list].magdb_lamost_ref =
	  (xmms_cfg_read_int(cfgfile, tmp, "MagDB_LAMOST_REF",  &i_buf)) ? i_buf : 0;
	hg->obj[i_list].magdb_lamost_teff =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_TEFF",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_lamost_logg =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_LOGG",  &f_buf)) ? f_buf : -10;
	hg->obj[i_list].magdb_lamost_feh =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_FEH",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_lamost_hrv =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_LAMOST_HRV",  &f_buf)) ? f_buf : -99999;
      }

      // MagDB GSC
      hg->obj[i_list].magdb_gsc_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_GSC_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_gsc_hits>0){
	hg->obj[i_list].magdb_gsc_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gsc_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_b =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_B",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_v =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_V",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gsc_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GSC_K",  &f_buf)) ? f_buf : 100;
      }

      // MagDB PS1
      hg->obj[i_list].magdb_ps1_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_PS1_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_ps1_hits>0){
	hg->obj[i_list].magdb_ps1_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_ps1_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Z",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_ps1_y =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_PS1_Y",  &f_buf)) ? f_buf : 100;
      }

      // MagDB SDSS
      hg->obj[i_list].magdb_sdss_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_SDSS_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_sdss_hits>0){
	hg->obj[i_list].magdb_sdss_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_sdss_u =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_U",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_r =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_R",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_i =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_I",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_sdss_z =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_SDSS_Z",  &f_buf)) ? f_buf : 100;
      }

      // MagDB GAIA
      hg->obj[i_list].magdb_gaia_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_GAIA_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_gaia_hits>0){
	hg->obj[i_list].magdb_gaia_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_gaia_g =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_G",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_gaia_p =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_GAIA_P",  &f_buf)) ? f_buf : 100;
      }

      // MagDB 2MASS
      hg->obj[i_list].magdb_2mass_hits=
	(xmms_cfg_read_int    (cfgfile, tmp, "MagDB_2MASS_Hits",  &i_buf)) ? i_buf : -1;
      if(hg->obj[i_list].magdb_2mass_hits>0){
	hg->obj[i_list].magdb_2mass_sep =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_Sep",  &f_buf)) ? f_buf : -1;
	hg->obj[i_list].magdb_2mass_j =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_J",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_2mass_h =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_H",  &f_buf)) ? f_buf : 100;
	hg->obj[i_list].magdb_2mass_k =
	  (xmms_cfg_read_double(cfgfile, tmp, "MagDB_2MASS_K",  &f_buf)) ? f_buf : 100;
      }

      if(xmms_cfg_read_double  (cfgfile, tmp, "PA",     &f_buf)) hg->obj[i_list].pa    =f_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Guide",  &i_buf)) hg->obj[i_list].guide =i_buf;
      if(xmms_cfg_read_string (cfgfile, tmp, "Note",   &c_buf)) hg->obj[i_list].note  =c_buf;

      // NST
      if(xmms_cfg_read_string  (cfgfile, tmp, "NST_File",  &c_buf)){
	hg->nst[hg->nst_max].filename =c_buf;

	if(xmms_cfg_read_int  (cfgfile, tmp, "NST_Type",  &i_buf)){
	  hg->nst[hg->nst_max].type =i_buf;
	}
	else{
	  hg->nst[hg->nst_max].type =NST_TYPE_TSC;
	}
	
	hg->i_max=i_list;
	{
	  gboolean ret;
	  gchar *basename, *dirname;
	  
	  switch(hg->nst[hg->nst_max].type){
	  case NST_TYPE_TSC:
	    if(hg->filename_nst) g_free(hg->filename_nst);
	    hg->filename_nst=g_strdup(hg->nst[hg->nst_max].filename);
	    ret=MergeNST(hg);

	    if(!ret){
	      dirname=g_path_get_dirname(hg->filename_hoe);
	      basename=g_path_get_basename(hg->filename_nst);
	      if(hg->filename_nst) g_free(hg->filename_nst);
	      hg->filename_nst=g_strconcat(to_utf8(dirname),
					   G_DIR_SEPARATOR_S,
					   to_utf8(basename),
					   NULL);
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "dialog-information", 
#else
			    GTK_STOCK_DIALOG_INFO,
#endif
			    POPUP_TIMEOUT*1,
			    "Retrying to Load",
			    " ",
			    hg->filename_nst,
			    NULL);
	      ret=MergeNST(hg);

	      if(dirname) g_free(dirname);
	      if(basename) g_free(basename);
	    }
	    if(ret){
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "network-transmit", 
#else
			    GTK_STOCK_OK, 
#endif
			    POPUP_TIMEOUT*1,
			    "Succeeded to Load Non-Sidereal Tracking File",
			    " ",
			    hg->filename_nst,
			    NULL);
	    }
	    break;

	  case NST_TYPE_JPL:
	    if(hg->filename_jpl) g_free(hg->filename_jpl);
	    hg->filename_jpl=g_strdup(hg->nst[hg->nst_max].filename);
	    ret=MergeJPL(hg);

	    if(!ret){
	      dirname=g_path_get_dirname(hg->filename_hoe);
	      basename=g_path_get_basename(hg->filename_jpl);
	      if(hg->filename_jpl) g_free(hg->filename_jpl);
	      hg->filename_jpl=g_strconcat(to_utf8(dirname),
					   G_DIR_SEPARATOR_S,
					   to_utf8(basename),
					   NULL);
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "dialog-information", 
#else
			    GTK_STOCK_DIALOG_INFO, 
#endif
			    POPUP_TIMEOUT*1,
			    "Retrying to Load",
			    " ",
			    hg->filename_jpl,
			    NULL);
	      ret=MergeJPL(hg);

	      if(dirname) g_free(dirname);
	      if(basename) g_free(basename);
	    }
	    if(ret){
	      popup_message(hg->w_top, 
#ifdef USE_GTK3
			    "document-open", 
#else
			    GTK_STOCK_OK, 
#endif
			    POPUP_TIMEOUT*1,
			    "Succeeded to Load Non-Sidereal Tracking File",
			    " ",
			    hg->filename_jpl,
			    NULL);
	      }
	    break;
	  }

	  if(ret){
	    if(hg->skymon_mode==SKYMON_SET){
	      calcpa2_skymon(hg);
	    }
	    else{
	      calcpa2_main(hg);
	    }
	  }
	  else{
	    hg->obj[i_list].i_nst=-1;
	  }
	}
      }
      else{
	hg->obj[i_list].i_nst=-1;
      }
      
      for(i_set=0;i_set<MAX_USESETUP;i_set++){
	sprintf(f_tmp,"SetUp-%d",i_set+1);
	if(xmms_cfg_read_boolean(cfgfile, tmp, f_tmp,  &b_buf)) hg->obj[i_list].setup[i_set]=b_buf;
      }

      if(xmms_cfg_read_string (cfgfile, tmp, "GS_Name", &c_buf)){
	hg->obj[i_list].gs.name=c_buf;
	hg->obj[i_list].gs.flag=TRUE;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_RA",   &f_buf)) hg->obj[i_list].gs.ra=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Dec",   &f_buf)) hg->obj[i_list].gs.dec=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Epoch",   &f_buf)) hg->obj[i_list].gs.equinox=f_buf;
	if(xmms_cfg_read_double    (cfgfile, tmp, "GS_Sep",   &f_buf)) hg->obj[i_list].gs.equinox=f_buf;
      }
      else{
	hg->obj[i_list].gs.flag=FALSE;
      }
      
      if(xmms_cfg_read_int  (cfgfile, tmp, "BandMax",  &i_buf))
	hg->obj[i_list].trdb_band_max=i_buf;
      else
	hg->obj[i_list].trdb_band_max=0;
	
      // Band
      for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
	sprintf(bname, "Object-%d_Band-%d",i_list,i_band);

	if(hg->obj[i_list].trdb_mode[i_band]) 
	  g_free(hg->obj[i_list].trdb_mode[i_band]);
	if(xmms_cfg_read_string  (cfgfile, bname, "Mode",  &c_buf))
	  hg->obj[i_list].trdb_mode[i_band]=c_buf;
	else
	  hg->obj[i_list].trdb_mode[i_band]=NULL;

	if(hg->obj[i_list].trdb_band[i_band]) 
	  g_free(hg->obj[i_list].trdb_band[i_band]);
	if(xmms_cfg_read_string  (cfgfile, bname, "Band",  &c_buf))
	  hg->obj[i_list].trdb_band[i_band]=c_buf;
	else
	  hg->obj[i_list].trdb_band[i_band]=NULL;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;
	
	if(xmms_cfg_read_int  (cfgfile, bname, "Shot",  &i_buf))
	  hg->obj[i_list].trdb_shot[i_band]=i_buf;
	else
	  hg->obj[i_list].trdb_shot[i_band]=0;
      }
      make_band_str(hg, i_list, hg->trdb_da);
    }

    // TRDB
    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Mode",  &i_buf))
      hg->trdb_da=i_buf;
    else
      hg->trdb_da=TRDB_TYPE_SMOKA;

    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Arcmin",  &i_buf))
      hg->trdb_arcmin_used=i_buf;
    else
      hg->trdb_arcmin_used=2;
    hg->trdb_arcmin=hg->trdb_arcmin_used;

    // SMOKA
    if(xmms_cfg_read_int  (cfgfile, "SMOKA", "Inst",  &i_buf))
      hg->trdb_smoka_inst_used=i_buf;
    else
      hg->trdb_smoka_inst_used=0;
    hg->trdb_smoka_inst=hg->trdb_smoka_inst_used;

    if(hg->trdb_smoka_date_used) g_free(hg->trdb_smoka_date_used);
    if(xmms_cfg_read_string(cfgfile, "SMOKA", "Date", &c_buf)) 
      hg->trdb_smoka_date_used =c_buf;
    else
      hg->trdb_smoka_date_used=g_strdup_printf("1998-01-01..%d-%02d-%02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_smoka_date) g_free(hg->trdb_smoka_date);
    hg->trdb_smoka_date=g_strdup(hg->trdb_smoka_date_used);

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Shot", &b_buf))
      hg->trdb_smoka_shot_used =b_buf;
    else
      hg->trdb_smoka_shot_used =TRUE;
    hg->trdb_smoka_shot=hg->trdb_smoka_shot_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Imag", &b_buf))
      hg->trdb_smoka_imag_used =b_buf;
    else
      hg->trdb_smoka_imag_used =TRUE;
    hg->trdb_smoka_imag=hg->trdb_smoka_imag_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Spec", &b_buf))
      hg->trdb_smoka_spec_used =b_buf;
    else
      hg->trdb_smoka_spec_used =TRUE;
    hg->trdb_smoka_spec=hg->trdb_smoka_spec_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Ipol", &b_buf))
      hg->trdb_smoka_ipol_used =b_buf;
    else
      hg->trdb_smoka_ipol_used =TRUE;
    hg->trdb_smoka_ipol=hg->trdb_smoka_ipol_used;

    // HST
    if(xmms_cfg_read_int  (cfgfile, "HST", "Mode",  &i_buf))
      hg->trdb_hst_mode_used=i_buf;
    else
      hg->trdb_hst_mode_used=0;
    hg->trdb_hst_mode=hg->trdb_hst_mode_used;

    if(hg->trdb_hst_date_used) g_free(hg->trdb_hst_date_used);
    if(xmms_cfg_read_string(cfgfile, "HST", "Date", &c_buf)) 
      hg->trdb_hst_date_used =c_buf;
    else
      hg->trdb_hst_date_used=g_strdup_printf("1990-01-01..%d-%02d-%02d",
					     hg->skymon_year,
					     hg->skymon_month,
					     hg->skymon_day);
    if(hg->trdb_hst_date) g_free(hg->trdb_hst_date);
    hg->trdb_hst_date=g_strdup(hg->trdb_hst_date_used);

    if(xmms_cfg_read_int  (cfgfile, "HST", "Image",  &i_buf))
      hg->trdb_hst_image_used=i_buf;
    else
      hg->trdb_hst_image_used=0;
    hg->trdb_hst_image=hg->trdb_hst_image_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Spec",  &i_buf))
      hg->trdb_hst_spec_used=i_buf;
    else
      hg->trdb_hst_spec_used=0;
    hg->trdb_hst_spec=hg->trdb_hst_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Other",  &i_buf))
      hg->trdb_hst_other_used=i_buf;
    else
      hg->trdb_hst_other_used=0;
    hg->trdb_hst_other=hg->trdb_hst_other_used;

    // ESO
    if(xmms_cfg_read_int  (cfgfile, "ESO", "Mode",  &i_buf))
      hg->trdb_eso_mode_used=i_buf;
    else
      hg->trdb_eso_mode_used=0;
    hg->trdb_eso_mode=hg->trdb_eso_mode_used;

    if(hg->trdb_eso_stdate_used) g_free(hg->trdb_eso_stdate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "StDate", &c_buf)) 
      hg->trdb_eso_stdate_used =c_buf;
    else
      hg->trdb_eso_stdate_used=g_strdup("1980 01 01");
    if(hg->trdb_eso_stdate) g_free(hg->trdb_eso_stdate);
    hg->trdb_eso_stdate=g_strdup(hg->trdb_eso_stdate_used);

    if(hg->trdb_eso_eddate_used) g_free(hg->trdb_eso_eddate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "EdDate", &c_buf)) 
      hg->trdb_eso_eddate_used =c_buf;
    else
      hg->trdb_eso_eddate_used=g_strdup_printf("%4d %02d %02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_eso_eddate) g_free(hg->trdb_eso_eddate);
    hg->trdb_eso_eddate=g_strdup(hg->trdb_eso_eddate_used);

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Image",  &i_buf))
      hg->trdb_eso_image_used=i_buf;
    else
      hg->trdb_eso_image_used=0;
    hg->trdb_eso_image=hg->trdb_eso_image_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Spec",  &i_buf))
      hg->trdb_eso_spec_used=i_buf;
    else
      hg->trdb_eso_spec_used=0;
    hg->trdb_eso_spec=hg->trdb_eso_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "VLTI",  &i_buf))
      hg->trdb_eso_vlti_used=i_buf;
    else
      hg->trdb_eso_vlti_used=0;
    hg->trdb_eso_vlti=hg->trdb_eso_vlti_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Pola",  &i_buf))
      hg->trdb_eso_pola_used=i_buf;
    else
      hg->trdb_eso_pola_used=0;
    hg->trdb_eso_pola=hg->trdb_eso_pola_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Coro",  &i_buf))
      hg->trdb_eso_coro_used=i_buf;
    else
      hg->trdb_eso_coro_used=0;
    hg->trdb_eso_coro=hg->trdb_eso_coro_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Other",  &i_buf))
      hg->trdb_eso_other_used=i_buf;
    else
      hg->trdb_eso_other_used=0;
    hg->trdb_eso_other=hg->trdb_eso_other_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "SAM",  &i_buf))
      hg->trdb_eso_sam_used=i_buf;
    else
      hg->trdb_eso_sam_used=0;
    hg->trdb_eso_sam=hg->trdb_eso_sam_used;

    // Gemini
    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Inst",  &i_buf))
      hg->trdb_gemini_inst_used=i_buf;
    else
      hg->trdb_gemini_inst_used=GEMINI_INST_GMOS;
    hg->trdb_gemini_inst=hg->trdb_gemini_inst_used;

    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Mode",  &i_buf))
      hg->trdb_gemini_mode_used=i_buf;
    else
      hg->trdb_gemini_mode_used=0;
    hg->trdb_gemini_mode=hg->trdb_gemini_mode_used;

    if(hg->trdb_gemini_date_used) g_free(hg->trdb_gemini_date_used);
    if(xmms_cfg_read_string(cfgfile, "Gemini", "Date", &c_buf)) 
      hg->trdb_gemini_date_used =c_buf;
    else
      hg->trdb_gemini_date_used=g_strdup_printf("19980101-%4d%02d%02d",
						hg->skymon_year,
						hg->skymon_month,
						hg->skymon_day);
    if(hg->trdb_gemini_date) g_free(hg->trdb_gemini_date);
    hg->trdb_gemini_date=g_strdup(hg->trdb_gemini_date_used);


    // Line List
    for(i_line=0;i_line<MAX_LINE;i_line++){
      sprintf(tmp,"Line-%d",i_line+1);
      if(xmms_cfg_read_string (cfgfile, tmp, "Name",   &c_buf)) hg->line[i_line].name=c_buf;
      if(xmms_cfg_read_double    (cfgfile, tmp, "Wave",   &f_buf)) hg->line[i_line].wave=f_buf;
    }


    // Obs Plan
    if(xmms_cfg_read_int(cfgfile, "Plan","Start",&i_buf)) hg->plan_start =i_buf;
    if(xmms_cfg_read_int(cfgfile, "Plan","StartHour",&i_buf)) hg->plan_start_hour =i_buf;
    if(xmms_cfg_read_int(cfgfile, "Plan","StartMin",&i_buf)) hg->plan_start_min =i_buf;

    for(i_plan=0;i_plan<MAX_PLAN;i_plan++){
      sprintf(tmp,"Plan-%d",i_plan+1);
      if(xmms_cfg_read_int    (cfgfile, tmp, "Type",  &i_buf)) hg->plan[i_plan].type =i_buf;
      else{
	hg->i_plan_max=i_plan;
	break;
      }
      switch(hg->plan[i_plan].type){
      case PLAN_TYPE_COMMENT:
      case PLAN_TYPE_FOCUS:
      case PLAN_TYPE_I2:
      case PLAN_TYPE_SetAzEl:
	hg->plan[i_plan].setup=-1;
	break;

      default:
	if(xmms_cfg_read_int    (cfgfile, tmp, "Setup", &i_buf)) hg->plan[i_plan].setup  =i_buf;
	else hg->plan[i_plan].setup=-1;
	break;
      }
      if(xmms_cfg_read_int    (cfgfile, tmp, "Repeat",&i_buf)) hg->plan[i_plan].repeat =i_buf;
      else hg->plan[i_plan].repeat=1;
      
      if(xmms_cfg_read_boolean(cfgfile, tmp, "Slit_or",&b_buf))hg->plan[i_plan].slit_or=b_buf;
      else hg->plan[i_plan].slit_or=FALSE;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Slit_Width",&i_buf)) hg->plan[i_plan].slit_width =i_buf;
      else hg->plan[i_plan].slit_width=200;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Slit_Length",&i_buf)) hg->plan[i_plan].slit_length =i_buf;
      else hg->plan[i_plan].slit_length=2000;

      if(xmms_cfg_read_int    (cfgfile, tmp, "ObjI",     &i_buf)) hg->plan[i_plan].obj_i     =i_buf;
      else hg->plan[i_plan].exp=DEF_EXP;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Exp",     &i_buf)) hg->plan[i_plan].exp     =i_buf;
      else hg->plan[i_plan].exp=DEF_EXP;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Omode",   &i_buf)) hg->plan[i_plan].omode   =i_buf;
      else hg->plan[i_plan].omode=0;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Guide",   &i_buf)) hg->plan[i_plan].guide   =i_buf;
      else hg->plan[i_plan].guide=SV_GUIDE;

      if(xmms_cfg_read_int    (cfgfile, tmp, "FocusMode",   &i_buf)) hg->plan[i_plan].focus_mode   =i_buf;
      else hg->plan[i_plan].focus_mode=0;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Cmode",   &i_buf)) hg->plan[i_plan].cmode   =i_buf;
      else hg->plan[i_plan].cmode=0;

      if(xmms_cfg_read_int(cfgfile, tmp, "I2_pos",  &i_buf)) hg->plan[i_plan].i2_pos  =i_buf;
      else hg->plan[i_plan].i2_pos=PLAN_I2_IN;

      if(xmms_cfg_read_double    (cfgfile, tmp, "SetAz",   &f_buf)) hg->plan[i_plan].setaz=f_buf;
      else hg->plan[i_plan].setaz=-90.;

      if(xmms_cfg_read_double    (cfgfile, tmp, "SetEl",   &f_buf)) hg->plan[i_plan].setel=f_buf;
      else hg->plan[i_plan].setel=90.;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "Daytime", &b_buf)) hg->plan[i_plan].daytime =b_buf;
      else hg->plan[i_plan].daytime=FALSE;

      if(xmms_cfg_read_string (cfgfile, tmp, "Comment", &c_buf)) hg->plan[i_plan].comment =c_buf;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Comtype", &i_buf)) hg->plan[i_plan].comtype   =i_buf;
      else hg->plan[i_plan].comtype=PLAN_COMMENT_TEXT;

      if(xmms_cfg_read_int    (cfgfile, tmp, "Time",    &i_buf)) hg->plan[i_plan].time    =i_buf;
      else hg->plan[i_plan].time=0;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "PA_or", &b_buf)) hg->plan[i_plan].pa_or =b_buf;
      else hg->plan[i_plan].pa_or=FALSE;

      if(xmms_cfg_read_double    (cfgfile, tmp, "PA",   &f_buf)) hg->plan[i_plan].pa=f_buf;
      else hg->plan[i_plan].pa=0;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "SV_or", &b_buf)) hg->plan[i_plan].sv_or =b_buf;
      else hg->plan[i_plan].sv_or=FALSE;

      if(xmms_cfg_read_int    (cfgfile, tmp, "SV_exp", &i_buf)) hg->plan[i_plan].sv_exp   =i_buf;
      else hg->plan[i_plan].sv_exp=hg->exptime_sv;

      if(xmms_cfg_read_int    (cfgfile, tmp, "SV_fil", &i_buf)) hg->plan[i_plan].sv_fil   =i_buf;
      else hg->plan[i_plan].sv_fil=SV_FILTER_NONE;

      if(xmms_cfg_read_boolean(cfgfile, tmp, "BackUp", &b_buf)) hg->plan[i_plan].backup =b_buf;
      else hg->plan[i_plan].backup=FALSE;


      if(hg->plan[i_plan].txt) g_free(hg->plan[i_plan].txt);
      hg->plan[i_plan].txt = make_plan_txt(hg,hg->plan[i_plan]);
    }

    xmms_cfg_free(cfgfile);
  }

  calc_rst(hg);
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=hg->trdb_da;
  trdb_make_tree(hg);
  rebuild_trdb_tree(hg);
  hg->fcdb_type=fcdb_type_tmp;

  if(destroy_flag){
    gtk_widget_destroy(hg->all_note);

    flag_make_obj_tree=FALSE;
    flag_make_line_tree=FALSE;

    make_note(hg);
    ////flag_make_obj_list=FALSE;
    ////make_obj_list(hg,TRUE);
    //make_obj_tree(hg);
  }

}

gboolean is_number(GtkWidget *parent, gchar *s, gint line, const gchar* sect){
  gchar* msg;

  if(!s){
    msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"", line, sect);
    popup_message(parent, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Error: Input File is invalid.",
		  " ",
		  msg,
		  NULL);
 
    g_free(msg);
    return FALSE;
  }

  while(*s!='\0'){
    if(!is_num_char(*s)){
      msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"\n Irregal character code : \"%02x\"", 
			  line, sect,*s);
      popup_message(parent, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    "Error: Input File is invalid.",
		    " ",
		    msg,
		    NULL);
      
      g_free(msg);
      return FALSE;
    }
    s++;
  }
  return TRUE;
}

void clip_copy(GtkWidget *widget, gpointer gdata){
  GtkWidget *entry;
  GtkClipboard* clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  const gchar *c;

  entry=(GtkWidget *)gdata;

  c = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_clipboard_set_text (clipboard, c, strlen(c));
}

gchar *strip_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_str=0,i;

  tgt_name=g_strdup(obj_name);
  for(i=0;i<strlen(tgt_name);i++){
    if((obj_name[i]!=0x20)
       &&(obj_name[i]!=0x0A)
       &&(obj_name[i]!=0x0D)
       &&(obj_name[i]!=0x09)){
      tgt_name[i_str]=obj_name[i];
      i_str++;
    }
  }
  tgt_name[i_str]='\0';
  
  ret_name=g_strdup(tgt_name);
  if(tgt_name) g_free(tgt_name);
  return(ret_name);
}

gchar* to_utf8(gchar *input){
  return(g_locale_to_utf8(input,-1,NULL,NULL,NULL));
}

gchar* to_locale(gchar *input){
#ifdef USE_WIN32
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
  return(g_win32_locale_filename_from_utf8(input));
#else
  return(g_locale_from_utf8(input,-1,NULL,NULL,NULL));
#endif
}

void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  va_start(args, delay);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
  }

  my_signal_connect(dialog,"delete-event",destroy_popup, &timer);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtk_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.0);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);
  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_main();
}

void delete_disp_para(GtkWidget *w, GdkEvent *event, GtkWidget *dialog)
{
  close_disp_para(w,dialog);
}

void close_disp_para(GtkWidget *w, GtkWidget *dialog)
{
  gtk_main_quit();
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

void default_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=-1;
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}


void change_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=1;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}


gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  return(FALSE);
}

static void destroy_popup(GtkWidget *w, GdkEvent *event, gint *data)
{
  g_source_remove(*data);
  gtk_main_quit();
}


void my_file_chooser_add_filter (GtkWidget *dialog, 
				 const gchar *name,
				 ...)
{
  GtkFileFilter *filter;
  gchar *name_tmp;
  va_list args;
  gchar *pattern, *ptncat=NULL, *ptncat2=NULL;

  filter=gtk_file_filter_new();

  va_start(args, name);
  while(1){
    pattern=va_arg(args, gchar*);
    if(!pattern) break;
    gtk_file_filter_add_pattern(filter, pattern);
    if(!ptncat){
      ptncat=g_strdup(pattern);
    }
    else{
      if(ptncat2) g_free(ptncat2);
      ptncat2=g_strdup(ptncat);
      if(ptncat) g_free(ptncat);
      ptncat=g_strconcat(ptncat2,",",pattern,NULL);
    }
  }
  va_end(args);

  name_tmp=g_strconcat(name," [",ptncat,"]",NULL);
  gtk_file_filter_set_name(filter, name_tmp);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
  if(name_tmp) g_free(name_tmp);
  if(ptncat) g_free(ptncat);
  if(ptncat2) g_free(ptncat2);
}


void my_signal_connect(GtkWidget *widget, 
		       const gchar *detailed_signal,
		       void *func,
		       gpointer data)
{
  g_signal_connect(G_OBJECT(widget),
		   detailed_signal,
		   G_CALLBACK(func),
		   data);
}


void my_entry_set_width_chars(GtkEntry *entry, guint n){
  gtk_entry_set_width_chars(entry, n);
}


gchar* make_head(gchar* filename){
  gchar *fname, *p;

  p=strrchr(filename,'.');
  fname=g_strndup(filename,strlen(filename)-strlen(p));
  return(fname);
}

GtkWidget* gtkut_hbox_new(gboolean homogeneous, gint spacing){
  GtkWidget *w;
#ifdef USE_GTK3
  w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
#else
  w = gtk_hbox_new(homogeneous, spacing);
#endif
  return(w);
}

GtkWidget* gtkut_vbox_new(gboolean homogeneous, gint spacing){
  GtkWidget *w;
#ifdef USE_GTK3
  w = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
#else
  w = gtk_vbox_new(homogeneous, spacing);
#endif
  return(w);
}

#ifdef USE_GTK3
GtkWidget* gtkut_image_menu_item_new_with_label(GtkWidget *icon,
						const gchar *txt){
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  GtkWidget *label = gtk_label_new (txt);
  GtkWidget *menu_item = gtk_menu_item_new ();

  gtk_container_add (GTK_CONTAINER (box), icon);
  gtk_container_add (GTK_CONTAINER (box), label);

  gtk_container_add (GTK_CONTAINER (menu_item), box);
  
  gtk_widget_show_all (menu_item);
  return(menu_item);
}
#endif


#ifdef USE_GTK3
GtkWidget* gtkut_button_new_from_icon_name(gchar *txt,
					   const gchar *stock){
#else
GtkWidget* gtkut_button_new_from_stock(gchar *txt,
				       const gchar *stock){
#endif
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_BUTTON);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_MENU);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);
  
  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}
 
#ifdef USE_GTK3
GtkWidget* gtkut_toggle_button_new_from_icon_name(gchar *txt,
						   const gchar *stock){
#else
GtkWidget* gtkut_toggle_button_new_from_stock(gchar *txt,
					      const gchar *stock){
#endif
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_BUTTON);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name (stock, GTK_ICON_SIZE_MENU);
#else
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
#endif
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
  gtk_widget_show(button);
  return(button);
}

GtkWidget* gtkut_toggle_button_new_from_pixbuf(gchar *txt,
					      GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  gint w,h;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
   gtk_widget_show(button);
  return(button);
}


#ifdef USE_WIN32
gchar* WindowsVersion()
{
  // Get OS Info for WinXP and 2000 or later
  // for Win9x, OSVERSIONINFO should be used instead of OSVERSIONINFOEX
  OSVERSIONINFOEX osInfo;
  gchar *windowsName;
  static gchar buf[1024];

  windowsName = NULL;
  
  osInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);

  GetVersionEx ((LPOSVERSIONINFO)&osInfo);

  switch (osInfo.dwMajorVersion)
  {
  case 4:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT){
	  windowsName = g_strdup("Windows NT 4.0");
	}
	else{
	  windowsName = g_strdup("Windows 95");
	}
	break;

      case 10:
	windowsName = g_strdup("Windows 98");
	break;

      case 90:
	windowsName = g_strdup("Windows Me");
	break;
      }
    break;

  case 5:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	windowsName = g_strdup("Windows 2000");
	break;
	    
      case 1:
	windowsName = g_strdup("Windows XP");
	break;
	
      case 2:
	windowsName = g_strdup("Windows Server 2003");
	break;

      }
    break;

  case 6:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows Vista");
	else
	  windowsName = g_strdup("Windows Server 2008");
	break;
	
      case 1:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 7");
	else
	  windowsName = g_strdup("Windows Server 2008 R2");
	break;

      case 2:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8");
	else
	  windowsName = g_strdup("Windows Server 2012");
	break;

      case 3:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8.1");
	else
	  windowsName = g_strdup("Windows Server 2012 R2");
	break;
      }
    break;

  case 10:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 10");
	else
	  windowsName = g_strdup("Windows Server 2016");
	break;
      }	
    break;
  }

  if(!windowsName) windowsName = g_strdup("Windows UNKNOWN");
  
  //OutPut
  if(osInfo.wServicePackMajor!=0){
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s w/SP%d (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.wServicePackMajor,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  else{
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  g_free(windowsName);

  return(buf);
}
#endif


void calc_rst(typHOE *hg){
  gint i_list;
  gdouble JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn oequ;
  struct ln_zonedate zonedate;
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  double a0,d0;
  struct ln_zonedate rst_date;
  struct ln_rst_time rst;

  zonedate.years=hg->fr_year;
  zonedate.months=hg->fr_month;
  zonedate.days=hg->fr_day+1;
  zonedate.hours=0;
  zonedate.minutes=0;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)(hg->obs_timezone*60);
    
  JD = ln_get_julian_local_date(&zonedate);

  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;



  for (i_list=0; i_list<hg->i_max; i_list++){
    a0s=hg->obj[i_list].ra;
    ia0h=(int)(a0s/10000);
    a0s=a0s-(double)(ia0h)*10000;
    ia0m=(int)(a0s/100);
    a0s=a0s-(double)(ia0m)*100;
    
    d0s=hg->obj[i_list].dec;
    id0d=(int)(d0s/10000);
    d0s=d0s-(double)(id0d)*10000;
    id0m=(int)(d0s/100);
    d0s=d0s-(double)(id0m)*100;
	
    a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
    d0=id0d + id0m/60. + d0s/3600.;  //[deg]
	
    oequ.ra=a0*360/24;
    oequ.dec=d0;

    hg->obj[i_list].horizon
      =ln_get_object_rst_horizon (JD, &observer, &oequ, 15.0, &rst);

    hg->obj[i_list].rise=rst.rise;
    hg->obj[i_list].transit=rst.transit;
    hg->obj[i_list].set=rst.set;

  }
}

void UpdateTotalExp(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  get_total_basic_exp(hg);
}


void ver_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gint major=0, minor=0, micro=0;
  gboolean update_flag=FALSE;
  gint c_major, c_minor, c_micro;
  gchar *tmp;
  

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }

  c_major=g_strtod(MAJOR_VERSION,NULL);
  c_minor=g_strtod(MINOR_VERSION,NULL);
  c_micro=g_strtod(MICRO_VERSION,NULL);
  
  while((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,",");
    
    if(strncmp(tmp_char,"MAJOR",strlen("MAJOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	major=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MINOR",strlen("MINOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	minor=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MICRO",strlen("MICRO"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	micro=g_strtod(tmp_p,NULL);
      }
    }
  }
  fclose(fp);

  if(major>c_major){
    update_flag=TRUE;
  }
  else if(major==c_major){
    if(minor>c_minor){
      update_flag=TRUE;
    }
    else if(minor==c_minor){
      if(micro>c_micro){
	update_flag=TRUE;
      }
    }
  }

  if(update_flag){
    GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

    flagChildDialog=TRUE;
  
    dialog = gtk_dialog_new_with_buttons("HOE : Download the latest version?",
					 GTK_WINDOW(hg->w_top),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_Cancel",GTK_RESPONSE_CANCEL,
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);

    my_signal_connect(dialog, "destroy", NULL,NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							     GTK_RESPONSE_OK));


    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       hbox,FALSE, FALSE, 0);
    
#ifdef USE_GTK3
    pixmap=gtk_image_new_from_icon_name ("dialog-question",
					 GTK_ICON_SIZE_DIALOG);
#else
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				     GTK_ICON_SIZE_DIALOG);
#endif
    
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);
    
    vbox = gtkut_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

    tmp=g_strdup_printf("The current version : ver. %d.%d.%d",
			c_major,c_minor,c_micro);
    label = gtk_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);

    tmp=g_strdup_printf("The latest version  : ver. %d.%d.%d",
			major,minor,micro);
    label = gtk_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);
    

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("Do you go to the web page to download the latest version?");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, 0.5);
    gtk_widget_set_valign (label, 0.5);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      uri_clicked(NULL, (gpointer)hg);
    }
    gtk_widget_destroy(dialog);
    
    flagChildDialog=FALSE;
  }
  else{
    tmp=g_strdup_printf("HOE ver. %d.%d.%d is the latest version.",
			major,minor,micro);
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_OK,
#endif
		  POPUP_TIMEOUT*1,
		  tmp,
		  NULL);
    if(tmp) g_free(tmp);
  }
}

void CheckVer(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  ver_dl(hg);
  ver_txt_parse(hg);
}

void SyncCamZ(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  camz_dl(hg);
  camz_txt_parse(hg);
}

void RecalcRST(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  recalc_rst(hg);
}

void CalcCrossScan(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  Crosspara cp;
  gchar *str;
  
  hg=(typHOE *)gdata;

  cp=get_cross_angle(hg->wcent, hg->d_cross);
  str=g_strdup_printf(" Color=%s,  Cross Scan=%d",
		     (cp.col==COL_RED) ? "Red" : "Blue",
		     cp.cross);
  gtk_label_set_text(GTK_LABEL(hg->label_wcent),str);
  g_free(str);
}

void update_objtree(typHOE *hg){
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    objtree_update_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }
}

void recalc_rst(typHOE *hg){
  calc_moon(hg);
  calc_sun_plan(hg);
  calc_rst(hg);

  update_objtree(hg);
}


GtkWidget* gtkut_button_new_from_pixbuf(gchar *txt,
				       GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  gint w,h;
  
  box2=gtkut_hbox_new(TRUE,0);

  box=gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);
  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}



int main(int argc, char* argv[]){
  typHOE *hg;
#ifdef USE_WIN32
  WSADATA wsaData;
  int nErrorStatus;
#else
  GdkPixbuf *icon;
#endif

  hg=g_malloc0(sizeof(typHOE));

  setlocale(LC_ALL,"");

#ifndef USE_GTK3
  gtk_set_locale();
#endif

  gtk_init(&argc, &argv);

  param_init(hg);

  get_option(argc, argv, hg);

  // Gdk-Pixbufで使用
#if !GTK_CHECK_VERSION(2,21,8)
  gdk_rgb_init();
#endif

#ifndef USE_WIN32  
  icon = gdk_pixbuf_new_from_resource ("/icons/hoe_icon.png", NULL);
  gtk_window_set_default_icon(icon);
#endif

#ifdef USE_WIN32   // Initialize Winsock2
    nErrorStatus = WSAStartup(MAKEWORD(2,0), &wsaData);
    if(atexit((void (*)(void))(WSACleanup))){
      fprintf(stderr, "WSACleanup() : Failed\n");
      exit(-1);
    }
    if (nErrorStatus!=0) {
      fprintf(stderr, "WSAStartup() : Failed\n");
      exit(-1);
    }
#endif

  if(hg->filename_hoe){
    ReadHOE(hg, FALSE);
  }
  
  gui_init(hg);
  if((hg->filename_read)&&(!hg->filename_hoe)){
    ReadList(hg);
  }
  ////make_obj_list(hg,TRUE);
  make_obj_tree(hg);

  gtk_main();

}
