//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      hsc.h  --- Hyper Suprime Cam CCD chips information
//   
//                                           2012.10.22  A.Tajitsu

/*
[VERSION]
version: 116 CCDs for Summit4
comment: 2014/03/06 Updated by Utsumi, Y.
original: 20140206.cfg	# which file is the original one of this cfg file
[HEADER]
# row number
# key
# comment
# format f,	s, i, b: float, string, integer, bool
# separator ",\t"
*0:	T_GAIN1,	"Gain for channel 1",	f
*1:	T_GAIN2,	"Gain for channel 2",	f
*2:	T_GAIN3,	"Gain for channel 3",	f
*3:	T_GAIN4,	"Gain for channel 4",	f
(4:	T_CCDID,	"Name of CCD",	s 
*5:	T_XFLIP,	"CCD readout is x-flipped when create image",	b 
*6:	T_YFLIP,	"CCD readout is y-flipped when create image",	b 
7:	T_CCDSN,	"CCD Serial Number",   s
*8:	CRVAL1,		"Physical value of the reference pixel X",	f  =0.000
*9:	CRVAL2,		"Physical value of the reference pixel Y",	f  =0.000
10:	CRPIX1,		"Reference pixel in X (pixel)",	f
11:	CRPIX2,		"Reference pixel in Y (pixel)",	f
*12:	CDELT1,		"X Scale projected on detector (#/pix)",	f  =0.015
*13:	CDELT2,		"Y Scale projected on detector (#/pix)",	f  =0.015
*14:	CTYPE1,		"Pixel coordinate system",	s
*15:	CTYPE2,		"Pixel coordinate system",	s
*16:	CUNIT1,		"Units used in both CRVAL1 and CDELT1",	s
*17:	CUNIT2,		"Units used in both CRVAL2 and CDELT2",	s
18:	CD1_1,		"Pixel Coordinate translation matrix", f
19:	CD1_2,		"Pixel Coordinate translation matrix", f
20:	CD2_1,		"Pixel Coordinate translation matrix", f
21:	CD2_2,		"Pixel Coordinate translation matrix", f
22:	DET-ID,		"Sequential number assgined for the entire CCDs", i
*23:	T_NFRAME,	"Number of total frames",	i

typedef struct _HSCParam HSCParam;
struct _HSCParam{
  gint hsca;
  gint bees;
  
  gint ccdsn;
  
  gfloat crpix1;
  gfloat crpix2;

  gfloat cd11;
  gfloat cd12;
  gfloat cd21;
  gfloat cd22;

  gint det_id;
};

*/


#define HSC_MAX_SET 20

typedef struct _HSCfilter HSCfilter;
struct _HSCfilter{
  gchar *name;
  gint   id;

  gdouble good_mag;
  gdouble ag_exp;
  gboolean ag_flg;

  gdouble flat_v;
  gdouble flat_a;
  gint    flat_exp;
  gint    flat_w;
  gboolean flat_flg;

  gdouble sens;
  gdouble mag1e;
};

#include "hsc_filter.h"

#define HSC_FILTER_HOST "www.naoj.org"
#define HSC_FILTER_PATH "/Observing/Instruments/HSC/hsc_filter.ini"
#define HSC_FILTER_FILE "hsc_filter.ini"
#ifdef USE_WIN32
#define HSC_FILTER_LOCAL "hsc_filter.ini"
#else
#define HSC_FILTER_LOCAL ".hoe_hsc"
#endif

#define MAX_HSC_FIL 1000
HSCfilter hsc_filter[MAX_HSC_FIL];

// Dithering
enum{
  HSC_DITH_NO,
  HSC_DITH_5,
  HSC_DITH_N,
  NUM_HSC_DITH
};

static const gchar* hsc_dith_name[]={
  "No dither",
  "5-shot",
  "N-shot"
};


typedef struct _HSCpara HSCpara;
struct _HSCpara{
  gint filter;

  gchar *txt;
  gchar *def;
  gchar *dtxt;

  gdouble exp;
  
  gint osra;
  gint osdec;
  
  gint dith;
  gint dith_ra;
  gint dith_dec;
  gint dith_n;
  gint dith_t;
  gint dith_r;
  gboolean ag;
};


typedef struct _HSCmag HSCmag;
struct _HSCmag{
  gint hits;
  gchar* name;
  gdouble v;
  gdouble sep;

