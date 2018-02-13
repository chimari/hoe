#include"main.h"
#include<math.h>

#include"star_template.h"

// degree-to-radian conversion factor
gdouble degperrad = 180/M_PI;
// cross-disperser grating information
gdouble linespermm[] = {400 ,250};
// cross-disperser angle of incidence
gdouble cdinc = 22.5;
// these offsets include the blaze
gdouble offset[] = {0.06, 0.11};

// Here is the blue efficiency table, starting at order 112-194
#define O_ST_BLUE 112
#define O_ED_BLUE 194
#define O_NUM_BLUE (O_ED_BLUE-O_ST_BLUE+1)
gdouble effblue[] = { 0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       //  0
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 10
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 20
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 30
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 40
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 50
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 60
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 70
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 80
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 90
		      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       //100
		      0,       0,       0.085,   0.085,   0.085,   0.085,   0.085,   0.085,   0.085,   0.085,   //110
		      0.085,   0.085,   0.085,   0.085,   0.085,   0.087784,0.086704,0.087554,0.086582,0.087592,//120
		      0.087509,0.087882,0.087845,0.088858,0.088985,0.088620,0.089504,0.093069,0.091299,0.088672,//130
		      0.087771,0.086520,0.084479,0.083090,0.085234,0.088443,0.088331,0.084784,0.082451,0.083925,//140
		      0.081423,0.082069,0.085   ,0.088209,0.088370,0.085460,0.081906,0.077374,0.076884,0.075564,//150
		      0.075122,0.073752,0.072552,0.071219,0.070549,0.068661,0.066931,0.065203,0.064409,0.062922,//160
		      0.062202,0.060865,0.059569,0.058101,0.056936,0.055058,0.054503,0.052983,0.051858,0.050765,//170
		      0.048888,0.047976,0.044857,0.043673,0.042326,0.039296,0.037818,0.0355  ,0.033354,0.031353,//180
		      0.030791,0.029410,0.025   ,0.020   ,0.017,   0,       0,       0,       0,       0};      //190

// And ditto for the red, starting at order 59-167
#define O_ST_RED 59
#define O_ED_RED 167
#define O_NUM_RED (O_ED_RED-O_ST_RED+1)
gdouble effred[] ={ 0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       //  0
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 10
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 20
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 30
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 40
                    0,       0,       0,       0,       0,       0,       0,       0,       0,       0.0031,  // 50
                    0.0051,  0.0078,  0.0104,  0.0127,  0.017688,0.019432,0.021833,0.024501,0.027655,0.029959,// 60
                    0.031415,0.040452,0.044893,0.044226,0.045000,0.045842,0.051221,0.057022,0.062000,0.067162,// 70
		    0.070290,0.075556,0.085832,0.086150,0.085270,0.088988,0.097540,0.099889,0.099912,0.104433,// 80
		    0.107842,0.115514,0.116112,0.119078,0.121474,0.124810,0.122544,0.126695,0.123394,0.121893,// 90
		    0.121633,0.125974,0.123557,0.123824,0.125407,0.127096,0.127140,0.126522,0.126464,0.123   ,//100
		    0.120751,0.121257,0.123605,0.123947,0.122828,0.122889,0.120697,0.118756,0.117361,0.115087,//110
		    0.112064,0.113581,0.120014,0.119361,0.110412,0.106811,0.106197,0.104593,0.104305,0.103027,//120
		    0.101357,0.099824,0.096796,0.096700,0.091934,0.088648,0.086446,0.087390,0.086538,0.079018,//130
		    0.078778,0.077325,0.075719,0.073515,0.071880,0.072702,0.072   ,0.066   ,0.066,   0.067   ,//140
		    0.063   ,0.062   ,0.061   ,0.058,   0.057   ,0.055   ,0.052   ,0.049   ,0.048,   0.047   ,//150
		    0.043   ,0.041   ,0.039   ,0.037,   0.034   ,0.034   ,0.029   ,0.028   ,0,       0,       //160
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       //170
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       //180
		    0,       0,       0,       0,       0,       0,       0,       0,       0,       0};      //190

// The Blue ImRot efficiency, starting at order 59-194
// Measured in 9/22/2013
//  0      1      2      3      4      5      6     7       8      9
#define O_ST_IMRB 59
#define O_ED_IMRB 194
#define O_NUM_IMRB (O_ED_IMRB-O_ST_IMRB+1)
gdouble bimreff[] = {0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     //  0
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 10
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 20
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 30
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 40
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0.38,  // 50
		     0.38,  0.38,  0.38,  0.38,  0.38,  0.38,  0.38,  0.38,  0.3854,0.3878,// 60
		     0.3877,0.3944,0.4005,0.4176,0.4150,0.4137,0.4301,0.4410,0.4522,0.4640,// 70
		     0.4764,0.4860,0.4962,0.5008,0.5051,0.5102,0.5145,0.5228,0.5266,0.5309,// 80
		     0.5476,0.5495,9.5553,0.5571,0.5622,0.5601,0.5602,0.5609,0.5631,0.5616,// 90
		     0.5630,0.5619,0.5629,0.5647,0.5626,0.5617,0.5596,0.5563,0.5567,0.5487,//100
		     0.5487,0.5478,0.5465,0.5451,0.5428,0.5412,0.5386,0.5368,0.5313,0.5287,//110
		     0.5275,0.5232,0.5190,0.5165,0.5136,0.5081,0.5062,0.5024,0.5028,0.4980,//120
		     0.4933,0.4885,0.4816,0.4822,0.4759,0.4714,0.4671,0.4669,0.4630,0.4572,//130
		     0.4498,0.4495,0.4434,0.4425,0.4373,0.4090,0.4067,0.4024,0.3996,0.4004,//140
		     0.3952,0.3975,0.3946,0.3931,0.3944,0.3951,0.3950,0.3927,0.4,   0.4,   //150
		     0.4024,0.4044,0.4037,0.4043,0.4089,0.4100,0.4136,0.4184,0.4207,0.4283,//160
		     0.4423,0.4389,0.4384,0.4483,0.4512,0.4576,0.4562,0.4658,0.4605,0.4527,//170
		     0.4769,0.4780,0.4959,0.4968,0.5007,0.4953,0.4961,0.4929,0.4959,0.4885,//180
		     0.4934,0.5109,0.4888,0.5062,0.4991,0,     0,     0,     0,     0};    //190

