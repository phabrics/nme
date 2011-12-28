## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator scsi/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

##SUBDIRS = @TME_SCSI_SUBDIRS@
##DIST_SUBDIRS = 

pkglib_LTLIBRARIES = tme_scsi.la

tme_scsi_la_SOURCES = \
	acb4000.c \
	emulexmt02.c \
	disk-tme.c \
	tape-tme.c \
	cdrom-tme.c \
	scsi-disk.c \
	scsi-tape.c \
	scsi-cdrom.c \
	scsi-cdb.c \
	scsi-msg.c \
	scsi-device.c \
	scsi-bus.c
tme_scsi_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_scsi_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la 

include $(top_srcdir)/modules
