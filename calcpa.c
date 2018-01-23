#include"main.h"
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>



#define BUFFER_SIZE 1024


double adrad(double zrad, double wlnm,double h,double t,double p,double f);
double new_tu(int iyear, int month, int iday);

void calc_moon_plan();

void close_plot();
void cc_get_plot_mode();
void cc_get_plot_all();
gboolean draw_plot_cairo();
static void refresh_plot();
static void do_plot_moon();
void add_day();



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





void calcpa2_main(typHOE* hg){
  
  double  pi=3.141592;
  double  alambda=LONGITUDE_SUBARU;  //longitude[deg]
  double  alamh=alambda/360.*24.;    //[hour]
  double  phi=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi=sin(pi*phi/180.0);
  double  cosphi=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  char *c;
  char object_name[64];
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
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
  int i_list;
  time_t tt;
  struct tm *tmpt;
  


  tt = time(NULL);
  tmpt = localtime(&tt);

  iyear=tmpt->tm_year+1900;
  month=tmpt->tm_mon+1;
  iday=tmpt->tm_mday;

  ut=(double)tmpt->tm_hour+(double)(TIMEZONE_SUBARU-hg->timezone)
    +(double)tmpt->tm_min/60.
    +(double)tmpt->tm_sec/3600. -14.0;
  
  if(tmpt->tm_hour>14)
    iday=iday+1;    // HST18:00 -> UT4:00 on next day




  //#### begin ########
  //## 1st step: Date => GMST(-UT1) => LST
  tu0=new_tu(iyear,month,iday);
  //gmst0=6.0+41./60.+50.54841/3600.;
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

  hg->lst_hour=(gint)flst;
  hg->lst_min=(gint)((flst-(double)hg->lst_hour)*60.);
  hg->lst_sec=(gint)((flst-(double)hg->lst_hour-(double)hg->lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){

    a0s=hg->obj[i_list].ra;
    ia0h=(int)(a0s/10000);
    a0s=a0s-(double)(ia0h)*10000;
    ia0m=(int)(a0s/100);
    a0s=a0s-(double)(ia0m)*100;
    
    d0s=hg->obj[i_list].dec;
    id0d=(int)(d0s/10000);
    d0s=d0s-(double)(id0d)*10000;
    id0m=(int)(d0s/100);
    d0s=d0s-(double)(id0m)*100;
    


    a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
    d0=id0d + id0m/60. + d0s/3600.;  //[deg]
    a0rad=pi*a0/12.;  //[rad]
    d0rad=pi*d0/180.; //[rad]
    
    ha=flst-a0;             //hour angle [hour]
    if(ha<-12.){
      do{
	ha=ha+24.;
      }while(ha<-12.);
    }
    else if(ha>12.){
      do{
	ha=ha-24.;
      }while(ha>12.);
    }
    
    sinha=sin(pi*ha/12.0);
    cosha=cos(pi*ha/12.0);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=asin(sinphi*sin(d0rad)+cosphi*cosha*cos(d0rad)); // elevation [rad]
    ume=-sinha*cos(d0rad);
    den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
    az0=atan2(ume,den);     // azimath [rad]      
    if(az0<0.) az0=az0+2.*pi;
    el0deg=el0*180./pi;
    az0deg=az0*180./pi;
    if(el0deg<0.0){
      hg->obj[i_list].c_az=-1;
      hg->obj[i_list].c_el=-1;
      hg->obj[i_list].c_ha=-1;
      hg->obj[i_list].c_pa=-1;
      hg->obj[i_list].c_ad=-1;
    }
    else{
      
      //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
      el0d=el0+pi*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./pi;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./pi;
      if(ha1<-12.){
	do{
	  ha1=ha1+24.;
	}while(ha1<-12.);
      }
      else if(ha1>=12.){
	do{
	  ha1=ha1-24.;
	}while(ha1>=12.);
      }
      a1=flst-ha1;
      if(a1<0.){
	do{
	  a1=a1+24.;
	}while(a1<0.);
      }
      else if(a1>=24.){
	do{
	  a1=a1-24.0;
	}while(a1>=24.);
      }
      
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*pi/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/pi;
      
    
      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=pi*(90.0-el0deg)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
      hst=ut+14.0;
      

      hg->obj[i_list].c_az=az0deg-180;
      hg->obj[i_list].c_el=el0deg;
      hg->obj[i_list].c_ha=ha1;
      hg->obj[i_list].c_pa=padeg;
      hg->obj[i_list].c_ad=adsec;
      
    }
  }

  calc_moon(hg);

  {
    a0s=(gdouble)hg->moon.c_ra.hours*10000.
    + (gdouble)hg->moon.c_ra.minutes*100. + hg->moon.c_ra.seconds; 
    //a0s=hg->moon.ra;
    ia0h=(int)(a0s/10000);
    a0s=a0s-(double)(ia0h)*10000;
    ia0m=(int)(a0s/100);
    a0s=a0s-(double)(ia0m)*100;
    
    d0s=
      hg->moon.c_dec.neg==1 ? 
      -1.* ((gdouble)hg->moon.c_dec.degrees*10000. 
	  + (gdouble)hg->moon.c_dec.minutes*100. + hg->moon.c_dec.seconds)
    : (gdouble)hg->moon.c_dec.degrees*10000.
    + (gdouble)hg->moon.c_dec.minutes*100. + hg->moon.c_dec.seconds;
    //d0s=hg->moon.dec;
    id0d=(int)(d0s/10000);
    d0s=d0s-(double)(id0d)*10000;
    id0m=(int)(d0s/100);
    d0s=d0s-(double)(id0m)*100;
    


    a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
    d0=id0d + id0m/60. + d0s/3600.;  //[deg]
    a0rad=pi*a0/12.;  //[rad]
    d0rad=pi*d0/180.; //[rad]
    
    ha=flst-a0;             //hour angle [hour]
    if(ha<-12.){
      do{
	ha=ha+24.;
      }while(ha<-12.);
    }
    else if(ha>12.){
      do{
	ha=ha-24.;
      }while(ha>12.);
    }
    
    sinha=sin(pi*ha/12.0);
    cosha=cos(pi*ha/12.0);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=asin(sinphi*sin(d0rad)+cosphi*cosha*cos(d0rad)); // elevation [rad]
    ume=-sinha*cos(d0rad);
    den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
    az0=atan2(ume,den);     // azimath [rad]      
    if(az0<0.) az0=az0+2.*pi;
    el0deg=el0*180./pi;
    az0deg=az0*180./pi;
    if(el0deg<0.0){
      hg->moon.c_az=-1;
      hg->moon.c_el=-1;
      hg->moon.c_ha=-1;
    }
    else{
      
      //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
      el0d=el0+pi*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./pi;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./pi;
      if(ha1<-12.){
	do{
	  ha1=ha1+24.;
	}while(ha1<-12.);
      }
      else if(ha1>=12.){
	do{
	  ha1=ha1-24.;
	}while(ha1>=12.);
      }
      a1=flst-ha1;
      if(a1<0.){
	do{
	  a1=a1+24.;
	}while(a1<0.);
      }
      else if(a1>=24.){
	do{
	  a1=a1-24.0;
	}while(a1>=24.);
      }
      
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*pi/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/pi;
      
    
      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=pi*(90.0-el0deg)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
      hst=ut+14.0;
      

      hg->moon.c_az=az0deg-180;
      hg->moon.c_el=el0deg;
      hg->moon.c_ha=ha1;
      
    }
  }

}


void calcpa2_skymon(typHOE* hg){
  
  double  pi=3.141592;
  double  alambda=LONGITUDE_SUBARU;  //longitude[deg]
  double  alamh=alambda/360.*24.;    //[hour]
  double  phi=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi=sin(pi*phi/180.0);
  double  cosphi=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  char *c;
  char object_name[64];
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
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
  int i_list;
  


  iyear=hg->skymon_year;
  month=hg->skymon_month;
  iday=hg->skymon_day;

  ut=(double)hg->skymon_hour+(double)(TIMEZONE_SUBARU-hg->timezone)
    +(double)hg->skymon_min/60.
    +0  //sec
    -14.0;
  
  if(hg->skymon_hour>14)
    iday=iday+1;    // HST18:00 -> UT4:00 on next day




  //#### begin ########
  //## 1st step: Date => GMST(-UT1) => LST
  tu0=new_tu(iyear,month,iday);
  //gmst0=6.0+41./60.+50.54841/3600.;
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

  hg->skymon_lst_hour=(gint)flst;
  hg->skymon_lst_min=(gint)((flst-(double)hg->skymon_lst_hour)*60.);
  hg->skymon_lst_sec=(gint)((flst-(double)hg->skymon_lst_hour-(double)hg->skymon_lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){

    a0s=hg->obj[i_list].ra;
    ia0h=(int)(a0s/10000);
    a0s=a0s-(double)(ia0h)*10000;
    ia0m=(int)(a0s/100);
    a0s=a0s-(double)(ia0m)*100;
    
    d0s=hg->obj[i_list].dec;
    id0d=(int)(d0s/10000);
    d0s=d0s-(double)(id0d)*10000;
    id0m=(int)(d0s/100);
    d0s=d0s-(double)(id0m)*100;
    


    a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
    d0=id0d + id0m/60. + d0s/3600.;  //[deg]
    a0rad=pi*a0/12.;  //[rad]
    d0rad=pi*d0/180.; //[rad]
    
    ha=flst-a0;             //hour angle [hour]
    if(ha<-12.){
      do{
	ha=ha+24.;
      }while(ha<-12.);
    }
    else if(ha>12.){
      do{
	ha=ha-24.;
      }while(ha>12.);
    }
    
    sinha=sin(pi*ha/12.0);
    cosha=cos(pi*ha/12.0);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=asin(sinphi*sin(d0rad)+cosphi*cosha*cos(d0rad)); // elevation [rad]
    ume=-sinha*cos(d0rad);
    den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
    az0=atan2(ume,den);     // azimath [rad]      
    if(az0<0.) az0=az0+2.*pi;
    el0deg=el0*180./pi;
    az0deg=az0*180./pi;
    if(el0deg<0.0){
      hg->obj[i_list].s_az=-1;
      hg->obj[i_list].s_el=-1;
      hg->obj[i_list].s_ha=-1;
      hg->obj[i_list].s_pa=-1;
      hg->obj[i_list].s_ad=-1;
    }
    else{
      
      //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
      el0d=el0+pi*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./pi;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./pi;
      if(ha1<-12.){
	do{
	  ha1=ha1+24.;
	}while(ha1<-12.);
      }
      else if(ha1>=12.){
	do{
	  ha1=ha1-24.;
	}while(ha1>=12.);
      }
      a1=flst-ha1;
      if(a1<0.){
	do{
	  a1=a1+24.;
	}while(a1<0.);
      }
      else if(a1>=24.){
	do{
	  a1=a1-24.0;
	}while(a1>=24.);
      }
      
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*pi/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/pi;
      
    
      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=pi*(90.0-el0deg)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
      hst=ut+14.0;
      

      hg->obj[i_list].s_az=az0deg-180;
      hg->obj[i_list].s_el=el0deg;
      hg->obj[i_list].s_ha=ha1;
      hg->obj[i_list].s_pa=padeg;
      hg->obj[i_list].s_ad=adsec;
      
    }
  }

  calc_moon_skymon(hg);

}


void calcpa2_plan(typHOE* hg){
  
  double  pi=3.141592;
  double  alambda=LONGITUDE_SUBARU;  //longitude[deg]
  double  alamh=alambda/360.*24.;    //[hour]
  double  phi=LATITUDE_SUBARU;       //latitude [deg]      
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
      
      ut=hg->plan[i_plan].sod/3600.-14.0;
      flag_start=TRUE;
      do{
	if(ut<0){
	  iday=iday0+1;    // HST18:00 -> UT4:00 on next day
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
	if(ha<-12.){
	  do{
	    ha=ha+24.;
	  }while(ha<-12.);
	}
	else if(ha>12.){
	  do{
	    ha=ha-24.;
	  }while(ha>12.);
	}
    
	sinha=sin(pi*ha/12.0);
	cosha=cos(pi*ha/12.0);
    
	//### 2nd step: (RA,Dec) -> (AZ,EL)
	el0=asin(sinphi*sin(d0rad)+cosphi*cosha*cos(d0rad)); // elevation [rad]
	ume=-sinha*cos(d0rad);
	den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	az0=atan2(ume,den);     // azimath [rad]      
	if(az0<0.) az0=az0+2.*pi;
	el0deg=el0*180./pi;
	az0deg=az0*180./pi;
	/*
	if(el0deg<0.0){
	  hg->pp[i_pp].az=-1;
	  hg->pp[i_pp].el=-1;
	}
	else{*/
      	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=el0+pi*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
	  d1=d1rad*180./pi;
	  ume1=-sin(az0)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./pi;
	  if(ha1<-12.){
	    do{
	      ha1=ha1+24.;
	    }while(ha1<-12.);
	  }
	  else if(ha1>=12.){
	    do{
	      ha1=ha1-24.;
	    }while(ha1>=12.);
	  }
	  a1=flst-ha1;
	  if(a1<0.){
	    do{
	      a1=a1+24.;
	    }while(a1<0.);
	  }
	  else if(a1>=24.){
	    do{
	      a1=a1-24.0;
	    }while(a1>=24.);
	  }
      
    
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  delta_a=(a1-a0)*pi/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/pi;
      
    
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=pi*(90.0-el0deg)/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
	  hst=ut+14.0;
      

	  hg->pp[i_pp].az=az0deg-180;
	  hg->pp[i_pp].el=el0deg;
	  hg->pp[i_pp].ut=ut;
	  
	  hg->pp[i_pp].i_plan=i_plan;
	  hg->pp[i_pp].start=flag_start;
	  /*}*/

	ut+=0.1;
	flag_start=FALSE;
	i_pp++;
      }while((ut<(hg->plan[i_plan].sod+hg->plan[i_plan].time)/3600.-14.)
	     &&(i_pp<MAX_PP));

    }
  }
  hg->i_pp_max=i_pp;


  calc_sun_plan(hg);
  calc_moon_plan(hg);

}


void calc_moon(typHOE *hg){
  /* for Moon */
  double JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_rst_time rst;
  struct ln_zonedate set,rise;
  gdouble d_t, d_ss;
  gint d_mm;

  /* observers location (Subaru), used to calc rst */
  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;
        
  /* get the julian day from the local system time */
  JD = ln_get_julian_from_sys();
  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.c_ra=hms;
  hg->moon.c_dec=dms;
  /*
  hg->moon.ra=(gdouble)hms.hours*10000.
    + (gdouble)hms.minutes*100. + hms.seconds;
  
  hg->moon.dec=
    dms.neg==1 ? 
    -1.* ((gdouble)dms.degrees*10000. 
	  + (gdouble)dms.minutes*100. + dms.seconds)
    : (gdouble)dms.degrees*10000.
    + (gdouble)dms.minutes*100. + dms.seconds;
  */
  hg->moon.c_disk=ln_get_lunar_disk(JD);
  hg->moon.c_phase=ln_get_lunar_phase(JD);
  hg->moon.c_limb=ln_get_lunar_bright_limb(JD);

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.c_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->moon.c_circum=FALSE;

    hg->moon.c_rise.hours=rise.hours;
    hg->moon.c_rise.minutes=rise.minutes;
    hg->moon.c_rise.seconds=rise.seconds;
    hg->moon.c_set.hours=set.hours;
    hg->moon.c_set.minutes=set.minutes;
    hg->moon.c_set.seconds=set.seconds;

    ln_get_lunar_equ_coords (rst.set, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->moon.c_set.seconds+=d_ss;
    if(hg->moon.c_set.seconds>=60){
      hg->moon.c_set.minutes+=1;
      hg->moon.c_set.seconds-=60;
    }
    hg->moon.c_set.minutes+=d_mm;
    if(hg->moon.c_set.minutes>=60){
      hg->moon.c_set.hours+=1;
      hg->moon.c_set.minutes-=60;
      if(hg->moon.c_set.hours>=24){
	hg->moon.c_set.hours-=24;
      }
    }
    
    ln_get_lunar_equ_coords (rst.rise, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->moon.c_rise.seconds-=d_ss;
    if(hg->moon.c_rise.seconds<0){
      hg->moon.c_rise.minutes-=1;
      hg->moon.c_rise.seconds+=60;
    }
    hg->moon.c_rise.minutes-=d_mm;
    if(hg->moon.c_rise.minutes<0){
      hg->moon.c_rise.hours-=1;
      hg->moon.c_rise.minutes+=60;
      if(hg->moon.c_rise.hours<0){
	hg->moon.c_rise.hours+=24;
      }
    }
  }


  if (ln_get_solar_rst (JD, &observer, &rst) == 1){
    hg->sun.c_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->sun.c_circum=FALSE;

    hg->sun.c_set.hours=set.hours;
    hg->sun.c_set.minutes=set.minutes;
    hg->sun.c_set.seconds=set.seconds;

    hg->sun.c_rise.hours=rise.hours;
    hg->sun.c_rise.minutes=rise.minutes;
    hg->sun.c_rise.seconds=rise.seconds;

    ln_get_solar_equ_coords (rst.set, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->sun.c_set.seconds+=d_ss;
    if(hg->sun.c_set.seconds>=60){
      hg->sun.c_set.minutes+=1;
      hg->sun.c_set.seconds-=60;
    }
    hg->sun.c_set.minutes+=d_mm;
    if(hg->sun.c_set.minutes>=60){
      hg->sun.c_set.hours+=1;
      hg->sun.c_set.minutes-=60;
      if(hg->sun.c_set.hours>=24){
	hg->sun.c_set.hours-=24;
      }
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->sun.c_rise.seconds-=d_ss;
    if(hg->sun.c_rise.seconds<0){
      hg->sun.c_rise.minutes-=1;
      hg->sun.c_rise.seconds+=60;
    }
    hg->sun.c_rise.minutes-=d_mm;
    if(hg->sun.c_rise.minutes<0){
      hg->sun.c_rise.hours-=1;
      hg->sun.c_rise.minutes+=60;
      if(hg->sun.c_rise.hours<0){
	hg->sun.c_rise.hours+=24;
      }
    }

  }

}


void calc_moon_skymon(typHOE *hg){
  /* for Moon */
  double JD;
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


  /* observers location (Subaru), used to calc rst */
  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;

  local_date.years=hg->skymon_year;
  local_date.months=hg->skymon_month;
  local_date.days=hg->skymon_day;

  local_date.hours=hg->skymon_hour;
  local_date.minutes=hg->skymon_min;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(TIMEZONE_SUBARU*3600);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.s_ra=hms;
  hg->moon.s_dec=dms;

  ln_get_hrz_from_equ (&equ, &observer, JD, &hrz);
  if(hrz.az>180) hrz.az-=360;
  hg->moon.s_az=hrz.az;
  hg->moon.s_el=hrz.alt;
    
  hg->moon.s_disk=ln_get_lunar_disk(JD);
  hg->moon.s_phase=ln_get_lunar_phase(JD);
  hg->moon.s_limb=ln_get_lunar_bright_limb(JD);

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.s_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->moon.s_circum=FALSE;

    hg->moon.s_rise.hours=rise.hours;
    hg->moon.s_rise.minutes=rise.minutes;
    hg->moon.s_rise.seconds=set.seconds;
    hg->moon.s_set.hours=set.hours;
    hg->moon.s_set.minutes=set.minutes;
    hg->moon.s_set.seconds=set.seconds;

    ln_get_lunar_equ_coords (rst.set, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->moon.s_set.seconds+=d_ss;
    if(hg->moon.s_set.seconds>=60){
      hg->moon.s_set.minutes+=1;
      hg->moon.s_set.seconds-=60;
    }
    hg->moon.s_set.minutes+=d_mm;
    if(hg->moon.s_set.minutes>=60){
      hg->moon.s_set.hours+=1;
      hg->moon.s_set.minutes-=60;
      if(hg->moon.s_set.hours>=24){
	hg->moon.s_set.hours-=24;
      }
    }
    
    ln_get_lunar_equ_coords (rst.rise, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->moon.s_rise.seconds-=d_ss;
    if(hg->moon.s_rise.seconds<0){
      hg->moon.s_rise.minutes-=1;
      hg->moon.s_rise.seconds+=60;
    }
    hg->moon.s_rise.minutes-=d_mm;
    if(hg->moon.s_rise.minutes<0){
      hg->moon.s_rise.hours-=1;
      hg->moon.s_rise.minutes+=60;
      if(hg->moon.s_rise.hours<0){
	hg->moon.s_rise.hours+=24;
      }
    }
  }


  if (ln_get_solar_rst (JD, &observer, &rst) == 1){
    hg->sun.s_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->sun.s_circum=FALSE;

    hg->sun.s_rise.hours=rise.hours;
    hg->sun.s_rise.minutes=rise.minutes;
    hg->sun.s_rise.seconds=set.seconds;
    hg->sun.s_set.hours=set.hours;
    hg->sun.s_set.minutes=set.minutes;
    hg->sun.s_set.seconds=set.seconds;

    ln_get_solar_equ_coords (rst.set, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->sun.s_set.seconds+=d_ss;
    if(hg->sun.s_set.seconds>=60){
      hg->sun.s_set.minutes+=1;
      hg->sun.s_set.seconds-=60;
    }
    hg->sun.s_set.minutes+=d_mm;
    if(hg->sun.s_set.minutes>=60){
      hg->sun.s_set.hours+=1;
      hg->sun.s_set.minutes-=60;
      if(hg->sun.s_set.hours>=24){
	hg->sun.s_set.hours-=24;
      }
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->sun.s_rise.seconds-=d_ss;
    if(hg->sun.s_rise.seconds<0){
      hg->sun.s_rise.minutes-=1;
      hg->sun.s_rise.seconds+=60;
    }
    hg->sun.s_rise.minutes-=d_mm;
    if(hg->sun.s_rise.minutes<0){
      hg->sun.s_rise.hours-=1;
      hg->sun.s_rise.minutes+=60;
      if(hg->sun.s_rise.hours<0){
	hg->sun.s_rise.hours+=24;
      }
    }
  }
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
  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day;

  local_date.hours=hg->atw.s_set.hours;
  local_date.minutes=hg->atw.s_set.minutes;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(TIMEZONE_SUBARU*3600);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day+1;

  local_date.hours=hg->atw.s_rise.hours;
  local_date.minutes=hg->atw.s_rise.minutes;
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


void calc_sun_plan(typHOE *hg){
  /* for Moon */
  double JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_zonedate local_date;
  struct ln_rst_time rst;
  struct ln_date date;
  struct ln_zonedate set,rise;
  gdouble d_t,d_ss;
  gint d_mm;


  /* observers location (Subaru), used to calc rst */
  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;

  local_date.years=hg->fr_year;
  local_date.months=hg->fr_month;
  local_date.days=hg->fr_day;

  local_date.hours=17;
  local_date.minutes=0;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(TIMEZONE_SUBARU*3600);
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

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.s_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->moon.s_circum=FALSE;

    hg->moon.s_rise.hours=rise.hours;
    hg->moon.s_rise.minutes=rise.minutes;
    hg->moon.s_rise.seconds=set.seconds;
    hg->moon.s_set.hours=set.hours;
    hg->moon.s_set.minutes=set.minutes;
    hg->moon.s_set.seconds=set.seconds;

    ln_get_lunar_equ_coords (rst.set, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->moon.s_set.seconds+=d_ss;
    if(hg->moon.s_set.seconds>=60){
      hg->moon.s_set.minutes+=1;
      hg->moon.s_set.seconds-=60;
    }
    hg->moon.s_set.minutes+=d_mm;
    if(hg->moon.s_set.minutes>=60){
      hg->moon.s_set.hours+=1;
      hg->moon.s_set.minutes-=60;
      if(hg->moon.s_set.hours>=24){
	hg->moon.s_set.hours-=24;
      }
    }
    
    ln_get_lunar_equ_coords (rst.rise, &equ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+equ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->moon.s_rise.seconds-=d_ss;
    if(hg->moon.s_rise.seconds<0){
      hg->moon.s_rise.minutes-=1;
      hg->moon.s_rise.seconds+=60;
    }
    hg->moon.s_rise.minutes-=d_mm;
    if(hg->moon.s_rise.minutes<0){
      hg->moon.s_rise.hours-=1;
      hg->moon.s_rise.minutes+=60;
      if(hg->moon.s_rise.hours<0){
	hg->moon.s_rise.hours+=24;
      }
    }
  }


  if (ln_get_solar_rst (JD, &observer, &rst) == 1){
    hg->sun.s_circum=TRUE;
  }
  else {
    ln_get_local_date (rst.rise, &rise);
    ln_get_local_date (rst.set, &set);
    hg->sun.s_circum=FALSE;

    hg->sun.s_rise.hours=rise.hours;
    hg->sun.s_rise.minutes=rise.minutes;
    hg->sun.s_rise.seconds=set.seconds;
    hg->sun.s_set.hours=set.hours;
    hg->sun.s_set.minutes=set.minutes;
    hg->sun.s_set.seconds=set.seconds;

    ln_get_solar_equ_coords (rst.set, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->sun.s_set.seconds+=d_ss;
    if(hg->sun.s_set.seconds>=60){
      hg->sun.s_set.minutes+=1;
      hg->sun.s_set.seconds-=60;
    }
    hg->sun.s_set.minutes+=d_mm;
    if(hg->sun.s_set.minutes>=60){
      hg->sun.s_set.hours+=1;
      hg->sun.s_set.minutes-=60;
      if(hg->sun.s_set.hours>=24){
	hg->sun.s_set.hours-=24;
      }
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    d_t=0.140*sqrt(ALTITUDE_SUBARU/cos((LATITUDE_SUBARU+sequ.dec)*M_PI/180.)
		   /cos((LATITUDE_SUBARU-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->sun.s_rise.seconds-=d_ss;
    if(hg->sun.s_rise.seconds<0){
      hg->sun.s_rise.minutes-=1;
      hg->sun.s_rise.seconds+=60;
    }
    hg->sun.s_rise.minutes-=d_mm;
    if(hg->sun.s_rise.minutes<0){
      hg->sun.s_rise.hours-=1;
      hg->sun.s_rise.minutes+=60;
      if(hg->sun.s_rise.hours<0){
	hg->sun.s_rise.hours+=24;
      }
    }
  }


  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) == 1){
    hg->atw.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)TIMEZONE_SUBARU*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)TIMEZONE_SUBARU*3600);
    hg->atw.s_circum=FALSE;
    
    hg->atw.s_rise.hours=rise.hours;
    hg->atw.s_rise.minutes=rise.minutes;
    hg->atw.s_rise.seconds=set.seconds;
    hg->atw.s_set.hours=set.hours;
    hg->atw.s_set.minutes=set.minutes;
    hg->atw.s_set.seconds=set.seconds;
  }
}


void close_plot(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;


  gtk_widget_destroy(GTK_WIDGET(hg->plot_main));
  flagPlot=FALSE;
}


gboolean draw_plot_cairo(GtkWidget *widget, 
			 GdkEventExpose *event, 
			 gpointer userdata){
  cairo_t *cr;
  cairo_surface_t *surface;
  typHOE *hg;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list, i_plan;
  GdkPixmap *pixmap_plot;
  gint from_set, to_rise;
  double dx,dy,lx,ly;

  double put[250],plst[250],paz[250],pel[250],ppa[250],pad[250], phst[250];
  
  double  pi=3.141592;
  double  alambda; //=LONGITUDE_SUBARU;  //longitude[deg]
  double  alamh;   //=alambda/360.*24.;    //[hour]
  double  phi;     //=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char tmp[BUFFER_SIZE];
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
  double utstart,utend;
  double a0,d0,a0rad,d0rad;
  double ut,d_ut;
  double flst, ha, sinha, cosha;
  double el0, ume, den;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst, JD_hst;
  double a1;
  int i,iend;
  int width, height;
  gint x_hst;
  
  struct ln_zonedate zonedate;
  struct ln_date date;
  struct ln_equ_posn oequ;
  struct ln_rst_time orst;
  struct ln_hrz_posn ohrz;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD;

  if(!flagPlot) return (FALSE);

  hg=(typHOE *)userdata;
  calc_sun_plan(hg);

  if(hg->plot_all==PLOT_ALL_ALL){
    ihst0=15;
    ihst1=34;
  }
  else{
    ihst0=17;
    ihst1=31;
  }

  observer.lat = LATITUDE_SUBARU;
  observer.lng = LONGITUDE_SUBARU;


  if(hg->plot_output==PLOT_OUTPUT_PDF){
    width= PLOT_WIDTH;
    height= PLOT_HEIGHT;
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

  }
  else{
    width= widget->allocation.width;
    height= widget->allocation.height;
    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->plot_main), &width, &height);
    }
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    pixmap_plot = gdk_pixmap_new(widget->window,
				   width,
				   height,
				   -1);
  
    //cr = gdk_cairo_create(widget->window);
    cr = gdk_cairo_create(pixmap_plot);
  }


  /*
  if (supports_alpha)
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0); // transparen
  else
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); // opaque white
  */

  //cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
  if(hg->plot_output==PLOT_OUTPUT_PDF){
    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
  }
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  //if(hg->plot_target==PLOT_OBJTREE){
  if(hg->plot_all!=PLOT_ALL_PLAN){
    // Object Name etc.
    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    
    sprintf(tmp,"\"%s\"  RA=%09.2f Dec=%+010.2f Epoch=%7.2f",
	  hg->obj[hg->plot_i].name,
	   hg->obj[hg->plot_i].ra,
	    hg->obj[hg->plot_i].dec,
	    hg->obj[hg->plot_i].epoch);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, 12.0);
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,+extents.height);
    cairo_show_text(cr, tmp);
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
      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0);
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

      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0);
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
      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0);
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

      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0);
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
      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0);
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

      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0);
      sprintf(tmp,"AD[\"] (%d-%dA)", hg->wave1,hg->wave0);
      cairo_text_extents (cr, tmp, &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, tmp);
      cairo_restore (cr);


      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0);
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

      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 14.0);
      cairo_text_extents (cr, "Zenith PA[deg]", &extents);

      x = dx+lx-extents.width/2+x0-(dx+lx+x0);
      y = height/2+extents.height-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Zenith PA[deg]");
      cairo_restore (cr);
      
    }// AD
    break;
  }

  // HST
  {
    cairo_set_font_size (cr, 10.0);
    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    //if(hg->skymon_mode==SKYMON_SET){
    {
      sprintf(tmp,"%d:%02d",
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


      sprintf(tmp,"%d:%02d",
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

      // Astronomical Twilight = 18deg
      sprintf(tmp,"%d:%02d",
	      hg->atw.s_set.hours,hg->atw.s_set.minutes);

      if(hg->atw.s_set.hours>=ihst0){
	x=lx*((gfloat)hg->atw.s_set.hours-ihst0
	      +(gfloat)hg->atw.s_set.minutes/60.)/(gfloat)(ihst1-ihst0);

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


      sprintf(tmp,"%d:%02d",
	      hg->atw.s_rise.hours,hg->atw.s_rise.minutes);

      if(hg->atw.s_rise.hours<(ihst1-24)){
	x=lx*((ihst1-24)-(gfloat)hg->atw.s_rise.hours
	      -(gfloat)hg->atw.s_rise.minutes/60.)/(gfloat)(ihst1-ihst0);

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


    // vertical lines
    x_hst=0;

    cairo_set_font_size (cr, 10.0);
    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
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
	sprintf(tmp,"%d",(ihst0+x_hst)<24 ? ihst0+x_hst : ihst0+x_hst-24);
	cairo_text_extents (cr, tmp, &extents);
	x = dx+lx*(gfloat)x_hst/(gfloat)(ihst1-ihst0)-extents.width/2;
	y = dy+ly+extents.height+5;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
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
    /*
    if(hg->skymon_mode==SKYMON_SET){
      iyear=hg->skymon_year;
      month=hg->skymon_month;
      iday=hg->skymon_day;
      
      hour=hg->skymon_hour;
      min=hg->skymon_min;
      sec=0;
    }
    else{
      get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
    }

    if(hour<10){
      add_day(hg, &iyear, &month, &iday, -1);
    }
    */
  
    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 14.0);
    cairo_text_extents (cr, "HST", &extents);
    x = width/2-extents.width/2;
    y += extents.height+5;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, "HST");

    {
      gint iyear1,month1,iday1;
      
      cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 12.0);
      sprintf(tmp,"(%4d/%2d/%2d)",iyear,month,iday);
      cairo_text_extents (cr, tmp, &extents);
      x = dx;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
      
      iyear1=iyear;
      month1=month;
      iday1=iday;

      add_day(hg, &iyear1, &month1, &iday1, +1);
      sprintf(tmp,"(%4d/%2d/%2d)",iyear1,month1,iday1);
      cairo_text_extents (cr, tmp, &extents);
      x = dx+lx-extents.width;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
    }
      

  }// HST
  

  //////////////////////////////////////////
  

  //alambda=hg->obs_longitude;  //longitude[deg]
  alambda=LONGITUDE_SUBARU;  //longitude[deg]
  alamh=alambda/360.*24.;     //[hour]
  //phi=hg->obs_latitude;       //latitude [deg]      
  phi=LATITUDE_SUBARU;       //latitude [deg]      
  sinphi=sin(pi*phi/180.0);
  cosphi=cos(pi*phi/180.0);


  //if(hg->plot_target==PLOT_OBJTREE){
  if(hg->plot_all!=PLOT_ALL_PLAN){
    gboolean plot_flag=FALSE;

    zonedate.years=iyear;
    zonedate.months=month;
    zonedate.days=iday;
    zonedate.hours=ihst0;
    zonedate.minutes=0;
    zonedate.seconds=0;
    zonedate.gmtoff=(long)(TIMEZONE_SUBARU*3600);
    //zonedate.gmtoff=(long)hg->obs_timezone*3600;
    
    ln_zonedate_to_date(&zonedate, &date);
    
    utstart=(double)date.hours;
    utend=utstart+(double)(ihst1-ihst0);


    /* Lunar RA, DEC */
    if(hg->plot_moon){
      gdouble m_tr_el;

      JD = ln_get_julian_local_date(&zonedate);

      ln_get_lunar_rst (JD, &observer, &orst);
      ln_get_date (orst.transit, &odate);
      ln_date_to_zonedate(&odate,&transit,(long)TIMEZONE_SUBARU*3600);
      ln_get_lunar_equ_coords (orst.transit, &oequ);
      ln_get_hrz_from_equ (&oequ, &observer, orst.transit, &ohrz);
      m_tr_el=ohrz.alt;

      ut=utstart;
      
      i=1;
      d_ut=0.5;
      hst=(gdouble)ihst0;
      
      JD_hst = ln_get_julian_local_date(&zonedate);
      
      while(hst<=(gdouble)ihst1+d_ut*2){
	ln_get_lunar_equ_coords (JD_hst, &oequ);
	
	ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	if(ohrz.az>180) ohrz.az-=360;

	put[i]=ut;
	phst[i]=hst;

	paz[i]=ohrz.az;
	pel[i]=ohrz.alt;

	ut=ut+d_ut;
	JD_hst+=d_ut/24.;
	hst+=d_ut;
	
	i=i+1;

      }

      iend=i;
      
      cairo_rectangle(cr, dx,dy,lx,ly);
      cairo_clip(cr);
      cairo_new_path(cr);

      switch(hg->plot_mode){
      case PLOT_EL:
	{
	  gdouble x_tr,y_tr;
	  
	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 0.5);
	  cairo_set_line_width(cr,5.0);
	  
	  for(i=1;i<=iend-2;i++){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_stroke(cr);
	    
	  }
	  
	  x_tr=((transit.hours<ihst0)?((gdouble)transit.hours+24.):((gdouble)transit.hours))
	    +(gdouble)transit.minutes/60.;
	  
	  if((x_tr>ihst0)&&(x_tr<ihst1)){
	    cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(gdouble)(ihst1-ihst0),
			  dy+ly*(90.-m_tr_el)/90.);
	    
	    cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 1.0);
	    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size (cr, 11.0);
	    
	    cairo_text_extents (cr, "moon", &extents);
	    cairo_save(cr);
	    cairo_rotate (cr,-M_PI/2);
	    cairo_rel_move_to(cr,5.,+extents.height/2.);
	    cairo_reset_clip(cr);
	    cairo_show_text(cr,"moon");
	    cairo_restore(cr);
	  }
	}
	break;
      }
      cairo_reset_clip(cr);
    }


    
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

	//a0s=hg->obj[hg->plot_i].ra;
	a0s=hg->obj[i_list].ra;
	ia0h=(int)(a0s/10000);
	a0s=a0s-(double)(ia0h)*10000;
	ia0m=(int)(a0s/100);
	a0s=a0s-(double)(ia0m)*100;
	
	//d0s=hg->obj[hg->plot_i].dec;
	d0s=hg->obj[i_list].dec;
	id0d=(int)(d0s/10000);
	d0s=d0s-(double)(id0d)*10000;
	id0m=(int)(d0s/100);
	d0s=d0s-(double)(id0m)*100;
	
	
	
	a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
	d0=id0d + id0m/60. + d0s/3600.;  //[deg]
	a0rad=pi*a0/12.;  //[rad]
	d0rad=pi*d0/180.; //[rad]
	
	
	oequ.ra=a0*360/24;
	oequ.dec=d0;
	
	JD = ln_get_julian_local_date(&zonedate);
	ln_get_object_rst (JD, &observer, &oequ, &orst);
	ln_get_date (orst.transit, &odate);
	ln_date_to_zonedate(&odate,&transit,(long)TIMEZONE_SUBARU*3600);
	
	ut=utstart;
	
	i=1;
	d_ut=0.1;
	hst=(gdouble)ihst0;
	
	JD_hst = ln_get_julian_local_date(&zonedate);
	
	while(hst<=(gdouble)ihst1+d_ut){
	  ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	  flst=ln_get_mean_sidereal_time(JD_hst)+LONGITUDE_SUBARU/360.*24.;
	  
	  if(ohrz.az>180) ohrz.az-=360;
	  paz[i]=ohrz.az+180;
	  pel[i]=ohrz.alt;
	  
	  ha=flst-a0;             //hour angle [hour]
	  if(ha<-12.){
	    do{
	      ha=ha+24.;
	    }while(ha<-12.);
	  }
	  else if(ha>12.){
	    do{
	      ha=ha-24.;
	    }while(ha>12.);
	  }
	  
	  sinha=sin(pi*ha/12.0);
	  cosha=cos(pi*ha/12.0);
	  ume=-sinha*cos(d0rad);
	  den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	  
	  
	  //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	  el0d=pel[i]*pi/180.+pi*4./3600./180.;
	  d1rad=asin(sinphi*sin(el0d)+cosphi*cos(paz[i]*pi/180.)*cos(el0d));
	  d1=d1rad*180./pi;
	  ume1=-sin(paz[i]*pi/180.)*cos(el0d);
	  den1=cosphi*sin(el0d)-sinphi*cos(paz[i]*pi/180.)*cos(el0d);
	  ha1rad=atan2(ume1,den1);
	  ha1=ha1rad*12./pi;
	  if(ha1<-12.){
	    do{
	      ha1=ha1+24.;
	    }while(ha1<-12.);
	  }
	  else if(ha1>=12.){
	    do{
	      ha1=ha1-24.;
	    }while(ha1>=12.);
	  }
	  a1=flst-ha1;
	  if(a1<0.){
	    do{
	      a1=a1+24.;
	    }while(a1<0.);
	  }
	  else if(a1>=24.){
	    do{
	      a1=a1-24.0;
	    }while(a1>=24.);
	  }
	  
	  
	  //### 4th step: (RA1-RA,Dec1-Dec) => PA
	  //write(9,*)'#### Position Angle'
	  delta_a=(a1-a0)*pi/12.;   //[rad]
	  delta_d=d1rad-d0rad;      //[rad]
	  pa=atan2(delta_a,delta_d);      
	  padeg=180.*pa/pi;
	  //write(9,'(4h PA=,f8.3)')padeg
	  
	  
	  //### 5th step: Atmospheric Dispersion at 3500A
	  zrad=pi*(90.0-pel[i])/180.;
	  ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	  ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	  adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
	  
	  put[i]=ut;
	  phst[i]=hst;
	  
	  plst[i]=flst;
	  ppa[i]=padeg;
	  pad[i]=adsec;
	  
	  ut=ut+d_ut;
	  JD_hst+=d_ut/24.;
	  hst+=d_ut;
	  
	  i=i+1;
	  /* }*/
	}
      
	iend=i-1;
	
	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_rectangle(cr, dx,dy,lx,ly);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	switch(hg->plot_mode){
	case PLOT_EL:
	  {
	    gdouble x_tr,y_tr;
	    
	    if(i_list==hg->plot_i){
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	      cairo_set_line_width(cr,3.0);
	    }
	    else{
	      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8);
	      cairo_set_line_width(cr,1.0);
	    }
	    
	    for(i=1;i<=iend-1;i++){
	      //if((pel[i]>0)&&(pel[i+1]>0)){
	      x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(90-pel[i])/90;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(90-pel[i+1])/90;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	      
	      //cairo_arc(cr, x, y, 3, 0, 2*M_PI);
	      //cairo_fill(cr);
	      
	      //}
	    }
	    
	    x_tr=((transit.hours<ihst0)?((gdouble)transit.hours+24.):((gdouble)transit.hours))
	      +(gdouble)transit.minutes/60.;
	    if((x_tr>ihst0)&&(x_tr<ihst1)){
	      y_tr=fabs(LATITUDE_SUBARU-oequ.dec);
	      
	      cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(gdouble)(ihst1-ihst0),
			    dy+ly*y_tr/90.);
	      
	      if(i_list==hg->plot_i){
		cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
	      cairo_set_font_size (cr, 12.0);
	      }
	      else{
		cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, 9.0);
	      }
	      cairo_text_extents (cr, hg->obj[i_list].name, &extents);
	      cairo_save(cr);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_rel_move_to(cr,5.,+extents.height/2.);
	      cairo_reset_clip(cr);
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
		if( (abs(paz[i]-paz[i+1])<180) ) {
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(paz[i]<180){
		    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-paz[i])/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-(paz[i+1]-360))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-(paz[i]+360))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
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
		//if((pad[i]>0) && (pad[i+1]>0)){
		x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		y=dy+ly*(4-pad[i])/4.;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
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
		if( (abs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
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
  else{  // PLOT_PLAN
    gdouble dhst0;

    /* Lunar RA, DEC */
    if(hg->plot_moon){
      gdouble m_tr_el;

      zonedate.years=iyear;
      zonedate.months=month;
      zonedate.days=iday;
      zonedate.hours=ihst0;
      zonedate.minutes=0;
      zonedate.seconds=0;
      zonedate.gmtoff=(long)(TIMEZONE_SUBARU*3600);

      JD = ln_get_julian_local_date(&zonedate);

      ln_get_lunar_rst (JD, &observer, &orst);
      ln_get_date (orst.transit, &odate);
      ln_date_to_zonedate(&odate,&transit,(long)TIMEZONE_SUBARU*3600);
      ln_get_lunar_equ_coords (orst.transit, &oequ);
      ln_get_hrz_from_equ (&oequ, &observer, orst.transit, &ohrz);
      m_tr_el=ohrz.alt;

      ut=utstart;
      
      i=1;
      d_ut=0.5;
      hst=(gdouble)ihst0;
      
      JD_hst = ln_get_julian_local_date(&zonedate);
      
      while(hst<=(gdouble)ihst1+d_ut){
	ln_get_lunar_equ_coords (JD_hst, &oequ);
	
	ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	if(ohrz.az>180) ohrz.az-=360;

	put[i]=ut;
	phst[i]=hst;

	paz[i]=ohrz.az;
	pel[i]=ohrz.alt;

	ut=ut+d_ut;
	JD_hst+=d_ut/24.;
	hst+=d_ut;
	
	i=i+1;

      }
      
      cairo_rectangle(cr, dx,dy,lx,ly);
      cairo_clip(cr);
      cairo_new_path(cr);

      switch(hg->plot_mode){
      case PLOT_EL:
	{
	  gdouble x_tr,y_tr;
	  
	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 0.5);
	  cairo_set_line_width(cr,5.0);
	  
	  for(i=1;i<=iend-1;i++){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_stroke(cr);
	    
	  }
	  
	  x_tr=((transit.hours<ihst0)?((gdouble)transit.hours+24.):((gdouble)transit.hours))
	    +(gdouble)transit.minutes/60.;
	  
	  if((x_tr>ihst0)&&(x_tr<ihst1)){
	    cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(gdouble)(ihst1-ihst0),
			  dy+ly*(90.-m_tr_el)/90.);
	    
	    cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 1.0);
	    cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size (cr, 11.0);
	    
	    cairo_text_extents (cr, "moon", &extents);
	    cairo_save(cr);
	    cairo_rotate (cr,-M_PI/2);
	    cairo_rel_move_to(cr,5.,+extents.height/2.);
	    cairo_reset_clip(cr);
	    cairo_show_text(cr,"moon");
	    cairo_restore(cr);
	  }
	}
	break;
      }
      cairo_reset_clip(cr);
    }


  for(i_plan=0;i_plan<hg->i_plan_max;i_plan++){
    if(hg->plan[i_plan].type==PLAN_TYPE_OBJ){

      //a0s=padata->ra;
      a0s=hg->obj[hg->plan[i_plan].obj_i].ra;
      ia0h=(int)(a0s/10000);
      a0s=a0s-(double)(ia0h)*10000;
      ia0m=(int)(a0s/100);
      a0s=a0s-(double)(ia0m)*100;

      //d0s=padata->dec;
      d0s=hg->obj[hg->plan[i_plan].obj_i].dec;
      id0d=(int)(d0s/10000);
      d0s=d0s-(double)(id0d)*10000;
      id0m=(int)(d0s/100);
      d0s=d0s-(double)(id0m)*100;


      zonedate.years=iyear;
      zonedate.months=month;
      zonedate.days=iday;
      zonedate.hours=(gint)(hg->plan[i_plan].sod/60/60);
      zonedate.minutes=(hg->plan[i_plan].sod-(gint)zonedate.hours*60*60)/60;
      zonedate.seconds=0;
      zonedate.gmtoff=(long)(TIMEZONE_SUBARU*3600);
      //zonedate.gmtoff=(long)hg->obs_timezone*3600;

      ln_zonedate_to_date(&zonedate, &date);

      dhst0=(gdouble)hg->plan[i_plan].sod/60./60.;

      utstart=(double)date.hours+(double)date.minutes/60.;
      utend=utstart+(double)hg->plan[i_plan].time/60./60.;


      
      a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
      d0=id0d + id0m/60. + d0s/3600.;  //[deg]
      a0rad=pi*a0/12.;  //[rad]
      d0rad=pi*d0/180.; //[rad]

      oequ.ra=a0*360/24;
      oequ.dec=d0;
    

      ut=utstart;
      
      i=1;
      d_ut=0.1;
      
      hst=(gdouble)hg->plan[i_plan].sod/60./60.;
      
      JD_hst = ln_get_julian_local_date(&zonedate);

      while(ut<=utend){
	ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
	flst=ln_get_mean_sidereal_time(JD_hst)+LONGITUDE_SUBARU/360.*24.;

	if(ohrz.az>180) ohrz.az-=360;
	paz[i]=ohrz.az+180;
	pel[i]=ohrz.alt;

	ha=flst-a0;             //hour angle [hour]
	if(ha<-12.){
	  do{
	    ha=ha+24.;
	  }while(ha<-12.);
	}
	else if(ha>12.){
	  do{
	    ha=ha-24.;
	  }while(ha>12.);
	}
	
	sinha=sin(pi*ha/12.0);
	cosha=cos(pi*ha/12.0);
	ume=-sinha*cos(d0rad);
	den=cosphi*sin(d0rad)-sinphi*cosha*cos(d0rad);
	
	//### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	el0d=pel[i]*pi/180.+pi*4./3600./180.;
	d1rad=asin(sinphi*sin(el0d)+cosphi*cos(paz[i]*pi/180.)*cos(el0d));
	d1=d1rad*180./pi;
	ume1=-sin(paz[i]*pi/180.)*cos(el0d);
	den1=cosphi*sin(el0d)-sinphi*cos(paz[i]*pi/180.)*cos(el0d);
	ha1rad=atan2(ume1,den1);
	ha1=ha1rad*12./pi;

	if(ha1<-12.){
	  do{
	    ha1=ha1+24.;
	  }while(ha1<-12.);
	}
	else if(ha1>=12.){
	  do{
	    ha1=ha1-24.;
	  }while(ha1>=12.);
	}
	a1=flst-ha1;
	if(a1<0.){
	  do{
	    a1=a1+24.;
	  }while(a1<0.);
	}
	else if(a1>=24.){
	  do{
	    a1=a1-24.0;
	  }while(a1>=24.);
	}
	
	
	//### 4th step: (RA1-RA,Dec1-Dec) => PA
	//write(9,*)'#### Position Angle'
	delta_a=(a1-a0)*pi/12.;   //[rad]
	delta_d=d1rad-d0rad;      //[rad]
	pa=atan2(delta_a,delta_d);      
	padeg=180.*pa/pi;
	//write(9,'(4h PA=,f8.3)')padeg
	
	
	//### 5th step: Atmospheric Dispersion at 3500A
	zrad=pi*(90.0-pel[i])/180.;
	ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		  (double)hg->pres,f);  //@3500A default
	ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		  (double)hg->pres,f);  //@5500A default
	adsec=180.*3600.*(ad1-ad0)/pi;   //[arcsec]
	
	put[i]=ut;
	phst[i]=hst;
	plst[i]=flst;
	ppa[i]=padeg;
	pad[i]=adsec;


	JD_hst+=d_ut/24.;
	hst+=d_ut;
	ut=ut+d_ut;
	i=i+1;
	/* }*/

      }

      iend=i-1;
      
      cairo_set_source_rgba(cr, 1, 1, 1, 1);
      cairo_rectangle(cr, dx,dy,lx,ly);
      cairo_clip(cr);
      cairo_new_path(cr);
      
      switch(hg->plot_mode){
      case PLOT_EL:
	{
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    //if((pel[i]>0)&&(pel[i+1]>0)){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_set_line_width(cr,2.0);
	    cairo_stroke(cr);

	    if(i==1){
	      cairo_save (cr);
	      if(i_plan==hg->plot_i_plan){
		cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 11.0);
	      }
	      else{
		cairo_select_font_face (cr, SKYMON_FONT, CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, 9.0);
	      }
	      cairo_text_extents (cr, hg->obj[hg->plan[i_plan].obj_i].name,
				  &extents);

	      cairo_translate(cr, dx+lx*(phst[i]+(double)hg->plan[i_plan].time/60./60./2.-ihst0)/(gfloat)(ihst1-ihst0), dy+ly*(90-3)/90);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_move_to(cr, 0, +extents.height/2);

	      /*
	      if(pel[i]<75){
		cairo_translate (cr, x, y+extents.width/2);
		cairo_rotate (cr,-M_PI/2);
		cairo_move_to(cr, extents.width/2+10, -extents.height/2);
	      }
	      else{
		cairo_translate (cr, x, y-extents.width/2);
		cairo_rotate (cr,-M_PI/2);
		cairo_move_to(cr, -extents.width-10, -extents.height/2);
	      }
	      */
	      cairo_show_text(cr, hg->obj[hg->plan[i_plan].obj_i].name);
	      cairo_restore (cr);

	    }
	      
	      //cairo_arc(cr, x, y, 3, 0, 2*M_PI);
	      //cairo_fill(cr);
	    
	    //}
	  }
	}
	break;
      case PLOT_AZ:
	{
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(paz[i]-paz[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		y=dy+ly*(360-paz[i])/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		y=dy+ly*(360-paz[i+1])/360;
		cairo_line_to(cr,x,y);
		
		cairo_set_line_width(cr,2.0);
		cairo_stroke(cr);
	      }
	      else{
		if(paz[i]<180){
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(paz[i+1]-360))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(paz[i]+360))/360;
		  cairo_move_to(cr,x,y);

		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);

		  cairo_set_line_width(cr,2.0);
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
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
	      //if((pad[i]>0) && (pad[i+1]>0)){
	      x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(4-pad[i])/4.;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(4-pad[i+1])/4.;
	      cairo_line_to(cr,x,y);
	      
	      cairo_set_line_width(cr,2.0);
	      cairo_stroke(cr);
	      
	    }
	  }
	  
	  if(i_plan==hg->plot_i_plan){
	    cairo_set_source_rgba(cr, 1.0, 0.6, 0.6, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.4);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(ppa[i]-ppa[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_set_line_width(cr,2.0);
		cairo_stroke(cr);
	      }
	      else{
		if(ppa[i]<0){
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_set_line_width(cr,2.0);
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


  if(hg->plot_output==PLOT_OUTPUT_PDF){
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
  }

  cairo_destroy(cr);

  if(hg->plot_output==PLOT_OUTPUT_WINDOW){
    gdk_draw_drawable(widget->window,
		      widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		      pixmap_plot,
		      0,0,0,0,
		      width,
		    height);
    
    g_object_unref(G_OBJECT(pixmap_plot));
  }

  return TRUE;
}



static void refresh_plot (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->plot_output=PLOT_OUTPUT_WINDOW;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }
}


static void do_plot_moon (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  hg->plot_moon=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }
}


void pdf_plot (typHOE *hg)
{
  hg->plot_output=PLOT_OUTPUT_PDF;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }

  hg->plot_output=PLOT_OUTPUT_WINDOW;
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


// Create Plot Window
void create_plot_dialog(typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1;
  GtkWidget *frame, *check, *label, *button;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;


  hg->plot_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->plot_main), "HOE : Plot Window");
  
  my_signal_connect(hg->plot_main,
		    "destroy",
		    close_plot, 
		    (gpointer)hg);

  gtk_widget_set_app_paintable(hg->plot_main, TRUE);
  

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->plot_main), vbox);


  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Parameter");
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


  frame = gtk_frame_new ("Act.");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (refresh_plot), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Refresh");
#endif

  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_plot), (gpointer)hg);
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
		    G_CALLBACK (do_save_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

  button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_ABOUT);
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


  frame = gtk_frame_new ("Plot");
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

  
  // Drawing Area
  hg->plot_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->plot_dw, PLOT_WIDTH, PLOT_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), hg->plot_dw, TRUE, TRUE, 0);
  gtk_widget_set_app_paintable(hg->plot_dw, TRUE);
  gtk_widget_show(hg->plot_dw);

  my_signal_connect(hg->plot_dw, 
		    "expose-event", 
		    draw_plot_cairo,
		    (gpointer)hg);

  gtk_widget_show_all(hg->plot_main);

  gdk_window_raise(hg->plot_main->window);

  gdk_flush();
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

gdouble set_ul(gdouble lower, gdouble input, gdouble upper, gdouble step){

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

gdouble get_julian_day_of_epoch(gdouble epoch){
  gdouble diff_y;

  diff_y=epoch-2000.0;

  return(JD2000 + diff_y*365.25);
}

