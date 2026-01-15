#include "fsm.h"

static GameState_t curStatus = StartMenu;
static GameInfo_t CurData = {0};
static int figures[TURNTYPE][FIGURES][H_FIGMAX][W_FIGMAX] = {
    {{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}}},
    {{{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 1, 1, 0}},
     {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}},
     {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}}},
    {{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}}},
    {{{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}},
     {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}}},
};
static int curDownFigure[H_FIGMAX][W_FIGMAX] = {0};
static FigureState curDown = {0};
static FigureState next = {0};
static CoordLowLeftPointFigure Point = {0};
static FullRows Erase = {0};
static bool fl_pause_for_drop_down = false;

bool allocMatrix(int ***matrix, int rows, int columns) {
  if (matrix == NULL || rows <= 0 || columns <= 0 || rows > 1000 ||
      columns > 1000) {
    return false;
  }
  bool fl_noerror_alloc = true;
  *matrix = NULL;
  *matrix = (int **)(calloc(rows, sizeof(int *)));
  if (*matrix != NULL) {
    for (int i = 0; i < rows && fl_noerror_alloc == true; i++) {
      (*matrix)[i] = NULL;
      (*matrix)[i] = (int *)(calloc(columns, sizeof(int)));
      if ((*matrix)[i] == NULL) {
        fl_noerror_alloc = false;
        for (int j = 0; j < i; j++) {
          free((*matrix)[j]);
          (*matrix)[j] = NULL;
        }
        free(*matrix);
        *matrix = NULL;
      }
    }
  } else {
    fl_noerror_alloc = false;
  }
  return fl_noerror_alloc;
}

bool allocStart() {
  bool fl_noerror = allocMatrix(&CurData.field, H_FIELD, W_FIELD);
  if (fl_noerror == true) {
    fl_noerror = allocMatrix(&CurData.next, H_FIGMAX, W_FIGMAX);
  }
  return fl_noerror;
}

void auto_effectAttaching() {
  createStatic();
  recordIfFullRows();
  CurData.speed =
      BASE_CYCLES_FOR_SPEED - CurData.level * FACTOR_LEVEL_FOR_SPEED;
}

void auto_makeSpawn() {
  for (int i = 0; i < H_FIGMAX; i++) {
    for (int j = 0; j < W_FIGMAX; j++) {
      curDownFigure[i][j] = CurData.next[i][j];
    }
  }
  curDown = next;
  Point.y = -1;
  Point.x = W_FIELD / 2 - W_FIGMAX / 2;
  makeNextFigure();
  curStatus = MovingDown;
}

void auto_moveDown() {
  if (CurData.speed <= 0) {
    if (checkAttach() != Attaching) {
      moveDown();
      CurData.speed =
          BASE_CYCLES_FOR_SPEED - CurData.level * FACTOR_LEVEL_FOR_SPEED;
    }
  } else {
    (curStatus == DropDown) ? CurData.speed -= DROP_DOWN_REDUSE_FOR_SPEED
                            : CurData.speed--;
  }
}

GameState_t checkAttach() {
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX && curStatus != Attaching; i++) {
    int y = i + relativ_Y;
    if (y < 0 || y >= H_FIELD) {
      continue;
    }
    for (int j = 0; j < W_FIGMAX && curStatus != Attaching; j++) {
      int x = j + Point.x;
      if (x < 0 || x >= H_FIELD) {
        continue;
      }
      if (curDownFigure[i][j] == PIXEL_MOVING) {
        int y_next = y + 1;
        if (y_next == H_FIELD || CurData.field[y_next][x] == PIXEL_STATIC) {
          curStatus = Attaching;
        }
      }
    }
  }
  return curStatus;
}

GameState_t checkGameOver() {
  curStatus = Spawn;
  for (int j = 0; j < W_FIELD && curStatus != GameOver; j++) {
    if (CurData.field[0][j] == PIXEL_STATIC) {
      curStatus = GameOver;
    }
  }
  return curStatus;
}

void clearField() {
  for (int i = 0; i < H_FIELD; i++) {
    for (int j = 0; j < W_FIELD; j++) {
      CurData.field[i][j] = PIXEL_EMPTY;
    }
  }
}

void createStatic() {
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX; i++) {
    int y = i + relativ_Y;
    if (y < 0 || y >= H_FIELD) {
      continue;
    }
    for (int j = 0; j < W_FIGMAX; j++) {
      int x = j + Point.x;
      if (x < 0 || x >= W_FIELD) {
        continue;
      }
      if (curDownFigure[i][j] == PIXEL_MOVING) {
        CurData.field[y][x] = PIXEL_STATIC;
      }
    }
  }
}

