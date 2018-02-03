#include "target.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

struct stringList {
  char* stringVal;
  struct stringList* next;
};

struct target_st {
  struct target_st* next;
  char* tgtName;
  struct stringList* depList;
  struct stringList* ruleList;
};

global target* tgt_list = NULL;

target* new_target(char* name) {
  target newTarget = malloc(sizeof(target));
  newTarget->tgtName = strdup(*name);
  newTarget->next = NULL;
  newTarget->depList = NULL;
  newTarget->ruleList = NULL;
  
  target *currTgt = &tgt_list;
  if(*currTgt == NULL) {
    *tgt_list = newTarget;
  } else {
    while(*currTgt->next != NULL) {
      *currTgt = *currTgt->next;
    }
    *currTgt->next = newTarget;
  }
  return *tgt_list;
}

target* find_target(char* name) {
  target *currTgt = &tgt_list;
  while(*currTgt->next != NULL) {
    if(!strcmp(*currTgt->tgtName, *name)){
      return currTgt;
    } else {
      *currTgt = *currTgt->next;
    }
  }
}

void add_dependency_target(target* tgt, char* dep) {
  stringList *newDep = malloc(sizeof(stringList));
  *newDep->stringVal = strdup(*dep);
  *newDep->next = NULL;
    
  stringList *currDep = &depList;
  if(*currDep == NULL) {
    *depList = newDep;
  } else {
    while(*currDep->next != NULL) {
      *currDep = *currDep->next;
    }
    *currDep->next = newDep;
  }
}

void add_rule_target(target* tgt, char* rule) {
  stringList *newRule = malloc(sizeof(stringList));
  *newRule->stringVal = strdup(*rule);
  *newRule->next = NULL;
    
  stringList *currRule = &ruleList;
  if(*currRule == NULL) {
    *ruleList = newRule;
  } else {
    while(*currRule->next != NULL) {
      *currRule = *currRule->next;
    }
    *currRule->next = newRule;
  }
}

void for_each_rule(target* tgt, list_action action) {
  stringList currRule = tgt->ruleList
  while(currRule->next != NULL) {
    action(currRule->stringVal);
    currRule = currRule->next;
  }
}

void for_each_dependency(target* tgt, vlist_action action) {
  stringList currDep = tgt->depList
  while(currDep->next != NULL) {
    action(currDep->stringVal);
    currDep = currDep->next;
  }
}
