//    HDS OPE file Editor
//      fc.h : Header for Finding Chart
//                                           2019.01.06  A.Tajitsu

gboolean flagHSCDialog=FALSE;
GdkPixbuf *pixbuf_fc=NULL, *pixbuf2_fc=NULL;
#ifdef USE_GTK3
GdkPixbuf *pixbuf_fcbk=NULL;
#else
GdkPixmap *pixmap_fcbk=NULL;
#endif
gboolean  flag_dssall_finish=FALSE;
gboolean  flag_dssall_kill=FALSE;


static void fcdb_toggle ();

void fc_item_redl();
void fc_item2();

void fc_dl();
void fc_dl_draw_all();

gboolean progress_timeout();

void close_hsc_dither();
void set_hsc_dither();

void do_fc();

void set_fc_frame_col();
void set_fc_frame_col_pdf();

void create_fc_dialog();

void close_fc();
static void thread_cancel_fc();
static gboolean delete_fc();

void draw_fc_obj();

void translate_to_center();
void set_pa();
void rot_pa();
void translate_hsc_offset();
void translate_hsc_dith();
void translate_to_hsc_chip();

#ifdef USE_GTK3
gboolean draw_fc_cb();
#else
gboolean expose_fc_cairo();
#endif
gboolean configure_fc_cb();
gboolean resize_draw_fc();
static gboolean button_draw_fc();
static void refresh_fc();
static void orbit_fc();
static void hsc_dith_ol_fc();

void set_hsc_show_dith_label();
static void hsc_show_dith_back();
static void hsc_show_dith_forward();
static void cc_get_hsc_show_dith();

static void cc_get_fc_inst();
static void cc_get_fc_mode();
static void cc_get_fc_mode_pdf();

static void do_print_fc();
static void draw_page();

gboolean check_dssall ();
glong get_file_size();

static void show_fc_help ();
static void close_fc_help();

void create_fcdb_para_dialog();
void change_fcdb_para();
void radio_fcdb();

gdouble hsc_sat_radius();
void draw_hsc_sat();

void draw_gs();
void draw_nst();
void draw_pts();
void draw_fcdb1();
void draw_fcdb2();
void draw_hds();
void draw_ircs();
void draw_ird();
void draw_comics();
void draw_focas();
void draw_moircs();
void draw_spcam();
void draw_hsc();
void draw_fmos();
void draw_fc_label();
void draw_hsc_dither();
void draw_pa();


///// global functions
/*
gdouble current_yrs();

void fc_item();
void fc_item_trdb();
void fc_item_plan();

void create_fc_all_dialog();

gboolean draw_fc_cairo();

void set_fc_mode();
void cc_get_fc_mode0();

void pdf_fc();
*/
