
#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "sched/types.hpp"

extern sched::scheduler_type sched_type;
extern size_t num_threads;
extern bool show_database;
extern bool dump_database;
extern bool time_execution;
extern bool memory_statistics;
extern bool running;
extern char *program_running;

void parse_sched(char *);
void help_schedulers(void);
bool run_program(int, char **, const char *);

#endif
