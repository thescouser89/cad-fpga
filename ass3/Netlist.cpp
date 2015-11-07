#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include "Netlist.h"

using namespace std;

namespace Netlist {

    int ADJ_MATRIX[MAX_BLOCK][MAX_BLOCK];
    map<int, int> BLOCK_EDGES;

    void connect(int i, int j) {
        // increment the number of edges to block i and j
        BLOCK_EDGES[i]++;
        BLOCK_EDGES[j]++;

        // increment the number of edges from i to j
        ++ADJ_MATRIX[i][j];
        ++ADJ_MATRIX[j][i];
    }

    bool is_connected(int i, int j) {

        if (i == j) return true;

        return ADJ_MATRIX[i][j] != 0;
    }

    int num_edges(int i, int j) {
        return ADJ_MATRIX[i][j];
    }

    void parse_file(ifstream &in) {
        map<int, shared_ptr<vector<int>>> net_to_block;
        populate_net_to_block(net_to_block, in);
        populate_adjacency_matrix(net_to_block);
    }

    void populate_net_to_block(map<int, shared_ptr<vector<int>>> &net_to_block, ifstream &in) {
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
                    insert_block_to_netlist(net_to_block, val, block);
                }
            }
        }
    }

    void insert_block_to_netlist(map<int, shared_ptr<vector<int>>> &net_to_block, int net, int block) {
        if (net_to_block[net] == nullptr) {
            net_to_block[net] = shared_ptr<vector<int>>(new vector<int>());
        }
        net_to_block[net]->push_back(block);
    }

    void populate_adjacency_matrix(map<int, shared_ptr<vector<int>>> &net_to_block) {
        for (auto it = net_to_block.begin(); it != net_to_block.end(); ++it) {

            auto blocks = it->second;

            // if there are less than 2 blocks to connect in the netlist, nothing to connect!
            if (blocks->size() < 2) continue;

            for (size_t i = 0; i < blocks->size() - 1; ++i) {
                for (size_t j = i + 1; j < blocks->size(); ++j) {
                    connect((*blocks)[i], (*blocks)[j]);
                }
            }
        }
    }
}
