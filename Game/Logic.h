#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
    }

    vector<move_pos> find_best_turns(const bool color)
    {
    }

private:
    // Функция, делающая ход на доске и возвращающая обновленное состояние доски
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const {
        // Если в ходе была побита шашка, удаляем её с доски
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0;

        // Проверка на возможность превращения шашки в дамку
        // Если белая шашка достигает противоположного края (нулевой ряд), или
        // если чёрная шашка достигает противоположного края (седьмой ряд), превращаем её в дамку
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;

        // Перемещение фигуры на новую позицию
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        // Очистка начальной позиции фигуры
        mtx[turn.x][turn.y] = 0;

        // Возврат обновленной матрицы доски
        return mtx;
    }

    // Функция для расчёта оценки позиции на доске, принимающая текущую матрицу позиций и цвет первого бота
    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const {
        // color - кто является максимизирующим игроком
        double w = 0, wq = 0, b = 0, bq = 0; // Инициализация переменных для подсчёта фигур
        for (POS_T i = 0; i < 8; ++i) { // Перебор всех строк матрицы
            for (POS_T j = 0; j < 8; ++j) { // Перебор всех столбцов матрицы
                // Подсчитываем количество белых шашек
                w += (mtx[i][j] == 1);
                // Подсчитываем количество белых дамок
                wq += (mtx[i][j] == 3);
                // Подсчитываем количество чёрных шашек
                b += (mtx[i][j] == 2);
                // Подсчитываем количество чёрных дамок
                bq += (mtx[i][j] == 4);
                // Если режим подсчета учета номера и потенциала активирован
                if (scoring_mode == "NumberAndPotential") {
                    // Учитываем потенциальное продвижение для белых шашек (чем ближе к королевской линии, тем выше значимость)
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);
                    // Учитываем потенциальное продвижение для чёрных шашек
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        // Если первый бот не белых, меняем местами значения черных и белых
        if (!first_bot_color) {
            swap(b, w);
            swap(bq, wq);
        }
        // Если на доске нет белых фигур, возвращаем бесконечность как максимальную проигрышную оценку
        if (w + wq == 0)
            return INF;
        // Если на доске нет чёрных фигур, возвращаем 0 как минимальную проигрышную оценку
        if (b + bq == 0)
            return 0;
        int q_coef = 4; // Базовый коэффициент для дамок
        // Если режим подсчета с учетом потенциала используется, изменяем коэффициент для дамок
        if (scoring_mode == "NumberAndPotential") {
            q_coef = 5;
        }
        // Возвращаем отношение атакующего потенциала черных к белым, учитывая вес дамок
        return (b + bq * q_coef) / (w + wq * q_coef);
    }

    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
                                double alpha = -1)
    {
    }

    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
                               double beta = INF + 1, const POS_T x = -1, const POS_T y = -1)
    {
    }

public:
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board());
    }

    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board());
    }

