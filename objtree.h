//    HDS OPE file Editor
//      objtree.h : Header for Main Target List Treeview
//                                           2019.01.09  A.Tajitsu


static void objtree_add_columns();
static GtkTreeModel *create_items_model ();
//void objtree_update_item();
void objtree_update_radec_item();
static void cell_edited ();
static void cell_toggled_check ();
static void cell_toggled_std ();
static void cell_toggled_adi ();
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
static GtkTreeModel * create_aomode_model ();
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
void pm_dialog();
//void addobj_dialog();
//void strchg();
//void str_replace();
//gchar *make_simbad_id();
//void update_c_label();
void swap_obj();
