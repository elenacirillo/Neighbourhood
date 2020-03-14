#!/usr/bin/env python3

import sys
import csv
import random
from random import seed
import numpy as np
import os
import random


def createFolder(directory):
    try:
        if not os.path.exists(directory):
            os.makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' +  directory)


def fun_generating_node(nN):
    ID=1
    LofNodes=[]
    for i in range(nN):
        LofNodes.append("n"+str(ID))
        ID=ID+1
    return LofNodes


def fun_Loftime2(Lofdata,Lofcost):
    result = []
    result_forCPP = []
    
    # L_VM = ['VMtype'] (with unique entries)
    L_VM = []
    for i in Lofcost:
        L_VM.append(i[0])
    L_VM = list(dict.fromkeys(L_VM))    

    # L_VM_GPU = ['VMtype', 'GpuType']
    L_VM_GPU = []
    for i in L_VM:
        for j in Lofcost:
            if i==j[0]:
                L_VM_GPU.append([i,j[1]])
    
    # L_VM_GPU_n = ['VMtype', 'GpuType', 'max # GPUs of that type']
    L_VM_GPU_n=[]
    for i in L_VM_GPU:
        Max = 0
        for j in Lofcost:
            if (j[0] == i[0] and j[1] == i[1] and int(j[2])>Max):
                Max = int(j[2])
        L_VM_GPU_n.append([i[0],i[1],Max])
    
    # result = ['Jobs', 'VMtype', 'GpuNumber', 'ExecutionTime']
    #
    # result_forCPP = ['Jobs', 'VMtype', 'GpuType', 'GpuNumber', 
    #                  'max # GPUs of that type', 'ExecutionTime']
    for k in L_VM_GPU_n:
        n_GPU = 1
        while(n_GPU <= k[2]):
            for z in Lofdata:
                if (z[5]==k[1]  and int(z[6])==n_GPU):
                    result.append([z[4],k[0],z[6],z[7]])
                    result_forCPP.append([z[4],k[0],k[1],z[6],k[2],z[7]])
            n_GPU = n_GPU + 1
    
    return (result, result_forCPP)


def fun_generate_Lof_jobs(Lofdata):
    b = []
    b_forCPP = []
    
    # b = ['Application','Images','Epochs','Batchsize','Jobs']
    #
    # b_forCPP = ['Application','Images','Epochs','Batchsize','Jobs','min',
    #             'max']
    for x in Lofdata:
        flag = 0
        for j in b:
            if x[4] in j:
                flag = 1
                break
        if flag==0:
            b.append(x[0:5])
            b_forCPP.append([*x[0:5],x[8],x[9]])
    
    return (b, b_forCPP)


def fun_generate_deadline(job,SubmissionTime):
    min_time = float(job[5])
    max_time = float(job[6])
    return (SubmissionTime + random.uniform(min_time, 2 * max_time))


def fun_generating_tardinessweight():
    return (2*random.uniform(0.0015,0.0075))


def fun_LofVMs(Lofcost,Lofdata):
    b=[]
    for x in Lofcost:
        flag=0
        for y in Lofdata:
            if x[1]==y[5]:
                flag=1
        if flag==1:
            b.append(x[0])
    
    return (b)


def fun_nGPU_VM(VM_type,Lofcost):
    n_of_GPU=0
    
    for i in Lofcost:
        if i[0]==VM_type:
            n_of_GPU=i[2]

    return (n_of_GPU)


def fun_generate_cost(VM_type,Lofcost):
    cost=0
    for i in Lofcost:
        if i[0]==VM_type:
            cost=i[3]
    return(float(cost))
        
