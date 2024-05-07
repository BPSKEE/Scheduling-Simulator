#Brandon Skeens
#826416091

CXX=g++

CXXFLAGS=-std=c++11 -Wall -g3 -c

OBJS = schedule.o

PROGRAM = schedule


$(PROGRAM) : $(OBJS) 
	$(CXX) -o $(PROGRAM) $^

schedule.o : schedule.cpp schedule.h log.h
	$(CXX) $(CXXFLAGS) schedule.cpp
	
	
clean :
	rm -f *.o $(PROGRAM)

