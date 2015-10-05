import Queue
grid = 2

max_dimension = (grid * 2) + 2

def logic_to_actual_coordinate(x, y):
    return ((x * 2) + 1, y * 2 + 1)


def is_segment_horizontal(x, y):
    return y % 2 == 0


def is_segment_vertical(x, y):
    return not is_segment_horizontal(x, y)

def process_unidirectional(x, y, segment):
    if is_segment_horizontal(x, y):
        if segment % 2 == 0:
            potential_segments = [(x + 2, y, segment),
                                  (x + 1, y - 1, segment),
                                  (x + 1, y + 1, segment + 1)]
        else:
            potential_segments = [(x - 2, y, segment),
                                  (x - 1, y + 1, segment),
                                  (x - 1, y - 1, segment - 1)]

    else: # vertical
        if segment % 2 == 0:
            potential_segments = [(x, y - 2, segment),
                                  (x + 1, y - 1, segment),
                                  (x - 1, y - 1, segment + 1)]
        else:
            potential_segments = [(x - 1, y + 1, segment),
                                  (x, y + 2, segment),
                                  (x + 1, y + 1, segment - 1)]


    return [(x_t, y_t, seg_t) for (x_t, y_t, seg_t) in potential_segments if (0 <= x_t <= max_dimension) and (0 <= y_t <= max_dimension)]

def process_bidirectional(x, y, segment):
    if is_segment_horizontal(x, y):
        potential_segments = [(x + 2, y, segment),     # first define potential left connections to switch
                              (x + 1, y - 1, segment),
                              (x + 1, y + 1, segment),
                              (x - 2, y, segment),     # now define potential right connections to switch
                              (x - 1, y - 1, segment),
                              (x - 1, y + 1, segment)]

    else: # vertical
        potential_segments = [(x - 1, y - 1, segment),  # vertical top to switch
                              (x, y - 2, segment),
                              (x + 1, y - 1, segment),
                              (x + 1, y + 1, segment), # vertical bottom to switch
                              (x, y + 2, segment),
                              (x - 1, y + 1, segment)]

    return [(x_t, y_t, seg_t) for (x_t, y_t, seg_t) in potential_segments if (0 <= x_t <= max_dimension) and (0 <= y_t <= max_dimension)]

def block_connectivity(x, y, segment, unidirectional=False):

    if unidirectional:
        return process_unidirectional(x, y, segment)
    else:
        return process_bidirectional(x, y, segment)


segments = 2

visited = [[[0 for k in xrange(segments)] for j in xrange(max_dimension)] for i in xrange(max_dimension)]

a  = [(2, 1, 0), (2, 1, 1)]
destination = [(5, 0, 0), (5, 0, 1)]

for (x, y, z) in a:
    visited[x][y][z] = 1

for (x, y, z) in destination:
    visited[x][y][z] = 9

q = Queue.Queue()
for item in a:
    q.put(item)

while not q.empty():
    (x, y, segment) = q.get()
    if visited[x][y][segment] == 9:
        break

    # already visited
    if visited[x][y][segment] == 2:
        break

    visited[x][y][segment] = 2
    for item in block_connectivity(x, y, segment, True):
        q.put(item)

import pprint
pprint.pprint(visited)




print process_unidirectional(3, 2, 1)
