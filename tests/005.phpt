--TEST--
hidefl_fetch() function: double.data
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('double'));

--EXPECT--
float(123.456)
