# This script is written by Minhyuk Kweon
# 2020.07.07
#
# mh.kweon@postech.ac.kr

import os
import signal
import sys
import shlex
import subprocess as sp
import argparse as ap
import multiprocessing as mp
from datetime import datetime
from time import time as timer, sleep

# TCAD 
RESULT_OP =(("4mod5-v1_22"      , 0         , 0     ),
            ("mod5mils_65"      , 0         , 0     ),
            ("alu-v0_27"        , 3         , 0     ),
            ("decod24-v2_43"    , 0         , 0     ),
            ("4gt13_92"         , 21        , 0     ),
            ("ising_model_10"   , 0         , 0     ),
            ("ising_model_13"   , 0         , 0     ),
            ("ising_model_16"   , 0         , 0     ),
            ("qft_10"           , 39        , 0.015 ),
            ("qft_13"           , 96        , 0.031 ),
            ("qft_16"           , 192       , 0.062 ),
            ("qft_20"           , 360       , 0.109 ),
            ("rd84_142"         , 108       , 0.046 ),
            ("adr4_197"         , 1224      , 0.218 ),
            ("radd_250"         , 1047      , 0.186 ),
            ("z4_268"           , 855       , 0.202 ),
            ("sym6_145"         , 1017      , 0.202 ),
            ("misex1_241"       , 1098      , 0.249 ),
            ("rd73_252"         , 2193      , 0.343 ),
            ("cycle10_2_110"    , 1968      , 0.348 ),
            ("square_root_7"    , 1788      , 0.406 ),
            ("sqn_258"          , 3057      , 0.563 ),
            ("rd84_253"         , 5697      , 0.892 ),
            ("co14_215"         , 5061      , 1.062 ),
            ("sym9_193"         , 13746     , 2.091 ),
            ("urf5_158"         , 58947     , 9.312 ),
            ("hwb9_119"         , 89355     , 12.909),
            ("urf4_187"         , 168366    , 38.42 )
        )

# define paths
dirpos = "../example"
binaryName = "./FSQM"
outpos = "../output"
revpos = "../reverse"
logpos = "../log"

#
curTime = datetime.now().strftime('%m_%d_%H_%M')
benchlist_dir = []
benchlist = []
benchDir = sorted(os.listdir(dirpos))

# result include tuple (baseline result, our result) with template ()
benchresult = []

### function define ###
def ExecuteCommand( curBench, result, alpha, beta, gamma, time_limit):
    inputPath = "%s/%s" % (dirpos, curBench)
    outputPath = "%s/%s" % (outpos, curBench)
    reversePath = "%s/%s" % (revpos, curBench)
    logPath = "%s/%s.%s.log" % (logpos, curBench, curTime)
    exeStr = "%s %s %s %s %s %s %s | tee %s" % (binaryName, inputPath, outputPath, reversePath, alpha, beta, gamma, logPath)
    print( exeStr )
    run = RunCommand(exeStr, time_limit)
    if(run!=None):
        ourResult = ParseLog(logPath)
        TCADResult = FindTCAD(curBench[:-5])
        result.append( [(curBench[:-5], alpha, beta, gamma), TCADResult, ourResult] )
    else:
        print('time out')
    
def RunCommand(exeStr, time_limit):
    try:
        proc = sp.Popen(exeStr, shell=True)
        return proc.communicate(timeout = time_limit)
    except sp.TimeoutExpired:
        print('Time out!')
        pid = proc.pid
        proc.terminate()
        proc.kill()
        return None

def FindTCAD( name ):
    swap, time = 0, 0
    for rslt in RESULT_OP:
        curBench = rslt[0]
        if curBench == name:
            swap = rslt[1]
            time = rslt[2]
    return (swap, time)

def ParseLog( logPath ):
    swap, time = 0, 0
    f = open(logPath)
    for line in f:
        if line[0:16] == "# add_cnot_num: " :
            swap = int(line[16:])
        if line[0:11] == "END Program":
            time = line.rstrip('\n').split(' ')[-1]
    f.close()
    cnot_num = [ swap, time ]
    return cnot_num

def getdirList( ):
    for curBench in benchDir:
        if curBench[-5:] == ".qasm":
            benchlist_dir.append(curBench)

