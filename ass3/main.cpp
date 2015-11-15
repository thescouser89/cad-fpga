#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include <map>
#include <bits/stl_algo.h>
#include "graphics.h"
#include "Netlist.h"
#include "Node.h"
#include "BranchAndBound.h"

// Callbacks for event-driven window handling.
void draw_screen(void);

const t_bound_box initial_coords = t_bound_box(0, 0, 1000, 1000);


/**
 * Ordered from block that has highest number of edges/nets to block that have
 * least number of edges/nets
 */

bool cmp_pair(const pair<int, int>& a, const pair<int, int>& b) {
    return a.second < b.second;
}

shared_ptr<vector<int>> get_order_evaluation() {
    vector<pair<int, int>> pairs;

    for (auto itr = Netlist::BLOCK_EDGES.begin(); itr != Netlist::BLOCK_EDGES.end(); ++itr)
        pairs.push_back(*itr);

    // sort based on edges
    sort(pairs.begin(), pairs.end(), cmp_pair);

    shared_ptr<vector<int>> order(new vector<int>());

    // order is created with block with highest number of edges to least number
    for (auto i = pairs.begin(); i != pairs.end(); i++) {
        order->insert(order->begin(), i->first);
    }

    return order;
}


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

    shared_ptr<vector<int>> order = get_order_evaluation();
    shared_ptr<Node::Node> n = BranchAndBound::initial_solution();
    cout << "Best initial Solution: " << n->calculate_lower_bound() << endl;


//    BranchAndBound::breadth_first_search(order);
//    BranchAndBound::depth_first_search(order);
    BranchAndBound::parallel_depth_first_search(order);
//    BranchAndBound::parallel_breadth_first_search(order);
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
