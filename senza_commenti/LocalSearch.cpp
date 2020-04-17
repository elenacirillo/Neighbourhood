#include "LocalSearch.hpp"

LocalSearch::LocalSearch(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  Greedy_version4(args, d, file_jobs, file_times, file_nodes)
{}

job_schedule_t 
LocalSearch::perform_scheduling (unsigned max_random_iter)
{
  // (STEP #0)
  preprocessing();

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

  std::swap(opened_nodes, nodes);
  last_node_idx = best_lni;

  //double temp = objective_function(best_schedule, find_first_finish_time(best_schedule));
  double temp = evaluate_objective(best_schedule);
  
  // Perform Local Search
  bool ls_updated = perform_local_search(best_schedule);

  if (ls_updated) // Se migliora la schedule faccio l'update
  {

    best_schedule = local_best_schedule;
    std::cout << "    * Better configuration found via local search * " << std::endl;
    ++LS_count;

    std::ofstream ofs;
    ofs.open("../TEST_OUTPUTS_2.csv",std::ios_base::app);
    //ofs << "\n" << temp << ", " << objective_function(best_schedule, find_first_finish_time(best_schedule));
    ofs << "\n" << temp << ", " << evaluate_objective(best_schedule);
    ofs.close();

  }
  else 
  {
      std::ofstream ofs;
      ofs.open("../TEST_OUTPUTS_2.csv",std::ios_base::app);
      ofs << "\n 0.0,";
      ofs.close();
  }

  std::cout << "\n\t### MINIMUM COST: " << minTotalCost << "; idx: " 
            << best_idx << std::endl;

  return best_schedule;
}

double
LocalSearch::find_last_finish_time (const job_schedule_t& last_schedule) const
{
  double lft = 0.;

  job_schedule_t::const_iterator cit;
  for (cit = last_schedule.cbegin(); cit != last_schedule.cend(); ++cit)
    lft = std::max(lft, (cit->second).get_selectedTime());

  return lft;
}

double
LocalSearch::evaluate_objective(job_schedule_t& job_schedule) const
{
  double fft = find_first_finish_time(job_schedule);
  double lft = find_last_finish_time(job_schedule);

  return std::max(lft - fft, fft - lft) ;
}

bool
LocalSearch::perform_local_search(job_schedule_t& actual_schedule)
{ 
  local_best_schedule = actual_schedule;
  best_schedule_value_t = evaluate_objective(actual_schedule);

  unsigned iter = 0;
  bool changed = true;
  bool changed_at_least_once = false;

  while ((iter < MAX_ITER) and changed) 
  {
    changed = visit_neighbor();
    changed_at_least_once = (changed == true) ? true : changed_at_least_once;
    iter++;
  }
  return changed_at_least_once;
}


bool
LocalSearch::assign_to_selected_node (const Job& j,
                          job_schedule_t& new_schedule,
                          unsigned node_index)
{
  const setup_time_t &tjvg = ttime.at(j.get_ID());
  Dstar dstar(j, tjvg, current_time);
  if (!full_greedy)
    dstar.set_random_parameter(alpha);
  setup_time_t::const_iterator best_stp_it = dstar.get_best_setup(generator);
  bool assigned = false;
  const Setup& best_stp = best_stp_it->first;

  Node& new_node = nodes[node_index];
  if (compare_configuration(best_stp, new_node) &&
        best_stp.get_nGPUs() <= new_node.get_remainingGPUs())
  {
    assigned = true;
    new_node.set_remainingGPUs(best_stp.get_nGPUs());
    Schedule sch(best_stp_it, node_index);
    new_schedule[j] = sch;
  }
  return assigned;
}


bool
LocalSearch::update_schedule (Schedule & old_sch, Schedule & new_sch)
{
  new_sch.set_simTime(old_sch.get_simTime());
  new_sch.set_completionPercent(old_sch.get_completionPercent());
  new_sch.set_cP_step(old_sch.get_cP_step()); // dovrebbe essere zero.
  new_sch.set_tardiness(old_sch.get_tardiness());
  return true;
}


bool
LocalSearch::equalSchedule(const Schedule & lhs, const Schedule & rhs)
{
  if (lhs.isEmpty() == rhs.isEmpty())
    return 1;

  return (lhs.get_startTime() == rhs.get_startTime()
      && lhs. get_completionPercent() == rhs.get_completionPercent()
      && lhs. get_startTime() == rhs.get_startTime()
      && lhs.get_cP_step() == rhs.get_cP_step()
      && lhs.get_simTime() == rhs.get_simTime()
      && lhs.get_tardiness() == rhs.get_tardiness()
      && lhs.get_vmCost() == rhs.get_vmCost()
      && lhs.get_tardinessCost() == rhs.get_tardinessCost()
      && lhs.get_setup() == rhs.get_setup()
      && lhs.get_selectedTime() == rhs.get_selectedTime()
      && lhs.get_node_idx() == rhs.get_node_idx());
}

/* NUOVA VERSIONE ASSEGNAMENTO A NODO CHE FA ANCHE IL CONTROLLO SUL SUBOTTIMO
bool
LocalSearch::assign_to_selected_node (const Job& J,
                          job_schedule_t& new_schedule,
                          unsigned node_index)
{
    Node& new_node = nodes[node_index];
    bool assigned = false;
    
    const setup_time_t& tjvg = ttime.at(J.get_ID());
    Dstar dstarA (J, tjvg, current_time);
    if (! full_greedy)
        dstar.set_random_parameter(alpha);

    while (!dstar.is_end() && !assigned)
    {
        setup_time_t::const_iterator best_stp = dstar.get_best_setup(generator);
        const Setup& best_stp = best_stp_it->first;
        if (compare_configuration(best_stp, new_node) &&
            best_stp.get_nGPUs() <= new_node.get_remainingGPUs())
        {
            assigned = true;
            new_node.set_remainingGPUs(best_stp.get_nGPUs());
            Schedule sch(best_stp_it, node_index);
            new_schedule[j] = sch;
        }
    }
  return assigned;
} */

void
LocalSearch::printer(job_schedule_t& job_schedule)
{
  std::cout << "Job, Node, VM, GPU, nGPUs, max_nGPUs, cost, time, deadline, current_time, tardiness" << std::endl;
  for (const auto & js: job_schedule)
  {
    const Job & j = js.first;
    const Schedule & sch = js.second;
    const Setup & stp = sch.get_setup();
    std::cout << j.get_ID() << ", " << sch.get_node_idx() << ", " << stp.get_VMtype() << ", "
              << stp.get_GPUtype() << ", " <<  stp.get_nGPUs() << ", " << stp.get_maxnGPUs() << ", "
              << stp.get_cost() << ", " << sch.get_selectedTime() << ", "
              << j.get_deadline() << ", " << current_time << ", "
              << (j.get_deadline() < current_time)
              << std::endl;
  }
  return;
}