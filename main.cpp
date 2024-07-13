#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;
void move_character(int &y, int &x, int ch);
void draw_art_player(const vector<string>& art);
void game(const vector<string> art_player, const vector<string> player);
void draw_panel();
void main_menu(const vector<string> art_player, const vector<string> player);
void display_menu(const vector<string>& options, int selected);
void draw_player(const vector<string>& art, int x, int y, const vector<string>& player);
bool can_move(int y, int x, const vector<string>& player);
vector<string> read(const string& filepath);

void draw_art_player(const vector<string>& art) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int start_y = max_y - art.size();
    if (start_y < 0) {
        start_y = 0;
    }

    for (size_t i = 0; i < art.size(); ++i) {
        mvprintw(start_y + i, 0, art[i].c_str());
    }
}

void draw_player(const vector<string>& art, int x, int y) {
	for (size_t i = 0; i < art.size(); ++i) {
        mvprintw(y + i, x, art[i].c_str());
    }
}

void draw_panel() {
    int max_x;
    getmaxyx(stdscr, max_x, max_x);
	int x = COLS / 2;
    int y = LINES / 4;
    mvhline(LINES - 20, 0, '=', max_x);
    mvhline(LINES - 1, 0, '=', max_x);
    mvprintw(LINES - y, x, "Name: Ahggw", max_x);
    for (int i = 0; i < 18; ++i) {
        mvaddch(LINES - 2 - i, max_x - 1, '|'); // Рисуем '|' в правом столбце
    }
}

bool can_move(int y, int x, const vector<string>& player) {
    // Проверка границ
    if (y < 0 || y >= LINES || x < 0 || x >= COLS) {
        return false;
    }

    // Check for collision with '=' in the area where the player will be drawn
    for (size_t i = 0; i < player.size(); ++i) {
        if (mvinch(y + i, x) == '=') {
            return false;
        }
    }

    // Check the area two cells down for feet position
    for (size_t i = 0; i < player.size(); ++i) {
        if (mvinch(y + i, x) == '=') {
            return false;
        }
    }

    return true;
}

void move_character(int &y, int &x, int ch, const vector<string>& player) {
    int new_y = y;
    int new_x = x;

    switch (ch) {
        case 'w':
            new_y = y - 1;
            break;
        case 's':
            new_y = y + 1;
            break;
        case 'a':
            new_x = x - 1;
            break;
        case 'd':
            new_x = x + 1;
            break;
    }

    // Проверка возможности движения
    if (can_move(new_y, new_x, player)) {
        y = new_y;
        x = new_x;
    }
}

vector<string> read(const string& filepath) {
    ifstream file(filepath);
    vector<string> art;
    string line;

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filepath << endl;
        return art;
    }

    while (getline(file, line)) {
        art.push_back(line);
    }

    file.close();
    return art;
}

void game(const vector<string> art_player, const vector<string> player) {
	noecho();             // Отключение отображения вводимых символов
    curs_set(0);          // Скрыть курсор
	int x = COLS / 2;
    int y = LINES / 4;
    int ch;

    // Первоначальное размещение персонажа
    draw_panel();
    draw_art_player(art_player);
    draw_player(player, x, y);
    refresh();

    while ((ch = getch()) != 'q') { // Цикл продолжается до нажатия 'q'
        clear(); // Clear the screen
        draw_art_player(art_player);
        draw_panel();
        move_character(y, x, ch, player); // Move the character
        draw_player(player, x, y);
        refresh();
    }
}

void display_menu(const vector<string>& options, int selected) {
    clear(); // Очистка экрана
    for (size_t i = 0; i < options.size(); ++i) {
        if (i == selected) {
			attron(A_REVERSE); // Выделение текущего выбора
			mvprintw(i, 0, options[i].c_str());
			attroff(A_REVERSE); // Сброс выделения
        } else {
            mvprintw(i, 0, options[i].c_str());
        }
    }
    refresh(); // Обновление экрана
}

void main_menu(const vector<string> art_player, const vector<string> player) {
    vector<string> options = {
        "Start Game",
        "Help",
        "Exit"
    };

    int selected = 0; // Индекс выбранного пункта меню
    int ch;

    while (true) {
        display_menu(options, selected); // Отображение меню

        ch = getch(); // Ожидание нажатия клавиши
        switch (ch) {
            case 'w': // Вверх
                if (selected > 0) {
                    selected--;
                }
                break;
            case 's': // Вниз
                if (selected < options.size() - 1) {
                    selected++;
                }
                break;
            case '\n': // Нажатие Enter
                if (selected == 0) {
					clear();
                    game(art_player, player);
				}else if (selected == options.size() - 1) {
                    endwin();
                    exit(0);
                } 
                break;
        }
    }
}

int main() {
    initscr();            // Инициализация ncurses
    cbreak();             // Отключение канонического режима
    echo();             // Отключение отображения вводимых символов
    curs_set(0);          // Скрыть курсор
    keypad(stdscr, TRUE); // Включение обработки специальных клавиш

    vector<string> art_player = read("model/knight.txt");
	vector<string> player = read("model/player.txt");
	vector<string> art_start = read("model/start.txt");
	
    if (!art_player.empty(), !player.empty(), !art_start.empty()) {
        mvprintw(1, 1, "Error");
    } else {
        cerr << "Error: Art file is empty or could not be read." << endl;
        endwin();
        return 1;
    }
    
    main_menu(art_player, player);

    endwin(); // Завершение работы ncurses

    return 0;
}
