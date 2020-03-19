#include "fileIO.hpp"

void
read_csv (std::ifstream& ifs, table_t& table)
{
  std::string line;

  // drop first line
  getline(ifs, line);

  // read file and store values in table
  while (getline(ifs, line))
  {
    std::istringstream iss(line);
    std::string cell;

    // read all values in a row
    row_t row;
    while (getline(iss, cell, ','))
      row.push_back(cell);

    // store new row in table
    table.push_back(row);
  }
}

void
create_map (time_table_t& ttime, const std::string& filename)
{
  // open file
  std::ifstream ifs(filename);

  if (ifs)
  {
    // read file
    table_t table;
    read_csv(ifs, table);
    typename table_t::const_iterator it;

    for(it = table.cbegin(); it != table.cend(); ++it)
    {
      row_t row = *it;
      row_t stp = {row[1], row[2], row[3], row[4], row[5]};
      ttime[row[0]].insert({Setup(stp), std::stod(row[6])});
    }
  }
  else
    std::cerr << "ERROR in create_map: file " << filename
              << " cannot be opened" << std::endl;
}

void
print_map (const time_table_t& ttime, const std::string& filename)
{
  // open file
  std::ofstream ofs(filename);

  if (ofs)
  {
    ofs << "UniqueJobsID,";
    Setup::print_names(ofs,',');
    ofs << "ExecutionTime\n";
    time_table_t::const_iterator it;
    for(it = ttime.cbegin(); it != ttime.cend(); ++it)
    {
      const setup_time_t& tjvg = it->second;
      setup_time_t::const_iterator it2;
      for(it2 = tjvg.cbegin(); it2 != tjvg.cend(); ++it2)
      {
        ofs << it->first << ',';
        (it2->first).print(ofs,',');
        ofs << it2->second << '\n';
      }
    }
  }
  else
    std::cerr << "ERROR in print_map: file " << filename
              << " cannot be opened" << std::endl;
}

void
print_result (const std::vector<job_schedule_t>& res, 
              const std::vector<Node>& nodes,
              const std::string& filename)
{
  // open file
  std::ofstream ofs(filename);

  if (ofs)
  {
    Schedule::print_names(ofs);
    for (const job_schedule_t& jsmap : res)
    {
      job_schedule_t::const_iterator cit;
      for (cit = jsmap.cbegin(); cit != jsmap.cend(); ++cit)
      {
        const Job& j = cit->first;
        const Schedule& sch = cit->second;
        sch.print(j, nodes, ofs);
      }
    }
  }
  else
    std::cerr << "ERROR in print_result: file " << filename
              << " cannot be opened" << std::endl;
}