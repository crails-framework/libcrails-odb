if ($config.libcrails_odb.with_sqlite == true)
  ./: {*/ -build/} doc{README.md} manifest
else
  ./: {*/ -build/ -tests/} doc{README.md} manifest

tests/: install = false
