//    hoe : Subaru HDS OPE file Editor
//        calcpa.c :  Calculate Objects' Position & Plot
//                                           2018.02.14  A.Tajitsu

#include"main.h"
#include"calcpa.h"

// Function for calculation of atmospheric dispersion
double adrad(double zrad, double wlnm,double h,double t,double p,double f){
  double wlinv, c, en0m1, r0, r1;

  wlinv=1./wlnm;
  c=2371.34 + 683939.7/(130.0-wlinv*wlinv)+4547.3/(38.9-wlinv*wlinv);
  en0m1=1.0E-8*c*(p/t)*(1.0+p*(57.9E-8 - 9.325E-4/t+0.25844/(t*t)))
    *(1-0.16*f/p);
  r0=en0m1*(1-h);
  r1=0.5*en0m1*en0m1-en0m1*h;
  return(r0*tan(zrad) + r1*(tan(zrad)*tan(zrad)*tan(zrad)));
 
}


double new_tu(int iyear, int month, int iday){
  time_t t, t2000;
  struct tm tmpt,tmpt2000;
  gdouble itu;

  tmpt2000.tm_year=2000-1900;
  tmpt2000.tm_mon=0;
  tmpt2000.tm_mday=1;
  tmpt2000.tm_hour=0;
  tmpt2000.tm_min=0;
  tmpt2000.tm_sec=0;

  t2000=mktime(&tmpt2000);

  tmpt.tm_year=iyear-1900;
  tmpt.tm_mon=month-1;
  //tmpt.tm_mday=iday-1;
  tmpt.tm_mday=iday;
  tmpt.tm_hour=0;
  tmpt.tm_min=0;
  tmpt.tm_sec=0;

  t=mktime(&tmpt);

  itu=(t-t2000)/60./60./24./36525.;
  return(itu);
}


void calc_moon_plan(typHOE *hg){
  /* for Moon */
  double JD,JD_end;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_zonedate local_date;
  struct ln_rst_time rst;
  struct ln_zonedate set,rise;
  struct ln_hrz_posn hrz;
  gdouble d_t,d_ss;
  gint d_mm;
  gint i_pp;


  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day;

  local_date.hours=hg->atw18.s_set.hours;
  local_date.minutes=hg->atw18.s_set.minutes;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(hg->obs_timezone*60);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day+1;

  local_date.hours=hg->atw18.s_rise.hours;
  local_date.minutes=hg->atw18.s_rise.minutes;
  local_date.seconds=0.;

  JD_end = ln_get_julian_local_date(&local_date);
  i_pp=0;
  
  do{
    /* Lunar RA, DEC */
    ln_get_lunar_equ_coords (JD, &equ);

    ln_deg_to_hms(equ.ra, &hms);
    ln_deg_to_dms(equ.dec, &dms);
    
    hg->moon.p_ra[i_pp]=hms;
    hg->moon.p_dec[i_pp]=dms;

    ln_get_hrz_from_equ (&equ, &observer, JD, &hrz);
    if(hrz.az>180) hrz.az-=360;
    hg->moon.p_az[i_pp]=hrz.az;
    hg->moon.p_el[i_pp]=hrz.alt;
    
    i_pp++;
    JD+=30./60./24.;
  } while((JD<JD_end)&&(i_pp<MAX_PP));

  hg->i_pp_moon_max=i_pp;
}


typPlanMoon init_typPlanMoon(){
  /* for Moon */
  typPlanMoon moon;

  ln_deg_to_hms(0, &moon.ra);
  ln_deg_to_dms(0, &moon.dec);
    
  moon.az=0;
  moon.el=0;
  moon.disk =0;
  moon.phase=0;
  moon.limb =0;

  moon.age  =-1;
  moon.sep  =-1;
  
  return(moon);
}


typPlanMoon calc_typPlanMoon(typHOE *hg, glong sod, gdouble az, gdouble el){
  /* for Moon */
  typPlanMoon moon;
  double JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ;
  struct ln_zonedate local_date;
  struct ln_hrz_posn hrz;
  gdouble d_t,d_ss;
  gint d_mm;
  gint i_pp;


  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day;

  local_date.hours=0;
  local_date.minutes=0;
  local_date.seconds=1.;

  local_date.gmtoff=(long)(hg->obs_timezone*60);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);
  JD += (gdouble)sod/60./60./24.;

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &moon.ra);
  ln_deg_to_dms(equ.dec, &moon.dec);
    
  ln_get_hrz_from_equ (&equ, &observer, JD, &hrz);
  if(hrz.az>180) hrz.az-=360;
  moon.az=hrz.az;
  moon.el=hrz.alt;

  moon.disk =ln_get_lunar_disk(JD);
  moon.phase=ln_get_lunar_phase(JD);
  moon.limb =ln_get_lunar_bright_limb(JD);

  moon.age  =(moon.limb>180) ? 
    (180-moon.phase)/360.*29.5 :      // Jo-gen
    moon.phase/360.*29.5 + 29.5/2.;   // Ka-gen

  if((el>0)&&(moon.el>0)){
    moon.sep=deg_sep(az, el, moon.az, moon.el);
  }
  else{
    moon.sep=-1;
  }
  
  return(moon);
}

gdouble get_moon_age(gdouble JD){
  gdouble phase, limb, age;
  
  phase=ln_get_lunar_phase(JD);
  limb =ln_get_lunar_bright_limb(JD);

  age  =(limb>180) ? 
    (180-phase)/360.*29.5 :      // Jo-gen
    phase/360.*29.5 + 29.5/2.;   // Ka-gen

  return(age);
}


gdouble get_moon_sep(gdouble paz, gdouble pel, gdouble JD_hst){
  gint i, i_moon=-1;
  gdouble d_jd1, d_jd2, d_jd;
  gdouble az_moon, el_moon;

  for (i=0;i<i_moon_max-1;i++){
    if(jd_moon[i]<JD_hst){
      if(JD_hst<jd_moon[i+1]){
	i_moon=i;
	break;
      }
    }
  }

  if(i_moon<=0) return(-1);
  if(i_moon>=i_moon_max-1) return(-1);

  d_jd1=JD_hst-jd_moon[i_moon];
  d_jd2=jd_moon[i_moon+1]-JD_hst;

  d_jd=d_jd1+d_jd2;
  az_moon=paz_moon[i_moon]*d_jd2/d_jd
    +paz_moon[i_moon+1]*d_jd1/d_jd;
  el_moon=pel_moon[i_moon]*d_jd2/d_jd
    +pel_moon[i_moon+1]*d_jd1/d_jd;

  if(el_moon<0) return(-1);

  return(deg_sep(paz,pel,az_moon,el_moon));
}


// See http://www.stjarnhimlen.se/comp/ppcomp.html#13
//     http://chiron.blog.ocn.ne.jp/astrocal/2012/11/de_e827.html
void calc_moon_topocen(typHOE *hg, 
		       gdouble JD,
		       struct ln_equ_posn * equ_geoc,
		       struct ln_equ_posn * equ)
{
  gdouble mpar, gclat, rho, HA, g, topRA, topDec, d_moon;
  gdouble r_earth=6378.137;
  gdouble flst;
  gdouble N, e2;
  gdouble f=1./298.257222101;
  
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24/360;
  
  d_moon=ln_get_lunar_earth_dist(JD);  //km
  mpar=asin((r_earth+hg->obs_altitude/1000.)/d_moon)*180./M_PI; //degree
  gclat=(hg->obs_latitude 
	 - 0.1924*sin(2*hg->obs_latitude*M_PI/180.))*M_PI/180.; //rad
  rho  =0.99833 + 0.00167 * cos(2*hg->obs_latitude*M_PI/180.)
        + hg->obs_altitude/1000./r_earth; // r_earth
  HA = flst - (equ_geoc->ra * 24./360.); // hour
  g = atan(tan(gclat) / cos(HA*M_PI/12.)); // rad
    
  equ->ra = equ_geoc->ra - mpar * rho * cos(gclat)
    * sin(HA*M_PI/12.) / cos(equ_geoc->dec*M_PI/180.);  // degree
  if(equ_geoc->dec>89.99){
    equ->dec = equ_geoc->dec - mpar * rho * sin(-equ_geoc->dec*M_PI/180.) 
      *cos(HA*M_PI/12.);  // degree
  }
  else{
    equ->dec = equ_geoc->dec - mpar * rho * sin(gclat) *
      sin(g-equ_geoc->dec*M_PI/180.) / sin(g);  // degree
  }
}


void close_plot(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;


  gtk_widget_destroy(GTK_WIDGET(hg->plot_main));
  flagPlot=FALSE;
}

#ifdef USE_GTK3
gboolean draw_plot_cb(GtkWidget *widget,
			cairo_t *cr, 
			gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixbuf_plot) draw_plot_cairo(widget,hg);
  gdk_cairo_set_source_pixbuf(cr, pixbuf_plot, 0, 0);
  cairo_paint(cr);
  return(TRUE);
}

#else
gboolean expose_plot_cairo(GtkWidget *widget,
			   GdkEventExpose *event, 
			   gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixmap_plot) draw_plot_cairo(hg->plot_dw,hg);
  
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    GtkStyle *style=gtk_widget_get_style(widget);
    gtk_widget_get_allocation(widget,allocation);
    
    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_plot,
		      0,0,0,0,
		      allocation->width,
		      allocation->height);
    g_free(allocation);
  }

  return (TRUE);
}
#endif

gboolean configure_plot_cb(GtkWidget *widget,
			GdkEventConfigure *event, 
			gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  draw_plot_cairo(widget,hg);
  return(TRUE);
}


void do_plot_moon (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  hg->plot_moon=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,hg);
  }
}

void do_plot_pam (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  hg->plot_pam=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,hg);
  }
}


void cc_get_plot_mode (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_mode=n;

    refresh_plot(widget, hg);
  }
}


void cc_get_plot_all (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_all=n;

    refresh_plot(widget, hg);
  }
}


gdouble set_ul(gdouble lower, gdouble input, gdouble upper){
  gdouble step;
  step=upper-lower;

  if(input<lower){
    do{
      input+=step;
    }while(input<lower);
  }
  else if(input>=upper){
    do{
      input-=step;
    }while(input>=upper);
  }

  return input;
}


gdouble hdspa_deg(gdouble phi, gdouble dec, gdouble ha){
  gdouble pdeg, zdeg;

  pdeg=atan2((tan(phi)*cos(dec)-sin(dec)*cos(ha)),sin(ha))*180./M_PI;
  zdeg=acos(sin(phi)*sin(dec)+cos(phi)*cos(dec)*cos(ha))*180./M_PI;

  return(set_ul(-180., -(pdeg-zdeg)+HDS_PA_OFFSET, 180.));
}


//////////////////////////////////////////////////////////////////
/////// global functions
//////////////////////////////////////////////////////////////////

void calcpa2_main(typHOE* hg){
  double JD, JD_hst;
  struct ln_lnlat_posn observer;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gdouble objd;
  struct ln_hrz_posn hrz;

  double  phi;     //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  int iyear;
  int month;
  int iday;
  int min,hour;
  gdouble sec;
  double a0,d0,d0rad;
  double ut;
  double ut_offset;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i_list;
  time_t tt;

  struct ln_zonedate zonedate;
  struct ln_date date;
  gdouble dAz, dEl, dAz1, dAz2;
  gint i_min_c_rt=-1;

  double ha1rad_1;
  
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  ln_get_date_from_sys(&date);
  ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*60);
  JD = ln_get_julian_local_date(&zonedate);

  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24./360.;
  flst=set_ul(0., flst, 24.);

  hg->lst_hour=(gint)flst;
  hg->lst_min=(gint)((flst-(double)hg->lst_hour)*60.);
  hg->lst_sec=((flst-(double)hg->lst_hour-(double)hg->lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].i_nst>=0){ //Non-Sidereal
      if(hg->nst[hg->obj[i_list].i_nst].eph[0].jd>JD){
	hg->obj[i_list].ra=hg->nst[hg->obj[i_list].i_nst].eph[0].ra;
	hg->obj[i_list].dec=hg->nst[hg->obj[i_list].i_nst].eph[0].dec;
	hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[0].equinox;
	hg->nst[hg->obj[i_list].i_nst].c_fl=-1;
      }
      else if(hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].jd<JD){
	hg->obj[i_list].ra=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].ra;
	hg->obj[i_list].dec=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].dec;
	hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].equinox;
	hg->nst[hg->obj[i_list].i_nst].c_fl=1;
      }
      else{
	gint i;
	gdouble dJD, dJD1, ra1,ra2,dec1,dec2,dra,ddec;

	for(i=0;i<hg->nst[hg->obj[i_list].i_nst].i_max;i++){
	  if(hg->nst[hg->obj[i_list].i_nst].eph[i].jd>JD){
	    dJD=hg->nst[hg->obj[i_list].i_nst].eph[i].jd
	      -hg->nst[hg->obj[i_list].i_nst].eph[i-1].jd;
	    dJD1=JD-hg->nst[hg->obj[i_list].i_nst].eph[i-1].jd;
	    ra1=ra_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i-1].ra);
	    ra2=ra_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i].ra);
	    dec1=dec_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i-1].dec);
	    dec2=dec_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i].dec);
	    dra=ra2-ra1;
	    ddec=dec2-dec1;
	    
	    hg->obj[i_list].ra=deg_to_ra(ra1+dra*dJD1/dJD);
	    hg->obj[i_list].dec=deg_to_dec(dec1+ddec*dJD1/dJD);

	    hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[i-1].equinox;
	    hg->nst[hg->obj[i_list].i_nst].c_fl=0;
	    break;
	  }
	}
      }
    } // Non-Sidereal

    object.ra=ra_to_deg(hg->obj[i_list].ra);
    object.dec=dec_to_deg(hg->obj[i_list].dec);

    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->obj[i_list].equinox),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->obj[i_list].c_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    a0=ln_hms_to_deg(&hobject_prec.ra)/360.*24.; //[hour]
    d0rad=ln_dms_to_rad(&hobject_prec.dec);

    ha=flst-a0;             //hour angle [hour]
    ha=set_ul(-12., ha, 12.);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=hrz.alt*M_PI/180.;
    az0=(180.+hrz.az)*M_PI/180.;
    if(az0<0.) az0=az0+2.*M_PI;

    if(hrz.alt<0.0){
      hg->obj[i_list].c_az=-1;
      hg->obj[i_list].c_el=-1;
      hg->obj[i_list].c_rt=-1;
      hg->obj[i_list].c_ha=-99;
      hg->obj[i_list].c_pa=-180;
      hg->obj[i_list].c_ad=-1;
      hg->obj[i_list].c_vaz=-100;
      hg->obj[i_list].c_vpa=-100;
      hg->obj[i_list].c_sep=-1;
      hg->obj[i_list].c_hpa=0;
      hg->obj[i_list].c_vhpa=-100;
    }
    else{
      el0d=el0+M_PI*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./M_PI;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./M_PI;
      
      ha1=set_ul(-12., ha1, 12.);
      a1=flst-ha1;
      a1=set_ul(0., a1, 24.);
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*M_PI/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/M_PI;

      {
	struct ln_hrz_posn hrz_1;
	double flst_1, ha_1;
	double el0_1, az0_1, el0d_1, d1_1, d1rad_1;
	double ume1_1, den1_1;
	double ha1_1, a1_1;
	double delta_a_1, delta_d_1, pa_1, padeg_1;

	flst_1 = ln_get_mean_sidereal_time(JD+1./60./24.)
	  + hg->obs_longitude *24/360;
	flst_1=set_ul(0., flst_1, 24.);
	ln_get_hrz_from_equ (&object_prec, &observer, JD+1./60./24., &hrz_1);

	ha_1=flst_1-a0;             //hour angle [hour]
	ha_1=set_ul(-12., ha_1, 12.);

	el0_1=(gdouble)hrz_1.alt*M_PI/180.;
	az0_1=(180.+(gdouble)hrz_1.az)*M_PI/180.;
	if(az0_1<0.) az0_1=az0_1+2.*M_PI;

	el0d_1=el0_1+M_PI*4./3600./180.;
	d1rad_1=asin(sinphi*sin(el0d_1)+cosphi*cos(az0_1)*cos(el0d_1));
	d1_1=d1rad_1*180./M_PI;
	ume1_1=-sin(az0_1)*cos(el0d_1);
	den1_1=cosphi*sin(el0d_1)-sinphi*cos(az0_1)*cos(el0d_1);
	ha1rad_1=atan2(ume1_1,den1_1);
	ha1_1=ha1rad_1*12./M_PI;
	
	ha1_1=set_ul(-12., ha1_1, 12.);
	a1_1=flst_1-ha1_1;
	a1_1=set_ul(0., a1_1, 24.);

	delta_a_1=(a1_1-a0)*M_PI/12.;   //[rad]
	delta_d_1=d1rad_1-d0rad;      //[rad]
	pa_1=atan2(delta_a_1,delta_d_1);      
	padeg_1=180.*pa_1/M_PI;
	

	hg->obj[i_list].c_vaz=hrz_1.az-hrz.az;
	hg->obj[i_list].c_vaz=set_ul(-180.,hg->obj[i_list].c_vaz,180.);
	hg->obj[i_list].c_vpa=padeg_1-padeg;
	hg->obj[i_list].c_vpa=set_ul(-180.,hg->obj[i_list].c_vpa,180.);
      }

      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=M_PI*(90.0-hrz.alt)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      if(hrz.alt>3){
	if(hg->wave1<hg->wave0){
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
	else{
	  adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
      }
      else{
	adsec=100;
      }
      hst=ut+ut_offset;


      if(hrz.az>180) hrz.az-=360;
      hg->obj[i_list].c_az=hrz.az;
      hg->obj[i_list].c_el=hrz.alt;
      hg->obj[i_list].c_ha=ha1;
      hg->obj[i_list].c_pa=padeg;
      hg->obj[i_list].c_ad=adsec;

      // HDS PA w/o ImR
      hg->obj[i_list].c_hpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
      hg->obj[i_list].c_vhpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad_1)
	-hg->obj[i_list].c_hpa;
      hg->obj[i_list].c_vhpa=set_ul(-180.,hg->obj[i_list].c_vhpa,180.);


#ifdef USE_XMLRPC
      if(hg->telstat_flag){
	if( hg->obj[i_list].c_el<0 ){
	  hg->obj[i_list].c_rt=-1;
	}
	else if( hg->obj[i_list].c_az>90 ){
	  dAz1 = fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	  dAz2 = fabs(hg->stat_az -hg->pa_a0 - (hg->obj[i_list].c_az-360));
	  dAz = (dAz1>dAz2) ? dAz2 : dAz1;
	}
	else if ( hg->obj[i_list].c_az<-90 ){
	  dAz1 = fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	  dAz2 = fabs(hg->stat_az -hg->pa_a0 - (hg->obj[i_list].c_az+360));
	  dAz = (dAz1>dAz2) ? dAz2 : dAz1;
	}
	else{
	  dAz=fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	}
	dEl=fabs(hg->stat_el -hg->pa_a1 - hg->obj[i_list].c_el);
	
	if( (dAz/hg->vel_az) > (dEl/hg->vel_el) )
	  hg->obj[i_list].c_rt=dAz/hg->vel_az;
	else
	  hg->obj[i_list].c_rt=dEl/hg->vel_el;

	if(i_min_c_rt==-1){
	  i_min_c_rt=i_list;
	}
	else if(hg->obj[i_list].c_rt < hg->obj[i_min_c_rt].c_rt){
	  i_min_c_rt=i_list;
	}
      }
      else{
	hg->obj[i_list].c_rt=-1;
      }

      hg->obj[i_list].check_lock=FALSE;
#endif      
    }
  }

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    object.ra=ra_to_deg(hg->std[i_list].ra);
    object.dec=dec_to_deg(hg->std[i_list].dec);

    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->std[i_list].equinox),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->std[i_list].c_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    if(hrz.alt<0.0){
      hg->std[i_list].c_az=-1;
      hg->std[i_list].c_el=-1;
    }
    else{
      if(hrz.az>180) hrz.az-=360;
      hg->std[i_list].c_az=hrz.az;
      hg->std[i_list].c_el=hrz.alt;
    }
  }

