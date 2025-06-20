// Header for Subaru IRCS
//             Dec 2018  A. Tajitsu (Subaru Telescope, NAOJ)

#define IRCS_MAX_SET 20

#define IRCS_DEF_PA 90
#define IRCS_DEF_NDR 16
#define IRCS_DEF_COADDS 1
#define IRCS_FLAT_REPEAT 12
#define IRCS_COMP_REPEAT 6

#define IRCS_MINIMUM_EXP 0.006

#define IRCS_SET_HOST "www.naoj.org"
#define IRCS_SET_PATH "/Instruments/IRCS/ircs_set.ini"
#define IRCS_SET_FILE "ircs_set.ini"

static const gint IRCS_mas[]={
  52,
  20
};

// Mode
enum{IRCS_MODE_IM,
     IRCS_MODE_PI,
     IRCS_MODE_GR,
     IRCS_MODE_PS,
     IRCS_MODE_EC,
     NUM_IRCS_MODE};

static const gchar* ircs_mode_name[]={
  "Imaging",
  "Pol-Imaging",
  "Grism",
  "Pol-Grism",
  "Echelle"
};

static const gchar* ircs_mode_initial[]={
  "IM",
  "PI",
  "GR",
  "PS",
  "EC"
};

//// Imaging ////
// Scale
enum{IRCS_MAS_52,
     IRCS_MAS_20,
     NUM_IRCS_MAS};

// Dithering
enum{IRCS_DITH_S5,
     IRCS_DITH_S9,
     IRCS_DITH_D5,
     IRCS_DITH_D9,
     IRCS_DITH_V3,
     IRCS_DITH_V5,
     IRCS_DITH_ABBA,
     IRCS_DITH_OSO,
     IRCS_DITH_OSSO,
     IRCS_DITH_SS,
     IRCS_DITH_NO,
     NUM_IRCS_DITH};

typedef struct _IRCS_DITH_Entry IRCS_DITH_Entry;
struct _IRCS_DITH_Entry{
  gchar *name;
  gchar *def;
  gint  shot;

  gboolean f_im;
  gboolean f_pi;
  gboolean f_gr;
  gboolean f_ps;
  gboolean f_ec;

  gboolean n_im;
  gboolean n_pi;
  gboolean n_gr;
  gboolean n_ps;
  gboolean n_ec;
};

