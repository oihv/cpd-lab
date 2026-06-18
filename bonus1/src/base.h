#ifndef BASE_H
#define BASE_H

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "time.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 10
#define MAX_STATE_LENGTH 5
#define MAX_ALPHABETS 5
#define MAX_ALPHABET_LENGTH 5
#define MAX_FINAL_STATES 5
#define MAX_STATE_SET 8
#define MAX_QUEUE_SIZE 10
#define MAX_SNAP_NUMS 256

// Types
typedef enum { Ok, Err } Result;

typedef struct {
  size_t item[MAX_QUEUE_SIZE];
  size_t pop_idx;
  size_t count;
} Queue;

typedef struct {
  size_t state_idxs[MAX_STATES];
  size_t count;
} StateList;

typedef struct {
  StateList set[MAX_STATES];
  size_t count;
} StateSetList;

typedef struct {
  char names[MAX_STATES][MAX_STATE_LENGTH];
  size_t count;
} StateNames;

typedef struct {
  char names[MAX_ALPHABETS][MAX_ALPHABET_LENGTH];
  uint8_t count;
} AlphabetNames;

typedef struct {
  StateNames states;
  AlphabetNames alphabets;
  size_t start_idx;
  StateList final;
  StateList transition[MAX_STATES][MAX_ALPHABETS]; // points to a set of states
} NFA;

typedef struct {
  StateSetList states; // group of states in the global state
  AlphabetNames alphabets;
  size_t start_idx;
  StateList final; // index of the states in this struct
  size_t transition[MAX_STATES][MAX_ALPHABETS]; // points to the "state set"
} DFA;

typedef enum {
  STEP_EPS_CLOSURE,    // Highlighting epsilon search
  STEP_QUEUE_POP,      // Highlighting state being processed
  STEP_NEW_STATE,      // Highlighting state added to queue
  STEP_ADD_TRANSITION, // Showing result moving to DFA
  STEP_SUBSET_CONSTRUCTION
} StepType;

typedef enum { NFA_TO_DFA, EPS_NFA_TO_NFA } AlgoType;

typedef struct {
  StepType type;
  AlgoType algo_type;
  NFA nfa;
  NFA* nfa_src;
  DFA dfa;
  bool dfa_transition_filled[MAX_STATES]
                            [MAX_ALPHABETS]; // determine whether or not
  Queue queue;
  size_t active_state; // index of the state wanted to be highlighted
  size_t active_alphabet;
  StateList new_state; 
  StateList eps_closure;
  char description[128];
} Snapshot;

typedef struct {
  Snapshot steps[MAX_SNAP_NUMS];
  int count;
} History;

// API
// utils
Result queue_push(Queue *q, size_t new_item);
bool queue_is_empty(Queue *q);
Result queue_pop(Queue *q, size_t *item);
void queue_to_state_list(Queue *q, StateList *s);

void state_list_sprint(char names[][MAX_STATE_LENGTH], StateList *s, char *str);
void transition_filled_copy(bool dst[][MAX_ALPHABETS],
                            bool src[][MAX_ALPHABETS]);

#endif // !BASE_H
