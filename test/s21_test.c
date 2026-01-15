#include <check.h>

#include "fsm.h"

Suite *s21_suite();
bool is_not_square(const GameInfo_t Data);
void moveDown_On_8_rows();
bool isReachRightEndFeield(const GameInfo_t Data);
bool isReachLeftEndFeield(const GameInfo_t Data);
bool isSaveFigure(const GameInfo_t Data);
bool isEqualFields(const int prev_field[H_FIELD][W_FIELD],
                   const GameInfo_t Data);
void fixStateField(int prev_field[H_FIELD][W_FIELD], const GameInfo_t Data);

int main() {
  SRunner *sr = srunner_create(NULL);
  srunner_add_suite(sr, s21_suite());
  srunner_run_all(sr, CK_NORMAL);
  int failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (failed == 0) ? 0 : 1;
}

START_TEST(test_allocStart_and_freeGame) {
  ck_assert_int_eq(allocMatrix(NULL, H_FIELD, W_FIELD), 0);
  int **matrix = NULL;
  ck_assert_int_eq(allocMatrix(&matrix, -1, W_FIELD), 0);
  ck_assert_int_eq(allocMatrix(&matrix, H_FIELD, -1), 0);
  ck_assert_int_eq(allocMatrix(&matrix, (int)2e9, W_FIELD), 0);
  ck_assert_int_eq(allocMatrix(&matrix, 2000000, (int)2e9), 0);
  ck_assert_int_eq(allocStart(), 1);
  freeGame();
  GameInfo_t Data = updateCurrentState();
  ck_assert_ptr_null(Data.field);
  ck_assert_ptr_null(Data.next);
}
END_TEST

START_TEST(test_setInitialValues) {
  setInitialValues();
  GameInfo_t Data = updateCurrentState();
  ck_assert_int_eq(Data.score, 0);
  ck_assert_int_eq(Data.level, 0);
  ck_assert_int_eq(Data.speed, BASE_CYCLES_FOR_SPEED);
  ck_assert_int_eq(Data.pause, 0);
  int high = getHighScore();
  ck_assert_int_eq(Data.high_score, high);
  FullRows fullRows = getFullRows();
  ck_assert_int_eq(fullRows.count, 0);
  ck_assert_int_eq(fullRows.rows[0], 0);
}
END_TEST

START_TEST(test_initiateDate) {
  GameInfo_t Data = updateCurrentState();  // до выделения памяти и первичной
                                           // инициализации переменных
  ck_assert_int_eq(getCurrentState(), StartMenu);
  ck_assert_int_eq(Data.score, 0);
  ck_assert_int_eq(Data.level, 0);
  ck_assert_int_eq(Data.speed, 0);  // Not yet inicilize
  ck_assert_int_eq(Data.pause, 0);
  ck_assert_int_eq(Data.high_score, 0);  // Not yet inicilize
  ck_assert_ptr_null(Data.field);        // Not yet allocate
  ck_assert_ptr_null(Data.next);         // Not yet allocate
  allocStart();
  updateCurrentState();  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);      // from StartMenu in Spawn
  Data = updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  int high = getHighScore();
  ck_assert_int_eq(Data.high_score, high);
  ck_assert_int_eq(Data.speed, BASE_CYCLES_FOR_SPEED);
  freeGame();
}
END_TEST

START_TEST(test_auto_Attaching_Spawn) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  while (getCurrentState() != Attaching) {  // спуск пока не произойдет
                                            // Attaching
    updateCurrentState();  // save MovingDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  int count = 31 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 30
                         // строк (больше высоты поля)
  while (count > 0) {
    ck_assert_int_eq(getCurrentState(), Attaching);
    count--;
    updateCurrentState();  // save Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  ck_assert_int_eq(checkGameOver(),
                   Spawn);  // from Attaching in Spawn or in GameOver
  ck_assert_int_eq(getCurrentState(), Spawn);
  updateCurrentState();  // from Spawn in MovingDown again
  ck_assert_int_eq(getCurrentState(), MovingDown);
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  freeGame();
}
END_TEST

