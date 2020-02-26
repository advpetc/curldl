#
# Makefile
# Haoyang, 2020-02-07 17:36
#

# What to call the final executable
TARGET = downloader

# Which object files that the executable consists of
OBJS= downloader.o

# What compiler to use
CC = g++

# Compiler flags, -g for debug, -c to make an object file
CFLAGS = -c -g

# This should point to a directory that holds libcurl, if it isn't
# in the system's standard lib dir
# We also set a -L to include the directory where we have the openssl
# libraries
LDFLAGS = -L/usr/bin/

# We need -lcurl for the curl stuff
# We need -lsocket and -lnsl when on Solaris
# We need -lssl and -lcrypto when using libcurl with SSL support
# We need -lpthread for the pthread example
LIBS = -lcurl

# Link the target with all objects and libraries
$(TARGET) : $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

# Compile the source files into object files
downloader.o : downloader.cpp
	$(CC) $(CFLAGS) $<

.PHONY : clean
clean :
	-rm -f file downloader.o downloader
# vim:ft=make
#
