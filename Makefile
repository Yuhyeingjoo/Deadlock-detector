all:
	gcc -o test1 test1.c -pthread -g
	gcc -shared -fPIC -o mymutex.so ddmon.c -ldl 
	gcc ddtect.c -o check -pthread
	gcc -o test2 test2.c -pthread -g
clean:
	rm check mymutex.so test1
