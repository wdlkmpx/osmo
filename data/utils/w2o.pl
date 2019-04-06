#!/usr/bin/perl
# File: w2o.pl
# Author: (c) joakim@bildrulle.nu, 2009, Mutual none-obligatory as-is license
# Usage:  iconv -t UTF-8 -f UTF-16 wammu-back-file | w2o.pl > imports.csv

@text = <STDIN>;
chop @text;
$text = join(' ',@text);
@db   = split(/\[/,$text);
foreach $line (@db ) { 
    ($id, $ln) = split(/\]/,$line);
    $ln =~ s/ = /=/g;
    $ln =~ s/"//g;
    $ln =~ s/\r//g;
    $ln =~ s/ Entry/\n/g;
#    print "$id - $ln\n";
    %fields = ();
    foreach $f (split(/\n/, $ln)){
	($key, $val) = split(/=/, $f);
	$fields{$key} = $val;
	if ($val ne ""){
	    $key =~ s/Type/Text/g;
	    $fields{$val} = $key;
	}
    }
#DEBUG
#    foreach $f (keys(%fields)){
#	print "  $f - $fields{$f}\n";
#    }

# For Wammu SIM card backup
     print "\"$fields{$fields{'Name'}}\",";
     print "\"$fields{$fields{'NumberGeneral'}}\"";

# For Wammu PHONE memory backup
#    print "\"$fields{$fields{'FirstName'}}\",";
#    print "\"$fields{$fields{'Company'}}\",";
#    print "\"$fields{$fields{'NumberWork'}}\",";
#    print "\"$fields{$fields{'NumberMobile'}}\",";
#    print "\"$fields{$fields{'NumberGeneral'}}\",";
#    print "\"$fields{$fields{'Email'}}\",";
#    print "\"$fields{$fields{'Jobtitle'}}\",";
#    print "\"$fields{$fields{'LUID'}}\",";
#    print "\"$fields{$fields{'Country'}}\",";
#    print "\"$fields{$fields{'Zip'}}\",";
#    print "\"$fields{$fields{'State'}}\",";
#    print "\"$fields{$fields{'Note'}}\",";
#    print "\"$fields{$fields{'City'}}\"";
    print "\n";
}


 	  	 
