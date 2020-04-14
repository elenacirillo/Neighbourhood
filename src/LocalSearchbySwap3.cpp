#include "LocalSearchbySwap3.hpp"

LocalSearchbySwap3::LocalSearchbySwap3(const std::string &args, const std::string &d,
                                     const std::string &file_jobs, const std::string &file_times,
                                     const std::string &file_nodes) : LocalSearchbySwap(args, d, file_jobs, file_times, file_nodes)
{
}

void

LocalSearchbySwap3::fill_swapping_sets(void)
{
    A_job_ids = top_margin_jobs();
    B_job_ids = top_tardiness_waiting_jobs();
}

double LocalSearchbySwap3::evaluate_objective(job_schedule_t& j)
{
  double fft = find_first_finish_time(j);
  return objective_function(j,fft); 
}  

job_schedule_t LocalSearchbySwap3::perform_swap(const std::vector<int>& swap_indices)
{
    job_schedule_t new_schedule = local_best_schedule;
    std::vector<Node> open_nodes = nodes;
    for (unsigned idx_A = 0; idx_A < swap_indices.size(); ++idx_A)
    {
        int idx_B = swap_indices[idx_A];
        if (idx_B > -1 and idx_B < B_job_ids.size()) // and 
        // new_schedule.find(A_job_ids[idx_A])->first.get_ID() == new_schedule.find(B_job_ids[idx_B])->first.get_ID())
        {
            //std::cout << " --- index di B " << idx_B << std::endl;
            //std::cout << "   --- ciclo su indici di B" << std::endl; //TOREMOVE
            job_schedule_t temp = new_schedule;
            const auto &  elem_of_A = temp.find(A_job_ids[idx_A]);
            Schedule & old_sch_A = elem_of_A->second;
            const unsigned old_node_A = elem_of_A->second.get_node_idx();
            const Job & jobA = elem_of_A->first;
            //Node & nodeA = nodes[old_node_A];
            const auto & elem_of_B = temp.find(B_job_ids[idx_B]);
            //Schedule & old_sch_B = elem_of_B->second;
            //const unsigned old_node_B = elem_of_B->second.get_node_idx();
            const Job & jobB = elem_of_B->first;
            //Node  & nodeB = nodes[old_node_B];
            /*
                std::cout << "  -- elem_of_A contiene: " << elem_of_A->first.get_ID()<< std::endl;//TOREMOVE
                std::cout << "  -- seleziono vecchio nodo di elem_of_A: "<<old_node_A<< std::endl; //TOREMOVE
                std::cout << "  -- elem_of_B contiene: " << elem_of_B->first.get_ID()<< std::endl;//TOREMOVE
                std::cout << "  -- seleziono vecchio nodo di elem_of_B: "<<old_node_B<< std::endl; //TOREMOVE
            */	

            // prima provo ad assegnare A dove c'era B
            /*
            unsigned node_B_remaining_GPU = nodeB.get_remainingGPUs();
            unsigned GPU_used_by_B = elem_of_B->second.get_setup().get_nGPUs();
            std::cout << "STO PER SETTARE NUMERO DI GPU: " << tmp << std::endl; //TOREMOVE
            std::cout << "al momento sul nodo di B ci sono " << node_B_remaining_GPU << " GPU rimanenti e " << GPU_used_by_B  << " usate dal Job B" << std::endl;
            unsigned old_node_B_new_numGPU = node_B_remaining_GPU + GPU_used_by_B;
            std::cout << " su questo nodo voglio settare " << old_node_B_new_numGPU << std::endl;
            */

            // erase_job_from_node(old_sch_B);
            // std::cout << "HO RI-SETTATO LE GPU ? ora le rimanenti sono " << nodeB.get_remainingGPUs() << std::endl; //TOREMOVE

                //bool assignedAtoB = assign_to_selected_node(jobA, temp, old_node_B);
                //std::cout << "   --- HO ASSEGNATO A a B?: " << assignedAtoB << std::endl; //TOREMOVE

                //if (assignedAtoB)
                //{
            /*
                    //unsigned num_gpu_used_by_A = elem_of_A->second.get_setup().get_nGPUs();
                    //temp.erase(elem_of_A);
                    //std::cout << "HO SCANCELLATO IL JOB IN IDX_A" << std::endl;                                       //TOREMOVE
                    //std::cout << "old schedule of A " << old_A.get_selectedTime() << std::endl;                       //TOREMOVE
                    //std::cout << "idx_A " << idx_A << std::endl;                                                      //TOREMOVE
                    //std::cout << "job A " << A_job_ids[idx_A].get_ID() << std::endl;                                  //TOREMOVE
                    //std::cout << "nuova sch " << temp.find(A_job_ids[idx_A])->second.get_selectedTime() << std::endl; //TOREMOVE

                    //std::cout << "STO PER SETTARE NUMERO DI GPU: " << std::endl;
                    //auto tmpB = nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A;
                    //std::cout << "Setto GPU al nuovo nodo di B: " << tmpB << std::endl;

                    //nodes[old_node_A].set_remainingGPUs(nodes[old_node_A].get_remainingGPUs() + num_gpu_used_by_A)

                    unsigned node_A_remaining_GPU = nodes[old_node_A].get_remainingGPUs();
                    unsigned GPU_used_by_A = elem_of_A->second.get_setup().get_nGPUs();
                    //std::cout << "STO PER SETTARE NUMERO DI GPU: " << tmp << std::endl; //TOREMOVE
                //    std::cout << "al momento sul nodo di A ci sono " << node_A_remaining_GPU << " GPU rimanenti e " << GPU_used_by_A  << " usate dal Job A" << std::endl;
                
                    unsigned old_node_A_new_numGPU = node_A_remaining_GPU + GPU_used_by_A;
                //   std::cout << " su questo nodo voglio settare " << old_node_A_new_numGPU << std::endl;

                    //nodes[old_node_A].set_remainingGPUs(old_node_A_new_numGPU);
            */ // TO REMOVE
            erase_job_from_node(old_sch_A);

            // std::cout << "HO RI-SETTATO LE GPU di A ? ora le rimanenti sono " << nodes[old_node_A].get_remainingGPUs() << std::endl;
            bool assignedBtoA = assign_to_selected_node(jobB, temp, old_node_A);
            temp[jobA]=Schedule();
            //std::cout << "   --- HO ASSEGNATO B a A? : " << assignedBtoA << std::endl;

            if (assignedBtoA)
            {
                std::cout << "   --- swap " << idx_A << " RIUSCITO !" << std::endl;
                swap(new_schedule,temp);
                std::cout << jobA.get_ID() << "ha schedule empty" << new_schedule[jobA].isEmpty()<< std::endl;
                std::cout << jobB.get_ID() << "non ha schedule empty" << new_schedule[jobB].isEmpty()<< std::endl;
    
             }
            else
            {
        	    //std::cout << "   --- swap " << idx_A << " non riuscito" << std::endl;	
                //std::cout << "B NON ASSEGNATO AD A" << std::endl;
	            swap(nodes, open_nodes);
                //nodes[old_node_A].set_remainingGPUs(node_A_remaining_GPU);
                //nodes[old_node_B].set_remainingGPUs(node_B_remaining_GPU);
            }
        }
        else
        {
            //std::cout << "   --- swap " << idx_A << " non riuscito" << std::endl;
      	    //std::cout << "A NON ASSEGNATO A B" << std::endl;
	        swap(nodes, open_nodes);
	        //nodes[old_node_B].set_remainingGPUs(node_B_remaining_GPU);
            return new_schedule;
        }
    }   
  return new_schedule;
}


