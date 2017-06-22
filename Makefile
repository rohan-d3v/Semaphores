#Rohan Krishna Ramkhumar
#rxr353
#Makefile for assignment 6: System V Semaphores
#Reference used: Makefile from Jan 24 Recitation

CC = gcc
OUT = output.o
SRC = baboon.c
TXT1 = output1.txt
TXT2 = output2.txt
all: 	as6

as6: 
	$(CC) -o $(OUT) $(SRC)
	chmod 700 $(OUT)
	 ./$(OUT) aaaaaaabbbaaaabbbbbb >$(TXT1)
	 ./$(OUT) aaaaaaabbbaaaabaabbb >$(TXT2)

	
clean:
	rm -f $(OUT)
	rm -f $(SOUT)	
