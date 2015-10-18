#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <limits>
#include "analytical_placer.h"

using namespace std;

Block::Block(int block_num): block_num(block_num) {
    fixed = false;
}

void Block::set_coord(double _x, double _y) {
    x = _x;
    y = _y;
}

void Block::add_net(int net) {
    nets.insert(net);
}

void Block::cout_data() {
    cout << "Block: " << block_num << ", Fixed?: " << fixed << " Nets: ";
    set <int>::iterator it;
    for (it = nets.begin(); it != nets.end(); it++) {
        cout << *it << ", ";
    }
    cout << "x: " << x << " y: " << y << endl;
}

double Block::own_weight(map<int, double> *net_weight) {
    double weight = 0;
    set <int>::iterator it;
    for (it = nets.begin(); it != nets.end(); it++) {
        weight += (*net_weight)[*it];
    }
    return weight;
}

double Block::weight(map<int, double> *net_weight, Block *other) {
    double weight = 0;
    set<int> intersect_net;
    find_intersect(other, &intersect_net);

    set <int>::iterator it;
    for (it = intersect_net.begin(); it != intersect_net.end(); it++) {
        weight += (*net_weight)[*it];
    }
    return weight;
}

bool Block::is_connected(Block *other) {
    set<int> intersect;
    find_intersect(other, &intersect);
    return !intersect.empty();
}

void Block::find_intersect(Block *other, set<int> *intersect) {
    set<int> *other_nets = &(other->nets);

    set_intersection(nets.begin(), nets.end(),other_nets->begin(),other_nets->end(),
                     std::inserter(*intersect,intersect->begin()));
}

/**
 * Parse the input file and extract the block/net definitions and the location
 * of the fixed blocks
 */
void parse_input_file(ifstream *input_file,
                      map<int, double>* net_weight,
                      map<int, Block*> *block_num_to_block,
                      map<int, set<Block*>*> *net_to_block) {

    parse_block_net_connections(input_file, net_weight, block_num_to_block, net_to_block);
    parse_fixed_block_positions(input_file, block_num_to_block);
}

/**
 * Parse the input file to extract the block/net definitions only
 */
void parse_block_net_connections(ifstream *input_file,
                                 map<int, double>* net_weight,
                                 map<int, Block*>* block_num_to_block,
                                 map<int, set<Block*>*> *net_to_block) {

    map<int, int> net_block_connections;

    string line;
    while (getline(*input_file, line)) {
        istringstream iss(line);

        int val;
        int count = 0;
        int block_value;
        bool finished_net_processing = false;

        while (iss >> val) {
            if (count == 0 && val == -1) {      // end of block net definition
                finished_net_processing = true;
                break;
            } else if (count == 0) {        // block value defined at count = 0
                block_value = val;
                (*block_num_to_block)[block_value] = new Block(block_value);
            } else if (val != -1) {
                // increment number of nodes connected to net
                net_block_connections[val]++;
                // add net value to block
                Block *block = (*block_num_to_block)[block_value];
                block->add_net(val);

                if ((*net_to_block)[val] == NULL) {
                    (*net_to_block)[val] = new set<Block*>;
                }

                (*net_to_block)[val]->insert(block);

            } else {
                break;
            }
            count++;
        }
        if (finished_net_processing) {
            break;
        }
    }

    // so right now net_weight just have the number of blocks connected to a net
    // what we really want to know is the weight of the net. we'll do that
    // calculation in `calculate_actual_weight`
    calculate_actual_weight(&net_block_connections, net_weight);
}

// *****************************************************************************
// TODO: maybe we can play with this later
// *****************************************************************************
/**
 * Set the weight of each net based on the clique model
 */
void calculate_actual_weight(map<int, int>* net_block_connections,
                             map<int, double>* net_weight) {

    map<int, int>::iterator it;
    for (it = net_block_connections->begin(); it != net_block_connections->end(); it++) {
        (*net_weight)[it->first] = (double) 2 / it->second;
    }
}

/**
 * Parse the input file to extract the fixed block locations only
 */
void parse_fixed_block_positions(ifstream *input_file,
                                 map<int, Block*>* block_num_to_block) {

    string line;
    while (getline(*input_file, line)) {
        istringstream iss(line);
        int val;
        int count = 0;
        bool end_processing = false;
        int block, x_val, y_val;
        while(iss >> val) {
            if (val == -1 && count == 0) {
                end_processing = true;      // finished processing fixed blocks
                break;
            }
            switch (count) {
                case 0: block = val;    // block value defined at count = 0
                        break;
                case 1: x_val = val; break;
                case 2: y_val = val; break;
                default: cout << "Something went wrong while parsing fixed block" << endl;
            }
            count++;
        }
        if (end_processing) {
            break;
        }
        // add the Coordinates to the block and set it as fixed
        (*block_num_to_block)[block]->set_coord(x_val, y_val);
        (*block_num_to_block)[block]->fixed = true;
    }
}

