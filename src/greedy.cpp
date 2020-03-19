#include "greedy.hpp"

Greedy::Greedy (const std::string& args, const std::string& d, 
                const std::string& file_jobs, const std::string& file_times, 
                const std::string& file_nodes):
  Heuristic(args, d, file_jobs, file_times, file_nodes)
{}

bool
Greedy::compare_pressure (const Job& j1, const Job& j2)
{
  return j1.get_pressure() > j2.get_pressure();
}

bool
Greedy::compare_configuration (const Setup& stp, const Node& n) const
{
  return (stp.get_VMtype() == n.get_VMtype() && 
          stp.get_GPUtype() == n.get_GPUtype());
}

void
Greedy::sort_jobs_list (void)
{
  // sort list
  submitted_jobs.sort(compare_pressure);
}

void
Greedy::random_swap (void)
{
  // print sorted queue before random swap
  //std::cout << "sorted queue: ";
  //for (const Job& j : submitted_jobs)
  //  std::cout << j.get_ID() << "; ";
  //std::cout << std::endl;

  // random swap
  std::bernoulli_distribution distribution; // TODO: set p
  std::list<Job>::iterator it = submitted_jobs.begin();
  for (unsigned idx = 0; idx < submitted_jobs.size(); ++idx)
  {
    std::list<Job>::iterator current_it = it;
    it++;
    double w1 = current_it->get_tardinessWeight();
    double w2 = it->get_tardinessWeight();
    double p = (w1 > w2) ? (0.5 + pi) : (0.5 - pi);
    distribution.param(std::bernoulli_distribution::param_type(p));
    bool swap = distribution(generator);
    if (it != submitted_jobs.end() && swap)
      std::swap(*current_it, *it);
  }
}

double
Greedy::objective_function (job_schedule_t& new_schedule, double elapsed_time)
{
  double vmCost = 0.;
  double tardiCost = 0.;
  double worstTardiCost = 0.;
  double sim_time = current_time + elapsed_time;

  // loop over last schedule
  for (job_schedule_t::iterator it = new_schedule.begin();
       it != new_schedule.end(); ++it)
  {
    const Job& j = it->first;
    Schedule& sch = it->second;

    // if schedule is not empty, compute cost of VM and tardiness cost
    if (! sch.isEmpty())
    {
      // set execution time
      sch.set_executionTime(elapsed_time);
      
      // get number of used GPUs on the current node
      unsigned node_idx = sch.get_node_idx();
      unsigned g = nodes[node_idx].get_usedGPUs();

      // compute cost of VM
      sch.compute_vmCost(g);
      vmCost += sch.get_vmCost();

      // compute tardiness
      double tardiness = std::max(sim_time - j.get_deadline(), 0.);
      sch.set_tardiness(tardiness);
      sch.compute_tardinessCost(j.get_tardinessWeight());
      tardiCost += sch.get_tardinessCost();
    }
    else
    {
      double worstCaseTardiness = 100 * j.get_maxExecTime() * 
                                  j.get_tardinessWeight();
      worstTardiCost += worstCaseTardiness;
    }
  }

  //std::cout << "\tvmCost: " << vmCost << "; tardiCost: " << tardiCost
  //          << "; worstCaseTardiness: " << worstTardiCost << std::endl;
  //std::cout << "," << vmCost << "," << tardiCost
  //          << "," << worstTardiCost << std::endl;
  return (vmCost + tardiCost + worstTardiCost);
}

bool
Greedy::assign_to_existing_node (const Job& j, 
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
      assigned = true;
      node.set_remainingGPUs(best_stp.get_nGPUs());

      Schedule sch(best_stp_it, idx);
      new_schedule[j] = sch;
    }
  }
  return assigned;
}

void
Greedy::assign_to_new_node (const Job& j, 
                            setup_time_t::const_iterator best_stp_it,
                            job_schedule_t& new_schedule)
{
  const Setup& best_stp = best_stp_it->first;

  // open a new node with the required configuration
  Node& n = nodes[last_node_idx];
  n.open_node(best_stp);
  n.set_remainingGPUs(best_stp.get_nGPUs());

  // assign
  Schedule sch(best_stp_it, last_node_idx);
  new_schedule[j] = sch;

  // update index of last used node
  last_node_idx++;
}

setup_time_t::const_iterator
Greedy::select_setup_in_range (range_t range, const Node& node, 
                               unsigned g) const
{
  unsigned GPUs_on_node = node.get_usedGPUs() + node.get_remainingGPUs();

  bool found = false;
  setup_time_t::const_iterator new_stp_it = range.first;
  while (new_stp_it != range.second && ! found)
  {
    const Setup& new_stp = new_stp_it->first;
    if (new_stp.get_nGPUs() == g && new_stp.get_maxnGPUs() == GPUs_on_node)
      found = true;
    else
      ++new_stp_it;
  }

  return new_stp_it;
}

