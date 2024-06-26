//    HDS OPE file Editor
//      votable.c : pursing VOTable, imported from libVOTable
//                                           2012.10.22  A.Tajitsu
/* libVOTable - VOTABLE parser 
 Copyright (C) 2005  Malapert Jean-christophe - TERAPIX - IAP/CNRS

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 any later version.
     
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
     
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Please use the following e_mail for questions, feedback and bug fixes <malapert@iap.fr>

 */


//-------------------------------------------------------------
//- Project: libVotable
//- Filename: votable.c
//-------------------------------------------------------------
//- Author: J-C MALAPERT
//- Creation date: 25/11/2004
//- Last modification date: 12/05/2005
//-------------------------------------------------------------
//- Comments:
//- 25/11/2004    Creation
//- 12/01/2005    First Release
//- 12/05/2005    Bug report & fix by Jamie Stevens about
//                indice called "position"
//-------------------------------------------------------------


#include "main.h"
#include "votable.h"

gchar *rm_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_obj,i_tgt=0;

  if((tgt_name=(gchar *)g_malloc(sizeof(gchar)*(strlen(obj_name)+1)))
     ==NULL){
    fprintf(stderr, "!!! Memory allocation error in rm_spc().\n");
    fflush(stderr);
    return(NULL);
  }

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(obj_name[i_obj]!=0x20){
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}

void make_band_str(typHOE *hg, gint i, gint mode){
  gint i_band;
  gchar *tmp_str1=NULL, *tmp_str2=NULL, *tmp_str3=NULL;

  switch(mode){
  case TRDB_TYPE_SMOKA:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "%s %.0lfs(%d)" : "%s %.1lfs(%d)",	       
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",	       
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else{
	if(g_ascii_strncasecmp(hg->obj[i].trdb_mode[i_band],
			       "imag",strlen("imag"))==0){
	  tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				   "%s: %s %.0lfs(%d)" : "%s: %s %.1lfs(%d)",
				   hg->obj[i].trdb_mode[i_band],
				   hg->obj[i].trdb_band[i_band],
				   hg->obj[i].trdb_exp[i_band],
				   hg->obj[i].trdb_shot[i_band]);
	}
	else{
	  tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				   "%s: [%s] %.0lfs(%d)" : "%s: [%s] %.1lfs(%d)",
				   hg->obj[i].trdb_mode[i_band],
				   hg->obj[i].trdb_band[i_band],
				   hg->obj[i].trdb_exp[i_band],
				   hg->obj[i].trdb_shot[i_band]);
	}
      }
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;

  case TRDB_TYPE_HST:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "%s %.0lfs(%d)" : "%s %.1lfs(%d)",
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else{
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;


  case TRDB_TYPE_ESO:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
			       "%s %.0lfs(%d)" : "%s %.1lfs(%d)",
			       hg->obj[i].trdb_mode[i_band],
			       hg->obj[i].trdb_exp[i_band],
			       hg->obj[i].trdb_shot[i_band]);

      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }

      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }

    break;

  case TRDB_TYPE_GEMINI:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
			       "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",
			       hg->obj[i].trdb_band[i_band],
			       hg->obj[i].trdb_exp[i_band],
			       hg->obj[i].trdb_shot[i_band]);
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;
  }

  if(hg->obj[i].trdb_str) g_free(hg->obj[i].trdb_str);
  hg->obj[i].trdb_str=g_strdup(tmp_str3);

  if(tmp_str3) g_free(tmp_str3);
  tmp_str3=NULL;
}

static list_field *insert_field(xmlTextReaderPtr reader, 
				  list_field *list, 
				  int position) {

  list_field *vlist_field;
  list_field *vlist_move;
  vlist_field = NULL;
  /* Memory allocation for the new element */
  QMALLOC(vlist_field, list_field, 1);

  /* Copy value */
  vlist_field->ID  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ID");
  vlist_field->unit  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit");
  vlist_field->datatype  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"datatype");
  vlist_field->precision  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"precision");
  vlist_field->width  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"width");
  vlist_field->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");
  vlist_field->name = xmlTextReaderGetAttribute(reader,(const xmlChar *)"name");
  vlist_field->ucd  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ucd");
  vlist_field->arraysize  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"arraysize");
  vlist_field->type  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"type");
  vlist_field->position = position;

  /* Join with the next element of the list */
  vlist_field->next = list;

  return(vlist_field);
}



static list_tabledata *insert_tabledata(xmlTextReaderPtr reader,
					list_tabledata *list, 
					int position) {

  list_tabledata *vlist_tabledata;
  list_tabledata *vlist_move;
  vlist_tabledata = NULL;

  /* Memory allocation for the new element */
  QMALLOC(vlist_tabledata,list_tabledata,1);

  /* Copy value */
  vlist_tabledata->value  = xmlTextReaderValue(reader);
  vlist_tabledata->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");
  vlist_tabledata->colomn = position;

  /* Join with the next element of the list */
  vlist_tabledata->next = list;

  return(vlist_tabledata);
}



static list_table *insert_table(xmlTextReaderPtr reader) {

  list_table *vlist_table;

  /* Memory allocation for the new element */
  QMALLOC(vlist_table,list_table,1);

  /* Copy value */
  vlist_table->ID  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ID");
  vlist_table->name  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"name");
  vlist_table->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");

  return(vlist_table);
}


int Move_to_Next_VO_Fields (xmlTextReaderPtr reader) {

  int ret;
  xmlChar *name;
  
  ret = 1;

  /* Reading file */
  ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    /* Searching FIELD tag */
    if (xmlStrcmp(name,(const xmlChar *)"FIELD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      xmlFree(name);
      return(RETURN_OK);  
    } else {
      ret = xmlTextReaderRead(reader);
      if (name!=NULL)
	xmlFree(name);
    }

  }
  return(RETURN_ERROR);
}



int Move_to_Next_VO_Table (xmlTextReaderPtr reader) {

  int ret;
  xmlChar *name;

  ret = 1;
  /* Reading file */
  ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    /* Searching TABLE tag */
    if (xmlStrcmp(name,(const xmlChar *)"TABLE") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      xmlFree(name);
      return(RETURN_OK);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name!=NULL)
	xmlFree(name);
    }
  }
  return(RETURN_ERROR);
}



void Extract_Att_VO_Table(xmlTextReaderPtr reader, 
			  VOTable *votablePtr,
			  gchar *fname,
			  GtkWidget *parent) {

  xmlChar *name;
  int ret;

  ret = 1;
  /* Free memory if needed */
  if (votablePtr->table != NULL)
    Free_VO_Table(votablePtr->table);
  if(reader == NULL) 
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    /* Reading file */
    name = xmlTextReaderName(reader);
     if (name == NULL)
        name = xmlStrdup(BAD_CAST "--");
     /* Searching TABLE tag */
    if (xmlStrcmp(name,(const xmlChar *)"TABLE") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      votablePtr->table = insert_table(reader); 
      ret = 0;
      xmlFree(name);
    } else {
      ret = xmlTextReaderRead(reader);
      if(ret==-1){
	popup_message(parent, 
#ifdef USE_GTK3
		      "dialog-warning", 
#else
		      GTK_STOCK_DIALOG_WARNING,
#endif
		      POPUP_TIMEOUT*2,
		      "Error : XML table cannot be parsed.",
		      " ",
		      fname,
		      NULL);
      }
      if (name!=NULL)
	xmlFree(name);
    }
  }
}



void Extract_VO_Fields ( xmlTextReaderPtr reader,
			 VOTable *votablePtr,
			 int *nbFields, 
			 int **columns) {

  int ret;
  int position;
  int i;
  xmlChar *name;
  
  /* Free memory if needed */
  if (votablePtr->field != NULL) {
    Free_VO_Fields(votablePtr->field,columns);
    votablePtr->field = NULL;
  }

  /* Init variable */
  position = 0;
  ret = 1;

  /* Reading file */
  if(reader == NULL)
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);    
    if (name == NULL)
      name = xmlStrdup(BAD_CAST "--");
    /* Searching FIELD tag */
    if (xmlStrcmp(name,(const xmlChar *)"FIELD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      /* Number of FIELD met */
      position++;
      /* Insert in the linking list the attribute values of the element */
      votablePtr->field = insert_field(reader,votablePtr->field,position);
      /* go on reading */
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
    }
    else if(xmlStrcmp(name,(const xmlChar *)"DATA") == 0 
	    && xmlTextReaderNodeType(reader) == 1) {
      ret = 0; 
      xmlFree(name);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name != NULL)
	xmlFree(name);
    }
  }

  /* Memory allocation for columns in order to avoid to user to do that*/
  QMALLOC(*columns,int,position);
  /* Field tag number found */
  *nbFields = position;
  /* Initialization of columns */ 
  for(i=0;i<position;i++) 
    (*columns)[i] = 0;
}



int Extract_VO_TableData (xmlTextReaderPtr reader, 
			  VOTable *votablePtr,  
			  int nbcolumns, 
			  int *columns) {

  xmlChar *name;
  int column_number;
  int ret,cnt,nblines;
  int *pinit;

  nblines = 0;
  /* Free memory if needed */
  if (votablePtr->tabledata != NULL)
    Free_VO_Tabledata(votablePtr->tabledata);
  /* Initialization */
  ret = 1;
  column_number = 0;
  pinit = columns;
  if(reader == NULL)
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    if (name == NULL)
      name = xmlStrdup(BAD_CAST "--");
    /* Search TD node*/
    if (xmlStrcmp(name,(const xmlChar *)"TD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      /* Retrieve TD tag value */
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
      column_number++;
      /* retrieve all data for columns selected */
      for(cnt=0;cnt<nbcolumns;cnt++) 
	if (columns[cnt] == column_number)
	  votablePtr->tabledata = insert_tabledata(reader,votablePtr->tabledata,column_number);
      
      columns = pinit;
      /* Start a TR tag */
      if (column_number == nbcolumns) {
	column_number = 0;
	nblines++;
      }
    } else if(xmlStrcmp(name,(const xmlChar *)"TABLEDATA") == 0 
	      && xmlTextReaderNodeType(reader) == 15) {
        ret = 0;
	xmlFree(name);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name != NULL)
	xmlFree(name);
    }
  }
  return(nblines);
}


xmlTextReaderPtr Init_VO_Parser(const char *filename,
				VOTable *votablePtr) {

  xmlTextReaderPtr reader;

  /* Initialisation linking lists */
  votablePtr->field = NULL;
  votablePtr->tabledata = NULL;
  votablePtr->table = NULL;

  /* Init xml Memory */
  xmlInitMemory();

  /* Reading file */
  if ((reader = xmlReaderForFile(filename, NULL, 0)) == NULL) {
    fprintf(stderr,"xmlReaderForFile failed\n");
    //exit(EXIT_READING);
  }

  return(reader);
}



static void Free_VO_Table(list_table *vlist_table) {

  if(vlist_table != NULL) {
    if(vlist_table->ID != NULL)
      xmlFree(vlist_table->ID);
    
    if(vlist_table->name != NULL)
      xmlFree(vlist_table->name);
    
    if(vlist_table->ref != NULL)
      xmlFree(vlist_table->ref);

    xmlFree(vlist_table);
    vlist_table = NULL;
  }
}



static void Free_VO_Fields(list_field *vlist_field, 
			   int **column) {

  /* Cleanup memory */
  list_field *vfield_move, *tmpPtr_field;
  if(*column != NULL) {
    free(*column);    
    *column = NULL;
  }

  for(vfield_move=vlist_field;vfield_move!=NULL;vfield_move=tmpPtr_field)
  {
    tmpPtr_field = vfield_move->next;
    if (vfield_move != NULL) {
      if (vfield_move->ID != NULL)
	xmlFree(vfield_move->ID);      
      if (vfield_move->name != NULL)
	xmlFree(vfield_move->name);
      if (vfield_move->unit != NULL)
	xmlFree(vfield_move->unit);
      if (vfield_move->datatype != NULL)
	xmlFree(vfield_move->datatype);
      if (vfield_move->precision != NULL)
	xmlFree(vfield_move->precision);
      if (vfield_move->width != NULL)
	xmlFree(vfield_move->width);
      if (vfield_move->ref != NULL)
	xmlFree(vfield_move->ref);
      if (vfield_move->ucd != NULL)
	xmlFree(vfield_move->ucd);
      if (vfield_move->arraysize != NULL)
	xmlFree(vfield_move->arraysize);
      if (vfield_move->type != NULL)
	xmlFree(vfield_move->type);
      free(vfield_move);
    }
  }
  vlist_field = NULL;
}



