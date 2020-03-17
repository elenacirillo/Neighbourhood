#ifndef JOB_HH
#define JOB_HH

#include <array>
#include <vector>
#include <ostream>
#include <limits>

#include "utilities.hpp"

class Job {

public:
  // type definitions
  typedef std::array<std::string,6> info_t;
  typedef std::array<double,5> time_info_t;

  // friend operator==
  friend bool operator== (const Job&, const Job&);

private:
  
  // job_info = ['Application', 'Images', 'Epochs', 'Batchsize',
  //             'OriginalID', 'ID']
  info_t job_info;

  // time_info = ['SubmissionTime', 'Deadline', 'TardinessWeight', 
  //              'MinExecutionTime', 'MaxExecutionTime']
  time_info_t time_info;

  // pressure = MinExecutionTime - Deadline
  double pressure;

public:
  /*  constructors
  *
  *   Input(1):  void              default
  *
  *   Input(2):  const row_t&      list of elements used to initialize all the
  *                                class members (see utilities.hpp to inspect
  *                                row_t type)
  */
  Job (void) = default;
  Job (const row_t&);
  
  // getters
  const std::string& get_originalID (void) const {return job_info[4];}
  const std::string& get_ID (void) const {return job_info[5];} 
  double get_submissionTime (void) const {return time_info[0];}
  double get_deadline (void) const {return time_info[1];}
  double get_tardinessWeight (void) const {return time_info[2];}
  double get_minExecTime (void) const {return time_info[3];}
  double get_maxExecTime (void) const {return time_info[4];}
  double get_pressure (void) const {return pressure;}

  // setters
  void set_minExecTime (double m) {time_info[3] = m;}
  void set_maxExecTime (double M) {time_info[4] = M;}

  /*  update_pressure
  *
  *   Input:  double   current_time
  *                    pressure = current_time + min_exec_time - Deadline
  */
  void update_pressure (double);

  /*  print_names (static)
  *
  *   Input:  std::ostream&       where to print names of fields stored in 
  *                               job_info and time_info
  *           char endline='\n'   last character to be printed (default \n)
  */
  static void print_names (std::ostream&, char endline='\n');

  /*  print
  *
  *   Input:  std::ostream&       where to print job info
  *           char endline='\n'   last character to be printed (default \n)
  */
  void print (std::ostream&, char endline='\n') const;
  
};

// operator==   two jobs are equal if they have the same ID
bool operator== (const Job&, const Job&);

// specialization of the hash function
namespace std {    
  template<>
  struct hash<Job> {
    std::size_t operator() (const Job& j) const {
        return std::hash<std::string>() (j.get_ID());
    }
  };
}

#endif /* JOB_HH */