#include "greedy_version2.hpp"

Greedy_version2::Greedy_version2 (const std::string& args, 
                                  const std::string& d, 
                                  const std::string& file_jobs, 
                                  const std::string& file_times, 
                                  const std::string& file_nodes):
  Greedy_version1(args, d, file_jobs, file_times, file_nodes)
{}

bool
Greedy_version2::compare_configuration (const Setup& stp, const Node& n) const
{
  return (stp.get_GPUtype() == n.get_GPUtype());
}

setup_time_t::const_iterator
Greedy_version2::select_largest_setup (const Job& j, unsigned nGPUs,
                                       const std::string& GPUtype) const
{
  const setup_time_t& tjvg = ttime.at(j.get_ID());
  setup_time_t::const_iterator cit;
  setup_time_t::const_iterator found_it = tjvg.cend();
  unsigned maxnGPUs = 0;
  for (cit = tjvg.cbegin(); cit != tjvg.cend(); ++cit)
  {
    const Setup& new_stp = cit->first;
    if (new_stp.get_GPUtype() == GPUtype && new_stp.get_nGPUs() == nGPUs)
    {
      if (new_stp.get_maxnGPUs() > maxnGPUs)
      {
        maxnGPUs = new_stp.get_maxnGPUs();
        found_it = cit;
      }
    }
  }
  return found_it;
}

setup_time_t::const_iterator
Greedy_version2::select_setup (const Job& j, const std::string& VMtype,
                               const std::string& GPUtype, unsigned g) const
{
  const setup_time_t& tjvg = ttime.at(j.get_ID());
  bool found = false;

  Setup temp_stp(VMtype, GPUtype);

  range_t range_it = tjvg.equal_range(temp_stp);

  setup_time_t::const_iterator new_stp_it = range_it.first;
  while (new_stp_it != range_it.second && ! found)
  {
    const Setup& new_stp = new_stp_it->first;
    if (new_stp.get_nGPUs() == g)  
      found = true;
    else
      ++new_stp_it;
  }
  return new_stp_it;
}

bool
Greedy_version2::assign_to_existing_node (const Job& j, 
                                      setup_time_t::const_iterator best_stp_it,
                                      job_schedule_t& new_schedule)
{
  bool assigned = false;
  const Setup& best_stp = best_stp_it->first;
  
  for (unsigned idx = 0; idx < last_node_idx && !assigned; ++idx)
  {
    Node& node = nodes[idx];

    // if the best setup fits in an already opened node, assign
    if (compare_configuration(best_stp, node) && 
        best_stp.get_nGPUs() <= node.get_remainingGPUs())
    {
      const std::string& VMtype = node.get_VMtype();
      const std::string& GPUtype = node.get_GPUtype();
      unsigned nGPUs = best_stp.get_nGPUs();
      
      assigned = true;
      node.set_remainingGPUs(nGPUs);

      setup_time_t::const_iterator selected_stp_it;
      selected_stp_it = select_setup(j, VMtype, GPUtype, nGPUs);

      Schedule sch(selected_stp_it, idx);
      new_schedule[j] = sch;
    }
  }
  return assigned;
}

void
Greedy_version2::assign_to_new_node (const Job& j, 
                                     setup_time_t::const_iterator best_stp_it,
                                     job_schedule_t& new_schedule)
{
  const Setup& best_stp = best_stp_it->first;

  // determine a new setup with the same GPUtype and nGPUs, but the largest
  // possible number of available GPUs
  const std::string& GPUtype = best_stp.get_GPUtype();
  unsigned nGPUs = best_stp.get_nGPUs();
  setup_time_t::const_iterator largest_it = select_largest_setup(j, nGPUs, 
                                                                 GPUtype);

  Greedy::assign_to_new_node(j, largest_it, new_schedule);
}