static void Free_VO_Tabledata(list_tabledata *vlist_tabledata) {

  list_tabledata *vtabledata_move, *tmpPtr_tabledata;
  for(vtabledata_move=vlist_tabledata;vtabledata_move!=NULL;vtabledata_move=tmpPtr_tabledata)
  {
    tmpPtr_tabledata = vtabledata_move->next;
    if (vtabledata_move != NULL) {
      if (vtabledata_move->value != NULL)
	xmlFree(vtabledata_move->value);       
      if (vtabledata_move->ref != NULL)
	xmlFree(vtabledata_move->ref);    
      free(vtabledata_move);
    }
  }
  vlist_tabledata = NULL;
}



int Free_VO_Parser(xmlTextReaderPtr reader,
		   VOTable *votablePtr,
		   int **column) {
  list_tabledata *vtabledata_move, *tmpPtr_tabledata;

  /* Cleanup memory */
   if (votablePtr->field != NULL)
    Free_VO_Fields(votablePtr->field,column);
  if (votablePtr->table != NULL)
    Free_VO_Table(votablePtr->table);
  if (votablePtr->tabledata != NULL)
    Free_VO_Tabledata(votablePtr->tabledata);

  xmlFreeTextReader(reader);
  
  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();
  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();

  return(RETURN_OK);
}



void stddb_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0;
  gdouble d_ra0,d_dec0;

  reader = Init_VO_Parser(hg->std_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->std_i_max=0;
    return;
  }

  d_ra0=ra_to_deg(hg->obj[hg->std_i].ra);
  d_dec0=dec_to_deg(hg->obj[hg->std_i].dec);

  Extract_Att_VO_Table(reader,&votable,hg->std_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ROT:Vsini") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[14] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->std[i_list].name) g_free(hg->std[i_list].name);
      hg->std[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->std[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].d_ra=0.0;
      }
      hg->std[i_list].ra=deg_to_ra(hg->std[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->std[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].d_dec=0.0;
      }
      hg->std[i_list].dec=deg_to_dec(hg->std[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(hg->std[i_list].sp) g_free(hg->std[i_list].sp);
      hg->std[i_list].sp=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->std[i_list].rot=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].rot=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->std[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->std[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->std[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->std[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->std[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->std[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){
      if(vtabledata_move->value){
	hg->std[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){
      if(vtabledata_move->value){
	hg->std[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].k=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){
      if(vtabledata_move->value){
	hg->std[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[14]){
      if(vtabledata_move->value){
	hg->std[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->std[i_list].pmdec=0;
      }
    }
  }
  hg->std_i_max=i_list;
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->std_i_max;i_list++){
    if(!hg->std[i_list].sp) hg->std[i_list].sp=g_strdup("---");
    if((fabs(hg->std[i_list].pmra)>50)||(fabs(hg->std[i_list].pmdec)>50)){
      hg->std[i_list].pm=TRUE;
    }
    else{
      hg->std[i_list].pm=FALSE;
    }
    hg->std[i_list].equinox=2000.00;
    hg->std[i_list].sep=deg_sep(d_ra0,d_dec0,
				hg->std[i_list].d_ra,hg->std[i_list].d_dec);
  }
}


void fcdb_simbad_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag, sep;
  gint i_mag;
  gint nop_band;
  gdouble yrs, new_d_ra, new_d_dec;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"OTYPE_S") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[14] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
      hg->fcdb[i_list].sp=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[14]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  yrs=current_yrs(hg);
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].sp) hg->fcdb[i_list].sp=g_strdup("---");
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;

    new_d_ra=hg->fcdb[i_list].d_ra+
      hg->fcdb[i_list].pmra/1000/60/60*yrs;
    new_d_dec=hg->fcdb[i_list].d_dec+
      hg->fcdb[i_list].pmdec/1000/60/60*yrs;
    
    hg->fcdb[i_list].sep=deg_sep(new_d_ra,new_d_dec,
    				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    //hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
    //				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_simbad_band){
      case FCDB_BAND_NOP:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	  nop_band=FCDB_BAND_R;
	}
	else if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	  nop_band=FCDB_BAND_V;
	}
	else if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	  nop_band=FCDB_BAND_I;
	}
	break;
      case FCDB_BAND_U:
	if(hg->fcdb[i_list].u<mag){
	  mag=hg->fcdb[i_list].u;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_B:
	if(hg->fcdb[i_list].b<mag){
	  mag=hg->fcdb[i_list].b;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_V:
	if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_R:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_I:
	if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_J:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_H:
	if(hg->fcdb[i_list].h<mag){
	  mag=hg->fcdb[i_list].h;
	  i_mag=i_list;
	}
	break;
	
      case FCDB_BAND_K:
	if(hg->fcdb[i_list].k<mag){
	  mag=hg->fcdb[i_list].k;
	  i_mag=i_list;
	}
	break;
      }
    }

    if((hg->magdb_simbad_band==FCDB_BAND_NOP)&&(mag>99)){
      sep=+100;
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb[i_list].sep<sep){
	  sep=hg->fcdb[i_list].sep;
	  mag=+100;
	  i_mag=i_list;
	}
      }
    }

    if((mag<99)||
       ((hg->magdb_simbad_band==FCDB_BAND_NOP)&&(hg->fcdb_i_max!=0))){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_SIMBAD;
	if(hg->magdb_simbad_band==FCDB_BAND_NOP){
	  hg->obj[hg->fcdb_i].magdb_band=nop_band;
	}
	else{
	  hg->obj[hg->fcdb_i].magdb_band=hg->magdb_simbad_band;
	}
      }
      hg->obj[hg->fcdb_i].magdb_simbad_hits=hg->fcdb_i_max;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE); 
      hg->obj[hg->fcdb_i].magdb_simbad_sep=hg->fcdb[i_mag].sep;
      if(hg->obj[hg->fcdb_i].magdb_simbad_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_name);
      hg->obj[hg->fcdb_i].magdb_simbad_name=g_strdup(hg->fcdb[i_mag].name);
      if(hg->obj[hg->fcdb_i].magdb_simbad_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_type);
      hg->obj[hg->fcdb_i].magdb_simbad_type=g_strdup(hg->fcdb[i_mag].otype);
      if(hg->obj[hg->fcdb_i].magdb_simbad_sp) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_sp);
      hg->obj[hg->fcdb_i].magdb_simbad_sp=g_strdup(hg->fcdb[i_mag].sp);
      if(hg->magdb_pm){
	hg->obj[hg->fcdb_i].pm_ra=hg->fcdb[i_mag].pmra;
	hg->obj[hg->fcdb_i].pm_dec=hg->fcdb[i_mag].pmdec;
      }
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_simbad_hits=0;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE); 
      hg->obj[hg->fcdb_i].magdb_simbad_sep=-1;
      if(hg->obj[hg->fcdb_i].magdb_simbad_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_name);
      hg->obj[hg->fcdb_i].magdb_simbad_name=NULL;
      if(hg->obj[hg->fcdb_i].magdb_simbad_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_type);
      hg->obj[hg->fcdb_i].magdb_simbad_type=NULL;
      if(hg->obj[hg->fcdb_i].magdb_simbad_sp) 
	g_free(hg->obj[hg->fcdb_i].magdb_simbad_sp);
      hg->obj[hg->fcdb_i].magdb_simbad_sp=NULL;
      if(hg->magdb_pm){
	//hg->obj[hg->fcdb_i].pm_ra=0.0;
	//hg->obj[hg->fcdb_i].pm_dec=0.0;
      }
    }

    if(hg->obj[hg->fcdb_i].magj>99)
      hg->obj[hg->fcdb_i].magj         =hg->obj[hg->fcdb_i].magdb_simbad_j;
    if(hg->obj[hg->fcdb_i].magh>99)
      hg->obj[hg->fcdb_i].magh         =hg->obj[hg->fcdb_i].magdb_simbad_h;
    if(hg->obj[hg->fcdb_i].magk>99)
      hg->obj[hg->fcdb_i].magk         =hg->obj[hg->fcdb_i].magdb_simbad_k;
  }
}

void fcdb_hsc_simbad_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag, sep;
  gint i_mag;
  gint mag6=0, mag7=0, mag8=0, mag9=0, mag10=0, mag11=0;
  gint i_max;
  gdouble mag_max;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
      columns[3] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  mag_max=100;
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    if(hg->fcdb[i_list].v<mag_max){
      i_max=i_list;
      mag_max=hg->fcdb[i_list].v;
    }

    if(hg->fcdb[i_list].v < 7.0){
      mag6++;
    }
    else if(hg->fcdb[i_list].v < 8.0){
      mag7++;
    }
    else if(hg->fcdb[i_list].v < 9.0){
      mag8++;
    }
    else if(hg->fcdb[i_list].v < 10.0){
      mag9++;
    }
    else if(hg->fcdb[i_list].v < 11.0){
      mag10++;
    }
    else if(hg->fcdb[i_list].v < 12.0){
      mag11++;
    }
  }

  hg->obj[hg->fcdb_i].hscmag.hits=hg->fcdb_i_max;
  if(hg->obj[hg->fcdb_i].hscmag.name) g_free(hg->obj[hg->fcdb_i].hscmag.name); 
  hg->obj[hg->fcdb_i].hscmag.name=g_strdup(hg->fcdb[i_max].name);
  hg->obj[hg->fcdb_i].hscmag.v=mag_max;
  hg->obj[hg->fcdb_i].hscmag.sep=hg->fcdb[i_max].sep;
  
  hg->obj[hg->fcdb_i].hscmag.mag6 =mag6;
  hg->obj[hg->fcdb_i].hscmag.mag7 =mag7;
  hg->obj[hg->fcdb_i].hscmag.mag8 =mag8;
  hg->obj[hg->fcdb_i].hscmag.mag9 =mag9;
  hg->obj[hg->fcdb_i].hscmag.mag10=mag10;
  hg->obj[hg->fcdb_i].hscmag.mag11=mag11;
}


void fcdb_ned_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble sep;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Object Name") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Type") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Velocity") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Redshift") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Magnitude and Filter") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"References") == 0) 
      columns[7] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_ned_ref)||(hg->fcdb[i_list].ref!=0)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].nedvel=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].nedvel=-99999;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].nedz=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].nedz=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(hg->fcdb[i_list].nedmag) g_free(hg->fcdb[i_list].nedmag);
      hg->fcdb[i_list].nedmag=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].ref=0;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].nedmag) hg->fcdb[i_list].nedmag=g_strdup("---");
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }


  if(magextract){
    sep=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb[i_list].sep<sep){
	sep=hg->fcdb[i_list].sep;
	i_mag=i_list;
      }
    }

    if(hg->fcdb_i_max!=0){
      hg->obj[hg->fcdb_i].magdb_ned_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_ned_sep=hg->fcdb[i_mag].sep;
      hg->obj[hg->fcdb_i].magdb_ned_z=hg->fcdb[i_mag].nedz;
      hg->obj[hg->fcdb_i].magdb_ned_ref=hg->fcdb[i_mag].ref;
      if(hg->obj[hg->fcdb_i].magdb_ned_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_name);
      hg->obj[hg->fcdb_i].magdb_ned_name=g_strdup(hg->fcdb[i_mag].name);
      if(hg->obj[hg->fcdb_i].magdb_ned_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_type);
      hg->obj[hg->fcdb_i].magdb_ned_type=g_strdup(hg->fcdb[i_mag].otype);
      if(hg->obj[hg->fcdb_i].magdb_ned_mag) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_mag);
      hg->obj[hg->fcdb_i].magdb_ned_mag=g_strdup(hg->fcdb[i_mag].nedmag);
    }
    else{
      hg->obj[hg->fcdb_i].magdb_ned_hits=0;
      hg->obj[hg->fcdb_i].magdb_ned_sep=-1;
      hg->obj[hg->fcdb_i].magdb_ned_z=-100;
      hg->obj[hg->fcdb_i].magdb_ned_ref=0;
      if(hg->obj[hg->fcdb_i].magdb_ned_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_name);
      hg->obj[hg->fcdb_i].magdb_ned_name=NULL;
      if(hg->obj[hg->fcdb_i].magdb_ned_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_type);
      hg->obj[hg->fcdb_i].magdb_ned_type=NULL;
      if(hg->obj[hg->fcdb_i].magdb_ned_mag) 
	g_free(hg->obj[hg->fcdb_i].magdb_ned_mag);
      hg->obj[hg->fcdb_i].magdb_ned_mag=NULL;
    }
  }
}


