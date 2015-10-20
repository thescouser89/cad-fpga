#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <list>
#include <limits>
#include <queue>
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

    list<int> Ap;
    list<int> Ai;
    list<double> Ax;
    list<double> x_solns;
    list<double> y_solns;
    int count = 0;
    int col_index = 0;
    size_t n = not_fixed.size();
    // x
    for (it2 = not_fixed.begin(); it2 != not_fixed.end(); it2++) {
        // y
        Ap.push_back(count);
        int row_index = 0;
        for (it3 = not_fixed.begin(); it3 != not_fixed.end(); it3++) {
            // so we go down first, then move to the right

            if (it2 == it3) {
                double w = (*it2)->own_weight(net_weight);
                cout << w << " ";
                if (w != 0) {
                    Ax.push_back(w);
                    Ai.push_back(row_index);
                    count++;
                }
            } else {
                double w = (*it2)->weight(net_weight, *it3);
                cout << -w << " ";
                if (w != 0) {
                    Ax.push_back(-w);
                    Ai.push_back(row_index);
                    count++;
                }
            }
            row_index++;
        }

        // find the right hand-side of the matrix, "b"
        set<Block*>::iterator fixed_iter;
        double x_position = 0;
        double y_position = 0;
        for (fixed_iter = fixed.begin(); fixed_iter != fixed.end(); fixed_iter++) {
            x_position += (*fixed_iter)->weight(net_weight, *it2) * (*fixed_iter)->x;
            y_position += (*fixed_iter)->weight(net_weight, *it2) * (*fixed_iter)->y;
        }
        x_solns.push_back(x_position);
        y_solns.push_back(y_position);
        cout << " | (" << x_position << ", " << y_position << ")" << endl;

        col_index++;
    }
    Ap.push_back(count);

    list<int>::iterator i;

    cout << endl;
    cout << "n: " << n << endl;
    cout << endl;

    cout << "Ap" << endl;
    for (i = Ap.begin(); i != Ap.end(); i++) {
        cout << *i << " ";
    }
    cout << endl;
    cout << endl;

    cout << "Ai" << endl;
    for (i = Ai.begin(); i != Ai.end(); i++) {
        cout << *i << " ";
    }
    cout << endl;
    cout << endl;

    cout << "Ax" << endl;
    list<double>::iterator i2;
    for (i2 = Ax.begin(); i2 != Ax.end(); i2++) {
        cout << *i2 << " ";
    }
    cout << endl;
    cout << endl;

    cout << "x:" << endl;
    for (i2 = x_solns.begin(); i2 != x_solns.end(); i2++) {
        cout << *i2 << " ";
    }
    cout << endl;
    cout << endl;

    cout << "y:" << endl;
    for (i2 = y_solns.begin(); i2 != y_solns.end(); i2++) {
        cout << *i2 << " ";
    }
    cout << endl;
    cout << endl;
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

class Quadrant {
    public:
    list<Block*> *all_blocks;
    double x_begin, y_begin;
    double x_end, y_end;

    Quadrant(double x_begin, double y_begin,
             double x_end, double y_end,
             list<Block*> *all_blocks):
             x_begin(x_begin), y_begin(y_begin),
             x_end(x_end), y_end(y_end),
            all_blocks(all_blocks) {}

    double y_higher_left() {
        return y_begin + (double) 3 * (y_end - y_begin) / 4;
    }

    double y_higher_right() {
        return y_begin + (double) 3 * (y_end - y_begin) / 4;
    }

    double y_lower_left() {
        return y_begin + (double) (y_end - y_begin) / 4;
    }

    double y_lower_right() {
        return y_begin + (double) (y_end - y_begin) / 4;
    }

    double x_higher_left() {
        return x_begin + (double) (x_end - x_begin) / 4;
    }

    double x_higher_right() {
        return x_begin + (double) 3 * (x_end - x_begin) / 4;

    }

    double x_lower_left() {
        return x_begin + (double) (x_end - x_begin) / 4;
    }

    double x_lower_right() {
        return x_begin + (double) 3 * (x_end - x_begin) / 4;

    }

    double x_center() {
        return x_begin + (double) (x_end - x_begin) / 2;
    }

    double y_center() {
        return y_begin + (double) (y_end - y_begin) / 2;

    }

    void quadrant_process(queue<Quadrant*> *quadrant_queue) {

        // *********************************************************************
        // TODO: populate queue
        // *********************************************************************
        // TODO: initialize that list
        list<Block*> *lower_left_quadrant_blocks;
        list<Block*> *lower_right_quadrant_blocks;
        list<Block*> *higher_left_quadrant_blocks;
        list<Block*> *higher_right_quadrant_blocks;

        // lower left
        quadrant_queue->push(new Quadrant(x_begin, y_begin, x_center(), y_center(), lower_left_quadrant_blocks));

        // lower right
        quadrant_queue->push(new Quadrant(x_center(), y_begin, x_end, y_center(), lower_right_quadrant_blocks));

        // upper left
        quadrant_queue->push(new Quadrant(x_begin, y_center(), x_center(), y_end, higher_left_quadrant_blocks));

        // upper right
        quadrant_queue->push(new Quadrant(x_center(), y_center(), x_end, y_end, higher_right_quadrant_blocks));
    }

    // TODO: implement this
    void partition() {}
};

// overlap_removal
// initially get a new Temp, with all lists empty, and with coordinates set.
// the Temp gets processed, where the lists get populated, and 4 new Temps are
// produced and added to a queue
//
// if it passes the condition, return
// else, process the entire queue, then evaluate the condition again


void overlap_removal(map<int, double>* net_weight,
                     map<int, Block*> *block_num_to_block,
                     map<int, set<Block*>*> *net_to_block,
                     Quadrant *q) {

    queue<Quadrant*> q_queue;
    q_queue.push(q);

    while(true) {
        size_t queue_len = q_queue.size();
        for (int i = 0; i < queue_len; i++) {
            Quadrant *to_process = q_queue.front();
            q_queue.pop();

            cout << to_process->x_center() << ", " << to_process->y_center();
            cout << endl;
            to_process->quadrant_process(&q_queue);
        }
            cout << endl;

        // *********************************************************************
        // TODO: evaluate if this is good enough
        // if so, break from loop.
        // *********************************************************************
        cout << " >>> Processing\n" << endl;
        break;
    }
}

int main() {
     overlap_removal(NULL, NULL, NULL,
                     new Quadrant(0, 0, 100, 100, NULL));
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
