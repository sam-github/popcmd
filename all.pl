
$index = 1;

while (0 == system("popcmd sroberts\@pop.uniserve.com retr $index > $index.txt"))
{
	$index++;
}

