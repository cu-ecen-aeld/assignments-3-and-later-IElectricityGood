
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
CFLAGS  = -Wall -Werror

# built executable:
TARGET = writer

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET) $(TARGET).o
