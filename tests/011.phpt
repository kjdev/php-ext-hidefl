--TEST--
hidefl_fetch() function: object
--INI--
hidefl.data_path = /home/kamijo/works/bitbucket/php-ext-hidefl/tests
--SKIPIF--
<?php
echo "skip"
?>
--FILE--
<?php
if (!extension_loaded('hidefl')) {
    dl('hidefl.' . PHP_SHLIB_SUFFIX);
}

var_dump(hidefl_fetch('object'));

--EXPECT--
