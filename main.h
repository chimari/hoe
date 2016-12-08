#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

#undef USE_OSX

#include<gtk/gtk.h>
#include <cairo.h>
#include <cairo-pdf.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#ifdef HAVE_PWD_H
#include<pwd.h>
#endif
#include<sys/types.h>
#include<errno.h>
#include<math.h>
#include<string.h>

#ifdef USE_WIN32
#include <windows.h>
#include <winnt.h>
#endif

#include "libnova/libnova.h"

#define WWW_BROWSER "firefox"
#ifdef USE_WIN32
#define DSS_URL "http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "http://simbad.u-strasbg.fr/simbad/sim-coo?CooDefinedFrames=none&CooEpoch=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2"
#elif defined(USE_OSX)
#define DSS_URL "open http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick\\&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf\\&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "open http://simbad.u-strasbg.fr/simbad/sim-coo?CooDefinedFrames=none\\&CooEpoch=2000\\&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf\\&submit=submit%%20query\\&Radius.unit=arcmin\\&CooEqui=2000\\&CooFrame=FK5\\&Radius=2"
#else
#define DSS_URL "\"http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey\""
#define SIMBAD_URL "\"http://simbad.u-strasbg.fr/simbad/sim-coo?CooDefinedFrames=none&CooEpoch=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2\""
#endif

#define DSS_ARCMIN_MIN 1
#define DSS_ARCMIN 3
#define DSS_ARCMIN_MAX 60
#define DSS_PIX 500

#define FC_HOST_STSCI "archive.stsci.edu"
#define FC_PATH_STSCI "/cgi-bin/dss_search?v=%s&r=%d+%d+%lf&d=%s%d+%d+%lf&e=J2000&h=%d.0&w=%d.0&f=gif&c=none&fov=NONE&v3="
#ifdef USE_WIN32
#define FC_FILE_GIF "dss.gif"
#define FC_FILE_JPEG "dss.jpg"
#define FC_FILE_HTML "dss.html"
#else
#define FC_FILE_GIF "/tmp/dss.gif"
#define FC_FILE_JPEG "/tmp/dss.jpg"
#define FC_FILE_HTML "/tmp/dss.html"
#endif

#define FC_HOST_ESO "archive.eso.org"
#define FC_PATH_ESO "/dss/dss?ra=%d%%20%d%%20%lf&dec=%s%d%%20%d%%20%lf&mime-type=image/gif&x=%d.0&y=%d.0&Sky-Survey=%s"


#define FC_HOST_SKYVIEW "skyview.gsfc.nasa.gov"
#define FC_PATH_SKYVIEW "/current/cgi/runquery.pl?survey=%s&coordinates=J%.1lf&projection=Tan&scaling=%s&sampler=LI&lut=colortables/blue-white.bin&size=%lf,%lf&pixels=%d&position=%lf,%lf"

#define FC_SRC_STSCI_DSS1R "poss1_red"
#define FC_SRC_STSCI_DSS1B "poss1_blue"
#define FC_SRC_STSCI_DSS2R "poss2ukstu_red"
#define FC_SRC_STSCI_DSS2B "poss2ukstu_blue"
#define FC_SRC_STSCI_DSS2IR "poss2ukstu_ir"

#define FC_SRC_ESO_DSS2R "DSS2"

#define FC_SRC_SKYVIEW_DSS1R "DSS1%20Red"
#define FC_SRC_SKYVIEW_DSS1B "DSS1%20Blue"
#define FC_SRC_SKYVIEW_DSS2R "DSS2%20Red"
#define FC_SRC_SKYVIEW_DSS2B "DSS2%20Blue"
#define FC_SRC_SKYVIEW_DSS2IR "DSS2%20IR"
#define FC_SRC_SKYVIEW_2MASSJ "2MASS-J"
#define FC_SRC_SKYVIEW_2MASSH "2MASS-H"
#define FC_SRC_SKYVIEW_2MASSK "2MASS-K"

