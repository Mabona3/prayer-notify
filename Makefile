-include config.mk

all: CFLAGS+=-g -DDebug -fsanitize=address,undefined -fno-omit-frame-pointer
all: $(MAIN)

release: CFLAGS+=-s -O2
release: $(MAIN)

deps:
	echo $(DEPS_OBJ)
	echo $(DEPS_LINK)
	echo $(DEPS_SRC)

$(MAIN): $(OBJ_FILES) $(DEPS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(INC_FLAGS) $(DEPS_LIB) $(DEPS_LINK)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_FLAGS)

$(DEPS_OBJ): 
	echo $(dir $(patsubst %/,%,$(dir $@)))
	echo $@
	@$(MAKE) -C $(dir $(patsubst %/,%,$(dir $@)))

run: $(MAIN)
	@./$(MAIN)

debug: $(MAIN)
	@$(DEBUGGER) ./$(MAIN)

install: $(MAIN)
	@mkdir -p $(DIST_DIR)$(BINDIR)
	@mkdir -p $(DIST_DIR)$(SERVICEDIR)
	@install -Dm544 $(SERVICE) $(DIST_DIR)$(SERVICEDIR)
	@install -Dm755 $(MAIN) $(DIST_DIR)$(BINDIR)

uninstall:
	@rm -f $(BINDIR)/$(NAME)
	@rm -f $(SERVICEDIR)/$(SERVICE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run debug test install uninstall release
