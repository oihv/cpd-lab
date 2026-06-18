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

void transition_filled_copy(bool dst[][MAX_ALPHABETS],
                            bool src[][MAX_ALPHABETS]) {
  for (int i = 0; i < MAX_STATES; i++) {
    for (int j = 0; j < MAX_ALPHABETS; j++) {
      dst[i][j] = src[i][j];
    }
  }
}