void eraseFigure() {
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX; i++) {
    int y = i + relativ_Y;
    if (y < 0 || y >= H_FIELD) {
      continue;
    }
    for (int j = 0; j < W_FIGMAX; j++) {
      int x = j + Point.x;
      if (x < 0 || x >= W_FIELD) {
        continue;
      }
      if (CurData.field[y][x] != PIXEL_STATIC) {
        CurData.field[y][x] = PIXEL_EMPTY;
      }
    }
  }
}

void finger_dropDown() {
  if (curStatus == MovingDown) {
    curStatus = DropDown;
  }
}

void finger_makeExitStatus() { curStatus = Exit; }

void finger_moveLeft() {
  if (curStatus == MovingDown) {
    moveLeft();
  }
}

void finger_moveRight() {
  if (curStatus == MovingDown) {
    moveRight();
  }
}

void finger_plug() {}

void finger_startNewGame() {
  if (curStatus != Exit) {
    if (curStatus != StartMenu) {
      resetErase();
      clearField();
      curStatus = StartMenu;
    } else {
      curStatus = Spawn;
      makeNextFigure();
      setInitialValues();
    }
  }
}

void finger_turn90() {
  if (curStatus == MovingDown) {
    turn90();
  }
}

void finger_turnPause() {
  if (curStatus == MovingDown) {
    curStatus = OnPause;
    fl_pause_for_drop_down = false;
  } else if (curStatus == DropDown) {
    curStatus = OnPause;
    fl_pause_for_drop_down = true;
  } else if (curStatus == OnPause && fl_pause_for_drop_down == false) {
    curStatus = MovingDown;
  } else if (curStatus == OnPause && fl_pause_for_drop_down == true) {
    curStatus = DropDown;
  }
}

void freeGame() {
  removeMatrix(&CurData.field, H_FIELD);
  removeMatrix(&CurData.next, H_FIGMAX);
}

GameState_t getCurrentState() { return curStatus; }

FullRows getFullRows() { return Erase; }

int getHighScore() {
  int num = 0;
  FILE *fp = fopen(FILE_HIGHSCORE, "r");
  if (fp != NULL) {
    fscanf(fp, "%d", &num);
    fclose(fp);
  }
  return num;
}

void makeNextFigure() {
  next.turnType = 0;
  srand(time(NULL));
  next.numFigure = rand() % FIGURES;
  for (int i = 0; i < H_FIGMAX; i++) {
    for (int j = 0; j < W_FIGMAX; j++) {
      CurData.next[i][j] = figures[next.turnType][next.numFigure][i][j];
    }
  }
}

void moveDown() {
  eraseFigure();
  Point.y++;
  placeFigure();
}

void moveLeft() {
  bool fl_can_move = true;
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX && fl_can_move == true; i++) {
    bool fl_not_break = true;
    for (int j = 0; j < W_FIGMAX && fl_not_break == true; j++) {
      if (curDownFigure[i][j] == PIXEL_MOVING) {
        fl_not_break = false;
        int y = i + relativ_Y;
        int x = j + Point.x;
        if (x < 1) {
          fl_can_move = false;
        } else if (y >= 0 && y < H_FIELD && x < W_FIELD &&
                   CurData.field[y][x - 1] == PIXEL_STATIC) {
          fl_can_move = false;
        }
      }
    }
  }
  if (fl_can_move == true) {
    eraseFigure();
    Point.x--;
    placeFigure();
  }
}

void moveRight() {
  bool fl_can_move = true;
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX && fl_can_move == true; i++) {
    bool fl_not_break = true;
    for (int j = W_FIGMAX - 1;
         j >= 0 && fl_not_break == true && fl_can_move == true; j--) {
      if (curDownFigure[i][j] == PIXEL_MOVING) {
        fl_not_break = false;
        int y = i + relativ_Y;
        int x = j + Point.x;
        if (x >= W_FIELD - 1) {
          fl_can_move = false;
        } else if (y >= 0 && y < H_FIELD && x >= 0 &&
                   CurData.field[y][x + 1] == PIXEL_STATIC) {
          fl_can_move = false;
        }
      }
    }
  }
  if (fl_can_move == true) {
    eraseFigure();
    Point.x++;
    placeFigure();
  }
}

void placeFigure() {
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX; i++) {
    int y = i + relativ_Y;
    if (y < 0 || y >= H_FIELD) {
      continue;
    }
    for (int j = 0; j < W_FIGMAX; j++) {
      int x = j + Point.x;
      if (x < 0 || x >= W_FIELD) {
        continue;
      }
      if (CurData.field[y][x] != PIXEL_STATIC) {
        CurData.field[y][x] = curDownFigure[i][j];
      }
    }
  }
}

