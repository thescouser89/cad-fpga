//
// Created by dustin on 11/7/15.
//

#ifndef ASS3_NODE_H
#define ASS3_NODE_H

#include <memory>

using namespace std;

namespace Node {
    enum class Direction {Left, Right};


    class Node {
    public:
        Node(int block_num, Direction dir);

        Direction get_direction();

        shared_ptr<Node> get_parent();
        void set_parent(shared_ptr<Node>);

        int get_block_num();

        int get_number_left();
        void set_number_left(int num);

        int get_number_right();
        void set_number_right(int num);

        int get_level();
        void set_level(int level);

        int calculate_lower_bound();
        void set_lower_bound(int lower_bound);

        void print_info();

    private:
        int block_num = 0;
        int number_left = 0;
        int number_right = 0;
        int level = 0;
        int lower_bound = -1;

        Direction direction;
        shared_ptr<Node> parent = nullptr;
    };

    // Helper Functions
    shared_ptr<Node> create_root(shared_ptr<vector<int>>);
    shared_ptr<Node> get_branch(shared_ptr<Node> parent, shared_ptr<vector<int>> order, Direction dir);
}
#endif //ASS3_NODE_H
