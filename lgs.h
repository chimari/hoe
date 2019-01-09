//////////////////////////////////////////////////////////////////////////
//////////// for LGS PRM file
//////////////////////////////////////////////////////////////////////////


typedef struct _LGS_Points_Entry LGS_Points_Entry;
struct _LGS_Points_Entry{
  gdouble az;
  gdouble el;
  gchar *name;
};

static const LGS_Points_Entry LGS_AzEl[]={
  {90.00000,	90.00000,	"zenith"},
  {00.00000,	80.00000,	"Elevation80N"},
  {22.50000,	80.00000,	"Elevation80NNE"},
  {45.00000,	80.00000,	"Elevation80NE"},
  {67.50000,	80.00000,	"Elevation80ENE"},
  {90.00000,	80.00000,	"Elevation80E"},
  {112.50000,	80.00000,	"Elevation80ESE"},
  {135.00000,	80.00000,	"Elevation80SE"},
  {157.50000,	80.00000,	"Elevation80SSE"},
  {180.00000,	80.00000,	"Elevation80S"},
  {202.50000,	80.00000,	"Elevation80SSW"},
  {225.00000,	80.00000,	"Elevation80SW"},
  {247.50000,	80.00000,	"Elevation80WSW"},
  {270.00000,	80.00000,	"Elevation80W"},
  {295.50000,	80.00000,	"Elevation80WNW"},
  {315.00000,	80.00000,	"Elevation80NW"},
  {337.50000,	80.00000,	"Elevation80NNW"},
  {00.00000 ,	60.00000,	"Elevation60N"},
  {45.00000 ,	60.00000,	"Elevation60NE"},
  {90.00000 ,	60.00000,	"Elevation60E"},
  {135.00000,	60.00000,	"Elevation60SE"},
  {180.00000,	60.00000,	"Elevation60S"},
  {225.00000,	60.00000,	"Elevation60SW"},
  {270.00000,	60.00000,	"Elevation60W"},
  {315.00000,	60.00000,	"Elevation60NW"},
  {00.00000 ,	45.00000,	"Elevation45N"},
  {22.50000 ,	45.00000,	"Elevation45NNE"},
  {45.00000 ,	45.00000,	"Elevation45NE"},
  {67.50000 ,	45.00000,	"Elevation45ENE"},
  {90.00000 ,	45.00000,	"Elevation45E"},
  {112.50000,	45.00000,	"Elevation45ESE"},
  {135.00000,	45.00000,	"Elevation45SE"},
  {157.50000,	45.00000,	"Elevation45SSE"},
  {180.00000,	45.00000,	"Elevation45S"},
  {202.50000,	45.00000,	"Elevation45SSW"},
  {225.00000,	45.00000,	"Elevation45SW"},
  {247.50000,	45.00000,	"Elevation45WSW"},
  {270.00000,	45.00000,	"Elevation45W"},
  {295.50000,	45.00000,	"Elevation45WNW"},
  {315.00000,	45.00000,	"Elevation45NW"},
  {337.50000,	45.00000,	"Elevation45NNW"},
  {00.00000 ,	30.00000,	"Elevation30N"},
  {22.50000 ,	30.00000,	"Elevation30NNE"},
  {45.00000 ,	30.00000,	"Elevation30NE"},
  {67.50000 ,	30.00000,	"Elevation30ENE"},
  {90.00000 ,	30.00000,	"Elevation30E"},
  {112.50000,	30.00000,	"Elevation30ESE"},
  {135.00000,	30.00000,	"Elevation30SE"},
  {157.50000,	30.00000,	"Elevation30SSE"},
  {180.00000,	30.00000,	"Elevation30S"},
  {202.50000,	30.00000,	"Elevation30SSW"},
  {225.00000,	30.00000,	"Elevation30SW"},
  {247.50000,	30.00000,	"Elevation30WSW"},
  {270.00000,	30.00000,	"Elevation30W"},
  {295.50000,	30.00000,	"Elevation30WNW"},
  {315.00000,	30.00000,	"Elevation30NW"},
  {337.50000,	30.00000,	"Elevation30NNW"},
  {00.00000 ,	25.00000,	"Elevation25N"},
  {45.00000 ,	25.00000,	"Elevation25NE"},
  {90.00000 ,	25.00000,	"Elevation25E"},
  {135.00000,	25.00000,	"Elevation25SE"},
  {180.00000,	25.00000,	"Elevation25S"},
  {225.00000,	25.00000,	"Elevation25SW"},
  {270.00000,	25.00000,	"Elevation25W"},
  {315.00000,	25.00000,	"Elevation25NW"},
  {0.0000000,   0.00000,        NULL}};


#define LGS_NAME "Subaru_LGS_589nm_5W_1.2urad_143MHz"
#define LGS_FNAME_BASE "PRM_Subaru_LGS_589nm5W2.2urad_"

gboolean check_lgs_only();
void lgs_do_create_prm();
void lgs_do_setup_sa();
gboolean Check_LGS_SA();

