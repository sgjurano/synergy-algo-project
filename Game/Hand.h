#pragma once

#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс Hand отвечает за обработку взаимодействия игрока с доской через пользовательские события
class Hand {
public:
    // Конструктор класса Hand, принимающий указатель на объект доски
    Hand(Board *board) : board(board) {}

    // Метод для получения ячейки от игрока
    tuple<Response, POS_T, POS_T> get_cell() const {
        SDL_Event windowEvent;  // Структура для хранения событий окна
        Response resp = Response::OK;  // Инициализация типа ответа
        int x = -1, y = -1;  // Координаты нажатия мыши
        int xc = -1, yc = -1;  // Индексы клетки на доске, соответствующие координатам нажатия

        // Цикл обработки событий, продолжается до получения значимого ответа
        while (true) {
            // Проверяем, имеются ли события в очереди
            if (SDL_PollEvent(&windowEvent)) {
                switch (windowEvent.type) {
                case SDL_QUIT:  // Если событие - закрытие окна, устанавливаем соответствующий ответ
                    resp = Response::QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN:  // Если нажата кнопка мыши, обрабатываем это событие
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    // Преобразуем координаты клика на экране в индексы клетки на доске
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    // Обработка действий в зависимости от положения клика
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1) {
                        resp = Response::BACK;  // Запрос на отмену хода
                    } else if (xc == -1 && yc == 8) {
                        resp = Response::REPLAY;  // Запрос на переигровку
                    } else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8) {
                        resp = Response::CELL;  // Нормальное нажатие на клетку доски
                    } else {
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT:  // Обработка события изменения размера окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        board->reset_window_size();  // Изменение размеров окна
                        break;
                    }
                }
                // Если ответ больше не "ОК", выходим из цикла обработки событий
                if (resp != Response::OK)
                    break;
            }
        }
        // Возвращаем результирующий ответ и индексы клетки
        return {resp, xc, yc};
    }

    // Метод ожидания событий, возвращает тип ответа
    Response wait() const {
        SDL_Event windowEvent;  // Структура для событий окна
        Response resp = Response::OK;  // Инициализация переменной ответа

        // Цикл ожидания пока не будет получен значимый ответ
        while (true) {
            // Проверяем, имеются ли события в очереди
            if (SDL_PollEvent(&windowEvent)) {
                switch (windowEvent.type) {
                case SDL_QUIT:  // Обработка события выхода из приложения
                    resp = Response::QUIT;
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:  // Обработка события изменения размера окна
                    board->reset_window_size();
                    break;
                case SDL_MOUSEBUTTONDOWN: {  // Обработка события нажатия кнопки мыши
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8) {
                        resp = Response::REPLAY;  // Обработчик события переигровки
                    }
                }
                break;
                }
                // Если ответ больше не "ОК", выходим из цикла обработки событий
                if (resp != Response::OK)
                    break;
            }
        }
        // Возвращаем полученный ответ
        return resp;
    }

private:
    Board *board;  // Указатель на объект доски, необходимый для манипуляций с игрой
};
