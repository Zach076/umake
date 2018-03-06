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
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
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
int outOfDate(target* tgt);

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

  char* fileName = "./uMakefile";
  FILE* makefile;

  size_t  bufsize = 0;
  char*   line    = NULL;
  int i = 0;
  int start = 0;
  int lastSpace = 0;
  target *currTgt = NULL;
  target* tgtList = NULL;
  int first = 1;
  int envLine = 0;

  if(access(fileName, R_OK) != -1) {
    makefile = fopen(fileName, "r");
  }else{
    fprintf(stderr, "The file %s doesn't exist", fileName);
    exit(EXIT_FAILURE);
  }

  ssize_t linelen = getline(&line, &bufsize, makefile);

  while(-1 != linelen) {
    i = 0;
    start = 0;

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    while(line[i] != '#' && line[i] != '\0') {
        ++i;
    }

    line[i] = '\0';

    i=0;
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

  int i = 0;
  int in = 0;
  int out = 1;
  int closeIn = 0;
  int closeOut = 0;

  if(*argcp) {

    const pid_t cpid = fork();

    switch(cpid) {

    case -1: {
      perror("fork");
      break;
    }

    case 0: {
      
      while(argumentArray[i] != NULL && strcmp(argumentArray[i], ">>") && strcmp(argumentArray[i], ">")) {
        ++i;
      }
      //set output to argumentArray[i+1] and argumentArray[i] to null
      if(argumentArray[i] != NULL) {
        if(!strcmp(argumentArray[i], ">>")) {
          out = open(argumentArray[i+1], O_WRONLY | O_CREAT, 0666);
          closeOut = 1;
        } else if(!strcmp(argumentArray[i], ">")){
          out = open(argumentArray[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0666);
          closeOut = 1;
        }
        dup2(out, 1);
        argumentArray[i] = NULL;
      }

      i=0;
      while(argumentArray[i] != NULL && strcmp(argumentArray[i], "<")) {
        ++i;
      }
      //set input to argumentArray[i+1] and argumentArray[i] to null
      if(argumentArray[i] != NULL) {
        if(!strcmp(argumentArray[i], "<")) {
          in = open(argumentArray[i+1], O_RDONLY);
          closeIn = 1;
          dup2(out, 1);
        }
        argumentArray[i] = NULL;
      }

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
    if(closeOut) {
      close(out);
      closeOut = 0;
    }
    if(closeIn) {
      close(in);
      closeIn = 0;
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
    if(!isExecuted(tgt) && outOfDate(tgt)) {
      setExecuted(tgt, 1);
      for_each_rule(tgt, processline);
    }
  }
}

int outOfDate(target* tgt) {
  char* tgtFileName = malloc(sizeof(getName(tgt))+2);
  char* depFileName = malloc(sizeof(char)+2);
  time_t tgtTime;

  //if the target file doesn't exist, return tru for update
  strcpy(tgtFileName, "./");
  strcat(tgtFileName, getName(tgt));
  if(!access(tgtFileName, R_OK)) {
    tgtTime = timeOf(tgt);
  } else {
    //FILE NOT FOUND UPDATE NEEDED
    return 1;
  }
  //if dependency list is empty return false for update
  int i = 0;
  if(getDep_i(tgt, i) == NULL) {
    return 0;
  }
  //return true for update once a dependency is found to be more recent
  time_t depTime = timeOf(find_target(getDep_i(tgt, i)));
  while(getDep_i(tgt, i) != NULL ) {
    free(depFileName);
    depFileName = malloc(sizeof(getDep_i(tgt, i))+2);
    strcpy(tgtFileName, "./");
    if(!access(strcat(tgtFileName, getName(tgt)), R_OK)) {
      //DEPENDENCY FILE NOT FOUND UPDATE NEEDED
      return 1;
    }
    depTime = timeOf(find_target(getDep_i(tgt, i)));
    if(difftime(tgtTime, depTime) < 0) {
      return 1;
    }
    ++i;
  }

  return 0;
}
