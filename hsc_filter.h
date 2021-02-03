//    hoe : Subaru HDS++ OPE file Editor
//      hsc_filter.h  --- Hyper Suprime Cam default filter info
//   
//                                           2021.02.03  A.Tajitsu

enum{
  HSC_FIL_G,
  HSC_FIL_R2,
  HSC_FIL_I2,
  HSC_FIL_Z,
  HSC_FIL_Y,
  HSC_FIL_SEP1,
  HSC_FIL_NB0387,
  //HSC_FIL_NB0391,
  HSC_FIL_NB0400,
  HSC_FIL_NB0430,
  HSC_FIL_NB0468,
  HSC_FIL_NB0497,
  HSC_FIL_NB0515,
  HSC_FIL_NB0527,
  HSC_FIL_NB0656,
  HSC_FIL_NB0718,
  HSC_FIL_NB0816,
  HSC_FIL_NB0921,
  HSC_FIL_NB0926,
  HSC_FIL_IB0945,
  HSC_FIL_NB0973,
  HSC_FIL_NB1010,
  NUM_HSC_FIL
};
  
static const HSCfilter hsc_filter_stock[] = {
  {"HSC-g",   5,  13.5,  0.2,  TRUE,   6.0,  6.33,  24,  10,  TRUE,  27.8,  29.0},  //HSC_FIL_G,	 
  {"HSC-r2",  9,  14.5,  0.2,  TRUE,   4.0,  5.10,   9,  10,  TRUE,  27.2,  29.1},  //HSC_FIL_R2,	 
  {"HSC-i2",  8,  14.0,  0.2,  TRUE,   4.0,  5.10,   6,  10,  TRUE,  26.5,  28.7},  //HSC_FIL_I2,	 
  {"HSC-z",   2,  13.0,  0.3,  TRUE,   4.0,  5.10,  12,  10,  TRUE,  25.9,  27.7},  //HSC_FIL_Z,	 
  {"HSC-Y",   1,  14.0,  0.3,  TRUE,   4.0,  5.10,  13,  10,  TRUE,  25.1,  27.4},  //HSC_FIL_Y,	 
  {NULL,     -1,    -1,   -1, FALSE,    -1,    -1,  -1,  -1, FALSE,    -1,    -1},  //HSC_FIL_SEP1,	 
  {"NB0387", 18,  11.5, 10.0,  TRUE, 120.0,  5.10,  30, 600,  TRUE,  25.7,  24.6},  //HSC_FIL_NB0387,
  //{"NB0391", 22,  11.5, 10.0, FALSE, 120.0,  5.10,  30, 600, FALSE,  25.7,  24.6},  //HSC_FIL_NB0391,
  {"NB0400", 23,  11.5, 10.0,  TRUE, 120.0,  5.10,  30, 600, FALSE,  26.0,  25.4},  //HSC_FIL_NB0400,
  {"NB0430", 26,  11.5, 10.0,  TRUE, 120.0,  5.10,  30, 600, FALSE,  25.8,  25.2},  //HSC_FIL_NB0430,
  {"NB0468", 19,  12.5,  0.5,  TRUE,  60.0,  5.10,  30, 600, FALSE,  26.3,  26.0},  //HSC_FIL_NB0468,
  {"NB0497", 25,  12.5,  0.5,  TRUE,  60.0,  5.10,  30, 600, FALSE,  26.0,  25.7},  //HSC_FIL_NB0468,
  {"NB0515", 17,  12.5,  0.5,  TRUE,  60.0,  5.10,  30, 600, FALSE,  26.2,  25.9},  //HSC_FIL_NB0515,
  {"NB0527", 16,  12.5,  0.5,  TRUE,  60.0,  5.10,  30, 600, FALSE,  26.3,  25.9},  //HSC_FIL_NB0527,
  {"NB0656",  7,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  25.8,  26.1},  //HSC_FIL_NB0656,
  {"NB0718", 15,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  25.8,  25.9},  //HSC_FIL_NB0718,
  {"NB0816", 14,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  25.6,  25.6},  //HSC_FIL_NB0816,
  {"NB0921", 13,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  25.4,  25.7},  //HSC_FIL_NB0921,
  {"NB0926", 20,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  24.8,  25.7},  //HSC_FIL_NB0926,
  {"IB0945", 21,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  24.8,  26.4},  //HSC_FIL_IB0945,
  {"NB0973", 12,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  24.2,  25.1},  //HSC_FIL_NB0973,
  {"NB1010", 11,  12.5,  0.5,  TRUE,   6.0,  6.33,  30,  10, FALSE,  23.4,  24.1}   //HSC_FIL_NB1010,
};