  gint mag6;
  gint mag7;
  gint mag8;
  gint mag9;
  gint mag10;
  gint mag11;
};


typedef struct _HSCchips HSCchips;
struct _HSCchips{
  gint hsca;
  gint bees;
  
  gint ccdsn;
  
  gfloat crpix1;
  gfloat crpix2;
  
  gfloat cd1_1;
  gfloat cd1_2;
  gfloat cd2_1;
  gfloat cd2_2;
  
  gint det_id;
};

#define HSC_CHIP_ALL 116

typedef struct _HSCdead HSCdead;
struct _HSCdead{
  gint hsca;
  gint bees;
  
  gint ch;

  gfloat crpix1;
  gfloat crpix2;
  
  gfloat cd1_1;
  gfloat cd1_2;
  gfloat cd2_1;
  gfloat cd2_2;
};

static const HSCdead hsc_dead[] = { 
  {20, 0, 3, 5269.833,	  4128.500,	0.000,	-0.015,	-0.015,	0.000},
  {20, 0, 4, 5269.833,	  4128.500,	0.000,	-0.015,	-0.015,	0.000},
  {51, 0, 1, -15956.833,  8597.167,	0.000,	-0.015,	-0.015,	0.000},
  {9,  1, 4, -1098.167,	  8863.833,	0.000,	0.015,	0.015,	0.000},
  {47, 1, 1, -11711.500, 13332.500,	0.000,	0.015,	0.015,	0.000},
  {47, 1, 2, -11711.500, 13332.500,	0.000,	0.015,	0.015,	0.000},
  {47, 1, 3, -11711.500, 13332.500,	0.000,	0.015,	0.015,	0.000},
  {47, 1, 4, -11711.500, 13332.500,	0.000,	0.015,	0.015,	0.000},
  {51, 1, 1, -15956.833,  8863.833,	0.000,	0.015,	0.015,	0.000},
  {51, 1, 2, -15956.833,  8863.833,	0.000,	0.015,	0.015,	0.000},
  {53, 1, 3, -13834.167, -4542.167,	0.000,	0.015,	0.015,	0.000}
};

#define HSC_DEAD_ALL 11

