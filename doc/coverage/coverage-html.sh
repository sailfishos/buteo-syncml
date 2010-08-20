#!/bin/sh
#/*
#* This file is part of buteo-syncml package
#*
#* Copyright (C) 2010 Nokia Corporation. All rights reserved.
#*
#* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
#*
#* Redistribution and use in source and binary forms, with or without 
#* modification, are permitted provided that the following conditions are met:
#*
#* Redistributions of source code must retain the above copyright notice, 
#* this list of conditions and the following disclaimer.
#* Redistributions in binary form must reproduce the above copyright notice, 
#* this list of conditions and the following disclaimer in the documentation 
#* and/or other materials provided with the distribution.
#* Neither the name of Nokia Corporation nor the names of its contributors may 
#* be used to endorse or promote products derived from this software without 
#* specific prior written permission.
#*
#* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
#* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
#* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
#* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
#* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
#* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
#* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
#* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
#* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
#* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
#* THE POSSIBILITY OF SUCH DAMAGE.
#* 
#*/

#alias echo='echo -e'\E[31;40m
#check for the Makefile 
#COVERAGE_LOG=/var/log/libmeegosyncml_coverage.log
#if [ -f $COVERAGE_LOG ]
#then 
#rm -f $COVERAGE_LOG
#fi
WD=$PWD
STACKDIR=../../
cd $STACKDIR
if [ -f Makefile ];
then 
echo "Makefile Exists.. Removing it and creating one"
find . -name Makefile | xargs rm -f 
qmake
else
echo "Makefile does not exist .. Creating one... "
qmake
fi
#run the coverage script 
echo "Running the coverage script .. "
sh $WD/coverage.sh 
if [ $? -ne 0 ];
then
echo "Coverage Script Failed"
echo "Exiting the script"
exit 1
else 
echo "Coverage Script ran successfully "
fi
echo "Running lcov"
lcov -d . -c -o doc/coverage/coverage.info 
if [ $? -ne 0 ];
then 
echo "Script Failed while using Lcov"
exit 1
else 
echo "Lcov successfully Ran"
fi
cd doc/coverage
echo "Generating Documentation"
genhtml -o cov coverage.info 
if [ $? -ne 0 ];
then 
echo "Documentation Generation Failed"
exit 1
else 
echo "Documentation Generation Successful"
fi
