#!/usr/bin/env python

##
# Copyright (c) 2005-2007 Apple Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# DRI: Cyrus Daboo, cdaboo@apple.com
##

import os
import sys

#
# Create a new set of patch files for dependent projects.
#

projects = (("../../../Libraries/open-powerplant", "open-powerplant"), 
            ("../../../Libraries/MoreFiles", "MoreFiles"),
            ("../../../Libraries/MoreFilesX", "MoreFilesX"), )
cwd = os.getcwd()
libpatches = cwd

svn = "/usr/local/bin/svn"

def makepatches(project):
    
    # First delete existing patch files.
    path = os.path.join(libpatches, project[1])
    print "Removing patches from directory: %s" % (path[len(cwd) + 1:],)
    for file in os.listdir(path):
        fpath = os.path.join(path, file)
        if os.path.isfile(fpath):
            os.remove(fpath)
    
    # Now execute svn status to find files needing to be patched.
    os.system("%s status \"%s\" > temppatch" % (svn, project[0]))
    
    f = open("temppatch")
    for line in f:
        
        # We are only interested in new files and modified files.
        if line and line[0] == "?":
            new = True
        elif line and line[0] == "M":
            new = False
        else:
            continue
        
        # Ignore file names starting with ".".
        line = line[1:]
        line = line.strip()
        if os.path.basename(line)[0] == ".":
            continue
        
        # Ignore modified directories - we will patch the files in them
        if not new and os.path.isdir(line):
            continue
        
        # Generate the name of the patch file we want to create.
        patch = line
        patch = patch[patch.rfind(os.sep) + 1:]
        patch = patch.replace(os.sep, ".") + ".patch"
        
        patch = os.path.join(path, patch)

        print ("Creating diff file %s for existing file %s", "Creating diff file %s for new file %s")[new] % (patch[len(cwd) + 1:], line)
        
        # Generate an svn diff.
        # NB For new files we have to first add them then do the diff, then revert,
        # otherwise svn diff complains about an unversioned resource.
        if new:
            os.system("%s add -q \"%s\"" % (svn, line))
        os.system("%s diff \"%s\" > tempdiff" % (svn, line))
        if new:
            os.system("%s revert -q \"%s\"" % (svn, line))
        
        # Now replace the directory prefix in the diff output and write to the actual patch file.
        repl = "../%s/" % (project[0],)
        fin = open("tempdiff")
        fout = open(patch, 'w')
        for line2 in fin:
            fout.write(line2.replace(repl, ""))
        fout.close()
        fin.close()
        os.remove("tempdiff")
        
    f.close()
    os.remove("temppatch")
    
if __name__ == "__main__":

    try:
        for project in projects:
            makepatches(project)
    except Exception, e:
        sys.exit(str(e))