#ifdef USE_XMLRPC
  if(hg->auto_check_lock){
    if(hg->telstat_flag){
      if(i_min_c_rt!=-1){
	if(hg->obj[i_min_c_rt].c_rt<2){
	  hg->obj[i_min_c_rt].check_lock=TRUE;
	}
      }
    }
  }
#endif

  calc_moon(hg);

  {
    hobject.ra=hg->moon.c_ra;
    hobject.dec=hg->moon.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->moon.c_az=-1;
      hg->moon.c_el=-1;

      hg->obj[i_list].c_sep=-1;
    }
    else{
      
      hg->moon.c_az=hrz.az;
      hg->moon.c_el=hrz.alt;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(hg->obj[i_list].c_el>0){
	  hg->obj[i_list].c_sep=deg_sep(hg->obj[i_list].c_az,
					hg->obj[i_list].c_el,
					hg->moon.c_az,
					hg->moon.c_el);
	}
	else{
	  hg->obj[i_list].c_sep=-1;
	}
      }
    }
  }

  {

    hobject.ra=hg->sun.c_ra;
    hobject.dec=hg->sun.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->sun.c_az=-1;
      hg->sun.c_el=-1;
    }
    else{
      
      hg->sun.c_az=hrz.az;
      hg->sun.c_el=hrz.alt;
    }
  }

  objtree_update_radec(hg);
  update_c_label(hg);
}


void calcpa2_skymon(typHOE* hg){
  double JD;
  struct ln_lnlat_posn observer;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gdouble objd;
  struct ln_hrz_posn hrz;
  
  double  phi;     //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  int iyear;
  int month;
  int iday;
  int hour,min;
  gdouble sec;
  double a0,d0,a0rad,d0rad;
  double ut;
  double ut_offset;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i_list;

  double ha1rad_1;

  struct ln_zonedate zonedate;
  struct ln_date date;
  
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  zonedate.years=hg->skymon_year;
  zonedate.months=hg->skymon_month;
  zonedate.days=hg->skymon_day;
  zonedate.hours=hg->skymon_hour;
  zonedate.minutes=hg->skymon_min;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)hg->obs_timezone*60;

  JD = ln_get_julian_local_date(&zonedate);
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24/360;

  flst=set_ul(0., flst, 24.);

  hg->skymon_lst_hour=(gint)flst;
  hg->skymon_lst_min=(gint)((flst-(double)hg->skymon_lst_hour)*60.);
  hg->skymon_lst_sec=((flst-(double)hg->skymon_lst_hour-(double)hg->skymon_lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].i_nst>=0){ //Non-Sidereal
      if(hg->nst[hg->obj[i_list].i_nst].eph[0].jd>JD){
	hg->obj[i_list].ra=hg->nst[hg->obj[i_list].i_nst].eph[0].ra;
	hg->obj[i_list].dec=hg->nst[hg->obj[i_list].i_nst].eph[0].dec;
	hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[0].equinox;
	hg->nst[hg->obj[i_list].i_nst].s_fl=-1;
      }
      else if(hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].jd<JD){
	hg->obj[i_list].ra=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].ra;
	hg->obj[i_list].dec=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].dec;
	hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[hg->nst[hg->obj[i_list].i_nst].i_max-1].equinox;
	hg->nst[hg->obj[i_list].i_nst].s_fl=1;
      }
      else{
	gint i;
	gdouble dJD, dJD1, ra1,ra2,dec1,dec2,dra,ddec;

	for(i=0;i<hg->nst[hg->obj[i_list].i_nst].i_max;i++){
	  if(hg->nst[hg->obj[i_list].i_nst].eph[i].jd>JD){
	    dJD=hg->nst[hg->obj[i_list].i_nst].eph[i].jd
	      -hg->nst[hg->obj[i_list].i_nst].eph[i-1].jd;
	    dJD1=JD-hg->nst[hg->obj[i_list].i_nst].eph[i-1].jd;
	    ra1=ra_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i-1].ra);
	    ra2=ra_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i].ra);
	    dec1=dec_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i-1].dec);
	    dec2=dec_to_deg(hg->nst[hg->obj[i_list].i_nst].eph[i].dec);
	    dra=ra2-ra1;
	    ddec=dec2-dec1;

	    hg->obj[i_list].ra=deg_to_ra(ra1+dra*dJD1/dJD);
	    hg->obj[i_list].dec=deg_to_dec(dec1+ddec*dJD1/dJD);

	    hg->obj[i_list].equinox=hg->nst[hg->obj[i_list].i_nst].eph[i-1].equinox;
	    hg->nst[hg->obj[i_list].i_nst].s_fl=0;
	    break;
	  }
	}
      }
    } // Non-Sidereal

    object.ra=ra_to_deg(hg->obj[i_list].ra);
    object.dec=dec_to_deg(hg->obj[i_list].dec);

    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->obj[i_list].equinox),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->obj[i_list].s_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    a0=ln_hms_to_deg(&hobject_prec.ra)/360.*24.; //[hour]
    a0rad=ln_hms_to_rad(&hobject_prec.ra);
    d0rad=ln_dms_to_rad(&hobject_prec.dec);

    ha=flst-a0;             //hour angle [hour]
    ha=set_ul(-12., ha, 12.);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=hrz.alt*M_PI/180.;
    az0=(180.+hrz.az)*M_PI/180.;
    if(az0<0.) az0=az0+2.*M_PI;

    if(hrz.alt<0.0){
      hg->obj[i_list].s_az=-1;
      hg->obj[i_list].s_el=-1;
      hg->obj[i_list].s_ha=-99;
      hg->obj[i_list].s_pa=-180;
      hg->obj[i_list].s_ad=-1;
      hg->obj[i_list].s_vpa=-100;
      hg->obj[i_list].s_vaz=-100;
      hg->obj[i_list].s_sep=-1;
      hg->obj[i_list].s_hpa=-180;     
      hg->obj[i_list].s_hpa=-100;     
    }
    else{
      
      //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
      el0d=el0+M_PI*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./M_PI;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./M_PI;

      ha1=set_ul(-12., ha1, 12.);
      a1=flst-ha1;
      a1=set_ul(0., a1, 24.);
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*M_PI/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/M_PI;
      
      {
	struct ln_hrz_posn hrz_1;
	double flst_1, ha_1;
	double el0_1, az0_1, el0d_1, d1_1, d1rad_1;
	double ume1_1, den1_1;
	double ha1_1, a1_1;
	double delta_a_1, delta_d_1, pa_1, padeg_1;

	flst_1 = ln_get_mean_sidereal_time(JD+1./60./24.)
	  + hg->obs_longitude *24/360;
	flst_1=set_ul(0., flst_1, 24.);
	ln_get_hrz_from_equ (&object_prec, &observer, JD+1./60./24., &hrz_1);

	ha_1=flst_1-a0;             //hour angle [hour]
	ha_1=set_ul(-12., ha_1, 12.);

	el0_1=(gdouble)hrz_1.alt*M_PI/180.;
	az0_1=(180.+(gdouble)hrz_1.az)*M_PI/180.;
	if(az0_1<0.) az0_1=az0_1+2.*M_PI;

	el0d_1=el0_1+M_PI*4./3600./180.;
	d1rad_1=asin(sinphi*sin(el0d_1)+cosphi*cos(az0_1)*cos(el0d_1));
	d1_1=d1rad_1*180./M_PI;
	ume1_1=-sin(az0_1)*cos(el0d_1);
	den1_1=cosphi*sin(el0d_1)-sinphi*cos(az0_1)*cos(el0d_1);
	ha1rad_1=atan2(ume1_1,den1_1);
	ha1_1=ha1rad_1*12./M_PI;
	
	ha1_1=set_ul(-12., ha1_1, 12.);
	a1_1=flst_1-ha1_1;
	a1_1=set_ul(0., a1_1, 24.);

	delta_a_1=(a1_1-a0)*M_PI/12.;   //[rad]
	delta_d_1=d1rad_1-d0rad;      //[rad]
	pa_1=atan2(delta_a_1,delta_d_1);      
	padeg_1=180.*pa_1/M_PI;

	hg->obj[i_list].s_vaz=hrz_1.az-hrz.az;
	hg->obj[i_list].s_vaz=set_ul(-180.,hg->obj[i_list].s_vaz,180.);
	hg->obj[i_list].s_vpa=padeg_1-padeg;
	hg->obj[i_list].s_vpa=set_ul(-180.,hg->obj[i_list].s_vpa,180.);
      }
    
      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=M_PI*(90.0-hrz.alt)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      if(hrz.alt>3){
	if(hg->wave1<hg->wave0){
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
	else{
	  adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
      }
      else{
	adsec=100;
      }

      hst=ut+ut_offset;
      

      if(hrz.az>180) hrz.az-=360;
      hg->obj[i_list].s_az=hrz.az;
      hg->obj[i_list].s_el=hrz.alt;
      hg->obj[i_list].s_ha=ha1;
      hg->obj[i_list].s_pa=padeg;
      hg->obj[i_list].s_ad=adsec;

      // HDS PA w/o ImR
      hg->obj[i_list].s_hpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
      hg->obj[i_list].s_vhpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad_1)
	-hg->obj[i_list].s_hpa;
      hg->obj[i_list].s_vhpa=set_ul(-180.,hg->obj[i_list].s_vhpa,180.);
    }
  }

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    object.ra=ra_to_deg(hg->std[i_list].ra);
    object.dec=dec_to_deg(hg->std[i_list].dec);

    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->std[i_list].equinox),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->std[i_list].s_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    if(hrz.alt<0.0){
      hg->std[i_list].s_az=-1;
      hg->std[i_list].s_el=-1;
    }
    else{
      if(hrz.az>180) hrz.az-=360;
      hg->std[i_list].s_az=hrz.az;
      hg->std[i_list].s_el=hrz.alt;
    }
  }

  calc_moon_skymon(hg);

  {
    hobject.ra=hg->moon.s_ra;
    hobject.dec=hg->moon.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->moon.s_az=-1;
      hg->moon.s_el=-1;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	hg->obj[i_list].s_sep=-1;
      }
    }
    else{
      
      hg->moon.s_az=hrz.az;
      hg->moon.s_el=hrz.alt;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(hg->obj[i_list].s_el>0){
	  hg->obj[i_list].s_sep=deg_sep(hg->obj[i_list].s_az,
					hg->obj[i_list].s_el,
					hg->moon.s_az,
					hg->moon.s_el);
	}
	else{
	  hg->obj[i_list].s_sep=-1;
	}
      }
    }
  }

  {
    hobject.ra=hg->sun.s_ra;
    hobject.dec=hg->sun.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->sun.s_az=-1;
      hg->sun.s_el=-1;
    }
    else{
      
      hg->sun.s_az=hrz.az;
      hg->sun.s_el=hrz.alt;
      
    }
  }

  if(hg->init_flag){
    objtree_update_radec(hg);
    update_c_label(hg);
  }
}


void calcpa2_plan(typHOE* hg){
  
  double  pi=3.141592;
  double  alambda=hg->obs_longitude;  //longitude[deg]
  double  alamh=alambda/360.*24.;    //[hour]
  double  phi=hg->obs_latitude;      //latitude [deg]      
  double  sinphi=sin(pi*phi/180.0);
  double  cosphi=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char *c;
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday, iday0;
  int ihst0=18, ihst1=30;
  double a0,d0,a0rad,d0rad;
  double tu0,gmst0,gmst1,gmst;
  double ut;
  double flmst, flst, ha, sinha, cosha;
  double el0, ume, den, az0, el0deg, az0deg;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i_plan,i_pp=0;
  gboolean flag_start;
  

  iyear=hg->fr_year;
  month=hg->fr_month;
  iday0=hg->fr_day;

  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if((hg->plan[i_plan].type==PLAN_TYPE_OBJ)&&(!hg->plan[i_plan].backup)){
      
      ut=hg->plan[i_plan].sod/3600.-(24.+hg->obs_timezone/60.);
      flag_start=TRUE;

      do{
	if(ut<0){
	  iday=iday0+1;    // HST18:00 -> UT4:00 on next day
	}
	else if(ut>24){
	  break;
	}
	else{
	  iday=iday0;
	}

	tu0=new_tu(iyear,month,iday);
	gmst0=24110.54841/3600.;
	gmst1=8640184.812866*tu0 + 0.093104*tu0*tu0 - 0.0000062*tu0*tu0*tu0;
	gmst=gmst0 + gmst1/3600.;  //[hour]
	if(gmst>=24.){
	  do{
	    gmst=gmst-24.;
	  }while(gmst>=24.);
	}
	else if(gmst<0.){
	  do{
	    gmst=gmst+24.;
	  }while(gmst<0.);
	}

	flmst=gmst+ut+alamh;
	flst=flmst;
	if(flst<0.){
	  do{
	    flst=flst+24.0;
	  }while(flst<0.);
	}
	else if(flmst>=24.){
	  do{
	    flst=flst-24.0;
	  }while(flst>=24.);
	}
	else{
	  flst=flmst;
	}

	a0s=hg->obj[hg->plan[i_plan].obj_i].ra;
	ia0h=(int)(a0s/10000);
	a0s=a0s-(double)(ia0h)*10000;
	ia0m=(int)(a0s/100);
	a0s=a0s-(double)(ia0m)*100;
    
	d0s=hg->obj[hg->plan[i_plan].obj_i].dec;
	id0d=(int)(d0s/10000);
	d0s=d0s-(double)(id0d)*10000;
	id0m=(int)(d0s/100);
	d0s=d0s-(double)(id0m)*100;


	a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
	d0=id0d + id0m/60. + d0s/3600.;  //[deg]
	a0rad=pi*a0/12.;  //[rad]
	d0rad=pi*d0/180.; //[rad]
    
	ha=flst-a0;             //hour angle [hour]
	ha=set_ul(-12., ha, 12.);
	
	sinha=sin(M_PI*ha/12.0);
	cosha=cos(M_PI*ha/12.0);
    
	//### 2nd step: (RA,Dec) -> (AZ,EL)
	el0=asin(sinphi*sin(d0rad)+cosphi*cosha*cos(d0rad)); // elevation [rad]
	ume=-sinha*cos(d0rad);
	den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	az0=atan2(ume,den);     // azimath [rad]      
	if(az0<0.) az0=az0+2.*M_PI;
	el0deg=el0*180./M_PI;
	az0deg=az0*180./M_PI;

	//### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	el0d=el0+M_PI*4./3600./180.;
	d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
	d1=d1rad*180./M_PI;
	ume1=-sin(az0)*cos(el0d);
	den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
	ha1rad=atan2(ume1,den1);
	ha1=ha1rad*12./M_PI;
	ha1=set_ul(-12., ha1, 12.);
	a1=flst-ha1;
	a1=set_ul(0., ha1, 24.);
	
	
	//### 4th step: (RA1-RA,Dec1-Dec) => PA
	delta_a=(a1-a0)*M_PI/12.;   //[rad]
	delta_d=d1rad-d0rad;      //[rad]
	pa=atan2(delta_a,delta_d);      
	padeg=180.*pa/M_PI;
	
	
	//### 5th step: Atmospheric Dispersion at 3500A
	zrad=pi*(90.0-el0deg)/180.;
	ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		  (double)hg->pres,f);  //@3500A default
	ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		  (double)hg->pres,f);  //@5500A default
	adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	hst=ut+14.0;
	
	
	hg->pp[i_pp].az=az0deg-180;
	hg->pp[i_pp].el=el0deg;
	hg->pp[i_pp].ut=ut;
	
	hg->pp[i_pp].i_plan=i_plan;
	hg->pp[i_pp].start=flag_start;
	/*}*/
	
	ut+=0.01;
	flag_start=FALSE;
	i_pp++;
      }while((ut<(hg->plan[i_plan].sod+hg->plan[i_plan].time)/3600.-(24.+hg->obs_timezone/60.))
	     &&(i_pp<MAX_PP));

    }
  }
  hg->i_pp_max=i_pp;

  calc_sun_plan(hg);
  calc_moon_plan(hg);

}


