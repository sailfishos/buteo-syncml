#!/bin/bash
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
#Author - Srikanth Kavoori
# This file should run unittests for libmaemosyncml
# and create the result file with unittest rate
# and coverage to this folder with name
# libmaemosyncml-results
#
# The release number should be in the file
# this script generates the results automatically 
# for the latest weekXxX directory under libmaemosyncml
#this script updation should be rarely needed 

#Script Specific Variables 

TARGET=libmaemosyncml
WD=$PWD
if [ $# == 0 ];
then
echo "Enviroment is set to trunk "
ROOTDIR=$WD/../..
TESTS_DIR=$ROOTDIR/Tests
STACK_DIR=$ROOTDIR/maemosyncml
RESULTS_FILE=$WD/$TARGET-results
else 
echo "Enviroment is set to tags"
WD=$1
TARGET_WEEK=$(ls -c $WD/../$TARGET | head -1)
TESTS_DIR=$WD/../$TARGET/$TARGET_WEEK/Tests
STACK_DIR=$WD/../$TARGET/$TARGET_WEEK/maemosyncml
RESULTS_DIR=$2
RESULTS_FILE=$RESULTS_DIR/$TARGET-results_$TARGET_WEEK
export SBOX_USE_CCACHE=no
ccache -c
fi
export LD_LIBRARY_PATH=$STACK_DIR:$LD_LIBRARY_PATH
TEMPFILE1=$WD/.temp_results
if [ -f $TEMPFILE1 ]
then
	rm -f $TEMPFILE1
fi

TEMPFILE2=$WD/.gcov_info.txt
if [ -f $TEMPFILE2 ]
then
       rm -f $TEMPFILE2
fi

TEMPFILE3=$WD/.percent
if [ -f $TEMPFILE3 ]
then
      rm -f $TEMPFILE3
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STACK_DIR

echo "Running the unit tests for $TARGET..."
echo "Results will be stored in: $RESULTS_FILE ...."
#Build the library with coverage 
echo "Building Stack in $STACK_DIR"
cd $STACK_DIR
qmake
make clean
make coverage 
echo "tests directory is $TESTS_DIR"
cd $TESTS_DIR
qmake
make clean
make 
echo "Running the tests in $PWD"
./libmaemosyncml-tests > $TEMPFILE1
echo "Getting the Coverage Results"
cd $STACK_DIR
echo $PWD
rm -f moc_*
file_list=$(ls *.gcno)
#echo "Printing File List :$file_list "
for file in $file_list
do
   echo "Running gcov on $file"
   gcov $file >> $TEMPFILE2
done
# get coverage information for the files using perl
echo "executing perl gcov_info.pl $TEMPFILE2 $TEMPFILE3"
GCOV_REPORT=$WD/gcov_report.txt
perl $WD/gcov_info.pl $TEMPFILE2 $TEMPFILE3 $GCOV_REPORT

if [ ! $? -eq 0 ]; then
#	echo "Perl Script for gcov information exit normal"
#else 
 echo "Perl Script for gcov information Failed to execute ... Exiting ...  "
 exit 0
fi

SUMMARY_FILE=$WD/.summary_file
if [ -f $SUMMARY_FILE ]
then
      rm -f $SUMMARY_FILE
fi
perl $WD/test_info.pl $TEMPFILE1 $SUMMARY_FILE

if [ ! $? -eq 0 ]; then
#echo "Perl Script for test information exit normal"
#else
echo "Perl Script for test information Failed to execute ... Exiting ...  "
exit 0
fi


echo "Writing the file $RESULTS_FILE"
echo "#Results for $TARGET_WEEK  " > $RESULTS_FILE   

echo "Results Summary STARTED " >> $RESULTS_FILE
                                                                                                
echo "#Current gcov reported coverage (line rate) is" >> $RESULTS_FILE
cat $TEMPFILE3 >> $RESULTS_FILE


echo "Unit test Results Summary " >> $RESULTS_FILE
cat $SUMMARY_FILE >> $RESULTS_FILE 

echo "Results Summary ENDED " >> $RESULTS_FILE


echo "(all unit tests are not refactored yet)" >> $RESULTS_FILE 
echo "#Coverage report is visible in $WD/gcov_report.txt" >> $RESULTS_FILE
echo "****************UNIT_TEST Results **************"  >> $RESULTS_FILE
cat $TEMPFILE1 >> $RESULTS_FILE
rm -f $TEMPFILE1 $TEMPFILE2 $TEMPFILE3 $SUMMARY_FILE
cd $STACK_DIR
rm -f *.gcno *.gcda *.gcov
cd $WD
echo "$RESULTS_FILE created"