START_TEST(test_auto_moveDown) {
  allocStart();
  userInput(Start, false);                 // from StartMenu in Spawn
  GameInfo_t Data = updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(Data.speed, BASE_CYCLES_FOR_SPEED);
  int speed = BASE_CYCLES_FOR_SPEED;
  int count = 21 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 20
                         // строк до соприкосновения с нижней границей поля
  while (getCurrentState() !=
         Attaching) {  // спуск в режиме MovingDown до соприкосновения с нижней
                       // границей поля
    ck_assert_int_eq(getCurrentState(), MovingDown);
    ck_assert_int_eq(
        Data.speed,
        speed);  // подтверждение для режима MovingDown установленного
                 // количества итераций до сдига вниз на одну строку
    if (speed <= 0)
      speed = BASE_CYCLES_FOR_SPEED;
    else
      speed--;
    count--;
    Data = updateCurrentState();  // save MovingDown until Attaching
  }
  ck_assert_int_eq(
      count,
      0);  // подтверждение прохождения всех 20 строк с верху до самого низа
  checkGameOver();       // from Attachin in Spawn or in GameOver
  updateCurrentState();  // from Spawn in MovingDown
  count = 21 * (BASE_CYCLES_FOR_SPEED +
                1);  // кол-во итераций, соответствующее прохождению 20 строк до
                     // соприкосновения с нижней границей поля
  while (getCurrentState() !=
         Attaching) {  // спуск в режиме MovingDown до соприкосновения со
                       // статическим объектом
    ck_assert_int_eq(getCurrentState(), MovingDown);
    updateCurrentState();  // save MovingDown until Attaching
  }
  ck_assert_int_ge(count, 0);  // подтверждение прохождения меньше 20 строк до
                               // соприкосновения со статическим объектом
  checkGameOver();       // from Attachin in Spawn or in GameOver
  updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Down, false);       // from MovingDown in DropDown
  Data = updateCurrentState();  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  speed = BASE_CYCLES_FOR_SPEED;
  ck_assert_int_eq(Data.speed, speed);
  while (getCurrentState() !=
         Attaching) {  // спуск в режиме DropDown до соприкосновения со
                       // статическим объектом
    ck_assert_int_eq(getCurrentState(), DropDown);
    ck_assert_int_eq(
        Data.speed, speed);  // подтверждение для режима DropDown установленного
                             // количества итераций до сдига вниз на одну строку
    if (speed <= 0)
      speed = BASE_CYCLES_FOR_SPEED;
    else
      speed -= DROP_DOWN_REDUSE_FOR_SPEED;
    Data = updateCurrentState();  // save DropDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  freeGame();
}
END_TEST

START_TEST(test_save_StartMenu) {
  allocStart();
  updateCurrentState();  // до первичной инициализации переменных
  ck_assert_int_eq(getCurrentState(), StartMenu);
  updateCurrentState();  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  int count = 31 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 30
                         // строк (больше высоты поля)
  while (count > 0) {
    ck_assert_int_eq(getCurrentState(), StartMenu);
    count--;
    updateCurrentState();  // save StartMenu
  }
  ck_assert_int_eq(getCurrentState(), StartMenu);
  freeGame();
}
END_TEST

START_TEST(test_save_OnPause) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  userInput(Pause, false);  // from MovingDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  updateCurrentState();  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  int count = 31 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 30
                         // строк (больше высоты поля)
  while (count > 0) {
    ck_assert_int_eq(getCurrentState(), OnPause);
    count--;
    updateCurrentState();  // save OnPause
  }
  ck_assert_int_eq(getCurrentState(), OnPause);
  freeGame();
}
END_TEST

START_TEST(test_save_GameOver) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  GameInfo_t Data = updateCurrentState();  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  bool having_high_static_element = false;
  for (int j = 0; j < W_FIELD; j++) {
    if (Data.field[0][j] == PIXEL_STATIC) {
      having_high_static_element = true;
    }
  }
  ck_assert_int_eq(having_high_static_element, true);
  int count = 31 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 30
                         // строк (больше высоты поля)
  while (count > 0) {
    ck_assert_int_eq(getCurrentState(), GameOver);
    count--;
    updateCurrentState();  // save GameOver
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  freeGame();
}
END_TEST

