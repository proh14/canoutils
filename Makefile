/ := ./
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

BINARIES := $(foreach b, $(BINS), src/$b/$b)
BINS-COPY := $(foreach b, $(BINS), bins/$b)

define make-rule
@ $(MAKE) -C $(1) $(2) --no-print-directory /=$(realpath .)/

endef

prop = $(foreach b, $(BINARIES), $(call make-rule, $(dir $b), $(1)))

.PHONY: all
all: $(BINS-COPY)

$(BINS-COPY): $(BINARIES)
	@ mkdir -p $(dir $@)
	$Q cp $< $@ 

$(BINARIES):
	$(call make-rule, $(dir $@), $(notdir $@))

.PHONY: clean
clean:
	$(call prop, clean)
	@ $(LOG_TIME) $@

.PHONY: fclean
fclean:
	$(call prop, fclean)
	$(RM) -r bins
	@ $(LOG_TIME) $@

.PHONY: re
.NOTPARALLEL: re
re: fclean all