void calc_moon(typHOE *hg){
  /* for Moon */
  double JD, JD_adj;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ, equ_geoc;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_rst_time rst, rst0, rst1;
  struct ln_date date;
  struct ln_zonedate set,rise, zonedate;

  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;
        
  /* get the julian day from the local system time */
  JD = ln_get_julian_from_sys();

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ_geoc);  //geocentric
  calc_moon_topocen(hg, JD, &equ_geoc, &equ);
  
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.c_ra=hms;
  hg->moon.c_dec=dms;

  ln_get_solar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->sun.c_ra=hms;
  hg->sun.c_dec=dms;

  ln_get_mercury_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mercury.c_ra=hms;
  hg->mercury.c_dec=dms;
  hg->mercury.c_mag=ln_get_mercury_magnitude (JD);

  ln_get_venus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->venus.c_ra=hms;
  hg->venus.c_dec=dms;
  hg->venus.c_mag=ln_get_venus_magnitude (JD);

  ln_get_mars_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mars.c_ra=hms;
  hg->mars.c_dec=dms;
  hg->mars.c_mag=ln_get_mars_magnitude (JD);

  ln_get_jupiter_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->jupiter.c_ra=hms;
  hg->jupiter.c_dec=dms;
  hg->jupiter.c_mag=ln_get_jupiter_magnitude (JD);

  ln_get_saturn_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->saturn.c_ra=hms;
  hg->saturn.c_dec=dms;
  hg->saturn.c_mag=ln_get_saturn_magnitude (JD);

  ln_get_uranus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->uranus.c_ra=hms;
  hg->uranus.c_dec=dms;
  hg->uranus.c_mag=ln_get_uranus_magnitude (JD);

  ln_get_neptune_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->neptune.c_ra=hms;
  hg->neptune.c_dec=dms;
  hg->neptune.c_mag=ln_get_neptune_magnitude (JD);

  ln_get_pluto_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->pluto.c_ra=hms;
  hg->pluto.c_dec=dms;
  hg->pluto.c_mag=ln_get_pluto_magnitude (JD);

  hg->moon.c_disk=ln_get_lunar_disk(JD);
  hg->moon.c_phase=ln_get_lunar_phase(JD);
  hg->moon.c_limb=ln_get_lunar_bright_limb(JD);

  hg->moon.c_age=(hg->moon.c_limb>180) ? 
    (180-hg->moon.c_phase)/360.*29.5 :     // Jo-gen
    hg->moon.c_phase/360.*29.5 + 29.5/2.;  // Ka-gen

  if (ln_get_lunar_rst (JD, &observer, &rst) != 0){
    hg->moon.c_circum=TRUE;
  }
  else {
    hg->moon.c_circum=FALSE;
    
    if(rst.rise>JD){  // Next Rise_Set or Current Rise_Set
      if(ln_get_lunar_rst (JD-1.0, &observer, &rst0)!=1){
	if(rst0.set>JD){
	  rst.rise=rst0.rise;
	  rst.set=rst0.set;
	  rst.transit=rst0.transit;
	}
      }
    }

    ln_get_lunar_equ_coords (rst.set, &equ_geoc);
    calc_moon_topocen(hg, rst.set, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				equ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.c_set.hours=zonedate.hours;
    hg->moon.c_set.minutes=zonedate.minutes;
    hg->moon.c_set.seconds=zonedate.seconds;
    
    ln_get_lunar_equ_coords (rst.rise, &equ_geoc);
    calc_moon_topocen(hg, rst.rise, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				equ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.c_rise.hours=zonedate.hours;
    hg->moon.c_rise.minutes=zonedate.minutes;
    hg->moon.c_rise.seconds=zonedate.seconds;
  }


  if (ln_get_solar_rst (JD, &observer, &rst) != 0){
    hg->sun.c_circum=TRUE;
  }
  else {
    hg->sun.c_circum=FALSE;

    ln_get_solar_equ_coords (rst.set, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				sequ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.c_set.hours=zonedate.hours;
    hg->sun.c_set.minutes=zonedate.minutes;
    hg->sun.c_set.seconds=zonedate.seconds;
    
    if(rst.rise<rst.set){
      ln_get_solar_rst (JD+1, &observer, &rst);
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				sequ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.c_rise.hours=zonedate.hours;
    hg->sun.c_rise.minutes=zonedate.minutes;
    hg->sun.c_rise.seconds=zonedate.seconds;
  }

  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) != 0){
    hg->atw18.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw18.c_circum=FALSE;

    hg->atw18.c_set.hours=set.hours;
    hg->atw18.c_set.minutes=set.minutes;
    hg->atw18.c_set.seconds=set.seconds;

    hg->atw18.c_rise.hours=rise.hours;
    hg->atw18.c_rise.minutes=rise.minutes;
    hg->atw18.c_rise.seconds=rise.seconds;
  }

  // Nautic Twilight = 12deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_NAUTIC_HORIZON, 
				&rst) != 0){
    hg->atw12.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw12.c_circum=FALSE;

    hg->atw12.c_set.hours=set.hours;
    hg->atw12.c_set.minutes=set.minutes;
    hg->atw12.c_set.seconds=set.seconds;

    hg->atw12.c_rise.hours=rise.hours;
    hg->atw12.c_rise.minutes=rise.minutes;
    hg->atw12.c_rise.seconds=rise.seconds;
  }

  // Civil Twilight = 06deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_CIVIL_HORIZON, 
				&rst) != 0){
    hg->atw06.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw06.c_circum=FALSE;

    hg->atw06.c_set.hours=set.hours;
    hg->atw06.c_set.minutes=set.minutes;
    hg->atw06.c_set.seconds=set.seconds;

    hg->atw06.c_rise.hours=rise.hours;
    hg->atw06.c_rise.minutes=rise.minutes;
    hg->atw06.c_rise.seconds=rise.seconds;
  }
}


void calc_moon_skymon(typHOE *hg){
  /* for Moon */
  double JD, JD_adj;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ, equ_geoc;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_zonedate local_date;
  struct ln_rst_time rst, rst0;
  struct ln_date date;
  struct ln_zonedate set,rise, zonedate;

  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  local_date.years=hg->skymon_year;
  local_date.months=hg->skymon_month;
  local_date.days=hg->skymon_day;

  local_date.hours=hg->skymon_hour;
  local_date.minutes=hg->skymon_min;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(hg->obs_timezone*60);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ_geoc);
  calc_moon_topocen(hg, JD, &equ_geoc, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.s_ra=hms;
  hg->moon.s_dec=dms;

  ln_get_solar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->sun.s_ra=hms;
  hg->sun.s_dec=dms;

  /*
  ln_get_mercury_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mercury.s_ra=hms;
  hg->mercury.s_dec=dms;
  hg->mercury.s_mag=ln_get_mercury_magnitude (JD);

  ln_get_venus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->venus.s_ra=hms;
  hg->venus.s_dec=dms;
  hg->venus.s_mag=ln_get_venus_magnitude (JD);

  ln_get_mars_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mars.s_ra=hms;
  hg->mars.s_dec=dms;
  hg->mars.s_mag=ln_get_mars_magnitude (JD);

  ln_get_jupiter_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->jupiter.s_ra=hms;
  hg->jupiter.s_dec=dms;
  hg->jupiter.s_mag=ln_get_jupiter_magnitude (JD);

  ln_get_saturn_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->saturn.s_ra=hms;
  hg->saturn.s_dec=dms;
  hg->saturn.s_mag=ln_get_saturn_magnitude (JD);

  ln_get_uranus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->uranus.s_ra=hms;
  hg->uranus.s_dec=dms;
  hg->uranus.s_mag=ln_get_uranus_magnitude (JD);

  ln_get_neptune_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->neptune.s_ra=hms;
  hg->neptune.s_dec=dms;
  hg->neptune.s_mag=ln_get_neptune_magnitude (JD);

  ln_get_pluto_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->pluto.s_ra=hms;
  hg->pluto.s_dec=dms;
  hg->pluto.s_mag=ln_get_pluto_magnitude (JD);
  */
  
  hg->moon.s_disk=ln_get_lunar_disk(JD);
  hg->moon.s_phase=ln_get_lunar_phase(JD);
  hg->moon.s_limb=ln_get_lunar_bright_limb(JD);

  hg->moon.s_age=(hg->moon.s_limb>180) ? 
    (180-hg->moon.s_phase)/360.*29.5 :      // Jo-gen
    hg->moon.s_phase/360.*29.5 + 29.5/2.;   // Ka-gen

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.s_circum=TRUE;
  }
  else {
    hg->moon.s_circum=FALSE;
    
    if(rst.rise>JD){  // Next Rise_Set or Current Rise_Set
      if(ln_get_lunar_rst (JD-1.0, &observer, &rst0)!=1){
	if(rst0.set>JD){
	  rst.rise=rst0.rise;
	  rst.set=rst0.set;
	  rst.transit=rst0.transit;
	}
      }
    }

    ln_get_lunar_equ_coords (rst.set, &equ_geoc);
    calc_moon_topocen(hg, rst.set, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				equ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.s_set.hours=zonedate.hours;
    hg->moon.s_set.minutes=zonedate.minutes;
    hg->moon.s_set.seconds=zonedate.seconds;
    
    ln_get_lunar_equ_coords (rst.rise, &equ_geoc);
    calc_moon_topocen(hg, rst.rise, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				equ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.s_rise.hours=zonedate.hours;
    hg->moon.s_rise.minutes=zonedate.minutes;
    hg->moon.s_rise.seconds=zonedate.seconds;
  }


  if (ln_get_solar_rst (JD, &observer, &rst) != 0){
    hg->sun.s_circum=TRUE;
  }
  else {
    hg->sun.s_circum=FALSE;

    ln_get_solar_equ_coords (rst.set, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				sequ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.s_set.hours=zonedate.hours;
    hg->sun.s_set.minutes=zonedate.minutes;
    hg->sun.s_set.seconds=zonedate.seconds;

    
    if(rst.rise<rst.set){
      ln_get_solar_rst (JD+1, &observer, &rst);
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				sequ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.s_rise.hours=zonedate.hours;
    hg->sun.s_rise.minutes=zonedate.minutes;
    hg->sun.s_rise.seconds=zonedate.seconds;
  }

  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) != 0){
    hg->atw18.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw18.s_circum=FALSE;
    
    hg->atw18.s_rise.hours=rise.hours;
    hg->atw18.s_rise.minutes=rise.minutes;
    hg->atw18.s_rise.seconds=set.seconds;
    hg->atw18.s_set.hours=set.hours;
    hg->atw18.s_set.minutes=set.minutes;
    hg->atw18.s_set.seconds=set.seconds;
  }

  // Nautic Twilight = 12deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_NAUTIC_HORIZON, 
				&rst) != 0){
    hg->atw12.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw12.s_circum=FALSE;

    hg->atw12.s_rise.hours=rise.hours;
    hg->atw12.s_rise.minutes=rise.minutes;
    hg->atw12.s_rise.seconds=set.seconds;
    hg->atw12.s_set.hours=set.hours;
    hg->atw12.s_set.minutes=set.minutes;
    hg->atw12.s_set.seconds=set.seconds;
  }

  // Civil Twilight = 06deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_CIVIL_HORIZON, 
				&rst) != 0){
    hg->atw06.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw06.s_circum=FALSE;

    hg->atw06.s_rise.hours=rise.hours;
    hg->atw06.s_rise.minutes=rise.minutes;
    hg->atw06.s_rise.seconds=set.seconds;
    hg->atw06.s_set.hours=set.hours;
    hg->atw06.s_set.minutes=set.minutes;
    hg->atw06.s_set.seconds=set.seconds;
  }
}


void calc_sun_plan(typHOE *hg){
  /* for Moon */
  double JD, JD_adj;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ, equ_geoc;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_zonedate local_date;
  struct ln_rst_time rst, rst0;
  struct ln_date date;
  struct ln_zonedate set, rise, zonedate;

  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day;

  local_date.hours=17;
  local_date.minutes=0;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(hg->obs_timezone*60);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.s_ra=hms;
  hg->moon.s_dec=dms;

  hg->moon.s_disk=ln_get_lunar_disk(JD);
  hg->moon.s_phase=ln_get_lunar_phase(JD);
  hg->moon.s_limb=ln_get_lunar_bright_limb(JD);

  if (ln_get_lunar_rst (JD, &observer, &rst) != 0){
    hg->moon.s_circum=TRUE;
  }
  else {
    hg->moon.s_circum=FALSE;
    
    if(rst.rise>JD){  // Next Rise_Set or Current Rise_Set
      if(ln_get_lunar_rst (JD-1.0, &observer, &rst0)!=1){
	if(rst0.set>JD){
	  rst.rise=rst0.rise;
	  rst.set=rst0.set;
	  rst.transit=rst0.transit;
	}
      }
    }

    ln_get_lunar_equ_coords (rst.set, &equ_geoc);
    calc_moon_topocen(hg, rst.set, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				equ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.s_set.hours=zonedate.hours;
    hg->moon.s_set.minutes=zonedate.minutes;
    hg->moon.s_set.seconds=zonedate.seconds;
    
    ln_get_lunar_equ_coords (rst.rise, &equ_geoc);
    calc_moon_topocen(hg, rst.rise, &equ_geoc, &equ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				equ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->moon.s_rise.hours=zonedate.hours;
    hg->moon.s_rise.minutes=zonedate.minutes;
    hg->moon.s_rise.seconds=zonedate.seconds;
  }


  if (ln_get_solar_rst (JD, &observer, &rst) != 0){
    hg->sun.s_circum=TRUE;
  }
  else {
    hg->sun.s_circum=FALSE;

    ln_get_solar_equ_coords (rst.set, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.set,
				sequ,
				hg,
				FALSE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.s_set.hours=zonedate.hours;
    hg->sun.s_set.minutes=zonedate.minutes;
    hg->sun.s_set.seconds=zonedate.seconds;
    
    if(rst.rise<rst.set){
      ln_get_solar_rst (JD+1, &observer, &rst);
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    JD_adj=get_alt_adjusted_rst(rst.rise,
				sequ,
				hg,
				TRUE);
    ln_get_local_date(JD_adj, &zonedate, 
		      hg->obs_timezone);
    hg->sun.s_rise.hours=zonedate.hours;
    hg->sun.s_rise.minutes=zonedate.minutes;
    hg->sun.s_rise.seconds=zonedate.seconds;

  }


  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) != 0){
    hg->atw18.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*60);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*60);
    hg->atw18.s_circum=FALSE;
    
    hg->atw18.s_rise.hours=rise.hours;
    hg->atw18.s_rise.minutes=rise.minutes;
    hg->atw18.s_rise.seconds=set.seconds;
    hg->atw18.s_set.hours=set.hours;
    hg->atw18.s_set.minutes=set.minutes;
    hg->atw18.s_set.seconds=set.seconds;
  }
}


gboolean draw_plot_cairo_old(GtkWidget *widget, typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list, i_plan;

  gint from_set, to_rise;
  double dx,dy,lx,ly;

  double paz[250],pel[250],ppa[250],pad[250],pjd[250];
  double sep[200],phpa[200];
  
  double  pi=3.141592;
  double  alambda; //longitude[deg]
  double  alamh;   //=alambda/360.*24.;    //[hour]
  double  phi;     //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char *tmp;
  char *c;
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
  int hour, min, sec;
  int ihst0, ihst1;
  gdouble JD0, JD1, JD_mt, JD_tr, JD_tr0;
  gboolean Flag_moon=FALSE;
  double utstart,utend;
  double ut_offset;
  double a0,d0,a0rad,d0rad;
  double ut,d_ut;
  double flst, ha, sinha, cosha;
  double el0, ume, den;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, JD_hst;
  double a1;
  int i,iend;
  int width, height;
  gint x_hst;
  gdouble x_tr,y_tr;
  
  struct ln_zonedate zonedate;
  struct ln_date date;
  struct ln_equ_posn oequ, oequ_geoc;
  struct ln_equ_posn oequ_prec;
  struct ln_rst_time orst;
  struct ln_hrz_posn ohrz;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD;
  double obs_latitude=LATITUDE_SUBARU;
  double obs_longitude=LONGITUDE_SUBARU;
  double obs_altitude=ALTITUDE_SUBARU;

  if(!flagPlot) return (FALSE);

  calc_sun_plan(hg);

  if((hg->plot_all!=PLOT_ALL_PLAN)&&(hg->skymon_mode==SKYMON_CUR)){
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }
  else{
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0;
  }

  ihst0=17;
  ihst1=31;

  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=ihst0;
  zonedate.minutes=0;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)(hg->obs_timezone*60);

  JD0 = ln_get_julian_local_date(&zonedate);
  if(hour<10){
    JD0 = JD0 -1.0;
  }
  JD1 = JD0+(gdouble)(ihst1-ihst0)/24.;


  observer.lat = obs_latitude;
  observer.lng = obs_longitude;


  switch(hg->plot_output){
  case PLOT_OUTPUT_PDF:
    width= PLOT_WIDTH;
    height= PLOT_HEIGHT;
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

    cairo_set_source_rgb(cr, 1, 1, 1);
    break;

  default:
    {
      GtkAllocation *allocation=g_new(GtkAllocation, 1);
      gtk_widget_get_allocation(widget,allocation);

      width= allocation->width;
      height= allocation->height;
      g_free(allocation);
    }

    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->plot_dw), &width, &height);
    }
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

#ifdef USE_GTK3
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);

    cr = cairo_create(surface);
#else
    if(pixmap_plot) g_object_unref(G_OBJECT(pixmap_plot));
    pixmap_plot = gdk_pixmap_new(gtk_widget_get_window(widget),
				 width,
				 height,
				 -1);
  
    cr = gdk_cairo_create(pixmap_plot);
