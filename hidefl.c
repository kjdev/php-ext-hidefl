
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_scandir.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "php_hidefl.h"

#define HIDEFL_DEFAULT_EXTENSION ".data"

static int le_hidefl;

ZEND_DECLARE_MODULE_GLOBALS(hidefl)

ZEND_FUNCTION(hidefl_fetch);
ZEND_FUNCTION(hidefl_wrap);
ZEND_FUNCTION(hidefl_save);

ZEND_BEGIN_ARG_INFO_EX(arginfo_hidefl_fetch, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, thaw)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hidefl_wrap, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hidefl_save, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hidefl.data_path", (char*)NULL, PHP_INI_SYSTEM,
                      OnUpdateString, data_path, zend_hidefl_globals,
                      hidefl_globals)
    STD_PHP_INI_ENTRY("hidefl.data_extension", (char*)NULL, PHP_INI_SYSTEM,
                      OnUpdateString, data_extension, zend_hidefl_globals,
                      hidefl_globals)
    STD_PHP_INI_BOOLEAN("hidefl.load_unserialize", "1", PHP_INI_SYSTEM,
                        OnUpdateBool, load_unserialize,  zend_hidefl_globals,
                        hidefl_globals)
PHP_INI_END()

static const zend_function_entry hidefl_functions[] = {
    ZEND_FE(hidefl_fetch, arginfo_hidefl_fetch)
    ZEND_FE(hidefl_wrap, arginfo_hidefl_wrap)
    ZEND_FE(hidefl_save, arginfo_hidefl_save)
    ZEND_FE_END
};

static inline void
hidefl_init_globals(zend_hidefl_globals *hidefl_globals)
{
    hidefl_globals->data_path = NULL;
    hidefl_globals->data_extension = NULL;
    hidefl_globals->load_unserialize = 1;
}

static inline int
hidefl_serialize(smart_str *retval, zval **struc TSRMLS_DC)
{
    php_serialize_data_t var_hash;

    PHP_VAR_SERIALIZE_INIT(var_hash);
    php_var_serialize(retval, struc, &var_hash TSRMLS_CC);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);

    return SUCCESS;
}

static inline int
hidefl_unserialize(zval *retval, const char *payload,
                   size_t payload_len TSRMLS_DC)
{
    php_unserialize_data_t var_hash;

    PHP_VAR_UNSERIALIZE_INIT(var_hash);
    if (!php_var_unserialize(&retval, (const unsigned char **)&payload,
                             (const unsigned char *)payload + payload_len,
                             &var_hash TSRMLS_CC)) {
        PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
        zval_ptr_dtor(&retval);
        return FAILURE;
    }
    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

    return SUCCESS;
}

static inline int
hidefl_plist_key(char **key, char *id)
{
    int key_len = 0;
    key_len = spprintf(key, 0, "hidefl:id=%s", id);
    key_len += 1;
    return key_len;
}

static inline int
hidefl_load_data(const char *data_file TSRMLS_DC)
{
    char *p;
    char key[MAXPATHLEN] = {0,};
    unsigned int key_len;

    if (access(data_file, R_OK) != 0) {
        zend_error(E_WARNING, "hidefl cannot read %s", data_file);
        return FAILURE;
    }

    p = strrchr(data_file, DEFAULT_SLASH);
    if (p && p[1]) {
        strlcpy(key, p+1, sizeof(key));
        p = strrchr(key, '.');
        if (p) {
            p[0] = '\0';
            key_len = strlen(key);

            zval *data = NULL;
            struct stat sb;
            FILE *fp;
            char *contents;
            long contents_len = 0;

            if (VCWD_STAT(data_file, &sb) == -1) {
                return FAILURE;
            }

            fp = fopen(data_file, "rb");
            if (!fp || sb.st_size == 0) {
                return FAILURE;
            }

            contents_len = sizeof(char) * sb.st_size;
            contents = emalloc(contents_len);
            contents_len = fread(contents, 1, contents_len, fp);

            if (HIDEFL_G(load_unserialize)) {
                /* unserialize */
                data = (zval *)pemalloc(sizeof(zval), 1);
                INIT_PZVAL(data);
                if (hidefl_unserialize(data, contents,
                                       contents_len TSRMLS_CC) != SUCCESS) {
                    if (data) {
                        pefree(data, 1);
                    }
                    efree(contents);
                    fclose(fp);
                    return FAILURE;
                }
            } else {
                /* string */
                data = (zval *)pemalloc(sizeof(zval), 1);
                INIT_PZVAL(data);
                Z_TYPE_P(data) = IS_STRING;
                data->value.str.val = pestrndup(contents, contents_len, 1);
                data->value.str.len = contents_len;
            }

            efree(contents);
            fclose(fp);

            zend_rsrc_list_entry le;
            le.type = le_hidefl;
            le.ptr = data;

            char *plist_key = NULL;
            int plist_key_len = 0;
            plist_key_len = hidefl_plist_key(&plist_key, key);

            if (data == NULL ||
                zend_hash_update(&EG(persistent_list), plist_key,
                                 plist_key_len, (void *)&le,
                                 sizeof(zend_rsrc_list_entry),
                                 NULL) == FAILURE) {
                if (plist_key) {
                    efree(plist_key);
                }
                if (data) {
                    pefree(data, 1);
                }
                zend_error(E_ERROR, "Unable to add %s to the hidefl data",
                           data_file);
                return FAILURE;
            }

            if (plist_key) {
                efree(plist_key);
            }

            return SUCCESS;
        }
    }

    return FAILURE;
}

