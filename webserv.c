#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define BACKLOG 10	 // how many pending connections queue will hold
#define GET_BUFFER 200

typedef struct {
    const char *extension;
    const char *content_type;
} type_map;

type_map map_type [] = {
    {".text", "text/plain"},
		{".htm", "text/html"},
		{".html", "text/html"},
    {".gif", "image/gif"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {NULL, NULL},
};

default_content_type = "text/css";

//===========================================================================
//====== Signal Handelling
//===========================================================================

void interrupt_handler(int signo)
{

  fprintf(stderr, "\nHandled Interrupt (ctrl-c)\n");

  // Kill all processess
  int groupID = getpgid(getpid());
  kill(-groupID, SIGKILL);

}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void register_signal_handlers()
{

		// Setup Interrupt handler
		struct sigaction sa_INT;
		sa_INT.sa_handler = &interrupt_handler;
		sa_INT.sa_flags = SA_RESTART;

		if (sigaction(SIGINT, &sa_INT, NULL) == -1)
		{
			perror(stderr, "Server -- sigaction() error");
			exit(-1);
		}

		// Setup Child Handler
		struct sigaction sa;

		/* clean all the dead processes */
		sa.sa_handler = sigchld_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;

		if(sigaction(SIGCHLD, &sa, NULL) == -1)
		{
			perror("Server -- sigaction() error");
			exit(-1);
		}

}
//===========================================================================
//====== Server Request Handelling Functions
//===========================================================================

void send_header()
{

}

void send_error()
{

}

void list_directory()
{
	
}

void present_html()
{

}

void present_img()
{

}

void execute_cgi()
{

}

//===========================================================================
//====== Server Processing Functions
//===========================================================================

void get_extension()
{

}

void get_request()
{

}

void get_arguements()
{

}

void verify_request()
{

}



// int respond(int socket)
// {
// write(socket, "HTTP/1.1 200 OK\n", 16);
// write(socket, "Content-length: 46\n", 19);
// write(socket, "Content-Type: text/html\n\n", 25);
// write(socket, "<html><body><H1>Hello world</H1></body></html>",46);
// }

//===========================================================================
//====== SERVER
//===========================================================================

void start_server (int port)
{
	// listen on sock_fd, new connection on new_fd */
	int sockfd, new_fd;
	// my address information
	struct sockaddr_in my_addr;
	// connector’s address information
	struct sockaddr_in client_addr;

  int yes = 1;
  int client_size;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Server -- socket error");
		exit(-1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("Server -- setsockopt error");
		exit(-1);
	}

	/* host byte order */
	my_addr.sin_family = AF_INET;
	/* short, network byte order */
	my_addr.sin_port = htons(port);
	/* automatically fill with my IP */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("Server-Using %s and port %d...\n", inet_ntoa(my_addr.sin_addr), port);

	/* zero the rest of the struct */
	memset(&(my_addr.sin_zero), '\0', 8);

	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < -1)
	{
		perror("Server -- bind error");
		exit(-1);
	}

	if(listen(sockfd, BACKLOG) == -1)
	{
		perror("Server -- listen error");
		exit(-1);
	}


	char http_get_buffer[GET_BUFFER];
	// char* http_get_buffer = malloc(GET_BUFFER);

	// Keep accepting connections
	while(1)
	{
		client_size = sizeof(client_addr);
		if((new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_size)) == -1)
		{
			perror("Server-accept() error");
			continue;
		}

		printf("Server: Got connection from %s\n", inet_ntoa(client_addr.sin_addr));
		printf("ON SOCKET %d\n", new_fd);

		/* this is the child process */
		if(fork() == 0)
		{

			/* child doesn’t need the listener */
			close(sockfd);



			// read (new_fd, http_get_buffer, GET_BUFFER);
			recv(new_fd, http_get_buffer, GET_BUFFER, 0);
			printf("Received string = %s\n", http_get_buffer);
			respond(new_fd);



			close(new_fd);
			exit(0);
		}
		else{
			printf("Server-send is OK...!\n");
		}

		/* parent doesn’t need this*/
		close(new_fd);
		printf("Server-new socket, new_fd closed successfully...\n");

	} // End While


}


//===========================================================================
//====== Main
//===========================================================================

int main (int argc, char **argv)
{
  int port_num;

  if (argc > 1){      // check if port num is between 5000-65536
    port_num = strtoimax(argv[1], NULL, 10);

    if(port_num < 5000 || port_num > 65536){
      fprintf(stderr, "Port number must between 5000-65536\n");
      return -1;
    }
    /* could not handle case where port number is something like 777jk -- tried to use end pointed unsecessfully */
  }
  else{
    fprintf(stderr, "Must insert port number between 5000-65536\n");
    return -1;
  }


	register_signal_handlers();
	start_server(port_num);

  return 0;


}
