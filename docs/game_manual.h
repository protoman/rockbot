#ifndef GAME_MANUAL_H
#define GAME_MANUAL_H

#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>


#define PAGE_LINES 22

struct st_manual_page {
    char line[PAGE_LINES][50];
    int line_number;

    st_manual_page() {
        clear();
    }

    set_line(int n, std::string text) {
        sprintf(line[n], "%s", text.c_str());
    }

    add_line(std::string text) {
        if (line_number < PAGE_LINES-1) {
            sprintf(line[line_number], "%s", text.c_str());
            line_number++;
        }
    }

    clear() {
        for (int i=0; i<PAGE_LINES; i++) {
            line[i][0] = '\0';
        }
        line_number = 0;
    }
};

class game_manual
{
public:
    game_manual();
    void execute();

private:
    void generate_pages();
    void draw_page(int page_n);

private:
    std::vector<st_manual_page> page_list;
};

#endif // GAME_MANUAL_H
