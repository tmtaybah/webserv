// Standard Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// Libraries for networking
#include <netinet/in.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Others
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>  // For directory traversing


#define BACKLOG 10	 // how many pending connections queue will hold
#define GET_BUFFER 200
#define MAX_ARG 20
#define BUFF_SIZE 1024

typedef struct {
	const char *extension;
	const char *content_type;
} type_map;

type_map map_type [] = {
	{".text", "text/plain"},
	{".htm", "text/html"},
	{".html", "text/html"},
	{".cgi", "text/html"},
	{".gif", "image/gif"},
	{".jpeg", "image/jpeg"},
	{".jpg", "image/jpeg"},
	{NULL, NULL},
};

char* default_content_type = "text/html";
char* file_extensions = ".html, .text, .gif, .jpeg, .jpg, .htm";

//===========================================================================
//====== Signal Handelling
//===========================================================================

void interrupt_handler(int signo)
{

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
		perror("Server -- sigaction() error");
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

void send_header(int socket_fd, char* content_type, int no_content_type)
{
	char *header = "HTTP/1.1 200 OK\n";
	write(socket_fd, header, strlen(header));

	if (no_content_type == 0)
	{
		char* content_header = "Content-Type: ";
		write(socket_fd, content_header, strlen(content_header));
		write(socket_fd, content_type, strlen(content_type));
		write(socket_fd, "\n\n", strlen("\n\n"));
	}
}

void send_request_error(int socket_fd)
{
	char *error = "HTTP/1.1 404 Not Found\n";
	char* content_header = "Content-Type: text/html\n\n";
	write(socket_fd, error, strlen(error));
	write(socket_fd, content_header, strlen(content_header));

	char* start =  "<!DOCTYPE html><html><head></head>";
	char* body =   "<body><h1>";
	char* message = "404 Not Found";
	char* end_body = "</h1></body>";

	write(socket_fd, start, strlen(start));
	write(socket_fd, body, strlen(body));
	write(socket_fd, message, strlen(message));
	write(socket_fd, end_body, strlen(end_body));

}

void send_method_error(int socket_fd)
{
	char *error = "HTTP/1.1 501 Not Implemented\n";
	char* content_header = "Content-Type: text/html\n\n";
	write(socket_fd, error, strlen(error));
	write(socket_fd, content_header, strlen(content_header));

	char* start =  "<!DOCTYPE html><html><head></head>";
	char* body =   "<body><h1>";
	char* message = "501 Not Implemented";
	char* end_body = "</h1></body>";

	write(socket_fd, start, strlen(start));
	write(socket_fd, body, strlen(body));
	write(socket_fd, message, strlen(message));
	write(socket_fd, end_body, strlen(end_body));
}


void list_directory(int socket_fd, char* request)
{

	// fprintf(stderr, "LISTING DIRECTORIES\n");
	char* start =  "<!DOCTYPE html><html><head></head>";
	char* body =   "<body> <h1>Index of ";
	char * part2 = "</h1><table>";

	write(socket_fd, start, strlen(start));
	write(socket_fd, body, strlen(body));
	write(socket_fd, request, strlen(request));
	write(socket_fd, part2, strlen(part2));

	// Traverse Directory

	DIR           *d;
	struct dirent *dir;
	d = opendir(request);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			write(socket_fd, "<tr><td>", strlen("<tr><td>"));
			write(socket_fd, "<a href=\"", strlen("<a href=\""));
			write(socket_fd, dir->d_name, strlen(dir->d_name));
			write(socket_fd, "</a>", strlen("</a>"));
			write(socket_fd, dir->d_name, strlen(dir->d_name));
			write(socket_fd, "</td></tr>", strlen("</td></tr>"));
		}

		closedir(d);
	}

	char *end = "</table></body></html>";
	write(socket_fd, end, strlen(end));

}

void execute_cgi(int socket_fd, char* command, char** args)
{

	dup2(socket_fd, 1);
	close(socket_fd);

	// fprintf(stderr, "COMMAND IS %s\n", command);

	if( execvp(command, args) == -1 )
	{
		perror("Exec error");
		exit(-1);
	}
}

void send_file(int socket_fd, char* file_name)
{
	FILE *file = fopen(file_name, "r");

	if(file==NULL)
	{
		printf("Server -- File open error");
		exit(-1);
	}

	/* Read data from file and send it */
	while(1)
	{
		unsigned char buff[BUFF_SIZE];
		int bytes_read = fread(buff,1,BUFF_SIZE,file);

		/* If read was success, send data. */
		if(bytes_read > 0)
		{
			write(socket_fd, buff, bytes_read);
		}

	}

}

