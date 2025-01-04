#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // Функция запуска игры в шашки
    int play() {
        // Записываем время начала игры для последующего измерения длительности игры
        auto start = chrono::steady_clock::now();

        // Проверяем, является ли данный запуск игрой-реваншем (повторной)
        if (is_replay) {
            // Инициализируем логику игры и перезагружаем начальную конфигурацию
            logic = Logic(&board, &config);
            config.reload();
            board.redraw();  // Перерисовываем доску
        } else {
            // Начинаем отрисовку доски
            board.start_draw();
        }
        // Сбрасываем флаг реванша
        is_replay = false;

        int turn_num = -1;  // Инициализация номера хода
        bool is_quit = false;  // Флаг для определения выхода из игры

        // Извлекаем из конфигурации максимальное количество ходов
        const int Max_turns = config("Game", "MaxNumTurns");

        // Цикл, управляющий игровым процессом, продолжается максимум до Max_turns ходов
        while (++turn_num < Max_turns) {
            beat_series = 0;  // Сбрасываем количество подряд битьев

            // Определяем возможные ходы для текущего игрока
            logic.find_turns(turn_num % 2);

            // Если нет возможных ходов, выходим из цикла
            if (logic.turns.empty())
                break;

            // Устанавливаем максимальную глубину для логики бота на текущем ходу
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));

            // Проверяем, является ли текущий игрок человеком (не бот)
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot"))) {
                // Выполняем ход игрока и получаем его ответ
                auto resp = player_turn(turn_num % 2);

                // Обрабатываем различные виды ответов игрока
                if (resp == Response::QUIT) {
                    is_quit = true;  // Игрок решил выйти из игры
                    break;
                } else if (resp == Response::REPLAY) {
                    is_replay = true;  // Игрок захотел сыграть ещё раз
                    break;
                } else if (resp == Response::BACK) {
                    // Игрок решил отменить ход

                    // Проверяем, играем ли против бота и есть ли отменённые действия в истории
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2) {
                        board.rollback();  // Откатываем ход
                        --turn_num;  // Возвращаем номер хода
                    }
                    if (!beat_series) {
                        --turn_num;  // Уменьшаем номер хода
                    }
                    board.rollback();  // Откатываем ход
                    --turn_num;  // Уменьшаем номер хода
                    beat_series = 0;  // Сбрасываем серию битв
                }
            } else {
                // Ход выполняет бот
                bot_turn(turn_num % 2);
            }
        }

        // Записываем время окончания игры
        auto end = chrono::steady_clock::now();

        // Логгируем время игры в файл
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();

        // Если игрок запросил реванш, запускаем игру заново
        if (is_replay)
            return play();

        // Проверяем, вышел ли игрок
        if (is_quit)
            return 0;

        int res = 2;  // По умолчанию результат равен ничьей

        // Определяем результат игры
        if (turn_num == Max_turns) {
            res = 0;  // Игра достигла максимального числа ходов, ничья
        } else if (turn_num % 2) {
            res = 1;  // Выиграли чёрные
        }

        // Показываем итог игры
        board.show_final(res);

        // Ждём ответа от игрока (например, на вопрос, хочет ли он сыграть ещё раз)
        auto resp = hand.wait();
        if (resp == Response::REPLAY) {
            is_replay = true;
            return play();  // Рестарт игры при запросе реванша
        }

        return res;  // Возвращаем результат игры
    }

  private:
    // Функция, выполняющая ход бота в игре
    void bot_turn(const bool color) {
        // Записываем время начала хода бота для измерения его длительности
        auto start = chrono::steady_clock::now();

        // Получаем задержку для бота из конфигурации (в миллисекундах)
        auto delay_ms = config("Bot", "BotDelayMS");

        // Создаем новый поток для обеспечения равномерной задержки для каждого хода
        thread th(SDL_Delay, delay_ms);

        // Находим лучшие ходы для бота в зависимости от его цвета
        auto turns = logic.find_best_turns(color);

        // Ждём завершения потока с задержкой
        th.join();

        bool is_first = true; // Флаг для определения первого хода в серии
        // Выполняем ходы
        for (auto turn : turns) {
            // Если это не первый ход, выполняем задержку, имитируя мышление бота между перемещениями
            if (!is_first) {
                SDL_Delay(delay_ms);
            }
            is_first = false; // Помечаем, что первый ход уже выполнен
            
            // Увеличиваем счётчик серии побитий, если в текущем ходе была побита фигура
            beat_series += (turn.xb != -1);
            
            // Перемещаем фигуру на доске
            board.move_piece(turn, beat_series);
        }

        // Записываем время окончания хода бота
        auto end = chrono::steady_clock::now();

        // Логируем время выполнения хода бота в файл
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();
    }

