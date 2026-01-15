/**
 * @file defines.h
 * @brief Перечень всех макросов #defines для игры
 */

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <locale.h>
#include <wchar.h>

//! Имя файла с рекордами (создается в процессе игры если больше 0)
#define FILE_HIGHSCORE "HighScore.txt"

//! Количество вертикальных границ
#define BORDS_V 3
//! Количество горизонтальных границ
#define BORDS_G 2

//! Ширина игрового поля
#define W_FIELD 10
//! Высота игрового поля
#define H_FIELD 20

//! Ширина поля игровой информации
#define W_INFO 10

//! Количество фигур
#define FIGURES 7
//! Количество вращений фигур
#define TURNTYPE 4
//! Количество строк матрицы для фигур
#define W_FIGMAX 4
//! Количество столбцов матрицы для фигур
#define H_FIGMAX 4

//! Размер получаемых очков, если заполнена одна строка
#define SCORES_1R 100
//! Размер получаемых очков, если заполнены две строки
#define SCORES_2R 300
//! Размер получаемых очков, если заполнены три строки
#define SCORES_3R 700
//! Размер получаемых очков, если заполнены четыре строки
#define SCORES_4R 1500

//! Обозначение пустого пикселя
#define PIXEL_EMPTY 0
//! Обозначение пикселя со статическим элементом
#define PIXEL_STATIC -1
//! Обозначение пикселя с двигающимся элементом
#define PIXEL_MOVING 1
//! Вид пикселя пустого элемента
#define LOOK_PXL_EMPTY ACS_BULLET
//! Вид пикселя статического элемента
#define LOOK_PXL_STATIC ACS_BLOCK
//! Вид пикселя двигающегося элемента
#define LOOK_PXL_MOVING ACS_BLOCK

//! Количество очков за один уровень
#define SCORES_FOR_LEVEL 600

//! Базовое количество запусков функции movingDown(), после которого происходит
//! сдвиг фигуры на одну строку (для состояния MovingDown)
#define BASE_CYCLES_FOR_SPEED 38
//! Коэффициент снижения базового количества запусков функции movingDown() при
//! увеличении уровня на 1
#define FACTOR_LEVEL_FOR_SPEED 3
//! Регрессивное снижение базового количества запусков функции movingDown(),
//! если состояние DropDown
#define DROP_DOWN_REDUSE_FOR_SPEED 35

#endif