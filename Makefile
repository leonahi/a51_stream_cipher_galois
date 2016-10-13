#----------------------------------------------------------------------------
# Build process stream cipher A5/1 
#
# Coder - Nahit Pawar
#         PhD Indian Institute of Technology Bombay, India
#
#----------------------------------------------------------------------------

CPP    = g++ -std=c++11
CFLAGS = -g
OFLAGS = -g
RM     = rm

TEST_FILE = large_test_file.txt 

EXEC   = a5
OBJECTS = \
    main.o \
	helper.o

.PHONEY: clean
.PHONEY: all
.PHONEY: compile
.PHONEY: acknowledge


all: $(OBJECTS) compile acknowledge

main.o : main.cpp
	$(CPP) $(OFLAGS) -c $^

helper.o : helper.cpp
	$(CPP) $(OFLAGS) -c $^

compile:
	$(CPP) $(CFLAGS) $(OBJECTS) -o $(EXEC)

prof: run
	gprof $(EXEC) gmon.out > analysis.txt


run:
	./$(EXEC) $(TEST_FILE) cipher_text.txt nahitpawar
	
clean:
	$(RM) $(OBJECTS) $(EXEC)
	$(RM) *~

acknowledge:
	@echo " "
	@echo "Perfecto! Compilation Done Successfully..."
	@echo " "
