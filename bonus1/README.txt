## States
The name of the states is only stored once in a `StateNames` struct, all the other references to the name of the state will then only need to refer to the name of the state by the index of the state in this struct.

## TODO
- [x] nfa to dfa
- [x] eps nfa to nfa
- [ ] implement animation
  - [x] draw transition table
  - [ ] implement step tracing for functions: nfa to dfa, eps nfa to nfa (with snapshots)
  - [ ] 
