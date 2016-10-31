CC = gcc

all: myftp.o myftpd.o

myftp.o: myftp.c
	gcc myftp.c -o myftp -lmhash

myftpd.o: myftpd.c
	gcc myftpd.c -o myftpd -lmhash

clean:
	$(RM) count *.o *~
	rm -f myftp myftpd
