#include <set>
#include <map>

using namespace std;

class Block {
    public:
    int block_num;
    bool fixed;
    double x;
    double y;
    set<int> nets;

    public:
    Block(int block_num);
    void set_coord(double _x, double _y);
    void add_net(int net);

    void cout_data();

    double own_weight(map<int, double> *net_weight);
    double weight(map<int, double> *net_weight, Block *other);
    bool is_connected(Block *other);

    private:
    void find_intersect(Block *other, set<int> *intersect);
};


// -----------------------------------------------------------------------------
// function definitions
// -----------------------------------------------------------------------------
void parse_input_file(ifstream *, map<int, double>*, map<int, Block*> *, map<int, set<Block*>*> *);
void parse_block_net_connections(ifstream *, map<int, double>*, map<int, Block*>*, map<int, set<Block*>*> *);
void parse_fixed_block_positions(ifstream *, map<int, Block*>*);
void calculate_actual_weight(map<int, int>*, map<int, double>*);
void generate_matrix(map<int, double> *, map<int, Block*> *);
void iterate_block(map<int, Block*>* block_num_to_block);
void iterate_net_to_block(map<int, set<Block *>*> *net_to_block);
void iterate_net_weight(map<int, double>* net_weight);
