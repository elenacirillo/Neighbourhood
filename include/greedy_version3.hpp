#ifndef GREEDY_VERSION3_HH
#define GREEDY_VERSION3_HH

#include "greedy_version2.hpp"

/*
*
    VERSION #3:

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
       first-fit approach

    3) postprocessing phase:
       3.1) for all opened nodes with idle GPUs, a new configuration is
            selected, with the same VMtype and GPUtype and maxnGPUs equal
            to the number of used GPUs (or usedGPUs+1, since there are no
            available configurations with, for instance, 3 GPUs), if possible
       3.2) the GPUs remaining idle are assigned to the job with best 
            improvement in performance (: with the highest speed-up)
*
*/

class Greedy_version3: public Greedy_version2 {

protected:
  /* postprocessing
  *   aims to reduce the number of idle GPUs in selected configurations
  *
  *   Input:    job_schedule_t&         proposed schedule to be updated
  */
  virtual void postprocessing (job_schedule_t&) override;

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
  Greedy_version3 (const std::string&, const std::string&, const std::string&, 
                   const std::string&, const std::string&);

  /* destructor
  */
  virtual ~Greedy_version3 (void) = default;

};

#endif /* GREEDY_VERSION3_HH */