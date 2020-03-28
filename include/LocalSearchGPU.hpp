#ifndef LOCAL_SEARCH_GPU_HH
#define LOCAL_SEARCH_GPU_HH

#include "LocalSearch.hpp"

// TODO: inserirle dentro la classe e adattare i return type

// map a max_nGPU in the set of setup with that max_nGPU
typedef std::unordered_map<unsigned, std::unordered_set<Setup>> GPUs_setups_t;

// neighboorhood type: maps the index of a node into a possible setup
typedef std::unordered_multimap<unsigned, Setup> neighborhood_t;

// associates to every node the set of jobs in execution on that specific node
typedef std::unordered_map<unsigned, std::unordered_set<Job>> node_jobs_t;

class LocalSearchGPU : public LocalSearch
{

private:

    // maps a max_nGPUs into all the possible setups with the same maxGPU
    GPUs_setups_t GPUs_setups;

    // pairings node_idx - jobs on that node (it should be updated every time initial_schedule changes)
    node_jobs_t node_jobs;

    // visit the neighborhood of initial schedule
    bool visit_neighbor(void) override;

    // generates the neighboorhood to explore
    neighborhood_t generate_neighborhood(void);

    // changes the schedule according to node and GPU_type
    job_schedule_t change_GPU(unsigned, const Setup &);

    //initialize the map GPU-VM starting from ttime (ttime is a protected memeber of the parent class Heuristic)
    void initialize_GPUs_setups(void);

    // update the set node_jobs starting from initial_schedule
    void update_node_jobs(void);

    // find the indexes of top nodes to change in the neighbourhood
    std::set<unsigned> get_tardiness_nodes(unsigned);

public:

    // constructor
    LocalSearchGPU (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);

};

#endif /* LOCAL_SEARCH_GPU_HH */
