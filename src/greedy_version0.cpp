#include "greedy_version0.hpp"

Greedy_version0::Greedy_version0 (const std::string& args, 
                                  const std::string& d, 
                                  const std::string& file_jobs, 
                                  const std::string& file_times, 
                                  const std::string& file_nodes):
  Greedy(args, d, file_jobs, file_times, file_nodes)
{}

bool
Greedy_version0::assign_to_suboptimal (const Job& j, const setup_time_t& tjvg,
                                       Dstar& dstar, 
                                       job_schedule_t& new_schedule)
{
  bool assigned = false;

  for (unsigned idx = 0; idx < last_node_idx && !assigned; ++idx)
  {
    Node& node = nodes[idx];

    if (node.get_remainingGPUs() > 0)
    {
      const std::string& VMtype = node.get_VMtype();
      const std::string& GPUtype = node.get_GPUtype();

      Setup temp_stp(VMtype,GPUtype);
      
      setup_time_t::const_iterator stp_it = tjvg.find(temp_stp);
      if (stp_it != tjvg.cend())
      {
        const Setup& stp = stp_it->first;
        if (stp.get_nGPUs() <= node.get_remainingGPUs())
        {
          // assign
          assigned = true;
          node.set_remainingGPUs(stp.get_nGPUs());

          Schedule sch(stp_it, idx);  
          new_schedule[j] = sch;
        }
      }
    }
  }
  return assigned;
}