//===========================================================================
//====== Server Processing Functions
//===========================================================================


/*
* Returns the extension for the requested file.
*/
char* get_extension(char* request)
{
	char *extension = strrchr(request, '.');

	return extension;
}

/*
* Returns the html content type for the extension
*/
char* get_content_type(char* extension)
{

	if (extension == NULL)
	{
		return default_content_type;
	}

	type_map *map = map_type;

	while(map->extension)
	{
		if(strcmp(map->extension, extension) == 0){
			return map->content_type;
		}
		map++;
	}

	return default_content_type;
}



/*
* Verifies if the request file exists
*/
int verify_request(int socket_fd, char* request)
{

	// Handle no request case
	if ( strlen(request) == 0)
	{
		request = ".";
	}

	if( access( request, F_OK ) != -1 )
	{
		// file exists
		return 1;
	}
	else
	{
		// file doesn't exist or is a directory
		struct stat statbuf;

		// Get stats
		if (lstat(request, &statbuf) < 0)
		{
			fprintf (stderr,"Stat error for %s\n", request);
		}

		// Directory
		if (S_ISDIR(statbuf.st_mode) > 0)
		{
			return 1;
		}

		send_request_error(socket_fd);
		return 0;
	}

}

/*
* Returns the next token, or null.
* Replaces '\n' at the end of the token with '\0'.
*/
char* get_next_token(char* line, int start)
{
	char delimeter[] = " &";
	char *token;
	int len;

	// Get first token
	if (start == 1)
	{
		token = strtok(line, delimeter);
	}
	else
	{
		token = strtok(NULL, delimeter);
	}

	if (token == NULL)
	{
		return NULL;
	}

	len = strlen(token);

	//Fix token if needed (remove new line)
	if(len > 1 && token[len-1] == '\n')
	{
		token[len-1] = '\0';
		len -= 1;
	}

	return token;
}

/*
* Processes the GET request into the request file and it's argument.
* Plus, calls the request handelling functions.
*/
void process_request(int socket_fd, char* http_header)
{
	// char* request =
	// fprintf(stderr, "IN PROCESS REQUEST\n");

	// Array to hold args
	char *args[MAX_ARG];
	char *request;
	char *token;
	char *ret;

	// Get first token
	token = get_next_token(http_header, 1);
	ret = strstr(token, "GET");
	if (ret == NULL)
	{
		send_method_error(socket_fd);
	}
	ret = strstr(token, "HTTP/");

	int len;
	int count = 0;
	int arg_count = 0;

	while (token != NULL)
	{
		if (ret != NULL)
		{
			break;
		}
		else if (count != 0)
		{
			if (count == 1)
			{
				token = token + 1;
				request = token;
			}

			len = strlen(token);
			args[arg_count] = malloc(len + 1);
			strcpy(args[arg_count], token);

			arg_count ++;
		}

		token = get_next_token(http_header, 0);
		ret = strstr(token, "HTTP/");

		count++;

	}

	args[arg_count] = NULL; // Args has to end in NULL

	// fprintf(stderr, "DONE WITH LOOP\n");
	// fprintf(stderr, "ARG COUNT IS %d\n", arg_count);
	// fprintf(stderr, "Request is %s\n", request);
	//
	// int i=0;
	// for(i=0; i <= arg_count; i++)
	// {
	// 	fprintf(stderr, "Arg %d is %s\n", i, args[i]);
	// }

	int valid = verify_request(socket_fd, request);
	// fprintf(stderr, "VALID REQUEST? %d\n", valid);

	if(valid == 1)
	{
		char* extension = get_extension(request);
		// fprintf(stderr, "EXTENSION IS %s\n", extension);

		char* type = get_content_type(extension);
		// fprintf(stderr, "CONTENT TYPE  IS %s\n", type);

		if(extension == NULL)
		{
			send_header(socket_fd, type, 0);
			if (strlen(request) == 0)
			{
				list_directory(socket_fd, ".");
			}
			else
			{
				list_directory(socket_fd, request);
			}
		}

		if(strstr(file_extensions, extension) != NULL)
		{
			send_header(socket_fd, type, 0);
			send_file(socket_fd, request);
		}
		else if(strstr(extension, ".cgi"))
		{
			send_header(socket_fd, type, 1);
			execute_cgi(socket_fd, request, args);
		}

	}

	// fprintf(stderr, "DONE WITH PROCESSING\n");

}

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

			recv(new_fd, http_get_buffer, GET_BUFFER, 0);
			process_request(new_fd, http_get_buffer);

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
