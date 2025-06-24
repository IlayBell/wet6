CXX=g++
CXXFLAGS=-g -Wall -std=c++11
CLINK=$(CXX)
OBJS=main.o NIC_sim.o L2.o L3.o L4.o 
EXEC="prog.exe"
RM=rm -rf

prog.exe: $(OBJS)
	$(CLINK) $(OBJS) -o $(EXEC)

main.o: NIC_sim.hpp L2.h L3.h L4.h common.hpp packets.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

NIC_sim.o: NIC_sim.hpp L2.h L3.h L4.h common.hpp packets.hpp
	$(CXX) $(CXXFLAGS) -c NIC_sim.cpp

L2.o: L2.h L3.h L4.h common.hpp packets.hpp
	$(CXX) $(CXXFLAGS) -c L2.cpp

L3.o: L3.h L4.h common.hpp packets.hpp
	$(CXX) $(CXXFLAGS) -c L3.cpp

L4.o: L4.h common.hpp packets.hpp
	$(CXX) $(CXXFLAGS) -c L4.cpp

clean:
	$(RM) *.o *.exe
