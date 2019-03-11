//    HOE : Subaru HDS++ OPE file Editor
//       main.h    :  Main header
//   
//                                           2003.10.23  A.Tajitsu

/////////////////// Including Header Files //////////////////
// config.h created by configure
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

// version.h created by configure
#include"version.h"

// for Gtk+
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <cairo.h>
#include <cairo-pdf.h>

// other standard headers
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <errno.h>
#include <math.h>
#include <string.h>
#include <strings.h>

// for Windows
#ifdef USE_WIN32
#include <winsock2.h>
#include <windows.h>
#include <winnt.h>
#endif

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#include<locale.h>


// for macOS menu
#ifdef USE_GTKMACINTEGRATION
#include<gtkmacintegration/gtkosxapplication.h>
#endif

// local headers
#include"configfile.h"

#include "libnova/libnova.h"
#include "resources.h"

#include "std.h"
#include "post.h"
#include "post_sdss.h"
#include "post_lamost.h"
#include "post_kepler.h"
#include "post_smoka.h"
#include "post_hst.h"
#include "post_eso.h"
#include "get_gemini.h"

#include "general_gui.h"
#include "menu.h"
#include "io_gui.h"
#include "callbacks.h"
#include "hds.h"
#include "ircs.h"
#include "lgs.h"
#include "hsc.h"

#ifdef USE_WIN32
#define USER_CONFFILE "hoe.ini"
#else
#define USER_CONFFILE ".hoe"
#endif

#define AU_IN_KM 149597870.700

#ifdef SIGRTMIN
#define SIGHSKYMON1 SIGRTMIN
#define SIGHSKYMON2 SIGRTMIN+1
#else
#define SIGHSKYMON1 SIGUSR1
#define SIGHSKYMON2 SIGUSR2
#endif


#define WWW_BROWSER "firefox"

#define DEFAULT_URL "http://www.naoj.org/Observing/Instruments/HDS/hoe/"

#define CAMZ_HOST "hds.skr.jp"
#define CAMZ_PATH "/CamZ"
#define VER_HOST "www.naoj.org"
#define VER_PATH "/Observing/Instruments/HDS/hoe/ver"


#ifdef USE_WIN32
#define DSS_URL "http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "http://%s/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML"
#define DR8_URL "http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf"
#define DR14_URL "http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf"
#define NED_URL "http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define MAST_URL "http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER"
#define MASTP_URL "https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type"
#define SPITZER_URL "http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true"
#define CASSIS_URL "http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120"
#define RAPID_URL "http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptypes<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define MIRSTD_URL "http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define SSLOC_URL "http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=HTML"
#define STD_SIMBAD_URL "http://%s/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML"
#define FCDB_NED_URL "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define FCDB_SDSS_URL "http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_URL "http://dr4.lamost.org/spectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "https://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d"
#define FCDB_SMOKA_SHOT_URL "https://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all"
#define FCDB_HST_URL "http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst&dataid=%s"
#define FCDB_ESO_URL "http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s&format=DecimDeg&tab_stat_plot=on&aladin_colour=aladin_instrument"
#define FCDB_GEMINI_URL "https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s"
#define TRDB_GEMINI_URL "https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT"
#define HASH_URL "http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d"

#elif defined(USE_OSX)
// in OSX    
//    - add  "open" at the beginning
//    - "&" --> "\\&"
#define DSS_URL "open http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick\\&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf\\&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "open http://%s/simbad/sim-coo?CooDefinedFrames=none\\&CooEqudefinox=2000\\&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf\\&submit=submit%%20query\\&Radius.unit=arcmin\\&CooEqui=2000\\&CooFrame=FK5\\&Radius=2\\&output.format=HTML"
#define DR8_URL "open http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf\\&dec=%s%d:%d:%.2lf"
#define DR14_URL "open http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf\\&dec=%s%lf"
#define NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search\\&in_csys=Equatorial\\&in_equinox=J2000.0\\&lon=%d%%3A%d%%3A%.2lf\\&lat=%s%d%%3A%d%%3A%.2lf\\&radius=2.0\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&z_constraint=Unconstrained\\&z_value1=\\&z_value2=\\&z_unit=z\\&ot_include=ANY\\&nmp_op=ANY\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=Distance+to+search+center\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define MAST_URL "open http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf\\&max_records=10\\&action=Search\\&resolver=SIMBAD\\&missions[]=EUVE\\&missions[]=WFC3-IMAGE\\&missions[]=WFPC1\\&missions[]=WFPC2\\&missions[]=FOC\\&missions[]=ACS-IMAGE\\&missions[]=UIT\\&missions[]=STIS-IMAGE\\&missions[]=COS-IMAGE\\&missions[]=GALEX\\&missions[]=XMM-OM\\&missions[]=NICMOS-IMAGE\\&missions[]=FUSE\\&missions[]=IMAPS\\&missions[]=BEFS\\&missions[]=TUES\\&missions[]=IUE\\&missions[]=COPERNICUS\\&missions[]=HUT\\&missions[]=WUPPE\\&missions[]=GHRS\\&missions[]=STIS-SPECTRUM\\&missions[]=COS-SPECTRUM\\&missions[]=WFC3-SPECTRUM\\&missions[]=ACS-SPECTRUM\\&missions[]=FOS\\&missions[]=HPOL\\&missions[]=NICMOS-SPECTRUM\\&missions[]=FGS\\&missions[]=HSP\\&missions[]=KEPLER"
#define MASTP_URL "open https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "open https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "open https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "open http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All\\&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf\\&mode=cone\\&radius=2\\&radunits=arcmin\\&range=6.25+Deg.\\&data=Data+Set+Type\\&radnum=2222\\&irsa=IRSA+Only\\&submit=Get+Inventory\\&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type\\&url=%%2Fworkspace%%2FTMP_3hX3SO_29666\\&dir=%%2Fwork%%2FTMP_3hX3SO_29666\\&snull=matches+only\\&datav=Data+Set+Type"
#define SPITZER_URL "open http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition\\&DoSearch=true\\&SearchByPosition.field.radius=0.033333333\\&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000\\&SimpleTargetPanel.field.resolvedBy=nedthensimbad\\&MoreOptions.field.prodtype=aor,pbcd\\&startIdx=0\\&pageSize=0\\&shortDesc=Position\\&isBookmarkAble=true\\&isDrillDownRoot=true\\&isSearchResult=true"
#define CASSIS_URL "open http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf\\&dec=%.6lf&radius=120"
#define RAPID_URL "open http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define MIRSTD_URL "open http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define SSLOC_URL "open http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST&output.format=HTML"
#define STD_SIMBAD_URL "open http://%s/simbad/sim-id?Ident=%s\\&NbIdent=1\\&Radius=2\\&Radius.unit=arcmin\\&submit=submit+id\\&output.format=HTML"
#define FCDB_NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s\\&extend=no\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=RA+or+Longitude\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define FCDB_SDSS_URL "open http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_URL "open http://dr4.lamost.org/spectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "open https://smoka.nao.ac.jp/info.jsp?frameid=%s\\&date_obs=%s\\&i=%d"
#define FCDB_SMOKA_SHOT_URL "open https://smoka.nao.ac.jp/fssearch?frameid=%s*\\&instruments=%s\\&obs_mod=all\\&data_typ=all\\&dispcol=default\\&diff=1000\\&action=Search\\&asciitable=table\\&obs_cat=all"
#define FCDB_HST_URL "open http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst\\&dataid=%s"
#define FCDB_ESO_URL "open http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s\\&format=DecimDeg\\&tab_stat_plot=on\\&aladin_colour=aladin_instrument"
#define FCDB_GEMINI_URL "open https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s"
#define TRDB_GEMINI_URL "open https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT"
#define HASH_URL "open http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d"

#else
// in UNIX    
//    - just add a pair of \" at the beginning and the end of each phrase.
#define DSS_URL "\"http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey\""
#define SIMBAD_URL "\"http://%s/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML\""
#define DR8_URL "\"http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf\""
#define DR14_URL "\"http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf\""
#define NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define MAST_URL "\"http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER\""
#define MASTP_URL "\"https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf\""
#define KECK_URL "\"https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single\""
#define GEMINI_URL "\"https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT\""
#define IRSA_URL "\"http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type\""
#define SPITZER_URL "\"http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true\""
#define CASSIS_URL "\"http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120\""
#define RAPID_URL "\"http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define MIRSTD_URL "\"http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define SSLOC_URL "\"http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=HTML\""
#define STD_SIMBAD_URL "\"http://%s/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML\""
#define FCDB_NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define FCDB_SDSS_URL "\"http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s\""
#define FCDB_LAMOST_URL "\"http://dr4.lamost.org/spectrum/view?obsid=%d\""
#define FCDB_SMOKA_URL "\"https://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d\""
#define FCDB_SMOKA_SHOT_URL "\"https://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all\""
#define FCDB_HST_URL "\"http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst&dataid=%s\""
#define FCDB_ESO_URL "\"http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s&format=DecimDeg&tab_stat_plot=on&aladin_colour=aladin_instrument\""
#define FCDB_GEMINI_URL "\"https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s\""
#define TRDB_GEMINI_URL "\"https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT\""
#define HASH_URL "\"http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d\""
#endif

#define DSS_ARCMIN_MIN 1
#define DSS_ARCMIN 3
#define DSS_ARCMIN_MAX 120
#define PANSTARRS_MAX_ARCMIN 25
#define DSS_PIX 1500

enum{FC_MODE_OBJ, FC_MODE_TRDB, FC_MODE_REDL, FC_MODE_PLAN};

#define FC_HOST_STSCI "archive.stsci.edu"
#define FC_PATH_STSCI "/cgi-bin/dss_search?v=%s&r=%d+%d+%lf&d=%s%d+%d+%lf&e=J2000&h=%d.0&w=%d.0&f=gif&c=none&fov=NONE&v3="
#define FC_FILE_GIF "dss.gif"
#define FC_FILE_JPEG "dss.jpg"
#define FC_FILE_HTML "dss.html"

#define FC_HOST_ESO "archive.eso.org"
#define FC_PATH_ESO "/dss/dss?ra=%d%%20%d%%20%lf&dec=%s%d%%20%d%%20%lf&mime-type=image/gif&x=%d.0&y=%d.0&Sky-Survey=%s"


#define FC_HOST_SKYVIEW "skyview.gsfc.nasa.gov"
#define FC_PATH_SKYVIEW "/current/cgi/runquery.pl?survey=%s&coordinates=J%.1lf&projection=Tan&scaling=%s&sampler=LI&lut=colortables/blue-white.bin%ssize=%lf,%lf&pixels=%d&position=%lf,%lf"

#define FC_SRC_STSCI_DSS1R "poss1_red"
#define FC_SRC_STSCI_DSS1B "poss1_blue"
#define FC_SRC_STSCI_DSS2R "poss2ukstu_red"
#define FC_SRC_STSCI_DSS2B "poss2ukstu_blue"
#define FC_SRC_STSCI_DSS2IR "poss2ukstu_ir"

#define FC_SRC_ESO_DSS1R "DSS1"
#define FC_SRC_ESO_DSS2R "DSS2-red"
#define FC_SRC_ESO_DSS2B "DSS2-blue"
#define FC_SRC_ESO_DSS2IR "DSS2-infrared"

#define FC_SRC_SKYVIEW_GALEXF "GALEX%20Far%20UV"
#define FC_SRC_SKYVIEW_GALEXN "GALEX%20Near%20UV"
#define FC_SRC_SKYVIEW_DSS1R "DSS1%20Red"
#define FC_SRC_SKYVIEW_DSS1B "DSS1%20Blue"
#define FC_SRC_SKYVIEW_DSS2R "DSS2%20Red"
#define FC_SRC_SKYVIEW_DSS2B "DSS2%20Blue"
#define FC_SRC_SKYVIEW_DSS2IR "DSS2%20IR"
#define FC_SRC_SKYVIEW_SDSSU "SDSSu"
#define FC_SRC_SKYVIEW_SDSSG "SDSSg"
#define FC_SRC_SKYVIEW_SDSSR "SDSSr"
#define FC_SRC_SKYVIEW_SDSSI "SDSSi"
#define FC_SRC_SKYVIEW_SDSSZ "SDSSz"
#define FC_SRC_SKYVIEW_2MASSJ "2MASS-J"
#define FC_SRC_SKYVIEW_2MASSH "2MASS-H"
#define FC_SRC_SKYVIEW_2MASSK "2MASS-K"
#define FC_SRC_SKYVIEW_WISE34 "WISE%203.4"
#define FC_SRC_SKYVIEW_WISE46 "WISE%204.6"
#define FC_SRC_SKYVIEW_WISE12 "WISE%2012"
#define FC_SRC_SKYVIEW_WISE22 "WISE%2022"
#define FC_SRC_SKYVIEW_AKARIN60 "AKARI%20N60"
#define FC_SRC_SKYVIEW_AKARIWS "AKARI%20WIDE-S"
#define FC_SRC_SKYVIEW_AKARIWL "AKARI%20WIDE-L"
#define FC_SRC_SKYVIEW_AKARIN160 "AKARI%20N160"
#define FC_SRC_SKYVIEW_NVSS "NVSS"

#define FC_HOST_SDSS "casjobs.sdss.org"
#define FC_PATH_SDSS "/ImgCutoutDR7/getjpeg.aspx?ra=%lf&dec=%+lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_SDSS8 "skyservice.pha.jhu.edu"
#define FC_PATH_SDSS8 "/DR8/ImgCutout/getjpeg.aspx?ra=%lf&dec=%lf&scale=%f&opt=&width=%d&height=%d&opt=%s%s&query=%s%s"
#define SDSS_SCALE 0.39612
#define FC_HOST_SDSS13 "skyserver.sdss.org"
#define FC_PATH_SDSS13 "/dr13/SkyServerWS/ImgCutout/getjpeg?TaskName=Skyserver.Chart.image&ra=%lf&dec=%lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_PANCOL "ps1images.stsci.edu"
#define FC_PATH_PANCOL "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=color&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANG "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=g&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANR "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=r&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANI "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=i&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANZ "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=z&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANY "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=y&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="

