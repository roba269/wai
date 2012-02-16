CC=g++
OPT=-Wall -static -O3
INCLUDE=-I/usr/include/mysql
LIB=-lmysqlclient
.PHONY: clean

all:
	$(CC) $(OPT) -c match_renju.cpp
	$(CC) $(OPT) -c player.cpp
	$(CC) $(OPT) -c match.cpp
	$(CC) $(OPT) $(INCLUDE) -c scheduler.cpp
	$(CC) $(OPT) $(INCLUDE) -c db_wrapper.cpp
	$(CC) $(OPT) $(INCLUDE) -c compiler.cpp
	$(CC) $(OPT) -c main.cpp
	$(CC) $(OPT) -c simple_match.cpp
	$(CC) $(OPT) -c sandbox.cpp
	$(CC) $(OPT) -c match_main.cpp
	$(CC) -Wall match_renju.o player.o match.o scheduler.o db_wrapper.o compiler.o main.o simple_match.o sandbox.o -lmysqlclient -lpthread -o wai.exe -O3
	$(CC) -Wall match_renju.o player.o match.o scheduler.o db_wrapper.o compiler.o match_main.o simple_match.o sandbox.o -lmysqlclient -lpthread -o match.exe -O3
	$(CC) $(OPT) test_ai/renju_bf.cpp -o test_ai/renju_bf.exe -O3
	$(CC) $(OPT) test_ai/renju_random.cpp -o test_ai/renju_random.exe -O3
	$(CC) -Wall -static test_ai/tle.cpp -o test_ai/tle.exe
	$(CC) -Wall -static test_ai/mle.cpp -o test_ai/mle.exe
	$(CC) -Wall -static test_judge/renju_judge.cpp -o test_judge/renju_judge.exe
clean:
	rm *.o *.exe *.out
	rm test_ai/*.o test_ai/*.exe
