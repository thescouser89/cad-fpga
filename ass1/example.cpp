#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include <queue>
#include <stack>
#include "graphics.h"

using namespace std;

// Callbacks for event-driven window handling.
void drawscreen (void);
void act_on_new_button_func (void (*drawscreen_ptr) (void));
void act_on_button_press (float x, float y, t_event_buttonPressed event);
void act_on_mouse_move (float x, float y);
void act_on_key_press (char c);
void draw_segment_line(int x, int y, int segment);
void draw_logicbox();
void draw_logicbox_pins();
void draw_switchbox();
void draw_segments();


// A handy delay function for the animation example
void delay (long milliseconds);

// State variables for the example showing entering lines and rubber banding
// and the new button example.
static bool line_entering_demo = false;
static bool have_rubber_line = false;
static t_point rubber_pt;         // Last point to which we rubber-banded.
static std::vector<t_point> line_pts;  // Stores the points entered by user clicks.
static int num_new_button_clicks = 0;


// You can use any coordinate system you want.
// The coordinate system below matches what you have probably seen in math
// (ie. with origin in bottom left).
// Note, that text orientation is not affected. Zero degrees will always be the normal, readable, orientation.
const t_bound_box initial_coords = t_bound_box(0,0,1000,1000);


// =========== Custom variables used to keep state =================
static size_t grid = 5;
static size_t size_grid = grid + 2;
static size_t segments_per_channel = 4;

static int max_x;
static int max_y;
static int ***visited;

static const t_point start_point = t_point(50, 50);
static const float square_width = 70;

// Used as markers for the visited array
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
        if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment)); // right
        if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment)); // bottom
        if (is_coord_valid(x - 2, y)) q->push_back(new Coord(x - 2, y, segment));         // left
    } else {
        // top segments to switchbox
        if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment)); // right
        if (is_coord_valid(x, y - 2)) q->push_back(new Coord(x, y - 2, segment));         // bottom
        if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment)); // left

        // bottom segments to switchbox
        if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment)); // right
        if (is_coord_valid(x, y + 2)) q->push_back(new Coord(x, y + 2, segment));         // top
        if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment)); // left
    }
}


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
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment - 1)); // bottom
            if (is_coord_valid(x - 2, y)) q->push_back(new Coord(x - 2, y, segment));             // left
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // top
        }
    } else {
        if (segment % 2 == 0) {
            // from the top even segment to the switchbox
            if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment));     // right
            if (is_coord_valid(x, y - 2)) q->push_back(new Coord(x, y - 2, segment));             // bottom
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment + 1)); // left
        } else {
            // from the bottom odd segment to the switchbox
            if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment - 1)); // right
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // left
            if (is_coord_valid(x, y + 2)) q->push_back(new Coord(x, y + 2, segment));             // top
        }
    }
}

void process_unidirectional_reverse(Coord* to_process, vector<Coord*>* q) {
    int x = to_process->x;
    int y = to_process->y;
    int segment = to_process->segment;

    q->clear();

    if (is_segment_horizontal(y)) {
        if (segment % 2 == 0) {
            // from the right even segment away from the switchbox
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment + 1)); // bottom
            if (is_coord_valid(x - 2, y)) q->push_back(new Coord(x - 2, y, segment));             // left
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // top
        } else {
            // from the left odd segment away from the switchbox
            if (is_coord_valid(x + 2, y)) q->push_back(new Coord(x + 2, y, segment));             // right
            if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment));     // bottom
            if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment - 1)); // top
        }
    } else {
        if (segment % 2 == 0) {
            // from the bottom even segment away from the switchbox
            if (is_coord_valid(x + 1, y + 1)) q->push_back(new Coord(x + 1, y + 1, segment + 1)); // right
            if (is_coord_valid(x - 1, y + 1)) q->push_back(new Coord(x - 1, y + 1, segment));     // left
            if (is_coord_valid(x, y + 2)) q->push_back(new Coord(x, y + 2, segment));             // top
        } else {
            // from the top odd segment away from the switchbox
            if (is_coord_valid(x + 1, y - 1)) q->push_back(new Coord(x + 1, y - 1, segment));     // right
            if (is_coord_valid(x, y - 2)) q->push_back(new Coord(x, y - 2, segment));             // bottom
            if (is_coord_valid(x - 1, y - 1)) q->push_back(new Coord(x - 1, y - 1, segment - 1)); // left
        }
    }
}

