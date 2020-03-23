#include "LocalSearchbySwap.hpp"

LocalSearchbySwap::LocalSearchbySwap(const std::string &args, const std::string &d,
                                     const std::string &file_jobs, const std::string &file_times,
                                     const std::string &file_nodes) : LocalSearch(args, d, file_jobs, file_times, file_nodes)
{
}

std::vector<std::vector<int>>
LocalSearchbySwap::get_base_possible_swaps(void) const
{
  /*
    Data una `size` restituisce tutti i vettori possibili di dimensione = size tali che gli elementi di tali vettori siano:
    - o uguali a -1
    - o uguali a un numero in [0,size) che non compaia già nel vettore
    Inoltre due vettori con gli stessi elementi ma in posizioni diverse sono considerati il medesimo vettore.
  */
  std::vector<std::vector<int>> res;
  for (int j = 0; j < neigh_size; ++j)
  {
    std::string bitmask(j, 1);     // j chars 1 n.b. char 1 NON char '1'
    bitmask.resize(neigh_size, 0); // size-j chars 0 n.b. char 0 NON char '0'

    // creo il vettore di interi, lo pusho nel vettore da restiture e perumuto la bitmask
    do
    {
      std::vector<int> to_push(neigh_size); //vettore da pushare in res
      for (int i = 0; i < neigh_size; ++i)  // [0..size) scorro gli interi
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

std::vector<std::vector<int>>
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

std::vector<std::vector<int>>
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

row_j LocalSearchbySwap::get_top(const map_value_j &map)
{
  row_j N;
  if (map.size() == 0)
  {
    std::cout << "ERROR: get_top mappa vuota" << std::endl;
    return N;
  }
/*  auto it = map.begin();
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
  return N;*/
  unsigned cnt=0;
  for(auto it=map.begin(); it!=map.end() and cnt<neigh_size; ++it)
  {
    N.push_back(it->second);
    cnt++;
  }
  return N;

}

row_j LocalSearchbySwap::top_tardiness_jobs()
{
  map_value_j omega_t_ordered; //faccio una mappa così me li ordina per tardiness weight (omega)
  for (const auto &i : local_best_schedule)
  {
    //if (i.second.get_vmCost() != 0)
      omega_t_ordered.insert(std::make_pair(i.first.get_tardinessWeight(), i.first));
  }
  return get_top(omega_t_ordered);
}

row_j LocalSearchbySwap::top_cost_jobs(void)
{
  map_value_j costs_ordered;
  for (const auto &i : local_best_schedule)
  { // qua non dovrebbe servire il controllo vmCost != 0 perchè prendiamo i maggiori per vmCost
    costs_ordered.insert(std::make_pair(i.second.get_vmCost(), i.first));
  }
  return get_top(costs_ordered);
}

row_j LocalSearchbySwap::top_margin_jobs(void)
{
  map_value_j margin_ordered;
  for (const auto &i : local_best_schedule)
  {
    if (i.second.get_vmCost() != 0)
    {
      margin_ordered.insert(std::make_pair((i.second.get_selectedTime() - i.first.get_deadline()), i.first));
    }
  }
  return get_top(margin_ordered);
}

void LocalSearchbySwap::fill_swapping_sets(void)
{
  std::cout << "  -- Sono nel neigh numero: " << neigh_def << std::endl;

  if (neigh_def != neigh_2 && neigh_def != neigh_3) // qualsiasi cosa che non sia due e tre è 1
  {
    A_job_ids = top_tardiness_jobs();
    B_job_ids = top_cost_jobs();

    std::cout << "  -- A size: " << A_job_ids.size() << std::endl;
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

bool LocalSearchbySwap::visit_neighbor()
{
  // perform local search only if there are enough jobs
  if(local_best_schedule.size() < neigh_size)
  {
    std::cout<< "Ce ne stan troppo poghi"<< std::endl;//TOREMOVE 
    return false;
  }

  //ciclo sugli intorni per farli tutti e tre
  possible_swap_indices = get_base_possible_swaps();
  std::cout << " - Ho ottenuto tutti i possibili indici di swap" << std::endl; //TOREMOVE

  std::cout << " - Entro in fill_swapping_sets()" << std::endl; //TOREMOVE
  fill_swapping_sets();

  if (A_job_ids.size() == 0 || B_job_ids.size() == 0)
  {
    std::cout << " - ERRORE: A o B vuoti dopo il fill" << std::endl; //TOREMOVE
    return false;
  }
  /* start //TOREMOVE*/
  std::cout << " - Ho fillato i vettori A e B da swappare" << std::endl;
  std::cout << " - A vector: "; //TOREMOVE
  row_t A_str;
  for (auto el : A_job_ids){A_str.push_back(el.get_ID());}
  for (auto el : A_str){std::cout << el << " ";};
  std::cout << std::endl;
  std::cout << " - B vector: ";
  row_t B_str;
  for (auto el : B_job_ids){B_str.push_back(el.get_ID());}
  for (auto el : B_str){std::cout << el << " ";};
  std::cout << std::endl;
  /*END TO REMOVE*/

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
    std::cout << " - sto per entrare in perform_swap" << std::endl;
    job_schedule_t candidate_schedule = perform_swap(v);

    double candidate_value = evaluate_objective(candidate_schedule);
    std::cout << " - candidate value: " << candidate_value << std::endl;
    std::cout << " - best value : " << best_schedule_value_t << std::endl;

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
LocalSearchbySwap::perform_swap(const std::vector<int> &swap_indices)
{

  std::cout << "  --Inizio perform_swap, best_schedule_value = " << best_schedule_value_t << std::endl; //TOREMOVE

  // PRINTINO DEI TUTTE LE INFO SU A e B 
 std::cout << "\n\n### PRINTO INFO SU A e B PER LO SWAP ###"<< std::endl;

  for (auto el : swap_indices) std::cout << el << " ";
  std::cout << std::endl;
  for(auto el: A_job_ids)
  {
    std::cout<<"ID: "<<el.get_ID()<<"   ";
    std::cout<<"GPU USATE: "<<local_best_schedule.find(el)->second.get_setup().get_nGPUs()<< "   ";
    std::cout<<"NODO USATO: "<<local_best_schedule.find(el)->second.get_node_idx()<< "   ";
    std::cout<<"GPU RIMANENTI SUL NODO: "<<nodes[local_best_schedule.find(el)->second.get_node_idx()].get_remainingGPUs()<< "   ";
    std::cout<<"GPU USATE SUL NODO: "<<nodes[local_best_schedule.find(el)->second.get_node_idx()].get_usedGPUs()<< "   ";
    std::cout<<std::endl; 
  }
    std::cout<<"Vettore B: "<<std::endl;    
  for(auto el: B_job_ids)
  {
    std::cout<<"ID: "<<el.get_ID()<<"   ";
    std::cout<<"GPU USATE: "<<local_best_schedule.find(el)->second.get_setup().get_nGPUs()<< "   ";
    std::cout<<"NODO USATO: "<<local_best_schedule.find(el)->second.get_node_idx()<< "   ";
    std::cout<<"GPU RIMANENTI SUL NODO: "<<nodes[local_best_schedule.find(el)->second.get_node_idx()].get_remainingGPUs()<< "   ";
    std::cout<<"GPU USATE SUL NODO: "<<nodes[local_best_schedule.find(el)->second.get_node_idx()].get_usedGPUs()<< "   ";
    
    std::cout<<std::endl;    
  }
 std::cout << "\n\n"<< std::endl;




  //end PRINTINO

  job_schedule_t new_schedule = local_best_schedule;
  std::vector<Node> open_nodes = nodes;
  for (unsigned idx_A = 0; idx_A < swap_indices.size(); ++idx_A)
  {
    std::cout << "  -- ciclo su indici di A" << std::endl;          //TOREMOVE
    std::cout << "  -- index_A e A job id: " << idx_A << std::endl; //TOREMOVE
    std::cout << " " << A_job_ids[idx_A].get_ID() << std::endl;     //TOREMOVE
    int idx_B = swap_indices[idx_A];
    std::cout << "index di B: " << idx_B << std::endl; //TOREMOVE
    if (idx_B > -1 and idx_B < B_job_ids.size() and 
		new_schedule.find(A_job_ids[idx_A])->first.get_ID() == new_schedule.find(B_job_ids[idx_B])->first.get_ID())
    {
      std::cout << "  -- ciclo su indici di B" << std::endl; //TOREMOVE
      job_schedule_t temp = new_schedule;
      //A_job_ids[idx_A].print(std::cout);

      const auto elem_of_A = temp.find(A_job_ids[idx_A]);
      std::cout << "  -- elem_of_A contiene: " << elem_of_A->first.get_ID()<< std::endl;//TOREMOVE
      Schedule old_A = elem_of_A->second; //TOREMOVE: serve solo a stampare una info
      const unsigned old_node_A = elem_of_A->second.get_node_idx();
      std::cout << "  -- seleziono vecchio nodo di elem_of_A: "<<old_node_A<< std::endl; //TOREMOVE
      Job jobA = elem_of_A->first;


      const auto elem_of_B = temp.find(B_job_ids[idx_B]);
      std::cout << "  -- elem_of_B contiene: " << elem_of_B->first.get_ID()<< std::endl;//TOREMOVE
      //Schedule old_B = elem_of_B->second;
      const unsigned old_node_B = elem_of_B->second.get_node_idx();
      std::cout << "  -- seleziono vecchio nodo di elem_of_B: "<<old_node_B<< std::endl; //TOREMOVE
      Job jobB = elem_of_B->first;
	


      // prima provo ad assegnare A dove c'era B
      const setup_time_t &tjvg_A = ttime.at(jobA.get_ID());
      Dstar dstarA(jobA, tjvg_A, current_time);
      if (!full_greedy)
        dstarA.set_random_parameter(alpha);
      setup_time_t::const_iterator best_stpA = dstarA.get_best_setup(generator);

      //temp.erase(elem_of_B);
      //std::cout << "HO SCANCELLATO IL JOB IN IDX_B" << std::endl; //TOREMOVE

      int node_B_remaining_GPU = nodes[old_node_B].get_remainingGPUs();
      int GPU_used_by_B = elem_of_B->second.get_setup().get_nGPUs();
      //std::cout << "STO PER SETTARE NUMERO DI GPU: " << tmp << std::endl; //TOREMOVE
      std::cout << "al momento sul nodo di B ci sono " << node_B_remaining_GPU << " GPU rimanenti e " << GPU_used_by_B  << " usate dal Job B" << std::endl;
      
      int old_node_B_new_numGPU = node_B_remaining_GPU + GPU_used_by_B;
      std::cout << " su questo nodo voglio settare " << old_node_B_new_numGPU << std::endl;

      // l'errore è qua ! set_remaining funziona solo con unsigned e non setta ma decresce il numero delle rimanenti,
      //mentre noi volevamo usarlo per aumentare il numero
      //nodes[old_node_B].set_remainingGPUs(old_node_B_new_numGPU); 
      
      row_t row_t_B (nodes[old_node_B].get_VMtype(), nodes[old_node_B].get_GPUtype(),
		nodes[old_node_B].get_usedGPUs(), 
      const Setup new_setup_B (row_t_B);
      nodes[old_node_B].change_setup(new_setup_B);
      std::cout << "HO RI-SETTATO LE GPU ? ora le rimanenti sono " << nodes[old_node_B].get_remainingGPUs() << std::endl; //TOREMOVE

      bool assignedAtoB = assign_to_selected_node(jobA, best_stpA, temp, old_node_B);
      std::cout << "HO ASSEGNATO AL NODO?: " << assignedAtoB << std::endl; //TOREMOVE

      if (assignedAtoB)
      {

        //unsigned num_gpu_used_by_A = elem_of_A->second.get_setup().get_nGPUs();
        //temp.erase(elem_of_A);
        //std::cout << "HO SCANCELLATO IL JOB IN IDX_A" << std::endl;                                       //TOREMOVE
        std::cout << "old schedule of A " << old_A.get_selectedTime() << std::endl;                       //TOREMOVE
        std::cout << "idx_A " << idx_A << std::endl;                                                      //TOREMOVE
        std::cout << "job A " << A_job_ids[idx_A].get_ID() << std::endl;                                  //TOREMOVE
        std::cout << "nuova sch " << temp.find(A_job_ids[idx_A])->second.get_selectedTime() << std::endl; //TOREMOVE

        //update_schedule(old_A, temp.find(A_job_ids[idx_A])->second);

        std::cout << "STO PER SETTARE NUMERO DI GPU: " << std::endl;
        //auto tmpB = nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A;
        //std::cout << "Setto GPU al nuovo nodo di B: " << tmpB << std::endl;

        //nodes[old_node_A].set_remainingGPUs(nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A);

      int node_A_remaining_GPU = nodes[old_node_A].get_remainingGPUs();
      int GPU_used_by_A = elem_of_A->second.get_setup().get_nGPUs();
      //std::cout << "STO PER SETTARE NUMERO DI GPU: " << tmp << std::endl; //TOREMOVE
      std::cout << "al momento sul nodo di A ci sono " << node_A_remaining_GPU << " GPU rimanenti e " << GPU_used_by_A  << " usate dal Job A" << std::endl;
      
      int old_node_A_new_numGPU = node_A_remaining_GPU + GPU_used_by_A;
      std::cout << " su questo nodo voglio settare " << old_node_A_new_numGPU << std::endl;

      nodes[old_node_A].set_remainingGPUs(old_node_A_new_numGPU);

        std::cout << "HO RI-SETTATO LE GPU di A ? ora le rimanenti sono " << nodes[old_node_A].get_remainingGPUs() << std::endl;
        const setup_time_t &tjvg_B = ttime.at(jobB.get_ID());
        Dstar dstarB(jobB, tjvg_B, current_time);
        if (!full_greedy)
          dstarB.set_random_parameter(alpha);
        setup_time_t::const_iterator best_stpB = dstarB.get_best_setup(generator);
        bool assignedBtoA = assign_to_selected_node(jobB, best_stpB, temp, old_node_A);
        std::cout << "sto per chiamare update_schedule su B" << std::endl;
        //update_schedule(old_B, temp.find(B_job_ids[idx_B])->second);
        if (assignedBtoA)
        {
          new_schedule = temp;
        }
        else
        {
	  swap(nodes, open_nodes);
          //nodes[old_node_A].set_remainingGPUs(node_A_remaining_GPU);
          //nodes[old_node_B].set_remainingGPUs(node_B_remaining_GPU);
        }
        
      }
      else
	{
	  swap(nodes, open_nodes);
	  //nodes[old_node_B].set_remainingGPUs(node_B_remaining_GPU);
	  return local_best_schedule;
	}
      
    }
  }
  swap(new_schedule, local_best_schedule);
  return local_best_schedule;
}
