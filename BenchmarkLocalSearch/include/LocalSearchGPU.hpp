#ifndef LOCAL_SEARCH_GPU_HH
#define LOCAL_SEARCH_GPU_HH

#include "LocalSearch.hpp"

class LocalSearchGPU : public LocalSearch
{

public:
	// maps a VMtype (std::string) into a list of GPUs (std::list<std::string>) that can be mounted on that VM
    typedef std::unordered_map<std::string, std::list<std::string>> vm_gpus_t;

    // neighboorhood type: maps the index of a node into a GPU_type
    typedef std::unordered_multimap<unsigned, std::string> neighborhood_t;

    // associates to every node the set of jobs in execution on that specific node
    typedef std::unordered_map<unsigned, std::unordered_set<Job>> node_jobs_t;

private:
   
    // avaiable pairings VM-GPU (initialized by the constructor)
    vm_gpus_t vm_gpus;

    // pairings node_idx - jobs on that node (it should be updated every time initial_schedule changes)
    node_jobs_t node_jobs;

    // visit the neighborhood of initial schedule
    bool visit_neighbor(void) override;

    // generates the neighboorhood to explore
    neighborhood_t generate_neighborhood(void);

    // changes the schedule according to node and GPU_type
    job_schedule_t change_GPU(unsigned, std::string);

    //initialize the map GPU-VM starting from ttime (ttime is a protected memeber of the parent class Heuristic)
    void initialize_vm_gpus(void);

    // update the set node_jobs starting from initial_schedule
    void update_node_jobs(void);

public:
    
    LocalSearchGPU (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);

};

#endif /* LOCAL_SEARCH_GPU_HH */