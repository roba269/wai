CC=g++
OPT=-Wall -static -O3

.PHONY: clean

clean:
	rm *.o *.exe *.out
	rm test_ai/*.o test_ai/*.exe
all:
	$(CC) $(OPT) -c match_renju.cpp
	$(CC) $(OPT) -c player.cpp
	$(CC) $(OPT) -c match.cpp
	$(CC) $(OPT) -c main.cpp
	$(CC) $(OPT) match_renju.o player.o match.o main.o -o wai.exe -O3
	$(CC) $(OPT) test_ai/renju_bf.cpp -o test_ai/renju_bf.exe -O3
	$(CC) $(OPT) test_ai/renju_random.cpp -o test_ai/renju_random.exe -O3
	$(CC) -Wall -static test_ai/tle.cpp -o test_ai/tle.exe
	$(CC) -Wall -static test_ai/mle.cpp -o test_ai/mle.exe
