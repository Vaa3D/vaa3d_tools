#!/usr/bin/perl

use strict;

#match <\dt>

my ($cfile, $outfile, @args) = @ARGV;
my @lcargs;
my @ucargs;

#Create lower case and upper case arrays
foreach my $arg (@args) {
  push(@lcargs, lc($arg));
  push(@ucargs, uc($arg));
}
#print "@lcargs"."\n";

my %argmap = (T => [@args], t => [@lcargs], U => [@ucargs]);
#print "map:", "@{$argmap{T}}", "\n";

my $cond = 1;
my $cond_level = 0;
my $off_level = 100; #max level 100

open(CFH, '<', $cfile);
open(OUT, '>', $outfile);

my $line_num = 0;
while (my $line = <CFH>) {
  $line_num++;
  my @tokens = split(/(<\d+[TtU]=\w+[^>]*>|<\/[TtU]>)/, $line);
  if (!$tokens[0]) {
    shift(@tokens);
    if (($tokens[1] eq "\n") || ($tokens[1] eq "\r")) {
      #print("new line: ", $tokens[1]);
      pop(@tokens);
    }
  }

  while ($line = shift(@tokens)) {
    #print($line, "\n");
    if ($line =~ m/<(\d+)([TtU])=(\w+[^>]*)>/) { #match block begin
      #print("$line_num\n");
      $cond_level++;
#print($line);
      my $index = $1-1;
      my $t = $2;
      $line = $3;
      $line =~ s/\s//g;
      my @values = split(/,/,$line);
      #print "$index: "."@values"."\n";
      $cond = 0;
      if ($cond_level <= $off_level) {
        foreach my $value (@values) {
          if (${$argmap{$t}}[$index] eq $value) {
            $cond = 1;
            last;
          }
        }
        if ($cond == 0) {
          $off_level = $cond_level;
        }
      }
      #print($cond, "\n");
    } elsif ($line =~ m/<\/[TtU]>/) { #match block end
      $cond_level--;
      #print("$line_num: ", "$cond_level", " : ", "$off_level", "\n");
      if (($cond == 0) && ($cond_level < $off_level)) {
        $cond = 1;
        $off_level = 100;
      }
    } elsif ($line =~ m/<\d+[TtU]>/) { #macro replacement
      if ($cond == 1) {
        $line =~ s/<(\d+)([TtU])>/${$argmap{$2}}[$1-1]/g;
        print OUT $line;
      }
    } elsif ($cond == 1) { #plain copy
      print OUT $line;
    }
  }
}

close(CFH);
close(OUT);

if ($cond_level != 0) {
  print STDERR "Syntax error: condition block unmatched.\n";
}