START_TEST(test_save_Exit) {
  allocStart();
  userInput(Terminate, false);  // from StartMenu in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  updateCurrentState();  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  int count = 31 * (BASE_CYCLES_FOR_SPEED +
                    1);  // кол-во итераций, соответствующее прохождению 30
                         // строк (больше высоты поля)
  while (count > 0) {
    ck_assert_int_eq(getCurrentState(), Exit);
    count--;
    updateCurrentState();  // save Exit
  }
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_dropDown) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Down, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Down, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Down, false);  // from MovingDown in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Pause, false);  // from DropDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Down, false);  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Pause, false);  // from OnPause in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Down, false);  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  while (getCurrentState() != Attaching) {
    ck_assert_int_eq(getCurrentState(), DropDown);  // save DropDown
    updateCurrentState();                           // DropDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Down, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Down, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Down, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_StarMenu) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Terminate, false);  // from StartMenu in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Terminate, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_Spawn) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Terminate, false);  // from Spawn in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_MovingDown) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Terminate, false);  // from MovingDown in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_DropDown) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Terminate, false);  // from DropDown in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_Pause) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  userInput(Pause, false);  // from MovingDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Terminate, false);  // from OnPause in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_Attaching) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Terminate, false);  // from Attaching in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_Exit_from_GameOver) {
  allocStart();
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_moveLeft) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Left, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Left, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  GameInfo_t Data = updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Left, false);  // save MovingDown + сдвиг влево на 1
  ck_assert_int_eq(getCurrentState(), MovingDown);
  moveDown_On_8_rows();  // спуск фигуры на 8 строк, чтобы полностью разместить
                         // её в поле
  updateCurrentState();
  int prev_field[H_FIELD][W_FIELD] = {0};
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что на текущий момент поля одинаковые
  userInput(Left, false);  // сдвиг влево на 1
  ck_assert_int_eq(isEqualFields(prev_field, Data),
                   false);  // подтвеждение, что после Left поля не одинаковые
  ck_assert_int_eq(
      isReachLeftEndFeield(Data),
      false);  // подтвеждение ещё не достигнутой левой границы поля
  userInput(Left, false);  // сдвиг влево на 1
  userInput(Left, false);  // сдвиг влево на 1
  userInput(Left, false);  // сдвиг влево на 1
  ck_assert_int_eq(isReachLeftEndFeield(Data),
                   true);  // подтвеждение достижения левой границы поля
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  userInput(Left, false);  // сдвиг влево на 1
  userInput(Left, false);  // сдвиг влево на 1
  ck_assert_int_eq(
      isSaveFigure(Data),
      true);  // подтвеждение не прохождения фигурой левой границы поля
  ck_assert_int_eq(isEqualFields(prev_field, Data),
                   true);  // подтвеждение, что после Left для крайнего
                           // положения поля одинаковые
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Pause, false);  // from MovingDown in OnPause
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Left, false);  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Left когда OnPause поля одинаковые
  userInput(Pause, false);  // from OnPause in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Left, false);  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Left когда DropDown поля одинаковые
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Left, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Left когда Attaching поля одинаковые
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Left, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Left, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_moveRight) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Right, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Right, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  GameInfo_t Data = updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Right, false);  // save MovingDown + сдвиг вправо на 1
  ck_assert_int_eq(getCurrentState(), MovingDown);
  moveDown_On_8_rows();  // спуск фигуры на 8 строк, чтобы полностью разместить
                         // её в поле
  updateCurrentState();
  int prev_field[H_FIELD][W_FIELD] = {0};
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что на текущий момент поля одинаковые
  userInput(Right, false);  // сдвиг вправо на 1
  ck_assert_int_eq(isEqualFields(prev_field, Data),
                   false);  // подтвеждение, что после Right поля не одинаковые
  ck_assert_int_eq(
      isReachRightEndFeield(Data),
      false);  // подтвеждение ещё не достигнутой правой границы поля
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Right, false);  // сдвиг вправо на 1
  ck_assert_int_eq(isReachRightEndFeield(Data),
                   true);  // подтвеждение достижения правой границы поля
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  userInput(Right, false);  // сдвиг вправо на 1
  userInput(Right, false);  // сдвиг вправо на 1
  ck_assert_int_eq(
      isSaveFigure(Data),
      true);  // подтвеждение не прохождения фигурой правой границы поля
  ck_assert_int_eq(isEqualFields(prev_field, Data),
                   true);  // подтвеждение, что после Right для крайнего
                           // положения поля одинаковые
  userInput(Left, false);   // сдвиг влево на 1
  userInput(Left, false);   // сдвиг влево на 1
  userInput(Left, false);   // сдвиг влево на 1
  userInput(Pause, false);  // from MovingDown in OnPause
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Right, false);  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Right когда OnPause поля одинаковые
  userInput(Pause, false);  // from OnPause in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Right, false);  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Right когда DropDown поля одинаковые
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Right, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Right когда Attaching поля одинаковые
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Right, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Right, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_plug) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Up, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Up, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Up, false);  // save MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Pause, false);  // from MovingDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Up, false);  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Pause, false);  // from OnPause in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Up, false);  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Up, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Up, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Up, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_startNewGame) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Start, false);  // from Spawn in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Start, false);  // from MovingDown in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Start, false);  // from DropDown in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  userInput(Pause, false);  // from MovingDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Start, false);  // from OnPause in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Start, false);  // from Attaching in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  updateCurrentState();     // from Spawn in MovingDown
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Start, false);  // from GameOver in StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Terminate, false);  // from StartMenu in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Start, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_turn90) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Action, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Action, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  GameInfo_t Data = updateCurrentState();  // from Spawn in MovingDown
  moveDown_On_8_rows();  // спуск фигуры на 8 строк, чтобы полностью разместить
                         // её в поле
  updateCurrentState();
  int prev_field[H_FIELD][W_FIELD] = {0};
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_ge(
      Data.speed,
      2);  // чтобы следующий вызов updateCurrentState() не привел к
           // автоматичпескому снижению фигуры на одну строку, нужен speed > 0
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Action, false);  // save MovingDown + поворот фигуры
  ck_assert_int_eq(getCurrentState(), MovingDown);
  bool fields_equal = isEqualFields(prev_field, Data);
  if (is_not_square(Data) == true) {
    ck_assert_int_eq(
        fields_equal,
        false);  // если не квадрат, то предыдущее поле будет отличаться
  } else {
    ck_assert_int_eq(fields_equal, true);
  }
  userInput(Pause, false);  // from MovingDown in OnPause
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Action, false);  // save OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Action когда OnPause поля одинаковые
  userInput(Pause, false);  // from OnPause in MovingDown
  userInput(Down, false);   // from MovingDown in DropDown
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Action, false);  // save DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Action когда DropDown поля одинаковые
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  fixStateField(
      prev_field,
      Data);  // запись текущего поля для сравнения после его изменения
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Action, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  ck_assert_int_eq(
      isEqualFields(prev_field, Data),
      true);  // подтвеждение, что после Action когда Attaching поля одинаковые
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Action, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Action, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_finger_turnPause) {
  allocStart();
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Pause, false);  // save StartMenu
  ck_assert_int_eq(getCurrentState(), StartMenu);
  userInput(Start, false);  // from StartMenu in Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  userInput(Pause, false);  // save Spawn
  ck_assert_int_eq(getCurrentState(), Spawn);
  updateCurrentState();  // from Spawn in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Pause, false);  // from MovingDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Pause, false);  // from OnPause in MovingDown
  ck_assert_int_eq(getCurrentState(), MovingDown);
  userInput(Down, false);  // from MovingDown in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  userInput(Pause, false);  // from DropDown in OnPause
  ck_assert_int_eq(getCurrentState(), OnPause);
  userInput(Pause, false);  // from OnPause in DropDown
  ck_assert_int_eq(getCurrentState(), DropDown);
  while (getCurrentState() != Attaching) {
    updateCurrentState();  // MovingDown until Attaching
  }
  ck_assert_int_eq(getCurrentState(), Attaching);
  userInput(Pause, false);  // save Attaching
  ck_assert_int_eq(getCurrentState(), Attaching);
  while (checkGameOver() !=
         GameOver) {  // спуск фигур в режиме MovingDown до завершения игры в
                      // результате появления статического элемента на самой
                      // верхней строке
    while (getCurrentState() != Attaching) {
      updateCurrentState();  // MovingDown
    }
    updateCurrentState();  // save Attaching (сделать PIXEL_STATIC для
                           // последующего checkGameOver)
  }
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Pause, false);  // save GameOver
  ck_assert_int_eq(getCurrentState(), GameOver);
  userInput(Terminate, false);  // from GameOver in Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  userInput(Pause, false);  // save Exit
  ck_assert_int_eq(getCurrentState(), Exit);
  freeGame();
}
END_TEST

