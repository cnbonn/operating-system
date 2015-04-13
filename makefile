CC = g++
LIBS = 
FLAGS = 
EXECS = dash 

all: $(EXECS)

dash: prog1.cpp
	$(CC) $(FLAGS) -o $@ $? $(LIBS) 

clean:
	$(RM) $(EXECS)
