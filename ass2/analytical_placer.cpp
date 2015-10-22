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
#include "./Include/umfpack.h"

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

double Block::own_weight(map<int, double> *net_weight, map<int, Block*> *block_num_to_block) {
    double own_weight = 0;
    map<int, Block*>::iterator it;
    for (it = block_num_to_block->begin(); it != block_num_to_block->end(); it++) {
        Block *other = it->second;
        if (other == this) {
            continue;
        }

        own_weight += weight(net_weight, other);
    }
    return own_weight;
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
                double w = (*it2)->own_weight(net_weight, block_num_to_block);
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

    int *Ap_a = new int[Ap.size()];
    int *Ai_a = new int[Ai.size()];
    double *Ax_a = new double[Ax.size()];
    double *x_solns_a = new double[x_solns.size()];
    double *y_solns_a = new double[y_solns.size()];

    double *x_values = new double[n];
    double *y_values = new double[n];

    copy(Ap.begin(), Ap.end(), Ap_a);
    copy(Ai.begin(), Ai.end(), Ai_a);
    copy(Ax.begin(), Ax.end(), Ax_a);
    copy(x_solns.begin(), x_solns.end(), x_solns_a);
    copy(y_solns.begin(), y_solns.end(), y_solns_a);

    matrix_solver(n, Ap_a, Ai_a, Ax_a, x_solns_a, x_values);
    matrix_solver(n, Ap_a, Ai_a, Ax_a, y_solns_a, y_values);
    for (int i = 0; i < n; i++) {
        cout << x_values[i] << ", " << y_values[i] << endl;
    }
    int count2 = 0;
    for (it2 = not_fixed.begin(); it2 != not_fixed.end(); it2++) {
        Block *blk = *it2;
        blk->set_coord(x_values[count2], y_values[count2]);
        count2++;
    }
}

