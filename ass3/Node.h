//
// Created by dustin on 11/7/15.
//

#ifndef ASS3_NODE_H
#define ASS3_NODE_H

#include <memory>
#include <set>

using namespace std;

namespace Node {
    enum class Direction {Left, Right};

    const float total_x = 2000;
    const float total_y = 2000;

    const double y_displacement_from_parent = 50;
    const double x_root_position = total_x / 2;
    const double y_root_position = total_y - 30;


    class Node {
    public:
        double x_pos = 0;
        double y_pos = 0;

        set<int> nets_intersected;

        Node(int block_num, Direction dir);

        Direction get_direction();

        shared_ptr<Node> get_parent();
        void set_parent(shared_ptr<Node>);

        shared_ptr<Node> get_left();
        void set_left(shared_ptr<Node>);

        shared_ptr<Node> get_right();
        void set_right(shared_ptr<Node>);

        int get_block_num();

        int get_number_left();
        void set_number_left(int num);

        int get_number_right();
        void set_number_right(int num);

        int get_level();
        void set_level(int level);

        int calculate_lower_bound();
        int get_lower_bound();
        void set_lower_bound(int lower_bound);

        shared_ptr<Node> get_opposite_side();
        shared_ptr<Node> get_same_side();

        set<int> get_nets_intersected();
        void copy_nets_intersected(set<int> &);
        void print_info();

    private:
        unsigned int block_num = 0;
        unsigned int number_left = 0;
        unsigned int number_right = 0;
        unsigned int level = 0;
        int lower_bound = -1;

        Direction direction;
        shared_ptr<Node> parent = nullptr;

        shared_ptr<Node> left = nullptr;
        shared_ptr<Node> right = nullptr;
    };

    // Helper Functions
    shared_ptr<Node> create_root(shared_ptr<vector<int>>);
    shared_ptr<Node> get_branch(shared_ptr<Node> parent, shared_ptr<vector<int>> order, Direction dir);
}
#endif //ASS3_NODE_H
