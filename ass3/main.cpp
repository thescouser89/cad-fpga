#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include "graphics.h"
#include "Netlist.h"

// Callbacks for event-driven window handling.
void draw_screen(void);

const t_bound_box initial_coords = t_bound_box(0, 0, 1000, 1000);

int main(int argc, char* argv[]) {
    if (argc == 1) {
        cout << "You must specify a config file" << endl;
        return 1;
    }

//    std::cout << "About to start graphics.\n";
//
//    init_graphics("Some Example Graphics", WHITE);
//    set_visible_world(initial_coords);
//    update_message("Interactive graphics example.");
//    event_loop(NULL, NULL, NULL, draw_screen);
//    close_graphics();
//    std::cout << "Graphics closed down.\n";

    ifstream netlist_file(argv[1]);
    Netlist::parse_file(netlist_file);
    return (0);
}


void draw_screen(void) {

    set_draw_mode(DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
    clearscreen();  /* Should precede drawing for all drawscreens */

    setfontsize(10);
    setlinestyle(SOLID);
    setlinewidth(1);
    setcolor(BLACK);

    drawline(200, 120, 200, 200);
}
