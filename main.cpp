#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <vector>
#include <string>

using namespace std;
void move_character(int &y, int &x, int ch);
void draw_art_player(const vector<string>& art);
void game(const vector<string> art_player, const vector<string> player);
void main_menu(const vector<string> art_player, const vector<string> player);
void display_menu(const vector<string>& options, int selected);
void draw_player(const vector<string>& art, int x, int y, const vector<string>& player);
bool can_move(int y, int x, const vector<string>& player);
vector<string> read(const string& filepath);
bool resized = false;

void handle_resize(int sig) {
    resized = true;
}

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
        mvaddch(LINES - 2 - i, max_x - 1, '|');
    }
}

bool can_move(int y, int x, const vector<string>& player) {
    if (y < 0 || y >= LINES || x < 0 || x >= COLS) {
        return false;
    }

    for (size_t i = 0; i < player.size(); ++i) {
        if (mvinch(y + i, x) == '=') {
            return false;
        }
    }

    for (size_t i = 0; i < player.size(); ++i) {
        if (mvinch(y + i, x) == '=') {
            return false;
        }
    }

    return true;
}

void erase_character(const vector<string>& player, int x, int y) {
    for (int i = 0; i < player.size(); ++i) {
        move(y + i, x);
        for (int j = 0; j < player[i].size(); ++j) {
            addch(' ');
        }
    }
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
	noecho();
    curs_set(0);
	int x = COLS / 2;
    int y = LINES / 4;
    int old_x = x;
    int old_y = y;
    int ch;
	signal(SIGWINCH, handle_resize);
    draw_panel();
    draw_art_player(art_player);
    draw_player(player, x, y);
    refresh();
	
    while ((ch = getch()) != 'q') {
		if (resized) {
            endwin();
            refresh();
            clear();
            resized = false;
            draw_panel();
            draw_art_player(art_player);
        }
        
		if (ch != ERR) {
			erase_character(player, old_x, old_y);
            move_character(y, x, ch, player);
            old_x = x;
            old_y = y;
		}
        draw_art_player(art_player);
        draw_panel();
        draw_player(player, x, y);
        refresh();
    }
}

void display_menu(const vector<string>& options, int selected) {
    clear();
    for (size_t i = 0; i < options.size(); ++i) {
        if (i == selected) {
			attron(A_REVERSE);
			mvprintw(i, 0, options[i].c_str());
			attroff(A_REVERSE);
        } else {
            mvprintw(i, 0, options[i].c_str());
        }
    }
    refresh();
}

void main_menu(const vector<string> art_player, const vector<string> player) {
    vector<string> options = {
        "Start Game",
        "Help",
        "Exit"
    };

    int selected = 0;
    int ch;

    while (true) {
        display_menu(options, selected);

        ch = getch();
        switch (ch) {
            case 'w':
                if (selected > 0) {
                    selected--;
                }
                break;
            case 's':
                if (selected < options.size() - 1) {
                    selected++;
                }
                break;
            case '\n':
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

void show_warning(const vector<string>& art) {
    int row, col;
    getmaxyx(stdscr, row, col); // Получение размеров окна
    int start_y = (row - art.size()) / 2;

    for (int i = 0; i < art.size(); ++i) {
        int start_x = (col - art[i].size()) / 2;
        mvprintw(start_y + i, start_x, "%s", art[i].c_str());
    }
    refresh();
    napms(5000); // Задержка на 5 секунд
    clear();
    refresh();
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
	vector<string> art_warning = read("model/warning.txt");
	
    if (!art_player.empty(), !player.empty(), !art_start.empty(), !art_warning.empty()) {

    } else {
        cerr << "Error: Art file is empty or could not be read." << endl;
        endwin();
        return 1;
    }
    mvprintw(1, 1, "Open full screen");
    show_warning(art_warning);
    main_menu(art_player, player);
    endwin();
    return 0;
}
