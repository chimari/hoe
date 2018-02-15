//    HDS OPE file Editor
//      fc_output.c : Output CSV for Finding Chart List
//                                           2010.3.15  A.Tajitsu

#include"main.h"
#include"version.h"

void fcdb_out_simbad(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"SIMBAD MAIN_ID\", \"RA (deg)\", \"Dec (deg)\", \"Sp type\", \"Obj type\", \"mag_U\", \"mag_B\", \"mag_V\", \"mag_R\", \"mag_I\", \"mag_J\", \"mag_H\", \"mag_K\", \"PM_RA (mas/yr)\", \"PM_Dec (mas/yr)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, \"%s\", \"%s\", %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.6lf, %.6lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].sp,
	    hg->fcdb[i_list].otype,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].k,
	    hg->fcdb[i_list].pmra,
	    hg->fcdb[i_list].pmdec,
	    hg->fcdb[i_list].sep*60.);
  }
}

void fcdb_out_ned(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Name\", \"RA (deg)\", \"Dec (deg)\", \"Obj type\", \"Velocity\", \"Redshift\", \"Mag & Filter\", \"References\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, \"%s\", %.2lf, %.6lf, \"%s\", %d, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].otype,
	    hg->fcdb[i_list].nedvel,
	    hg->fcdb[i_list].nedz,
	    hg->fcdb[i_list].nedmag,
	    hg->fcdb[i_list].ref,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_gsc(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"HST ID\", \"RA (deg)\", \"Dec (deg)\", \"mag_U\", \"mag_B\", \"mag_V\", \"mag_R\", \"mag_I\", \"mag_J\", \"mag_H\", \"mag_K\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].k,
	    hg->fcdb[i_list].sep*60.);
  }
}

void fcdb_out_ps1(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Name\", \"RA mean (deg)\", \"Dec mean (deg)\", \"nDetections\",  \"mag_g\", \"mag_r\", \"mag_i\", \"mag_z\", \"mag_y\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %d, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].ref,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_sdss(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object ID\", \"RA (deg)\", \"Dec (deg)\", \"mag_u\", \"mag_g\", \"mag_r\", \"mag_i\", \"mag_z\", \"class\", \"Redshift\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, \"%s\", %.6lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].otype,
	    hg->fcdb[i_list].nedz,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_usno(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"USNO ID\", \"RA (deg)\", \"Dec (deg)\", \"mag_B1\", \"mag_R1\", \"mag_B2\", \"mag_R2\", \"mag_I2\", \"PM_RA (mas/yr)\", \"PM_Dec (mas/yr)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.6lf, %.6lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].pmra,
	    hg->fcdb[i_list].pmdec,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_gaia(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Source\", \"RA (deg)\", \"Dec (deg)\", \"<G mag>\", \"Parallax\", \"PM_RA (mas/yr)\", \"PM_Dec (mas/yr)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.6lf, %.6lf, %.6lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].plx,
	    hg->fcdb[i_list].pmra,
	    hg->fcdb[i_list].pmdec,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_2mass(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Designation\", \"RA (deg)\", \"Dec (deg)\", \"mag_J\", \"mag_H\", \"mag_K\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.2lf, %.2lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].k,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_wise(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"WISE\", \"RA (deg)\", \"Dec (deg)\", \"mag_J\", \"mag_H\", \"mag_K\", \"mag_W1 (3.4um)\", \"mag_W2 (4.6um)\", \"mag_W3 (12um)\", \"mag_W4 (22um)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    hg->fcdb[i_list].k,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_irc(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Name\", \"RA (deg)\", \"Dec (deg)\", \"S9W\", \"Q(S9W)\", \"L18W\", \"Q(L18W)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %d, %.2lf, %d, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    (gint)hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    (gint)hg->fcdb[i_list].r,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_fis(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Name\", \"RA (deg)\", \"Dec (deg)\", \"N60\", \"Q(N60)\", \"WIDE-S\", \"Q(WIDE-S)\", \"WIDE-L\", \"Q(WIDE-L)\", \"N160\", \"Q(N160)\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %.2lf, %d, %.2lf, %d, %.2lf, %d, %.2lf, %d, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    (gint)hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    (gint)hg->fcdb[i_list].r,
	    hg->fcdb[i_list].i,
	    (gint)hg->fcdb[i_list].j,
	    hg->fcdb[i_list].h,
	    (gint)hg->fcdb[i_list].k,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_lamost(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Designation\", \"RA (deg)\", \"Dec (deg)\", \"T_eff\", \"log g\", \"[Fe/H]\", \"V_helio\", \"Class\", \"SubClass\", \"Obs. ID\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", %.5lf, %.5lf, %d, %.2lf, %+.2lf, %.1lf, \"%s\", \"%s\", %d, %.5lf\n",
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    (gint)hg->fcdb[i_list].u,
	    hg->fcdb[i_list].b,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].r,
	    hg->fcdb[i_list].otype,
	    hg->fcdb[i_list].sp,
	    hg->fcdb[i_list].ref,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_smoka(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Frame ID\", \"Date Obs.\", \"Obs. Mode\", \"Data Type\",  \"Object\", \"Filter\", \"Wavelength\", \"RA (deg)\", \"Dec (deg)\", \"Exptime\", \"Observer\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %.5lf, %.5lf, %.2lf, \"%s\", %.5lf\n",
	    hg->fcdb[i_list].fid,
	    hg->fcdb[i_list].date,
	    hg->fcdb[i_list].mode,
	    hg->fcdb[i_list].type,
	    hg->fcdb[i_list].name,
	    hg->fcdb[i_list].fil,
	    hg->fcdb[i_list].wv,
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].obs,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_hst(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Dataset\", \"Taget Name\", \"RA (deg)\", \"Dec (deg)\", \"Start Time\", \"Exptime\",  \"Instrument\", \"Filters/Gratings\", \"Central Wavelength\", \"Proposal ID\", \"Apertures\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", \"%s\", %.5lf, %.5lf, \"%s\", %.2lf, \"%s\", \"%s\", %.1lf, \"%s\", \"%s\", %.5lf\n",
	    hg->fcdb[i_list].fid,
	    hg->fcdb[i_list].name,	
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].date,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].mode,
	    hg->fcdb[i_list].fil,
	    hg->fcdb[i_list].v,
	    hg->fcdb[i_list].obs,
	    hg->fcdb[i_list].type,
	    hg->fcdb[i_list].sep*60.);
  }
}