void recordIfFullRows() {
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX; i++) {
    int y = i + relativ_Y;
    if (y < 0 || y >= H_FIELD) {
      continue;
    }
    bool fl_full_row = 1;
    for (int j = 0; j < W_FIELD && fl_full_row == true; j++) {
      if (CurData.field[y][j] == PIXEL_EMPTY) {
        fl_full_row = false;
      }
    }
    if (fl_full_row == true) {
      Erase.rows[Erase.count++] = y;
    }
  }
  if (Erase.count != 0) {
    recordScores();
    CurData.level = CurData.score / SCORES_FOR_LEVEL;
    if (CurData.level > 10) {
      CurData.level = 10;
    }
  }
}

void recordScores() {
  if (Erase.count == 1) {
    CurData.score += SCORES_1R;
  } else if (Erase.count == 2) {
    CurData.score += SCORES_2R;
  } else if (Erase.count == 3) {
    CurData.score += SCORES_3R;
  } else if (Erase.count == 4) {
    CurData.score += SCORES_4R;
  }
  if (CurData.score > CurData.high_score) {
    writeHighScore();
    CurData.high_score = CurData.score;
  }
}

void removeMatrix(int ***matrix, int rows) {
  for (int i = 0; i < rows; i++) {
    free((*matrix)[i]);
    (*matrix)[i] = NULL;
  }
  free(*matrix);
  *matrix = NULL;
}

void resetErase() {
  for (int i = 0; i < Erase.count; i++) {
    Erase.rows[i] = 0;
  }
  Erase.count = 0;
}

void setInitialValues() {
  CurData.score = 0;
  CurData.high_score = getHighScore();
  CurData.level = 0;
  CurData.speed = BASE_CYCLES_FOR_SPEED;
  CurData.pause = 0;
  resetErase();
}

void shiftDownField() {
  for (int c = Erase.count; c > 0; c--) {
    for (int i = Erase.rows[Erase.count - 1]; i > 0; i--) {
      for (int j = 0; j < W_FIELD; j++) {
        CurData.field[i][j] = CurData.field[i - 1][j];
      }
    }
    for (int j = 0; j < W_FIELD; j++) {
      CurData.field[0][j] = PIXEL_EMPTY;
    }
    int relativ_Y = Point.y - (H_FIGMAX - 1);
    if (relativ_Y < 0) {
      for (int j = 0; j < W_FIGMAX; j++) {
        int x = j + Point.x;
        if (x < 0 || x >= W_FIELD) {
          continue;
        }
        if (curDownFigure[-relativ_Y - 1][j] == PIXEL_MOVING) {
          CurData.field[0][x] = PIXEL_STATIC;
        }
      }
      Point.y++;
    }
  }
}

void turn90() {
  curDown.turnType++;
  if (curDown.turnType > TURNTYPE - 1) {
    curDown.turnType = 0;
  }
  bool fl_can_turn = true;
  int relativ_Y = Point.y - (H_FIGMAX - 1);
  for (int i = 0; i < H_FIGMAX && fl_can_turn == true; i++) {
    for (int j = 0; j < W_FIGMAX && fl_can_turn == true; j++) {
      if (figures[curDown.turnType][curDown.numFigure][i][j] == PIXEL_MOVING) {
        int y = i + relativ_Y;
        int x = j + Point.x;
        if (x < 0 || x >= W_FIELD || y >= H_FIELD) {
          fl_can_turn = false;
        } else if (y >= 0 && CurData.field[y][x] == PIXEL_STATIC) {
          fl_can_turn = false;
        }
      }
    }
  }
  if (fl_can_turn == true) {
    for (int i = 0; i < H_FIGMAX; i++) {
      for (int j = 0; j < W_FIGMAX; j++) {
        curDownFigure[i][j] =
            figures[curDown.turnType][curDown.numFigure][i][j];
        ;
      }
    }
    eraseFigure();
    placeFigure();
  }
}

GameInfo_t updateCurrentState() {
  switch (getCurrentState()) {
    case StartMenu:
      break;
    case Spawn:
      auto_makeSpawn();
      break;
    case MovingDown:
      auto_moveDown();
      break;
    case Attaching:
      auto_effectAttaching();
      break;
    case OnPause:
      break;
    case DropDown:
      auto_moveDown();
      break;
    case GameOver:
      break;
    case Exit:
      break;
  }
  return CurData;
}

void userInput(UserAction_t action, bool hold) {
  if (hold == false) {
    switch (action) {
      case Start:
        finger_startNewGame();
        break;
      case Pause:
        finger_turnPause();
        break;
      case Terminate:
        finger_makeExitStatus();
        break;
      case Left:
        finger_moveLeft();
        break;
      case Right:
        finger_moveRight();
        break;
      case Up:
        finger_plug();
        break;
      case Down:
        finger_dropDown();
        break;
      case Action:
        finger_turn90();
        break;
    }
  }
}

void writeHighScore() {
  FILE *fp = fopen(FILE_HIGHSCORE, "w");
  if (fp != NULL) {
    fprintf(fp, "%d", CurData.score);
    fclose(fp);
  }
}