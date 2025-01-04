#pragma once

#include <stdlib.h>

typedef int8_t POS_T;  // Определяем POS_T как тип int8_t для хранения координат

// Структура move_pos используется для описания перемещения шашки на доске
struct move_pos {
    POS_T x, y;             // Начальная позиция (откуда шашка перемещается)
    POS_T x2, y2;           // Конечная позиция (куда шашка перемещается)
    POS_T xb = -1, yb = -1; // Позиция сбитой шашки (если была), инициализирована значением -1

    // Конструктор для задания перемещения без сбития шашки
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2) {
        // Стандартный конструктор перемещения, без информации о сбитых шашках
    }

    // Конструктор для задания перемещения с возможным сбитием шашки
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb) {
        // Конструктор с указанием начальной и конечной позиции, включая сбитую шашку
    }

    // Оператор сравнения, проверяющий совпадение двух перемещений (без учета сбитой шашки)
    bool operator==(const move_pos &other) const {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }

    // Оператор неравенства, проверяющий несоответствие двух перемещений
    bool operator!=(const move_pos &other) const {
        return !(*this == other);
    }
};
