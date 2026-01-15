#include "frontend.h"

static WINDOW *win;

int main() {
  setlocale(LC_ALL, "");
  if (initscr() != NULL) {
    if (allocStart() == true) {
      tune_ncurses();
      game_loop();
      freeGame();
    }
    endwin();
  }
  return 0;
}

void game_loop() {
  struct timespec ts = {0, 10000000L};
  win = newwin(H_FIELD + BORDS_G, W_FIELD + W_INFO + BORDS_V, 0, 0);
  while (getCurrentState() != Exit) {
    getUserInput();
    GameInfo_t Data_now = updateCurrentState();
    GameState_t State_now = getCurrentState();
    if (State_now == Attaching) {
      showDataNow(&Data_now);
      nanosleep(&ts, NULL);
      Data_now = updateCurrentState();
      showDataNow(&Data_now);
      FullRows EraseRows = getFullRows();
      if (EraseRows.count > 0) {
        showDestroyRows(&EraseRows);
        shiftDownField();
        resetErase();
      }
      checkGameOver();
    } else if (State_now == StartMenu) {
      showStartMenu(&Data_now);
    } else {
      showDataNow(&Data_now);
    }
    nanosleep(&ts, NULL);
  }
  delwin(win);
}

void getUserInput() {
  int pressed_key = getch();
  if (pressed_key != ERR) {
    switch (pressed_key) {
      case 's':
      case 'S':
        userInput(Start, false);
        break;
      case 'p':
      case 'P':
        userInput(Pause, false);
        break;
      case 'q':
      case 'Q':
        userInput(Terminate, false);
        break;
      case KEY_LEFT:
        userInput(Left, false);
        break;
      case KEY_RIGHT:
        userInput(Right, false);
        break;
      case KEY_UP:
        userInput(Up, false);
        break;
      case KEY_DOWN:
        userInput(Down, false);
        break;
      case ' ':
        userInput(Action, false);
        break;
    }
  }
}

void drawBoards() {
  int start_y = 0;
  int start_x = 0;
  for (int i = start_y + 1; i < 1 + H_FIELD; i++) {
    mvwaddch(win, i, start_x, ACS_VLINE);
    mvwaddch(win, i, start_x + W_FIELD + 1, ACS_VLINE);
    mvwaddch(win, i, start_x + W_FIELD + W_INFO + 2, ACS_VLINE);
  }
  for (int j = start_x + 1; j < 2 + W_FIELD + W_INFO; j++) {
    mvwaddch(win, start_y, j, ACS_HLINE);
    mvwaddch(win, start_y + H_FIELD + 1, j, ACS_HLINE);
  }
  mvwaddch(win, start_y, start_x, ACS_ULCORNER);
  mvwaddch(win, start_y, start_x + W_FIELD + 1, ACS_TTEE);
  mvwaddch(win, start_y, start_x + W_FIELD + W_INFO + 2, ACS_URCORNER);
  mvwaddch(win, start_y + H_FIELD + 1, start_x, ACS_LLCORNER);
  mvwaddch(win, start_y + H_FIELD + 1, start_x + W_FIELD + 1, ACS_BTEE);
  mvwaddch(win, start_y + H_FIELD + 1, start_x + W_FIELD + W_INFO + 2,
           ACS_LRCORNER);
}

void drawField(const GameInfo_t *Data) {
  for (int i = 0; i < H_FIELD; i++) {
    for (int j = 0; j < W_FIELD; j++) {
      if (Data->field[i][j] == PIXEL_EMPTY) {
        mvwaddch(win, i + 1, j + 1, LOOK_PXL_EMPTY | A_BOLD);
      } else if (Data->field[i][j] == PIXEL_STATIC) {
        mvwaddch(win, i + 1, j + 1, LOOK_PXL_STATIC | A_BOLD);
      } else if (Data->field[i][j] == PIXEL_MOVING) {
        mvwaddch(win, i + 1, j + 1, LOOK_PXL_MOVING | A_BOLD);
      }
    }
  }
}

