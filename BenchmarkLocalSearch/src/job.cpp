#include "job.hpp"

Job::Job (const row_t& info)
{
  typename row_t::const_iterator it = info.cbegin();
  for (unsigned k=0; k<job_info.size(); ++k)
    job_info[k] = *it++;

  for (unsigned k=0; k<time_info.size(); ++k)
    time_info[k] = std::stod(*it++);

  pressure = time_info[3] - time_info[1];
}

void
Job::update_pressure (double current_time)
{
  // pressure = min_exec_time + current_time - deadline
  pressure = time_info[3] + current_time - time_info[1];
}

void 
Job::print (std::ostream& ofs, char endline) const
{
  for (unsigned j=0; j<job_info.size(); ++j)
    ofs << job_info[j] << ",";
  
  for (unsigned j=0; j<time_info.size(); ++j)
    ofs << time_info[j] << ((j<time_info.size()-1) ? ',' : endline);
}

void
Job::print_names (std::ostream& ofs, char endline)
{
  ofs << "Application,Images,Epochs,Batchsize,Jobs,UniqueJobsID,"
      << "SubmissionTime,Deadline,Tardinessweight,MinExecTime,MaxExecTime"
      << endline;
}

bool
operator== (const Job& j1, const Job& j2)
{
  return (j1.job_info[5] == j2.job_info[5]);
}