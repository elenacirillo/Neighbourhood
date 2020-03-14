#include <iostream>
#include <cstring>

#include "greedy_version0.hpp"
#include "greedy_version1.hpp"
#include "greedy_version2.hpp"
#include "greedy_version3.hpp"
#include "greedy_version4.hpp"
#include "LocalSearchbySwap.hpp"
#include "LocalSearchGPU.hpp"
#include "analysis.hpp"
#include "builders.hpp"

void print_help (void);

int main (int argc, char *argv[])
{
  if (argc < 8)
  {
    if (argc < 2 || (strcmp(argv[1],"-h")!=0 && strcmp(argv[1],"--help")!=0))
      std::cerr << "\nERROR: too few or wrong arguments\n" << std::endl;
    print_help();
  }
  else
  {
    // create factory (TODO: move this where it should be)
    typedef std::unique_ptr<BaseBuilder<Heuristic>> GreedyBuilder;
    typedef std::map<std::string, GreedyBuilder> factory_t;
    factory_t factory;
    factory["FedeCpp"]    = GreedyBuilder(new Builder<Greedy_version0, Heuristic>);
    factory["FedeCpp1"]   = GreedyBuilder(new Builder<Greedy_version1, Heuristic>);
    factory["FedeCpp2"]   = GreedyBuilder(new Builder<Greedy_version2, Heuristic>);
    factory["FedeCpp3"]   = GreedyBuilder(new Builder<Greedy_version3, Heuristic>);
    factory["FedeCpp4"]   = GreedyBuilder(new Builder<Greedy_version4, Heuristic>);
    factory["FedeCpp_R"]  = GreedyBuilder(new Builder<Greedy_version0, Heuristic>);
    factory["FedeCpp1_R"] = GreedyBuilder(new Builder<Greedy_version1, Heuristic>);
    factory["FedeCpp2_R"] = GreedyBuilder(new Builder<Greedy_version2, Heuristic>);
    factory["FedeCpp3_R"] = GreedyBuilder(new Builder<Greedy_version3, Heuristic>);
    factory["FedeCpp4_R"] = GreedyBuilder(new Builder<Greedy_version4, Heuristic>);
    factory["LocalSearchbySwap"] = GreedyBuilder(new Builder<LocalSearchbySwap, Heuristic>);
    //factory["LocalSearchGPU"] = GreedyBuilder(new Builder<LocalSearchGPU, Heuristic>);

    // method
    std::string method = argv[1];

    // ARGS = "nInitialJ-nN-nJ-lambdaa-mu-myseed"
    std::ostringstream oss;
    for (unsigned j=2; j<8; ++j)
      oss << argv[j] << ((j < 7) ? "-" : "");
    std::string ARGS = oss.str();

    // delta = "-delta"
    std::string delta = "-" + std::string(argv[8]);

    // names of folders:
    //    for data: data_folder/nInitialJ-nN-nJ-lambdaa-mu-myseed/
    //    for results: result_folder/nInitialJ-nN-nJ-lambdaa-mu-myseed-delta/

    // name of files containing data
    std::string jobs_list_filename = "Lof_selectjobs_forCPP.csv";
    std::string times_filename = "SelectJobs_times.csv";
    std::string nodes_filename = "tNodes.csv";

    // initialization of greedy
    factory_t::const_iterator where = factory.find(method);
    if (where != factory.end())
    {
      std::unique_ptr<Heuristic> G = where->second->create(ARGS, delta,
                                                        jobs_list_filename,
                                                        times_filename,
                                                        nodes_filename);

      // parameter for randomization and number of random iterations
      unsigned cpp_seed = 0;
      unsigned n_random_iter = 0;
      std::string result_filename = method + "_schedule.csv";
      if (method == "FedeCpp_R"  || method == "FedeCpp1_R" ||
          method == "FedeCpp2_R" || method == "FedeCpp3_R" ||
          method == "FedeCpp4_R")
      {
        if (argc < 10)
          std::cerr << "\nERROR: additional arguments are required "
                    << "for random methods\n" << std::endl;
        else
        {
          cpp_seed = std::stoi(argv[9]);
          n_random_iter = std::stoi(argv[10]);
          result_filename = method + "_schedule_" + std::to_string(cpp_seed) +
                            ".csv";
        }
      }

      // algorithm
      G->algorithm(cpp_seed, n_random_iter);

      // print resulting schedule
      G->print_schedule(result_filename);

      // analysis
      Analysis A(ARGS, delta);
      A.perform_analysis(result_filename);
      A.print(method, cpp_seed);
    }
    else
      std::cerr << "ERROR: method " << method << " is not registered"
                << std::endl;
  }
  return 0;
}

void print_help (void)
{
  std::cout << "usage: ./main [arguments]\n";
  std::cout << "\nrequired arguments (for all methods):\n";
  std::cout << "\tmethod:       name of the method you want to use\n";
  std::cout << "\tnInitialJ:    number of initial jobs\n";
  std::cout << "\tnN:           number of nodes\n";
  std::cout << "\tnJ:           number of jobs\n";
  std::cout << "\tlambda:       average of inter-arrival times for generated jobs\n";
  std::cout << "\tmu:           (set equal to 1)\n";
  std::cout << "\tseed:         seed used in data generation\n";
  std::cout << "\tdelta:        (set equal to 0.5)\n";
  std::cout << "\nadditional arguments (for random methods):\n";
  std::cout << "\tcpp_seed:     seed for randomization\n";
  std::cout << "\tnRandomIter:  number of random iterations\n" << std::endl;
}
