#!/usr/bin/perl
my $title = shift || "unknown";

print qq{<?xml version="1.0" encoding="UTF-8"?>
<document>
<metadata>
	<title>$title</title>
</metadata>
<section>
<title>$title</title>
<pre>
};

while (<>) {
	print xml_entify($_);
}
print q{
</pre>
</section>
</document>
};
exit 0;

sub xml_entify {
        my ($str, $no_escape_tilde) = @_;
	return undef unless(defined($str));
        $str =~ s/[\x00-\x08\x10\x0B\x0C\x0E-\x1F]//g;
#        $test =~ s/([&<>"\x{80}-\x{10ffff}])/"&#".ord($1).";"/eg;
	$str =~ s/&/&amp;/g;
	$str =~ s/</&lt;/g;
	$str =~ s/>/&lt;/g;
        $str =~ s/~/~~/g unless $no_escape_tilde;
        return $str;
}

