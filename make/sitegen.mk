DIST_DIR   := out/dist
PAGE_FILES := $(shell find $(PAGE_DIR) -type f ! -name '*.yate')
DIST_FILES := $(patsubst $(PAGE_DIR)/%,$(DIST_DIR)/%,$(PAGE_FILES))
POST_SRCS  := $(wildcard content/*.post)
POST_PAGES := $(patsubst content/%.post,$(DIST_DIR)/posts/%/index.html,$(POST_SRCS))

SITEGEN_SRC := $(SRC_DIR)/tools/sitegen/main.c

SITEGEN_NAVBAR_OBJ := $(OBJ_ROOT_DIR)/src/tools/sitegen/generic.o
SITEGEN_INDEX_OBJ  := $(OBJ_ROOT_DIR)/src/tools/sitegen/index.o
SITEGEN_POST_OBJ   := $(OBJ_ROOT_DIR)/src/tools/sitegen/post.o

SITEGEN_NAVBAR_DEP := $(DEP_ROOT_DIR)/src/tools/sitegen/generic.d
SITEGEN_INDEX_DEP  := $(DEP_ROOT_DIR)/src/tools/sitegen/index.d
SITEGEN_POST_DEP   := $(DEP_ROOT_DIR)/src/tools/sitegen/post.d

SITEGEN_NAVBAR := $(BIN_DIR)/sitegen-generic
SITEGEN_INDEX  := $(BIN_DIR)/sitegen-index
SITEGEN_POST   := $(BIN_DIR)/sitegen-post

SITEGEN_DEPS := \
	$(DEP_ROOT_DIR)/src/tools/sitegen/index.d \
	$(DEP_ROOT_DIR)/src/tools/sitegen/post.d

.PHONY: dist serve

tools: $(SITEGEN_INDEX) $(SITEGEN_POST)
dist: $(DIST_FILES) $(DIST_DIR)/index.html $(POST_PAGES)

serve: dist
	$(PY) -m http.server 8000 --directory $(DIST_DIR)
dev:
	$(PY) $(SCRIPTS_DIR)/dev-server.py $(BUILD)

$(GEN_DIR)/%.h: $(PAGE_DIR)/%.yate $(BIN_DIR)/yate
	@mkdir -p $(dir $@)
	$(BIN_DIR)/yate -i $< -o $@

$(DIST_DIR)/%: $(PAGE_DIR)/%
	@mkdir -p $(dir $@)
	cp $< $@

$(DIST_DIR)/index.html: $(SITEGEN_INDEX) $(GEN_DIR)/index.html.h $(POST_PAGES)
	@mkdir -p $(dir $@)
	$(SITEGEN_INDEX) -i content -o $@

$(DIST_DIR)/posts/%/index.html: content/%.post $(SITEGEN_POST) $(GEN_DIR)/post.html.h
	@mkdir -p $(dir $@)
	$(SITEGEN_POST) -i $< -o $@

$(SITEGEN_INDEX): $(SITEGEN_INDEX_OBJ) $(BLOG_LIB) $(ARGPARSE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(SITEGEN_POST): $(SITEGEN_POST_OBJ) $(BLOG_LIB) $(ARGPARSE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(SITEGEN_NAVBAR_OBJ): $(SITEGEN_SRC) $(GEN_DIR)/navbar.html.h
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(SITEGEN_NAVBAR_DEP))
	$(CC) $(CFLAGS) -DMODE=1 -DTEMPLATE_HEADER=\"navbar.html.h\" -DBASE_URL='"$(BASE_URL)"' \
		-MMD -MP -MF $(SITEGEN_NAVBAR_DEP) -c $< -o $@

$(SITEGEN_INDEX_OBJ): $(SITEGEN_SRC) $(GEN_DIR)/index.html.h $(GEN_DIR)/navbar.html.h
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(SITEGEN_INDEX_DEP))
	$(CC) $(CFLAGS) -DMODE=1 -DTEMPLATE_HEADER=\"index.html.h\" -DBASE_URL='"$(BASE_URL)"' \
		-MMD -MP -MF $(SITEGEN_INDEX_DEP) -c $< -o $@

$(SITEGEN_POST_OBJ): $(SITEGEN_SRC) $(GEN_DIR)/post.html.h $(GEN_DIR)/navbar.html.h
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(SITEGEN_POST_DEP))
	$(CC) $(CFLAGS) -DMODE=2 -DTEMPLATE_HEADER=\"post.html.h\" -DBASE_URL='"$(BASE_URL)"' \
		-MMD -MP -MF $(SITEGEN_POST_DEP) -c $< -o $@
