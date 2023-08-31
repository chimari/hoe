// seimei.h for Kyoto Seimei-3.8m Telescope
//             Dec 2021  A. Tajitsu (Okayama Branch, Subaru Telescope, NAOJ)

#define SEIMEI_TIME_FOCUS 180

#define SEIMEI_PC_UI "192.168.1.23"
#define SEIMEI_OBJECT_PATH "object"
#define SEIMEI_PC_SCRIPT "192.168.1.144"
#define SEIMEI_SCRIPT_PATH "script"

#define DEFAULT_SEIMEI_USER "seimei"

///////// KOOLS-IFU

#define KOOLS_TIME_ACQ 60
#define KOOLS_TIME_PC  60
#define KOOLS_TIME_AG  60
#define KOOLS_TIME_READOUT 22

#define KOOLS_SIZE 2
#define KOOLS_X_ARCSEC 7.7
#define KOOLS_Y_ARCSEC 8.1
#define ZWOCAM_X_ARCSEC 105.
#define ZWOCAM_Y_ARCSEC 70.
#define ZWOCAM_RETICLE1_ARCSEC 4.
#define ZWOCAM_RETICLE2_ARCSEC 20.
#define ZWOCAM_RETICLE3_ARCSEC 40.
#define KOOLS_SUNSET_OFFSET 50
#define KOOLS_SUNRISE_OFFSET 50

enum {
  KOOLS_GRISM_VPH_B,
  KOOLS_GRISM_VPH_R_O56,
  KOOLS_GRISM_VPH_R_NO_O56,
  KOOLS_GRISM_VPH683_O56,
  KOOLS_GRISM_VPH683_NO_O56,
  KOOLS_GRISM_VPH495,
  NUM_KOOLS_GRISM
};

static const gchar* kools_grism_name[]={
  "VPH-blue",
  "VPH-red_O56",
  "VPH-red_no_O56",
  "VPH683_O56",
  "VPH683_no_O56",
  "VPH495"
};


typedef struct _KOOLSpara KOOLSpara;
struct _KOOLSpara{
  gint grism;
  gboolean sh;
  gboolean pc;
  gboolean ag;
  gboolean nw;
  gint pa;
};


#ifdef USE_GTK3
static GdkRGBA col_kools_grism [NUM_KOOLS_GRISM]
= {
  {0.80, 0.80, 1.00, 1}, //B
  {1.00, 0.80, 0.80, 1}, //R O56
  {1.00, 0.60, 0.60, 1}, //R noO56
  {1.00, 1.00, 0.80, 1}, //683
  {1.00, 1.00, 0.60, 1}, //683 noO56
  {0.80, 1.00, 0.80, 1}  //495
};
#else
static GdkColor col_kools_grism [NUM_KOOLS_GRISM]
= {
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //B
  {0, 0xFFFF, 0xCCCC, 0xCCCC}, //R O56
  {0, 0xFFFF, 0xAAAA, 0xAAAA}, //R noO56
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //683 O56
  {0, 0xFFFF, 0xFFFF, 0xAAAA}, //683 noO56
  {0, 0xCCCC, 0xFFFF, 0xCCCC} //495
};
#endif


////////////////// TriCCS

#define TRICCS_TIME_ACQ 30
#define TRICCS_TIME_PC  60
#define TRICCS_TIME_AG  60
#define TRICCS_TIME_READOUT 3

#define TRICCS_SIZE 15
#define TRICCS_X_ARCMIN 12.6
#define TRICCS_Y_ARCMIN 7.5

#define TRICCS_SUNSET_OFFSET 50
#define TRICCS_SUNRISE_OFFSET 50

enum {
  TRICCS_FILTER_GRI,
  TRICCS_FILTER_GRZ,
  NUM_TRICCS_FILTER
};


static const gchar* triccs_filter_name[]={
  "g/r/i",
  "g/r/z"
};

static const gchar* triccs_filter_prm[]={
  "gri",
  "grz"
};


enum {
  TRICCS_GAIN_AUTO,
  TRICCS_GAIN_1,
  TRICCS_GAIN_2,
  TRICCS_GAIN_4,
  TRICCS_GAIN_8,
  TRICCS_GAIN_16,
  TRICCS_GAIN_32,
  NUM_TRICCS_GAIN
};

static const gchar* triccs_gain_name[]={
  "auto",
  "1",
  "2",
  "4",
  "8",
  "16",
  "32"
};

typedef struct _TRICCSpara TRICCSpara;
struct _TRICCSpara{
  gint filter;
  gint gain;
  gint frames;
  gboolean sh;
  gboolean pc;
  gboolean ag;
  gint pa;
};

#ifdef USE_GTK3
static GdkRGBA col_triccs_filter [NUM_TRICCS_FILTER]
= {
  {0.80, 0.80, 1.00, 1}, //gri
  {1.00, 0.80, 0.80, 1}  //grz
};
#else
static GdkColor col_kools_grism [NUM_KOOLS_GRISM]
= {
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //gri
  {0, 0xFFFF, 0xCCCC, 0xCCCC}  //grz
};
#endif


void KOOLS_OH_TAB_create();