START_TEST(test_getCurrentState) {
  GameState_t State = getCurrentState();
  ck_assert_int_eq(State, StartMenu);
}
END_TEST

START_TEST(test_getFullRows) {
  FullRows fullRows = getFullRows();
  ck_assert_int_eq(fullRows.count, 0);
  ck_assert_int_eq(fullRows.rows[0], 0);
}
END_TEST

Suite *s21_suite() {
  Suite *s = suite_create("s21_tetris_fsm");
  TCase *tc_auto_state = tcase_create("Auto_state");
  TCase *tc_user_input = tcase_create("User_input");
  TCase *tc_getters = tcase_create("Getters");

  tcase_add_test(tc_auto_state, test_allocStart_and_freeGame);
  tcase_add_test(tc_auto_state, test_setInitialValues);

  tcase_add_test(tc_auto_state, test_initiateDate);
  tcase_add_test(tc_auto_state, test_auto_Attaching_Spawn);
  tcase_add_test(tc_auto_state, test_auto_moveDown);
  tcase_add_test(tc_auto_state, test_save_StartMenu);
  tcase_add_test(tc_auto_state, test_save_OnPause);
  tcase_add_test(tc_auto_state, test_save_GameOver);
  tcase_add_test(tc_auto_state, test_save_Exit);

  tcase_add_test(tc_user_input, test_finger_dropDown);
  tcase_add_test(tc_user_input, test_finger_Exit_from_StarMenu);
  tcase_add_test(tc_user_input, test_finger_Exit_from_Spawn);
  tcase_add_test(tc_user_input, test_finger_Exit_from_MovingDown);
  tcase_add_test(tc_user_input, test_finger_Exit_from_DropDown);
  tcase_add_test(tc_user_input, test_finger_Exit_from_Pause);
  tcase_add_test(tc_user_input, test_finger_Exit_from_Attaching);
  tcase_add_test(tc_user_input, test_finger_Exit_from_GameOver);
  tcase_add_test(tc_user_input, test_finger_moveLeft);
  tcase_add_test(tc_user_input, test_finger_moveRight);
  tcase_add_test(tc_user_input, test_finger_plug);
  tcase_add_test(tc_user_input, test_finger_startNewGame);
  tcase_add_test(tc_user_input, test_finger_turn90);
  tcase_add_test(tc_user_input, test_finger_turnPause);

  tcase_add_test(tc_getters, test_getCurrentState);
  tcase_add_test(tc_getters, test_getFullRows);

  suite_add_tcase(s, tc_auto_state);
  suite_add_tcase(s, tc_user_input);
  suite_add_tcase(s, tc_getters);
  return s;
}

