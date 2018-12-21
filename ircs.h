// Header for Subaru IRCS
//             Dec 2018  A. Tajitsu (Subaru Telescope, NAOJ)

#define IRCS_MAX_SET 20

static const gint IRCS_im_mas[]={
  52,
  20
};

static const gint IRCS_pi_mas[]={
  52,
  20
};

static const gint IRCS_gr_mas[]={
  52,
  20
};

static const gint IRCS_ps_mas[]={
  52,
  20
};

static const gint IRCS_ec_mas[]={
  55,
  -55
};

// Mode
enum{IRCS_MODE_IM,
     IRCS_MODE_PI,
     IRCS_MODE_GR,
     IRCS_MODE_PS,
     IRCS_MODE_EC,
     NUM_IRCS_MODE};


//// Imaging ////
// Scale
enum{IRCS_IM52,
     IRCS_IM20,
     NUM_IRCS_IM};

enum{IRCS_PI52,
     IRCS_PI20,
     NUM_IRCS_PI};

// Dithering
enum{IRCS_IM_DITH_S5,
     IRCS_IM_DITH_S9,
     IRCS_IM_DITH_D5,
     IRCS_IM_DITH_D9,
     IRCS_IM_DITH_NO,
     NUM_IRCS_IM_DITH};

enum{IRCS_PI_DITH_V3,
     IRCS_PI_DITH_V5,
     IRCS_PI_DITH_OS,
     IRCS_PI_DITH_NO,
     NUM_IRCS_PI_DITH};

typedef struct _IRCS_DITH_Entry IRCS_DITH_Entry;
struct _IRCS_DITH_Entry{
  gchar *name;
  gchar *def;
  gint  obj;
  gint  sky;
};

enum{IRCS_DITHOS_OSO,
     IRCS_DITHOS_OSSO,
     NUM_IRCS_DITHOS};

static const IRCS_DITH_Entry IRCS_im_dith[]={
  {"Standard 5", "S5", 5, 0},
  {"Standard 9", "S9", 9, 0},
  {"Diamond 5",  "D5", 5, 0},
  {"Diamond 9",  "D9", 9, 0},
  {"No Dither",  "NO", 1, 0}
};

static const IRCS_DITH_Entry IRCS_pi_dith[]={
  {"Vertical 3", "V3", 3, 0},
  {"Vertical 5", "V5", 5, 0},
  {"Obj/Sky",    "D2XS",  2, 0},
  {"No Dither",  "NO", 1, 0}
};

// 52mas
enum{IRCS_IM52_Y,
     IRCS_IM52_J,
     IRCS_IM52_H,
     IRCS_IM52_K,
     IRCS_IM52_KP,
     IRCS_IM52_HK,
     IRCS_IM52_LP,
     IRCS_IM52_SEP1,
     IRCS_IM52_CH4S,
     IRCS_IM52_HC,
     IRCS_IM52_CH4L,
     IRCS_IM52_FE,
     IRCS_IM52_NB2070,
     IRCS_IM52_NB2090,
     IRCS_IM52_H210,
     IRCS_IM52_BRG,
     IRCS_IM52_KC,
     IRCS_IM52_H2O,
     IRCS_IM52_PAH,
     IRCS_IM52_H3P,
     IRCS_IM52_BRA,
     IRCS_IM52_SEP2,
     IRCS_IM52_YN,
     IRCS_IM52_JN,
     IRCS_IM52_HN,
     IRCS_IM52_KN,
     IRCS_IM52_KNN,
     IRCS_IM52_KPN,
     IRCS_IM52_HKN,
     IRCS_IM52_LPN,
     IRCS_IM52_SEP3,
     IRCS_IM52_NB2070N,
     IRCS_IM52_H210N,
     IRCS_IM52_BRGN,
     IRCS_IM52_H2ON,
     IRCS_IM52_PAHN,
     IRCS_IM52_BRACN,
     IRCS_IM52_BRAN,
     NUM_IRCS_IM52};

enum{IRCS_PI52_Y,
     IRCS_PI52_J,
     IRCS_PI52_H,
     IRCS_PI52_K,
     IRCS_PI52_KP,
     IRCS_PI52_HK,
     IRCS_PI52_LP,
     IRCS_PI52_SEP1,
     IRCS_PI52_H210,
     IRCS_PI52_BRG,
     IRCS_PI52_SEP2,
     IRCS_PI52_JN,
     IRCS_PI52_HN,
     IRCS_PI52_KN,
     IRCS_PI52_KPN,
     IRCS_PI52_HKN,
     IRCS_PI52_LPN,
     NUM_IRCS_PI52};

// 20mas
enum{IRCS_IM20_Y,
     IRCS_IM20_J,
     IRCS_IM20_H,
     IRCS_IM20_K,
     IRCS_IM20_KP,
     IRCS_IM20_HK,
     IRCS_IM20_LP,
     IRCS_IM20_MP,
     IRCS_IM20_SEP1,
     IRCS_IM20_CH4S,
     IRCS_IM20_HC,
     IRCS_IM20_CH4L,
     IRCS_IM20_FE,
     IRCS_IM20_NB2070,
     IRCS_IM20_NB2090,
     IRCS_IM20_H210,
     IRCS_IM20_BRG,
     IRCS_IM20_KC,
     IRCS_IM20_H2O,
     IRCS_IM20_PAH,
     IRCS_IM20_BRAC,
     IRCS_IM20_BRA,
     IRCS_IM20_SEP2,
     IRCS_IM20_YN,
     IRCS_IM20_JN,
     IRCS_IM20_HN,
     IRCS_IM20_KN,
     IRCS_IM20_KNN,
     IRCS_IM20_KPN,
     IRCS_IM20_HKN,
     IRCS_IM20_LPN,
     IRCS_IM20_MPN,
     IRCS_IM20_SEP3,
     IRCS_IM20_NB2070N,
     IRCS_IM20_H210N,
     IRCS_IM20_BRGN,
     IRCS_IM20_H2ON,
     IRCS_IM20_PAHN,
     IRCS_IM20_BRACN,
     IRCS_IM20_BRAN,
     NUM_IRCS_IM20};

enum{IRCS_PI20_Y,
     IRCS_PI20_J,
     IRCS_PI20_H,
     IRCS_PI20_K,
     IRCS_PI20_KP,
     IRCS_PI20_HK,
     IRCS_PI20_LP,
     IRCS_PI20_MP,
     IRCS_PI20_SEP1,
     IRCS_PI20_H210,
     IRCS_PI20_BRG,
     IRCS_PI20_SEP2,
     IRCS_PI20_JN,
     IRCS_PI20_HN,
     IRCS_PI20_KN,
     IRCS_PI20_KPN,
     IRCS_PI20_HKN,
     IRCS_PI20_LPN,
     NUM_IRCS_PI20};

typedef struct _IRCS_IM_Entry IRCS_IM_Entry;
struct _IRCS_IM_Entry{
  gchar *name;
  gchar *def;
  gdouble wv1;
  gdouble wv2;
  guint cw1;
  guint cw2;
  guint cw3;
  guint slw;
  guint pcw1;
  guint pcw2;
  guint pcw3;
  guint pslw;
  gint cam;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;
};


