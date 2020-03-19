#include "LocalSearchbySwap.hpp"


LocalSearchbySwap::LocalSearchbySwap(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{}

std::vector< std::vector<int> >
LocalSearchbySwap::get_base_possible_swaps(void) const
{
  /*
    Data una `size` restituisce tutti i vettori possibili di dimensione = size tali che gli elementi di tali vettori siano:
    - o uguali a -1
    - o uguali a un numero in [0,size) che non compaia già nel vettore
    Inoltre due vettori con gli stessi elementi ma in posizioni diverse sono considerati il medesimo vettore.
  */
  std::vector< std::vector<int> > res;
  for (int j = 0; j < neigh_size; ++j)
  {
    std::string bitmask(j, 1); // j chars 1 n.b. char 1 NON char '1'
    bitmask.resize(neigh_size, 0); // size-j chars 0 n.b. char 0 NON char '0'

    // creo il vettore di interi, lo pusho nel vettore da restiture e perumuto la bitmask
    do {
      std::vector<int> to_push(neigh_size); //vettore da pushare in res
        for (int i = 0; i < neigh_size; ++i) // [0..size) scorro gli interi
        {
            if (bitmask[i]) // se nella posizione i-esima della bitmask c'è 1
              to_push[i] = i; // il corripondente elemento del vettore diventa i
            else
              to_push[i] = -1; // altrimenti -1
        }
      res.push_back(to_push);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end())); //questo while è come fare un ciclo sui tutte le rappresentazioni binarie dei numeri [0,size)
  }
  std::vector<int> to_push(neigh_size);
  std::iota(to_push.begin(),to_push.end(),0); // aggiungo il caso della rappresentazione binaria di size
  res.push_back(to_push);
  return res;
}

std::vector< std::vector<int> >
LocalSearchbySwap::permute_unique(std::vector<int> vec) const
{
    /*
    Dato un vettore restituisce tutti i vettori ottenibili permutando gli elementi
  */
    std::sort(vec.begin(), vec.end());
    std::vector<std::vector<int> > res;
    if(vec.empty()) {
        return res;
    }
    do {
        res.push_back(vec);
    } while (std::next_permutation(vec.begin(), vec.end()));
    return res;
}

std::vector< std::vector<int> >
LocalSearchbySwap::get_possible_swaps(void) const
{
  /*
    Data una `size` restituisce tutti i vettori possibili di dimensione = size tali che gli elementi di tali vettori siano:
    - o uguali a -1
    - o uguali a un numero in [0,size) che non compaia già nel vettore
    Inoltre due vettori con gli stessi elementi ma in posizioni diverse sono considerati il medesimo vettore.
  */
  std::vector< std::vector<int> > res;
  std::vector< std::vector<int> > base_res = get_base_possible_swaps();
  for( auto & base_vec: base_res)
  {
    std::vector< std::vector<int> > to_push = permute_unique(base_vec);
    res.insert(res.end(),to_push.begin(),to_push.end());
  }

  return res;

}

row_j
LocalSearchbySwap::get_top(const map_value_j& map) 
{
  row_j N;

  auto it = map.begin();
  unsigned i = 0;
  while (i < neigh_size)
  {
    //std::pair <map_it, map_it>
    auto ret = map.equal_range(it->first);
    for (auto j = ret.first; j != ret.second && i < neigh_size; ++j)
    {
      N.push_back(j->second);
      i++;
    }
    it++;
  }
  return N;
}

row_j
LocalSearchbySwap::top_tardiness_jobs() 
{
  map_value_j omega_t_ordered; //faccio una mappa così me li ordina per tardiness weight (omega)
  for (const auto & i : local_best_schedule)
  {
    if (i.second.get_vmCost() != 0)
      omega_t_ordered.insert(std::make_pair (i.first.get_tardinessWeight(), i.first));
  }
  return get_top(omega_t_ordered);
}