// The Red ImRot efficiency, orders 59-194
// Measured in 9/22/2013
//    0      1      2      3      4      5      6      7      8      9
#define O_ST_IMRR 59
#define O_ED_IMRR 194
#define O_NUM_IMRR (O_ED_IMRR-O_ST_IMRR+1)
gdouble rimreff[] = {0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     //  0
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 10
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 20
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 30
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 40
		     0,     0,     0,     0,     0,     0,     0,     0,     0,     0.52,  // 50
		     0.52,  0.52,  0.52,  0.52,  0.52,  0.52,  0.52,  0.52,  0.5253,0.5163,// 60
		     0.5108,0.5099,0.5088,0.5066,0.51,  0.5121,0.5147,0.5218,0.5241,0.5364,// 70
		     0.5364,0.5440,0.5430,0.5450,0.5487,0.5536,0.5561,0.5546,0.5591,0.5582,// 80
		     0.5738,0.5723,0.5711,0.5723,0.5706,0.5638,0.5666,0.5649,0.5695,0.5664,// 90
		     0.5662,0.5646,0.5656,0.5650,0.5632,0.5623,0.5579,0.5553,0.5546,0.551, //100
		     0.5515,0.5488,0.5481,0.5459,0.5412,0.5384,0.5357,0.5284,0.5274,0.5214,//110
		     0.5181,0.5163,0.5119,0.5067,0.4979,0.4931,0.4860,0.4820,0.4722,0.4634,//120
		     0.4458,0.4383,0.4275,0.4181,0.4105,0.4002,0.3904,0.3811,0.3720,0.3634,//130
		     0.3526,0.3459,0.3354,0.3268,0.3231,0.3159,0.3063,0.3013,0.2919,0.2842,//140
		     0.2765,0.2704,0.2639,0.2595,0.2581,0.2579,0.2612,0.2799,0.28,  0.28,  //150
		     0.2912,0.3051,0.3141,0.3210,0.3246,0.3269,0.3262,0.3259,0.3199,0.3151,//160
		     0.3055,0.2963,0.2855,0.2727,0.2598,0.2431,0.2256,0.2085,0.1870,0.1685,//170
		     0.1475,0.1323,0.1083,0.0930,0.0738,0.0583,0.0436,0.0342,0.0261,0.0156,//180
		     0.0164,0.0131,0.0080,0.0119,0.0158};                                  //190

// The ADC efficiency, starting at order 59-194
#define O_ST_ADC 59
#define O_ED_ADC 194
#define O_NUM_ADC (O_ED_ADC-O_ST_ADC+1)
gdouble adceff[] = {0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     //  0
		    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 10
		    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 20
		    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 30
		    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 40
		    0,    0,    0,    0,    0,    0,    0,    0,    0,    0.925, // 50
		    0.925,0.925,0.925,0.925,0.925,0.922,0.867,0.931,0.925,0.931, // 60
		    0.919,0.932,0.919,0.967,0.955,0.948,0.967,0.957,0.950,0.949, // 70
		    0.965,0.977,0.956,0.951,0.952,0.958,0.961,0.946,0.912,0.913, // 80
		    0.918,0.905,0.919,0.920,0.912,0.914,0.927,0.908,0.919,0.928, // 90
		    0.923,0.920,0.919,0.927,0.925,0.928,0.923,0.928,0.933,0.935, //100
		    0.936,0.936,0.930,0.932,0.922,0.906,0.920,0.920,0.915,0.913, //110
		    0.906,0.908,0.904,0.902,0.892,0.912,0.924,0.918,0.916,0.900, //120
		    0.908,0.899,0.892,0.889,0.889,0.891,0.878,0.880,0.882,0.868, //130
		    0.881,0.883,0.870,0.869,0.874,0.876,0.871,0.877,0.880,0.865, //140
		    0.870,0.880,0.840,0.805,0.788,0.789,0.781,0.768,0.751,0.713, //150
		    0.719,0.704,0.678,0.659,0.629,0.594,0.557,0.522,0.475,0.435, //160
		    0.390,0.343,0.293,0.250,0.206,0.166,0.124,0.093,0.068,0.047, //170
		    0.033,0.020,0.013,0.008,0.000,0.000,0.000,0.000,0.000,0.000, //180
		    0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000};//190

// The Image Slicer 0".3x5 efficiency, starting at order 59-194
gdouble is35eff[] = {0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //  0
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 10
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 20
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 30
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 40
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0.345,// 50
		     0.346,0.347,0.348,0.356,0.357,0.365,0.390,0.407,0.430,0.456,// 60
		     0.475,0.480,0.506,0.555,0.560,0.610,0.642,0.667,0.686,0.692,// 70
		     0.710,0.715,0.735,0.750,0.770,0.771,0.772,0.773,0.774,0.775,// 80
		     0.776,0.778,0.779,0.780,0.780,0.781,0.783,0.786,0.789,0.792,// 90
		     0.797,0.800,0.805,0.808,0.810,0.815,0.821,0.820,0.820,0.820,//100
		     0.821,0.821,0.822,0.822,0.822,0.823,0.823,0.823,0.824,0.824,//110
		     0.824,0.824,0.824,0.824,0.824,0.825,0.825,0.825,0.825,0.825,//120
		     0.825,0.825,0.825,0.825,0.825,0.826,0.826,0.826,0.826,0.826,//130
		     0.826,0.826,0.826,0.826,0.826,0.827,0.827,0.827,0.827,0.827,//140
		     0.827,0.827,0.827,0.827,0.827,0.828,0.828,0.828,0.828,0.828,//150
		     0.828,0.828,0.828,0.828,0.828,0.828,0.829,0.828,0.822,0.821,//160
		     0.810,0.805,0.794,0.777,0.760,0.732,0.712,0.694,0.661,0.636,//170
		     0.620,0.592,0.570,0.551,0.532,0.510,0.490,0.470,0.452,0.444,//180
		     0.438,0.436,0.431,0.415,0.403,0.350,0,    0,    0,    0};	 //190

// The Image Slicer #2 0".45x3 efficiency, starting at order 59-194
gdouble is453eff[] = {0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //  0
		      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	  // 10
		      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	  // 20
		      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	  // 30
		      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	  // 40
		      0,    0,    0,    0,    0,    0,    0,    0,    0,    0.767,// 50
		      0.798,0.827,0.835,0.853,0.870,0.873,0.869,0.880,0.882,0.891,// 60
		      0.893,0.896,0.902,0.907,0.922,0.919,0.925,0.925,0.921,0.928,// 70
		      0.956,0.960,0.953,0.957,0.957,0.954,0.956,0.949,0.955,0.951,// 80
		      0.947,0.947,0.945,0.944,0.941,0.938,0.935,0.936,0.930,0.932,// 90
		      0.926,0.927,0.932,0.926,0.926,0.926,0.923,0.925,0.928,0.933,//100
		      0.925,0.927,0.932,0.927,0.929,0.932,0.932,0.923,0.925,0.926,//110
		      0.922,0.929,0.921,0.926,0.920,0.926,0.921,0.921,0.920,0.927,//120
		      0.919,0.922,0.917,0.916,0.916,0.916,0.913,0.910,0.913,0.915,//130
		      0.916,0.918,0.916,0.908,0.911,0.904,0.900,0.901,0.895,0.909,//140
		      0.907,0.910,0.912,0.893,0.905,0.911,0.904,0.910,0.908,0.916,//150
		      0.906,0.901,0.911,0.900,0.905,0.896,0.888,0.889,0.868,0.877,//160
		      0.878,0.873,0.870,0.865,0.845,0.850,0.838,0.764,0.738,0.719,//170
		      0.710,0.678,0.658,0.642,0.613,0.586,0.562,0.527,0.514,0.497,//180
		      0.478,0.461,0.447,0.436,0.424,0,    0,    0,    0,    0};	  //190

