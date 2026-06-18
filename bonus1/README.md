# Compiler Design and Principles Bonus Assignment
NFA to DFA conversion algorithm, extended to also cover E-NFA.

## Run
On linux, run `./run.sh`.
On windows, run `./run.bat`.
If `Justfile` is available, `just run gui` will do.
To run the build version for the web, locally, run `python3 -m http.server`
Or you can access the hosted version in [github pages](oihv.github.io/cpd-lab)

## Controls
Left and right arrow key to go back and forward.
Space for pause.

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
- [x] new page for entering inputs and selection (try to integrate raygui)
- [ ] show error on screen (if there's any)
- [ ] mark start input with an arrow
- [ ] mark final input with a bounding box
- [ ] implement more details in the history (e.g. go in depth on epsilon closure, subset construction)
- [ ] there's a bug for e-nfa1.txt with algo ENFA to DFA, the unchecked_states shows incorrect values

## Codebase Introduction

## States
The name of the states is only stored once in a `StateNames` struct, all the other references to the name of the state will then only need to refer to the name of the state by the index of the state in this struct.
