#include "BranchAndBound.h"

#include <memory>
#include <queue>
#include <stack>
#include <vector>
#include <iostream>

#include "Netlist.h"
#include "Node.h"

using namespace std;

namespace BranchAndBound {
    void depth_first_search(shared_ptr<vector<int>> order) {
        // we already visited the root
        int nodes_visited = 1;

        int total_blocknums = order->size();
        int total_node_partitioned = total_blocknums / 2;

        int best = 50000;
        shared_ptr<Node::Node> best_node = nullptr;

        stack<shared_ptr<Node::Node>> s;

        shared_ptr<Node::Node> root = Node::create_root(order);
        s.push(root);

        while (!s.empty()) {
            shared_ptr<Node::Node> evaluate = s.top();
            s.pop();

            shared_ptr<Node::Node> left_child = Node::get_branch(evaluate,
                                                                 order,
                                                                 Node::Direction::Left);
            shared_ptr<Node::Node> right_child = Node::get_branch(evaluate,
                                                                  order,
                                                                  Node::Direction::Right);

            if (left_child == nullptr && right_child == nullptr) {
                // reached the leaf, and a solution
                if (evaluate->calculate_lower_bound() < best) {
                    best = evaluate->calculate_lower_bound();
                    best_node = evaluate;
                }
                continue;
            }

            // if solution balanced
            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (right_child->calculate_lower_bound() < best) {
                    s.push(right_child);
                }
            }

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (left_child->calculate_lower_bound() < best) {
                    s.push(left_child);
                }
            }
        }
        if (best_node != nullptr) {
            shared_ptr<Node::Node> haa = best_node;
            while (haa != nullptr) {
                cout << "Best: " << haa->get_block_num();
                if (haa->get_direction() == Node::Direction::Left) {
                    cout << "Left";
                } else {
                    cout << "Right";
                }
                cout << endl;
                haa = haa->get_parent();
            }
            cout << "Best lower bound: " << best << endl;
            cout << "Nodes visited: " << nodes_visited << endl;
        }

    }

    void breadth_first_search(shared_ptr<vector<int>> order) {

        // we already visit the root
        int nodes_visited = 1;

        int total_blocknums = order->size();
        int total_node_partitioned = total_blocknums / 2;

        int best = 500000;
        shared_ptr<Node::Node> best_node = nullptr;

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

            if (left_child == nullptr && right_child == nullptr) {
                // reached the leaf, and a solution
                if (evaluate->calculate_lower_bound() <= best) {
                    best = evaluate->calculate_lower_bound();
                    best_node = evaluate;
                }
                continue;
            }

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (left_child->calculate_lower_bound() < best) {
                    q.push(left_child);
                }
            }


            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (right_child->calculate_lower_bound() < best) {
                    q.push(right_child);
                }
            }
        }


        if (best_node != nullptr) {
            shared_ptr<Node::Node> haa = best_node;
            while (haa != nullptr) {
                cout << "Best: " << haa->get_block_num();
                if (haa->get_direction() == Node::Direction::Left) {
                    cout << "Left";
                } else {
                    cout << "Right";
                }
                cout << endl;
                haa = haa->get_parent();
            }
            cout << "Best lower bound: " << best << endl;
            cout << "Nodes visited: " << nodes_visited << endl;
        }
    }

    void lowest_bound_first(shared_ptr<vector<int>> order) {

    }

    void parallel_breadth_first_search(shared_ptr<vector<int>> order) {
        // we already visit the root
        int nodes_visited = 1;

        int total_blocknums = order->size();
        int total_node_partitioned = total_blocknums / 2;

        int best = 500000;
        shared_ptr<Node::Node> best_node = nullptr;

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

            if (left_child == nullptr && right_child == nullptr) {
                // reached the leaf, and a solution
                if (evaluate->calculate_lower_bound() <= best) {
                    best = evaluate->calculate_lower_bound();
                    best_node = evaluate;
                }
                continue;
            }

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (left_child->calculate_lower_bound() < best) {
                    q.push(left_child);
                }
            }


            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                nodes_visited++;

                if (right_child->calculate_lower_bound() < best) {
                    q.push(right_child);
                }
            }
        }


        if (best_node != nullptr) {
            shared_ptr<Node::Node> haa = best_node;
            while (haa != nullptr) {
                cout << "Best: " << haa->get_block_num();
                if (haa->get_direction() == Node::Direction::Left) {
                    cout << "Left";
                } else {
                    cout << "Right";
                }
                cout << endl;
                haa = haa->get_parent();
            }
            cout << "Best lower bound: " << best << endl;
            cout << "Nodes visited: " << nodes_visited << endl;
        }

    }
}
