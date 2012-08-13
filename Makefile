CC=g++
OPT=-Wall -static -g
INCLUDE=-I/usr/include/mysql
LIB=-lmysqlclient
.PHONY: clean

all:
	# $(CC) $(OPT) -c match_renju.cpp
	# $(CC) $(OPT) -c player.cpp
	# $(CC) $(OPT) -c match.cpp
	$(CC) $(OPT) $(INCLUDE) -c scheduler.cpp
	$(CC) $(OPT) $(INCLUDE) -c db_wrapper.cpp
	$(CC) $(OPT) $(INCLUDE) -c compiler.cpp
	$(CC) $(OPT) -c main.cpp
	$(CC) $(OPT) $(INCLUDE) -c simple_match.cpp
	$(CC) $(OPT) $(INCLUDE) -c hvc_match.cpp
	$(CC) $(OPT) -c sandbox.cpp
	$(CC) $(OPT) -c match_main.cpp
	$(CC) $(OPT) -c hvc_main.cpp
	$(CC) $(OPT) -DMONGO_USE_LONG_LONG_INT -Imongo_driver -c waid.cpp
	gcc -DMONGO_USE_LONG_LONG_INT $(OPT) -Imongo_driver mongo_driver/*.c waid.o -o waid.exe
	# $(CC) -Wall match_renju.o player.o match.o scheduler.o db_wrapper.o compiler.o main.o simple_match.o sandbox.o -lmysqlclient -lpthread -o wai.exe -O3
	$(CC) -Wall scheduler.o db_wrapper.o compiler.o match_main.o simple_match.o sandbox.o -lmysqlclient -lpthread -o match.exe -O3
	$(CC) -Wall scheduler.o db_wrapper.o compiler.o hvc_main.o hvc_match.o sandbox.o -lmysqlclient -lpthread -o hvc_match.exe -O3
	# $(CC) -Wall main.o compiler.o scheduler.o db_wrapper.o sandbox.o simple_match.o -lmysqlclient -lpthread -o waid.exe -O3
	$(CC) $(OPT) test_ai/renju_bf.cpp -o test_ai/renju_bf.exe -O3
	$(CC) $(OPT) test_ai/renju_random.cpp -o test_ai/renju_random.exe -O3
	$(CC) -Wall -static test_ai/tle.cpp -o test_ai/tle.exe
	$(CC) -Wall -static test_ai/mle.cpp -o test_ai/mle.exe
	$(CC) -Wall -static test_ai/re.cpp -o test_ai/re.exe
	$(CC) -Wall -static test_judge/renju_simple_judge.cpp -o test_judge/renju_simple_judge.exe
	$(CC) -Wall -static test_judge/renju_restricted_judge.cpp -o test_judge/renju_restricted_judge.exe
	$(CC) -Wall -static test_judge/othello_judge.cpp -o test_judge/othello_judge.exe
	$(CC) -Wall -static test_judge/xiangqi_judge.cpp -o test_judge/xiangqi_judge.exe
	$(CC) -Wall -static test_judge/tankcraft_judge.cpp -o test_judge/tankcraft_judge.exe
	$(CC) $(OPT) test_ai/othello_greedy.cpp -o test_ai/othello_greedy.exe -O3
	$(CC) $(OPT) test_ai/xiangqi_sample.cpp -o test_ai/xiangqi_sample.exe -O3
	$(CC) $(OPT) test_ai/tankcraft_sample.cpp -o test_ai/tankcraft_sample.exe -O3
tank:
	$(CC) -Wall -static test_judge/tankcraft_judge.cpp -o test_judge/tankcraft_judge.exe
	$(CC) $(OPT) test_ai/tankcraft_sample.cpp -o test_ai/tankcraft_sample.exe -O3
clean:
	rm *.o *.exe *.out
	rm test_ai/*.o test_ai/*.exe