void
Greedy::change_nodes_configurations (const job_schedule_t& current_schedule)
{
  const Job& temp_j = current_schedule.cbegin()->first;
  const setup_time_t& temp_tjvg = ttime.at(temp_j.get_ID());
  
  for (unsigned node_idx = 0; node_idx < last_node_idx; ++node_idx)
  {
    Node& node = nodes[node_idx];
    if (node.get_remainingGPUs() > 0)
    {
      double minCost = node.get_cost();
      setup_time_t::const_iterator new_it = temp_tjvg.cend();
      
      for (setup_time_t::const_iterator temp_it = temp_tjvg.cbegin();
           temp_it != temp_tjvg.cend(); ++temp_it)
      {
        const Setup& temp_stp = temp_it->first;
        bool checkGPUtype = (temp_stp.get_GPUtype() == node.get_GPUtype());
        bool checkGPUs_1 = (temp_stp.get_maxnGPUs() == node.get_usedGPUs());
        bool checkGPUs_2 = (temp_stp.get_maxnGPUs() == node.get_usedGPUs()+1);
        bool checkCost = (temp_stp.get_cost() < minCost);
        if (checkGPUtype && (checkGPUs_1 || checkGPUs_2) && checkCost)
        {
          minCost = temp_stp.get_cost();
          new_it = temp_it;
        }
      }

      if (new_it != temp_tjvg.cend())
      {
        const Setup& new_stp = new_it->first;
        node.change_setup(new_stp);
      }
    }
  }  
}

void
Greedy::perform_assignment (const Job& j, job_schedule_t& new_schedule)
{
  // determine the setups s.t. deadline is matched
  const setup_time_t& tjvg = ttime.at(j.get_ID());

  Dstar dstar(j, tjvg, current_time);
  
  if (! full_greedy)
    dstar.set_random_parameter(alpha);

  // determine the best setup:
  //   if it is possible to match deadline, the best setup is the 
  //   cheapest; otherwise, the best setup is the fastest
  setup_time_t::const_iterator best_stp;
  best_stp = dstar.get_best_setup(generator);

  // check the already opened nodes...
  bool assigned = assign_to_existing_node(j, best_stp, new_schedule);

  // if the already opened nodes are not suitable...
  if (! assigned)
  {
    // if it is possible, open a new node with the optimal configuration
    // else, assign to an available suboptimal configuration
    if (last_node_idx < nodes.size())
    { 
      assigned = true;
      assign_to_new_node(j, best_stp, new_schedule);
    }
    else
      assigned = assign_to_suboptimal(j, tjvg, dstar, new_schedule);
  }

  // if job j cannot be assigned to any configuration, add an empty 
  // schedule
  if (!assigned)
    new_schedule[j] = Schedule(); 
}

job_schedule_t 
Greedy::perform_scheduling (unsigned max_random_iter)
{
  // (STEP #0)
  preprocessing();

  // initialization of minimum total cost, best schedule and corresponding
  // index by a step of pure greedy
  full_greedy = true;
  job_schedule_t best_schedule;
  scheduling_step(best_schedule);
  double fft = find_first_finish_time(best_schedule);
  double minTotalCost = objective_function(best_schedule,fft);
  full_greedy = false;
  
  std::vector<Node> opened_nodes(nodes);
  unsigned best_lni = last_node_idx;
  unsigned best_idx = 0;

  // random iterations
  for (unsigned random_iter = 1; random_iter < max_random_iter; 
       ++random_iter)
  {
    std::cout << "\n\tRANDOM ITERATION " << random_iter << std::endl;
    //std::cout << "," << random_iter;

    // determine new schedule
    job_schedule_t new_schedule;
    scheduling_step(new_schedule, true);

    // update best schedule according to the value of the objective
    bool updated = update_best_schedule(new_schedule, minTotalCost, 
                                        best_schedule, opened_nodes);
    if (updated)
    {
      best_idx = random_iter;
      best_lni = last_node_idx;
    }
  }

  std::cout << "\n\t### MINIMUM COST: " << minTotalCost << "; idx: " 
            << best_idx << std::endl;

  std::swap(opened_nodes, nodes);
  last_node_idx = best_lni;

  return best_schedule;
}

void
Greedy::preprocessing (void)
{
  // update minimum execution time, maximum execution time and pressure of 
  // all submitted jobs
  for (Job& j : submitted_jobs)
  {
    update_minMax_exec_time(j);
    j.update_pressure(current_time);
  }

  // sort the queue
  sort_jobs_list();
}

void
Greedy::scheduling_step (job_schedule_t& new_schedule, bool r_swap)
{
  if (! full_greedy)
  {
    if (r_swap)
    {
      sort_jobs_list();
      random_swap();
    }
    close_nodes();
  }

  std::string queue = "";

  // perform assignment of all submitted jobs (STEP #1)
  for (const Job& j : submitted_jobs)
  {
    queue += (j.get_ID() + "; ");
    perform_assignment(j, new_schedule);
  }

  // perform postprocessing (STEP #2)
  postprocessing(new_schedule);

  //std::cout << "\tn_submitted_jobs: " << submitted_jobs.size()
  //          << "; queue: " << queue
  //          << "; n_used_nodes: " << last_node_idx << std::endl;
}

void
Greedy::postprocessing (job_schedule_t& current_schedule)
{
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
    }
  }
}

bool
Greedy::update_best_schedule (job_schedule_t& new_schedule,
                              double& minTotalCost, 
                              job_schedule_t& best_schedule,
                              std::vector<Node>& opened_nodes)
{
  bool updated = false;

  // find execution time of first ending job
  double first_finish_time = find_first_finish_time(new_schedule);

  // compute cost of current schedule
  double current_cost = objective_function(new_schedule, first_finish_time);

  // determine best schedule
  if (current_cost < minTotalCost)
  {
    updated = true;
    minTotalCost = current_cost;
    std::swap(new_schedule, best_schedule);
    std::swap(nodes, opened_nodes);
  }

  return updated;
}
