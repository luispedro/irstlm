package IrstLMRegressionTesting;

use strict;

# if your tests need a new version of the test data, increment this
# and make sure that a irstlm-regression-tests-vX.Y is available
use constant TESTING_DATA_VERSION => '0.1';

# find the data directory in a few likely locations and make sure
# that it is the correct version
sub find_data_directory
{
  my ($test_script_root, $data_dir) = @_;
	my $data_version = TESTING_DATA_VERSION;
  my @ds = ();
  my $mrtp = "irstlm-reg-test-data-$data_version";
  push @ds, $data_dir if defined $data_dir;
  push @ds, "$test_script_root/$mrtp";
  push @ds, "/Users/nicolabertoldi/workspace/irstlm/$mrtp";
  push @ds, "/tmp/$mrtp";
  push @ds, "/var/tmp/$mrtp";
  foreach my $d (@ds) {
    print STDERR "d:$d\n";
    next unless (-d $d);
    return $d;
  }
  print STDERR<<EOT;

You do not appear to have the regression testing data installed.
You may either specify a non-standard location when running
the test suite with the --data-dir option, 
or, you may install it in any one of the following
standard locations: $test_script_root, /tmp, or /var/tmp with these
commands:
  cd <DESIRED_INSTALLATION_DIRECTORY>
 MODIFY ACCORDING TO IRSTLM  
  wget http://www.statmt.org/moses/reg-testing/moses-regression-tests-v$data_version.tar
  tar xf irstlm-regression-tests-v$data_version.tar
  rm irstlm-regression-tests-v$data_version.tar

EOT
	exit 1;
}

1;



