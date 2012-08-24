--TEST--
hidefl_fetch() function: false.data
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('false'));

--EXPECT--
bool(false)