#endif
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
    break;
  }
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if((hg->plot_all!=PLOT_ALL_PLAN) && (hg->plot_i>=0)){
    // Object Name etc.
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    
    tmp=g_strdup_printf("\"%s\"  RA=%09.2f Dec=%+010.2f Equinox=%7.2f",
			hg->obj[hg->plot_i].name,
			hg->obj[hg->plot_i].ra,
			hg->obj[hg->plot_i].dec,
			hg->obj[hg->plot_i].equinox);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,+extents.height);
    cairo_show_text(cr, tmp);
    g_free(tmp);
  }

  /* draw a rectangle */
 
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_rectangle(cr,  dx,dy, lx,ly);
  cairo_fill(cr);


  switch(hg->plot_mode){
  case PLOT_EL:
    // El
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*75/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*45/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*45/90);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "80", &extents);
      x = dx+lx+5;
      y = dy+ly*(90-80)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "80");
    
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);

    }// El
    break;
    
  case PLOT_AZ:
    // Az
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*45/360);
      cairo_line_to ( cr, width-dx, dy+ly*45/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*90/360);
      cairo_line_to ( cr, width-dx, dy+ly*90/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*135/360);
      cairo_line_to ( cr, width-dx, dy+ly*135/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*180/360);
      cairo_line_to ( cr, width-dx, dy+ly*180/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*225/360);
      cairo_line_to ( cr, width-dx, dy+ly*225/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*270/360);
      cairo_line_to ( cr, width-dx, dy+ly*270/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*315/360);
      cairo_line_to ( cr, width-dx, dy+ly*315/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      
      // Az Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "Azimuth[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Azimuth[deg]");
      cairo_restore (cr);

    }// Az
    break;
  case PLOT_AD:
    // AD
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*(4-0.5)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-0.5)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
     

      // AD Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);

      cairo_text_extents (cr, "4.0", &extents);
      x = dx-extents.width-5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "4.0");
      
      cairo_text_extents (cr, "2.0", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(4-2)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "2.0");
      
      cairo_text_extents (cr, "1.0", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(4-1)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "1.0");
      
      cairo_text_extents (cr, "0.5", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(4-0.5)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0.5");
      
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      tmp=g_strdup_printf("AD[\"] (%d-%dA)", hg->wave1,hg->wave0);
      cairo_text_extents (cr, tmp, &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, tmp);
      g_free(tmp);
      cairo_restore (cr);


      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "+180", &extents);
      x = dx+lx+5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "Zenith PA[deg]", &extents);

      x = dx+lx-extents.width/2+x0-(dx+lx+x0);
      y = height/2+extents.height-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Zenith PA[deg]");
      cairo_restore (cr);
      
    }// AD
    break;

  case PLOT_MOONSEP:
    // Moon Separation
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_set_source_rgba(cr, 0.4, 0.8, 0.4,1.0);

      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

      cairo_move_to ( cr, dx, dy+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0);
      cairo_stroke(cr);


      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5);

	do{
	  if(y_f%90){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5;
      y = dy+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      x0=extents.width;

      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);



      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "180", &extents);
      x = dx+lx+5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "180");
      
      cairo_text_extents (cr, "135", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "135");
      
      cairo_text_extents (cr, "90", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");

      cairo_text_extents (cr, "45", &extents);
      x = dx+lx+5;
      y = dy+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "45");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "135", &extents);
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "Separation from the Moon[deg]", &extents);

      x = dx+lx-extents.width/2-(dx+lx);
      y = 5+x0;
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Separation from the Moon[deg]");
      cairo_restore (cr);
      
    }
    break;

  case PLOT_HDSPA:
    // HDS PA w/o ImR
    {
      gfloat x0, w1;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*45/360);
      cairo_line_to ( cr, width-dx, dy+ly*45/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*90/360);
      cairo_line_to ( cr, width-dx, dy+ly*90/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*135/360);
      cairo_line_to ( cr, width-dx, dy+ly*135/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*180/360);
      cairo_line_to ( cr, width-dx, dy+ly*180/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*225/360);
      cairo_line_to ( cr, width-dx, dy+ly*225/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*270/360);
      cairo_line_to ( cr, width-dx, dy+ly*270/360);
      cairo_set_line_width(cr,2.0);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*315/360);
      cairo_line_to ( cr, width-dx, dy+ly*315/360);
      cairo_set_line_width(cr,0.5);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }


      
      // HDS PA  Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "S is UP", &extents);
      x = dx+lx+5;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "S is UP");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_text_extents (cr, "  & Zenith PA [deg]", &extents);
      w1=extents.width;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);

      x = dx-(extents.width+w1)/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "HDS PA w/o ImR[deg]");

      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);
      x += extents.width;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);

      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "  & Zenith PA[deg]");

      cairo_restore (cr);

    }// HDS PA w/o ImR
    break;
  }

  // HST
  {
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    //if(hg->skymon_mode==SKYMON_SET){
    {
      tmp=g_strdup_printf("%d:%02d",
			  hg->sun.s_set.hours,hg->sun.s_set.minutes);

      if(hg->sun.s_set.hours>=ihst0){
	x=lx*((gfloat)hg->sun.s_set.hours-ihst0
	      +(gfloat)hg->sun.s_set.minutes/60.)/(gfloat)(ihst1-ihst0);

	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, x,ly);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+x, dy-5);
	cairo_show_text(cr, tmp);
      }
      else{
	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx, dy-5);
	cairo_show_text(cr, tmp);
      }
      g_free(tmp);


      tmp=g_strdup_printf("%d:%02d",
			  hg->sun.s_rise.hours,hg->sun.s_rise.minutes);

      if(hg->sun.s_rise.hours<(ihst1-24)){
	x=lx*((ihst1-24)-(gfloat)hg->sun.s_rise.hours
	      -(gfloat)hg->sun.s_rise.minutes/60.)/(gfloat)(ihst1-ihst0);

	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+lx-x-extents.width, dy-5);
	cairo_show_text(cr, tmp);
      }
      else{
	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+lx-extents.width, dy-5);
	cairo_show_text(cr, tmp);
      }
      g_free(tmp);

      // Astronomical Twilight = 18deg
      tmp=g_strdup_printf("%d:%02d",
			  hg->atw18.s_set.hours,hg->atw18.s_set.minutes);

      if(hg->atw18.s_set.hours>=ihst0){
	x=lx*((gfloat)hg->atw18.s_set.hours-ihst0
	      +(gfloat)hg->atw18.s_set.minutes/60.)/(gfloat)(ihst1-ihst0);

	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, x,ly);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+x, dy+extents.height+5);
	cairo_show_text(cr, tmp);
      }
      else{
	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx, dy+extents.height+5);
	cairo_show_text(cr, tmp);
      }
      g_free(tmp);

      tmp=g_strdup_printf("%d:%02d",
			  hg->atw18.s_rise.hours,hg->atw18.s_rise.minutes);

      if(hg->atw18.s_rise.hours<(ihst1-24)){
	x=lx*((ihst1-24)-(gfloat)hg->atw18.s_rise.hours
	      -(gfloat)hg->atw18.s_rise.minutes/60.)/(gfloat)(ihst1-ihst0);

	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+lx-x-extents.width, dy+extents.height+5);
	cairo_show_text(cr, tmp);
      }
      else{
	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx+lx-extents.width, dy+extents.height+5);
	cairo_show_text(cr, tmp);
      }
    }
    g_free(tmp);

    // vertical lines
    x_hst=0;

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    do{
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

      cairo_move_to ( cr, dx+lx*(gfloat)x_hst/(gfloat)(ihst1-ihst0), dy);
      cairo_line_to ( cr, dx+lx*(gfloat)x_hst/(gfloat)(ihst1-ihst0), ly+dy);

      if((ihst0+x_hst)%2==0){
	cairo_set_line_width(cr,1.0);
      }
      else{
	cairo_set_line_width(cr,0.5);
      }
      cairo_stroke(cr);
    

      cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);

      if((ihst0+x_hst)%2==0){
	cairo_set_line_width(cr,1.0);
	tmp=g_strdup_printf("%d",
			    (ihst0+x_hst)<24 ? ihst0+x_hst : ihst0+x_hst-24);
	cairo_text_extents (cr, tmp, &extents);
	x = dx+lx*(gfloat)x_hst/(gfloat)(ihst1-ihst0)-extents.width/2;
	y = dy+ly+extents.height+5;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
	g_free(tmp);
      }

      x_hst++;
    }while((ihst0+x_hst)<=ihst1);


    // Date
    iyear=hg->fr_year;
    month=hg->fr_month;
    iday=hg->fr_day;
      
    hour=0;
    min=0;
    sec=0;
  
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
    cairo_text_extents (cr, "HST", &extents);
    x = width/2-extents.width/2;
    y += extents.height+5;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, "HST");

    {
      gint iyear1,month1,iday1;
      
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      tmp=g_strdup_printf("(%4d/%2d/%2d)",iyear,month,iday);
      cairo_text_extents (cr, tmp, &extents);
      x = dx;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
      g_free(tmp);
      
      iyear1=iyear;
      month1=month;
      iday1=iday;

      add_day(hg, &iyear1, &month1, &iday1, +1);
      tmp=g_strdup_printf("(%4d/%2d/%2d)",iyear1,month1,iday1);
      cairo_text_extents (cr, tmp, &extents);
      x = dx+lx-extents.width;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
      g_free(tmp);
    }
      

    // Current Time
    if(hg->plot_all!=PLOT_ALL_PLAN){
      if(hg->skymon_mode==SKYMON_CUR){
	get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
      }
      else{
	iyear=hg->skymon_year;
	month=hg->skymon_month;
	iday=hg->skymon_day;
	
	hour=hg->skymon_hour;
	min=hg->skymon_min;
	sec=0.0;
      }
      if(hour<10){
	add_day(hg, &iyear, &month, &iday, -1);
      }
      
      if(((gfloat)hour+(gfloat)min/60.<(ihst1-24.)) 
	 || (((gfloat)hour+(gfloat)min/60.>=ihst0)&&((gfloat)hour+(gfloat)min/60.<ihst1))){
	if((gfloat)hour+(gfloat)min/60.<(ihst1-24)){
	  hour+=24;
	}
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	x=dx+lx*((gfloat)hour-ihst0+(gfloat)min/60.)/(ihst1-ihst0);
	cairo_move_to ( cr, x, dy);
	cairo_line_to ( cr, x, ly+dy);
	cairo_set_line_width(cr,3.0);
	cairo_stroke(cr);
	
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	if(hour>=24){
	  tmp=g_strdup_printf("%d:%02d",hour-24,min);
	}
	else if(hour<0){
	  tmp=g_strdup_printf("%d:%02d",hour+24,min);
	}
	else{
	  tmp=g_strdup_printf("%d:%02d",hour,min);
	}
	cairo_text_extents (cr, tmp, &extents);
	x += -extents.width/2;
	y = dy -5;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);

	if(hg->plot_i>=0){
	  if(hg->plot_mode==PLOT_AZ){
	    gdouble vaz;
	    
	    if(hg->skymon_mode==SKYMON_CUR){
	      vaz=hg->obj[hg->plot_i].c_vaz;
	    }
	    else{
	      vaz=hg->obj[hg->plot_i].s_vaz;
	    }
	    
	    if(vaz!=-100){
	      x -= -extents.width/2;
	      y -= extents.height+2;
	      
	      tmp=g_strdup_printf("%+.2fdeg/min",vaz);
	      cairo_text_extents (cr, tmp, &extents);
	      x += -extents.width/2;
	      cairo_move_to(cr, x, y);
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
	      cairo_show_text(cr, tmp);
	      if(tmp) g_free(tmp);
	    }
	  }
	  else if(hg->plot_mode==PLOT_AD){
	    gdouble vpa;
	    
	    if(hg->skymon_mode==SKYMON_CUR){
	      vpa=hg->obj[hg->plot_i].c_vpa;
	    }
	    else{
	      vpa=hg->obj[hg->plot_i].s_vpa;
	    }
	    
	    if(vpa!=-100){
	      x -= -extents.width/2;
	      y -= extents.height+2;
	      
	      tmp=g_strdup_printf("%+.3fdeg/min",vpa);
	      cairo_text_extents (cr, tmp, &extents);
	      x += -extents.width/2;
	      cairo_move_to(cr, x, y);
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	      cairo_show_text(cr, tmp);
	      if(tmp) g_free(tmp);
	    }
	  }
	  else if( hg->plot_mode==PLOT_MOONSEP ){
	    gdouble sep;
	    
	    if(hg->skymon_mode==SKYMON_CUR){
	      sep=hg->obj[hg->plot_i].c_sep;
	    }
	    else{
	      sep=hg->obj[hg->plot_i].s_sep;
	    }
	    
	    if(sep>0){
	      x -= -extents.width/2;
	      y -= extents.height+2;
	      
	      tmp=g_strdup_printf("%.1fdeg",sep);
	      cairo_text_extents (cr, tmp, &extents);
	      x += -extents.width/2;
	      cairo_move_to(cr, x, y);
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	      cairo_show_text(cr, tmp);
	      if(tmp) g_free(tmp);
	    }
	  }
	  else if( hg->plot_mode==PLOT_HDSPA ){
	    gdouble vpa;
	    
	    if(hg->skymon_mode==SKYMON_CUR){
	      vpa=hg->obj[hg->plot_i].c_vhpa;
	    }
	    else{
	      vpa=hg->obj[hg->plot_i].s_vhpa;
	    }
	    
	    if(vpa!=-100){
	      x -= -extents.width/2;
	      y -= extents.height+2;
	      
	      tmp=g_strdup_printf("%+.3fdeg/min",vpa);
	      cairo_text_extents (cr, tmp, &extents);
	      x += -extents.width/2;
	      cairo_move_to(cr, x, y);
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	      cairo_show_text(cr, tmp);
	      if(tmp) g_free(tmp);
	    }
	  }
	}
      }
    }
  }// HST
  
  //////////////////////////////////////////
  

  alambda=obs_longitude;  //longitude[deg]
  alamh=alambda/360.*24.;    //[hour]
  phi=obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=ihst0;
  zonedate.minutes=0;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)(hg->obs_timezone*60);
  
  ln_zonedate_to_date(&zonedate, &date);
  
  utstart=(double)date.hours;
  utend=utstart+(double)(ihst1-ihst0);

  ut_offset=(double)(zonedate.hours-date.hours+ (gdouble)zonedate.minutes/60.);

  /* Lunar RA, DEC */
  if(((hg->plot_moon)&&(hg->plot_mode==PLOT_EL))
     ||(hg->plot_mode==PLOT_MOONSEP)){     
    JD = JD0;

    ln_get_lunar_rst (JD, &observer, &orst);
    ln_get_date (orst.transit, &odate);
    ln_date_to_zonedate(&odate,&moon_transit,(long)hg->obs_timezone*60);

    JD_mt = ln_get_julian_local_date(&moon_transit);
    
    if(JD_mt<(JD0+JD1)/2.-12.){ // RST in Next Day
      JD = JD+1.0;

      ln_get_lunar_rst (JD, &observer, &orst);
      ln_get_date (orst.transit, &odate);
      ln_date_to_zonedate(&odate,&moon_transit,(long)hg->obs_timezone*60);

      JD_mt = ln_get_julian_local_date(&moon_transit);
    }

    if(fabs(JD_mt-JD_moon_stock)>0.5){ // Re-calc Moon??
      Flag_moon=TRUE;
    }

    if(Flag_moon){  // Calc El at lunar transit
      ln_get_lunar_equ_coords (orst.transit, &oequ_geoc);
      calc_moon_topocen(hg, orst.transit, &oequ_geoc, &oequ);
    
      ln_get_hrz_from_equ (&oequ, &observer, orst.transit, &ohrz);
      moon_tr_el=ohrz.alt;
    }
    
    i=1;
    d_ut=(JD1-JD0)*24.0/190.0;
    //d_ut=16.0/190.0;
    
    JD_hst = JD0;
    //JD_hst = JD_mt-8./24.;
    
      
    while(JD_hst<JD1+d_ut/24.){
    //while(JD_hst<JD_mt+8./24.){
      if(Flag_moon){  // Revise lunar caluclation
	ln_get_lunar_equ_coords (JD_hst, &oequ_geoc);
	calc_moon_topocen(hg, JD_hst, &oequ_geoc, &oequ);
	ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	if(ohrz.az>180) ohrz.az-=360;
	
	paz_moon[i]=ohrz.az;
	pel_moon[i]=ohrz.alt;
      }
      pjd[i]=JD_hst;
      
      ut=ut+d_ut;
      JD_hst+=d_ut/24.;
      i=i+1;
    }

    iend=i-1;

    if(Flag_moon){  
      JD_moon_stock = JD_mt;
    }

      
    cairo_rectangle(cr, dx,dy,lx,ly);
    cairo_clip(cr);
    cairo_new_path(cr);
    
    switch(hg->plot_mode){
    case PLOT_EL:
    case PLOT_MOONSEP:
      if(hg->plot_moon){
	
	cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 0.5);
	cairo_set_line_width(cr,5.0);
	
	for(i=1;i<=iend-1;i++){
	  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
	  y=dy+ly*(90-pel_moon[i])/90;
	  cairo_move_to(cr,x,y);
	  
	  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
	  y=dy+ly*(90-pel_moon[i+1])/90;
	  cairo_line_to(cr,x,y);
	  
	  cairo_stroke(cr);
	  
	}
	
	if((JD_mt>JD0)&&(JD_mt<JD1)){
	  cairo_save(cr);
	  cairo_reset_clip(cr);
	  cairo_move_to(cr,dx+lx*(JD_mt-JD0)/(JD1-JD0),
			dy+ly*(90.-moon_tr_el)/90.);
	  
	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 1.0);

	  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0);
	  
	  tmp=g_strdup_printf("%.1lf days",(hg->skymon_mode==SKYMON_CUR)?
			      hg->moon.c_age : hg->moon.s_age);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_rel_move_to(cr,-extents.width/2.,-5.);
	  cairo_show_text(cr,tmp);
	  cairo_rel_move_to(cr,-extents.width/2.,0);

	  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	  
	  cairo_text_extents (cr, "moon", &extents);
	  cairo_rel_move_to(cr,-extents.width/2.,-12.);
	  cairo_show_text(cr,"moon");
	  cairo_restore(cr);
	}
      }
      break;
    }
    cairo_reset_clip(cr);
  }

  if(hg->plot_all!=PLOT_ALL_PLAN){
    gboolean plot_flag=FALSE;

    // Objects
    for(i_list=0;i_list<hg->i_max;i_list++){
      switch(hg->plot_all){ 
      case PLOT_ALL_SINGLE:
	if(i_list==hg->plot_i)
	  plot_flag=TRUE;
	else
	  plot_flag=FALSE;
	break;
	
      case PLOT_ALL_SELECTED:
	plot_flag=hg->obj[i_list].check_sm;
	break;

      case PLOT_ALL_ALL:
	plot_flag=TRUE;
	break;
      }
      
      if(plot_flag){
	gdouble az0, el0;
	oequ.ra=ra_to_deg(hg->obj[i_list].ra);
	oequ.dec=dec_to_deg(hg->obj[i_list].dec);
	
	//JD = ln_get_julian_local_date(&zonedate);
	JD = JD0;
	
	ln_get_equ_prec2 (&oequ, 
			  get_julian_day_of_epoch(hg->obj[i_list].equinox),
			  JD, &oequ_prec);
	ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
	ln_get_date (orst.transit, &odate);
	ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);

	// Transit time for object
	JD_tr = ln_get_julian_local_date(&transit);

	if((JD_tr<JD0)||(JD_tr>JD1)){
	  if(JD_tr<JD0){
	    JD=JD+1.0;
	  }
	  else{
	    JD=JD-1.0;
	  }
	
	  ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[i_list].equinox),
			    JD, &oequ_prec);
	  ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
	  ln_get_date (orst.transit, &odate);
	  ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);

	  JD_tr0 = ln_get_julian_local_date(&transit);

	  JD = (JD0+JD1)/2.0;
	  if(fabs(JD-JD_tr0)<fabs(JD-JD_tr)){
	    JD_tr=JD_tr0;
	  }
	}

	a0=oequ_prec.ra*24./360.; //[hour]
	d0rad=oequ_prec.dec*M_PI/180.;
	
	ut=utstart;
	
	i=1;
	d_ut=(JD1-JD0)*24.0/190.0;
	
	JD_hst = JD0;
	
	while(JD_hst<JD1+d_ut/24.){
	  ln_get_hrz_from_equ (&oequ_prec, &observer, JD_hst, &ohrz);

	  flst=ln_get_mean_sidereal_time(JD_hst)+LONGITUDE_SUBARU/360.*24.;
	  
	  if(ohrz.az>180) ohrz.az-=360;
	  paz[i]=ohrz.az+180;
	  pel[i]=ohrz.alt;
	  
	  if(hg->plot_mode==PLOT_MOONSEP){
	    sep[i]=deg_sep(paz[i]-180,pel[i],paz_moon[i],pel_moon[i]);
	  }

	  ha=flst-a0;             //hour angle [hour]
	  ha=set_ul(-12., ha, 12.);
	  
	  el0=(gdouble)ohrz.alt*M_PI/180.;
	  az0=(180.+(gdouble)ohrz.az)*M_PI/180.;
	  if(az0<0.) az0=az0+2.*M_PI;
	  
	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=el0+M_PI*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
	  d1=d1rad*180./M_PI;
	  ume1=-sin(az0)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./M_PI;
	  
	  ha1=set_ul(-12., ha1, 12.);
	  a1=flst-ha1;
	  a1=set_ul(0., a1, 24.);
	  
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  delta_a=(a1-a0)*M_PI/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/M_PI;
	  
	  
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=M_PI*(90.0-ohrz.alt)/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  if(ohrz.alt>3){
	    if(hg->wave1<hg->wave0){
	      adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	    }
	    else{
	      adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	    }
	  }
	  else{
	    adsec=100;
	  }
	
	  pjd[i]=JD_hst;

	  ppa[i]=padeg;
	  pad[i]=adsec;
	  
	  // HDS PA w/o ImR
	  phpa[i]=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);

	  if(hg->plot_mode==PLOT_MOONSEP){
	    sep[i]=deg_sep(paz[i]-180,pel[i],paz_moon[i],pel_moon[i]);
	  }

	  ut=ut+d_ut;
	  JD_hst+=d_ut/24.;
	  i=i+1;
	}
      
	iend=i-1;
	
	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_rectangle(cr, dx,dy,lx,ly);
	cairo_clip(cr);
	cairo_new_path(cr);

	switch(hg->plot_mode){
	case PLOT_EL:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	    }
	    
	    if((JD_tr>JD0)&&(JD_tr<JD1)){
	      y_tr=fabs(obs_latitude-oequ.dec);
	      
	      cairo_save(cr);
	      cairo_reset_clip(cr);
	      cairo_move_to(cr,dx+lx*(JD_tr-JD0)/(JD1-JD0),
			    dy+ly*y_tr/90.);
	      if(i_list==hg->plot_i){
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
		
		if(oequ_prec.dec>obs_latitude){
		  tmp=g_strdup_printf("North(%.2f)",
				      90.0-fabs(oequ_prec.dec-obs_latitude));
		}
		else{
		  tmp=g_strdup_printf("South(%.2f)",
				      90.0-fabs(oequ_prec.dec-obs_latitude));
		}
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
		
		cairo_text_extents (cr, tmp, &extents);
		cairo_rel_move_to(cr,-extents.width/2.,-5);
		cairo_show_text(cr,tmp);
		if(tmp) g_free(tmp);
		
		cairo_move_to(cr,dx+lx*(JD_tr-JD0)/(JD1-JD0),
			      dy+ly*y_tr/90.);
		cairo_rel_move_to(cr,0,-extents.height-5);
		
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      }
	      else{
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	      }

	      cairo_text_extents (cr, hg->obj[i_list].name, &extents);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_rel_move_to(cr,5.,+extents.height/2.);
	      cairo_show_text(cr,hg->obj[i_list].name);
	      cairo_restore(cr);
	    }
	  }
	  break;
	case PLOT_AZ:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(paz[i]-paz[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(paz[i]<180){
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-paz[i])/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(paz[i+1]-360))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(paz[i]+360))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-paz[i+1])/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;

	case PLOT_AD:
	  {
	    
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
		x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		y=dy+ly*(4-pad[i])/4.;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		y=dy+ly*(4-pad[i+1])/4.;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	    }
	    
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.4);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;

	case PLOT_MOONSEP:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	    }
	    
	    if((x_tr>ihst0)&&(x_tr<ihst1)){
	      y_tr=fabs(obs_latitude-oequ_prec.dec);
	      
	      cairo_save(cr);
	      cairo_reset_clip(cr);
	      cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(gdouble)(ihst1-ihst0),
			    dy+ly*y_tr/90.);
	      
	      if(i_list==hg->plot_i){
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
		
		if(oequ_prec.dec>obs_latitude){
		  tmp=g_strdup_printf("North(%.2f)",
				      90.0-fabs(oequ_prec.dec-obs_latitude));
		}
		else{
		  tmp=g_strdup_printf("South(%.2f)",
				      90.0-fabs(oequ_prec.dec-obs_latitude));
		}
		
		cairo_text_extents (cr, tmp, &extents);
		cairo_rel_move_to(cr,-extents.width/2.,-5.);
		cairo_show_text(cr,tmp);
		if(tmp) g_free(tmp);
		cairo_restore(cr);
	      }
	    }
	    
	    // Moon Separation
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-2;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		y=dy+ly*(180-sep[i])/180;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		y=dy+ly*(180-sep[i+1])/180;
	      cairo_line_to(cr,x,y);
	      
	      if(pel_moon[i]<0){
		if(i_list==hg->plot_i){
		  cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 0.5);
		  cairo_set_line_width(cr,1.5);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.2);
		  cairo_set_line_width(cr,0.5);
		}
	      }
	      else{
		if(i_list==hg->plot_i){
		  cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
		  cairo_set_line_width(cr,3.0);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
		  cairo_set_line_width(cr,1.0);
		}
	      }

	      cairo_stroke(cr);
	      }
	    }
	    
	  }
	  break;

	case PLOT_HDSPA:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	    cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(phpa[i]-phpa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		  
		}
		else{
		  if(phpa[i]<0){
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]-360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	    
	    if(hg->plot_all==PLOT_ALL_SINGLE){
	      cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
	      cairo_set_line_width(cr,1.0);
	      
	      for(i=1;i<=iend-1;i++){
		if((pel[i]>0)&&(pel[i+1]>0)){
		  if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    if(ppa[i]<0){
		      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		      y=dy+ly*(360-(ppa[i]+180))/360;
		      cairo_move_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		      y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		      cairo_line_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		      y=dy+ly*(360-(ppa[i]+360+180))/360;
		      cairo_move_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		      y=dy+ly*(360-(ppa[i+1]+180))/360;
		      cairo_line_to(cr,x,y);
		      
		      cairo_stroke(cr);
		    }
		  }
		}
	      }
	    }
	  }
	  break;
	}
      }
    }
  }
  else{  // PLOT_PLAN
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	gdouble hst_sod;
	
	//zonedate.hours=(gint)(hg->plan[i_plan].sod/60/60);
	//zonedate.minutes=(hg->plan[i_plan].sod-(gint)zonedate.hours*60*60)/60;
	//zonedate.seconds=0;

	//ln_zonedate_to_date(&zonedate, &date);
	//JD = ln_get_julian_local_date(&zonedate);

	oequ.ra=ra_to_deg(hg->obj[hg->plan[i_plan].obj_i].ra);
	oequ.dec=dec_to_deg(hg->obj[hg->plan[i_plan].obj_i].dec);
	
	ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[hg->plan[i_plan].obj_i].equinox),
			  JD, &oequ_prec);
	ln_get_hrz_from_equ (&oequ_prec, &observer, JD, &ohrz);


	//utstart=(double)date.hours+(double)date.minutes/60.;
	//utend=utstart+(double)hg->plan[i_plan].time/60./60.;

	a0=oequ_prec.ra*24./360.; //[hour]
	d0rad=oequ_prec.dec*M_PI/180.;

	ut=utstart;
      
	i=1;
	d_ut=(gdouble)(ihst1-ihst0)/190.0;
      
	hst_sod=(gdouble)hg->plan[i_plan].sod/60./60.;
	
	JD_hst = JD0;

	while(ut<=utend){
	  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	  flst=ln_get_mean_sidereal_time(JD_hst)+LONGITUDE_SUBARU/360.*24.;

	  if(ohrz.az>180) ohrz.az-=360;
	  paz[i]=ohrz.az+180;
	  pel[i]=ohrz.alt;

	  if(hg->plot_mode==PLOT_MOONSEP){
	    sep[i]=deg_sep(paz[i]-180,pel[i],paz_moon[i],pel_moon[i]);
	  }
	  
	  ha=flst-a0;             //hour angle [hour]
	  ha=set_ul(-12., ha, 12.);
	  
	  sinha=sin(M_PI*ha/12.0);
	  cosha=cos(M_PI*ha/12.0);
	  ume=-sinha*cos(d0rad);
	  den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	  
	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=pel[i]*M_PI/180.+M_PI*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(paz[i]*M_PI/180.)*cos(el0d));
	  d1=d1rad*180./M_PI;
	  ume1=-sin(paz[i]*M_PI/180.)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(paz[i]*M_PI/180.)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./M_PI;
	  
	  ha1=set_ul(-12., ha1, 12.);
	  a1=flst-ha1;
	  a1=set_ul(0., a1, 24.);
	  
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  //write(9,*)'#### Position Angle'
	  delta_a=(a1-a0)*M_PI/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/M_PI;
	  //write(9,'(4h PA=,f8.3)')padeg
	
	
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=pi*(90.0-pel[i])/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	
	  pjd[i]=JD_hst;
	  ppa[i]=padeg;
	  pad[i]=adsec;

	  // HDS PA w/o ImR
	  phpa[i]=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);

	  JD_hst+=d_ut/24.;
	  ut=ut+d_ut;
	  i=i+1;
	}
	
	iend=i-1;
	
	cairo_reset_clip(cr);

	if((hg->plot_mode==PLOT_EL)&&(!hg->plan[i_plan].backup)){
	  cairo_save (cr);

	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_select_font_face (cr, hg->fontfamily_all, 
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_select_font_face (cr, hg->fontfamily_all, 
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
	  }
	  cairo_text_extents (cr, hg->obj[hg->plan[i_plan].obj_i].name,
			      &extents);
	  
	  cairo_translate(cr, 
			  dx+lx*(hst_sod+(double)hg->plan[i_plan].time/60./60./2.-ihst0)/(gfloat)(ihst1-ihst0), dy+ly*(90-3)/90);
	  cairo_rotate (cr,-M_PI/2);
	  cairo_move_to(cr, 0, +extents.height/2);
	  
	  cairo_show_text(cr, hg->obj[hg->plan[i_plan].obj_i].name);
	  cairo_restore (cr);
	}      

	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_rectangle(cr, 
			(gdouble)((gint)(dx+lx*(hst_sod-ihst0)/(ihst1-ihst0)+0.5)),
			dy,
			(gdouble)((gint)(lx*((gdouble)hg->plan[i_plan].time/60./60.)/(ihst1-ihst0)+0.5)),
			ly);
	cairo_clip(cr);
	cairo_new_path(cr);
      
	switch(hg->plot_mode){
	case PLOT_EL:
	  {
	    
	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,2.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
	      y=dy+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	  }
	}
	break;

      case PLOT_AZ:
	{
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (fabs(paz[i]-paz[i+1])<180) ) {
		x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		y=dy+ly*(360-paz[i])/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		y=dy+ly*(360-paz[i+1])/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(paz[i]<180){
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(paz[i+1]-360))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(paz[i]+360))/360;
		  cairo_move_to(cr,x,y);

		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);

		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;
      case PLOT_AD:
	{
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
	      x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
	      y=dy+ly*(4-pad[i])/4.;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
	      y=dy+ly*(4-pad[i+1])/4.;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);      
	    }
	  }
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.4);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		y=dy+ly*(360-(ppa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		y=dy+ly*(360-(ppa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(ppa[i]<0){
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;

	case PLOT_MOONSEP:
	  {
	    // Moon Separation
	    for(i=1;i<=iend-2;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		y=dy+ly*(180-sep[i])/180;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		y=dy+ly*(180-sep[i+1])/180;
	      cairo_line_to(cr,x,y);
	      
	      if(pel_moon[i]<0){
		if(i_plan==hg->plot_i_plan){
		  cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
		  cairo_set_line_width(cr,4.0);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.2);
		  cairo_set_line_width(cr,2.0);
		}
	      }
	      else{
		if(i_plan==hg->plot_i_plan){
		  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
		  cairo_set_line_width(cr,4.0);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
		  cairo_set_line_width(cr,2.0);
		}
	      }

	      cairo_stroke(cr);
	      }
	    }
	    
	  }
	  break;

	case PLOT_HDSPA:
	  {
	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,2.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(phpa[i]-phpa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		  
		}
		else{
		  if(phpa[i]<0){
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i]-360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }

	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
	      cairo_set_line_width(cr,2.0);
	    }
	      
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-JD0)/(JD1-JD0);
		    y=dy+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;
	}
      }
    }
  }

  cairo_destroy(cr);

  switch(hg->plot_output){
  case PLOT_OUTPUT_PDF:
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
    break;

  default:
#ifdef USE_GTK3
    if(pixbuf_plot) g_object_unref(G_OBJECT(pixbuf_plot));
    pixbuf_plot=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    {
      GtkStyle *style=gtk_widget_get_style(widget);

      gdk_draw_drawable(gtk_widget_get_window(widget),
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_plot,
			0,0,0,0,
			width,
			height);
    }
    
    //g_object_unref(G_OBJECT(pixmap_plot));
#endif
    break;
  }

  return TRUE;
}