void fcdb_out_eso(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Dataset\", \"Object\", \"RA (deg)\", \"Dec (deg)\", \"Exptime\", \"Instrument\", \"Mode\", \"Prog. ID\", \"Release Date\", \"Dist. (arcmin)\"\n");
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    fprintf(fp,"\"%s\", \"%s\", %.5lf, %.5lf, %.2lf, \"%s\", \"%s\", \"%s\", \"%s\", %.5lf\n",
	    hg->fcdb[i_list].fid,
	    hg->fcdb[i_list].name,	
	    hg->fcdb[i_list].d_ra,
	    hg->fcdb[i_list].d_dec,
	    hg->fcdb[i_list].u,
	    hg->fcdb[i_list].mode,
	    hg->fcdb[i_list].type,
	    hg->fcdb[i_list].obs,
	    hg->fcdb[i_list].date,
	    hg->fcdb[i_list].sep*60.);
  }
}


void Export_FCDB_List(typHOE *hg){
  FILE *fp;

  if(hg->fcdb_i_max<=0) return;

  if((fp=fopen(hg->filename_fcdb,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_fcdb);
    exit(1);
  }

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    fcdb_out_simbad(hg, fp);
    break;

  case FCDB_TYPE_NED:
    fcdb_out_ned(hg, fp);
    break;

  case FCDB_TYPE_GSC:
    fcdb_out_gsc(hg, fp);
    break;

  case FCDB_TYPE_PS1:
    fcdb_out_ps1(hg, fp);
    break;

  case FCDB_TYPE_SDSS:
    fcdb_out_sdss(hg, fp);
    break;

  case FCDB_TYPE_LAMOST:
    fcdb_out_lamost(hg, fp);
    break;

  case FCDB_TYPE_USNO:
    fcdb_out_usno(hg, fp);
    break;

  case FCDB_TYPE_GAIA:
    fcdb_out_gaia(hg, fp);
    break;

  case FCDB_TYPE_2MASS:
    fcdb_out_2mass(hg, fp);
    break;

  case FCDB_TYPE_WISE:
    fcdb_out_wise(hg, fp);
    break;

  case FCDB_TYPE_IRC:
    fcdb_out_irc(hg, fp);
    break;

  case FCDB_TYPE_FIS:
    fcdb_out_fis(hg, fp);
    break;

  case FCDB_TYPE_SMOKA:
    fcdb_out_smoka(hg, fp);
    break;

  case FCDB_TYPE_HST:
    fcdb_out_hst(hg, fp);
    break;

  case FCDB_TYPE_ESO:
    fcdb_out_eso(hg, fp);
    break;
  }
  
  fclose(fp);
}


