#include "BranchAndBound.h"

#include <atomic>
#include <algorithm>
#include <memory>
#include <queue>
#include <stack>
#include <vector>
#include <iostream>
#include <cmath>
#include <mutex>
#include <thread>
#include <float.h>
#include <sstream>

#include "Netlist.h"
#include "Node.h"
#include "graphics.h"

using namespace std;

namespace BranchAndBound {

    /**
     * Helper method to draw stuff in graphics mode
     */
    static void draw_node(shared_ptr<Node::Node> node, bool prune, bool unbalanced) {
        stringstream ss;
        ss << node->get_block_num();
        if (node->get_direction() == Node::Direction::Left) {
            ss << "L";
        } else {
            ss << "R";
        }

        if (prune) {
            setcolor(DARKGREY);
        }
        if (unbalanced) {
            setcolor(YELLOW);
        }
        drawtext(node->x_pos, node->y_pos, ss.str(), FLT_MAX, FLT_MAX);
        setcolor(BLACK);

        shared_ptr<Node::Node> parent = node->get_parent();
        if (parent != nullptr) {
            setcolor(BLUE);
            drawline(node->x_pos, node->y_pos + 10, parent->x_pos, parent->y_pos - 10);
            setcolor(BLACK);
        }
    }

    // *************************************************************************
    // Initial Solution
    // *************************************************************************

    static shared_ptr<set<int>> get_block_set() {
        shared_ptr<set<int>> block_set(new set<int>());

        for (auto i = Netlist::BLOCK_EDGES.begin(); i != Netlist::BLOCK_EDGES.end(); i++) {
            block_set->insert(i->first);
        }

        return block_set;
    }

    bool cmp_node(const pair<int, shared_ptr<set<int>>>& a, const pair<int, shared_ptr<set<int>>>& b) {
        return a.first > b.first;
    }

    static shared_ptr<vector<int>> descending_number_blocks() {
        vector<pair<int, shared_ptr<set<int>>>> pairs;

        // generate a map of block_size -> block set for each netlist
        for (auto i = Netlist::NET_BLOCKS.begin(); i != Netlist::NET_BLOCKS.end(); i++) {
            shared_ptr<set<int>> blocks = i->second;
            pairs.push_back(pair<int, shared_ptr<set<int>>>((int)blocks->size(), blocks));
        }

        sort(pairs.begin(), pairs.end(), cmp_node);

        shared_ptr<vector<int>> to_return(new vector<int>());
        set<int> blocks_in_vector;

        for (auto i = pairs.begin(); i != pairs.end(); i++) {
            shared_ptr<set<int>> blocks = i->second;
            for (auto j = blocks->begin(); j != blocks->end(); j++) {

                // if *j block not in vector
                if (blocks_in_vector.find(*j) == blocks_in_vector.end()) {
                    to_return->push_back(*j);
                    blocks_in_vector.insert(*j);
                }
            }
        }
        reverse(to_return->begin(), to_return->end());

        return to_return;
    }

    /**
     * Generate an initial solution for Branch and Bound
     *
     * It's pretty awful :(
     */
    shared_ptr<Node::Node> initial_solution() {
        shared_ptr<set<int>> block_set = get_block_set();

        int block_left_side = int(block_set->size() / 2);
        shared_ptr<vector<int>> tmp = descending_number_blocks();

        Node::Direction dir = Node::Direction::Left;
        shared_ptr<Node::Node> parent = nullptr;
        shared_ptr<Node::Node> n;

        for (auto i = tmp->begin(); i != tmp->end(); i++) {
            n = shared_ptr<Node::Node>(new Node::Node(*i, dir));
            n->set_parent(parent);

            if (parent == nullptr) n->set_number_left(1);

            if (parent != nullptr) {
                n->copy_nets_intersected(parent->nets_intersected);

                if (parent->get_direction() == Node::Direction::Left) {
                    n->set_left(parent);
                    n->set_right(parent->get_right());
                } else {
                    n->set_left(parent->get_left());
                    n->set_right(parent);
                }

                if (dir == Node::Direction::Left) {
                    n->set_number_left(parent->get_number_left() + 1);
                    n->set_number_right(parent->get_number_right());
                } else {
                    n->set_number_left(parent->get_number_left());
                    n->set_number_right(parent->get_number_right() + 1);
                }
            }
            n->calculate_lower_bound();
            parent = n;

            if (n->get_number_left() == block_left_side) {
                dir = Node::Direction::Right;
            }
        }
        return n;
    }
    // *************************************************************************
    // End :: Initial Solution
    // *************************************************************************