void fcdb_gsc_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0,i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"hstID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"UMag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BMag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"VMag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IMag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassJMag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassHMag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassKsMag") == 0) 
      columns[10] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_gsc_fil)||(hg->fcdb[i_list].r<=hg->fcdb_gsc_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_gsc_band){
      case GSC_BAND_U:
	if(hg->fcdb[i_list].u<mag){
	  mag=hg->fcdb[i_list].u;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_B:
	if(hg->fcdb[i_list].b<mag){
	  mag=hg->fcdb[i_list].b;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_V:
	if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_R:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_I:
	if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_J:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_H:
	if(hg->fcdb[i_list].h<mag){
	  mag=hg->fcdb[i_list].h;
	  i_mag=i_list;
	}
	break;
	
      case GSC_BAND_K:
	if(hg->fcdb[i_list].k<mag){
	  mag=hg->fcdb[i_list].k;
	  i_mag=i_list;
	}
	break;
      }
    }
    
    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_GSC;
	hg->obj[hg->fcdb_i].magdb_band=hg->magdb_gsc_band;
      }
      hg->obj[hg->fcdb_i].magdb_gsc_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_gsc_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE); 
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_gsc_hits=0;
      hg->obj[hg->fcdb_i].magdb_gsc_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE); 
    }
  }
}


void fcdb_ircs_gsc_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0,i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"hstID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"UMag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BMag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"VMag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IMag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassJMag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassHMag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassKsMag") == 0) 
      columns[10] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  ircs_gs_selection(hg, MAGDB_TYPE_GSC, GSC_BAND_R);
}




void fcdb_ps1_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAmean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DECmean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gMeanPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iMeanPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zMeanPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yMeanPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanApMag") == 0) 
	columns[9] = vfield_move->position;
    }
    else if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"raMean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"decMean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gMeanPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iMeanPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zMeanPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yMeanPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanApMag") == 0) 
	columns[9] = vfield_move->position;
    }
    else{
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"raMean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"decMean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rApMag") == 0) 
	columns[9] = vfield_move->position;
    }
  }
  
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_ps1_fil)||(hg->fcdb[i_list].r<=hg->fcdb_ps1_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //ndetections
      if(vtabledata_move->value){
	hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].ref=0;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ //g-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].r<-900) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].i<-900) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){ //z-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].j<-900) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){ //y-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].h<-900) hg->fcdb[i_list].h=+100;
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){ //r Ap mag
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].k<-900) hg->fcdb[i_list].k=+100;
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
    if((hg->fcdb[i_list].r<100)&&(hg->fcdb[i_list].k<100)){
      hg->fcdb[i_list].u=hg->fcdb[i_list].k-hg->fcdb[i_list].r;
    }
    else{
      hg->fcdb[i_list].u=+100;
    }
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_ps1_band){
      case PS1_BAND_G:
	if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	}
	break;
	
      case PS1_BAND_R:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	}
	break;
	
      case PS1_BAND_I:
	if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	}
	break;
	
      case PS1_BAND_Z:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
	
      case PS1_BAND_Y:
	if(hg->fcdb[i_list].h<mag){
	  mag=hg->fcdb[i_list].h;
	  i_mag=i_list;
	}
	break;
      }
    }

    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_PS1;
	hg->obj[hg->fcdb_i].magdb_band=hg->magdb_ps1_band;
      }
      hg->obj[hg->fcdb_i].magdb_ps1_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_ps1_apsf=hg->fcdb[i_mag].u;
      hg->obj[hg->fcdb_i].magdb_ps1_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);      
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_ps1_hits=0;
      hg->obj[hg->fcdb_i].magdb_ps1_apsf=100;
      hg->obj[hg->fcdb_i].magdb_ps1_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE);      
    }
  }
}


void fcdb_ircs_ps1_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(hg->fcdb_ps1_dr==FCDB_PS1_OLD){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAmean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DECmean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gMeanPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iMeanPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zMeanPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yMeanPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanApMag") == 0) 
	columns[9] = vfield_move->position;
    }
    else if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"raMean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"decMean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gMeanPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iMeanPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zMeanPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yMeanPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanApMag") == 0) 
	columns[9] = vfield_move->position;
    }
    else{
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"raMean") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"decMean") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gPSFMag") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rPSFMag") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iPSFMag") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zPSFMag") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yPSFMag") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rApMag") == 0) 
	columns[9] = vfield_move->position;
    }
  }
  
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //ndetections
      if(vtabledata_move->value){
	hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].ref=0;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ //g-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].r<-900) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].i<-900) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){ //z-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].j<-900) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){ //y-band
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].h<-900) hg->fcdb[i_list].h=+100;
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){ //r-band Ap
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].k<-900) hg->fcdb[i_list].k=+100;
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
    if((hg->fcdb[i_list].r<100)&&(hg->fcdb[i_list].k<100)){
      hg->fcdb[i_list].u=hg->fcdb[i_list].k-hg->fcdb[i_list].r;
    }
    else{
      hg->fcdb[i_list].u=+100;
    }
  }
 
  ircs_gs_selection(hg, MAGDB_TYPE_PS1, PS1_BAND_R);
}


void fcdb_hds_gsc_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0,i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"hstID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"UMag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BMag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"VMag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IMag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassJMag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassHMag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"tmassKsMag") == 0) 
      columns[10] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  hds_sv_mode_selection(hg);
}




void fcdb_sdss_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"u") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"g") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"r") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"i") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"z") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"redshift") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"class") == 0) 
	columns[9] = vfield_move->position;
    }
    else{ // SPEC
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ObjID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"u") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"g") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"r") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"i") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"z") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"redshift") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"class") == 0) 
	columns[9] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"specObjID") == 0) 
	columns[10] = vfield_move->position;
    }
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(strlen(vtabledata_move->value)<64){
	//if (strncmp((const char *)vtabledata_move->value, "\nSELECT TOP 5000", 
	//	  strlen("\nSELECT TOP 5000")) != 0){
	if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
	hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
	if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG){
	  i_list++;
	  i_all++;
	}
	else if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_SPEC){
	  printf("%s\n",hg->fcdb[i_list].sp);
	  if(strcmp(hg->fcdb[i_list].sp,"0")!=0){
	    i_list++;
	  }
	  i_all++;
	}
      }
    }
    else if (vtabledata_move->colomn == columns[1]){
      hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // u
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<-900) hg->fcdb[i_list].u=+100;
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ // g
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  // r
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].r<-900) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  // i
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].i<-900) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){  // z
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].j<-900) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  // redshift
      if(vtabledata_move->value){
	if(strcmp((const char *)vtabledata_move->value,"0")==0){
	  hg->fcdb[i_list].nedz=-100;
	}
	else{
	  hg->fcdb[i_list].nedz=atof((const char*)vtabledata_move->value);
	}
      }
      else{
	hg->fcdb[i_list].nedz=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  // class
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      if(strcmp((const char *)vtabledata_move->value,"0")==0){
	hg->fcdb[i_list].otype=g_strdup("---");
      }
      else{
	hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
      }
    }
    else if (hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_SPEC){
      if(vtabledata_move->colomn == columns[10]){
	if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
	hg->fcdb[i_list].sp=g_strdup((const char*)vtabledata_move->value);
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");

    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_sdss_band){
      case SDSS_BAND_U:
	if(hg->fcdb[i_list].u<mag){
	  mag=hg->fcdb[i_list].u;
	  i_mag=i_list;
	}
	break;
	
      case SDSS_BAND_G:
	if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	}
	break;
	
      case SDSS_BAND_R:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	}
	break;
	
      case SDSS_BAND_I:
	if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	}
	break;
	
      case SDSS_BAND_Z:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
      }
    }

    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_SDSS;
	hg->obj[hg->fcdb_i].magdb_band=hg->magdb_sdss_band;
      }
      hg->obj[hg->fcdb_i].magdb_sdss_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_sdss_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);      
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_sdss_hits=0;
      hg->obj[hg->fcdb_i].magdb_sdss_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE);      
    }
  }
}


void fcdb_usno_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"USNO-B1.0") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"B1mag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"R1mag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"B2mag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"R2mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Imag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDEC") == 0) 
      columns[9] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_usno_fil)||
	 ((hg->fcdb[i_list].r<=hg->fcdb_usno_mag)||(hg->fcdb[i_list].j<=hg->fcdb_usno_mag))){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //B1
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //R1
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  //B2
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].i)<1e-5) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  //R2
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){  //I2
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].h)<1e-5) hg->fcdb[i_list].h=+100;
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }
}



void fcdb_ucac_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"UCAC4") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Bmag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gmag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Vmag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rmag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"imag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Jmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Hmag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Kmag") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDEC") == 0) 
      columns[12] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_ucac_fil)||
	 (hg->fcdb[i_list].r<=hg->fcdb_ucac_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //B
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].b)<1e-5) hg->fcdb[i_list].b=+100;
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //g
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].u)<1e-5) hg->fcdb[i_list].u=+100;
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  //V
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  //r
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){  //i
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].i)<1e-5) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  //J
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  //H
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].h)<1e-5) hg->fcdb[i_list].h=+100;
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){  //K
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].k)<1e-5) hg->fcdb[i_list].k=+100;
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_ucac_band){
      case UCAC_BAND_B:
	if(hg->fcdb[i_list].b<mag){
	  mag=hg->fcdb[i_list].b;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_G:
	if(hg->fcdb[i_list].u<mag){
	  mag=hg->fcdb[i_list].u;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_V:
	if(hg->fcdb[i_list].v<mag){
	  mag=hg->fcdb[i_list].v;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_R:
	if(hg->fcdb[i_list].r<mag){
	  mag=hg->fcdb[i_list].r;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_I:
	if(hg->fcdb[i_list].i<mag){
	  mag=hg->fcdb[i_list].i;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_J:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_H:
	if(hg->fcdb[i_list].h<mag){
	  mag=hg->fcdb[i_list].h;
	  i_mag=i_list;
	}
	break;
	
      case UCAC_BAND_K:
	if(hg->fcdb[i_list].k<mag){
	  mag=hg->fcdb[i_list].k;
	  i_mag=i_list;
	}
	break;
      }
    }
    
    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_UCAC;
	hg->obj[hg->fcdb_i].magdb_band=hg->magdb_ucac_band;
      }
      hg->obj[hg->fcdb_i].magdb_ucac_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_ucac_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);      
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_ucac_hits=0;
      hg->obj[hg->fcdb_i].magdb_ucac_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE); 
    }
  }
}


void fcdb_gaia_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Source") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_ICRS") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DE_ICRS") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Gmag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Plx") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"e_Plx") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDE") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BPmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RPmag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RV") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Teff") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"AG") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"E(BP-RP)") == 0) 
      columns[13] = vfield_move->position;
  }
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      if((!hg->fcdb_gaia_fil)||(hg->fcdb[i_list].v<=hg->fcdb_gaia_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //G
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //Parallax
      if(vtabledata_move->value){
	gdouble au_pc=206264.806247;  // AU/pc

	hg->fcdb[i_list].plx=atof((const char*)vtabledata_move->value);
	hg->fcdb[i_list].h=1.0/au_pc
	  /(hg->fcdb[i_list].plx/1000.0/60.0/60.0*M_PI/180.0)/1000.0;
      }
      else{
	hg->fcdb[i_list].plx=-1;
	hg->fcdb[i_list].h=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  //e_Parallax
      if(vtabledata_move->value){
	hg->fcdb[i_list].eplx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].eplx=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  //BP
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].b)<1e-5) hg->fcdb[i_list].b=+100;
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  //RP
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){  // RV
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=-99999;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){ // Teff
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){  //AG
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){  //E(BP-RP)
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=-1;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb[i_list].v<mag){
	mag=hg->fcdb[i_list].v;
	i_mag=i_list;
      }
    }
    
    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_GAIA;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_gaia_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_gaia_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);      
      if(hg->magdb_pm){
	hg->obj[hg->fcdb_i].pm_ra=hg->fcdb[i_mag].pmra;
	hg->obj[hg->fcdb_i].pm_dec=hg->fcdb[i_mag].pmdec;
      }
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_gaia_hits=0;
      hg->obj[hg->fcdb_i].magdb_gaia_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE);      
    }
  }
}


void fcdb_ircs_gaia_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble yrs;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Source") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_ICRS") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DE_ICRS") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Gmag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Plx") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"e_Plx") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDE") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BPmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RPmag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RV") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Teff") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"AG") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"E(BP-RP)") == 0) 
      columns[13] = vfield_move->position;
  }
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //G  (.v --> .r)
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //Parallax
      if(vtabledata_move->value){
	gdouble au_pc=206264.806247;  // AU/pc

	hg->fcdb[i_list].plx=atof((const char*)vtabledata_move->value);
	hg->fcdb[i_list].h=1.0/au_pc
	  /(hg->fcdb[i_list].plx/1000.0/60.0/60.0*M_PI/180.0)/1000.0;
      }
      else{
	hg->fcdb[i_list].plx=-1;
	hg->fcdb[i_list].h=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  //e_Parallax
      if(vtabledata_move->value){
	hg->fcdb[i_list].eplx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].eplx=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  //RP
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  //BP
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].b)<1e-5) hg->fcdb[i_list].b=+100;
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){  // RV
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=-99999;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){ // Teff
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){  //AG
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){  //E(BP-RP)
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=-1;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    if(hg->fcdb[i_list].pm){
      yrs=current_yrs(hg)-15.5;
      hg->fcdb[i_list].d_ra=hg->fcdb[i_list].d_ra+
	hg->fcdb[i_list].pmra/1000/60/60*yrs;
      hg->fcdb[i_list].d_dec=hg->fcdb[i_list].d_dec+
	hg->fcdb[i_list].pmdec/1000/60/60*yrs;
    }
      
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }

  ircs_gs_selection(hg, MAGDB_TYPE_GAIA, 0);
}