static const IRCS_IM_Entry IRCS_im52_set[] = {
  //Name                   Def         wv1    wv2 cw1 cw2 cw3 slw pc1 pc2 pc3 psl  cam   
  {"Y",                    "Y",      0.969, 1.071, 13, 14,  1,  1, 13, 14, 13, 11, 1000,   300,  13, 1000}, //IRCS_IM52_Y
  {"J",                    "J",      1.170, 1.330,  1,  3,  1,  1,  1,  3, 13, 11, 1000,   300,  13, 1000}, //IRCS_IM52_J  
  {"H",                    "H",      1.490, 1.780,  1,  5,  1,  1,  1,  5, 13, 11,  750,   200,   4,  200}, //IRCS_IM52_H,     
  {"K",                    "K",      2.030, 2.370,  1,  7,  1,  1,  1,  7, 13, 11, 1000,   300,   5,  330}, //IRCS_IM52_K,
  {"K-prime",              "KP",     1.950, 2.290,  1,  9,  1,  1,  1,  9, 13, 11, 1000,   300,   5,  420}, //IRCS_IM52_Kp,    
  {"HK",                   "HK",     1.490, 2.370,  1, 14,  1,  1,  1, 14, 13, 11,  875,   200,   4,  200}, //IRCS_IM52_HK,     
  {"L-prime",              "LP",     3.420, 4.120,  1, 11,  1,  1,  1, 11, 13, 11, 1500,  0.20,   0.006,  0.20}, //IRCS_IM52_Lp,    
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"CH4 short",            "CH4S",   1.520, 1.620, 13,  1,  9,  1,  0,  0,  0,  0,  750,   300,      5, 1000}, //IRCS_IM52_CH4S, 
  {"H cont",               "HC",     1.540, 1.558,  1,  1,  3,  1,  0,  0,  0,  0,  750,   300,      5, 1000}, //IRCS_IM52_HC,
  {"CH4 long",             "CH4L",   1.640, 1.740, 13,  1, 10,  1,  0,  0,  0,  0,  750,   300,      5, 1000}, //IRCS_IM52_CH4L,  
  {"[Fe II]",              "FE",     1.632, 1.656, 13,  1,  4,  1,  0,  0,  0,  0,  750,   300,      5, 1000}, //IRCS_IM52_FE,    
  {"NB2070",               "NB2070", 2.055, 2.085,  1,  9,  6,  1,  0,  0,  0,  0, 1000,   300,      5, 1000}, //IRCS_IM52_NB2070,
  {"NB2090",               "NB2090", 2.073, 2.108,  1,  1,  5,  1,  0,  0,  0,  0, 1000,   300,      5, 1000}, //IRCS_IM52_NB2090,
  {"H2 1-0S(1)",           "H210",   2.106, 2.138, 11,  9,  1,  1, 11,  9, 13, 11, 1000,   300,      5, 1000}, //IRCS_IM52_H210,
  {"Br gamma",             "BRG",    2.150, 2.182, 10,  1,  1,  1, 10,  1, 13, 11, 1000,   300,      5, 1000}, //IRCS_IM52_BRG,
  {"K cont",               "KC",     2.299, 2.329,  1,  7,  8,  1,  0,  0,  0,  0, 1000,   300,      5, 1000}, //IRCS_IM52_KC,    
  {"H2O Ice",              "H2O",    2.974, 3.126,  4,  1,  1,  1,  0,  0,  0,  0, 1500,   300,  0.006, 1000}, //IRCS_IM52_H2O,   
  {"PAH",                  "PAH",    3.270, 3.320,  1,  1,  7,  1,  0,  0,  0,  0, 1500,   0.20, 0.006, 1000}, //IRCS_IM52_PAH,   
  {"H3+",                  "H3P",    3.402, 3.424,  1,  1, 11,  1,  0,  0,  0,  0, 1700,   0.20, 0.006, 1000}, //IRCS_IM52_H3P and BRAC (w/ND),   
  {"Br alpha",             "BRA",    4.032, 4.082,  1,  1, 12,  1,  0,  0,  0,  0, 1700,   0.20, 0.006, 1000}, //IRCS_IM52_BRA,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"Y + ND(1%)",           "YN",     0.969, 1.071, 13, 14,  2,  1,  0,  0,  0,  0, 1000,   300,  13, 1000}, //IRCS_IM52_YN
  {"J + ND(1%)",           "JN",     1.170, 1.330,  1,  3,  2,  1, 12,  3, 13, 11, 1000,   300,  13, 1000}, //IRCS_IM52_JN  
  {"H + ND(1%)",           "HN",     1.490, 1.780,  1,  5,  2,  1, 12,  5, 13, 11,  750,   200,   4,  200}, //IRCS_IM52_HN,     
  {"K + ND(1%)",           "KN",     2.030, 2.370,  1,  7,  2,  1, 12,  7, 13, 11, 1000,   300,   5,  330}, //IRCS_IM52_KN,
  {"K + ND(0.01%)",        "KNN",    2.030, 2.370, 12,  7,  2,  1,  0,  0,  0,  0, 1000,   300,   5,  330}, //IRCS_IM52_KNN,
  {"K-prime + ND(1%)",     "KPN",    1.950, 2.290,  1,  9,  2,  1, 12,  9, 13, 11, 1000,   300,   5,  420}, //IRCS_IM52_KPN,    
  {"HK + ND(1%)",          "HKN",    1.490, 2.370,  1, 14,  2,  1, 12, 14, 13, 11,  875,   200,   4,  200}, //IRCS_IM52_HKN,     
  {"L-prime + ND(1%)",     "LPN",    3.420, 4.120,  1, 11,  2,  1, 12, 11, 13, 11, 1500,  0.20,   0.006,  0.20}, //IRCS_IM52_LPN,    
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"NB2070 + ND(1%)",      "NB2070N",2.055, 2.085, 12,  9,  6,  1,  0,  0,  0,  0, 1000,  300,   5,  1000}, //IRCS_IM52_NB2070N,
  {"H2 1-0S(1) + ND(1%)",  "H210N",  2.106, 2.138, 11,  9,  2,  1,  0,  0,  0,  0, 1000,  300,   5,  1000}, //IRCS_IM52_H210N,
  {"Br gamma + ND(1%)",    "BRGN",   2.150, 2.182, 10,  1,  2,  1,  0,  0,  0,  0, 1000,  300,   5,  1000}, //IRCS_IM52_BRGN,
  {"H2O Ice + ND(1%)",     "H2ON",   2.974, 3.126,  4,  1,  2,  1,  0,  0,  0,  0, 1500,  300,   0.006,  1000}, //IRCS_IM52_H2ON,   
  {"PAH + ND(1%)",         "PAHN",   3.270, 3.320, 12,  1,  7,  1,  0,  0,  0,  0, 1500,  0.20,  0.006,  1000}, //IRCS_IM52_PAHN,   
  {"H3+ + ND(1%)",         "BRACN",  3.402, 3.424, 12,  1, 11,  0,  0,  0,  0,  1, 1700,  0.20,  0.006,  1000}, //IRCS_IM52_BRACN (H3P w/ND),   
  {"Br alpha + ND(1%)",    "BRAN",   4.032, 4.082, 12,  1, 12,  1,  0,  0,  0,  0, 1700,  0.20,  0.006,  1000}  //IRCS_IM52_BRAN,
};

