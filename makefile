all:
	gcc -g main.c simu.c list.o -o simu

demo:
	./simu

valgrind-check:
	valgrind --leak-check=full ./simu

valgrind-show:
	valgrind --track-origins=yes ./simu
	 
clean:
	rm -f simu