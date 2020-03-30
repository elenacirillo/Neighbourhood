#include "heuristic.hpp"

Heuristic::Heuristic (const std::string& args, const std::string& d, 
                      const std::string& file_jobs, 
                      const std::string& file_times, 
                      const std::string& file_nodes)
{
  ARGS = args;
  delta = d;
  std::string folder = data_folder + ARGS + "/";
  create_container(jobs, folder + file_jobs);
  create_map(ttime, folder + file_times);
  create_container(nodes, folder + file_nodes);
}

double
Heuristic::submit_job (double elapsed_time)
{
  double t = INF;
  if (! jobs.empty())
  {
    t = jobs.front().get_submissionTime();
    if (t <= (current_time + elapsed_time))
    {
      submitted_jobs.push_back(jobs.front());
      jobs.pop_front();
    }
  }
  return t;
}

void
Heuristic::remove_ended_jobs (void)
{
  unsigned last_iter = scheduled_jobs.size() - 1;
  job_schedule_t& last_schedule = scheduled_jobs[last_iter];

  job_schedule_t::const_iterator cit;
  for (cit = last_schedule.cbegin(); cit != last_schedule.cend(); ++cit)
  {
    const Job& j = cit->first;
    const Schedule& sch = cit->second;

    if (sch.get_completionPercent() >= (100-TOL))
    {
      std::list<Job>::iterator it;
      bool found = false;
      for (it=submitted_jobs.begin(); it!=submitted_jobs.end() && !found; 
           ++it)
      {
        if (*it == j)
        {
          found = true;
          submitted_jobs.erase(it);
        }
      }
    }
  }
}

void
Heuristic::close_nodes (void)
{
  for (Node& n : nodes)
    n.close_node();
  last_node_idx = 0;
}

void
Heuristic::update_execution_times (unsigned iter, double elapsed_time)
{
  unsigned last_iter = scheduled_jobs.size() - 1;
  job_schedule_t& last_schedule = scheduled_jobs[last_iter];

  job_schedule_t::const_iterator schit;
  for (schit = last_schedule.cbegin(); schit != last_schedule.cend(); ++schit)
  { 
    const Job& j = schit->first;
    const Schedule& sch = schit->second;
    double cp = sch.get_completionPercent();
    double cp_step = sch.get_cP_step();
    if (! sch.isEmpty() && cp < (100-TOL))
    {
      setup_time_t& tjvg = ttime.at(j.get_ID());
      setup_time_t::iterator it;
      for (it = tjvg.begin(); it != tjvg.end(); ++it)
      {
        double t = it->second * (100 - cp_step) / 100;
        it->second = (t > 0) ? t : INF;
      }
    }
  }
}

void
Heuristic::update_minMax_exec_time (Job& j) const
{
  double min_exec_time = INF;
  double max_exec_time = 0.;
  const setup_time_t& tjvg = ttime.at(j.get_ID());
  setup_time_t::const_iterator cit;
  for (cit = tjvg.cbegin(); cit != tjvg.cend(); ++cit)
  {
    min_exec_time = std::min(min_exec_time, cit->second);
    max_exec_time = std::max(max_exec_time, cit->second);
  }
  j.set_minExecTime(min_exec_time);
  j.set_maxExecTime(max_exec_time);
}

