## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/host/x86:

EXTRA_DIST = recode-x86.c \
	rc-x86-flags.c \
	rc-x86-conds.c \
	rc-x86-tlb.c \
	rc-x86-chain.c \
	rc-x86-rws.c \
	rc-x86-insns.c \
	rc-x86-regs.c \
	rc-x86-subs.S \
	misc-x86.c

BUILT_SOURCES = rc-x86-subs.c

MAINTAINERCLEANFILES = $(BUILT_SOURCES)

.S.o:
	$(CCAS) $(CCASFLAGS) -o $@ -c $<

.o.bin:
	$(OBJCOPY) --output-target=binary $< $@

.o.nm:
	$(NM) $< > $@

.o.dis:
	$(OBJDUMP) -d $< > $@

rc-x86-subs.c: rc-x86-subs.bin rc-x86-subs.nm rc-x86-subs.dis
	@echo '/* automatically generated, do not edit! */' > $@
	@echo '' >> $@
	@echo '/* offsets in the hand-coded x86 recode subs: */' >> $@
	$(AWK) '/ T / { print "#define " $$3 "\t(0x" $$1 ")" }' rc-x86-subs.nm >> $@
	@echo '' >> $@
	@echo 'static const tme_uint8_t _tme_recode_x86_subs[] = {' >> $@
	$(HEXDUMP) -e '16/1 "0x%02x, " "\n"' rc-x86-subs.bin | \
	$(SED) -e 's/0x  ,/0x00,/g' >> $@
	@echo '};' >> $@