#define FC_HOST_SDSS "casjobs.sdss.org"
#define FC_PATH_SDSS "/ImgCutoutDR7/getjpeg.aspx?ra=%lf&dec=%+lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_SDSS8 "skyservice.pha.jhu.edu"
#define FC_PATH_SDSS8 "/DR8/ImgCutout/getjpeg.aspx?ra=%lf&dec=%lf&scale=%f&opt=&width=%d&height=%d&opt=%s%s&query=%s%s"
#define SDSS_SCALE 0.39612
#define FC_HOST_SDSS10 "skyservice.pha.jhu.edu"
#define FC_PATH_SDSS10 "/DR10/ImgCutout/getjpeg.aspx?ra=%lf&dec=%lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_SDSS12 "skyservice.pha.jhu.edu"
#define FC_PATH_SDSS12 "/DR12/ImgCutout/getjpeg.aspx?ra=%lf&dec=%lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"


#define HDS_SLIT_MASK_ARCSEC 9.2

#define SPCAM_X_ARCMIN 34
#define SPCAM_Y_ARCMIN 27
#define SPCAM_GAP_ARCSEC 14.

#define FOCAS_R_ARCMIN 6
#define FOCAS_GAP_ARCSEC 5.

#define IRCS_X_ARCSEC 54.
#define IRCS_Y_ARCSEC 54.

#define MOIRCS_X_ARCMIN 4.0
#define MOIRCS_Y_ARCMIN 7.0
#define MOIRCS_GAP_ARCSEC 2.
#define MOIRCS_VIG1X_ARCSEC 29.
#define MOIRCS_VIG1Y_ARCSEC 29.
#define MOIRCS_VIG2X_ARCSEC 47.
#define MOIRCS_VIG2Y_ARCSEC 45.
#define MOIRCS_VIGR_ARCMIN 6.

#define HOE_HTTP_ERROR_GETHOST  -1
#define HOE_HTTP_ERROR_SOCKET   -2
#define HOE_HTTP_ERROR_CONNECT  -3
#define HOE_HTTP_ERROR_TEMPFILE -4

// Sky Monitor
#if GTK_CHECK_VERSION(2,8,0)
#define USE_SKYMON
#else
#undef USE_SKYMON
#endif

#ifdef USE_SKYMON
#define SKYMON_SIZE 500
#define SKYMON_FONT "Suns"
#endif

// ポップアップメッセージ
#define GTK_MSG
// エラーポップアップのタイムアウト[sec]
#define POPUP_TIMEOUT 2


//#define VERSION "0.8.0"
#define AZEL_INTERVAL 60*1000

#ifdef USE_SKYMON
#define SKYMON_INTERVAL 200
#define SKYMON_STEP 10
#endif

#define MAX_OBJECT 500
#define MAX_PLAN 200
#define MAX_PP 200

#define DEF_EXP 600

#define DEF_SV_EXP 1000
#define DEF_SV_CALC 60
#define DEF_SV_SLITX 314.0
#define DEF_SV_SLITY 315.0
#define DEF_SV_ISX 411.0
#define DEF_SV_ISY 332.0
#define DEF_SV_IS3X 446.0
#define DEF_SV_IS3Y 346.0

#define CAMZ_B -356
#define CAMZ_R -326

#define D_CROSS 130

#define MAG_SVFILTER1 8.0
#define MAG_SVFILTER2 4.0

#define BUFFSIZE 256

#define LONGITUDE_SUBARU -155.4706 //[deg]
#define LATITUDE_SUBARU 19.8255    //[deg]
#define ALTITUDE_SUBARU 4163    //[m]
//#define LONGITUDE_SUBARU -(155.+28./60.+50./3600.) //[deg]
//#define LATITUDE_SUBARU (19.+49./60.+43./3600.)    //[deg]

