PROGRAM := minesweeper
CC := gcc
CFLAG := -Wall -Wextra
INCLUDE := -I./include
SUFFIX := .c
SRCDIR := ./src
LIBDIR := ./library
OBJDIR := $(SRCDIR)
SRCS := $(wildcard $(SRCDIR)/*$(SUFFIX))
LIBS := $(wildcard $(LIBDIR)/*$(SUFFIX))
OBJS := $(SRCS:%.c=%.o)
TARGET := ./$(PROGRAM)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(INCLUDE) $(CFLAG) $(SRCS) $(LIBS) -L -llogger.a -lncurses -o ./$@

test: $(SRCS)
	$(CC) $(INCLUDE) -Wall ./src/test.c $(LIBS) -L -llogger.a -o ./$@

wintest: $(SRCS)
	$(CC) $(INCLUDE) -Wall -Wno-stringop-truncation ./src/mainwin.c $(LIBS) -L -llogger.a -o ./$@

clean:
	echo a

fclean: clean
	rm $(TARGET)

re: fclean all
