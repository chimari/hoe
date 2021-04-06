// Header for GUI common for all instruments
#ifdef USE_WIN32
#define GMAP_URL "http://maps.google.com/maps?q=%lf,%lf%%28here!%%29&hl=en"
#elif defined(USE_OSX)
#define GMAP_URL "open http://maps.google.com/maps?q=%lf,%lf%%28here!%%29\\&hl=en"
#else
#define GMAP_URL "\"http://maps.google.com/maps?q=%lf,%lf%%28here!%%29&hl=en\""
#endif

////////////// gui_init() Create Main GUI
void gui_init();

////////////// delete_quit() : Change "delete event of main window to quit from the program.
gboolean delete_quit ();
gboolean delete_main_quit();

////////////// make_note() Create TABS in Main Window
void make_note();

void set_obs_param_from_preset();
void SetObsPreset();
void RadioPresetObs();
void CheckGmap();

///// GUI creation
void GUI_GENERAL_TAB_create();
void GUI_OBSERVATORY_TAB_create();
void GUI_TARGET_TAB_create();
void GUI_STD_TAB_create();
void GUI_FCDB_TAB_create();
void GUI_TRDB_TAB_create();


///// Callbacks
void ChangeFontButton();
void ChangeFontButton_all();
void get_font_family_size();

void UpdateTotalExp();

void popup_fr_calendar();
void select_fr_calendar();
void set_fr_e_date();

void ReadPass();
void GetPass();

void clip_copy();

void cc_get_combo_box_trdb();

void set_win_title();


