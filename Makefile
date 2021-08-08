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
WIN_TARGET := ./win_$(PROGRAM)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(INCLUDE) $(CFLAG) $(SRCDIR)/main.c -L -llogger -o ./$@

win: $(WIN_TARGET)

$(WIN_TARGET): $(SRCS)
	$(CC) $(INCLUDE) $(CFLAG) $(SRCDIR)/main_win.c -L -llogger -o ./$@

test: $(SRCS)
	$(CC) $(INCLUDE) -Wall ./src/test.c $(LIBS) -L -llogger -o ./$@

wintest: $(SRCS)
	$(CC) $(INCLUDE) -Wall -Wno-stringop-truncation ./src/mainwin.c $(LIBS) -L -llogger -o ./$@

clean:
	echo a

fclean: clean
	rm $(TARGET)

re: fclean all
