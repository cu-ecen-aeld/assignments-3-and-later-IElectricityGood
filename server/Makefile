
# Example use cases:
#
# Cross compile:
# 	make CROSS_COMPILE=aarch64-none-linux-gnu-
#
# Native compile:
#	make
#
# Clean:
#	make clean

ifndef CROSS_COMPILE
	CC = gcc
else
	CC = $(CROSS_COMPILE)gcc
endif

# compiler flags:
#  -Wall turns on compiler warnings
#  -Werror makes warnings errors
#  -static: https://stackoverflow.com/a/24607099
CFLAGS  = -Wall -Werror -static

# built executable:
TARGET = aesdsocket

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET) $(TARGET).o
