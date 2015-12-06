#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//============================= Main ==================================

int main (int argc, char **argv)
{

  if (argc > 1){      // check if port num is between 5000-65536
    int port_num = strtoimax(argv[1], NULL, 10);

    if(port_num < 5000 || port_num > 65536){
        fprintf(stderr, "Port number must between 5000-65536\n");
        return -1;
    }

  }
  else{
    fprintf(stderr, "Must insert port number between 5000-65536\n");
    return -1;
  }


}
