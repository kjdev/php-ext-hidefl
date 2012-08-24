
#ifndef PHP_HIDEFL_H
#define PHP_HIDEFL_H

#define HIDEFL_EXT_VERSION "0.0.1"

extern zend_module_entry hidefl_module_entry;
#define phpext_hidefl_ptr &hidefl_module_entry

#ifdef PHP_WIN32
#   define PHP_HIDEFL_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_HIDEFL_API __attribute__ ((visibility("default")))
#else
#   define PHP_HIDEFL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(hidefl)
    char *data_path;
    char *data_extension;
    zend_bool load_unserialize;
ZEND_END_MODULE_GLOBALS(hidefl)

#ifdef ZTS
#define HIDEFL_G(v) TSRMG(hidefl_globals_id, zend_hidefl_globals *, v)
#else
#define HIDEFL_G(v) (hidefl_globals.v)
#endif

#endif  /* PHP_HIDEFL_H */