void block_connectivity_reverse(Coord* to_process, vector<Coord*>* q, bool unidirectional=false) {
    if (unidirectional) {
        process_unidirectional_reverse(to_process, q);
    } else {
        process_bidirectional(to_process, q);
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
    for (size_t i = 0; i < segments_per_channel; ++i) {
        vec->push_back(new Coord(segment_coord_x, segment_coord_y, i));
    }
}

void initialize_array(int ****array) {
    cout << max_x << " " << max_y << " " << segments_per_channel << endl;
    (*array) = new int**[max_x + 1];

    for (int i = 0; i < max_x + 1; ++i) {
        (*array)[i] = new int*[max_y + 1];

        for (int j = 0; j < max_y + 1; ++j) {
            (*array)[i][j] = new int[segments_per_channel]();
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
    for(size_t i = 0; i < vec->size(); i++) {
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

int main() {

    std::cout << "About to start graphics.\n";

    // Set the name of the window (in UTF-8), with white background.
    init_graphics("CAD Assignment 1", WHITE);
    set_visible_world(initial_coords);
    update_message("Dustin Kut Moy Cheung");

    // Pass control to the window handling routine.  It will watch for user input
    // and redraw the screen / pan / zoom / etc. the graphics in response to user
    // input or windows being moved around the screen.  We have to pass in
    // at least one callback -- the one to redraw the graphics.
    // Three other callbacks can be provided to handle mouse button presses,
    // mouse movement and keyboard button presses in the graphics area,
    // respectively. Those 3 callbacks are optional, so we can pass NULL if
    // we don't need to take any action on those events, and we do that
    // below.
    // This function will return if and when
    // the user presses the proceed button.

    event_loop(NULL, NULL, NULL, drawscreen);
    t_bound_box old_coords = get_visible_world(); // save the current view for later;

    /**** Draw an interactive still picture again.  I'm also creating one new button. ****/
    update_message("yolo");
    create_button ("Window", "0 Clicks", act_on_new_button_func); // name is UTF-8

    // Enable mouse movement (not just button presses) and key board input.
    // The appropriate callbacks will be called by event_loop.
    set_keypress_input(true);
    set_mouse_move_input(true);
    line_entering_demo = true;

    // draw the screen once before calling event loop, so the picture is correct
    // before we get user input.
    set_visible_world(old_coords); // restore saved coords -- this takes us back to where the user panned/zoomed.
    drawscreen();

    // Call event_loop again so we get interactive graphics again.
    // This time pass in all the optional callbacks so we can take
    // action on mouse buttons presses, mouse movement, and keyboard
    // key presses.
    event_loop(act_on_button_press, act_on_mouse_move, act_on_key_press, drawscreen);

    close_graphics ();
    std::cout << "Graphics closed down.\n";

    return (0);
}

void draw_segment_line(int x, int y, int segment) {

    bool is_horizontal = (y % 2 == 0);

    int start_x_posn = start_point.x;
    int start_y_posn = start_point.y;

    int x_posn = start_x_posn + (square_width * x);
    int y_posn = start_y_posn + (square_width * y);

    size_t width_per_segment = square_width / (segments_per_channel + 1);
    size_t spacing = width_per_segment * (segment + 1);

    setcolor(YELLOW);
    setlinewidth(5);

    if(is_horizontal) {
        drawline(x_posn,
                 y_posn + spacing,
                 x_posn + square_width,
                 y_posn + spacing);
    } else {
        drawline(x_posn + spacing,
                 y_posn,
                 x_posn + spacing,
                 y_posn + square_width);
    }

}

void draw_switchbox() {
    // set color of switchbox
    color_types color_switchbox = LIGHTGREY;

    t_bound_box switchbox;
    t_point next_point;

    for(size_t j = 0; j < size_grid; ++j) {

        // move up to draw another row
        next_point = start_point + t_point(0, j * 2 * square_width);
        switchbox = t_bound_box(next_point, square_width, square_width);

        for(size_t i = 0; i < size_grid; ++i) {
            setcolor(color_switchbox);
            // draw the box
            fillrect(switchbox);

            switchbox += t_point(2 * square_width, 0);
        }
    }
}

void draw_segments() {
    // draw the horizontal segments
    const t_point segment_start_point_horizontal = start_point + t_point(square_width, 0);

    t_point next_point;

    setlinestyle(SOLID);
    setcolor(BLUE);

    for (size_t j = 0; j < size_grid; ++j) {
        next_point = segment_start_point_horizontal + t_point(0, j * 2 * square_width);
        for (size_t i = 0; i < size_grid - 1; ++i) {

            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(next_point.x,
                         next_point.y + spacing,
                         next_point.x + square_width,
                         next_point.y + spacing);
            }
            next_point += t_point(2 * square_width, 0);
        }
    }

    const t_point segment_start_point_vertical = start_point + t_point(0, square_width);

    // draw the vertical segments
    for (size_t j = 0; j < size_grid - 1; ++j) {
        next_point = segment_start_point_vertical + t_point(0, j * 2 * square_width);
        for (size_t i = 0; i < size_grid; ++i) {
            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(next_point.x + spacing,
                         next_point.y,
                         next_point.x + spacing,
                         next_point.y + square_width);
            }
            next_point += t_point(2 * square_width, 0);
        }
    }
}

void draw_logicbox() {

    setcolor(BLACK);
    const t_point logic_start_point = t_point(square_width, square_width) + start_point;
    const size_t half_width = square_width / 2;
    t_point square_logicbox;

    for (size_t j = 0; j < size_grid - 1; ++j) {

        square_logicbox = t_point(0, j * 2 * square_width) + logic_start_point;

        for (size_t i = 0; i < size_grid - 1; ++i) {
            // draw the logicbox
            drawrect(square_logicbox, t_point(square_width, square_width) + square_logicbox);

            // draw the label
            setfontsize(10);
            drawtext(square_logicbox.x + half_width, square_logicbox.y + half_width,
                     std::to_string(i) + ", " + std::to_string(j), 200.0, FLT_MAX);

            // move to the right
            square_logicbox += t_point(2 * square_width, 0);
        }
    }
}

void draw_logicbox_pins() {
    setfontsize(8);
    // draw pin 1
    t_point initial_point = start_point + t_point(square_width, square_width);
    t_point current_point;
    for (size_t j = 0; j < size_grid - 1; ++j) {

        current_point = t_point(0, j * 2 * square_width) + initial_point;

        for(size_t i = 0; i < size_grid - 1; ++i) {
            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(current_point.x + (square_width / 4),
                         current_point.y,
                         current_point.x + (square_width / 4),
                         current_point.y - spacing);

                if (segment == 1) {
                drawtext(current_point.x + (square_width / 4), current_point.y + 8,
                         "1", 200.0, FLT_MAX);
                }
                drawtext(current_point.x + (square_width / 4), current_point.y - spacing,
                         "x", 200.0, FLT_MAX);
            }
            current_point += t_point(2 * square_width, 0);
        }
    }

    // draw pin 2
    initial_point = start_point + t_point(2 * square_width, 2 * square_width);
    for (size_t j = 0; j < size_grid - 1; ++j) {

        current_point = t_point(0, j * 2 * square_width) + initial_point;

        for(size_t i = 0; i < size_grid - 1; ++i) {
            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(current_point.x,
                         current_point.y - (square_width / 4),
                         current_point.x + spacing,
                         current_point.y - (square_width / 4));
                if (segment == 1) {
                    drawtext(current_point.x - 8, current_point.y - (square_width / 4),
                             "2", 200.0, FLT_MAX);
                }
                drawtext(current_point.x + spacing, current_point.y - (square_width / 4),
                         "x", 200.0, FLT_MAX);
            }
            current_point += t_point(2 * square_width, 0);

        }
    }

    // draw pin 3
    initial_point = start_point + t_point(2 * square_width, 2 * square_width);
    for (size_t j = 0; j < size_grid - 1; ++j) {
        current_point = t_point(0, j * 2 * square_width) + initial_point;
        for(size_t i = 0; i < size_grid - 1; ++i) {
            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(current_point.x - (square_width / 4),
                         current_point.y,
                         current_point.x - (square_width / 4),
                         current_point.y + spacing);
                if (segment == 1) {
                    drawtext(current_point.x - (square_width / 4), current_point.y - 8,
                             "3", 200.0, FLT_MAX);
                }
                drawtext(current_point.x - (square_width / 4), current_point.y + spacing,
                         "x", 200.0, FLT_MAX);
            }
            current_point += t_point(2 * square_width, 0);
        }
    }

    // draw pin 4
    initial_point = start_point + t_point(square_width, square_width);
    for (size_t j = 0; j < size_grid - 1; ++j) {
        current_point = t_point(0, j * 2 * square_width) + initial_point;
        for(size_t i = 0; i < size_grid - 1; ++i) {
            for (size_t segment = 1; segment <= segments_per_channel; ++segment) {
                size_t width_per_segment = square_width / (segments_per_channel + 1);
                size_t spacing = width_per_segment * segment;
                drawline(current_point.x,
                         current_point.y + (square_width / 4),
                         current_point.x - spacing,
                         current_point.y + (square_width / 4));
                if (segment == 1) {
                    drawtext(current_point.x + 8, current_point.y + (square_width / 4),
                             "4", 200.0, FLT_MAX);
                }
                drawtext(current_point.x - spacing, current_point.y + (square_width / 4),
                         "x", 200.0, FLT_MAX);
            }
            current_point += t_point(2 * square_width, 0);
        }
    }
}

