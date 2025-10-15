#include <bits/stdc++.h>
using namespace std;

int64_t PRIME = 2305843009213693951; // 2^61 - 1
int64_t ALPHA = 696969; // range of "target_value"

/* 
A structure to hold a DPF key.
- root: The root seed of the DPF tree.
- flag: The flag associated with the root seed.
- cw: A vector of correction words for each layer of the DPF tree.
- fcw0: A vector of flag correction words for all nodes not in the path to target node in a layer.
- fcw1: A vector of flag correction words for the node in the path to target node in a layer.
*/
struct dpf_key_type {
    int64_t root;
    uint8_t flag;
    vector<int64_t> cw;
    vector<uint8_t> fcw0;
    vector<uint8_t> fcw1;
    int64_t final_cw;
};

/*
A function to generate a random 64-bit integer in the range [1, PRIME).
With the seed provided as argument it initializes the random number generator with that seed.
*/
inline int64_t random_uint(int seed=0) {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    if(seed != 0) {
        gen.seed(seed);
    }
    static std::uniform_int_distribution<int64_t> dis(1, PRIME);
    return dis(gen);
}

/*
Given a 64bit random number(say 's') it generates two 64bit random number using 's' as the seed.
It returns a vector of size 2 containing the two generated random numbers.
*/
vector<int64_t> length_doubling_PRG(int64_t seed) {
    vector<int64_t> output(2);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int64_t> dis(1, PRIME);
    gen.seed(seed);
    output[0] = dis(gen);
    output[1] = dis(gen);
    return output;
}

/*
A function that extends the seeds and flags of a layer to the next layer using length_doubling_PRG.
*/
void expand_layer(vector<int64_t> &seeds, vector<uint8_t> &flags) {
    int n = seeds.size();
    vector<int64_t> new_seeds(2 * n);
    vector<uint8_t> new_flags(2 * n);
    for(int i = 0;i < n;i++) {
        vector<int64_t> prg_output = length_doubling_PRG(seeds[i]);
        new_seeds[2 * i] = prg_output[0];
        new_seeds[2 * i + 1] = prg_output[1];
        new_flags[2 * i] = prg_output[0]%2;
        new_flags[2 * i + 1] = prg_output[1]%2;
    }
    seeds = new_seeds;
    flags = new_flags;
}

