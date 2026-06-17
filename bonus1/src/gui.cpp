#include "base.h"
#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define RAYGUI_IMPLEMENTATION
// #include "raygui.h"

#include "draw.cpp"
#include "lib.cpp"

#define WIN_W 1500
#define WIN_H 600

GuiData data;

void UpdateDrawFrame() {
  BeginDrawing();
  ClearBackground(DARKGRAY);

  if (IsKeyPressed(KEY_SPACE)) {
    data.autoplay = !data.autoplay;
  }
  if (IsKeyPressed(KEY_LEFT) && data.curr_snap_idx > 0) {
    data.curr_snap_idx -= 1;
    data.progress = 0.;
  }
  if (IsKeyPressed(KEY_RIGHT) && data.curr_snap_idx < data.hist.count - 1) {
    data.curr_snap_idx += 1;
    data.progress = 0.;
  }
  snapshot_draw((Vector2){.x = 0, .y = 0}, (Vector2){.x = WIN_W, .y = WIN_H},
                &data.hist.steps[data.curr_snap_idx], &data.nfa, data.progress);
  if (data.progress < 1.) {
    data.progress += GetFrameTime() / data.animation_duration;
  } else if (data.progress >= 1.) {
    data.progress = 1.;
    if (data.autoplay) {
      data.progress = 0.;
      if (data.curr_snap_idx < data.hist.count - 1)
        data.curr_snap_idx += 1;
      else if (data.curr_snap_idx == data.hist.count - 1)
        data.autoplay = false;
    }
  }

  sprintf(data.progress_str, "%zu/%d", data.curr_snap_idx + 1, data.hist.count);
  DrawText(data.progress_str, WIN_W - 50, 20, 20, WHITE);

  if (!data.autoplay) {
    Color color = WHITE;
    color.a = 100;
    int text_w = MeasureText("PAUSED", 40);
    DrawText("PAUSED", WIN_W / 2 - text_w / 2, 20, 40, color);
  }

  EndDrawing();
}

int main() {
  InitWindow(WIN_W, WIN_H, "Hello raylib");
  ConfigFlags flag = FLAG_WINDOW_MAXIMIZED;
  SetTargetFPS(60);

  bool showMessageBox = false;
  FILE *fp = fopen("test/nfa1.txt", "r");

  data.nfa = {0};
  data.dfa = {0};
  data.hist = {};
  data.autoplay = true;

  data.curr_snap_idx = 0;
  data.progress = 0.;
  data.animation_duration = 0.75;

  nfa_parse(&data.nfa, fp);
  StateSetList state = nfa_default_state_set_list(&data.nfa);
  nfa_to_dfa(&data.nfa, &data.dfa, &data.hist);

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