#define STDDB_PATH_SSLOC "/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_RAPID "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_MIRSTD "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_FILE_XML "simbad.xml"

#define FCDB_HOST_SIMBAD_STRASBG "simbad.u-strasbg.fr"
#define FCDB_HOST_SIMBAD_HARVARD "simbad.harvard.edu"
#define FCDB_PATH "/simbad/sim-sam?Criteria=region%%28box%%2C%lf%s%lf%%2C%+lfm%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_PATH_HSC_SIMBAD "/simbad/sim-sam?Criteria=region%%28circle%%2C%lf%s%lf%%2C%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_FILE_XML "database_fc.xml"
#define FCDB_FILE_TXT "database_fc.txt"
#define FCDB_FILE_HTML "database_fc.html"
#define FCDB_FILE_JSON "database_fc.json"

enum{ FCDB_SIMBAD_STRASBG, FCDB_SIMBAD_HARVARD } FCDBSimbad;
enum{ FCDB_VIZIER_STRASBG, FCDB_VIZIER_NAOJ, 
      FCDB_VIZIER_HARVARD } FCDBVizieR;

// Instrument
#define HDS_SLIT_MASK_ARCSEC 9.2
//micron
#define HDS_SLIT_LENGTH 10000
#define HDS_SLIT_WIDTH 500
#define HDS_PA_OFFSET (-58.4)
#define HDS_SIZE 3

#define HDS_DEF_PA 0
#define HDS_FLAT_REPEAT 10

#define FMOS_SIZE 40
#define FMOS_R_ARCMIN 30

#define SPCAM_X_ARCMIN 34
#define SPCAM_Y_ARCMIN 27
#define SPCAM_GAP_ARCSEC 14.
#define SPCAM_SIZE 40

#define HSC_R_ARCMIN 90

#define FOCAS_R_ARCMIN 6
#define FOCAS_GAP_ARCSEC 5.
#define FOCAS_SIZE 10

#define IRCS_X_ARCSEC 54.
#define IRCS_Y_ARCSEC 54.
#define IRCS_SIZE 3
#define IRCS_TTGS_ARCMIN 2

#define COMICS_X_ARCSEC 30.
#define COMICS_Y_ARCSEC 40.
#define COMICS_SIZE 3

#define MOIRCS_X_ARCMIN 4.0
#define MOIRCS_Y_ARCMIN 7.0
#define MOIRCS_GAP_ARCSEC 2.
#define MOIRCS_VIG1X_ARCSEC 29.
#define MOIRCS_VIG1Y_ARCSEC 29.
#define MOIRCS_VIG2X_ARCSEC 47.
#define MOIRCS_VIG2Y_ARCSEC 45.
#define MOIRCS_VIGR_ARCMIN 6.
#define MOIRCS_SIZE 10

#define HSC_SIZE 110

#define HOE_HTTP_ERROR_GETHOST  -1
#define HOE_HTTP_ERROR_SOCKET   -2
#define HOE_HTTP_ERROR_CONNECT  -3
#define HOE_HTTP_ERROR_TEMPFILE -4

// Sky Monitor
#if GTK_CHECK_VERSION(2,8,0)
#define USE_SKYMON
#else
#undef USE_SKYMON
#endif

#define SKYMON_WINSIZE 500

#ifdef USE_WIN32
#define SKYMON_FONT "arial 10"
#else
#define SKYMON_FONT "Suns 10"
#endif

// ポップアップメッセージ
#define GTK_MSG
// エラーポップアップのタイムアウト[sec]
#define POPUP_TIMEOUT 2


//#define VERSION "0.8.0"
#define AZEL_INTERVAL 60*1000

#ifdef USE_SKYMON
#define SKYMON_INTERVAL 200
#define SKYMON_STEP 10
#endif

#define MAX_OBJECT 5000
#define MAX_ROPE 32
#define MAX_PLAN 200
#define MAX_PP 200
#define MAX_STD 100
#define MAX_FCDB 5000
#define MAX_TRDB_BAND 100

#define DEF_EXP 600

#define DEF_SV_EXP 1000
#define DEF_SV_CALC 60
#define DEF_SV_SLITX 315.0
#define DEF_SV_SLITY 315.0
#define DEF_SV_ISX 421.0
#define DEF_SV_ISY 330.0
#define DEF_SV_IS3X 455.0
#define DEF_SV_IS3Y 348.0

#define CAMZ_B -356
#define CAMZ_R -326

#define D_CROSS 130

#define MAG_SVFILTER1 8.0
#define MAG_SVFILTER2 4.0
#define MAG_SV2SEC 13.0
#define MAG_SV3SEC 14.5
#define MAG_SV5SEC 16.0

#define BUFFSIZE 65535

#define LONGITUDE_SUBARU -155.4706 //[deg]
#define LATITUDE_SUBARU 19.8255    //[deg]
#define ALTITUDE_SUBARU 4163    //[m]
#define TZNAME_SUBARU "HST"
//#define LONGITUDE_SUBARU -(155.+28./60.+50./3600.) //[deg]
//#define LATITUDE_SUBARU (19.+49./60.+43./3600.)    //[deg]

#define TIMEZONE_SUBARU -600
#define WAVE1_SUBARU 3500   //A
#define WAVE0_SUBARU 5500   //A
#define TEMP_SUBARU 0       //C
#define PRES_SUBARU 625     //hPa

#define VEL_AZ_SUBARU 0.50
#define VEL_EL_SUBARU 0.50

#define DEF_ECHELLE 900


#define TIME_SETUP_FIELD 300
#define TIME_ACQ 60
#define TIME_FOCUS_AG 300
#define TIME_FOCUS_SV 300
#define TIME_SETUP_FULL 600
#define TIME_SETUP_EASY 120
#define TIME_SETUP_SLIT 60
#define TIME_SETUP_IS 600
#define TIME_SETUP_BIN 60
#define TIME_SETUP_COL 60
#define TIME_I2 60
#define TIME_COMP 180
#define TIME_FLAT 180


// Instruments
enum{INST_HDS,
     INST_IRCS,
     INST_HSC,
     NUM_INST};

static const gchar* inst_name_short[]={
  "HDS",
  "IRCS",
  "HSC"};

static const gchar* inst_name_long[]={
  "High Dispersion Spectrograph",
  "InfraRed Camera and Spectrograph",
  "Hyper Suprime-Cam"};


enum{ AZEL_NORMAL, AZEL_POSI, AZEL_NEGA} AZElMode;


// Image Slicer
enum{ IS_NO, IS_030X5, IS_045X3, IS_020X3} ISMode;

#define IS_FLAT_FACTOR 1.35


enum{
  PLAN_TYPE_COMMENT,
  PLAN_TYPE_OBJ,
  PLAN_TYPE_FOCUS,
  PLAN_TYPE_BIAS,
  PLAN_TYPE_FLAT,
  PLAN_TYPE_COMP,
  PLAN_TYPE_SETUP,
  PLAN_TYPE_I2,
  PLAN_TYPE_SetAzEl
};

enum{
  PLAN_CMODE_FULL,
  PLAN_CMODE_EASY,
  PLAN_CMODE_SLIT,
  PLAN_CMODE_1ST
};

enum{
  PLAN_FOCUS1,  // HDS: FocusSV,   IRCS: FocusOBE
  PLAN_FOCUS2   // HDS: FocusAG,   IRCS: LGS Calibration
};

enum{
  PLAN_OMODE_FULL,
  PLAN_OMODE_SET,
  PLAN_OMODE_GET
};

enum{
  PLAN_I2_IN,
  PLAN_I2_OUT
};

enum{
  PLAN_START_EVENING,
  PLAN_START_SPECIFIC
};

enum{
  PLAN_COMMENT_TEXT,
  PLAN_COMMENT_SUNRISE,
  PLAN_COMMENT_SUNSET
};

enum{
  SV_FILTER_NONE,
  SV_FILTER_R,
  SV_FILTER_BP530,
  SV_FILTER_ND2,
};

enum{
  NOTE_GENERAL,
  NOTE_AG,
  NOTE_HDS,
  NOTE_IRCS,
  NOTE_HSC,
  NOTE_HSCFIL,
  NOTE_OH,
  NOTE_OBJ,
  NOTE_STDDB,
  NOTE_FCDB,
  NOTE_TRDB,
  NOTE_LINE,
  NOTE_ETC,
  NUM_NOTE
};


enum
{
  COLUMN_OBJTREE_CHECK,
  COLUMN_OBJTREE_NUMBER,
  COLUMN_OBJTREE_NAME,
  COLUMN_OBJTREE_STD,
  COLUMN_OBJTREE_EXP,
  COLUMN_OBJTREE_REPEAT,
  COLUMN_OBJTREE_GS,
  COLUMN_OBJTREE_MAG,
  COLUMN_OBJTREE_MAGSRC,
  COLUMN_OBJTREE_SNR,
  COLUMN_OBJTREE_SNR_COL,
  COLUMN_OBJTREE_RA,
  COLUMN_OBJTREE_RA_COL,
  COLUMN_OBJTREE_DEC,
  COLUMN_OBJTREE_DEC_COL,
  COLUMN_OBJTREE_EQUINOX,
  COLUMN_OBJTREE_HORIZON,
  COLUMN_OBJTREE_RISE,
  COLUMN_OBJTREE_RISE_COL,
  COLUMN_OBJTREE_TRANSIT,
  COLUMN_OBJTREE_TRANSIT_COL,
  COLUMN_OBJTREE_SET,
  COLUMN_OBJTREE_SET_COL,
  COLUMN_OBJTREE_PA,
  COLUMN_OBJTREE_GUIDE,
  COLUMN_OBJTREE_AOMODE,
  COLUMN_OBJTREE_PAM,
  COLUMN_OBJTREE_ADI,
  COLUMN_OBJTREE_SETUP1,
  COLUMN_OBJTREE_SETUP2,
  COLUMN_OBJTREE_SETUP3,
  COLUMN_OBJTREE_SETUP4,
  COLUMN_OBJTREE_SETUP5,
  COLUMN_OBJTREE_HSC_MAGV,
  COLUMN_OBJTREE_HSC_MAGSEP,
  COLUMN_OBJTREE_HSC_MAG6,
  COLUMN_OBJTREE_HSC_MAG7,
  COLUMN_OBJTREE_HSC_MAG8,
  COLUMN_OBJTREE_HSC_MAG9,
  COLUMN_OBJTREE_HSC_MAG10,
  COLUMN_OBJTREE_HSC_MAG11,
  COLUMN_OBJTREE_NOTE,
  NUM_OBJTREE_COLUMNS
};


// StdTreeview
enum
{
  COLUMN_STD_NUMBER,
  COLUMN_STD_NAME,
  COLUMN_STD_RA,
  COLUMN_STD_DEC,
  COLUMN_STD_SP,
  COLUMN_STD_SEP,
  COLUMN_STD_ROT,
  COLUMN_STD_U,
  COLUMN_STD_B,
  COLUMN_STD_V,
  COLUMN_STD_R,
  COLUMN_STD_I,
  COLUMN_STD_J,
  COLUMN_STD_H,
  COLUMN_STD_K,
  NUM_COLUMN_STD
};

// FCDBTreeview
enum
{
  COLUMN_FCDB_NUMBER,
  COLUMN_FCDB_NAME,
  COLUMN_FCDB_RA,
  COLUMN_FCDB_DEC,
  COLUMN_FCDB_SEP,
  COLUMN_FCDB_OTYPE,
  COLUMN_FCDB_SP,
  COLUMN_FCDB_U,
  COLUMN_FCDB_B,
  COLUMN_FCDB_V,
  COLUMN_FCDB_R,
  COLUMN_FCDB_I,
  COLUMN_FCDB_J,
  COLUMN_FCDB_H,
  COLUMN_FCDB_K,
  COLUMN_FCDB_NEDMAG,
  COLUMN_FCDB_NEDZ,
  COLUMN_FCDB_REF,
  COLUMN_FCDB_PLX,
  COLUMN_FCDB_EPLX,
  COLUMN_FCDB_FID,
  COLUMN_FCDB_DATE,
  COLUMN_FCDB_MODE,
  COLUMN_FCDB_TYPE,
  COLUMN_FCDB_FIL,
  COLUMN_FCDB_WV,
  COLUMN_FCDB_OBS,
  NUM_COLUMN_FCDB
};


