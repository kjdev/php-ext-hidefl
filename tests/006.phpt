--TEST--
hidefl_fetch() function: string.data
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('string'));

--EXPECT--
string(12) "Hello World!"
