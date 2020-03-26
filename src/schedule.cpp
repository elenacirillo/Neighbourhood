#include "schedule.hpp"

Schedule::Schedule (setup_time_t::const_iterator s, unsigned n):
  csetup(s), node_idx(n), empty_schedule(false)
{}

const Setup&
Schedule::get_setup (void) const
{
  assert(! empty_schedule);
  return csetup->first;
}

double 
Schedule::get_selectedTime (void) const 
{
  double t = std::numeric_limits<double>::infinity();
  if (! empty_schedule)
    t = csetup->second;
  return t;
}

unsigned
Schedule::get_node_idx (void) const
{
  assert(! empty_schedule);
  return node_idx;
}

void
Schedule::set_tardiness (double t) 
{
  tardiness = t;
  if (tardiness < 1e-7)
    tardinessCost = 0.0;
}

void
Schedule::compute_vmCost (unsigned g)
{
  if (! empty_schedule)
  {
    vmCost = execution_time * (csetup->first).get_cost() / 3600 * 
             (csetup->first).get_nGPUs() / g;
  }
}

void
Schedule::compute_tardinessCost (double tw)
{
  if (! empty_schedule)
    tardinessCost = tardiness * tw;
}

void
Schedule::print_names (std::ostream& ofs, char endline)
{
  ofs << "n_iterate,sim_time,";
  Job::print_names(ofs,',');
  ofs << "ExecutionTime,CompletionPercent,StartTime,FinishTime,";
  Node::print_names(ofs,',');
  Setup::print_names(ofs,',');
  ofs << "Tardiness,VMcost,TardinessCost,TotalCost" << endline;
}

void
Schedule::print (const Job& j, const std::vector<Node>& nodes, 
                 std::ostream& ofs, char endline) const
{
  ofs << iter << ',' << sim_time << ',';
  j.print(ofs,',');
  ofs << execution_time << ',' << completion_percent << ',' 
      << start_time     << ',' << finish_time        << ',';
  if (! empty_schedule)
  {
    nodes[node_idx].print(ofs,',');
    (csetup->first).print(ofs,',');
  }
  else
  {
    Node n;
    n.print(ofs, ',');
    Setup s;
    s.print(ofs,',');
  }
  ofs << tardiness << ',' << vmCost << ',' << tardinessCost << ',' 
      << (vmCost+tardinessCost) << endline;
}