// Tree DataBase
enum
{
  COLUMN_TRDB_NUMBER,
  COLUMN_TRDB_NAME,
  COLUMN_TRDB_DATA,
  COLUMN_TRDB_GSC_HITS,
  COLUMN_TRDB_GSC_SEP,
  COLUMN_TRDB_GSC_U,
  COLUMN_TRDB_GSC_B,
  COLUMN_TRDB_GSC_V,
  COLUMN_TRDB_GSC_R,
  COLUMN_TRDB_GSC_I,
  COLUMN_TRDB_GSC_J,
  COLUMN_TRDB_GSC_H,
  COLUMN_TRDB_GSC_K,
  COLUMN_TRDB_PS1_HITS,
  COLUMN_TRDB_PS1_SEP,
  COLUMN_TRDB_PS1_G,
  COLUMN_TRDB_PS1_R,
  COLUMN_TRDB_PS1_I,
  COLUMN_TRDB_PS1_Z,
  COLUMN_TRDB_PS1_Y,
  COLUMN_TRDB_SDSS_HITS,
  COLUMN_TRDB_SDSS_SEP,
  COLUMN_TRDB_SDSS_U,
  COLUMN_TRDB_SDSS_G,
  COLUMN_TRDB_SDSS_R,
  COLUMN_TRDB_SDSS_I,
  COLUMN_TRDB_SDSS_Z,
  COLUMN_TRDB_GAIA_HITS,
  COLUMN_TRDB_GAIA_SEP,
  COLUMN_TRDB_GAIA_G,
  COLUMN_TRDB_GAIA_P,
  COLUMN_TRDB_GAIA_EP,
  COLUMN_TRDB_GAIA_RP,
  COLUMN_TRDB_GAIA_BP,
  COLUMN_TRDB_GAIA_RV,
  COLUMN_TRDB_GAIA_TEFF,
  COLUMN_TRDB_GAIA_AG,
  COLUMN_TRDB_GAIA_EBR,
  COLUMN_TRDB_GAIA_DIST,
  COLUMN_TRDB_KEPLER_HITS,
  COLUMN_TRDB_KEPLER_SEP,
  COLUMN_TRDB_KEPLER_NAME,
  COLUMN_TRDB_KEPLER_K,
  COLUMN_TRDB_KEPLER_R,
  COLUMN_TRDB_KEPLER_J,
  COLUMN_TRDB_KEPLER_TEFF,
  COLUMN_TRDB_KEPLER_LOGG,
  COLUMN_TRDB_KEPLER_FEH,
  COLUMN_TRDB_KEPLER_EBV,
  COLUMN_TRDB_KEPLER_RAD,
  COLUMN_TRDB_KEPLER_PM,
  COLUMN_TRDB_KEPLER_GR,
  COLUMN_TRDB_KEPLER_2MASS,
  COLUMN_TRDB_2MASS_HITS,
  COLUMN_TRDB_2MASS_SEP,
  COLUMN_TRDB_2MASS_J,
  COLUMN_TRDB_2MASS_H,
  COLUMN_TRDB_2MASS_K,
  COLUMN_TRDB_SIMBAD_HITS,
  COLUMN_TRDB_SIMBAD_SEP,
  COLUMN_TRDB_SIMBAD_U,
  COLUMN_TRDB_SIMBAD_B,
  COLUMN_TRDB_SIMBAD_V,
  COLUMN_TRDB_SIMBAD_R,
  COLUMN_TRDB_SIMBAD_I,
  COLUMN_TRDB_SIMBAD_J,
  COLUMN_TRDB_SIMBAD_H,
  COLUMN_TRDB_SIMBAD_K,
  COLUMN_TRDB_SIMBAD_NAME,
  COLUMN_TRDB_SIMBAD_TYPE,
  COLUMN_TRDB_SIMBAD_SP,
  COLUMN_TRDB_NED_HITS,
  COLUMN_TRDB_NED_SEP,
  COLUMN_TRDB_NED_NAME,
  COLUMN_TRDB_NED_TYPE,
  COLUMN_TRDB_NED_MAG,
  COLUMN_TRDB_NED_Z,
  COLUMN_TRDB_NED_REF,
  COLUMN_TRDB_LAMOST_HITS,
  COLUMN_TRDB_LAMOST_SEP,
  COLUMN_TRDB_LAMOST_NAME,
  COLUMN_TRDB_LAMOST_TYPE,
  COLUMN_TRDB_LAMOST_SP,
  COLUMN_TRDB_LAMOST_REF,
  COLUMN_TRDB_LAMOST_TEFF,
  COLUMN_TRDB_LAMOST_LOGG,
  COLUMN_TRDB_LAMOST_FEH,
  COLUMN_TRDB_LAMOST_HRV,
  NUM_COLUMN_TRDB
};



enum
{
  COLUMN_NUMBER_TEXT,
  NUM_NUMBER_COLUMNS
};

enum
{
  NST_TYPE_TSC,
  NST_TYPE_JPL,
};




#define PS_FILE "plot.ps"
#define PA_INPUT "pos.ip"

#define OPE_EXTENSION "ope"
#define HOE_EXTENSION "hoe"
#define LIST1_EXTENSION "list"
#define LIST2_EXTENSION "lst"
#define LIST3_EXTENSION "txt"
#define PLAN_EXTENSION "plan_txt"
#define SERVICE_EXTENSION "service_txt"
#define PROMS_EXTENSION "proms_txt"
#define LGS_EXTENSION "lgs_txt"
#define PDF_EXTENSION "pdf"
#define YAML_EXTENSION "yml"
#define CSV_EXTENSION "csv"
#define NST1_EXTENSION "dat"
#define NST2_EXTENSION "tsc"
#define NST3_EXTENSION "eph"

#define MAX_LINE 20
enum{PLOT_PSFILE, PLOT_XWIN} plot_device;
enum{MODE_EFS, MODE_FSR} ModeEFS;


// Finding Chart
// Finding Chart
enum{FC_STSCI_DSS1R, 
     FC_STSCI_DSS1B, 
     FC_STSCI_DSS2R,
     FC_STSCI_DSS2B,
     FC_STSCI_DSS2IR,
     FC_SEP1,
     FC_ESO_DSS1R,
     FC_ESO_DSS2R,
     FC_ESO_DSS2B,
     FC_ESO_DSS2IR,
     FC_SEP2,
     FC_SKYVIEW_GALEXF,
     FC_SKYVIEW_GALEXN,
     FC_SKYVIEW_DSS1R,
     FC_SKYVIEW_DSS1B,
     FC_SKYVIEW_DSS2R,
     FC_SKYVIEW_DSS2B,
     FC_SKYVIEW_DSS2IR,
     FC_SKYVIEW_SDSSU,
     FC_SKYVIEW_SDSSG,
     FC_SKYVIEW_SDSSR,
     FC_SKYVIEW_SDSSI,
     FC_SKYVIEW_SDSSZ,
     FC_SKYVIEW_2MASSJ,
     FC_SKYVIEW_2MASSH,
     FC_SKYVIEW_2MASSK,
     FC_SKYVIEW_WISE34,
     FC_SKYVIEW_WISE46,
     FC_SKYVIEW_WISE12,
     FC_SKYVIEW_WISE22,
     FC_SKYVIEW_AKARIN60,
     FC_SKYVIEW_AKARIWS,
     FC_SKYVIEW_AKARIWL,
     FC_SKYVIEW_AKARIN160,
     FC_SKYVIEW_NVSS,
     FC_SEP3,
     FC_SDSS,
     FC_SDSS13,
     FC_SEP4,
     FC_PANCOL,
     FC_PANG,
     FC_PANR,
     FC_PANI,
     FC_PANZ,
     FC_PANY,
     NUM_FC} ModeFC;


static const gchar* FC_name[]={
  "STScI: DSS1 (Red)",         // FC_STSCI_DSS1R, 
  "STScI: DSS1 (Blue)",        // FC_STSCI_DSS1B, 
  "STScI: DSS2 (Red)",         // FC_STSCI_DSS2R,
  "STScI: DSS2 (Blue)",        // FC_STSCI_DSS2B,
  "STScI: DSS2 (IR)",          // FC_STSCI_DSS2IR,
  NULL,                        // FC_SEP1,
  "ESO: DSS1 (Red)",           // FC_ESO_DSS1R,
  "ESO: DSS2 (Red)",           // FC_ESO_DSS2R,
  "ESO: DSS2 (Blue)",          // FC_ESO_DSS2B,
  "ESO: DSS2 (IR)",            // FC_ESO_DSS2IR,
  NULL,                        // FC_SEP2,
  "SkyView: GALEX (Far UV)",   // FC_SKYVIEW_GALEXF,
  "SkyView: GALEX (Near UV)",  // FC_SKYVIEW_GALEXN,
  "SkyView: DSS1 (Red)",       // FC_SKYVIEW_DSS1R,
  "SkyView: DSS1 (Blue)",      // FC_SKYVIEW_DSS1B,
  "SkyView: DSS2 (Red)",       // FC_SKYVIEW_DSS2R,
  "SkyView: DSS2 (Blue)",      // FC_SKYVIEW_DSS2B,
  "SkyView: DSS2 (IR)",        // FC_SKYVIEW_DSS2IR,
  "SkyView: SDSS (u)",         // FC_SKYVIEW_SDSSU,
  "SkyView: SDSS (g)",         // FC_SKYVIEW_SDSSG,
  "SkyView: SDSS (r)",         // FC_SKYVIEW_SDSSR,
  "SkyView: SDSS (i)",         // FC_SKYVIEW_SDSSI,
  "SkyView: SDSS (z)",         // FC_SKYVIEW_SDSSZ,
  "SkyView: 2MASS (J)",        // FC_SKYVIEW_2MASSJ,
  "SkyView: 2MASS (H)",        // FC_SKYVIEW_2MASSH,
  "SkyView: 2MASS (K)",        // FC_SKYVIEW_2MASSK,
  "SkyView: WISE (3.4um)",     // FC_SKYVIEW_WISE34,
  "SkyView: WISE (4.6um)",     // FC_SKYVIEW_WISE46,
  "SkyView: WISE (12um)",      // FC_SKYVIEW_WISE12,
  "SkyView: WISE (22um)",      // FC_SKYVIEW_WISE22,
  "SkyView: AKARI N60",        // FC_SKYVIEW_AKARIN60,
  "SkyView: AKARI WIDE-S",     // FC_SKYVIEW_AKARIWS,
  "SkyView: AKARI WIDE-L",     // FC_SKYVIEW_AKARIWL,
  "SkyView: AKARI N160",       // FC_SKYVIEW_AKARIN160,
  "SkyView: NVSS (1.4GHz)",    // FC_SKYVIEW_NVSS,
  NULL,                        // FC_SEP3,
  "SDSS DR7 (color)",          // FC_SDSS,
  "SDSS DR14 (color)",         // FC_SDSS13,
  NULL,                        // FC_SEP4,
  "PanSTARRS-1 (color)",       // FC_PANCOL,
  "PanSTARRS-1 (g)",           // FC_PANG,
  "PanSTARRS-1 (r)",           // FC_PANR,
  "PanSTARRS-1 (i)",           // FC_PANI,
  "PanSTARRS-1 (z)",           // FC_PANZ,
  "PanSTARRS-1 (y)"};          // FC_PANY


static const gchar* FC_img[]={
  "DSS (POSS1 Red)",           // FC_STSCI_DSS1R, 
  "DSS (POSS1 Blue)",          // FC_STSCI_DSS1B, 
  "DSS (POSS2 Red)",           // FC_STSCI_DSS2R,
  "DSS (POSS2 Blue)",          // FC_STSCI_DSS2B,
  "DSS (POSS2 IR)",            // FC_STSCI_DSS2IR,
  NULL,                        // FC_SEP1,
  "DSS (POSS1 Red)",           // FC_ESO_DSS1R,
  "DSS (POSS2 Red)",           // FC_ESO_DSS2R,
  "DSS (POSS2 Blue)",          // FC_ESO_DSS2B,
  "DSS (POSS2 IR)",            // FC_ESO_DSS2IR,
  NULL,                        // FC_SEP2,
  "GALEX (Far UV)",            // FC_SKYVIEW_GALEXF,
  "GALEX (Near UV)",           // FC_SKYVIEW_GALEXN,
  "DSS (POSS1 Red)",           // FC_SKYVIEW_DSS1R,
  "DSS (POSS1 Blue)",          // FC_SKYVIEW_DSS1B,
  "DSS (POSS2 Red)",           // FC_SKYVIEW_DSS2R,
  "DSS (POSS2 Blue)",          // FC_SKYVIEW_DSS2B,
  "DSS (POSS2 IR)",            // FC_SKYVIEW_DSS2IR,
  "SDSS (u-band)",             // FC_SKYVIEW_SDSSU,
  "SDSS (g-band)",             // FC_SKYVIEW_SDSSG,
  "SDSS (r-band)",             // FC_SKYVIEW_SDSSR,
  "SDSS (i-band)",             // FC_SKYVIEW_SDSSI,
  "SDSS (z-band)",             // FC_SKYVIEW_SDSSZ,
  "2MASS (J-band)",            // FC_SKYVIEW_2MASSJ,
  "2MASS (H-band)",            // FC_SKYVIEW_2MASSH,
  "2MASS (K-band)",            // FC_SKYVIEW_2MASSK,
  "WISE (3.4um)",              // FC_SKYVIEW_WISE34,
  "WISE (4.6um)",              // FC_SKYVIEW_WISE46,
  "WISE (12um)",               // FC_SKYVIEW_WISE12,
  "WISE (22um)",               // FC_SKYVIEW_WISE22,
  "AKARI N60",                 // FC_SKYVIEW_AKARIN60,
  "AKARI WIDE-S",              // FC_SKYVIEW_AKARIWS,
  "AKARI WIDE-L",              // FC_SKYVIEW_AKARIWL,
  "AKARI N160",                // FC_SKYVIEW_AKARIN160,
  "NVSS (1.4GHz)",             // FC_SKYVIEW_NVSS,
  NULL,                        // FC_SEP3,
  "SDSS (DR7/color)",          // FC_SDSS,
  "SDSS (DR14/color)",         // FC_SDSS13,
  NULL,                        // FC_SEP4,
  "PanSTARRS-1 (color)",       // FC_PANCOL,
  "PanSTARRS-1 (g-band)",      // FC_PANG,
  "PanSTARRS-1 (r-band)",      // FC_PANR,
  "PanSTARRS-1 (i-band)",      // FC_PANI,
  "PanSTARRS-1 (z-band)",      // FC_PANZ,
  "PanSTARRS-1 (y-band)"};     // FC_PANY

