#include <iostream>
#include <queue>
#include <vector>

using namespace std;

// =============================================================================
// Global Variables
// =============================================================================
int grid;
int max_dimension;
int max_y;
int max_x;
int segments_W;
int ***visited;


const int TARGET = 999999;
const int ORIGIN = 0;
const int UNAVAILABLE = 888888;

class Coord {
    public:
        int x;
        int y;
        int segment;
        Coord(int x, int y, int segment) {
            this->x = x;
            this->y = y;
            this->segment = segment;
        }

};

int x_logic_to_actual_coord(int x) {
    return (x * 2) + 1;
}

int y_logic_to_actual_coord(int y) {
    return (y * 2) + 1;
}

bool is_segment_horizontal(int y) {
    return y % 2 == 0;
}

bool is_segment_vertical(int y) {
    return !is_segment_horizontal(y);
}

// TODO: need to add upper bound limit
bool is_coord_valid(int x, int y) {
    return (x >= 0) && (x <= max_x) && (y >= 0) && (y <= max_y);
}

void process_bidirectional(Coord* to_process, vector<Coord*>* q) {
    int x = to_process->x;
    int y = to_process->y;
    int segment = to_process->segment;

    q->clear();

    if (is_segment_horizontal(y)) {
        // left segments to switchbox
        if (is_coord_valid(x + 2, y)) q->push_back(new Coord(x + 2, y, segment));         // right
        if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment)); // bottom
        if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment)); // left

        // right segments to switchbox
        if (is_coord_valid(x - 2, y)) q->push_back(new Coord(x - 2, y, segment));         // left
        if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment)); // bottom
        if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment)); // right
    } else {
        // top segments to switchbox
        if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment)); // left
        if (is_coord_valid(x, y - 2)) q->push_back(new Coord(x, y - 2, segment));         // bottom
        if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment)); // right

        // bottom segments to switchbox
        if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment)); // right
        if (is_coord_valid(x, y + 2)) q->push_back(new Coord(x, y + 2, segment));         // top
        if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment)); // left
    }
}

// add stuff to queue
void process_unidirectional(Coord* to_process, vector<Coord*>* q) {
    int x = to_process->x;
    int y = to_process->y;
    int segment = to_process->segment;

    q->clear();

    if (is_segment_horizontal(y)) {
        if (segment % 2 == 0) {
            // from the left even segment to the switchbox
            if (is_coord_valid(x + 2, y)) q->push_back(new Coord(x + 2, y, segment));             // right
            if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment));     // bottom
            if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment + 1)); // top
        } else {
            // from the right odd segment to the switchbox
            if (is_coord_valid(x - 2, y)) q->push_back(new Coord(x - 2, y, segment));             // left
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // top
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment - 1)); // bottom
        }
    } else {
        if (segment % 2 == 0) {
            // from the top even segment to the switchbox
            if (is_coord_valid(x, y - 2)) q->push_back(new Coord(x, y - 2, segment));             // bottom
            if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment));     // right
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment + 1)); // left
        } else {
            // from the bottom odd segment to the switchbox
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // left
            if (is_coord_valid(x, y + 2)) q->push_back(new Coord(x, y + 2, segment));             // top
            if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment - 1)); // right
        }
    }
}



void block_connectivity(Coord* to_process, vector<Coord*>* q, bool unidirectional=false) {
    if (unidirectional) {
        process_unidirectional(to_process, q);
    } else {
        process_bidirectional(to_process, q);
    }
}

/**
 * Here x, and y are the coordinates of the logicbox, not its real coordinates
 */
