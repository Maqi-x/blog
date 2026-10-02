CC ?= cc
AR ?= ar

BUILD ?= release

rwildcard = \
	$(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) \
	$(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

SRC_DIR     := src
DEPS_DIR    := deps
INCLUDE_DIR := include

OBJ_ROOT_DIR := build/$(BUILD)/obj
DEP_ROOT_DIR := build/$(BUILD)/dep

OUT_DIR  := out/$(BUILD)
LIB_DIR  := $(OUT_DIR)/lib
BIN_DIR  := $(OUT_DIR)/bin

BLOG_LIB := $(LIB_DIR)/libblog.a

CSTD     := -std=c11
WARNINGS := -Wall -Wextra -Werror=implicit-fallthrough

CFLAGS_COMMON := \
	$(CSTD) $(WARNINGS) \
	-I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/tools \
	-I$(DEPS_DIR)/strlib/src

ifeq ($(BUILD),debug)
	CFLAGS  := $(CFLAGS_COMMON) -Og -g -fsanitize=address,undefined
	LDFLAGS := -fsanitize=address,undefined
else ifeq ($(BUILD),release)
	CFLAGS  := $(CFLAGS_COMMON) -O3 -DNDEBUG
	LDFLAGS := -flto
else
	$(error Unknown BUILD=$(BUILD))
endif

BLOG_SRCS := $(call rwildcard,$(SRC_DIR)/blog,*.c)
BLOG_OBJS := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(BLOG_SRCS))

TOOL_DIRS  := $(sort $(dir $(wildcard $(SRC_DIR)/tools/*/)))
TOOL_NAMES := $(patsubst $(SRC_DIR)/tools/%/,%,$(TOOL_DIRS))
TOOLS_EXES := $(foreach tool,$(TOOL_NAMES),$(BIN_DIR)/$(tool))

.PHONY: all clean lib tools

all: lib tools
lib: $(BLOG_LIB)
tools: $(TOOLS_EXES)

$(BLOG_LIB): $(BLOG_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

define TOOL_RULE
TOOL_SRCS_$(1) := $$(call rwildcard,$(SRC_DIR)/tools/$(1),*.c)
TOOL_OBJS_$(1) := $$(patsubst %.c,$$(OBJ_ROOT_DIR)/%.o,$$(TOOL_SRCS_$(1)))

$$(BIN_DIR)/$(1): $$(TOOL_OBJS_$(1)) $(BLOG_LIB)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(TOOL_OBJS_$(1)) $(BLOG_LIB) $$(LDFLAGS) -o $$@
endef

$(foreach tool,$(TOOL_NAMES),$(eval $(call TOOL_RULE,$(tool))))

ALL_C_SRCS := $(BLOG_SRCS) $(foreach tool,$(TOOL_NAMES),$(TOOL_SRCS_$(tool)))
DEPS       := $(patsubst %.c,$(DEP_ROOT_DIR)/%.d,$(ALL_C_SRCS))

$(OBJ_ROOT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(DEP_ROOT_DIR)/$(dir $<)
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/$*.d -c $< -o $@

-include $(DEPS)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCDIR ?= $(PREFIX)/include

install: all
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(INCDIR)
	$(foreach tool,$(TOOL_NAMES),install -m755 $(BIN_DIR)/$(tool) $(DESTDIR)$(BINDIR)/$(tool);)
	install -m644 $(BLOG_LIB) $(DESTDIR)$(LIBDIR)/libblog.a
	cp -R $(INCLUDE_DIR)/blog $(DESTDIR)$(INCDIR)/

uninstall:
	$(foreach tool,$(TOOL_NAMES),rm -f $(DESTDIR)$(BINDIR)/$(tool);)
	rm -f $(DESTDIR)$(LIBDIR)/libblog.a
	rm -rf $(DESTDIR)$(INCDIR)/blog

clean:
	rm -rf build out
