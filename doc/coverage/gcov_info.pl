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
open(EXTRACTED_INFO,">$ARGV[2]");

$exclude_string = "/usr/include/";
$exclude_string2 = ".h";
$coverage_count_string ="Lines executed:";
$count = 0;
$filecount = 0;
$coverage_percent = 0;

while(<INPUTFILE>) {
my($line) = $_;
if ($count == 3) {
  $count = 0; 
} 
if(/$exclude_string/ || /$exclude_string2/) {
  #print "Found usr_include or header file on line $. \n" ;
   $count ++;
}

if ($count == 0) {
  # Print the line to the extracted_info and add a newline
  print EXTRACTED_INFO "$line\n";
  if(/$coverage_count_string/) {
    $filecount ++;
    $index = rindex($line,':') + 1;
    $percent = substr($line,$index);
    $perc_index = rindex($percent,'%');
    $percent = substr($percent,0,$perc_index);
    $coverage_percent += $percent;
  }
} else {
  #count should be 1 or 2 omit the line 
  $count ++;
}
}
if($filecount != 0 ) {
    $coverage_percent=sprintf("%.2f",$coverage_percent/$filecount);
}
print "Total No. of files : $filecount Coverage Percent : $coverage_percent ";
open(PERCENTFILE,">$ARGV[1]");
print PERCENTFILE "$coverage_percent\n" ;
close(PERCENTFILE);
close(INPUTFILE);
close(EXTRACTED_INFO);
