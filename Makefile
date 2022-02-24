


one:
	mkfifo fifo-0-1 fifo-1-0
two:
	mkfifo -m 666 fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-1-2 fifo-2-1
ex3s1:  # swich one for ex3.dat
	./a2w22_withGLobal psw1 ex3.dat null psw2 100-110
ex3s2:
	./a2w22_withGLobal psw2 ex3.dat psw1 null 200-210


cleanone:
	unlink fifo-0-1
	unlink fifo-1-0

cleantwo:
	rm -rf fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-1-2 fifo-2-1 

