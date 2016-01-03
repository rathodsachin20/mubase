#target: dependencies
#[tab] system command

SRCDIR = ./src
OBJDIR = ./obj
BINDIR = ./bin
INCDIR = ./include
INPUT = input
TEMP = bin
CFLAGS = -c -I$(INCDIR)
CC = g++
CP = cp

all: $(BINDIR)/exec

$(BINDIR)/exec	:	$(OBJDIR)/dm.o \
		$(OBJDIR)/bm.o \
		$(OBJDIR)/sm.o \
		$(OBJDIR)/smutil.o \
		$(OBJDIR)/bt.o \
		$(OBJDIR)/sputil.o \
		$(OBJDIR)/sp.o \
		$(OBJDIR)/rm.o \
		$(OBJDIR)/sort.o \
		$(OBJDIR)/iter.o \
		$(OBJDIR)/lex.yy.o \
		$(OBJDIR)/main.o
		$(CC) $(OBJDIR)/dm.o $(OBJDIR)/bm.o $(OBJDIR)/sm.o $(OBJDIR)/smutil.o $(OBJDIR)/bt.o $(OBJDIR)/sputil.o $(OBJDIR)/sp.o $(OBJDIR)/rm.o  $(OBJDIR)/sort.o $(OBJDIR)/iter.o $(OBJDIR)/lex.yy.o  $(OBJDIR)/main.o -o $(BINDIR)/exec -I$(INCDIR)
		$(CP) $(INPUT)/test.txt $(TEMP)
	
$(OBJDIR)/dm.o	:	$(SRCDIR)/dm.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/dm.o $(SRCDIR)/dm.cpp 

$(OBJDIR)/bm.o	:	$(SRCDIR)/bm.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/bm.o $(SRCDIR)/bm.cpp
		
$(OBJDIR)/sm.o	:	$(SRCDIR)/sm.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/sm.o $(SRCDIR)/sm.cpp

$(OBJDIR)/smutil.o	:	$(SRCDIR)/smutil.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/smutil.o $(SRCDIR)/smutil.cpp

$(OBJDIR)/bt.o	:	$(SRCDIR)/bt.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/bt.o $(SRCDIR)/bt.cpp
		


$(OBJDIR)/sp.o	:	$(SRCDIR)/sp.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/sp.o $(SRCDIR)/sp.cpp
$(OBJDIR)/sputil.o	:	$(SRCDIR)/sputil.c
		$(CC) $(CFLAGS) -o $(OBJDIR)/sputil.o $(SRCDIR)/sputil.c
$(OBJDIR)/rm.o	:	$(SRCDIR)/rm.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/rm.o $(SRCDIR)/rm.cpp
$(OBJDIR)/sort.o	:	$(SRCDIR)/sort.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/sort.o $(SRCDIR)/sort.cpp
$(OBJDIR)/iter.o	:	$(SRCDIR)/iter.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/iter.o $(SRCDIR)/iter.cpp
$(OBJDIR)/lex.yy.o	:	$(SRCDIR)/lex.yy.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/lex.yy.o $(SRCDIR)/lex.yy.cpp
$(OBJDIR)/main.o	:	$(SRCDIR)/main.cpp
		$(CC) $(CFLAGS) -o $(OBJDIR)/main.o $(SRCDIR)/main.cpp


		
clean:
		rm -rf $(OBJDIR)/*.o $(BINDIR)/exec $(BINDIR)/test.txt 
