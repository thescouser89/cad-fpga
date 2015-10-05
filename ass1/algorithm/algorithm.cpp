#include <iostream>
#include <queue>
#include <vector>

using namespace std;

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
    return (x >= 0) && (y >= 0);
}

void process_bidirectional(Coord* to_process, queue<Coord*>* q) {
    int x = to_process->x;
    int y = to_process->y;
    int segment = to_process->segment;

    if (is_segment_horizontal(y)) {
        // left segments to switchbox
        if (is_coord_valid(x + 2, y)) q->push(new Coord(x + 2, y, segment));         // right
        if (is_coord_valid(x + 1, y - 1)) q->push(new Coord(x + 1, y - 1, segment)); // bottom
        if (is_coord_valid(x + 1, y + 1)) q->push(new Coord(x + 1, y + 1, segment)); // left

        // right segments to switchbox
        if (is_coord_valid(x - 2, y)) q->push(new Coord(x - 2, y, segment));         // left
        if (is_coord_valid(x - 1, y - 1)) q->push(new Coord(x - 1, y - 1, segment)); // bottom
        if (is_coord_valid(x - 1, y + 1)) q->push(new Coord(x - 1, y + 1, segment)); // right
    } else {
        // top segments to switchbox
        if (is_coord_valid(x - 1, y - 1)) q->push(new Coord(x - 1, y - 1, segment)); // left
        if (is_coord_valid(x, y - 2)) q->push(new Coord(x, y - 2, segment));         // bottom
        if (is_coord_valid(x + 1, y - 1)) q->push(new Coord(x + 1, y - 1, segment)); // right

        // bottom segments to switchbox
        if (is_coord_valid(x + 1, y + 1)) q->push(new Coord(x + 1, y + 1, segment)); // right
        if (is_coord_valid(x, y + 2)) q->push(new Coord(x, y + 2, segment));         // top
        if (is_coord_valid(x - 1, y + 1)) q->push(new Coord(x - 1, y + 1, segment)); // left
    }
}

// add stuff to queue
void process_unidirectional(Coord* to_process, queue<Coord*>* q) {
    int x = to_process->x;
    int y = to_process->y;
    int segment = to_process->segment;

    if (is_segment_horizontal(y)) {
        if (segment % 2 == 0) {
            // from the left even segment to the switchbox
            if (is_coord_valid(x + 2, y)) q->push(new Coord(x + 2, y, segment));             // right
            if (is_coord_valid(x + 1, y - 1)) q->push(new Coord(x + 1, y - 1, segment));     // bottom
            if (is_coord_valid(x + 1, y + 1)) q->push(new Coord(x + 1, y + 1, segment + 1)); // top
        } else {
            // from the right odd segment to the switchbox
            if (is_coord_valid(x - 2, y)) q->push(new Coord(x - 2, y, segment));             // left
            if (is_coord_valid(x - 1, y + 1)) q->push(new Coord(x - 1, y + 1, segment));     // top
            if (is_coord_valid(x - 1, y - 1)) q->push(new Coord(x - 1, y - 1, segment - 1)); // bottom
        }
    } else {
        if (segment % 2 == 0) {
            // from the top even segment to the switchbox
            if (is_coord_valid(x, y - 2)) q->push(new Coord(x, y - 2, segment));             // bottom
            if (is_coord_valid(x + 1, y - 1)) q->push(new Coord(x + 1, y - 1, segment));     // right
            if (is_coord_valid(x - 1, y - 1)) q->push(new Coord(x - 1, y - 1, segment + 1)); // left
        } else {
            // from the bottom odd segment to the switchbox
            if (is_coord_valid(x - 1, y + 1)) q->push(new Coord(x - 1, y + 1, segment));     // left
            if (is_coord_valid(x, y + 2)) q->push(new Coord(x, y + 2, segment));             // top
            if (is_coord_valid(x + 1, y + 1)) q->push(new Coord(x + 1, y + 1, segment - 1)); // right
        }
    }
}

void block_connectivity(Coord* to_process, queue<Coord*>* q, bool unidirectional=false) {
    if (unidirectional) {
        process_unidirectional(to_process, q);
    } else {
        process_bidirectional(to_process, q);
    }
}


int main(void) {
    queue<Coord*> to_process_queue;
    Coord* to_process = new Coord(3, 2, 1);
    block_connectivity(to_process, &to_process_queue, true);

    while(!to_process_queue.empty()) {
        Coord* haha = to_process_queue.front();
        to_process_queue.pop();
        cout << haha->x << " " << haha->y << " " << haha->segment << endl;
        delete haha;
    }
    return 0;
}