void refresh_plot (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->plot_output=PLOT_OUTPUT_WINDOW;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,hg);
    gtk_widget_queue_draw(hg->plot_dw);
  }
}


void pdf_plot (typHOE *hg)
{
  hg->plot_output=PLOT_OUTPUT_PDF;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,hg);
  }

  hg->plot_output=PLOT_OUTPUT_WINDOW;
}


// Create Plot Window
void create_plot_dialog(typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1, *ebox;
  GtkWidget *frame, *check, *label, *button;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;

  flagPlot=TRUE;

  hg->plot_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->plot_main), "HOE : Plot Window");
  
  my_signal_connect(hg->plot_main,
		    "destroy",
		    close_plot, 
		    (gpointer)hg);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->plot_main), vbox);


  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  frame = gtkut_frame_new ("<b>Parameter</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Elevation",
		       1, PLOT_EL, -1);
    if(hg->plot_mode==PLOT_EL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Azimuth",
		       1, PLOT_AZ, -1);
    if(hg->plot_mode==PLOT_AZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Atmospheric Dispersion",
		       1, PLOT_AD, -1);
    if(hg->plot_mode==PLOT_AD) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Separation from the Moon",
		       1, PLOT_MOONSEP, -1);
    if(hg->plot_mode==PLOT_MOONSEP) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS PA w/o ImR",
		       1, PLOT_HDSPA, -1);
    if(hg->plot_mode==PLOT_HDSPA) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_mode,
		       (gpointer)hg);
  }


  frame = gtkut_frame_new ("<b>Action</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (refresh_plot), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Refresh");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"window-close");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_plot), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Quit");
#endif


  icon = gdk_pixbuf_new_from_resource ("/icons/pdf_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (do_save_plot_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

  icon = gdk_pixbuf_new_from_resource ("/icons/moon_icon.png", NULL);
  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->plot_moon);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (do_plot_moon), 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Plot Moon");
#endif

  icon = gdk_pixbuf_new_from_resource ("/icons/lgs_icon.png", NULL);
  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->plot_pam);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (do_plot_pam), 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Overlay LGS Collisions");
#endif
  

  frame = gtkut_frame_new ("<b>Plot</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Single Object",
		       1, PLOT_ALL_SINGLE, -1);
    if(hg->plot_all==PLOT_ALL_SINGLE) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Selected Objects",
		       1, PLOT_ALL_SELECTED, -1);
    if(hg->plot_all==PLOT_ALL_SELECTED) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Objects",
		       1, PLOT_ALL_ALL, -1);
    if(hg->plot_all==PLOT_ALL_ALL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Observing Plan",
		       1, PLOT_ALL_PLAN, -1);
    if(hg->plot_all==PLOT_ALL_PLAN) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_all,
		       (gpointer)hg);
  }

  frame = gtkut_frame_new ("<b>Centered on</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MidNight",
		       1, PLOT_CENTER_MIDNIGHT, -1);
    if(hg->plot_center==PLOT_CENTER_MIDNIGHT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Current",
		       1, PLOT_CENTER_CURRENT, -1);
    if(hg->plot_center==PLOT_CENTER_CURRENT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Meridian",
		       1, PLOT_CENTER_MERIDIAN, -1);
    if(hg->plot_center==PLOT_CENTER_MERIDIAN) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_center,
		       (gpointer)hg);
  }

  
  // Drawing Area
  ebox=gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(vbox), ebox, TRUE, TRUE, 0);
  hg->plot_dw = gtk_drawing_area_new();

  gtk_widget_set_events(hg->plot_dw, GDK_SCROLL_MASK |
                        GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK);
  
#ifdef USE_GTK3
  my_signal_connect(hg->plot_dw, 
		    "draw", 
		    draw_plot_cb,
		    (gpointer)hg);
#else
  my_signal_connect(hg->plot_dw, 
		    "expose-event", 
		    expose_plot_cairo,
		    (gpointer)hg);
#endif
  my_signal_connect(hg->plot_dw, 
		    "configure-event", 
		    configure_plot_cb,
		    (gpointer)hg);
  
  my_signal_connect(hg->plot_dw, 
		    "scroll-event", 
		    resize_plot_cairo,
		    (gpointer)hg);
  
  gtk_widget_set_size_request (hg->plot_dw, 
			       (gint)(hg->sz_plot*1.4), 
			       hg->sz_plot);
  gtk_container_add(GTK_CONTAINER(ebox), hg->plot_dw);
  gtk_widget_set_app_paintable(hg->plot_dw, TRUE);
  gtk_widget_show(hg->plot_dw);

  gtk_widget_set_events(ebox, GDK_BUTTON_PRESS_MASK);
  my_signal_connect(ebox, 
		    "button-press-event", 
		    plot_button_signal,
		    (gpointer)hg);

  gtk_widget_show_all(hg->plot_main);

  gdk_window_raise(gtk_widget_get_window(hg->plot_main));

  draw_plot_cairo(hg->plot_dw,hg);
}


gboolean plot_button_signal(GtkWidget *widget, 
			    GdkEventButton *event, 
			    gpointer userdata){
  typHOE *hg;
  gint x,y;
  gint i_slot, i_slot_max, i_sel=-1;
  gint r_min=15, r;

  hg=(typHOE *)userdata;

  if (!hg->plot_pam) return(FALSE);
  if (hg->lgs_pam_i_max<=0) return(FALSE);

    if ( event->button==1 ) {
#ifdef USE_GTK3
    gdk_window_get_device_position(gtk_widget_get_window(widget),
				   event->device, &x,&y,NULL);
#else
    gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif

    i_slot_max=hg->lgs_pam[hg->obj[hg->plot_i].pam].line;
    
    for(i_slot=0;i_slot<i_slot_max;i_slot++){
      
      if(hg->pam_x[i_slot]>0){
	r=abs(hg->pam_x[i_slot]-x);
	if(r<r_min){
	  i_sel=i_slot;
	  r_min=r;
	}
      }
    }

    hg->pam_slot_i=i_sel;
      
    if(i_sel>=0){
      refresh_plot(NULL, (gpointer)hg);

      if(!flagPAM){
	create_pam_dialog(hg);
      }
	 
      {
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->pam_tree));
	GtkTreePath *path;
	GtkTreeIter  iter;
	  
	path=gtk_tree_path_new_first();
	
	for(i_slot=0;i_slot<i_slot_max;i_slot++){
	  if(i_slot==hg->pam_slot_i){
	    gtk_widget_grab_focus (hg->pam_tree);
	    gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->pam_tree), path, NULL, FALSE);
	    break;
	  }
	  else{
	    gtk_tree_path_next(path);
	  }
	}
	gtk_tree_path_free(path);
      }
    }
  }
  
  return FALSE;
}



void add_day(typHOE *hg, int *year, int *month, int *day, gint add_day)
{
  double JD;
  struct ln_date date;

  date.years=*year;
  date.months=*month;
  date.days=*day;
  
  date.hours=0;
  date.minutes=0;
  date.seconds=0;
  
  JD = ln_get_julian_day (&date);

  JD=JD+(gdouble)add_day;
  ln_get_date (JD, &date);

  *year=date.years;
  *month=date.months;
  *day=date.days;
}


gdouble get_julian_day_of_epoch(gdouble epoch){
  gdouble diff_y;

  diff_y=epoch-2000.0;

  return(JD2000 + diff_y*365.25);
}


gdouble get_julian_from_local_date(int iyear, int month, int iday,
				   int hour, int min, double sec,
				   long offset){
  struct ln_zonedate zonedate;

  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=hour;
  zonedate.minutes=min;
  zonedate.seconds=sec;
  zonedate.gmtoff=offset;

  return(ln_get_julian_local_date(&zonedate));
}


void geocen_to_topocen(typHOE *hg,
		       gdouble JD,
		       gdouble geo_d,
		       struct ln_equ_posn * equ_geoc,
		       struct ln_equ_posn * equ)
{
  gdouble mpar, gclat, rho, HA, g, topRA, topDec, geo_d_km;
  gdouble r_earth=6378.137;
  gdouble flst;
  gdouble f=1./298.257222101;
  
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24/360;
  
  //d_moon=ln_get_lunar_earth_dist(JD);  //km
  geo_d_km=geo_d*(AU_IN_KM);  //km
  mpar=asin((r_earth+hg->obs_altitude/1000.)/geo_d_km)*180./M_PI; //degree
  gclat=(hg->obs_latitude 
	 - 0.1924*sin(2*hg->obs_latitude*M_PI/180.))*M_PI/180.; //rad
  rho  =0.99833 + 0.00167 * cos(2*hg->obs_latitude*M_PI/180.)
        + hg->obs_altitude/1000./r_earth; // r_earth
  HA = flst - (equ_geoc->ra * 24./360.); // hour
  g = atan(tan(gclat) / cos(HA*M_PI/12.)); // rad
    
  equ->ra = equ_geoc->ra - mpar * rho * cos(gclat)
    * sin(HA*M_PI/12.) / cos(equ_geoc->dec*M_PI/180.);  // degree
  if(equ_geoc->dec>89.99){
    equ->dec = equ_geoc->dec - mpar * rho * sin(-equ_geoc->dec*M_PI/180.) 
      *cos(HA*M_PI/12.);  // degree
  }
  else{
    equ->dec = equ_geoc->dec - mpar * rho * sin(gclat) *
      sin(g-equ_geoc->dec*M_PI/180.) / sin(g);  // degree
  }
}

