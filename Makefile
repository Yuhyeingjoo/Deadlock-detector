all:
	gcc -o ./test/test1 ./test/test1.c -pthread -g
	gcc -shared -fPIC -o mymutex.so ddmon.c -ldl 
	gcc ddtect.c -o check -pthread
clean:
	rm check mymutex.so channel
