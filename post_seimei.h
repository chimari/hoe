//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_seimei.h  --- POST body for Seimei Obs script
//   
//                                           2017.11.15  A.Tajitsu

#define MAGDB_HOST_SEIMEI_SH "ooruri.kusastro.kyoto-u.ac.jp"
#define MAGDB_PATH_SEIMEI_SH_KOOLS "/~maehara/kools_obs_script/generate_commands2.py"
#define MAGDB_PATH_SEIMEI_SH_TRICCS "/~maehara/triccs_obs_script/generate_commands2.py"

static const PARAMpost seimei_kools_post[] = {
  {POST_INPUT, "propid", NULL},
  {POST_INPUT, "observer", NULL},
  {POST_INPUT, "object1", NULL},
  {POST_INPUT, "grism1", NULL},
  {POST_INPUT, "exptime1", NULL},
  {POST_INPUT, "nexp1", NULL},
  {POST_INPUT, "ptc1", NULL},
  {POST_INPUT, "ag1", NULL},
  {POST_INPUT, "nw1", NULL},
  {POST_NULL,  "object2", NULL}, 
  {POST_CONST, "grism2", "VPH-blue"},
  {POST_NULL,  "exptime2", NULL},
  {POST_NULL,  "nexp2", NULL},
  {POST_CONST, "ptc2", "1"},
  {POST_CONST, "ag2", "1"},
  {POST_CONST, "nw2", "0"},
  {POST_NULL,  "object3", NULL}, 
  {POST_CONST, "grism3", "VPH-blue"},
  {POST_NULL,  "exptime3", NULL},
  {POST_NULL,  "nexp3", NULL},
  {POST_CONST, "ptc3", "1"},
  {POST_CONST, "ag3", "1"},
  {POST_CONST, "nw3", "0"},
  {POST_NULL,  "object4", NULL}, 
  {POST_CONST, "grism4", "VPH-blue"},
  {POST_NULL,  "exptime4", NULL},
  {POST_NULL,  "nexp4", NULL},
  {POST_CONST, "ptc4", "1"},
  {POST_CONST, "ag4", "1"},
  {POST_CONST, "nw4", "0"},
  {POST_NULL,  "object5", NULL}, 
  {POST_CONST, "grism5", "VPH-blue"},
  {POST_NULL,  "exptime5", NULL},
  {POST_NULL,  "nexp5", NULL},
  {POST_CONST, "ptc5", "1"},
  {POST_CONST, "ag5", "1"},
  {POST_CONST, "nw5", "0"},
  {POST_NULL,  "object6", NULL}, 
  {POST_CONST, "grism6", "VPH-blue"},
  {POST_NULL,  "exptime6", NULL},
  {POST_NULL,  "nexp6", NULL},
  {POST_CONST, "ptc6", "1"},
  {POST_CONST, "ag6", "1"},
  {POST_CONST, "nw6", "0"},
  {POST_NULL,  "object7", NULL}, 
  {POST_CONST, "grism7", "VPH-blue"},
  {POST_NULL,  "exptime7", NULL},
  {POST_NULL,  "nexp7", NULL},
  {POST_CONST, "ptc7", "1"},
  {POST_CONST, "ag7", "1"},
  {POST_CONST, "nw7", "0"},
  {POST_NULL,  "object8", NULL}, 
  {POST_CONST, "grism8", "VPH-blue"},
  {POST_NULL,  "exptime8", NULL},
  {POST_NULL,  "nexp8", NULL},
  {POST_CONST, "ptc8", "1"},
  {POST_CONST, "ag8", "1"},
  {POST_CONST, "nw8", "0"},
  {POST_NULL,  "object9", NULL}, 
  {POST_CONST, "grism9", "VPH-blue"},
  {POST_NULL,  "exptime9", NULL},
  {POST_NULL,  "nexp9", NULL},
  {POST_CONST, "ptc9", "1"},
  {POST_CONST, "ag9", "1"},
  {POST_CONST, "nw9", "0"},
  {POST_NULL,  "object10", NULL}, 
  {POST_CONST, "grism10", "VPH-blue"},
  {POST_NULL,  "exptime10", NULL},
  {POST_NULL,  "nexp10", NULL},
  {POST_CONST, "ptc10", "1"},
  {POST_CONST, "ag10", "1"},
  {POST_CONST, "nw10", "0"},
  {POST_CONST, "submit", "generate+commands%21"},
  {POST_NULL,  NULL, NULL}};