// *****************************************************************************
// debug functions
// *****************************************************************************
void iterate_block(map<int, Block*>* block_num_to_block) {
    map<int, Block*>::iterator it;
    cout << "*******************************" << endl;
    for (it = block_num_to_block->begin(); it != block_num_to_block->end(); it++) {
        cout << "block: " << it->first << " :: ";
        it->second->cout_data();
    }
    cout << "*******************************" << endl;
}

void iterate_net_to_block(map<int, set<Block *>*> *net_to_block) {
    map<int, set<Block*>*>::iterator it;
    for (it = net_to_block->begin(); it != net_to_block->end(); it++) {
        cout << "---------------------------" << endl;
        cout << "net: " << it->first << endl;
        set<Block*> *temp = it->second;
        set<Block*>::iterator it2;
        for (it2 = temp->begin(); it2 != temp->end(); it2++) {
            (*it2)->cout_data();
        }
        cout << "---------------------------" << endl;
    }
}

void iterate_net_weight(map<int, double>* net_weight) {
    map<int, double>::iterator it;
    cout << "===============================" << endl;
    for (it = net_weight->begin(); it != net_weight->end(); it++) {
        cout << "net: " << it->first << ", weight: " << it->second << endl;
    }
    cout << "===============================" << endl;
}

void generate_matrix(map<int, double> *net_weight,
                     map<int, Block*> *block_num_to_block) {

    set<Block*> not_fixed;
    set<Block*> fixed;

    map<int, Block*>::iterator it;
    for (it = block_num_to_block->begin(); it != block_num_to_block->end(); it++) {
        Block *temp = it->second;
        if (temp->fixed) {
            fixed.insert(temp);
        } else {
            not_fixed.insert(temp);
        }
    }

    set<Block*>::iterator it2;
    set<Block*>::iterator it3;
    for (it2 = not_fixed.begin(); it2 != not_fixed.end(); it2++) {
        for (it3 = not_fixed.begin(); it3 != not_fixed.end(); it3++) {

            if (it2 == it3) {
                cout << (*it2)->own_weight(net_weight) << " ";
            } else {
                cout << "-" << (*it2)->weight(net_weight, *it3) << " ";
            }
        }
        set<Block*>::iterator fixed_iter;
        double x_position = 0;
        double y_position = 0;
        for (fixed_iter = fixed.begin(); fixed_iter != fixed.end(); fixed_iter++) {
            x_position += (*fixed_iter)->weight(net_weight, *it2) * (*fixed_iter)->x;
            y_position += (*fixed_iter)->weight(net_weight, *it2) * (*fixed_iter)->y;
        }
        cout << " | (" << x_position << ", " << y_position << ")" << endl;
    }
}

double calculate_hpwl(map<int, set<Block*>*> *net_to_block) {

    map<int, set<Block*>*>::iterator it;

    double total_hpwl = 0;

    cout << "################################################" << endl;
    for (it = net_to_block->begin(); it != net_to_block->end(); it++) {
        int net = it->first;
        set<Block*> *block_set = it->second;

        double min_x = std::numeric_limits<double>::max();
        double min_y = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::min();
        double max_y = std::numeric_limits<double>::min();

        set<Block*>::iterator it2;
        for (it2 = block_set->begin(); it2 != block_set->end(); it2++) {
            Block* blk = *it2;
            if (blk->x < min_x) min_x = blk->x;
            if (blk->y < min_y) min_y = blk->y;
            if (blk->x > max_x) max_x = blk->x;
            if (blk->y > max_y) max_y = blk->y;
        }

        double hpwl = max_x - min_x + max_y - min_y;
        total_hpwl += hpwl;

        cout << "Net: " << net;
        cout << ":: min: (" << min_x << ", " << min_y << ") ";
        cout << " max: (" << max_x << ", " << max_y << ") ";
        cout << ":: hpwl: " << hpwl << endl;
    }
    cout << "=== Total hpwl: " << total_hpwl << " ===" << endl;
    cout << "################################################" << endl;

    return total_hpwl;
}

/*
int main(int argc, char * argv[]) {
    map<int, double> net_weight;
    map<int, Block*> block_num_to_block;

    // useful to calculate HPWL
    map<int, set<Block*>*> net_to_block;

    if (argc == 1) {
        cout << "You must specify the config file" << endl;
        return 1;
    }
    ifstream myfile(argv[1]);
    parse_input_file(&myfile, &net_weight, &block_num_to_block, &net_to_block);
    iterate_block(&block_num_to_block);
    iterate_net_weight(&net_weight);
    iterate_net_to_block(&net_to_block);

    generate_matrix(&net_weight, &block_num_to_block);
}
*/