static const gchar* FC_host[]={
  FC_HOST_STSCI,         // FC_STSCI_DSS1R, 
  FC_HOST_STSCI,         // FC_STSCI_DSS1B, 
  FC_HOST_STSCI,         // FC_STSCI_DSS2R,
  FC_HOST_STSCI,         // FC_STSCI_DSS2B,
  FC_HOST_STSCI,         // FC_STSCI_DSS2IR,
  NULL,                  // FC_SEP1,
  FC_HOST_ESO,           // FC_ESO_DSS1R,
  FC_HOST_ESO,           // FC_ESO_DSS2R,
  FC_HOST_ESO,           // FC_ESO_DSS2B,
  FC_HOST_ESO,           // FC_ESO_DSS2IR,
  NULL,                  // FC_SEP2,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_GALEXF,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_GALEXN,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS1R,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS1B,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2R,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2B,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2IR,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSU,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSG,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSR,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSI,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSZ,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSJ,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSH,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSK,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE34,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE46,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE12,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE22,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIN60,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIWS,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIWL,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIN160,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_NVSS,
  NULL,                  // FC_SEP3,
  FC_HOST_SDSS,          // FC_SDSS,
  FC_HOST_SDSS,          // FC_SDSS13,
  NULL,                  // FC_SEP4,
  FC_HOST_PANCOL,        // FC_PANCOL,
  FC_HOST_PANCOL,        // FC_PANG,
  FC_HOST_PANCOL,        // FC_PANR,
  FC_HOST_PANCOL,        // FC_PANI,
  FC_HOST_PANCOL,        // FC_PANZ,
  FC_HOST_PANCOL};       // FC_PANY

// Guiding mode for HDS
enum{ NO_GUIDE, AG_GUIDE, SV_GUIDE, SVSAFE_GUIDE, NUM_GUIDE_MODE} GuideMode;

// AO mode for IRCS
enum{AOMODE_NO, AOMODE_NGS_S, AOMODE_NGS_O, AOMODE_LGS_S, AOMODE_LGS_O, NUM_AOMODE};
static const gchar* aomode_name[]={"w/o AO", "NGS(self)", "NGS(offset)", "LGS(self-TT)", "LGS(TT-GS)"};

// SV Read Area
enum{ SV_PART, SV_FULL} SVArea;

#ifdef USE_SKYMON
// SKYMON Mode
enum{ SKYMON_CUR, SKYMON_SET, SKYMON_PLAN_OBJ, SKYMON_PLAN_TIME} SkymonMode;

#define SUNSET_OFFSET 25
#define SUNRISE_OFFSET 25

#define SKYMON_DEF_OBJSZ 10
#endif

// SIZE　OF GUI ENTRY
#define SMALL_ENTRY_SIZE 24
#define LARGE_ENTRY_SIZE 28

#define HSKYMON_HTTP_ERROR_GETHOST  -1
#define HSKYMON_HTTP_ERROR_SOCKET   -2
#define HSKYMON_HTTP_ERROR_CONNECT  -3
#define HSKYMON_HTTP_ERROR_TEMPFILE -4
#ifdef USE_SSL
#define HSKYMON_HTTP_ERROR_SSL -5
#endif
#define HSKYMON_HTTP_ERROR_FORK -6

// SOSs
#define SOSS_HOSTNAME "sumda.sum.subaru.nao.ac.jp"
#define SOSS_PATH "Procedure"	 //#define SOSS_PATH "tmp"
#define PY_COM "python"
#define SFTP_LOG "hoe_sftp.log"

// Plot Mode
enum{ PLOT_EL, PLOT_AZ, PLOT_AD, PLOT_MOONSEP, PLOT_HDSPA} PlotMode;
enum{ PLOT_OBJTREE, PLOT_PLAN} PlotTarget;
enum{ PLOT_OUTPUT_WINDOW, PLOT_OUTPUT_PDF} PlotOutput;
enum{ SKYMON_OUTPUT_WINDOW, SKYMON_OUTPUT_PDF} SkymonOutput;
enum{ PLOT_ALL_SINGLE, PLOT_ALL_SELECTED,PLOT_ALL_ALL,PLOT_ALL_PLAN} PlotAll;
enum{ PLOT_CENTER_MIDNIGHT, PLOT_CENTER_CURRENT,PLOT_CENTER_MERIDIAN} PlotCenter;

#define PLOT_INTERVAL 60*1000

#define PLOT_WINSIZE 400

#define PLOT_WIDTH 600
#define PLOT_HEIGHT 400

#define FC_WINSIZE 400
enum{ FC_OUTPUT_WINDOW, FC_OUTPUT_PDF, FC_OUTPUT_PRINT, FC_OUTPUT_PDF_ALL} FCOutput;
enum{ FC_INST_HDS, FC_INST_HDSAUTO, FC_INST_HDSZENITH, FC_INST_NONE, FC_INST_IRCS, FC_INST_COMICS, FC_INST_FOCAS, FC_INST_MOIRCS, FC_INST_FMOS, FC_INST_SPCAM, FC_INST_HSCDET,FC_INST_HSCA, FC_INST_NO_SELECT} FCInst;
enum{ FC_SCALE_LINEAR, FC_SCALE_LOG, FC_SCALE_SQRT, FC_SCALE_HISTEQ, FC_SCALE_LOGLOG} FCScale;

#define EFS_WIDTH 800
#define EFS_HEIGHT 600
enum{ EFS_PLOT_EFS, EFS_PLOT_FSR} EFSMode;
enum{ EFS_OUTPUT_WINDOW, EFS_OUTPUT_PDF} EFSOutput;

//===ETC===========
enum {ETC_MENU, ETC_OBJTREE, ETC_LIST};
// magnitude zeropoints and wavelengths
enum {BAND_U, BAND_B, BAND_V, BAND_R, BAND_I, BAND_NUM};
static const char* etc_filters[] = {"U","B","V","R","I"};
enum{ETC_SPEC_POWERLAW,ETC_SPEC_BLACKBODY,ETC_SPEC_TEMPLATE,ETC_SPEC_NUM};
enum{ETC_WAVE_CENTER,ETC_WAVE_SPEC,ETC_WAVE_NUM};
enum{ST_O5V, ST_O9V, ST_B0V, ST_B3III, ST_B3V, ST_B8V, ST_A0V, 
     ST_A5V, ST_F0V, ST_F5V, ST_G0V, ST_G5V, ST_K0V, ST_NUM};
static const gchar* etc_st_name[]={
  "O5V","O9V","B0V","B3III","B3V","B8V","A0V",
  "A5V","F0V","F5V","G0V","G5V","K0V"
};
enum{ETC_ADC_IN, ETC_ADC_OUT};
enum{ETC_IMR_NO, ETC_IMR_BLUE, ETC_IMR_RED};

//====MagDB=========
// GSC
enum {GSC_BAND_U,GSC_BAND_B,GSC_BAND_V,GSC_BAND_R,GSC_BAND_I,
      GSC_BAND_J,GSC_BAND_H,GSC_BAND_K,NUM_GSC_BAND};
static const char* 
gsc_band[NUM_GSC_BAND] = {"U","B","V","R","I","J","H","K"};
// SDSS --> post_sdss.h
// PS1
enum {PS1_BAND_G,PS1_BAND_R,PS1_BAND_I,
      PS1_BAND_Z,PS1_BAND_Y,NUM_PS1_BAND};
static const char* 
ps1_band[NUM_PS1_BAND] = {"g","r","i","z","y"};
// GAIA  only G-band
// 2MASS
enum {TWOMASS_BAND_J,TWOMASS_BAND_H,TWOMASS_BAND_K,
      NUM_TWOMASS_BAND};
static const char* 
twomass_band[NUM_TWOMASS_BAND] = {"J","H","K"};



#define SKYMON_WIDTH 600
#define SKYMON_HEIGHT 600

//=====================  FCDB  =====================//

#define FCDB_ARCMIN_MAX 100
#define FCDB_PS1_ARCMIN_MAX 60
#define FCDB_USNO_ARCMIN_MAX 24

#define STDDB_PATH_SSLOC "/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_RAPID "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_MIRSTD "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_FILE_XML "simbad.xml"

#define FCDB_HOST_SIMBAD_STRASBG "simbad.u-strasbg.fr"
#define FCDB_HOST_SIMBAD_HARVARD "simbad.harvard.edu"
#define FCDB_PATH "/simbad/sim-sam?Criteria=region%%28box%%2C%lf%s%lf%%2C%+lfm%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_FILE_XML "database_fc.xml"
#define FCDB_FILE_TXT "database_fc.txt"
#define FCDB_FILE_HTML "database_fc.html"
#define FCDB_FILE_JSON "database_fc.json"

#define FCDB_HOST_NED "ned.ipac.caltech.edu"
#define FCDB_NED_PATH "/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=%.2lf&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY%sout_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=0&img_stamp=YES&of=xml_main"

#define FCDB_HOST_GSC "gsss.stsci.edu"
#define FCDB_GSC_PATH "/webservices/vo/ConeSearch.aspx?RA=%lf&DEC=%+lf&SR=%lf%sMAX_OBJ=5000&FORMAT=VOTable&CAT=GSC23"

#define FCDB_HOST_PS1 "gsss.stsci.edu"
#define FCDB_PS1_PATH  "/webservices/vo/CatalogSearch.aspx?CAT=PS1V3OBJECTS&RA=%lf&DEC=%+lf&SR=%lf&MINDET=%d%sMAXOBJ=5000"

#define FCDB_HOST_SDSS "skyserver.sdss.org"
#define FCDB_SDSS_PATH "/dr14/en/tools/search/x_results.aspx"

#define FCDB_HOST_USNO "www.nofs.navy.mil"
#define FCDB_USNO_PATH "/cgi-bin/vo_cone.cgi?CAT=USNO-B1&RA=%lf&DEC=%+lf&SR=%lf%sVERB=1"

#define FCDB_HOST_VIZIER_STRASBG "vizier.u-strasbg.fr"
#define FCDB_HOST_VIZIER_NAOJ "vizier.nao.ac.jp"
#define FCDB_HOST_VIZIER_HARVARD "vizier.cfa.harvard.edu"

#define FCDB_HOST_GAIA "vizier.u-strasbg.fr"
#define FCDB_GAIA_PATH "/viz-bin/votable?-source=I/345/gaia2&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_2MASS "gsss.stsci.edu"
#define FCDB_2MASS_PATH "/webservices/vo/CatalogSearch.aspx?CAT=2MASS&RA=%lf&DEC=%+lf&SR=%lf%sMAXOBJ=5000"

#define FCDB_HOST_WISE "vizier.u-strasbg.fr"
#define FCDB_WISE_PATH "/viz-bin/votable?-source=II/311/wise&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_IRC "vizier.u-strasbg.fr"
#define FCDB_IRC_PATH "/viz-bin/votable?-source=II/297/irc&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_FIS "vizier.u-strasbg.fr"
#define FCDB_FIS_PATH "/viz-bin/votable?-source=II/298/fis&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_LAMOST "dr4.lamost.org"
#define FCDB_LAMOST_PATH "/q"

#define FCDB_HOST_KEPLER "archive.stsci.edu"
#define FCDB_KEPLER_PATH "/kepler/kic10/search.php"

#define FCDB_HOST_SMOKA "smoka.nao.ac.jp"
#define FCDB_SMOKA_PATH "/fssearch"

#define FCDB_HOST_HST "archive.stsci.edu"
#define FCDB_HST_PATH "/hst/search.php"

#define FCDB_HOST_ESO "archive.eso.org"
#define FCDB_ESO_PATH "/wdb/wdb/eso/eso_archive_main/query"

#define FCDB_HOST_GEMINI "archive.gemini.edu"
#define FCDB_GEMINI_PATH "/jsonsummary/sr=%d/notengineering%sra=%.6lf/science/dec=%s%.6lf/NotFail/OBJECT/present/canonical"
#define TRDB_GEMINI_PATH "/jsonsummary/sr=%d/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT/present/canonical"



#define ADDOBJ_SIMBAD_PATH "/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=VOTABLE"
#define ADDOBJ_NED_PATH "/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES&of=xml_main"

#define FC_MAX_MAG 5

#define DBACCESS_VER     (-1)
#define DBACCESS_HSCFIL  (-2)
#define DBACCESS_HDSCAMZ (-3)

// FCDB_TYPE
enum
{
  FCDB_TYPE_SIMBAD,
  FCDB_TYPE_NED,
  FCDB_TYPE_GSC,
  FCDB_TYPE_PS1,
  FCDB_TYPE_SDSS,
  FCDB_TYPE_LAMOST,
  FCDB_TYPE_USNO,
  FCDB_TYPE_GAIA, 
  FCDB_TYPE_KEPLER,
  FCDB_TYPE_2MASS,
  FCDB_TYPE_WISE,
  
  FCDB_TYPE_IRC,
  FCDB_TYPE_FIS,
  FCDB_TYPE_SMOKA,
  FCDB_TYPE_HST,
  FCDB_TYPE_ESO,
  FCDB_TYPE_GEMINI,
  FCDB_TYPE_WWWDB_SMOKA,
  FCDB_TYPE_WWWDB_HST,
  FCDB_TYPE_WWWDB_ESO,
  TRDB_TYPE_SMOKA,
  TRDB_TYPE_HST,
  
  TRDB_TYPE_ESO,
  TRDB_TYPE_GEMINI,
  // Until here for FCDB on Finding Chart
  TRDB_TYPE_WWWDB_SMOKA,
  TRDB_TYPE_WWWDB_HST,
  TRDB_TYPE_WWWDB_ESO,
  TRDB_TYPE_FCDB_SMOKA,
  TRDB_TYPE_FCDB_HST,
  TRDB_TYPE_FCDB_ESO,
  TRDB_TYPE_FCDB_GEMINI,
  MAGDB_TYPE_SIMBAD,
  MAGDB_TYPE_NED,
  
  MAGDB_TYPE_LAMOST,
  MAGDB_TYPE_GSC,
  MAGDB_TYPE_PS1,
  MAGDB_TYPE_SDSS,
  MAGDB_TYPE_GAIA,
  MAGDB_TYPE_KEPLER,
  MAGDB_TYPE_2MASS,
  
  MAGDB_TYPE_IRCS_GSC,
  MAGDB_TYPE_IRCS_PS1,
  MAGDB_TYPE_IRCS_GAIA,
  
  MAGDB_TYPE_HSC_SIMBAD,

  NUM_DB_ALL
};


