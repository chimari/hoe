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



#ifndef USE_WIN32
void ChildTerm();
#endif // USE_WIN32

void gui_init();
void make_note();
GtkWidget *make_menu();

static void fs_set_opeext();
static void fs_set_hoeext();
static void fs_set_list1ext();
static void fs_set_list2ext();
static void fs_set_list3ext();
static void cc_get_toggle_sm();
static void cc_usesetup();

void do_quit();
void do_open();
void do_open2();
void do_open_ope();
void do_upload();
void do_download_log();
void do_merge();
void do_save();
void do_save_fc_pdf_all();
void do_save_hoe();
void do_save_FCDB_List();
gchar* repl_nonalnum(gchar * obj_name, const gchar c_repl);
gchar* trdb_file_name();
void do_save_TRDB_CSV();
void do_read_hoe();
void create_quit_dialog();
void show_version();
void do_edit();
void do_plan();
void do_skymon();
void do_name_edit();
void do_efs_cairo();
void do_update_exp_ps();

void param_init();
void make_obj_list();
gchar* cut_spc();
void ChangeFontButton();
void ChangeFontButton_all();
void ReadList();
void ReadList2();
void UploadOPE();
void DownloadLOG();
void ReadListOPE();
void MergeList();
void WriteOPE();
void WriteYAML();
void WritePlan();
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

