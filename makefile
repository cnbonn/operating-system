CC = g++
LIBS = 
FLAGS = 
EXECS = sim 

all: $(EXECS)

sim: main.cpp
	$(CC) $(FLAGS) -o $@ $? $(LIBS) 

clean:
	$(RM) $(EXECS)
