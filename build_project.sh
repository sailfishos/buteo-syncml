#!/usr/bin/env sh
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
# todo add proper exit codes
# This script is for executing (re)building the library from scratch 
# and running unit tests
# To create the debian packages, use "dpkg-buildpackage"
export PATH=/home/bamboo2/qt/qtsdk-2009.02/qt/bin:$PATH
if [ -f Makefile ]; then
   make distclean
fi
rm -rf tests_meego/results  
qmake libmeegosyncml.pro
# For single core envs, maybe remove -j2
make -j3
cd tests_meego
export LD_LIBRARY_PATH=$PWD/../src/
if [ -d results ]; then 
    rm -f results/*
else 
    mkdir results
fi
./libmeegosyncml-tests -xml > results/tests.xml
cd results
# todo catch if the parsing fails
# csplit -f test- tests.xml "/<\?xml version=\"1.0\" encoding=\"ISO-8859-1\"\?>/" '{*}'
awk '/^<\?xml/{n++}{print > n "-split.xml" }' tests.xml
# csplit -f test- tests.xml "/^<?xml version=\"1.0\"/" '{*}'
#rm test-00
for i in *split*; do
    xsltproc ../../Scripts/tests2junit.xsl $i > $i-junit.xml
done
