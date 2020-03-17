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
    parser.add_argument('-o', "--output", 
                        help="String to be added to the name of the results files", 
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
    existing_cpp    = ast.literal_eval(config['Methods']['existing_cpp'])

    ##########################################################################

    costs = recursivedict()

    found_methods = set()

    for content in os.listdir(args.results_directory):
        combined_path = os.path.join(args.results_directory, content)
        if os.path.isdir(combined_path):
            for file in os.listdir(combined_path):
                if os.path.isdir(file):
                    continue
                if not file.startswith("cost-"):
                    continue
                tokens = file.split("-")
                if tokens[0] != "cost":
                    continue
                
                method = tokens[1]
                if method not in existing_cpp:
                    continue
                is_random = (method.endswith("_R"))
                
                number_initial_jobs = tokens[2]
                if number_initial_jobs != "1":
                    continue
                nodes = tokens[3]
                jobs = tokens[4]
                lambdaa = tokens[5]
                mu = tokens[6]
                seeds = tokens[7].replace(".csv", "")
                
                if is_random:
                    seeds = seeds.split("_")
                    seed = seeds[0]
                    cppseed = seeds[1]
                    # TEMP: only for backward compatibility
                    #if len(seeds)==1:
                    #    cppseed = str(int(seed) + 10)
                    #else:
                    #    cppseed = seeds[1]
                else:
                    seed = seeds
                    cppseed = seed

                total_cost = 0.0

                logging.debug("Examining %s", str(method + "-" + cppseed))

                cost_data = csv.reader(open(os.path.join(combined_path, file), 
                                            "r"))
                next(cost_data)
                for row in cost_data:
                    total_cost = total_cost + float(row[4])

                experiment = (method, nodes, jobs, lambdaa, mu, seed)

                found_methods.add(method)

                costs[experiment][cppseed] = total_cost
    logging.debug("Found methods %s", str(found_methods))

    if not found_methods:
        logging.error("No cpp result found")
        sys.exit(1)

    results_file_name = "total_costs" + args.output + ".csv"
    results_file = open(results_file_name, "w")
    results_file.write("Method, Nodes, Jobs, Lambda, Mu, Seed, CppSeed, TotalCost")
    results_file.write("\n")

    averages_file_name = "total_costs_averaged" + args.output + ".csv"
    averages_file = open(averages_file_name, "w")
    averages_file.write("Method, Nodes, Jobs, Lambda, Mu, Seed, TotalCost")
    averages_file.write("\n")

    for experiment in sorted(costs):
        logging.debug("Examining %s", str(experiment))

        results = costs[experiment]

        results_file.write(",".join(experiment))
        averages_file.write(",".join(experiment))

        average = 0.0
        ncppseed = 0

        for cppseed in results:
            if ncppseed == 0:
                results_file.write("," + str(cppseed) +\
                                   "," + str(results[cppseed]) + "\n")
            else:
                results_file.write(",,,,,," + str(cppseed) +\
                                   "," + str(results[cppseed]) + "\n")
            average += results[cppseed]
            ncppseed += 1

        average /= ncppseed
        averages_file.write("," + str(average) + "\n")

    results_file.close()
    averages_file.close()


if __name__ == "__main__":
    main()