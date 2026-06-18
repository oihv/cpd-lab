#include "base.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "draw.cpp"
#include "lib.cpp"
#include "test_input.h"
#include "utils.cpp"

// preconfigured default size
#define WIN_W 960
#define WIN_H 540
#define MAX_LINES 24

typedef enum { SCREEN_INPUT, SCREEN_MAIN } Screen;

NFA nfa;
NFA nfa_temp = {0};
DFA dfa;
History hist;
bool autoplay;
size_t curr_snap_idx;
char progress_str[10];
float progress;
float animation_duration;
int win_h;
int win_w;
bool fullscreen;
Screen curr_screen = SCREEN_INPUT;
char input_buff[MAX_LINES][64] = {0};
char input_labels[5][32] = {
    "States:", "Alphabet:", "Start:", "Final:", "Transitions:"};
size_t transition_count = 1;
size_t transition_max = MAX_LINES - 4;
float text_padding = 7;
bool text_box_focus[MAX_LINES] = {0};
bool dropdown_focus = false;
int dropdown_select = 0;
char input_buff_one_line[MAX_LINES * 24] = {0};
int test_input_idx = 0;
bool test_input_dropdown_focus = false;

void test_input_to_input_buff(size_t idx) {
  strcpy(input_buff_one_line, test_input[idx]);
  size_t i = 0;
  char *tok = strtok(input_buff_one_line, "\n");
  while (tok) {
    strcpy(input_buff[i++], tok);
    tok = strtok(NULL, "\n");
  }
  transition_count = i - 4;
}

void dump_input_buff() {
  input_buff_one_line[0] = '\0';
  for (int i = 0; i < 4 + transition_count; i++) {
    if (i < 5)
      sprintf(input_buff_one_line + strlen(input_buff_one_line), "%s ",
              input_labels[i]);
    if (i == 4) // add extra newline
      sprintf(input_buff_one_line + strlen(input_buff_one_line), "\n");
    sprintf(input_buff_one_line + strlen(input_buff_one_line), "%s\n",
            input_buff[i]);
  }
}

void init_animation() {
  nfa = {0};
  nfa_temp = {0};
  dfa = {0};
  hist = {};
  autoplay = true;

  curr_snap_idx = 0;
  progress = 0.;
  animation_duration = 0.75;

  dump_input_buff();

  switch (dropdown_select) {
  case 0: // NFA to DFA
    nfa_parse_str(&nfa, input_buff_one_line);
    nfa_to_dfa(&nfa, &dfa, &hist);
    break;
  case 1: // eps NFA to NFA
    eps_nfa_parse_str(&nfa, input_buff_one_line);
    eps_nfa_to_nfa(&nfa, &nfa_temp, &hist);
    break;
  case 2: // eps NFA to NFA (combine 0 and 1)
    eps_nfa_parse_str(&nfa, input_buff_one_line);
    eps_nfa_to_nfa(&nfa, &nfa_temp, &hist);
    nfa_to_dfa(&nfa_temp, &dfa, &hist);
    printf("%d\n", nfa_temp.alphabets.count);
    break;
  }
}

