#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



/*
* Adapted form http://todbot.com/blog/2006/12/06/arduino-serial-c-code-to-talk-to-arduino/
* And the sample code provided by the professor
*/


// Arudino constants
char *portname = "/dev/cu.usbmodem1451";

// For message
#define MAX_CHAR 50 // Maximum number of character to print out to LCD
char* msg;


int main(int argc, char *argv[])
{

  //===========================================================================
  //====== Get Message
  //===========================================================================

  msg = malloc(MAX_CHAR * sizeof(char));

  printf("Content-type: text/plain\n\n");
  // Get message to send to arudino
  if( argc > 1)
  {
    if (strlen(argv[1]) > MAX_CHAR)
    {
      printf("ERROR: Message has to be less than %d characters", MAX_CHAR);
      exit(-1);
    }

    strcpy(msg, argv[1]);

  }
  else
  {
    printf("ERROR: You must provide a message that is less than %d characters", MAX_CHAR);
    exit(-1);
  }


  //===========================================================================
  //====== Setup Connection & Get Set Options
  //===========================================================================


  int fd;

  /* Open the file descriptor in non-blocking mode */
  if ((fd = open(portname, O_RDWR | O_NONBLOCK)) == -1)
  {
    printf("Couldn't open Arudino");
    exit(-1);
  }

  /* Set up the control structure */
  struct termios toptions;

  /* Get currently set options for the tty */
  tcgetattr(fd, &toptions);

  //===========================================================================
  //====== Setup Custom Options
  //===========================================================================

  // Set baud rate to 9600
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);

  //----------------------------------------
  //----- Setup Control Flags
  //----------------------------------------

  // 8N1
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  // no flow control
  toptions.c_cflag &= ~CRTSCTS;



  //----------------------------------------
  //----- Setup Input Flags
  //----------------------------------------

  toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl


  //----------------------------------------
  //----- Setup Output Flags
  //----------------------------------------


  /* disable output processing */
  toptions.c_oflag &= ~OPOST;


  //----------------------------------------
  //----- Setup Local Flags & Others
  //----------------------------------------

  toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
  toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines

  toptions.c_cc[VMIN]  = 0;
  toptions.c_cc[VTIME] = 0;




  //===========================================================================
  //====== Save options, Reset Arudino, and Flush Serial Buffer
  //===========================================================================

  // Commit options
  tcsetattr(fd, TCSANOW, &toptions);

  if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
    printf("Couldn't update serical communication attributes.");
    return -1;
  }

  sleep(2); //required to make flush work, for some reason
  tcflush(fd, TCIOFLUSH);


  //===========================================================================
  //====== Send Message
  //===========================================================================

  // Write message
  int len = strlen(msg);
  int w = write(fd, msg, len);

  if( w != len ) {
    printf("Couldn't write string.\n");
    return -1;
  }

  // Close serial communication
  close(fd);

  printf("Sent \"%s\" to the Arudino", msg);

  return 0;
}
