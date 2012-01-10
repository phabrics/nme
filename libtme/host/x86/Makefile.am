## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/host/x86:

AUTOMAKE_OPTIONS = 1.4 gnu

EXTRA_DIST = recode-x86.c \
	rc-x86-flags.c \
	rc-x86-conds.c \
	rc-x86-tlb.c \
	rc-x86-chain.c \
	rc-x86-rws.c \
	rc-x86-insns.c \
	rc-x86-regs.c \
	rc-x86-subs-asm.S \
	rc-x86-subs.c \
	misc-x86.c

make-rc-x86-subs.c:
	echo '/* automatically generated, do not edit! */' >> $@
	echo '' >> $@
	echo -n '#if TME_RECODE_SIZE_HOST ' >> $@
	if test `uname -p` = i386; then echo -n '==' >> $@; else echo -n '>' >> $@; fi
	echo ' TME_RECODE_SIZE_32' >> $@
	echo '' >> $@
	gcc -o rc-x86-subs-asm.o -c rc-x86-subs-asm.S
	echo '/* offsets in the hand-coded '`uname -p`' x86 recode subs: */' >> $@
	nm rc-x86-subs-asm.o | \
	  awk '/ T / { print "#define " $$3 "\t(0x" $$1 ")" }' >> $@
	echo '' >> $@
	echo '/* the hand-coded '`uname -p`' x86 recode subs: */' >> $@
	objcopy --output-target=binary rc-x86-subs-asm.o rc-x86-subs-asm.bin
	echo 'static const tme_uint8_t _tme_recode_x86_subs[] = {' >> $@
	hexdump -e '16/1 "0x%02x, " "\n"' rc-x86-subs-asm.bin | \
	  sed -e 's/0x  ,/0x00,/g' >> $@
	echo '};' >> $@
	echo '' >> $@
	echo '#endif /* TME_RECODE_SIZE_HOST */' >> $@