gdouble ra_to_deg(gdouble ra){
  double a0s;
  int ia0h,ia0m;
  double a0;
 
  a0s=ra;
  ia0h=(int)(a0s/10000);
  a0s=a0s-(double)(ia0h)*10000;
  ia0m=(int)(a0s/100);
  a0s=a0s-(double)(ia0m)*100;

  a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
  
  return(a0*360/24);
}

gdouble dec_to_deg(gdouble dec){
  double d0s;
  int id0d,id0m;
  double d0;
 
  d0s=dec;
  id0d=(int)(d0s/10000);
  d0s=d0s-(double)(id0d)*10000;
  id0m=(int)(d0s/100);
  d0s=d0s-(double)(id0m)*100;

  d0=id0d + id0m/60. + d0s/3600.;  //[deg]
  
  return(d0);
}

gdouble deg_to_ra(gdouble d_ra){
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  gdouble ra;

  object.ra=d_ra;
  object.dec=0;

  ln_equ_to_hequ (&object, &hobject);

  ra=(gdouble)hobject.ra.hours*10000
    +(gdouble)hobject.ra.minutes*100
    +(gdouble)hobject.ra.seconds;

  return(ra);
}


gdouble deg_to_dec(gdouble d_dec){ 
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  gdouble dec;

  object.ra=0;
  object.dec=d_dec;

  ln_equ_to_hequ (&object, &hobject);

  if(hobject.dec.neg){
    dec=-(gdouble)hobject.dec.degrees*10000
      -(gdouble)hobject.dec.minutes*100
      -(gdouble)hobject.dec.seconds;
  }
  else{
    dec=(gdouble)hobject.dec.degrees*10000
      +(gdouble)hobject.dec.minutes*100
      +(gdouble)hobject.dec.seconds;
  }
  
  return(dec);
}


gdouble deg_sep(gdouble az1, gdouble alt1, gdouble az2, gdouble alt2){
  double d;
  double x,y,z;
  double a1,a2,d1,d2;
  
  /* covert to radians */
  a1 = ln_deg_to_rad(az1);
  d1 = ln_deg_to_rad(alt1);
  a2 = ln_deg_to_rad(az2);
  d2 = ln_deg_to_rad(alt2);
  
  x = (cos(d1) * sin (d2)) 
    - (sin(d1) * cos(d2) * cos(a2 - a1));
  y = cos(d2) * sin(a2 - a1);
  z = (sin (d1) * sin (d2)) + (cos(d1) * cos(d2) * cos(a2 - a1));
  
  x = x * x;
  y = y * y;
  d = atan2(sqrt(x + y), z);
  
  return ln_rad_to_deg(d);
}


gdouble date_to_jd(gdouble date){
  struct ln_zonedate local_date;
  gint dtmp;
  gdouble ttmp;
  gdouble JD;

  local_date.gmtoff=0;

  dtmp=local_date.years=(gint)(date/1000000);
  ttmp=date-(gdouble)dtmp*1000000;

  local_date.years=dtmp/10000;
  local_date.months=(dtmp-local_date.years*10000)/100;
  local_date.days=dtmp-local_date.years*10000-local_date.months*100;

  local_date.hours=(gint)(ttmp/10000);
  local_date.minutes=(gint)((ttmp-(gdouble)local_date.hours*10000)/100);
  local_date.seconds=ttmp-(gdouble)local_date.hours*10000
    -(gdouble)local_date.minutes*100;

  JD=ln_get_julian_local_date(&local_date);
  
  return(JD);
}


gdouble get_meridian_JD(typHOE *hg){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  struct ln_zonedate zonedate;
  struct ln_date date;
  struct ln_equ_posn oequ;
  struct ln_equ_posn oequ_prec;
  struct ln_rst_time orst;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD_now, JD;

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  if(hg->skymon_mode==SKYMON_CUR){
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }
  else{
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
      
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0.0;
  }

  oequ.ra=ra_to_deg(hg->obj[hg->plot_i].ra);
  oequ.dec=dec_to_deg(hg->obj[hg->plot_i].dec);
  
  JD_now = get_julian_from_local_date(iyear,month,iday,hour,min,sec,
				      (long)hg->obs_timezone*60);

  ln_get_equ_prec2 (&oequ, 
		    get_julian_day_of_epoch(hg->obj[hg->plot_i].equinox),
		    JD_now, &oequ_prec);
  ln_get_object_rst (JD_now, &observer, &oequ_prec, &orst);
  ln_get_date (orst.transit, &odate);
  ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);

  JD = ln_get_julian_local_date(&transit);

  if((JD-JD_now)>0.5){
    JD_now=JD_now-1.0;
    ln_get_equ_prec2 (&oequ, 
		      get_julian_day_of_epoch(hg->obj[hg->plot_i].equinox),
		      JD_now, &oequ_prec);
    ln_get_object_rst (JD_now, &observer, &oequ_prec, &orst);
    ln_get_date (orst.transit, &odate);
    ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);
  }
  else if((JD-JD_now)<-0.5){
    JD_now=JD_now+1.0;
    ln_get_equ_prec2 (&oequ, 
		      get_julian_day_of_epoch(hg->obj[hg->plot_i].equinox),
		      JD_now, &oequ_prec);
    ln_get_object_rst (JD_now, &observer, &oequ_prec, &orst);
    ln_get_date (orst.transit, &odate);
    ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);
  }

  return(ln_get_julian_local_date(&transit));
}


gboolean resize_plot_cairo(GtkWidget *widget, 
			   GdkEventScroll *event, 
			   gpointer userdata){
  typHOE *hg;
  GdkScrollDirection direction;
  gint old_zoom;
  gint x, y;
  
  direction = event->direction;
  hg=(typHOE *)userdata;
  
  old_zoom=hg->plot_zoom;
  
  if(flagPlot){
    if(old_zoom==0){
      GtkAllocation *allocation=g_new(GtkAllocation, 1);
      gint width;
    
      gtk_widget_get_allocation(widget,allocation);
      width= allocation->width;
    
#ifdef USE_GTK3
      gdk_window_get_device_position(gtk_widget_get_window(widget),
				     event->device, &x,&y,NULL);
#else
      gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif
      if((x > (gint)(0.1*(gdouble)width))
	 && (x < (gint)(0.9*(gdouble)width))){
	hg->plot_zoomx=x;
	hg->plot_zoomr=((gdouble)x-0.5*(gdouble)width)
	  /(0.8*(gdouble)width);
      }
      else{
	hg->plot_zoomx=-1;
      }
      
      g_free(allocation);
    }
      
    if(direction & GDK_SCROLL_DOWN){
      if(hg->plot_zoom>0){
	hg->plot_zoom--;
      }
    }
    else{
      if(hg->plot_zoom<9){
	hg->plot_zoom++;
      }
    }
    
    if(hg->plot_zoom!=old_zoom){
      draw_plot_cairo(hg->plot_dw,hg);
    }
  }
  
  return(FALSE);
}
  

gboolean draw_plot_cairo(GtkWidget *widget, typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list, i_plan;
  gint from_set, to_rise;
  double dx,dy,dy2,lx,ly;
  double y0,y1,ymin,ymax;
  double x_pos, y_pos;

  double paz[200],pel[200],ppa[200],pad[200], pjd[200];
  double sep[200],phpa[200];
  
  double  alambda; //longitude[deg]
  double  alamh;   //=alambda/360.*24.;    //[hour]
  double  phi;     //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char *tmp;
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  gfloat ihst0, ihst1;
  gdouble jd0, jd1;
  gfloat ihst1_moon;
  //int ihst0=17, ihst1=31;
  double a0,d0,d0rad;
  double ut,d_ut;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec;
  double a1;
  int i,iend;
  int width, height;
  
  struct ln_zonedate zonedate, zonedate0, zonedate1;
  struct ln_date date;
  struct ln_equ_posn oequ, oequ_geoc;
  struct ln_equ_posn oequ_prec;
  struct ln_rst_time orst;
  struct ln_hrz_posn ohrz;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD, JD_hst, JD_mt, JD_tr, JD_tr0;
  gboolean Flag_moon=FALSE;
  double utstart,utend;
  double ume, den;
  double hst_mt;
  gdouble y_tr;
  gdouble scale;
  double sinha, cosha;

  gboolean plot_flag=FALSE;

  gint time_label, d_time_label;
  gfloat min_offset;

  gdouble atw_set, atw_rise, sun_rise, sun_set;

  gboolean am_flag=FALSE;

  if(!flagPlot) return (FALSE);

  get_plot_time(hg);

  my_get_local_date(hg->plot_jd0, &zonedate0,hg->obs_timezone);
  ihst0=set_ul(0., (float)zonedate0.hours+(float)zonedate0.minutes/60., 24.);

  my_get_local_date(hg->plot_jd1, &zonedate1,hg->obs_timezone);
  ihst1=set_ul(0., (float)zonedate1.hours+(float)zonedate1.minutes/60., 24.);

  /*
  printf("zoom=%d Start=%d/%d/%d %d:%d -- End=%d/%d/%d %d:%d\n",
	 hg->plot_zoom,
	 zonedate0.years,zonedate0.months,zonedate0.days,
	 zonedate0.hours,zonedate0.minutes,
	 zonedate1.years,zonedate1.months,zonedate1.days,
	 zonedate1.hours,zonedate1.minutes);
  */

  while(ihst1<ihst0){
    ihst1+=24;
  }

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;


  if(hg->plot_output==PLOT_OUTPUT_PDF){
    width= (gint)(hg->sz_plot*1.5);
    height= hg->sz_plot;
    scale=1.0;

    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    if(hg->plot_pam){
      dy2=height*0.2;
      ly=height*0.7;
    }
    else{
      dy2=height*0.1;
      ly=height*0.8;
    }

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 
    
  }
  else{
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);

    width= allocation->width;
    height= allocation->height;
    g_free(allocation);

    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
    }
    scale=1.0;
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    if(hg->plot_pam){
      dy2=height*0.2;
      ly=height*0.7;
    }
    else{
      dy2=height*0.1;
      ly=height*0.8;
    }

#ifdef USE_GTK3
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);

    cr = cairo_create(surface);
#else
    if(pixmap_plot) g_object_unref(G_OBJECT(pixmap_plot));
    pixmap_plot = gdk_pixmap_new(gtk_widget_get_window(widget),
				 width,
				 height,
				 -1);
  
    cr = gdk_cairo_create(pixmap_plot);