// The Image Slicer #3 0".20x3 efficiency, starting at order 59-194
gdouble is23eff[] = {0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //  0
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 10
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 20
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 30
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	 // 40
		     0,    0,    0,    0,    0,    0,    0,    0,    0,    0.613,// 50
		     0.656,0.665,0.664,0.685,0.681,0.689,0.704,0.718,0.676,0.714,// 60
		     0.682,0.688,0.727,0.709,0.696,0.714,0.716,0.718,0.725,0.720,// 70
		     0.723,0.720,0.721,0.722,0.721,0.726,0.722,0.720,0.722,0.718,// 80
		     0.718,0.725,0.717,0.707,0.730,0.731,0.730,0.732,0.730,0.730,// 90
		     0.729,0.728,0.727,0.726,0.723,0.727,0.726,0.722,0.725,0.722,//100
		     0.718,0.719,0.718,0.716,0.717,0.718,0.728,0.725,0.727,0.730,//110
		     0.730,0.723,0.728,0.729,0.726,0.733,0.732,0.729,0.730,0.730,//120
		     0.728,0.737,0.736,0.737,0.733,0.732,0.733,0.736,0.732,0.736,//130
		     0.734,0.733,0.729,0.727,0.723,0.721,0.716,0.701,0.709,0.674,//140
		     0.667,0.653,0.641,0.629,0.621,0.607,0.590,0.582,0.565,0.559,//150
		     0.544,0.529,0.519,0.505,0.501,0.483,0.477,0.465,0.454,0.449,//160
		     0.441,0.432,0.434,0.428,0.430,0.419,0.431,0.000,0.000,0.000,//170
		     0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,//180
		     0.000,0.000,0.000,0.000,0.000,0,    0,    0,    0,    0};	 //190


gdouble st_f(gint sptype, gint i_st){
 switch(sptype){
 case ST_O5V:
   return (st[i_st].o5v);
   break;
   
 case ST_O9V:
   return (st[i_st].o9v);
   break;

 case ST_B0V:
   return (st[i_st].b0v);
   break;

 case ST_B3III:
   return (st[i_st].b3iii);
   break;

 case ST_B3V:
   return (st[i_st].b3v);
   break;

 case ST_B8V:
   return (st[i_st].b8v);
   break;

 case ST_A0V:
   return (st[i_st].a0v);
   break;

 case ST_A5V:
   return (st[i_st].a5v);
   break;

 case ST_F0V:
   return (st[i_st].f0v);
   break;

 case ST_F5V:
   return (st[i_st].f5v);
   break;

 case ST_G0V:
   return (st[i_st].g0v);
   break;

 case ST_G5V:
   return (st[i_st].g5v);
   break;

 case ST_K0V:
   return (st[i_st].k0v);
   break;
 }
}

gdouble erf(gdouble ip) {
  gdouble phi;
  phi = (ip>2.85) ? 
    1.0 :
    -0.01450944+ip*(1.294778
		    +ip*(-0.4078223+ip*(-0.09605448+ip*(0.070197+ip*(-0.009418276)))));
  return(phi);
}

gdouble centwave(gdouble ip, gdouble echrot) {
  return ((596075+101112*tan(2*(echrot-0.425+0.15)/degperrad))/ip);
}

// calculate tilt angle of cross-disperser
Crosspara get_cross_angle(gint wcent, gdouble dcross){
  gdouble beta, tilt;
  gint grating;
  Crosspara cp;

  grating=(wcent>4400) ? COL_RED : COL_BLUE;

  beta = asin(0.5e-7*(gdouble)wcent*linespermm[grating]/cos(cdinc*M_PI/180));
  tilt = beta*degperrad+offset[grating]+dcross/3600.;
  cp.cross = (int)(tilt*3600.);
  cp.col = grating;

  return(cp);
}

