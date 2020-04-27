ssu_mntr : main.o ssu_mntr.o
	gcc main.o ssu_mntr.o -o ssu_mntr

main.o : main.c ssu_mntr.h
	gcc -c main.c

ssu_mntr.o : ssu_mntr.c ssu_mntr.h
	gcc -c ssu_mntr.c

clean :
	rm *.o
	rm ssu_mntr