#define TIMEZONE_SUBARU -10
#define WAVE1_SUBARU 3500   //A
#define WAVE0_SUBARU 5500   //A
#define TEMP_SUBARU 0       //C
#define PRES_SUBARU 625     //hPa

#define DEF_ECHELLE 900


#define TIME_SETUP_FIELD 300
#define TIME_ACQ 60
#define TIME_FOCUS_AG 300
#define TIME_SETUP_FULL 600
#define TIME_SETUP_EASY 120
#define TIME_SETUP_SLIT 60
#define TIME_I2 60
#define TIME_COMP 180
#define TIME_FLAT 180




// Setup
enum{ StdUb, StdUa, StdBa, StdBc, StdYa, StdI2b, StdYd, StdYb, StdYc, StdI2a, StdRa, StdRb, StdNIRc, StdNIRb, StdNIRa, StdHa} StdSetup;

// Binning Mode
enum{ BIN11, BIN21, BIN22, BIN24, BIN41, BIN44} BinMode;
#define MAX_BINNING BIN44+1
#define HALF_BINNING BIN22+1

enum{ AZEL_NORMAL, AZEL_POSI, AZEL_NEGA} AZElMode;

// Image Rotator
enum{ IMR_NO, IMR_LINK, IMR_ZENITH} ImRMode;

// Image Slicer
enum{ IS_NO, IS_030X5, IS_045X3, IS_020X3} ISMode;

#define IS_FLAT_FACTOR 1.35

// Color
enum{COL_BLUE, COL_RED} CrossColor;

enum{
  PLAN_TYPE_COMMENT,
  PLAN_TYPE_OBJ,
  PLAN_TYPE_FOCUS,
  PLAN_TYPE_BIAS,
  PLAN_TYPE_FLAT,
  PLAN_TYPE_COMP,
  PLAN_TYPE_SETUP,
  PLAN_TYPE_I2,
  PLAN_TYPE_SetAzEl
} PlanType;

enum{
  PLAN_CMODE_FULL,
  PLAN_CMODE_EASY,
  PLAN_CMODE_SLIT
} PlanCmode;

enum{
  PLAN_FOCUS_SV,
  PLAN_FOCUS_AG
} PlanFocus;

enum{
  PLAN_OMODE_FULL,
  PLAN_OMODE_SET,
  PLAN_OMODE_GET
} PlanOmode;

enum{
  PLAN_I2_IN,
  PLAN_I2_OUT
} PlanI2;

enum{
  PLAN_START_EVENING,
  PLAN_START_SPECIFIC
} PlanStart;

enum{
  PLAN_COMMENT_TEXT,
  PLAN_COMMENT_SUNRISE,
  PLAN_COMMENT_SUNSET
} PlanComment;

enum{
  SV_FILTER_NONE,
  SV_FILTER_R,
  SV_FILTER_BP530,
  SV_FILTER_ND2,
} SVFilter;


enum
{
  COLUMN_OBJTREE_CHECK,
  COLUMN_OBJTREE_NUMBER,
  COLUMN_OBJTREE_NAME,
  COLUMN_OBJTREE_EXP,
  COLUMN_OBJTREE_REPEAT,
  COLUMN_OBJTREE_MAG,
  COLUMN_OBJTREE_RA,
  COLUMN_OBJTREE_DEC,
  COLUMN_OBJTREE_EPOCH,
  COLUMN_OBJTREE_HORIZON,
  COLUMN_OBJTREE_RISE,
  COLUMN_OBJTREE_RISE_COL,
  COLUMN_OBJTREE_TRANSIT,
  COLUMN_OBJTREE_TRANSIT_COL,
  COLUMN_OBJTREE_SET,
  COLUMN_OBJTREE_SET_COL,
  COLUMN_OBJTREE_PA,
  COLUMN_OBJTREE_GUIDE,
  COLUMN_OBJTREE_SETUP1,
  COLUMN_OBJTREE_SETUP2,
  COLUMN_OBJTREE_SETUP3,
  COLUMN_OBJTREE_SETUP4,
  COLUMN_OBJTREE_SETUP5,
  COLUMN_OBJTREE_NOTE,
  NUM_OBJTREE_COLUMNS
};