static const HSCchips hsc_chips[] = {
   {0,	0,	99,		 -5343.500,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	73},
   {1,	0,	56,		 -5343.500,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	72},
   {2,	0,	118,		 -5343.500,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	71},
   {3,	0,	133,		 -5343.500,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	70},
   {4,	0,	73,		 -3220.833,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	65},
   {5,	0,	174,		 -3220.833,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	64},
   {6,	0,	59,		 -3220.833,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	63},
   {7,	0,	125,		 -3220.833,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	62},
   {8,	0,	160,		 -1098.167,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	57},
   {9,	0,	32,		 -1098.167,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	56},
   {10,	0,	52,		 -1098.167,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	55},
   {11,	0,	69,		 -1098.167,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	54},
   {12,	0,	127,		  1024.500,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	49},
   {13,	0,	21,		  1024.500,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	48},
   {14,	0,	148,		  1024.500,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	47},
   {15,	0,	103,		  1024.500,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	46},
   {16,	0,	168,		  3147.167,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	41},
   {17,	0,	173,		  3147.167,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	40},
   {18,	0,	178,		  3147.167,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	39},
   {19,	0,	43,		  3147.167,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	38},
   {20,	0,	48,		  5269.833,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	33},
   {21,	0,	58,		  5269.833,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	32},
   {22,	0,	136,		  5269.833,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	31},
   {23,	0,	38,		  5269.833,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	30},
   {24,	0,	162,		  7392.500,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	25},
   {25,	0,	34,		  7392.500,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	24},
   {26,	0,	76,		  7392.500,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	23},
   {27,	0,	102,		  7392.500,	 17534.500,		0.000,	-0.015,	-0.015,	0.000,	22},
   {28,	0,	35,		  9515.167,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	18},
   {29,	0,	128,		  9515.167,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	17},
   {30,	0,	149,		  9515.167,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	16},
   {31,	0,	143,		-13308.500,	 11691.167,		0.015,	0.000,	0.000,	-0.015,	100},
   {32,	0,	134,		 -7466.167,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	80},
   {33,	0,	65,		 -7466.167,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	79},
   {34,	0,	158,		 -7466.167,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	78},
   {35,	0,	74,		 15357.500,	 11691.167,		-0.015,	0.000,	0.000,	0.015,	102},
   {36,	0,	122,		 -9588.833,	 -9277.500,		0.000,	-0.015,	-0.015,	0.000,	89},
   {37,	0,	180,		 -9588.833,	 -4808.833,		0.000,	-0.015,	-0.015,	0.000,	88},
   {38,	0,	36,		 -9588.833,	  -340.167,		0.000,	-0.015,	-0.015,	0.000,	87},
   {39,	0,	176,		 -9588.833,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	86},
   {40,	0,	101,		 -9588.833,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	85},
   {41,	0,	30,		 -9588.833,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	84},
   {42,	0,	85,		-11711.500,	 -9277.500,		0.000,	-0.015,	-0.015,	0.000,	95},
   {43,	0,	172,		-11711.500,	 -4808.833,		0.000,	-0.015,	-0.015,	0.000,	94},
   {44,	0,	41,		-11711.500,	  -340.167,		0.000,	-0.015,	-0.015,	0.000,	93},
   {45,	0,	140,		-11711.500,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	92},
   {46,	0,	33,		-11711.500,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	91},
   {47,	0,	155,		-11711.500,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	90},
   {48,	0,	109,		-15956.833,	 -4808.833,		0.000,	-0.015,	-0.015,	0.000,	111},
   {51,	0,	113,		-15956.833,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	110},
   {52,	0,	114,		-13834.167,	 -9277.500,		0.000,	-0.015,	-0.015,	0.000,	109},
   {53,	0,	141,		-13834.167,	 -4808.833,		0.000,	-0.015,	-0.015,	0.000,	99},
   {54,	0,	146,		-13834.167,	  -340.167,		0.000,	-0.015,	-0.015,	0.000,	98},
   {55,	0,	95,		-13834.167,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	97},
   {56,	0,	96,		-13834.167,	  8597.167,		0.000,	-0.015,	-0.015,	0.000,	96},
   {57,	0,	108,		-13834.167,	 13065.833,		0.000,	-0.015,	-0.015,	0.000,	108},
   //[BEES1]
   {0,	1,	88,		 -5343.500,	  4395.167,		0.000,	0.015,	0.015,	0.000,	26},
   {1,	1,	94,		 -5343.500,	  8863.833,		0.000,	0.015,	0.015,	0.000,	27},
   {2,	1,	70,		 -5343.500,	 13332.500,		0.000,	0.015,	0.015,	0.000,	28},
   {3,	1,	157,		 -5343.500,	 17801.167,		0.000,	0.015,	0.015,	0.000,	29},
   {4,	1,	169,		 -3220.833,	  4395.167,		0.000,	0.015,	0.015,	0.000,	34},
   {5,	1,	171,		 -3220.833,	  8863.833,		0.000,	0.015,	0.015,	0.000,	35},
   {6,	1,	170,		 -3220.833,	 13332.500,		0.000,	0.015,	0.015,	0.000,	36},
   {7,	1,	61,		 -3220.833,	 17801.167,		0.000,	0.015,	0.015,	0.000,	37},
   {8,	1,	165,		 -1098.167,	  4395.167,		0.000,	0.015,	0.015,	0.000,	42},
   {9,	1,	49,		 -1098.167,	  8863.833,		0.000,	0.015,	0.015,	0.000,	43},
   {10,	1,	91,		 -1098.167,	 13332.500,		0.000,	0.015,	0.015,	0.000,	44},
   {11,	1,	119,		 -1098.167,	 17801.167,		0.000,	0.015,	0.015,	0.000,	45},
   {12,	1,	120,		  1024.500,	  4395.167,		0.000,	0.015,	0.015,	0.000,	50},
   {13,	1,	45,		  1024.500,	  8863.833,		0.000,	0.015,	0.015,	0.000,	51},
   {14,	1,	86,		  1024.500,	 13332.500,		0.000,	0.015,	0.015,	0.000,	52},
   {15,	1,	121,		  1024.500,	 17801.167,		0.000,	0.015,	0.015,	0.000,	53},
   {16,	1,	175,		  3147.167,	  4395.167,		0.000,	0.015,	0.015,	0.000,	58},
   {17,	1,	54,		  3147.167,	  8863.833,		0.000,	0.015,	0.015,	0.000,	59},
   {18,	1,	98,		  3147.167,	 13332.500,		0.000,	0.015,	0.015,	0.000,	60},
   {19,	1,	71,		  3147.167,	 17801.167,		0.000,	0.015,	0.015,	0.000,	61},
   {20,	1,	138,		  5269.833,	  4395.167,		0.000,	0.015,	0.015,	0.000,	66},
   {21,	1,	44,		  5269.833,	  8863.833,		0.000,	0.015,	0.015,	0.000,	67},
   {22,	1,	161,		  5269.833,	 13332.500,		0.000,	0.015,	0.015,	0.000,	68},
   {23,	1,	66,		  5269.833,	 17801.167,		0.000,	0.015,	0.015,	0.000,	69},
   {24,	1,	132,		  7392.500,	  4395.167,		0.000,	0.015,	0.015,	0.000,	74},
   {25,	1,	79,		  7392.500,	  8863.833,		0.000,	0.015,	0.015,	0.000,	75},
   {26,	1,	47,		  7392.500,	 13332.500,		0.000,	0.015,	0.015,	0.000,	76},
   {27,	1,	92,		  7392.500,	 17801.167,		0.000,	0.015,	0.015,	0.000,	77},
   {28,	1,	53,		  9515.167,	  4395.167,		0.000,	0.015,	0.015,	0.000,	81},
   {29,	1,	151,		  9515.167,	  8863.833,		0.000,	0.015,	0.015,	0.000,	82},
   {30,	1,	64,		  9515.167,	 13332.500,		0.000,	0.015,	0.015,	0.000,	83},
   {31,	1,	130,		-13575.167,	 11691.167,		-0.015,	0.000,	0.000,	0.015,	103},
   {32,	1,	57,		 -7466.167,	  4395.167,		0.000,	0.015,	0.015,	0.000,	19},
   {33,	1,	67,		 -7466.167,	  8863.833,		0.000,	0.015,	0.015,	0.000,	20},
   {34,	1,	117,		 -7466.167,	 13332.500,		0.000,	0.015,	0.015,	0.000,	21},
   {35,	1,	131,		 15624.167,	 11691.167,		0.015,	0.000,	0.000,	-0.015,	101},
   {36,	1,	90,		 -9588.833,	 -9010.833,		0.000,	0.015,	0.015,	0.000,	10},
   {37,	1,	31,		 -9588.833,	 -4542.167,		0.000,	0.015,	0.015,	0.000,	11},
   {38,	1,	179,		 -9588.833,	   -73.500,		0.000,	0.015,	0.015,	0.000,	12},
   {39,	1,	166,		 -9588.833,	  4395.167,		0.000,	0.015,	0.015,	0.000,	13},
   {40,	1,	46,		 -9588.833,	  8863.833,		0.000,	0.015,	0.015,	0.000,	14},
   {41,	1,	72,		 -9588.833,	 13332.500,		0.000,	0.015,	0.015,	0.000,	15},
   {42,	1,	89,		-11711.500,	 -9010.833,		0.000,	0.015,	0.015,	0.000,	4},
   {43,	1,	163,		-11711.500,	 -4542.167,		0.000,	0.015,	0.015,	0.000,	5},
   {44,	1,	159,		-11711.500,	   -73.500,		0.000,	0.015,	0.015,	0.000,	6},
   {45,	1,	150,		-11711.500,	  4395.167,		0.000,	0.015,	0.015,	0.000,	7},
   {46,	1,	37,		-11711.500,	  8863.833,		0.000,	0.015,	0.015,	0.000,	8},
   {47,	1,	156,		-11711.500,	 13332.500,		0.000,	0.015,	0.015,	0.000,	9},
   {48,	1,	107,		-15956.833,	 -4542.167,		0.000,	0.015,	0.015,	0.000,	104},
   {51,	1,	110,		-15956.833,	  8863.833,		0.000,	0.015,	0.015,	0.000,	105},
   {52,	1,	111,		-13834.167,	 -9010.833,		0.000,	0.015,	0.015,	0.000,	106},
   {53,	1,	126,		-13834.167,	 -4542.167,		0.000,	0.015,	0.015,	0.000,	0},  
   {54,	1,	39,		-13834.167,	   -73.500,		0.000,	0.015,	0.015,	0.000,	1},  
   {55,	1,	100,		-13834.167,	  4395.167,		0.000,	0.015,	0.015,	0.000,	2},
   {56,	1,	123,		-13834.167,	  8863.833,		0.000,	0.015,	0.015,	0.000,	3},
   {57,	1,	106,		-13834.167,	 13332.500,		0.000,	0.015,	0.015,	0.000,	107},
//[BEES2]	# for AG chip
   {1,	2,	116,		-15956.833,	   -73.500,		0.000,	0.015,	0.015,	0.000,  112},
   {2,	2,	105,		-15956.833,	  4395.167,		0.000,	0.015,	0.015,	0.000,	113},
   {17,	2,	80,		-15956.833,	  -340.167,		0.000,	-0.015,	-0.015,	0.000,	115},
   {18,	2,	135,		-15956.833,	  4128.500,		0.000,	-0.015,	-0.015,	0.000,	114}
};