static const gchar* db_name[]={  
  "SIMBAD",         //FCDB_TYPE_SIMBAD,
  "NED",            //FCDB_TYPE_NED,
  "GSC 2.3",        //FCDB_TYPE_GSC,
  "PanSTARRS1",     //FCDB_TYPE_PS1,
  "SDSS DR14",      //FCDB_TYPE_SDSS,
  "LAMOST DR4",     //FCDB_TYPE_LAMOST,
  "USNO",           //FCDB_TYPE_USNO,
  "GAIA DR2",       //FCDB_TYPE_GAIA, 
  "Kepler",         //FCDB_TYPE_KEPLER,
  "2MASS",          //FCDB_TYPE_2MASS,
  "WISE",           //FCDB_TYPE_WISE,
  "Akari/IRC",      //FCDB_TYPE_IRC,
  "Akari/FIS",      //FCDB_TYPE_FIS,
  "Subaru(SMOKA)",  //FCDB_TYPE_SMOKA,
  "HST archive",    //FCDB_TYPE_HST,
  "ESO archive",    //FCDB_TYPE_ESO,
  "Gemini archive", //FCDB_TYPE_GEMINI,
  "Subaru(SMOKA)",  //FCDB_TYPE_WWWDB_SMOKA,
  "HST archive",    //FCDB_TYPE_WWWDB_HST,
  "ESO archive",    //FCDB_TYPE_WWWDB_ESO,
  "Subaru(SMOKA)",  //TRDB_TYPE_SMOKA,
  "HST archive",    //TRDB_TYPE_HST,
  "ESO archive",    //TRDB_TYPE_ESO,
  "Gemini archive", //TRDB_TYPE_GEMINI,
  // Until here for FCDB on Finding Chart
  "SMOKA",          //TRDB_TYPE_WWWDB_SMOKA,
  "HST archive",    //TRDB_TYPE_WWWDB_HST,
  "ESO archive",    //TRDB_TYPE_WWWDB_ESO,
  "Subaru(SMOKA)",  //TRDB_TYPE_FCDB_SMOKA,
  "HST archive",    //TRDB_TYPE_FCDB_HST,
  "ESO archive",    //TRDB_TYPE_FCDB_ESO,
  "Gemini archive", //TRDB_TYPE_FCDB_GEMINI,
  "SIMBAD",         //MAGDB_TYPE_SIMBAD,
  "NED",            //MAGDB_TYPE_NED,
  "LAMOST DR4",     //MAGDB_TYPE_LAMOST,
  "GSC 2.3",        //MAGDB_TYPE_GSC,
  "PanSTARRS1",     //MAGDB_TYPE_PS1,
  "SDSS DR14",      //MAGDB_TYPE_SDSS,
  "GAIA DR2",       //MAGDB_TYPE_GAIA,
  "Kepler IC10",    //MAGDB_TYPE_KEPLER
  "2MASS",          //MAGDB_TYPE_2MASS,
  "GSC 2.3",        //MAGDB_TYPE_IRCS_GSC,
  "PanSTARRS1",     //MAGDB_TYPE_IRCS_PS1,
  "GAIA DR2",       //MAGDB_TYPE_IRCS_GAIA,
  "SIMBAD"          //MAGDB_TYPE_HSC_SIMBAD,
};


enum{ WWWDB_SIMBAD, 
      WWWDB_NED, 
      WWWDB_DR8, 
      WWWDB_DR14, 
      WWWDB_MAST, 
      WWWDB_MASTP,
      WWWDB_KECK, 
      WWWDB_GEMINI, 
      WWWDB_IRSA, 
      WWWDB_SPITZER, 
      WWWDB_CASSIS, 
      WWWDB_SEP1, 
      WWWDB_SSLOC, 
      WWWDB_RAPID, 
      WWWDB_MIRSTD, 
      WWWDB_SEP2, 
      WWWDB_SMOKA, 
      WWWDB_HST, 
      WWWDB_ESO} WWWDBMode;

enum{ STDDB_SSLOC, 
      STDDB_RAPID, 
      STDDB_MIRSTD, 
      STDDB_ESOSTD, 
      STDDB_IRAFSTD, 
      STDDB_CALSPEC, 
      STDDB_HDSSTD} STDDBMode;

#define STD_DRA 20
#define STD_DDEC 10
#define STD_VSINI 100
#define STD_VMAG 8
#define STD_SPTYPE "A0"
#define STD_IRAS12 5
#define STD_IRAS25 10
#define STD_CAT "FS"
#define STD_MAG1 5
#define STD_MAG2 15
#define STD_BAND "Jmag"
#define STD_SPTYPE_ALL "%20"
#define STD_SPTYPE_O   "%26(sptype>=O0%26sptype<=O9.9)"
#define STD_SPTYPE_B   "%26(sptype>=B0%26sptype<=B9.9)"
#define STD_SPTYPE_A   "%26(sptype>=A0%26sptype<=A9.9)"
#define STD_SPTYPE_F   "%26(sptype>=F0%26sptype<=F9.9)"
#define STD_SPTYPE_G   "%26(sptype>=G0%26sptype<=G9.9)"
#define STD_SPTYPE_K   "%26(sptype>=K0%26sptype<=K9.9)"
#define STD_SPTYPE_M   "%26(sptype>=M0%26sptype<=M11.9)"

enum
{
  FCDB_BAND_NOP,
  FCDB_BAND_U,
  FCDB_BAND_B,
  FCDB_BAND_V,
  FCDB_BAND_R,
  FCDB_BAND_I,
  FCDB_BAND_J,
  FCDB_BAND_H,
  FCDB_BAND_K,
  NUM_FCDB_BAND
};

static char* simbad_band[NUM_FCDB_BAND]=
  {"(Nop.)", "U", "B", "V", "R", "I", "J", "H", "K"};

enum
{
  FCDB_OTYPE_ALL,
  FCDB_OTYPE_STAR,
  FCDB_OTYPE_ISM,
  FCDB_OTYPE_PN,
  FCDB_OTYPE_HII,
  FCDB_OTYPE_GALAXY,
  FCDB_OTYPE_QSO,
  FCDB_OTYPE_GAMMA,
  FCDB_OTYPE_X,
  FCDB_OTYPE_IR,
  FCDB_OTYPE_RADIO,
  NUM_FCDB_OTYPE
};

enum
{
  FCDB_NED_OTYPE_ALL,
  FCDB_NED_OTYPE_EXTRAG,
  FCDB_NED_OTYPE_QSO,
  FCDB_NED_OTYPE_STAR,
  FCDB_NED_OTYPE_SN,
  FCDB_NED_OTYPE_PN,
  FCDB_NED_OTYPE_HII,
  NUM_FCDB_NED_OTYPE
};

enum
{
    LIST_DEFAULT,
    LIST_MAG,
    LIST_STYLE_NUM
};

//=====================  end of FCDB  =====================//



typedef struct _EPHpara EPHpara;
struct _EPHpara{
  gdouble jd;
  gdouble ra;
  gdouble dec;
  gdouble equinox;
  gdouble geo_d;
};

typedef struct _NSTpara NSTpara;
struct _NSTpara{
  gchar*  filename;
  gint    type;
  gint    i_max;
  EPHpara* eph;
  gint    c_fl;
  gint    s_fl;
};


typedef struct _GSpara GSpara;
struct _GSpara{
  gboolean flag;
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble equinox;
  gdouble mag;
  gint    src;
  gdouble sep;


};

typedef struct _OBJpara OBJpara;
struct _OBJpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble pm_ra;
  gdouble pm_dec;
  gdouble equinox;
  gdouble mag;
  gdouble snr;
  gboolean sat;
  gboolean std;

  gint i_nst;

  gint horizon;
  gdouble rise;
  gdouble transit;
  gdouble set;

  gint guide;
  guint aomode;
  gint pam;
  gboolean adi;
  
  gdouble pa;
  gboolean setup[MAX_USESETUP];

  gint exp;
  gint repeat;
  gint svfilter;

  /*
  GtkWidget *w_az;
  GtkWidget *w_ha;
  GtkWidget *w_pa;
  */
  gdouble c_az;
  gdouble c_el;
  gdouble c_elmax;
  gdouble c_ha;
  gdouble c_pa;
  gdouble c_ad;
  gdouble c_rt;
  gdouble c_vaz;
  gdouble c_vpa;
  gdouble c_sep;
  gdouble c_hpa;
  gdouble c_vhpa;

#ifdef USE_SKYMON
  gdouble s_az;
  gdouble s_el;
  gdouble s_elmax;
  gdouble s_ha;
  gdouble s_pa;
  gdouble s_ad;
  gdouble s_vaz;
  gdouble s_vpa;
  gdouble s_sep;
  gdouble s_hpa;
  gdouble s_vhpa;
#endif

  gboolean check_sm;

  gchar *note;

  gdouble x;
  gdouble y;

  gchar *trdb_str;
  gchar *trdb_mode[MAX_TRDB_BAND];
  gchar *trdb_band[MAX_TRDB_BAND];
  gdouble trdb_exp[MAX_TRDB_BAND];
  gint trdb_shot[MAX_TRDB_BAND];
  gint trdb_band_max;

  gint magdb_used;
  gint magdb_band;

  gint magdb_gsc_hits;
  gint magdb_ps1_hits;
  gint magdb_sdss_hits;
  gint magdb_gaia_hits;
  gint magdb_kepler_hits;
  gint magdb_2mass_hits;
  gint magdb_simbad_hits;
  gint magdb_ned_hits;
  gint magdb_lamost_hits;

  gdouble magdb_gsc_sep;
  gdouble magdb_ps1_sep;
  gdouble magdb_sdss_sep;
  gdouble magdb_gaia_sep;
  gdouble magdb_kepler_sep;
  gdouble magdb_2mass_sep;
  gdouble magdb_simbad_sep;
  gdouble magdb_ned_sep;
  gdouble magdb_lamost_sep;

  gdouble magdb_gsc_u;
  gdouble magdb_gsc_b;
  gdouble magdb_gsc_v;
  gdouble magdb_gsc_r;
  gdouble magdb_gsc_i;
  gdouble magdb_gsc_j;
  gdouble magdb_gsc_h;
  gdouble magdb_gsc_k;
  gdouble magdb_ps1_g;
  gdouble magdb_ps1_r;
  gdouble magdb_ps1_i;
  gdouble magdb_ps1_z;
  gdouble magdb_ps1_y;
  gdouble magdb_sdss_u;
  gdouble magdb_sdss_g;
  gdouble magdb_sdss_r;
  gdouble magdb_sdss_i;
  gdouble magdb_sdss_z;
  gdouble magdb_gaia_g;
  gdouble magdb_gaia_p;
  gdouble magdb_gaia_ep;
  gdouble magdb_gaia_bp;
  gdouble magdb_gaia_rp;
  gdouble magdb_gaia_rv;
  gdouble magdb_gaia_teff;
  gdouble magdb_gaia_ag;
  gdouble magdb_gaia_ebr;
  gdouble magdb_gaia_dist;
  gchar *magdb_kepler_name;
  gdouble magdb_kepler_k;
  gdouble magdb_kepler_r;
  gdouble magdb_kepler_j;
  gdouble magdb_kepler_teff;
  gdouble magdb_kepler_logg;
  gdouble magdb_kepler_feh;
  gdouble magdb_kepler_ebv;
  gdouble magdb_kepler_rad;
  gdouble magdb_kepler_pm;
  gdouble magdb_kepler_gr;
  gchar *magdb_kepler_2mass;
  gdouble magdb_2mass_j;
  gdouble magdb_2mass_h;
  gdouble magdb_2mass_k;
  gdouble magdb_simbad_u;
  gdouble magdb_simbad_b;
  gdouble magdb_simbad_v;
  gdouble magdb_simbad_r;
  gdouble magdb_simbad_i;
  gdouble magdb_simbad_j;
  gdouble magdb_simbad_h;
  gdouble magdb_simbad_k;

  gchar *magdb_simbad_name;
  gchar *magdb_simbad_type;
  gchar *magdb_simbad_sp;

  gchar *magdb_ned_name;
  gchar *magdb_ned_type;
  gchar *magdb_ned_mag;
  gdouble magdb_ned_z;
  gint magdb_ned_ref;

  gchar *magdb_lamost_name;
  gint magdb_lamost_ref;
  gdouble magdb_lamost_teff;
  gdouble magdb_lamost_logg;
  gdouble magdb_lamost_feh;
  gdouble magdb_lamost_hrv;
  gchar *magdb_lamost_type;
  gchar *magdb_lamost_sp;

  HSCmag hscmag;

  GSpara gs;
};

typedef struct _PlanMoonpara typPlanMoon;
struct _PlanMoonpara{
  struct ln_hms ra;
  struct ln_dms dec;
  gdouble az;
  gdouble el;
  gdouble disk;
  gdouble phase;
  gdouble limb;
  gdouble age;
  gdouble sep;
};

typedef struct _PLANpara PLANpara;
struct _PLANpara{
  guint type;
  gchar *txt;
  
  gint setup;  // Obj Flat
  guint repeat;  // Obj Flat Bias
  gboolean slit_or;
  guint slit_width;  
  guint slit_length;
  
  // Get Object
  gint  obj_i;
  guint exp;

  gdouble dexp;
  guint shot;
  guint coadds;
  guint ndr;
  
  gint dith;
  gdouble dithw;
  gint osra;
  gint osdec;
  gdouble sssep;
  gint ssnum;

  gint skip;
  gint stop;

  guint omode;
  gint guide;
  gint aomode;
  gboolean adi;

  // BIAS
  // Flat
  // Comp
  // Focus
  gint focus_mode;
  gdouble focus_z;
  gdouble delta_z;
  guint focus_is;
  gint cal_mode;

  // Setup
  guint cmode;
  gint  colinc;
  gdouble  colv;
  gboolean  is_change;
  gboolean  bin_change;

  // I2
  guint i2_pos;

  gboolean daytime;
  
  gchar *comment;
  guint comtype;

  gint time;
  gint stime;
  glong sod;

  gboolean pa_or;
  gdouble pa;
  gboolean sv_or;
  guint sv_exp;
  guint sv_fil;
  gboolean backup;

  // HSC
  gboolean hsc_30;

  // Az El
  gdouble setaz;
  gdouble setel;

