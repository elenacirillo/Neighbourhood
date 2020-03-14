#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Dec 16 19:38:35 2019

@author: federicafilippini
"""

import argparse
import datetime
import distutils.spawn
import logging
import os
import re
import shlex
import shutil
import signal
import subprocess
import sys
import threading


def positive_integer(value):
    pos_int = int(value)
    if pos_int <= 0:
        raise argparse.ArgumentTypeError("%s must be a positive integer" % value)
    return pos_int

class StoreOrUpdateMin(argparse.Action):
    first_parsed = True
    def __call__(self, parser, namespace, values, option_string=None):
        if self.first_parsed == True:
            self.first_parsed = False
            setattr(namespace, self.dest, values)
        else:
            setattr(namespace, self.dest, min(namespace.j, values))


#Return children of a process
def GetChildren(parent_pid):
    ret = set()
    ps_command = subprocess.Popen("ps -o pid --ppid %d --noheaders" % parent_pid, shell=True, stdout=subprocess.PIPE)
    ps_output = ps_command.stdout.read()
    ps_command.wait()
    for pid_str in ps_output.split("\n")[:-1]:
        ret.add(int(pid_str))
    return ret


#Kill a process than kill its children
def kill_proc_tree(pid):
    children = GetChildren(pid)
    os.kill(pid, signal.SIGKILL)
    for child in children:
        kill_proc_tree(child)


#Process benchmark in list
def execute_tests(named_list,thread_index):
    global passed_benchmark
    global total_benchmark
    global line_index
    global children
    global failure
    lines = open(named_list).readlines()
    with lock:
        local_index = line_index
        line_index += 1
    while local_index < len(lines) and not (failure and args.stop):
        cwd = ComputeDirectory(lines[local_index])
        failed_output_file_name = os.path.join(cwd, "CppTests_failed_output")
        if os.path.exists(failed_output_file_name):
            os.remove(failed_output_file_name)
        timing_violation_report_file_name = os.path.join(cwd, "HLS_output/Synthesis/timing_violation_report")
        if os.path.exists(timing_violation_report_file_name):
            os.remove(timing_violation_report_file_name)
        tool_return_value_file_name = os.path.join(cwd, "CppTests_return_value")
        if args.restart and os.path.exists(os.path.join(cwd, "CppTests_return_value")):
            tool_return_value_file = open(tool_return_value_file_name, "r")
            return_value = tool_return_value_file.read()
            tool_return_value_file.close()
            if return_value == "0":
                with lock:
                    total_benchmark += 1
                    passed_benchmark += 1
                    logging.info("   SKIPPING --- OVERALL: " + 
                                 str(passed_benchmark) + " passed, " + 
                                 str(total_benchmark-passed_benchmark) + 
                                 " failed, " + 
                                 str(len(lines)-total_benchmark) + 
                                 " queued --- " + 
                                 lines[local_index].replace("\\", ""))
                    local_index = line_index
                    line_index += 1
                continue
        HLS_output_directory = os.path.join(cwd, "HLS_output")
        if os.path.exists(HLS_output_directory):
            shutil.rmtree(HLS_output_directory)
        output_file_name = os.path.join(cwd, "CppTests_execution_output")
        output_file = open(output_file_name, "w")
        local_args = lines[local_index]
        if local_args[0] == "\"":
            local_args = local_args[1:-1]
        tokens = shlex.split(lines[local_index])
        args_without_benchmark_name = ""
        for token in tokens:
            if token.find("--benchmark-name") == -1:
                args_without_benchmark_name += token + " "
        local_args = args_without_benchmark_name
        #local_command = "ulimit " + args.ulimit + "; exec timeout " + \
        #                args.timeout + " " + tool_exe
        local_command = tool_exe
        local_command = local_command + " " + local_args
        output_file.write("#" * 80 + "\n")
        output_file.write("cd " + cwd + "; ")
        output_file.write(local_command + "\n")
        output_file.write("start time " + str(datetime.datetime.now())+ "\n")
        output_file.write("#" * 80 + "\n")
        output_file.flush()
        return_value = -1
        with lock_creation_destruction:
            if not (failure and args.stop):
                children[thread_index] = subprocess.Popen(local_command, 
                                                          stderr=output_file, 
                                                          stdout=output_file, 
                                                          cwd=cwd, shell=True, 
                                                          executable="/bin/bash")
        try:
            return_value = children[thread_index].wait()
        except:
            pass
        with lock_creation_destruction:
            if return_value != 0 and (args.stop or args.returnfail):
                failure = True
            if failure and args.stop:
                for local_thread_index in range(n_jobs):
                    if children[local_thread_index] != None:
                        if children[local_thread_index].poll() == None:
                            try:
                                kill_proc_tree(children[local_thread_index].pid)
                            except OSError:
                               pass
        output_file.write("\n" + "#" * 80 + "\n")
        output_file.write("finish time " + str(datetime.datetime.now())+ "\n")
        output_file.write("#" * 80 + "\n")
        os.fsync(output_file.fileno())
        output_file.close()
        tool_return_value_file = open(tool_return_value_file_name, "w")
        tool_return_value_file.write(str(return_value))
        tool_return_value_file.close()
        args_file = open(os.path.join(cwd, "args"), "w")
        args_file.write(lines[local_index])
        args_file.close()
        if not (failure and args.stop) or (return_value != -9 and return_value != 0):
            if return_value != 0:
                shutil.copy(output_file_name, str(os.path.join(os.path.dirname(output_file_name), "CppTests_failed_output")))
            if os.path.exists(timing_violation_report_file_name):
                annotated_timing_violation_report_file = open(os.path.join(cwd, "timing_violation_report"), "w")
                annotated_timing_violation_report_file.write("#" * 80 + "\n")
                annotated_timing_violation_report_file.write("cd " + cwd + "; ")
                annotated_timing_violation_report_file.write(local_command + "\n")
                annotated_timing_violation_report_file.write("#" * 80 + "\n")
                annotated_timing_violation_report_file.flush()
                timing_violation_report_file = open(timing_violation_report_file_name)
                annotated_timing_violation_report_file.write(timing_violation_report_file.read())
                timing_violation_report_file.close()
                annotated_timing_violation_report_file.close()
            with lock:
                total_benchmark += 1
                if return_value == 0:
                    passed_benchmark += 1
                    if not args.no_clean:
                        for sub in os.listdir(cwd):
                            if os.path.isdir(os.path.join(cwd, sub)):
                                shutil.rmtree(os.path.join(cwd, sub))
                            else:
                                if sub != "CppTests_return_value" and sub != "CppTests_execution_output" and sub != "args":
                                    os.remove(os.path.join(cwd, sub))
                    logging.info("   SUCCESS --- OVERALL: " + str(passed_benchmark) + " passed, " + str(total_benchmark-passed_benchmark) + " failed, " + str(len(lines)-total_benchmark) + " queued --- " + lines[local_index].replace("\\", ""))
                elif return_value == 124:
                    logging.info("   FAILURE (Timeout) --- OVERALL: " + str(passed_benchmark) + " passed, " + str(total_benchmark-passed_benchmark) + " failed, " + str(len(lines)-total_benchmark) + " queued --- " + lines[local_index].replace("\\", ""))
                elif return_value == 153:
                    logging.info("   FAILURE (File size limit exceeded) --- OVERALL: " + str(passed_benchmark) + " passed, " + str(total_benchmark-passed_benchmark) + " failed, " + str(len(lines)-total_benchmark) + " queued --- " + lines[local_index].replace("\\", ""))
                else:
                    logging.info("   FAILURE --- OVERALL: " + str(passed_benchmark) + " passed, " + str(total_benchmark-passed_benchmark) + " failed, " + str(len(lines)-total_benchmark) + " queued --- " + lines[local_index].replace("\\", ""))
            with lock:
                local_index = line_index
                line_index += 1


#Computing relative path
def ComputeDirectory(line):
    configuration_name = ""
    benchmark_name = ""
    tokens = shlex.split(line)
    for token in tokens:
        if token.find("--benchmark-name") != -1:
            benchmark_name = token[len("--benchmark-name="):]
    new_dir = os.path.join(abs_path, benchmark_name)
    return new_dir


line_index = 0
failure = False

# parse arguments
parser = argparse.ArgumentParser(description="Performs tests", fromfile_prefix_chars='@')
parser.add_argument('-l', "--benchmarks_list", help="The file containing the list of tests to be performed", nargs='*', action="append")
parser.add_argument('-o', "--output", help="The directory where output files we be put (default=\"build/output\")", default="build/output")
parser.add_argument('-j', help="The number of jobs which execute the benchmarks (default=\"1\")", default=1, type=positive_integer, action=StoreOrUpdateMin)

parser.add_argument("--exec", help="The executable (default=run_experiment.sh)", default="script/sh/run_experiment.sh")

parser.add_argument('-t', "--timeout", help="Timeout for tool execution (default=60m)", default="60m")
parser.add_argument('-a', "--args", help="A set of arguments to be passed to the tool", nargs='*', action='append')
parser.add_argument('-c', "--commonargs", help="A set of arguments to be passed to the tool", nargs='*', action='append')
parser.add_argument("--table", help="Print the results in tex format", default="results.tex")
parser.add_argument("--csv", help="Print the results in csv format")

parser.add_argument("--ulimit", help="The ulimit options", default="-f 8388608 -v 16777216 -s 16384")
parser.add_argument("--stop", help="Stop the execution on first error (default=false)", default=False, action="store_true")
parser.add_argument("--returnfail", help="Return FAILURE in case at least one test fails (default=false)", default=False, action="store_true")
parser.add_argument("--mail", help="Send a mail with the result")
parser.add_argument("--no-clean", help="Do not clean produced files", default=False, action="store_true")
parser.add_argument("--restart", help="Restart last execution (default=false)", default=False, action="store_true")
parser.add_argument("--script", help="Set the bash script in the generated tex", default="")

args = parser.parse_args()
n_jobs = args.j # set this, because it will be overwritten by the parse of modified_argv
logging.basicConfig(level=logging.INFO,format='%(levelname)s: %(message)s')

#The absolute path of current script
abs_script = os.path.abspath(sys.argv[0])
main_folder = abs_script.split('/')[:-3]
main_folder = "/".join(main_folder)

#The table to be produced
table = os.path.abspath(args.table)

#The csv to be produced
if args.csv != None:
    csv = os.path.abspath(args.csv)
else:
    csv = None

#Check if output directory exists, if yes abort
if os.path.exists(args.output) and not args.restart:
    logging.error("Output directory " + args.output + " already exists. Please remove it or specify a different one with -o")
    sys.exit(1)

#Create the folder and enter in it
abs_path = os.path.abspath(args.output)

if args.restart:
    if not os.path.exists(abs_path):
        args.restart = False
if not args.restart:
    os.mkdir(abs_path)
os.chdir(abs_path)

if args.exec.startswith("script/"):
    args.exec = main_folder + "/" + args.exec

#Skipping if all benchmarks already pass
if args.restart:
    failed_counter_file_name = os.path.join(abs_path, "failed_counter")
    if os.path.exists(failed_counter_file_name):
        failed_counter_file = open(failed_counter_file_name)
        failed_counter = failed_counter_file.read()
        if failed_counter == "0" and args.junitdir == "" and args.perfpublisherdir == "":
            logging.info("Already pass")
            sys.exit(0)

#Check tool executable
tool_exe = ""
if os.path.isfile(args.exec) and os.access(args.exec, os.X_OK):
    tool_exe = args.exec
elif os.path.isfile(os.path.join(os.path.dirname(abs_script),args.exec)) \
     and\
     os.access(os.path.join(os.path.dirname(abs_script),args.exec), os.X_OK):
    tool_exe = os.path.join(os.path.dirname(abs_script), args.exec)
else:
    #Check exec in the path
    for path in os.environ["PATH"].split(os.pathsep):
        exe_file = os.path.join(path, args.exec)
        if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
            tool_exe = exe_file
if tool_exe == "":
    if args.exec != "script/sh/run_experiment.sh":
        if not os.path.isfile(args.exec):
            logging.error(args.exec + " does not exist")
        else:
            logging.error(args.exec + " is not an executable")
    else:
        logging.error("run_experiment.sh not found")
    sys.exit(1)
logging.info("executable found: " + tool_exe)

#Check mutt executable
if args.mail != None:
    #Check mutt in the path
    for path in os.environ["PATH"].split(os.pathsep):
        exe_file = os.path.join(path, "mutt")
        if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
            mutt = exe_file
    if mutt == None:
        logging.error("mutt not found")
        sys.exit(1)

if not args.restart:
    #Check if file lists exist
    abs_lists = []
    if args.benchmarks_list != None:
        for relative_list in args.benchmarks_list:
            #First look in the current directory
            if os.path.exists(os.path.abspath(relative_list[0])):
                abs_lists.append(os.path.abspath(relative_list[0]))
            #Then look in script directory
            elif os.path.exists(os.path.join(os.path.dirname(abs_script), relative_list[0])):
                abs_lists.append(os.path.join(os.path.dirname(abs_script), relative_list[0]))
            else:
                logging.error(relative_list[0] + " does not exist")
                sys.exit(1)
    else:
        logging.error("Benchmarks not found")
        sys.exit(2)

    #Reordering elements in each row
    reordered_list_name = os.path.join(abs_path, "reordered_list")
    reordered_list = open(reordered_list_name, "w")
    
    logging.info("Preparing benchmark list")
    logging.info("   Reordering arguments")
    for abs_list in abs_lists:
        list_file = open(abs_list)
        lines = list_file.readlines()
        list_file.close()
        for line in lines:
            if line.strip() == "":
                continue
            if line[0] =='#':
                continue
            tokens = shlex.split(line)
            parameters = list()
            #Flag used to ad-hoc manage --param arg
            follow_param = False
            for token in tokens:
                if token[0] == '-':
                    parameters.append(re.escape(token))
                    if token.find("--param") != -1:
                        follow_param = True;
                    else:
                        follow_param = False;
                else:
                    if follow_param == True:
                        parameters.append(re.escape(token))
                    else:
                        reordered_list.write(token + " ")
                    follow_param = False;
            for parameter in parameters:
                reordered_list.write(re.escape(parameter) + " ")
            reordered_list.write("\n")
    reordered_list.close()

    #Adding parameters
    logging.info("   Considering all tool arguments")
    arg_lists = args.args
    if not arg_lists:
        arg_lists = [("")]
    arged_list_name = os.path.join(abs_path, "arged_list")
    arged_list = open(arged_list_name, "w")
    lines = open(reordered_list_name).readlines()
    for arg_list in arg_lists:
        for line in lines:
            arged_list.write(line.rstrip())
            if len(arg_list) > 0:
                arg = arg_list[0]
                if arg[0] == "\"":
                    arg = arg[1:-1]
                arged_list.write(" " + arg)
            if args.commonargs != None and len(args.commonargs) > 0:
                for commonarg in args.commonargs:
                    arged_list.write(" " + commonarg[0].replace("#", " "))
            arged_list.write("\n")
    arged_list.close()

    #Name of benchmarks
    benchmark_names = set()        
    
    #Adding benchmark name
    logging.info("   Adding benchmark name")
    named_list_name = os.path.join(abs_path, "named_list")
    named_list = open(named_list_name, "w")
    lines = open(arged_list_name).readlines()
    for line in lines:
        named_list.write(line.rstrip())
        #Retrieve benchmark name and seed
        benchmark_name = ""
        tokens = shlex.split(line)
        for token in tokens:
            if token.find("--benchmark-name") != -1:
                benchmark_name = token[len("--benchmark-name="):]
        if benchmark_name == "":
            list_of_params = line.split()
            for w in list_of_params:
                benchmark_name = benchmark_name + w + "-" 
            benchmark_name = benchmark_name[:-1]
            named_list.write(" --benchmark-name=" + benchmark_name)
        logging.info("     " + benchmark_name)
        if benchmark_name in benchmark_names:
            logging.error("Duplicated configuration name - benchmark name: " + full_name)
            sys.exit(1)
        benchmark_names.add(benchmark_name)
        named_list.write("\n")
    named_list.close()        
    
    #Generating folder
    logging.info("   Generating output directories")
    lines = open(named_list_name).readlines()
    for line in lines:
        new_dir = ComputeDirectory(line)
        logging.info("      Creating directory " + new_dir)
        os.makedirs(new_dir)
else:
    logging.info("   Skipping generation of lists and directories")
    named_list_name = os.path.join(abs_path, "named_list")
    if not os.path.exists(named_list_name):
        logging.error("List of previous run not found")
        sys.exit(1)

#Create threads
logging.info("   Launching tool")
lock = threading.RLock()
lock_creation_destruction = threading.RLock()
passed_benchmark = 0
total_benchmark = 0
threads = []
children = [None] * n_jobs
for thread_index in range(n_jobs):
    threads.insert(thread_index, threading.Thread(target=execute_tests, args=(named_list_name, thread_index)))
    threads[thread_index].daemon=True
    threads[thread_index].start()

try:
    #Wait threads
    for thread_index in range(n_jobs):
        while threads[thread_index].isAlive():
            threads[thread_index].join(100)
except KeyboardInterrupt:
    logging.error("SIGINT received")
    failure = True
    for local_thread_index in range(n_jobs):
        if children[local_thread_index] != None:
           if children[local_thread_index].poll() == None:
              try:
                 kill_proc_tree(children[local_thread_index].pid)
              except OSError:
                 pass
    sys.exit(1)

#Prepare final report
report_file_name = os.path.join(abs_path, "report")
#report_file = open(report_file_name)
#lines = report_file.readlines()
#report_file.close()
report_file = open(report_file_name, "w")
command = [tool_exe, "--version"]
subprocess.call(command, stderr=report_file, stdout=report_file)
report_file.write("SYSTEM INFORMATION:\n")
report_file.flush()
#command = ["lsb_release", "-a"]
#subprocess.call(command, stderr=report_file, stdout=report_file)
report_file.write("\n")
report_file.write("CURRENT TIME:\n")
report_file.write(str(datetime.datetime.now())+ "\n\n")
report_file.write("PASSED TESTS:\n")
report_file.write(str(passed_benchmark) + "/" + str(total_benchmark) + "\n\n")

failed_counter_file_name = os.path.join(abs_path, "failed_counter")
failed_counter_file = open(failed_counter_file_name, "w")
failed_counter_file.write(str(total_benchmark - passed_benchmark))
failed_counter_file.close()

for line in lines:
    report_file.write(line)
report_file.close()

if args.mail != None:
    outcome = ""
    if args.stop:
        if failure:
            outcome = "FAILURE"
        else:
            outcome = "SUCCESS"
    else:
        outcome = str(passed_benchmark) + "/" + str(total_benchmark)
    full_name = "Cpp_tests"
    local_command = "cat " + report_file_name + " | mutt -s \"" + full_name + ": " + outcome + "\" " + args.mail
    subprocess.call(local_command, shell=True)
if failure:
    sys.exit(1)
sys.exit(0)