void fcdb_hds_gaia_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble yrs;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Source") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_ICRS") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DE_ICRS") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Gmag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Plx") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"e_Plx") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDE") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BPmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RPmag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RV") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Teff") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"AG") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"E(BP-RP)") == 0) 
      columns[13] = vfield_move->position;
  }
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //G  (.v --> .r)
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //Parallax
      if(vtabledata_move->value){
	gdouble au_pc=206264.806247;  // AU/pc

	hg->fcdb[i_list].plx=atof((const char*)vtabledata_move->value);
	hg->fcdb[i_list].h=1.0/au_pc
	  /(hg->fcdb[i_list].plx/1000.0/60.0/60.0*M_PI/180.0)/1000.0;
      }
      else{
	hg->fcdb[i_list].plx=-1;
	hg->fcdb[i_list].h=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  //e_Parallax
      if(vtabledata_move->value){
	hg->fcdb[i_list].eplx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].eplx=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  //RP
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  //BP
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].b)<1e-5) hg->fcdb[i_list].b=+100;
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){  // RV
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=-99999;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){ // Teff
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){  //AG
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){  //E(BP-RP)
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=-1;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    if(hg->fcdb[i_list].pm){
      yrs=current_yrs(hg)-15.5;
      hg->fcdb[i_list].d_ra=hg->fcdb[i_list].d_ra+
	hg->fcdb[i_list].pmra/1000/60/60*yrs;
      hg->fcdb[i_list].d_dec=hg->fcdb[i_list].d_dec+
	hg->fcdb[i_list].pmdec/1000/60/60*yrs;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }

  hds_sv_mode_selection(hg);
}


void fcdb_2mass_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble mag;
  gint i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"designation") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"j_m") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"h_m") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"k_m") == 0) 
      columns[5] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      if((!hg->fcdb_2mass_fil)||(hg->fcdb[i_list].h<=hg->fcdb_2mass_mag)){
	i_list++;
      }
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      switch(hg->magdb_2mass_band){
      case TWOMASS_BAND_J:
	if(hg->fcdb[i_list].j<mag){
	  mag=hg->fcdb[i_list].j;
	  i_mag=i_list;
	}
	break;
	
      case TWOMASS_BAND_H:
	if(hg->fcdb[i_list].h<mag){
	  mag=hg->fcdb[i_list].h;
	  i_mag=i_list;
	}
	break;
	
      case TWOMASS_BAND_K:
	if(hg->fcdb[i_list].k<mag){
	  mag=hg->fcdb[i_list].k;
	  i_mag=i_list;
	}
	break;
      }
    }
    
    
    if(mag<99){  // There is a magnitude to be used
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){  
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_2MASS;
	hg->obj[hg->fcdb_i].magdb_band=hg->magdb_2mass_band;
      }
      hg->obj[hg->fcdb_i].magdb_2mass_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_2mass_sep=hg->fcdb[i_mag].sep;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);           
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].mag=100;
	hg->obj[hg->fcdb_i].magdb_used=0;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_2mass_hits=0;
      hg->obj[hg->fcdb_i].magdb_2mass_sep=-1;
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE);      
    }
  }

  if(hg->obj[hg->fcdb_i].magj>99)
    hg->obj[hg->fcdb_i].magj         =hg->obj[hg->fcdb_i].magdb_2mass_j;
  if(hg->obj[hg->fcdb_i].magh>99)
    hg->obj[hg->fcdb_i].magh         =hg->obj[hg->fcdb_i].magdb_2mass_h;
  if(hg->obj[hg->fcdb_i].magk>99)
    hg->obj[hg->fcdb_i].magk         =hg->obj[hg->fcdb_i].magdb_2mass_k;
}


void fcdb_wise_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"WISE") == 0)
      columns[0] = vfield_move->position;
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W1mag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W2mag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W3mag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W4mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Jmag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Hmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Kmag") == 0) 
      columns[9] = vfield_move->position;
  }
  

  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      if((!hg->fcdb_wise_fil)||(hg->fcdb[i_list].u<=hg->fcdb_wise_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_irc_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S09") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S09") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S18") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S18") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[6] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=0;
      }
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=0;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_fis_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S65") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S65") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S90") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S90") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S140") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S140") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S160") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S160") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[10] = vfield_move->position;
  }

  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_list++;
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].u=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=0;
      }
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=0;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=0;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].h=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=0;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[10]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_lamost_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble sep;
  int i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  switch(hg->fcdb_lamost_dr){
  case FCDB_LAMOST_DR5:
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_designation") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"stellar_teff") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"stellar_logg") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"stellar_feh") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"stellar_rv") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_class") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_subclass") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"catalogue_obsid") == 0) 
	columns[9] = vfield_move->position;
    }
    break;

  case FCDB_LAMOST_DR7:
  case FCDB_LAMOST_DR8:
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_designation") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_teff") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_logg") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_feh") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_rv") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_class") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_subclass") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"combined_obsid") == 0) 
	columns[9] = vfield_move->position;
    }
    break;

  case FCDB_LAMOST_DR7M:
  case FCDB_LAMOST_DR8M:
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_designation") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_teff_lasp") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_logg_lasp") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_feh_lasp") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_rv_b0") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_class") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_subclass") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"med_combined_obsid") == 0) 
	columns[9] = vfield_move->position;
    }
    break;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // Teff
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ // log g
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].b<-10) hg->fcdb[i_list].b=-10;
      }
      else{
	hg->fcdb[i_list].b=-10;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){ // [Fe/H]
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  // HRV
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=-99999;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[8]){
      if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
      hg->fcdb[i_list].sp=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[9]){  // ObsID
      if(vtabledata_move->value){
	hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].ref=0;
      }
      i_all++;
      i_list++;
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    switch(hg->fcdb_lamost_dr){
    case FCDB_LAMOST_DR7M:
    case FCDB_LAMOST_DR8M:
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      hg->fcdb[i_list].otype=NULL;
      if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
      hg->fcdb[i_list].sp=NULL;
      break;

    default:
      break;
    }
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].sp) hg->fcdb[i_list].sp=g_strdup("---");

    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }


  if(magextract){
    sep=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb[i_list].sep<sep){
	sep=hg->fcdb[i_list].sep;
	i_mag=i_list;
      }
    }

    if(hg->fcdb_i_max!=0){
      hg->obj[hg->fcdb_i].magdb_lamost_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_lamost_sep=hg->fcdb[i_mag].sep;
      hg->obj[hg->fcdb_i].magdb_lamost_ref=hg->fcdb[i_mag].ref;
      hg->obj[hg->fcdb_i].magdb_lamost_teff=hg->fcdb[i_mag].u;
      hg->obj[hg->fcdb_i].magdb_lamost_logg=hg->fcdb[i_mag].b;
      hg->obj[hg->fcdb_i].magdb_lamost_feh=hg->fcdb[i_mag].v;
      hg->obj[hg->fcdb_i].magdb_lamost_hrv=hg->fcdb[i_mag].r;
      if(hg->obj[hg->fcdb_i].magdb_lamost_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_name);
      hg->obj[hg->fcdb_i].magdb_lamost_name=g_strdup(hg->fcdb[i_mag].name);
      if(hg->obj[hg->fcdb_i].magdb_lamost_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_type);
      hg->obj[hg->fcdb_i].magdb_lamost_type=g_strdup(hg->fcdb[i_mag].otype);
      if(hg->obj[hg->fcdb_i].magdb_lamost_sp) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_sp);
      hg->obj[hg->fcdb_i].magdb_lamost_sp=g_strdup(hg->fcdb[i_mag].sp);
    }
    else{
      hg->obj[hg->fcdb_i].magdb_lamost_hits=0;
      hg->obj[hg->fcdb_i].magdb_lamost_sep=-1;
      hg->obj[hg->fcdb_i].magdb_lamost_ref=0;
      hg->obj[hg->fcdb_i].magdb_lamost_teff=-1;
      hg->obj[hg->fcdb_i].magdb_lamost_logg=-10;
      hg->obj[hg->fcdb_i].magdb_lamost_feh=100;
      hg->obj[hg->fcdb_i].magdb_lamost_hrv=-99999;
      if(hg->obj[hg->fcdb_i].magdb_lamost_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_name);
      hg->obj[hg->fcdb_i].magdb_lamost_name=NULL;
      if(hg->obj[hg->fcdb_i].magdb_lamost_type) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_type);
      hg->obj[hg->fcdb_i].magdb_lamost_type=NULL;
      if(hg->obj[hg->fcdb_i].magdb_lamost_sp) 
	g_free(hg->obj[hg->fcdb_i].magdb_lamost_sp);
      hg->obj[hg->fcdb_i].magdb_lamost_sp=NULL;
    }
  }
}


void fcdb_kepler_vo_parse(typHOE *hg, gboolean magextract) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gdouble sep;
  gdouble mag;
  int i_mag;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Kepler ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA (J2000)") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dec (J2000)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"R Mag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"J Mag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"KEP Mag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Teff") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Log G") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Metallicity") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"E(B-V)") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Radius") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Total PM") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"G-R color") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"2MASS ID") == 0) 
      columns[13] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_all++;
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // r Mag
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ // J Mag
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){ // Kp Mag
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){ // Teff
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){ // log g
      if(vtabledata_move->value){
	hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].h<-10) hg->fcdb[i_list].h=-10;
      }
      else{
	hg->fcdb[i_list].h=-10;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){ // [Fe/H]
      if(vtabledata_move->value){
	hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].b=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  // E(B-V)
      if(vtabledata_move->value){
	hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].k=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[10]){  // Radius
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].i=-100;
      }
    }
    else if (vtabledata_move->colomn == columns[11]){  // PM
      if(vtabledata_move->value){
	hg->fcdb[i_list].plx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].plx=-10000;
      }
    }
    else if (vtabledata_move->colomn == columns[12]){  // g-r
      if(vtabledata_move->value){
	hg->fcdb[i_list].eplx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].eplx=100;
      }
    }
    else if (vtabledata_move->colomn == columns[13]){  // 2MASS ID
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");


  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");

    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  if(magextract){
    mag=+100;
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb[i_list].v<mag){
	mag=hg->fcdb[i_list].v;
	i_mag=i_list;
      }
    }
    
    if(mag<99){
      if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){
	hg->obj[hg->fcdb_i].mag=mag;
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_KEPLER;
	hg->obj[hg->fcdb_i].magdb_band=0;
      }
      hg->obj[hg->fcdb_i].magdb_kepler_hits=hg->fcdb_i_max;
      hg->obj[hg->fcdb_i].magdb_kepler_sep=hg->fcdb[i_mag].sep;
      if(hg->obj[hg->fcdb_i].magdb_kepler_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_kepler_name);
      hg->obj[hg->fcdb_i].magdb_kepler_name=g_strdup(hg->fcdb[i_mag].name);
      if(hg->obj[hg->fcdb_i].magdb_kepler_2mass) 
	g_free(hg->obj[hg->fcdb_i].magdb_kepler_2mass);
      hg->obj[hg->fcdb_i].magdb_kepler_2mass=g_strdup(hg->fcdb[i_mag].otype);
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, TRUE);      
    }
    else{
      if(hg->magdb_ow){
	hg->obj[hg->fcdb_i].magdb_kepler_hits=0;
	hg->obj[hg->fcdb_i].magdb_kepler_sep=-1;
	hg->obj[hg->fcdb_i].magdb_kepler_k=100;
      }
      magdb_mag_copy(hg, hg->fcdb_i, i_mag, hg->fcdb_type, FALSE);      
      if(hg->obj[hg->fcdb_i].magdb_kepler_name) 
	g_free(hg->obj[hg->fcdb_i].magdb_kepler_name);
      hg->obj[hg->fcdb_i].magdb_kepler_name=NULL;
      if(hg->obj[hg->fcdb_i].magdb_kepler_2mass) 
	g_free(hg->obj[hg->fcdb_i].magdb_kepler_2mass);
      hg->obj[hg->fcdb_i].magdb_kepler_2mass=NULL;
    }
  }
}