def generate_data(nInitialJ, nN, nJ, lambdaa, mu, myseed):

    ## seed for controling the selected jobs 
    seed(myseed)
    
    # generate folder to store new data
    folder_name = '../../build/data/from_loading_data/' + str(nInitialJ) + '-' + str(nN) +\
                  '-' + str(nJ) + '-' + str(lambdaa) + '-' + str(mu) + '-' +\
                  str(myseed)
    createFolder(folder_name)
    
    address = folder_name+'/'

    # generate submission times for the required nInitialJ + nJ jobs
    s = np.random.poisson(lambdaa/nN, nJ)
    AttendTimeTemp = [0]
    for i in range(nInitialJ-1):
        AttendTimeTemp.append(0)
    for i in range(len(s)):
        nn = len(AttendTimeTemp)
        AttendTimeTemp.append(AttendTimeTemp[nn-1]+s[i])
    nJ = nJ+nInitialJ

    # load data from data.csv
    #
    # Lofdata = ['Application', 'Images', 'Epochs', 'Batchsize', 'Jobs', 
    #            'GpuType', 'GpuNumber', 'ExecutionTime', 'min', 'max']
    Lofdata = []
    with open('../../build/data/data.csv', 'r') as f:
        reader = csv.reader(f)
        Lofdata = list(reader)
    Lofdata_colnames = Lofdata.pop(0)
    
    # load costs information from GPU-cost.csv
    #
    # Lofcost = ['VMType', 'GpuType', 'GpuNumber', 'cost']
    Lofcost = []
    with open('../../build/data/GPU-cost.csv', 'r') as f:
        reader = csv.reader(f)
        Lofcost = list(reader)
    Lofcost_colnames = Lofcost.pop(0)
    
    ## making time file from data file and loading it as data['time']
    #
    # Loftime = ['Jobs', 'VMtype', 'GpuNumber', 'ExecutionTime']
    #
    # Loftime_forCPP = ['Jobs', 'VMtype', 'GpuType', 'GpuNumber', 
    #                   'Max_GpuNumber', 'ExecutionTime']
    (Loftime, Loftime_forCPP) = fun_Loftime2(Lofdata, Lofcost)
    
    # generate nN nodes and save them in tNodes.csv file
    with open(address+'tNodes.csv','w') as resultFyle:
        resultFyle.write('Nodes' + '\n')
        for r in fun_generating_node(nN):
            resultFyle.write(r + '\n')
    resultFyle.close()

    # Lof_jobs = ['Application', 'Images', 'Epochs', 'Batchsize', 'Jobs']
    #
    # Lof_jobs_forCPP = ['Application', 'Images', 'Epochs', 'Batchsize', 
    #                    'Jobs', 'MinExecTime', 'MaxExecTime']]
    # (with one single entry for every job)
    (Lof_jobs, Lof_jobs_forCPP) = fun_generate_Lof_jobs(Lofdata)
    
    # select nJ jobs from Lof_jobs attaching them an unique ID and the 
    # relative submission time read from AttendTimeTemp
    #
    # Lof_selectjobs_forCPP = ['Application', 'Images', 'Epochs', 'Batchsize', 
    #                          'Jobs', 'UniqueJobsID', 'SubmissionTime', 
    #                          'Deadline', 'Tardinessweight', 'MinExecTime', 
    #                          'MaxExecTime']
    #
    # L_deadline = ['UniqueJobsID', 'deadline']
    #
    # L_tardinessweight = ['UniqueJobsID', 'tardinessweight']
    #
    Lof_selectjobs_forCPP = []
    ID = 1
    if len(Lof_jobs) >1:
        len_Lof_jobs = len(Lof_jobs) - 1
        for i in range(nJ):
            idx = random.randint(1,len_Lof_jobs)
            r = Lof_jobs[idx]
            rCPP = Lof_jobs_forCPP[idx]
            jid = 'JJ' + str(ID)
            deadline = fun_generate_deadline(rCPP,AttendTimeTemp[i])
            tw = fun_generating_tardinessweight()
            Lof_selectjobs_forCPP.append([*rCPP[0:5],jid,AttendTimeTemp[i],
                                         deadline,tw,rCPP[5],rCPP[6]])
            ID = ID + 1
    else:
        len_Lof_jobs = 1
        for i in range(nJ):
            r = Lof_jobs[0]
            rCPP = Lof_jobs_forCPP[0]
            jid = 'JJ' + str(ID)
            deadline = fun_generate_deadline(rCPP,AttendTimeTemp[i])
            tw = fun_generating_tardinessweight()
            Lof_selectjobs_forCPP.append([*rCPP[0:5],jid,AttendTimeTemp[i],
                                         deadline,tw,rCPP[5],rCPP[6]])
            ID = ID + 1
    
    # write list of selected jobs on file
    with open(address+'Lof_selectjobs_forCPP.csv', 'w') as outputdeadline:
        writer = csv.writer(outputdeadline, lineterminator='\n')
        writer.writerows([['Application', 'Images', 'Epochs', 'Batchsize', 
                           'Jobs', 'UniqueJobsID', 'SubmissionTime', 
                           'Deadline', 'Tardinessweight', 'MinExecTime', 
                           'MaxExecTime']])
        writer.writerows(Lof_selectjobs_forCPP)
    outputdeadline.close()

    # SelectJobs_times = ['UniqueJobsID', 'VMtype', 'GpuType', 'GpuNumber', 
    #                     'cost','ExecutionTime']
    SelectJobs_times = []
    for i in range(nJ):
        for j in Loftime_forCPP:
            if j[0] == Lof_selectjobs_forCPP[i][4]:
                for k in Lofcost:
                    if j[1] == k[0]:
                        SelectJobs_times.append([Lof_selectjobs_forCPP[i][5],
                                                 *j[1:-1],
                                                 k[3],j[-1]])
    #
    with open(address+'SelectJobs_times.csv', 'w') as outputdeadline:
        writer = csv.writer(outputdeadline, lineterminator='\n')
        writer.writerows([['UniqueJobsID', 'VMtype', 'GpuType', 'GpuNumber', 
                           'maxGpuNumber', 'cost','ExecutionTime']])
        writer.writerows(SelectJobs_times)
    outputdeadline.close()

    
if __name__ == '__main__':
    nInitialJ = int(sys.argv[1])
    nN        = int(sys.argv[2])
    nJ        = int(sys.argv[3])
    lambdaa   = int(sys.argv[4])
    mu        = int(sys.argv[5])
    myseed    = int (sys.argv[6])
    if (nInitialJ == 0 or nN == 0 or mu == 0 or lambdaa == 0):
        print('Sorry!! the inputs are not valuable. Data generation failed.')
    else:
        generate_data(nInitialJ, nN, nJ, lambdaa, mu, myseed)
        
