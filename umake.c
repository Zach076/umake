/* CSCI 347 micro-make
 *
 * 09 AUG 2017, Aran Clauson
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* CONSTANTS */


/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
char** arg_parse(char* line);
void processline(char* line);

/* Main entry point.
 * argc    A count of command-line arguments
 * argv    The command-line argument valus
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */
int main(int argc, const char* argv[]) {

  FILE* makefile = fopen("./uMakefile", "r");

  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    if(line[0] == '\t')
      processline(&line[1]);


    linelen = getline(&line, &bufsize, makefile);
  }

  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 *
 */
void processline (char* line) {

  const pid_t cpid = fork();
  switch(cpid) {

  case -1: {
    perror("fork");
    break;
  }

  case 0: {
    execlp(line, line, (char*)(0));
    perror("execlp");
    exit(EXIT_FAILURE);
    break;
  }

  default: {
    int   status;
    const pid_t pid = wait(&status);
    if(-1 == pid) {
      perror("wait");
    }
    else if (pid != cpid) {
      fprintf(stderr, "wait: expected process %d, but waited for process %d",
              cpid, pid);
    }
    break;
  }
  }
}

/* Arg Parse function
 * Puts every argument separated by a space into an array of strings (2D char array)
 */
char** arg_parse(char* line) {
  boolean done = false;
  boolean lastValid = false;
  int length = 0;
  int i = 0;
  int numArgs = 0;
  //while loop to check how much memory is needed for array
  while(!done) {
    if(line+i == '\0') {
      done = true;
    } else if((line+i == ' ' || line+i == '\t') && !lastValid) {
      ++i;
    } else if((line+i == ' ' || line+i == '\t') && lastValid) {
      lastValid = false;
      line+i = '\0';
      ++length;
      ++numArgs;
      ++i;
    } else {
      lastValid = true;
      ++length;
      ++i;
    }
  }
  //allocate space for array and set each to the respective chars in line
  char **parsed = malloc((length+1) * sizeof(char));
  //set row pointers
  done = false;
  int arrayLoc = 0;
  lastValid = false;
  i = 0;
  while(!done) {
    if(line+i == '\0' && numArgs == 0) {
      done = true;
    } else if((line+i == ' ' || line+i == '\t') && !lastValid) {
      ++i;
    } else if((line+i == ' ' || line+i == '\t') && lastValid) {
      lastValid = false;
      --numArgs;
      ++arrayLoc;
      ++i;
    } else if(!lastValid) {
      lastValid = true;
      *parsed+arrayLoc = line+i;
      ++i;
    } else {
      ++i;
    }
  }
  //once done set last array pointer to null
  *parsed+arrayLoc = '\0';
  return **parsed;
}
