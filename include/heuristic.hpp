#ifndef HEURISTIC_HH
#define HEURISTIC_HH

#include "fileIO.hpp"
#include "schedule.hpp"

#include <random>
#include <fstream>

class Heuristic {

public:
  	unsigned LS_count = 0; //TODO: SERVO SOLO PER IL TEST, ELIMINAMI!!!!
protected:

  // ARGS = "nInitialJ-nN-nJ-lambdaa-mu-myseed"; delta = "-delta"
  std::string ARGS = "";
  std::string delta = "";

  const double TOL = 1e-7;
  const double scheduling_interval = INF;

  double current_time = 0.;

  // list of all jobs
  std::list<Job> jobs;
  // list of all nodes
  std::vector<Node> nodes;
  // table tjvg for all j (see utilities.hpp for type definition)
  time_table_t ttime;

  // list of submitted jobs
  std::list<Job> submitted_jobs;
  // list of scheduled jobs
  std::vector<job_schedule_t> scheduled_jobs;

  // index of last used node
  unsigned last_node_idx = 0;

  // random parameters and random numbers generator
  bool full_greedy = true;
  std::default_random_engine generator;   // generator
  double alpha = 0.05;                    // parameter for setup selection
  double pi = 0.05;                       // parameter for random swaps in 
                                          //  submitted_jobs

  /* submit_job
  *   add a new job to the queue according to its submission time
  *
  *   Input:  double    elapsed time between previous and current iteration
  *
  *   Output: double    submission time of new job
  */
  double submit_job (double);

  /* remove_ended_jobs
  *   remove from the queue all jobs that have already been executed
  */
  void remove_ended_jobs (void);

  /* sort_jobs_list
  */
  virtual void sort_jobs_list (void) = 0;

  /* close_nodes
  *   close all the currently opened nodes
  */
  void close_nodes (void);

  /* objective_function
  *   evaluate objective function of the current schedule
  *
  *   Input:  job_schedule_t&     new proposed schedule
  *           double              elapsed time between previous and current 
  *                               iteration
  *
  *   Output: double              total cost of the proposed schedule
  */
  virtual double objective_function (job_schedule_t&, double) = 0;

  /* update_execution_times
  *   update execution time of jobs that have been partially executed
  *
  *   Input:  unsigned    iteration number
  *           double      elapsed time between previous and current iteration
  */
  void update_execution_times (unsigned, double);

  /* update_minMax_exec_time
  *   compute and update minimum and maximum execution time of the given job
  */
  void update_minMax_exec_time (Job&) const;

  /* update_scheduled_jobs
  *   update list of scheduled jobs with information about execution time,
  *   costs, etc
  *
  *   Input:  unsigned    iteration number
  *           double      elapsed time between previous and current iteration
  *
  *   Output: bool        true if completion percent of all jobs is 100
  */
  bool update_scheduled_jobs (unsigned, double);

  /* find_first_finish_time
  *   return the execution time of the first ending job in the given schedule
  */
  double find_first_finish_time (const job_schedule_t&) const;

  /* perform_assigment
  *   assigns the selected job to the new proposed schedule
  *
  *   Input:  const Job&                      job to be assigned
  *           job_schedule_t&                 new proposed schedule
  */
  virtual void perform_assignment (const Job&, job_schedule_t&) = 0;

  /* perform_scheduling
  *   preforms the scheduling process that assigns the best configuration to
  *   each submitted jobs (as long as resources are available)
  *
  *   Input:  unsigned              maximum number of random iterations
  *
  *   Output: job_schedule_t        proposed schedule
  */
  virtual job_schedule_t perform_scheduling (unsigned) = 0;

  /* preprocessing
  */
  virtual void preprocessing (void) = 0;

  /* scheduling_step
  *   sorts the list of submitted jobs and performs scheduling of all
  *   submitted jobs
  *
  *   Input:    job_schedule_t&     empty schedule to be built
  *             bool                true if random swap of submitted_jobs is 
  *                                 required
  */
  virtual void scheduling_step (job_schedule_t&, bool r_swap = false) = 0;

  /* postprocessing
  *
  *   Input:    job_schedule_t&         proposed schedule to be updated
  */
  virtual void postprocessing (job_schedule_t&) = 0;

  /* update_best_schedule
  *
  *   Input:    job_schedule_t&         new proposed schedule
  *             double&                 minimum cost
  *             job_schedule_t&         best schedule
  *             std::vector<Node>&      vector of opened nodes
  *
  *   Output:   bool                    true of best schedule has been updated
  */
  virtual bool update_best_schedule (job_schedule_t&, double&, 
                                     job_schedule_t&,
                                     std::vector<Node>&) = 0;

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
  Heuristic (const std::string&, const std::string&, const std::string&, 
          const std::string&, const std::string&);
  
  /* destructor
  */
  virtual ~Heuristic (void) = default;

  /* algorithm
  *
  *   Input:  unsigned      seed for random numbers generator
  *           unsigned      maximum number of random iterations
  */
  virtual void algorithm (unsigned, unsigned);
  
  /* print_data
  *   prints data about jobs, nodes and execution times used by the algorithm
  *   using functions from fileIO.hpp (see for further information)
  */
  void print_data (void) const;

  /* print_schedule
  *   prints the schedule produced by the algorithm using the function
  *   print_result from fileIO.hpp (see for further information)
  *
  *   Input:    const std::string&    filename where the schedule should be
  *                                   printed
  */
  void print_schedule (const std::string& filename = "schedule.csv") const;

};

#endif /* HEURISTIC_HH */
