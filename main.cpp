#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

void move_character(int &y, int &x, int ch, const vector<string>& player);
void draw_art_player(const vector<string>& art);
void game(const vector<string> art_player, const vector<string> player, const vector<string> map, const string& name, int level);
void main_menu(const vector<string> art_player, const vector<string> player, const vector<string> art_start, const vector<string> art_help, const vector<string> art_exit, const vector<string> map, const string& name, int level);
void display_menu(const vector<string>& options, int selected, const vector<string>& art_start, const vector<string>& art_help, const vector<string>& art_exit);
void draw_player(const vector<string>& art, int x, int y);
bool can_move(int y, int x, const vector<string>& player);
vector<string> read(const string& filepath);
bool resized = false;

struct Player {
    string name;
    int level;
};

Player loadPlayerInfo(const string& filename) {
    Player player;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        if (getline(iss, key, ':')) {
            string value;
            if (getline(iss, value)) {
                if (key == "name") {
                    player.name = value;
                } else if (key == "lvl") {
                    player.level = stoi(value);
                }
            }
        }
    }
    return player;
}

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

void draw_panel(const string& name, int level) {
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);
    int x = COLS / 2;
    int y = LINES / 4;
    mvhline(LINES - 20, 0, '=', max_x);
    mvhline(LINES - 1, 0, '=', max_x);
    mvprintw(LINES - y, x, "Name: %s", name.c_str());
    mvprintw(LINES - y + 1, x, "lvl: %d", level);
    for (int i = 0; i < 18; ++i) {
        mvaddch(LINES - 2 - i, max_x - 1, '|');
    }
}

void drawMap(WINDOW* win, const vector<string>& map, int startRow, int startCol) {
    int height, width;
    getmaxyx(win, height, width);

    for (int i = 0; i < height && i + startRow < map.size(); ++i) {
        for (int j = 0; j < width && j + startCol < map[i + startRow].size(); ++j) {
            mvwaddch(win, i, j, map[i + startRow][j + startCol]);
        }
    }

    wrefresh(win);
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

    return true;
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

void erase_character(const vector<string>& player, int x, int y) {
    for (size_t i = 0; i < player.size(); ++i) {
        move(y + i, x);
        clrtoeol();
    }
}

void move_character(int &y, int &x, int ch, const vector<string>& player) {
    int new_y = y;
    int new_x = x;

    switch (ch) {
        case 'w':
        case KEY_UP:
            new_y = y - 1;
            break;
        case 's':
        case KEY_DOWN:
            new_y = y + 1;
            break;
        case 'a':
        case KEY_LEFT:
            new_x = x - 1;
            break;
        case 'd':
        case KEY_RIGHT:
            new_x = x + 1;
            break;
    }
    if (can_move(new_y, new_x, player)) {
        y = new_y;
        x = new_x;
    }
}

void game(const vector<string> art_player, const vector<string> player, const vector<string> map, const string& name, int level) {
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    int x = COLS / 2;
    int y = LINES / 4;
    int old_x = x;
    int old_y = y;
    int ch;
    int startRow = 0;
    int startCol = 0;
    
    signal(SIGWINCH, handle_resize);
    
    draw_panel(name, level);
    draw_art_player(art_player);
    draw_player(player, x, y);
    refresh();
    
    while ((ch = getch()) != 27) {  // 27 - код клавиши ESC
        if (resized) {
            endwin();
            refresh();
            clear();
            resized = false;
            draw_panel(name, level);
            draw_art_player(art_player);
        }
        
        if (ch != ERR) {
            erase_character(player, old_x, old_y);
            move_character(y, x, ch, player);
            old_x = x;
            old_y = y;
        }
        draw_art_player(art_player);
        draw_panel(name, level);
        draw_player(player, x, y);
        refresh();
    }
}

void display_menu(const vector<string>& options, int selected, const vector<string>& art_start, const vector<string>& art_help, const vector<string>& art_exit) {
    clear();
    int row, col;
    getmaxyx(stdscr, row, col);

    const vector<string>* art = nullptr;
    if (selected == 0) {
        art = &art_start;
    } else if (selected == 1) {
        art = &art_help;
    } else if (selected == 2) {
        art = &art_exit;
    }

    if (art != nullptr) {
        int art_start_row = (row - art->size()) / 2;
        for (size_t i = 0; i < art->size(); ++i) {
            int start_col = (col - (*art)[i].length()) / 2;
            if (i == art->size() - 1) {
                mvprintw(art_start_row + i, start_col, (*art)[i].c_str());
            } else {
                mvprintw(art_start_row + i, start_col, (*art)[i].c_str());
            }
        }
    }

    for (size_t i = 0; i < options.size(); ++i) {
        int start_col = (col - options[i].length()) / 2;
        if (i == selected) {
            attron(A_REVERSE);
            mvprintw(row - (options.size() - i), start_col, options[i].c_str());
            attroff(A_REVERSE);
        } else {
            mvprintw(row - (options.size() - i), start_col, options[i].c_str());
        }
    }
    refresh();
}

void main_menu(const vector<string> art_player, const vector<string> player, const vector<string> art_start, const vector<string> art_help, const vector<string> art_exit, const vector<string> map, const string& name, int level) {
    nodelay(stdscr, FALSE);
    
    vector<string> options = {
        "Start Game",
        "   Help   ",
        "   Exit   "
    };
	mousemask(ALL_MOUSE_EVENTS, NULL); // Enable mouse events
    int selected = 0;
    int ch;

    while (true) {
        display_menu(options, selected, art_start, art_help, art_exit);

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
            case KEY_UP:
                if (selected > 0) {
                    selected--;
                }
                break;
            case KEY_DOWN:
                if (selected < options.size() - 1) {
                    selected++;
                }
                break;
            case '\n':
                if (selected == 0) {
                    clear();
                    game(art_player, player, map, name, level);
                } else if (selected == options.size() - 1) {
                    endwin();
                    exit(0);
                }
                break;
        }
    }
}

void show_warning(const vector<string>& art) {
    int row, col;
    getmaxyx(stdscr, row, col);
    int start_y = (row - art.size()) / 2;
    WINDOW* win = newwin(row, col, 0, 0);
	box(win, 0, 0);
	wrefresh(win);

    for (int i = 0; i < art.size(); ++i) {
        int start_x = (col - art[i].size()) / 2;
        mvwprintw(win, start_y + i, start_x, "%s", art[i].c_str());
    }
    wrefresh(win);
    napms(1500);
    clear();
    delwin(win);
}

int main() {
	initscr();
    cbreak();
    echo();
    curs_set(0); 
    keypad(stdscr, TRUE);
    
    vector<string> art_player = read("model/knight.txt");
	vector<string> player = read("model/player.txt");
	vector<string> art_start = read("model/start.txt");
	vector<string> art_warning = read("model/warning.txt");
	vector<string> art_help = read("model/help.txt");
	vector<string> art_exit = read("model/exit.txt");
	vector<string> map = read("model/map.txt");
	Player player_stat = loadPlayerInfo("saves/player.txt");
	
    if (!art_player.empty(), !player.empty(), !art_start.empty(), !art_warning.empty(), !art_help.empty(), !art_exit.empty()) {

    } else {
        cerr << "Error: Art file is empty or could not be read." << endl;
        endwin();
        return 1;
    }
    mvprintw(1, 1, "Open full screen");
    show_warning(art_warning);
    main_menu(art_player, player, art_start, art_help, art_exit, map, player_stat.name, player_stat.level);
    endwin();
    return 0;
}
