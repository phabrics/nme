## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator scsi/:

AUTOMAKE_OPTIONS = 1.4 gnu

##SUBDIRS = @@PACKAGE@_@SUB_DIR@_SUBDIRS@
##DIST_SUBDIRS = 

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la

@PACKAGE@_@SUB_DIR@_la_SOURCES = \
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
@PACKAGE@_@SUB_DIR@_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk
