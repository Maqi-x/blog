DIST_DIR   := out/dist
PAGE_FILES := $(shell find $(PAGE_DIR) -type f ! -name '*.yate')
DIST_FILES := $(patsubst $(PAGE_DIR)/%,$(DIST_DIR)/%,$(PAGE_FILES))
POST_SRCS  := $(wildcard content/*.post)
POST_PAGES := $(patsubst content/%.post,$(DIST_DIR)/posts/%.html,$(POST_SRCS))

SITEGEN_SRC       := $(SRC_DIR)/tools/sitegen/main.c
SITEGEN_INDEX_OBJ := $(OBJ_ROOT_DIR)/src/tools/sitegen/index.o
SITEGEN_POST_OBJ  := $(OBJ_ROOT_DIR)/src/tools/sitegen/post.o
SITEGEN_INDEX     := $(BIN_DIR)/sitegen-index
SITEGEN_POST      := $(BIN_DIR)/sitegen-post

SITEGEN_DEPS      := \
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

$(DIST_DIR)/posts/%.html: content/%.post $(SITEGEN_POST) $(GEN_DIR)/post.html.h
	@mkdir -p $(dir $@)
	$(SITEGEN_POST) -i $< -o $@

$(SITEGEN_INDEX): $(SITEGEN_INDEX_OBJ) $(BLOG_LIB) $(ARGPARSE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(SITEGEN_POST): $(SITEGEN_POST_OBJ) $(BLOG_LIB) $(ARGPARSE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(SITEGEN_INDEX_OBJ): $(SITEGEN_SRC) $(GEN_DIR)/index.html.h
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEP_ROOT_DIR)/src/tools/sitegen/index.d)
	$(CC) $(CFLAGS) -DRENDER_MODE=0 -DTEMPLATE_HEADER=\"index.html.h\" \
		-MMD -MP -MF $(DEP_ROOT_DIR)/src/tools/sitegen/index.d -c $< -o $@

$(SITEGEN_POST_OBJ): $(SITEGEN_SRC) $(GEN_DIR)/post.html.h
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEP_ROOT_DIR)/src/tools/sitegen/post.d)
	$(CC) $(CFLAGS) -DRENDER_MODE=1 -DTEMPLATE_HEADER=\"post.html.h\" \
		-MMD -MP -MF $(DEP_ROOT_DIR)/src/tools/sitegen/post.d -c $< -o $@