void drawInfo(const GameInfo_t *Data) {
  int start_y = 1;
  int start_x = W_FIELD + 2;
  mvwaddstr(win, start_y++, start_x, "   HIGH:  ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "  SCORE:  ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "  Level:  ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "   Next:  ");
  mvwaddstr(win, start_y++, start_x, "  ┌────┐  ");
  mvwaddstr(win, start_y++, start_x, "  │    │  ");
  mvwaddstr(win, start_y++, start_x, "  │    │  ");
  mvwaddstr(win, start_y++, start_x, "  │    │  ");
  mvwaddstr(win, start_y++, start_x, "  │    │  ");
  mvwaddstr(win, start_y++, start_x, "  └────┘  ");
  for (int i = 0; i < H_FIGMAX; i++) {
    for (int j = 0; j < W_FIGMAX; j++) {
      if (Data->next[i][j] != PIXEL_EMPTY) {
        mvwaddch(win, 12 + i, W_FIELD + 5 + j, LOOK_PXL_MOVING | A_BOLD);
      } else {
        mvwaddch(win, 12 + i, W_FIELD + 5 + j, LOOK_PXL_EMPTY | A_BOLD);
      }
    }
  }
  GameState_t State_now = getCurrentState();
  if (State_now == OnPause) {
    mvwaddstr(win, start_y++, start_x, "  PAUSE   ");
    mvwaddstr(win, start_y++, start_x, "p Continue");
    mvwaddstr(win, start_y++, start_x, "q  Qiut   ");
    mvwaddstr(win, start_y++, start_x, "s  Menu   ");
  } else if (State_now == GameOver) {
    mvwaddstr(win, start_y++, start_x, " GAME OVER");
    mvwaddstr(win, start_y++, start_x, "q  Qiut   ");
    mvwaddstr(win, start_y++, start_x, "s  Menu   ");
    mvwaddstr(win, start_y++, start_x, "          ");
  } else {
    mvwaddstr(win, start_y++, start_x, "          ");
    mvwaddstr(win, start_y++, start_x, "          ");
    mvwaddstr(win, start_y++, start_x, "          ");
    mvwaddstr(win, start_y++, start_x, "          ");
  }
  mvwprintw(win, 2, start_x, " %08d", Data->high_score);
  mvwprintw(win, 5, start_x, " %08d", Data->score);
  mvwprintw(win, 8, start_x, "    %02d", Data->level);
}

void drawStartMenu() {
  mvwaddstr(win, H_FIELD / 2, W_FIELD / 2 - 2, "TETRIS");
  int start_y = 1;
  int start_x = W_FIELD + 2;
  mvwaddstr(win, start_y++, start_x, " CONTROL  ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, " Common:  ");
  mvwaddstr(win, start_y++, start_x, "s - Start ");
  mvwaddstr(win, start_y++, start_x, "p - Pause ");
  mvwaddstr(win, start_y++, start_x, "q - Qiut  ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "  Move:   ");
  mvwaddstr(win, start_y++, start_x, "SP - Turn ");
  mvwaddstr(win, start_y++, start_x, "← - Left  ");
  mvwaddstr(win, start_y++, start_x, "→ - Right ");
  mvwaddstr(win, start_y++, start_x, "↓ - Drop  ");
  mvwaddstr(win, start_y++, start_x, "↑ - Unused");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
  mvwaddstr(win, start_y++, start_x, "          ");
}

void showDataNow(const GameInfo_t *Data) {
  drawInfo(Data);
  drawField(Data);
  wrefresh(win);
}

void showDestroyRows(const FullRows *EraseRows) {
  struct timespec ts = {0, 40000000L};
  for (int blink = 3; blink > 0; blink--) {
    for (int i = 0; i < EraseRows->count; i++) {
      for (int j = 0; j < W_FIELD; j++) {
        mvwaddch(win, EraseRows->rows[i] + 1, j + 1, LOOK_PXL_EMPTY | A_BOLD);
      }
    }
    wrefresh(win);
    nanosleep(&ts, NULL);
    for (int i = 0; i < EraseRows->count; i++) {
      for (int j = 0; j < W_FIELD; j++) {
        mvwaddch(win, EraseRows->rows[i] + 1, j + 1, LOOK_PXL_STATIC | A_BOLD);
      }
    }
    wrefresh(win);
    nanosleep(&ts, NULL);
  }
}

void showStartMenu(const GameInfo_t *Data) {
  drawField(Data);
  drawStartMenu();
  drawBoards();
  wrefresh(win);
}

void tune_ncurses() {
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  curs_set(0);
}