void run_algorithm() {
    // =========================================================================
    // Setup
    // =========================================================================
    // setup the global variables
    max_y = (grid * 2) + 2;
    max_x = (grid * 2) + 2;
    bool unidirectional = true;

    initialize_array(&visited);

    queue<Coord*> to_process_queue;

    vector<Coord*> vec_segments;
    segments_connected_to_logicbox_coord(0, 0, 4, &vec_segments);

    // initial pins
    for (size_t i = 0; i < vec_segments.size(); i++) {
        mark_as_visited(vec_segments[i], 1);
    }
    add_content_vector_to_queue(&vec_segments, &to_process_queue);

    // now let's add the target
    vec_segments.clear();
    segments_connected_to_logicbox_coord(5, 5, 3, &vec_segments);
    // target pins
    for (size_t i = 0; i < vec_segments.size(); i++) {
        mark_as_visited(vec_segments[i], TARGET);
    }
    vec_segments.clear();

    int count = 1;

    bool found = false;
    Coord* found_coord = nullptr;
    Coord* target_coord = nullptr;
    while(!to_process_queue.empty()) {
        count++;
        int size_of_queue = to_process_queue.size();
        for(int i = 0; i < size_of_queue; i++) {
            Coord* haha = to_process_queue.front();

            to_process_queue.pop();
            block_connectivity(haha, &vec_segments, unidirectional);

            for (size_t j = 0; j < vec_segments.size(); j++) {
                Coord* hoho = vec_segments[j];
                if (is_target(hoho)) {
                    found = true;
                    found_coord = haha;
                    target_coord = hoho;
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

    stack<Coord*> soln;

    // traceback
    cout << "Traceback" << endl;

    if (target_coord != nullptr) {
        soln.push(target_coord);
    }

    while (found_coord != nullptr) {
        int count_found_coord = visited[found_coord->x][found_coord->y][found_coord->segment];

        soln.push(found_coord);
        if (count_found_coord == 1) {
            break;
        }

        block_connectivity_reverse(found_coord, &vec_segments, unidirectional);
        for (size_t i = 0; i < vec_segments.size(); i++) {
            Coord* lower = vec_segments[i];
            if (visited[lower->x][lower->y][lower->segment] == count_found_coord - 1) {
                found_coord = lower;
                break;
            }
        }
    }

    while(!soln.empty()) {
        Coord* path = soln.top();
        soln.pop();
        draw_segment_line(path->x, path->y, path->segment);
    }

    cout << "Cleanup" << endl;
    // =========================================================================
    // Cleanup
    // =========================================================================
    cleanup_array(&visited);
}

void drawscreen(void) {

    set_draw_mode(DRAW_NORMAL);
    clearscreen();

    setfontsize(10);
    setlinestyle(SOLID);
    setlinewidth(1);
    setcolor (BLACK);


    draw_switchbox();
    draw_segments();
    draw_logicbox();
    draw_logicbox_pins();

    run_algorithm();
}

void delay(long milliseconds) {
    // if you would like to use this function in your
    // own code you will need to #include <chrono> and
    // <thread>
    std::chrono::milliseconds duration(milliseconds);
    std::this_thread::sleep_for(duration);
}


void act_on_new_button_func(void (*drawscreen_ptr) (void)) {

    char old_button_name[200], new_button_name[200];
    std::cout << "You pressed the new button!\n";
    setcolor (MAGENTA);
    setfontsize (12);
    drawtext (500, 500, "You pressed the new button!", 10000.0, FLT_MAX);
    sprintf (old_button_name, "%d Clicks", num_new_button_clicks);
    num_new_button_clicks++;
    sprintf (new_button_name, "%d Clicks", num_new_button_clicks);
    change_button_text (old_button_name, new_button_name);

    // Re-draw the screen (a few squares are changing colour with time)
    drawscreen_ptr ();
}


void act_on_button_press(float x, float y, t_event_buttonPressed event) {

    /* Called whenever event_loop gets a button press in the graphics *
     * area.  Allows the user to do whatever he/she wants with button *
     * clicks.                                                        */

    std::cout << "User clicked a mouse button at coordinates ("
        << x << "," << y << ")";
    if (event.shift_pressed || event.ctrl_pressed) {
        std::cout << " with ";
        if (event.shift_pressed) {
            std::cout << "shift ";
            if (event.ctrl_pressed)
                std::cout << "and ";
        }
        if (event.ctrl_pressed)
            std::cout << "control ";
        std::cout << "pressed.";
    }
    std::cout << std::endl;

    if (line_entering_demo) {
        line_pts.push_back(t_point(x,y));
        have_rubber_line = false;

        // Redraw screen to show the new line.  Could do incrementally, but this is easier.
        drawscreen();
    }
}


void act_on_mouse_move(float x, float y) {
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // system (as defined in your call to init_world) is returned

    std::cout << "Mouse move at " << x << "," << y << ")\n";
    if (line_pts.size() > 0 ) {
        // Rubber banding to a previously entered point.
        // Go into XOR mode.  Make sure we set the linestyle etc. for xor mode, since it is
        // stored in different state than normal mode.
        set_draw_mode(DRAW_XOR);
        setlinestyle(SOLID);
        setcolor(WHITE);
        setlinewidth(1);
        int ipt = line_pts.size()-1;

        if (have_rubber_line) {
            // Erase old line.
            drawline (line_pts[ipt], rubber_pt);
        }
        have_rubber_line = true;
        rubber_pt.x = x;
        rubber_pt.y = y;
        drawline (line_pts[ipt], rubber_pt);   // Draw new line
    }
}


void act_on_key_press(char c) {
    // function to handle keyboard press event, the ASCII character is returned
    std::cout << "Key press: " << c << std::endl;
}

