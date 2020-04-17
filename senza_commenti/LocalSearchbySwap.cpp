#include "LocalSearchbySwap.hpp"

LocalSearchbySwap::LocalSearchbySwap(const std::string &args, const std::string &d,
                                     const std::string &file_jobs, const std::string &file_times,
                                     const std::string &file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{}

LocalSearchbySwap::vecvec
LocalSearchbySwap::get_base_possible_swaps(void) const
{
  /*
    Data una `size` restituisce tutti i vettori possibili di dimensione = size tali che gli elementi di tali vettori siano:
    - o uguali a -1
    - o uguali a un numero in [0,size) che non compaia già nel vettore
    Inoltre due vettori con gli stessi elementi ma in posizioni diverse sono considerati il medesimo vettore.
  */
  std::vector<std::vector<int>> res;
  for (unsigned j = 0; j < neigh_size; ++j)
  {
    std::string bitmask(j, 1);     // j chars 1 n.b. char 1 NON char '1'
    bitmask.resize(neigh_size, 0); // size-j chars 0 n.b. char 0 NON char '0'

    // creo il vettore di interi, lo pusho nel vettore da restiture e perumuto la bitmask
    do
    {
      std::vector<int> to_push(neigh_size); //vettore da pushare in res
      for (unsigned i = 0; i < neigh_size; ++i)  // [0..size) scorro gli interi
      {
        if (bitmask[i])   // se nella posizione i-esima della bitmask c'è 1
          to_push[i] = i; // il corripondente elemento del vettore diventa i
        else
          to_push[i] = -1; // altrimenti -1
      }
      res.push_back(to_push);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end())); //questo while è come fare un ciclo sui tutte le rappresentazioni binarie dei numeri [0,size)
  }
  std::vector<int> to_push(neigh_size);
  std::iota(to_push.begin(), to_push.end(), 0); // aggiungo il caso della rappresentazione binaria di size
  res.push_back(to_push);
  return res;
}

LocalSearchbySwap::vecvec
LocalSearchbySwap::permute_unique(std::vector<int> vec) const
{
  /*
    Dato un vettore restituisce tutti i vettori ottenibili permutando gli elementi
  */
  std::sort(vec.begin(), vec.end());
  std::vector<std::vector<int>> res;
  if (vec.empty())
  {
    return res;
  }
  do
  {
    res.push_back(vec);
  } while (std::next_permutation(vec.begin(), vec.end()));
  return res;
}

LocalSearchbySwap::vecvec
LocalSearchbySwap::get_possible_swaps(void) const
{
  /*
    Data una `size` restituisce tutti i vettori possibili di dimensione = size tali che gli elementi di tali vettori siano:
    - o uguali a -1
    - o uguali a un numero in [0,size) che non compaia già nel vettore
    Inoltre due vettori con gli stessi elementi ma in posizioni diverse sono considerati il medesimo vettore.
  */
  std::vector<std::vector<int>> res;
  std::vector<std::vector<int>> base_res = get_base_possible_swaps();
  for (auto &base_vec : base_res)
  {
    std::vector<std::vector<int>> to_push = permute_unique(base_vec);
    res.insert(res.end(), to_push.begin(), to_push.end());
  }

  return res;
}

LocalSearchbySwap::row_j
LocalSearchbySwap::get_top(const map_value_j &map)
{
  row_j N;
  if (map.size() == 0)
  {
    return N;
  }

  unsigned cnt = 0;
  for (auto it = map.begin(); it != map.end() and cnt < neigh_size; ++it)
  {
    if (! local_best_schedule.find(it->second)->second.isEmpty() )
		{
      N.push_back(it->second);
    	cnt++;
    }
  }

  return N;
}

LocalSearchbySwap::row_j
LocalSearchbySwap::top_tardiness_jobs()
{
  map_value_j omega_t_ordered;
  for (const auto &i : local_best_schedule)
      omega_t_ordered.insert(std::make_pair(i.first.get_tardinessWeight(), i.first));

  return get_top(omega_t_ordered);
}

LocalSearchbySwap::row_j
LocalSearchbySwap::top_cost_jobs(void)
{
  map_value_j costs_ordered;
  job_schedule_t temp = local_best_schedule;
  for(const auto &j: A_job_ids)
    temp.erase(j);

  for (const auto &i: temp)
    if (! i.second.isEmpty() )
		{
      Node & N = nodes[i.second.get_node_idx()];
      double cost = N.get_cost() * i.second.get_setup().get_nGPUs() / (N.get_remainingGPUs() + N.get_usedGPUs());
      costs_ordered.insert(std::make_pair(cost, i.first));
    }
  return get_top(costs_ordered);
}

