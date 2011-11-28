CC=g++
OPT=-Wall -static -O3

all:
	$(CC) $(OPT) -c match_renju.cpp
	$(CC) $(OPT) -c player.cpp
	$(CC) $(OPT) -c main.cpp -O3
	$(CC) $(OPT) match_renju.o player.o main.o -o wai.exe -O3
	$(CC) $(OPT) test_ai/renju_bf.cpp -o test_ai/renju_bf.exe -O3
	$(CC) $(OPT) test_ai/renju_random.cpp -o test_ai/renju_random.exe -O3
