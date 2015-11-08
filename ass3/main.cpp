#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include "graphics.h"
#include "Netlist.h"
#include "Node.h"
#include "BranchAndBound.h"

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

    for (int i = 1; i <= 30; i++) {
        for (int j = 1; j <= 30; j++) {
            cout << Netlist::num_edges(i, j) << " ";
        }
        cout << endl;
    }
    shared_ptr<vector<int>> order(new vector<int>());
    order->push_back(1);
    order->push_back(3);
    order->push_back(6);
    order->push_back(11);
    order->push_back(10);
    order->push_back(14);
    order->push_back(15);
    order->push_back(13);
    order->push_back(2);
    order->push_back(4);
    order->push_back(5);
    order->push_back(7);
    order->push_back(8);
    order->push_back(9);
    order->push_back(12);
    order->push_back(16);
//    order->push_back(17);
//    order->push_back(18);
//    order->push_back(19);
//    order->push_back(20);
//    order->push_back(21);
//    order->push_back(22);
//    order->push_back(23);
//    order->push_back(24);
//    order->push_back(25);
//    order->push_back(26);
//    order->push_back(27);
//    order->push_back(28);
//    order->push_back(29);
//    order->push_back(30);


//    BranchAndBound::breadth_first_search(order);
    BranchAndBound::depth_first_search(order);
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
