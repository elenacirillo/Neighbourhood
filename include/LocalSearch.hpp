#ifndef LOCAL_SEARCH_HH
#define LOCAL_SEARCH_HH

#include "greedy_version4.hpp"
#include <set>
#include <unordered_set>
#include <algorithm>

class LocalSearch : public Greedy_version4 {

protected:

	// maximum number of local search iterations
	unsigned long MAX_ITER = 10;

	// tolerance
	double TOL;

	/*
    *  binary parameter to decide the type of search:
    *  		"true" if best improvement
    *  		"false" if 1st improvement
	*/
	bool best_fit = false;

	// size of the neighborhood
    unsigned neigh_size = 3;

	// tabu list (tabu search optimization)
	std::vector<job_schedule_t> previous_best; // per non ricadere in ottimi locali già visitati nelle iterazioni
    
    // schedule after Federica's random iteration
    job_schedule_t initial_schedule; // TODO: vedere se si può rimuovere

    // best schedule found by local search so far
    job_schedule_t local_best_schedule;
    
    // obj function value of the best schedule found by local search so far
    double best_schedule_value_t = INF;

	double evaluate_objective(job_schedule_t& job_schedule) const;
    //double evaluate_objective(job_schedule_t& job_schedule, double elapsed_time);
	// evaluate non può essere const perch objective_function chiamata al suo interno non è const
	// allo stesso modo non può prendere in ingresso un const job_schedule
    
    // 
    virtual bool visit_neighbor() = 0;
	
	//
	virtual bool assign_to_selected_node (const Job&,
                                        setup_time_t::const_iterator,
                                        job_schedule_t&, unsigned);

    //
	double true_first_finish_time(job_schedule_t& job_schedule) const;
	//
	double true_last_finish_time(job_schedule_t& job_schedule) const;

	// 
	bool perform_local_search(job_schedule_t&);

	//oveeride perform_scheduling, apply LocalSearch at the end
	job_schedule_t perform_scheduling (unsigned max_random_iter) override;

	bool update_schedule (Schedule & old_sch, Schedule & new_sch);

	bool equalSchedule(const Schedule & lhs, const Schedule & rhs);

public:
	// constructor
    LocalSearch(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearch (void) = default;

};

#endif /* LOCAL_SEARCH_HH */
