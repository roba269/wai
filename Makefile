CC=g++

all:
	$(CC) -c match_renju.cpp -O3
	$(CC) -c player.cpp -O3
	$(CC) -c main.cpp -O3
	$(CC) match_renju.o player.o main.o -o wai.exe -O3
	$(CC) test_ai/renju_bf.cpp -o test_ai/renju_bf.exe -O3
	$(CC) test_ai/renju_random.cpp -o test_ai/renju_random.exe -O3
