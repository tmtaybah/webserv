all:  webserv

webserv:
   gcc -o webserv webserv.c

clean:
	rm  *.dat
