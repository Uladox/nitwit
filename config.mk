
# flags
DEBUG_CFLAGS = -std=gnu99 -g -Wall -DASSUME_UTF8
RELEASE_CFLAGS = -std=gnu99 -Ofast -Wall -DASSUME_UTF8
# DEBUG_CFLAGS = -std=c99 -pedantic -g -Wall -D_XOPEN_SOURCE=700 -DASSUME_UTF8
# RELEASE_CFLAGS = -std=c99 -pedantic -Ofast -Wall -D_XOPEN_SOURCE=700 -DASSUME_UTF8

# libs
SERVER_LIBS = -lunistring -pthread -ldl
CLIENT_LIBS = -lunistring

# compiler and linker
CC = cc
