SRC_FILES = \
	server/ntwt_server.c \
	client/ntwt_client.c \
	shared/socket/ntwt_socket.c \
	shared/interpreter/ntwt_interpreter.c \
	shared/practise/ntwt_practise.c

DEBUG_PATH = bin/debug
DEBUG_FILES = $(patsubst %,$(DEBUG_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

RELEASE_PATH = bin/release
RELEASE_FILES = $(patsubst %,$(RELEASE_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

debug: $(DEBUG_PATH)/nitwit_server $(DEBUG_PATH)/nitwit_client
release: $(RELEASE_PATH)/nitwit_server $(RELEASE_PATH)/nitwit_client

define make-execs
$(DEBUG_PATH)/$(1): $(addprefix $(DEBUG_PATH)/,$(2)) $(4)
	gcc -std=gnu99 -g     -Wall -Werror $$^ -o $$@ $(3)
$(RELEASE_PATH)/$(1): $(addprefix $(RELEASE_PATH)/,$(2)) $(4)
	gcc -std=gnu99 -Ofast -Wall -Werror $$^ -o $$@ $(3)
endef

SERVER_FILES = \
	ntwt_server.o \
	ntwt_socket.o \
	ntwt_interpreter.o \
	ntwt_practise.o

$(eval $(call make-execs,nitwit_server,$(SERVER_FILES),-pthread -ldl,));
$(eval $(call make-execs,nitwit_client,ntwt_client.o ntwt_socket.o,\
	shared/interpreter/ntwt_interpreter.h));

define make-objs
$(patsubst %.c, $(DEBUG_PATH)/%.o, $(notdir $(1))): $(1)   \
	$(wildcard $(patsubst %.c, %.h, $(1)))
	gcc -std=gnu99 -g     -c -Wall -Werror $$< -o $$@
$(patsubst %.c, $(RELEASE_PATH)/%.o, $(notdir $(1))): $(1) \
	$(wildcard $(patsubst %.c, %.h, $(1)))
	gcc -std=gnu99 -Ofast -c -Wall -Werror $$< -o $$@
endef

$(foreach src,$(SRC_FILES),$(eval $(call make-objs,$(src))));