void matrix_solver(int n, int *Ap, int *Ai, double *Ax, double *b, double *x) {
    double *null = (double *) NULL ;
    void *Symbolic, *Numeric ;
    (void) umfpack_di_symbolic (n, n, Ap, Ai, Ax, &Symbolic, null, null) ;
    (void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
    umfpack_di_free_symbolic (&Symbolic) ;
    (void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, null, null) ;
    umfpack_di_free_numeric (&Numeric) ;
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

Quadrant::Quadrant(double x_begin, double y_begin,
             double x_end, double y_end):
             x_begin(x_begin), y_begin(y_begin),
             x_end(x_end), y_end(y_end) {}

    double Quadrant::y_higher_left() {
        return y_begin + (double) 3 * (y_end - y_begin) / 4;
    }

    double Quadrant::y_higher_right() {
        return y_begin + (double) 3 * (y_end - y_begin) / 4;
    }

    double Quadrant::y_lower_left() {
        return y_begin + (double) (y_end - y_begin) / 4;
    }

    double Quadrant::y_lower_right() {
        return y_begin + (double) (y_end - y_begin) / 4;
    }

    double Quadrant::x_higher_left() {
        return x_begin + (double) (x_end - x_begin) / 4;
    }

    double Quadrant::x_higher_right() {
        return x_begin + (double) 3 * (x_end - x_begin) / 4;

    }

    double Quadrant::x_lower_left() {
        return x_begin + (double) (x_end - x_begin) / 4;
    }

    double Quadrant::x_lower_right() {
        return x_begin + (double) 3 * (x_end - x_begin) / 4;

    }

    double Quadrant::x_center() {
        return x_begin + (double) (x_end - x_begin) / 2;
    }

    double Quadrant::y_center() {
        return y_begin + (double) (y_end - y_begin) / 2;

    }

    void Quadrant::quadrant_process(list<Quadrant*> *quadrant_queue) {


        // lower left
        Quadrant *lower_left = new Quadrant(x_begin, y_begin, x_center(), y_center());
        // lower right
        Quadrant *lower_right = new Quadrant(x_center(), y_begin, x_end, y_center());
        // upper left
        Quadrant *upper_left = new Quadrant(x_begin, y_center(), x_center(), y_end);
        // upper right
        Quadrant *upper_right = new Quadrant(x_center(), y_center(), x_end, y_end);

        partition(&(lower_left->all_blocks),
                  &(lower_right->all_blocks),
                  &(upper_left->all_blocks),
                  &(upper_right->all_blocks));

        quadrant_queue->push_back(lower_left);
        quadrant_queue->push_back(lower_right);
        quadrant_queue->push_back(upper_left);
        quadrant_queue->push_back(upper_right);
    }

    void Quadrant::partition(list<Block*> *lower_left,
                   list<Block*> *lower_right,
                   list<Block*> *upper_left,
                   list<Block*> *upper_right) {

        list<Block*> all_blocks_cpy = all_blocks;
        int items_size = all_blocks_cpy.size();
        int quarter_items_size = items_size / 4;

        double x_lower_left_cnr = x_begin;
        double y_lower_left_cnr = y_begin;

        double x_lower_right_cnr = x_end;
        double y_lower_right_cnr = y_begin;

        double x_upper_left_cnr = x_begin;
        double y_upper_left_cnr = y_end;

        double x_upper_right_cnr = x_end;
        double y_upper_right_cnr = y_end;

        // we'll use the fact that map always iterate from lowest member
        // to highest.
        map<double, Block*> distance_to_lower_left_cnr;
        map<double, Block*> distance_to_upper_left_cnr;
        map<double, Block*> distance_to_upper_right_cnr;

        list<Block*>::iterator it;

        // assign the lower left quadrant blocks!
        for (it = all_blocks_cpy.begin(); it != all_blocks_cpy.end(); it++) {
            Block *blk = *it;
            // blk->x blk->y
            double distance_from_lower_left_cnr =
                cartesian_distance_squared(blk->x, blk->y, x_lower_left_cnr, y_lower_left_cnr);

            distance_to_lower_left_cnr[distance_from_lower_left_cnr] = blk;
        }

        map<double, Block*>::iterator it_map;
        int count = 0;
        for (it_map = distance_to_lower_left_cnr.begin(); it_map != distance_to_lower_left_cnr.end(); it_map++) {
            lower_left->push_back(it_map->second);

            all_blocks_cpy.remove(it_map->second);

            count++;
            if (count >= quarter_items_size) break;
        }

        // assign the top right quadrant blocks!
        for (it = all_blocks_cpy.begin(); it != all_blocks_cpy.end(); it++) {
            Block *blk = *it;
            // blk->x blk->y
            double distance_from_upper_right_cnr =
                cartesian_distance_squared(blk->x, blk->y, x_upper_right_cnr, y_upper_right_cnr);

            distance_to_upper_right_cnr[distance_from_upper_right_cnr] = blk;
        }

        count = 0;
        for (it_map = distance_to_upper_right_cnr.begin(); it_map != distance_to_upper_right_cnr.end(); it_map++) {
            upper_right->push_back(it_map->second);

            all_blocks_cpy.remove(it_map->second);

            count++;
            if (count >= quarter_items_size) break;
        }

        // assign the top left quadrant blocks!
        for (it = all_blocks_cpy.begin(); it != all_blocks_cpy.end(); it++) {
            Block *blk = *it;
            // blk->x blk->y
            double distance_from_upper_left_cnr =
                cartesian_distance_squared(blk->x, blk->y, x_upper_left_cnr, y_upper_left_cnr);

            distance_to_upper_left_cnr[distance_from_upper_left_cnr] = blk;
        }

        count = 0;
        for (it_map = distance_to_upper_left_cnr.begin(); it_map != distance_to_upper_left_cnr.end(); it_map++) {
            upper_left->push_back(it_map->second);

            all_blocks_cpy.remove(it_map->second);

            count++;
            if (count >= quarter_items_size) break;
        }

        // finally, bottom right will be the remaining blocks not assigned
        list<Block*>::iterator it_blk;
        for (it_blk = all_blocks_cpy.begin(); it_blk != all_blocks_cpy.end(); it_blk++) {
            lower_right->push_back(*it_blk);
        }
    }

    double Quadrant::cartesian_distance_squared(double x1, double y1, double x2, double y2) {
        double sub_x = x1 - x2;
        double sub_y = y1 - y2;
        return sub_x * sub_x + sub_y * sub_y;
    }

// overlap_removal
// initially get a new Temp, with all lists empty, and with coordinates set.
// the Temp gets processed, where the lists get populated, and 4 new Temps are
// produced and added to a queue
//
// if it passes the condition, return
// else, process the entire queue, then evaluate the condition again

static int block_count = 100000;
static int net_count = 100000;

void overlap_removal(map<int, double>* net_weight,
                     map<int, Block*> *block_num_to_block,
                     map<int, set<Block*>*> *net_to_block,
                     list<Quadrant*> *q_queue) {


    int count = 0;

    size_t queue_len = q_queue->size();
    for (size_t i = 0; i < queue_len; i++) {
        Quadrant *to_process = q_queue->front();
        q_queue->pop_front();

        to_process->quadrant_process(q_queue);
    }
    cout << endl;
    list<Quadrant*>::iterator it;
    for (it = q_queue->begin(); it != q_queue->end(); it++) {
        Quadrant *qua = *it;
        Block *quadrant_center = new Block(block_count);
        quadrant_center->fixed = true;
        block_count++;
        quadrant_center->set_coord(qua->x_center(), qua->y_center());

        list<Block*>::iterator it2;
        for (it2 = qua->all_blocks.begin(); it2 != qua->all_blocks.end(); it2++) {
            Block *blk = *it2;
            blk->add_net(net_count);
            quadrant_center->add_net(net_count);
            // insert weight
            (*net_weight)[net_count] = 150;
            (*block_num_to_block)[quadrant_center->block_num] = quadrant_center;
            set<Block*> *net_blk_set = new set<Block*>();

            net_count++;
            // no need to add it to net_to_block since we don't want to count
            // those connections in our hpwl calculation
        }
    }
}

/*
int main() {
    Quadrant *q = new Quadrant(0, 0, 100, 100);
    list<Block*> *blk = &(q->all_blocks);
    Block *first = new Block(1);
    Block *second = new Block(2);
    Block *third = new Block(3);
    Block *fourth = new Block(4);
    Block *fifth = new Block(5);

    first->set_coord(100, 90);
    second->set_coord(5, 10);
    third->set_coord(5, 50);
    fourth->set_coord(80, 50);
    fifth->set_coord(101, 101);

    blk->push_back(first);
    blk->push_back(second);
    blk->push_back(third);
    blk->push_back(fourth);
    blk->push_back(fifth);

     overlap_removal(NULL, NULL, NULL, q);
}
*/
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
