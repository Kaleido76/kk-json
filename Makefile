CC = g++
CFLAGS = -c -Wall

LIBNAME = libkkjson.a

SRC = kkjson.cpp
OBJ = $(SRC:.cpp=.o)

LIBDIR = ./
LDFLAGS = -L$(LIBDIR) -lkkjson

TESTSRC = test.cpp
TESTOBJ = $(TESTSRC:.cpp=.o)
TESTTARGET = test

all: $(LIBNAME) $(TESTTARGET)

$(LIBNAME): $(OBJ)
	ar rcs $(LIBNAME) $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(TESTTARGET): $(TESTOBJ) $(LIBNAME)
	$(CC) -o $@ $(TESTOBJ) $(LDFLAGS)

clean:
	rm -f $(LIBNAME) $(OBJ) $(TESTTARGET) $(TESTOBJ)
