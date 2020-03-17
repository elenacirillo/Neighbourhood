#ifndef DSTAR_HH
#define DSTAR_HH

#include "utilities.hpp"
#include "setup.hpp"
#include "job.hpp"

#include <cassert>
#include <random>
#include <cmath>

class Dstar {

private:
  // type definitions
  typedef std::multimap<double, setup_time_t::const_iterator> dstar_map_t;
  typedef dstar_map_t::const_iterator const_iterator;

  // setups s.t. job j is executed before deadline (ordered by min cost)
  dstar_map_t D_star_j;

  // setups s.t. job j is NOT executed before deadline (ordered by min 
  // execution time)
  dstar_map_t D_star_j_C;

  // parameter for randomization
  double alpha = 0.;

public:
  /*  constructor
  *
  *   input:  const Job&               job j
  *           const setup_time_t&      tjvg (see utilities.hpp for type def)
  *           double                   current time
  *
  */
  Dstar (const Job&, const setup_time_t&, double);

  /* sets parameter for randomization
  */
  void set_random_parameter (double a) {alpha = a;}

  /* determines the best setup:
  *   if it is possible to match deadline, the best setup is the cheapest
  *   otherwise, the best setup is the fastest
  *
  *   Input:  std::default_random_engine    random numbers generator
  *
  */
  setup_time_t::const_iterator get_best_setup (std::default_random_engine&);

  /* is_end
  *   returns true if D_star_j.empty() AND D_star_j_C.empty()
  */
  bool is_end (void) const;

  /* print
  *   prints D_star_j and D_star_j_C
  *
  *   Input:  std::ostream&       where to print the containers
  */
  void print (std::ostream&) const;
};

#endif /* DSTAR_HH */