row_j
LocalSearchbySwap::top_cost_jobs(void) 
{
  map_value_j costs_ordered;
  for (const auto & i : local_best_schedule)
  { // qua non dovrebbe servire il controllo vmCost != 0 perchè prendiamo i maggiori per vmCost
    costs_ordered.insert(std::make_pair(i.second.get_vmCost(), i.first));
  }
  return get_top(costs_ordered);
}

row_j
LocalSearchbySwap::top_margin_jobs(void) 
{
  map_value_j margin_ordered;
  for (const auto & i : local_best_schedule)
  {
    if (i.second.get_vmCost() != 0)
      {
        margin_ordered.insert(std::make_pair ((i.second.get_selectedTime() - i.first.get_deadline()), i.first));
      }
  }
  return get_top(margin_ordered);
}

void
LocalSearchbySwap::fill_swapping_sets(void)
{
    std::cout << "SONO NEL NEIGH: " << neigh_def << std::endl;

  if (neigh_def != neigh_2 && neigh_def != neigh_3) // qualsiasi cosa che non sia due e tre è 1
  {
    A_job_ids = top_tardiness_jobs();
    B_job_ids = top_cost_jobs();
    
    std::cout << "INISIEME A size: " << A_job_ids.size()<<std::endl;
    std::cout << "JOBBINO 1 ID: " << A_job_ids[0].get_ID()<<std::endl;


  }

  if (neigh_def == neigh_2)
  {
    A_job_ids = top_tardiness_jobs();
    B_job_ids = top_margin_jobs();
  }

  if (neigh_def == neigh_3) // A e B hanno dimensioni diverse !
  {
    // A è la coda, cioò submitted_jobs ?
    B_job_ids = top_cost_jobs();
  }
}

bool
LocalSearchbySwap::visit_neighbor()
{
  //ciclo sugli intorni per farli tutti e tre
  possible_swap_indices = get_base_possible_swaps();
    std::cout << "HO GLI INDICI!: " << possible_swap_indices[0][0]<< std::endl;
  
  fill_swapping_sets();
    std::cout << "HO FILLATO " << std::endl;
  bool changed = false;
  /*
  // BEST FIT
  if (best_fit)
  {
    for (auto v : possible_swap_indices) // or for (unisgned i = 0; i < possible_swap_indices; ++i) e passare i a perform_swap
    {
      job_schedule_t candidate_schedule = perform_swap(v); // qui accade la magia dello scambio
      double candidate_value = evaluate_objective(candidate_schedule);
      if (candidate_value < best_schedule_value_t)
      {
        best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;
      }}}

  // FIRST FIT
  if (!best_fit)
  {
    for (auto v : possible_swap_indices) // or for (unisgned i = 0; i < possible_swap_indices; ++i) e passare i a perform_swap
    {
      job_schedule_t candidate_schedule = perform_swap(v); // qui accade la magia dello scambio
      double candidate_value = evaluate_objective(candidate_schedule);
      if (candidate_value < best_schedule_value_t)
      {
        best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;
        break;
      }}} */

   //in maniera più compatta
  for (auto v : possible_swap_indices)
    {
      std::cout << "STO PER SWAPPARE.........." << std::endl;
      job_schedule_t candidate_schedule = perform_swap(v);
      
      double candidate_value = evaluate_objective(candidate_schedule);
      std::cout << "CANDIDATE VALUE: " << candidate_value << std::endl;
      std::cout << "BEST VALUE: " << best_schedule_value_t << std::endl;

      if (candidate_value < best_schedule_value_t)
      {
        local_best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;
        if (!best_fit)
          return changed;
      }
    }

   return changed;
}

