#ifndef ASS3_BRANCHANDBOUND_H
#define ASS3_BRANCHANDBOUND_H

#include <memory>
#include <vector>

#include "Netlist.h"
#include "Node.h"

using namespace std;

namespace BranchAndBound {
    extern bool GRAPHICS_ON;


    shared_ptr<Node::Node> initial_solution();
    void depth_first_search(shared_ptr<vector<int>> order);
    void parallel_depth_first_search(shared_ptr<vector<int>> order);
    void breadth_first_search(shared_ptr<vector<int>> order);

    void parallel_breadth_first_search(shared_ptr<vector<int>> order);
}

#endif //ASS3_BRANCHANDBOUND_H
