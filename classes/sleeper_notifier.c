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
#include <src/monitor.h>

#define SleeperNotifier_method(name) PHP_METHOD(pocketmine_snooze_SleeperNotifier, name)

typedef struct _pmmpthread_sleeper_notifier_t {
	zval shared_object_zv;
	pmmpthread_zend_object_t *shared_threaded;
	zend_long notifier_id;
	zend_object std;
} pmmpthread_sleeper_notifier_t;

static inline pmmpthread_sleeper_notifier_t *pmmpthread_sleeper_notifier_fetch(zend_object *obj) {
	return (pmmpthread_sleeper_notifier_t *)((char *)obj - XtOffsetOf(pmmpthread_sleeper_notifier_t, std));
}

static zend_object_handlers pmmpthread_sleeper_notifier_handlers;

static void pmmpthread_sleeper_notifier_free(zend_object *object) {
	pmmpthread_sleeper_notifier_t *notifier = pmmpthread_sleeper_notifier_fetch(object);

	zval_ptr_dtor(&notifier->shared_object_zv);
	zend_object_std_dtor(&notifier->std);
}

static HashTable *pmmpthread_sleeper_notifier_get_gc(zend_object *object, zval **table, int *n) {
	pmmpthread_sleeper_notifier_t *notifier = pmmpthread_sleeper_notifier_fetch(object);
	zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();

	if (!Z_ISUNDEF(notifier->shared_object_zv)) {
		zend_get_gc_buffer_add_zval(buffer, &notifier->shared_object_zv);
	}

	zend_get_gc_buffer_use(buffer, table, n);
	return zend_std_get_properties(object);
}

zend_object *pmmpthread_sleeper_notifier_ctor(zend_class_entry *entry) {
	pmmpthread_sleeper_notifier_t *notifier = zend_object_alloc(sizeof(pmmpthread_sleeper_notifier_t), entry);

	zend_object_std_init(&notifier->std, entry);
	object_properties_init(&notifier->std, entry);
	notifier->std.handlers = &pmmpthread_sleeper_notifier_handlers;

	ZVAL_UNDEF(&notifier->shared_object_zv);
	notifier->shared_threaded = NULL;
	notifier->notifier_id = 0;

	return &notifier->std;
}

void pmmpthread_sleeper_notifier_init_handlers(void) {
	memcpy(&pmmpthread_sleeper_notifier_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	pmmpthread_sleeper_notifier_handlers.offset = XtOffsetOf(pmmpthread_sleeper_notifier_t, std);
	pmmpthread_sleeper_notifier_handlers.free_obj = pmmpthread_sleeper_notifier_free;
	pmmpthread_sleeper_notifier_handlers.get_gc = pmmpthread_sleeper_notifier_get_gc;
}

/* {{{ proto SleeperNotifier::__construct(ThreadSafeArray $sharedObject, int $notifierId) */
SleeperNotifier_method(__construct)
{
	zval *shared_zv;
	zend_long notifier_id;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_OBJECT_OF_CLASS(shared_zv, pmmpthread_ce_array)
		Z_PARAM_LONG(notifier_id)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_sleeper_notifier_t *notifier = pmmpthread_sleeper_notifier_fetch(Z_OBJ_P(getThis()));

	if (!Z_ISUNDEF(notifier->shared_object_zv)) {
		zval_ptr_dtor(&notifier->shared_object_zv);
	}

	ZVAL_COPY(&notifier->shared_object_zv, shared_zv);
	notifier->shared_threaded = PMMPTHREAD_FETCH_FROM(Z_OBJ_P(shared_zv));
	notifier->notifier_id = notifier_id;

	zend_update_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("sharedObject"), shared_zv);
	zend_update_property_long(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("notifierId"), notifier_id);
} /* }}} */

/* {{{ proto void SleeperNotifier::wakeupSleeper() */
SleeperNotifier_method(wakeupSleeper)
{
	zend_parse_parameters_none_throw();

	pmmpthread_sleeper_notifier_t *notifier = pmmpthread_sleeper_notifier_fetch(Z_OBJ_P(getThis()));
	if (UNEXPECTED(notifier->shared_threaded == NULL)) {
		zend_throw_error(NULL, "SleeperNotifier was not initialized properly");
		return;
	}

	pmmpthread_object_t *ts_obj = notifier->shared_threaded->ts_obj;
	zend_ulong notifier_id = (zend_ulong)notifier->notifier_id;

	if (EXPECTED(pmmpthread_monitor_lock(&ts_obj->monitor))) {
		if (!zend_hash_index_exists(&ts_obj->props.hash, notifier_id)) {
			zval zstorage;
			ZVAL_LONG(&zstorage, (zend_long)notifier_id);
			zend_hash_index_update(&ts_obj->props.hash, notifier_id, &zstorage);
			ts_obj->props.modcount++;
			pmmpthread_monitor_notify(&ts_obj->monitor);
		}
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
} /* }}} */
