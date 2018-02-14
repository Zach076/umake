/* CSCI 347 micro-make
 *
 * 09 AUG 2017, Aran Clauson
 * Modified by Zach Richardson
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "arg_parse.h"
#include "target.h"

/* CONSTANTS */


/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
int expand(char* orig, char* new, int newsize);
void executeTarget(target* tgt);
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
  int i = 0;
  int start = 0;
  int lastSpace = 0;
  target *currTgt = NULL;

  while(-1 != linelen) {
    i = 0;
    start = 0;

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    if(line[0] == '\t') {
      add_rule_target(currTgt, line);
    } else {
      while(line[i] != '\0') {
        if(line[i] == ':') {
          line[i] = '\0';
          currTgt = new_target(&line[start]);
        } else if(line[i] == '=') {
          line[i] = '\0';
          setenv(line[start], line[i+1], 1);
        } else if(line[i] == ' ') {
          line[i] = '\0';
          lastSpace = 1;
          if(start != 0) {
            add_dependency_target(currTgt, &line[start]);
          }
        } else if(lastSpace == 1) {
          start = i;
          lastSpace = 0;
        }
        ++i;
      }
    }

    linelen = getline(&line, &bufsize, makefile);
  }
  executeTarget(getTargets());

  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 *
 */
void processline (char* line) {

  int* argcp = malloc(sizeof(int));
  char** argumentArray = arg_parse(line, argcp);
  if(*argcp) {

    const pid_t cpid = fork();
    switch(cpid) {

    case -1: {
      perror("fork");
      break;
    }

    case 0: {
      execvp(*argumentArray, argumentArray);
      perror("execvp");
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
  free(argumentArray);
}

int expand(char* orig, char* new, int newsize) {
  int i = 0;
  int start = 0;
  int lookingForEnd = 0;
  while(orig[i] != '\0') {
    if(orig[i] == '$' && orig[i+1] == '{') {
      lookingForEnd = 1;
      start = i;
    } else if(orig[i] == '}' && lookingForEnd) {
      getenv(strndup(orig[start+2], i-(start+2)));
    } else if(!lookingForEnd) {
      //put char into new
    }
    i++
  }
  if(lookingForEnd) {
    //put orig[start] into new
  }
}

void executeTarget(target* tgt) {
  for_each_dependency(tgt, executeTarget);

  for_each_rule(tgt, processline);
}
