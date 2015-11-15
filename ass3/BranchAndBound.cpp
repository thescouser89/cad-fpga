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

#include "Netlist.h"
#include "Node.h"

using namespace std;

namespace BranchAndBound {

    static shared_ptr<set<int>> get_block_set() {
        shared_ptr<set<int>> block_set(new set<int>());

        for (auto i = Netlist::BLOCK_EDGES.begin(); i != Netlist::BLOCK_EDGES.end(); i++) {
            block_set->insert(i->first);
        }

        return block_set;
    }

    bool cmp_node(pair<unsigned long, shared_ptr<set<int>>>& a, pair<unsigned long, shared_ptr<set<int>>>& b) {
        return a.first > b.first;
    }

    static shared_ptr<vector<int>> descending_number_blocks() {
        vector<pair<unsigned long, shared_ptr<set<int>>>> pairs;

        // generate a map of block_size -> block set for each netlist
        for (auto i = Netlist::NET_BLOCKS.begin(); i != Netlist::NET_BLOCKS.end(); i++) {
            shared_ptr<set<int>> blocks = i->second;
            pairs.push_back(pair<unsigned long, shared_ptr<set<int>>>(blocks->size(), blocks));
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

    void depth_first_search(shared_ptr<vector<int>> order) {
        // we already visited the root
        unsigned long long nodes_visited = 1;

        int total_blocknums = (int)order->size();
        int total_node_partitioned = total_blocknums / 2;


        shared_ptr<Node::Node> best_node = initial_solution();
        int best = initial_solution()->calculate_lower_bound();

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
        int real_loop_passed = (int)pow(2, level);
        while (!q->empty()) {
            count++;
            if (count == real_loop_passed) break;

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

            // if solution balanced
            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                nodes_visited_parallel++;

                if (right_child->calculate_lower_bound() < best_parallel) {
                    s->push(right_child);
                }
            }

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                nodes_visited_parallel++;

                if (left_child->calculate_lower_bound() < best_parallel) {
                    s->push(left_child);
                }
            }
        }
    }

    void parallel_depth_first_search(shared_ptr<vector<int>> order) {
        best_node_parallel = initial_solution();
        best_parallel = best_node_parallel->calculate_lower_bound();

        shared_ptr<queue<shared_ptr<Node::Node>>> q = shallow_breadth_first_search(order, 4);
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

        if (best_node_parallel != nullptr) {
            shared_ptr<Node::Node> haa = best_node_parallel;
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
            cout << "Best lower bound: " << best_parallel << endl;
            cout << "Nodes visited: " << nodes_visited_parallel << endl;
        }

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

            // if solution balanced
            if (!(left_child->get_number_left() > total_node_partitioned ||
                  left_child->get_number_right() > total_node_partitioned)) {

                // increment nodes visited since we'll call calculate_lower_bound here
                nodes_visited++;

                if (left_child->calculate_lower_bound() < best) {
                    q.push(left_child);
                }
            }


            if (!(right_child->get_number_left() > total_node_partitioned ||
                  right_child->get_number_right() > total_node_partitioned)) {

                // increment nodes visited since we'll call calculate_lower_bound here
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

                // if solution balanced
                if (!(left_child->get_number_left() >
                      total_node_partitioned ||
                      left_child->get_number_right() >
                      total_node_partitioned)) {

#pragma omp critical
                    nodes_visited++;

                    if (left_child->calculate_lower_bound() < best) {
#pragma omp critical
                        q.push(left_child);
                    }
                }


                if (!(right_child->get_number_left() >
                      total_node_partitioned ||
                      right_child->get_number_right() >
                      total_node_partitioned)) {

#pragma omp critical
                    nodes_visited++;

                    if (right_child->calculate_lower_bound() < best) {
#pragma omp critical
                        q.push(right_child);
                    }
                }
#pragma omp critical
                {
                    semaphore--;
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