void calc_rst();
void RecalcRST();

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

  // Main Window 
  hg->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  my_signal_connect(hg->w_top, "destroy",
		    gtk_main_quit,NULL);
  gtk_container_set_border_width(GTK_CONTAINER(hg->w_top),0);
  gtk_window_set_title(GTK_WINDOW(hg->w_top),"HOE : HDS OPE file Editor");

  hg->w_box = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->w_top), hg->w_box);



  menubar=make_menu(hg);
  gtk_box_pack_start(GTK_BOX(hg->w_box), menubar,FALSE, FALSE, 0);

  make_note(hg);
  

  gtk_widget_show_all(hg->w_top);

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
      GtkWidget *vbox;
      GtkWidget *entry;
      GtkWidget *combo, *combo0, *fil1_combo, *fil2_combo;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *check;
      GtkWidget *button;
      gchar tmp[64];
      GtkTooltips *tooltip;
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
					//gtk_container_add (GTK_CONTAINER (scrwin), table);
      gtk_widget_set_size_request(scrwin, -1, 480);  
      
      
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


      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 0, 1,
		       GTK_FILL,GTK_FILL,0,0);

      label = gtk_label_new ("Date");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->fr_year,
						hg->fr_year-10, hg->fr_year+10,
						1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->fr_year);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->fr_month,
						1, 12, 1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->fr_month);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->fr_day,
						1, 31, 1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->fr_day);


      label = gtk_label_new ("  x");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->nights,
						1, 10,
						1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->nights);

      label = gtk_label_new ("nights");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);


      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("ID");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      gtk_entry_set_text(GTK_ENTRY(entry),hg->prop_id);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),8);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->prop_id);

      label = gtk_label_new ("  Pass");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      if(hg->prop_pass)
	gtk_entry_set_text(GTK_ENTRY(entry),hg->prop_pass);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),8);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->prop_pass);
      
      label = gtk_label_new ("   Time Zone");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->timezone,
						-12, +12,
						1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->timezone);

 

      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_table_attach(GTK_TABLE(table1), hbox, 0, 1, 2, 3,
		       GTK_FILL,GTK_FILL,0,0);

      label = gtk_label_new ("Observer");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
      if(hg->observer)
	gtk_entry_set_text(GTK_ENTRY(entry),hg->observer);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),25);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry,
			 &hg->observer);


      label = gtk_label_new ("   OCS");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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


      // Environment for AD Calc.
      frame = gtk_frame_new ("Environment for AD Calc.");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 1, 2,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(4,4,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


      // OBS Wavelength
      label = gtk_label_new ("Obs WL [A]");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Wavelength0
      label = gtk_label_new ("Guide WL [A]");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Temperature
      label = gtk_label_new ("  Temperature [C]");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Pressure
      label = gtk_label_new ("  Pressure [hPa]");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


      // Environment for AD Calc.
      frame = gtk_frame_new ("Web Browser");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 2, 3,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(2,1,FALSE);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

#ifndef USE_WIN32
#ifndef USE_OSX
      label = gtk_label_new ("Command");
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      
      entry = gtk_entry_new ();
      gtk_table_attach(GTK_TABLE(table1), entry, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
      gtk_entry_set_text(GTK_ENTRY(entry),
			 hg->www_com);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->www_com);
#endif
#endif

      frame = gtk_frame_new ("Font");
      gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 3, 4,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      
      hbox = gtk_hbox_new(FALSE,5);
      gtk_container_add (GTK_CONTAINER (frame), hbox);
      
      label = gtk_label_new ("Base");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
      GtkWidget *combo, *combo0, *fil1_combo, *fil2_combo;
      GtkAdjustment *adj;
      GtkWidget *spinner;
      GtkWidget *check;
      GtkWidget *button;
      gchar tmp[64];
      GtkTooltips *tooltip;
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
					//gtk_container_add (GTK_CONTAINER (scrwin), table);
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_factor,
						1, 5, 1.0, 1.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->exptime_factor);


      label = gtk_label_new ("Brightness");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->brightness,
						0, 10000, 1000.0, 1000.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->exptime_sv,
						100, 100000, 
						100.0, 100.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->exptime_sv);


      label = gtk_label_new ("Read Region");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_region,
						100, 400, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_region);


      label = gtk_label_new ("Calc Region");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 3, 4,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_calc,
						10, 150, 
						5.0, 5.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 3, 4,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_calc);


      label = gtk_label_new ("Integrate");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 4, 5,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_integrate,
						1, 5, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     FALSE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 4, 5,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->sv_integrate);

      label = gtk_label_new ("Acquisition Time[s]");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 5, 6,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_acq,
						30, 300, 
						10.0, 10.0, 0);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    Slit_Y ");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_slitx,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_slity);

      label = gtk_label_new ("IS_X ");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    IS_Y ");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_isx,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->sv_isy);


      label = gtk_label_new ("IS3_X ");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("    IS3_Y ");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->sv_is3x,
						100, 500, 
						0.5, 0.5, 0);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
      GtkTooltips *tooltip;
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
					//gtk_container_add (GTK_CONTAINER (scrwin), table);
      gtk_widget_set_size_request(scrwin, -1, 480);  
      
      

      // CamZ
      frame = gtk_frame_new ("Camera Z [um]");
      gtk_table_attach(GTK_TABLE(table), frame, 2, 3, 0, 5,
		       GTK_FILL,GTK_FILL,0,0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	
      table1 = gtk_table_new(4,2,FALSE);
      gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
      gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
      gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
      gtk_container_add (GTK_CONTAINER (frame), table1);
      
      label = gtk_label_new ("Blue");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" Red");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("dCross");
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);


      adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_b,
						-500, -200, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->camz_b);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->camz_r,
						-500, -200, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->camz_r);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->d_cross,
						-500, 500, 
						1.0, 10.0, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj,
			 &hg->d_cross);
      spinner =  gtk_spin_button_new (adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

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
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 0, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("CCD Binning");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("Slit Width/Length");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 4, 7, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 7, 8, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new ("Filter1/Filter2");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 8, 11, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new (" ");
      gtk_table_attach(GTK_TABLE(table1), label, 11, 12, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("ImR");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 12, 13, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      label = gtk_label_new("IS");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 13, 14, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
      
      label = gtk_label_new("I2");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
	      gtk_list_store_set(store, &iter, 0, binname[i_bin],
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
	  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 4, 5, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);

	  label = gtk_label_new ("/");
	  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
	  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 6, 7, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);


	  {
	    GtkListStore *store;
	    GtkTreeIter iter;	  
	    int i_fil;
	    
	    store = gtk_list_store_new(1, G_TYPE_STRING);
	    
	    
	    for(i_fil=0;i_fil<MAX_FILTER1;i_fil++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 0, filtername1[i_fil],-1);
	      gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
	    }


	    fil1_combo = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(store),0);
	    gtk_table_attach(GTK_TABLE(table1), fil1_combo, 8, 9, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    
	    g_object_unref(store);
	    
	    if(!hg->setup[i_use].fil1)
	      hg->setup[i_use].fil1=g_strdup(setups[StdUb].fil1);
	    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(fil1_combo)->child),
			       hg->setup[i_use].fil1);
	    gtk_entry_set_editable(GTK_ENTRY(GTK_BIN(fil1_combo)->child),TRUE);
	    
	    
	    my_entry_set_width_chars(GTK_ENTRY(GTK_BIN(fil1_combo)->child),6);
	    gtk_widget_show(fil1_combo);
	    my_signal_connect (GTK_BIN(fil1_combo)->child,"changed",
			       cc_get_entry,
			       &hg->setup[i_use].fil1);
	  }

	  label = gtk_label_new ("/");
	  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	  gtk_table_attach(GTK_TABLE(table1), label, 9, 10, i_use+1, i_use+2,
			   GTK_SHRINK,GTK_SHRINK,0,0);


	  {
	    GtkListStore *store;
	    GtkTreeIter iter;	  
	    int i_fil;
	    
	    store = gtk_list_store_new(1, G_TYPE_STRING);
	    
	    
	    for(i_fil=0;i_fil<MAX_FILTER2;i_fil++){
	      gtk_list_store_append(store, &iter);
	      gtk_list_store_set(store, &iter, 0, filtername2[i_fil],-1);
	      gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
	    }

	    fil2_combo = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(store),0);
	    gtk_table_attach(GTK_TABLE(table1), fil2_combo, 10, 11, i_use+1, i_use+2,
			     GTK_SHRINK,GTK_SHRINK,0,0);
	    
	    g_object_unref(store);
	   
	    if(!hg->setup[i_use].fil2)
	      hg->setup[i_use].fil2=g_strdup(setups[StdUb].fil2);
	    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(fil2_combo)->child),
			       hg->setup[i_use].fil2);
	    gtk_entry_set_editable(GTK_ENTRY(GTK_BIN(fil2_combo)->child),TRUE);
	    
	    my_entry_set_width_chars(GTK_ENTRY(GTK_BIN(fil2_combo)->child),6);
	    gtk_widget_show(fil2_combo);
	    my_signal_connect (GTK_BIN(fil2_combo)->child,"changed",
			       cc_get_entry,
			       &hg->setup[i_use].fil2);
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("Cross Scan");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("Echelle");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);

      label = gtk_label_new("CamR");
      gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
	  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 2, 3, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	  //gtk_signal_connect (GTK_OBJECT (&GTK_SPIN_BUTTON(spinner)->entry),
	  //		      "value_changed",
	  //		      GTK_SIGNAL_FUNC (cc_get_entry_int),
	  //		      &hg->nonstd[i].cross);
	  
	  
	  adj = (GtkAdjustment *)gtk_adjustment_new(hg->nonstd[i].echelle,
						    -3600, 3600, 
						    60.0,60.0,0);
	  my_signal_connect (adj, "value_changed",
			     cc_get_adj,
			     &hg->nonstd[i].echelle);
	  spinner =  gtk_spin_button_new (adj, 0, 0);
	  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
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
	  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
				 TRUE);
	  gtk_table_attach(GTK_TABLE(table1), spinner, 4, 5, i+1, i+2,
			   GTK_FILL,GTK_SHRINK,0,0);
	  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
	  //gtk_signal_connect (GTK_OBJECT (&GTK_SPIN_BUTTON(spinner)->entry),
	  //		      "value_changed",
	  //		      GTK_SIGNAL_FUNC (cc_get_entry_int),
	  //		      &hg->nonstd[i].camr);
	}
      }

      label = gtk_label_new ("HDS");
      //      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), scrwin, label);
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
      gchar tmp[12];

      table = gtk_table_new (2, 2, FALSE);

      
      hg->sw_objtree = gtk_scrolled_window_new (NULL, NULL);
      gtk_table_attach_defaults (GTK_TABLE(table), hg->sw_objtree, 0, 2, 0, 1);
      gtk_container_set_border_width (GTK_CONTAINER (hg->sw_objtree), 5);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hg->sw_objtree),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);

      make_obj_tree(hg);

      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      frame = gtk_frame_new ("View");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

      button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (plot2_objtree_item), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);

      button=gtkut_button_new_from_stock("FC",GTK_STOCK_ABOUT);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (fc_item), (gpointer)hg);


      hg->f_objtree_arud = gtk_frame_new ("Edit the List");
      gtk_box_pack_start (GTK_BOX (hbox), hg->f_objtree_arud, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hg->f_objtree_arud), 2);
	
      hbox1 = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (hg->f_objtree_arud), hbox1);

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			addobj_dialog, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Add");
#endif

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			remove_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Remove");
#endif

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			up_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Up");
#endif

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
      my_signal_connect(button,"pressed",
      			down_item_objtree, 
      			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Down");
#endif

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (RecalcRST), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Refresh Rise & Set Time (El=15)");
#endif


      frame = gtk_frame_new ("Web Browsing");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtk_hbox_new(FALSE,2);
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
	icon = gdk_pixbuf_new_from_inline(sizeof(safari_icon), safari_icon, 
					  FALSE, NULL);
#elif defined(USE_WIN32)
	icon = gdk_pixbuf_new_from_inline(sizeof(ie_icon), ie_icon, 
					  FALSE, NULL);
#else
	if(strcmp(hg->www_com,"firefox")==0){
	  icon = gdk_pixbuf_new_from_inline(sizeof(firefox_icon), firefox_icon, 
					    FALSE, NULL);
	}
	else{
	  icon = gdk_pixbuf_new_from_inline(sizeof(chrome_icon), chrome_icon, 
					    FALSE, NULL);
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

      hbox1 = gtk_hbox_new(FALSE,2);
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
    
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (stddb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Search");
#endif
      
      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

      frame = gtk_frame_new ("Find Object");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
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
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),10);
      my_signal_connect (entry, "changed", cc_search_text, (gpointer)hg);
      my_signal_connect (entry, "activate", search_item, (gpointer)hg);
      
      hg->tree_search_label = gtk_label_new ("     ");
      gtk_box_pack_start(GTK_BOX(hbox1),hg->tree_search_label,FALSE,FALSE,0);
      

      frame = gtk_frame_new ("Set Default Parameters");
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 2);

      hbox1 = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
      gtk_container_add (GTK_CONTAINER (frame), hbox1);

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
	gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE, FALSE, 0);
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
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      adj = (GtkAdjustment *)gtk_adjustment_new(hg->def_pa,
						-360.0, 360.0, 0.1, 0.1, 0);
      my_signal_connect (adj, "value_changed",
			 cc_get_adj_double,
			 &hg->def_pa);
      spinner =  gtk_spin_button_new (adj, 1, 1);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
      gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			     TRUE);
      my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
      gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE, FALSE, 0);



      label = gtk_label_new ("  Exp");
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox1),label,FALSE, FALSE, 0);

      entry = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox1),entry,FALSE, FALSE, 0);
      sprintf(tmp,"%d",hg->def_exp);
      gtk_entry_set_text(GTK_ENTRY(entry),tmp);
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_signal_connect (entry,
			 "changed",
			 cc_get_entry_int,
			 &hg->def_exp);
      my_entry_set_width_chars(GTK_ENTRY(entry),4);


      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_OK);
      gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE, FALSE, 0);
      my_signal_connect(button,"pressed",
			export_def, 
			(gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Apply to the List");
#endif

      
      label = gtk_label_new ("Main Target");
      gtk_widget_show(label);
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
      
    }

    // STDDB
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *button;
      GtkWidget *sw;
      GtkTreeModel *items_model;
      GdkPixbuf *icon;

      vbox = gtk_vbox_new (FALSE, 5);
      label = gtk_label_new ("Standard");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
    

      hg->stddb_button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_APPLY);
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
    
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
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
      hbox = gtk_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