void fcdb_smoka_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *buf_tmp1=NULL, *buf_tmp2=NULL;
  int i_list=0, i_all=0;
  gint pos_fid,pos_date,pos_mode,pos_type,pos_obj,pos_fil,pos_wv,
    pos_ra,pos_dec,pos_exp,pos_obs;
  gint len_fid,len_date,len_mode,len_type,len_obj,len_fil,len_wv,
    len_ra,len_dec,len_exp,len_obs;
  struct lnh_equ_posn equ;


  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
      fclose(fp);
      return;
    }
    else{
      if(strncmp(buf,"<pre>",strlen("<pre>"))==0){
	if(buf) g_free(buf);
	break;
      }
      else{
	if(buf) g_free(buf);
      }
    }
  }

  if((buf=fgets_new(fp))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be read.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    fclose(fp);
    return;
  }
  else if(strncmp(buf,"FRAMEID",strlen("FRAMEID"))==0){
    cpp=buf;
    pos_fid=0;
    if((cp = strstr(cpp, "DATE_OBS")) != NULL){
      len_fid=strlen(cpp)-strlen(cp);
      pos_date=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FITS_SIZE")) != NULL){
      len_date=strlen(cpp)-strlen(cp)-pos_date;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBS_MODE")) != NULL){
      pos_mode=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DATA_TYPE")) != NULL){
      len_mode=strlen(cpp)-strlen(cp)-pos_mode;
      pos_type=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBJECT")) != NULL){
      len_type=strlen(cpp)-strlen(cp)-pos_type;
      pos_obj=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FILTER")) != NULL){
      len_obj=strlen(cpp)-strlen(cp)-pos_obj;
      pos_fil=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "WVLEN")) != NULL){
      len_fil=strlen(cpp)-strlen(cp)-pos_fil;
      pos_wv=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DISPERSER")) != NULL){
      len_wv=strlen(cpp)-strlen(cp)-pos_wv;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "RA2000")) != NULL){
      pos_ra=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DEC2000")) != NULL){
      len_ra=strlen(cpp)-strlen(cp)-pos_ra;
      pos_dec=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "GALLONG")) != NULL){
      len_dec=strlen(cpp)-strlen(cp)-pos_dec;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXPTIME")) != NULL){
      pos_exp=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBSERVER")) != NULL){
      len_exp=strlen(cpp)-strlen(cp)-pos_exp;
      pos_obs=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXP_ID")) != NULL){
      len_obs=strlen(cpp)-strlen(cp)-pos_obs;
    }

    if(buf) g_free(buf);
  }
  else{
    if(buf) g_free(buf);
    fclose(fp);

    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;

    return;
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
      fclose(fp);
      return;
    }
    else if(strncmp(buf,"</pre>",strlen("</pre>"))==0){
      if(buf) g_free(buf);
      break;
    }
    else if(strlen(buf)>=(pos_obs+len_obs)){  // Table parse

      // FRAMEID
      cp=buf;
      cp+=pos_fid;
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strstrip(g_strndup(cp,len_fid));

      // DATE_OBS
      cp=buf;
      cp+=pos_date;
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strstrip(g_strndup(cp,len_date));

      // OBS_MODE
      cp=buf;
      cp+=pos_mode;
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strstrip(g_strndup(cp,len_mode));

      // DATA_TYPE
      cp=buf;
      cp+=pos_type;
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strstrip(g_strndup(cp,len_type));

      // OBJECT
      cp=buf;
      cp+=pos_obj;
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strstrip(g_strndup(cp,len_obj));

      // FILTER
      cp=buf;
      cp+=pos_fil;
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strstrip(g_strndup(cp,len_fil));

      // WV_LEN
      cp=buf;
      cp+=pos_wv;
      if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
      hg->fcdb[i_list].wv=rm_spc(g_strndup(cp,len_wv));

      // RA2000
      cp=buf;
      cp+=pos_ra;
      buf_tmp1=g_strstrip(g_strndup(cp,len_ra));
      cpp=buf_tmp1;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.hours=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-6);
      equ.ra.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_ra=ln_hms_to_deg(&equ.ra);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);

      
      // DEC2000
      cp=buf;
      cp+=pos_dec;
      buf_tmp1=g_strstrip(g_strndup(cp,len_dec));
      cpp=buf_tmp1;
      if(cpp[0]==0x2d){
	equ.dec.neg=1;
      }
      else{
	equ.dec.neg=0;
      }
      cpp+=1;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.degrees=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-7);
      equ.dec.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_dec=ln_dms_to_deg(&equ.dec);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
      
      // EXPTIME
      cp=buf;
      cp+=pos_exp;
      buf_tmp1=g_strstrip(g_strndup(cp,len_exp));
      hg->fcdb[i_list].u=(gdouble)g_strtod(buf_tmp1,NULL);
      if(buf_tmp1) g_free(buf_tmp1);

      // OBSERVER
      cp=buf;
      cp+=pos_obs;
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strstrip(g_strndup(cp,len_obs));

      if(buf) g_free(buf);
      i_list++;
      if(i_list==MAX_FCDB) break;
    }
  }  
  i_all=i_list;
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  fclose(fp);

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  return;
}

gboolean trdb_smoka_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *buf_tmp1=NULL, *buf_tmp2=NULL;
  int i_list=0, i_all=0;
  gint pos_fid,pos_date,pos_mode,pos_type,pos_obj,pos_fil,pos_wv,
    pos_ra,pos_dec,pos_exp,pos_obs;
  gint len_fid,len_date,len_mode,len_type,len_obj,len_fil,len_wv,
    len_ra,len_dec,len_exp,len_obs;
  struct lnh_equ_posn equ;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return(FALSE);
  }
  
  hg->obj[hg->fcdb_i].trdb_band_max=0;

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
      fclose(fp);      
      return(FALSE);
    }
    else{
      if(strncmp(buf,"<pre>",strlen("<pre>"))==0){
	if(buf) g_free(buf);
	break;
      }
      else{
	if(buf) g_free(buf);
      }
    }
  }

  if((buf=fgets_new(fp))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be read.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    fclose(fp);
    return(FALSE);
  }
  else if(strncmp(buf,"FRAMEID",strlen("FRAMEID"))==0){ // Header parse
    cpp=buf;
    pos_fid=0;
    if((cp = strstr(cpp, "DATE_OBS")) != NULL){
      len_fid=strlen(cpp)-strlen(cp);
      pos_date=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FITS_SIZE")) != NULL){
      len_date=strlen(cpp)-strlen(cp)-pos_date;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBS_MODE")) != NULL){
      pos_mode=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DATA_TYPE")) != NULL){
      len_mode=strlen(cpp)-strlen(cp)-pos_mode;
      pos_type=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBJECT")) != NULL){
      len_type=strlen(cpp)-strlen(cp)-pos_type;
      pos_obj=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FILTER")) != NULL){
      len_obj=strlen(cpp)-strlen(cp)-pos_obj;
      pos_fil=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "WVLEN")) != NULL){
      len_fil=strlen(cpp)-strlen(cp)-pos_fil;
      pos_wv=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DISPERSER")) != NULL){
      len_wv=strlen(cpp)-strlen(cp)-pos_wv;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "RA2000")) != NULL){
      pos_ra=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DEC2000")) != NULL){
      len_ra=strlen(cpp)-strlen(cp)-pos_ra;
      pos_dec=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "GALLONG")) != NULL){
      len_dec=strlen(cpp)-strlen(cp)-pos_dec;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXPTIME")) != NULL){
      pos_exp=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBSERVER")) != NULL){
      len_exp=strlen(cpp)-strlen(cp)-pos_exp;
      pos_obs=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXP_ID")) != NULL){
      len_obs=strlen(cpp)-strlen(cp)-pos_obs;
    }

    if(buf) g_free(buf);
  }
  else{
    // No data
    if(buf) g_free(buf);
    fclose(fp);

    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    hg->obj[hg->fcdb_i].trdb_band_max=0;
    if(hg->obj[hg->fcdb_i].trdb_str) g_free(hg->obj[hg->fcdb_i].trdb_str);
    hg->obj[hg->fcdb_i].trdb_str=NULL;

    return(TRUE);
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else if(strncmp(buf,"</pre>",strlen("</pre>"))==0){
      if(buf) g_free(buf);
      break;
    }
    else if(strlen(buf)>=(pos_obs+len_obs)){  // Table parse
      // FRAMEID
      cp=buf;
      cp+=pos_fid;
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strstrip(g_strndup(cp,len_fid));

      // DATE_OBS
      cp=buf;
      cp+=pos_date;
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strstrip(g_strndup(cp,len_date));

      // OBS_MODE
      cp=buf;
      cp+=pos_mode;
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strstrip(g_strndup(cp,len_mode));

      // DATA_TYPE
      cp=buf;
      cp+=pos_type;
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strstrip(g_strndup(cp,len_type));

      // OBJECT
      cp=buf;
      cp+=pos_obj;
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strstrip(g_strndup(cp,len_obj));

      // FILTER
      cp=buf;
      cp+=pos_fil;
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strstrip(g_strndup(cp,len_fil));

      // WV_LEN
      cp=buf;
      cp+=pos_wv;
      if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
      hg->fcdb[i_list].wv=rm_spc(g_strndup(cp,len_wv));

      // RA2000
      cp=buf;
      cp+=pos_ra;
      buf_tmp1=g_strstrip(g_strndup(cp,len_ra));
      cpp=buf_tmp1;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.hours=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-6);
      equ.ra.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_ra=ln_hms_to_deg(&equ.ra);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);

      
      // DEC2000
      cp=buf;
      cp+=pos_dec;
      buf_tmp1=g_strstrip(g_strndup(cp,len_dec));
      cpp=buf_tmp1;
      if(cpp[0]==0x2d){
	equ.dec.neg=1;
      }
      else{
	equ.dec.neg=0;
      }
      cpp+=1;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.degrees=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-7);
      equ.dec.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_dec=ln_dms_to_deg(&equ.dec);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
      
      // EXPTIME
      cp=buf;
      cp+=pos_exp;
      buf_tmp1=g_strstrip(g_strndup(cp,len_exp));
      hg->fcdb[i_list].u=(gdouble)g_strtod(buf_tmp1,NULL);
      if(buf_tmp1) g_free(buf_tmp1);

      // OBSERVER
      cp=buf;
      cp+=pos_obs;
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strstrip(g_strndup(cp,len_obs));

      if(buf) g_free(buf);
      i_list++;
      if(i_list==MAX_FCDB) break;
    }
  }  
  i_all=i_list;
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  fclose(fp);

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){

      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	// HSC or SupCam (Imag only)
	// Imaging  (Mode:Ignore)
	if(strcmp(hg->fcdb[i_list].fil,
		  hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	  if(strcmp(hg->fcdb[i_list].type, "FOCUSING")!=0){
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	  }
	}
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	// HDS or FMOS (Spec only)
	// Spec.  (Mode:Ignore)
	if(strcmp(hg->fcdb[i_list].wv,
		  hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	  hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	  flag_band=TRUE;
	}
      }
      else{
	// Other Inst.
	if(strcmp(hg->fcdb[i_list].mode,
		  hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	  if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				 "imag",strlen("imag"))==0){ 
	    // Mode==Imag Band->Fil
	    if(strcmp(hg->fcdb[i_list].fil,
		      hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	      hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	      hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	      flag_band=TRUE;
	      break;
	    }
	  }
	  else if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				      "slitview",strlen("slitview"))!=0){
	    if(strcmp(hg->fcdb[i_list].wv,
		      hg->obj[hg->fcdb_i].trdb_band[i_band])==0){ 
	      // Mode!=Imag Band->Wv
	      hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	      hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	      flag_band=TRUE;
	      break;
	    }
	  }
	}
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	// HSC or SupCam (Imag only)
	if(strcmp(hg->fcdb[i_list].type, "FOCUSING")!=0){
	  if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	    g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	  hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	    =g_strdup("IMAG");

	  if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	    g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].fil);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	// HDS or FMOS (Spec only)
	if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	  =g_strdup("SPEC");

	if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup(hg->fcdb[i_list].wv);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
      else{
	// Other Inst.
	if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	  =g_strdup(hg->fcdb[i_list].mode);
	
	if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band_max],
			       "imag",strlen("imag"))==0){
	  // Mode==Imag Band->Fil
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].fil);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
	else if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				    "slitview",strlen("slitview"))!=0){
	  // Mode!=Imag Band->Wv.
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].wv);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
      
      }
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_SMOKA);

  return(TRUE);
}




