SRC_FILES = \
	server/ntwt_server.c \
	client/ntwt_client.c \
	shared/socket/ntwt_socket.c \
	shared/practise/ntwt_practise.c \
	shared/interpreter/ntwt_interpreter.c \
	shared/asm/ntwt_asm_compiler.c \
	shared/unicode/ntwt_unihelpers.c


DEBUG_PATH = bin/debug
DEBUG_FILES = $(patsubst %,$(DEBUG_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

RELEASE_PATH = bin/release
RELEASE_FILES = $(patsubst %,$(RELEASE_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

SERVER_FILES = \
	ntwt_server.o \
	ntwt_socket.o \
	ntwt_interpreter.o \
	ntwt_practise.o

CLIENT_FILES = \
	ntwt_client.o \
	ntwt_socket.o \
	ntwt_asm_compiler.o \
	ntwt_unihelpers.o

ntwt_server.o = \
	shared/socket/ntwt_socket.h \
	shared/interpreter/ntwt_interpreter.h

ntwt_client.o = \
	shared/socket/ntwt_socket.h \
	shared/interpreter/ntwt_interpreter.h \
	shared/asm/ntwt_asm_compiler.h \
	shared/unicode/ntwt_unihelpers.h

ntwt_interpreter.o = \
	shared/interpreter/states.c \
	shared/interpreter/subfuncts/saving.c \
	shared/interpreter/subfuncts/threaded.c \
	shared/interpreter/macros/define.h \
	shared/interpreter/macros/undef.h

ifndef ASSUME_UTF8
ASSUME_UTF8 = 1
endif

debug: $(DEBUG_PATH)/nitwit_server $(DEBUG_PATH)/nitwit_client
release: $(RELEASE_PATH)/nitwit_server $(RELEASE_PATH)/nitwit_client

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
	$(DEBUG_PATH)/*.o $(RELEASE_PATH)/*.o