#ifdef USE_OSX
      icon = gdk_pixbuf_new_from_inline(sizeof(safari_icon), safari_icon, 
					FALSE, NULL);
#elif defined(USE_WIN32)
      icon = gdk_pixbuf_new_from_inline(sizeof(ie_icon), ie_icon, 
					FALSE, NULL);
#else
      if(strcmp(hg->www_com,"firefox")==0){
	icon = gdk_pixbuf_new_from_inline(sizeof(firefox_icon), firefox_icon, 
					  FALSE, NULL);
      }
      else{
	icon = gdk_pixbuf_new_from_inline(sizeof(chrome_icon), chrome_icon, 
					  FALSE, NULL);
      }
#endif
      button=gtkut_button_new_from_pixbuf("Browse", icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (std_simbad), (gpointer)hg);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (add_item_std), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"to Main Target List");
#endif
      
      label= gtk_label_new ("    ");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 make_std_tgt, (gpointer)hg);
      
      hg->std_tgt = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->std_tgt,TRUE, TRUE, 0);
      gtk_entry_set_editable(GTK_ENTRY(hg->std_tgt),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->std_tgt),50);

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
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

      vbox = gtk_vbox_new (FALSE, 5);
      label = gtk_label_new ("Finding Chart / DB");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (do_save_FCDB_List), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif
      
      hg->fcdb_label= gtk_label_new (hg->fcdb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_label, TRUE, TRUE, 0);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
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
      hbox = gtk_hbox_new (FALSE, 4);
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
	icon = gdk_pixbuf_new_from_inline(sizeof(safari_icon), safari_icon, 
					  FALSE, NULL);