void fcdb_hst_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dataset") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Target Name") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA (J2000)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dec (J2000)") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Start Time") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Exp Time") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Instrument") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Filters/Gratings") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Central Wavelength") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Proposal ID") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Apertures") == 0) 
      columns[10] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
      i_all++;
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[4]){  // StartDate
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[5]){ // ExpTime
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  // Instrument
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[7]){  // Filters
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[8]){ // Central Wv
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<0) hg->fcdb[i_list].v=-1;
      }
      else{
	hg->fcdb[i_list].v=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  // Prop ID
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[10]){  // Apertures
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

}

gboolean trdb_hst_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return(FALSE);
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dataset") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Target Name") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA (J2000)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dec (J2000)") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Start Time") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Exp Time") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Instrument") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Filters/Gratings") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Central Wavelength") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Proposal ID") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Apertures") == 0) 
      columns[10] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
      i_all++;
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[4]){  // StartDate
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[5]){ // ExpTime
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  // Instrument
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[7]){  // Filters
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[8]){ // Central Wv
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<0) hg->fcdb[i_list].v=-1;
      }
      else{
	hg->fcdb[i_list].v=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  // Prop ID
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[10]){  // Apertures
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){
      if(strcmp(hg->fcdb[i_list].mode,
		hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	  // Mode==Imag Band->Fil
	  if(strcmp(hg->fcdb[i_list].fil,
		    hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	    break;
	  }
	}
	else{
	  if((int)hg->fcdb[i_list].v
	     ==atoi(hg->obj[hg->fcdb_i].trdb_band[i_band])){
	    // Mode!=Imag Band->Wv
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	    break;
	  }
	}
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	=g_strdup(hg->fcdb[i_list].mode);
      
      if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
      if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	// Mode==Imag Band->Fil
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup(hg->fcdb[i_list].fil);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
      else{
	// Mode!=Imag Band->Wv.
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup_printf("%d",(int)hg->fcdb[i_list].v);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_HST);

  return(TRUE);
}

void fcdb_eso_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return;
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"object") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"exptime") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"prog_id") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_id") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_tech") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ins_id") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"data_release_date") == 0) 
      columns[8] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){  // Name
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_all++;
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){  // RA
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){  // DEC
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // ExpTime
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  // Prop ID
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[5]){  // Frame ID
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[6]){  // Mode
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[7]){ // Instrument
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[8]){  // Release Date
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  return;
}

