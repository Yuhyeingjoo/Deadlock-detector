all:
	gcc -o ./test/test1 ./test/test1.c -pthread -g
	gcc -shared -fPIC -o mymutex.so ddmon.c -ldl 
	gcc ddtect.c -o check -pthread
	gcc -o ./test/test3 ./test/test3.c -pthread -g
	gcc -o ./test/test4 ./test/test4.c -pthread -g
	gcc -o ./test/test5 ./test/test5.c -pthread -g
	gcc -o ./test/test6 ./test/test6.c -pthread -g
clean:
	rm check mymutex.so test1
