#include "LocalSearchGPU1.hpp"

//________________________________________________________________________________________________________________________________

// constructor
LocalSearchGPU1::LocalSearchGPU1(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearchGPU(args, d, file_jobs, file_times, file_nodes)
{
  // initialize all_neighborhoods starting from ttime
  initialize_GPUs_setups();
}

//________________________________________________________________________________________________________________________________-


// initialize all_neighborhoods starting from ttime (ttime is a protected member of the parent class Heuristic)
void
LocalSearchGPU1::initialize_GPUs_setups(void)
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
// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU1::visit_neighbor()
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

    // TODO: rimuovere
    //std::cout << std::endl << "# Sto visitando il nodo: " << node_idx << std::endl;
    //Node & nn = nodes[node_idx];
    //std::cout << "# old_stp: " << nn.get_VMtype() << "," << nn.get_GPUtype() << "," << nn.get_usedGPUs() << "," << nn.get_cost() << std::endl;
    //std::cout << "# new_stp: "; new_stp.print(std::cout);

    // if the point of the neighbor doesn't coincide with the one where I am
    if (new_stp.get_VMtype()  != nodes[node_idx].get_VMtype() &&
        new_stp.get_GPUtype() != nodes[node_idx].get_GPUtype())
    {
      // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
      job_schedule_t candidate_schedule = change_GPU(node_idx, new_stp);

      //std::cout << std::endl << "change_GPU candidate_schedule:\n";
      //printer(candidate_schedule);

      // evaluate the objective in this point of the neighborhood
      double candidate_value = evaluate_objective(candidate_schedule);

      //std::cout << "Obj function candidate_value: " << candidate_value << std::endl;

      // if this schedule improves the objective function
      if(candidate_value < best_schedule_value_t)
      {
        // update the best schedule and the best value
        local_best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;

        //printer(local_best_schedule);

        // update the configuration of the node accordingly:

        // GPU used with the old configuration (deve rimanere invariato nel cambio di VM)
        unsigned used_GPUs = nodes[node_idx].get_usedGPUs();

        // cambio la configuration del nodo a partire da quella del neighbor
        nodes[node_idx].change_setup(new_stp);

        // ora però devo aggiornare il numero di GPU in uso su questo nodo
        nodes[node_idx].set_remainingGPUs(used_GPUs);

        //std::cout <<std::endl << "Ho aggiornato la config del nodo" << std::endl;
        //Node & nn = nodes[node_idx];
        //std::cout << "new_stp: " << nn.get_VMtype() << "," << nn.get_GPUtype() << "," << nn.get_usedGPUs() << "," << nn.get_cost() << "\n\n";  

        // if I'm not performing a best improvement search
        if(!best_fit)
        {
          break;
        }

      }

    } // end if

  } // end for

  return changed;

}

//________________________________________________________________________________________________________________________________

// create a new schedule by changing the type of GPU on node node_idx and updating the schedules of the jobs on that node
// input: new_stp, va modificato per ogni job aggiornando nGPUs
job_schedule_t
LocalSearchGPU1::change_GPU(unsigned node_idx, const Setup & new_stp)
{
  // new schedule to modify
  job_schedule_t new_schedule = local_best_schedule;

  // get the jobs in execution on the node "node_idx"
  std::unordered_set<Job> jobs_to_modify = node_jobs[node_idx];

  //std::cout << std::endl << "Scorro i jobs sul nodo" << std::endl << std::endl;

  // cycle over these jobs
  for(Job j: jobs_to_modify)
  {
    //std::cout << j.get_ID() << std::endl; // TODO: remove

    // schedule of job j
    Schedule & sch = new_schedule[j];

    // If the shcedule is not empty
    if(!sch.isEmpty())
    {

      // old setup
      const Setup & old_stp = sch.get_setup();

      //std::cout << "old_stp: "; old_stp.print(std::cout);

      // old number of GPU used
      unsigned nGPUs = old_stp.get_nGPUs(); // questa info è diversa per ogni job!

      // row to initialize a new setup
      row_t build_stp;
      build_stp.push_back(new_stp.get_VMtype());
      build_stp.push_back(new_stp.get_GPUtype());
      build_stp.push_back(std::to_string(nGPUs));
      build_stp.push_back(std::to_string(new_stp.get_maxnGPUs()));
      build_stp.push_back(std::to_string(new_stp.get_cost()));

      // new setup
      Setup job_stp(build_stp); // cambio il nome in job_stp

      //std::cout << "job_stp: "; new_stp.print(std::cout);

      // get the range of setup_time_t with this setup (same VMtype and GPUtype)
      auto range = ttime[j.get_ID()].equal_range(job_stp);
      setup_time_t::const_iterator start = range.first;
      setup_time_t::const_iterator finish = range.second;

      setup_time_t::const_iterator c_newsetup;

      // I'm looking for the setup with exactly this number of GPUs
      for(auto it=start; it!=finish; ++it)
      {
        const Setup & stp = it->first;
        if(stp.get_nGPUs() == nGPUs)
        {
          c_newsetup = it;
        }
      }

      // modify the schedule of job j according to the new setup
      sch.change_setup(c_newsetup);

  }

  }

  return new_schedule;

}

//________________________________________________________________________________________________________________________________

// generate the neighborhood of local_best_schedule
LocalSearchGPU1::neighborhood_t
LocalSearchGPU1::generate_neighborhood(void)
{
  // neighbourhood
  neighborhood_t neighbourhood;

  // nodes with the highest number of jobs in tardiness
  std::unordered_set<unsigned> tochange = get_tardiness_nodes(neigh_size);

  // TODO: INSERT IT IN get_tardiness_nodes and randomize the selection
  // If no nodes have jobs in tardiness
  /*if (tochange.empty())
  {
    // I take the first neigh_size nodes // TODO: randomize this selection
    for(int i=0; i < std::min(last_node_idx,neigh_size); ++i)
    {
      tochange.insert(i);
    }
  }
  */
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
      neighbourhood.insert({n,stp});
    }

  }

  return neighbourhood;

}

//________________________________________________________________________________________________________________________________