gboolean trdb_eso_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  int i_list=0, i_all=0;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    return(FALSE);
  }

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"object") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"exptime") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"prog_id") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_id") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_tech") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ins_id") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"data_release_date") == 0) 
      columns[8] = vfield_move->position;
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){  // Name
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      i_all++;
      i_list++;
    }
    else if (vtabledata_move->colomn == columns[1]){  // RA
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_ra=0.0;
      }
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){  // DEC
      if(vtabledata_move->value){
	hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].d_dec=0.0;
      }
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // ExpTime
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
      }
      else{
	hg->fcdb[i_list].u=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  // Prop ID
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[5]){  // Frame ID
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[6]){  // Mode
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[7]){ // Instrument
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[8]){  // Release Date
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    // No trdb_band for ESO
    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){
      if(strcmp(hg->fcdb[i_list].type,
		hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	flag_band=TRUE;
	break;
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	=g_strdup(hg->fcdb[i_list].type);
      
      if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_band[i_band_max]=NULL;

      hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
      hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
      i_band_max++;
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_ESO);

  return(TRUE);
}

void addobj_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  gdouble tmp_d_ra, tmp_d_dec;

  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  if(!reader) {
    fprintf (stderr,"!!Cannot initialize xmlTextRedader!! Skipped.\n");
    return;
  }

  if(hg->addobj_voname) g_free(hg->addobj_voname);
  hg->addobj_voname=NULL;

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->w_top);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  if(hg->addobj_type==FCDB_TYPE_SIMBAD){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"OTYPE_S") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_U") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_B") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_R") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_I") == 0) 
	columns[9] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_J") == 0) 
	columns[10] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_H") == 0) 
	columns[11] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_K") == 0) 
	columns[12] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
      columns[13] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[14] = vfield_move->position;
      /*
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"OTYPE_S") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[7] = vfield_move->position;*/
    }
  }
  else if (hg->addobj_type==FCDB_TYPE_NED){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Object Name") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Type") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Magnitude and Filter") == 0) 
	columns[4] = vfield_move->position;
    }
  }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->addobj_voname) g_free(hg->addobj_voname);
      hg->addobj_voname=g_strdup((const char*)vtabledata_move->value);
      break;
    }
    else if (vtabledata_move->colomn == columns[1]){
      if(vtabledata_move->value){
	tmp_d_ra=atof((const char*)vtabledata_move->value);
      }
      else{
	tmp_d_ra=0.0;
      }
      hg->addobj_ra=deg_to_ra(tmp_d_ra);
    }
    else if (vtabledata_move->colomn == columns[2]){
      if(vtabledata_move->value){
	tmp_d_dec=atof((const char*)vtabledata_move->value);
      }
      else{
	tmp_d_dec=0.0;
      }
      hg->addobj_dec=deg_to_dec(tmp_d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){
      if(hg->addobj_votype) g_free(hg->addobj_votype);
      hg->addobj_votype=g_strdup((const char*)vtabledata_move->value);
    }
    else{
      if(hg->addobj_type==FCDB_TYPE_SIMBAD){
	if (vtabledata_move->colomn == columns[4]){
	  if(hg->addobj_vosp) g_free(hg->addobj_vosp);
	  hg->addobj_vosp=g_strdup((const char*)vtabledata_move->value);
	}
	else if (vtabledata_move->colomn == columns[5]){
	  if(vtabledata_move->value){
	    hg->addobj_u=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_u=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[6]){
	  if(vtabledata_move->value){
	    hg->addobj_b=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_b=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[7]){
	  if(vtabledata_move->value){
	    hg->addobj_v=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_v=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[8]){
	  if(vtabledata_move->value){
	    hg->addobj_r=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_r=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[9]){
	  if(vtabledata_move->value){
	    hg->addobj_i=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_i=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[10]){
	  if(vtabledata_move->value){
	    hg->addobj_j=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_j=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[11]){
	  if(vtabledata_move->value){
	    hg->addobj_h=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_h=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[12]){
	  if(vtabledata_move->value){
	    hg->addobj_k=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_k=+100;
	  }
	}
	else if (vtabledata_move->colomn == columns[13]){
	  if(vtabledata_move->value){
	    hg->addobj_pm_ra=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_pm_ra=0.0;
	  }
	}
	else if (vtabledata_move->colomn == columns[14]){
	  if(vtabledata_move->value){
	    hg->addobj_pm_dec=atof((const char*)vtabledata_move->value);
	  }
	  else{
	    hg->addobj_pm_dec=0.0;
	  }
	}
      }
      else if(hg->addobj_type==FCDB_TYPE_NED){
	if (vtabledata_move->colomn == columns[4]){
	  if(hg->addobj_magsp) g_free(hg->addobj_magsp);
	  hg->addobj_magsp=g_strdup((const char*)vtabledata_move->value);
	}
      }
    }
  }
  
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  if(!hg->addobj_votype) hg->addobj_votype=g_strdup("(type unknown)");
  
  if(hg->addobj_type==FCDB_TYPE_SIMBAD){
    if(hg->addobj_magsp) g_free(hg->addobj_magsp);
    if(hg->addobj_r<99){
      if(hg->addobj_vosp)
	hg->addobj_magsp=g_strdup_printf("R=%.2lf %s",hg->addobj_r,hg->addobj_vosp);
      else
	hg->addobj_magsp=g_strdup_printf("R=%.2lf",hg->addobj_r);
    }
    else if(hg->addobj_v<99){
      if(hg->addobj_vosp)
	hg->addobj_magsp=g_strdup_printf("V=%.2lf %s",hg->addobj_v,hg->addobj_vosp);
      else
	hg->addobj_magsp=g_strdup_printf("V=%.2lf",hg->addobj_v);
    }
    else{
      if(hg->addobj_vosp)
	hg->addobj_magsp=g_strdup_printf("R/V=unknown %s",hg->addobj_vosp);
      else
	hg->addobj_magsp=g_strdup("R/V=unknown");
    }
  }
  else if(hg->addobj_type==FCDB_TYPE_NED){
    if(!hg->addobj_magsp) hg->addobj_magsp=g_strdup("mag=unknown");
  }
}


void addobj_transient_txt_parse(typHOE *hg) {
  FILE *fp;
  gint i=0, i_name=-1, i_ra=-1, i_dec=-1, i_type=-1,
    i_z=-1, i_mag=-1, i_host=-1, i_date=-1;
  gchar *buf=NULL, *buf1=NULL, *tmp_char=NULL;
  gchar *c_name=NULL, *c_ra=NULL, *c_dec=NULL, *c_type=NULL,
    *c_z=NULL, *c_mag=NULL, *c_host=NULL, *c_date=NULL,
    *str_z=NULL, *str_mag=NULL, *str_host=NULL, *str_date=NULL;
  gint ra_h, ra_m, dec_d, dec_m;
  gdouble ra_s, dec_s;
  gboolean dec_n;
  gint n_obj=0;
  gchar *ip_name, *tgt_name;

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }

  ip_name=strip_spc(hg->addobj_name);

  // Header
  if((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,"\t");
    
    while(tmp_char!=NULL){
      if(strncmp(tmp_char,"\"Name\"",strlen("\"Name\""))==0){
	i_name=i;
      }
      else if(strncmp(tmp_char,"\"RA\"",strlen("\"RA\""))==0){
	i_ra=i;
      }
      else if(strncmp(tmp_char,"\"DEC\"",strlen("\"DEC\""))==0){
	i_dec=i;
      }
      else if(strncmp(tmp_char,"\"Obj. Type\"",strlen("\"Obj Type\""))==0){
	i_type=i;
      }
      else if(strncmp(tmp_char,"\"Host Name\"",strlen("\"Host Name\""))==0){
	i_host=i;
      }
      else if(strncmp(tmp_char,"\"Host Redshift\"",strlen("\"Host Redshift\""))==0){
	i_z=i;
      }
      else if(strncmp(tmp_char,"\"Discovery Mag\"",strlen("\"Discovery Mag\""))==0){
	i_mag=i;
      }
      else if(strncmp(tmp_char,"\"Discovery Date (UT)\"",strlen("\"Discovery Date (UT)\""))==0){
	i_date=i;
      }
      
      tmp_char=(char *)strtok(NULL,"\t");
      i++;
    }
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File is invalid.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }

  g_free(buf);

  if((i_name<0) || (i_ra<0) || (i_dec<0)){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File is invalid.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }  
  
  // Data
  buf=fgets_new(fp);
  
  while(buf){
    // Name
    buf1=g_strdup(buf);
    tmp_char=(char *)strtok(buf1,"\t");
    for(i=0;i<i_name;i++){
      tmp_char=(char *)strtok(NULL,"\t");
    }   
    if(strlen(tmp_char)>2){
      c_name=g_strndup(tmp_char+1,strlen(tmp_char)-2);
    }
    g_free(buf1);
    
    // RA
    buf1=g_strdup(buf);
    tmp_char=(char *)strtok(buf1,"\t");
    for(i=0;i<i_ra;i++){
      tmp_char=(char *)strtok(NULL,"\t");
    }
    if(strlen(tmp_char)>2){
      c_ra=g_strndup(tmp_char+1,strlen(tmp_char)-2);
    }
    g_free(buf1);
    
    // Dec
    buf1=g_strdup(buf);
    tmp_char=(char *)strtok(buf1,"\t");
    for(i=0;i<i_dec;i++){
      tmp_char=(char *)strtok(NULL,"\t");
    }
    if(strlen(tmp_char)>2){
      c_dec=g_strndup(tmp_char+1,strlen(tmp_char)-2);
    }
    g_free(buf1);
    
    // Type
    if(i_type > 0){
      buf1=g_strdup(buf);
      tmp_char=(char *)strtok(buf1,"\t");
      for(i=0;i<i_type;i++){
	tmp_char=(char *)strtok(NULL,"\t");
      }
      if(strlen(tmp_char)>2){
	c_type=g_strndup(tmp_char+1,strlen(tmp_char)-2);
      }
      g_free(buf1);
    }

    // Host
    if(i_host > 0){
      buf1=g_strdup(buf);
      tmp_char=(char *)strtok(buf1,"\t");
      for(i=0;i<i_host;i++){
	tmp_char=(char *)strtok(NULL,"\t");
      }
      if(strlen(tmp_char)>2){
	c_host=g_strndup(tmp_char+1,strlen(tmp_char)-2);
      }
      g_free(buf1);
    }
    
    // Z
    if(i_z > 0){
      buf1=g_strdup(buf);
      tmp_char=(char *)strtok(buf1,"\t");
      for(i=0;i<i_z;i++){
	tmp_char=(char *)strtok(NULL,"\t");
      }
      if(strlen(tmp_char)>2){
	c_z=g_strndup(tmp_char+1,strlen(tmp_char)-2);
      }
      g_free(buf1);
    }
    
    // mag
    if(i_mag > 0){
      buf1=g_strdup(buf);
      tmp_char=(char *)strtok(buf1,"\t");
      for(i=0;i<i_mag;i++){
	tmp_char=(char *)strtok(NULL,"\t");
      }
      if(strlen(tmp_char)>2){
	c_mag=g_strndup(tmp_char+1,strlen(tmp_char)-2);
      }
      g_free(buf1);
    }
    
    // Date
    if(i_date > 0){
      buf1=g_strdup(buf);
      tmp_char=(char *)strtok(buf1,"\t");
      for(i=0;i<i_date;i++){
	tmp_char=(char *)strtok(NULL,"\t");
      }
      if(strlen(tmp_char)>2){
	c_date=g_strndup(tmp_char+1,strlen(tmp_char)-2);
      }
      g_free(buf1);
    }


    if(n_obj>0){ // found 2 or more!
      if(!c_name) return;
      tgt_name=strip_spc(c_name);
      if(strcasecmp(ip_name, tgt_name)==0){
	g_free(tgt_name);
	break;
      }
      else{
	g_free(tgt_name);
	
	g_free(c_name);
	g_free(c_ra);
	g_free(c_dec);
	g_free(c_type);
	g_free(c_host);
	g_free(c_z);
	g_free(c_date);
	g_free(c_mag);

      	c_name=NULL;
	c_ra=NULL;
	c_dec=NULL;
	c_type=NULL;
	c_host=NULL;
	c_z=NULL;
	c_date=NULL;
	c_mag=NULL;
}
    }
    n_obj++;
    buf=fgets_new(fp);
  }
 
  g_free(ip_name);
  g_free(buf);
  fclose(fp);

  
  // Name
  if(hg->addobj_voname) g_free(hg->addobj_voname);
  if(c_name){
    hg->addobj_voname=g_strdup(c_name);
    g_free(c_name);
  }
  else{
    hg->addobj_voname=NULL;
    return;
  }

  // RA
  tmp_char=(char *)strtok(c_ra,":");
  ra_h=atoi(tmp_char);
  tmp_char=(char *)strtok(NULL,":");
  ra_m=atoi(tmp_char);
  tmp_char=(char *)strtok(NULL,":");
  ra_s=atof(tmp_char);
  hg->addobj_ra=ra_h*10000+ra_m*100+ra_s;
  if(c_ra) g_free(c_ra);

  // Dec
  if(c_dec[0]=='-'){
    dec_n=TRUE;
  }
  else{
    dec_n=FALSE;
  }
  tmp_char=(char *)strtok(c_dec,":");
  dec_d=abs(atoi(tmp_char));
  tmp_char=(char *)strtok(NULL,":");
  dec_m=atoi(tmp_char);
  tmp_char=(char *)strtok(NULL,":");
  dec_s=atof(tmp_char);
  if(dec_n){
    hg->addobj_dec=-dec_d*10000-dec_m*100-dec_s;
  }
  else{
    hg->addobj_dec=dec_d*10000+dec_m*100+dec_s;
  }
  if(c_dec) g_free(c_dec);

  // Type
  if(hg->addobj_votype) g_free(hg->addobj_votype);
  if(c_type){
    hg->addobj_votype=g_strdup(c_type);
  }
  else{
    hg->addobj_votype=g_strdup("(type unknown)");
  }
  
  hg->addobj_pm_ra=0.0;
  hg->addobj_pm_dec=0.0;


  // mag
  if(c_mag){
    str_mag=g_strdup_printf("mag=%s, ",c_mag);
    g_free(c_mag);
  }
  else{
    str_mag=g_strdup(" ");
  }
  
  // Host
  if(c_host){
    str_host=g_strdup_printf("in %s, ",c_host);
    g_free(c_host);
  }
  else{
    str_host=g_strdup(" ");
  }

  // Z
  if(c_z){
    str_z=g_strdup_printf("z=%s, ",c_z);
    g_free(c_z);
  }
  else{
    str_z=g_strdup(" ");
  }

  // Date
  if(c_date){
    str_date=g_strdup_printf("%s",c_date);
    g_free(c_date);
  }
  else{
    str_date=g_strdup(" ");
  }
  
  if(hg->addobj_magsp) g_free(hg->addobj_magsp);
  hg->addobj_magsp=g_strconcat(str_mag, str_host, str_z, str_date, NULL);

  g_free(str_mag);
  g_free(str_host);
  g_free(str_z);
  g_free(str_date);
}


void camz_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gint i;

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }
  
  while((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,",");
    
    if(strncmp(tmp_char,"CamZ_B",strlen("CamZ_B"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hg->camz_b=g_strtod(tmp_p,NULL);
	gtk_adjustment_set_value(hg->camz_b_adj,hg->camz_b);
      }
    }
    if(strncmp(tmp_char,"CamZ_R",strlen("CamZ_R"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hg->camz_r=g_strtod(tmp_p,NULL);
	gtk_adjustment_set_value(hg->camz_r_adj,hg->camz_r);
      }
    }
    if(strncmp(tmp_char,"dCross",strlen("dCross"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hg->d_cross=g_strtod(tmp_p,NULL);
	gtk_adjustment_set_value(hg->d_cross_adj,hg->d_cross);
      }
    }
    if(strncmp(tmp_char,"Echelle",strlen("Echelle"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	for(i=0;i<MAX_NONSTD;i++){
	  hg->nonstd[i].echelle=g_strtod(tmp_p,NULL);
	  gtk_adjustment_set_value(hg->echelle_adj[i],hg->nonstd[i].echelle);
	}
      }
    }
    if(strncmp(tmp_char,"Date",strlen("Date"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	if(hg->camz_date) g_free(hg->camz_date);
	hg->camz_date=g_strdup_printf("<b>%s</b>", tmp_p);
	gtk_label_set_markup(GTK_LABEL(hg->camz_label),hg->camz_date);
      }
    }
  }
  fclose(fp);
}


void ircs_gs_selection(typHOE *hg, gint src, gint band){
  int i_list=0, j_list;
  gdouble tgt_ngs_mag, tgt_ttgs_mag, ngs_mag, ttgs_mag;
  gint i_ttgs=-1, i_ngs=-1, i_ngs_tgt=-1, i_ttgs_tgt=-1;
  gboolean ds_flag;
  gdouble sep, yrs;
  
  // Guide Star Selection
  {
    ngs_mag=+100;
    ttgs_mag=+100;
    tgt_ngs_mag=+100;
    tgt_ttgs_mag=+100;
    
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      // Target Itself
      if(hg->fcdb[i_list].sep*60.*60.<(gdouble)hg->ircs_magdb_r_tgt){ // with R_tgt
	if(hg->fcdb[i_list].r<hg->ircs_magdb_mag_ngs){  // NGS or LGS
	  if(hg->fcdb[i_list].r<tgt_ngs_mag){
	    if(hg->ircs_magdb_dse){  ////////////// Start of DS Exclusion //////////////
	      ds_flag=FALSE;

	      for(j_list=0;j_list<hg->fcdb_i_max;j_list++){
		if(j_list!=i_list){
		  if(hg->fcdb[j_list].r<hg->fcdb[i_list].r+hg->ircs_magdb_dse_mag){
		    sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				hg->fcdb[j_list].d_ra,hg->fcdb[j_list].d_dec)*60.*60.;
		    if((sep>hg->ircs_magdb_dse_r1)
		       &&(sep<hg->ircs_magdb_dse_r2)){
		      ds_flag=TRUE;
		      break;
		    }
		  }
		}
	      }
	      
	      if(!ds_flag){
		tgt_ngs_mag=hg->fcdb[i_list].r;
		i_ngs_tgt=i_list;
	      }
	    }   ////////////// End of DS Exclusion //////////////
	    else{
	      tgt_ngs_mag=hg->fcdb[i_list].r;
	      i_ngs_tgt=i_list;
	    }
	  }
	}
	else{  // LGS (self)
	  if(hg->fcdb[i_list].r<tgt_ttgs_mag){
	    if(hg->ircs_magdb_dse){  ////////////// Start of DS Exclusion //////////////
	      ds_flag=FALSE;

	      for(j_list=0;j_list<hg->fcdb_i_max;j_list++){
		if(j_list!=i_list){
		  if(hg->fcdb[j_list].r<hg->fcdb[i_list].r+hg->ircs_magdb_dse_mag){
		    sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				hg->fcdb[j_list].d_ra,hg->fcdb[j_list].d_dec)*60.*60.;
		    if((sep>hg->ircs_magdb_dse_r1)
		       &&(sep<hg->ircs_magdb_dse_r2)){
		      ds_flag=TRUE;
		      break;
		    }
		  }
		}
	      }
	      
	      if(!ds_flag){
		tgt_ttgs_mag=hg->fcdb[i_list].r;
		i_ttgs_tgt=i_list;
	      }
	    }   ////////////// End of DS Exclusion //////////////
	    else{
	      tgt_ttgs_mag=hg->fcdb[i_list].r;
	      i_ttgs_tgt=i_list;
	    }
	  }
	}
      }

      // NGS
      if(hg->fcdb[i_list].sep*60.*60.<(gdouble)hg->ircs_magdb_r_ngs){
	if(hg->fcdb[i_list].r<hg->ircs_magdb_mag_ngs){
	  if(hg->fcdb[i_list].r<ngs_mag){
	    if(hg->ircs_magdb_dse){  ////////////// Start of DS Exclusion //////////////
	      ds_flag=FALSE;

	      for(j_list=0;j_list<hg->fcdb_i_max;j_list++){
		if(j_list!=i_list){
		  if(hg->fcdb[j_list].r<hg->fcdb[i_list].r+hg->ircs_magdb_dse_mag){
		    sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				hg->fcdb[j_list].d_ra,hg->fcdb[j_list].d_dec)*60.*60.;
		    if((sep>hg->ircs_magdb_dse_r1)
		       &&(sep<hg->ircs_magdb_dse_r2)){
		      ds_flag=TRUE;
		      break;
		    }
		  }
		}
	      }
	      
	      if(!ds_flag){
		ngs_mag=hg->fcdb[i_list].r;
		i_ngs=i_list;
	      }
	    }   ////////////// End of DS Exclusion //////////////
	    else{
	      ngs_mag=hg->fcdb[i_list].r;
	      i_ngs=i_list;
	    }
	  }
	}
      }

      // TTGS
      if(hg->fcdb[i_list].sep*60.*60.<(gdouble)hg->ircs_magdb_r_ttgs){
	if(hg->fcdb[i_list].r<ttgs_mag){
	  if(hg->ircs_magdb_dse){  ////////////// Start of DS Exclusion //////////////
	    ds_flag=FALSE;
	    
	    for(j_list=0;j_list<hg->fcdb_i_max;j_list++){
	      if(j_list!=i_list){
		if(hg->fcdb[j_list].r<hg->fcdb[i_list].r+hg->ircs_magdb_dse_mag){
		  sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
			      hg->fcdb[j_list].d_ra,hg->fcdb[j_list].d_dec)*60.*60.;
		  if((sep>hg->ircs_magdb_dse_r1)
		     &&(sep<hg->ircs_magdb_dse_r2)){
		    ds_flag=TRUE;
		    break;
		  }
		}
	      }
	    }
	    
	    if(!ds_flag){
	      ttgs_mag=hg->fcdb[i_list].r;
	      i_ttgs=i_list;
	    }
	  }   ////////////// End of DS Exclusion //////////////
	  else{
	    ttgs_mag=hg->fcdb[i_list].r;
	    i_ttgs=i_list;
	  }
	}
      }
    }

    if(i_ngs_tgt>=0){ // Found Target = NGS
      hg->obj[hg->fcdb_i].gs.flag=FALSE;
      hg->obj[hg->fcdb_i].aomode=AOMODE_NGS_S;

      hg->obj[hg->fcdb_i].mag=tgt_ngs_mag;
      hg->obj[hg->fcdb_i].magdb_used=src;
      hg->obj[hg->fcdb_i].magdb_band=band;
    }
    else if(i_ttgs_tgt>=0){ // Found Target = TTGS
      hg->obj[hg->fcdb_i].gs.flag=FALSE;
      hg->obj[hg->fcdb_i].aomode=AOMODE_LGS_S;

      hg->obj[hg->fcdb_i].mag=tgt_ttgs_mag;
      hg->obj[hg->fcdb_i].magdb_used=src;
      hg->obj[hg->fcdb_i].magdb_band=band;
    }
    else if (i_ngs>=0){ // Found Offset NGS
      hg->obj[hg->fcdb_i].gs.flag=TRUE;
      hg->obj[hg->fcdb_i].aomode=AOMODE_NGS_O;
      if(hg->obj[hg->fcdb_i].gs.name) g_free(hg->obj[hg->fcdb_i].gs.name);
      hg->obj[hg->fcdb_i].gs.name=g_strdup(hg->fcdb[i_ngs].name);
      hg->obj[hg->fcdb_i].gs.ra=hg->fcdb[i_ngs].ra;
      hg->obj[hg->fcdb_i].gs.dec=hg->fcdb[i_ngs].dec;
      hg->obj[hg->fcdb_i].gs.equinox=hg->fcdb[i_ngs].equinox;
      hg->obj[hg->fcdb_i].gs.sep=hg->fcdb[i_ngs].sep;

      hg->obj[hg->fcdb_i].gs.mag=ngs_mag;
      hg->obj[hg->fcdb_i].gs.src=src;
    }
    else if (i_ttgs>=0){ // Found Offset TTGS
      hg->obj[hg->fcdb_i].gs.flag=TRUE;
      hg->obj[hg->fcdb_i].aomode=AOMODE_LGS_O;
      if(hg->obj[hg->fcdb_i].gs.name) g_free(hg->obj[hg->fcdb_i].gs.name);
      hg->obj[hg->fcdb_i].gs.name=g_strdup(hg->fcdb[i_ttgs].name);
      hg->obj[hg->fcdb_i].gs.ra=hg->fcdb[i_ttgs].ra;
      hg->obj[hg->fcdb_i].gs.dec=hg->fcdb[i_ttgs].dec;
      hg->obj[hg->fcdb_i].gs.equinox=hg->fcdb[i_ttgs].equinox;
      hg->obj[hg->fcdb_i].gs.sep=hg->fcdb[i_ttgs].sep;

      hg->obj[hg->fcdb_i].gs.mag=ttgs_mag;
      hg->obj[hg->fcdb_i].gs.src=src;
    }
    else{
      hg->obj[hg->fcdb_i].gs.flag=FALSE;
      hg->obj[hg->fcdb_i].aomode=AOMODE_NO;
    }
  }    
}


void hds_sv_mode_selection(typHOE *hg){
  int i_list, i_tgt=-1, i_fov=-1, i_ds=-1, i_hits=0;
  gdouble mag_tgt, mag_fov, mag_ds, sep;
  gchar *tmp;

  mag_tgt=+100;
  mag_fov=+100;
  mag_ds=+100;
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);

    // Target
    if(hg->fcdb[i_list].sep*60.*60. < hg->hds_magdb_r_tgt){
      i_hits++;
      if(hg->fcdb[i_list].r<mag_tgt){
	mag_tgt=hg->fcdb[i_list].r;
	i_tgt=i_list;
      }
    }

    // FOV star
    if(hg->fcdb[i_list].r<mag_fov){
      mag_fov=hg->fcdb[i_list].r;
      i_fov=i_list;
    }
  }

  if(i_tgt>=0){
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
		  hg->fcdb[i_tgt].d_ra,hg->fcdb[i_tgt].d_dec);
      
      // Double Star companion
      if(sep*60.*60. < hg->hds_magdb_r_ds){	
	if((i_list!=i_tgt) && (hg->fcdb[i_list].r<mag_ds)
	   && (hg->fcdb[i_list].r<mag_tgt+hg->hds_magdb_mag_ds)){
	  mag_ds=hg->fcdb[i_list].r;
	  i_ds=i_list;
	}
      }
    }
  }


 
	
  if(i_tgt>=0){
    if((hg->magdb_ow)||(fabs(hg->obj[hg->fcdb_i].mag)>99)){ // Identified Target
      hg->obj[hg->fcdb_i].mag=mag_tgt;
      switch(hg->fcdb_type){
      case MAGDB_TYPE_HDS_GSC:
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_GSC;
	hg->obj[hg->fcdb_i].magdb_band=GSC_BAND_R;
	break;
	
      case MAGDB_TYPE_HDS_GAIA:
	hg->obj[hg->fcdb_i].magdb_used=MAGDB_TYPE_GAIA;
	hg->obj[hg->fcdb_i].magdb_band=0;
	break;
      }
    }

    switch(hg->fcdb_type){
    case MAGDB_TYPE_HDS_GSC:
      hg->obj[hg->fcdb_i].magdb_gsc_hits=i_hits;
      hg->obj[hg->fcdb_i].magdb_gsc_u=hg->fcdb[i_tgt].u;
      hg->obj[hg->fcdb_i].magdb_gsc_b=hg->fcdb[i_tgt].b;
      hg->obj[hg->fcdb_i].magdb_gsc_v=hg->fcdb[i_tgt].v;
      hg->obj[hg->fcdb_i].magdb_gsc_r=hg->fcdb[i_tgt].r;
      hg->obj[hg->fcdb_i].magdb_gsc_i=hg->fcdb[i_tgt].i;
      hg->obj[hg->fcdb_i].magdb_gsc_j=hg->fcdb[i_tgt].j;
      hg->obj[hg->fcdb_i].magdb_gsc_h=hg->fcdb[i_tgt].h;
      hg->obj[hg->fcdb_i].magdb_gsc_k=hg->fcdb[i_tgt].k;
      hg->obj[hg->fcdb_i].magdb_gsc_sep=hg->fcdb[i_tgt].sep;
      break;
      
    case MAGDB_TYPE_HDS_GAIA:
      hg->obj[hg->fcdb_i].magdb_gaia_hits=i_hits;
      hg->obj[hg->fcdb_i].magdb_gaia_g=hg->fcdb[i_tgt].v;
      hg->obj[hg->fcdb_i].magdb_gaia_p=hg->fcdb[i_tgt].plx;
      hg->obj[hg->fcdb_i].magdb_gaia_ep=hg->fcdb[i_tgt].eplx;
      hg->obj[hg->fcdb_i].magdb_gaia_bp=hg->fcdb[i_tgt].b;
      hg->obj[hg->fcdb_i].magdb_gaia_rp=hg->fcdb[i_tgt].r;
      hg->obj[hg->fcdb_i].magdb_gaia_rv=hg->fcdb[i_tgt].i;
      hg->obj[hg->fcdb_i].magdb_gaia_teff=hg->fcdb[i_tgt].u;
      hg->obj[hg->fcdb_i].magdb_gaia_ag=hg->fcdb[i_tgt].j;
      hg->obj[hg->fcdb_i].magdb_gaia_dist=hg->fcdb[i_tgt].h;
      hg->obj[hg->fcdb_i].magdb_gaia_ebr=hg->fcdb[i_tgt].k;
      hg->obj[hg->fcdb_i].magdb_gaia_sep=hg->fcdb[i_tgt].sep;
      break;
    }

    if(i_ds>=0){  // Target w/Companion
      hg->obj[hg->fcdb_i].guide=SVSAFE_GUIDE;
      
      tmp=g_strdup_printf("The object \"%s\" has a close companion.",
			  hg->obj[hg->fcdb_i].name);
      popup_message(hg->plan_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    tmp,
		    "You should use <b>SV (Safe)</b> mode for this target.",
		    NULL);
      g_free(tmp);

      if(hg->obj[hg->fcdb_i].note){
	if(strncmp(hg->obj[hg->fcdb_i].note,"!!!!! ",strlen("!!!!! "))!=0){
	  tmp=g_strdup(hg->obj[hg->fcdb_i].note);
	  g_free(hg->obj[hg->fcdb_i].note);
	  switch(hg->fcdb_type){
	  case MAGDB_TYPE_HDS_GSC:
	    hg->obj[hg->fcdb_i].note=g_strconcat("!!!!! A close companion is found in GSC !!!!!  ",tmp,NULL);
	    break;
	    
	  case MAGDB_TYPE_HDS_GAIA:
	    hg->obj[hg->fcdb_i].note=g_strconcat("!!!!! A close companion is found in GAIA !!!!!  ",tmp,NULL);
	    break;
	  }
	  g_free(tmp);
	}
	
      }
      else{
	switch(hg->fcdb_type){
	case MAGDB_TYPE_HDS_GSC:
	  hg->obj[hg->fcdb_i].note=g_strdup("!!!!! A close companion is found in GSC !!!!!");
	  break;
	  
	case MAGDB_TYPE_HDS_GAIA:
	  hg->obj[hg->fcdb_i].note=g_strdup("!!!!! A close companion is found in GAIA !!!!!");
	  break;
	}
      }     
    }
    else if(i_tgt==i_fov){ // Target == the Brightest star in FoV
      if(mag_tgt < hg->hds_magdb_mag_tgt){
	hg->obj[hg->fcdb_i].guide=SV_GUIDE;
      }
      else{
      hg->obj[hg->fcdb_i].guide=SVSAFE_GUIDE;
      }
    }
    else{ // Another bright star in FoV
      hg->obj[hg->fcdb_i].guide=SVSAFE_GUIDE;
    }
  }
  else{ // couldn't identify target in the catalog
    hg->obj[hg->fcdb_i].guide=SVSAFE_GUIDE;

    tmp=g_strdup_printf("The object \"%s\" cannot be found in the catalog.",
			hg->obj[hg->fcdb_i].name);
      popup_message(hg->plan_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT,
		    tmp,
		    "You should use <b>SV (Safe)</b> mode for this target.",
		    NULL);
    g_free(tmp);

    if(hg->obj[hg->fcdb_i].note){
      if(strncmp(hg->obj[hg->fcdb_i].note,"!!!!! ",strlen("!!!!! "))!=0){
	tmp=g_strdup(hg->obj[hg->fcdb_i].note);
	g_free(hg->obj[hg->fcdb_i].note);
	switch(hg->fcdb_type){
	case MAGDB_TYPE_HDS_GSC:
	  hg->obj[hg->fcdb_i].note=g_strconcat("!!!!! No target found in GSC !!!!!  ",tmp,NULL);
	  break;
	  
	case MAGDB_TYPE_HDS_GAIA:
	  hg->obj[hg->fcdb_i].note=g_strconcat("!!!!! No target found in GAIA !!!!!  ",tmp,NULL);
	  break;
	}
	g_free(tmp);
      }
      
    }
    else{
      switch(hg->fcdb_type){
      case MAGDB_TYPE_HDS_GSC:
	hg->obj[hg->fcdb_i].note=g_strdup("!!!!! No target found in GSC !!!!!");
	break;
	
      case MAGDB_TYPE_HDS_GAIA:
	hg->obj[hg->fcdb_i].note=g_strdup("!!!!! No target found in GAIA !!!!!");
	break;
      }
    }
      
    
    if(hg->magdb_ow){
      hg->obj[hg->fcdb_i].mag=100;
      hg->obj[hg->fcdb_i].magdb_used=0;
      hg->obj[hg->fcdb_i].magdb_band=0;
    }

    switch(hg->fcdb_type){
    case MAGDB_TYPE_HDS_GSC:
      hg->obj[hg->fcdb_i].magdb_gsc_hits=0;
      hg->obj[hg->fcdb_i].magdb_gsc_u=100;
      hg->obj[hg->fcdb_i].magdb_gsc_b=100;
      hg->obj[hg->fcdb_i].magdb_gsc_v=100;
      hg->obj[hg->fcdb_i].magdb_gsc_r=100;
      hg->obj[hg->fcdb_i].magdb_gsc_i=100;
      hg->obj[hg->fcdb_i].magdb_gsc_j=100;
      hg->obj[hg->fcdb_i].magdb_gsc_h=100;
      hg->obj[hg->fcdb_i].magdb_gsc_k=100;
      hg->obj[hg->fcdb_i].magdb_gsc_sep=-1;
      break;
      
    case MAGDB_TYPE_HDS_GAIA:
      hg->obj[hg->fcdb_i].magdb_gaia_hits=0;
      hg->obj[hg->fcdb_i].magdb_gaia_g=100;
      hg->obj[hg->fcdb_i].magdb_gaia_p=-1;
      hg->obj[hg->fcdb_i].magdb_gaia_ep=-1;
      hg->obj[hg->fcdb_i].magdb_gaia_bp=100;
      hg->obj[hg->fcdb_i].magdb_gaia_rp=100;
      hg->obj[hg->fcdb_i].magdb_gaia_rv=-99999;
      hg->obj[hg->fcdb_i].magdb_gaia_teff=-1;
      hg->obj[hg->fcdb_i].magdb_gaia_ag=100;
      hg->obj[hg->fcdb_i].magdb_gaia_ebr=-1;
      hg->obj[hg->fcdb_i].magdb_gaia_dist=-1;
      hg->obj[hg->fcdb_i].magdb_gaia_sep=-1;
      break;
    }
    
  }

  hg->obj[hg->fcdb_i].sv_checked=TRUE;
}
