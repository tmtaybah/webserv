all: webserv arduino

webserv:
	gcc -o webserv webserv.c

arduino:
	gcc -o arduino.cgi arduino.c

clean:
	rm  *.dat arduino.cgi histogram.jpeg webserv
