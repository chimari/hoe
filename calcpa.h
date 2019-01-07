//    hoe : Subaru HDS OPE file Editor
//        calcpa.h :  Header for Calculate Objects' Position & Plot
//                                           2019.01.06  A.Tajitsu


#ifdef USE_GTK3
GdkPixbuf *pixbuf_plot=NULL;
#else
GdkPixmap *pixmap_plot=NULL;
#endif
double paz_moon[200],pel_moon[200];
double JD_moon_stock=0;
struct ln_zonedate moon_transit;
double moon_tr_el;


double adrad();
double new_tu();

void calc_moon_plan();
void calc_moon_topocen();

void close_plot();
#ifdef USE_GTK3
gboolean draw_plot_cb();
#else
gboolean expose_plot_cairo();
#endif
gboolean configure_plot_cb();

void do_plot_moon();

void cc_get_plot_mode();
void cc_get_plot_all();

gdouble set_ul();

gdouble hdspa_deg();


///// global functions
/*
void calcpa2_main();
void calcpa2_skymon();
void calcpa2_plan();

void calc_moon();
void calc_moon_skymon();
void calc_sun_plan();

gboolean draw_plot_cairo();

void refresh_plot();
void pdf_plot();

void create_plot_dialog();

void add_day();
gdouble get_julian_day_of_epoch();

void geocen_to_topocen();

gdouble ra_to_deg();
gdouble dec_to_deg();
gdouble deg_to_ra();
gdouble deg_to_dec();

gdouble deg_sep();
gdouble date_to_jd();
*/