  gdouble az0;
  gdouble az1;
  gdouble el0;
  gdouble el1;

  gchar *txt_az;
  gchar *txt_el;
  
  typPlanMoon moon;
};


typedef struct _PPpara PPpara;
struct _PPpara{
  gdouble ut;

  gdouble az;
  gdouble el;

  guint i_plan;
  
  gboolean start;
};


typedef struct _PApara PApara;
struct _PApara{
  gchar *name;
  gdouble ra;
  gdouble dec;

  gint year;
  gint month;
  gint day;
};


typedef struct _Setuppara Setuppara;
struct _Setuppara{
  gint  setup;
  gboolean use;
  guint    binning;
  guint    slit_width;
  guint    slit_length;
  gchar    *fil1;
  gchar    *fil2;
  guint    imr;
  guint    is;
  gboolean i2;
};


typedef struct _Nonstdpara Nonstdpara;
struct _Nonstdpara{
  guint col;
  guint cross;
  gint  echelle;
  gint  camr;
};


typedef struct _Crosspara Crosspara;
struct _Crosspara{
  guint col;
  guint cross;
};

#define MAX_ETC 200

typedef struct _etcpara ETCpara;
struct _etcpara{
  gint order;
  gint pix_s;
  gint pix_c;
  gint pix_e;
  gdouble w_s;
  gdouble w_c;
  gdouble w_e;
  gdouble disp;
  gdouble flux;
  gint peak;
  gdouble snr;
  gdouble isgain;
  gdouble snr_gain;
  gboolean sat;
  gboolean bad;
  gint ccd;
  gboolean isgap;
};

typedef struct _Linepara Linepara;
struct _Linepara{
  gchar *name;
  gdouble  wave;
};


typedef struct _STDpara STDpara;
struct _STDpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble d_ra;
  gdouble d_dec;
  gdouble pmra;
  gdouble pmdec;
  gboolean pm;
  gdouble equinox;
  gchar *sp;
  gdouble sep;
  gdouble rot;
  gdouble u;
  gdouble b;
  gdouble v;
  gdouble r;
  gdouble i;
  gdouble j;
  gdouble h;
  gdouble k;
  gdouble c_az;
  gdouble c_el;
  gdouble c_elmax;
  gdouble s_az;
  gdouble s_el;
  gdouble s_elmax;

  gdouble x;
  gdouble y;
};

typedef struct _FCDBpara FCDBpara;
struct _FCDBpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble d_ra;
  gdouble d_dec;
  gdouble pmra;
  gdouble pmdec;
  gboolean pm;
  gdouble equinox;
  gchar *otype;
  gchar *sp;
  gdouble sep;
  gchar *nedmag;
  gdouble nedvel;
  gdouble nedz;
  gdouble plx;
  gdouble eplx;
  gdouble u;
  gdouble b;
  gdouble v;
  gdouble r;
  gdouble i;
  gdouble j;
  gdouble h;
  gdouble k;
  gchar *fid;
  gchar *date;
  gchar *mode;
  gchar *type;
  gchar *fil;
  gchar *wv;
  gchar *obs;
  gdouble x;
  gdouble y;
  gint ref;
};

typedef struct _HMSpara my_hms;
struct _HMSpara{
  gint hours;
  gint minutes;
  gdouble seconds;
};

typedef struct _Moonpara typMoon;
struct _Moonpara{
  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  gdouble c_ha;
  gdouble c_disk;
  gdouble c_phase;
  gdouble c_limb;
  gdouble c_age;
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

#ifdef USE_SKYMON
  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  gdouble s_ha;
  gdouble s_disk;
  gdouble s_phase;
  gdouble s_limb;
  gdouble s_age;
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;

  struct ln_hms p_ra[MAX_PP];
  struct ln_dms p_dec[MAX_PP];
  gdouble p_az[MAX_PP];
  gdouble p_el[MAX_PP];
#endif
};

typedef struct _Sunpara typSun;
struct _Sunpara{
  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;
};


typedef struct _Planetpara typPlanet;
struct _Planetpara{
  gchar *name;

  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  gdouble c_mag;

  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  gdouble s_mag;
};


typedef struct _typHOE typHOE;
struct _typHOE{
  gchar *temp_dir;
  gchar *home_dir;

  gint inst;
  gboolean init_flag;

#ifdef USE_WIN32
  HANDLE hThread_dss;
  HANDLE hThread_stddb;
  HANDLE hThread_fcdb;
  unsigned int dwThreadID_dss;
  unsigned int dwThreadID_stddb;
  unsigned int dwThreadID_fcdb;
#endif

#ifdef USE_GTK3
  GtkCssProvider *provider;
#endif

  gdouble vel_az;
  gdouble vel_el;

  gint sz_skymon;
  gint sz_plot;
  gint sz_fc;

  guint page[NUM_NOTE];

  GtkWidget *w_top;
  GtkWidget *plan_main;

  GtkWidget *w_box;
  GtkWidget *all_note;
  GtkWidget *scrwin;
  GtkWidget *setup_scrwin;

  GtkWidget *plan_note;
  GtkWidget *query_note;

  GtkWidget *pbar;
  GtkWidget *plabel;
  GtkWidget *pbar2;

  GtkPrintContext *context;

  gchar *fontname;  
  gchar *fontfamily;  
  gchar *fontname_all;  
  gchar *fontfamily_all;  
  gint  skymon_allsz;

  gdouble win_cx;
  gdouble win_cy;
  gdouble win_r;

#ifdef USE_SKYMON
  gint skymon_timer;
#endif
  gchar *filename_read;
  gchar *filename_write;
  gchar *filename_pdf;
  gchar *filename_txt;
  gchar *filename_hoe;
  gchar *filename_log;
  gchar *filename_fcdb;
  gchar *filename_trdb;
  gchar *filename_nst;
  gchar *filename_jpl;
  gchar *filename_tscconv;
  gchar *filename_prm1;
  gchar *filename_prm2;
  gchar *filename_lgs_pam;
  gchar *filename_pamout;
  gchar *dirname_pamout;
  gchar *filehead;

  guint list_style;
  guint list_read;

  gint i_max;
  
  OBJpara obj[MAX_OBJECT];
  NSTpara nst[MAX_ROPE];
  PLANpara plan[MAX_PLAN];
  STDpara std[MAX_STD];
  FCDBpara fcdb[MAX_FCDB];
  ETCpara etc[MAX_ETC];

  PPpara pp[MAX_PP];

  guint fr_year,fr_month,fr_day;
  gdouble fr_moon;
  GtkWidget *fr_e;
  GtkWidget *label_moon;
  GtkWidget *label_sun;
  gchar *prop_id;
  gchar *prop_pass;
  GtkWidget *e_pass;
  gchar *observer;
  gint obs_timezone;
  gchar *obs_tzname;
  gdouble obs_longitude;
  gdouble obs_latitude;
  gdouble obs_altitude;
  guint wave1;
  guint wave0;
  guint pres;
  gint  temp;
  GtkWidget *label_stat_base;
  GtkWidget *label_stat_plan;

  Setuppara setup[MAX_USESETUP];
  Binpara binning[MAX_BINNING];
  gint camz_b;
  gint camz_r;
  gchar *camz_date;
  GtkWidget *camz_label;
  gint d_cross;
  Nonstdpara nonstd[MAX_NONSTD];
  GtkAdjustment *camz_b_adj;
  GtkAdjustment *camz_r_adj;
  GtkAdjustment *d_cross_adj;
  GtkAdjustment *echelle_adj[MAX_NONSTD];

  gint wcent;
  GtkWidget *label_wcent;

  guint exptime_factor;
  guint brightness;
  guint sv_area;
  guint sv_integrate;
  guint sv_region;
  guint sv_calc;
  guint exptime_sv;
  gdouble sv_slitx;
  gdouble sv_slity;
  gdouble sv_isx;
  gdouble sv_isy;
  gdouble sv_is3x;
  gdouble sv_is3y;

  guint oh_acq;
  guint oh_ngs1;
  guint oh_ngs2;
  guint oh_ngs3;
  guint oh_lgs;
  
  gint def_guide;
  gdouble def_pa;
  guint def_exp;
  gint def_aomode;
  
  Linepara line[MAX_LINE];

  guint efs_setup;
  gchar* efs_ps;

  guint e_list;
  guint e_exp;
  guint e_times;
  GtkAdjustment *e_adj;
  GtkWidget *e_button[MAX_USESETUP];
  //GtkWidget *c_label;

  gint lst_hour;
  gint lst_min;
  gint lst_sec;

#ifdef USE_SKYMON
  gint skymon_lst_hour;
  gint skymon_lst_min;
  gint skymon_lst_sec;
#endif

  gchar *add_obj;
  GtkWidget *e_add_obj;
  gint add_num;

  gdouble expmag_mag;
  gint expmag_exp;

  gint azel_mode;

  gint wwwdb_mode;
  gint stddb_mode;


#ifdef USE_SKYMON
  GtkWidget *skymon_main;
  GtkWidget *skymon_dw;
  GtkWidget *skymon_e_date;
  GtkWidget *skymon_frame_mode;
  GtkWidget *skymon_frame_date;
  GtkWidget *skymon_frame_time;
  GtkWidget *skymon_frame_sz;
  GtkWidget *skymon_button_set;
  GtkWidget *skymon_button_fwd;
  GtkWidget *skymon_button_rev;
  GtkWidget *skymon_button_morn;
  GtkWidget *skymon_button_even;
  GtkAdjustment *skymon_adj_min;
  gint skymon_mode;
  guint skymon_year,skymon_month,skymon_day;
  gint skymon_min,skymon_hour;
  gint skymon_time;
  gint skymon_objsz;
#endif

  typMoon moon;
  typSun sun;
  typSun atw06;
  typSun atw12;
  typSun atw18;

  typPlanet mercury;
  typPlanet venus;
  typPlanet mars;
  typPlanet jupiter;
  typPlanet saturn;
  typPlanet uranus;
  typPlanet neptune;
  typPlanet pluto;

  gint plot_mode;
  gint plot_center;
  gint plot_zoom;
  gint plot_zoomx;
  gdouble plot_zoomr;
  gint plot_target;
  gboolean plot_moon;
  gboolean plot_pam;
  GtkWidget *plot_main;
  GtkWidget *plot_dw;
  gint plot_i;
  gint plot_i_plan;
  gint plan_trace;
  gdouble plan_jd1;
  gdouble plan_jd2;
  gint plot_output;
  gdouble plot_jd0;
  gdouble plot_jd1;
  gint skymon_output;

  gint efs_mode;
  GtkWidget *efs_main;
  GtkWidget *efs_dw;
  gint efs_output;

  GtkWidget *etc_tree;
  GtkWidget *etc_sw;
  GtkWidget *etc_label;
  gchar *etc_label_text;
  gchar *etc_prof_text;
  gint etc_i;
  gint etc_mode;
  gint etc_filter;
  gdouble etc_mag;
  gdouble etc_z;
  gint etc_spek;
  gdouble etc_alpha;
  gint etc_bbtemp;  
  gint etc_sptype;
  gint etc_adc;
  gint etc_imr;
  gint etc_exptime;
  gdouble etc_seeing;
  gint etc_setup;
  gint etc_i_max;
  GtkAdjustment *etc_z_adj;
  gint etc_wave;
  gint etc_waved;

  gint pm_i;

  GtkWidget *objtree;
  GtkWidget *sw_objtree;

  GtkWidget *linetree;
  GtkWidget *sw_linetree;

  GtkWidget *tree_search_label;
  gchar *tree_search_text;
  guint tree_search_i;
  guint tree_search_iobj[MAX_OBJECT];
  guint tree_search_imax;

  gchar *www_com;

  gint fc_mode;
  gint fc_mode0;
  gint fc_mode_get;
  gint fc_mode_def;
  gint fc_inst;
  gint fc_output;
  GtkWidget *fc_frame_col;
  GtkWidget *fc_frame_col_pdf;
  GtkWidget *fc_button_flip;
  gint fc_shift_x;
  gint fc_shift_y;
  gint dss_arcmin;
  gint dss_arcmin_ip;
  gint dss_pix;
  gint dss_scale;
  gboolean dss_invert;

  gint dss_i;
  gchar *dss_host;
  gchar *dss_path;
  gchar *dss_src;
  gchar *dss_tmp;
  gchar *dss_file;
  gint dss_pa;
  GtkAdjustment *fc_adj_dss_pa;
  GtkAdjustment *fc_adj_dss_arcmin;
  GtkWidget *check_hsc_sat;
  gboolean dss_flip;
  gboolean dss_draw_slit;
  gboolean sdss_photo;
  gboolean sdss_spec;
  GtkWidget *fc_main;
  GtkWidget *fc_dw;
  gint fc_mag;
  gint fc_magx;
  gint fc_magy;
  gint fc_magmode;
  gint fc_ptn;
  gint fc_ptx1;
  gint fc_pty1;
  gint fc_ptx2;
  gint fc_pty2;

  gchar *hsc_filter_ver;
  gboolean hsc_filter_updated;
  gdouble hsc_focus_z;
  gdouble hsc_delta_z;
  gboolean hsc_sat;
  GtkWidget *hsc_show_main;
  gboolean hsc_show_ol;
  gint hsc_show_dith_i;
  gint hsc_show_dith_p;
  gint hsc_show_dith_ra;
  gint hsc_show_dith_dec;
  gint hsc_show_dith_n;
  gint hsc_show_dith_r;
  gint hsc_show_dith_t;
  gint hsc_show_osra;
  gint hsc_show_osdec;
  GtkWidget *hsc_label_dith;
  
  guint alldss_check_timer;

  GtkWidget *plan_tree;
  guint i_plan_max;

  guint i_pp_max;
  guint i_pp_moon_max;

  GtkWidget *f_objtree_arud;

  gboolean  plan_tmp_or;
  guint  plan_tmp_sw;
  guint  plan_tmp_sl;
  guint  plan_tmp_setup;

