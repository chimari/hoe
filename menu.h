// Header for main menu

void make_menu();

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///////////////  Callbacks    for File I/Os  ---> io_gui.c
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

//// Menu -> File
void do_init_list ();
void do_change_inst();
void do_quit();


//// Menu -> Edit
void do_plan();
void do_edit();


//// Menu -> Tool
void do_skymon();

//// Menu -> Update
void do_update_exp_list();


//// Menu -> About
void show_version();


//// Others
void do_plot();

////////////////////////////////////////////////////////////
////////////////  Functions calling from menu callbacks
////////////////////////////////////////////////////////////

void SelectInst();
gboolean CheckInst();
void create_quit_dialog();

