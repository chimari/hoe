// Header for GUI for file I/Os

//////////////////////////////////////////////////////////////
///////////////  Common Functions
//////////////////////////////////////////////////////////////

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...);
gboolean CheckChildDialog();
gboolean CheckDefDup();
gboolean ow_dialog();



///////////////////////////////////////////////////////////////////
////////// Open File
///////////////////////////////////////////////////////////////////

enum
{
    OPEN_FILE_READ_LIST,
    OPEN_FILE_MERGE_LIST,
    OPEN_FILE_MERGE_OPE,
    OPEN_FILE_UPLOAD_OPE,
    OPEN_FILE_EDIT_OPE,
    OPEN_FILE_READ_HOE,
    OPEN_FILE_MERGE_HOE,
    OPEN_FILE_READ_NST,
    OPEN_FILE_READ_JPL,
    OPEN_FILE_CONV_JPL,
    NUM_OPEN_FILE
};

void action_read_list();
void action_merge_list();
void select_list_style();
void do_merge_ope();
void do_upload_ope();
void do_open_hoe();
void do_merge_hoe();
void OpenFile();

void ReadList();
void MergeList();
void MergeListOPE();
void UploadOPE();

///////////////////////////////////////////////////////////////////
////////// Save File
///////////////////////////////////////////////////////////////////

enum
{
    SAVE_FILE_BASE_OPE,
    SAVE_FILE_PLAN_OPE,
    SAVE_FILE_HOE,
    SAVE_FILE_PDF_PLOT,	
    SAVE_FILE_PDF_SKYMON,
    SAVE_FILE_PDF_EFS,	
    SAVE_FILE_PDF_FC,	    
    SAVE_FILE_PDF_FC_ALL,
    SAVE_FILE_PLAN_TXT,
    SAVE_FILE_PROMS_TXT,
    SAVE_FILE_SERVICE_TXT,
    SAVE_FILE_IRCS_LGS_TXT,
    SAVE_FILE_FCDB_CSV,
    SAVE_FILE_TRDB_CSV,
    SAVE_FILE_PLAN_YAML,
    SAVE_FILE_CONV_JPL,
    SAVE_FILE_DOWNLOAD_LOG,
    NUM_SAVE_FILE
};

////////// OPE save
void do_save_base_ope();
void do_save_plan_ope();
////////// HOE save
void do_save_hoe();
////////// PDF save
void do_save_plot_pdf();
void do_save_skymon_pdf();
void do_save_efs_pdf();
void do_save_fc_pdf();
void do_save_fc_pdf_all();
////////// text files
void do_save_plan_txt();
void do_save_proms_txt();
void do_save_service_txt();
////////// CSV files
void do_save_fcdb_csv();
void do_save_trdb_csv();
///////// YAML files
void do_save_plan_yaml();
///////// HDS Obs Log from sumda
void do_download_log();

void SaveFile();


///////////////////////////////////////////////////////////////////
////////// Non-Sidereal Tracking
///////////////////////////////////////////////////////////////////

void do_open_NST();
void do_open_JPL();
void do_conv_JPL();

gboolean MergeNST();
gboolean MergeJPL();
void ConvJPL();


///////////////////////////////////////////////////////////////////
//////////   core procedure of Read/Write HOE file
///////////////////////////////////////////////////////////////////

void WriteHOE();
void ReadHOE_ObjList();
void ReadHOE();
void MergeListHOE();

///////////////////////////////////////////////////////////////////
//////////   core procedure of Read/Write Conf (HOME$/.hoe) file
///////////////////////////////////////////////////////////////////

void WriteConf();
void ReadConf();