#elif defined(USE_WIN32)
	icon = gdk_pixbuf_new_from_inline(sizeof(ie_icon), ie_icon, 
					  FALSE, NULL);
#else
	if(strcmp(hg->www_com,"firefox")==0){
	  icon = gdk_pixbuf_new_from_inline(sizeof(firefox_icon), firefox_icon, 
					    FALSE, NULL);
	}
	else{
	  icon = gdk_pixbuf_new_from_inline(sizeof(chrome_icon), chrome_icon, 
					    FALSE, NULL);
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
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (add_item_fcdb), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"to Main Target List");
#endif
      
      label= gtk_label_new ("    ");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 make_fcdb_tgt, (gpointer)hg);
      
      hg->fcdb_tgt = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->fcdb_tgt,TRUE, TRUE, 0);
      gtk_entry_set_editable(GTK_ENTRY(hg->fcdb_tgt),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->fcdb_tgt),50);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
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

      vbox = gtk_vbox_new (FALSE, 5);
      label = gtk_label_new ("List Query");
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), vbox, label);
      
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (do_save_TRDB_CSV), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"Save queried List to CSV file");
#endif

      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
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
      gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
      my_entry_set_width_chars(GTK_ENTRY(entry),10);
      my_signal_connect (entry, "changed", trdb_cc_search_text, (gpointer)hg);
      my_signal_connect (entry, "activate", trdb_search_item, (gpointer)hg);
      
      hg->trdb_search_label = gtk_label_new ("     ");
      gtk_box_pack_start(GTK_BOX(hbox),hg->trdb_search_label,FALSE,FALSE,0);

      hg->trdb_label= gtk_label_new (hg->trdb_label_text);
      gtk_box_pack_start(GTK_BOX(hbox), hg->trdb_label, TRUE, TRUE, 0);
      
     
      hg->trdb_sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (hg->trdb_sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hg->trdb_sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), hg->trdb_sw, TRUE, TRUE, 0);

      trdb_append_tree(hg);

      /* some buttons */
      hbox = gtk_hbox_new (FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock("Show Detail",GTK_STOCK_GO_BACK);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_dbtab), (gpointer)hg);

#ifdef USE_OSX
      icon = gdk_pixbuf_new_from_inline(sizeof(safari_icon), safari_icon, 
					FALSE, NULL);
#elif defined(USE_WIN32)
      icon = gdk_pixbuf_new_from_inline(sizeof(ie_icon), ie_icon, 
					FALSE, NULL);
#else
      if(strcmp(hg->www_com,"firefox")==0){
	icon = gdk_pixbuf_new_from_inline(sizeof(firefox_icon), firefox_icon, 
					  FALSE, NULL);
      }
      else{
	icon = gdk_pixbuf_new_from_inline(sizeof(chrome_icon), chrome_icon, 
					  FALSE, NULL);
      }
#endif
      button=gtkut_button_new_from_pixbuf("Browse", icon);
      g_object_unref(icon);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			 G_CALLBACK (trdb_simbad), (gpointer)hg);
      /*
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (add_item_fcdb), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,"to Main Target List");
#endif
      
      label= gtk_label_new ("    ");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
      
      button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 make_fcdb_tgt, (gpointer)hg);
      
      hg->fcdb_tgt = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hg->fcdb_tgt,TRUE, TRUE, 0);
      gtk_entry_set_editable(GTK_ENTRY(hg->fcdb_tgt),FALSE);
      my_entry_set_width_chars(GTK_ENTRY(hg->fcdb_tgt),50);
      
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
      			 G_CALLBACK (clip_copy), (gpointer)hg->fcdb_tgt);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Copy to clipboard");
#endif
      */
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

      hbox = gtk_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);

      button=gtkut_button_new_from_stock("Clear",GTK_STOCK_CLEAR);
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_init), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

      button=gtkut_button_new_from_stock("Stellar Absorption",GTK_STOCK_YES);
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_star), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

      button=gtkut_button_new_from_stock("Nebular Emission",GTK_STOCK_YES);
      g_signal_connect (button, "clicked",
                      G_CALLBACK (linetree_nebula), (gpointer)hg);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);


      label = gtk_label_new ("Line List");
      gtk_widget_show(label);
      gtk_notebook_append_page (GTK_NOTEBOOK (hg->all_note), table, label);
      
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
  
  //File/Open List
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Open List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open,(gpointer)hg);


  //File/Open List for Planet Search
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Open List for Planet Search");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open2,(gpointer)hg);


  //File/Merge List
  image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Merge List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_merge,(gpointer)hg);


  //File/Import List from OPE
  image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Import List from OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_open_ope,(gpointer)hg);


  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Write OPE
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Write OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save,(gpointer)hg);


  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Upload OPE
  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Upload OPE");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_upload,(gpointer)hg);

  image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Download LOG");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_download_log,(gpointer)hg);


  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Load Config
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Load Config");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_read_hoe,(gpointer)hg);


  //File/Save Config
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save Config");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_hoe,(gpointer)hg);


  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Quit
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_quit,NULL);



  //// Edit