LocalSearchbySwap::row_j
LocalSearchbySwap::top_margin_jobs(void)
{
  map_value_j margin_ordered;
  job_schedule_t temp = local_best_schedule;
  for(const auto &j: A_job_ids)
    temp.erase(j);

  for (const auto &i : temp)
    if (! i.second.isEmpty() )
      margin_ordered.insert(std::make_pair((i.second.get_selectedTime() - i.first.get_deadline()), i.first));

  return get_top(margin_ordered);
}

void
LocalSearchbySwap::fill_swapping_sets(void)
{
  if (neigh_def != neigh_2)
  {
    A_job_ids = top_tardiness_jobs();
    B_job_ids = top_cost_jobs();
  }
  else
  {
    A_job_ids = top_tardiness_jobs();
    B_job_ids = top_margin_jobs();
  }
}

bool
LocalSearchbySwap::visit_neighbor()
{
  if(local_best_schedule.size() < 2*neigh_size)
    return false;
  possible_swap_indices = get_possible_swaps();

  fill_swapping_sets();

  unsigned A_sz = A_job_ids.size();
  unsigned B_sz = B_job_ids.size();

  if (A_sz == 0 || B_sz == 0 || A_sz != B_sz || A_sz != neigh_size)
    return false;

  bool changed = false;
  unsigned iter = 0;

  for (auto v : possible_swap_indices)
  {
    std::vector<Node> open = nodes;

    job_schedule_t candidate_schedule = perform_swap(v);
    double candidate_value = evaluate_objective(candidate_schedule);

    if (candidate_value < best_schedule_value_t)
    {
      local_best_schedule = candidate_schedule;
      best_schedule_value_t = candidate_value;
      changed = true;
      if (!best_fit)
        return changed;
    }
    else 
      swap(open, nodes);
    iter ++;
  }
  return changed;
}

job_schedule_t
LocalSearchbySwap::perform_swap(const std::vector<int> &swap_indices)
{
  job_schedule_t new_schedule = local_best_schedule;
  std::vector<Node> open_nodes = nodes;
  
  for (unsigned idx_A = 0; idx_A < swap_indices.size(); ++idx_A)
  {
    int idx_B = swap_indices[idx_A];
    if (idx_B > -1 and idx_B < B_job_ids.size())
    {
      job_schedule_t temp = new_schedule;

      const auto &  elem_of_A = temp.find(A_job_ids[idx_A]);
      Schedule & old_sch_A = elem_of_A->second;
      const unsigned old_node_A = elem_of_A->second.get_node_idx();
      const Job & jobA = elem_of_A->first;

      const auto & elem_of_B = temp.find(B_job_ids[idx_B]);
      Schedule & old_sch_B = elem_of_B->second;
      const unsigned old_node_B = elem_of_B->second.get_node_idx();
      const Job & jobB = elem_of_B->first;

      erase_job_from_node(old_sch_B);
      bool assignedAtoB = assign_to_selected_node(jobA, temp, old_node_B);

      if (assignedAtoB)
      {
        erase_job_from_node(old_sch_A);
        bool assignedBtoA = assign_to_selected_node(jobB, temp, old_node_A);

        if (assignedBtoA)
          swap(new_schedule,temp);
        else
	        swap(nodes, open_nodes);
      }
      else
      {
	      swap(nodes, open_nodes);
	      return new_schedule;
	    }
      
    }
  }
  return new_schedule;
}


void
LocalSearchbySwap::erase_job_from_node(Schedule & sch)
{
  Node & node = nodes[sch.get_node_idx()];

  unsigned GPU_used_by_job = sch.get_setup().get_nGPUs();

  unsigned max = node.get_usedGPUs() + node.get_remainingGPUs();
  unsigned used = node.get_usedGPUs() - GPU_used_by_job;
  if (node.get_usedGPUs() <= GPU_used_by_job)
    used = 0;
 
  row_t Row {node.get_VMtype(), node.get_GPUtype(), std::to_string(used), std::to_string(max), std::to_string(node.get_cost())}; 
 
  const Setup new_setup (Row);
  node.change_setup(new_setup);
  node.set_remainingGPUs(used);
}
