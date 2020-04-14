#ifndef LOCAL_SEARCH_HH
#define LOCAL_SEARCH_HH

#include "greedy_version4.hpp"
#include <set>
#include <unordered_set>
#include <algorithm>

class LocalSearch : public Greedy_version4 {

protected:

	// TODO: rimuovere
	// tiene traccia di quante volte la Local Search ha successo
	unsigned LS_count = 0;

	// maximum number of local search iterations
	unsigned long MAX_ITER = 10;

	/*
    *  binary parameter to decide the type of search:
    *  		"true" if best improvement
    *  		"false" if 1st improvement
	*/
	bool best_fit = true;

	// size of the neighborhood
    unsigned neigh_size = 4;

	// tabu list (tabu search optimization)
	//std::vector<job_schedule_t> previous_best;
    
    // schedule after Federica's random iteration
    //job_schedule_t initial_schedule; // TODO: vedere se si può rimuovere

    // best schedule found by local search so far
    job_schedule_t local_best_schedule;
    
    // obj function value of the best schedule found by local search so far
    double best_schedule_value_t = INF;

	virtual double evaluate_objective(job_schedule_t&);
    //double evaluate_objective(job_schedule_t&) const;
    //double evaluate_objective(job_schedule_t& job_schedule); // NON CONST perchè chiama objective_function che non lo è
    
    // 
    virtual bool visit_neighbor() = 0;

	//
	virtual bool assign_to_selected_node (const Job&,
                                        job_schedule_t&, unsigned);

	/* NUOVA VERSIONE ASSEGNAMENTO A NODO CHE FA ANCHE IL CONTROLLO SUL SUBOTTIMO
	bool assign_to_selected_node (const Job&, job_schedule_t&, unsigned);
	*/

	//  last_finish_time come Federica fa il first
	double find_last_finish_time (const job_schedule_t&) const;

	// 
	bool perform_local_search(job_schedule_t&);

	//oveeride perform_scheduling, apply LocalSearch at the end
	job_schedule_t perform_scheduling (unsigned max_random_iter) override;

	bool update_schedule (Schedule & old_sch, Schedule & new_sch);

	bool equalSchedule(const Schedule & lhs, const Schedule & rhs); // 

public:
	// constructor
    LocalSearch(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearch (void) = default;

	// NICO
	void printer(job_schedule_t& job_schedule);

};

#endif /* LOCAL_SEARCH_HH */
