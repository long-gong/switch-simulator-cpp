#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from subprocess import call
from os.path import abspath, join, exists
from os import chdir, getcwd, listdir, mkdir
from send2trash import send2trash
from shutil import move, copy
from argparse import ArgumentParser
import sys

CWD = getcwd()
app_dir = ['common',
    'iLQF',
    'iSLIP',
    'iSLIP_ShakeUp',
    'MWM',
    'O1',
    'QPP_QPA_iSLIP',
    'QPP_QPA_Serena',
    'QPS_iSLIP',
    'QPS_Serena',
    'Serena']



parser = ArgumentParser(description='QPS build CLI')
parser.add_argument('-c', action='store_true', dest='clean', default=False,
                    help='Clean (remove all *.o and executable files)')
parser.add_argument('-a', action='store_true', dest='all', default=False,
                    help='Clean, build, and move')
parser.add_argument('-m', action='store_true', dest='move', default=False,
                    help='Move executable files to targets= directory')
parser.add_argument('-b', action='store_true', dest='build', default=False,
                    help='Build')
parser.add_argument('-t', action='store', default="targets",
                    dest='target',
                    help='Set a target directory',type=str)


parser.add_argument('-v', action='version', version='%(prog)s 1.0')

def build(app_dir):
    for app in app_dir:
        print "building ", app
        full_path = abspath(app)
        print "cd to ", full_path
        chdir(full_path)
        print "call make"
        call(["make"])
        print "building finished"
        chdir(CWD)

def clean(app_dir):
    for app in app_dir:
        print "cleaning ", app
        full_path = abspath(app)
        print "cd to ", full_path
        chdir(full_path)
        print "clean"
        for f in listdir(full_path):
            if f.endswith('.o') or f.endswith('_sim') or f.endswith('.txt'):
                print "moving ", f, "to trash"
                send2trash(join(full_path, f))
                continue
        chdir(CWD)


def gather(app_dir, target_dir="targets"):
    target_dir = abspath(target_dir)
    bin_dir = join(target_dir, 'bin')
    results_dir = join(target_dir, 'results')
    for d in [target_dir, bin_dir, results_dir]:
        if exists(d):
            if len(listdir(d)) != 0:
                print "directory ", d, " already exists and not empty!\nPlease change to another one"
                exit(1)
        else:
            mkdir(d)
    for app in app_dir:
        flag = False
        full_path = abspath(app)
        for f in listdir(full_path):
            if f.endswith('_sim'):
                print "moving ", f
                move(join(full_path, f), bin_dir)
                flag = True 
                break
        if not flag:
            print "No executable file is found in ", full_path

    my_py_dir = abspath('./py')
    for f in listdir(my_py_dir):
        print "Copying ", f
        if f.endswith('.py') or f.endswith('.yaml'):
            copy(join(my_py_dir, f), bin_dir)

def all(app_dir, target_dir=None):
    clean(app_dir)

    build(app_dir)
    if not (target_dir is None):
        gather(app_dir, target_dir)
    else:
        gather(app_dir[1::])      

if __name__ == '__main__':
    
    print sys.argv[1::]
    args = parser.parse_args(sys.argv[1::])

    print "all: ", args.all 
    print "clean: ", args.clean 
    print "build: ", args.build 
    print "move: ", args.move 
    print "target: ", args.target

    if args.all:
        print args.target
        all(app_dir, target_dir=args.target)

    if args.build:
        build(app_dir)

    if args.move:
        gather(app_dir, target_dir=args.target)

    if args.clean:
        clean(app_dir)









