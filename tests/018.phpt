--TEST--
hidefl_fetch() function: hash.data
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
hidefl.load_unserialize = 0
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('hash'));

--EXPECT--
array(3) {
  ["a"]=>
  string(1) "A"
  ["b"]=>
  string(1) "B"
  ["c"]=>
  string(1) "C"
}
