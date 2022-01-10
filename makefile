enigma: main.o utilities.o
	g++ --std=c++11 -g main.o utilities.o -o enigma

main.o: main.cpp errors.h enigma.h
	g++ --std=c++11 -Wall -g -c main.cpp

utilities.o: utilities.cpp utilities.h
	g++ --std=c++11 -Wall -g -c utilities.cpp