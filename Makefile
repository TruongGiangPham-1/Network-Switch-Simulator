


one:
	mkfifo fifo-0-1 fifo-1-0
two:
	mkfifo -m 666 fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-1-2 fifo-2-1
cleanone:
	unlink fifo-0-1
	unlink fifo-1-0

cleantwo:
	rm -rf fifo-1-0 fifo-0-1 fifo-2-0 fifo-0-2 fifo-1-2 fifo-2-1 

