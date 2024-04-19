CC = gcc

CFLAGS := @$/compile_flags.txt

CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wp,-U_FORTIFY_SOURCE
CFLAGS += -iquote $/src

LDFLAGS := -fwhole-program -flto
LDFLAGS += -Wl,--gc-sections

BUILD_DIR := $/.build

LDFLAGS := -fwhole-program -flto

OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)

.PHONY: all
all: $(OUT)

-include $(DEP)

$(OUT): $(OBJ)
	@ mkdir -p $(dir $@)
	$Q $(CC) -o $@ $(OBJ) $(CFLAGS) $(LDLIBS) $(LDFLAGS)
	@ $(LOG_TIME) "LD $(C_GREEN) $@ $(C_RESET)"

$(BUILD_DIR)/%.o: %.c
	@ mkdir -p $(dir $@)
	$Q $(CC) $(CFLAGS) -o $@ -c $< || exit 1
	@ $(LOG_TIME) "CC $(C_PURPLE) $(notdir $@) $(C_RESET)"

.PHONY: clean
clean:
	$(RM) $(OBJ)

.PHONY: fclean
fclean:
	$(RM) -r $(OBJ) $(BUILD_DIR) $(OUT)

.PHONY: re
.NOTPARALLEL: re
re: fclean
	@ $(MAKE) all
