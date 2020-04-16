#ifndef LOCAL_SEARCH_GPU2_HH
#define LOCAL_SEARCH_GPU2_HH

#include "LocalSearchGPU.hpp"

class LocalSearchGPU2 : public LocalSearchGPU
{

private:
    
    // visit the neighborhood of initial schedule
    bool visit_neighbor(void) override;

    // generates the neighboorhood to explore
    std::unordered_set<unsigned> generate_neighborhood(void);

    // changes the schedule according to node and GPU_type
    job_schedule_t change_nGPU(unsigned);


public:

    // constructor
    LocalSearchGPU2 (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
    virtual ~LocalSearchGPU2 (void) = default;
};

#endif /* LOCAL_SEARCH_GPU2_HH */
