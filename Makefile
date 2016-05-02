include config.mk

all: debug

SRC := \
	client/client.c \
	client/client_io.c \
	gen/output/op_map.c \
	server/server.c \
	server/server_args.c \
	shared/asm/compiler.c \
	shared/asm/lex.c \
	shared/hash/hashmap.c \
	shared/socket/socket.c \
	shared/unicode/unihelpers.c \
	shared/vm/plugin.c \
	shared/vm/state.c \
	shared/vm/vm.c \
	shared/vm/vm_data.c


DEBUG_PATH := bin/debug
RELEASE_PATH := bin/release

SERVER_FILES := \
	server.o \
	server_args.o \
	socket.o \
	state.o \
	vm.o \
	plugin.o

CLIENT_FILES := \
	client.o \
	client_io.o \
	compiler.o \
	hashmap.o \
	lex.o \
	op_map.o \
	socket.o \
	unihelpers.o \
	vm_data.o

GENERATED_FILES := \
	$(DEBUG_PATH)/nitwit_server \
	$(DEBUG_PATH)/nitwit_client \
	$(RELEASE_PATH)/nitwit_server \
	$(RELEASE_PATH)/nitwit_client \
	$(DEBUG_PATH)/*.o \
	$(RELEASE_PATH)/*.o \
	gen/output/op_map.c \
	gen/programs/map_gen

server_deps := \
	shared/socket/socket.h \
	shared/vm/vm.h

client_deps := \
	client/client_io.h \
	shared/asm/compiler.h \
	shared/socket/socket.h \
	shared/unicode/unihelpers.h \
	shared/vm/vm.h

op_map := \
	gen/input/op_map_closer.c \
	gen/input/op_map_opener.c \
	gen/output/op_map.h \
	gen/programs/bin/map_gen

map_gen := \
	gen/programs/map_gen.c \
	shared/hash/hashmap.c \
	shared/hash/hashmap.h \
	shared/vm/vm.h

debug: $(DEBUG_PATH)/nitwit_server $(DEBUG_PATH)/nitwit_client
release: $(RELEASE_PATH)/nitwit_server $(RELEASE_PATH)/nitwit_client
gen: gen/programs/bin/map_gen

gen/output/op_map.c: $(op_map)
	@(cd "gen/output" && ../programs/bin/map_gen);

gen/programs/bin/map_gen: $(map_gen)
	@gcc -o gen/programs/bin/map_gen gen/programs/map_gen.c \
	shared/hash/hashmap.c -lunistring

# Makes the executable programs (server and client)
define make-execs
# 1 is the executable name
# 2 is the .o files to build
# 3 is the libraries to link with
$(DEBUG_PATH)/$(1): $(addprefix $(DEBUG_PATH)/,$(2))
	@$(CC)  $(DEBUG_CFLAGS) $$^ -o $$@ $(3)
$(RELEASE_PATH)/$(1): $(addprefix $(RELEASE_PATH)/,$(2))
	@$(CC)  $(RELEASE_CFLAGS) $$^ -o $$@ $(3)
endef

$(eval $(call make-execs,nitwit_server, \
	$(SERVER_FILES),$(SERVER_LIBS)));
$(eval $(call make-execs,nitwit_client, \
	$(CLIENT_FILES),$(CLIENT_LIBS)));

# Makes the object files (.c -> .o)
define make-objs
# 1 is the .c file
# looks at deps if there are more
$(eval x = $(patsubst %.c, %_deps, $(notdir $(1))))
$(patsubst %.c, $(DEBUG_PATH)/%.o, $(notdir $(1))): $(1) $($(x)) \
	$(wildcard $(patsubst %.c, %.h, $(1))) # header check
	@$(CC) $(DEBUG_CFLAGS) -c $$< -o $$@
$(patsubst %.c, $(RELEASE_PATH)/%.o, $(notdir $(1))): $(1) $($(x)) \
	$(wildcard $(patsubst %.c, %.h, $(1)))
	@$(CC) $(RELEASE_CFLAGS) -c $$< -o $$@
endef

$(foreach src,$(SRC),$(eval $(call make-objs,$(src))));

# Removes the generated files, duh
clean:
	@rm -f $(GENERATED_FILES)

.PHONY: clean