/*
A function that generates DPF keys for two parties given the domain size, target index and target value.
It returns a vector of size 2 containing the DPF keys for both parties.
*/
vector<dpf_key_type> generateDPF(int64_t domain_size, int64_t target_index, int64_t target_value) {
    
    // CHECK: target_index in [0, domain_size)
    assert(target_index < domain_size);

    // Increase domain size to nearest power of 2
    if((domain_size & (domain_size - 1)) != 0) {
        int new_domain_size = 1;
        while(new_domain_size < domain_size) {
            new_domain_size *= 2;
        }
        domain_size = new_domain_size;
    }
    
    // CHECK: domain_size is power of 2
    assert((domain_size & (domain_size - 1)) == 0);

    // Initialize DPF keys for both parties
    vector<dpf_key_type> dpf_keys(2);
    
    // Initialize root seeds of both DPF trees
    dpf_keys[0].root = random_uint();
    dpf_keys[1].root = random_uint();
    
    int64_t initialize_flags = random_uint();
    dpf_keys[0].flag = initialize_flags % 2;
    dpf_keys[1].flag = (initialize_flags + 1) % 2;

    // CHECK: root flags are different
    assert(dpf_keys[0].flag != dpf_keys[1].flag);

    int max_depth = log2(domain_size);

    // Vectors to hold seeds and flags at current layer for both trees
    vector<int64_t> left_tree_seeds, right_tree_seeds;
    vector<uint8_t> left_tree_flags, right_tree_flags;

    // Initially current layer has only root seed and flag
    left_tree_seeds.push_back(dpf_keys[0].root);
    right_tree_seeds.push_back(dpf_keys[1].root);
    left_tree_flags.push_back(dpf_keys[0].flag);
    right_tree_flags.push_back(dpf_keys[1].flag);

    for(int layer = 1;layer <= max_depth;layer++) {

        // Determine direction to target node at current layer (0 means left, 1 means right)
        int direction = (target_index >> (max_depth - layer)) & 1;
        // Index of node in current layer that is in the path to target node
        int direction_among_child = target_index / (1<<(max_depth-layer));

        vector<int64_t> old_left_tree_seeds = left_tree_seeds;
        vector<int64_t> old_right_tree_seeds = right_tree_seeds;
        vector<uint8_t> old_left_tree_flags = left_tree_flags;
        vector<uint8_t> old_right_tree_flags = right_tree_flags;

        // Expand both trees to next layer
        expand_layer(left_tree_seeds, left_tree_flags);
        expand_layer(right_tree_seeds, right_tree_flags);
        
        int64_t L0 = left_tree_seeds[0],L1 = right_tree_seeds[0],R0 = left_tree_seeds[1],R1 = right_tree_seeds[1];
        uint8_t fL0 = left_tree_flags[0],fL1 = right_tree_flags[0],fR0 = left_tree_flags[1],fR1 = right_tree_flags[1];
        
        for(int i=2;i<(1<<layer);i++) {
            if(i%2) {
                R0 = R0 ^ left_tree_seeds[i];
                R1 = R1 ^ right_tree_seeds[i];

                fR0 = fR0 ^ left_tree_flags[i];
                fR1 = fR1 ^ right_tree_flags[i];
                // CHECK: flags should be either 0 or 1
                assert(fR0 <= 1 && fR1 <= 1);
            } else {
                L0 = L0 ^ left_tree_seeds[i];
                L1 = L1 ^ right_tree_seeds[i];
                
                fL0 = fL0 ^ left_tree_flags[i];
                fL1 = fL1 ^ right_tree_flags[i];
                // CHECK: flags should be either 0 or 1
                assert(fL0 <= 1 && fL1 <= 1);
            }
        }

        // Compute correction words based on direction
        int64_t cw,fcw0,fcw1;
        if(direction){
            cw = L0 ^ L1;
            fcw0 = fL0 ^ fL1;
            fcw1 = fR0 ^ fR1 ^ 1;
            
            // CHECK: flags should be either 0 or 1
            assert(fcw0 <= 1);
            assert(fcw1 <= 1);
        }else{
            cw = R0 ^ R1;
            fcw0 = fR0 ^ fR1;
            fcw1 = fL0 ^ fL1 ^ 1;
            
            // CHECK: flags should be either 0 or 1
            assert(fcw1 <= 1);
            assert(fcw0 <= 1);
        }

        dpf_keys[0].cw.push_back(cw);
        dpf_keys[0].fcw0.push_back(fcw0);
        dpf_keys[0].fcw1.push_back(fcw1);
    
        dpf_keys[1].cw.push_back(cw);
        dpf_keys[1].fcw0.push_back(fcw0);
        dpf_keys[1].fcw1.push_back(fcw1);

        // Apply correction words to the seeds and flags in current layer before proceeding to next layer
        for(int i = 0;i < (1<<layer);i++) {
            if(old_left_tree_flags[i/2]) {
                left_tree_seeds[i] = left_tree_seeds[i] ^ cw;
                if(i == direction_among_child) {
                    left_tree_flags[i] = left_tree_flags[i] ^ fcw1;
                } else {
                    left_tree_flags[i] = left_tree_flags[i] ^ fcw0;
                }
                // CHECK: flags should be either 0 or 1
                assert(left_tree_flags[i] <= 1);
            }
            if(old_right_tree_flags[i/2]) {
                right_tree_seeds[i] = right_tree_seeds[i] ^ cw;
                if(i == direction_among_child) {
                    right_tree_flags[i] = right_tree_flags[i] ^ fcw1;
                } else {
                    right_tree_flags[i] = right_tree_flags[i] ^ fcw0;
                }
                // CHECK: flags should be either 0 or 1
                assert(right_tree_flags[i] <= 1);
            }
        }

        // if we are in final layer set final correction word
        if(layer == max_depth) {
            int64_t final_cw;
            for(int i=0;i<(1<<layer);i++){
                if(i == direction_among_child){
                    final_cw = (left_tree_seeds[i] ^ right_tree_seeds[i] ^ target_value);
                }
            }
            dpf_keys[0].final_cw = final_cw;
            dpf_keys[1].final_cw = final_cw;
        }
    }
    return dpf_keys;
}


