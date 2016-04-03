SRC_FILES = \
	server/server.c \
	server/server_args.c \
	client/client.c \
	client/client_io.c \
	shared/socket/socket.c \
	shared/practise/practise.c \
	shared/vm/vm.c \
	shared/vm/state.c \
	shared/vm/vm_data.c \
	shared/asm/asm_compiler.c \
	shared/unicode/unihelpers.c \
	shared/hashmap/hashmap.c \
	gen/output/op_map.c


DEBUG_PATH = bin/debug
DEBUG_FILES = $(patsubst %,$(DEBUG_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

RELEASE_PATH = bin/release
RELEASE_FILES = $(patsubst %,$(RELEASE_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

SERVER_FILES = \
	server.o \
	server_args.o \
	socket.o \
	vm.o \
	state.o \
	practise.o

CLIENT_FILES = \
	client.o \
	client_io.o \
	socket.o \
	asm_compiler.o \
	vm_data.o \
	unihelpers.o \
	hashmap.o \
	op_map.o

server.o = \
	shared/socket/socket.h \
	shared/vm/vm.h

client.o = \
	client/client_io.h \
	shared/socket/socket.h \
	shared/vm/vm.h \
	shared/asm/asm_compiler.h \
	shared/unicode/unihelpers.h

op_map = \
	gen/programs/bin/map_gen \
	gen/output/op_map.h \
	gen/input/op_map_opener.c \
	gen/input/op_map_closer.c \

map_gen = \
	gen/programs/map_gen.c \
	shared/hashmap/hashmap.h \
	shared/hashmap/hashmap.c \
	shared/vm/vm.h

ifndef ASSUME_UTF8
ASSUME_UTF8 = 1
endif

debug: $(DEBUG_PATH)/nitwit_server $(DEBUG_PATH)/nitwit_client
release: $(RELEASE_PATH)/nitwit_server $(RELEASE_PATH)/nitwit_client
gen: gen/programs/bin/map_gen

gen/output/op_map.c: $(op_map)
	(cd "gen/output" && ../programs/bin/map_gen);

gen/programs/bin/map_gen: $(map_gen)
	gcc -o gen/programs/bin/map_gen gen/programs/map_gen.c \
	shared/hashmap/hashmap.c -lunistring

define make-execs
$(DEBUG_PATH)/$(1): $(addprefix $(DEBUG_PATH)/,$(3))
	gcc -std=gnu99 -g -Wall $(2) $$^ -o $$@ $(4)
$(RELEASE_PATH)/$(1): $(addprefix $(RELEASE_PATH)/,$(3))
	gcc -std=gnu99 -Ofast -Wall $(2) $$^ -o $$@ $(4)
endef

$(eval $(call make-execs,nitwit_server,-DASSUME_UTF8=$(ASSUME_UTF8), \
	$(SERVER_FILES),-lunistring -pthread -ldl));
$(eval $(call make-execs,nitwit_client,-DASSUME_UTF8=$(ASSUME_UTF8), \
	$(CLIENT_FILES),-lunistring));

define make-objs
$(eval x = $(patsubst %.c, %.o, $(notdir $(1))))
$(patsubst %.c, $(DEBUG_PATH)/%.o, $(notdir $(1))): $(1) $($(x)) \
	$(wildcard $(patsubst %.c, %.h, $(1)))
	gcc -std=gnu99 -g -c -Wall $(2) $$< -o $$@
$(patsubst %.c, $(RELEASE_PATH)/%.o, $(notdir $(1))): $(1) $($(x)) \
	$(wildcard $(patsubst %.c, %.h, $(1)))
	gcc -std=gnu99 -Ofast -c -Wall $(2) $$< -o $$@
endef

$(foreach src,$(SRC_FILES),$(eval $(call make-objs,$(src), \
	-DASSUME_UTF8=$(ASSUME_UTF8))));

.PHONY: clean
clean:
	rm -f $(DEBUG_PATH)/nitwit_server $(DEBUG_PATH)/nitwit_client \
	$(RELEASE_PATH)/nitwit_server $(RELEASE_PATH)/nitwit_client \
	$(DEBUG_PATH)/*.o $(RELEASE_PATH)/*.o gen/output/op_map.c