private:
    // Функция поиска возможных ходов для указанного цвета фигур на доске
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx) {
        vector<move_pos> res_turns; // Вектор для хранения всех найденных ходов
        bool have_beats_before = false; // Флаг, указывающий, были ли найдены ходы с битьём ранее

        // Проходим по всем клеткам доски
        for (POS_T i = 0; i < 8; ++i) {
            for (POS_T j = 0; j < 8; ++j) {
                // Проверяем, есть ли фигура в клетке и совпадает ли её цвет с текущим цветом хода
                if (mtx[i][j] && mtx[i][j] % 2 != color) {
                    // Ищем возможные ходы из данной клетки
                    find_turns(i, j, mtx);

                    // Если найдены ходы с битьём и ранее таких ходов не было
                    if (have_beats && !have_beats_before) {
                        have_beats_before = true; // Обновляем флаг
                        res_turns.clear(); // Очистка списка ходов для добавления только ходов с битьём
                    }
                    
                    // Если ранее найдены ходы с битьём или если таких ходов не было
                    if ((have_beats_before && have_beats) || !have_beats_before) {
                        // Добавляем найденные ходы в общий список
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }

        // Обновляем общий вектор ходов
        turns = res_turns;

        // Перемешиваем вектор ходов для случайности
        shuffle(turns.begin(), turns.end(), rand_eng);
        
        // Обновляем статус наличия ходов с битьём
        have_beats = have_beats_before;
    }

    // Функция для поиска возможных ходов для фигуры на позиции (x, y) на доске mtx
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx) {
        turns.clear();         // Очистка вектора ходов перед поиском новых
        have_beats = false;    // Флаг наличия побитий сбрасывается в false
        POS_T type = mtx[x][y];// Определение типа фигуры по позиции на доске

        // Проверка возможностей для взятия (битья)
        switch (type) {
        case 1:
        case 2:
            // Проверка для обычных шашек
            for (POS_T i = x - 2; i <= x + 2; i += 4) {
                for (POS_T j = y - 2; j <= y + 2; j += 4) {
                    // Проверка на выход за границы доски
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2; // Позиция между начальной и конечной, потенциально занятая
                    // Проверка условий невозможности хода: конечная позиция занята или нет вражеской фигуры на пути
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    // Добавление возможного хода (с побитием) в вектор ходов
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default:
            // Проверка для дамок
            for (POS_T i = -1; i <= 1; i += 2) { // Перебор направлений по вертикальной оси
                for (POS_T j = -1; j <= 1; j += 2) { // Перебор направлений по горизонтальной оси
                    POS_T xb = -1, yb = -1; // Инициализация позиций возможного побития
                    // Перебор возможных позиций вдоль диагонали
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j) {
                        if (mtx[i2][j2]) {
                            // Если встречена своя фигура или уже была обнаружена вражеская, прерываем
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1)) {
                                break;
                            }
                            xb = i2; // Фиксация позиции вражеской фигуры
                            yb = j2;
                        }
                        // Если была встречена вражеская фигура, сохраняем ход с побитием
                        if (xb != -1 && xb != i2) {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }

        // Проверка, были ли обнаружены какие-либо ходы с побитием
        if (!turns.empty()) {
            have_beats = true; // Обновление флага наличия побитий
            return; // Выход из функции, если найдены ходы с битьем
        }

        // Проверка других возможных ходов без битья
        switch (type) {
        case 1:
        case 2:
            // Проверка ходов для обычных шашек
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1); // Определение направления хода
                for (POS_T j = y - 1; j <= y + 1; j += 2) {
                    // Проверка на выход за границы и занятость клетки
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    // Добавление возможного хода в вектор ходов
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default:
            // Проверка ходов для дамок
            for (POS_T i = -1; i <= 1; i += 2) { // Перебор направлений по вертикальной оси
                for (POS_T j = -1; j <= 1; j += 2) { // Перебор направлений по горизонтальной оси
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j) {
                        // Прерывание, если клетка занята
                        if (mtx[i2][j2])
                            break;
                        // Добавление возможного хода дамки
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

public:
    vector<move_pos> turns;       // Вектор для хранения всех возможных ходов, которые могут быть сделаны
    bool have_beats;              // Флаг, указывающий, существуют ли ходы с битьём на текущий момент
    int Max_depth;                // Максимальная глубина поиска

private:
    default_random_engine rand_eng; // Генератор случайных чисел для случайного перемешивания ходов или других стохастических процессов
    string scoring_mode;            // Строка, определяющая текущий режим подсчёта баллов для оценки позиций на доске
    string optimization;            // Строка, обозначающая используемую стратегию оптимизации в алгоритмах игры
    vector<move_pos> next_move;     // Вектор, содержащий последовательность ходов, запланированных выполнить в следующем ходе
    vector<int> next_best_state;    // Вектор, содержащий состояния, соответствующие лучшим ходам на текущую глубину поиска
    Board *board;                   // Указатель на объект класса Board, представляющий текущее состояние доски
    Config *config;                 // Указатель на объект класса Config, содержащий настройки и конфигурации игры
};
