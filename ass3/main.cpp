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
    order->push_back(2);
    order->push_back(3);
    order->push_back(4);
    order->push_back(5);

    shared_ptr<Node::Node> root = Node::create_root(order);
    root->print_info();
    shared_ptr<Node::Node> child = Node::get_branch(root, order, Node::Direction::Right);
    child->print_info();
    child = Node::get_branch(child, order, Node::Direction::Right);
    child->print_info();
    child = Node::get_branch(child, order, Node::Direction::Left);
    child->print_info();
    child = Node::get_branch(child, order, Node::Direction::Left);
    child->print_info();

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