ZEND_RSRC_DTOR_FUNC(hidefl_list_dtor)
{
    if (rsrc->ptr) {
        zval *data = (zval *)rsrc->ptr;
        if (data) {
            pefree(data, 1);
        }
        rsrc->ptr = NULL;
    }
}

ZEND_MINIT_FUNCTION(hidefl)
{
    ZEND_INIT_MODULE_GLOBALS(hidefl, hidefl_init_globals, NULL);

    le_hidefl = zend_register_list_destructors_ex(
        NULL, hidefl_list_dtor, "Hidefl persistent context", module_number);

    REGISTER_INI_ENTRIES();

    if (HIDEFL_G(data_path)) {
        char *p = NULL;
        char file[MAXPATHLEN] = {0,};
        int i, ndir;
        struct dirent **nlist = NULL;
        if ((ndir = php_scandir(HIDEFL_G(data_path), &nlist,
                                0, php_alphasort)) > 0) {
            for (i = 0; i < ndir; i++) {
                if (!(p = strrchr(nlist[i]->d_name, '.')) ||
                    (p && HIDEFL_G(data_extension)
                     && strcmp(p, HIDEFL_G(data_extension)) != 0) ||
                    (p && strcmp(p, HIDEFL_DEFAULT_EXTENSION) != 0)) {
                    free(nlist[i]);
                    continue;
                }
                snprintf(file, MAXPATHLEN, "%s%c%s",
                         HIDEFL_G(data_path), DEFAULT_SLASH, nlist[i]->d_name);
                hidefl_load_data(file TSRMLS_CC);
                free(nlist[i]);
            }
            free(nlist);
        }
    }

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(hidefl)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(hidefl)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Hidefl support", "enabled");
    php_info_print_table_row(2, "Extension Version", HIDEFL_EXT_VERSION);
    php_info_print_table_end();
}

zend_module_entry hidefl_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "hidefl",
    hidefl_functions,
    ZEND_MINIT(hidefl),
    ZEND_MSHUTDOWN(hidefl),
    NULL,
    NULL,
    ZEND_MINFO(hidefl),
#if ZEND_MODULE_API_NO >= 20010901
    HIDEFL_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_HIDEFL
ZEND_GET_MODULE(hidefl)
#endif

ZEND_FUNCTION(hidefl_fetch)
{
    char *key = NULL;
    long key_len = 0;
    zend_bool thaw = 0;
    zval *data = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s|b", &key, &key_len, &thaw) == FAILURE) {
        RETURN_FALSE;
    }

    char *plist_key = NULL;
    int plist_key_len = 0;
    plist_key_len = hidefl_plist_key(&plist_key, key);

    zend_rsrc_list_entry *le = NULL;
    if (zend_hash_find(&EG(persistent_list), plist_key, plist_key_len,
                       (void *)&le) == SUCCESS) {
        if (le->type == le_hidefl) {
            data = le->ptr;
        }
    }

    if (plist_key) {
        efree(plist_key);
    }

    if (!data) {
        RETURN_FALSE;
    }

    if (HIDEFL_G(load_unserialize)) {
        *return_value = *data;
        zval_copy_ctor(return_value);
    } else {
        /* unserialize */
        if (hidefl_unserialize(return_value, Z_STRVAL_P(data),
                               Z_STRLEN_P(data) TSRMLS_CC) != SUCCESS) {
            zval_dtor(return_value);
            RETVAL_FALSE;
        }
    }
}

ZEND_FUNCTION(hidefl_wrap)
{
    RETURN_FALSE;
}

ZEND_FUNCTION(hidefl_save)
{
    char *key;
    long key_len;
    zval **struc;
    smart_str buf = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "sZ", &key, &key_len, &struc) == FAILURE) {
        return;
    }

    hidefl_serialize(&buf, struc TSRMLS_CC);

    if (!buf.c) {
        RETURN_FALSE;
    }

    if (HIDEFL_G(data_path) && buf.c) {
        FILE *fp;
        char file[MAXPATHLEN] = {0,};

        if (HIDEFL_G(data_extension)) {
            snprintf(file, MAXPATHLEN, "%s%c%s%s",
                     HIDEFL_G(data_path), DEFAULT_SLASH,
                     key, HIDEFL_G(data_extension));
        } else {
            snprintf(file, MAXPATHLEN, "%s%c%s%s",
                     HIDEFL_G(data_path), DEFAULT_SLASH,
                     key, HIDEFL_DEFAULT_EXTENSION);
        }

        fp = fopen(file, "wb");
        if (!fp) {
            smart_str_free(&buf);
            RETVAL_FALSE;
        }

        fwrite(buf.c, 1, buf.len, fp);

        fclose(fp);
    } else {
        smart_str_free(&buf);
        RETVAL_FALSE;
    }

    smart_str_free(&buf);

    RETVAL_TRUE;
}
