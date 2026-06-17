#include "raylib.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

#define MAX_STATES 10
#define MAX_STATE_LENGTH 5
#define MAX_ALPHABETS 5
#define MAX_ALPHABET_LENGTH 5
#define MAX_FINAL_STATES 5
#define MAX_STATE_SET 8

#define MAX_QUEUE_SIZE 10

typedef enum { Ok, Err } Result;

typedef struct {
  size_t item[MAX_QUEUE_SIZE];
  size_t pop_idx;
  size_t count;
} Queue;

Result queue_push(Queue *q, size_t new_item) {
  if (q->count == MAX_QUEUE_SIZE)
    return Err;
  q->item[q->count++ % MAX_QUEUE_SIZE] = new_item;
  q->count = q->count % MAX_QUEUE_SIZE;
  return Ok;
}

Result queue_pop(Queue *q, size_t *item) {
  if (q->count == 0)
    return Err;
  *item = q->item[q->pop_idx++ % MAX_QUEUE_SIZE];
  q->pop_idx = q->pop_idx % MAX_QUEUE_SIZE;
  q->count--;
  if (q->count == 0)
    q->pop_idx = 0;
  return Ok;
}

bool queue_is_empty(Queue *q) { return q->count == 0; }

typedef struct {
  size_t state_idxs[MAX_STATES];
  size_t count;
} StateList;

// Return Err when not found
Result find_state(char *name, char states[][MAX_STATE_LENGTH],
                  uint8_t state_count, size_t *idx) {
  for (int i = 0; i < state_count; i++) {
    if (!strncmp(name, states[i], strlen(name))) {
      *idx = i;
      return Ok;
    }
  }
  return Err;
}