/*
A function that evaluates a DPF key at a specific target index and returns the resulting vector.
*/
vector<int64_t> EvalFull(int64_t domain_size, dpf_key_type dpf_key,int64_t target_index){
    int max_depth = dpf_key.cw.size();

    vector<int64_t> seeds(1);
    vector<uint8_t> flags(1);

    seeds[0] = dpf_key.root;
    flags[0] = dpf_key.flag;

    int64_t final_cw = dpf_key.final_cw;
    for(int layer = 0;layer < max_depth;layer++) {
        int direction_among_child = target_index / (1<<(max_depth-1-layer));
        vector<int64_t> old_seeds = seeds;
        vector<uint8_t> old_flags = flags;
        expand_layer(seeds, flags);
        int n = seeds.size();
        
        // CHECK: n should be 2^(layer+1) and direction_among_child should be in [0, n)
        assert(n == (1<<(layer+1)));
        assert(direction_among_child < n);
        
        for(int i = 0;i < n;i++) {
            uint8_t parent_flag = old_flags[i/2];
            int64_t parent_seed = old_seeds[i/2];
            if(parent_flag) {

                seeds[i] = seeds[i] ^ dpf_key.cw[layer];

                // CHECK: flags should be either 0 or 1
                assert(dpf_key.fcw0[layer] <= 1);
                assert(dpf_key.fcw1[layer] <= 1);
                if(i == direction_among_child) {
                    flags[i] = flags[i] ^ dpf_key.fcw1[layer];
                } else {
                    flags[i] = flags[i] ^ dpf_key.fcw0[layer];
                }
                // CHECK: flags should be either 0 or 1
                assert(flags[i] <= 1);
            }
        }
    }

    // apply final correction word
    for(int i = 0;i < seeds.size();i++) {
        if(flags[i]) {
            seeds[i] = seeds[i] ^ dpf_key.final_cw;
        }
    }

    // Trim to domain size
    vector<int64_t> result(domain_size, 0);
    for(int i = 0;i < domain_size;i++) {
        result[i] = seeds[i];
    }
    return result;
}

/*
A function that checks the correctness of the generated DPF keys by evaluating them and verifying the output.
*/
bool check_dpf_correctness(int64_t domain_size, int64_t target_index, int64_t target_value, vector<dpf_key_type> dpf_keys) {
    vector<int64_t> left_tree_result = EvalFull(domain_size, dpf_keys[0], target_index);
    vector<int64_t> right_tree_result = EvalFull(domain_size, dpf_keys[1], target_index);

    for(int k = 0; k < domain_size; k++) {
        int64_t val = left_tree_result[k] ^ right_tree_result[k];
        if(k != target_index) {
            if(val != 0){
                return false;
            }
        } else {
            if(val != target_value){
                return false;
            }
        }
    }
    return true;
}

/* take command line arguments <domain_size> and <target_index> */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <domain_size> <no of dpfs>" << endl;
        return 1;
    }
    
    int domain_size = atoi(argv[1]);
    int num_dpf = atoi(argv[2]);

    for( int i=0;i<num_dpf;i++){
        int target_index = random_uint() % domain_size;
        int target_value = random_uint() % ALPHA; // target value in [0, ALPHA)

        vector<dpf_key_type> keys = generateDPF(domain_size, target_index, target_value);
        vector<int64_t> left_tree_result = EvalFull(domain_size, keys[0], target_index);
        vector<int64_t> right_tree_result = EvalFull(domain_size, keys[1], target_index);


        bool flag = check_dpf_correctness(domain_size, target_index, target_value, keys);
        cout << "DPF " << i+1 << ": " << (flag ? "PASSED" : "FAILED") << endl;
    }
    return 0;
}