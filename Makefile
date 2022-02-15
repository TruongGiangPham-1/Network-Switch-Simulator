


one:
	mkfifo fifo-0-1 fifo-1-0

cleanone:
	unlink fifo-0-1
	unlink fifo-1-0