enum
{
  COLUMN_NUMBER_TEXT,
  NUM_NUMBER_COLUMNS
};



#define MAX_SETUP StdHa+1

#define MAX_USESETUP 5
// You should edit create_items_model in objtree.c, 
// when you change this value. (Number of G_TYPE_BOOLEAN)
// Add COLUMN_OBJTREE_SETUP%d above, too

#define MAX_NONSTD 4

#define PS_FILE "plot.ps"
#define PA_INPUT "pos.ip"

#define OPE_EXTENSION "ope"
#define HOE_EXTENSION "hoe"
#define LIST1_EXTENSION "list"
#define LIST2_EXTENSION "lst"
#define LIST3_EXTENSION "txt"
#define PLAN_EXTENSION "_plan.txt"
#define PDF_EXTENSION "pdf"
#define YAML_EXTENSION "yml"

#define MAX_LINE 20
enum{PLOT_PSFILE, PLOT_XWIN} plot_device;
enum{MODE_EFS, MODE_FSR} ModeEFS;

// OCS version
enum{ OCS_SOSS, OCS_GEN2} OCSVer;


// Finding Chart
enum{FC_STSCI_DSS1R, 
     FC_STSCI_DSS1B, 
     FC_STSCI_DSS2R,
     FC_STSCI_DSS2B,
     FC_STSCI_DSS2IR,
     FC_ESO_DSS2R,
     FC_SKYVIEW_DSS1R,
     FC_SKYVIEW_DSS1B,
     FC_SKYVIEW_DSS2R,
     FC_SKYVIEW_DSS2B,
     FC_SKYVIEW_DSS2IR,
     FC_SKYVIEW_2MASSJ,
     FC_SKYVIEW_2MASSH,
     FC_SKYVIEW_2MASSK,
     FC_SDSS,
     FC_SDSS8,
     FC_SDSS12} ModeFC;

// Guiding mode
enum{ NO_GUIDE, AG_GUIDE, SV_GUIDE, SVSAFE_GUIDE, NUM_GUIDE_MODE} GuideMode;

// SV Read Area
enum{ SV_PART, SV_FULL} SVArea;

#ifdef USE_SKYMON
// SKYMON Mode
enum{ SKYMON_CUR, SKYMON_SET, SKYMON_PLAN_OBJ, SKYMON_PLAN_TIME} SkymonMode;

#define SUNSET_OFFSET 25
#define SUNRISE_OFFSET 25

#define SKYMON_DEF_OBJSZ 10
#endif

// SIZE　OF GUI ENTRY
#define SMALL_ENTRY_SIZE 24
#define LARGE_ENTRY_SIZE 28


// SOSs
#define SOSS_HOSTNAME "sumda.sum.subaru.nao.ac.jp"
#define SOSS_PATH "Procedure"	 //#define SOSS_PATH "tmp"
#define PY_COM "python"
#define SFTP_PY "hoe_sftp.py"
#define SFTP_LOG "hoe_sftp.log"

// Plot Mode
enum{ PLOT_EL, PLOT_AZ, PLOT_AD} PlotMode;
enum{ PLOT_OBJTREE, PLOT_PLAN} PlotTarget;
enum{ PLOT_OUTPUT_WINDOW, PLOT_OUTPUT_PDF} PlotOutput;
enum{ SKYMON_OUTPUT_WINDOW, SKYMON_OUTPUT_PDF} SkymonOutput;
enum{ PLOT_ALL_SINGLE, PLOT_ALL_SELECTED,PLOT_ALL_ALL,PLOT_ALL_PLAN} PlotAll;

#define PLOT_INTERVAL 60*1000

#define PLOT_WIDTH 600
#define PLOT_HEIGHT 400

