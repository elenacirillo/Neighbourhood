#include "LocalSearch.hpp"

LocalSearch::LocalSearch(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  Greedy_version4(args, d, file_jobs, file_times, file_nodes)
{}



double
LocalSearch::evaluate_objective(job_schedule_t& job_schedule) const
{
  double fft = true_first_finish_time(job_schedule);
  double lft = true_last_finish_time(job_schedule);
  return std::max(lft - fft, fft - lft) ;
}

// get the time of the first job to finish
double
LocalSearch::true_first_finish_time(job_schedule_t& job_schedule) const
{
  double fft = INF;

  job_schedule_t::const_iterator cit;
  for (cit = job_schedule.cbegin(); cit != job_schedule.cend(); ++cit)
  {
    const Schedule & sch = cit->second;
    double time_needed = sch.get_selectedTime(); // overall time needed to execute the job
    double completion_percent = sch.get_completionPercent(); // speriamo sia decimale
    double time_remaining = (1-completion_percent) * time_needed;

    fft = std::min(fft, time_remaining);
  }

  return fft;
}

// get the time of the last job to finish
double
LocalSearch::true_last_finish_time(job_schedule_t& job_schedule) const
{
  double lft = 0;

  job_schedule_t::const_iterator cit;
  for (cit = job_schedule.cbegin(); cit != job_schedule.cend(); ++cit)
  {
    const Schedule & sch = cit->second;
    double time_needed = sch.get_selectedTime(); // overall time needed to execute the job
    double completion_percent = sch.get_completionPercent(); // speriamo sia decimale
    double time_remaining = (1-completion_percent) * time_needed;

    lft = std::max(lft, time_remaining);
  }

  return lft;
}



/*
double
LocalSearch::evaluate_objective(job_schedule_t& job_schedule, double elapsed_time)
{
  double fft = find_first_finish_time(job_schedule);
  double lft = find_last_finish_time(job_schedule);
  return (objective_function(job_schedule, elapsed_time) + fft - lft) ;
}
*/
bool
LocalSearch::update_best_schedule (job_schedule_t& new_schedule,
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
  updated = search_better_schedule(best_schedule)? true:updated;
  std::swap(best_schedule, this->best_schedule);
  minTotalCost = objective_function(best_schedule,find_first_finish_time(best_schedule));

  return updated;
}

bool
LocalSearch::search_better_schedule(job_schedule_t& actual_schedule)
{

  if(actual_schedule.size() < neigh_size)
  {
    std::cout<< "Ce ne stan troppo poghi"<< std::endl; 
    return false;
  }
  std::cout<< "SEI DENTRO SEARCH BETTER SCHEDULE"<< std::endl; 
  previous_best.clear();
  best_schedule = actual_schedule;
  initial_schedule = actual_schedule;
  std::cout<< "INITIAL SCHEDULE HA SIZE: "<< initial_schedule.size()<< std::endl; 
  previous_best.push_back(actual_schedule);
   /*double elapsed_time = std::min(
                                scheduling_interval,
                                first_finish_time
                              );
   best_schedule_value_t = evaluate_objective(actual_schedule, );
   */
  best_schedule_value_t = evaluate_objective(actual_schedule);
  std::cout<< "HO CALCOLATO LA OBJ"<< std::endl; 

  unsigned iter = 0;
  bool changed = false;
  bool stop = false;
  while ((iter < MAX_ITER) and !(stop))
  {
    std::cout<< "STO PER VISITARE...."<< std::endl; 
    
    changed = visit_neighbor();


    std::cout<< "HO VISITATOOOO"<< std::endl; 

    std::cout<< "DELTA VALUE = "<< best_schedule_value_t << std::endl; 

    // se visit_neighbor è true, changed diventa true, altrimenti rimane quello che era prima

    //se ho visitato l'intorno ma non ho trovato una opzione miglore
    if (!changed)
      {
        break; //usciamo dal while
      }
    //controllo che la best appena trovata (quindi solo se changed = true) non sia una che avevo già trovato
    // questo può succedere solo se usiamo come objective function la funzione totale, mentre con il solo delta (differenza dei finisch time)
    // non aggiorniamo se non è strettamente migliroante

    /*
    for (const job_schedule_t & i : previous_best)
      if (i == best_schedule) // sono in un ciclo perchè ho ottenuto una funzione già trovata
        {
          stop = true;
          break;
        }
    */
    iter++;
  }
  if (changed) //TODO entrare qui
  {
    std::cout << "better configuration found via local search" << std::endl;
  }
  return changed;
}


bool
LocalSearch::assign_to_selected_node (const Job& j,
                          setup_time_t::const_iterator best_stp_it,
                          job_schedule_t& new_schedule,
                          unsigned node_index)
{
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