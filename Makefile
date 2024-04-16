include base.mk

BINS := date
BINS += expr
BINS += pwd
BINS += md5
BINS += kill
BINS += hostname
BINS += df
BINS += chio
BINS += ps
BINS += rm
BINS += echo
BINS += mkdir
BINS += rmdir
BINS += ed
BINS += ls
BINS += dd
BINS += domainname
BINS += cp
BINS += cat
BINS += ln
BINS += mv
BINS += chmod
BINS += sh

MK-BIN-PATH = src/$(strip $(1))/$(strip $(1))
BINARIES := $(foreach b, $(BINS), $(call MK-BIN-PATH, $b))

define make-rule
@ $(MAKE) -C $(1) $(2) --no-print-directory

endef

prop = $(foreach b, $(BINARIES), $(call make-rule, $(dir $b), $(1)))

.PHONY: all
all: $(BINARIES)

$(BINARIES):
	$(call make-rule, $(dir $@), $(notdir $@))

.PHONY: clean
clean:
	$(call prop, clean)
	@ $(LOG_TIME) $@

.PHONY: fclean
fclean:
	$(call prop, fclean)
	@ $(LOG_TIME) $@

.PHONY: re
re:
	$(call prop, re)