#ifdef GTK_STOCK_EDIT
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("Edit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Edit/PLan Editor
  image=gtk_image_new_from_stock (GTK_STOCK_INDENT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Obs. Plan Editor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_plan,(gpointer)hg);

  bar =gtk_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //Edit/Saved OPE File
#ifdef GTK_STOCK_EDIT
  image=gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Saved OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_edit,(gpointer)hg);

  //Edit/Select OPE File
  image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Text Editor (Select OPE)");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_name_edit,(gpointer)hg);



  //// Plot
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Plot");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Plot/Echelle Format Simulator
  image=gtk_image_new_from_stock (GTK_STOCK_PRINT_PREVIEW, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Echelle Format Simulator");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_efs_cairo,(gpointer)hg);

  //Plot/Echelle Format Simulator
  pixbuf = gdk_pixbuf_new_from_inline(sizeof(icon_pdf), icon_pdf, 
				      FALSE, NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  16,16,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
  popup_button =gtk_image_menu_item_new_with_label ("PDF Finding Charts");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_save_fc_pdf_all,(gpointer)hg);



  //// SkyMon
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("SkyMon");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //SkyMon/Sky Monitor
  image=gtk_image_new_from_stock (GTK_STOCK_PRINT_PREVIEW, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Sky Monitor");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_skymon,(gpointer)hg);



  //// Update
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Update");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Update/Exptime
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Exptime");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_update_exp_ps,(gpointer)hg);


  ////Database
#ifdef GTK_STOCK_ABOUT
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("Database");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  // SMOKA
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("SMOKA : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_smoka, (gpointer)hg);

  // HST
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("HST archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_hst, (gpointer)hg);

  // ESO
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("ESO archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_eso, (gpointer)hg);

  // Gemini
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Gemini archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_gemini, (gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // Standard
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for Standard");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     create_std_para_dialog, (gpointer)hg);

  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for DB query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     fcdb_para_item, (gpointer)hg);

  //// Info
#ifdef GTK_STOCK_INFO
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef GTK_STOCK_ABOUT
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_version, NULL);


  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}

static void fs_set_opeext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." OPE_EXTENSION);
}

static void fs_set_hoeext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." HOE_EXTENSION);
}

static void fs_set_list1ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST1_EXTENSION);
}


static void fs_set_list2ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST2_EXTENSION);
}


static void fs_set_list3ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST3_EXTENSION);
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
      draw_skymon_cairo(cdata->hg->skymon_dw,NULL,
			(gpointer)cdata->hg);
      gdk_window_raise(cdata->hg->skymon_main->window);
    }
  }
}

void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=GTK_ADJUSTMENT(widget)->value;
}

void cc_get_adj_double (GtkWidget *widget, gdouble * gdata)
{
  *gdata=GTK_ADJUSTMENT(widget)->value;
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
    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(cdata->fil1_combo)->child),
		       setups[i_set].fil1);
    g_free(cdata->hg->setup[cdata->i_use].fil1);
    cdata->hg->setup[cdata->i_use].fil1=g_strdup(setups[i_set].fil1);
    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(cdata->fil2_combo)->child),
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


void create_quit_dialog ()
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Quit Program",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 


  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				   GTK_ICON_SIZE_DIALOG);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Do you want to quit this program?");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    exit(0);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
}

void do_quit (gpointer gdata, guint callback_action, GtkWidget *widget)
{
  if(!flagChildDialog){
    create_quit_dialog();
  }
  else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Please close all child dialogs before quitting.",
		    NULL);
#else
      g_print ("Please close all child dialogs before quitting.\n");
#endif
  }
}