int compare(const void *a, const void *b) {
  int int_a = *((int *)a);
  int int_b = *((int *)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

typedef struct {
  StateList set[MAX_STATES];
  size_t count;
} StateSetList;

int compare_state_set(const void *a, const void *b) {
  StateSetList ssl_a = *((StateSetList *)a);
  StateSetList ssl_b = *((StateSetList *)b);

  if (ssl_a.count == ssl_b.count)
    return 0;
  else if (ssl_a.count < ssl_b.count)
    return 1;
  else
    return -1;
}

Result find_state_list(StateSetList *states, StateList *s, size_t *idx) {
  if (states->count == 0)
    return Err;
  for (int i = 0; i < states->count; i++) {
    if (states->set[i].count != s->count)
      continue;
    for (int j = 0; j < s->count; j++) {
      if (states->set[i].state_idxs[j] != s->state_idxs[j])
        continue;
      *idx = i;
      return Ok;
    }
  }
  return Err;
}

bool state_list_contain_idx(size_t *key_idx, StateList *list) {
  for (int i = 0; i < list->count; i++) {
    if (*key_idx == list->state_idxs[i]) {
      return true;
    }
  }
  return false;
}

void state_list_print(char names[][MAX_STATE_LENGTH], StateList *s) {
  if (s->count == 0) {
    printf("-");
    return;
  }
  printf("{");
  for (int j = 0; j < s->count; j++) {
    printf("%s", names[s->state_idxs[j]]);
    if (j != s->count - 1)
      printf(" ");
  }
  printf("}");
}

void state_list_sprint(char names[][MAX_STATE_LENGTH], StateList *s,
                       char *str) {
  if (s->count == 0) {
    sprintf(str + strlen(str), "-");
    return;
  }
  sprintf(str + strlen(str), "{");
  for (int j = 0; j < s->count; j++) {
    sprintf(str + strlen(str), "%s", names[s->state_idxs[j]]);
    if (j != s->count - 1)
      sprintf(str + strlen(str), " ");
  }
  sprintf(str + strlen(str), "}");
}

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

StateSetList nfa_default_state_set_list(NFA *nfa) {
  StateSetList res = {.count = nfa->states.count};
  for (int i = 0; i < res.count; i++) {
    res.set[i].state_idxs[0] = i;
    res.set[i].count = 1;
  }
  return res;
}

Result nfa_parse(NFA *nfa, FILE *fp) {
  char buff[64];
  while (fgets(buff, 64, fp)) {
    if (!strncmp(buff, "States:", 7)) {
      char *token = strtok(buff + 7, " \t\n\r");
      while (token) {
        strcpy(nfa->states.names[nfa->states.count++], token);
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Alphabet:", 9)) {
      char *token = strtok(buff + 9, " \t\n\r");
      while (token) {
        strcpy(nfa->alphabets.names[nfa->alphabets.count++], token);
        token = strtok(NULL, " \t\n\r");
      }
      // put "eps" in the last slot, if there's any
      for (int i = 0; i < nfa->alphabets.count; i++) {
        if (!strncmp(nfa->alphabets.names[i], "eps", 3)) {
          if (i != nfa->alphabets.count - 1) {
            char *last = nfa->alphabets.names[nfa->alphabets.count - 1];
            char *eps = nfa->alphabets.names[i];
            char temp_str[MAX_ALPHABET_LENGTH] = {0};
            strcpy(temp_str, last);
            strcpy(last, eps);
            strcpy(eps, temp_str);
          }
          break;
        }
      }
    } else if (!strncmp(buff, "Start:", 6)) {
      char *token = strtok(buff + 6, " \t\n\r");
      if (token) {
        if (find_state(token, nfa->states.names, nfa->states.count,
                       &nfa->start_idx) != Ok) {
          fprintf(stderr,
                  "ERROR: Start state is not in States (Start state: %s)\n",
                  token);
          return Err;
        }
      }
    } else if (!strncmp(buff, "Final:", 6)) {
      char *token = strtok(buff + 6, " \t\n\r");
      while (token) {
        if (find_state(token, nfa->states.names, nfa->states.count,
                       &nfa->final.state_idxs[nfa->final.count++]) != Ok) {
          fprintf(stderr,
                  "ERROR: Final state is not in States (Final state: %s)\n",
                  token);
          return Err;
        }
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Transitions:", 12)) {
      while (fgets(buff, sizeof(buff), fp)) {
        char *token = strtok(buff, " \t\n\r");
        size_t idx;
        if (find_state(token, nfa->states.names, nfa->states.count, &idx) !=
            Ok) {
          fprintf(stderr,
                  "ERROR: Transition from state is not in States (from state: "
                  "%s)\n",
                  token);
          return Err;
        }
        size_t from_idx = idx;
        token = strtok(NULL, " \t\n\r");
        if (find_state(token, nfa->alphabets.names, nfa->alphabets.count,
                       &idx) != Ok) {
          fprintf(
              stderr,
              "ERROR: Transition alphabets not in Alphabets (alphabet: %s)\n",
              token);
          return Err;
        }
        size_t alphabet_idx = idx;
        token = strtok(NULL, " \t\n\r");
        StateList *s = &nfa->transition[from_idx][alphabet_idx];
        while (token) {
          if (!strncmp(token, "-", 1))
            break; // skip "-"
          if (find_state(token, nfa->states.names, nfa->states.count, &idx) !=
              Ok) {
            fprintf(
                stderr,
                "ERROR: Transition to state is not in States (to state: %s)\n",
                token);
            return Err;
          }
          assert(s->count + 1 <= MAX_STATES);
          s->state_idxs[s->count++] = idx;
          token = strtok(NULL, " \t\n\r");
        }
        qsort(s->state_idxs, s->count, sizeof(size_t), compare);
      }
    }
  }
  return Ok;
}

// eps-NFA is just an NFA that have eps as a transition alphabet
Result eps_nfa_parse(NFA *nfa, FILE *fp) {
  nfa_parse(nfa, fp);
  bool found = false;
  for (int i = 0; i < nfa->alphabets.count; i++) {
    if (!strncmp(nfa->alphabets.names[i], "eps", 3)) {
      found = true;
      break;
    }
  }
  if (!found) {
    fprintf(stderr, "ERROR: %s: eps-NFA don't have eps transition alphabet\n",
            __FUNCTION__);
    return Err;
  }
  return Ok;
}

Result eps_nfa_to_nfa(NFA *nfa) {
  size_t eps_i = 0;
  bool found = false;
  for (int j = 0; j < nfa->alphabets.count; j++) {
    if (!strncmp(nfa->alphabets.names[j], "eps", 3)) {
      eps_i = j;
      found = true;
      break;
    }
  }
  if (!found) {
    fprintf(stderr, "ERROR: %s: eps-NFA don't have eps transition alphabet\n",
            __FUNCTION__);
  }
  for (int i = 0; i < nfa->states.count; i++) {
    StateList eps_closure = {0};
    Queue unchecked_states = {0};
    eps_closure.state_idxs[eps_closure.count++] = i;
    queue_push(&unchecked_states, i);
    // compute eps closure
    while (!queue_is_empty(&unchecked_states)) {
      size_t curr = queue_pop(&unchecked_states, &curr);
      for (int j = 0; j < nfa->transition[i][eps_i].count; j++) {
        size_t to = nfa->transition[i][eps_i].state_idxs[j];
        if (!state_list_contain_idx(&to, &eps_closure)) {
          eps_closure.state_idxs[eps_closure.count++] = to;
          queue_push(&unchecked_states, to);
        }
      }
    }
    // add state to final, if the closure contains one of the final states
    for (int j = 0; j < eps_closure.count; j++) {
      if (state_list_contain_idx(&eps_closure.state_idxs[j], &nfa->final)) {
        if (!state_list_contain_idx((size_t *)&i, &nfa->final)) {
          nfa->final.state_idxs[nfa->final.count++] = i;
        }
      }
    }
    // subset construction
    // for each alphabet
    for (int j = 0; j < nfa->alphabets.count; j++) {
      StateList new_s = {0};
      // for each state in "from"
      for (int k = 0; k < eps_closure.count; k++) {
        StateList *to = &nfa->transition[eps_closure.state_idxs[k]][j];
        // for each state in "to"
        for (int l = 0; l < to->count; l++) {
          if (!state_list_contain_idx(&to->state_idxs[l], &new_s)) {
            new_s.state_idxs[new_s.count++] = to->state_idxs[l];
          };
        }
      }
      qsort(new_s.state_idxs, new_s.count, sizeof(size_t), compare);
      nfa->transition[i][j] = new_s;
    }
  }
  qsort(nfa->final.state_idxs, nfa->final.count, sizeof(size_t), compare);
  nfa->alphabets.count--;
  return Ok;
}

// get passed "q0 q1}, "
// also inserts the parsed state into the states var
Result state_set_parse(StateList *states, char *buff, StateNames *name_list) {
  states->count = 0;
  // char buff_cp[MAX_STATE_LENGTH];
  // strcpy(buff_cp, buff);

  if (!buff || *buff == '\0')
    return Err;
  while (buff[strlen(buff) - 1] != '}') {
    if (strlen(buff) == 1) {
      fprintf(stderr,
              "ERROR: when parsing DFA, a state set is not terminated by "
              "closing parentheses '}'\n");
      return Err;
    }
    buff[strlen(buff) - 1] = '\0';
  }
  // buff[strlen(buff) - 1] = '\0';

  char *ptr;
  char *token = strtok_r(buff, " ", &ptr);
  bool end = false;

  while (token && !end) {
    if (token[strlen(token) - 1] == '}') {
      token[strlen(token) - 1] = '\0';
      end = true;
    }
    if (token[0] == '{') {
      token++; // skip '{'
    }

    size_t idx;
    if (find_state(token, name_list->names, name_list->count, &idx) == Err) {
      fprintf(stderr,
              "ERROR: when parsing DFA, can't find state name %s in the global "
              "name list\n",
              token);
      return Err;
    }

    assert(states->count + 1 <= MAX_STATES);
    states->state_idxs[states->count++] = idx;

    token = strtok_r(NULL, " ", &ptr);
  }
  qsort(states->state_idxs, states->count, sizeof(size_t), compare);

  return Ok;
}

// Assuming DFA is parsed after getting the global name list
// Other DFA is also needed because is also needed because there needs to be a
// standard for the sequence of the state sets
Result dfa_parse(DFA *dfa, StateSetList *state_set, FILE *fp,
                 StateNames *name_list) {
  const size_t buff_length = 128;
  char buff[buff_length];
  while (fgets(buff, buff_length, fp)) {
    if (!strncmp(buff, "States:", 7)) {
      char *ptr;
      char *token = strtok_r(buff + 8, "{", &ptr);
      while (token) {
        if (!strcmp(token, " ")) {
          token = strtok_r(NULL, "{", &ptr);
        }
        char tok_cp[buff_length];
        strcpy(tok_cp, token);
        state_set_parse(&dfa->states.set[dfa->states.count++], tok_cp,
                        name_list);
        token = strtok_r(NULL, "{", &ptr);
      }
      // match the state set with the other state set
      size_t idx;
      for (int i = 0; i < dfa->states.count; i++) {
        if (find_state_list(state_set, &dfa->states.set[i], &idx) != Ok) {
          fprintf(stderr,
                  "ERROR: parsed DFA didn't contain the same state set\n");
          return Err;
        }
        memcpy(&dfa->states.set[i], &state_set->set[idx], sizeof(StateList));
      }
    } else if (!strncmp(buff, "Alphabet:", 9)) {
      char *token = strtok(buff + 9, " \t\n\r");
      while (token) {
        strcpy(dfa->alphabets.names[dfa->alphabets.count++], token);
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Start:", 6)) {
      char *token = strtok(buff + 6, "{");
      if (token) {
        token++;                         // remove '{'
        token[strlen(token) - 1] = '\0'; // remove '}'
        if (find_state(token, name_list->names, dfa->states.count,
                       &dfa->start_idx) != Ok) {
          fprintf(stderr,
                  "ERROR: Start state is not in States (Start state: %s)\n",
                  token);
          return Err;
        }
      }
    } else if (!strncmp(buff, "Final:", 6)) {
      char *ptr;
      char *token = strtok_r(buff + 6, "{", &ptr);
      while (token) {
        if (!strcmp(token, " ")) {
          token = strtok_r(NULL, "{", &ptr);
        }
        size_t idx;
        StateList curr_list = {0};
        if (state_set_parse(&curr_list, token, name_list) != Ok) {
          fprintf(stderr, "%s: can't find final state (%s)\n", __FUNCTION__,
                  token);
        }
        qsort(&dfa->final.state_idxs, dfa->final.count, sizeof(size_t),
              compare);
        if (find_state_list(&dfa->states, &curr_list, &idx) != Ok) {
          fprintf(stderr,
                  "ERROR: %s, when parsing DFA, State not in States (State "
                  "%s)\n",
                  __FUNCTION__, token);
          return Err;
        }
        dfa->final.state_idxs[dfa->final.count++] = idx;
        token = strtok_r(NULL, "{", &ptr);
      }
    } else if (!strncmp(buff, "Transitions:", 12)) {
      while (fgets(buff, sizeof(buff), fp)) {
        char *ptr;
        char buff_cp[buff_length];
        strcpy(buff_cp, buff); // buff_cp always hold the original buffer
        char tok_cp[buff_length];
        char *token = strtok_r(buff, "{", &ptr); // from
        size_t idx;
        StateList curr_list = {0};
        strcpy(tok_cp, token);
        state_set_parse(&curr_list, tok_cp, name_list);
        if (find_state_list(&dfa->states, &curr_list, &idx) != Ok) {
          fprintf(
              stderr,
              "ERROR: %s, Transition from state is not in States (from state: "
              "%s)\n",
              __FUNCTION__, token);
          return Err;
        }
        size_t from_idx = idx;
        strcpy(buff, buff_cp);
        token = strtok_r(buff, "}",
                         &ptr); // transition, this resets from original buffer
        token = strtok_r(NULL, "}", &ptr); // so we take the next tok
        token = strtok_r(token, " ", &ptr);
        if (find_state(token, dfa->alphabets.names, dfa->alphabets.count,
                       &idx) != Ok) {
          fprintf(stderr,
                  "ERROR: %s, Transition alphabet not in Alphabets "
                  "(alphabet: %s)\n",
                  __FUNCTION__, token);
          return Err;
        }
        size_t alphabet_idx = idx;
        strcpy(buff, buff_cp);
        token = strtok_r(buff, "{", &ptr); // to
        token = strtok_r(NULL, "{", &ptr); // to
        state_set_parse(&curr_list, token, name_list);
        if (find_state_list(&dfa->states, &curr_list, &idx) != Ok) {
          fprintf(stderr,
                  "ERROR: %s, Transition to state is not in States (to state: "
                  "%s)\n",
                  __FUNCTION__, token);
          return Err;
        }
        dfa->transition[from_idx][alphabet_idx] = idx;
      }
    }
  }
  return Ok;
}

bool dfa_compare(DFA *a, DFA *b) {
  if (a->states.count != b->states.count) {
    fprintf(stderr, "%s, states is not equal\n", __FUNCTION__);
    return false;
  } else if (a->alphabets.count != b->alphabets.count) {
    fprintf(stderr, "%s, alphabets is not equal\n", __FUNCTION__);
    return false;
  } else if (a->start_idx != b->start_idx) {
    fprintf(stderr, "%s, start_idx is not equal\n", __FUNCTION__);
    return false;
  } else if (a->final.count != b->final.count) {
    fprintf(stderr, "%s, final states is not equal\n", __FUNCTION__);
    return false;
  }
  for (int i = 0; i < a->states.count; i++) {
    for (int j = 0; j < a->states.set[i].count; j++) {
      if (a->states.set[i].state_idxs[j] != b->states.set[i].state_idxs[j]) {
        fprintf(stderr, "%s, states is not equal\n", __FUNCTION__);
        return false;
      }
    }
  }
  for (int i = 0; i < a->alphabets.count; i++) {
    if (strcmp(a->alphabets.names[i], b->alphabets.names[i])) {
      fprintf(stderr, "%s, alphabets is not equal\n", __FUNCTION__);
      return false;
    }
  }
  for (int i = 0; i < a->final.count; i++) {
    if (a->final.state_idxs[i] != b->final.state_idxs[i]) {
      fprintf(stderr, "%s, alphabets is not equal\n", __FUNCTION__);
      return false;
    }
  }
  for (int i = 0; i < a->states.count; i++) {
    for (int j = 0; j < a->alphabets.count; j++) {
      if (a->transition[i][j] != b->transition[i][j]) {
        fprintf(stderr, "%s, transition from %u to %u is not equal\n",
                __FUNCTION__, i, j);
        return false;
      }
    }
  }

  return true;
}

Result nfa_to_dfa(NFA *nfa, DFA *dfa) {
  dfa->alphabets = nfa->alphabets;
  dfa->start_idx = nfa->start_idx;

  Queue unchecked_states = {0};
  // add start state to states
  dfa->states.set[dfa->states.count++] =
      StateList{.state_idxs = {nfa->start_idx}, .count = 1};

  for (int i = 0; i < nfa->alphabets.count; i++) {
    size_t idx;
    if (find_state_list(&dfa->states, &nfa->transition[0][i], &idx) != Ok) {
      dfa->transition[0][i] = dfa->states.count;
      queue_push(&unchecked_states, dfa->states.count);
      dfa->states.set[dfa->states.count++] = nfa->transition[0][i];
    } else {
      dfa->transition[0][i] = idx;
    }
  }

  while (!queue_is_empty(&unchecked_states)) {
    size_t curr_idx;
    if (queue_pop(&unchecked_states, &curr_idx) == Err) {
      printf("ERROR: popping queue %d\n", __LINE__);
      return Err;
    };

    // subset construction
    // for each alphabet
    for (int i = 0; i < nfa->alphabets.count; i++) {
      StateList new_s =
          nfa->transition[dfa->states.set[curr_idx].state_idxs[0]][i];
      // for each state in "from"
      for (int j = 1; j < dfa->states.set[curr_idx].count; j++) {
        StateList *to =
            &nfa->transition[dfa->states.set[curr_idx].state_idxs[j]][i];
        // for each state in "to"
        for (int k = 0; k < to->count; k++) {
          if (!state_list_contain_idx(&to->state_idxs[k], &new_s)) {
            new_s.state_idxs[new_s.count++] = to->state_idxs[k];
          };
        }
      }
      qsort(new_s.state_idxs, new_s.count, sizeof(size_t), compare);
      size_t found_idx;
      if (find_state_list(&dfa->states, &new_s, &found_idx) != Ok) {
        dfa->transition[curr_idx][i] = dfa->states.count;
        queue_push(&unchecked_states, dfa->states.count);
        dfa->states.set[dfa->states.count++] = new_s;
      } else {
        dfa->transition[curr_idx][i] = found_idx;
      }
    }
  }

  for (int i = 0; i < dfa->states.count; i++) {
    for (int j = 0; j < nfa->final.count; j++) {
      if (state_list_contain_idx(&nfa->final.state_idxs[j],
                                 &dfa->states.set[i])) {
        dfa->final.state_idxs[dfa->final.count++] = i;
        break;
      }
    }
  }
  return Ok;
};

void dfa_print(DFA *dfa, char names[][MAX_STATE_LENGTH]) {
  printf("States: ");
  for (int i = 0; i < dfa->states.count; i++) {
    state_list_print(names, &dfa->states.set[i]);
    if (i != dfa->states.count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Alphabet: ");
  for (int i = 0; i < dfa->alphabets.count; i++) {
    printf("%s", dfa->alphabets.names[i]);
    if (i != dfa->alphabets.count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Start: %s\n", names[dfa->start_idx]);

  printf("Final: ");
  for (int i = 0; i < dfa->final.count; i++) {
    state_list_print(names, &dfa->states.set[dfa->final.state_idxs[i]]);
    if (i != dfa->final.count - 1)
      printf(" ");
  }
  printf("\n\n");

  printf("Transitions:\n");
  for (int i = 0; i < dfa->states.count; i++) {
    for (int j = 0; j < dfa->alphabets.count; j++) {
      state_list_print(names, &dfa->states.set[i]);
      printf(" ");
      printf("%s ", dfa->alphabets.names[j]);
      state_list_print(names, &dfa->states.set[dfa->transition[i][j]]);
      printf("\n");
    }
  }
}

void nfa_print(NFA *nfa, char names[][MAX_STATE_LENGTH]) {
  printf("States: ");
  for (int i = 0; i < nfa->states.count; i++) {
    printf("%s", names[i]);
    if (i != nfa->states.count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Alphabet: ");
  for (int i = 0; i < nfa->alphabets.count; i++) {
    printf("%s", nfa->alphabets.names[i]);
    if (i != nfa->alphabets.count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Start: %s\n", names[nfa->start_idx]);

  printf("Final: ");
  for (int i = 0; i < nfa->final.count; i++) {
    printf("%s", names[nfa->final.state_idxs[i]]);
    if (i != nfa->final.count - 1)
      printf(" ");
  }
  printf("\n\n");

  printf("Transitions:\n");
  for (int i = 0; i < nfa->states.count; i++) {
    for (int j = 0; j < nfa->alphabets.count; j++) {
      printf("%s", names[i]);
      printf(" ");
      printf("%s ", nfa->alphabets.names[j]);
      state_list_print(names, &nfa->transition[i][j]);
      printf("\n");
    }
  }
}

void nfa_transition_table_draw(Vector2 pos, Vector2 size, NFA *nfa) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (nfa->states.count + 1);
  int cell_h = table_h / (nfa->states.count + 1);
  Vector2 padding = (Vector2){.x = 5, .y = 5};
  char buff[MAX_STATE_LENGTH * MAX_STATES] = {0};
  // row
  for (int i = 0; i < nfa->states.count + 1; i++) {
    Vector2 temp_pos = pos;
    temp_pos.y += i * cell_h;
    // col
    for (int j = 0; j < nfa->alphabets.count + 1; j++) {
      temp_pos.x += j * cell_w;
      Vector2 text_temp_pos = temp_pos;
      text_temp_pos.x += padding.x;
      text_temp_pos.y += padding.y;
      if (i == 0 && j == 0) {
        continue; // skip first cell
      }
      if (i == 0) { // draw alphabet names first in the first column
        DrawText(nfa->alphabets.names[j - 1], text_temp_pos.x, text_temp_pos.y,
                 14, BLACK);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x, temp_pos.y + table_h,
                 BLACK);
      } else if (j == 0) { // draw state names
        DrawText(nfa->states.names[i - 1], text_temp_pos.x, text_temp_pos.y, 14,
                 GREEN);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x + table_w, temp_pos.y,
                 BLACK);
      } else {
        buff[0] = 0;
        state_list_sprint(nfa->states.names, &nfa->transition[i - 1][j - 1],
                          buff);
        DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, BLUE);
      }
    }
  }
}
void dfa_transition_table_draw(Vector2 pos, Vector2 size, DFA *dfa,
                               StateNames *names) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (dfa->states.count + 1);
  int cell_h = table_h / (dfa->states.count + 1);
  Vector2 padding = (Vector2){.x = 5, .y = 5};
  char buff[MAX_STATE_LENGTH * MAX_STATES] = {0};
  // row
  for (int i = 0; i < dfa->states.count + 1; i++) {
    Vector2 temp_pos = pos;
    temp_pos.y += i * cell_h;
    // col
    for (int j = 0; j < dfa->alphabets.count + 1; j++) {
      temp_pos.x += j * cell_w;
      Vector2 text_temp_pos = temp_pos;
      text_temp_pos.x += padding.x;
      text_temp_pos.y += padding.y;
      if (i == 0 && j == 0) {
        continue; // skip first cell
      }
      if (i == 0) { // draw alphabet names first in the first column
        DrawText(dfa->alphabets.names[j - 1], text_temp_pos.x, text_temp_pos.y,
                 14, BLACK);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x, temp_pos.y + table_h,
                 BLACK);
      } else if (j == 0) { // draw state names
        buff[0] = 0;
        state_list_sprint(names->names,
                          &dfa->states.set[i - 1],
                          buff);
        DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, GREEN);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x + table_w, temp_pos.y,
                 BLACK);
      } else {
        buff[0] = 0;
        state_list_sprint(names->names,
                          &dfa->states.set[dfa->transition[i - 1][j - 1]],
                          buff);
        DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, BLUE);
      }
    }
  }
}
