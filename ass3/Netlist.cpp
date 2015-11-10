#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include "Netlist.h"

using namespace std;

namespace Netlist {

    map<int, shared_ptr<set<int>>> BLOCK_NETS;
    map<int, int> BLOCK_EDGES;

    void parse_file(ifstream &in) {
        populate_block_nets(in);
    }

    shared_ptr<set<int>> nets_connected_to(int i, int j) {
        shared_ptr<set<int>> from = BLOCK_NETS[i];
        shared_ptr<set<int>> to = BLOCK_NETS[j];

        shared_ptr<set<int>> intersection(new set<int>());

        set_intersection(from->begin(), from->end(), to->begin(), to->end(),
                         inserter(*intersection, intersection->begin()));

        return intersection;
    }

    void populate_block_nets(ifstream &in) {
        string line;
        int val;

        while (getline(in, line)) {

            istringstream iss(line);
            int count = 0;
            int block;

            while (iss >> val) {
                // -1 signals stop reading!
                if (val == -1) continue;

                ++count;
                // first element in the line, this is the block number
                if (count == 1) {
                    block = val;
                } else {
                    insert_block_to_netlist(val, block);
                }
            }
        }
    }

    void insert_block_to_netlist(int val, int block) {
        shared_ptr<set<int>> nets = BLOCK_NETS[block];

        // create set if it doesn't exist
        if (nets == nullptr) {
            shared_ptr<set<int>> new_nets(new set<int>());
            BLOCK_NETS[block] = new_nets;
            nets = new_nets;
        }
        nets->insert(val);
    }
}
