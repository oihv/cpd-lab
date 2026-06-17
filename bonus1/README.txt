## States
The name of the states is only stored once in a `StateNames` struct, all the other references to the name of the state will then only need to refer to the name of the state by the index of the state in this struct.

## TODO
- [x] nfa to dfa
- [x] eps nfa to nfa
- [ ] implement animation
  - [x] draw transition table
  - [x] implement step tracing for functions: nfa to dfa, eps nfa to nfa (with snapshots)
  - [ ] animate for other type of snapshots too
  - [x] nfa to dfa snapshots
  - [ ] eps nfa to nfa snapshots
- [x] WASM port
- [ ] new page for entering inputs and selection (try to integrate raygui)
