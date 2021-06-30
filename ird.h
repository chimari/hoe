#define IRD_TIME_ACQ 60
#define IRD_TIME_READOUT 8
#define IRD_NGS_MAG1            10
#define IRD_NGS_MAG2            14
#define IRD_TIME_AO_NGS1        240  // NGS mag < MAG1
#define IRD_TIME_AO_NGS2        600  // NGS mag = MAG1 - MAG2
#define IRD_TIME_AO_NGS3        900  // NGS mag > MAG2
#define IRD_TIME_AO_LGS         900  // LGS 


#define IRD_DEF_PA 0

#define IRD_SIZE 3
#define IRD_X_ARCSEC 20.
#define IRD_Y_ARCSEC 10.
#define IRD_TTGS_ARCMIN 2


#define IRD_SET_HOST "www.naoj.org"
#define IRD_SET_PATH "/Observing/Instruments/ird_set.ini"
#define IRD_SET_FILE "ird_set.ini"

enum {
  IRD_COMP_STAR_COMB,
  IRD_COMP_STAR_THAR,
  IRD_COMP_COMB_THAR,
  NUM_IRD_COMP_MODE
};

static const gchar* ird_comp_mode_name[]={
  "Star fiber w/ Comb  PF=-62",
  "Star fiber w/ ThAr  ND No.12",
  "Comb fiber w/ ThAr  ND No.8"
};

static const gchar* ird_comp_obj[]={
  "comb_star",
  "ThAr_star",
  "ThAr_comb"
};

static const gdouble ird_comp_exp[]={
  60, 1.5, 1.5
};

static const gint ird_comp_repeat[]={
  5, 30, 30
};


enum {
  IRD_FLAT_COMB,
  IRD_FLAT_STAR,
  NUM_IRD_FLAT_MODE
};

static const gchar* ird_flat_mode_name[]={
  "Comb fiber w/ NsIR-CAL [3.0A]",
  "Star fiber w/ NsIR-CAL [2.0A]",
};

static const gchar* ird_flat_obj[]={
  "flat_comb",
  "flat_star"
};

static const gdouble ird_flat_exp[]={
  27, 15
};

static const gint ird_flat_repeat[]={
  50, 50
};


void IRD_OH_TAB_create();
void IRD_param_init();
void IRD_WriteOPE();
void IRD_WriteOPE_OBJ();

void IRD_WriteOPE_OBJ_plan();

void IRD_WriteOPE_COMP_plan();
void IRD_WriteOPE_FLAT_plan();
void IRD_WriteOPE_DARK_plan();
void IRD_WriteOPE_FOCUS_plan();

gint get_pf();
gint ird_oh_ao();

void ird_do_export_def_list();

void IRD_Read_Overhead();
void ird_sync_overhead();
void ird_overhead_dl();
