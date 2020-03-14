#ifndef GREEDY_HH
#define GREEDY_HH

#include "heuristic.hpp"
#include "dstar.hpp"

class Greedy: public Heuristic {

protected:
  // type definitions
  typedef std::pair<setup_time_t::const_iterator, 
                    setup_time_t::const_iterator> range_t;

  /* compare_pressure (static)
  *   returns true if the 1st job's pressure is higher than the 2nd's
  */
  static bool compare_pressure (const Job&, const Job&);

  /* compare_configuration
  *   returns true if the given Setup and Node have the same configuration 
  *   (same VMtype and GPUtype)
  */
  virtual bool compare_configuration (const Setup&, const Node&) const;

  /* sort_jobs_list
  *   sorts the submitted_jobs list according to compare_pressure
  */
  virtual void sort_jobs_list (void) override;

  /* random_swap
  *   Randomly swaps jobs in the sorted list
  */
  void random_swap (void);

  /* objective_function
  *   evaluate objective function of the current schedule
  *
  *   Input:  job_schedule_t&     new proposed schedule
  *           double              elapsed time between previous and current 
  *                               iteration
  *
  *   Output: double              total cost of the proposed schedule
  */
  virtual double objective_function (job_schedule_t&, double) override;

  /* assign_to_existing_node:
  *   assigns the given job to an already opened node
  *
  *   Input:  const Job&                      job to be assigned
  *           setup_time_t::const_iterator    iterator to the best setup
  *           job_schedule_t&                 new proposed schedule
  *
  *   Output: bool                            true if job has been assigned
  */
  virtual bool assign_to_existing_node (const Job&, 
                                        setup_time_t::const_iterator,
                                        job_schedule_t&);
  
  /* assign_to_new_node:
  *   assigns the given job to a new node
  *
  *   Input:  const Job&                      job to be assigned
  *           setup_time_t::const_iterator    iterator to the best setup
  *           job_schedule_t&                 new proposed schedule
  */
  virtual void assign_to_new_node (const Job&, 
                                   setup_time_t::const_iterator,
                                   job_schedule_t&);

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
                                     Dstar&, job_schedule_t&) = 0;

  // selects from a range of setups with VMtype and GPUtype equal to the 
  // configuration in the given Node the one with required maxnGPUs and
  // nGPUs
  setup_time_t::const_iterator select_setup_in_range (range_t, const Node&, 
                                                      unsigned) const;

  /* change_nodes_configurations 
  *   loops over the opened nodes and changes configuration of those with
  *   idle GPUs
  *
  *   Input:  const job_schedule_t&           proposed schedule
  */
  void change_nodes_configurations (const job_schedule_t&);

  /* perform_assignment 
  *   assigns the selected job to the new proposed schedule
  *
  *   Input:  const Job&                      job to be assigned
  *           job_schedule_t&                 new proposed schedule
  */
  virtual void perform_assignment (const Job&, job_schedule_t&) override;

  /* perform_scheduling
  *   preforms the scheduling process that assigns the best configuration to
  *   each submitted jobs (as long as resources are available)
  *
  *   Input:  unsigned              maximum number of random iterations
  *
  *   Output: job_schedule_t        proposed schedule
  */
  virtual job_schedule_t perform_scheduling (unsigned) override;

  /* preprocessing
  *   computes the pressure of all submitted jobs and sorts the queue 
  *   accordingly
  */
  virtual void preprocessing (void) override;

  /* scheduling_step
  *   sorts the list of submitted jobs and performs scheduling of all
  *   submitted jobs
  *
  *   Input:    job_schedule_t&     empty schedule to be built
  *             bool                true if random swap of submitted_jobs is 
  *                                 required
  */
  virtual void scheduling_step (job_schedule_t&, bool r_swap = false) override;

  /* postprocessing
  *   aims to reduce the number of idle GPUs in selected configurations
  *
  *   Input:    job_schedule_t&         proposed schedule to be updated
  */
  virtual void postprocessing (job_schedule_t&) override;

  /* update_best_schedule
  *
  *   Input:    job_schedule_t&         new proposed schedule
  *             double&                 minimum cost
  *             job_schedule_t&         best schedule
  *             std::vector<Node>&      vector of opened nodes
  *
  *   Output:   bool                    true if best schedule has been updated
  */
  virtual bool update_best_schedule (job_schedule_t&, double&, 
                                     job_schedule_t&,
                                     std::vector<Node>&) override;

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
  Greedy (const std::string&, const std::string&, const std::string&, 
          const std::string&, const std::string&);
  
  /* destructor
  */
  virtual ~Greedy (void) = default;

};

#endif /* GREEDY_HH */
