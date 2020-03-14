#ifndef LOCAL_SEARCH_HH
#define LOCAL_SEARCH_HH

#include "greedy_version4.hpp"
#include <algorithm>

class LocalSearch : public Greedy_version4 {

protected:
	// maximum number of local search iterations
	unsigned long MAX_ITER;
	// tolerance
	double TOL;
	/*
    *  binary parameter to decide the type of search:
    *  		"true" if best improvement
    *  		"false" if 1st improvement
	*/
	bool best_fit = false; // TODO: initialize it in the constructor and update the code of the functions that uses this datum

	// tabu list (tabu search optimization)
	std::vector<job_schedule_t> previous_best; // per non ricadere in ottimi locali già visitati nelle iterazioni
    job_schedule_t initial_schedule;
    job_schedule_t best_schedule;
    double best_schedule_value_t = INF;
	double evaluate_objective(job_schedule_t& job_schedule) const;
    //double evaluate_objective(job_schedule_t& job_schedule, double elapsed_time);
	// evaluate non può essere const perch objective_function chiamata al suo interno non è const
	// allo stesso modo non può prendere in ingresso un const job_schedule
    virtual bool visit_neighbor() = 0;
	virtual bool assign_to_selected_node (const Job&,
                                        setup_time_t::const_iterator,
                                        job_schedule_t&, unsigned);

    //
	double true_first_finish_time(job_schedule_t& job_schedule) const;
	//
	double true_last_finish_time(job_schedule_t& job_schedule) const;

	void search_better_schedule(job_schedule_t&);

	bool update_best_schedule (job_schedule_t& new_schedule,
                              double& minTotalCost, 
                              job_schedule_t& best_schedule,
                              std::vector<Node>& opened_nodes) override;

	bool update_schedule (Schedule & old_sch, Schedule & new_sch);

	bool equalSchedule(const Schedule & lhs, const Schedule & rhs);

public:
    LocalSearch(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearch (void) = default;

};

#endif /* LOCAL_SEARCH_HH */