static const IRCS_IM_Entry IRCS_pi52_set[] = {
  //Name                   Def         wv1    wv2 cw1 cw2 cw3 slw pc1 pc2 pc3 psl  cam    
  {"Y",                    "Y",      0.969, 1.071, 13, 14,  1,  1, 13, 14, 13, 11, 1000,    300,  13, 1000}, //IRCS_PI52_Y
  {"J",                    "J",      1.170, 1.330,  1,  3,  1,  1,  1,  3, 13, 11, 1000,    300,  13, 1000}, //IRCS_PI52_J  
  {"H",                    "H",      1.490, 1.780,  1,  5,  1,  1,  1,  5, 13, 11,  750,    200,   4,  200}, //IRCS_PI52_H,     
  {"K",                    "K",      2.030, 2.370,  1,  7,  1,  1,  1,  7, 13, 11, 1000,    300,   5,  330}, //IRCS_PI52_K,
  {"K-prime",              "KP",     1.950, 2.290,  1,  9,  1,  1,  1,  9, 13, 11, 1000,    300,   5,  420}, //IRCS_PI52_Kp,    
  {"HK",                   "HK",     1.490, 2.370,  1, 14,  1,  1,  1, 14, 13, 11,  875,    200,   4,  200}, //IRCS_PI52_HK,     
  {"L-prime",              "LP",     3.420, 4.120,  1, 11,  1,  1,  1, 11, 13, 11, 1500,   0.20,   0.006,  0.20}, //IRCS_PI52_Lp,    
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H2 1-0S(1)",           "H210",   2.106, 2.138, 11,  9,  1,  1, 11,  9, 13, 11, 1000,    300,   5, 1000}, //IRCS_PI52_H210,
  {"Br gamma",             "BRG",    2.150, 2.182, 10,  1,  1,  1, 10,  1, 13, 11, 1000,    300,   5, 1000}, //IRCS_PI52_BRG,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"J + ND(1%)",           "JN",     1.170, 1.330,  1,  3,  2,  1, 12,  3, 13, 11, 1000,    300,  13, 1000}, //IRCS_PI52_JN  
  {"H + ND(1%)",           "HN",     1.490, 1.780,  1,  5,  2,  1, 12,  5, 13, 11,  750,    200,   4,  200}, //IRCS_PI52_HN,     
  {"K + ND(1%)",           "KN",     2.030, 2.370,  1,  7,  2,  1, 12,  7, 13, 11, 1000,    300,   5,  330}, //IRCS_PI52_KN,
  {"K-prime + ND(1%)",     "KPN",    1.950, 2.290,  1,  9,  2,  1, 12,  9, 13, 11, 1000,    300,   5,  420}, //IRCS_PI52_KPN,    
  {"HK + ND(1%)",          "HKN",    1.490, 2.370,  1, 14,  2,  1, 12, 14, 13, 11,  875,    200,   4,  200}, //IRCS_PI52_HKN,     
  {"L-prime + ND(1%)",     "LPN",    3.420, 4.120,  1, 11,  2,  1, 12, 11, 13, 11, 1500,   0.20, 0.006,  0.20} //IRCS_PI52_LPN,    
};