#define FC_WIDTH 400
#define FC_HEIGHT 400
enum{ FC_OUTPUT_WINDOW, FC_OUTPUT_PDF, FC_OUTPUT_PDF_ALL} FCOutput;
enum{ FC_INST_NONE, FC_INST_HDS, FC_INST_IRCS, FC_INST_FOCAS, FC_INST_MOIRCS, FC_INST_SPCAM} FCInst;

#define EFS_WIDTH 800
#define EFS_HEIGHT 600
enum{ EFS_PLOT_EFS, EFS_PLOT_FSR} EFSMode;
enum{ EFS_OUTPUT_WINDOW, EFS_OUTPUT_PDF} EFSOutput;

#define SKYMON_WIDTH 600
#define SKYMON_HEIGHT 600


typedef struct _SetupEntry SetupEntry;
struct _SetupEntry{
  gchar *initial;
  gchar *col;
  gchar *cross;
  gchar *fil1;
  gchar *fil2;

  gchar *f1_amp;
  guint f1_fil1;
  guint f1_fil2;
  guint f1_fil3;
  guint f1_fil4;
  guint f1_exp;

  gchar *f2_amp;
  guint f2_fil1;
  guint f2_fil2;
  guint f2_fil3;
  guint f2_fil4;
  guint f2_exp;

  gdouble slit_length;
  gdouble cross_scan;
};


typedef struct _Binpara Binpara;
struct _Binpara{
  gchar *name;
  guint x;
  guint y;
  guint readout;
};


typedef struct _OBJpara OBJpara;
struct _OBJpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble epoch;
  gdouble mag;

  gint horizon;
  gdouble rise;
  gdouble transit;
  gdouble set;

  gint guide;
  
  gdouble pa;
  gboolean setup[MAX_USESETUP];

  gint exp;
  gint repeat;
  gint svfilter;

  /*
  GtkWidget *w_az;
  GtkWidget *w_ha;
  GtkWidget *w_pa;
  */
  gdouble c_az;
  gdouble c_el;
  gdouble c_ha;
  gdouble c_pa;
  gdouble c_ad;

#ifdef USE_SKYMON
  gdouble s_az;
  gdouble s_el;
  gdouble s_ha;
  gdouble s_pa;
  gdouble s_ad;
#endif

  gboolean check_sm;

  gchar *note;
};


typedef struct _PLANpara PLANpara;
struct _PLANpara{
  guint type;
  gchar *txt;
  
  guint setup;  // Obj Flat
  guint repeat;  // Obj Flat Bias
  gboolean slit_or;
  guint slit_width;  
  guint slit_length;
  
  // Get Object
  gint  obj_i;
  //gchar *objname;
  //gchar *objnote;
  //gchar *tgt;
  guint exp;

  guint omode;
  gint guide;


  // BIAS
  // Flat
  // Comp
  // Focus
  guint focus_mode;

  // Setup
  guint cmode;

  // I2
  guint i2_pos;

  gboolean daytime;
  
  gchar *comment;
  guint comtype;

  gint time;
  glong sod;

  gboolean pa_or;
  gdouble pa;
  gboolean sv_or;
  guint sv_exp;
  guint sv_fil;
  gboolean backup;

  gdouble setaz;
  gdouble setel;

  gdouble az0;
  gdouble az1;
  gdouble el0;
  gdouble el1;

  gchar *txt_az;
  gchar *txt_el;
};


typedef struct _PPpara PPpara;
struct _PPpara{
  gdouble ut;

  gdouble az;
  gdouble el;

  guint i_plan;
  
  gboolean start;
};


typedef struct _PApara PApara;
struct _PApara{
  gchar *name;
  gdouble ra;
  gdouble dec;

  gint year;
  gint month;
  gint day;
};


typedef struct _Setuppara Setuppara;
struct _Setuppara{
  gint  setup;
  gboolean use;
  guint    binning;
  guint    slit_width;
  guint    slit_length;
  gchar    *fil1;
  gchar    *fil2;
  guint    imr;
  guint    is;
  gboolean i2;
};


