// Header for HDS OPE output


// Standard Setup
enum{ StdUb,
      StdUa,
      StdBa,
      StdBc,
      StdYa,
      StdI2b,
      StdYd,
      StdYb,
      StdYc,
      StdI2a,
      StdRa,
      StdRb,
      StdNIRc,
      StdNIRb,
      StdNIRa,
      StdHa,
      MAX_SETUP};

// Binning Mode
enum{BIN11,
     BIN21,
     BIN22,
     BIN24,
     BIN41,
     BIN44,
     MAX_BINNING};

#define MAX_USESETUP 5
// You should edit create_items_model in objtree.c, 
// when you change this value. (Number of G_TYPE_BOOLEAN)
// Add COLUMN_OBJTREE_SETUP%d above, too

#define MAX_NONSTD 4

// Color
enum{COL_BLUE, COL_RED};


static const char* binname[]={"1x1 [86s]",
			      "2x1 [60s]",
			      "2x2 [44s]",
			      "2x4 [36s]",
			      "4x1 [44s]",
			      "4x4 [33s]"};

static const char* filtername1[]={"Free",
				  "OG530",
				  "SQ",
				  "U340",
				  "ND1",
				  "Halpha",
				  "O5007"};
#define MAX_FILTER1 7

static const char* filtername2[]={"Free",
				  "KV370",
				  "KV389",
				  "SC42",
				  "SC46",
				  "GG495"};
#define MAX_FILTER2 6

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

// Ya is temporary (using Yb setting)
static const SetupEntry HDS_setups[] = {
  {"Ub",  "Blue","Blue",  "Free",  "Free",  "4.0",2,1,3,2,24, "4.0",1,1,1,2,16, 4.0,17100}, 
  {"Ua",  "Blue","Blue",  "Free",  "Free",  "4.0",2,1,3,2,24, "4.0",1,1,1,2,16, 4.0,17820}, 
  {"Ba",  "Blue","Blue",  "Free",  "Free",  "4.0",2,1,1,2,12, "4.0",1,1,2,2, 4, 5.0,19260}, 
  {"Bc",  "Blue","Blue",  "Free",  "Free",  "4.0",2,1,1,2,12, "4.0",1,1,2,2, 4, 6.0,19890}, 
  {"Ya",  "Blue","Blue",  "Free",  "Free",  "4.0",2,1,1,2,12, "4.0",2,1,1,2, 24, 8.0,21960}, 
  {"I2b", "Red", "Red",   "Free",  "Free",  "3.0",2,1,1,2,16, "4.0",2,1,1,2, 16, 3.6,14040}, 
  {"Yd",  "Red", "Red",   "Free",  "Free",  "3.0",2,1,1,2,12, "4.0",2,1,1,2, 8, 4.4,15480}, 
  {"Yb",  "Red", "Red",   "Free",  "KV370", "3.0",2,1,1,2,12, "4.0",2,1,1,2, 8, 4.4,15730}, 
  {"Yc",  "Red", "Red",   "Free",  "KV389",  "3.0",2,1,1,2,12, "4.0",2,1,1,2, 5, 5.0,16500}, 
  {"I2a", "Red", "Red",   "Free",  "SC46",  "3.0",2,1,1,2,12, "3.0",2,1,1,2, 12, 7.0,18000}, 
  {"Ra",  "Red", "Red",   "Free",  "SC46",  "3.0",2,1,1,2,12, "3.0",2,1,1,2, 12, 7.0,18455}, 
  {"Rb",  "Red", "Red",   "Free",  "SC46",  "3.0",2,1,1,2,12, "3.0",2,1,1,2, 12, 8.0,19080}, 
  {"NIRc","Red", "Red",   "OG530", "Free",  "3.0",2,1,1,2,10, "3.0",2,1,1,2,10, 10.0,21360}, 
  {"NIRb","Red", "Red",   "OG530", "Free",  "3.0",2,1,1,2,10, "3.0",2,1,1,2,10, 10.0,22860}, 
  {"NIRa","Red", "Red",   "OG530", "Free",  "3.0",2,1,1,2,10, "3.0",2,1,1,2,10, 15.0,25200}, 
  {"Ha",  "Red", "Mirror","Halpha","Free",  "4.0",2,1,1,2,15, "4.0",2,1,1,2,15, 60.0,0}
};


// Image Rotator
enum{ IMR_NO, IMR_LINK, IMR_ZENITH};

void HDS_TAB_create();
void HDS_SVAG_TAB_create();
void HDS_LINE_TAB_create();
void HDS_ETC_TAB_create();

void hds_calc_cross_scan();
void hds_sync_camz();
void hds_cc_get_fil_combo();
void hds_cc_usesetup();
void hds_efs_for_etc();

void HDS_WriteOPE();
void WriteYAML();
void WritePlan();
void HDS_WriteService();
void WritePROMS();
void WriteOPE_BIAS();
void WriteOPE_COMP();
void WriteOPE_FLAT();
void WriteOPE_BIAS_plan();
void WriteYAML_BIAS_plan();
void WriteOPE_COMP_plan();
void WriteYAML_COMP_plan();
void WriteOPE_FOCUS_plan();
void WriteOPE_COMMENT_plan();
void WriteOPE_FLAT_plan();
void WriteYAML_FLAT_plan();
void WriteOPE_OBJ_plan();
void WriteYAML_OBJ_plan();
void WriteOPE_SetUp_plan();
void WriteYAML_SetUp_plan();

#ifdef USE_SSL
void HDS_DownloadLOG();
#endif

void hds_do_etc();
void hds_do_etc_list();
void hds_do_export_def_list();
void hds_do_efs_cairo();
