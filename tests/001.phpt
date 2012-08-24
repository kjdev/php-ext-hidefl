--TEST--
hidefl_fetch() function
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

/*
var_dump(hidefl_fetch('array1'));
array(1) {
  ["answer"]=>
  string(2) "42"
}
*/

var_dump(hidefl_fetch('test'));

--EXPECT--
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  array(1) {
    [0]=>
    string(1) "b"
  }
}