typedef struct _Nonstdpara Nonstdpara;
struct _Nonstdpara{
  guint col;
  guint cross;
  gint  echelle;
  gint  camr;
};

typedef struct _Linepara Linepara;
struct _Linepara{
  gchar *name;
  gdouble  wave;
};

typedef struct _HMSpara my_hms;
struct _HMSpara{
  gint hours;
  gint minutes;
  gdouble seconds;
};

typedef struct _Moonpara typMoon;
struct _Moonpara{
  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  gdouble c_ha;
  gdouble c_disk;
  gdouble c_phase;
  gdouble c_limb;
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

#ifdef USE_SKYMON
  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  gdouble s_ha;
  gdouble s_disk;
  gdouble s_phase;
  gdouble s_limb;
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;

  struct ln_hms p_ra[MAX_PP];
  struct ln_dms p_dec[MAX_PP];
  gdouble p_az[MAX_PP];
  gdouble p_el[MAX_PP];
#endif
};

typedef struct _Sunpara typSun;
struct _Sunpara{
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

#ifdef USE_SKYMON
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;
#endif
};


typedef struct _typHOE typHOE;
struct _typHOE{
  GtkWidget *w_top;
  GtkWidget *w_box;
  GtkWidget *all_note;
  GtkWidget *scrwin;

  GtkWidget *plan_note;

  GtkWidget *pbar;
  GtkWidget *plabel;
  GtkWidget *pbar2;

#ifdef USE_SKYMON
  guint skymon_timer;
#endif
  gchar *filename_read;
  gchar *filename_write;
  gchar *filename_pdf;
  gchar *filename_txt;
  gchar *filename_hoe;
  gchar *filename_log;
  gchar *filehead;

  gint i_max;
  
  OBJpara obj[MAX_OBJECT];
  PLANpara plan[MAX_PLAN];

  PPpara pp[MAX_PP];

  gint fr_year,fr_month,fr_day;
  gint nights;
  gchar *prop_id;
  gchar *prop_pass;
  gchar *observer;
  gint timezone;
  gint ocs;

  guint wave1;
  guint wave0;
  guint pres;
  gint  temp;

  Setuppara setup[MAX_USESETUP];

  Binpara binning[MAX_BINNING];

  gint camz_b;
  gint camz_r;
  gint d_cross;

  guint exptime_factor;
  guint brightness;
  guint sv_area;
  guint sv_integrate;
  guint sv_acq;
  guint sv_region;
  guint sv_calc;
  guint exptime_sv;
  gdouble sv_slitx;
  gdouble sv_slity;
  gdouble sv_isx;
  gdouble sv_isy;
  gdouble sv_is3x;
  gdouble sv_is3y;

  gint def_guide;
  gdouble def_pa;
  guint def_exp;
  
  Nonstdpara nonstd[MAX_NONSTD];

  Linepara line[MAX_LINE];

  guint efs_setup;
  gchar* efs_ps;

  guint e_list;
  guint e_exp;
  guint e_times;
  GtkWidget *e_entry;
  GtkAdjustment *e_adj;
  GtkWidget *e_button[MAX_USESETUP];
  //GtkWidget *c_label;

  gint lst_hour;
  gint lst_min;
  gint lst_sec;

#ifdef USE_SKYMON
  gint skymon_lst_hour;
  gint skymon_lst_min;
  gint skymon_lst_sec;
#endif

  gchar *add_obj;
  GtkWidget *e_add_obj;
  gint add_num;

  GtkWidget *e_exp8mag;
  gint exp8mag;

  gint azel_mode;

