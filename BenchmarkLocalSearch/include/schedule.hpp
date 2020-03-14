#ifndef SCHEDULE_HH
#define SCHEDULE_HH

#include "job.hpp"
#include "setup.hpp"
#include "node.hpp"

#include <cassert>

class Schedule {

private:
  setup_time_t::const_iterator csetup;
  unsigned node_idx;

  bool empty_schedule = true;

  unsigned iter = 0;
  double sim_time = 0;
  double execution_time = 0.;
  double completion_percent = 0.;
  double completion_percent_step = 0.;
  double start_time = 0.;
  double finish_time = 0.;
  double tardiness = 0.;
  double vmCost = 0.;
  double tardinessCost = 0.;

public:
  /*  constructors
  *
  *   input(1):  void              default
  *
  *   input(2):  setup_time_t::const_iterator     required to initialize a 
  *              unsigned                         full schedule with all the
  *                                               parameters that are not
  *                                               computed afterwards
  *
  */
  Schedule (void) = default;
  Schedule (setup_time_t::const_iterator, unsigned);

  // getters
  bool isEmpty (void) const {return empty_schedule;}
  //
  double get_startTime (void) const {return start_time;}
  double get_completionPercent (void) const {return completion_percent;}
  double get_cP_step (void) const {return completion_percent_step;}
  double get_simTime (void) const {return sim_time;}
  double get_tardiness (void) const {return tardiness;}
  double get_vmCost (void) const {return vmCost;}
  double get_tardinessCost (void) const {return tardinessCost;}
  //
  const Setup& get_setup (void) const;
  double get_selectedTime (void) const;
  unsigned get_node_idx (void) const;

  // setters
  void set_iter (unsigned it) {iter = it;}
  void set_simTime (double st) {sim_time = st;}
  void set_executionTime (double et) {execution_time = et;}
  void set_completionPercent (double cp) {completion_percent = cp;}
  void set_cP_step (double cp) {completion_percent_step = cp;}
  void set_finishTime (double ft) {finish_time = ft;}
  void set_tardiness (double t) {tardiness = t;}
  void set_startTime (double st) {start_time = st;}
  void compute_vmCost (unsigned);
  void compute_tardinessCost (double);
  void change_setup (setup_time_t::const_iterator stp) {csetup = stp;}

  // print
  static void print_names (std::ostream& ofs, char endline = '\n');
  void print (const Job&, const std::vector<Node>&, std::ostream&, 
              char endline = '\n') const;
};

#endif /* SCHEDULE_HH */
