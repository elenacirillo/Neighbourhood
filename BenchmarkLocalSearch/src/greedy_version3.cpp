#include "greedy_version3.hpp"

Greedy_version3::Greedy_version3 (const std::string& args, 
                                  const std::string& d, 
                                  const std::string& file_jobs, 
                                  const std::string& file_times, 
                                  const std::string& file_nodes):
  Greedy_version2(args, d, file_jobs, file_times, file_nodes)
{}

void
Greedy_version3::postprocessing (job_schedule_t& current_schedule)
{
  // highest speed-up pair
  std::pair<Job, setup_time_t::const_iterator> best_speedup;
  double previous_delta = 0.;

  // change configuration of nodes with idle GPUs
  change_nodes_configurations(current_schedule);

  // loop over the current schedule
  for (job_schedule_t::iterator schit = current_schedule.begin();
       schit != current_schedule.end(); ++schit)
  {
    const Job& j = schit->first;
    Schedule& sch = schit->second;
    const setup_time_t& tjvg = ttime.at(j.get_ID());

    // if the schedule is not empty
    if (! sch.isEmpty())
    {
      unsigned node_idx = sch.get_node_idx();
      const Setup& stp = sch.get_setup();

      Node& node = nodes[node_idx];
      Setup node_stp(node.get_VMtype(), node.get_GPUtype());

      // if the configuration of node has been changed, change the current 
      // schedule accordingly
      if (stp != node_stp)
      {
        range_t range = tjvg.equal_range(node_stp);

        setup_time_t::const_iterator new_stp_it;
        new_stp_it = select_setup_in_range (range, node, stp.get_nGPUs());
        
        if (new_stp_it != range.second)
        {
          const Setup& new_stp = new_stp_it->first;
          node.set_remainingGPUs(new_stp.get_nGPUs());
          sch.change_setup(new_stp_it);
        }
      }

      // if there are still idle GPUs in the current configuration, determine 
      // the job that would get the highest speed-up by getting the 
      // additional resources
      if (node.get_remainingGPUs() > 0)
      {
        const Setup& stp = sch.get_setup();

        range_t range = tjvg.equal_range(node_stp);

        unsigned new_nGPUs = stp.get_nGPUs() + node.get_remainingGPUs();
        setup_time_t::const_iterator new_stp_it;
        new_stp_it = select_setup_in_range (range, node, new_nGPUs);

        if (new_stp_it != range.second)
        {
          double delta = sch.get_selectedTime() - new_stp_it->second;

          if (delta > previous_delta)
          {
            previous_delta = delta;
            best_speedup = {j, new_stp_it};
          }
        }
      }
    }
  }

  // assign the additional resources to the job with highest speed-up
  if (previous_delta > 0.)
  {
    const Job& temp_j = best_speedup.first;
    setup_time_t::const_iterator new_stp_it = best_speedup.second;
    job_schedule_t::iterator schit = current_schedule.find(temp_j);
    if (schit != current_schedule.end())
    {
      Schedule& sch = schit->second;
      unsigned node_idx = sch.get_node_idx();
      Node& node = nodes[node_idx];
      const Setup& new_stp = new_stp_it->first;
      node.set_remainingGPUs(new_stp.get_nGPUs());
      sch.change_setup(new_stp_it);
    }
  }
}