void screen_input() {
  float pos_y = 50;
  int row_num = 4 + transition_count;
  for (int i = 0; i < row_num; i++) {
    if (i < 5)
      GuiLabel((Rectangle){50, pos_y + (TEXT_SIZE + text_padding) * i, 50,
                           TEXT_SIZE},
               input_labels[i]);
    if (GuiTextBox((Rectangle){120, pos_y + (TEXT_SIZE + text_padding) * i,
                               (float)win_w - 170, (float)TEXT_SIZE},
                   input_buff[i], sizeof(input_buff[i]), text_box_focus[i]))
      text_box_focus[i] = !text_box_focus[i];
    if (i == 4 + transition_count - 1) { // last row
      if (GuiButton((Rectangle){50,
                                pos_y + (TEXT_SIZE + text_padding) * (i + 1),
                                100, TEXT_SIZE},
                    "+ add transition") &&
          transition_count < transition_max) {
        transition_count++;
      }
      if (GuiButton((Rectangle){170,
                                pos_y + (TEXT_SIZE + text_padding) * (i + 1),
                                130, TEXT_SIZE},
                    "- remove transition") &&
          transition_count > 1) {
        transition_count--;
      }
    }
  }
  if (GuiButton((Rectangle){50 + static_cast<float>((win_w - 170) / 4.),
                            pos_y + (row_num + 1) * (TEXT_SIZE + text_padding) + TEXT_SIZE + text_padding,
                            150, TEXT_SIZE},
                "Load test input")) {
    test_input_to_input_buff(test_input_idx);
  };
  if (GuiButton((Rectangle){50 + static_cast<float>((win_w - 170) / 2.),
                            pos_y + (row_num + 1) * (TEXT_SIZE + text_padding),
                            150, TEXT_SIZE},
                "Start")) {
    init_animation();
    curr_screen = SCREEN_MAIN;
  };
  if (GuiDropdownBox(
          (Rectangle){50, pos_y + (row_num + 1) * (TEXT_SIZE + text_padding) + TEXT_SIZE + text_padding,
                      static_cast<float>((win_w - 170) / 4.), TEXT_SIZE},
          test_input_desc, &test_input_idx,
          test_input_dropdown_focus)) {
    test_input_dropdown_focus = !test_input_dropdown_focus;
  }
  if (GuiDropdownBox(
          (Rectangle){50, pos_y + (row_num + 1) * (TEXT_SIZE + text_padding),
                      static_cast<float>((win_w - 170) / 2.), TEXT_SIZE},
          "NFA to DFA; eps-NFA to NFA; eps-NFA to DFA", &dropdown_select,
          dropdown_focus))
    dropdown_focus = !dropdown_focus;
}

void UpdateDrawFrame() {
  BeginDrawing();
  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

  if (IsKeyPressed(KEY_F) && fullscreen) {
    fullscreen = false;
    win_w = WIN_W;
    win_h = WIN_H;
    ToggleFullscreen();
  } else if (IsKeyPressed(KEY_F) && !fullscreen) {
    fullscreen = true;
    int monitor = GetCurrentMonitor();
    win_w = GetMonitorWidth(monitor);
    win_h = GetMonitorHeight(monitor);
    ToggleFullscreen();
  }

  switch (curr_screen) {
  case SCREEN_INPUT: {
    screen_input();
  } break;

  case SCREEN_MAIN:
    if (IsKeyPressed(KEY_SPACE)) {
      autoplay = !autoplay;
    }
    if (IsKeyPressed(KEY_LEFT) && curr_snap_idx > 0) {
      curr_snap_idx -= 1;
      progress = 0.;
    }
    if (IsKeyPressed(KEY_RIGHT) && curr_snap_idx < hist.count - 1) {
      curr_snap_idx += 1;
      progress = 0.;
    }

    snapshot_draw((Vector2){.x = 0, .y = 0},
                  (Vector2){.x = (float)win_w, .y = (float)win_h},
                  &hist.steps[curr_snap_idx], progress);

    if (progress < 1.) {
      progress += GetFrameTime() / animation_duration;
    } else if (progress > 1.) {
      progress = 1.;
      if (autoplay) {
        progress = 0.;
        if (curr_snap_idx < hist.count - 1)
          curr_snap_idx += 1;
        else if (curr_snap_idx == hist.count - 1)
          autoplay = false;
      }
    }

    sprintf(progress_str, "%zu/%d", curr_snap_idx + 1, hist.count);
    DrawText(progress_str, win_w - 80, 20, 20, WHITE);

    if (!autoplay) {
      Color color = WHITE;
      color.a = 100;
      int text_w = MeasureText("PAUSED", 40);
      DrawText("PAUSED", win_w / 2 - text_w / 2, 20, 40, color);
    }
    if (GuiButton((Rectangle){(float)win_w - 170, (float)win_h - 50, 150, 50},
                  "Back to menu")) {
      curr_screen = SCREEN_INPUT;
    }
    break;
  }

  EndDrawing();
}

int main() {
  InitWindow(WIN_W, WIN_H, "Finite Automaton Conversion Algorithms");
  ConfigFlags flag = FLAG_WINDOW_MAXIMIZED;
  SetTargetFPS(60);
  fullscreen = false;
  win_w = WIN_W;
  win_h = WIN_H;
  GuiLoadStyle("./assets/style_dark.rgs");

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }
#endif
  CloseWindow();
  return 0;
}
