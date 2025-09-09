-include config.mk

all: CFLAGS+=-g -DDebug
all: $(MAIN)

release: CFLAGS+=-s -O3
release: $(MAIN)

$(MAIN): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(MAIN)
	@./$(MAIN)

debug: $(MAIN)
	@$(DEBUGGER) ./$(MAIN)

install: $(MAIN)
	@mkdir -p $(DIST_DIR)$(BINDIR)
	@mkdir -p $(DIST_DIR)$(SERVICEDIR)
	@install -m 544 $(SERVICE) $(DIST_DIR)$(SERVICEDIR)
	@install -m 755 $(MAIN) $(DIST_DIR)$(BINDIR)

uninstall:
	@rm -f $(BINDIR)/$(NAME)
	@rm -f $(SERVICEDIR)/$(SERVICE)

clean:
	rm -rf $(BUILD_DIR)

