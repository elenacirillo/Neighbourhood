#ifndef FILEIO_HH
#define FILEIO_HH

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "utilities.hpp"
#include "job.hpp"
#include "setup.hpp"
#include "node.hpp"
#include "schedule.hpp"

/*  read_csv
*     reads a csv file and stores the content in a table (see utilities.hpp 
*     for further information about the used type).
*
*   NOTE: the first row of the csv file is assumed to store the name 
*         of the relative columns, thus its content is dropped
*
*   Input:  std::ifstream&    file to be read
*           table_t&          table where to store values read from file
*/
void read_csv (std::ifstream&, table_t&);

/*  create_container
*     reads a csv file using read_csv and stores the relative information in 
*     the given container
*
*   NOTE (1): the container can be of whatever type ExtT s.t. the method 
*             insert is supported (e.g. std::vector, std::map, etc.)
*
*   NOTE (2): the internal type IntT of the elements to be stored in the 
*             container can be whatever type s.t. it has a constructor that 
*             takes as parameter an element of type row_t (e.g. Job, Node,etc.)
*
*   Input:  ExtT<IntT>&         container to be filled
*           const std::string&  name of file storing information to fill the
*                               container
*/
template <template <typename...> class ExtT, typename IntT, typename ... Ts>
void
create_container (ExtT<IntT,Ts...>&, const std::string&);

/*  print_container
*     prints the elements of a given container on a file whose name is passed
*     as parameter
*
*   NOTE (1): the container can be of whatever type ExtT s.t. range for on its 
*             elements is supported (e.g. std::vector, std::map, etc.)
*
*   NOTE (2): the internal type IntT of the elements stored in the 
*             container can be whatever type s.t. methods print_names and 
*             print are implemented (e.g. Job, Node, etc.)
*
*   Input:  const ExtT<IntT>&   container to be printed
*           const std::string&  name of file where to print the container
*/
template <template <typename...> class ExtT, typename IntT, typename ... Ts>
void
print_container (const ExtT<IntT,Ts...>&, const std::string&);

/*  create_map
*     creates a map storing information about jobs and their execution times
*     in different configurations, reading information from a csv file
*
*   Input:  time_table_t&       map to be filled (see utilities.hpp)
*           const std::string&  name of file storing information
*/
void
create_map (time_table_t&, const std::string&);

/*  print_map
*     prints the map storing information about jobs and their execution times
*     in different configurations, reading information from a csv file
*
*   Input:  time_table_t&       map to be printed (see utilities.hpp)
*           const std::string&  name of file where to print the map
*/
void
print_map (const time_table_t&, const std::string&);

/*  print_result
*
*   Input:  const std::vector<job_schedule_t>&    full schedule to be printed
*           const std::vector<Node>&              vector of nodes
*           const std::string&                    filename
*/
void
print_result (const std::vector<job_schedule_t>&, const std::vector<Node>&,
              const std::string&);


// implementation of template functions
//
template <template <typename...> class ExtT, typename IntT, typename ... Ts>
void
create_container (ExtT<IntT,Ts...>& cont, const std::string& filename)
{
  // open file
  std::ifstream ifs(filename);

  if (ifs)
  {
    // read file
    table_t table;
    read_csv(ifs, table);

    for (typename table_t::const_iterator it = table.cbegin();
         it != table.cend(); ++it)
    {
      // add new element to the cont
      cont.insert(cont.end(),IntT(*it));
    }
  }
  else
    std::cerr << "ERROR in create_container: file " << filename
              << " cannot be opened" << std::endl;
}
//
//
template <template <typename...> class ExtT, typename IntT, typename ... Ts>
void
print_container (const ExtT<IntT,Ts...>& cont, const std::string& filename)
{
  // open file
  std::ofstream ofs(filename);

  if (ofs)
  {
    IntT::print_names(ofs);
    for (const IntT& elem : cont)
      elem.print(ofs);
  }
  else
    std::cerr << "ERROR in print_container: file " << filename
              << " cannot be opened" << std::endl;
}


#endif /* FILEIO_HH */