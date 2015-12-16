#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



/*
* Adapted form http://chrisheydrick.com/2012/06/17/how-to-read-serial-data-from-an-arduino-in-linux-with-c-part-3/
* And the sample code provided by the professor
*/


// Arudino constants
char *portname = "/dev/cu.usbmodem1451";


// For message
#define MAX_CHAR 50
char msg[MAX_CHAR]; // Maximum number of character to print out to LCD


int main(int argc, char *argv[])
{

  //===========================================================================
  //====== Get Message
  //===========================================================================

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
  if ((fd = open(portname, O_RDWR | O_NOCTTY)) == -1)
  {
    perror("Error");
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
    perror("init_serialport: Couldn't set term attributes");
    return -1;
  }

  // Wait for the Arduino to reset
  usleep(1000*1000);


  //===========================================================================
  //====== Send Message
  //===========================================================================

  // Write message
  int w = write(fd, msg, strlen(msg));

  // Close serial communication
  close(fd);

  printf("Sent \"%s\" to the Arudino", msg);

  fprintf(stderr, "DONE WITH SENDING");

  return 0;
}