void trdb_out(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\",  \"Data\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].trdb_band_max>0){
      fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", \"%s\"\n",
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	      hg->obj[i_list].trdb_str);
    }
    else{
      fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\"\n",
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--");
    }
  }
}

void magdb_out_simbad(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"Simbad Name\", \"Object Type\", \"Spectral Type\", \"U\", \"B\", \"V\", \"R\", \"I\", \"J\", \"H\", \"K\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, \"%s\", \"%s\", \"%s\", %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_simbad_hits,
	    hg->obj[i_list].magdb_simbad_sep*3600,
	    (hg->obj[i_list].magdb_simbad_name)?
	    (hg->obj[i_list].magdb_simbad_name) : "---",
	    (hg->obj[i_list].magdb_simbad_type)?
	    (hg->obj[i_list].magdb_simbad_type) : "---",
	    (hg->obj[i_list].magdb_simbad_sp)?
	    (hg->obj[i_list].magdb_simbad_sp) : "---",
	    hg->obj[i_list].magdb_simbad_u,
	    hg->obj[i_list].magdb_simbad_b,
	    hg->obj[i_list].magdb_simbad_v,
	    hg->obj[i_list].magdb_simbad_r,
	    hg->obj[i_list].magdb_simbad_i,
	    hg->obj[i_list].magdb_simbad_j,
	    hg->obj[i_list].magdb_simbad_h,
	    hg->obj[i_list].magdb_simbad_k);
  }
}

void magdb_out_ned(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"NED Name\", \"Object Type\", \"Mag.\", \"z\", \"ref#\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, \"%s\", \"%s\", \"%s\", %.6lf, %d\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_ned_hits,
	    hg->obj[i_list].magdb_ned_sep*3600,
	    (hg->obj[i_list].magdb_ned_name)?
	    (hg->obj[i_list].magdb_ned_name) : "---",
	    (hg->obj[i_list].magdb_ned_type)?
	    (hg->obj[i_list].magdb_ned_type) : "---",
	    (hg->obj[i_list].magdb_ned_mag)?
	    (hg->obj[i_list].magdb_ned_mag) : "---",
	    hg->obj[i_list].magdb_ned_z,
	    hg->obj[i_list].magdb_ned_ref);
  }
}

void magdb_out_lamost(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"LAMOST Name\", \"Obs ID\", \"Teff\", \"log g\", \"[Fe/H]\", \"HRV\", \"Type\", \"Sp.\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, \"%s\", %d, %.0lf, %.2lf, %+.2lf, %+.1lf, \"%s\", \"%s\"\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_lamost_hits,
	    hg->obj[i_list].magdb_lamost_sep*3600,
	    (hg->obj[i_list].magdb_lamost_name)?
	    (hg->obj[i_list].magdb_lamost_name) : "---",
	    hg->obj[i_list].magdb_lamost_ref,
	    hg->obj[i_list].magdb_lamost_teff,
	    hg->obj[i_list].magdb_lamost_logg,
	    hg->obj[i_list].magdb_lamost_feh,
	    hg->obj[i_list].magdb_lamost_hrv,
	    (hg->obj[i_list].magdb_lamost_type)?
	    (hg->obj[i_list].magdb_lamost_type) : "---",
	    (hg->obj[i_list].magdb_lamost_sp)?
	    (hg->obj[i_list].magdb_lamost_sp) : "---");
  }
}


