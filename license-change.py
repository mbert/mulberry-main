#!/usr/bin/python

import getopt
import os
import sys

newlicense = """/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

"""

def process_directory(path, delete_unlicensed, undo):
    for file in os.listdir(path):
        fpath = os.path.join(path, file)
        if os.path.isfile(fpath):
            if undo:
                undo_file(fpath)
            else:
                process_file(fpath, delete_unlicensed)
        elif os.path.isdir(fpath):
            process_directory(fpath, delete_unlicensed, undo)

def undo_file(fpath):
    if (fpath.endswith(".cp.unlicensed") or
        fpath.endswith(".cc.unlicensed") or
        fpath.endswith(".cpp.unlicensed") or
        fpath.endswith(".c.unlicensed") or
        fpath.endswith(".h.unlicensed")):
        fpath = fpath[:-11]
        print fpath
        os.rename(fpath, "%s.licensed" % fpath)
        os.rename("%s.unlicensed" % fpath, fpath)
        os.remove("%s.licensed" % fpath)

def process_file(fpath, delete_unlicensed):
    if delete_unlicensed and \
       (fpath.endswith(".cp.unlicensed") or
        fpath.endswith(".cc.unlicensed") or
        fpath.endswith(".cpp.unlicensed") or
        fpath.endswith(".c.unlicensed") or
        fpath.endswith(".h.unlicensed")):
        os.remove(fpath)
    elif not delete_unlicensed and \
         (fpath.endswith(".cp") or
          fpath.endswith(".cc") or
          fpath.endswith(".cpp") or
          fpath.endswith(".c") or
          fpath.endswith(".h")):
        replace_license(fpath)

def replace_license(fpath):
    print fpath
    fin = open(fpath, "rU")
    fout = open("%s.licensed" % fpath, "w")

    first = True
    ignore = False
    reallyignore = False
    ignored = []
    for line in fin:
        if first:
            fout.write(newlicense)
            if line.startswith("/*"):
                ignore = True
        first = False
        if not ignore:
            fout.write(line)
        else:
            ignored.append(line)
        if ignore:
            if line.find("Copyright") != -1:
                reallyignore = True
            if line.find("*/") != -1:
                ignore = False
                if not reallyignore:
                    fout.write("".join(ignored))
    
    fin.close()
    fout.close()
    os.rename(fpath, "%s.unlicensed" % fpath)
    os.rename("%s.licensed" % fpath, fpath)

if __name__ == "__main__":

    delete_unlicensed = False
    undo = False

    options, args = getopt.getopt(sys.argv[1:], "ru")

    for option, value in options:
        if option == "-r":
            delete_unlicensed = True
        elif option == "-r":
            undo = True

    paths = args

    for path in paths:
        process_directory(path, delete_unlicensed, undo)
            