getdirList()
if len(sys.argv) <= 1 or sys.argv[1] == "help":
    print("usage: python run.py <arg>")
    print("")
    print("list of <arg>")
    print("     help        : print how to use run.py")
    print("     list        : print file number of bench directory")
    print("     all         : execute all bench file in directory")
    print("     #           : execute one bench file(or more) in directory")
    print("                   to look file number do list")
    print("     <benchname> : execute one bench file of benchname")
    sys.exit(1)
elif sys.argv[1] == "list":
    for index, curBench in enumerate(benchlist_dir):
        print( str(index) + " : " + curBench )
    sys.exit(1)
elif sys.argv[1].isdigit() and len(sys.argv) == 2:
    benchNum = int(sys.argv[1])
    try:
        benchlist_dir[benchNum]
    except:
        print("ERROR: NO EXISTING BENCH FILE")
        print("Please do python run.py list")
        sys.exit(1)
    ExecuteCommand( benchlist_dir[benchNum], benchresult, 0.8, 9, 3, 50)
        
elif sys.argv[1] == "all" or len(sys.argv) >= 3:
    if len(sys.argv) >= 3:
        for argnum in range(1, len(sys.argv)):
            num = int(sys.argv[argnum])
            try:
                benchlist_dir[num]
            except:
                print("ERROR: NO EXISTING BENCH FILE")
                print("Please do python run.py list")
                sys.exit(1)
            benchlist.append(benchlist_dir[num])
    elif sys.argv[1] == "all":
        benchlist = benchlist_dir

    if __name__ == '__main__':
        procs = []
        manager = mp.Manager()
        results = manager.list()
        for index, curBench in enumerate(benchlist):
            proc = mp.Process(target=ExecuteCommand,
                    args=(curBench, results, 0.8, 9, 3, 50))
            procs.append(proc)
            proc.start()
        for proc in procs:
            proc.join()
    benchresult = results[:]

elif sys.argv[1] == "initial" or len(sys.argv) >= 3:
    if len(sys.argv) >= 3:
        for argnum in range(1, len(sys.argv)):
            num = int(sys.argv[argnum])
            try:
                benchlist_dir[num]
            except:
                print("ERROR: NO EXISTING BENCH FILE")
                print("Please do python run.py list")
                sys.exit(1)
            benchlist.append(benchlist_dir[num])
    elif sys.argv[1] == "all":
        benchlist = benchlist_dir

    if __name__ == '__main__':
        procs = []
        manager = mp.Manager()
        results = manager.list()
        for index, curBench in enumerate(benchlist_initial):
            proc = mp.Process(target=ExecuteCommand,
                    args=(curBench, results, 0.8, 9, 3, 50))
            procs.append(proc)
            proc.start()
        for proc in procs:
            proc.join()
    benchresult = results[:]

else:
    print("ERROR: NO ACCEPTABLE ARGUMENT")
    sys.exit(1)

benchresult = sorted(benchresult, key=lambda row: row[0][0])
avg_set = []
print(" ")
print(" ")
print("----------------------------- Result ---------------------------------------")
print("                    |       TCAD        |                 Ours              ")
print("  bench_name        |  #CNOT  | RunTime |  #CNOT  | RunTime | improve")
print("----------------------------------------------------------------------------")
for benchResult in benchresult:
    s1 = '%-19s' % benchResult[0][0]
    s_beta  = '%4s' % benchResult[0][2]
    s2 = '%7s' % benchResult[1][0]
    s3 = '%7s' % benchResult[1][1]
    s4 = '%7s' % benchResult[2][0]  #CNOT
    s5 = '%7s' % benchResult[2][1]  #RunTime

    try:
        avg = round(int(s4) / int(s2), 6)
        avg_set.append(avg)
        print( s1, "|",
                s2, "|", s3, "|", 
                s4, "|", s5, "|",
                avg)
    except ZeroDivisionError as e:
        print( s1, "|",
                s2, "|", s3, "|",
                s4, "|", s5, "|",
                e)
avg_improve = round(sum(avg_set)/len(avg_set), 6)
print("----------------------------------------------------------------------------")
print("==> avg_improve: ", avg_improve)
