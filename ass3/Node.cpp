// Usage:
// Node::create_root

// followed by Node:get_branch for the branches of root
// then evaluate the lowerbound for each branch

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>
#include "Node.h"
#include "Netlist.h"

using namespace std;

namespace Node {
    bool GRAPHICS_ON = false;

    Node::Node(int block_num, Direction dir): block_num(block_num), direction(dir) {
    }

    Direction Node::get_direction() { return this->direction; }

    shared_ptr<Node> Node::get_parent() { return this->parent; }

    void Node::set_parent(shared_ptr<Node> parent) {
        this->parent = parent;
    }

    shared_ptr<Node> Node::get_left() { return this->left; }

    void Node::set_left(shared_ptr<Node> left) {
        this->left = left;
    }

    shared_ptr<Node> Node::get_right() { return this->right; }

    void Node::set_right(shared_ptr<Node> right) {
        this->right = right;
    }

    int Node::get_block_num() { return this->block_num; }

    int Node::get_number_left() { return this->number_left; }
    void Node::set_number_left(int num) { this->number_left = num; }

    int Node::get_number_right() { return this->number_right; }
    void Node::set_number_right(int num) { this->number_right = num; }

    int Node::get_level() { return this->level; }
    void Node::set_level(int level) { this->level = level; }

    int Node::calculate_lower_bound() {

        // already calculated lower bound
        if (this->lower_bound != -1) {
            return this->lower_bound;
        }

        // not yet calculated lower bound
        shared_ptr<Node> opposite_side = this->get_opposite_side();

        while(opposite_side != nullptr) {
            set<int> difference;

            shared_ptr<set<int>> nets_crossed = \
            Netlist::nets_connected_to(this->get_block_num(), opposite_side->get_block_num());

            this->nets_intersected.insert(nets_crossed->begin(), nets_crossed->end());

            opposite_side = opposite_side->get_same_side();
        }

        this->lower_bound = this->nets_intersected.size();

        return this->lower_bound;
    }

    int Node::get_lower_bound() { return this->lower_bound; }
    void Node::set_lower_bound(int lower_bound) { this->lower_bound = lower_bound; }

    shared_ptr<Node> Node::get_opposite_side() {
        if (this->get_direction() == Direction::Left) {
            return this->get_right();
        } else {
            return this->get_left();
        }
    }

    shared_ptr<Node> Node::get_same_side() {
        if (this->get_direction() == Direction::Left) {
            return this->get_left();
        } else {
            return this->get_right();
        }
    }

    set<int> Node::get_nets_intersected() { return nets_intersected; }
    void Node::copy_nets_intersected(set<int> &set_to_copy) { this->nets_intersected = set_to_copy; }

    void Node::print_info() {
        std::cout << "-------------------" << endl;
        std::cout << "Blocknum: " << this->block_num << endl;
        if (this->direction == Direction::Left) {
            std::cout << "Direction: Left" << endl;
        } else {
            std::cout << "Direction: Right" << endl;
        }

        std::cout << "Left nodes: " << this->number_left << endl;
        std::cout << "Right nodes: " << this->number_right << endl;
        if (this->left != nullptr) {
            std::cout << "Left: " << this->left->get_block_num() << endl;
        } else {
            std::cout << "No Left" << endl;
        }
        if (this->right != nullptr) {
            std::cout << "Right: " << this->right->get_block_num() << endl;
        } else {
            std::cout << "No Right" << endl;
        }
        std::cout << "Lower Bound: " << this->get_lower_bound() << endl;
        std::cout << "-=-=-=-=-=-=-=-=-=-" << endl;
    }

    shared_ptr<Node> create_root(shared_ptr<vector<int>> order) {

        // we'll always start the root in the left direction
        Node *root = new Node((*order)[0], Direction::Left);

        root->set_level(0);
        root->set_number_left(1);
        root->set_lower_bound(0);
        root->x_pos = x_root_position;
        root->y_pos = y_root_position;

        return shared_ptr<Node>(root);
    }

    shared_ptr<Node> get_branch(shared_ptr<Node> parent, shared_ptr<vector<int>> order, Direction dir) {
        unsigned int level_parent = parent->get_level();
        unsigned int level_child = level_parent + 1;

        if (level_child >= order->size()) {
            return nullptr;
        }

        Node *child = new Node((*order)[level_child], dir);

        child->set_parent(parent);
        child->copy_nets_intersected(parent->nets_intersected);

        if (parent->get_direction() == Direction::Left) {
            child->set_left(parent);
            child->set_right(parent->get_right());
        } else {
            child->set_left(parent->get_left());
            child->set_right(parent);
        }



        shared_ptr<Node> grandparent = parent->get_parent();
        if (dir == Direction::Left) {
            child->set_number_left(parent->get_number_left() + 1);
            child->set_number_right(parent->get_number_right());

            if (grandparent == nullptr) {
                child->x_pos = parent->x_pos / 2;
            } else {
                double midpoint = parent->x_pos - grandparent->x_pos;
                if (midpoint < 0) midpoint *= -1;
                double half_midpoint  = midpoint / 2;
                child->x_pos = parent->x_pos - half_midpoint;
            }

            child->y_pos = parent->y_pos - y_displacement_from_parent;
        } else {
            child->set_number_left(parent->get_number_left());
            child->set_number_right(parent->get_number_right() + 1);

            if (grandparent == nullptr) {
                child->x_pos = parent->x_pos + parent->x_pos / 2;
            } else {
                double midpoint = parent->x_pos - grandparent->x_pos;
                if (midpoint < 0) midpoint *= -1;
                double half_midpoint  = midpoint / 2;
                child->x_pos = parent->x_pos + half_midpoint;
            }

            child->y_pos = parent->y_pos - y_displacement_from_parent;
        }

        child->set_level(level_child);
        return shared_ptr<Node>(child);
    }
}
