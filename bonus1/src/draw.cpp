// row and col are 0 indexing
Rectangle nfa_transition_table_get_rect(Vector2 pos, Vector2 size, NFA *nfa,
                                        size_t row, size_t col) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (nfa->alphabets.count + 1);
  int cell_h = table_h / (nfa->states.count + 1);

  Rectangle res = {0};
  res.x = pos.x + (cell_w * col);
  res.y = pos.y + (cell_h * row);
  res.width = cell_w;
  res.height = cell_h;

  return res;
}

Rectangle dfa_transition_table_get_rect(Vector2 pos, Vector2 size, DFA *dfa,
                                        size_t row, size_t col) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (dfa->alphabets.count + 1);
  int cell_h = table_h / (dfa->states.count + 1);

  Rectangle res = {0};
  res.x = pos.x + (cell_w * col);
  res.y = pos.y + (cell_h * row);
  res.width = cell_w;
  res.height = cell_h;

  return res;
}

void nfa_transition_table_draw(Vector2 pos, Vector2 size, NFA *nfa,
                               bool transition_flag[][MAX_ALPHABETS]) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (nfa->alphabets.count + 1);
  int cell_h = table_h / (nfa->states.count + 1);
  Vector2 padding = (Vector2){.x = 5, .y = 5};
  char buff[MAX_STATE_LENGTH * MAX_STATES] = {0};
  // row
  for (int i = 0; i < nfa->states.count + 1; i++) {
    Vector2 temp_pos = pos;
    temp_pos.y += i * cell_h;
    // col
    for (int j = 0; j < nfa->alphabets.count + 1; j++) {
      temp_pos.x = pos.x + (j * cell_w);
      Vector2 text_temp_pos = temp_pos;
      text_temp_pos.x += padding.x;
      text_temp_pos.y += padding.y;
      if (i == 0 && j == 0) {
        continue; // skip first cell
      }
      if (i == 0) { // draw alphabet names first in the first column
        DrawText(nfa->alphabets.names[j - 1], text_temp_pos.x, text_temp_pos.y,
                 14, WHITE);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x, temp_pos.y + table_h,
                 WHITE);
      } else if (j == 0) { // draw state names
        DrawText(nfa->states.names[i - 1], text_temp_pos.x, text_temp_pos.y, 14,
                 GREEN);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x + table_w, temp_pos.y,
                 WHITE);
      } else {
        buff[0] = 0;
        if (transition_flag && transition_flag[i - 1][j - 1] == false) {
          sprintf(buff, "-");
        } else {
          state_list_sprint(nfa->states.names, &nfa->transition[i - 1][j - 1],
                            buff);
          DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, SKYBLUE);
        }
      }
    }
  }
}
void dfa_transition_table_draw(Vector2 pos, Vector2 size, DFA *dfa,
                               StateNames *names,
                               bool transition_flag[][MAX_ALPHABETS]) {
  Vector2 margin = (Vector2){.x = 5, .y = 5};
  pos.x += margin.x;
  pos.y += margin.y;
  float table_w = size.x - 2 * margin.x;
  float table_h = size.y - 2 * margin.y;
  int cell_w = table_w / (dfa->alphabets.count + 1);
  int cell_h = table_h / (dfa->states.count + 1);
  Vector2 padding = (Vector2){.x = 5, .y = 5};
  char buff[MAX_STATE_LENGTH * MAX_STATES] = {0};
  // row
  for (int i = 0; i < dfa->states.count + 1; i++) {
    Vector2 temp_pos = pos;
    temp_pos.y += i * cell_h;
    // col
    for (int j = 0; j < dfa->alphabets.count + 1; j++) {
      temp_pos.x = pos.x + (j * cell_w);
      Vector2 text_temp_pos = temp_pos;
      text_temp_pos.x += padding.x;
      text_temp_pos.y += padding.y;
      if (i == 0 && j == 0) {
        continue; // skip first cell
      }
      if (i == 0) { // draw alphabet names first in the first column
        DrawText(dfa->alphabets.names[j - 1], text_temp_pos.x, text_temp_pos.y,
                 14, WHITE);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x, temp_pos.y + table_h,
                 WHITE);
      } else if (j == 0) { // draw state names
        buff[0] = 0;
        state_list_sprint(names->names, &dfa->states.set[i - 1], buff);
        DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, GREEN);
        DrawLine(temp_pos.x, temp_pos.y, temp_pos.x + table_w, temp_pos.y,
                 WHITE);
      } else {
        buff[0] = 0;
        if (transition_flag && transition_flag[i - 1][j - 1] == false) {
          sprintf(buff, "-");
        } else {
          state_list_sprint(names->names,
                            &dfa->states.set[dfa->transition[i - 1][j - 1]],
                            buff);
        }
        DrawText(buff, text_temp_pos.x, text_temp_pos.y, 14, SKYBLUE);
      }
    }
  }
}