// HSC_Treeview
enum
{
  COLUMN_HSC_NUMBER,
  COLUMN_HSC_FILTER,
  COLUMN_HSC_DEF,
  COLUMN_HSC_DITH,
  COLUMN_HSC_OSRA,
  COLUMN_HSC_OSDEC,
  COLUMN_HSC_EXP,
  COLUMN_HSC_COLFG,
  COLUMN_HSC_COLBG,
  NUM_COLUMN_HSC
};

// HSCFIL_Treeview
enum
{
  COLUMN_HSCFIL_NUMBER,
  COLUMN_HSCFIL_NAME,
  COLUMN_HSCFIL_ID,
  COLUMN_HSCFIL_AGMAG,
  COLUMN_HSCFIL_AGEXP,
  COLUMN_HSCFIL_AGFLG,
  //COLUMN_HSCFIL_FLATW,
  //COLUMN_HSCFIL_FLATV,
  //COLUMN_HSCFIL_FLATA,
  COLUMN_HSCFIL_FLATEXP,
  COLUMN_HSCFIL_FLATFLG,
  COLUMN_HSCFIL_SENS,
  COLUMN_HSCFIL_MAG1E,
  NUM_COLUMN_HSCFIL
};

#ifdef USE_GTK3
static GdkRGBA col_hsc_setup [HSC_MAX_SET]
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
static GdkColor col_hsc_setup [HSC_MAX_SET]
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

