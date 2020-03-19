import os
import ast
import configparser as cp
import argparse
import csv
import logging

import collections

import pandas as pd


def recursivedict():
    return collections.defaultdict(recursivedict)


def main ():

    parser = argparse.ArgumentParser(description="Compute averages of random methods")

    parser.add_argument("results_directory", help="The directory containing the folders with the results to be processed")
    parser.add_argument('-c', "--config", help="Configuration file", 
                        default="config.ini")
    parser.add_argument('-d', "--debug", help="Enable debug messages", 
                        default=False, action="store_true")
    parser.add_argument('-o', "--output", help="String to be added to the name of the results files", 
                        default="")

    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')
    else:
        logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

    if not os.path.exists(args.results_directory):
        logging.error("%s does not exist", args.results_directory)
        sys.exit(1)

    ##########################################################################

    ## configuration parameters
    config = cp.ConfigParser()
    config.read(args.config)

    # methods
    baseline_method = ast.literal_eval(config['Methods']['baseline_method'])

    ##########################################################################

    costs = recursivedict()

    for file in os.listdir(args.results_directory):
        combined_path = os.path.join(args.results_directory, file)
        if os.path.isdir(file):
            continue
        if not file.startswith("total_costs_averaged"):
            continue

        logging.debug("Examining %s", str(file))

        cost_data = csv.reader(open(combined_path, "r"))
        next(cost_data)
        
        for row in cost_data:
            method = row[0]
            nodes = row[1]
            jobs = row[2]
            lambdaa = row[3]
            mu = row[4]
            seed = row[5]
            cost = float(row[6])

            experiment = (nodes, jobs, lambdaa, mu, seed)

            costs[experiment][method] = cost

    results_file_name = "results" + args.output + ".csv"
    results_file = open(results_file_name, "w")
    results_file.write("Nodes, Jobs, Lambda, Mu, Seed, Baseline, Best_Greedy, ")
    results_file.write("Best_Random, Gain (baseline VS best greedy), ")
    results_file.write("Gain (baseline VS best random), ")
    results_file.write("Gain (best greedy VS best random), TC baseline, ")
    results_file.write("TC best_greedy, TC best_random\n")

    for experiment in sorted(costs):
        logging.debug("Examining %s", str(experiment))
        results_file.write(",".join(experiment))
        results = costs[experiment]

        best_greedy = ()
        best_random = ()
        baseline = ()

        for method in results:
            cost = results[method]
            if method.endswith("_R"):
                if not best_random or cost < best_random[1]:
                    best_random = (method, cost)
            else:
                if not best_greedy or cost < best_greedy[1]:
                    best_greedy = (method, cost)
            if method == baseline_method:
                baseline = (method, cost)

        gain_b_gr = (baseline[1] - best_greedy[1]) / baseline[1]
        gain_b_rand = (baseline[1] - best_random[1]) / baseline[1]
        gain_gr_rand = (best_greedy[1] - best_random[1]) / best_greedy[1]
        results_file.write(", " + baseline[0] + ", " + best_greedy[0] +\
                           ", " + best_random[0] +\
                           ", {:.2f}%".format(gain_b_gr * 100) +\
                           ", {:.2f}%".format(gain_b_rand * 100) +\
                           ", {:.2f}%".format(gain_gr_rand * 100) +\
                           ", {:.2f}".format(baseline[1]) +\
                           ", {:.2f}".format(best_greedy[1]) +\
                           ", {:.2f}".format(best_random[1]))
        results_file.write("\n")

    results_file.close()


if __name__ == "__main__":
    main()