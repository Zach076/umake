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
#include <string.h>
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
void executeTarget(char* tgtName);
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
  target* tgtList = NULL;
  int first = 1;
  int envLine = 0;

  while(-1 != linelen) {
    i = 0;
    start = 0;

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    envLine = 0;
    if(line[0] == '\t') {
      add_rule_target(currTgt, line);
    } else {
      while(!envLine && line[i] != '\0') {
        if(line[i] == ':') {
          line[i] = '\0';
          currTgt = new_target(&line[start]);
          if(first) {
            first = 0;
            tgtList = find_target(&line[start]);
          }
        } else if(line[i] == '=') {
          line[i] = '\0';
          setenv(&line[start], &line[i+1], 1);
          envLine = 1;
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
      if(start != 0) {
        add_dependency_target(currTgt, &line[start]);
      }
    }

    linelen = getline(&line, &bufsize, makefile);
  }
  i=0;
  while(tgtList != NULL && getNext(tgtList) != NULL) {
    if(!isExecuted(tgtList)) {
      executeTarget(getName(tgtList));
    }
    tgtList = getNext(tgtList);
  }

  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 *
 */
void processline (char* line) {

  int* argcp = malloc(sizeof(int));
  char* expandedLine = malloc(1024 * sizeof(char));
  if(!expand(line, expandedLine, 1024)) {
    return;
  }
  char** argumentArray = arg_parse(expandedLine, argcp);
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
  int newLen = 0;
  int lookingForEnd = 0;
  while(orig[i] != '\0') {
    if(orig[i] == '$' && orig[i+1] == '{' && !lookingForEnd) {
      lookingForEnd = 1;
      start = i;
    } else if(orig[i] == '}' && lookingForEnd) {
      lookingForEnd = 0;
      if(newLen + (i-start+2) < newsize) {
        newLen = newLen + sprintf(&new[newLen], "%s", getenv(strndup(&orig[start+2], i-(start+2))));
      } else {
        fprintf(stderr, "Buffer overflow while expanding environment variables.");
        return 0;
      }
    } else if(!lookingForEnd) {
      if(newLen + 1 < newsize) {
        newLen = 1 + newLen + (0 * snprintf(&new[newLen], 2, "%s", &orig[i]));
      } else {
        fprintf(stderr, "Buffer overflow while expanding environment variables.");
        return 0;
      }
    }
    ++i;
  }
  if(lookingForEnd) {
    fprintf(stderr, "No \'}\' found. Mismatched braces.");
    return 0;
  }
  return 1;
}

void executeTarget(char* tgtName) {
  target* tgt = NULL;
  tgt = find_target(tgtName);
  if(tgt != NULL) {
    for_each_dependency(tgt, executeTarget);

    tgt = find_target(tgtName);
    if(!isExecuted(tgt)) {
      setExecuted(tgt, 1);
      for_each_rule(tgt, processline);
    }
  }
}
