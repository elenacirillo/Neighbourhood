#ifndef LOCAL_SEARCH_GPU_HH
#define LOCAL_SEARCH_GPU_HH

#include "LocalSearch.hpp"

class LocalSearchGPU : public LocalSearch
{

public:  
    // map a max_nGPU in the set of setup with that max_nGPU
    typedef std::unordered_map<unsigned, std::unordered_set<Setup>> GPUs_setups_t;

    // associates to every node the set of jobs in execution on that specific node
    typedef std::unordered_map<unsigned, std::unordered_set<Job>> node_jobs_t;

protected:

    // pairings node_idx - jobs on that node (it should be updated every time initial_schedule changes)
    node_jobs_t node_jobs;

    // visit the neighborhood of initial schedule
    virtual bool visit_neighbor(void)=0;

    // update the set node_jobs starting from initial_schedule
    void update_node_jobs(void);

    // find the indexes of top nodes to change in the neighbourhood
    std::unordered_set<unsigned> get_tardiness_nodes(unsigned);

public:

    // constructor
    LocalSearchGPU (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
    virtual ~LocalSearchGPU (void) = default;
};

#endif /* LOCAL_SEARCH_GPU_HH */