void segments_connected_to_logicbox_coord(int x, int y, int pin, vector<Coord*>* vec) {
    int actual_x = x_logic_to_actual_coord(x);
    int actual_y = y_logic_to_actual_coord(y);

    int segment_coord_x = 0;
    int segment_coord_y = 0;

    switch(pin) {
        case 1:
            segment_coord_x = actual_x;
            segment_coord_y = actual_y - 1;
            break;
        case 2:
            segment_coord_x = actual_x + 1;
            segment_coord_y = actual_y;
            break;
        case 3:
            segment_coord_x = actual_x;
            segment_coord_y = actual_y + 1;
            break;
        case 4:
            segment_coord_x = actual_x - 1;
            segment_coord_y = actual_y;
            break;
        default:
            break;
    }

    // make sure the vector is empty
    vec->clear();

    // add the segments to the vector
    for (int i = 0; i < segments_W; ++i) {
        vec->push_back(new Coord(segment_coord_x, segment_coord_y, i));
    }
}

void initialize_array(int ****array) {
    *array = new int**[max_x + 1];

    for (int i = 0; i < max_x + 1; ++i) {
        (*array)[i] = new int*[max_y + 1];

        for (int j = 0; j < max_y + 1; ++j) {
            (*array)[i][j] = new int[segments_W];
        }
    }
}

void cleanup_array(int ****array) {
    for (int i = 0; i < max_x + 1; ++i) {
        for (int j = 0; j < max_y + 1; ++j) {
            delete [] (*array)[i][j];
        }
        delete [] (*array)[i];
    }
    delete [] (*array);
}

void add_content_vector_to_queue(vector<Coord*>* vec, queue<Coord*>* q) {
    for(int i = 0; i < vec->size(); i++) {
        Coord* test = (*vec)[i];
        q->push(test);
    }

}

bool is_already_visited(Coord* coord) {
    int x = coord->x;
    int y = coord->y;
    int segment = coord->segment;

    return visited[x][y][segment] != 0;
}

bool is_target(Coord* coord) {
    int x = coord->x;
    int y = coord->y;
    int segment = coord->segment;

    return visited[x][y][segment] == TARGET;
}

void mark_as_visited(Coord* coord, int num) {
    int x = coord->x;
    int y = coord->y;
    int segment = coord->segment;
    visited[x][y][segment] = num;
}

int main(void) {

    // =========================================================================
    // Setup
    // =========================================================================
    // setup the global variables
    grid = 3;
    max_dimension = (grid * 2) + 2;

    max_y = max_dimension;
    max_x = max_dimension;

    segments_W = 4;


    initialize_array(&visited);

    queue<Coord*> to_process_queue;

    vector<Coord*> vec_segments;
    segments_connected_to_logicbox_coord(0, 0, 1, &vec_segments);

    // initial pins
    for (int i = 0; i < vec_segments.size(); i++) {
        mark_as_visited(vec_segments[i], 1);
    }
    add_content_vector_to_queue(&vec_segments, &to_process_queue);

    // now let's add the target
    vec_segments.clear();
    segments_connected_to_logicbox_coord(1, 1, 3, &vec_segments);
    // target pins
    for (int i = 0; i < vec_segments.size(); i++) {
        mark_as_visited(vec_segments[i], TARGET);
    }
    vec_segments.clear();

    int count = 1;

    bool found = false;
    Coord* found_coord;
    while(!to_process_queue.empty()) {
        count++;
        int size_of_queue = to_process_queue.size();

        for(int i = 0; i < size_of_queue; i++) {
            Coord* haha = to_process_queue.front();

            to_process_queue.pop();
            block_connectivity(haha, &vec_segments);

            for (int j = 0; j < vec_segments.size(); j++) {
                Coord* hoho = vec_segments[j];
                if (is_target(hoho)) {
                    found = true;
                    found_coord = haha;
                    break;
                } else if (is_already_visited(hoho)) {
                    continue;
                } else {
                    to_process_queue.push(hoho);
                    mark_as_visited(hoho, count);
                }
            }
            if (found) {
                break;
            }
            delete haha;
        }
        if (found) {
            break;
        }
    }

    cout << "Cleanup" << endl;
    // =========================================================================
    // Cleanup
    // =========================================================================
    cleanup_array(&visited);

    return 0;
}