bool is_not_square(GameInfo_t Data) {
  bool fl_not_square = true;
  bool fl_find_first_element = false;
  int first_y = 0;
  int first_x = 0;
  for (int i = 0; i < H_FIELD && fl_find_first_element == false; i++) {
    for (int j = 0; j < W_FIELD && fl_find_first_element == false; j++) {
      if (Data.field[i][j] != PIXEL_EMPTY) {
        fl_find_first_element = true;
        first_y = i;
        first_x = j;
      }
    }
  }
  if (Data.field[first_y + 1][first_x] != PIXEL_EMPTY &&
      Data.field[first_y][first_x + 1] != PIXEL_EMPTY &&
      Data.field[first_y + 1][first_x + 1] != PIXEL_EMPTY) {
    fl_not_square = false;
  }
  return fl_not_square;
}

void moveDown_On_8_rows() {
  int count = 9 * (BASE_CYCLES_FOR_SPEED +
                   1);  // кол-во итераций, соответствующее прохождению 8 строк
                        // (чтобы фигура полностью разместилась в поле)
  while (count > 0) {
    count--;
    updateCurrentState();  // save MovingDown
  }
}

bool isReachRightEndFeield(const GameInfo_t Data) {
  bool fl_is_right_End = false;
  for (int i = 0; i < H_FIELD && fl_is_right_End == false; i++) {
    if (Data.field[i][W_FIELD - 1] != PIXEL_EMPTY) {
      fl_is_right_End = true;
    }
  }
  return fl_is_right_End;
}

bool isReachLeftEndFeield(const GameInfo_t Data) {
  bool fl_is_left_End = false;
  for (int i = 0; i < H_FIELD && fl_is_left_End == false; i++) {
    if (Data.field[i][0] != PIXEL_EMPTY) {
      fl_is_left_End = true;
    }
  }
  return fl_is_left_End;
}

bool isSaveFigure(const GameInfo_t Data) {
  int count_mov_pixels = 0;
  for (int i = 0; i < H_FIELD; i++) {
    for (int j = 0; j < W_FIELD; j++) {
      if (Data.field[i][j] == PIXEL_MOVING) {
        count_mov_pixels++;
      }
    }
  }
  return count_mov_pixels == 4;
}

bool isEqualFields(const int prev_field[H_FIELD][W_FIELD],
                   const GameInfo_t Data) {
  bool fl_is_equal_fields = true;
  for (int i = 0; i < H_FIELD && fl_is_equal_fields == true; i++) {
    for (int j = 0; j < W_FIELD && fl_is_equal_fields == true; j++) {
      if (prev_field[i][j] != Data.field[i][j]) {
        fl_is_equal_fields = false;
      }
    }
  }
  return fl_is_equal_fields;
}

void fixStateField(int prev_field[H_FIELD][W_FIELD], const GameInfo_t Data) {
  for (int i = 0; i < H_FIELD; i++) {
    for (int j = 0; j < W_FIELD; j++) {
      prev_field[i][j] = Data.field[i][j];
    }
  }
}