#ifndef GREEDY_VERSION2_HH
#define GREEDY_VERSION2_HH

#include "greedy_version1.hpp"

/*
*
    VERSION #2:

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
*
*/

class Greedy_version2: public Greedy_version1 {

protected:
  /* compare_configuration
  *   returns true if the given Setup and Node have the same configuration 
  *   (same GPUtype)
  */
  virtual bool compare_configuration (const Setup&,const Node&) const override;
  
  /* select_largest_setup
  *   selects a setup with the given GPUtype and nGPUs, but the maximum 
  *   possible maxnGPUs
  *
  *   Input:  const Job&                      job to be assigned
  *           unsigned                        number of required GPUs
  *           const std::string&              required GPU type
  *
  *   Output: setup_time_t::const_iterator    iterator to the largest setup
  */
  setup_time_t::const_iterator select_largest_setup (const Job&, unsigned,
                                                     const std::string&) const;
  
/*
*
  // TODO: questo dovrebbe essere identico a Greedy::select_setup_in_range...
  // l'unica distinzione è il controllo su maxnGPUs...
  // ma per un dato tipo di VM e tipo di GPU, maxnGPUs non è fissato 
  // a priori?!?
*
*/
  /* select_setup
  *   selects a setup with the given VMtype, GPUtype and nGPUs
  *
  *   Input:  const Job&                      job to be assigned
  *           const std::string&              required VM type
  *           const std::string&              required GPU type
  *           unsigned                        number of required GPUs
  *
  *   Output: setup_time_t::const_iterator    iterator to the new setup
  */
  setup_time_t::const_iterator select_setup (const Job&, const std::string&,
                                             const std::string&, 
                                             unsigned) const;

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

  /* assign_to_new_node:
  *   assigns the given job to a new node
  *
  *   Input:  const Job&                      job to be assigned
  *           setup_time_t::const_iterator    iterator to the best setup
  *           job_schedule_t&                 new proposed schedule
  */
  virtual void assign_to_new_node (const Job&, 
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
  Greedy_version2 (const std::string&, const std::string&, const std::string&, 
                   const std::string&, const std::string&);

  /* destructor
  */
  virtual ~Greedy_version2 (void) = default;

};

#endif /* GREEDY_VERSION2_HH */