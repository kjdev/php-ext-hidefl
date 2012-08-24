# Hidef lite function Extension for PHP #

This extension allows hidef lite.

## php.ini Option ##

### hidefl.data_path _string_ ###

* default: ""
* change: PHP\_INI\_SYSTEM

constants data file path.

### hidefl.data_extension _string_ ###

* default: ".data"
* change: PHP\_INI\_SYSTEM

constants data file extension.

### hidefl.load_unserialize _bool_ ###

* default: 1
* change: PHP\_INI\_SYSTEM

data file load to run unserialize.


## Function ##

* hidefl\_fetch — Get constants data
* hidefl\_save — Set constants data


### hidefl\_fetch — Get constants data ###

#### Description ####

mixed **hidefl\_fetch** ( string _$key_ [ , bool _$thaw_ ] )

Get constants data by simple files.

#### Pameters ####

* _key_

  Constants data key name.

* _thaw_

  Unused.

#### Return Values ####

Returns TRUE on success or FALSE on failure.


### hidefl\_save — Set constants data ###

#### Description ####

bool **hidefl\_save** ( string _$key_ , mixed _$value_ ] )

Set constants data by simple files.

#### Pameters ####

* _key_

  Constants data key name.

* _valued_

  Constants data.

#### Return Values ####

Returns TRUE on success or FALSE on failure.

## Examples ##

    hidefl_fetch('test');

    hidefl_save('test', array('a', 'b'));
