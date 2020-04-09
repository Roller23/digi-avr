#ifndef __TESTS_
#define __TESTS_

/*
  A simple testing framework that allows to run code tests in separate processes
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"
#define BLUE "\x1B[34m"

static struct {
  int total;
  int passed;
} tests_stats = {0, 0};

#define run_test(fn_name, code)\
  {\
    pid_t pid = fork();\
    if (pid == 0) {\
      printf(BLUE "starting " fn_name " tests...\n" RESET);\
      code\
      return 0;\
    } else {\
      int status;\
      int err = waitpid(pid, &status, 0);\
      if (err == -1) {\
        perror(RED "Error running tests for " fn_name "!" RESET);\
      } else {\
        tests_stats.total++;\
        if (status == 0) {\
          printf(GREEN fn_name " tests passed!\n" RESET);\
          tests_stats.passed++;\
        } else {\
          printf(RED fn_name " tests failed! exit status: %d\n" RESET, status);\
        }\
      }\
    }\
  }\


#define tests_summary()\
  printf("%s%d/%d tests passed\n" RESET,\
  tests_stats.passed == tests_stats.total ? GREEN : RED, tests_stats.passed, tests_stats.total);\

#endif //__TESTS__