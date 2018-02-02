#include "target.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

struct target_st {
  struct target_st* next;
  char* name;
  char** dep_list;
  int dep_len;
  char** rule_list;
  int rule_len;
};

target* new_target(char* name) {
  struct target_st* target;
  strdup(target.name, *name);
  target->dep_last = 0;
  target->rule_last = 0;
  target->dep_list = malloc(target.dep_list, sizeof(char*)*(dep_len));
  target->rule_list = malloc(target.rule_list, sizeof(char*)*(rule_len));
  return target;
}

target* find_target(char* name) {
  //honestly dont know
}

void add_dependency_target(target* tgt, char* dep) {
  tgt.dep_list = realloc(tgt.dep_list, sizeof(char*)*(dep_len+1));
  tgt.dep_list[dep_len++] = dep;
}

void add_rule_target(target* tgt, char* rule) {
  tgt.rule_list = realloc(tgt.rule_list, sizeof(char*)*(rule_len+1));
  tgt.rule_list[rule_len++] = rule;
}
void for_each_rule(target* tgt, void(*action)(char*)) {

}

void for_each_dependency(target* tgt, void(*action)(char*)) {

}
