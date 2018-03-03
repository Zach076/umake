#include "target.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

struct stringList_st {
  char* stringVal;
  stringList* next;
};

struct target_st {
  target* next;
  char* tgtName;
  stringList* depList;
  stringList* ruleList;
  int executed;
};

static target* tgt_list = NULL;

target* new_target(char* name) {
  target *newTarget = malloc(sizeof(target));
  newTarget->tgtName = strdup(name);
  newTarget->next = NULL;
  newTarget->depList = NULL;
  newTarget->ruleList = NULL;
  newTarget->executed = 0;

  target *currTgt = tgt_list;
  if(currTgt == NULL) {
    currTgt = newTarget;
    tgt_list = currTgt;
    return currTgt;
  } else {
    while(currTgt->next != NULL) {
      currTgt = currTgt->next;
    }
    currTgt->next = newTarget;
  }
  return currTgt->next;
}

target* find_target(char* name) {
  target *currTgt = tgt_list;
  while(currTgt->next != NULL) {
    if(!strcmp(currTgt->tgtName, name)){
      return currTgt;
    } else {
      currTgt = currTgt->next;
    }
  }
  if(!strcmp(currTgt->tgtName, name)){
    return currTgt;
  } else {
    fprintf(stderr, "The target %s doesn't exist", name);
    exit(EXIT_FAILURE);
  }
}

void add_dependency_target(target* tgt, char* dep) {
  stringList *newDep = malloc(sizeof(stringList));
  newDep->stringVal = strdup(dep);
  newDep->next = NULL;

  stringList *currDep;
  currDep = tgt->depList;
  if(currDep == NULL) {
    tgt->depList = newDep;
  } else {
    while(currDep->next != NULL) {
      currDep = currDep->next;
    }
    currDep->next = newDep;
  }
}

void add_rule_target(target* tgt, char* rule) {
  stringList *newRule = malloc(sizeof(stringList));
  newRule->stringVal = strdup(rule);
  newRule->next = NULL;

  stringList *currRule;
  currRule = tgt->ruleList;
  if(currRule == NULL) {
    tgt->ruleList = newRule;
  } else {
    while(currRule->next != NULL) {
      currRule = currRule->next;
    }
    currRule->next = newRule;
  }
}

void for_each_rule(target* tgt, list_action action) {
  stringList *currRule = tgt->ruleList;
  while(currRule->next != NULL) {
    action(currRule->stringVal);
    currRule = currRule->next;
  }
  action(currRule->stringVal);
}

void for_each_dependency(target* tgt, list_action action) {
  stringList* currDep = tgt->depList;
  if(currDep != NULL && currDep->stringVal != NULL) {
    while(currDep->next != NULL) {
      action(currDep->stringVal);
      currDep = currDep->next;
    }
    action(currDep->stringVal);
  }
}

time_t timeOf(target* tgt) {
  struct stat* buf = malloc(sizeof(struct stat));
  time_t *tgtTime = malloc(sizeof(time_t));
  char* fileName = malloc(sizeof(*(tgt->tgtName))+2);

  strcpy(fileName, "./");
  if(!stat(strcat(fileName, tgt->tgtName), buf)) {
    *tgtTime = buf->st_mtime;
  } else {
    fprintf(stderr, "The file %s doesn't exist timeOf", fileName);
    exit(EXIT_FAILURE);
  }

  free(buf);
  free(fileName);
  return *tgtTime;
}

target* getNext(target* tgt) {
  return tgt->next;
}

char* getName(target* tgt) {
  return tgt->tgtName;
}

void setExecuted(target* tgt, int i) {
  tgt->executed = i;
}

int isExecuted(target* tgt) {
  return tgt->executed;
}

char* getDep_i(target* tgt, int i) {
  int j = 0;
  stringList* currDep = tgt->depList;
  if(currDep != NULL && currDep->stringVal != NULL) {
    while(j != i) {
      currDep = currDep->next;
      ++j;
    }
    return currDep->stringVal;
  }
  fprintf(stderr, "Deplist empty");
  exit(EXIT_FAILURE);
}
