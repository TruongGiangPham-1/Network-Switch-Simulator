# ------------------------------------------------------------
# Makefile-cc: Makefile for CMPUT 379 A2
#
# Usage: make        // compile a2w22.cpp
#        make a2w22 // compile a2w22.cpp
#        make onefifo  // create fifos for ex1.dat
#        make twofifo  // create fifos for ex2.dat and ex3.dat
#        make ex3s1    // runs psw1 switch for ex3.dat
#        make ex3s2    // runs psw2 switch for ex3.dat
#        make ex2s1    // runs psw1 switch for ex2.dat
#        make ex2s2    // runs psw2 switch for ex2.dat
#        make tar    // produce submit.tar
#        make cleanfifo // remove all fifo
#        make clean  // remove submit.tar and msh379 executable
# ------------------------------------------------------------

a2w22:
	g++ a2w22.cpp -o a2w22

allfifo: # make all fifos for 7 switches
	mkfifo -m 666 fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-3-0 fifo-0-3 fifo-4-0 fifo-0-4 fifo-5-0 fifo-0-5 fifo-6-0 fifo-0-6 fifo-7-0 fifo-0-7
	mkfifo -m 666 fifo-1-2 fifo-2-1 fifo-2-3 fifo-3-2 fifo-3-4 fifo-4-3 fifo-4-5 fifo-5-4 fifo-5-6 fifo-6-5 fifo-6-7 fifo-7-6
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

tar:
	tar -cvf submit.tar Makefile a2w22.cpp report.pdf 

cleanfifo: # remove all fifos in the current directory
	find . -type p -delete 
clean:
	rm -rf a2w22 submit.tar