job_schedule_t
LocalSearchbySwap::perform_swap(const std::vector<int>& swap_indices)
{

  for(auto el: swap_indices)
    std::cout<<el<< " ";
  std::cout<<std::endl;


  job_schedule_t new_schedule = local_best_schedule;
  for (unsigned idx_A = 0; idx_A < swap_indices.size(); ++idx_A)
  {
    std::cout << "CICLO SUGLI INDICI DI A" << std::endl;
    int idx_B = swap_indices[idx_A];
    if (idx_B > -1 and idx_B<B_job_ids.size())
    {
      job_schedule_t temp = new_schedule;
      std::cout << "HO CREATE TEMP" << std::endl;

      //A_job_ids[idx_A].print(std::cout);

      const auto elem_of_A = temp.find(A_job_ids[idx_A]);
      std::cout << "PRENDO ELEM DI A" << std::endl;
      Schedule old_A = elem_of_A-> second;
      std::cout << "PRENDO LA VECCHIA SCHEDULE DI A" << std::endl;
      const unsigned old_node_A = elem_of_A->second.get_node_idx();
      std::cout << "PRENDO VECCHIO NODO DI A" << std::endl;
      Job jobA = elem_of_A->first;
      std::cout << "JOB A, CHE HA TARDINESS" << jobA.get_tardinessWeight << std::endl;


      const auto elem_of_B = temp.find(B_job_ids[idx_B]);
      Schedule old_B = elem_of_B-> second;
      const unsigned old_node_B = elem_of_B->second.get_node_idx();
      Job jobB = elem_of_B->first;
      std::cout << "HO APPENA SELEZIONATO CHI SCAMBIARE EHEHEHEHE" << std::endl;
      // codice da perform_assignement, nella parte dove lo assegna a un nodo esistente

      // prima provo ad assegnare A dove c'era B
      const setup_time_t& tjvg_A = ttime.at(jobA.get_ID());
      Dstar dstarA (jobA, tjvg_A, current_time);
      if (! full_greedy)
        dstarA.set_random_parameter(alpha);
      setup_time_t::const_iterator best_stpA = dstarA.get_best_setup(generator);

      unsigned num_gpu_used_by_B = elem_of_B->second.get_setup().get_nGPUs();
      temp.erase(elem_of_B);
      std::cout << "HO SCANCELLATO IL JOB IN IDX_B" << std::endl;


      std::cout << "STO PER SETTARE NUMERO DI GPU: "<<nodes[old_node_B].get_remainingGPUs()+num_gpu_used_by_B << std::endl;
      nodes[old_node_B].set_remainingGPUs(nodes[old_node_B].get_remainingGPUs()+num_gpu_used_by_B);

      std::cout << "HO RI-SETTATO LE GPU" << std::endl;

      bool assignedAtoB = assign_to_selected_node(jobA, best_stpA, temp, old_node_B);
      std::cout << "HO ASSEGNATO AL NODO?: " << assignedAtoB <<  std::endl;

      if (assignedAtoB)
      {

        unsigned num_gpu_used_by_A = elem_of_A->second.get_setup().get_nGPUs();
        temp.erase(elem_of_A);
        std::cout << "HO SCANCELLATO IL JOB IN IDX_A" << std::endl;
        update_schedule(old_A, temp.find(A_job_ids[idx_A])->second);

        std::cout << "STO PER SETTARE NUMERO DI GPU: "<<nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A<< std::endl;
        nodes[old_node_A].set_remainingGPUs(nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A);
        std::cout << "HO RI-SETTATO LE GPU di A" << std::endl;
        const setup_time_t& tjvg_B = ttime.at(jobB.get_ID());
        Dstar dstarB (jobB, tjvg_B, current_time);
        if (! full_greedy)
          dstarB.set_random_parameter(alpha);
        setup_time_t::const_iterator best_stpB = dstarB.get_best_setup(generator);
        bool assignedBtoA = assign_to_selected_node(jobB, best_stpB, temp, old_node_A);
        update_schedule(old_B, temp.find(B_job_ids[idx_B])->second);
        if (assignedBtoA)
        {
          new_schedule = temp;
        }
      }
    }
  }
  swap(new_schedule, local_best_schedule);
  return local_best_schedule;
}