void snapshot_draw(Vector2 pos, Vector2 size, Snapshot *snap,
                   float progress) {
  NFA* nfa = snap->nfa_src;
  int text_h = 20; // desc will be written at the bottom of the screen
  char buff[256];

  Vector2 margin = {.x = 10, .y = 10};
  pos.x += margin.x;
  pos.y += margin.y;
  size.x -= 2 * margin.x;
  size.y -= 2 * margin.y;

  Vector2 left_fa_size = size;
  left_fa_size.x = size.x / 3 - 2*margin.x;
  left_fa_size.y -= text_h * 3 + 2*margin.y;
  char left_title[10], right_title[10];
  switch (snap->algo_type) {
  case NFA_TO_DFA:
    sprintf(left_title, "NFA");
    sprintf(right_title, "DFA");
    break;
  case EPS_NFA_TO_NFA:
    sprintf(left_title, "eps-NFA");
    sprintf(right_title, "NFA");
  }
  DrawText(left_title, pos.x, pos.y, 30, WHITE);
  pos.y += 30; // offset for text
  nfa_transition_table_draw(pos, left_fa_size, nfa, NULL);

  Vector2 dfa_pos = pos;
  dfa_pos.x += left_fa_size.x;
  Vector2 right_fa_size = size;
  right_fa_size.x = size.x * (2. / 3);
  right_fa_size.y = left_fa_size.y;
  DrawText(right_title, dfa_pos.x, dfa_pos.y - 30, 30, WHITE);

  switch (snap->type) {
  case STEP_EPS_CLOSURE: {
    Rectangle rect = nfa_transition_table_get_rect(
        dfa_pos, right_fa_size, &snap->nfa, snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});

    rect = nfa_transition_table_get_rect(
        pos, left_fa_size, nfa, 0, nfa->alphabets.count);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .b = 125, .a = 50});
    rect = nfa_transition_table_get_rect(
        pos, left_fa_size, nfa, snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .b = 125, .a = 50});
  } break;
  case STEP_QUEUE_POP: {
    // highlight selected state
    Rectangle rect = dfa_transition_table_get_rect(
        dfa_pos, right_fa_size, &snap->dfa, snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
    break;
  }
  case STEP_NEW_STATE: {
    Rectangle rect = dfa_transition_table_get_rect(
        dfa_pos, right_fa_size, &snap->dfa, snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
    break;
  }
  case STEP_ADD_TRANSITION: {
    Rectangle rect;
    switch (snap->algo_type) {

    case NFA_TO_DFA:
      rect = dfa_transition_table_get_rect(dfa_pos, right_fa_size, &snap->dfa,
                                           snap->active_state + 1,
                                           snap->active_alphabet + 1);
      break;
    case EPS_NFA_TO_NFA:
      rect = nfa_transition_table_get_rect(dfa_pos, right_fa_size, &snap->nfa,
                                           snap->active_state + 1,
                                           snap->active_alphabet + 1);
      break;
    }
    Vector2 rect_pos;
    rect_pos.x = rect.x;
    rect_pos.y = rect.y;
    rect_pos = Vector2Lerp((Vector2){.x = 0, .y = pos.y + left_fa_size.y},
                           rect_pos, progress);
    DrawRectangle(rect_pos.x, rect_pos.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
  } break;
  case STEP_SUBSET_CONSTRUCTION: {
    Rectangle rect;
    switch (snap->algo_type) {
    case NFA_TO_DFA:
      rect = dfa_transition_table_get_rect(dfa_pos, right_fa_size, &snap->dfa,
                                           snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
      break;
    case EPS_NFA_TO_NFA:
      rect = nfa_transition_table_get_rect(dfa_pos, right_fa_size, &snap->nfa,
                                           snap->active_state + 1, 0);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
    rect = nfa_transition_table_get_rect(dfa_pos, right_fa_size, &snap->nfa, 0,
                                         snap->active_alphabet + 1);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                  (Color){.r = 0, .g = 255, .a = 50});
      break;
    }
  } break;
  }

  switch (snap->algo_type) {
  case NFA_TO_DFA:

    dfa_transition_table_draw(dfa_pos, right_fa_size, &snap->dfa, &nfa->states,
                              snap->dfa_transition_filled);
    break;
  case EPS_NFA_TO_NFA:
    nfa_transition_table_draw(dfa_pos, right_fa_size, &snap->nfa,
                              snap->dfa_transition_filled);
    sprintf(buff, "eps_closure: ");
    state_list_sprint(nfa->states.names, &snap->eps_closure, buff);
    DrawText(buff, pos.x + size.x / 2, pos.y + left_fa_size.y + text_h, 14,
             WHITE);
    break;
  }
  dfa_transition_table_draw(dfa_pos, right_fa_size, &snap->dfa, &nfa->states,
                            snap->dfa_transition_filled);

  DrawText(snap->description, pos.x, pos.y + left_fa_size.y, 14, WHITE);

  sprintf(buff, "unchecked_states: ");
  StateList s = {0};
  queue_to_state_list(&snap->queue, &s);
  for (int i = 0; i < s.count; i++) {
    state_list_sprint(nfa->states.names, &snap->dfa.states.set[s.state_idxs[i]],
                      buff + strlen(buff));
  }
  DrawText(buff, pos.x, pos.y + left_fa_size.y + text_h, 14, WHITE);
  sprintf(buff, "new_state: ");
  state_list_sprint(nfa->states.names, &snap->new_state, buff + strlen(buff));
  DrawText(buff, pos.x, pos.y + left_fa_size.y + 2 * text_h, 14, WHITE);
}
