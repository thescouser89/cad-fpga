#ifndef ASS3_NETLIST_H
#define ASS3_NETLIST_H

#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <set>

using namespace std;

namespace Netlist {

    // =========================================================================
    // Constant and variable definitions
    // =========================================================================
    extern map<int, shared_ptr<set<int>>> BLOCK_NETS;

    extern map<int, shared_ptr<set<int>>> NET_BLOCKS;

    // count the number of edges each block is connected to
    extern map<int, int> BLOCK_EDGES;


    // =========================================================================
    // Function Definitions
    // =========================================================================
    int number_of_blocks();

    shared_ptr<set<int>> nets_connected_to(int i, int j);

    /* parse the netlist file and populate ADJ_MATRIX */
    void parse_file(ifstream &);

    void populate_block_nets(ifstream &);

    /* helper method for parse_file */
    void insert_block_to_netlist(int net, int block);
}
#endif //ASS3_NETLIST_H
