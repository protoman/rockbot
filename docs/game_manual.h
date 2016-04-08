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
        clear_text();
    }

    void set_line_text(int n, std::string text) {
        sprintf(line[n], "%s", text.c_str());
    }

    void add_line_text(std::string text) {
        if (line_number < PAGE_LINES-1) {
            sprintf(line[line_number], "%s", text.c_str());
            line_number++;
        }
    }

    void clear_text() {
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
