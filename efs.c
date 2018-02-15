//    hoe : Subaru HDS OPE file Editor
//        efs.c :  Echelle Format Simulator
//                                           2018.02.14  A.Tajitsu

#include"main.h"
#include<math.h>


#define BUFFER_SIZE 1024

enum{CROSS_RED, CROSS_BLUE} cross_color;


void efs();

void close_efs();
void create_efs_dialog();
gboolean draw_efs_cairo();
gdouble nx();
gdouble ny();
gdouble ny2();

static void refresh_efs();
void cc_get_efs_mode();


// global arguments
int mout[9];
double wlout[9];
double ypix1[4][51], ypix2[4][51];
int n1end,  n2end;
double wl[MAX_LINE+1],xline[1000][4],yline1[1000][4],yline2[1000][4];
double xpt[1000],ypt[1000];
double wlfmax[201],wlfmin[201],xfmax[201],xfmin[201],yfmax[201],yfmin[201];
gchar line_name[MAX_LINE+1][BUFFER_SIZE];

gboolean flagEFS=FALSE;



void efs(int ncross,double theta_E_mes, double theta_C,double det_rot, int nlines){

  int    order1[51],order2[51];
  int    ypixmin1[51],ypixcen1[51],ypixmax1[51];
  double wlnmmin1[51],wlnmcen1[51],wlnmmax1[51];
  int    ypixmin2[51],ypixcen2[51],ypixmax2[51];
  double wlnmmin2[51],wlnmcen2[51],wlnmmax2[51];
  double wl0[201],wlmax[201],wlmin[201],wlb[201];
  double y0[201],ymax[201],ymin[201];
  double fsr[201];

  double rad, a_E, theta_E, a_C, a_CB, a_CR, cos_gam_C,cos_eps_E;
  double ccdgap, mccdgap, ccdmin, ccdmax, mccdmax, ccdsize, pixsize;
  double delta_rot, delta_pos, d_wly1, d_wly2, d_wlx0;
  double fcam;
  int   mmin1, mmin2, mmax1, mmax2, mmid1, mmid2;
  double wlmax1, wlmax2, wlmin1, wlmin2, wlmid1, wlmid2, wl_pix1, wl_pix2;
  int   n1, n2;

  double beta_E, beta_C, wl_cent;
  int  i,j,m;
  double deltawl , fsrpr;
  double d_wlx, wlpr1, wlpr2, wlpr3, wlpr4, wlpr5, ald, cdmin, cdcen, cdmax;
  double aordsep, wl1, wl2048, wl4096, yline;
  int yintmin, yintmax, yint0;
  double rough_x;


  rad=3.14159/180.0;
  a_E=1./316.0;
  theta_E=71.26;
  a_CB=1./4000.;
  a_CR=1./2500.;
  //cos_gam_C=0.92388;
  cos_gam_C=cos((22.5-0./3600.)*rad);
  //cos_eps_E=0.99452;
  cos_eps_E=cos((6-0./3600.0)*rad); //After EQ
  ccdgap=43.;
  mccdgap=-ccdgap;
  ccdmin=ccdgap;
  ccdmax=2048.+ccdgap;
  mccdmax=-ccdmax;
  ccdsize=5.5296;
  pixsize=0.00135;
  delta_rot=2048.*sin(rad*det_rot);
  //  +delta_rot for y=4096, -delta_rot for y=1
  delta_pos=20.;            // correction on detector position
  d_wly1=1.00;  
  d_wly2=1.00;
  d_wlx0=0.00;

  // parameters 
  fcam=77.0;

  mmin1=200;
  mmax1=0;
  mmin2=200;
  mmax2=0;
  mmid1=0;
  mmid2=0;
 
  wlmin1=1000.;
  wlmin2=1000.;
  wlmax1=0.;
  wlmax2=0.;
  wlmid1=0.;
  wlmid2=0.; 
  wl_pix1=0.; 
  wl_pix2=0.; 
  n1=1;
  n2=1;

  // variables 
  //      wl_cent,deltawl
  //	 m,m_max,m_min
  //	 yintmin,yint0,yintmax
  //	double wl1,wl2048,wl4096
  //   begin

  if(ncross==CROSS_BLUE){
    a_C=a_CB;
  }
  else{
    a_C=a_CR;
  }
   
  beta_E=theta_E;
  beta_C=theta_C-22.5;
  wl_cent=2.0*a_C*sin(rad*theta_C)*cos_gam_C;
  for(i=1;i<=nlines;i++){
    for(j=1;j<=3;j++){
      xline[i][j]=-10000.0;
      yline1[i][j]=-10000.0;
      yline2[i][j]=-10000.0;
    }
  }
  m=199;

  while(m>50){
    wlb[m]=2.0*a_E*cos_eps_E*sin(rad*theta_E)/(double)m;
    wl0[m]=wlb[m]+a_E*cos_eps_E*cos(rad*theta_E)
      *(tan(2.*rad*theta_E_mes))/(double)m;
    deltawl=a_E*cos_eps_E*cos(rad*beta_E)*0.5*ccdsize/(fcam*(double)m);
    wlmax[m]=wl0[m]+deltawl*d_wly1;
    wlmin[m]=wl0[m]-deltawl*d_wly2;
    if(wl0[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    y0[m]=fcam*(wl0[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize + delta_pos;
    if(wlmax[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    ymax[m]=fcam*(wlmax[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize-delta_rot + delta_pos +20.;
    if(wlmin[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    ymin[m]=fcam*(wlmin[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize+delta_rot + delta_pos;

    //*** Calculation of free spectrum range ***
    fsr[m]=wlb[m]/(double)m;
    wlfmax[m]=wlb[m] + 0.5*fsr[m];
    wlfmin[m]=wlb[m] - 0.5*fsr[m];
    xfmax[m]=0.5*wlb[m]*fcam/(a_E*cos_eps_E*cos(rad*beta_E))/pixsize;
    xfmin[m]=-xfmax[m];
    yfmax[m]=fcam*(wlfmax[m]-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize -delta_rot + delta_pos;
    yfmin[m]=fcam*(wlfmin[m]-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize +delta_rot + delta_pos;
    wlpr1=1.0e+7*wlb[m];     // [nm] 
    wlpr2=1.0e+7*wlfmin[m];  // [nm] 
    wlpr3=1.0e+7*wlfmax[m];  // [nm] 
    fsrpr=1.0e+7*fsr[m];     // [nm] 
    wlpr4=1.0e+7*wlmin[m];   // [nm] 
    wlpr5=1.0e+7*wlmax[m];   // [nm] 
    ald=2.0e+6*deltawl/ccdsize;  // [nm/mm]
    cdmin=(yfmin[m]-yfmin[199])*pixsize*10.; // [mm]
    cdcen=(y0[m]-yfmin[199])*pixsize*10.;    // [mm]
    cdmax=(yfmax[m]-yfmin[199])*pixsize*10.; // [mm]
    aordsep=cdmax-cdmin;                     // [mm]

    if( (ymax[m]>ccdmax) || 
	((ymin[m]<ccdgap)&&(ymax[m]>mccdgap)) ||
	(ymin[m]<mccdmax)){
    }
    else{
      yintmax = (int)(ymax[m]);
      yint0   = (int)(y0[m]);
      yintmin = (int)(ymin[m]);
      wl1    = 1.0e+7*wlmin[m];
      wl2048 = 1.0e+7*wl0[m];
      wl4096 = 1.0e+7*wlmax[m];
      for(i=1;i<=nlines;i++){
	if((wl[i]>=wl1)&&(wl[i]<=wl4096)){
	  j=1;
	  if(xline[i][j]>0.0){
	    j=2;
	    if(xline[i][j]>0.0){
	      j=3;
	      xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;//!!!!
	    }
	    else{
	      xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;
	    }
	  }
	  else{ 
	    xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;
	  }
	  yline=fcam*(1.0e-7*wl[i]-wl_cent)/
	    (a_C*cos(rad*beta_C))/pixsize+delta_pos
	    -delta_rot*(xline[i][j]-2048.)/2048.;
	  if(yline>-ccdgap){
	    yline1[i][j]=2048.0+ccdgap-yline;
	    //yline1[i][j]=2048.0-yline;
	    yline2[i][j]=-90000.;
	  }
	  else{
	    yline1[i][j]=-90000.;
	    yline2[i][j]=-yline-ccdgap;
	  }
	}
      }
      
      if(ymin[m]>-ccdgap){
	order1[n1]=m;
	ypixmin1[n1]=2048+(int)(ccdgap)-yintmin;
	ypixcen1[n1]=2048+(int)(ccdgap)-yint0;
	ypixmax1[n1]=2048+(int)(ccdgap)-yintmax;
	wlnmmin1[n1]=wl1;
	wlnmcen1[n1]=wl2048;
	wlnmmax1[n1]=wl4096;
	//            write(6,101)order1[n1],ypixmin1[n1],ypixcen1[n1],
	//&       ypixmax1[n1],wlnmmin1[n1],wlnmcen1[n1],wlnmmax1[n1]
	// 101        format(i5,3i5,3f10.3)
	ypix1[1][n1]=(double)(ypixmin1[n1]);
	ypix1[2][n1]=(double)(ypixcen1[n1]);
	ypix1[3][n1]=(double)(ypixmax1[n1]);
	n1=n1+1;
	if(m<mmin1) mmin1=m;
	if(wl1<wlmin1) wlmin1=wl1;
	if(m>mmax1) mmax1=m;
	if(wl4096>wlmax1) wlmax1=wl4096;
	if(yint0<1024){
	  mmid1=m ;
	  wlmid1=wl2048;
	  wl_pix1=(wl4096-wl1)/4096.;
	}
      }
      else {
	order2[n2]=m;
	ypixmin2[n2]=-(int)(ccdgap)-yintmin;
	ypixcen2[n2]=-(int)(ccdgap)-yint0;  
	ypixmax2[n2]=-(int)(ccdgap)-yintmax;
	wlnmmin2[n2]=wl1;
	wlnmcen2[n2]=wl2048;
	wlnmmax2[n2]=wl4096;
	//      write(6,101)order2[n2],ypixmin2[n2],ypixcen2[n2],
	//&       ypixmax2[n2],wlnmmin2[n2],wlnmcen2[n2],wlnmmax2[n2]
	ypix2[1][n2]=(double)(ypixmin2[n2]);
	ypix2[2][n2]=(double)(ypixcen2[n2]);
	ypix2[3][n2]=(double)(ypixmax2[n2]);
	n2=n2+1;
	if(m<mmin2) mmin2=m;
	if(wl1<wlmin2) wlmin2=wl1;
	if(m>mmax2) mmax2=m;
	if(wl4096>wlmax2) wlmax2=wl4096;
	if(yint0<-1024){
	  mmid2=m; 
	  wlmid2=wl2048;
	  wl_pix2=(wl4096-wl1)/4096.;
	}
      }
    }
     m--;
  }

  // 22   continue

  n1end=n1-1;
  n2end=n2-1;
  mout[1]=mmin1;
  mout[2]=mmid1;
  mout[3]=mmax1;
  mout[4]=mmin2;
  mout[5]=mmid2;
  mout[6]=mmax2;
  wlout[1]=wlmin1;
  wlout[2]=wlmid1;
  wlout[3]=wlmax1;
  wlout[4]=wl_pix1;
  wlout[5]=wlmin2;
  wlout[6]=wlmid2;
  wlout[7]=wlmax2;
  wlout[8]=wl_pix2;
}


// Create EFS Window
void close_efs(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;


  gtk_widget_destroy(GTK_WIDGET(hg->efs_main));
  flagEFS=FALSE;
}


void go_efs(typHOE *hg){
  if(flagEFS){
    gdk_window_raise(hg->efs_main->window);
    draw_efs_cairo(hg->efs_dw,NULL,(gpointer)hg);
    return;
  }
  else{
    flagEFS=TRUE;
  }
  
  create_efs_dialog(hg);
}



void create_efs_dialog(typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1;
  GtkWidget *frame, *check, *label, *button;
  GSList *group=NULL;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;


  hg->efs_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->efs_main), "HOE : Echelle Format Simulator");
  //gtk_widget_set_usize(hg->skymon_main, SKYMON_SIZE, SKYMON_SIZE);
  
  my_signal_connect(hg->efs_main,
		    "destroy",
		    close_efs, 
		    (gpointer)hg);

  gtk_widget_set_app_paintable(hg->efs_main, TRUE);
  

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->efs_main), vbox);


  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);


  frame = gtk_frame_new ("Mode");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Echelle Format Simulator",
		       1, EFS_PLOT_EFS, -1);
    if(hg->efs_mode==EFS_PLOT_EFS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Free Spectral Range",
		       1, EFS_PLOT_FSR, -1);
    if(hg->efs_mode==EFS_PLOT_FSR) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_efs_mode,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Act.");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);


  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (refresh_efs), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Refresh");
#endif


  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_efs), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Quit");
#endif

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_pdf), icon_pdf, 
				    FALSE, NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (do_save_efs_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

 
  // Drawing Area
  hg->efs_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->efs_dw, EFS_WIDTH, EFS_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), hg->efs_dw, TRUE, TRUE, 0);
  gtk_widget_set_app_paintable(hg->efs_dw, TRUE);
  gtk_widget_show(hg->efs_dw);

  my_signal_connect(hg->efs_dw, 
		    "expose-event", 
		    draw_efs_cairo,
		    (gpointer)hg);

  gtk_widget_show_all(hg->efs_main);

  gdk_window_raise(hg->efs_main->window);

  gdk_flush();
}



gboolean draw_efs_cairo(GtkWidget *widget, 
			 GdkEventExpose *event, 
			 gpointer userdata){
  cairo_t *cr;
  cairo_surface_t *surface;
  typHOE *hg=(typHOE *)userdata;;
  cairo_text_extents_t extents;
  double x,y;
  GdkPixmap *pixmap_efs;
  gint from_set, to_rise;
  double dx,dy,lx,ly;
  int width, height;

  if(!flagEFS) return (FALSE);

  if(hg->efs_output==EFS_OUTPUT_PDF){
    width=EFS_WIDTH;
    height=EFS_HEIGHT;

    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  }
  else{
    width= widget->allocation.width;
    height= widget->allocation.height;
    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->efs_main), &width, &height);
    }
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    pixmap_efs = gdk_pixmap_new(widget->window,
				width,
				height,
				-1);
  
    cr = gdk_cairo_create(pixmap_efs);
    
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
  }
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);


  // from efs_main
  {
    int  i, line_max;
    char buf[BUFFER_SIZE],buf2[BUFFER_SIZE];
    int  dummy;
    char *c;
    char tmp[BUFFER_SIZE];
    double rad;
    double cos_gam_C;
    double a_C,a_CB, a_CR;
    double theta_CB0, theta_CR0;
    double fcam;
    double pixsize;
    double d_det_rot, d_theta_CB, d_theta_CR, d_theta_E_mes;
    int   ncross;
    char  cross[2];
    int   howto;
    
    int nlines;
    double wlc, theta_C, wl_center, theta_E_mes, det_rot, aaa, theta_C0;
    char th_ec[30],crossrot[30];
    
    
    int nonstd_iset=-1;
    
    rad=3.14159/180.0;
    //cos_gam_C=0.92388;   // cos(gamma_c)=cos(22.5deg)
    cos_gam_C=cos((22.5-0./3600.)*rad);
    a_CB=1./4000;
    a_CR=1./2500;
    theta_CB0=4.76;
    theta_CR0=5.00;
    fcam=77.0;
    pixsize=0.00135;
    //d_det_rot=0.1;
    d_det_rot=0.1+2800./3600.; // After EQ
    //d_theta_CB=0.06;
    d_theta_CB=0.06-20./3600.;  // After EQ
    //d_theta_CR=0.11;
    d_theta_CR=0.11-20./3600.;  // After EQ
    //d_theta_E_mes=0.0;
    //d_theta_E_mes=550.0/3600.0;  // After EQ
    //d_theta_E_mes=600.0/3600.0;  // After EQ on 2010.4
    d_theta_E_mes=590.0/3600.0;  // After EQ on 2011.10
    
    

    // Read Line List
    for(i=0;i<=MAX_LINE;i++){
      wl[i]=0.0;
    }
    
    line_max=0;
    for(i=0;i<MAX_LINE;i++){
      if((hg->line[i].name)&&(hg->line[i].wave>0)){
	strcpy(line_name[line_max+1],hg->line[i].name);
	wl[line_max+1]=(double)hg->line[i].wave*(1+hg->etc_z)/10.;
	line_max++;
      }
    }
    
    nlines=line_max;
    
    // Cross Disperser
    if(hg->setup[hg->efs_setup].setup<0){  // None Std
      nonstd_iset=-hg->setup[hg->efs_setup].setup-1;
      if(hg->nonstd[nonstd_iset].col==COL_BLUE){
	a_C=a_CB;
	ncross=CROSS_BLUE;
      }
      else{
	a_C=a_CR;
	ncross=CROSS_RED;
      }
    }
    else if(hg->setup[hg->efs_setup].setup<StdI2b){ // Std Blue
      a_C=a_CB;
      ncross=CROSS_BLUE;
    }
    else{  //if (hg->setup[hg->efs_setup].setup<StdHa){  // Std Red
      a_C=a_CR;
      ncross=CROSS_RED;
    }
    
    // Cross Angle(=-1), WaveLength(=1)
    howto=-1;
    //if(nonstd_iset==-1){
    if(hg->setup[hg->efs_setup].setup<0){  // None Std
      theta_C=((double)hg->nonstd[nonstd_iset].cross-(gdouble)hg->d_cross)/60/60;
      theta_E_mes=(double)hg->nonstd[nonstd_iset].echelle/60./60.;
      det_rot=(double)hg->nonstd[nonstd_iset].camr/60./60.;
    }
    else{
      theta_C=(double)(setups[hg->setup[hg->efs_setup].setup].cross_scan)/60./60.;
      //theta_E_mes=1440./60./60.;
      theta_E_mes=DEF_ECHELLE/60./60.; //After EQ
      det_rot=-3600./60./60.;
      
    }
    sprintf(th_ec, "Echelle=%.4lf[deg]",theta_E_mes);
    theta_E_mes=theta_E_mes+d_theta_E_mes;
    theta_E_mes=theta_E_mes-0.425;
    
    
    det_rot= det_rot - d_det_rot;
    
    sprintf(crossrot,"Cross-rot=%.4lf[deg]",theta_C+(gdouble)hg->d_cross/60./60.);

    if(ncross==CROSS_BLUE){
      theta_C=theta_C-d_theta_CB;
    }
    else{
      theta_C=theta_C-d_theta_CR;
    }



    efs(ncross, theta_E_mes,theta_C, det_rot, nlines);
    

  // plot
  {
    char tmp[BUFFER_SIZE];
    double xmin0, xmax0, ymin0, ymax0;
    int   i,n,j;
    double x[4],y1[4],y2[4],y[4];
    double old_y2;
    double min_sep;
    float lenx[5],leny[5];
    double y1_1, y1_3, y3_1, y3_3, y2_1, y2_3, y4_1, y4_3, y2_2, y1_2;
    double xt_title, yt_title;
    double xt_cr, xt_crs, yt_cr, xt_ec, yt_ec, xt_det;
    int   i1, i2;
    double xccd_min, xccd_max, yccd1_min, yccd1_max, yccd2_min, yccd2_max;
    double xt_wave, xt_ord, yt_ord;
    int   imod;
    double xt_o, yt_o, xt_w, yt_w,ytext;
    char  order[10], wave[10], text[15][10];
    double fcam=77.0;
    double rad=3.14159/180.0;
    double pixsize=0.00135;
      // double d_det_rot=0.1;
    double d_det_rot=0.1+2800./3600.;  // After EQ
    double slit_pix;
    char line_txt[21][256];
    int line_flag[21];
    gdouble ccdgap=43.;
    gdouble rx, ry, xd, yd;
    gdouble yobj1, yobj2;
    

    // ### PLOT ###
    
    if(hg->efs_mode==EFS_PLOT_EFS){
      xmin0=000.;
      ymin0=-500.;
      xmax0=6500.;
      ymax0=5000.;

      rx=(gdouble)width/(xmax0-xmin0); 
      ry=(gdouble)height/(ymax0-ymin0);
      xd=dx/2;
      yd=height-dy;
    }
    else {// FSR
      if(ncross==CROSS_RED){
	xmax0=xfmax[56]+100.;
	ymax0=yfmax[56]+100;
	xmin0=xfmin[56]-100.;
	ymin0=yfmin[160]-100;
      }
      else{
	xmax0=xfmax[100]+1000.;
	ymax0=yfmax[100]+100;
	xmin0=xfmin[100]-1000.;
	ymin0=yfmin[199]-100;
      }
      rx=(gdouble)width/(xmax0-xmin0)*0.5; 
      ry=(gdouble)height/(ymax0-ymin0)*0.85;
      xd=dx*5;
      yd=height-dy/2.;
    }

    
    for(i=1;i<=8;i++){
      sprintf(text[i],"%.3lf",wlout[i]);
    }	
    for(i=1;i<=6;i++){
      sprintf(text[i+8],"%d",mout[i]);
    }	
    
    if(hg->efs_mode==EFS_PLOT_EFS){
      // Slit Length
      switch(hg->setup[hg->efs_setup].is){
      case IS_030X5:
	slit_pix=(8.4)/0.138;
	break;
      case IS_045X3:
	slit_pix=(4.8)/0.138;
	break;
      default:
	slit_pix=((double)hg->setup[hg->efs_setup].slit_length)/0.138/500;
      }
      
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      
      // CCD Chip
      {

	cairo_rectangle(cr,nx(0.,rx,xd),ny(2005.,ry,yd),4096.*rx,2048.*ry);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(0.,rx,xd),ny(2005.,ry,yd),4096.*rx,2048.*ry);
	cairo_set_source_rgba(cr, 0.95, 0.95, 1.0, 1.0);
	cairo_fill(cr);

	cairo_rectangle(cr,nx(0.,rx,xd),ny(4139.,ry,yd),4096.*rx,2048.*ry);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(0.,rx,xd),ny(4139.,ry,yd),4096.*rx,2048.*ry);
	cairo_set_source_rgba(cr, 1.0, 0.95, 0.95, 1.0);
	cairo_fill(cr);

	// 0.15um pitch (Just for CCD replacement test)
	/*
	cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(2005.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(2005.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	cairo_set_source_rgba(cr, 0.95, 0.95, 1.0, 0.5);
	cairo_fill(cr);

	cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(4139.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(4139.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	cairo_set_source_rgba(cr, 1.0, 0.95, 0.95, 0.5);
	cairo_fill(cr);
	*/
      }

      x[0]=1.;
      x[1]=2048.;
      x[2]=4096.;
      for(n=1;n<=n1end;n++){
	y1[0]=4096-ypix1[1][n]+43.;
	y1[1]=4096-ypix1[2][n]+43.;
	y1[2]=4096-ypix1[3][n]+43.;
	if(n==1)     y1_1=y1[0];
	if(n==1)     y3_1=y1[2];
	if(n==n1end) y1_3=y1[0];
	if(n==n1end) y3_3=y1[2];
	
	lenx[0]=x[0];
	lenx[1]=x[0];
	lenx[2]=x[2];
	lenx[3]=x[2];
	
	leny[0]=y1[0]-slit_pix/2;
	leny[1]=y1[0]+slit_pix/2;
	leny[2]=y1[2]+slit_pix/2;
	leny[3]=y1[2]-slit_pix/2;
	
	cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.2);
	cairo_set_line_width(cr,slit_pix*ry);
	cairo_move_to(cr,nx(x[0],rx,xd),ny(y1[0],ry,yd));
	cairo_line_to(cr,nx(x[2],rx,xd),ny(y1[2],ry,yd));
	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
	cairo_set_line_width(cr,1.0);
	cairo_move_to(cr,nx(x[0],rx,xd),ny(y1[0],ry,yd));
	cairo_line_to(cr,nx(x[2],rx,xd),ny(y1[2],ry,yd));
	cairo_stroke(cr);
	
      }
      
      for(n=1;n<=n2end;n++){
	y2[0]=2048-ypix2[1][n]-43.;
	y2[1]=2048-ypix2[2][n]-43.;
	y2[2]=2048-ypix2[3][n]-43.;
	if(n==1)     y2_1=y2[0];
	if(n==n2end) y2_3=y2[0];
	if(n==1)     y4_1=y2[2];
	if(n==n2end) y4_3=y2[2];
	
	lenx[0]=x[0];
	lenx[1]=x[0];
	lenx[2]=x[2];
	lenx[3]=x[2];
	
	leny[0]=y2[0]-slit_pix/2;
	leny[1]=y2[0]+slit_pix/2;
	leny[2]=y2[2]+slit_pix/2;
	leny[3]=y2[2]-slit_pix/2;
	
	cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.2);
	cairo_set_line_width(cr,slit_pix*ry);
	cairo_move_to(cr, nx(x[0],rx,xd),ny(y2[0],ry,yd));
	cairo_line_to(cr, nx(x[2],rx,xd),ny(y2[2],ry,yd));
	cairo_stroke(cr);
	cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
	cairo_set_line_width(cr,1.0);
	cairo_move_to(cr, nx(x[0],rx,xd),ny(y2[0],ry,yd));
	cairo_line_to(cr, nx(x[2],rx,xd),ny(y2[2],ry,yd));
	cairo_stroke(cr);

	if(n==1){
	  old_y2=y2[2];
	}
	else if(n==2){
	  min_sep=((y2[2]-slit_pix/2)-(old_y2+slit_pix/2))
	    /hg->binning[hg->setup[hg->efs_setup].binning].x;
	}
      }      
      cairo_set_font_size (cr, 10.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      switch(hg->setup[hg->efs_setup].is){
      case IS_030X5:
	sprintf(tmp,"Minimum order gap is %.2lf pix w/0\".30x5 Image Slicer",
		min_sep);
	break;
      case IS_045X3:
	sprintf(tmp,"Minimum order gap is %.2lf pix w/0\".45x3 Image Slicer",
		min_sep);
	break;
      default:
	sprintf(tmp,"Minimum order gap is %.2lf pix w/%.2f\" slit length.",
		min_sep,(float)hg->setup[hg->efs_setup].slit_length/500);
      }
      if (min_sep<5){
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
      }
      else{
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.8);
      }
      cairo_text_extents (cr, tmp, &extents);
      cairo_move_to(cr, dx, height-extents.height);
      cairo_show_text(cr, tmp);
      
      //      badcolumn in R
      cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.7);
      cairo_set_line_width(cr,1.5);
      
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1106.+2091.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1106.+2091.,ry,yd));
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1111.+2091.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1111.+2091.,ry,yd));
      
      cairo_move_to(cr, nx(1446.,rx,xd),ny(2048.- 938.+2091.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 938.+2091.,ry,yd));
      cairo_move_to(cr, nx(1446.,rx,xd),ny(2048.- 943.+2091.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 943.+2091.,ry,yd));
      
      //      badcolumn in B
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-  91.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-  91.-43.,ry,yd));
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.- 128.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 128.-43.,ry,yd));
      
      cairo_move_to(cr, nx(2244.,rx,xd),ny(2048.- 359.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 359.-43.,ry,yd));
      
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1721.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1721.-43.,ry,yd));
      cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1742.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1742.-43.,ry,yd));
      
      cairo_move_to(cr, nx(2711.,rx,xd),ny(2048.-1921.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1921.-43.,ry,yd));
      cairo_move_to(cr, nx(3118.,rx,xd),ny(2048.-1958.-43.,ry,yd));
      cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1958.-43.,ry,yd));
      
      cairo_stroke(cr);


      // Wavelength
      y2_2=1024.;
      y1_2=3072.;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 11.0);
      cairo_set_line_width(cr,2.0);
      
      cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(-43.,ry,yd)-10.);
      cairo_text_path(cr, text[5]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(-43.,ry,yd)-10.);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[5]);
      
      cairo_move_to(cr,nx(1750.,rx,xd),ny(y2_2,ry,yd));
      cairo_text_path(cr, text[6]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(1750.,rx,xd),ny(y2_2,ry,yd));
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[6]);
      
      cairo_text_extents (cr, text[7], &extents);
      cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		    ny(2005.,ry,yd)+extents.height+10.);
      cairo_text_path(cr, text[7]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		    ny(2005.,ry,yd)+extents.height+10.);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[7]);
      
      cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(2091.,ry,yd)-10.);
      cairo_text_path(cr, text[1]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(2091.,ry,yd)-10.);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[1]);
      
      cairo_move_to(cr,nx(1750.,rx,xd),ny(y1_2,ry,yd));
      cairo_text_path(cr, text[2]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(1750.,rx,xd),ny(y1_2,ry,yd));
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[2]);
      
      cairo_text_extents (cr, text[3], &extents);
      cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		    ny(4139.,ry,yd)+extents.height+10.);
      cairo_text_path(cr, text[3]);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		    ny(4139.,ry,yd)+extents.height+10.);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,text[3]);
      
      
      y1_2=3000.;
      y2_2=1000.;
      
      // ORDER
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      
      cairo_text_extents (cr, text[14], &extents);
      cairo_move_to(cr,xd-extents.width-5,ny(y2_1,ry,yd));
      cairo_show_text(cr,text[14]);
      
      cairo_text_extents (cr, text[12], &extents);
      cairo_move_to(cr,xd-extents.width-5,ny(y2_3,ry,yd));
      cairo_show_text(cr,text[12]);
      
      cairo_text_extents (cr, text[11], &extents);
      cairo_move_to(cr,xd-extents.width-5,ny(y1_1,ry,yd));
      cairo_show_text(cr,text[11]);
      
      cairo_text_extents (cr, text[9], &extents);
      cairo_move_to(cr,xd-extents.width-5,ny(y1_3,ry,yd));
      cairo_show_text(cr,text[9]);
      
      cairo_text_extents (cr, "WAVELENGTH (nm)", &extents);
      cairo_move_to(cr,nx(4096.,rx,xd)-extents.width,ny(4200.,ry,yd));
      cairo_show_text(cr,"WAVELENGTH (nm)");
      
      cairo_text_extents (cr, "ORDER", &extents);
      cairo_move_to(cr,xd-extents.width/2.,ny(4200.,ry,yd));
      cairo_show_text(cr,"ORDER");
      
      cairo_text_extents (cr, "CCD-1", &extents);
      cairo_move_to(cr,nx(0.,rx,xd)+10., ny(4139.,ry,yd)+10.+extents.height);
      cairo_text_path(cr,"CCD-1");
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(0.,rx,xd)+10., ny(4139.,ry,yd)+10.+extents.height);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,"CCD-1");
      
      cairo_text_extents (cr, "CCD-2", &extents);
      cairo_move_to(cr,nx(0.,rx,xd)+10., ny(2005.,ry,yd)+10.+extents.height);
      cairo_text_path(cr,"CCD-2");
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
      cairo_stroke(cr);
      cairo_move_to(cr,nx(0.,rx,xd)+10., ny(2005.,ry,yd)+10.+extents.height);
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
      cairo_show_text(cr,"CCD-2");

      xt_title=0.;
      yt_title=4700.;
      xt_cr=500.;
      xt_crs=2000.;
      yt_cr=4500.;
      xt_ec=500.;
      yt_ec=4350.;
      xt_det=2000.;
    }
    else {
      if(ncross==CROSS_RED){
	i1=56;
	i2=160;
      }
      else{
	i1=100;
	i2=199;
      }
      
      cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
      cairo_rectangle(cr,nx(xmin0,rx,xd), ny2(ymin0,ry,yd,ymin0),
		      (xmax0-xmin0)*rx,(ymin0-ymax0)*ry);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      cairo_rectangle(cr,nx(xmin0,rx,xd), ny2(ymin0,ry,yd,ymin0),
		      (xmax0-xmin0)*rx,(ymin0-ymax0)*ry);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
      cairo_fill(cr);

      cairo_set_line_width(cr,1.0);
      cairo_set_source_rgba(cr, 0, 0, 0, 1.0);

      for(i=i1;i<=i2;i++){
	x[0]=xfmin[i];
	y[0]=yfmin[i];
	x[1]=xfmax[i];
	y[1]=yfmax[i];
	//cpgline(2,(float *)x,(float *)y);
	cairo_move_to(cr,nx(x[0],rx,xd), ny2(y[0],ry,yd,ymin0));
	cairo_line_to(cr,nx(x[1],rx,xd), ny2(y[1],ry,yd,ymin0));
	cairo_stroke(cr);
      }

      xccd_min=-2050.+fcam*rad*theta_E_mes/pixsize;
      xccd_max=2050.+fcam*rad*theta_E_mes/pixsize;
      yccd1_min=-2091.;
      yccd1_max=-43.;
      yccd2_min=-yccd1_min;
      yccd2_max=-yccd1_max;
      
      cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
      cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd1_max,ry,yd,ymin0),
		      (xccd_max-xccd_min)*rx,(yccd1_max-yccd1_min)*ry);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd1_max,ry,yd,ymin0),
		      (xccd_max-xccd_min)*rx,(yccd1_max-yccd1_min)*ry);
      cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
      cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd2_max,ry,yd,ymin0),
		      (xccd_max-xccd_min)*rx,(yccd2_max-yccd2_min)*ry);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd2_max,ry,yd,ymin0),
		      (xccd_max-xccd_min)*rx,(yccd2_max-yccd2_min)*ry);
      cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
      cairo_fill(cr);

      // New Chip 0.15um
      /*
      cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
      cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd1_max,ry*0.15/0.135,yd,ymin0),
		      (xccd_max-xccd_min)*rx*0.15/0.135,(yccd1_max-yccd1_min)*ry*0.15/0.135);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd1_max,ry*0.15/0.135,yd,ymin0),
		      (xccd_max-xccd_min)*rx*0.15/0.135,(yccd1_max-yccd1_min)*ry*0.15/0.135);
      cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
      cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd2_max,ry*0.15/0.135,yd,ymin0),
		      (xccd_max-xccd_min)*rx*0.15/0.135,(yccd2_max-yccd2_min)*ry*0.15/0.135);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd2_max,ry*0.15/0.135,yd,ymin0),
		      (xccd_max-xccd_min)*rx*0.15/0.135,(yccd2_max-yccd2_min)*ry*0.15/0.135);
      cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
      cairo_fill(cr);
      */

      xt_wave=xmax0-500.;
      xt_ord=xmin0-500.;
      yt_ord=ymax0+100.;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 11.0);
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

      cairo_text_extents (cr, "WAVELENGTH (nm)", &extents);
      cairo_move_to(cr,nx(xmax0,rx,xd)-extents.width/2.,ny2(ymax0,ry,yd,ymin0)-extents.height/2.);
      cairo_show_text(cr,"WAVELENGTH (nm)");

      cairo_text_extents (cr, "ORDER", &extents);
      cairo_move_to(cr,nx(xmin0,rx,xd)-extents.width/2.,ny2(ymax0,ry,yd,ymin0)-extents.height/2.);
      cairo_show_text(cr,"ORDER");

      cairo_text_extents (cr, "CCD-1", &extents);
      cairo_move_to(cr,nx(xccd_min,rx,xd)-extents.width-5.,ny2(1000.,ry,yd,ymin0));
      cairo_show_text(cr,"CCD-1");
      
      cairo_text_extents (cr, "CCD-2", &extents);
      cairo_move_to(cr,nx(xccd_min,rx,xd)-extents.width-5.,ny2(-1000.,ry,yd,ymin0));
      cairo_show_text(cr,"CCD-2");

      cairo_set_font_size (cr, 10.0);

      for(i=i1;i<=i2;i++){
	imod=i-(int)(i/10)*10;
	if(imod==0){
	  xt_o=xfmin[i];
	  yt_o=yfmin[i];
	  xt_w=xfmax[i];
	  yt_w=yfmax[i];

	  sprintf(order,"%d",i);
	  sprintf(wave,"%6.1lf",wlfmax[i]*1.e+7);

	  cairo_text_extents (cr, order, &extents);
	  cairo_move_to(cr,nx(xt_o,rx,xd)-extents.width-5.,ny2(yt_o,ry,yd,ymin0));
	  cairo_show_text(cr,order);

	  cairo_move_to(cr,nx(xt_w,rx,xd),ny2(yt_w,ry,yd,ymin0));
	  cairo_show_text(cr,wave);
	}
      }     

      xt_cr=xmin0-500.;
      xt_crs=xmin0+3000.;
      yt_cr=ymax0+1000.;
      xt_ec=xt_cr-1000;
      yt_ec=ymax0+600.;
      xt_det=xt_crs;
      xt_title=xmin0-700.;
      yt_title=ymax0+1500.;
    }

  // ### Setting: ###
  if(hg->setup[hg->efs_setup].setup<0){  // None Std
    sprintf(tmp,"Setup-%d : NonStd-%d %dx%dbin",
	    hg->efs_setup+1,
	    -hg->setup[hg->efs_setup].setup,
	    hg->binning[hg->setup[hg->efs_setup].binning].x,
	    hg->binning[hg->setup[hg->efs_setup].binning].y);
  }
  else{
    sprintf(tmp,"Setup-%d : Std%s %dx%dbin",
	    hg->efs_setup+1,
	    setups[hg->setup[hg->efs_setup].setup].initial,
	    hg->binning[hg->setup[hg->efs_setup].binning].x,
	    hg->binning[hg->setup[hg->efs_setup].binning].y);
  }
  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 16.0);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  
  cairo_text_extents (cr, tmp, &extents);
  cairo_move_to(cr,dx,extents.height+5);
  cairo_show_text(cr,tmp);

  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 10.0);

  cairo_move_to(cr,dx,extents.height+5);

  if(ncross==CROSS_RED){
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_text_extents (cr, "Cross=RED", &extents);
    cairo_rel_move_to(cr,extents.width/5.,extents.height+5);
    cairo_show_text(cr,"Cross=RED");
  }
  else{
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_text_extents (cr, "Cross=BLUE", &extents);
    cairo_rel_move_to(cr,extents.width/5.,extents.height+5);
    cairo_show_text(cr,"Cross=BLUE");
  }

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  sprintf(tmp," / %s / %s / Cam-rot=%.4f[deg]",
	  crossrot,
	  th_ec,
	  det_rot+d_det_rot);
  cairo_show_text(cr,tmp);

  //   plot lines specified
  if(hg->efs_mode==EFS_PLOT_EFS){
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.8, 1.0);
    
    for(i=1;i<=nlines;i++){
      line_flag[i]=0;
      sprintf(line_txt[i],"%7.2lfA",wl[i]*10);
    }

    for(j=1;j<=3;j++){
      for(i=1;i<=nlines;i++){
	xpt[i]=xline[i][j];
	if(yline1[i][j]>0.0){
	  ypt[i]=4096.-yline1[i][j]+ccdgap;
	  if(xpt[i]>0){
	    if(line_flag[i]==0){
	      sprintf(tmp," : CCD1(%4.0lf, %4.0lf)",
		      ((4096+ccdgap)-ypt[i])/hg->binning[hg->setup[hg->efs_setup].binning].x,
		      xpt[i]/hg->binning[hg->setup[hg->efs_setup].binning].y);
	    }
	    else{
	      sprintf(tmp,"(%4.0lf, %4.0lf)",
		      ((4096+ccdgap)-ypt[i])/hg->binning[hg->setup[hg->efs_setup].binning].x,
		      xpt[i]/hg->binning[hg->setup[hg->efs_setup].binning].y);
	    }
	    strcat(line_txt[i],tmp);
	    line_flag[i]++;
	  }
  	}
	else if(yline2[i][j]>0.0){
	  ypt[i]=2048.-yline2[i][j]-ccdgap;
	  if(xpt[i]>0){
	    if(line_flag[i]==0){
	      sprintf(tmp," : CCD2(%4.0lf, %4.0lf)",
		      ((2048-ccdgap)-ypt[i])/hg->binning[hg->setup[hg->efs_setup].binning].x,
		      xpt[i]/hg->binning[hg->setup[hg->efs_setup].binning].y);
	    }
	    else{
	      sprintf(tmp,"(%4.0lf, %4.0lf)",
		      ((2048-ccdgap)-ypt[i])/hg->binning[hg->setup[hg->efs_setup].binning].x,
		      xpt[i]/hg->binning[hg->setup[hg->efs_setup].binning].y);
	    }
	    strcat(line_txt[i],tmp);
	    line_flag[i]++;
	  }
	}
      }

      cairo_set_line_width(cr,2.0);

      for(i=1;i<=nlines;i++){
	sprintf(tmp,"%d",i);
	
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, nx(xpt[i],rx,xd)-extents.width-5.,ny(ypt[i],ry,yd));
	cairo_text_path(cr,tmp);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr, nx(xpt[i],rx,xd)-extents.width-5.,ny(ypt[i],ry,yd));
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.8, 1.0);
	cairo_show_text(cr,tmp);
	cairo_arc(cr, 
		  nx(xpt[i],rx,xd),ny(ypt[i],ry,yd),
		  3,0, 2 * M_PI);
	cairo_fill(cr);
      
      }
 
    }

    ytext=0.;
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 13.0);
    cairo_text_extents (cr, "Line Name", &extents);
    yobj1=extents.height;

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 10.0);
    cairo_text_extents (cr, "Pixel Value", &extents);
    yobj2=extents.height;

    
    for(i=1;i<=nlines;i++){
      ytext=ytext+yobj1+7.;
      sprintf(tmp,"%2d. %s",i,line_name[i]);
      cairo_move_to(cr,nx(4150.,rx,xd),ny(4700.,ry,yd)+ytext);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size (cr, 13.0);
      cairo_show_text(cr,tmp);
 
      ytext=ytext+yobj2+4.;
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0);
      cairo_move_to(cr,nx(4250.,rx,xd),ny(4700.,ry,yd)+ytext);
      if(line_flag[i]!=0){
	cairo_show_text(cr,line_txt[i]);
      }
      else{
	sprintf(line_txt[i],"%7.2lfA : ---- ",wl[i]*10);
	cairo_show_text(cr,line_txt[i]);
      }
    }
  }

  
  }
  }
    

  if(hg->efs_output==EFS_OUTPUT_PDF){
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
  }

  cairo_destroy(cr);

  if(hg->efs_output==EFS_OUTPUT_WINDOW){
    gdk_draw_drawable(widget->window,
		      widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		      pixmap_efs,
		      0,0,0,0,
		      width,
		      height);
    
    g_object_unref(G_OBJECT(pixmap_efs));
  }

  return TRUE;
}



gdouble nx(gdouble x, gdouble rx, gdouble xd){
  return(x*rx+xd);
}

gdouble ny(gdouble y, gdouble ry, gdouble yd){
  gdouble rev_y=y*ry;

  return(-rev_y+yd);
}

gdouble ny2(gdouble y, gdouble ry, gdouble yd, gdouble y0){
  gdouble rev_y=(y-y0)*ry;

  return(-rev_y+yd);
}

static void refresh_efs (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->efs_output=EFS_OUTPUT_WINDOW;

  if(flagEFS){
    draw_efs_cairo(hg->efs_dw,NULL,
		   (gpointer)hg);
  }
}


void pdf_efs (typHOE *hg)
{
  hg->efs_output=EFS_OUTPUT_PDF;

  if(flagEFS){
    draw_efs_cairo(hg->efs_dw,NULL,
		    (gpointer)hg);
  }

  hg->efs_output=EFS_OUTPUT_WINDOW;
}


void cc_get_efs_mode (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->efs_mode=n;

    refresh_efs(widget, hg);
  }
}

