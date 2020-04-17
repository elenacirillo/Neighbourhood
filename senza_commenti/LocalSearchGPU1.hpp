#ifndef LOCAL_SEARCH_GPU1_HH
#define LOCAL_SEARCH_GPU1_HH

#include "LocalSearchGPU.hpp"

class LocalSearchGPU1 : public LocalSearchGPU
{
public:
    // neighboorhood type: maps the index of a node into a possible setup
    typedef std::unordered_multimap<unsigned, Setup> neighborhood_t;

private:

    // initialize the map GPU-VM starting from ttime (ttime is a protected memeber of the parent class Heuristic)
    void initialize_GPUs_setups(void);
    
    // maps a max_nGPUs into all the possible setups with the same maxGPU
    GPUs_setups_t GPUs_setups;

    // visit the neighborhood of initial schedule
    bool visit_neighbor(void) override;

    // generates the neighboorhood to explore
    neighborhood_t generate_neighborhood(void);

    // changes the schedule according to node and GPU_type
    job_schedule_t change_GPU(unsigned, const Setup &);


public:

    // constructor
    LocalSearchGPU1 (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
    ~LocalSearchGPU1 (void) = default;
};

#endif /* LOCAL_SEARCH_GPU1_HH */