  GtkWidget *plan_obj_combo;
  GtkAdjustment *plan_obj_adj;
  GtkAdjustment *plan_exp_adj;
  GtkAdjustment *plan_dexp_adj;
  GtkWidget *plan_obj_guide_combo;
  gint  plan_obj_i;
  guint  plan_obj_exp;
  gdouble  plan_obj_dexp;
  guint  plan_obj_repeat;
  GtkWidget *plan_adi_check;
  gboolean plan_adi;
  gboolean plan_backup;

  guint  plan_ircs_ndr;
  guint  plan_ircs_coadds;
  GtkWidget *plan_dith_combo;
  guint  plan_dith;
  gdouble  plan_dithw;
  GtkAdjustment *plan_dithw_adj;
  gint  plan_osra;
  GtkAdjustment *plan_osra_adj;
  gint  plan_osdec;
  GtkAdjustment *plan_osdec_adj;
  gdouble  plan_sssep;
  GtkAdjustment *plan_sssep_adj;
  gint  plan_ssnum;
  GtkAdjustment *plan_ssnum_adj;
  gint  plan_skip;
  gint  plan_skip_upper;
  GtkAdjustment *plan_skip_adj;
  GtkWidget *plan_skip_label;
  gint  plan_stop;
  gint  plan_stop_upper;
  GtkAdjustment *plan_stop_adj;
  GtkWidget *plan_stop_label;

  GtkAdjustment *plan_e_dexp_adj;
  GtkAdjustment *plan_e_dithw_adj;
  GtkAdjustment *plan_e_osra_adj;
  GtkAdjustment *plan_e_osdec_adj;
  GtkAdjustment *plan_e_sssep_adj;
  GtkAdjustment *plan_e_ssnum_adj;
  GtkAdjustment *plan_e_skip_adj;
  GtkAdjustment *plan_e_stop_adj;
  GtkWidget *plan_e_skip_label;
  GtkWidget *plan_e_stop_label;
  GtkWidget *plan_e_check_hsc_30;
  
  guint  plan_e_tmp_setup;
  GtkWidget *plan_e_dith_combo;
  
  guint  plan_obj_omode;
  guint  plan_obj_guide;

  gboolean plan_setazel_daytime;
  gdouble plan_setaz;
  gdouble plan_setel;

  guint  plan_bias_repeat;
  gboolean plan_bias_daytime;

  gint plan_comp_mode;
  guint  plan_comp_repeat;
  gboolean plan_comp_daytime;

  gint plan_flat_mode;
  guint  plan_flat_repeat;
  gboolean plan_flat_daytime;

  GtkWidget *plan_focus_combo;
  gint  plan_focus_mode;
  gdouble plan_focus_z;
  gdouble plan_delta_z;
  GtkWidget *plan_hbox_dz;
  gboolean plan_hsc_30;
  GtkWidget *check_hsc_30;

  guint  plan_setup_cmode;
  gboolean plan_setup_daytime;

  guint  plan_i2_pos;
  gboolean plan_i2_daytime;

  gchar  *plan_comment;
  guint  plan_comment_time;
  guint  plan_comment_type;

  guint  plan_start;
  guint  plan_start_hour;
  guint  plan_start_min;

  guint  plan_delay;

  gint plot_all;

  gchar *std_file;
  gchar *std_host;
  gchar *std_path;
  gint  std_i;
  gint  std_i_max;
  GtkWidget *stddb_tree;
  GtkWidget *std_tgt;
  gint stddb_tree_focus;
  GtkWidget *stddb_label;
  GtkWidget *stddb_button;
  gchar *stddb_label_text;
  gboolean stddb_flag;

  gint std_dra;
  gint std_ddec;
  gint std_vsini;
  gint std_vmag;
  gchar *std_sptype;
  gint std_iras12;
  gint std_iras25;
  gchar *std_cat;
  gint std_mag1;
  gint std_mag2;
  gchar *std_band;
  gchar *std_sptype2;

  gboolean fc_all_magskip;

  gint fcdb_type;
  gboolean fcdb_post;
  gchar *fcdb_file;
  gint fcdb_simbad;
  gint fcdb_vizier;
  gchar *fcdb_host;
  gchar *fcdb_path;
  gint fcdb_i;
  gint fcdb_tree_focus;
  gdouble fcdb_d_ra0;
  gdouble fcdb_d_dec0;
  gint  fcdb_i_max;
  gint  fcdb_i_all;
  GtkWidget *fcdb_tree;
  GtkWidget *fcdb_sw;
  GtkWidget *fcdb_label;
  GtkWidget *fcdb_frame;
  GtkWidget *fcdb_button;
  GtkWidget *fcdb_tgt;
  gchar *fcdb_label_text;
  gboolean fcdb_flag;
  gint fcdb_band;
  gint fcdb_mag;
  gint fcdb_otype;
  gint fcdb_ned_diam;
  gint fcdb_ned_otype;
  gboolean fcdb_auto;
  gboolean fcdb_ned_ref;
  gboolean fcdb_gsc_fil;
  gint fcdb_gsc_mag;
  gint fcdb_gsc_diam;
  gboolean fcdb_ps1_fil;
  gint fcdb_ps1_mag;
  gint fcdb_ps1_diam;
  gint fcdb_ps1_mindet;
  gint fcdb_sdss_search;
  gint fcdb_sdss_magmin[NUM_SDSS_BAND];
  gint fcdb_sdss_magmax[NUM_SDSS_BAND];
  gboolean fcdb_sdss_fil[NUM_SDSS_BAND];
  gint fcdb_sdss_diam;
  gint fcdb_usno_mag;
  gint fcdb_usno_diam;
  gboolean fcdb_usno_fil;
  gint fcdb_gaia_mag;
  gint fcdb_gaia_diam;
  gboolean fcdb_gaia_fil;
  gint fcdb_kepler_mag;
  gboolean fcdb_kepler_fil;
  gint fcdb_2mass_mag;
  gint fcdb_2mass_diam;
  gboolean fcdb_2mass_fil;
  gint fcdb_wise_mag;
  gint fcdb_wise_diam;
  gboolean fcdb_wise_fil;
  gboolean fcdb_smoka_shot;
  gboolean fcdb_smoka_subaru[NUM_SMOKA_SUBARU];
  gboolean fcdb_smoka_kiso[NUM_SMOKA_KISO];
  gboolean fcdb_smoka_oao[NUM_SMOKA_OAO];
  gboolean fcdb_smoka_mtm[NUM_SMOKA_MTM];
  gboolean fcdb_smoka_kanata[NUM_SMOKA_KANATA];
  gboolean fcdb_hst_image[NUM_HST_IMAGE];
  gboolean fcdb_hst_spec[NUM_HST_SPEC];
  gboolean fcdb_hst_other[NUM_HST_OTHER];
  gboolean fcdb_eso_image[NUM_ESO_IMAGE];
  gboolean fcdb_eso_spec[NUM_ESO_SPEC];
  gboolean fcdb_eso_vlti[NUM_ESO_VLTI];
  gboolean fcdb_eso_pola[NUM_ESO_POLA];
  gboolean fcdb_eso_coro[NUM_ESO_CORO];
  gboolean fcdb_eso_other[NUM_ESO_OTHER];
  gboolean fcdb_eso_sam[NUM_ESO_SAM];
  gint fcdb_gemini_inst;

  GtkWidget *trdb_tree;
  GtkWidget *trdb_sw;
  GtkWidget *trdb_label;
  GtkWidget *trdb_combo;
  gint trdb_i_max;
  gint trdb_tree_focus;
  gboolean trdb_disp_flag;
  gchar *trdb_label_text;
  GtkWidget *trdb_search_label;
  gchar *trdb_search_text;
  guint trdb_search_i;
  guint trdb_search_iobj[MAX_OBJECT];
  guint trdb_search_imax;
  gint trdb_used;
  gint trdb_da;
  gint trdb_arcmin;
  gint trdb_arcmin_used;

  gint trdb_smoka_inst;
  gint trdb_smoka_inst_used;
  gchar *trdb_smoka_date;
  gchar *trdb_smoka_date_used;
  gboolean trdb_smoka_shot;
  gboolean trdb_smoka_shot_used;
  gboolean trdb_smoka_imag;
  gboolean trdb_smoka_imag_used;
  gboolean trdb_smoka_spec;
  gboolean trdb_smoka_spec_used;
  gboolean trdb_smoka_ipol;
  gboolean trdb_smoka_ipol_used;

  gint trdb_hst_mode;
  gint trdb_hst_mode_used;
  gchar *trdb_hst_date;
  gchar *trdb_hst_date_used;
  gint trdb_hst_image;
  gint trdb_hst_image_used;
  gint trdb_hst_spec;
  gint trdb_hst_spec_used;
  gint trdb_hst_other;
  gint trdb_hst_other_used;

  gint trdb_eso_mode;
  gint trdb_eso_mode_used;
  gchar *trdb_eso_stdate;
  gchar *trdb_eso_stdate_used;
  gchar *trdb_eso_eddate;
  gchar *trdb_eso_eddate_used;
  gint trdb_eso_image;
  gint trdb_eso_image_used;
  gint trdb_eso_spec;
  gint trdb_eso_spec_used;
  gint trdb_eso_vlti;
  gint trdb_eso_vlti_used;
  gint trdb_eso_pola;
  gint trdb_eso_pola_used;
  gint trdb_eso_coro;
  gint trdb_eso_coro_used;
  gint trdb_eso_other;
  gint trdb_eso_other_used;
  gint trdb_eso_sam;
  gint trdb_eso_sam_used;

  gint trdb_gemini_inst;
  gint trdb_gemini_inst_used;
  gint trdb_gemini_mode;
  gint trdb_gemini_mode_used;
  gchar *trdb_gemini_date;
  gchar *trdb_gemini_date_used;

  gint addobj_type;
  gchar *addobj_name;
  gchar *addobj_voname;
  gchar *addobj_votype;
  gdouble addobj_ra;
  gdouble addobj_dec;
  gdouble addobj_pm_ra;
  gdouble addobj_pm_dec;
  gchar *addobj_magsp;
  GtkWidget *addobj_label;
  GtkWidget *addobj_entry_ra;
  GtkWidget *addobj_entry_dec;
  GtkWidget *addobj_entry_pm_ra;
  GtkWidget *addobj_entry_pm_dec;

  gboolean orbit_flag;
  gint nst_max;

  GtkWidget *mode_frame;
  GtkWidget *mode_label;

  gboolean magdb_ow;
  gboolean magdb_pm;
  gboolean magdb_skip;
  gint magdb_arcsec;
  gint magdb_mag;
  gint magdb_gsc_band;
  gint magdb_ps1_band;
  gint magdb_sdss_band;
  gint magdb_2mass_band;
  gint magdb_simbad_band;

  // IRCS
  GtkWidget* ircs_vbox;

  guint ircs_mode;

  gdouble ircs_exp;
  GtkAdjustment *ircs_exp_adj;
  
  guint ircs_im_mas;
  guint ircs_im_band[NUM_IRCS_MAS];
  guint ircs_im_dith;
  gdouble ircs_im_dithw;
  gint ircs_im_osra;
  gint ircs_im_osdec;
  GtkWidget *ircs_im_label[NUM_IRCS_MAS];

  guint ircs_pi_mas;
  guint ircs_pi_band[NUM_IRCS_MAS];
  guint ircs_pi_dith;
  gdouble ircs_pi_dithw;
  gint ircs_pi_osra;
  gint ircs_pi_osdec;
  GtkWidget *ircs_pi_label[NUM_IRCS_MAS];

  guint ircs_gr_mas;
  guint ircs_gr_band[NUM_IRCS_MAS];
  guint ircs_gr_slit[NUM_IRCS_MAS];
  guint ircs_gr_dith;
  gdouble ircs_gr_dithw;
  gint ircs_gr_osra;
  gint ircs_gr_osdec;
  gdouble ircs_gr_sssep;
  gint ircs_gr_ssnum;
  GtkWidget *ircs_gr_label[NUM_IRCS_MAS];
  GtkWidget *ircs_gr_label2[NUM_IRCS_MAS];

  guint ircs_ps_mas;
  guint ircs_ps_band[NUM_IRCS_MAS];
  guint ircs_ps_slit[NUM_IRCS_MAS];
  guint ircs_ps_dith;
  gdouble ircs_ps_dithw;
  gint ircs_ps_osra;
  gint ircs_ps_osdec;
  gdouble ircs_ps_sssep;
  gint ircs_ps_ssnum;
  GtkWidget *ircs_ps_label[NUM_IRCS_MAS];
  GtkWidget *ircs_ps_label2[NUM_IRCS_MAS];

  guint ircs_ec_mas;
  guint ircs_ecd_band;
  guint ircs_ecd_slit;
  guint ircs_ecm_band;
  guint ircs_ecm_slit;
  gint ircs_ecm_ech;
  gint ircs_ecm_xds;
  guint ircs_ec_dith;
  gdouble ircs_ec_dithw;
  gint ircs_ec_osra;
  gint ircs_ec_osdec;
  gdouble ircs_ec_sssep;
  gint ircs_ec_ssnum;
  GtkWidget *ircs_ec_label[NUM_IRCS_EC];

  IRCSpara ircs_set[IRCS_MAX_SET];
  guint ircs_i;
  guint ircs_i_max;

  GtkWidget *ircs_tree;

  gboolean ircs_magdb_skip;
  gdouble ircs_magdb_mag_ngs;
  gdouble ircs_magdb_mag_ttgs;
  guint ircs_magdb_r_tgt;
  guint ircs_magdb_r_ngs;
  guint ircs_magdb_r_ttgs;
  gboolean ircs_magdb_dse;
  gdouble  ircs_magdb_dse_mag;
  gdouble  ircs_magdb_dse_r1;
  gdouble  ircs_magdb_dse_r2;

  gint hsc_magdb_arcmin;
  
  gchar* lgs_sa_name;
  gchar* lgs_sa_email;
  gint   lgs_sa_phone1;
  gint   lgs_sa_phone2;
  gint   lgs_sa_phone3;

