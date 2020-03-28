#include "LocalSearch.hpp"

// NICO
void LocalSearch::printer(job_schedule_t& job_schedule)
{
  std::cout << "Job, Node, VM, GPU, nGPUs, max_nGPUs, cost, time, deadline, current_time, TARDINESS" << std::endl;
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
  std::cout << std::endl;
  return;
}

LocalSearch::LocalSearch(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  Greedy_version4(args, d, file_jobs, file_times, file_nodes)
{}

//________________________________________________________________________________________________________________________________
// é la funzione di Greedy, noi modifchiamo solo la fine, ci inseriamo e facciamo la Local Search

job_schedule_t 
LocalSearch::perform_scheduling (unsigned max_random_iter)
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

  std::swap(opened_nodes, nodes);
  last_node_idx = best_lni;

  // STAMPO LA SUA SCHEDULEEE
  std::cout << std::endl << "Schedule di Federica:" << std::endl;
  printer(best_schedule);

  // Perform Local Search

  bool ls_updated = perform_local_search(best_schedule);

  if (ls_updated) // Se migliora la schedule faccio l'update
  {
    best_schedule = local_best_schedule;
    std::cout << "    * Better configuration found via local search * " << std::endl;
  }
  
  //NICO
  //std::cout << "\nBEST SCHEDULE:" << std::endl; 
  //printer(best_schedule);

  std::cout << "\n\t### MINIMUM COST: " << minTotalCost << "; idx: " 
            << best_idx << std::endl;

  return best_schedule;
}

//________________________________________________________________________________________________________________________________
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

  //std::cout << " -* fft:" << fft << std::endl;
  //std::cout << " -* lft:" << lft << std::endl;
  
  return std::max(lft - fft, fft - lft) ;
}

// get the time of the first job to finish
double
LocalSearch::true_first_finish_time(job_schedule_t& job_schedule) const
{
  double fft = INF;

  // cycle over the schedule
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
LocalSearch::evaluate_objective(job_schedule_t& job_schedule)
{
  double fft = find_first_finish_time(job_schedule);
  double lft = find_last_finish_time(job_schedule);
  return (objective_function(job_schedule, fft) + fft - lft) ;
}
*/


// TODO: forse la cancelliamo
/*
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
  std::swap(nodes, opened_nodes);
  if (current_cost < minTotalCost)
  {
    updated = true;
    minTotalCost = current_cost;
    std::swap(new_schedule, best_schedule);
  }
  updated = search_better_schedule(best_schedule)? true:updated;
  if(updated)
  {
    std::swap(best_schedule, local_best_schedule);
    minTotalCost = objective_function(best_schedule,find_first_finish_time(best_schedule));//TODO: non performare questo calcolo se non ho updatato
  }
  else
  {
  std::swap(nodes, opened_nodes);
  }
  return updated;
}
*/


bool
LocalSearch::perform_local_search(job_schedule_t& actual_schedule)
{
  std::cout<< "        PERFORM LOCAL SEARCH\n" << std::endl;//TOREMOVE 
  
  // initialize members
  //previous_best.clear();
  local_best_schedule = actual_schedule;
  initial_schedule = actual_schedule;
  //std::cout<< "- Initial Schedule size: "<< initial_schedule.size()<< std::endl;//TOREMOVE 
  //previous_best.push_back(actual_schedule);

  // evaluate objective function in the actual_schedule
  best_schedule_value_t = evaluate_objective(actual_schedule);
  std::cout<< "- Ho valutato l'obj fun delta, valore = " << best_schedule_value_t << std::endl; 

  // Iniziamo la local search
  unsigned iter = 0;
  bool changed = false;
  bool changed_at_least_once = false;
  bool stop = false;

  while ((iter < MAX_ITER) and !(stop)) //for (unsigned iter = 0; iter < MAX_ITER && !stop; +iter ++)
  {
    std::cout << std::endl;
    std::cout << "- iter " << iter << " of " << MAX_ITER << std::endl;
    //std::cout<< "- sto per entrare in visit_neighbor()"<< std::endl; 
    
    changed = visit_neighbor();
   
    std::cout<< "- uscito da visit_neighbor(). changed?: "<< changed << std::endl;//TOREMOVE 

    std::cout<< "- best_schedule_value_t:  "<< best_schedule_value_t << std::endl; //TOREMOVE 
    
    //_______________________________________________________________
    // TODO: rimuovere
    if(changed)
    {
      std::cout << "\nNuova schedule: " << std::endl;
      printer(local_best_schedule);

      std::cout << std::endl << "Configurazione Nodi: " << std::endl;
      for (unsigned i = 0; i < last_node_idx; ++i)
      {
        Node & nn = nodes[i];
        std::cout << "Node_idx: " << i << ",   ";
        std::cout << "old_stp: " << nn.get_VMtype() << "," << nn.get_GPUtype() << "," << nn.get_usedGPUs() << "," << nn.get_cost() << std::endl;
      }
      std::cout << std::endl;
    }   
    //_______________________________________________________________
 

    changed_at_least_once = (changed == true) ? true : changed_at_least_once;

    //se ho visitato l'intorno ma non ho trovato una opzione miglore
    if (!changed)
    {
      stop = true;
      //break; //usciamo dal while
    }

    /*
    for (const job_schedule_t & i : previous_best)
      if (equalScheule(i, local_best_schedule)) // sono in un ciclo perchè ho ottenuto una funzione già trovata
        {
          stop = true;
          break;
        }
    */
    iter++;
  }

  /*if (changed_at_least_once) //TODO entrare qui
  {
    std::cout << "    * Better configuration found via local search *" << std::endl;
  }
*/
  return changed_at_least_once;
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
  /*
  if (! compare_configuration(best_stp, new_node))
  {
    std::cout << "    ---- assignement failed: incorrect configuration" << std::endl;
  }
  if (best_stp.get_nGPUs() > new_node.get_remainingGPUs())
  {
    std::cout << "    ---- assignement failed: not enough gpu" << std::endl;
  }
  */
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
}

*/
