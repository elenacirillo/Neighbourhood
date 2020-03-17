#ifndef GREEDY_VERSION4_HH
#define GREEDY_VERSION4_HH

#include "greedy_version3.hpp"

/*
*
    VERSION #4:

    1) jobs are ordered with respect to pressure (current time + minimum 
       execution time - deadline)

    2) each job in the ordered queue is assigned to:
       2.1) the optimal configuration on an already opened node, if possible
       2.2) a new node, equipped with the optimal configuration in terms of
            GPUType and with the VMType that allows to select the highest 
            number of available GPUs, if possible
       2.3) the best among the suboptimal configurations available on an 
            already opened node with residual space, if possible
       
       NOTE: assignment to already opened nodes is performed with a 
       best-fit approach

    3) postprocessing phase:
       3.1) for all opened nodes with idle GPUs, a new configuration is
            selected, with the same VMtype and GPUtype and maxnGPUs equal
            to the number of used GPUs (or usedGPUs+1, since there are no
            available configurations with, for instance, 3 GPUs), if possible
       3.2) the GPUs remaining idle are assigned to the job with best 
            improvement in performance (: with the highest speed-up)
*
*/

class Greedy_version4: public Greedy_version3 {

protected:
  // parameters for randomization
  double beta = 0.2;                      // parameter for node selection

  /* select_best_node
  *   returns the best node the given setup fits in; a node is the best-fit 
  *   if the new setup (almost) saturates all its idle GPUs
  *
  *   Input:  const Setup&    setup to be allocated in an opened node
  *
  *   Outpu:  unsigned        index of the best opened node
  */
  unsigned select_best_node (const Setup&);

  /* assign_to_existing_node:
  *   assigns the given job to an already opened node
  *
  *   Input:  const Job&                      job to be assigned
  *           setup_time_t::const_iterator    iterator to the best setup
  *           job_schedule_t&                 new proposed schedule
  */
  virtual bool assign_to_existing_node (const Job&,
                                        setup_time_t::const_iterator,
                                        job_schedule_t&) override;

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
  Greedy_version4 (const std::string&, const std::string&, const std::string&, 
                   const std::string&, const std::string&);

  /* destructor
  */
  virtual ~Greedy_version4 (void) = default;

};

#endif /* GREEDY_VERSION4_HH */