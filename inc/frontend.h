/**
 * @file frontend.h
 * @brief Графическая часть реализации интеррактивной игры
 */

#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#define _POSIX_C_SOURCE 200112L  // POSIX.1-2001

#include <locale.h>
#include <ncurses.h>
#include <time.h>
#include <wchar.h>

#include "defines.h"
#include "fsm.h"

/**
 * @brief Функция игрового цикла. Пока пользователь не нажмет на выход, игра
 * активна
 */
void game_loop();

/**
 * @brief Функция для получения сигналов пользоавтеля с клавиатуры и их
 * обработки с запуском соответствующих функций
 */
void getUserInput();

/**
 * @brief Функция для формирования в окне границ
 */
void drawBoards();

/**
 * @brief Функция для формирования в окне игрового поля в соответствии с
 * полученными на вход данными
 * @param Data Указатель на текущий объект структуры игровой информации
 */
void drawField(const GameInfo_t *Data);

/**
 * @brief Функция для формирования в окне игровой информации в соответствии с
 * полученными на вход данными
 * @param Data Указатель на текущий объект структуры игровой информации
 */
void drawInfo(const GameInfo_t *Data);

/**
 * @brief Функция для фомирования в окне стартового меню
 */
void drawStartMenu();

/**
 * @brief Функция для обновления и отображения окна со всеми актуальными на
 * текущий момент игровыми данными
 * @param Data Указатель на текущий объект структуры игровой информации
 */
void showDataNow(const GameInfo_t *Data);

/**
 * @brief Функция для мигания в окне удаляемых из игрового поля заполненных
 * строк
 * @param EraseRows Указатель текущий объект структуры FullRows с данными о
 * количестве и номерах заполненных строк
 */
void showDestroyRows(const FullRows *EraseRows);

/**
 * @brief Функция для обновления и отображения окна с информацией стартового
 * меню
 * @param Data Указатель на текущий объект структуры игровой информации
 */
void showStartMenu(const GameInfo_t *Data);

/**
 * @brief Функция настройки API библиотеки ncurses перед игрой
 */
void tune_ncurses();

#endif