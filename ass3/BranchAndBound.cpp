#include "BranchAndBound.h"

#include <memory>
#include <queue>
#include <vector>

#include "Netlist.h"
#include "Node.h"

namespace BranchAndBound {
    void depth_first_search(shared_ptr<vector<int>> order) {

    }

    void breadth_first_search(shared_ptr<vector<int>> order) {

        int best = 50000000000;

        queue<shared_ptr<Node::Node>> q;

        shared_ptr<Node::Node> root = Node::create_root(order);
        q.push(root);

        while (!q.empty()) {
            shared_ptr<Node::Node> evaluate = q.front();
            q.pop();

            shared_ptr<Node::Node> left_child = Node::get_branch(evaluate,
                                                                 order,
                                                                 Node::Direction::Left);
            shared_ptr<Node::Node> right_child = Node::get_branch(evaluate,
                                                                  order,
                                                                  Node::Direction::Right);
            if (left_child->calculate_lower_bound() < best) {
                // TODO: still have to evaluate if solution is balanced
                q.push(left_child);
            }

            if (right_child->calculate_lower_bound() < best) {
                q.push(right_child);
            }
        }
    }

    void lowest_bound_first(shared_ptr<vector<int>> order) {

    }

    void parallel_breadth_first_search(shared_ptr<vector<int>> order) {

    }
}