static const PARAMpost seimei_triccs_post[] = {
  {POST_INPUT, "propid", NULL},
  {POST_INPUT, "observer", NULL},
  {POST_INPUT, "object1", NULL},
  {POST_INPUT, "filter1", "gri"},
  {POST_INPUT, "gain1",   NULL},
  {POST_INPUT, "exptime1", NULL},
  {POST_INPUT, "nframe1", NULL},
  {POST_INPUT, "nexp1", NULL},
  {POST_INPUT, "ptc1", NULL},
  {POST_INPUT, "ag1", NULL},
  {POST_NULL,  "object2", NULL}, 
  {POST_CONST, "filter2", "gri"},
  {POST_CONST, "gain2", "auto"},
  {POST_NULL,  "exptime2", NULL},
  {POST_NULL,  "nframe2", NULL},
  {POST_NULL,  "nexp2", NULL}, 
  {POST_CONST, "ptc2", "0"},
  {POST_CONST, "ag2", "1"},
  {POST_NULL,  "object3", NULL}, 
  {POST_CONST, "filter3", "gri"},
  {POST_CONST, "gain3", "auto"},
  {POST_NULL,  "exptime3", NULL},
  {POST_NULL,  "nframe3", NULL},
  {POST_NULL,  "nexp3", NULL}, 
  {POST_CONST, "ptc3", "0"},
  {POST_CONST, "ag3", "1"},
  {POST_NULL,  "object4", NULL}, 
  {POST_CONST, "filter4", "gri"},
  {POST_CONST, "gain4", "auto"},
  {POST_NULL,  "exptime4", NULL},
  {POST_NULL,  "nframe4", NULL},
  {POST_NULL,  "nexp4", NULL}, 
  {POST_CONST, "ptc4", "0"},
  {POST_CONST, "ag4", "1"},
  {POST_NULL,  "object5", NULL}, 
  {POST_CONST, "filter5", "gri"},
  {POST_CONST, "gain5", "auto"},
  {POST_NULL,  "exptime5", NULL},
  {POST_NULL,  "nframe5", NULL},
  {POST_NULL,  "nexp5", NULL}, 
  {POST_CONST, "ptc5", "0"},
  {POST_CONST, "ag5", "1"},
  {POST_NULL,  "object6", NULL}, 
  {POST_CONST, "filter6", "gri"},
  {POST_CONST, "gain6", "auto"},
  {POST_NULL,  "exptime6", NULL},
  {POST_NULL,  "nframe6", NULL},
  {POST_NULL,  "nexp6", NULL}, 
  {POST_CONST, "ptc6", "0"},
  {POST_CONST, "ag6", "1"},
  {POST_NULL,  "object7", NULL}, 
  {POST_CONST, "filter7", "gri"},
  {POST_CONST, "gain7", "auto"},
  {POST_NULL,  "exptime7", NULL},
  {POST_NULL,  "nframe7", NULL},
  {POST_NULL,  "nexp7", NULL}, 
  {POST_CONST, "ptc7", "0"},
  {POST_CONST, "ag7", "1"},
  {POST_NULL,  "object8", NULL}, 
  {POST_CONST, "filter8", "gri"},
  {POST_CONST, "gain8", "auto"},
  {POST_NULL,  "exptime8", NULL},
  {POST_NULL,  "nframe8", NULL},
  {POST_NULL,  "nexp8", NULL}, 
  {POST_CONST, "ptc8", "0"},
  {POST_CONST, "ag8", "1"},
  {POST_NULL,  "object9", NULL}, 
  {POST_CONST, "filter9", "gri"},
  {POST_CONST, "gain9", "auto"},
  {POST_NULL,  "exptime9", NULL},
  {POST_NULL,  "nframe9", NULL},
  {POST_NULL,  "nexp9", NULL}, 
  {POST_CONST, "ptc9", "0"},
  {POST_CONST, "ag9", "1"},
  {POST_NULL,  "object10", NULL}, 
  {POST_CONST, "filter10", "gri"},
  {POST_CONST, "gain10", "auto"},
  {POST_NULL,  "exptime10", NULL},
  {POST_NULL,  "nframe10", NULL},
  {POST_NULL,  "nexp10", NULL}, 
  {POST_CONST, "ptc10", "0"},
  {POST_CONST, "ag10", "1"},
  {POST_CONST,  "submit", "generate+commands%21"},
  {POST_NULL,  NULL, NULL}};