#endif
  }

  if(hg->plot_output==PLOT_OUTPUT_PDF){
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
  }
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if((hg->plot_all!=PLOT_ALL_PLAN) && (hg->plot_i>=0)){
    // Object Name etc.
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    
    tmp=g_strdup_printf("\"%s\"  RA=%09.2f Dec=%+010.2f Equinox=%7.2f",
			hg->obj[hg->plot_i].name,
			hg->obj[hg->plot_i].ra,
			hg->obj[hg->plot_i].dec,
			hg->obj[hg->plot_i].equinox);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,+extents.height);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
  }

  /* draw a rectangle */
 
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_rectangle(cr,  dx,dy2, lx,ly);
  cairo_fill(cr);


  switch(hg->plot_mode){
  case PLOT_EL:
    // El
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy2);
      cairo_line_to ( cr, width-dx, dy2);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy2);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){

	cairo_move_to ( cr, dx, height-dy-ly*70/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*70/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*80/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*80/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*50/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*50/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*40/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*40/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*20/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*20/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*45/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*45/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "80", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(90-80)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "80");
    
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);

    }// El
    break;
    
  case PLOT_AZ:
    // Az
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy2);
      cairo_line_to ( cr, width-dx, dy2);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy2);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy2+ly*45/360);
      cairo_line_to ( cr, width-dx, dy2+ly*45/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*90/360);
      cairo_line_to ( cr, width-dx, dy2+ly*90/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*135/360);
      cairo_line_to ( cr, width-dx, dy2+ly*135/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*180/360);
      cairo_line_to ( cr, width-dx, dy2+ly*180/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*225/360);
      cairo_line_to ( cr, width-dx, dy2+ly*225/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*270/360);
      cairo_line_to ( cr, width-dx, dy2+ly*270/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy2+ly*315/360);
      cairo_line_to ( cr, width-dx, dy2+ly*315/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy2+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy2+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }


      
      // Az Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "Azimuth[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Azimuth[deg]");
      cairo_restore (cr);

    }// Az
    break;
  case PLOT_AD:
    // AD
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy2);
      cairo_line_to ( cr, width-dx, dy2);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy2);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-3.5)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-3.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-2.5)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-2.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-1.5)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-1.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy2+ly*(4-0.5)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-0.5)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
     

      // AD Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);

      cairo_text_extents (cr, "4.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "4.0");
      
      cairo_text_extents (cr, "2.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(4-2)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "2.0");
      
      cairo_text_extents (cr, "1.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(4-1)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "1.0");
      
      cairo_text_extents (cr, "0.5", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(4-0.5)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0.5");

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy2+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy2+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);

      tmp=g_strdup_printf("AD[\"] (%d-%dA)", hg->wave1,hg->wave0);
      cairo_text_extents (cr, tmp, &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
      cairo_restore (cr);


      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "+180", &extents);
      x = dx+lx+5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "Zenith PA[deg]", &extents);

      x = dx+lx-extents.width/2+x0-(dx+lx+x0);
      y = height/2+extents.height-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Zenith PA[deg]");
      cairo_restore (cr);
      
    }// AD
    break;
  case PLOT_MOONSEP:
    // Moon Separation
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy2+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy2);
      cairo_line_to ( cr, width-dx, dy2);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy2);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_set_source_rgba(cr, 0.4, 0.8, 0.4,1.0);

      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

      cairo_move_to ( cr, dx, dy2+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy2+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);


      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%90){
	    cairo_move_to ( cr, dx, dy2+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy2+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      x0=extents.width;

      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);



      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "180", &extents);
      x = dx+lx+5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "180");
      
      cairo_text_extents (cr, "135", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "135");
      
      cairo_text_extents (cr, "90", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");

      cairo_text_extents (cr, "45", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "45");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "135", &extents);
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "Separation from the Moon[deg]", &extents);

      x = dx+lx-extents.width/2-(dx+lx);
      y = 5+x0;
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Separation from the Moon[deg]");
      cairo_restore (cr);
      
    }
    break;
  case PLOT_HDSPA:
    // HDS PA w/o ImR
    {
      gfloat x0, w1;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy2);
      cairo_line_to ( cr, width-dx, dy2);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy2);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy2+ly*45/360);
      cairo_line_to ( cr, width-dx, dy2+ly*45/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*90/360);
      cairo_line_to ( cr, width-dx, dy2+ly*90/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*135/360);
      cairo_line_to ( cr, width-dx, dy2+ly*135/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*180/360);
      cairo_line_to ( cr, width-dx, dy2+ly*180/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*225/360);
      cairo_line_to ( cr, width-dx, dy2+ly*225/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy2+ly*270/360);
      cairo_line_to ( cr, width-dx, dy2+ly*270/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy2+ly*315/360);
      cairo_line_to ( cr, width-dx, dy2+ly*315/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy2+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy2+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }


      
      // HDS PA  Text
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "S is UP", &extents);
      x = dx+lx+5*scale;
      y = dy2-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "S is UP");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5*scale;
      y = dy2+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);
      cairo_text_extents (cr, "  & Zenith PA [deg]", &extents);
      w1=extents.width;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);

      x = dx-(extents.width+w1)/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "HDS PA w/o ImR[deg]");

      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);
      x += extents.width;

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "  & Zenith PA[deg]");

      cairo_restore (cr);

    }// HDS PA w/o ImR
    break;
  }

  // HST
  {
    cairo_rectangle(cr, dx,dy2,lx,ly);
    cairo_clip(cr);


    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    if(hg->skymon_mode!=SKYMON_CUR){
      sun_set=(gfloat)hg->sun.s_set.hours+(gfloat)hg->sun.s_set.minutes/60.;
      sun_rise=(gfloat)hg->sun.s_rise.hours+(gfloat)hg->sun.s_rise.minutes/60.+24.;
      
      if( (ihst1<24) && ((ihst1<sun_set)&&((sun_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy2, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (sun_set>ihst0) && (sun_set<ihst1) ){
	  x=lx*(sun_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy2, x,ly);
	  cairo_fill(cr);
	  
	  tmp=g_strdup_printf("%d:%02d",
			      hg->sun.s_set.hours,hg->sun.s_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
	
	if( (sun_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (sun_rise-24.)<ihst1){
	    x=lx*(ihst1-(sun_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( sun_rise < ihst1 ){
	    x=lx*(ihst1-sun_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  tmp=g_strdup_printf("%d:%02d",
			      hg->sun.s_rise.hours,hg->sun.s_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy2, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
      }


      // Astronomical Twilight = 18deg

      atw_set=(gfloat)hg->atw18.s_set.hours+(gfloat)hg->atw18.s_set.minutes/60.;
      atw_rise=(gfloat)hg->atw18.s_rise.hours+(gfloat)hg->atw18.s_rise.minutes/60.+24.;

      if( (ihst1<24) && ((ihst1<atw_set)&&((atw_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy2, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (atw_set>ihst0) && (atw_set<ihst1) ){
	  x=lx*(atw_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy2, x,ly);
	  cairo_fill(cr);
	  
	  tmp=g_strdup_printf("%d:%02d",
			      hg->atw18.s_set.hours,hg->atw18.s_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
	
	if( (atw_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (atw_rise-24.)<ihst1){
	    x=lx*(ihst1-(atw_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( atw_rise < ihst1 ){
	    x=lx*(ihst1-atw_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  tmp=g_strdup_printf("%d:%02d",
			      hg->atw18.s_rise.hours,hg->atw18.s_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy2, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
      }
    }
    else{
      sun_set=(gfloat)hg->sun.c_set.hours+(gfloat)hg->sun.c_set.minutes/60.;
      sun_rise=(gfloat)hg->sun.c_rise.hours+(gfloat)hg->sun.c_rise.minutes/60.+24.;
      
      if( (ihst1<24) && ((ihst1<sun_set)&&((sun_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy2, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (sun_set>ihst0) && (sun_set<ihst1) ){
	  x=lx*(sun_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy2, x,ly);
	  cairo_fill(cr);
	  
	  tmp=g_strdup_printf("%d:%02d",
			      hg->sun.c_set.hours,hg->sun.c_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
	
	if( (sun_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (sun_rise-24.)<ihst1){
	    x=lx*(ihst1-(sun_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( sun_rise < ihst1 ){
	    x=lx*(ihst1-sun_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  tmp=g_strdup_printf("%d:%02d",
			      hg->sun.c_rise.hours,hg->sun.c_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy2, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
      }


      // Astronomical Twilight = 18deg

      atw_set=(gfloat)hg->atw18.c_set.hours+(gfloat)hg->atw18.c_set.minutes/60.;
      atw_rise=(gfloat)hg->atw18.c_rise.hours+(gfloat)hg->atw18.c_rise.minutes/60.+24.;

      if( (ihst1<24) && ((ihst1<atw_set)&&((atw_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy2, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (atw_set>ihst0) && (atw_set<ihst1) ){
	  x=lx*(atw_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy2, x,ly);
	  cairo_fill(cr);
	  
	  tmp=g_strdup_printf("%d:%02d",
			      hg->atw18.c_set.hours,hg->atw18.c_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
	
	if( (atw_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (atw_rise-24.)<ihst1){
	    x=lx*(ihst1-(atw_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( atw_rise < ihst1 ){
	    x=lx*(ihst1-atw_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  tmp=g_strdup_printf("%d:%02d",
			      hg->atw18.c_rise.hours,hg->atw18.c_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy2, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy2+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
      }

    }
    cairo_reset_clip(cr);


    // vertical lines
    cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
    
    min_offset=ihst0-(gint)ihst0;
    time_label=(gint)ihst0+1;
    do{
      cairo_move_to ( cr, dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0), dy2);
      cairo_line_to ( cr, dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0), ly+dy2);
      if(time_label%2)
	cairo_set_line_width(cr,0.5*scale);
      else
	cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      time_label+=1;
    }while(time_label<=(gint)ihst1);


    if(ihst1-ihst0<6.0){
      double dashes[] = {8.0,  /* ink */
			 4.0,  /* skip */
			 8.0,  /* ink */
			 4.0   /* skip*/
      };

      int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
      double offset = -10.0;
      gfloat time_label_f = (gint)ihst0+0.5;
      
      if(time_label_f<ihst0) time_label_f+=1.0;

      cairo_save(cr);

      cairo_set_dash (cr, dashes, ndash, offset);
      cairo_set_line_width (cr, 0.5*scale);

      do{
	cairo_move_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), dy2);
	cairo_line_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), ly+dy2);
	cairo_stroke(cr);

	time_label_f+=1.0;
      }while(time_label_f<ihst1);
      
      cairo_restore(cr);
    }
    
    if(ihst1-ihst0<3.0){
      double dashes[] = {2.0,  /* ink */
			 3.0,  /* skip */
			 2.0,  /* ink */
			 3.0   /* skip*/
      };

      int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
      double offset = -10.0;
      gfloat time_label_f = (gint)ihst0+10./60.;
      int num=1;
      
      cairo_save(cr);

      cairo_set_dash (cr, dashes, ndash, offset);
      cairo_set_line_width (cr, 0.3*scale);

      do{
	if(num%3){
	  cairo_move_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), dy2);
	  cairo_line_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), ly+dy2);
	  cairo_stroke(cr);
	}
	num++;
	time_label_f+=10./60.;
      }while(time_label_f<ihst1);
      
      cairo_restore(cr);
    }
    
    
    cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    if(ihst1-ihst0<10.0){
      d_time_label=1;
      time_label=(gint)ihst0;
    }
    else{
      d_time_label=2;
      time_label=(gint)ihst0+1;
    }

    do{
      if(time_label>=24){
	tmp=g_strdup_printf("%d",time_label-24);
      }
      else{
	tmp=g_strdup_printf("%d",time_label);
      }

      if((gfloat)time_label>=ihst0){
	cairo_text_extents (cr, tmp, &extents);
	x = dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0)-extents.width/2;
	y = dy2+ly+extents.height+5*scale;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
      }
      if(tmp) g_free(tmp);
      
      time_label+=d_time_label;
    }while(time_label <= (gint)ihst1);

    if(ihst1-ihst0<3.0){  // draw 30 minutes in x label
      time_label=(gint)ihst0;
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale*0.9);
      
      do{
	if(time_label>=24){
	  tmp=g_strdup_printf("%d:30",time_label-24);
	}
	else{
	  tmp=g_strdup_printf("%d:30",time_label);
	}
	
	if((gfloat)time_label+0.5>=ihst0){
	  cairo_text_extents (cr, tmp, &extents);
	  x = dx+lx*(-min_offset+0.5+time_label-(gint)ihst0)/(ihst1-ihst0)-extents.width/2;
	  y = dy2+ly+extents.height+5*scale;
	  cairo_move_to(cr, x, y);
	  cairo_show_text(cr, tmp);
	}
	if(tmp) g_free(tmp);
      
	time_label+=d_time_label;
      }while((gdouble) time_label + 0.5 <= ihst1);
    }


    // Date
    if(hg->skymon_mode==SKYMON_CUR){
      get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
    }
    else{
      iyear=hg->skymon_year;
      month=hg->skymon_month;
      iday=hg->skymon_day;
      
      hour=hg->skymon_hour;
      min=hg->skymon_min;
      sec=0.0;
    }
  
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
    cairo_text_extents (cr, hg->obs_tzname, &extents);
    x = width/2-extents.width/2;
    y += extents.height+5*scale;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, hg->obs_tzname);

    // Date label
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

    tmp=g_strdup_printf("(%4d/%2d/%2d)",
			zonedate0.years,
			zonedate0.months,
			zonedate0.days);
    cairo_text_extents (cr, tmp, &extents);
    x = dx;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    if(zonedate0.days!=zonedate1.days){
      tmp=g_strdup_printf("(%4d/%2d/%2d)",
			  zonedate1.years,
			  zonedate1.months,
			  zonedate1.days);
      cairo_text_extents (cr, tmp, &extents);
      x = dx+lx-extents.width;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      am_flag=TRUE;
    }

    // Current Time
    if(hg->plot_all!=PLOT_ALL_PLAN){
      if(((gfloat)hour+(gfloat)min/60.<(ihst1-24.)) 
	 || (((gfloat)hour+(gfloat)min/60.>=ihst0)&&((gfloat)hour+(gfloat)min/60.<ihst1))){
	cairo_save(cr);
	
	if((gfloat)hour+(gfloat)min/60.<(ihst1-24)){
	  hour+=24;
	}

	if(hg->plot_pam){
	  cairo_set_source_rgba(cr, 0.0, 0.8, 0.0, 1.0);
	}
	else{
	  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	}
	x=dx+lx*((gfloat)hour-ihst0+(gfloat)min/60.)/(ihst1-ihst0);
	cairo_move_to ( cr, x, dy2);
	cairo_line_to ( cr, x, ly+dy2);
	cairo_set_line_width(cr,3.0*scale);
	cairo_stroke(cr);
      
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	if(hg->plot_pam){
	  cairo_set_source_rgba(cr, 0.0, 0.8, 0.0, 1.0);
	}
	else{
	  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	}
	if(hour>=24){
	  tmp=g_strdup_printf("%d:%02d",hour-24,min);
	}
	else if(hour<0){
	  tmp=g_strdup_printf("%d:%02d",hour+24,min);
	}
	else{
	  tmp=g_strdup_printf("%d:%02d",hour,min);
	}
	cairo_text_extents (cr, tmp, &extents);
	if(hg->plot_pam){
	  x += 5*scale + extents.height;
	  y = dy2 + ly -5*scale;
	}
	else{
	  x += -extents.width/2;
	  y = dy2 -5*scale;
	}
	cairo_move_to(cr, x, y);
	if(hg->plot_pam){
	  cairo_rotate (cr, -M_PI/2);
	}
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);

	if(hg->plot_mode==PLOT_AZ){
	  gdouble vaz;
	  
	  if(hg->skymon_mode==SKYMON_CUR){
	    vaz=hg->obj[hg->plot_i].c_vaz;
	  }
	  else{
	    vaz=hg->obj[hg->plot_i].s_vaz;
	  }
	  
	  if(vaz!=-100){
	    x -= -extents.width/2;
	    y -= extents.height+2;
	    
	    tmp=g_strdup_printf("%+.2fdeg/min",vaz);
	    cairo_text_extents (cr, tmp, &extents);
	    x += -extents.width/2;
	    cairo_move_to(cr, x, y);
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
	    cairo_show_text(cr, tmp);
	    if(tmp) g_free(tmp);
	  }
	}
	else if(hg->plot_mode==PLOT_AD){
	  gdouble vpa;
	  
	  if(hg->skymon_mode==SKYMON_CUR){
	    vpa=hg->obj[hg->plot_i].c_vpa;
	  }
	  else{
	    vpa=hg->obj[hg->plot_i].s_vpa;
	  }
	  
	  if(vpa!=-100){
	    x -= -extents.width/2;
	    y -= extents.height+2;
	    
	    tmp=g_strdup_printf("%+.3fdeg/min",vpa);
	    cairo_text_extents (cr, tmp, &extents);
	    x += -extents.width/2;
	    cairo_move_to(cr, x, y);
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	    cairo_show_text(cr, tmp);
	    if(tmp) g_free(tmp);
	  }
	}
	else if( hg->plot_mode==PLOT_MOONSEP ){
	  gdouble sep;
	  
	  if(hg->skymon_mode==SKYMON_CUR){
	    sep=hg->obj[hg->plot_i].c_sep;
	  }
	  else{
	    sep=hg->obj[hg->plot_i].s_sep;
	  }
	  
	  if(sep>0){
	    x -= -extents.width/2;
	    y -= extents.height+2;
	    
	    tmp=g_strdup_printf("%.1fdeg",sep);
	    cairo_text_extents (cr, tmp, &extents);
	    x += -extents.width/2;
	    cairo_move_to(cr, x, y);
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	    cairo_show_text(cr, tmp);
	    if(tmp) g_free(tmp);
	  }
	}
	else if( hg->plot_mode==PLOT_HDSPA ){
	  gdouble vpa;
	  
	  if(hg->skymon_mode==SKYMON_CUR){
	    vpa=hg->obj[hg->plot_i].c_vhpa;
	  }
	  else{
	    vpa=hg->obj[hg->plot_i].s_vhpa;
	  }
	  
	  if(vpa!=-100){
	    x -= -extents.width/2;
	    y -= extents.height+2;
	    
	    tmp=g_strdup_printf("%+.3fdeg/min",vpa);
	    cairo_text_extents (cr, tmp, &extents);
	    x += -extents.width/2;
	    cairo_move_to(cr, x, y);
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	    cairo_show_text(cr, tmp);
	    if(tmp) g_free(tmp);
	  }
	}
	cairo_restore(cr);
      }
    }
  }// HST

  
  // Plot PAM (LGS Collisions)
  if(hg->plot_pam){
    if(hg->obj[hg->plot_i].pam>=0){
      gint i_slot, n_pam;
      gdouble JD_st, JD_ed, old_JD_ed, JD0, JD1, dur_s;
      gdouble x_0, x_1, x_c;
      gboolean flag_plot;
      
      cairo_rectangle(cr, dx,0,lx,height);
      cairo_clip(cr);
      
      n_pam=hg->obj[hg->plot_i].pam;
      
      for(i_slot=0;i_slot<=hg->lgs_pam[n_pam].line;i_slot++){
	hg->pam_x[i_slot]=-1;
	hg->pam_y[i_slot]=-1;
	
	if(i_slot==0){
	  JD_st=hg->lgs_pam[n_pam].time[i_slot].st;
	  JD_ed=hg->lgs_pam[n_pam].time[i_slot].ed;
	  
	  if(JD_st>hg->plot_jd0){
	    JD0=hg->plot_jd0;
	    JD1=JD_st;
	    flag_plot=TRUE;
	  }
	  else{
	    flag_plot=FALSE;
	  }
	}
	else if(i_slot==hg->lgs_pam[n_pam].line){
	  if(old_JD_ed<hg->plot_jd1){
	  JD0=old_JD_ed;
	  JD1=hg->plot_jd1;
	  flag_plot=TRUE;
	  }
	  else{
	    flag_plot=FALSE;
	  }
	}
	else{
	  JD_st=hg->lgs_pam[n_pam].time[i_slot].st;
	  JD_ed=hg->lgs_pam[n_pam].time[i_slot].ed;
	  
	  JD0=old_JD_ed;
	  JD1=JD_st;
	  flag_plot=TRUE;
	}
	
	if(flag_plot){
	  dur_s=(JD1-JD0)*24.0*60.0*60.0;
	  
	  x_0=dx+lx*(JD0-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	  x_1=dx+lx*(JD1-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	  x_c=(x_0+x_1)/2.0;

	  if((hg->pam_slot_i>=0) && (hg->pam_slot_i==i_slot-1)){
	    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.8);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.4);
	  }
	  cairo_rectangle(cr,
			  x_0,
			  dy2,
			  x_1-x_0,
			  ly);
	  cairo_fill(cr);

	  if(i_slot>0){
	    if( (x_c>dx) && (x_c<dx+lx)){
	      if(hg->pam_slot_i==i_slot-1){
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.8);
		cairo_move_to(cr, x_c,   dy2-10);
		cairo_line_to(cr, x_c-5, dy2-20);
		cairo_line_to(cr, x_c+5, dy2-20);
	      }
	      else{
		cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.4);
		cairo_move_to(cr, x_c,   dy2-10);
		cairo_line_to(cr, x_c-4, dy2-18);
		cairo_line_to(cr, x_c+4, dy2-18);
	      }
	      cairo_close_path(cr);
	      cairo_fill(cr);
		
	      
	      hg->pam_x[i_slot-1]=(gint)x_c;
	      hg->pam_y[i_slot-1]=(gint)dy2-15;;

	      cairo_save (cr);
	      cairo_translate (cr, x_c, dy2-25);
	      cairo_rotate (cr,-M_PI/2);
	      
	      if(hg->pam_slot_i==i_slot-1){
		cairo_select_font_face (cr, hg->fontfamily_all,
					CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.3*scale);
	      }
	      else{
		cairo_select_font_face (cr, hg->fontfamily_all,
					CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	      }
	      
	      if(dur_s<60){
		tmp=g_strdup_printf("%d", (gint)dur_s);
	      }
	      else{
		tmp=g_strdup_printf("%d\'%02d",
				    (gint)(dur_s/60), (gint)dur_s%60);
	      }
	      cairo_text_extents (cr, tmp, &extents);
	      
	      x = 5;
	      y = extents.height/2.;
	      cairo_move_to(cr, x, y);
	      
	      cairo_show_text(cr, tmp);
	      cairo_restore (cr);
	    }
	  }
	
	}
	old_JD_ed=JD_ed;
      }
    
    
      cairo_reset_clip(cr);
    }
    else{
      cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.8);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.5*scale);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      tmp=g_strdup_printf("!!!!! No PAM is found for this target !!!!!");
      cairo_text_extents (cr, tmp, &extents);
      cairo_move_to(cr, width/2-extents.width/2, dy2/2);
      cairo_show_text(cr, tmp);
      g_free(tmp);
    }
  }
  

  //////////////////////////////////////////
  

  alambda=hg->obs_longitude;  //longitude[deg]
  alamh=alambda/360.*24.;     //[hour]
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  /*
  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=(gint)ihst0;
  zonedate.minutes=(gint)((ihst0-(gint)ihst0)*60.);
  zonedate.seconds=0.0;
  zonedate.gmtoff=(long)hg->obs_timezone*60;
  ln_zonedate_to_date(&zonedate, &date);
  */

  /* Lunar RA, DEC */
  if(((hg->plot_moon)&&(hg->plot_mode==PLOT_EL))
     ||(hg->plot_mode==PLOT_MOONSEP)){

    //JD = ln_get_julian_local_date(&zonedate);
    JD = (hg->plot_jd0+hg->plot_jd1)/2;


    // Re-calculate Moon Position?
    if(fabs(JD-JD_moon_stock)>0.8){  // Completely new date

      ln_get_lunar_rst (JD, &observer, &orst);
      ln_get_date (orst.transit, &odate);
      ln_date_to_zonedate(&odate,&moon_transit,(long)hg->obs_timezone*60);
      
      JD_mt = ln_get_julian_local_date(&moon_transit);

      if((JD-JD_mt)>0.5){  
	JD=JD+1.0;
	ln_get_lunar_rst (JD, &observer, &orst);
	ln_get_date (orst.transit, &odate);
	ln_date_to_zonedate(&odate,&moon_transit,(long)hg->obs_timezone*60);
      
	JD_mt = ln_get_julian_local_date(&moon_transit);
	//printf("Use -1 day transit\n");
      }
      else if((JD-JD_mt)<-0.5){ // use transit the next night
	JD=JD-1.0;
	ln_get_lunar_rst (JD, &observer, &orst);
	ln_get_date (orst.transit, &odate);
	ln_date_to_zonedate(&odate,&moon_transit,(long)hg->obs_timezone*60);
      
	JD_mt = ln_get_julian_local_date(&moon_transit);
	//printf("Use +1 day transit\n");
      }

      if(fabs(JD_mt-JD_moon_stock)>0.5){
	//printf("New Moon Calc.\n");
	Flag_moon=TRUE;
      }
      //printf("%lf   %lf   %lf\n",hg->plot_jd0,JD_mt,hg->plot_jd1);
    }
    else{
      JD_mt=JD_moon_stock;
    }
    
    // at first get transit elevation
    if(Flag_moon){  
      ln_get_lunar_equ_coords (orst.transit, &oequ_geoc);
      calc_moon_topocen(hg, orst.transit, &oequ_geoc, &oequ);

      ln_get_hrz_from_equ (&oequ, &observer, orst.transit, &ohrz);
      moon_tr_el=ohrz.alt;
    }
    
    i=1;
    d_ut=16.0/190.0;
    
    JD_hst = JD_mt-8./24.;
   
    while(JD_hst<JD_mt+8./24.){
      if(Flag_moon){  
	ln_get_lunar_equ_coords (JD_hst, &oequ_geoc);
	calc_moon_topocen(hg, JD_hst, &oequ_geoc, &oequ);
	ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	if(ohrz.az>180) ohrz.az-=360;

	paz_moon[i]=ohrz.az;
	pel_moon[i]=ohrz.alt;
	jd_moon[i]=JD_hst;
      }
      
      ut=ut+d_ut;
      JD_hst+=d_ut/24.;
      
      i=i+1;      
    }
    
    iend=i-1;

    if(Flag_moon){  
      i_moon_max=iend;
      JD_moon_stock = JD_mt;
    }
    
    cairo_rectangle(cr, dx,dy2,lx,ly);
    cairo_clip(cr);
    cairo_new_path(cr);
    
    switch(hg->plot_mode){
    case PLOT_EL:
    case PLOT_MOONSEP:
      if(hg->plot_moon){
	
	cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 0.5);
	cairo_set_line_width(cr,5.0*scale);
	
	for(i=1;i<=i_moon_max-1;i++){
	  x=dx+lx*(jd_moon[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	  y=dy2+ly*(90-pel_moon[i])/90;
	  cairo_move_to(cr,x,y);
	    
	  x=dx+lx*(jd_moon[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	  y=dy2+ly*(90-pel_moon[i+1])/90;
	  cairo_line_to(cr,x,y);
	  
	  cairo_stroke(cr);
	  
	}
	
	if((JD_mt>hg->plot_jd0)&&(JD_mt<hg->plot_jd1)){
	  cairo_move_to(cr,dx+lx*(JD_mt-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0),
			dy2+ly*(90.-moon_tr_el)/90.);
	  
	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 1.0);
	  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
	  
	  cairo_text_extents (cr, "moon", &extents);
	  cairo_get_current_point(cr,&x_pos,&y_pos);
	  if(y_pos-extents.width < dy2){
	    cairo_move_to(cr, x_pos, y_pos+extents.width+15*scale);
	  }

	  
	  cairo_save(cr);
	  cairo_rotate (cr,-M_PI/2);
	  cairo_rel_move_to(cr,5.*scale,+extents.height/2.);
	  cairo_reset_clip(cr);
	  cairo_show_text(cr,"moon");
	  cairo_restore(cr);
	}
      }
      break;
    }
    cairo_reset_clip(cr);
  }

  if(hg->plot_all!=PLOT_ALL_PLAN){
    // Objects
    for(i_list=0;i_list<hg->i_max;i_list++){
      switch(hg->plot_all){ 
      case PLOT_ALL_SINGLE:
	if(i_list==hg->plot_i)
	  plot_flag=TRUE;
	else
	  plot_flag=FALSE;
	break;
	
      case PLOT_ALL_SELECTED:
	plot_flag=hg->obj[i_list].check_sm;
	break;
	
      case PLOT_ALL_ALL:
	plot_flag=TRUE;
      break;
      }
      
      if(plot_flag){
	oequ.ra=ra_to_deg(hg->obj[i_list].ra);
	oequ.dec=dec_to_deg(hg->obj[i_list].dec);
	
	//JD = ln_get_julian_local_date(&zonedate);
	JD = (hg->plot_jd0+hg->plot_jd1)/2.0;
	
	ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[i_list].equinox),
			  JD, &oequ_prec);
	ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
	ln_get_date (orst.transit, &odate);
	ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);
	
	JD_tr = ln_get_julian_local_date(&transit);
	
	if((JD_tr<hg->plot_jd0)||(JD_tr>hg->plot_jd1)){
	  if(JD_tr<hg->plot_jd0){
	    JD=JD+1.0;
	  }
	  else{
	    JD=JD-1.0;
	  }
	  
	  ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[i_list].equinox),
			    JD, &oequ_prec);
	  ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
	  ln_get_date (orst.transit, &odate);
	  ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*60);
	  
	  JD_tr0 = ln_get_julian_local_date(&transit);
	  
	  JD = (hg->plot_jd0+hg->plot_jd1)/2.0;
	  if(fabs(JD-JD_tr0)<fabs(JD-JD_tr)){
	    JD_tr=JD_tr0;
	  }
	}
	
	a0=oequ_prec.ra*24./360.; //[hour]
	d0rad=oequ_prec.dec*M_PI/180.;
	
	i=1;
	d_ut=(gdouble)(hg->plot_jd1-hg->plot_jd0)*24.0/190.0;
	
	JD_hst=hg->plot_jd0;
	
	while(JD_hst<hg->plot_jd1+d_ut/24.){
	  ln_get_hrz_from_equ (&oequ_prec, &observer, JD_hst, &ohrz);
	  
	  flst=ln_get_mean_sidereal_time(JD_hst)+hg->obs_longitude/360.*24.;
	  
	  if(ohrz.az>180) ohrz.az-=360;
	  paz[i]=ohrz.az+180;
	  pel[i]=ohrz.alt;
	  
	  if(hg->plot_mode==PLOT_MOONSEP){
	    sep[i]=get_moon_sep(paz[i]-180,pel[i],JD_hst);
	  }
	  
	  ha=flst-a0;             //hour angle [hour]
	  ha=set_ul(-12., ha, 12.);
	  
	  el0=(gdouble)ohrz.alt*M_PI/180.;
	  az0=(180.+(gdouble)ohrz.az)*M_PI/180.;
	  if(az0<0.) az0=az0+2.*M_PI;
	  
	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=el0+M_PI*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
	  d1=d1rad*180./M_PI;
	  ume1=-sin(az0)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./M_PI;
	  
	  ha1=set_ul(-12., ha1, 12.);
	  a1=flst-ha1;
	  a1=set_ul(0., a1, 24.);
	  
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  delta_a=(a1-a0)*M_PI/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/M_PI;
	  
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=M_PI*(90.0-ohrz.alt)/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  if(ohrz.alt>3){
	    if(hg->wave1<hg->wave0){
	      adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	    }
	    else{
	      adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	    }
	  }
	  else{
	    adsec=100;
	  }
	  
	  pjd[i]=JD_hst;
	  
	  ppa[i]=padeg;
	  pad[i]=adsec;
	  // HDS PA w/o ImR
	  phpa[i]=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
	  
	  ut=ut+d_ut;
	  JD_hst+=d_ut/24.;
	  
	  
	  i=i+1;
	}
	
	iend=i-1;
	
	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_rectangle(cr, dx,dy2,lx,ly);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	//printf("%lf  %lf  %lf\n",hg->plot_jd0,JD_tr,hg->plot_jd1);
	
	switch(hg->plot_mode){
	case PLOT_EL:
	  {
	    
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	    }
	    
	    if((JD_tr>hg->plot_jd0)&&(JD_tr<hg->plot_jd1)){
	      y_tr=fabs(hg->obs_latitude-oequ_prec.dec);
	      
	      cairo_move_to(cr,dx+lx*(JD_tr-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0),
			    dy2+ly*y_tr/90.);
	      
	      if(i_list==hg->plot_i){
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
		
		if(oequ_prec.dec>hg->obs_latitude){
		  tmp=g_strdup_printf("North(%.2f)",
				      90.0-fabs(oequ_prec.dec-hg->obs_latitude));
		}
		else{
		  tmp=g_strdup_printf("South(%.2f)",
				      90.0-fabs(oequ_prec.dec-hg->obs_latitude));
		}
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
		
		cairo_text_extents (cr, tmp, &extents);
		cairo_save(cr);
		cairo_rel_move_to(cr,-extents.width/2.,-5*scale);
		cairo_reset_clip(cr);
		cairo_show_text(cr,tmp);
		if(tmp) g_free(tmp);

		cairo_move_to(cr,dx+lx*(JD_tr-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0),
			      dy2+ly*y_tr/90.);
		cairo_rel_move_to(cr,0,-extents.height-5*scale);

		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
		
	      }
	      else{
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	      }
	      
	      tmp=g_strdup_printf("%s",hg->obj[i_list].name);
	      cairo_text_extents (cr, tmp, &extents);
	      cairo_get_current_point(cr,&x_pos,&y_pos);
	      if(y_pos-extents.width < dy2){
		cairo_move_to(cr, x_pos, dy2+ly*y_tr/90.+15*scale+extents.width);
	      }
	      
	      cairo_save(cr);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_get_current_point(cr,&x_pos,&y_pos);
	      cairo_rel_move_to(cr,5.*scale,+extents.height/2.);
	      cairo_reset_clip(cr);
	      cairo_show_text(cr,tmp);
	      if(tmp) g_free(tmp);
	      cairo_restore(cr);
	    }
	  }
	  break;
	  
	case PLOT_AZ:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(paz[i]-paz[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(paz[i]<180){
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-paz[i])/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(paz[i+1]-360))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(paz[i]+360))/360;
		    cairo_move_to(cr,x,y);
		  
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-paz[i+1])/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_set_line_width(cr,2.0*scale);
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;
	  
	case PLOT_AD:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    for(i=1;i<=iend-1;i++){
	      if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
		x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(4-pad[i])/4.;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(4-pad[i+1])/4.;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
		
	      }
	    }
	    
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;

	case PLOT_MOONSEP:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	    }
	  
	    if((JD_tr>hg->plot_jd0)&&(JD_tr<hg->plot_jd1)){
	      y_tr=fabs(hg->obs_latitude-oequ_prec.dec);
	      
	      cairo_move_to(cr,dx+lx*(JD_tr-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0),
			    dy2+ly*y_tr/90.);
	      
	      if(i_list==hg->plot_i){
		cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
		cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
		
		if(oequ_prec.dec>hg->obs_latitude){
		  tmp=g_strdup_printf("North(%.2f)",
				      90.0-fabs(oequ_prec.dec-hg->obs_latitude));
		}
		else{
		  tmp=g_strdup_printf("South(%.2f)",
				      90.0-fabs(oequ_prec.dec-hg->obs_latitude));
		}
		
		cairo_text_extents (cr, tmp, &extents);
		cairo_save(cr);
		cairo_rel_move_to(cr,-extents.width/2.,-5.*scale);
		cairo_reset_clip(cr);
		cairo_show_text(cr,tmp);
		if(tmp) g_free(tmp);
		cairo_restore(cr);
	      }
	    }
	    
	    // Moon Separation
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-2;i++){
	      if(((pel[i]>0)&&(pel[i+1]>0))&&((sep[i]>0)&&sep[i+1]>0)){
		x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(180-sep[i])/180;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(180-sep[i+1])/180;
		cairo_line_to(cr,x,y);
		
		if(i_list==hg->plot_i){
		  cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
		  cairo_set_line_width(cr,3.0*scale);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
		  cairo_set_line_width(cr,1.0*scale);
		}
		
		cairo_stroke(cr);
	      }
	    }
	    
	  }
	  break;
	  
	case PLOT_HDSPA:
	  {
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	      cairo_set_line_width(cr,3.0*scale);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,1.0*scale);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(phpa[i]-phpa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		  
		}
		else{
		  if(phpa[i]<0){
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		  
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]-360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	    
	    if(hg->plot_all==PLOT_ALL_SINGLE){
	      cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
	      cairo_set_line_width(cr,1.0*scale);
	      
	      for(i=1;i<=iend-1;i++){
		if((pel[i]>0)&&(pel[i+1]>0)){
		  if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    if(ppa[i]<0){
		      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		      y=dy2+ly*(360-(ppa[i]+180))/360;
		      cairo_move_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		      y=dy2+ly*(360-(ppa[i+1]-360+180))/360;
		      cairo_line_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		      y=dy2+ly*(360-(ppa[i]+360+180))/360;
		      cairo_move_to(cr,x,y);
		      
		      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		      y=dy2+ly*(360-(ppa[i+1]+180))/360;
		      cairo_line_to(cr,x,y);
		      
		      cairo_stroke(cr);
		    }
		  }
		}
	      }
	    }
	  }
	  break;
	  
	}
      }
    }
  }
  
  else{  // PLOT_PLAN
    for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
      if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){
	gdouble hst_sod;
	
	//zonedate.hours=(gint)(hg->plan[i_plan].sod/60/60);
	//zonedate.minutes=(hg->plan[i_plan].sod-(gint)zonedate.hours*60*60)/60;
	//zonedate.seconds=0;

	//ln_zonedate_to_date(&zonedate, &date);
	//JD = ln_get_julian_local_date(&zonedate);

	oequ.ra=ra_to_deg(hg->obj[hg->plan[i_plan].obj_i].ra);
	oequ.dec=dec_to_deg(hg->obj[hg->plan[i_plan].obj_i].dec);
	
	ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[hg->plan[i_plan].obj_i].equinox),
			  JD, &oequ_prec);
	ln_get_hrz_from_equ (&oequ_prec, &observer, JD, &ohrz);


	//utstart=(double)date.hours+(double)date.minutes/60.;
	//utend=utstart+(double)hg->plan[i_plan].time/60./60.;

	a0=oequ_prec.ra*24./360.; //[hour]
	d0rad=oequ_prec.dec*M_PI/180.;

	i=1;
	d_ut=(gdouble)(hg->plot_jd1-hg->plot_jd0)*24.0/190.0;

	if(am_flag){
	  hst_sod=(gdouble)hg->plan[i_plan].sod/60./60.-24.;
	}
	else{
	  hst_sod=(gdouble)hg->plan[i_plan].sod/60./60.;
	}
	
	JD_hst = hg->plot_jd0;

	while(JD_hst<hg->plot_jd1+d_ut/24.){
	  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	  flst=ln_get_mean_sidereal_time(JD_hst)+LONGITUDE_SUBARU/360.*24.;

	  if(ohrz.az>180) ohrz.az-=360;
	  paz[i]=ohrz.az+180;
	  pel[i]=ohrz.alt;

	  if(hg->plot_mode==PLOT_MOONSEP){
	    sep[i]=deg_sep(paz[i]-180,pel[i],paz_moon[i],pel_moon[i]);
	  }
	  
	  ha=flst-a0;             //hour angle [hour]
	  ha=set_ul(-12., ha, 12.);
	  
	  sinha=sin(M_PI*ha/12.0);
	  cosha=cos(M_PI*ha/12.0);
	  ume=-sinha*cos(d0rad);
	  den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	  
	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=pel[i]*M_PI/180.+M_PI*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(paz[i]*M_PI/180.)*cos(el0d));
	  d1=d1rad*180./M_PI;
	  ume1=-sin(paz[i]*M_PI/180.)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(paz[i]*M_PI/180.)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./M_PI;
	  
	  ha1=set_ul(-12., ha1, 12.);
	  a1=flst-ha1;
	  a1=set_ul(0., a1, 24.);
	  
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  //write(9,*)'#### Position Angle'
	  delta_a=(a1-a0)*M_PI/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/M_PI;
	  //write(9,'(4h PA=,f8.3)')padeg
	
	
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=M_PI*(90.0-pel[i])/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	
	  pjd[i]=JD_hst;
	  ppa[i]=padeg;
	  pad[i]=adsec;

	  // HDS PA w/o ImR
	  phpa[i]=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);

	  ut=ut+d_ut;
	  JD_hst+=d_ut/24.;

	  i=i+1;
	}
	
	iend=i-1;
	
	cairo_reset_clip(cr);

	if((hg->plot_mode==PLOT_EL)&&(!hg->plan[i_plan].backup)){
	  cairo_save (cr);

	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_select_font_face (cr, hg->fontfamily_all, 
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_select_font_face (cr, hg->fontfamily_all, 
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);
	    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	  }
	  cairo_text_extents (cr, hg->obj[hg->plan[i_plan].obj_i].name,
			      &extents);
	  
	  cairo_translate(cr, 
			  dx+lx*(hst_sod+(double)hg->plan[i_plan].time/60./60./2.-ihst0)/(gfloat)(ihst1-ihst0), dy2+ly*(90-3)/90);
	  cairo_rotate (cr,-M_PI/2);
	  cairo_move_to(cr, 0, +extents.height/2);
	  
	  cairo_show_text(cr, hg->obj[hg->plan[i_plan].obj_i].name);
	  cairo_restore (cr);
	}      

	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_rectangle(cr, 
			(gdouble)((gint)(dx+lx*(hst_sod-ihst0)/(ihst1-ihst0)+0.5)),
			dy2,
			(gdouble)((gint)(lx*((gdouble)hg->plan[i_plan].time/60./60.)/(ihst1-ihst0)+0.5)),
			ly);
	cairo_clip(cr);
	cairo_new_path(cr);
      
	switch(hg->plot_mode){
	case PLOT_EL:
	  {
	    
	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,2.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	  }
	}
	break;

      case PLOT_AZ:
	{
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (fabs(paz[i]-paz[i+1])<180) ) {
		x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(360-paz[i])/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(360-paz[i+1])/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(paz[i]<180){
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(paz[i+1]-360))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(paz[i]+360))/360;
		  cairo_move_to(cr,x,y);

		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);

		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;
      case PLOT_AD:
	{
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
	      x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(4-pad[i])/4.;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
	      y=dy2+ly*(4-pad[i+1])/4.;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);      
	    }
	  }
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
	    cairo_set_line_width(cr,4.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.4);
	    cairo_set_line_width(cr,2.0);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(360-(ppa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(360-(ppa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(ppa[i]<0){
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;

	case PLOT_MOONSEP:
	  {
	    // Moon Separation
	    for(i=1;i<=iend-2;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(180-sep[i])/180;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		y=dy2+ly*(180-sep[i+1])/180;
	      cairo_line_to(cr,x,y);
	      
	      if(pel_moon[i]<0){
		if(i_plan==hg->plot_i_plan){
		  cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
		  cairo_set_line_width(cr,4.0);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.2);
		  cairo_set_line_width(cr,2.0);
		}
	      }
	      else{
		if(i_plan==hg->plot_i_plan){
		  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
		  cairo_set_line_width(cr,4.0);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
		  cairo_set_line_width(cr,2.0);
		}
	      }

	      cairo_stroke(cr);
	      }
	    }
	    
	  }
	  break;

	case PLOT_HDSPA:
	  {
	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	      cairo_set_line_width(cr,2.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(phpa[i]-phpa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		  
		}
		else{
		  if(phpa[i]<0){
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		  else{
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i]-360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(phpa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }

	    if(i_plan==hg->plot_i_plan){
	      cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
	      cairo_set_line_width(cr,4.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
	      cairo_set_line_width(cr,2.0);
	    }
	      
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (fabs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		  y=dy2+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(pjd[i+1]-hg->plot_jd0)/(hg->plot_jd1-hg->plot_jd0);
		    y=dy2+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	  break;
	}
      }
    }
  }

  switch(hg->plot_output){
  case PLOT_OUTPUT_PDF:
    cairo_destroy(cr);
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
    break;
  
  default:
    cairo_destroy(cr);
#ifdef USE_GTK3
    if(pixbuf_plot) g_object_unref(G_OBJECT(pixbuf_plot));
    pixbuf_plot=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    {
      GtkStyle *style=gtk_widget_get_style(widget);
      
      gdk_draw_drawable(gtk_widget_get_window(widget),
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_plot,
			0,0,0,0,
			width,
			height);
    }
#endif
    break;
  }

  return (FALSE);
}


void cc_get_plot_center (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_center=n;

    get_plot_time(hg);

    refresh_plot(widget, (gpointer)hg);
  }
}

void get_plot_time_current(typHOE *hg, gdouble delta_hst){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  gdouble JD;
  struct ln_zonedate zonedate;

  if(hg->skymon_mode==SKYMON_CUR){
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }
  else{
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
    
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0.0;
  }

  JD = get_julian_from_local_date(iyear,month,iday,hour,min,sec,
				  (long)hg->obs_timezone*60);

  hg->plot_jd0 = JD - delta_hst*0.25/24;
  hg->plot_jd1 = JD + delta_hst*0.75/24;
}

void get_plot_time_midnight(typHOE *hg, gdouble delta_hst){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  gdouble JD;
  struct ln_zonedate zonedate;

  if(hg->skymon_mode==SKYMON_CUR){
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }
  else{
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
    
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0.0;
  }

  if(hour<10){  // Show tonight
    JD = get_julian_from_local_date(iyear,month,iday,0,0,0.0,
				    (long)hg->obs_timezone*60);
  }
  else{         // Show the next night
    JD = get_julian_from_local_date(iyear,month,iday,0,0,0.0,
				    (long)hg->obs_timezone*60)+1.0;
  }

  if(hg->plot_zoom!=0){
    if(hg->plot_zoom>0){
      JD+=14.0/24.0*hg->plot_zoomr;
    }
  }

  hg->plot_jd0 = JD - delta_hst*0.5/24;
  hg->plot_jd1 = JD + delta_hst*0.5/24;
}


void get_plot_time_meridian(typHOE *hg, gdouble delta_hst){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  gdouble JD;
  struct ln_zonedate zonedate;

  JD = get_meridian_JD(hg);

  my_get_local_date(JD, &zonedate,hg->obs_timezone);

  hg->plot_jd0 = JD - delta_hst*0.5/24;
  hg->plot_jd1 = JD + delta_hst*0.5/24;
}


void get_plot_time(typHOE *hg){
  switch(hg->plot_center){
  case PLOT_CENTER_MIDNIGHT:
    get_plot_time_midnight(hg, 14.0*(gdouble)(10-hg->plot_zoom)/10.);
    break;

  case PLOT_CENTER_CURRENT:
    get_plot_time_current(hg, 10.0*(gdouble)(10-hg->plot_zoom)/10.);
    break;

  case PLOT_CENTER_MERIDIAN:
    get_plot_time_meridian(hg, 12.0*(gdouble)(10-hg->plot_zoom)/10.);
    break;
  }
}

gdouble get_alt_adjusted_rst(gdouble JD,
			     struct ln_equ_posn equ,
			     typHOE *hg,
			     gboolean rise_flag){
  gdouble JD_adj;
  gdouble d_min;
  
  d_min=0.140*sqrt(hg->obs_altitude
		   /cos((hg->obs_latitude+equ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-equ.dec)*M_PI/180.));
  
  if(rise_flag){
    JD_adj=JD-d_min/60./24.;
  }
  else{
    JD_adj=JD+d_min/60./24.;
  }
  
  return(JD_adj);
}

