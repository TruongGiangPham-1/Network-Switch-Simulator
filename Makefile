


onefifo: 
	mkfifo fifo-0-1 fifo-1-0
twofifo:
	mkfifo -m 666 fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-1-2 fifo-2-1
ex3s1:  # swich one for ex3.dat
	./a2w22 psw1 ex3.dat null psw2 100-110
ex3s2:
	./a2w22 psw2 ex3.dat psw1 null 200-210

ex2s1:
	./a2w22 psw1 ex2.dat null psw2 100-110
ex2s2:
	./a2w22 psw2 ex2.dat psw1 null 200-210
ex1s1:
	./a2w22 psw1 ex1.dat null null 100-110

clean: # remove all fifos in the current directory
	find . -type p -delete 