bool
Heuristic::update_scheduled_jobs (unsigned iter, double elapsed_time)
{
  bool all_completed = true;
  
  // select last schedule
  unsigned last_iter = scheduled_jobs.size() - 1;
  job_schedule_t& last_schedule = scheduled_jobs[last_iter];

  // compute simulation time
  double sim_time = current_time + elapsed_time;

  // loop over last schedule
  for (job_schedule_t::iterator it = last_schedule.begin();
       it != last_schedule.end(); ++it)
  {
    const Job& j = it->first;
    Schedule& sch = it->second;
    //std::cout << "JOB: " << j.get_ID();

    // set iteration number and simulation time
    sch.set_iter(last_iter+1);
    sch.set_simTime(sim_time);
    
    // if schedule is not empty, compute execution time, completion percent
    // and cost of VM
    double cp = 0.0;
    if (! sch.isEmpty())
    {
      sch.set_executionTime(elapsed_time);
      cp = elapsed_time * 100 / sch.get_selectedTime();
      sch.set_cP_step(cp);
      //std::cout << "; execution_time: " << elapsed_time
      //         << "; selected_time: " << sch.get_selectedTime()
      //         << "; cp: " << cp << std::endl;

      // get number of used GPUs on the current node
      unsigned node_idx = sch.get_node_idx();
      unsigned g = nodes[node_idx].get_usedGPUs();

      // compute cost of VM
      sch.compute_vmCost(g);
    }

    // in all iterations after the first one, update start time and completion
    // percent of jobs that have already been partially executed
    if (iter > 1)
    {
      const job_schedule_t& previous_schedule = scheduled_jobs[last_iter-1];
      job_schedule_t::const_iterator p_sch = previous_schedule.find(j);
      if (p_sch != previous_schedule.cend())
      {
        sch.set_startTime((p_sch->second).get_startTime());
        double prev_cp = (p_sch->second).get_completionPercent();
        cp = prev_cp + cp * (100 - prev_cp) / 100;
      }
      else
        sch.set_startTime(current_time);
    }
    else
      sch.set_startTime(current_time);

    // set completion percent
    sch.set_completionPercent(cp);

    // set finish time, tardiness and tardiness cost of jobs that are 
    // completed
    if (cp >= (100-TOL))
    {
      sch.set_finishTime(sim_time);
      double tardiness = std::max(sim_time - j.get_deadline(), 0.);
      sch.set_tardiness(tardiness);
      sch.compute_tardinessCost(j.get_tardinessWeight());
    }
    else
    {
      sch.set_tardiness(0.0);
      all_completed = false;
    }
  }

  return all_completed;
}

double
Heuristic::find_first_finish_time (const job_schedule_t& last_schedule) const
{
  double fft = INF;

  job_schedule_t::const_iterator cit;
  for (cit = last_schedule.cbegin(); cit != last_schedule.cend(); ++cit)
    fft = std::min(fft, (cit->second).get_selectedTime());

  return fft;
}

void
Heuristic::algorithm (unsigned myseed, unsigned max_random_iter)
{
  std::cout << "##################### algorithm #####################\n"
            << "myseed: " << myseed << "; max_random_iter: "
            << max_random_iter << std::endl;

  std::cout << "iter, random iteration, vmCost, tardiCost, worstCaseTardiness"
            << std::endl;

  // random numbers generator
  if (max_random_iter > 0)
    generator.seed(myseed);

  // intialization
  unsigned iter = 0;
  double first_finish_time = INF;
  bool all_completed = false;
  bool stop = false;
  
  while (!stop)
  {
    std::cout << "\n-------- iter " << iter << " --------" << std::endl;
    //std::cout << iter << ",";

    // add new job to the queue
    double elapsed_time = std::min(
                                scheduling_interval,
                                first_finish_time
                              );
    double next_submission_time = submit_job(elapsed_time);

    // compute elapsed time between current and previous iteration
    elapsed_time = std::min(
                          elapsed_time,
                          next_submission_time - current_time
                        );

    // update scheduled_jobs to compute execution costs
    if (iter > 0)
      all_completed = update_scheduled_jobs(iter, elapsed_time);
    
    // check stopping cryterion
    stop = (next_submission_time == INF) && all_completed;

    // TODO: rimuovere, lo abbiamo messo per confrontare i tempi di esecuzione
    if (stop)
    {

      std::cout << "TIME: " << current_time << std::endl;
    }

    if (!stop)
    {
      // close all currently opened nodes
      close_nodes();

      // update current_time according to the elapsed time between current 
      // and previous iteration
      current_time += elapsed_time;

      if (iter > 0)
      {
        // remove ended jobs from the queue
        remove_ended_jobs();
        // update execution time of jobs that have been partially executed
        update_execution_times(iter, elapsed_time);
      }

      // determine the best schedule
      job_schedule_t best_schedule = perform_scheduling(max_random_iter);

      // add the best schedule to scheduled_jobs
      scheduled_jobs.push_back(best_schedule);

      // find execution time of first ending job
      first_finish_time = find_first_finish_time(scheduled_jobs[iter]);
      iter++;
    }
  }
}

void
Heuristic::print_data (void) const
{
  std::string folder = result_folder + ARGS + delta + "/";
  print_container(jobs, folder + "jobs_list.csv");
  print_map(ttime, folder + "ttime.csv");
  print_container(nodes, folder + "nodes.csv");
}

void
Heuristic::print_schedule (const std::string& filename) const
{
  std::string folder = result_folder + ARGS + delta + "/";
  print_result(scheduled_jobs, nodes, folder + filename);
}