static const IRCS_DITH_Entry IRCS_dith[]={
  // name              def    shot    IM     PI     GR     PS     EC   IMxN   PIxN   GRxN   PSxN   ECxN
  {"Standard 5",       "S5",    5,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE},
  {"Standard 9",       "S9",    9,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE},
  {"Diamond 5",        "D5",    5,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE},
  {"Diamond 9",        "D9",    9,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE},
  {"Vertical 3",       "V3",    3, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE},
  {"Vertical 5",       "V5",    5, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
  {"A-B-B-A",          "D2",    4, FALSE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE,  TRUE,  TRUE},
  {"Obj-Sky-Obj",      "D2XS1", 3,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE},
  {"Obj-Sky-Sky-Obj",  "D2XS2", 4,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE},
  {"Slit Scan",        "SS",    0, FALSE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE, FALSE},
  {"No Dither",        "ST",    1,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE}
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
     IRCS_IM52_NB1984,
     IRCS_IM52_NB2070,
     IRCS_IM52_NB2090,
     IRCS_IM52_H210,
     IRCS_IM52_BRG,
     IRCS_IM52_KC,
     IRCS_IM52_H2O,
     //     IRCS_IM52_PAH,
     IRCS_IM52_NB1189,
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
     //IRCS_IM52_PAHN,
     //IRCS_IM52_NB1189N,
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
     IRCS_IM20_NB1984,
     IRCS_IM20_NB2070,
     IRCS_IM20_NB2090,
     IRCS_IM20_H210,
     IRCS_IM20_BRG,
     IRCS_IM20_KC,
     IRCS_IM20_H2O,
     //     IRCS_IM20_PAH,
     IRCS_IM20_NB1189,
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
     //IRCS_IM20_PAHN,
     //IRCS_IM20_NB1189N,
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
  gchar *markup;
  gchar *def;
  gchar *sdef;
  gdouble wv1;
  gdouble wv2;
  guint cw1;
  guint cw2;
  guint cw3;
  guint slw;
  gint cam;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;

  gdouble flat_amp;  // Negative --> No setup in CAL_Temp2010.ope
  gdouble flat_exp;
  gint    flat_nd;   // 0 --> should skip flats (ND)  : Voltage for dome flats
};


static IRCS_IM_Entry IRCS_im52_set[] = {
  //Name                                                         Def       sdef           wv1    wv2 cw1 cw2 cw3 slw  cam                        Dome Flat (52mas)
  {"Y",                    "Y",                                  "Y",      "IM52_Y",      0.969, 1.071, 13, 14,  1,  1, 1000,   300,    13, 1000,  5.1, 0.5,  80}, //IRCS_IM52_Y
  {"J",                    "J",                                  "J",      "IM52_J",      1.170, 1.330,  1,  3,  1,  1, 1000,   300,    13, 1000,  5.1, 1.0,  60}, //IRCS_IM52_J  
  {"H",                    "H",                                  "H",      "IM52_H",      1.490, 1.780,  1,  5,  1,  1,  750,   200,     4,  200,  5.1, 1.0,  40}, //IRCS_IM52_H,     
  {"K",                    "K",                                  "K",      "IM52_K",      2.030, 2.370,  1,  7,  1,  1, 1000,   300,     5,  330,  5.1, 1.5, 120}, //IRCS_IM52_K,
  {"K-prime",              "K-prime",                            "KP",     "IM52_KP",     1.950, 2.290,  1,  9,  1,  1, 1000,   300,     5,  420,  5.1, 1.4, 120}, //IRCS_IM52_Kp,    
  {"H + K-prime",          "H + K-prime",                        "HK",     "IM52_HK",     1.490, 2.290,  1, 14,  1,  1,  875,   200,     4,  200, -5.1, 1.0, 120}, //IRCS_IM52_HK,     
  {"L-prime",              "L-prime",                            "LP",     "IM52_LP",     3.420, 4.120,  1, 11,  1,  1, 1500,  0.20, 0.006, 0.20, -5.1,0.01, 120}, //IRCS_IM52_Lp,    
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		             
  {"CH4 short",            "CH<sub>4</sub> short",               "CH4S",   "IM52_CH4S",   1.520, 1.620, 13,  1,  9,  1,  750,   300,     5, 1000,  5.1, 1.2,  40}, //IRCS_IM52_CH4S, 
  {"H cont",               "H cont",                             "HC",     "IM52_HC",     1.540, 1.558,  1,  1,  3,  1,  750,   300,     5, 1000,  5.1, 1.5, 120}, //IRCS_IM52_HC,
  {"CH4 long",             "CH<sub>4</sub> long",                "CH4L",   "IM52_CH4L",   1.640, 1.740, 13,  1, 10,  1,  750,   300,     5, 1000,  5.1, 1.0,  80}, //IRCS_IM52_CH4L,  
  {"[Fe II]",              "[Fe II]",                            "FE",     "IM52_FE",     1.632, 1.656, 13,  1,  4,  1,  750,   300,     5, 1000,  5.1, 2.0, 120}, //IRCS_IM52_FE,    
  {"NB1984",               "NB &#x3BB;1984",                     "NB1984", "IM52_NB1984", 1.980, 1.990,  1,  9, 12,  1, 1000,   300,     5, 1000, -5.1,  10, 120}, //IRCS_IM52_NB1984,
  {"NB2070",               "NB &#x3BB;2070",                     "NB2070", "IM52_NB2070", 2.055, 2.085,  1,  9,  6,  1, 1000,   300,     5, 1000, -5.1,  10, 120}, //IRCS_IM52_NB2070,
  {"NB2090",               "NB &#x3BB;2090",                     "NB2090", "IM52_NB2090", 2.073, 2.108,  1,  1,  5,  1, 1000,   300,     5, 1000,  5.1,  10, 120}, //IRCS_IM52_NB2090,
  {"H2 1-0S(1)",           "H<sub>2</sub> 1-0S(1)",              "H210",   "IM52_H210",   2.106, 2.138, 11,  9,  1,  1, 1000,   300,     5, 1000,  5.1,  10, 120}, //IRCS_IM52_H210,
  {"Br gamma",             "Br &#x3B3;",                         "BRG",    "IM52_BRG",    2.150, 2.182, 10,  1,  1,  1, 1000,   300,     5, 1000,  5.1,12.5, 120}, //IRCS_IM52_BRG,
  {"K cont",               "K cont",                             "KC",     "IM52_KC",     2.299, 2.329,  1,  7,  8,  1, 1000,   300,     5, 1000,  5.1,  23, 120}, //IRCS_IM52_KC,    
  {"H2O Ice",              "H<sub>2</sub>O Ice",                 "H2O",    "IM52_H2O",    2.974, 3.126,  4,  1,  1,  1, 1500,   300, 0.006, 1000,  5.1, 0.5, 100}, //IRCS_IM52_H2O,   
  //  {"PAH",                  "PAH",                                "PAH",    "IM52_PAH",    3.270, 3.320,  1,  1,  7,  1, 1500,  0.20, 0.006, 1000,  5.1, 0.5, 100}, //IRCS_IM52_PAH,   
  {"NB1189",               "NB &#x3BB;1189",                     "NB1189", "IM52_NB1189", 1.175, 1.203,  1,  3,  7,  1, 1000,   300,     5, 1000, -5.1,  10, 120}, //IRCS_IM52_NB1189,   
  {"H3+",                  "H<sub>3</sub><sup>+</sup>",          "H3P",    "IM52_H3P",    3.402, 3.424,  1,  1, 11,  1, 1700,  0.20, 0.006, 1000, -5.1,  10, 120}, //IRCS_IM52_H3P and BRAC (w/ND),   
  {"Br alpha",             "Br &#x3B1;",                         "BRA",    "IM52_BRA",    4.032, 4.082,  1,  1, 12,  1, 1700,  0.20, 0.006, 1000, -5.1,  10, 120}, //IRCS_IM52_BRA,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		             
  {"Y + ND(1%)",           "Y + ND(1%)",                         "YN",     "IM52_YN",     0.969, 1.071, 13, 14,  2,  1, 1000,   300,    13, 1000,    0,   0, 0}, //IRCS_IM52_YN
  {"J + ND(1%)",           "J + ND(1%)",                         "JN",     "IM52_JN",     1.170, 1.330,  1,  3,  2,  1, 1000,   300,    13, 1000,    0,   0, 0}, //IRCS_IM52_JN  
  {"H + ND(1%)",           "H + ND(1%)",                         "HN",     "IM52_HN",     1.490, 1.780,  1,  5,  2,  1,  750,   200,     4,  200,    0,   0, 0}, //IRCS_IM52_HN,     
  {"K + ND(1%)",           "K + ND(1%)",                         "KN",     "IM52_KN",     2.030, 2.370,  1,  7,  2,  1, 1000,   300,     5,  330,    0,   0, 0}, //IRCS_IM52_KN,
  {"K + ND(0.01%)",        "K + ND(0.01%)",                      "KNN",    "IM52_KNN",    2.030, 2.370, 12,  7,  2,  1, 1000,   300,     5,  330,    0,   0, 0}, //IRCS_IM52_KNN,
  {"K-prime + ND(1%)",     "K-prime + ND(1%)",                   "KPN",    "IM52_KPN",    1.950, 2.290,  1,  9,  2,  1, 1000,   300,     5,  420,    0,   0, 0}, //IRCS_IM52_KPN,    
  {"H + K-prime + ND(1%)", "H + K-prime + ND(1%)",               "HKN",    "IM52_HKN",    1.490, 2.290,  1, 14,  2,  1,  875,   200,     4,  200,    0,   0, 0}, //IRCS_IM52_HKN,     
  {"L-prime + ND(1%)",     "L-prime + ND(1%)",                   "LPN",    "IM52_LPN",    3.420, 4.120,  1, 11,  2,  1, 1500,  0.20, 0.006, 0.20,    0,   0, 0}, //IRCS_IM52_LPN,    
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		             
  {"NB2070 + ND(1%)",      "NB &#x3BB;2070 + ND(1%)",            "NB2070N","IM52_NB2070N",2.055, 2.085, 12,  9,  6,  1, 1000,   300,     5,  1000,   0,   0, 0}, //IRCS_IM52_NB2070N,
  {"H2 1-0S(1) + ND(1%)",  "H<sub>2</sub> 1-0S(1) + ND(1%)",     "H210N",  "IM52_H210N",  2.106, 2.138, 11,  9,  2,  1, 1000,   300,     5,  1000,   0,   0, 0}, //IRCS_IM52_H210N,
  {"Br gamma + ND(1%)",    "Br &#x3B3; + ND(1%)",                "BRGN",   "IM52_BRGN",   2.150, 2.182, 10,  1,  2,  1, 1000,   300,     5,  1000,   0,   0, 0}, //IRCS_IM52_BRGN,
  {"H2O Ice + ND(1%)",     "H<sub>2</sub>O Ice + ND(1%)",        "H2ON",   "IM52_H2ON",   2.974, 3.126,  4,  1,  2,  1, 1500,   300, 0.006,  1000,   0,   0, 0}, //IRCS_IM52_H2ON,   
  //  {"PAH + ND(1%)",         "PAH + ND(1%)",                       "PAHN",   "IM52_PAHN",   3.270, 3.320, 12,  1,  7,  1, 1500,  0.20, 0.006,  1000,   0,   0, 0}, //IRCS_IM52_PAHN,   
  //{"NB1189 + ND(1%)",      "NB &#x3BB;1189 + ND(1%)",            "NB1189N", "IM52_NB1189N",   1.180, 1.200, 12,  3,  7,  1, 1000,  300, 5,  1000,   0,   0, 0}, //IRCS_IM52_NB1189N,   
  {"H3+ + ND(1%)",         "H<sub>3</sub><sup>+</sup> + ND(1%)", "BRACN",  "IM52_BRACN",  3.402, 3.424, 12,  1, 11,  0, 1700,  0.20, 0.006,  1000,   0,   0, 0}, //IRCS_IM52_BRACN (H3P w/ND),   
  {"Br alpha + ND(1%)",    "Br &#x3B1; + ND(1%)",                "BRAN",   "IM52_BRAN",   4.032, 4.082, 12,  1, 12,  1, 1700,  0.20, 0.006,  1000,   0,   0, 0}  //IRCS_IM52_BRAN,    
};

static IRCS_IM_Entry IRCS_pi52_set[] = {
  //Name                                        Def         wv1    wv2 cw1 cw2 cw3 slw   cam    
  {"Y",                "Y",                     "Y",      "PI52_Y",      0.969, 1.071, 13, 14, 13, 11, 1000,   300,    13,  1000, -5.1, 0.5,  80}, //IRCS_PI52_Y
  {"J",                "J",                     "J",      "PI52_J",      1.170, 1.330,  1,  3, 13, 11, 1000,   300,    13,  1000, -5.1, 0.5,  80}, //IRCS_PI52_J  
  {"H",                "H",                     "H",      "PI52_H",      1.490, 1.780,  1,  5, 13, 11,  750,   200,     4,   200, -5.1, 0.5,  60}, //IRCS_PI52_H,     
  {"K",                "K",                     "K",      "PI52_K",      2.030, 2.370,  1,  7, 13, 11, 1000,   300,     5,   330, -5.1, 1.5, 120}, //IRCS_PI52_K,
  {"K-prime",          "K-prime",               "KP",     "PI52_KP",     1.950, 2.290,  1,  9, 13, 11, 1000,   300,     5,   420, -5.1, 1.4, 120}, //IRCS_PI52_Kp,    
  {"H + K-prime",      "H + K-prime",           "HK",     "PI52_HK",     1.490, 2.290,  1, 14, 13, 11,  875,   200,     4,   200, -5.1, 1.0, 120}, //IRCS_PI52_HK,     
  {"L-prime",          "L-prime",               "LP",     "PI52_LP",     3.420, 4.120,  1, 11, 13, 11, 1500,  0.20, 0.006,  0.20, -5.1,0.01, 120}, //IRCS_PI52_Lp,    
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H2 1-0S(1)",       "H<sub>2</sub> 1-0S(1)", "H210",   "PI52_H210",   2.106, 2.138, 11,  9, 13, 11, 1000,   300,     5,  1000, -5.1,  10, 120}, //IRCS_PI52_H210,
  {"Br gamma",         "Br &#x3B3;",            "BRG",    "PI52_BRG",    2.150, 2.182, 10,  1, 13, 11, 1000,   300,     5,  1000, -5.1,  10, 120}, //IRCS_PI52_BRG,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"J + ND(1%)",       "J + ND(1%)",            "JN",     "PI52_JN",     1.170, 1.330, 12,  3, 13, 11, 1000,   300,    13,  1000,    0,   0, 0}, //IRCS_PI52_JN  
  {"H + ND(1%)",       "H + ND(1%)",            "HN",     "PI52_HN",     1.490, 1.780, 12,  5, 13, 11,  750,   200,     4,   200,    0,   0, 0}, //IRCS_PI52_HN,     
  {"K + ND(1%)",       "K + ND(1%)",            "KN",     "PI52_KN",     2.030, 2.370, 12,  7, 13, 11, 1000,   300,     5,   330,    0,   0, 0}, //IRCS_PI52_KN,
  {"K-prime + ND(1%)", "K-prime + ND(1%)",      "KPN",    "PI52_KPN",    1.950, 2.290, 12,  9, 13, 11, 1000,   300,     5,   420,    0,   0, 0}, //IRCS_PI52_KPN,    
  {"HK + ND(1%)",      "HK + ND(1%)",           "HKN",    "PI52_HKN",    1.490, 2.370, 12, 14, 13, 11,  875,   200,     4,   200,    0,   0, 0}, //IRCS_PI52_HKN,     
  {"L-prime + ND(1%)", "L-prime + ND(1%)",      "LPN",    "PI52_LPN",    3.420, 4.120, 12, 11, 13, 11, 1500,  0.20, 0.006,  0.20,    0,   0, 0} //IRCS_PI52_LPN,    
};

static IRCS_IM_Entry IRCS_im20_set[] = {
  //Name                                                        Def            wv1    wv2 cw1 cw2 cw3 slw  cam   
  {"Y",                   "Y",                                  "20Y",       "IM20_Y",       0.969, 1.071, 13, 14,  1,  1, 2500,   300,    80,  6000,  1.25, 0.5, 1}, //IRCS_IM20_Y
  {"J",                   "J",                                  "20J",       "IM20_J",       1.170, 1.330,  1,  3,  1,  1, 2500,   300,    80,  6000,   1.6, 0.5, 3}, //IRCS_IM20_J  
  {"H",                   "H",                                  "20H",       "IM20_H",       1.490, 1.780,  1,  5,  1,  1, 2500,   300,    12,  1300,   1.2, 0.5, 3}, //IRCS_IM20_H,     
  {"K",                   "K",                                  "20K",       "IM20_K",       2.030, 2.370,  1,  7,  1,  1, 2500,   300,    20,  2100,  1.21, 0.4, 3}, //IRCS_IM20_K,
  {"K-prime",             "K-prime",                            "20KP",      "IM20_KP",      1.950, 2.290,  1,  9,  1,  1, 2500,   300,    30,  2800, 1.212, 0.5, 3}, //IRCS_IM20_Kp,    
  {"H + K-prime",         "H + K-prime",                        "20HK",      "IM20_HK",      1.490, 2.370,  1, 14,  1,  1, 2500,   300,    12,  1300,   1.0, 0.5, 1}, //IRCS_IM20_HK,    
  {"L-prime",             "L-prime",                            "20LP",      "IM20_LP",      3.420, 4.120,  1, 11,  1,  1, 2500,  0.20, 0.006,  0.62,  -1.0, 0.1, 1}, //IRCS_IM20_Lp,    
  {"M-prime",             "M-prime",                            "MP",        "IM20_MP",      4.570, 4.790,  1, 13,  1,  1, 2500,  0.20, 0.006,  0.28,  -1.0, 0.1, 1}, //IRCS_IM20_Mp,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		                  
  {"CH4 short",           "CH<sub>4</sub> short",               "20CH4S",    "IM20_CH4S",    1.520, 1.620, 13,  1,  9,  1, 2500,   300,     5,  6000,   1.0, 0.5, 1}, //IRCS_IM20_CH4S,  
  {"H cont",              "H cont",                             "20HC",      "IM20_HC",      1.540, 1.558,  1,  1,  3,  1, 2500,   300,     5,  6000,  1.14, 0.5, 1}, //IRCS_IM20_HC,
  {"CH4 long",            "CH<sub>4</sub> long",                "20CH4L",    "IM20_CH4L",    1.640, 1.740, 13,  1, 10,  1, 2500,   300,     5,  6000,  0.97, 0.7, 1}, //IRCS_IM20_CH4L,  
  {"[Fe II]",             "[Fe II]",                            "20FE",      "IM20_FE",      1.632, 1.656, 13,  1,  4,  1, 2500,   300,     5,  6000,   1.1, 0.7, 1}, //IRCS_IM20_FE,    
  {"NB1984",              "NB &#x3BB;1984",                     "20NB1984",  "IM20_NB1984",  1.980, 1.990,  1,  9, 12,  1, 2500,   300,     5,  6000,  -1.0, 0.6, 1}, //IRCS_IM20_NB1984,
  {"NB2070",              "NB &#x3BB;2070",                     "20NB2070",  "IM20_NB2070",  2.055, 2.085,  1,  9,  6,  1, 2500,   300,     5,  6000,  -1.0, 0.6, 1}, //IRCS_IM20_NB2070,
  {"NB2090",              "NB &#x3BB;2090",                     "20NB2090",  "IM20_NB2090",  2.073, 2.108,  1,  1,  5,  1, 2500,   300,     5,  6000,   1.0, 0.6, 1}, //IRCS_IM20_NB2090,
  {"H2 1-0S(1)",          "H<sub>2</sub> 1-0S(1)",              "20H210",    "IM20_H210",    2.106, 2.138, 11,  9,  1,  1, 2500,   300,     5,  6000,  1.05, 0.6, 1}, //IRCS_IM20_H210,
  {"Br gamma",            "Br &#x3B3;",                         "20BRG",     "IM20_BRG",     2.150, 2.182, 10,  1,  1,  1, 2500,   300,     5,  6000,   1.0, 0.5, 1}, //IRCS_IM20_BRG,
  {"K cont",              "K cont",                             "20KC",      "IM20_KC",      2.299, 2.329,  1,  7,  8,  1, 2500,   300,     5,  6000,  1.05, 0.5, 2}, //IRCS_IM20_KC,    
  {"H2O Ice",             "H<sub>2</sub>O Ice",                 "20H2O",     "IM20_H2O",     2.974, 3.126,  4,  1,  1,  1, 2500,  0.20, 0.006,  1000,   0.9, 0.5, 1}, //IRCS_IM20_H2O,   
  //  {"PAH",                 "PAH",                                "20PAH",     "IM20_PAH",     3.270, 3.320,  1,  1,  7,  1, 2500,  0.20, 0.006,  1000,  0.95, 0.7, 1}, //IRCS_IM20_PAH,   
  {"NB1189",              "NB &#x3BB;1189",                     "20NB1189",  "IM20_NB1189",  1.175, 1.203,  1,  3,  7,  1, 2500,   300,     5,  6000,  -1.0, 0.6, 1}, //IRCS_IM20_NB1189,   
  {"H3+",                 "H<sub>3</sub><sup>+</sup>",          "20H3P",     "IM20_H3P",     3.402, 3.424,  1,  1, 11,  1, 2500,  0.20, 0.006,  1000,   0.8, 0.6, 1}, //IRCS_IM20_H3P and BRAC (w/ND),   
  {"Br alpha",            "Br &#x3B1;",                         "20BRA",     "IM20_BRA",     4.032, 4.082,  1,  1, 12,  1, 2500,  0.20, 0.006,  1000,   1.0, 0.5, 1}, //IRCS_IM20_BRA,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		                  
  {"Y + ND(1%)",          "Y + ND(1%)",                         "20YN",      "IM20_YN",      0.969, 1.071, 13, 14,  2,  1, 2500,   300,    80,  6000,     0,   0, 0}, //IRCS_IM20_YN
  {"J + ND(1%)",          "J + ND(1%)",                         "20JN",      "IM20_JN",      1.170, 1.330,  1,  3,  2,  1, 2500,   300,    80,  6000,     0,   0, 0}, //IRCS_IM20_JN  
  {"H + ND(1%)",          "H + ND(1%)",                         "20HN",      "IM20_HN",      1.490, 1.780,  1,  5,  2,  1, 2500,   300,    12,  1300,     0,   0, 0}, //IRCS_IM20_HN     
  {"K + ND(1%)",          "K + ND(1%)",                         "20KN",      "IM20_KN",      2.030, 2.370,  1,  7,  2,  1, 2500,   300,    20,  2100,     0,   0, 0}, //IRCS_IM20_KN
  {"K + ND(0.01%)",       "K + ND(0.01%)",                      "20KNN",     "IM20_KNN",     2.030, 2.370, 12,  7,  2,  1, 2500,   300,    20,  2100,     0,   0, 0}, //IRCS_IM20_KNN
  {"K-prime + ND(1%)",    "K-prime + ND(1%)",                   "20KPN",     "IM20_KPN",     1.950, 2.290,  1,  9,  2,  1, 2500,   300,    30,  2800,     0,   0, 0}, //IRCS_IM20_KPN,    
  {"HK + ND(1%)",         "HK + ND(1%)",                        "20HKN",     "IM20_HKN",     1.490, 2.370,  1, 14,  2,  1, 2500,   300,    12,  1300,     0,   0, 0}, //IRCS_IM20_HN     
  {"L-prime + ND(1%)",    "L-prime + ND(1%)",                   "20LPN",     "IM20_LPN",     3.420, 4.120,  1, 11,  2,  1, 2500,  0.20, 0.006,  0.62,     0,   0, 0}, //IRCS_IM20_LPN,    
  {"M-prime + ND(1%)",    "M-prime + ND(1%)",                   "MPN",       "IM20_MPN",     4.570, 4.790,  1, 13,  2,  1, 2500,  0.20, 0.006,  0.28,     0,   0, 0}, //IRCS_IM20_MPN,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},		                  
  {"NB2070 + ND(1%)",     "NB &#x3BB;2070 + ND(1%)",            "20NB2070N", "IM20_NB2070N", 2.055, 2.085, 12,  9,  6,  1, 2500,   300,    12,  6000,     0,   0, 0}, //IRCS_IM20_NB2070N,
  {"H2 1-0S(1) + ND(1%)", "H<sub>2</sub> 1-0S(1) + ND(1%)",     "20H210N",   "IM20_H210N",   2.106, 2.138, 11,  9,  2,  1, 2500,   300,    12,  6000,     0,   0, 0}, //IRCS_IM20_H210N,
  {"Br gamma + ND(1%)",   "Br &#x3B3; + ND(1%)",                "20BRGN",    "IM20_BRGN",    2.150, 2.182, 10,  1,  2,  1, 2500,   300,    12,  6000,     0,   0, 0}, //IRCS_IM20_BRGN,
  {"H2O Ice + ND(1%)",    "H<sub>2</sub>O Ice + ND(1%)",        "20H2ON",    "IM20_H2ON",    2.974, 3.126,  4,  1,  2,  1, 2500,  0.20, 0.006,  6000,     0,   0, 0}, //IRCS_IM20_H2ON,   
  //  {"PAH + ND(1%)",        "PAH + ND(1%)",                       "20PAHN",    "IM20_PAHN",    3.270, 3.320, 12,  1,  7,  1, 2500,  0.20, 0.006,  6000,     0,   0, 0}, //IRCS_IM20_PAHN,   
  //{"NB1189 + ND(1%)",     "NB &#x3BB;1189 + ND(1%)",            "20NB1189N", "IM20_NB1189N", 1.180, 1.200, 12,  3,  7,  1, 2500,   300,    12,  6000,     0,   0, 0}, //IRCS_IM20_NB1189N,   
  {"H3+ + ND(1%)",        "H<sub>3</sub><sup>+</sup> + ND(1%)", "20BRACN",   "IM20_BRACN",   3.402, 3.424, 12,  1, 11,  1, 2500,  0.20, 0.006,  6000,     0,   0, 0}, //IRCS_IM20_BRACN
  {"Br alpha + ND(1%)",   "Br &#x3B1; + ND(1%)",                "20BRAN",    "IM20_BRAN",    4.032, 4.082, 12,  1, 12,  1, 2500,  0.20, 0.006,  6000,     0,   0, 0}  //IRCS_IM20_BRAN,
};

static IRCS_IM_Entry IRCS_pi20_set[] = {
  //Name                                           Def         wv1    wv2 cw1 cw2 cw3 slw  cam   
  {"Y",                "Y",                        "20Y",  "IM20_Y",   0.969, 1.071, 13, 14, 13, 11, 2500,   300,    80,  6000,  1.25, 1.0, 1}, //IRCS_PI20_Y
  {"J",                "J",                        "20J",  "IM20_J",   1.170, 1.330,  1,  3, 13, 11, 2500,   300,    80,  6000,   1.6, 1.0, 3}, //IRCS_PI20_J  
  {"H",                "H",                        "20H",  "IM20_H",   1.490, 1.780,  1,  5, 13, 11, 2500,   300,    12,  1300,  1.28, 1.0, 3}, //IRCS_PI20_H,     
  {"K",                "K",                        "20K",  "IM20_K",   2.030, 2.370,  1,  7, 13, 11, 2500,   300,    20,  2100,  1.22, 1.0, 3}, //IRCS_PI20_K,
  {"K-prime",          "K-prime",                 "20KP",  "IM20_KP",  1.950, 2.290,  1,  9, 13, 11, 2500,   300,    30,  2800,  1.20, 1.2, 3}, //IRCS_PI20_Kp,    
  {"H + K-prime",      "H + K-prime",             "20HK",  "IM20_HK",  1.490, 2.290,  1, 14, 13, 11, 2500,   300,    12,  1300,  -1.0, 1.0, 1}, //IRCS_PI20_HK,    
  {"L-prime",          "L-prime",                 "20LP",  "IM20_LP",  3.420, 4.120,  1, 11, 13, 11, 2500,  0.20, 0.006,  0.62,  -1.0, 1.0, 1}, //IRCS_PI20_Lp,    
  {"M-prime",          "M-prime",                   "MP",  "IM20_MP",  4.570, 4.790,  1, 13, 13, 11, 2500,  0.20, 0.006,  0.28,  -1.0, 1.0, 1}, //IRCS_PI20_Mp,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H2 1-0S(1)",       "H<sub>2</sub> 1-0S(1)", "20H210","IM20_H210",  2.106, 2.138, 11,  9, 13, 11, 2500,   300,     5,  6000,  -1.1, 0.5, 1}, //IRCS_PI20_H210,
  {"Br gamma",         "Br &#x3B3;",             "20BRG", "IM20_BRG",  2.150, 2.182, 10,  1, 13, 11, 2500,   300,     5,  6000,  -1.0, 0.5, 1}, //IRCS_PI20_BRG,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"J + ND(1%)",       "J + ND(1%)",              "20JN",  "IM20_JN",  1.170, 1.330, 12,  3, 13, 11, 2500,   300,    80,  6000,     0,   0, 0}, //IRCS_PI20_JN  
  {"H + ND(1%)",       "H + ND(1%)",              "20HN",  "IM20_HN",  1.490, 1.780, 12,  5, 13, 11, 2500,   300,    12,  1300,     0,   0, 0}, //IRCS_PI20_HN     
  {"K + ND(1%)",       "K + ND(1%)",              "20KN",  "IM20_KN",  2.030, 2.370, 12,  7, 13, 11, 2500,   300,    20,  2100,     0,   0, 0}, //IRCS_PI20_KN
  {"K-prime + ND(1%)", "K-prime + ND(1%)",       "20KPN", "IM20_KPN",  1.950, 2.290, 12,  9, 13, 11, 2500,   300,    30,  2800,     0,   0, 0}, //IRCS_PI20_KPN,    
  {"HK + ND(1%)",      "HK + ND(1%)",            "20HKN", "IM20_HKN",  1.490, 2.370, 12, 14, 13, 11, 2500,   300,    12,  1300,     0,   0, 0}, //IRCS_PI20_HKN     
  {"L-prime + ND(1%)", "L-prime + ND(1%)",       "20LPN", "IM20_LPN",  3.420, 4.120, 12, 11, 13, 11, 2500,  0.20, 0.006,  0.62,     0,   0, 0}, //IRCS_PI20_LPN,    
  {"M-prime + ND(1%)", "M-prime + ND(1%)",         "MPN", "IM20_MPN",  4.570, 4.790, 12, 13, 13, 11, 2500,  0.20, 0.006,  0.28,     0,   0, 0} //IRCS_PI20_MPN,
};


// Grism
// 52mas
enum{IRCS_GR52_IZ,
     IRCS_GR52_ZJ,
     IRCS_GR52_J,
     IRCS_GR52_H,
     IRCS_GR52_K,
     IRCS_GR52_L,
     IRCS_GR52_ZJH,
     IRCS_GR52_HK,
     IRCS_GR52_SEP1,
     IRCS_PR52_W,
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
     IRCS_GR20_SEP1,
     IRCS_PR20_KL,
     NUM_IRCS_GR20};

enum{IRCS_PS20_IZ,
     IRCS_PS20_ZJ,
     IRCS_PS20_J,
     IRCS_PS20_H,
     IRCS_PS20_K,
     IRCS_PS20_L,
     NUM_IRCS_PS20};


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
  gchar *sdef;
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

  gdouble w_flat_amp;
  gdouble w_flat_exp;
  gint    w_flat_nd;
  gdouble n_flat_amp;
  gdouble n_flat_exp;
  gint    n_flat_nd;

  gdouble comp_exp;
  gdouble comp_coadds;
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
static IRCS_GR_Entry IRCS_gr52_set[] = {
  //Name               Def              wv1   wv2  r010   disp cw1 cw2 cw3  cam                      FLT 3W          FLT 4N        Comp
  {"Iz",              "IZ", "GR52_IZ", 0.95, 1.01, 1940,   2.7,  4,  4,  1, 1000,  300, 0.41, 1000,   1.7,  0.6, 1,    1.9,  0.5, 1,    1,   1}, //IRCS_GR52_IZ,
  {"zJ",              "ZJ", "GR52_ZJ", 1.03, 1.18, 1706,   3.1,  5,  4,  1, 1000,  300, 0.41, 1000,   1.4, 0.41, 1,   -1.0, 0.41, 1,    1,   1}, //IRCS_GR52_ZJ,
  {"J",                "J",  "GR52_J", 1.18, 1.38, 1432,   3.7,  6,  4,  1, 1000,  300, 0.41, 1000,  -1.0,  0.5, 1,   -1.0,  0.5, 1,    3,   1}, //IRCS_GR52_J,
  {"H",                "H",  "GR52_H", 1.49, 1.83, 1146,   4.7,  7,  4,  1,  750,  300, 0.41, 1000,  -1.4, 0.41, 1,   -1.2,  0.5, 1,   10,   1}, //IRCS_GR52_H,
  {"K",                "K",  "GR52_K", 1.93, 2.48,  869,   6.1,  8,  4,  1, 1000,  300, 0.41, 1000,   1.0,  0.5, 1,   -1.0,  1.0, 1,   10,   1}, //IRCS_GR52_K,
  {"L",                "L",  "GR52_L", 2.84, 4.16,  331,  15.9,  9, 10,  1, 1500,   10, 0.41, 1000,  -0.6, 0.41, 1,   -2.0,  0.4, 1,  0.5, 150}, //IRCS_GR52_L,
  {"zJH",            "ZJH","GR52_ZJH", 0.95, 1.50,  705,   8.5,  2,  2,  1,  875,  300, 0.41, 1000,   1.1, 0.41, 1,   -1.1,  0.5, 1, 0.41,   1}, //IRCS_GR52_ZJH,
  {"HK",              "HK", "GR52_HK", 1.40, 2.50,  442,  12.2,  3,  8,  1,  875,  300, 0.41, 1000,   1.1,  0.5, 1,   -1.0,  0.5, 1,  0.5,  10}, //IRCS_GR52_HK,
  {NULL,             NULL,  NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                                           //IRCS_GR52_SEP1,
  {"KL Prism",         "W",  "PR52_W", 1.93, 4.16,  110,   1.0,  1, 12,  1, 2500,  300, 0.41, 1000,   1.1,  0.5, 2,    1.1,  0.5, 2,   1,    1}  //IRCS_PR52_W,
};

static IRCS_GR_Entry IRCS_ps52_set[] = {
  //Name               Def   wv1   wv2  r010   disp cw1 cw2 cw3  cam                     FLT 3W          FLT 4N       Comp
  {"Iz",              "IZ",  "PS52_IZ", 0.95, 1.01, 1940,   2.7,  4,  4, 13, 1000, 300, 0.41, 1000,  -1.7,  0.6, 1,   -1.9,  0.5, 1,   1,   5}, //IRCS_PS52_IZ,
  {"zJ",              "ZJ",  "PS52_ZJ", 1.03, 1.18, 1706,   3.1,  5,  4, 13, 1000, 300, 0.41, 1000,  -1.4, 0.41, 1,   -1.0, 0.41, 1,   3,   3}, //IRCS_PS52_ZJ,
  {"J",                "J",   "PS52_J", 1.18, 1.38, 1432,   3.7,  6,  4, 13, 1000, 300, 0.41, 1000,  -1.0,  0.5, 1,   -1.0,  0.5, 1,   3,   1}, //IRCS_PS52_J,
  {"H",                "H",   "PS52_H", 1.49, 1.83, 1146,   4.7,  7,  4, 13,  750, 300, 0.41, 1000,  -1.4, 0.41, 1,   -1.2,  0.5, 1,  10,   1}, //IRCS_PS52_H,
  {"K",                "K",   "PS52_K", 1.93, 2.48,  869,   6.1,  8,  4, 13, 1000, 300, 0.41, 1000,  -1.0,  0.5, 1,   -1.0,  1.0, 1,  10,   1}, //IRCS_PS52_K,
  {"L",                "L",   "PS52_L", 2.84, 4.16,  331,  15.9,  9, 10, 13, 1500,  10, 0.41, 1000,  -0.6, 0.41, 1,   -2.0,  0.4, 1, 0.5, 150}, //IRCS_PS52_L,
  {"zJH",            "ZJH", "PS52_ZJH", 0.95, 1.50,  705,   8.5,  2,  2, 13,  875, 300, 0.41, 1000,  -1.1, 0.41, 1,   -1.1,  0.5, 1, 0.41, 10}, //IRCS_PS52_ZJH,
  {"HK",              "HK",  "PS52_HK", 1.40, 2.50,  442,  12.2,  3,  8, 13,  875, 300, 0.41, 1000,  -1.1,  0.5, 1,   -1.0,  0.5, 1, 1.0,  10}  //IRCS_PS52_HK,
};

// 20mas wide=FALSE  fc=OUT
static IRCS_GR_Entry IRCS_gr20_set[] = {
  //Name              Def   wv1    wv2  r010  disp cw1 cw2 cw3  cam                       FLT 3W          FLT 4N       Comp
  {"Iz",            "20IZ", "GR20_IZ", 0.95, 1.01,  813,  2.3,  4,  6,  1, 2500, 300, 0.41, 2000,   -1.2, 0.5, 1,    2.2, 0.6, 1,   1,   5}, //IRCS_GR20_IZ,
  {"zJ",            "20ZJ", "GR20_ZJ", 1.03, 1.18,  745,  2.7,  5,  6,  1, 2500, 300, 0.41, 2000,   -1.2, 0.5, 1,    2.0, 0.4, 1,   3,   3}, //IRCS_GR20_ZJ,
  {"J",              "20J",  "GR20_J", 1.19, 1.38,  677,  3.3,  6,  6,  1, 2500, 300, 0.41, 2000,   -1.2, 0.5, 1,    1.1, 0.6, 1,   3,   1}, //IRCS_GR20_J,
  {"H",              "20H",  "GR20_H", 1.47, 1.80,  501,  4.1,  7,  6,  1, 2500, 300, 0.41, 2000,   -1.2, 0.5, 1,    1.5, 0.4, 1,  10,   1}, //IRCS_GR20_H,
  {"K",              "20K",  "GR20_K", 1.92, 2.40,  393,  5.4,  8,  6,  1, 2500, 300, 0.41, 2000,   -1.2, 0.5, 1,    1.4, 1.5, 1,  10,   1}, //IRCS_GR20_K,
  {"L",              "20L",  "GR20_L", 2.84, 4.16,  200,    0,  9, 10,  1, 2500,  10, 0.41, 1000,   -1.2, 0.5, 1,    1.7, 0.5, 1, 0.5, 150}, //IRCS_GR20_L,
  {NULL,             NULL,   NULL,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                                       //IRCS_GR20_SEP1,
  {"KL Prism",        "KL", "PR20_KL", 1.93, 4.16,  110,  1.0,  1, 12,  1, 2500,  300, 0.41, 1000,  2.8,  0.5, 3,    2.7, 0.5, 3,   1,   1}  //IRCS_PR20_KL,
};

static IRCS_GR_Entry IRCS_ps20_set[] = {
  //Name              Def   wv1    wv2  r010  disp cw1 cw2 cw3  cam  
  {"Iz",            "20IZ", "PS20_IZ", 0.95, 1.01,  813,  2.3,  4,  6, 13, 2500, 300, 0.41, 2000,   -1.2, 1.0, 1,   -2.2, 0.6, 1,   1,   5}, //IRCS_PS20_IZ,
  {"zJ",            "20ZJ", "PS20_ZJ", 1.03, 1.18,  745,  2.7,  5,  6, 13, 2500, 300, 0.41, 2000,   -1.2, 1.0, 1,   -2.0, 0.4, 1,   3,   3}, //IRCS_PS20_ZJ,
  {"J",              "20J",  "PS20_J", 1.19, 1.38,  677,  3.3,  6,  6, 13, 2500, 300, 0.41, 2000,   -1.2, 1.0, 1,   -1.1, 0.6, 1,   3,   1}, //IRCS_PS20_J,
  {"H",              "20H",  "PS20_H", 1.47, 1.80,  501,  4.1,  7,  6, 13, 2500, 300, 0.41, 2000,   -1.2, 1.0, 1,   -1.5, 0.4, 1,  10,   1}, //IRCS_PS20_H,
  {"K",              "20K",  "PS20_K", 1.92, 2.40,  393,  5.4,  8,  6, 13, 2500, 300, 0.41, 2000,   -1.2, 1.0, 1,   -1.4, 1.5, 1,  10,   1}, //IRCS_PS20_K,
  {"L",              "20L",  "PS20_L", 2.84, 4.16,  200,    0,  9, 10, 13, 2500,  10, 0.41, 1000,   -1.2, 1.0, 1,   -1.7, 0.5, 1, 0.5, 150}  //IRCS_PS20_L,
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
  gchar *markup;
  gchar *def;
  gchar *sdef;
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

  gdouble flat_amp;
  gdouble flat_exp;
  gdouble flat_nd;
  gdouble comp_exp;
  gdouble comp_coadds;
};

// 55mas FC=out
static IRCS_ECD_Entry IRCS_ecd_set[] = {
  //Name                                                 Def   sdef            r1     r2     slwn slwm slww  spw    ech    xds                     FLAT         Comp
  {"Iz",              "Iz",                             "IZ", "ECD_IZ",        20000, 20000,    4,   7,  10,   2,  2600, -1000, 300, 0.41, 900, 1.8, 0.5, 1,  1, 1}, //IRCS_ECD_IZ,   
  {"zJ",              "zJ",                             "ZJ", "ECD_ZJ",        19000, 22600,    4,   7,  10,   3,  2240,  2000, 300, 0.41, 900, 2.1, 0.7, 1, 10, 1}, //IRCS_ECD_ZJ,   
  {"J",               "J",                              "J",  "ECD_J",         19000, 22600,    4,   7,  10,   4,  2240,     0, 300, 0.41, 900, 1.8, 0.6, 1, 10, 1}, //IRCS_ECD_J,	   
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H-",              "H<sup>-</sup>",                  "HM", "ECD_HM",        18800, 22900,    3,   6,   9,   5,  3610,  -500, 100, 0.41, 200, 1.6, 0.5, 1, 10, 1}, //IRCS_ECD_HM,   
  {"H+",              "H<sup>+</sup>",                  "HP", "ECD_HP",        18800, 22900,    3,   6,   9,   5,  1490,  -180, 100, 0.41, 200, 1.6, 0.5, 1, 10, 1}, //IRCS_ECD_HP,   
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"K-",              "K<sup>-</sup>",                  "KM", "ECD_KM",        18300, 22300,    3,   6,   9,   6,  5360,  -470, 100, 0.41, 300, 1.6,0.41, 1, 30, 1}, //IRCS_ECD_KM,   
  {"K+",              "K<sup>+</sup>",                  "KP", "ECD_KP",        18300, 22300,    3,   6,   9,   6,  -270,   200, 100, 0.41, 300, 1.4, 0.5, 1, 30, 1}, //IRCS_ECD_KP,   
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"LA+",             "LA<sup>+</sup>",                 "LAP","ECD_LAP",       17300, 25100,    2,   5,   9,   7, -4500,  1300,  10, 0.41,  30, 1.6, 0.5, 1, 0.5, 1}, //IRCS_ECD_LAP,  
  {"LA0",             "LA<sup>0</sup>",                 "LAZ","ECD_LAZ",       17300, 25100,    2,   5,   9,   7,  2500,   500,  10, 0.41,  30, 1.4, 0.5, 1, 0.5, 1}, //IRCS_ECD_LAZ,  
  {"LA-",             "LA<sup>-</sup>",                 "LAM","ECD_LAM",       17300, 25100,    2,   5,   9,   7, 10500,  -200,  10, 0.41,  30, 1.6, 0.5, 1, 0.5, 1}, //IRCS_ECD_LAM,  
  {"LB+",             "LB<sup>+</sup>",                 "LBP","ECD_LBP",       17300, 25100,    2,   5,   9,   7, -4500,  4300,  10, 0.41,  30, 1.8,0.41, 1, 0.5, 1}, //IRCS_ECD_LBP,  
  {"LB0",             "LB<sup>0</sup>",                 "LBZ","ECD_LBZ",       17300, 25100,    2,   5,   9,   7,  2500,  3600,  10, 0.41,  30, 1.8,0.41, 1, 0.5, 1}, //IRCS_ECD_LBZ,  
  {"LB-",             "LB<sup>-</sup>",                 "LBM","ECD_LBM",       17300, 25100,    2,   5,   9,   7, 10500,  2700,  10, 0.41,  30, 1.8,0.41, 1, 0.5, 1}, //IRCS_ECD_LBM,  
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"M--",             "M<sup>--</sup>",                 "MMM","ECD_MMM",       16800, 26300,    2,   5,   9,   8, 12000, -5500,   2, 0.41,   4, 1.8,0.41, 1, 0.5, 1}, //IRCS_ECD_MMM,  
  {"M-",              "M<sup>-</sup>",                  "MM", "ECD_MM",        16800, 26300,    2,   5,   9,   8,  4000, -5500,   2, 0.41,   4, 1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_MM,   
  {"M+",              "M<sup>+</sup>",                  "MP", "ECD_MP",        16800, 26300,    2,   5,   9,   8, -4000, -5500,   2, 0.41,   4, 1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_MP,   
  {"M++",             "M<sup>++</sup>",                 "MPP","ECD_MPP",       16800, 26300,    2,   5,   9,   8,-11000, -5500,   2, 0.41,   4, 1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_MPP,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"FeII 1650:60nm",  "FeII &#x3BB;1650:60nm",          "FEH",   "ECD_FEH",    18800, 22900,    2,   5,   0,  11,  5000,  -300, 100, 0.41, 200,1.75, 0.5, 1, 10, 1}, //IRCS_ECD_FEH,  
  {"FeII 1252:25nm",  "FeII &#x3BB;1252:25nm",          "FEJ",   "ECD_FEJ",    19000, 22600,    2,   5,   0,  10,  2240,     0, 300, 0.41, 900, 2.2, 0.5, 1, 10, 1}, //IRCS_ECD_FEJ,  
  {"He 1085:20nm",    "He &#x3BB;1085:20nm",            "HE1085","ECD_HE1085", 19000, 22600,    2,   5,   0,   9,  2240,     0, 300, 0.41, 900, 2.7, 0.5, 1, 10, 1}, //IRCS_ECD_HE1085,
  {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {"H(FeII)",         "H (FeII)",                       "HFE",  "ECD_HFE",     18800, 22900,    3,   6,   9,   5,  3610,  -300, 100, 0.41, 200,-1.8, 0.5, 1, 10, 1}, //IRCS_ECD_HFE,  
  {"K(H2)",           "K (H<sub>2</sub>)",              "KH2",  "ECD_KH2",     18300, 22300,    3,   6,   9,   6,  6450,   500, 100, 0.41, 300,-1.8, 0.5, 1, 30, 1}, //IRCS_ECD_KH2,  
  {"L(CH4) original", "L (CH<sub>4</sub>) original",    "LCH",  "ECD_LCH",     18300, 22300,    2,   5,   9,   7,  1200,   670,  10, 0.41,  30,-1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_LCH,  
  {"L(H3+)",          "L (H<sub>3</sub><sup>+</sup>)",  "LH3",  "ECD_LH3",     18300, 22300,    2,   5,   9,   7,  8350,  2912,  10, 0.41,  30,-1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_LH3,  
  {"M(CO)",           "M (CO)",                         "MCO",  "ECD_MCO",     16800, 26300,    2,   5,   9,   8,  8200, -5500,   2, 0.41,   4,-1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_MCO,  
  {"L(C2H6)",         "L (C<sub>2</sub>H<sub>6</sub>)", "LC2H6","ECD_LC2H6",   18300, 22300,    2,   5,   9,   7,  3350,  -250,  10, 0.41,  30,-1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_LC2H6,
  {"L(CH4)",          "L (CH<sub>4</sub>)",             "LCH4", "ECD_LCH4",    18300, 22300,    2,   5,   9,   7,  7350,  -200,  10, 0.41,  30,-1.8, 0.5, 1, 0.5, 1}, //IRCS_ECD_LCH4,
  {"L(H2O Ice)",      "L (H<sub>2</sub>O Ice)",         "LH2O", "ECD_LH2O",    18300, 22300,    2,   5,   9,   7,  8150,  1200,  10, 0.41,  30,-1.8, 0.5, 1, 0.5, 1}  //IRCS_ECD_LH2O, 
};


typedef struct _IRCS_ECM_Entry IRCS_ECM_Entry;
struct _IRCS_ECM_Entry{
  gchar *name;
  gchar *markup;
  gchar *sdef;
  guint slw_n;
  guint slw_m;
  guint slw_w;
  guint spw;
  gdouble w1;
  gdouble w2;

  gdouble defexp;
  gdouble minexp;
  gdouble maxexp;

  gdouble flat_amp;
  gdouble flat_exp;
  gdouble flat_nd;
  gdouble comp_exp;
  gdouble comp_coadds;
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

static IRCS_ECM_Entry IRCS_ecm_set[] = {
  //Name
  {"Iz",         "Iz", "ECM_IZ",                4,  7, 10,  2,  0.91, 1.00, 300, 0.41, 900, -1.8, 0.5,  1,  1, 1},  //IRCS_ECSPW_IZ,    
  {"zJ",         "zJ", "ECM_ZJ",                4,  7, 10,  3,  1.03, 1.17, 300, 0.41, 900, -2.1, 0.7,  1, 10, 1},  //IRCS_ECSPW_ZJ,    
  {"J",          "J",  "ECM_J",                 4,  7, 10,  4,  1.17, 1.37, 300, 0.41, 900, -1.8, 0.6,  1, 10, 1},  //IRCS_ECSPW_J,     
  {"H",          "H",  "ECM_H",                 3,  6,  9,  5,  1.47, 1.80, 100, 0.41, 200, -1.6, 0.5,  1, 10, 1},  //IRCS_ECSPW_H,     
  {"K",          "K",  "ECM_K",                 3,  6,  9,  6,  1.92, 2.52, 100, 0.41, 300, -1.4, 0.5,  1, 30, 1},  //IRCS_ECSPW_K,     
  {"L",          "L",  "ECM_L",                 2,  5,  9,  7,  2.90, 4.25,  10, 0.41,  30, -1.8,0.41,  1,0.5, 1},  //IRCS_ECSPW_L,     
  {"M",          "M",  "ECM_M",                 2,  5,  9,  8,  4.40, 6.00,   2, 0.41,   4, -1.8, 0.5,  1,0.5, 1},  //IRCS_ECSPW_M,
  {NULL,NULL,NULL,0,0,0,0,0,0,0,0,0,0},
  {"He 1085",    "He &#x3BB;1085",    "ECM_HE1085", 2,  5,  9,  9, 1.073,1.093, 300, 0.41, 900,-1.75, 0.5,  1, 10, 1},  //IRCS_ECSPW_HE1085,
  {"Fe II 1252", "Fe II &#x3BB;1252", "ECM_FE1252", 2,  5,  9, 10, 1.239,1.265, 300, 0.41, 900, -2.2, 0.5,  1, 10, 1},  //IRCS_ECSPW_FE1252,
  {"Fe II 1650", "Fe II &#x3BB;1650", "ECM_FE1650", 2,  5,  9, 11, 1.616,1.676, 100, 0.41, 200, -2.7, 0.5,  1, 10, 1}   //IRCS_ECSPW_FE1650,
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
  gchar *dtxt;

  gboolean std;

  gdouble exp;
  guint shot;
};


// IRCSTreeview
enum
{
  COLUMN_IRCS_NUMBER,
  COLUMN_IRCS_NAME,
  COLUMN_IRCS_DEF,
  COLUMN_IRCS_DITH,
  COLUMN_IRCS_EXP,
  COLUMN_IRCS_COLFG,
  COLUMN_IRCS_COLBG,
  NUM_COLUMN_IRCS
};


/*
#ifdef USE_GTK3
static GdkRGBA col_ircs_setup [IRCS_MAX_SET]
= {
  {0.80, 0.80, 1.00, 1}, //pale2
  {1.00, 1.00, 0.80, 1}, //orange2
  {1.00, 0.80, 1.00, 1}, //purple2
  {0.80, 1.00, 0.80, 1}, //green2
  {1.00, 0.80, 0.80, 1}, //pink2
  {0.80, 0.80, 1.00, 1}, //pale2
  {1.00, 1.00, 0.80, 1}, //orange2
  {1.00, 0.80, 1.00, 1}, //purple2
  {0.80, 1.00, 0.80, 1}, //green2
  {1.00, 0.80, 0.80, 1}, //pink2
  {0.80, 0.80, 1.00, 1}, //pale2
  {1.00, 1.00, 0.80, 1}, //orange2
  {1.00, 0.80, 1.00, 1}, //purple2
  {0.80, 1.00, 0.80, 1}, //green2
  {1.00, 0.80, 0.80, 1}, //pink2
  {0.80, 0.80, 1.00, 1}, //pale2
  {1.00, 1.00, 0.80, 1}, //orange2
  {1.00, 0.80, 1.00, 1}, //purple2
  {0.80, 1.00, 0.80, 1}, //green2
  {1.00, 0.80, 0.80, 1}  //pink2
};
#else
static GdkColor col_ircs_setup [IRCS_MAX_SET]
= {
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}, //pink2
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}, //pink2
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}, //pink2
  {0, 0xCCCC, 0xCCCC, 0xFFFF}, //pale2
  {0, 0xFFFF, 0xFFFF, 0xCCCC}, //orange2
  {0, 0xFFFF, 0xCCCC, 0xFFFF}, //purple2
  {0, 0xCCCC, 0xFFFF, 0xCCCC}, //green2
  {0, 0xFFFF, 0xCCCC, 0xCCCC}  //pink2
};
#endif
*/

// MagDB for Guide Star Selection
#define IRCS_MAGDB_MAG_TTGS 18
#define IRCS_MAGDB_MAG_NGS 16.5
#define IRCS_MAGDB_R_TGT_ARCSEC 3
#define IRCS_MAGDB_R_NGS_ARCSEC 30
#define IRCS_MAGDB_R_TTGS_ARCSEC 60
#define IRCS_MAGDB_DSE_R1   0.3
#define IRCS_MAGDB_DSE_R2   1.5
#define IRCS_MAGDB_DSE_MAG 1.0


// Overheads
#define IRCS_TIME_READOUT_NORMAL 0.41
#define IRCS_TIME_READOUT_FAST   0.119
#define IRCS_TIME_FITS           3.8
#define IRCS_NGS_MAG1            10
#define IRCS_NGS_MAG2            14
#define IRCS_TIME_AO_NGS1        300  // NGS mag < MAG1
#define IRCS_TIME_AO_NGS2        600  // NGS mag = MAG1 - MAG2
#define IRCS_TIME_AO_NGS3        900  // NGS mag > MAG2
#define IRCS_TIME_AO_LGS         900  // LGS 
#define IRCS_TIME_ACQ            300
#define IRCS_TIME_FILTER         60
#define IRCS_TIME_FOCUS_OBE      300
#define IRCS_TIME_FOCUS_LGS      600
#define IRCS_TIME_FLAT           300



//////////////////////////////////////////////////////////////////////////
//////////// proto types
//////////////////////////////////////////////////////////////////////////

void IRCS_TAB_create();
void IRCS_OH_TAB_create();
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
gchar* get_tgt_mag_txt();
gchar* ircs_get_filter_name();
gdouble ircs_get_slit_width();
gchar* ircs_make_setup_txt();
gchar* ircs_make_def();

void IRCS_WriteLGS();
void IRCS_WriteService();

gchar* ircs_plan_make_dtxt();
gchar* ircs_make_dtxt();

gboolean ircs_check_spec();
gboolean ircs_check_mode();
gboolean ircs_check_lgs();

gint ircs_get_shot();

gdouble ircs_get_flat_exp();
gdouble ircs_get_flat_amp();
gint ircs_get_flat_nd();
gdouble ircs_get_comp_exp();
gint ircs_get_comp_coadds();
gdouble ircs_get_comp_amp();

void IRCS_WriteOPE_FOCUS_plan();
void IRCS_WriteOPE_COMP_plan();
void IRCS_WriteOPE_FLAT_plan();
void IRCS_WriteOPE_OBJ_plan();

gint ircs_oh_ao();
gint ircs_get_1obj_time();

void up_item_ircs_tree ();
void down_item_ircs_tree ();

void ircs_do_save_lgs_txt();
void ircs_do_export_def_list();

gchar * ircs_get_mode_initial();

gboolean ircs_svcmag();
gboolean ircs_obsreq ();

void IRCS_Read_Cal();
void IRCS_Read_Overhead();
void ircs_sync_cal();
void ircs_sync_overhead();
void ircs_cal_dl();
void ircs_overhead_dl();
