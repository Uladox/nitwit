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
$(DEBUG_PATH)/$(1): $(addprefix $(DEBUG_PATH)/,$(2))
	gcc -g     -Wall -Werror $$^ -o $$@ $(3)
$(RELEASE_PATH)/$(1): $(addprefix $(RELEASE_PATH)/,$(2))
	gcc -Ofast -Wall -Werror $$^ -o $$@ $(3)
endef

SERVER_FILES = \
	ntwt_server.o \
	ntwt_socket.o \
	ntwt_interpreter.o \
	ntwt_practise.o

$(eval $(call make-execs,nitwit_server,$(SERVER_FILES),-pthread));
$(eval $(call make-execs,nitwit_client,ntwt_client.o ntwt_socket.o,));

define make-objs
$(patsubst %.c, $(DEBUG_PATH)/%.o, $(notdir $(1))): $(1)
	gcc -g     -c -Wall -Werror $$< -o $$@
$(patsubst %.c, $(RELEASE_PATH)/%.o, $(notdir $(1))): $(1)
	gcc -Ofast -c -Wall -Werror $$< -o $$@
endef

$(foreach src,$(SRC_FILES),$(eval $(call make-objs,$(src))));
