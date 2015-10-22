#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include <list>
#include <fstream>
#include "graphics.h"
#include "analytical_placer.h"
#include "./Include/umfpack.h"
// Callbacks for event-driven window handling.
void drawscreen (void);

const t_bound_box initial_coords = t_bound_box(0,0,1005,1005);
const size_t dim_max = 1000;
const size_t grid_num = 10;
const size_t radius_block = 5;
const size_t spacing = 20;
map<int, double> net_weight;
map<int, Block*> block_num_to_block;

// useful to calculate HPWL
map<int, set<Block*>*> net_to_block;

void calculate_stopping_condition() {
    list<Block*> not_fixed;
    map<int, Block*>::iterator it;
    for (it = block_num_to_block.begin(); it != block_num_to_block.end(); it++) {
        Block *temp = it->second;
        if (!temp->fixed) {
            not_fixed.push_back(temp);
        }
    }


    int items_per_region[10 * 10];
    for (int i = 0; i < 100; i++) {
        items_per_region[i] = 0;
    }
    list<Block*>::iterator it2;
    for (it2 = not_fixed.begin(); it2 != not_fixed.end(); it2++) {
        Block *blk = *it2;
        double x = blk->x;
        double y = blk->y;
        int region_x = (int)x / 10;
        int region_y = (int)y / 10;
        items_per_region[region_x * 10 + region_y]++;
    }

    int count = 0;
    for (int i = 0; i < 100; i++) {
        if (items_per_region[i] <= 2) {
            continue;
        } else {
            count += items_per_region[i] - 2;
        }
        cout << count << endl;
    }

    cout << "Stopping condition " << (double) count / not_fixed.size() << endl;
}

class coordinate {
    public:
    double x;
    double y;
};

void randomized_fixed_block_positions(map<int, Block*> *block_num_to_block) {
    list<Block*> fixed;
    vector<coordinate> coordinates;

    map<int, Block*>::iterator it;
    for (it = block_num_to_block->begin(); it != block_num_to_block->end(); it++) {
        Block *temp = it->second;
        if (temp->fixed) {
            fixed.push_back(temp);
            coordinate temptemp;
            temptemp.x = temp->x;
            temptemp.y = temp->y;
            coordinates.push_back(temptemp);
        }
    }

    list<Block*>::iterator it2;
    for (it2 = fixed.begin(); it2 != fixed.end(); it2++) {
        Block *fixed_blk = *it2;
        int size_coord = coordinates.size();
        int index = rand() % size_coord;
        fixed_blk->x = coordinates[index].x;
        fixed_blk->y = coordinates[index].y;
        coordinates.erase(coordinates.begin() + index);
    }
}

int main(int argc, char* argv[]) {

    if (argc == 1) {
        cout << "You must specify the config file" << endl;
        return 1;
    }
    ifstream myfile(argv[1]);
    parse_input_file(&myfile, &net_weight, &block_num_to_block, &net_to_block);
    generate_matrix(&net_weight, &block_num_to_block);

    iterate_block(&block_num_to_block);
    iterate_net_to_block(&net_to_block);
    iterate_net_weight(&net_weight);

    calculate_hpwl(&net_to_block);

    std::cout << "About to start graphics.\n";

    /**************** initialize display **********************/
    // Set the name of the window (in UTF-8), with white background.
    init_graphics("Some Example Graphics", WHITE); // you could pass a t_color RGB triplet instead
    set_visible_world(initial_coords);

    update_message("Interactive graphics example.");
    event_loop(NULL, NULL, NULL, drawscreen);

    Quadrant *q = new Quadrant(0, 0, 100, 100);

    list<Block*> *not_fixed = &(q->all_blocks);
    map<int, Block*>::iterator it;
    for (it = block_num_to_block.begin(); it != block_num_to_block.end(); it++) {
        Block *temp = it->second;
        if (!temp->fixed) {
            not_fixed->push_back(temp);
        }
    }
    list<Quadrant*> q_queue;
    q_queue.push_back(q);

    while (true) {
        overlap_removal(&net_weight, &block_num_to_block, &net_to_block, &q_queue);
        // randomized_fixed_block_positions(&block_num_to_block);
        generate_matrix(&net_weight, &block_num_to_block);
        calculate_hpwl(&net_to_block);
        update_message("Parititioning once");
        set_visible_world(initial_coords);
        init_graphics("Some Example Graphics", WHITE); // you could pass a t_color RGB triplet instead
        clearscreen();
        drawscreen();
        calculate_stopping_condition();
        event_loop(NULL, NULL, NULL, drawscreen);
    }
    close_graphics ();
    std::cout << "Graphics closed down.\n";
    return (0);
}

void draw_grid(void) {
    setlinestyle(SOLID);
    setlinewidth(1);
    setcolor(BLACK);

    double grid_increment = (double) dim_max / grid_num;

    for (size_t i = 0; i <= dim_max; i += grid_increment) {
        // horizontally
        drawline(0, i, dim_max, i);
        // vertically
        drawline(i, 0, i, dim_max);
    }
    drawtext(spacing, spacing, "0, 0", 200.0, FLT_MAX);
    drawtext(dim_max + spacing, dim_max - spacing, "100, 100", 200.0, FLT_MAX);
    drawtext(spacing, dim_max - spacing, "0, 100", 200.0, FLT_MAX);
    drawtext(dim_max + spacing, spacing, "100, 0", 200.0, FLT_MAX);
}

inline double to_draw_coord(double val) {
    return val * 10;
}

void draw_block(Block* blk) {

    if (blk->block_num > 10000) return;

    double radius = radius_block;
    if (blk->fixed) {
        setcolor(GREEN);
        radius += 1;
    } else {
        setcolor(BLUE);
    }
    fillarc(to_draw_coord(blk->x), to_draw_coord(blk->y), radius_block, 0, 360);
    drawtext(to_draw_coord(blk->x) + spacing, to_draw_coord(blk->y) + spacing,
             std::to_string(blk->block_num), 200.0, FLT_MAX);
    setcolor(BLACK);
}

void draw_quadrant_block(double x, double y) {
    setcolor(GREEN);
    fillarc(to_draw_coord(x), to_draw_coord(y), radius_block + 1, 0, 360);
    setcolor(BLACK);
}

void draw_connection(Block* from, Block* to) {
    setlinestyle(DASHED);
    drawline(to_draw_coord(from->x), to_draw_coord(from->y),
             to_draw_coord(to->x), to_draw_coord(to->y));
    setlinestyle(SOLID);
}

void draw_block_connections() {
    map<int, Block*>::iterator it;

    map<int, Block*>::iterator it2;
    for (it = block_num_to_block.begin(); it != block_num_to_block.end(); it++) {
        for (it2 = block_num_to_block.begin(); it2 != block_num_to_block.end(); it2++) {
            Block *from = it->second;
            Block *to = it2->second;

            if (it->first > 10000) continue;
            if (it2->first > 10000) continue;

            if (from == to) continue;

            if (from->is_connected(to)) {
                draw_connection(from, to);
            }
        }
    }
}

void draw_blocks() {
    map<int, Block*>::iterator it;
    for (it = block_num_to_block.begin(); it != block_num_to_block.end(); it++) {
        draw_block(it->second);
    }
}

void drawscreen(void) {

        set_draw_mode (DRAW_NORMAL);
        clearscreen();

        setfontsize (10);
        // we should probably draw the lines first, then draw the blocks,
        // so that the blocks overlap the lines
        draw_grid();

        draw_block_connections();
        draw_blocks();
}
