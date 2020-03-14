#ifndef GREEDY_VERSION1_HH
#define GREEDY_VERSION1_HH

#include "greedy.hpp"

/*
*
    VERSION #1:

    1) jobs are ordered with respect to pressure (current time + minimum 
       execution time - deadline)

    2) each job in the ordered queue is assigned to:
       2.1) the optimal configuration on an already opened node, if possible
       2.2) a new node, equipped with the optimal configuration in terms of
            VMType and GPUType (but the highest number of available GPUs), 
            if possible
       2.3) the best among the suboptimal configurations available on an 
            already opened node with residual space, if possible

       NOTE: assignment to already opened nodes is performed with a 
       first-fit approach

    3) postprocessing phase:
       3.1) for all opened nodes with idle GPUs, a new configuration is
            selected, with the same VMtype and GPUtype and maxnGPUs equal
            to the number of used GPUs (or usedGPUs+1, since there are no
            available configurations with, for instance, 3 GPUs), if possible
*
*/

class Greedy_version1: public Greedy {

protected:
  /* assign_to_suboptimal:
  *   assigns the given job to a suboptimal configuration, available in 
  *   an already opened node
  *
  *   Input:  const Job&                      job to be assigned
  *           const setup_time_t&             execution times of the given job
  *           Dstar&                          see dstar.hpp
  *           job_schedule_t&                 new proposed schedule
  *
  *   Output: bool                            true if job has been assigned
  */
  virtual bool assign_to_suboptimal (const Job&, const setup_time_t&,
                                     Dstar &, job_schedule_t&) override;

public:
  /*  constructor
  *
  *   Input:  const std::string&    ARGS = "nInitialJ-nN-nJ-lambdaa-mu-myseed"
  *           const std::string&    "-delta"
  *           const std::string&    name of file with the list of jobs
  *           const std::string&    name of file with execution times of jobs
  *           const std::string&    name of file with the list of nodes
  *
  */
  Greedy_version1 (const std::string&, const std::string&, const std::string&, 
                   const std::string&, const std::string&);

  /* destructor
  */
  virtual ~Greedy_version1 (void) = default;

};

#endif /* GREEDY_VERSION1_HH */