void etc_main(typHOE *hg){
  // area of primary mirror
  gdouble m1area = 52.81;
  gint spek=hg->etc_spek;
  
  // x-dispersion = 1.7763/m A/pixel (m=order)
  gdouble xdisp = 1.7727445;
  // half the gap between the CCDs in pixels
  gint halfgap = 42;
  // saturation level (ADU)
  gdouble satlevel = 32000;
  gdouble gain = 1.7;
  // pixelsize in m
  gdouble pixsize = 13.5e-6;
  // focal length in m
  gdouble flength = 0.77;
  // number of CCD pixels in y direction
  gint nypix = 2048;
  // Cross Disperser Offset
  gdouble dcross = hg->d_cross;
  //gint m0[] = {1800, 4400, 3640, 3100, 2550};
  // magnitude zero points
  // See http://svo2.cab.inta-csic.es/svo/theory/fps/
  gdouble m0[] = {1564.2, 4005.0, 3562.5, 2814.9, 2282.8, 
		   1571.5, 1024.0,   653.4};
  gdouble w0[] = {3570.6, 4378.4, 5466.1, 6695.6, 8565.1,
		  12100.9,16620.0, 21420.3};
  gdouble m0_sdss[] = {1568.5, 3965.9, 3162.0, 2602.0, 2244.7};
  gdouble w0_sdss[] = {3594.9, 4640.4, 6122.3, 7439.5, 8897.1};
  gdouble m0_ps1[] = {3909.1, 3151.4, 2584.6, 2273.1, 2206.0};
  gdouble w0_ps1[] = {4775.6, 6129.5, 7484.6, 8657.8, 9603.1};
  gdouble m0_gaia = 2861.3;
  gdouble w0_gaia = 5857.6;
  gdouble m0_2mass[] = {1594.0,   1024.0,   666.8};
  gdouble w0_2mass[] = {12350.0, 16620.0, 21590.0};
  
  // The standard HDS setups
  //enum {SETUP_USER,SETUP_UA,SETUP_UB,SETUP_BA,SETUP_BC,SETUP_YA,SETUP_YD,SETUP_YB,SETUP_YC,SETUP_RA,
  //	SETUP_RB,SETUP_NIRA,SETUP_NIRB,SETUP_NIRC,SETUP_I2A,SETUP_I2B,SETUP_NUM};
  //static const char* setups[] = {"User", "Ua","Ub","Ba","Bc","Ya","Yd","Yb","Yc","Ra","Rb","NIRa","NIRb","NIRc","I2a","I2b"};
  //gdouble xangle[] = {0,17820,17100,19260,19890,21960,15480,15730,16500,18455,19080,25200,22860,21360,18000,14040};
  gdouble xangle;
  
  // Get the form input
  gint setup=hg->setup[hg->etc_setup].setup;
  gdouble wcent;
  gdouble slitwidth=(gdouble)hg->setup[hg->etc_setup].slit_width/500.;
  gint grating;
  gdouble seeing=hg->etc_seeing;
  gint filter;
  gdouble mag=hg->etc_mag;
  gdouble alpha=hg->etc_alpha;
  gint bbtemp=hg->etc_bbtemp;
  gint sptype = hg->etc_sptype;
  gint exptime = hg->etc_exptime;
  gdouble z = hg->etc_z;
  gint imrot=hg->etc_imr;
  gint slitunit=hg->setup[hg->etc_setup].is;
  gint adc=hg->etc_adc;
  enum{MESSIA_4,MESSIA_5};
  gint messia=MESSIA_5;
  enum{ BIN11, BIN21, BIN22, BIN24, BIN41, BIN44, MAX_BINNING} BinMode;
  gint binning=hg->setup[hg->etc_setup].binning;
  gint xbin[]={1,2,2,2,4,4};
  gint ybin[]={1,1,2,4,1,4};
  // Echelle roation angle (deg)
  gdouble echrot;
  // Detector rotation angle (deg)
  gdouble detrot;
  gint rnsq=81;
 
  enum{SAT_NONE,SAT_BAD,SAT_SAT};
  gint result;
  gdouble normflux;
  gdouble normwave;
  gint order0;
  gdouble beta;
  gdouble tilt;
  gdouble slit_trans;
  gint npix;
  gdouble isgain_slit_trans;
  gdouble res;
  gint npix_slit;
  gdouble sum_frac;
  gdouble peak_frac;
  gdouble kmsres;
  gint order;
  gdouble flux[200];
  gdouble hcoverkt;
  gdouble wave;
  gint ccdchange;
  gdouble t;
  gboolean saturation;
  gboolean badcolumn;
  gint deadt;
  gdouble wc, wmin, wmax;
  gdouble yc, ymin, ymax;
  gdouble counts, counts_slit;
  gint ccd;
  gdouble ordereff;
  gdouble sky;
  gdouble isgain;
  gdouble trans;
  gint peak;
  gdouble snr, snr_slit, snr_gain;
  gdouble yy1,yy2;
  gdouble ybad0,ybad1,ybad2,ybad3,ybad4,ybad5;
  // pixel scale (arcsec/pixel)
  gdouble pixscale= 0.138 * (gdouble)xbin[binning];
  // Dark Changed, based on the measurement in 2010/05 
  // 1.8e-/pix/30min
  gdouble dark = 0.001 * (gdouble)xbin[binning] * (gdouble)ybin[binning];
  gint i_st;
  gdouble w, f, fluxatnorm, oldw,oldf;
  gint i_etc;
  gboolean isgap;
  gchar *prof_str, *prof_tmp;

  if(setup<0){
    echrot=(gdouble)hg->nonstd[hg->etc_setup].echelle/3600.;
    detrot=(gdouble)hg->nonstd[hg->etc_setup].camr/3600.;
    grating=hg->nonstd[hg->etc_setup].col;
    xangle=(gdouble)hg->nonstd[hg->etc_setup].cross;
  }
  else{
    echrot=(gdouble)DEF_ECHELLE/3600.;
    detrot=-1.0;
    grating = (setup<=StdYa) ? COL_BLUE : COL_RED;
    xangle=(gdouble)setups[hg->setup[hg->etc_setup].setup].cross_scan;
  }

  if(hg->etc_mode==ETC_OBJTREE){
    switch(hg->obj[hg->etc_i].magdb_used){
    case MAGDB_TYPE_SIMBAD:
      filter=hg->obj[hg->etc_i].magdb_band-1;
      normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0[filter];
      break;
      
    case MAGDB_TYPE_GSC:
      filter=hg->obj[hg->etc_i].magdb_band;
      normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0[filter];
      break;
      
    case MAGDB_TYPE_PS1:
      filter=hg->obj[hg->etc_i].magdb_band;
      normflux = m0_ps1[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0_ps1[filter];
      break;
      
    case MAGDB_TYPE_SDSS:
      filter=hg->obj[hg->etc_i].magdb_band;
      normflux = m0_sdss[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0_sdss[filter];
      break;
      
    case MAGDB_TYPE_GAIA:
      normflux = m0_gaia*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0_gaia;
      break;
      
    case MAGDB_TYPE_2MASS:
      filter=hg->obj[hg->etc_i].magdb_band;
      normflux = m0_2mass[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0_2mass[filter];
      break;

    default:
      filter=hg->etc_filter;
      normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[hg->etc_i].mag);
      normwave = w0[filter];
      break;
    }
  }
  else{
    filter=hg->etc_filter;
    normflux = m0[filter]*1e6 / pow(10,0.4*mag);
    normwave = w0[filter];
  }
  wcent = cos(cdinc*M_PI/180.0)*sin((xangle/3600.0-offset[grating])*M_PI/180.0)/(0.5e-7*linespermm[grating]);
  //($order0,@eff) = ($grating eq "red") ? (59,@effred) : (112,@effblue);
  order0 = (grating==COL_RED) ? O_ST_RED : O_ST_BLUE;

  // calculate tilt angle of cross-disperser
  beta = asin(0.5e-7*wcent*linespermm[grating]/cos(cdinc*M_PI/180));
  tilt = beta*degperrad+offset[grating]+dcross/3600.;
  
  prof_str=g_strdup_printf("Using the %s cross-disperser with a tilt angle of %5.3f degrees.\n", 
			   (grating==COL_RED) ? "Red" : "Blue",
			   tilt);
  //beta = cdinc/degperrad-beta;
  //beta = -(cdinc-tilt)/degperrad;
  
  // slit stuff and number of pixels to sum for point source
  switch(slitunit){
  case IS_030X5:
    slit_trans = 1 - exp(-0.832555*0.83255*1.5*1.5/seeing/seeing);
    npix = (int)(7.90/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.30000/seeing);
    res = 110000;
    npix_slit = (int)(2*seeing/pixscale) +1;
    break;
    
  case IS_045X3:
    slit_trans = erf(0.832555*1.36/seeing)*erf(0.832555*1.36/seeing);
    npix = (int)(5.00/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.45000/seeing);
    res = 85000;
    npix_slit = (int) (2*seeing/pixscale) +1;
    break;
    
  case IS_020X3:
    slit_trans = erf(0.832555*1.46/seeing)*erf(0.832555*0.60/seeing);
    npix = (int)(5.50/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.20000/seeing);
    res = 165000;
    npix_slit = (int)(2*seeing/pixscale) +1;
    break;
    
  default:
    slit_trans = erf(0.832555*slitwidth/seeing);
    npix = (int)(2*seeing/pixscale) +1;
    res = (slitwidth>0.225) ? 36000/slitwidth : 160000;
    break;
  }
  sum_frac = erf(0.832555*(gdouble)npix*pixscale/seeing);
  peak_frac = erf(0.832555*pixscale/seeing);
  
  kmsres = 300000/res;
  
  switch(binning){
  case BIN11:
    deadt=86;
      break;
      
  case BIN21:
    deadt=60;
    break;
    
  case BIN22:
    deadt=44;
    break;
    
  case BIN24:
    deadt=36;
    break;
    
  case BIN41:
    deadt=44;
    break;
    
  case BIN44:
    deadt=33;
    break;
  }

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  switch(slitunit){
  case IS_030X5:
    prof_str=g_strdup_printf("%sThe image slicer #1 (phi=1.5\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  case IS_045X3:
    prof_str=g_strdup_printf("%sThe image slicer #2 (1.35\"x1.35\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  case IS_020X3:
    prof_str=g_strdup_printf("%sThe image slicer #3 (0.60\"x1.46\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  default:
    prof_str=g_strdup_printf("%sThe %.2lf\" slit passes %.1lf%% of the light in %.2lf\" seeing.\n", 
			     prof_tmp,
			     slitwidth, 100*slit_trans, seeing);
    break;
  }
  g_free(prof_tmp);
  
  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sThe spectral resolution is %.0lf (%.2lf km/s).\n",
			   prof_tmp,
			   res, kmsres);
  g_free(prof_tmp);

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sUsing %dx%d binning mode, the total dead time for CCD readout is about %d seconds.\n", 
			   prof_tmp,
			   xbin[binning], ybin[binning], deadt);
  g_free(prof_tmp);

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sBinning over %d pixels includes %4.1f%% of the light.\n", 
			   prof_tmp,
			   npix, 100*sum_frac);
  g_free(prof_tmp);

  t=(gdouble)exptime;
  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sDark count is %.1fe- in total over %d binned pixels.\n\n", 
			   prof_tmp,
			   t*dark*(gdouble)npix, npix);
  g_free(prof_tmp);
  
  if (adc!=ETC_ADC_IN) {
    if(imrot!=ETC_IMR_NO){
      prof_tmp=g_strdup(prof_str);
      g_free(prof_str);
      prof_str=g_strdup_printf("%sYou are not using the ADC -- you may lose flux if you do not set the slit to the parallactic angle.\n",
			       prof_tmp);
      g_free(prof_tmp);
    }
    else{
      prof_tmp=g_strdup(prof_str);
      g_free(prof_str);
      prof_str=g_strdup_printf("%sYou are using neither the ADC nor the Image Rotator -- you will only be able to observe point sources near the zenith.\n",
			       prof_tmp);
      g_free(prof_tmp);
    }
  }
  else if (imrot==ETC_IMR_NO){
    prof_tmp=g_strdup(prof_str);
    g_free(prof_str);
    prof_str=g_strdup_printf("%sYou are not using the Image Rotator -- you will not be able to set the slit position angle.\n",
			     prof_tmp);
    g_free(prof_tmp);
  }
  
  
  // first, compute the input flux spectrum
  switch(spek){
  case ETC_SPEC_POWERLAW:
    for(order=1;order<201;order++){
      flux[order] = normflux * pow((centwave(order, echrot)/normwave),alpha);
    }
    break;
    
  case ETC_SPEC_BLACKBODY:
    hcoverkt = 144043478*(z+1)/(gdouble)bbtemp;
    for (order=1;order<201;order++) {
      wave = centwave(order, echrot);
      flux[order]=normflux * pow(normwave/wave,3) * (exp(hcoverkt/normwave) -1) / (exp(hcoverkt/wave) -1);
    }
    break;
    
  case ETC_SPEC_TEMPLATE:
    i_st=0;
    f=st_f(sptype,i_st);
    w=st[i_st].w;
    fluxatnorm = f;
    for (order=200; order>49; order--) {
      wave = centwave(order, echrot)/(z+1);
      while (wave < w) {
	flux[order--] = 0;
	wave = centwave(order, echrot)/(z+1);
	if(order==0) break;
      }
      while (w <wave) {
	oldw=w; 
	oldf=f;
	i_st++;
	f=st_f(sptype,i_st);
	w=st[i_st].w;
	f *= w*w;
	if (w<normwave/(z+1) && w>0) {
	  fluxatnorm = f;
	}
      }
      flux[order] = w ? ((w-wave)*oldf+(wave-oldw)*f)/(w-oldw) : 0;
    }
    
    for (order=200; order>49; order--) {
      flux[order] *= normflux/fluxatnorm;
    }
    break;
  }
  
  order=200;
  ccdchange=0;
  i_etc=0;
  
  while (order>50) {
    { // orderinfo
      wc = centwave(order, echrot);
      wmin = wc-2048*xdisp/(gdouble)order;
      wmax = wc+2048*xdisp/(gdouble)order;
      // The 40 and 60 account for spectral curvature in the system
      yc = 1e-7*flength*(wc-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+20;
      ymin = 1e-7*flength*(wmin-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+20+2048*sin((detrot-0.1)*M_PI/180);
      ymax = 1e-7*flength*(wmax-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+40-2048*sin((detrot-0.1)*M_PI/180);
      ccd = (yc<0) ? 1 : 2;
      yc = (yc<0) ? 
	(-yc-(gdouble)halfgap) : 
	((gdouble)nypix+(gdouble)halfgap+1-yc);
      ymin = (ccd==1) ? 
	(-ymin-(gdouble)halfgap) :
	(gdouble)nypix+(gdouble)halfgap+1-ymin;
      ymax = (ccd==1) ? 
	(-ymax-(gdouble)halfgap) : 
	(gdouble)nypix+(gdouble)halfgap+1-ymax;
    }
    
    if ((ccd==2) && (!ccdchange)) {
      ccdchange = 1;
    }
    
    // is this order completely on a CCD?
    if ((ymin>0) && (ymax>0) && (ymin<=(gdouble)nypix) && (ymax<=(gdouble)nypix)) {
      {
	if(grating==COL_RED){
	  ordereff = effred[order-1];
	}
	else{
	  ordereff = effblue[order-1];
	}
	// sky model: 20(lambda/1um)^2 uJy/arcsec^2
	sky = 3e-6*m1area*xdisp*(gdouble)(ybin[binning])
	  /(gdouble)order*slitwidth*pixscale*wc*ordereff;
	// Flux of object is in uJy
	counts = flux[order]*m1area*ordereff*xdisp*(gdouble)(ybin[binning])
	  /((gdouble)order*wc*0.06626)*slit_trans*sum_frac;
	// Make corrections depending on whether ADC and/or ImRot are in
	
	if (adc==ETC_ADC_IN) {
	  trans = adceff[order-1];
	  sky *= trans; 
	  counts *= trans;
	}
	
	switch(imrot){
	case ETC_IMR_BLUE:
	  trans = bimreff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  break;
	  
	case ETC_IMR_RED:
	  trans = rimreff[order-1];
	  sky *= trans;
	  counts *= trans;
	  break;
	}
	
	switch(slitunit){
	case IS_030X5:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is35eff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating == COL_RED){
	    if(order > 120){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  else{
	    if(order > 148){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;
	  
	case IS_045X3:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is453eff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating == COL_RED){
	    if(order > 150){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;
	  
	case IS_020X3:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is23eff[order-1];
	  sky *= trans;
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating ==COL_RED){
	    if(order > 150){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;

	default:
	  isgap=FALSE;
	}
	
	// $sky and $dark have accounted for binning
	//  Messia4/5
	if (messia == MESSIA_5) {
	  rnsq=4.4*4.4;
	  satlevel=50000;
	}
	
	snr = counts*t/sqrt(counts*t+((sky+dark)*t+rnsq)*(gdouble)npix);
	
	switch (slitunit) {
	case IS_030X5:
	  trans = is35eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	case IS_045X3:
	  trans = is453eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	case IS_020X3:
	  trans = is23eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	default:
	  peak = (int)(counts*peak_frac/sum_frac*t/gain);
	  break;
	}
	
	// Bad column
	if ( ymax < ymin ){
	  yy1 = ymax;
	  yy2 = ymin;
	}
	else{
	  yy1 = ymin;
	  yy2 = ymax;
	}
	
	if (ccd==2) {//Red CCD
	  ybad0=(4100.0-2864.0)/4100.0*(yy2-yy1)+yy1;
	  if ( (yy1<1207) && (yy2>1207) ){
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<2048-1207) && (ybad0>2048-1207) ){
	    badcolumn = TRUE;
	  }
	  else{
	    badcolumn = FALSE;
	  }
	  
	  if (peak>satlevel){
	    saturation = TRUE;
	  }
	  else{
	    saturation = FALSE;
	  }
	}
	else{// Blue CCD
	  ybad1=(4100.0-2696.0)/4100.0*(yy2-yy1)+yy1;
	  ybad2=(4100.0-2675.0)/4100.0*(yy2-yy1)+yy1;
	  ybad3=(4100.0-2243.0)/4100.0*(yy2-yy1)+yy1;
	  ybad4=(4100.0- 500.0)/4100.0*(yy2-yy1)+yy1;
	  ybad5=(4100.0- 300.0)/4100.0*(yy2-yy1)+yy1;
	  if ( (yy1<76)  && (ybad1>76) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<128) && (ybad2>128) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<361) && (ybad3>361) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<1721) && (ybad4>1721) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<1745) && (ybad5>1745) ) {
	    badcolumn = TRUE;
	  }
	  else{
	    badcolumn = FALSE;
	  }
	  
	  if (peak>satlevel){
	    saturation = TRUE;
	  }
	  else{
	    saturation = FALSE;
	  }
	}
	
	hg->etc[i_etc].order=order;
	hg->etc[i_etc].pix_s=(int)(ymin+0.499);
	hg->etc[i_etc].pix_c=(int)(yc+0.499);
	hg->etc[i_etc].pix_e=(int)(ymax+0.499);
	hg->etc[i_etc].w_s=wmin;
	hg->etc[i_etc].w_c=wc;
	hg->etc[i_etc].w_e=wmax;
	hg->etc[i_etc].disp=xdisp*(ybin[binning])/(gdouble)order;
	hg->etc[i_etc].flux=flux[order];
	hg->etc[i_etc].peak=peak;
	hg->etc[i_etc].snr=snr;
	hg->etc[i_etc].sat=saturation;
	hg->etc[i_etc].bad=badcolumn;
	hg->etc[i_etc].ccd=ccd;
	hg->etc[i_etc].isgap=isgap;
	
	if(slitunit==IS_NO){
	  hg->etc[i_etc].isgain=-1;
	  hg->etc[i_etc].snr_gain=-1;
	}
	else{
	  hg->etc[i_etc].isgain=isgain;
	  hg->etc[i_etc].snr_gain=snr_gain;
	}
	
	i_etc++;
      }
    }
    order--;
  }


  hg->etc_i_max=i_etc;
  if(hg->etc_prof_text) g_free(hg->etc_prof_text);
  hg->etc_prof_text=g_strdup(prof_str);
  g_free(prof_str);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->etc_label,hg->etc_prof_text);
#else
  fprintf(stderr, "%s",hg->etc_prof_text);
#endif
  
  rebuild_etc_tree(hg);
}


gdouble etc_obj(typHOE *hg, gint i_list){
  // area of primary mirror
  gdouble m1area = 52.81;
  gint spek=hg->etc_spek;
  
  // x-dispersion = 1.7763/m A/pixel (m=order)
  gdouble xdisp = 1.7727445;
  // half the gap between the CCDs in pixels
  gint halfgap = 42;
  // saturation level (ADU)
  gdouble satlevel = 32000;
  gdouble gain = 1.7;
  // pixelsize in m
  gdouble pixsize = 13.5e-6;
  // focal length in m
  gdouble flength = 0.77;
  // number of CCD pixels in y direction
  gint nypix = 2048;
  // Cross Disperser Offset
  gdouble dcross = hg->d_cross;
  //gint m0[] = {1800, 4400, 3640, 3100, 2550};
  // magnitude zero points
  // See http://svo2.cab.inta-csic.es/svo/theory/fps/
  gdouble m0[] = {1564.2, 4005.0, 3562.5, 2814.9, 2282.8, 
		   1571.5, 1024.0,   653.4};
  gdouble w0[] = {3570.6, 4378.4, 5466.1, 6695.6, 8565.1,
		  12100.9,16620.0, 21420.3};
  gdouble m0_sdss[] = {1568.5, 3965.9, 3162.0, 2602.0, 2244.7};
  gdouble w0_sdss[] = {3594.9, 4640.4, 6122.3, 7439.5, 8897.1};
  gdouble m0_ps1[] = {3909.1, 3151.4, 2584.6, 2273.1, 2206.0};
  gdouble w0_ps1[] = {4775.6, 6129.5, 7484.6, 8657.8, 9603.1};
  gdouble m0_gaia = 2861.3;
  gdouble w0_gaia = 5857.6;
  gdouble m0_2mass[] = {1594.0,   1024.0,   666.8};
  gdouble w0_2mass[] = {12350.0, 16620.0, 21590.0};
  
  gdouble xangle;
  
  // Get the form input
  gint setup=hg->setup[hg->etc_setup].setup;
  gdouble wcent;
  gdouble slitwidth=(gdouble)hg->setup[hg->etc_setup].slit_width/500.;
  gint grating;
  gdouble seeing=hg->etc_seeing;
  gint filter;
  gdouble mag=hg->etc_mag;
  gdouble alpha=hg->etc_alpha;
  gint bbtemp=hg->etc_bbtemp;
  gint sptype = hg->etc_sptype;
  gint exptime = hg->obj[i_list].exp;
  gdouble z = hg->etc_z;
  gint imrot=hg->etc_imr;
  gint slitunit=hg->setup[hg->etc_setup].is;
  gint adc=hg->etc_adc;
  enum{MESSIA_4,MESSIA_5};
  gint messia=MESSIA_5;
  enum{ BIN11, BIN21, BIN22, BIN24, BIN41, BIN44, MAX_BINNING} BinMode;
  gint binning=hg->setup[hg->etc_setup].binning;
  gint xbin[]={1,2,2,2,4,4};
  gint ybin[]={1,1,2,4,1,4};
  // Echelle roation angle (deg)
  gdouble echrot;
  // Detector rotation angle (deg)
  gdouble detrot;
  gint rnsq=81;
 
  gint result;
  gdouble normflux;
  gdouble normwave;
  gint order0;
  gdouble beta;
  gdouble tilt;
  gdouble slit_trans;
  gint npix;
  gdouble isgain_slit_trans;
  gdouble res;
  gint npix_slit;
  gdouble sum_frac;
  gdouble peak_frac;
  gdouble kmsres;
  gint order;
  gdouble flux[200];
  gdouble hcoverkt;
  gdouble wave;
  gint ccdchange;
  gdouble t;
  gboolean saturation;
  gboolean badcolumn;
  gint deadt;
  gdouble wc, wmin, wmax;
  gdouble yc, ymin, ymax;
  gdouble counts, counts_slit;
  gint ccd;
  gdouble ordereff;
  gdouble sky;
  gdouble isgain;
  gdouble trans;
  gint peak;
  gdouble snr, snr_slit, snr_gain;
  gdouble yy1,yy2;
  gdouble ybad0,ybad1,ybad2,ybad3,ybad4,ybad5;
  // pixel scale (arcsec/pixel)
  gdouble pixscale= 0.138 * (gdouble)xbin[binning];
  // Dark Changed, based on the measurement in 2010/05 
  // 1.8e-/pix/30min
  gdouble dark = 0.001 * (gdouble)xbin[binning] * (gdouble)ybin[binning];
  gint i_st;
  gdouble w, f, fluxatnorm, oldw,oldf;
  gint i_etc;
  gboolean isgap;
  gchar *prof_str, *prof_tmp;
  gboolean snr_measured=FALSE;
  gdouble ret;

  hg->obj[i_list].sat=FALSE;
  if(fabs(hg->obj[i_list].mag)>99) return(-1);

  if(setup<0){
    echrot=(gdouble)hg->nonstd[hg->etc_setup].echelle/3600.;
    detrot=(gdouble)hg->nonstd[hg->etc_setup].camr/3600.;
    grating=hg->nonstd[hg->etc_setup].col;
    xangle=(gdouble)hg->nonstd[hg->etc_setup].cross;
  }
  else{
    echrot=(gdouble)DEF_ECHELLE/3600.;
    detrot=-1.0;
    grating = (setup<=StdYa) ? COL_BLUE : COL_RED;
    xangle=(gdouble)setups[hg->setup[hg->etc_setup].setup].cross_scan;
  }

  switch(hg->obj[i_list].magdb_used){
  case MAGDB_TYPE_SIMBAD:
    filter=hg->obj[i_list].magdb_band-1;
    normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0[filter];
    break;

  case MAGDB_TYPE_GSC:
    filter=hg->obj[i_list].magdb_band;
    normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0[filter];
    break;

  case MAGDB_TYPE_PS1:
    filter=hg->obj[i_list].magdb_band;
    normflux = m0_ps1[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0_ps1[filter];
    break;

  case MAGDB_TYPE_SDSS:
    filter=hg->obj[i_list].magdb_band;
    normflux = m0_sdss[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0_sdss[filter];
    break;

  case MAGDB_TYPE_GAIA:
    normflux = m0_gaia*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0_gaia;
    break;

  case MAGDB_TYPE_2MASS:
    filter=hg->obj[i_list].magdb_band;
    normflux = m0_2mass[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0_2mass[filter];
    break;

  default:
    filter=hg->etc_filter;
    normflux = m0[filter]*1e6 / pow(10,0.4*hg->obj[i_list].mag);
    normwave = w0[filter];
    break;
  }
  
  wcent = cos(cdinc*M_PI/180.0)*sin((xangle/3600.0-offset[grating])*M_PI/180.0)/(0.5e-7*linespermm[grating]);

  // calculate tilt angle of cross-disperser
  beta = asin(0.5e-7*wcent*linespermm[grating]/cos(cdinc*M_PI/180));
  tilt = beta*degperrad+offset[grating]+dcross/3600.;
  
  prof_str=g_strdup_printf("Using the %s cross-disperser with a tilt angle of %5.3f degrees.\n", 
			   (grating==COL_RED) ? "Red" : "Blue",
			   tilt);
  
  // slit stuff and number of pixels to sum for point source
  switch(slitunit){
  case IS_030X5:
    slit_trans = 1 - exp(-0.832555*0.83255*1.5*1.5/seeing/seeing);
    npix = (int)(7.90/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.30000/seeing);
    res = 110000;
    npix_slit = (int)(2*seeing/pixscale) +1;
    break;
    
  case IS_045X3:
    slit_trans = erf(0.832555*1.36/seeing)*erf(0.832555*1.36/seeing);
    npix = (int)(5.00/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.45000/seeing);
    res = 85000;
    npix_slit = (int) (2*seeing/pixscale) +1;
    break;
    
  case IS_020X3:
    slit_trans = erf(0.832555*1.46/seeing)*erf(0.832555*0.60/seeing);
    npix = (int)(5.50/pixscale) +1;
    isgain_slit_trans = slit_trans / erf(0.832555*0.20000/seeing);
    res = 165000;
    npix_slit = (int)(2*seeing/pixscale) +1;
    break;
    
  default:
    slit_trans = erf(0.832555*slitwidth/seeing);
    npix = (int)(2*seeing/pixscale) +1;
    res = (slitwidth>0.225) ? 36000/slitwidth : 160000;
    break;
  }
  sum_frac = erf(0.832555*(gdouble)npix*pixscale/seeing);
  peak_frac = erf(0.832555*pixscale/seeing);
  
  kmsres = 300000/res;
  
  switch(binning){
  case BIN11:
    deadt=86;
      break;
      
  case BIN21:
    deadt=60;
    break;
    
  case BIN22:
    deadt=44;
    break;
    
  case BIN24:
    deadt=36;
    break;
    
  case BIN41:
    deadt=44;
    break;
    
  case BIN44:
    deadt=33;
    break;
  }

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  switch(slitunit){
  case IS_030X5:
    prof_str=g_strdup_printf("%sThe image slicer #1 (phi=1.5\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  case IS_045X3:
    prof_str=g_strdup_printf("%sThe image slicer #2 (1.35\"x1.35\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  case IS_020X3:
    prof_str=g_strdup_printf("%sThe image slicer #3 (0.60\"x1.46\") catches %.1lf%% of the light in %.2lf\" seeing.\n",
			     prof_tmp,
			     100*slit_trans, seeing);
    break;
    
  default:
    prof_str=g_strdup_printf("%sThe %.2lf\" slit passes %.1lf%% of the light in %.2lf\" seeing.\n", 
			     prof_tmp,
			     slitwidth, 100*slit_trans, seeing);
    break;
  }
  g_free(prof_tmp);
  
  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sThe spectral resolution is %.0lf (%.2lf km/s).\n",
			   prof_tmp,
			   res, kmsres);
  g_free(prof_tmp);

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sUsing %dx%d binning mode, the total dead time for CCD readout is about %d seconds.\n", 
			   prof_tmp,
			   xbin[binning], ybin[binning], deadt);
  g_free(prof_tmp);

  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sBinning over %d pixels includes %4.1f%% of the light.\n", 
			   prof_tmp,
			   npix, 100*sum_frac);
  g_free(prof_tmp);

  t=(gdouble)exptime;
  prof_tmp=g_strdup(prof_str);
  g_free(prof_str);
  prof_str=g_strdup_printf("%sDark count is %.1fe- in total over %d binned pixels.\n\n", 
			   prof_tmp,
			   t*dark*(gdouble)npix, npix);
  g_free(prof_tmp);
  
  if (adc!=ETC_ADC_IN) {
    if(imrot!=ETC_IMR_NO){
      prof_tmp=g_strdup(prof_str);
      g_free(prof_str);
      prof_str=g_strdup_printf("%sYou are not using the ADC -- you may lose flux if you do not set the slit to the parallactic angle.\n",
			       prof_tmp);
      g_free(prof_tmp);
    }
    else{
      prof_tmp=g_strdup(prof_str);
      g_free(prof_str);
      prof_str=g_strdup_printf("%sYou are using neither the ADC nor the Image Rotator -- you will only be able to observe point sources near the zenith.\n",
			       prof_tmp);
      g_free(prof_tmp);
    }
  }
  else if (imrot==ETC_IMR_NO){
    prof_tmp=g_strdup(prof_str);
    g_free(prof_str);
    prof_str=g_strdup_printf("%sYou are not using the Image Rotator -- you will not be able to set the slit position angle.\n",
			     prof_tmp);
    g_free(prof_tmp);
  }
  
  
  // first, compute the input flux spectrum
  switch(spek){
  case ETC_SPEC_POWERLAW:
    for(order=1;order<201;order++){
      flux[order] = normflux * pow((centwave(order, echrot)/normwave),alpha);
    }
    break;
    
  case ETC_SPEC_BLACKBODY:
    hcoverkt = 144043478*(z+1)/(gdouble)bbtemp;
    for (order=1;order<201;order++) {
      wave = centwave(order, echrot);
      flux[order]=normflux * pow(normwave/wave,3) * (exp(hcoverkt/normwave) -1) / (exp(hcoverkt/wave) -1);
    }
    break;
    
  case ETC_SPEC_TEMPLATE:
    i_st=0;
    f=st_f(sptype,i_st);
    w=st[i_st].w;
    fluxatnorm = f;
    for (order=200; order>49; order--) {
      wave = centwave(order, echrot)/(z+1);
      while (wave < w) {
	flux[order--] = 0;
	wave = centwave(order, echrot)/(z+1);
	if(order==0) break;
      }
      while (w <wave) {
	oldw=w; 
	oldf=f;
	i_st++;
	f=st_f(sptype,i_st);
	w=st[i_st].w;
	f *= w*w;
	if (w<normwave/(z+1) && w>0) {
	  fluxatnorm = f;
	}
      }
      flux[order] = w ? ((w-wave)*oldf+(wave-oldw)*f)/(w-oldw) : 0;
    }
    
    for (order=200; order>49; order--) {
      flux[order] *= normflux/fluxatnorm;
    }
    break;
  }
  
  order=200;
  ccdchange=0;
  i_etc=0;
  
  while (order>50) {
    { // orderinfo
      wc = centwave(order, echrot);
      wmin = wc-2048*xdisp/(gdouble)order;
      wmax = wc+2048*xdisp/(gdouble)order;
      // The 40 and 60 account for spectral curvature in the system
      yc = 1e-7*flength*(wc-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+20;
      ymin = 1e-7*flength*(wmin-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+20+2048*sin((detrot-0.1)*M_PI/180);
      ymax = 1e-7*flength*(wmax-wcent)*linespermm[grating]/(cos((tilt-cdinc)*M_PI/180)*pixsize)+40-2048*sin((detrot-0.1)*M_PI/180);
      ccd = (yc<0) ? 1 : 2;
      yc = (yc<0) ? 
	(-yc-(gdouble)halfgap) : 
	((gdouble)nypix+(gdouble)halfgap+1-yc);
      ymin = (ccd==1) ? 
	(-ymin-(gdouble)halfgap) :
	(gdouble)nypix+(gdouble)halfgap+1-ymin;
      ymax = (ccd==1) ? 
	(-ymax-(gdouble)halfgap) : 
	(gdouble)nypix+(gdouble)halfgap+1-ymax;
    }
    
    if ((ccd==2) && (!ccdchange)) {
      ccdchange = 1;
    }
    
    // is this order completely on a CCD?
    if ((ymin>0) && (ymax>0) && (ymin<=(gdouble)nypix) && (ymax<=(gdouble)nypix)) {
      {
	if(grating==COL_RED){
	  ordereff = effred[order-1];
	}
	else{
	  ordereff = effblue[order-1];
	}
	// sky model: 20(lambda/1um)^2 uJy/arcsec^2
	sky = 3e-6*m1area*xdisp*(gdouble)(ybin[binning])
	  /(gdouble)order*slitwidth*pixscale*wc*ordereff;
	// Flux of object is in uJy
	counts = flux[order]*m1area*ordereff*xdisp*(gdouble)(ybin[binning])
	  /((gdouble)order*wc*0.06626)*slit_trans*sum_frac;
	// Make corrections depending on whether ADC and/or ImRot are in
	
	if (adc==ETC_ADC_IN) {
	  trans = adceff[order-1];
	  sky *= trans; 
	  counts *= trans;
	}
	
	switch(imrot){
	case ETC_IMR_BLUE:
	  trans = bimreff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  break;
	  
	case ETC_IMR_RED:
	  trans = rimreff[order-1];
	  sky *= trans;
	  counts *= trans;
	  break;
	}
	
	switch(slitunit){
	case IS_030X5:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is35eff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating == COL_RED){
	    if(order > 120){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  else{
	    if(order > 148){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;
	  
	case IS_045X3:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is453eff[order-1];
	  sky *= trans; 
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating == COL_RED){
	    if(order > 150){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;
	  
	case IS_020X3:
	  counts_slit = counts / isgain_slit_trans;
	  trans = is23eff[order-1];
	  sky *= trans;
	  counts *= trans;
	  isgain = isgain_slit_trans * trans;
	  if(grating ==COL_RED){
	    if(order > 150){
	      isgap=TRUE;
	    }   
	    else{
	      isgap=FALSE;
	    }
	  }
	  break;

	default:
	  isgap=FALSE;
	}
	
	// $sky and $dark have accounted for binning
	//  Messia4/5
	if (messia == MESSIA_5) {
	  rnsq=4.4*4.4;
	  satlevel=50000;
	}
	
	snr = counts*t/sqrt(counts*t+((sky+dark)*t+rnsq)*(gdouble)npix);
	
	switch (slitunit) {
	case IS_030X5:
	  trans = is35eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	case IS_045X3:
	  trans = is453eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	case IS_020X3:
	  trans = is23eff[order-1];
	  peak = (int)(counts*peak_frac/sum_frac*t/gain*trans/3.0);
	  snr_slit = counts_slit*t/sqrt(counts_slit*t+((sky+dark)*t+rnsq)*npix_slit);
	  snr_gain = snr - snr_slit;
	  break;
	  
	default:
	  peak = (int)(counts*peak_frac/sum_frac*t/gain);
	  break;
	}
	
	// Bad column
	if ( ymax < ymin ){
	  yy1 = ymax;
	  yy2 = ymin;
	}
	else{
	  yy1 = ymin;
	  yy2 = ymax;
	}
	
	if (ccd==2) {//Red CCD
	  ybad0=(4100.0-2864.0)/4100.0*(yy2-yy1)+yy1;
	  if ( (yy1<1207) && (yy2>1207) ){
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<2048-1207) && (ybad0>2048-1207) ){
	    badcolumn = TRUE;
	  }
	  else{
	    badcolumn = FALSE;
	  }
	  
	  if (peak>satlevel){
	    hg->obj[i_list].sat=TRUE;
	    saturation = TRUE;
	  }
	  else{
	    saturation = FALSE;
	  }
	}
	else{// Blue CCD
	  ybad1=(4100.0-2696.0)/4100.0*(yy2-yy1)+yy1;
	  ybad2=(4100.0-2675.0)/4100.0*(yy2-yy1)+yy1;
	  ybad3=(4100.0-2243.0)/4100.0*(yy2-yy1)+yy1;
	  ybad4=(4100.0- 500.0)/4100.0*(yy2-yy1)+yy1;
	  ybad5=(4100.0- 300.0)/4100.0*(yy2-yy1)+yy1;
	  if ( (yy1<76)  && (ybad1>76) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<128) && (ybad2>128) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<361) && (ybad3>361) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<1721) && (ybad4>1721) ) {
	    badcolumn = TRUE;
	  }
	  else if ( (yy1<1745) && (ybad5>1745) ) {
	    badcolumn = TRUE;
	  }
	  else{
	    badcolumn = FALSE;
	  }
	  
	  if (peak>satlevel){
	    hg->obj[i_list].sat=TRUE;
	    saturation = TRUE;
	  }
	  else{
	    saturation = FALSE;
	  }
	}
	
	hg->etc[i_etc].order=order;
	hg->etc[i_etc].pix_s=(int)(ymin+0.499);
	hg->etc[i_etc].pix_c=(int)(yc+0.499);
	hg->etc[i_etc].pix_e=(int)(ymax+0.499);
	hg->etc[i_etc].w_s=wmin;
	hg->etc[i_etc].w_c=wc;
	hg->etc[i_etc].w_e=wmax;
	hg->etc[i_etc].disp=xdisp*(ybin[binning])/(gdouble)order;
	hg->etc[i_etc].flux=flux[order];
	hg->etc[i_etc].peak=peak;
	hg->etc[i_etc].snr=snr;
 	hg->etc[i_etc].sat=saturation;
	hg->etc[i_etc].bad=badcolumn;
	hg->etc[i_etc].ccd=ccd;
	hg->etc[i_etc].isgap=isgap;
	
	if(slitunit==IS_NO){
	  hg->etc[i_etc].isgain=-1;
	  hg->etc[i_etc].snr_gain=-1;
	}
	else{
	  hg->etc[i_etc].isgain=isgain;
	  hg->etc[i_etc].snr_gain=snr_gain;
	}
	
	if(hg->etc_wave==ETC_WAVE_CENTER){
	  if((ccd==2)&&(!snr_measured)){
	    ret=snr;
	    snr_measured=TRUE;
	  }
	}
	else{
	  if(!snr_measured){
	    if((wmin<hg->etc_waved)&&(hg->etc_waved<wmax)){
	      ret=snr;
	      snr_measured=TRUE;
	    }
	  }
	}
	
	i_etc++;
      }
    }
    order--;
  }


  if(snr_measured)
    return(ret);
  else
    return(-1);
}