  LGS_PAM_Entry lgs_pam[MAX_LGS_PAM];
  struct ln_zonedate pam_zonedate;
  gint lgs_pam_i_max;
  gchar *pam_name;
  GtkWidget* pam_main;
  GtkWidget *pam_tree;
  GtkWidget *pam_label_obj;
  GtkWidget *pam_label_pam;
  gint pam_slot_i;
  gint pam_obj_i;
  gint pam_x[MAX_LGS_PAM_TIME];
  gint pam_y[MAX_LGS_PAM_TIME];
  

  // HSC
  GtkWidget* hsc_vbox;
  GtkWidget* hsc_frame_5dith;
  GtkWidget* hsc_frame_ndith;
  GtkWidget* hsc_e_frame_5dith;
  GtkWidget* hsc_e_frame_ndith;
  GtkWidget *hsc_tree;
  GtkWidget *hscfil_tree;
  GtkWidget* hsc_label_filter_ver;

  gint hsc_filter;
  gint hsc_dith;
  gint hsc_dith_ra;
  gint hsc_dith_dec;
  gint hsc_dith_n;
  gint hsc_dith_r;
  gint hsc_dith_t;
  gboolean hsc_ag;
  gint hsc_osra;
  gint hsc_osdec;
  gdouble hsc_exp;

  HSCpara hsc_set[HSC_MAX_SET];
  guint hsc_i;
  guint hsc_i_max;
};


// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_obj;
}confPA;



// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_use;
  GtkAdjustment *length_adj;
  GtkWidget *fil1_combo;
  GtkWidget *fil2_combo;
}confSetup;

// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_use;
}confEdit;

// Struct for Callback
typedef struct{
  GtkWidget *dialog;
  gint mode;
}confProp;

// Struct for Callback
typedef struct{
  GtkWidget *dialog;
  gint mode;
  GSList *fcdb_group;
  gint fcdb_type;
}confPropFCDB;


#define is_num_char(c) ((c>='0' && c<='9')||(c==' ')||(c=='\t')||(c=='.')||(c=='+')||(c=='-')||(c=='\n'))

static const gchar* cal_month[]={"Jan",
				 "Feb",
				 "Mar",
				 "Apr",
				 "May",
				 "Jun",
				 "Jul",
				 "Aug",
				 "Sep",
				 "Oct",
				 "Nov",
				 "Dec"};

#ifdef USE_GTK3
static GdkRGBA color_comment = {0.87, 0.00, 0.00, 1};
static GdkRGBA color_focus =   {0.53, 0.27, 0.00, 1};
static GdkRGBA color_calib =   {0.00, 0.53, 0.00, 1};
static GdkRGBA color_black =   {0.00, 0.00, 0.00, 1};
static GdkRGBA color_red   =   {1.00, 0.00, 0.00, 1};
static GdkRGBA color_blue =    {0.00, 0.00, 1.00, 1};
static GdkRGBA color_white =   {1.00, 1.00, 1.00, 1};
static GdkRGBA color_gray1 =   {0.40, 0.40, 0.40, 1};
static GdkRGBA color_gray2 =   {0.80, 0.80, 0.80, 1};
static GdkRGBA color_pink =    {1.00, 0.40, 0.40, 1};
static GdkRGBA color_pink2 =   {1.00, 0.80, 0.80, 1};
static GdkRGBA color_pale =    {0.40, 0.40, 1.00, 1};
static GdkRGBA color_pale2 =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_orange =  {1.00, 0.80, 0.40, 1};
static GdkRGBA color_orange2 = {1.00, 1.00, 0.80, 1};
static GdkRGBA color_green  =  {0.40, 0.80, 0.80, 1};
static GdkRGBA color_green2 =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_purple2 = {1.00, 0.80, 1.00, 1};
static GdkRGBA color_com1 =    {0.00, 0.53, 0.00, 1};
static GdkRGBA color_com2 =    {0.73, 0.53, 0.00, 1};
static GdkRGBA color_com3 =    {0.87, 0.00, 0.00, 1};
#else
static GdkColor color_comment = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_focus = {0, 0x8888, 0x4444, 0x0000};
static GdkColor color_calib = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_black = {0, 0, 0, 0};
static GdkColor color_red   = {0, 0xFFFF, 0, 0};
static GdkColor color_blue = {0, 0, 0, 0xFFFF};
static GdkColor color_white = {0, 0xFFFF, 0xFFFF, 0xFFFF};
static GdkColor color_gray1 = {0, 0x6666, 0x6666, 0x6666};
static GdkColor color_gray2 = {0, 0xBBBB, 0xBBBB, 0xBBBB};
static GdkColor color_pink = {0, 0xFFFF, 0x6666, 0x6666};
static GdkColor color_pink2 = {0, 0xFFFF, 0xCCCC, 0xCCCC};
static GdkColor color_pale = {0, 0x6666, 0x6666, 0xFFFF};
static GdkColor color_pale2 = {0, 0xCCCC, 0xCCCC, 0xFFFF};
static GdkColor color_orange = {0, 0xFFFF, 0xCCCC, 0x6666};
static GdkColor color_orange2 = {0, 0xFFFF, 0xFFFF, 0xCCCC};
static GdkColor color_green = {0, 0x6666, 0xCCCC, 0x6666};
static GdkColor color_green2 = {0, 0xCCCC, 0xFFFF, 0xCCCC};
static GdkColor color_purple2 = {0, 0xFFFF, 0xCCCC, 0xFFFF};
static GdkColor color_com1 = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_com2 = {0, 0xBBBB, 0x8888, 0x0000};
static GdkColor color_com3 = {0, 0xDDDD, 0x0000, 0x0000};
#endif

////////////////////// Global Args //////////////////////
gboolean flagChildDialog;
gboolean flagSkymon;
gboolean flagPlot;
gboolean flagFC;
gboolean flagPlan;
gboolean flagPAM;
gboolean flag_getFCDB;
gboolean flag_make_obj_tree;
gboolean flag_make_line_tree;
gboolean flag_make_etc_tree;
int debug_flg;

#ifndef USE_WIN32
pid_t fc_pid;
#endif
pid_t fcdb_pid;
pid_t stddb_pid;


////////////////////// Proto types () //////////////////////
// main.c
#ifdef USE_GTK3
void css_change_col();
void css_change_pbar_height();
#endif
gchar* fgets_new();
gboolean is_separator();
#ifdef USE_WIN32
gchar* my_dirname();
gchar* get_win_home();
gchar* get_win_temp();
#endif
gchar* get_home_dir();
#ifndef USE_WIN32
void ChildTerm();
#endif
void ext_play();
void uri_clicked();
gchar* get_band_name();
void WritePass();
void init_obj();
void init_obj_magdb();
void init_inst();
void param_init();
gchar *cut_spc();
gchar *strip_spc();
gchar *make_tgt();
gdouble read_radec();
gint get_same_rb();
gint get_nonstd_flat();
void usage();
void get_option();
gchar* to_utf8();
gchar* to_locale();
gboolean is_number();
void popup_message(GtkWidget*, gchar*, gint , ...);
gboolean popup_dialog(GtkWidget*, gchar*, ...);
gboolean delete_disp_para();
void close_disp_para();
void default_disp_para();
void change_disp_para();
gboolean close_popup();
gboolean destroy_popup();
void my_signal_connect();
void my_entry_set_width_chars();
gchar* check_ext();
gchar* make_head();
GtkWidget * gtkut_hbox_new();
GtkWidget * gtkut_vbox_new();
GtkWidget * gtkut_table_new();
void gtkut_table_attach();
void gtkut_table_attach_defaults();
#ifdef USE_GTK3
GtkWidget * gtkut_button_new_from_icon_name();
GtkWidget * gtkut_toggle_button_new_from_icon_name();
GtkWidget * gtkut_image_menu_item_new_with_label();;
#else
GtkWidget * gtkut_button_new_from_stock();
GtkWidget * gtkut_toggle_button_new_from_stock();
#endif
GtkWidget * gtkut_button_new_from_pixbuf();
GtkWidget * gtkut_toggle_button_new_from_pixbuf();
#ifdef USE_WIN32
gchar* WindowsVersion();
#endif
void calc_rst();
void do_null();
void ver_txt_parse();
void ver_dl();
void CheckVer();
void RecalcRST();
void recalc_rst();
void update_objtree();


// calcpa.c
void calcpa2_main();
void calcpa2_skymon();
void calcpa2_plan();

void calc_moon();
void calc_moon_skymon();
void calc_sun_plan();
typPlanMoon init_typPlanMoon();
typPlanMoon calc_typPlanMoon();

gboolean draw_plot_cairo();

void refresh_plot();
void pdf_plot();

void create_plot_dialog();

void add_day();
gdouble get_julian_day_of_epoch();

void geocen_to_topocen();

gdouble ra_to_deg();
gdouble dec_to_deg();
gdouble deg_to_ra();
gdouble deg_to_dec();

gdouble deg_sep();
gdouble date_to_jd();


// edit.c
void create_opedit_dialog();

// efs.c
void go_efs();
void pdf_efs();

// etc.c
Crosspara get_cross_angle();
gint get_wcent();
void etc_main();
gdouble etc_obj();

// etctree.c
void rebuild_etc_tree();
void etc_append_tree();


// fc.c
gdouble current_yrs();
void fc_item();
void fc_item_trdb();
void fc_item_plan();
void fcdb_para_item();
gboolean progress_timeout();
void create_fc_all_dialog();
gboolean draw_fc_cairo();
void set_dss_arcmin_upper();
void set_fc_mode();
void cc_get_fc_mode0();
void pdf_fc();


//fc_output.c
void Export_FCDB_CSV();
void Export_TRDB_CSV();

// http_client.c
int ftp_c();
int scp_c();
int get_dss();
int get_stddb();
int get_fcdb();
int month_from_string_short();

// json_parse.c
void fcdb_gemini_json_parse();
void trdb_gemini_json_parse();

// julian_day.c
void my_get_local_date();
int get_gmtoff_from_sys ();

// line_tree.c
void make_line_tree();
void linetree_init();
void linetree_nebula();
void linetree_star();
void linetree_highz();

// objtree.c
void get_total_basic_exp();
void objtree_update_radec();
void make_obj_tree();
void objtree_update_item();
void add_item_objtree();
void up_item_objtree();
void down_item_objtree();
void remove_item_objtree();
void wwwdb_item();
void do_update_exp();
void hds_export_def ();
void ircs_export_def ();
void hsc_export_def ();
void plot2_objtree_item();
void etc_objtree_item();
void pm_objtree_item();
void pam_objtree_item();
void addobj_dialog();
void str_replace();
gchar *make_simbad_id();
void cc_search_text();
void search_item();
void update_c_label();
void strchg();


// plan.c 
void create_plan_dialog();
gchar * get_txt_tod();
gchar * make_plan_txt();
void remake_sod();
void plan_check_consistency();
void init_planpara();
void refresh_tree();

// skymon.c
void create_skymon_dialog();
gboolean draw_skymon_cairo();
void pdf_skymon();
void skymon_set_and_draw();
void skymon_set_time_current();
void get_current_obs_time();
void refresh_skymon();

// stdtree.c
void stddb_item();
void stddb_toggle();
void std_make_tree();
void std_add_columns();
GtkTreeModel *std_create_items_model ();
void std_focus_item ();
void std_simbad ();
void add_item_std();
void create_std_para_dialog();
void make_std_tgt();
void stddb_set_label();

// fcdbtree.c
void fcdb_dl();
gboolean delete_fcdb();
void cancel_fcdb();
void fcdb_make_tree();
#ifndef USE_WIN32
void fcdb_signal();
#endif
void fcdb_item();
GtkTreeModel * fcdb_create_items_model();
void fcdb_add_columns();
void fcdb_clear_tree();
void rebuild_fcdb_tree();
void fcdb_append_tree();
void fcdb_simbad();
void add_item_fcdb();
void add_item_gs();
void make_fcdb_tgt();
gchar* fcdb_csv_name();

// trdbtree.c
void trdb_smoka();
void trdb_hst();
void trdb_eso();
void trdb_gemini();
void trdb_simbad();
void trdb_append_tree();
void trdb_dbtab();
void trdb_cc_search_text();
void trdb_search_item();
void rebuild_trdb_tree();
void make_trdb_label();
void trdb_clear_tree();
void trdb_make_tree();
void fcdb_to_trdb();
gchar* trdb_csv_name();

// magdb.c
void magdb_gsc();
void ircs_magdb_gsc();
void magdb_ps1();
void ircs_magdb_ps1();
void magdb_sdss();
void magdb_gaia();
void ircs_magdb_gaia();
void magdb_kepler();
void magdb_2mass();
void magdb_simbad();
void hsc_magdb_simbad();
void magdb_ned();
void magdb_lamost();


// votable.c
void make_band_str();
void fcdb_simbad_vo_parse();
void fcdb_hsc_simbad_vo_parse();
void fcdb_ned_vo_parse();
void fcdb_gsc_vo_parse();
void fcdb_ircs_gsc_vo_parse();
void fcdb_ps1_vo_parse();
void fcdb_ircs_ps1_vo_parse();
void fcdb_sdss_vo_parse();
void fcdb_usno_vo_parse();
void fcdb_gaia_vo_parse();
void fcdb_ircs_gaia_vo_parse();
void fcdb_kepler_vo_parse();
void fcdb_2mass_vo_parse();
void fcdb_wise_vo_parse();
void fcdb_irc_vo_parse();
void fcdb_fis_vo_parse();
void fcdb_lamost_vo_parse();
void fcdb_smoka_txt_parse();
void trdb_smoka_txt_parse();
void fcdb_hst_vo_parse();
void trdb_hst_vo_parse();
void fcdb_eso_vo_parse();
void trdb_eso_vo_parse();
void addobj_vo_parse();
void stddb_vo_parse();
void camz_txt_parse();
void ircs_gs_selection();

// scp-client.c
int scp_write();
int scp_get();
