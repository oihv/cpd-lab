#include <stdio.h>
#include <stdint.h>
#include "lib.cpp"

int main(int argc, char* argv[]) {
  FILE* fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "ERROR: can't open file %s\n", argv[1]);
    return 1;
  }

  NFA nfa = {0};
  eps_nfa_parse(&nfa, fp);
  printf("----------------eps-NFA:---------------\n");
  nfa_print(&nfa, nfa.states.names);
  eps_nfa_to_nfa(&nfa);
  printf("----------------Regular NFA:---------------\n");
  nfa_print(&nfa, nfa.states.names);

}
