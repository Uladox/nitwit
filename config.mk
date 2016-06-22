# Customize below to fit your system

# flags
DEBUG_CFLAGS = -std=gnu99 -g -Wall -DASSUME_UTF8
RELEASE_CFLAGS = -std=gnu99 -Ofast -Wall #-DASSUME_UTF8
# DEBUG_CFLAGS = -std=c99 -pedantic -g -Wall -D_XOPEN_SOURCE=700 \
# 	-DASSUME_UTF8 -DC99_COMPLIANT
# #-fno-omit-frame-pointer -fsanitize=thread
# RELEASE_CFLAGS = -std=c99 -pedantic -Ofast -Wall -D_XOPEN_SOURCE=700 \
# 	-DASSUME_UTF8  -DC99_COMPLIANT

# Libs
SERVER_LIBS = -lnit -lthreadpass -lunistring -pthread -ldl
CLIENT_LIBS = -lnit -lspar -lunistring

# compiler and linker
CC = cc
