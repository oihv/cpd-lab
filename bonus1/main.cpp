#include <stdint.h>
#include <stdio.h>

#include "lib.cpp"

int main(int argc, char **argv) {
  if (argc == 0) {
    fprintf(stderr, "ERROR: please provide the text file for the NFA\n");
    return 1;
  } else if (argc == 1) {
    fprintf(stderr,
            "ERROR: please provide the text file for the DFA answer key\n");
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "ERROR: file can't be opened (file name: %s)", argv[1]);
    return 1;
  }

  // NFA
  NFA nfa_eps = {0};

  nfa_parse(&nfa, fp);

  DFA dfa_res;
  nfa_to_dfa(&nfa, &dfa_res);
  // dfa_print(&dfa_res, nfa.states.names);

  FILE *f_test = fopen(argv[2], "r");
  if (!f_test) {
    fprintf(stderr, "ERROR: file can't be opened (file name: %s)", argv[2]);
    return 1;
  }

  DFA dfa_key = {0};
  dfa_parse(&dfa_key, &dfa_res.states, f_test, &nfa.states);

  if (dfa_compare(&dfa_res, &dfa_key)) {
    printf("anjay bener woi\n");
  } else {
    dfa_print(&dfa_res, nfa.states.names);
  }

  fclose(fp);

  return 0;
}
