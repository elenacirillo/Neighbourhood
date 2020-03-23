#include "LocalSearchGPU.hpp"

//________________________________________________________________________________________________________________________________

// constructor
LocalSearchGPU::LocalSearchGPU(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{

  // initialize all_neighborhoods starting from ttime
  initialize_GPUs_setups();

}

//________________________________________________________________________________________________________________________________-

// initialize all_neighborhoods starting from ttime (ttime is a protected member of the parent class Heuristic)
void
LocalSearchGPU::initialize_GPUs_setups(void)
{
  if(! ttime.empty())
  {
    // consider the setups related to the first job (all jobs work with all setups)
    setup_time_t & setup_time = ttime.begin()->second;

    // cycle over all its setups
    for (auto pair: setup_time)
    {
      const Setup & stp = pair.first;
      unsigned max_nGPUs = stp.get_maxnGPUs();
      // TODO: remove cout
      //std::cout << "max_nGPUs=" << max_nGPUs << ",  nGPUs=" << nGPUs << ", time=" << pair.second << std::endl;

      GPUs_setups[max_nGPUs].insert(stp);

    }
  }

  // TODO: remove
  std::cout << "________________________________________________" << std::endl << std::endl;
  std::cout << "PRINTING THE STARTING MAP: <max_nGPU-setups>" << std::endl;
  for(auto pair: GPUs_setups)
  {
    std::cout << "max_nGPU: " << pair.first << ", setups: " << std::endl;
      for(auto s: pair.second)
      {
        s.print(std::cout);
      }
      std::cout << std::endl;
  }
  std::cout << "________________________________________________" << std::endl;

}

//________________________________________________________________________________________________________________________________


// update the data structure node_jobs acordingly to initial_schedule
void
LocalSearchGPU::update_node_jobs(void)
{
  node_jobs.clear();

  // cycle over the initial schedule
  // Inserisco controllo che la schedule non sia vuota
  for(auto pair: initial_schedule)
  {
    const Job & job = pair.first;
    Schedule & sch = pair.second;
    if(!sch.isEmpty())
    {
      unsigned node_idx = sch.get_node_idx();

      // insert the job in the unordered_set associated with the node idx
      node_jobs[node_idx].insert(job);
    }

  }
}

//________________________________________________________________________________________________________________________________
/*
--------------- VERSIONE ALTERNATIVA ---------------
Qui anzichè avere una struttura dati neighborhood eccetera
Scorro direttamente i nodi e poi i loro possibili setup.
é più pulito il codice e non abbiamo più generate_neighbor, 
però forse si perde l'idea di intorno dentro il codice.
*/


/*
// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU::visit_neighbor()
{
  // true if the function finds a better schedule and updates it
  bool changed = false;

  // update the data structure that links the nodes to the jobs ("change_GPU" needs it updated)
  update_node_jobs();

  // neighborhood to be explored
  //neighborhood_t neighborhood = generate_neighborhood();

  // indexes of the nodes to explore
  std::list<unsigned> nodes_to_change = get_nodes_to_change(); //NEL CASO è DA SCRIVERE

  // cycle over the nodes to change
  for (unsigned n_idx: nodes_to_change)
  {
    // node
    Node & n = nodes[n_idx];

    // maxnGPUs of that node
    unsigned max_nGPUs = n.get_usedGPUs() + n.get_remainingGPUs(); // forse solo get_usedGPUs!!

    // get the possible setups
    std::list<Setup> possible_setups = GPUs_setups[max_nGPUs];

    // cycle over the possible setups
    for (Setup stp: possible_setups)
    {
      // Now I am in a "point" of the neighborhood

      // modify the initial schedule accordingly to neighbor infos
      job_schedule_t candidate_schedule = change_GPU(node_idx, new_stp);

      // evaluate the objective in this point of the neighborhood
      double candidate_value = evaluate_objective(candidate_schedule);

      // if this schedule improves the objective function
      if(candidate_value < best_schedule_value_t)
      {
        // update the best schedule and the best value
        local_best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;

        // --------------------------------------------------
        // update the configuration of the node accordingly

        // GPU used with the old configuration (deve rimanere invariato nel cambio di VM)
        unsigned used_GPUs = nodes[node_idx].get_usedGPUs();

        // cambio la configuration del nodo a partire da quella del neighbor
        nodes[node_idx].change_setup(new_stp);

        // ora però devo aggiornare il numero di GPU in uso su questo nodo
        nodes[node_idx].set_remainingGPUs(used_GPUs);

        // --------------------------------------------------

        // if I'm not performing a best improvement search
        if(!best_fit)
        {
          return changed; // il break non andrebbe più bene, perchè ho 2 cicli
        }

      }

    }

  }
}
*/

//________________________________________________________________________________________________________________________________

// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU::visit_neighbor()
{
  // true if the function finds a better schedule and updates it
  bool changed = false;

  // update the data structure that links the nodes to the jobs ("change_GPU" needs it updated)
  update_node_jobs();

  // neighborhood to be explored
  neighborhood_t neighborhood = generate_neighborhood();

  // visit the neighbourhood
  for(auto neighbor: neighborhood)
  {
    // a neighbor is made of a pair <node_idx, Setup>
    unsigned node_idx = neighbor.first;
    Setup & new_stp = neighbor.second;

    // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
    job_schedule_t candidate_schedule = change_GPU(node_idx, new_stp);

    // evaluate the objective in this point of the neighborhood
    double candidate_value = evaluate_objective(candidate_schedule);

    // if this schedule improves the objective function
    if(candidate_value < best_schedule_value_t)
    {
      // update the best schedule and the best value
      local_best_schedule = candidate_schedule;
      best_schedule_value_t = candidate_value;
      changed = true;

      // --------------------------------------------------
      // update the configuration of the node accordingly

      // GPU used with the old configuration (deve rimanere invariato nel cambio di VM)
      unsigned used_GPUs = nodes[node_idx].get_usedGPUs();

      // cambio la configuration del nodo a partire da quella del neighbor
      nodes[node_idx].change_setup(new_stp);

      // ora però devo aggiornare il numero di GPU in uso su questo nodo
      nodes[node_idx].set_remainingGPUs(used_GPUs);

      // --------------------------------------------------

      // if I'm not performing a best improvement search
      if(!best_fit)
      {
        break;
      }

    }

  }

  return changed;

}


//________________________________________________________________________________________________________________________________

// create a new schedule by changing the type of GPU on node node_idx and updating the schedules of the jobs on that node
// input: temp_stp, va modificato per ogni job aggiornando nGPUs
job_schedule_t
LocalSearchGPU::change_GPU(unsigned node_idx, const Setup & temp_stp)
{
  // new schedule to modify
  job_schedule_t new_schedule = initial_schedule;

  // get the jobs in execution on the node "node_idx"
  std::unordered_set<Job> jobs_to_modify = node_jobs[node_idx];

  // cycle over these jobs
  for(Job j: jobs_to_modify)
  {
    // schedule of job j
    Schedule & sch = new_schedule[j];

    // old setup
    const Setup & old_stp = sch.get_setup();

    // if the setup is the same return
    //if(temp_stp == old_stp)
    //  break;

    // old number of GPU used
    unsigned nGPUs = old_stp.get_nGPUs(); // questa info è diversa per ogni job!

    // row to initialize a new setup
    row_t build_stp;
    build_stp.push_back(temp_stp.get_VMtype());
    build_stp.push_back(temp_stp.get_GPUtype());
    build_stp.push_back(std::to_string(nGPUs));
    build_stp.push_back(std::to_string(temp_stp.get_maxnGPUs()));
    build_stp.push_back(std::to_string(temp_stp.get_cost()));

    // new setup
    Setup new_stp(build_stp);

    // get the setup infos related to job "j" and "new_stp"
    setup_time_t::const_iterator c_newsetup = ttime[j.get_ID()].find(new_stp);

    // modify the schedule of job j according to the new setup
    sch.change_setup(c_newsetup);
  }

  return new_schedule;

}

//________________________________________________________________________________________________________________________________

// generate the neighborhood of initial_schedule
neighborhood_t
LocalSearchGPU::generate_neighborhood(void)
{
  // neighbourhood
  neighborhood_t neighbourhood;

  // nodes with the highest number of jobs in tardiness
  std::set<unsigned> tochange = top_tardiness_nodes(neigh_size);

  // If no nodes have jobs in tardiness
  if (tochange.empty())
  {
    // I take the first neigh_size nodes
    for(int i=0; i < std::min<unsigned long>(nodes.size()-1,neigh_size); ++i)
    {
      tochange.insert(i);
    }
  }

  // build the neighbourhood of each node
  for(unsigned n: tochange)
  {
    // max num of GPU for the current node
    unsigned max_nGPUs = nodes[n].get_usedGPUs() + nodes[n].get_remainingGPUs();

    // possible setups with the same max num of GPU
    std::unordered_set<Setup> stps = GPUs_setups[max_nGPUs];

    // cycle over the setups
    for(const Setup & stp: stps)
    {
      // insert in the neighborhood
      neighbourhood.insert({n,stp}); // setup
    }

  }

  return neighbourhood;

}

//________________________________________________________________________________________________________________________________

// returns the indexes of the neigh_size nodes with the highest number of jobs in tardiness;
std::set<unsigned> LocalSearchGPU::top_tardiness_nodes(unsigned top)
{
  // DA RISCRIVERE, ORA LA STO RESTITUENDO VUOTA PER FAR GIRARE IL PROGRAMMA COMUNQUE

  /*
  // TODO here I should add a comparator ot order by number of jobs in tardiness
  // map <node_idx, numero di job in tardiness>
  std::map<unsigned,unsigned> topnodes;
  //count the jobs in each node that have tardiness>0
  for (auto js:initial_schedule)
  {
    if(!js.second.isEmpty())
    {
      if (js.second.get_tardiness()>0)
      topnodes[js.second.get_node_idx()]++;
    }
  }
  std::set<unsigned> result;
  // da sistemare, devo inserire solo i primi top
  for(auto node_count:topnodes)
  {
    result.insert(node_count.first);
  }
  */
  std::set<unsigned> result;

  return result;
}

//________________________________________________________________________________________________________________________________
