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
static void cell_toggled_seimei_pc ();
static void cell_toggled_seimei_ag ();
static void cell_toggled_seimei_nw ();
static void cell_toggled ();
void objtree_cell_data_func();
void objtree_j_cell_data_func();
void objtree_h_cell_data_func();
void objtree_k_cell_data_func();
void objtree_mag_cell_data_func();
void objtree_magsrc_cell_data_func();
void objtree_rise_cell_data_func();
void objtree_set_cell_data_func();
void objtree_transit_cell_data_func();
static GtkTreeModel * create_repeat_model ();
static GtkTreeModel * create_guide_model ();
static GtkTreeModel * create_aomode_model ();
static GtkTreeModel * create_kools_grism_model ();
static GtkTreeModel * create_triccs_filter_model ();
static GtkTreeModel * create_triccs_gain_model ();
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
static void pm_simbad_query ();
static void pm_gaia_query ();
static void addobj_ned_query ();
void addobj_dl();
void pm_dl();
gchar* pm_get_new_radec();
void cc_get_entry_pm_ra();
void cc_get_entry_pm_dec();
void pm_dialog();
//void addobj_dialog();
//void strchg();
//void str_replace();
//gchar *make_simbad_id();
//void update_c_label();
void swap_obj();