void do_open (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_read,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_read));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_read));
  }

  my_file_chooser_add_filter(fdialog,"List File","*." OPE_EXTENSION,NULL);
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
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void do_upload (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File to be Uploaded",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input Log File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filename_log) g_free(hg->filename_log);
      hg->filename_log=g_strdup(dest_file);
      DownloadLOG(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select Input List File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input OPE File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filename_write) g_free(hg->filename_write);
      hg->filename_write=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      WriteOPE(hg, FALSE);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  fdialog = gtk_file_chooser_dialog_new("HOE : Input OPE File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filename_write) g_free(hg->filename_write);
      hg->filename_write=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      WriteOPE(hg, TRUE);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      if(hg->filename_txt) g_free(hg->filename_txt);
      hg->filename_txt=g_strdup(dest_file);
      WritePlan(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      if(hg->filename_txt) g_free(hg->filename_txt);
      hg->filename_txt=g_strdup(dest_file);
      WriteYAML(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_plot(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_skymon(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);
    
    dest_file=to_locale(fname);
    
    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_efs(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_fc(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);

      create_fc_all_dialog(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Input HOE File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);
      
      if(hg->filename_hoe) g_free(hg->filename_hoe);
      hg->filename_hoe=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      WriteHOE(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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


  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : CSV File to be Saved (FCDB)",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_fcdb) g_free(hg->filename_fcdb);
      hg->filename_fcdb=g_strdup(dest_file);
      
      Export_FCDB_List(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING,POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
  gchar *fname;
  gchar *iname;

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

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : CSV file to be Saved (List Query)",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_trdb) g_free(hg->filename_trdb);
      hg->filename_trdb=g_strdup(dest_file);
      
      Export_TRDB_CSV(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select HOE Config File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
    }
    else{
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}


void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GdkPixbuf *icon;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : About This Program",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_inline(sizeof(hoe_icon), hoe_icon, 
				    FALSE, NULL);
  pixmap = gtk_image_new_from_pixbuf(icon);
  g_object_unref(icon);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("HOE : HDS OPE file Editor,  version "VERSION);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : SkyMonitor=%s", 
	         "ON");
  label = gtk_label_new (buf);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  
  label = gtk_label_new ("Copyright(C) 2003-18 Akito Tajitsu");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("<tajitsu@naoj.org>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
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

  flagChildDialog=FALSE;
}


void do_edit(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(!hg->filename_write){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "No OPE files have been saved yet.",
		  NULL);
#else
    g_print ("No OPE files have been saved yet.\n");
#endif
    return;
  }

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs.\n");
#endif
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
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
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
    gdk_window_raise(hg->skymon_main->window);
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

  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  fdialog = gtk_file_chooser_dialog_new("HOE : Select OPE File to be Edited",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
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
  
  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }
  
  
  hg=(typHOE *)gdata;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Echelle Format Simulator",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("EFS : Echelle Format Simulator");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new ("Setup:");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gboolean flag_set=FALSE;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    hg->efs_setup=0;
    
    for(i_use=0;i_use<MAX_USESETUP;i_use++){
      if(hg->setup[i_use].use){
	if(hg->setup[i_use].setup<0){
	  sprintf(tmp,"Setup-%d : NonStd-%d",i_use+1,-hg->setup[i_use].setup);
	}
	else{
	  sprintf(tmp,"Setup-%d : Std%s",i_use+1,setups[hg->setup[i_use].setup].initial);
	}
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, tmp,
			   1, i_use, -1);

	if(!flag_set){
	  hg->efs_setup=i_use;
	  flag_set=TRUE;
	}
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE, FALSE, 0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),hg->efs_setup);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->efs_setup);
  }


  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    go_efs(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
}


void do_update_exp_ps (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button;
  GtkWidget *hbox, *entry, *check;
  GtkWidget *fdialog;
  typHOE *hg;
  gchar tmp[64];
  int i_use;
  
  if(flagChildDialog){
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
#else
    g_print ("Please close all child dialogs");
#endif
    return;
  }
  else{
    flagChildDialog=TRUE;
  }
  
  
  hg=(typHOE *)gdata;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new_with_buttons("HOE : Update Exptime for PS",
				       NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  label = gtk_label_new ("ExpTime for V=8mag");
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

  label = gtk_label_new ("[s]");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  check = gtk_check_button_new_with_label("Consider degradation by SecZ?");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->flag_secz);
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

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    do_update_exp(hg);
  }
  else{
    gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;
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


void param_init(typHOE *hg){
  time_t t;
  struct tm *tmpt;
  int i, i_band;

  // Global Args
  flagChildDialog=FALSE;
  flagSkymon=FALSE;
  flagPlot=FALSE;
  flagFC=FALSE;
  flag_getFCDB=FALSE;
  flag_make_obj_tree=FALSE;
  flag_make_line_tree=FALSE;




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

  hg->pixmap_fc=NULL;

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


  hg->ocs=OCS_GEN2;
  hg->timezone=TIMEZONE_SUBARU;
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

  hg->d_cross=D_CROSS;

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
  }

  hg->trdb_i_max=0;
  hg->trdb_disp_flag=TRUE;
  hg->trdb_smoka_inst=0;
  skymon_set_time_current(hg);
  hg->trdb_smoka_date=g_strdup_printf("1998-01-01..%d-%02d-%02d",
				      hg->fr_year,
				      hg->fr_month,
				      hg->fr_day);
  hg->trdb_arcmin=2;
  hg->trdb_label_text=g_strdup_printf("SMOKA List Query (%s)",
				      smoka_subaru[hg->trdb_smoka_inst].name);
  hg->trdb_used=TRDB_TYPE_SMOKA;
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

  hg->filename_hoe=NULL;
  hg->filename_log=NULL;

  hg->azel_mode=AZEL_NORMAL;

  hg->flag_bunnei=FALSE;
  hg->flag_secz=FALSE;
  hg->exp8mag=100;
  hg->secz_factor=0.10;

  hg->skymon_mode=SKYMON_CUR;
  hg->skymon_objsz=SKYMON_DEF_OBJSZ;

  hg->www_com=g_strdup(WWW_BROWSER);
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

  hg->obs_timezone = TIMEZONE_SUBARU*60;

  hg->fontname=g_strdup(SKYMON_FONT);
  hg->fontname_all=g_strdup(SKYMON_FONT);
  get_font_family_size(hg);

  calc_moon(hg);
  calc_sun_plan(hg);

}






gchar *cut_spc(gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name, *c;
  gint  i_bak,i;

  strcpy(tgt_name,obj_name);
  i_bak=strlen(tgt_name)-1;
  while((tgt_name[i_bak]==0x20)
	||(tgt_name[i_bak]==0x0A)
	||(tgt_name[i_bak]==0x0D)
	||(tgt_name[i_bak]==0x09)){
    //tgt_name[i_bak]=(char)NULL;
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

void ReadList(typHOE *hg){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  
  hg->flag_bunnei=FALSE;

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
      if(!is_number(tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      //hg->obj[i_list].ra=read_radec(tmp_char);

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      //hg->obj[i_list].dec=read_radec(tmp_char);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      
      if(tmp_char=(char *)strtok(NULL,"\n")){
	hg->obj[i_list].note=g_strdup(tmp_char);
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }

      hg->obj[i_list].exp=DEF_EXP;
      hg->obj[i_list].repeat=1;
      hg->obj[i_list].guide=SV_GUIDE;
      hg->obj[i_list].pa=0;
      
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

  hg->flag_bunnei=TRUE;

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
      if(!is_number(tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Magnitude")) break;
      hg->obj[i_list].mag=(gdouble)g_strtod(tmp_char,NULL);
      
      if(tmp_char=(char *)strtok(NULL,"\n")){
	hg->obj[i_list].note=g_strdup(tmp_char);
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }

      hg->obj[i_list].exp=DEF_EXP;
      hg->obj[i_list].repeat=1;
      hg->obj[i_list].guide=SV_GUIDE;
      hg->obj[i_list].pa=0;
      
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

  calc_rst(hg);
}

void UploadOPE(typHOE *hg){
  gint ans=0;

  if((ans=sftp_c(hg))<0){
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

  if((ans=sftp_get_c(hg))<0){
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
  
  hg->flag_bunnei=FALSE;

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
	    hg->obj[i_list].repeat=1;
	    hg->obj[i_list].guide=SV_GUIDE;
	    hg->obj[i_list].pa=0;
	  
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
	if(!is_number(tmp_char,hg->i_max-i_base+1,"RA")) break;
	tmp_obj.ra=(gdouble)g_strtod(tmp_char,NULL);
	
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(tmp_char,hg->i_max-i_base+1,"Dec")) break;
	tmp_obj.dec=(gdouble)g_strtod(tmp_char,NULL);
      
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(tmp_char,hg->i_max-i_base+1,"Equinox")) break;
	tmp_obj.equinox=(gdouble)g_strtod(tmp_char,NULL);
	
	if(tmp_char=(char *)strtok(NULL,"\n")){
	  tmp_obj.note=g_strdup(tmp_char);
	  tmp_obj.note=cut_spc(tmp_char);
	}
	else{
	  tmp_obj.note=NULL;
	}
	
	tmp_obj.check_sm=FALSE;
	tmp_obj.exp=DEF_EXP;
	tmp_obj.repeat=1;
	tmp_obj.guide=SV_GUIDE;
	tmp_obj.pa=0;
	
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




void WriteOPE(typHOE *hg, gboolean plan_flag){
  FILE *fp;
  int i_list=0, i_set, i_use, i_repeat, i_plan;
  gint to_year, to_month, to_day;
  gchar *tgt;

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
    fprintf(fp, "%s=OBJECT=\"%s\" RA=%09.2f DEC=%+010.2f EQUINOX=%7.2f\n",
	    tgt, hg->obj[i_list].name, 
      	    hg->obj[i_list].ra,  hg->obj[i_list].dec, hg->obj[i_list].equinox);
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
	  
	  if(hg->flag_bunnei){
	    fprintf(fp, " ,   Mv=%4.1lf",
		    hg->obj[i_list].mag);
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
	  if(hg->flag_bunnei){
	    if(hg->obj[i_list].mag>MAG_SVFILTER1){  // Filter=V
	      fprintf(fp,"SetupField SV_Filter01=1");
	    }
	    else if(hg->obj[i_list].mag>MAG_SVFILTER2){  //Filter=ND2
	      fprintf(fp,"SetupField SV_Filter01=3");
	    }
	    else{  //Filter=ND2
	      fprintf(fp,"SetupField SV_Filter01=2");// Filter=ND4
	    }
	  }
	  else{
	    fprintf(fp,"SetupField");
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

	  tgt=make_tgt(hg->obj[i_list].name);
	  fprintf(fp, " $DEF_PROTO $%s",tgt);
	  g_free(tgt);

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
	    if(hg->flag_bunnei){
	      fprintf(fp,"GetObject");
	    }
	    else{
	      fprintf(fp,"GetObject");
	    }
	    if(hg->setup[i_use].is!=IS_NO){
	      fprintf(fp," IS_FLAG=1");
	      if(hg->setup[i_use].is==IS_020X3){
		fprintf(fp," IS_Z_OFFSET=-0.40");
	      }
	    }
	    tgt=make_tgt(hg->obj[i_list].name);
	    fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
		    hg->obj[i_list].exp, hg->sv_integrate, tgt);
	    g_free(tgt);
	  }

	  if(hg->setup[i_use].i2){
	    fprintf(fp, "SETI2 $DEF_SPEC I2_POSITION=\"IN\"  $I2_Z\n");
	    for(i_repeat=0;i_repeat<hg->obj[i_list].repeat;i_repeat++){
	      if(hg->flag_bunnei){
		fprintf(fp,"GetObject");
	      }
	      else{
		fprintf(fp,"GetObject");
	      }
	      if(hg->setup[i_use].is!=IS_NO){
		fprintf(fp," IS_FLAG=1");
		if(hg->setup[i_use].is==IS_020X3){
		  fprintf(fp," IS_Z_OFFSET=-0.40");
		}
	      }
	      tgt=make_tgt(hg->obj[i_list].name);
	      fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
		      hg->obj[i_list].exp, hg->sv_integrate, tgt);
	      g_free(tgt);
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


void WritePlan(typHOE *hg){
  FILE *fp;
  int i_plan;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
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

    tgt=make_tgt(hg->obj[plan.obj_i].name);
    fprintf(fp, " $DEF_PROTO $%s", tgt);
    g_free(tgt);
    
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
      tgt=make_tgt(hg->obj[plan.obj_i].name);
      fprintf(fp, " $DEF_SPEC Exptime=%d SVIntegrate=%d $%s\n",
	      plan.exp, hg->sv_integrate, tgt);
      g_free(tgt);
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
  gchar tmp[12],f_tmp[12];
  int i_nonstd, i_set, i_list, i_line, i_plan;


  //filename = g_strconcat(g_get_home_dir(), "/save.hoe", NULL);
  filename = g_strdup(hg->filename_hoe);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // General 
  xmms_cfg_write_string(cfgfile, "General", "prog_ver",VERSION);
  if(hg->filename_write) xmms_cfg_write_string(cfgfile, "General", "OPE", hg->filename_write);
  if(hg->filename_read)  xmms_cfg_write_string(cfgfile, "General", "List",hg->filename_read);
  xmms_cfg_write_boolean(cfgfile, "General", "PSFlag",hg->flag_bunnei);
  xmms_cfg_write_boolean(cfgfile, "General", "SecZFlag",hg->flag_secz);
  xmms_cfg_write_double(cfgfile, "General", "SecZFactor",hg->secz_factor);

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
  xmms_cfg_write_int(cfgfile, "Header", "TZ",hg->timezone);
  //xmms_cfg_write_string(cfgfile, "Header", "WWWCom",hg->www_com);
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
    if(hg->flag_bunnei){
      xmms_cfg_write_double2(cfgfile, tmp, "Mag",hg->obj[i_list].mag,"%4.1f");
    }
    xmms_cfg_write_double2(cfgfile, tmp, "PA",hg->obj[i_list].pa,"%+7.2f");
    xmms_cfg_write_int(cfgfile, tmp, "Guide",hg->obj[i_list].guide);
    if(hg->obj[i_list].note) xmms_cfg_write_string(cfgfile, tmp, "Note",hg->obj[i_list].note);
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_write_boolean(cfgfile, tmp, f_tmp,hg->obj[i_list].setup[i_set]);
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
    xmms_cfg_remove_key(cfgfile,tmp, "Guide");
    xmms_cfg_remove_key(cfgfile,tmp, "Note");
    for(i_set=0;i_set<MAX_USESETUP;i_set++){
      sprintf(f_tmp,"SetUp-%d",i_set+1);
      xmms_cfg_remove_key(cfgfile,tmp, f_tmp);
    }
  }

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
  gchar tmp[12], f_tmp[12];
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_nonstd,i_set,i_list,i_line,i_plan;

  cfgfile = xmms_cfg_open_file(hg->filename_hoe);
  
  if (cfgfile) {
    
    // General 
    if(xmms_cfg_read_string(cfgfile, "General", "OPE",  &c_buf)) hg->filename_write=c_buf;
    if(xmms_cfg_read_string(cfgfile, "General", "List", &c_buf)) hg->filename_read =c_buf;
    if(xmms_cfg_read_boolean(cfgfile, "General", "PSFlag", &b_buf)) hg->flag_bunnei =b_buf;
    else hg->flag_bunnei = FALSE;
    if(xmms_cfg_read_boolean(cfgfile, "General", "SecZFlag", &b_buf)) hg->flag_secz =b_buf;
    else hg->flag_secz = FALSE;
    if(xmms_cfg_read_double(cfgfile, "General", "SecZFactor", &f_buf)) hg->secz_factor =f_buf;

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
    if(xmms_cfg_read_int   (cfgfile, "Header", "TZ",       &i_buf)) hg->timezone=i_buf;
    //if(xmms_cfg_read_string(cfgfile, "Header", "WWWCom",       &c_buf)) hg->www_com =c_buf;
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
      if(hg->flag_bunnei){
	if(xmms_cfg_read_double  (cfgfile, tmp, "Mag",  &f_buf)) hg->obj[i_list].mag =f_buf;
	else{
	  hg->i_max=i_list;
	  break;
	}
      }
      if(xmms_cfg_read_double  (cfgfile, tmp, "PA",     &f_buf)) hg->obj[i_list].pa    =f_buf;
      if(xmms_cfg_read_int    (cfgfile, tmp, "Guide",  &i_buf)) hg->obj[i_list].guide =i_buf;
      if(xmms_cfg_read_string (cfgfile, tmp, "Note",   &c_buf)) hg->obj[i_list].note  =c_buf;
      
      for(i_set=0;i_set<MAX_USESETUP;i_set++){
	sprintf(f_tmp,"SetUp-%d",i_set+1);
	if(xmms_cfg_read_boolean(cfgfile, tmp, f_tmp,  &b_buf)) hg->obj[i_list].setup[i_set]=b_buf;
      }
    }

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
      if(xmms_cfg_read_int    (cfgfile, tmp, "Setup", &i_buf)) hg->plan[i_plan].setup  =i_buf;
      else hg->plan[i_plan].setup=0;
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

gboolean is_number(gchar *s, gint line, const gchar* sect){
  gchar* msg;

  if(!s){
    msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"", line, sect);
#ifdef GTK_MSG
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		  "Error: Input File is invalid.",
		  " ",
		  msg,
		  NULL);
#else
    fprintf(stderr, "Error: Input File is invalid.\n%s",msg);
#endif
  
    g_free(msg);
    return FALSE;
  }

  while(*s!='\0'){
    if(!is_num_char(*s)){
      msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"\n Irregal character code : \"%02x\"", 
			  line, sect,*s);
#ifdef GTK_MSG
      popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT,
		    "Error: Input File is invalid.",
		    " ",
		    msg,
		    NULL);
#else
      fprintf(stderr, "Error: Input File is invalid.\n%s",msg);
#endif
      
      g_free(msg);
      return FALSE;
    }
    s++;
  }
  return TRUE;
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

  gtk_init(&argc, &argv);

  param_init(hg);

  get_option(argc, argv, hg);

  // Gdk-Pixbufで使用
  gdk_rgb_init();

#ifndef USE_WIN32  
  icon = gdk_pixbuf_new_from_inline(sizeof(hoe_icon), hoe_icon, 
				    FALSE, NULL);
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


void popup_message(gchar* stock_id,gint delay, ...){
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

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HOE : Message");

  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
  }

  my_signal_connect(dialog,"destroy",destroy_popup, &timer);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtk_label_new(msg1);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);
  gtk_main();
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

static void destroy_popup(GtkWidget *w, gint *data)
{
  gtk_timeout_remove(*data);
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


GtkWidget* gtkut_button_new_from_stock(gchar *txt,
				       const gchar *stock){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
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

GtkWidget* gtkut_toggle_button_new_from_stock(gchar *txt,
					      const gchar *stock){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
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
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,20,20,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,16,16,GDK_INTERP_BILINEAR);
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

void RecalcRST(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  recalc_rst(hg);
}

void recalc_rst(typHOE *hg){
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  calc_moon(hg);
  calc_sun_plan(hg);
  calc_rst(hg);

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->objtree));
  if(!gtk_tree_model_get_iter_first(model, &iter)) return;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    gtk_tree_model_get (model, &iter, COLUMN_OBJTREE_NUMBER, &i, -1);
    i--;
    objtree_update_item(hg, model, iter, i);
    if(!gtk_tree_model_iter_next(model, &iter)) break;
  }

  
}
GtkWidget* gtkut_button_new_from_pixbuf(gchar *txt,
				       GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,20,20,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,16,16,GDK_INTERP_BILINEAR);
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