#define HSC_FLAT_REPEAT 10

#define HSC_DEF_PA (-90)
#define HSC_DEF_OSRA 40
#define HSC_DEF_OSDEC 90
#define HSC_DEF_DITH_RA 60
#define HSC_DEF_DITH_DEC 60
#define HSC_DEF_NDITH 3
#define HSC_DEF_RDITH 120
#define HSC_DEF_TDITH 15
#define HSC_DEF_FOCUS_Z 3.45
#define HSC_DEF_DELTA_Z 0.40

#define HSC_SUNSET_OFFSET 35

#define HSC_TIME_ACQ 20
#define HSC_TIME_READOUT 35
#define HSC_TIME_FOCUS 360
#define HSC_TIME_FILTER 1800
#define HSC_TIME_FILTER_DAYTIME 600
#define HSC_TIME_FLAT_LAMP   900
#define HSC_TIME_FLAT_START 1800


// Proto-type
void HSC_TAB_create();
void HSCFIL_TAB_create();
void do_edit_hsc_setup();
void HSC_param_init();

void hsc_make_tree();
void hscfil_make_tree();
GtkTreeModel * hsc_create_items_model();
GtkTreeModel * hscfil_create_items_model();
void hsc_tree_update_item();
void hscfil_tree_update_item();
void hsc_add_columns();
void hscfil_add_columns();
void hsc_focus_item();
void hsc_activate_item();
void hsc_cell_data_func();
void hscfil_cell_data_func();
void hsc_swap_setup();

void hsc_dith_frame_set_sensitive();
void HSC_get_dith();
void HSC_add_setup();
void HSC_remove_setup();
void up_item_hsc_tree();
void down_item_hsc_tree();

gchar* hsc_make_txt();
gchar* hsc_make_def();
gchar* hsc_make_dtxt();

void HSC_WriteOPE();
void HSC_WriteOPE_obj();
void HSC_WriteOPE_OBJ_plan();
void HSC_WriteOPE_FOCUS_plan();
void HSC_WriteOPE_FLAT_plan();
void HSC_WriteOPE_SETUP_plan();

gint hsc_filter_get_from_id();

void hsc_do_export_def_list();

void HSC_Init_Filter();
void HSC_Read_Filter();
void hsc_sync_filter();
void hsc_fil_dl();
