#include "greedy_version4.hpp"

Greedy_version4::Greedy_version4 (const std::string& args, 
                                  const std::string& d, 
                                  const std::string& file_jobs, 
                                  const std::string& file_times, 
                                  const std::string& file_nodes):
  Greedy_version3(args, d, file_jobs, file_times, file_nodes)
{}

unsigned
Greedy_version4::select_best_node (const Setup& best_stp)
{
  // map of opened nodes, sorted by the level of saturation
  std::multimap<unsigned, unsigned> sorted_nodes;
  unsigned best_idx = last_node_idx;

  for (unsigned idx = 0; idx < last_node_idx; ++idx)
  {
    const Node& node = nodes[idx];

    // compute the difference between the number of idle GPUs on the 
    // current node and the number of required GPUs
    int current_diff = node.get_remainingGPUs() - best_stp.get_nGPUs();

    // if the best setup fits the current node, insert the pair in the map
    if (compare_configuration(best_stp, node) && current_diff >= 0)
      sorted_nodes.insert({current_diff, idx});
  }

  // select the best index
  if (sorted_nodes.size() > 0)
    best_idx = random_select(sorted_nodes, generator, beta);

  return best_idx;
}

bool
Greedy_version4::assign_to_existing_node (const Job& j, 
                                    setup_time_t::const_iterator best_stp_it,
                                    job_schedule_t& new_schedule)
{
  bool assigned = false;
  const Setup& best_stp = best_stp_it->first;

  unsigned best_idx = select_best_node(best_stp);
  
  if (best_idx < last_node_idx)
  {
    Node& node = nodes[best_idx];
    assigned = true;
    node.set_remainingGPUs(best_stp.get_nGPUs());

    Schedule sch(best_stp_it, best_idx);
    new_schedule[j] = sch;
  }

  return assigned;
}