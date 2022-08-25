
ifndef CROSS_COMPILE
	CC = gcc
else
	CC = $(CROSS_COMPILE)
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
