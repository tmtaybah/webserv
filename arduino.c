#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>


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
  fd = open(portname, O_RDWR | O_NOCTTY);

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

  /* 8 bits, no parity, no stop bits */
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  /* no hardware flow control */
  toptions.c_cflag &= ~CRTSCTS;
  /* enable receiver, ignore status lines */
  toptions.c_cflag |= CREAD | CLOCAL;

  //----------------------------------------
  //----- Setup Input Flags
  //----------------------------------------

  /* disable input/output flow control, disable restart chars */
  toptions.c_iflag &= ~(IXON | IXOFF | IXANY);
  /* disable canonical input, disable echo,
  disable visually erase chars,
  disable terminal-generated signals */
  toptions.c_iflag &= ~(ICANON | ECHOE | ISIG);


  //----------------------------------------
  //----- Setup Output Flags
  //----------------------------------------


  /* disable output processing */
  toptions.c_oflag &= ~OPOST;


  //----------------------------------------
  //----- Setup Local Flags
  //----------------------------------------




  //===========================================================================
  //====== Save options, Reset Arudino, and Flush Serial Buffer
  //===========================================================================

  // Commit options
  tcsetattr(fd, TCSANOW, &toptions);

  // Wait for the Arduino to reset
  usleep(1000*1000);
  // Flush anything already in the serial buffer
  tcflush(fd, TCIFLUSH);

  //===========================================================================
  //====== Send Message
  //===========================================================================

  // Write message
  int w = write(fd, msg, strlen(msg));

  // Close serial communication
  close(fd);

  printf("Sent \"%s\" to the Arudino", msg);

  return 0;
}