  gboolean flag_bunnei;
  gboolean flag_secz;
  gdouble secz_factor;

#ifdef USE_SKYMON
  GtkWidget *skymon_main;
  GtkWidget *skymon_dw;
  GtkWidget *skymon_frame_mode;
  GtkWidget *skymon_frame_date;
  GtkWidget *skymon_frame_time;
  GtkWidget *skymon_frame_sz;
  GtkWidget *skymon_button_set;
  GtkWidget *skymon_button_fwd;
  GtkWidget *skymon_button_rev;
  GtkWidget *skymon_button_morn;
  GtkWidget *skymon_button_even;
  GtkAdjustment *skymon_adj_year;
  GtkAdjustment *skymon_adj_month;
  GtkAdjustment *skymon_adj_day;
  GtkAdjustment *skymon_adj_hour;
  GtkAdjustment *skymon_adj_min;
  gint skymon_mode;
  gint skymon_year,skymon_month,skymon_day,skymon_min,skymon_hour;
  gint skymon_objsz;
#endif

  typMoon moon;
  typSun sun;
  typSun atw;

  gint plot_mode;
  gint plot_target;
  gboolean plot_moon;
  GtkWidget *plot_main;
  GtkWidget *plot_dw;
  gint plot_i;
  gint plot_i_plan;
  gint plot_output;
  gint skymon_output;

  gint efs_mode;
  GtkWidget *efs_main;
  GtkWidget *efs_dw;
  gint efs_output;

  GtkWidget *tree;
  GtkWidget *tree_label;
  guint tree_focus;

  GtkWidget *objtree;
  GtkWidget *sw_objtree;

  GtkWidget *linetree;
  GtkWidget *sw_linetree;

  gchar *www_com;

  gint fc_mode;
  gint fc_inst;
  gint fc_output;
  gint dss_arcmin;
  gint dss_arcmin_ip;
  gint dss_pix;
  gboolean dss_hist;

  gint dss_i;
  gchar *dss_host;
  gchar *dss_path;
  gchar *dss_src;
  gchar *dss_tmp;
  gchar *dss_file;
  gint dss_pa;
  gboolean dss_flip;
  gboolean dss_draw_slit;
  gboolean sdss_photo;
  gboolean sdss_spec;
  GtkWidget *fc_main;
  GtkWidget *fc_dw;

  guint alldss_check_timer;

  GtkWidget *plan_tree;
  guint i_plan_max;

  guint i_pp_max;
  guint i_pp_moon_max;

  GtkWidget *b_objtree_add;
  GtkWidget *b_objtree_remove;

  gboolean  plan_tmp_or;
  guint  plan_tmp_sw;
  guint  plan_tmp_sl;
  guint  plan_tmp_setup;

  GtkAdjustment *plan_obj_adj;
  GtkWidget *plan_obj_guide_combo;
  gint  plan_obj_i;
  guint  plan_obj_exp;
  guint  plan_obj_repeat;

  guint  plan_obj_omode;
  guint  plan_obj_guide;

  gboolean plan_setazel_daytime;
  gdouble plan_setaz;
  gdouble plan_setel;

  guint  plan_bias_repeat;
  gboolean plan_bias_daytime;

  gboolean plan_comp_daytime;

  guint  plan_flat_repeat;
  gboolean plan_flat_daytime;

  guint  plan_focus_mode;

  guint  plan_setup_cmode;
  gboolean plan_setup_daytime;

  guint  plan_i2_pos;
  gboolean plan_i2_daytime;

  gchar  *plan_comment;
  guint  plan_comment_time;
  guint  plan_comment_type;

  guint  plan_start;
  guint  plan_start_hour;
  guint  plan_start_min;

  guint  plan_delay;

  gint plot_all;
};


// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_obj;
}confPA;



// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_use;
  GtkWidget *length_entry;
  GtkWidget *fil1_combo;
  GtkWidget *fil2_combo;
}confSetup;

// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_use;
}confEdit;

#define is_num_char(c) ((c>='0' && c<='9')||(c==' ')||(c=='\t')||(c=='.')||(c=='+')||(c=='-')||(c=='\n'))

