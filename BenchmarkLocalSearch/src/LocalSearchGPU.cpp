#include "LocalSearcGPU.hpp"

//________________________________________________________________________________________________________________________________

// constructor
LocalSearchGPU::LocalSearchGPU(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{

  // initialize vm_gpus starting from ttime
  initialize_vm_gpus();

}

//________________________________________________________________________________________________________________________________-

// initialize the map GPU-VM starting from ttime (ttime is a protected memeber of the parent class Heuristic)
void
LocalSearchGPU::initialize_vm_gpus(void)
{
  if(! ttime.empty())
  {
    // consider the setups related to the first job
    setup_time_t & setup_time = ttime.begin()->second;
    // cycle over all its setups
    for(auto pair: setup_time)
    {
      // i'm interested only in the setup, not in the time
      Setup & stp = pair.first;
      // insert the correspondent couple VM-GPU
      vm_gpus[stp.get_VMtype()] = stp.get_GPUtype();
    }
  }
}

//________________________________________________________________________________________________________________________________

// update the data structure node_jobs acordingly to initial_schedule
void
LocalSearchGPU::update_node_jobs(void)
{
  node_jobs.clear();

  // cycle over the initial schedule
  for(auto pair: initial_schedule)
  {
    Job & job = pair.first;
    Schedule & sch = pair.second;

    unsigned node_idx = sch.get_node_idx();

    // insert the job in the unordered_set associated with the node idx
    node_jobs[sch.get_node_idx()].insert(job);
  }
}

//________________________________________________________________________________________________________________________________

// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU::visit_neighbor(bool best_fit) override
{
  // true if the function finds a better schedule and updates it
  bool changed = false;

  // update the data structure that links the nodes to the jobs (change_GPU needs it updated)
  update_node_jobs();

  // neighborhood to be explored
  neighborhood_t neighborhood = generate_neighborhood();

  // visit the neighboorhood
  for(auto neighbor: neighborhood)
  {
    // a neighbor is made of a node and a new GPU_type
    unsigned node_idx = neighbor.first;
    std::string GPU_type = neighbor.second;

    // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
    job_schedule_t candidate_schedule = change_GPU(node_idx, GPU_type);

    // evaluate the objective in this point of the neighborhood
    double candidate_value = evaluate_objective(candidate_schedule);

    // if this schedule improves the objective function
    if (candidate_value < best_schedule_value)
    {
      // update the best schedule and the best value
      best_schedule = candidate_schedule;
      best_schedule_value = candidate_value;
      changed = true;

      // update the configuration of the node accordingly
      nodes[node_idx].change_setup(/* ? ? ? */); // devo fare in modo che la configuration mantenga lo stesso numero di GPU USATE, come fare??

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
std::pair<job_schedule_t, setup>
LocalSearchGPU::change_GPU(unsigned node_idx, std::string GPU_type)
{
  // new schedule to modify
  job_schedule_t new_schedule = initial_schedule;
  
  // get the jobs in execution on the node "node_idx"
  std::unordered_set<Job> jobs_to_modify = node_jobs[node_idx];

  // new setup
  Setup stp;
  // ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?
  // purtroppo non esiste un modo per risalire al setup di un nodo direttamente dalla classe "Nodes", maledetta Federica
  // srutto quind la schedule
  if(!jobs_to_modify.empty())
  {
    // consider one of these jobs
    Job & j = *jobs_to_modify.begin();
    Schedule & sch = new_schedule[j];

    // old setup of the node
    stp = sch.get_setup();

  }
  // ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?

  // cycle over these jobs
  for(Job j: jobs_to_modify)
  {
    // schedule of job j
    Schedule & sch = new_schedule[j];

    // get the setup infos related to job "j" and "stp"
    setup_time_t::const_iterator csetup = ttime[j.get_ID()].find(stp);

    // modify the schedule of job j according to the new setup
    sch.change_setup(csetup);
  }

}

//________________________________________________________________________________________________________________________________

// generate the neighborhood of initial_schedule
neighborhood_t
LocalSearchGPU::generate_neighborhood(void)
{
  // neighborhood
  neighborhood_t neighborhood;

  // cycle over the nodes
  for(n: nodes){
    std::string VM_type = n.get_VMtype();
    // ?????????????????????????????????????????????
    std::list<std::string> possible_gpus = vm_gpus[]
  }

  return neighborhood;

}

//________________________________________________________________________________________________________________________________