static const IRCS_IM_Entry IRCS_im20_set[] = {
  //Name                       Def         wv1    wv2 cw1 cw2 cw3 slw pc1 pc2 pc3 psl  cam   
  {"Y",                        "20Y",    0.969, 1.071, 13, 14,  1,  1, 13, 14, 13, 11, 2500,  300,   80,  6000}, //IRCS_IM20_Y
  {"J",                        "20J",    1.170, 1.330,  1,  3,  1,  1,  1,  3, 13, 11, 2500,  300,   80,  6000}, //IRCS_IM20_J  
  {"H",                        "20H",    1.490, 1.780,  1,  5,  1,  1,  1,  5, 13, 11, 2500,  300,   12,  1300}, //IRCS_IM20_H,     
  {"K",                        "20K",    2.030, 2.370,  1,  7,  1,  1,  1,  7, 13, 11, 2500,  300,   20,  2100}, //IRCS_IM20_K,
  {"K-prime",                 "20KP",    1.950, 2.290,  1,  9,  1,  1,  1,  9, 13, 11, 2500,  300,   30,  2800}, //IRCS_IM20_Kp,    
  {"HK",                      "20HK",    1.490, 2.370,  1, 14,  1,  1,  1, 14, 13, 11, 2500,  300,   12,  1300}, //IRCS_IM20_HK,    
  {"L-prime",                 "20LP",    3.420, 4.120,  1, 11,  1,  1,  1, 11, 13, 11, 2500,  0.20, 0.006,  0.62}, //IRCS_IM20_Lp,    
  {"M-prime",                   "MP",    4.570, 4.790,  1, 13,  1,  1,  1, 13, 13, 11, 2500,  0.20, 0.006,  0.28}, //IRCS_IM20_Mp,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"CH4 short",             "20CH4S",    1.520, 1.620, 13,  1,  9,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_CH4S,  
  {"H cont",                  "20HC",    1.540, 1.558,  1,  1,  3,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_HC,
  {"CH4 long",              "20CH4L",    1.640, 1.740, 13,  1, 10,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_CH4L,  
  {"[Fe II]",                 "20FE",    1.632, 1.656, 13,  1,  4,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_FE,    
  {"NB2070",              "20NB2070",    2.055, 2.085,  1,  9,  6,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_NB2070,
  {"NB2090",              "20NB2090",    2.073, 2.108,  1,  1,  5,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_NB2090,
  {"H2 1-0S(1)",            "20H210",    2.106, 2.138, 11,  9,  1,  1, 11,  9, 13, 11, 2500,  300,    5,  6000}, //IRCS_IM20_H210,
  {"Br gamma",               "20BRG",    2.150, 2.182, 10,  1,  1,  1, 10,  1, 13, 11, 2500,  300,    5,  6000}, //IRCS_IM20_BRG,
  {"K cont",                  "20KC",    2.299, 2.329,  1,  7,  8,  1,  0,  0,  0,  0, 2500,  300,    5,  6000}, //IRCS_IM20_KC,    
  {"H2O Ice",                "20H2O",    2.974, 3.126,  4,  1,  1,  1,  0,  0,  0,  0, 2500,  0.20,0.006,  1000}, //IRCS_IM20_H2O,   
  {"PAH",                    "20PAH",    3.270, 3.320,  1,  1,  7,  1,  0,  0,  0,  0, 2500,  0.20,0.006,  1000}, //IRCS_IM20_PAH,   
  {"H3+",                    "20H3P",    3.402, 3.424,  1,  1, 11,  1,  0,  0,  0,  0, 2500,  0.20,0.006,  1000}, //IRCS_IM20_H3P and BRAC (w/ND),   
  {"Br alpha",               "20BRA",    4.032, 4.082,  1,  1, 12,  1,  0,  0,  0,  0, 2500,  0.20,0.006,  1000}, //IRCS_IM20_BRA,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"Y + ND(1%)",              "20YN",    0.969, 1.071, 13, 14,  2,  1,  0,  0,  0,  0, 2500,  300,   80,  6000}, //IRCS_IM20_YN
  {"J + ND(1%)",              "20JN",    1.170, 1.330,  1,  3,  2,  1, 12,  3, 13, 11, 2500,  300,   80,  6000}, //IRCS_IM20_JN  
  {"H + ND(1%)",              "20HN",    1.490, 1.780,  1,  5,  2,  1, 12,  5, 13, 11, 2500,  300,   12,  1300}, //IRCS_IM20_HN     
  {"K + ND(1%)",              "20KN",    2.030, 2.370,  1,  7,  2,  1, 12,  7, 13, 11, 2500,  300,   20,  2100}, //IRCS_IM20_KN
  {"K + ND(0.01%)",          "20KNN",    2.030, 2.370, 12,  7,  2,  1,  0,  0,  0,  0, 2500,  300,   20,  2100}, //IRCS_IM20_KNN
  {"K-prime + ND(1%)",       "20KPN",    1.950, 2.290,  1,  9,  2,  1, 12,  9, 13, 11, 2500,  300,   30,  2800}, //IRCS_IM20_KPN,    
  {"HK + ND(1%)",            "20HKN",    1.490, 2.370,  1, 14,  2,  1, 12, 14, 13, 11, 2500,  300,   12,  1300}, //IRCS_IM20_HN     
  {"L-prime + ND(1%)",       "20LPN",    3.420, 4.120,  1, 11,  2,  1, 12, 11, 13, 11, 2500,  0.20, 0.006,  0.62}, //IRCS_IM20_LPN,    
  {"M-prime + ND(1%)",         "MPN",    4.570, 4.790,  1, 13,  2,  1, 12, 13, 13, 11, 2500,  0.20, 0.006,  0.28}, //IRCS_IM20_MPN,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"NB2070 + ND(1%)",    "20NB2070N",    2.055, 2.085, 12,  9,  6,  1,  0,  0,  0,  0, 2500,  300,   12,  6000}, //IRCS_IM20_NB2070N,
  {"H2 1-0S(1) + ND(1%)",  "20H210N",    2.106, 2.138, 11,  9,  2,  1,  0,  0,  0,  0, 2500,  300,   12,  6000}, //IRCS_IM20_H210N,
  {"Br gamma + ND(1%)",     "20BRGN",    2.150, 2.182, 10,  1,  2,  1,  0,  0,  0,  0, 2500,  300,   12,  6000}, //IRCS_IM20_BRGN,
  {"H2O Ice + ND(1%)",      "20H2ON",    2.974, 3.126,  4,  1,  2,  1,  0,  0,  0,  0, 2500,  0.20,  0.006,  6000}, //IRCS_IM20_H2ON,   
  {"PAH + ND(1%)",          "20PAHN",    3.270, 3.320, 12,  1,  7,  1,  0,  0,  0,  0, 2500,  0.20,  0.006,  6000}, //IRCS_IM20_PAHN,   
  {"H3+ + ND(1%)",         "20BRACN",    3.402, 3.424, 12,  1, 11,  1,  0,  0,  0,  0, 2500,  0.20,  0.006,  6000}, //IRCS_IM20_BRACN
  {"Br alpha + ND(1%)",     "20BRAN",    4.032, 4.082, 12,  1, 12,  1,  0,  0,  0,  0, 2500,  0.20,  0.006,  6000}  //IRCS_IM20_BRAN,
};

static const IRCS_IM_Entry IRCS_pi20_set[] = {
  //Name                       Def         wv1    wv2 cw1 cw2 cw3 slw pc1 pc2 pc3 psl  cam   
  {"Y",                        "20Y",    0.969, 1.071, 13, 14,  1,  1, 13, 14, 13, 11, 2500,  300,   80,  6000}, //IRCS_PI20_Y
  {"J",                        "20J",    1.170, 1.330,  1,  3,  1,  1,  1,  3, 13, 11, 2500,  300,   80,  6000}, //IRCS_PI20_J  
  {"H",                        "20H",    1.490, 1.780,  1,  5,  1,  1,  1,  5, 13, 11, 2500,  300,   12,  1300}, //IRCS_PI20_H,     
  {"K",                        "20K",    2.030, 2.370,  1,  7,  1,  1,  1,  7, 13, 11, 2500,  300,   20,  2100}, //IRCS_PI20_K,
  {"K-prime",                 "20KP",    1.950, 2.290,  1,  9,  1,  1,  1,  9, 13, 11, 2500,  300,   30,  2800}, //IRCS_PI20_Kp,    
  {"HK",                      "20HK",    1.490, 2.370,  1, 14,  1,  1,  1, 14, 13, 11, 2500,  300,   12,  1300}, //IRCS_PI20_HK,    
  {"L-prime",                 "20LP",    3.420, 4.120,  1, 11,  1,  1,  1, 11, 13, 11, 2500,  0.20, 0.006,  0.62}, //IRCS_PI20_Lp,    
  {"M-prime",                   "MP",    4.570, 4.790,  1, 13,  1,  1,  1, 13, 13, 11, 2500,  0.20, 0.006,  0.28}, //IRCS_PI20_Mp,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H2 1-0S(1)",            "20H210",    2.106, 2.138, 11,  9,  1,  1, 11,  9, 13, 11, 2500,  300,    5,  6000}, //IRCS_PI20_H210,
  {"Br gamma",               "20BRG",    2.150, 2.182, 10,  1,  1,  1, 10,  1, 13, 11, 2500,  300,    5,  6000}, //IRCS_PI20_BRG,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"J + ND(1%)",              "20JN",    1.170, 1.330,  1,  3,  2,  1, 12,  3, 13, 11, 2500,  300,   80,  6000}, //IRCS_PI20_JN  
  {"H + ND(1%)",              "20HN",    1.490, 1.780,  1,  5,  2,  1, 12,  5, 13, 11, 2500,  300,   12,  1300}, //IRCS_PI20_HN     
  {"K + ND(1%)",              "20KN",    2.030, 2.370,  1,  7,  2,  1, 12,  7, 13, 11, 2500,  300,   20,  2100}, //IRCS_PI20_KN
  {"K-prime + ND(1%)",       "20KPN",    1.950, 2.290,  1,  9,  2,  1, 12,  9, 13, 11, 2500,  300,   30,  2800}, //IRCS_PI20_KPN,    
  {"HK + ND(1%)",            "20HKN",    1.490, 2.370,  1, 14,  2,  1, 12, 14, 13, 11, 2500,  300,   12,  1300}, //IRCS_PI20_HKN     
  {"L-prime + ND(1%)",       "20LPN",    3.420, 4.120,  1, 11,  2,  1, 12, 11, 13, 11, 2500,  0.20, 0.006,  0.62}, //IRCS_PI20_LPN,    
  {"M-prime + ND(1%)",         "MPN",    4.570, 4.790,  1, 13,  2,  1, 12, 13, 13, 11, 2500,  0.20, 0.006,  0.28} //IRCS_PI20_MPN,
};


// Grism
// Scale
enum{IRCS_GR52,
     IRCS_GR20,
     NUM_IRCS_GR};

enum{IRCS_PS52,
     IRCS_PS20,
     NUM_IRCS_PS};

// 52mas
enum{IRCS_GR52_IZ,
     IRCS_GR52_ZJ,
     IRCS_GR52_J,
     IRCS_GR52_H,
     IRCS_GR52_K,
     IRCS_GR52_L,
     IRCS_GR52_ZJH,
     IRCS_GR52_HK,
     NUM_IRCS_GR52};

enum{IRCS_PS52_IZ,
     IRCS_PS52_ZJ,
     IRCS_PS52_J,
     IRCS_PS52_H,
     IRCS_PS52_K,
     IRCS_PS52_L,
     IRCS_PS52_ZJH,
     IRCS_PS52_HK,
     NUM_IRCS_PS52};

// 20mas
enum{IRCS_GR20_IZ,
     IRCS_GR20_ZJ,
     IRCS_GR20_J,
     IRCS_GR20_H,
     IRCS_GR20_K,
     IRCS_GR20_L,
     NUM_IRCS_GR20};

enum{IRCS_PS20_IZ,
     IRCS_PS20_ZJ,
     IRCS_PS20_J,
     IRCS_PS20_H,
     IRCS_PS20_K,
     IRCS_PS20_L,
     NUM_IRCS_PS20};

// Dithering
enum{IRCS_GR_DITH_ABBA,
     IRCS_GR_DITH_OS,
     IRCS_GR_DITH_SS,
     IRCS_GR_DITH_NO,
     NUM_IRCS_GR_DITH};

enum{IRCS_PS_DITH_ABBA,
     IRCS_PS_DITH_OS,
     IRCS_PS_DITH_SS,
     IRCS_PS_DITH_NO,
     NUM_IRCS_PS_DITH};

static const IRCS_DITH_Entry IRCS_gr_dith[]={
  {"A-B-B-A",         "D2",    4, 0},
  {"Obj/Sky",         "D2XS",  2, 0},
  {"Slit Scan",       "SS",    0, 0},
  {"None",            "NO",    1, 0}
};

static const IRCS_DITH_Entry IRCS_ps_dith[]={
  {"A-B-B-A",         "D2",    4, 0},
  {"Obj/Sky",         "D2XS",  2, 0},
  {"Slit Scan",       "SS",    0, 0},
  {"None",            "NO",    1, 0}
};

// Slit
enum{IRCS_GR_WSLIT_010,
     IRCS_GR_WSLIT_015,
     IRCS_GR_WSLIT_0225,
     IRCS_GR_WSLIT_030,
     IRCS_GR_WSLIT_SEP1,
     IRCS_GR_WSLIT_045,
     IRCS_GR_WSLIT_060,
     IRCS_GR_WSLIT_090,
     NUM_IRCS_GR_WSLIT};

enum{IRCS_PS_WSLIT_010P,
     IRCS_PS_WSLIT_015P,
     IRCS_PS_WSLIT_0225P,
     IRCS_PS_WSLIT_060P,
     NUM_IRCS_PS_WSLIT};

enum{IRCS_GR_NSLIT_010,
     IRCS_GR_NSLIT_015,
     IRCS_GR_NSLIT_0225,
     IRCS_GR_NSLIT_SEP1,
     IRCS_GR_NSLIT_045,
     NUM_IRCS_GR_NSLIT};

enum{IRCS_PS_NSLIT_010P,
     IRCS_PS_NSLIT_015P,
     NUM_IRCS_PS_NSLIT};

typedef struct _IRCS_GR_Entry IRCS_GR_Entry;
struct _IRCS_GR_Entry{
  gchar *name;
  gchar *def;
  gdouble wv1;
  gdouble wv2;
  guint r010;
  gdouble disp;
  guint cw1;
  guint cw2;
  guint cw3;
  gint cam;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;
};

#define IRCS_GR_SLW_REFL3 13  // Wide reflective slit
#define IRCS_GR_SLW_REFL4 14

typedef struct _IRCS_GR_SL_Entry IRCS_GR_SL_Entry;
struct _IRCS_GR_SL_Entry{
  gchar *name;
  gdouble width;
  gdouble length;
  guint   rfl;
  guint   slw;
  gdouble slit_x;
  gdouble slit_y;
};

static const IRCS_GR_SL_Entry IRCS_gr_wslit[] = {
  {"0\".10x7\" (Refl4)",   0.10,   7,   4,  14,   491.5,   514.0},
  {"0\".15x7\" (Refl4)",   0.15,   7,   4,  14,   427.0,   514.0},
  {"0\".225x20\" (Refl4)", 0.225, 20,   4,  14,   785.0,   514.0},
  {"0\".30x14\" (Refl4)",  0.30,  14,   4,  14,    75.0,   514.0},
  {NULL, 0, 0, 0, 0, 0, 0},
  {"0\".45x18\" (Refl3W)",  0.45,  18,   3,  13,   505.0,   515.5},
  {"0\".60x15\" (Refl3W)",  0.60,  15,   3,  13,   833.0,   514.0},
  {"0\".90x15\" (Refl3W)",  0.90,  15,   3,  13,   187.0,   516.0}
};

static const IRCS_GR_SL_Entry IRCS_ps_wslit[] = {
  {"Pol: 0\".10x4\".4",    0.10, 4.4,   2,  12,   615.1,   512.5},
  {"Pol: 0\".15x4\".4",    0.15, 4.4,   2,  12,   415.5,   513.0},
  {"Pol: 0\".225x4\".4",  0.225, 4.4,   2,  12,   813.2,   511.5},
  {"Pol: 0\".60x4\".4",    0.60, 4.4,   2,  12,   223.2,   514.5}
};

static const IRCS_GR_SL_Entry IRCS_gr_nslit[] = {
  {"0\".10x7\".0 (Refl4)",  0.10,  7.0,  4,  14,   548.9,   536.5},
  {"0\".15x6\".5 (Refl4)",  0.15,  6.5,  4,  14,   283.0,   537.0},
  {"0\".225x6\".5 (Refl4)",0.225,  6.5,  4,  14,   852.0,   535.5},
  {NULL, 0, 0, 0, 0, 0, 0},
  {"0\".45x18\".0 (Refl3W)", 0.45,  18.0, 3,  13,   313.0,   536.5}
};

static const IRCS_GR_SL_Entry IRCS_ps_nslit[] = {
  {"Pol: 0\".10x4\".4",    0.10, 4.4,   2,  12,   761.0,   530.0},
  {"Pol: 0\".15x4\".4",    0.15, 4.4,   2,  12,   249.0,   532.0}
};


// 52mas wide=TRUE  fc=OUT
static const IRCS_GR_Entry IRCS_gr52_set[] = {
  //Name               Def   wv1   wv2  r010   disp cw1 cw2 cw3  cam   
  {"Iz",              "IZ", 0.95, 1.01, 1940,   2.7,  4,  4,  1, 1000,  300, 0.41, 1000}, //IRCS_GR52_IZ,
  {"zJ",              "ZJ", 1.03, 1.18, 1706,   3.1,  5,  4,  1, 1000,  300, 0.41, 1000}, //IRCS_GR52_ZJ,
  {"J",                "J", 1.18, 1.38, 1432,   3.7,  6,  4,  1, 1000,  300, 0.41, 1000}, //IRCS_GR52_J,
  {"H",                "H", 1.49, 1.83, 1146,   4.7,  7,  4,  1,  750,  300, 0.41, 1000}, //IRCS_GR52_H,
  {"K",                "K", 1.93, 2.48,  869,   6.1,  8,  4,  1, 1000,  300, 0.41, 1000}, //IRCS_GR52_K,
  {"L",                "L", 2.84, 4.16,  331,  15.9,  9, 10,  1, 1500,   10, 0.41, 1000}, //IRCS_GR52_L,
  {"zJH",            "ZJH", 0.95, 1.50,  705,   8.5,  2,  2,  1,  875,  300, 0.41, 1000}, //IRCS_GR52_ZJH,
  {"HK",              "HK", 1.40, 2.50,  442,  12.2,  3,  8,  1,  875,  300, 0.41, 1000}  //IRCS_GR52_HK,
};

static const IRCS_GR_Entry IRCS_ps52_set[] = {
  //Name               Def   wv1   wv2  r010   disp cw1 cw2 cw3  cam   
  {"Iz",              "IZ", 0.95, 1.01, 1940,   2.7,  4,  4, 13, 1000, 300, 0.41, 1000}, //IRCS_PS52_IZ,
  {"zJ",              "ZJ", 1.03, 1.18, 1706,   3.1,  5,  4, 13, 1000, 300, 0.41, 1000}, //IRCS_PS52_ZJ,
  {"J",                "J", 1.18, 1.38, 1432,   3.7,  6,  4, 13, 1000, 300, 0.41, 1000}, //IRCS_PS52_J,
  {"H",                "H", 1.49, 1.83, 1146,   4.7,  7,  4, 13,  750, 300, 0.41, 1000}, //IRCS_PS52_H,
  {"K",                "K", 1.93, 2.48,  869,   6.1,  8,  4, 13, 1000, 300, 0.41, 1000}, //IRCS_PS52_K,
  {"L",                "L", 2.84, 4.16,  331,  15.9,  9, 10, 13, 1500,  10, 0.41, 1000}, //IRCS_PS52_L,
  {"zJH",            "ZJH", 0.95, 1.50,  705,   8.5,  2,  2, 13,  875, 300, 0.41, 1000}, //IRCS_PS52_ZJH,
  {"HK",              "HK", 1.40, 2.50,  442,  12.2,  3,  8, 13,  875, 300, 0.41, 1000}  //IRCS_PS52_HK,
};

// 20mas wide=FALSE  fc=OUT
static const IRCS_GR_Entry IRCS_gr20_set[] = {
  //Name              Def   wv1    wv2  r010  disp cw1 cw2 cw3  cam  
  {"Iz",            "20IZ", 0.95, 1.01,  813,  2.3,  4,  6,  1, 2500, 300, 0.41, 2000}, //IRCS_GR20_IZ,
  {"zJ",            "20ZJ", 1.03, 1.18,  745,  2.7,  5,  6,  1, 2500, 300, 0.41, 2000}, //IRCS_GR20_ZJ,
  {"J",              "20J", 1.19, 1.38,  677,  3.3,  6,  6,  1, 2500, 300, 0.41, 2000}, //IRCS_GR20_J,
  {"H",              "20H", 1.47, 1.80,  501,  4.1,  7,  6,  1, 2500, 300, 0.41, 2000}, //IRCS_GR20_H,
  {"K",              "20K", 1.92, 2.40,  393,  5.4,  8,  6,  1, 2500, 300, 0.41, 2000}, //IRCS_GR20_K,
  {"L",              "20L", 2.84, 4.16,  200,    0,  9, 10,  1, 2500,  10, 0.41, 1000}  //IRCS_GR20_L,
};

static const IRCS_GR_Entry IRCS_ps20_set[] = {
  //Name              Def   wv1    wv2  r010  disp cw1 cw2 cw3  cam  
  {"Iz",            "20IZ", 0.95, 1.01,  813,  2.3,  4,  6, 13, 2500, 300, 0.41, 2000}, //IRCS_PS20_IZ,
  {"zJ",            "20ZJ", 1.03, 1.18,  745,  2.7,  5,  6, 13, 2500, 300, 0.41, 2000}, //IRCS_PS20_ZJ,
  {"J",              "20J", 1.19, 1.38,  677,  3.3,  6,  6, 13, 2500, 300, 0.41, 2000}, //IRCS_PS20_J,
  {"H",              "20H", 1.47, 1.80,  501,  4.1,  7,  6, 13, 2500, 300, 0.41, 2000}, //IRCS_PS20_H,
  {"K",              "20K", 1.92, 2.40,  393,  5.4,  8,  6, 13, 2500, 300, 0.41, 2000}, //IRCS_PS20_K,
  {"L",              "20L", 2.84, 4.16,  200,    0,  9, 10, 13, 2500,  10, 0.41, 1000}  //IRCS_PS20_L,
};


// Echelle
enum{IRCS_ECD,
     IRCS_ECM,
     NUM_IRCS_EC};

enum{IRCS_ECD_IZ,
     IRCS_ECD_ZJ,
     IRCS_ECD_J,
     IRCS_ECD_SEP1,
     IRCS_ECD_HM,
     IRCS_ECD_HP,
     IRCS_ECD_SEP2,
     IRCS_ECD_KM,
     IRCS_ECD_KP,
     IRCS_ECD_SEP3,
     IRCS_ECD_LAP,
     IRCS_ECD_LAZ,
     IRCS_ECD_LAM,
     IRCS_ECD_LBP,
     IRCS_ECD_LBZ,
     IRCS_ECD_LBM,
     IRCS_ECD_SEP4,
     IRCS_ECD_MMM,
     IRCS_ECD_MM,
     IRCS_ECD_MP,
     IRCS_ECD_MPP,
     IRCS_ECD_SEP5,
     IRCS_ECD_FEH,
     IRCS_ECD_FEJ,
     IRCS_ECD_HE1085,
     IRCS_ECD_SEP6,
     IRCS_ECD_HFE,
     IRCS_ECD_KH2,
     IRCS_ECD_LCH,
     IRCS_ECD_LH3,
     IRCS_ECD_MCO,
     IRCS_ECD_LC2H6,
     IRCS_ECD_LCH4,
     IRCS_ECD_LH2O,
     NUM_IRCS_ECD};


typedef struct _IRCS_ECD_Entry IRCS_ECD_Entry;
struct _IRCS_ECD_Entry{
  gchar *name;
  gchar *def;
  gdouble r1;
  gdouble r2;
  guint slw_n;
  guint slw_m;
  guint slw_w;
  guint spw;
  gint ech;
  gint xds;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;
};

// 55mas FC=out
static const IRCS_ECD_Entry IRCS_ecd_set[] = {
  //Name                Def   r1     r2  slwn slwm slww  spw    ech    xds
  {"Iz",                "IZ", 20000, 20000,    4,   7,  10,   2,  2600, -1000, 300, 0.41, 900}, //IRCS_ECD_IZ,   
  {"zJ",                "ZJ", 19000, 22600,    4,   7,  10,   3,  2240,  2000, 300, 0.41, 900}, //IRCS_ECD_ZJ,   
  {"J",                  "J", 19000, 22600,    4,   7,  10,   4,  2240,     0, 300, 0.41, 900}, //IRCS_ECD_J,	   
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H-",                "HM", 18800, 22900,    3,   6,   9,   5,  3610,  -500, 100, 0.41, 200}, //IRCS_ECD_HM,   
  {"H+",                "HP", 18800, 22900,    3,   6,   9,   5,  1490,  -180, 100, 0.41, 200}, //IRCS_ECD_HP,   
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"K-",                "KM", 18300, 22300,    3,   6,   9,   6,  5360,  -470, 100, 0.41, 300}, //IRCS_ECD_KM,   
  {"K+",                "KP", 18300, 22300,    3,   6,   9,   6,  -270,   200, 100, 0.41, 300}, //IRCS_ECD_KP,   
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"LA+",              "LAP", 17300, 25100,    2,   5,   9,   7, -4500,  1300,  10, 0.41,  30}, //IRCS_ECD_LAP,  
  {"LA0",              "LAZ", 17300, 25100,    2,   5,   9,   7,  2500,   500,  10, 0.41,  30}, //IRCS_ECD_LAZ,  
  {"LA-",              "LAM", 17300, 25100,    2,   5,   9,   7, 10500,  -200,  10, 0.41,  30}, //IRCS_ECD_LAM,  
  {"LB+",              "LBP", 17300, 25100,    2,   5,   9,   7, -4500,  4300,  10, 0.41,  30}, //IRCS_ECD_LBP,  
  {"LB0",              "LBZ", 17300, 25100,    2,   5,   9,   7,  2500,  3600,  10, 0.41,  30}, //IRCS_ECD_LBZ,  
  {"LB-",              "LBM", 17300, 25100,    2,   5,   9,   7, 10500,  2700,  10, 0.41,  30}, //IRCS_ECD_LBM,  
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"M--",              "MMM", 16800, 26300,    2,   5,   9,   8, 12000, -5500,   2, 0.41,   4}, //IRCS_ECD_MMM,  
  {"M-",                "MM", 16800, 26300,    2,   5,   9,   8,  4000, -5500,   2, 0.41,   4}, //IRCS_ECD_MM,   
  {"M+",                "MP", 16800, 26300,    2,   5,   9,   8, -4000, -5500,   2, 0.41,   4}, //IRCS_ECD_MP,   
  {"M++",              "MPP", 16800, 26300,    2,   5,   9,   8,-11000, -5500,   2, 0.41,   4}, //IRCS_ECD_MPP,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"FeII 1650:60nm",   "FEH", 18800, 22900,    2,   5,   0,  11,  5000,  -300, 100, 0.41, 200}, //IRCS_ECD_FEH,  
  {"FeII 1252:25nm",   "FEJ", 19000, 22600,    2,   5,   0,  10,  2240,     0, 300, 0.41, 900}, //IRCS_ECD_FEJ,  
  {"He 1085:20nm",  "HE1085", 19000, 22600,    2,   5,   0,   9,  2240,     0, 300, 0.41, 900}, //IRCS_ECD_HE1085,
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H(FeII)",          "HFE", 18800, 22900,    3,   6,   9,   5,  3610,  -300, 100, 0.41, 200}, //IRCS_ECD_HFE,  
  {"K(H2)",            "KH2", 18300, 22300,    3,   6,   9,   6,  6450,   500, 100, 0.41, 300}, //IRCS_ECD_KH2,  
  {"L(CH4) original",  "LCH", 18300, 22300,    2,   5,   9,   7,  1200,   670,  10, 0.41,  30}, //IRCS_ECD_LCH,  
  {"L(H3+)",           "LH3", 18300, 22300,    2,   5,   9,   7,  8350,  2912,  10, 0.41,  30}, //IRCS_ECD_LH3,  
  {"M(CO)",            "MCO", 16800, 26300,    2,   5,   9,   8,  8200, -5500,   2, 0.41,   4}, //IRCS_ECD_MCO,  
  {"L(C2H6)",        "LC2H6", 18300, 22300,    2,   5,   9,   7,  3350,  -250,  10, 0.41,  30}, //IRCS_ECD_LC2H6,
  {"L(CH4)",          "LCH4", 18300, 22300,    2,   5,   9,   7,  7350,  -200,  10, 0.41,  30}, //IRCS_ECD_LCH4,
  {"L(H2O Ice)",      "LH2O", 18300, 22300,    2,   5,   9,   7,  8150,  1200,  10, 0.41,  30}  //IRCS_ECD_LH2O, 
};


typedef struct _IRCS_ECM_Entry IRCS_ECM_Entry;
struct _IRCS_ECM_Entry{
  gchar *name;
  guint slw_n;
  guint slw_m;
  guint slw_w;
  guint spw;
  gdouble w1;
  gdouble w2;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;
};

enum{//IRCS_ECM_OPEN,
     IRCS_ECM_IZ,
     IRCS_ECM_ZJ,
     IRCS_ECM_J,
     IRCS_ECM_H,
     IRCS_ECM_K,
     IRCS_ECM_L,
     IRCS_ECM_M,
     IRCS_ECM_SEP1,
     IRCS_ECM_HE1085,
     IRCS_ECM_FE1252,
     IRCS_ECM_FE1650,
     NUM_IRCS_ECM};

static const IRCS_ECM_Entry IRCS_ecm_set[] = {
  //Name             
  //{"OPEN",           1,     0,    0, 100, 0.41, 900},  //IRCS_ECSPW_OPEN,  
  {"Iz",            4,  7, 10,  2,  0.91, 1.00, 300, 0.41, 900},  //IRCS_ECSPW_IZ,    
  {"zJ",            4,  7, 10,  3,  1.03, 1.17, 300, 0.41, 900},  //IRCS_ECSPW_ZJ,    
  {"J",             4,  7, 10,  4,  1.17, 1.37, 300, 0.41, 900},  //IRCS_ECSPW_J,     
  {"H",             3,  6,  9,  5,  1.47, 1.80, 100, 0.41, 200},  //IRCS_ECSPW_H,     
  {"K",             3,  6,  9,  6,  1.92, 2.52, 100, 0.41, 300},  //IRCS_ECSPW_K,     
  {"L",             2,  5,  9,  7,  2.90, 4.25,  10, 0.41,  30},  //IRCS_ECSPW_L,     
  {"M",             2,  5,  9,  8,  4.40, 6.00,   2, 0.41,   4},  //IRCS_ECSPW_M,
  {NULL,0,0,0,0,0,0,0,0,0},
  {"He 1085",       2,  5,  9,  9, 1.073,1.093, 300, 0.41, 900},  //IRCS_ECSPW_HE1085,
  {"Fe II 1252",    2,  5,  9, 10, 1.239,1.265, 300, 0.41, 900},  //IRCS_ECSPW_FE1252,
  {"Fe II 1650",    2,  5,  9, 11, 1.616,1.676, 100, 0.41, 200}   //IRCS_ECSPW_FE1650,
};





// Dithering
enum{IRCS_EC_DITH_ABBA,
     IRCS_EC_DITH_OS,
     IRCS_EC_DITH_SS,
     IRCS_EC_DITH_NO,
     NUM_IRCS_EC_DITH};


static const IRCS_DITH_Entry IRCS_ec_dith[]={
  {"A-B-B-A",         "D2",    4, 0},
  {"Obj/Sky",         "D2XS1", 2, 0},
  {"Slit Scan",       "SS",    0, 0},
  {"None",            "NO",    1, 0}
};



// Slit
enum{IRCS_EC_SLIT_N,
     IRCS_EC_SLIT_M,
     IRCS_EC_SLIT_W,
     NUM_IRCS_EC_SLIT};

typedef struct _IRCS_EC_SL_Entry IRCS_EC_SL_Entry;
struct _IRCS_EC_SL_Entry{
  gchar *def;
  gdouble width;
};


static const IRCS_EC_SL_Entry IRCS_ec_slit[] = {
  {"N", 0.14},
  {"M", 0.27},
  {"W", 0.54}
};

typedef struct _IRCS_EC_SLW_Entry IRCS_EC_SLW_Entry;
struct _IRCS_EC_SLW_Entry{
  gchar *name;
  gchar *def;
  gint slw;
  gdouble width;
  gdouble length;
  gdouble slit_x;
  gdouble slit_y;
};

static const IRCS_EC_SLW_Entry IRCS_ec_slw[] = {
  {NULL,   NULL,  0, 0, 0, 0, 0},
  {NULL,   NULL,  0, 0, 0, 0, 0},
  {"0\".14x6\".69 (L --)",     "N",   2,  0.14,   6.69, 512.0, 511.0},
  {"0\".14x5\".17 (H -- K)",   "N",   3,  0.14,   5.17, 508.5, 514.5},
  {"0\".14x3\".47 (-- J)",     "N",   4,  0.14,   3.47, 509.8, 511.0},
  {"0\".27x9\".37 (L --)",     "M",   5,  0.27,   9.37, 513.0, 511.0},
  {"0\".27x5\".17 (H -- K)",   "M",   6,  0.27,   5.17, 509.0, 511.2},
  {"0\".27x3\".47 (-- J)",     "M",   7,  0.27,   3.47, 510.0, 511.5},
  {NULL,   NULL,  0, 0, 0, 0, 0},
  {"0\".54x5\".17 (H -- K)",   "W",   9,  0.54,   5.17, 511.0, 511.5},
  {"0\".54x3\".47 (-- J)",     "W",  10,  0.54,   3.47, 513.5, 510.2},
};

typedef struct _IRCSpara IRCSpara;
struct _IRCSpara{
  guint mode;
  guint mas;
  guint band;
  guint slit;
  guint dith;

  gdouble dithw;
  guint osmode;
  gint osra;
  gint osdec;
  gdouble sssep;
  gint ssnum;

  gint ech;
  gint xds;

  guint cw1;
  guint cw2;
  guint cw3;
  guint slw;
  guint spw;
  gint cam;

  gdouble slit_x;
  gdouble slit_y;
  
  gchar *txt;
  gchar *def;

  gboolean std;

  gdouble exp;
};


// IRCSTreeview
enum
{
  COLUMN_IRCS_NUMBER,
  COLUMN_IRCS_NAME,
  COLUMN_IRCS_DEF,
  COLUMN_IRCS_EXP,
  COLUMN_IRCS_COLBG,
  NUM_COLUMN_IRCS
};

#ifdef USE_GTK3
static GdkRGBA col_ircs_setup [IRCS_MAX_SET]
= {
  {0.90, 0.90, 1.00, 1}, //pale2
  {1.00, 1.00, 0.90, 1}, //orange2
  {1.00, 0.90, 1.00, 1}, //purple2
  {0.90, 1.00, 0.90, 1}, //green2
  {1.00, 0.90, 0.90, 1}, //pink2
  {0.90, 0.90, 1.00, 1}, //pale2
  {1.00, 1.00, 0.90, 1}, //orange2
  {1.00, 0.90, 1.00, 1}, //purple2
  {0.90, 1.00, 0.90, 1}, //green2
  {1.00, 0.90, 0.90, 1}, //pink2
  {0.90, 0.90, 1.00, 1}, //pale2
  {1.00, 1.00, 0.90, 1}, //orange2
  {1.00, 0.90, 1.00, 1}, //purple2
  {0.90, 1.00, 0.90, 1}, //green2
  {1.00, 0.90, 0.90, 1}, //pink2
  {0.90, 0.90, 1.00, 1}, //pale2
  {1.00, 1.00, 0.90, 1}, //orange2
  {1.00, 0.90, 1.00, 1}, //purple2
  {0.90, 1.00, 0.90, 1}, //green2
  {1.00, 0.90, 0.90, 1}  //pink2
};
#else
static GdkColor col_ircs_setup [IRCS_MAX_SET]
= {
  {0, 0xDDDD, 0xDDDD, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xDDDD}, //orange2
  {0, 0xFFFF, 0xDDDD, 0xFFFF}, //purple2
  {0, 0xDDDD, 0xFFFF, 0xDDDD}, //green2
  {0, 0xFFFF, 0xDDDD, 0xDDDD}, //pink2
  {0, 0xDDDD, 0xDDDD, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xDDDD}, //orange2
  {0, 0xFFFF, 0xDDDD, 0xFFFF}, //purple2
  {0, 0xDDDD, 0xFFFF, 0xDDDD}, //green2
  {0, 0xFFFF, 0xDDDD, 0xDDDD}, //pink2
  {0, 0xDDDD, 0xDDDD, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xDDDD}, //orange2
  {0, 0xFFFF, 0xDDDD, 0xFFFF}, //purple2
  {0, 0xDDDD, 0xFFFF, 0xDDDD}, //green2
  {0, 0xFFFF, 0xDDDD, 0xDDDD}, //pink2
  {0, 0xDDDD, 0xDDDD, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xDDDD}, //orange2
  {0, 0xFFFF, 0xDDDD, 0xFFFF}, //purple2
  {0, 0xDDDD, 0xFFFF, 0xDDDD}, //green2
  {0, 0xFFFF, 0xDDDD, 0xDDDD}  //pink2
};
#endif


// MagDB for Guide Star Selection
#define IRCS_MAGDB_MAG_TTGS 18
#define IRCS_MAGDB_MAG_NGS 16.5
#define IRCS_MAGDB_R_TGT_ARCSEC 3
#define IRCS_MAGDB_R_NGS_ARCSEC 30
#define IRCS_MAGDB_R_TTGS_ARCSEC 60


void IRCS_TAB_create();
void IRCS_param_init();

void IRCS_get_mode();
void IRCS_add_setup();
void IRCS_remove_setup();

void cc_get_switch_page();

void IRCS_get_im_mas();
void IRCS_get_im52_band();
void IRCS_get_im20_band();
void IRCS_set_im_label();
void IRCS_add_im();

void IRCS_get_pi_mas();
void IRCS_get_pi52_band();
void IRCS_get_pi20_band();
void IRCS_set_pi_label();
void IRCS_add_pi();

void IRCS_get_gr_mas();
void IRCS_get_gr52_band();
void IRCS_get_gr20_band();
void IRCS_get_gr52_slit();
void IRCS_get_gr20_slit();
void IRCS_set_gr_label();
void IRCS_add_gr();

void IRCS_get_ps_mas();
void IRCS_get_ps52_band();
void IRCS_get_ps20_band();
void IRCS_get_ps52_slit();
void IRCS_get_ps20_slit();
void IRCS_set_ps_label();
void IRCS_add_ps();

void IRCS_get_ec_mas();
void IRCS_get_ecd_band();
void IRCS_get_ecm_band();
void IRCS_get_ecd_slit();
void IRCS_get_ecm_slit();
void IRCS_set_ec_label();
void IRCS_add_ec();

void ircs_set_def_exp();

GtkTreeModel* ircs_create_items_model();
void ircs_tree_update_item();
void ircs_add_columns();
void ircs_focus_item();
void ircs_make_tree();
void ircs_update_tree();
void ircs_cell_data_func();
void ircs_double_cell_data_func();

void ircs_swap_setup();

void IRCS_WriteOPE();
void IRCS_WriteOPE_obj();

gint IRCS_check_gs();

gchar* get_gs_txt();

void IRCS_WriteLGS();