    static void print_best_node(shared_ptr<Node::Node> best_node, int nodes_visited) {
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
            cout << "Best lower bound: " << best_node->calculate_lower_bound() << endl;
            cout << "Nodes visited: " << nodes_visited << endl;
        } else {
            cout << "No solution found :(" << endl;
        }
    }

    void evaluate_child(shared_ptr<Node::Node> child, queue<shared_ptr<Node::Node>>& s, int best, int total_node_partitioned) {
        // if solution balanced
        if (!(child->get_number_left() > total_node_partitioned ||
              child->get_number_right() > total_node_partitioned)) {

            if (child->calculate_lower_bound() < best) {
                s.push(child);
            }
        }
    }

    void evaluate_child(shared_ptr<Node::Node> child, stack<shared_ptr<Node::Node>>& s, int best, int total_node_partitioned) {
        // if solution balanced
        if (!(child->get_number_left() > total_node_partitioned ||
              child->get_number_right() > total_node_partitioned)) {

            if (child->calculate_lower_bound() < best) {
                s.push(child);
                if (Node::GRAPHICS_ON) draw_node(child, false, false);
            } else {
                if (Node::GRAPHICS_ON) draw_node(child, true, false);
            }
        } else {
            if (Node::GRAPHICS_ON) draw_node(child, false, true);
        }
    }

    void depth_first_search(shared_ptr<vector<int>> order) {
        // we already visited the root
        unsigned long long nodes_visited = 1;

        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;


        shared_ptr<Node::Node> best_node = initial_solution();
        int best = initial_solution()->calculate_lower_bound();

        stack<shared_ptr<Node::Node>> s;

        shared_ptr<Node::Node> root = Node::create_root(order);

        if (Node::GRAPHICS_ON) draw_node(root, false, false);
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

            nodes_visited++;
            evaluate_child(right_child, s, best, total_node_partitioned);
            nodes_visited++;
            evaluate_child(left_child, s, best, total_node_partitioned);
        }
        print_best_node(best_node, nodes_visited);
    }

    static shared_ptr<queue<shared_ptr<Node::Node>>> shallow_breadth_first_search(shared_ptr<vector<int>> order, int level) {

        // we already visit the root
        int nodes_visited = 1;

        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;

        shared_ptr<Node::Node> best_node = initial_solution();
        int best = best_node->calculate_lower_bound();

        shared_ptr<queue<shared_ptr<Node::Node>>> q(new queue<shared_ptr<Node::Node>>());

        shared_ptr<Node::Node> root = Node::create_root(order);
        q->push(root);

        int count= 0;
        int loops_to_process = (int)pow(2, level);
        while (!q->empty()) {
            count++;
            if (count == loops_to_process) break;

            shared_ptr<Node::Node> evaluate = q->front();
            q->pop();

            shared_ptr<Node::Node> left_child = Node::get_branch(evaluate,
                                                                 order,
                                                                 Node::Direction::Left);
            shared_ptr<Node::Node> right_child = Node::get_branch(evaluate,
                                                                  order,
                                                                  Node::Direction::Right);

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                // increment nodes visited since we'll call calculate_lower_bound here
                nodes_visited++;

                if (left_child->calculate_lower_bound() < best) {
                    q->push(left_child);
                }
            }

            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                // increment nodes visited since we'll call calculate_lower_bound here
                nodes_visited++;

                if (right_child->calculate_lower_bound() < best) {
                    q->push(right_child);
                }
            }
        }
        cout << "Queue size: " << q->size() << endl;
        return q;
    }

    static atomic_uint nodes_visited_parallel;
    static atomic_int best_parallel;
    static shared_ptr<Node::Node> best_node_parallel;
    static mutex update_best;

    void sub_depth_first_search_routine(shared_ptr<vector<int>> order, shared_ptr<stack<shared_ptr<Node::Node>>> s) {
        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;

        while (!s->empty()) {
            shared_ptr<Node::Node> evaluate = s->top();
            s->pop();

            shared_ptr<Node::Node> left_child = Node::get_branch(evaluate,
                                                                 order,
                                                                 Node::Direction::Left);
            shared_ptr<Node::Node> right_child = Node::get_branch(evaluate,
                                                                  order,
                                                                  Node::Direction::Right);

            if (left_child == nullptr && right_child == nullptr) {
                // reached the leaf, and a solution
                update_best.lock();
                if (evaluate->calculate_lower_bound() < best_parallel) {
                    best_parallel = evaluate->calculate_lower_bound();
                    best_node_parallel = evaluate;
                }
                update_best.unlock();
                continue;
            }

            nodes_visited_parallel++;
            // if solution balanced
            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {


                if (right_child->calculate_lower_bound() < best_parallel) {
                    s->push(right_child);
                }
            }

            nodes_visited_parallel++;
            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                if (left_child->calculate_lower_bound() < best_parallel) {
                    s->push(left_child);
                }
            }
        }
    }

    void parallel_depth_first_search(shared_ptr<vector<int>> order) {
        best_node_parallel = initial_solution();
        best_parallel = best_node_parallel->calculate_lower_bound();

        shared_ptr<queue<shared_ptr<Node::Node>>> q = shallow_breadth_first_search(order, 5);
        vector<shared_ptr<thread>> threads;

        while(!q->empty()) {
            shared_ptr<stack<shared_ptr<Node::Node>>> s(new stack<shared_ptr<Node::Node>>());
            s->push(q->front());
            q->pop();
            shared_ptr<thread> t(new thread(sub_depth_first_search_routine, order, s));
            threads.push_back(t);
        }

        for (auto i = threads.begin(); i != threads.end(); i++) {
            i->get()->join();
        }
        print_best_node(best_node_parallel, nodes_visited_parallel);
    }


    void breadth_first_search(shared_ptr<vector<int>> order) {

        // we already visit the root
        int nodes_visited = 1;

        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;

        shared_ptr<Node::Node> best_node = initial_solution();
        int best = best_node->calculate_lower_bound();

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

            nodes_visited++;
            evaluate_child(left_child, q, best, total_node_partitioned);
            nodes_visited++;
            evaluate_child(right_child, q, best, total_node_partitioned);
        }
        print_best_node(best_node, nodes_visited);
    }


    void parallel_breadth_first_search(shared_ptr<vector<int>> order) {
        // we already visit the root
        int nodes_visited = 1;

        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;

        shared_ptr<Node::Node> best_node = initial_solution();
        int best = best_node->calculate_lower_bound();

        queue<shared_ptr<Node::Node>> q;

        shared_ptr<Node::Node> root = Node::create_root(order);
        q.push(root);
        int semaphore = 0;
        #pragma omp parallel
        while (semaphore > 0 || !q.empty()) {
            shared_ptr<Node::Node> evaluate;
            bool processed = false;

            #pragma omp critical
            {
                if (!q.empty()) {
                    evaluate = q.front();
                    q.pop();
                    semaphore++;
                    processed = true;
                }
            }

            if (processed) {
                shared_ptr<Node::Node> left_child = Node::get_branch(
                        evaluate,
                        order,
                        Node::Direction::Left);
                shared_ptr<Node::Node> right_child = Node::get_branch(
                        evaluate,
                        order,
                        Node::Direction::Right);

                if (left_child == nullptr && right_child == nullptr) {
                    // reached the leaf, and a solution
                    #pragma omp critical
                    {
                        if (evaluate->calculate_lower_bound() <= best) {
                            best = evaluate->calculate_lower_bound();
                            best_node = evaluate;
                        }
                        semaphore--;
                    }
                    continue;
                }

                #pragma omp critical
                {
                    nodes_visited++;
                    evaluate_child(left_child, q, best, total_node_partitioned);
                    nodes_visited++;
                    evaluate_child(right_child, q, best, total_node_partitioned);

                    semaphore--;
                }
            }
        }
        print_best_node(best_node, nodes_visited);
    }
}
