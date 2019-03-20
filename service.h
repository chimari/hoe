// service.h for Subaru Service Prorams
//             Mar 2019  A. Tajitsu (Subaru Telescope, NAOJ)

// Service_Treeview
enum
{
  COLUMN_SVC_NUMBER,
  COLUMN_SVC_DAY,
  COLUMN_SVC_COLFG_DAY,
  COLUMN_SVC_ALLOC,
  COLUMN_SVC_MOON,
  COLUMN_SVC_COLFG_MOON,
  COLUMN_SVC_COLBG_MOON,
  COLUMN_SVC_MIN,
  COLUMN_SVC_MAX,
  COLUMN_SVC_OBJ,
  COLUMN_SVC_TIME,
  COLUMN_SVC_COLBG_TIME,
  NUM_COLUMN_SVC
};


enum{
  SVC_NIGHT_FULL,
  SVC_NIGHT_1ST,
  SVC_NIGHT_2ND,
  NUM_SVC_NIGHT
};


void do_calc_service();
void calc_service_sem();
void calc_service_night();
void create_calc_service_dialog();

void close_service();
void create_service_dialog();

GtkTreeModel * service_create_items_model();
void service_tree_update_item();
void service_add_columns();
void service_cell_data_func();
void service_make_tree();
int subZeller();

void service_set_date();
