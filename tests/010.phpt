--TEST--
hidefl_fetch() function: hash_nest.data
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('hash_nest'));

--EXPECT--
array(2) {
  ["a"]=>
  string(1) "A"
  ["b"]=>
  array(2) {
    ["b"]=>
    string(1) "B"
    ["c"]=>
    array(1) {
      ["c"]=>
      string(1) "C"
    }
  }
}