void magdb_out_gsc(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"U\", \"B\", \"V\", \"R\", \"I\", \"J\", \"H\", \"K\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_gsc_hits,
	    hg->obj[i_list].magdb_gsc_sep*3600,
	    hg->obj[i_list].magdb_gsc_u,
	    hg->obj[i_list].magdb_gsc_b,
	    hg->obj[i_list].magdb_gsc_v,
	    hg->obj[i_list].magdb_gsc_r,
	    hg->obj[i_list].magdb_gsc_i,
	    hg->obj[i_list].magdb_gsc_j,
	    hg->obj[i_list].magdb_gsc_h,
	    hg->obj[i_list].magdb_gsc_k);
  }
}

void magdb_out_ps1(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"g\", \"r\", \"i\", \"z\", \"y\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_ps1_hits,
	    hg->obj[i_list].magdb_ps1_sep*3600,
	    hg->obj[i_list].magdb_ps1_g,
	    hg->obj[i_list].magdb_ps1_r,
	    hg->obj[i_list].magdb_ps1_i,
	    hg->obj[i_list].magdb_ps1_z,
	    hg->obj[i_list].magdb_ps1_y);
  }
}


void magdb_out_sdss(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"u\", \"g\", \"r\", \"i\", \"z\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_sdss_hits,
	    hg->obj[i_list].magdb_sdss_sep*3600,
	    hg->obj[i_list].magdb_sdss_u,
	    hg->obj[i_list].magdb_sdss_g,
	    hg->obj[i_list].magdb_sdss_r,
	    hg->obj[i_list].magdb_sdss_i,
	    hg->obj[i_list].magdb_sdss_z);
  }
}

void magdb_out_gaia(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"G\", \"Parallax\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_gaia_hits,
	    hg->obj[i_list].magdb_gaia_sep*3600,
	    hg->obj[i_list].magdb_gaia_g,
	    hg->obj[i_list].magdb_gaia_p);
  }
}

void magdb_out_2mass(typHOE *hg, FILE *fp){
  int i_list;

  fprintf(fp, "\"Object\", \"RA\", \"Dec\", \"Note\", \"Hits\", \"Sep.\", \"J\", \"H\", \"K\"\n");
  for(i_list=0;i_list<hg->i_max;i_list++){
    fprintf(fp,"\"%s\", \"%09.2lf\", \"%+010.2lf\", \"%s\", %d, %.1lf, %.2lf, %.2lf, %.2lf\n",
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    (hg->obj[i_list].note) ? (hg->obj[i_list].note) : "--",
	    hg->obj[i_list].magdb_2mass_hits,
	    hg->obj[i_list].magdb_2mass_sep*3600,
	    hg->obj[i_list].magdb_2mass_j,
	    hg->obj[i_list].magdb_2mass_h,
	    hg->obj[i_list].magdb_2mass_k);
  }
}


void Export_TRDB_CSV(typHOE *hg){
  FILE *fp;

  if(hg->i_max<=0) return;

  if((fp=fopen(hg->filename_trdb,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_trdb);
    exit(1);
  }

  switch(hg->trdb_used){
  case TRDB_TYPE_SMOKA:
  case TRDB_TYPE_HST:
  case TRDB_TYPE_ESO:
  case TRDB_TYPE_GEMINI:
    trdb_out(hg, fp);
    break;

  case MAGDB_TYPE_SIMBAD:
    magdb_out_simbad(hg, fp);
    break;

  case MAGDB_TYPE_NED:
    magdb_out_ned(hg, fp);
    break;

  case MAGDB_TYPE_LAMOST:
    magdb_out_lamost(hg, fp);
    break;

  case MAGDB_TYPE_GSC:
    magdb_out_gsc(hg, fp);
    break;

  case MAGDB_TYPE_PS1:
    magdb_out_ps1(hg, fp);
    break;

  case MAGDB_TYPE_SDSS:
    magdb_out_sdss(hg, fp);
    break;

  case MAGDB_TYPE_GAIA:
    magdb_out_gaia(hg, fp);
    break;

  case MAGDB_TYPE_2MASS:
    magdb_out_2mass(hg, fp);
    break;
  }
  
  fclose(fp);
}

