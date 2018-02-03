#ifndef _TARGET_H_
#define _TARGET_H_

struct target_st;
typedef struct target_st target;
typedef void(*list_action)(char*);

target* new_target(char* name);
target* find_target(char* name);
void add_dependency_target(target* tgt, char* dep);
void add_rule_target(target* tgt, char* rule);
void for_each_rule(target* tgt, list_action action);
void for_each_dependency(target* tgt, list_action action);

#endif
