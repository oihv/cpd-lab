#include "base.h"

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

void queue_to_state_list(Queue *q, StateList *s) {
  size_t it = q->pop_idx;
  for (int i = 0; i < q->count; i++) {
    s->state_idxs[s->count++] = q->item[it];
    it = (it + 1) % MAX_QUEUE_SIZE;
  }
}

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
  StateList temp_transition[MAX_STATES][MAX_ALPHABETS] = {0};
  for (int i = 0; i < nfa->states.count; i++) {
    StateList eps_closure = {0};
    Queue unchecked_states = {0};
    eps_closure.state_idxs[eps_closure.count++] = i;
    queue_push(&unchecked_states, i);
    // compute eps closure
    while (!queue_is_empty(&unchecked_states)) {
      size_t curr;
      queue_pop(&unchecked_states, &curr);
      for (int j = 0; j < nfa->transition[curr][eps_i].count; j++) {
        size_t to = nfa->transition[curr][eps_i].state_idxs[j];
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
      temp_transition[i][j] = new_s;
    }
  }
  for (int i = 0; i < nfa->states.count; i++) {
    for (int j = 0; j < nfa->alphabets.count; j++) {
      nfa->transition[i][j] = temp_transition[i][j];
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

void transition_filled_copy(bool dst[][MAX_ALPHABETS],
                            bool src[][MAX_ALPHABETS]) {
  for (int i = 0; i < MAX_STATES; i++) {
    for (int j = 0; j < MAX_ALPHABETS; j++) {
      dst[i][j] = src[i][j];
    }
  }
}

// *hist* is used for animation, NULL if not used
Result nfa_to_dfa(NFA *nfa, DFA *dfa, History *hist) {
  bool dfa_transition_filled[MAX_STATES][MAX_ALPHABETS] = {0};
  dfa->alphabets = nfa->alphabets;
  dfa->start_idx = nfa->start_idx;
  char buff[MAX_STATE_LENGTH * MAX_STATES] = {0};

  Queue unchecked_states = {0};
  // add start state to states
  dfa->states.set[dfa->states.count++] =
      StateList{.state_idxs = {nfa->start_idx}, .count = 1};
  if (hist) {
    hist->steps[hist->count++] = {.type = STEP_NEW_STATE,
                                  .dfa = *dfa,
                                  .active_state = nfa->start_idx,
                                  .description =
                                      "Add start state to list of states"};
  }

  for (int i = 0; i < nfa->alphabets.count; i++) {
    size_t idx;
    if (find_state_list(&dfa->states, &nfa->transition[0][i], &idx) != Ok) {
      dfa->transition[0][i] = dfa->states.count;
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_ADD_TRANSITION,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = nfa->start_idx,
            .active_alphabet = (size_t)i,
        };
        dfa_transition_filled[0][i] = true;
        transition_filled_copy(handle->dfa_transition_filled,
                               dfa_transition_filled);
        buff[0] = '\0';
        state_list_sprint(nfa->states.names, &nfa->transition[0][i], buff);
        sprintf(handle->description, "Add transition from %s on \"%s\" to %s",
                nfa->states.names[0], nfa->alphabets.names[i], buff);
      }
      queue_push(&unchecked_states, dfa->states.count);
      dfa->states.set[dfa->states.count++] = nfa->transition[0][i];
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_NEW_STATE,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = dfa->states.count - 1,
        };
        transition_filled_copy(handle->dfa_transition_filled,
                               dfa_transition_filled);
        buff[0] = '\0';
        state_list_sprint(nfa->states.names,
                          &dfa->states.set[handle->active_state], buff);
        sprintf(handle->description,
                "State not found in current states: Add new state %s, and add "
                "it to unchecked_states",
                buff);
      }
    } else {
      dfa->transition[0][i] = idx;
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_ADD_TRANSITION,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = idx,
            .active_alphabet = (size_t)i,
        };
        dfa_transition_filled[0][i] = true;
        transition_filled_copy(handle->dfa_transition_filled,
                               dfa_transition_filled);
        buff[0] = '\0';
        state_list_sprint(nfa->states.names, &dfa->states.set[idx], buff);
        sprintf(handle->description, "Add transition from {%s} on \"%s\" to %s",
                nfa->states.names[0], nfa->alphabets.names[i], buff);
      }
    }
  }

  while (!queue_is_empty(&unchecked_states)) {
    size_t curr_idx;
    if (queue_pop(&unchecked_states, &curr_idx) == Err) {
      printf("ERROR: popping queue %d\n", __LINE__);
      return Err;
    };
    if (hist) {
      Snapshot *handle = &hist->steps[hist->count++];
      *handle = {
          .type = STEP_QUEUE_POP,
          .dfa = *dfa,
          .queue = unchecked_states,
          .active_state = curr_idx,
      };
      transition_filled_copy(handle->dfa_transition_filled,
                             dfa_transition_filled);
      sprintf(handle->description, "Pop unchecked_states queue");
    }

    // subset construction
    // for each alphabet
    for (int i = 0; i < nfa->alphabets.count; i++) {
      StateList new_s =
          nfa->transition[dfa->states.set[curr_idx].state_idxs[0]][i];
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_SUBSET_CONSTRUCTION,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = curr_idx,
            .active_alphabet = (size_t)i,
        };
        transition_filled_copy(handle->dfa_transition_filled,
                               dfa_transition_filled);
        buff[0] = '\0';
        state_list_sprint(nfa->states.names, &dfa->states.set[curr_idx], buff);
        sprintf(handle->description,
                "Subset construction started: populate new_state with states "
                "that can be reached with \"%s\" from %s",
                dfa->alphabets.names[i], buff);
      }
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
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_SUBSET_CONSTRUCTION,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = dfa->states.set[curr_idx].state_idxs[0],
            .active_alphabet = (size_t)i,
            .highlight_set = new_s,
        };
        transition_filled_copy(handle->dfa_transition_filled, dfa_transition_filled);
        buff[0] = '\0';
        state_list_sprint(nfa->states.names, &dfa->states.set[curr_idx], buff);
        sprintf(handle->description,
                "Subset construction completed: check whether new_state "
                "already exists in current_state");
      }
      size_t found_idx;
      if (find_state_list(&dfa->states, &new_s, &found_idx) != Ok) {
        queue_push(&unchecked_states, dfa->states.count);
        dfa->states.set[dfa->states.count++] = new_s;
        if (hist) {
          Snapshot *handle = &hist->steps[hist->count++];
          *handle = {
              .type = STEP_NEW_STATE,
              .dfa = *dfa,
              .queue = unchecked_states,
              .active_state = dfa->states.count - 1,
              .active_alphabet = (size_t)i,
              .highlight_set = new_s,
          };
          transition_filled_copy(handle->dfa_transition_filled, dfa_transition_filled);
          sprintf(handle->description,
                  "new_state was not in states: new state was added");
        }
        dfa->transition[curr_idx][i] = dfa->states.count - 1;
      } else {
        dfa->transition[curr_idx][i] = found_idx;
      }
      if (hist) {
        Snapshot *handle = &hist->steps[hist->count++];
        *handle = {
            .type = STEP_ADD_TRANSITION,
            .dfa = *dfa,
            .queue = unchecked_states,
            .active_state = curr_idx,
            .active_alphabet = (size_t)i,
            .highlight_set = new_s,
        };
        dfa_transition_filled[curr_idx][i] = true;
        transition_filled_copy(handle->dfa_transition_filled, dfa_transition_filled);
        sprintf(handle->description, "Added new transition");
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