Response player_turn(const bool color) {
    // Инициализация контейнера для хранения позиций доступных ячеек
    vector<pair<POS_T, POS_T>> cells;

    // Заполняем контейнер доступными ходами для текущего игрока
    for (auto turn : logic.turns) {
        cells.emplace_back(turn.x, turn.y);
    }

    // Подсвечиваем доступные клетки на доске
    board.highlight_cells(cells);

    // Инициализируем переменные для хранения позиции перемещаемой шашки
    move_pos pos = {-1, -1, -1, -1};
    POS_T x = -1, y = -1;

    // Попытка выполнить первый ход
    while (true) {
        // Получаем выбор ячейки пользователя
        auto resp = hand.get_cell();

        // Если выбор не является ячейкой, возвращаем ответ (выход, повтор и т.д.)
        if (get<0>(resp) != Response::CELL)
            return get<0>(resp);

        // Извлекаем координаты выбранной ячейки
        pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

        bool is_correct = false;  // Флаг для проверки корректности выбора

        // Проверяем, является ли выбор началом возможного хода
        for (auto turn : logic.turns) {
            if (turn.x == cell.first && turn.y == cell.second) {
                is_correct = true;  // Выбор корректен
                break;
            }

            // Проверяем, является ли выбор движением в рамках текущего перехода
            if (turn == move_pos{x, y, cell.first, cell.second}) {
                pos = turn;  // Устанавливаем позицию для движений
                break;
            }
        }

        // Если выбрана корректная позиция для движения, выходим из цикла
        if (pos.x != -1)
            break;

        // Если выбор некорректен, сбрасываем выделения и очищаем выбор
        if (!is_correct) {
            if (x != -1) {
                board.clear_active();
                board.clear_highlight();
                board.highlight_cells(cells);  // Подсвечиваем доступные клетки
            }
            x = -1;
            y = -1;
            continue;
        }

        // Запоминаем выбранные координаты
        x = cell.first;
        y = cell.second;

        // Очищаем подсветку и отмечаем активную клетку
        board.clear_highlight();
        board.set_active(x, y);

        // Собираем клетки, куда можно переместиться из выбранной позиции
        vector<pair<POS_T, POS_T>> cells2;
        for (auto turn : logic.turns) {
            if (turn.x == x && turn.y == y) {
                cells2.emplace_back(turn.x2, turn.y2);
            }
        }
        // Подсвечиваем клетки, доступные для следующего движения
        board.highlight_cells(cells2);
    }

    // Очищаем подсветку и активные клетки, а затем перемещаем шашку
    board.clear_highlight();
    board.clear_active();
    board.move_piece(pos, pos.xb != -1);

    // Если нет продолжения битья, возвращаем нормальный ответ
    if (pos.xb == -1)
        return Response::OK;

    // Начинаем серию битья
    beat_series = 1;

    // Цикл для продолжающегося битья
    while (true) {
        // Определяем возможные ходы для продожения битья
        logic.find_turns(pos.x2, pos.y2);

        // Если нет доступных битьев, выходим из цикла
        if (!logic.have_beats)
            break;

        vector<pair<POS_T, POS_T>> cells;
        // Собираем клетки, куда возможен удар
        for (auto turn : logic.turns) {
            cells.emplace_back(turn.x2, turn.y2);
        }
        // Подсвечиваем клетки, доступные для удара
        board.highlight_cells(cells);
        board.set_active(pos.x2, pos.y2);

        // Попытка выполнить ход во время серии ударов
        while (true) {
            auto resp = hand.get_cell();
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp);

            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};
            bool is_correct = false;
            for (auto turn : logic.turns) {
                if (turn.x2 == cell.first && turn.y2 == cell.second) {
                    is_correct = true;
                    pos = turn;
                    break;
                }
            }

            // Если некорректный выбор, повторяем попытку
            if (!is_correct)
                continue;

            // Если корректный выбор, очищаем выделения и перемещаем шашку
            board.clear_highlight();
            board.clear_active();
            beat_series += 1;
            board.move_piece(pos, beat_series);

            break;
        }
    }

    // Возвращаем успешный ответ
    return Response::OK;
}

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};
