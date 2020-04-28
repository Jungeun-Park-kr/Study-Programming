ftlmgr: ftlmgr.o fdevicedriver.o
	gcc ftlmgr.o fdevicedriver.o -o ftlmgr

ftlmgr.o: ftlmgr.c flash.h
	gcc -c ftlmgr.c

fdevicedriver.o: fdevicedriver.c flash.h
	gcc -c fdevicedriver.c

clean:
	rm *.o
	rm ftlmgr
