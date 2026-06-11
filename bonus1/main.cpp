#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_STATES 50
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

Result find_state_list(StateList states[MAX_STATES], size_t count, StateList s,
                       size_t *idx) {
  if (count == 0)
    return Err;
  for (int i = 0; i < count; i++) {
    if (states[i].count != s.count)
      continue;
    for (int j = 0; j < s.count; j++) {
      if (states[i].state_idxs[j] != s.state_idxs[j])
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
      return Ok;
    }
  }
  return Err;
}

void state_list_print(char states[][MAX_STATE_LENGTH], StateList *s) {
  printf("{");
  for (int j = 0; j < s->count; j++) {
    printf("%s", states[s->state_idxs[j]]);
    if (j != s->count - 1)
      printf(" ");
  }
  printf("}");
}

int main(int argc, char **argv) {
  if (argc < 0) {
    fprintf(stderr, "ERROR: please provide the text file for the NFA\n");
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "ERROR: file can't be opened (file name: %s)", argv[1]);
  }

  // NFA
  char states[MAX_STATES][MAX_STATE_LENGTH] = {0};
  size_t state_count = 0;
  char alphabets[MAX_ALPHABETS][MAX_ALPHABET_LENGTH] = {0};
  uint8_t alphabet_count = 0;
  size_t start_idx = 0;
  StateList nfa_final = {0};
  StateList transition[MAX_STATES][MAX_ALPHABETS] = {0};

  char buff[64];
  while (fgets(buff, 64, fp)) {
    if (!strncmp(buff, "States:", 7)) {
      char *token = strtok(buff + 7, " \t\n\r");
      while (token) {
        strcpy(states[state_count++], token);
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Alphabet:", 9)) {
      char *token = strtok(buff + 9, " \t\n\r");
      while (token) {
        strcpy(alphabets[alphabet_count++], token);
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Start:", 6)) {
      char *token = strtok(buff + 6, " \t\n\r");
      if (token) {
        if (find_state(token, states, state_count, &start_idx) != Ok) {
          fprintf(stderr,
                  "ERROR: Start state is not in States (Start state: %s)\n",
                  token);
          return 1;
        }
      }
    } else if (!strncmp(buff, "Final:", 6)) {
      char *token = strtok(buff + 6, " \t\n\r");
      while (token) {
        if (find_state(token, states, state_count,
                       &nfa_final.state_idxs[nfa_final.count++]) != Ok) {
          fprintf(stderr,
                  "ERROR: Final state is not in States (Final state: %s)\n",
                  token);
          return 1;
        }
        token = strtok(NULL, " \t\n\r");
      }
    } else if (!strncmp(buff, "Transitions:", 12)) {
      while (fgets(buff, sizeof(buff), fp)) {
        char *token = strtok(buff, " \t\n\r");
        size_t idx;
        if (find_state(token, states, state_count, &idx) != Ok) {
          fprintf(stderr,
                  "ERROR: Transition from state is not in States (from state: "
                  "%s)\n",
                  token);
          return 1;
        }
        size_t from_idx = idx;
        token = strtok(NULL, " \t\n\r");
        if (find_state(token, alphabets, alphabet_count, &idx) != Ok) {
          fprintf(
              stderr,
              "ERROR: Transition alphabetis not in Alphabets (alphabet: %s)\n",
              token);
          return 1;
        }
        size_t alphabet_idx = idx;
        token = strtok(NULL, " \t\n\r");
        StateList *s = &transition[from_idx][alphabet_idx];
        while (token) {
          if (!strncmp(token, "-", 1))
            break; // skip "-"
          if (find_state(token, states, state_count, &idx) != Ok) {
            fprintf(
                stderr,
                "ERROR: Transition to state is not in States (to state: %s)\n",
                token);
            return 1;
          }
          s->state_idxs[s->count++] = idx;
          token = strtok(NULL, " \t\n\r");
        }
        qsort(s->state_idxs, s->count, sizeof(size_t), compare);
      }
    }
  }

  // print nfa
  // for (int i = 0; i < state_count; i++) {
  //   for (int j = 0; j < alphabet_count; j++) {
  //     printf("%s ", states[i]);
  //     printf("%s: ", alphabets[j]);
  //     StateList s = transition[i][j];
  //     for (int k = 0; k < s.count; k++) {
  //       printf("%s ", states[s.state_idxs[k]]);
  //     }
  //     if (s.count == 0) printf("-");
  //     printf("\n");
  //   }
  // }

  Queue unchecked_states = {0};
  StateList dfa_states[MAX_STATES] = {0};
  size_t dfa_state_count = 0;
  StateList dfa_final = {0};
  size_t dfa_transition[MAX_STATES][MAX_ALPHABETS] = {0};

  // add start state to states
  dfa_states[dfa_state_count++] =
      StateList{.state_idxs = {start_idx}, .count = 1};

  for (int i = 0; i < alphabet_count; i++) {
    size_t idx;
    if (find_state_list(dfa_states, dfa_state_count, transition[0][i], &idx) !=
        Ok) {
      dfa_transition[0][i] = dfa_state_count;
      queue_push(&unchecked_states, dfa_state_count);
      for (int j = 0; j < nfa_final.count; j++) {
        if (state_list_contain_idx(&nfa_final.state_idxs[j],
                                   &transition[0][i])) {
          dfa_final.state_idxs[dfa_final.count++] = dfa_state_count;
        }
      }
      dfa_states[dfa_state_count++] = transition[0][i];
    } else {
      dfa_transition[0][i] = idx;
    }
  }

  while (!queue_is_empty(&unchecked_states)) {
    size_t curr_idx;
    if (queue_pop(&unchecked_states, &curr_idx) == Err) {
      printf("ERROR: popping queue %d\n", __LINE__);
      return 0;
    };

    // subset construction
    // for each alphabet
    for (int i = 0; i < alphabet_count; i++) {
      StateList new_s = transition[dfa_states[curr_idx].state_idxs[0]][i];
      // for each state in "from"
      for (int j = 1; j < dfa_states[curr_idx].count; j++) {
        // for each state in "to"
        StateList *to = &transition[dfa_states[curr_idx].state_idxs[j]][i];
        for (int k = 0; k < to->count; k++) {
          if (state_list_contain_idx(&to->state_idxs[k], &new_s) == Err) {
            new_s.state_idxs[new_s.count++] = to->state_idxs[k];
          };
        }
      }
      qsort(new_s.state_idxs, new_s.count, sizeof(size_t), compare);
      size_t found_idx;
      if (find_state_list(dfa_states, dfa_state_count, new_s, &found_idx) !=
          Ok) {
        dfa_transition[curr_idx][i] = dfa_state_count;
        queue_push(&unchecked_states, dfa_state_count);
        for (int j = 0; j < nfa_final.count; j++) {
          if (state_list_contain_idx(&nfa_final.state_idxs[j],
                                     &transition[0][i])) {
            dfa_final.state_idxs[dfa_final.count++] = dfa_state_count;
          }
        }
        dfa_states[dfa_state_count++] = new_s;
      } else {
        dfa_transition[curr_idx][i] = found_idx;
      }
    }
  }

  // print dfa
  printf("States: ");
  for (int i = 0; i < dfa_state_count; i++) {
    state_list_print(states, &dfa_states[i]);
    if (i != dfa_state_count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Alphabet: ");
  for (int i = 0; i < alphabet_count; i++) {
    printf("%s", alphabets[i]);
    if (i != alphabet_count - 1)
      printf(" ");
  }
  printf("\n");

  printf("Start: %s\n", states[start_idx]);

  printf("Final: ");
  for (int i = 0; i < dfa_final.count; i++) {
    state_list_print(states, &dfa_states[dfa_final.state_idxs[i]]);
    if (i != dfa_final.count - 1)
      printf(" ");
  }
  printf("\n\n");

  printf("Transitions:\n");
  for (int i = 0; i < dfa_state_count; i++) {
    for (int j = 0; j < alphabet_count; j++) {
      state_list_print(states, &dfa_states[i]);
      printf(" ");
      printf("%s ", alphabets[i]);
      state_list_print(states, &dfa_states[dfa_transition[i][j]]);
      printf("\n");
    }
  }

  fclose(fp);

  return 0;
}
