#include "arg_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

/* Arg Parse function
 * Puts every argument separated by a space into an array of strings (2D char array)
 */

char** arg_parse(char* line, int *argcp) {
  //int 0 for boolean false
  int booleanLastValid = 0;
  int i = 0;
  int numArgs = 0;
  //while loop to check how much memory is needed for array
  while(line[i] != '\0') {
    if(isspace(line[i]) && booleanLastValid == 0) {
      ++i;
    } else if(isspace(line[i]) && booleanLastValid == 1) {
      booleanLastValid = 0;
      ++i;
    } else if(booleanLastValid == 0){
      booleanLastValid = 1;
      ++numArgs;
      ++i;
    } else {
      ++i;
    }
  }
  *argcp = numArgs;
  //allocate space for array and set each to the respective chars in line
  char **parsed = malloc((numArgs+1) * sizeof(char*));
  //set row pointers
  int arrayLoc = 0;
  booleanLastValid = 0;
  i = 0;
  while(line[i] != '\0') {
    if(isspace(line[i]) && booleanLastValid == 0) {
      ++i;
    } else if(isspace(line[i]) && booleanLastValid == 1) {
      booleanLastValid = 0;
      line[i]='\0';
      ++i;
    } else if(booleanLastValid == 0) {
      booleanLastValid = 1;
      parsed[arrayLoc] = &(line[i]);
      ++arrayLoc;
      ++i;
    } else {
      ++i;
    }
  }
  //once done set last array pointer to null
  parsed[arrayLoc]='\0';
  return parsed;
}
