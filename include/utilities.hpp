#ifndef UTILITIES_HH
#define UTILITIES_HH

#include <string>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <limits>
#include <cassert>
#include <random>

/*  
    forward declarations of classes
*/
class Job;
class Setup;
class Node;
class Schedule;

/*
    type definitions
*/
typedef std::vector<std::string> row_t;
typedef std::list<row_t> table_t;

typedef std::unordered_multimap<Setup, double> setup_time_t;
typedef std::unordered_map<std::string, setup_time_t> time_table_t;

typedef std::unordered_map<Job, Schedule> job_schedule_t;

/*
    names of folders for data and results (change if needed)
*/
const std::string data_folder = "../../data/from_loading_data/";
const std::string result_folder = "../../data/results/";

/*
    constant expressions
*/
static constexpr double INF = std::numeric_limits<double>::infinity();

/*
    template function for random selection

    selects an element of type VAL from a container CONT<KEY,VAL>
    (e.g. std::multimap<unsigned, unsigned>)

    Input:    CONT<KEY,VAL,Ts...>&            the container to be inspected
              std::default_random_engine&     random numbers generator
              double                          parameter for random selection

    Output:   VAL                             the element that has been selected
*/
template <template <typename...> class CONT, typename KEY, typename VAL, typename ... Ts>
VAL
random_select (CONT<KEY,VAL,Ts...>& D, std::default_random_engine& generator, 
               double alpha)
{
  typename CONT<KEY,VAL>::const_iterator next_it = D.cbegin();
  
  // gamma = 0 --> uniform probability
  double gamma = 0.;
  
  if (alpha > 0)
  {
    unsigned n_elems = std::round(D.size() * alpha);

    std::vector<double> w(n_elems,1.);
    for (unsigned j = 1; j < w.size(); ++j)
      w[j] = w[j-1] * (1 - gamma);

    std::discrete_distribution<int> distribution(w.cbegin(), w.cend());
    unsigned idx = distribution(generator);

    for (unsigned count = 0; count < idx; ++count)
      next_it++;
  }

  assert(next_it != D.cend());
  VAL cit = next_it->second;
  D.erase(next_it);

  return cit;
};


#endif /* UTILITIES_HH */