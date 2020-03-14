#include "greedy_version1.hpp"

Greedy_version1::Greedy_version1 (const std::string& args, 
                                  const std::string& d, 
                                  const std::string& file_jobs, 
                                  const std::string& file_times, 
                                  const std::string& file_nodes):
  Greedy(args, d, file_jobs, file_times, file_nodes)
{}

bool
Greedy_version1::assign_to_suboptimal (const Job& j, 
                                       const setup_time_t& tjvg,
                                       Dstar& dstar, 
                                       job_schedule_t& new_schedule)
{
  bool assigned = false;
  while (!dstar.is_end() && !assigned)
  {
    setup_time_t::const_iterator best_stp = dstar.get_best_setup(generator);
    assigned = assign_to_existing_node(j, best_stp, new_schedule);
  }
  return assigned;
}