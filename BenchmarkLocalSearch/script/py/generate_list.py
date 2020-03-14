import os
import ast
import configparser as cp
import argparse

def createFolder(directory):
    try:
        if not os.path.exists(directory):
            os.makedirs(directory)
    except OSError:
        print ("Error: Creating directory. " +  directory)


def main ():

    parser = argparse.ArgumentParser(description="Generate input list")

    parser.add_argument('-c', "--config", help="Configuration file", 
                        default="config.ini")

    args = parser.parse_args()

    ##########################################################################

    ## configuration parameters
    config = cp.ConfigParser()
    config.read(args.config)

    # number of nodes
    nodes_min         = int(config['Nodes']['nodes_min'])
    nodes_max         = int(config['Nodes']['nodes_max'])
    nodes_step        = int(config['Nodes']['nodes_step'])

    # number of instances
    instances       = int(config['Instances']['instances'])
    from_seed       = int(config['Instances']['from_seed'])

    # parameters for randomization
    lambdaas        = ast.literal_eval(config['RandomParameters']['lambdaas'])
    n_random_iter   = int(config['RandomParameters']['n_random_iter'])
    n_cpp_seed      = int(config['RandomParameters']['n_cpp_seed'])

    # methods
    methods         = ast.literal_eval(config['Methods']['methods'])
    existing_cpp    = ast.literal_eval(config['Methods']['existing_cpp'])

    # jobs
    nInitialJ       = int(config['Jobs']['nInitialJ'])
    job_times_nodes = int(config['Jobs']['job_times_nodes'])

    # other parameters for Arezoo* models
    mu              = int(config['OtherParams']['mu'])
    delta           = float(config['OtherParams']['delta'])

    ##########################################################################

    ## list generation
    folder = "build/data/lists/"
    createFolder(folder)
    List_file = open(folder + "list_of_data.txt", "w")
    LList_file = open(folder + "list_of_inputs.txt", "w")
    nodes = nodes_min
    
    while nodes <= nodes_max:
        print(nodes)
    
        Jobs = job_times_nodes * nodes
    
        for lambdaa in lambdaas:
        
            for i in range(instances):
                myseed = 1000 * ((i+from_seed+1)**2)
            
                inputs = str(nInitialJ) + " " + str(nodes) + " " + str(Jobs) +\
                         " " + str(lambdaa) + " " + str(mu) + " " +\
                         str(myseed)
                List_file.write(inputs+"\n")

                for method in methods:
                    if method in existing_cpp:
                        if method.endswith("_R"):
                            ncppseed = n_cpp_seed
                        else:
                            ncppseed = 1

                        for j in range(ncppseed):
                            cppseed = myseed + 10*(j+2)

                            Linputs = method + " " + inputs + " " +\
                                      str(delta) +\
                                      " " + str(cppseed) + " " +\
                                      str(n_random_iter)
                            LList_file.write(Linputs+"\n")

        nodes = nodes + nodes_step

    List_file.close()
    

if __name__ == "__main__":
    main()