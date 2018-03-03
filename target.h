#ifndef _TARGET_H_
#define _TARGET_H_

#include <time.h>

struct stringList_st;
struct target_st;
typedef struct target_st target;
typedef struct stringList_st stringList;
typedef void(*list_action)(char*);

//makes a new target, appends it to target list and returns a pointer to the new target
target* new_target(char* name);

//given a target name will return a pointer to a target with the same name.
//If target name doesn't match any targets print error and exit.
target* find_target(char* name);

//Add dep name to depList of target given
void add_dependency_target(target* tgt, char* dep);

//Add rule to ruleList of target given
void add_rule_target(target* tgt, char* rule);

//for each rule the given target has, perform some action
void for_each_rule(target* tgt, list_action action);

//for each dep in given target, perform some action
void for_each_dependency(target* tgt, list_action action);

//will return mod time of given target assuming filename = ./tgtName
//if filename cannot be found, exit with proper error
time_t timeOf(target* tgt);

//will return pointer to next target in tgtlist
target* getNext(target* tgt);

//returns name of given tgt
char* getName(target* tgt);

//set executed int in tgt to int i
void setExecuted(target* tgt, int i);

//get executed flag from given tgt
int isExecuted(target* tgt);

//get pointer to the i'th dependency name
char* getDep_i(target* tgt, int i);

#endif
