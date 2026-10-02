/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                           |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: PocketMine Team                                              |
  +----------------------------------------------------------------------+
 */

#include <src/pmmpthread.h>
#include <src/object.h>

#define SleeperHandlerEntry_method(name) PHP_METHOD(pocketmine_snooze_SleeperHandlerEntry, name)

typedef struct _pmmpthread_sleeper_notifier_t {
	zval shared_object_zv;
	pmmpthread_zend_object_t *shared_threaded;
	zend_long notifier_id;
	zend_object std;
} pmmpthread_sleeper_notifier_t;

static inline pmmpthread_sleeper_notifier_t *pmmpthread_sleeper_notifier_fetch(zend_object *obj) {
	return (pmmpthread_sleeper_notifier_t *)((char *)obj - XtOffsetOf(pmmpthread_sleeper_notifier_t, std));
}

/* {{{ proto SleeperHandlerEntry::__construct(ThreadSafeArray $sharedObject, int $id) */
SleeperHandlerEntry_method(__construct)
{
	zval *shared_zv;
	zend_long id;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_OBJECT_OF_CLASS(shared_zv, pmmpthread_ce_array)
		Z_PARAM_LONG(id)
	ZEND_PARSE_PARAMETERS_END();

	zend_update_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("sharedObject"), shared_zv);
	zend_update_property_long(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("id"), id);
} /* }}} */

/* {{{ proto int SleeperHandlerEntry::getNotifierId() */
SleeperHandlerEntry_method(getNotifierId)
{
	zend_parse_parameters_none_throw();

	zval tmp;
	zval *id_zv = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("id"), 0, &tmp);
	if (id_zv == NULL || Z_TYPE_P(id_zv) != IS_LONG) {
		zend_throw_error(NULL, "id is not initialized or invalid");
		return;
	}

	RETURN_LONG(Z_LVAL_P(id_zv));
} /* }}} */

/* {{{ proto SleeperNotifier SleeperHandlerEntry::createNotifier() */
SleeperHandlerEntry_method(createNotifier)
{
	zend_parse_parameters_none_throw();

	zval tmp;
	zval *shared_zv = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("sharedObject"), 0, &tmp);
	if (shared_zv == NULL || Z_TYPE_P(shared_zv) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(shared_zv), pmmpthread_ce_array)) {
		zend_throw_error(NULL, "sharedObject is not initialized or invalid");
		return;
	}

	zval id_tmp;
	zval *id_zv = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("id"), 0, &id_tmp);
	if (id_zv == NULL || Z_TYPE_P(id_zv) != IS_LONG) {
		zend_throw_error(NULL, "id is not initialized or invalid");
		return;
	}

	object_init_ex(return_value, pmmpthread_ce_sleeper_notifier);
	pmmpthread_sleeper_notifier_t *notifier = pmmpthread_sleeper_notifier_fetch(Z_OBJ_P(return_value));

	ZVAL_COPY(&notifier->shared_object_zv, shared_zv);
	notifier->shared_threaded = PMMPTHREAD_FETCH_FROM(Z_OBJ_P(shared_zv));
	notifier->notifier_id = Z_LVAL_P(id_zv);

	zend_update_property(pmmpthread_ce_sleeper_notifier, Z_OBJ_P(return_value), ZEND_STRL("sharedObject"), shared_zv);
	zend_update_property_long(pmmpthread_ce_sleeper_notifier, Z_OBJ_P(return_value), ZEND_STRL("notifierId"), Z_LVAL_P(id_zv));
} /* }}} */
