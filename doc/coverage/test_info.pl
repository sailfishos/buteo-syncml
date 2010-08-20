#!/usr/bin/perl 
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
# Author : Srikanth Kavoori 
# Not Tested fully yet. There could be some issues.
# If you see any issues Let me know and i shall fix it :)
open(INPUTFILE, "<$ARGV[0]");
#	Totals: 0 passed, 2 failed, 0 skipped
$tests_string = "Totals:";
$pass_count = 0;
$fail_count = 0;
$skip_count = 0;
$total_count = 0;

while(<INPUTFILE>) {
my($line) = $_;
if(/$tests_string/) {
 
  #print $line;
  #print "Found usr_include or header file on line $. \n" ;
    $pass_start_index = rindex($line,':') + 1;
    $passed_index = rindex($line,"passed") - 1;
    $length = $passed_index - $start_index;
    $pass_count += substr($line,$pass_start_index,$length);

    #print  "pass_count $pass_count ";
    $fail_start_index = rindex($line,'passed,') + 8;
    $failed_index = rindex($line,"failed") - 1;
    $length = $failed_index - $fail_start_index;
    $fail_count += substr($line,$fail_start_index,$length);
    #print  "fail_count $fail_count ";

    $skip_start_index = rindex($line,'failed,') + 8;
    $skipped_index = rindex($line,"skipped") - 1;
    $length = $failed_index - $skip_start_index;
    $skip_count += substr($line,$skip_start_index,$length);
    #print  "skip_count $skip_count \n";
} 
}

#if($filecount != 0 ) {
#    $coverage_percent=sprintf("%.2f",$coverage_percent/$filecount);
#}
$total_count += $pass_count +  $fail_count + $skip_count;
print "Total No. TESTS: $total_count \n";
print "Passed: $pass_count \n";
print "Failed: $fail_count \n";
print "Skipped: $skip_count \n";

open(SUMMARY_FILE,">$ARGV[1]");
print SUMMARY_FILE "" ;
print SUMMARY_FILE "Total TESTS: $total_count \n";
print SUMMARY_FILE "Passed: $pass_count \n";
print SUMMARY_FILE "Failed: $fail_count \n";
print SUMMARY_FILE "Skipped: $skip_count \n";
close(SUMMARY_FILE);
close(INPUTFILE);
