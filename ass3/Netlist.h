#ifndef ASS3_NETLIST_H
#define ASS3_NETLIST_H

#include <fstream>
#include <map>
#include <memory>
#include <vector>

using namespace std;

namespace Netlist {

    // =========================================================================
    // Constant and variable definitions
    // =========================================================================
    const int MAX_BLOCK = 200;
    extern int ADJ_MATRIX[MAX_BLOCK][MAX_BLOCK];

    // count the number of edges each block is connected to
    extern map<int, int> BLOCK_EDGES;


    // =========================================================================
    // Function Definitions
    // =========================================================================

    /* parse the netlist file and populate ADJ_MATRIX */
    void parse_file(ifstream &);

    /* helper method for parse_file */
    void populate_net_to_block(map<int, shared_ptr<vector<int>>> &net_to_block, ifstream &in);
    /* helper method for parse_file */
    void insert_block_to_netlist(map<int, shared_ptr<vector<int>>> &net_to_block, int net, int block);
    /* helper method for parse_file */
    void populate_adjacency_matrix(map<int, shared_ptr<vector<int>>> &net_to_block);
}
#endif //ASS3_NETLIST_H
