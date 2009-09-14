package IrstLMRegressionTesting;

use strict;

die "*** Cannot find variable IRSTLM in environment ***\n"
if !$ENV{IRSTLM};

die "*** Cannot find directory IRSTLM: $ENV{IRSTLM} ***\n"
if ! -d $ENV{IRSTLM};

# if your tests need a new version of the test data, increment this
# and make sure that a irstlm-regression-tests-vX.Y is available
use constant TESTING_DATA_VERSION => '2.0';

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
  push @ds, "/tmp/$mrtp";
  push @ds, "/var/tmp/$mrtp";
  foreach my $d (@ds) {
    print STDERR "d:$d\n";
    next unless (-d $d);
    if (!-d "$d/lm") {
      print STDERR "Found $d but it is malformed: missing subdir lm/\n";
      next;
    }

    return $d;
  }
  print STDERR<<EOT;

You do not appear to have the regression testing data installed.
You may either specify a non-standard location (absolute path) 
when running the test suite with the --data-dir option, 
or, you may install it in any one of the following
standard locations: $test_script_root, /tmp, or /var/tmp with these
commands:
  cd <DESIRED_INSTALLATION_DIRECTORY>
  wget http://hermes.itc.it/people/bertoldi/irstlm/regression-test/irstlm-reg-test-data-$data_version.tgz
  (this is a temporary link)
  tar xzf irstlm-reg-test-data-$data_version.tgz
  rm irstlm-reg-test-data-$data_version.tgz

EOT
	exit 1;
}

1;



