bin_PROGRAMS    = hoe
SUBDIRS = . libnova
hoe_SOURCES   =\
	 main.c efs.c etc.c calcpa.c edit.c plan.c objtree.c linetree.c stdtree.c fcdbtree.c trdbtree.c etctree.c magdb.c lunar.c solar.c earth.c rise_set.c sidereal_time.c vsop87.c julian_day.c utility.c transform.c nutation.c dynamical_time.c precession.c http-client.c fc.c main.h configfile.c skymon.c votable.c json_parse.c fc_output.c mercury.c venus.c mars.c jupiter.c saturn.c uranus.c neptune.c pluto.c votable.h configfile.h hoe_icon.h ssl.h hsc.h std.h post.h post_sdss.h post_lamost.h post_smoka.h post_hst.h post_eso.h get_gemini.h star_template.h efs_icon.h etc_icon.h sky_icon.h

CLEANFILES      = *~ libnova/*~
DISTCLEANFILES  = .deps/*.P
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"

if HAVE_WINDRES
HOE_RES = hoe.res
hoe.res: hoe_res.rc
	$(WINDRES) $< -O coff -o $@
else
HOE_RES = 
endif

EXTRA_DIST = autogen.sh\
	     README.jp\
	     standard.list\
	     sample.list\
	     samplePS.list\
             hoe.ico\
             hoe_res.rc.in\
             hoe_icon48.png\
             makewin32.sh\
             makewin64.sh\
             makeosx.sh\
	     hoe.spec

INCLUDES        = @GTK_CFLAGS@ $(GTKMAC_CFLAGS) $(DISABLE_DEPRECATED) $(XML_CPPFLAGS)
LDADD           = @GTK_LIBS@ $(GTKMAC_LIBS) $(WINMM_LIBS) $(WINSOCK_LIBS) $(XML_LIBS) $(HOE_RES)


GZIP_ENV =
BZIP2_ENV =
TAR=tar

#ACLOCAL_AMFLAGS = -l m4

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	BZIP2=$(BZIP2_ENV) $(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

