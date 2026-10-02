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
#include <src/store.h>
#include <src/monitor.h>

#define SleeperHandler_method(name) PHP_METHOD(pocketmine_snooze_SleeperHandler, name)

typedef struct _pmmpthread_sleeper_handler_t {
	zval shared_object_zv;
	pmmpthread_zend_object_t *shared_threaded;
	HashTable handlers;
	zend_long next_sleeper_id;
	zend_object std;
} pmmpthread_sleeper_handler_t;

static inline pmmpthread_sleeper_handler_t *pmmpthread_sleeper_handler_fetch(zend_object *obj) {
	return (pmmpthread_sleeper_handler_t *)((char *)obj - XtOffsetOf(pmmpthread_sleeper_handler_t, std));
}

static zend_object_handlers pmmpthread_sleeper_handler_handlers;

static void pmmpthread_sleeper_handler_free(zend_object *object) {
	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(object);

	zend_hash_destroy(&handler->handlers);
	zval_ptr_dtor(&handler->shared_object_zv);
	zend_object_std_dtor(&handler->std);
}

static HashTable *pmmpthread_sleeper_handler_get_gc(zend_object *object, zval **table, int *n) {
	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(object);
	zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();

	if (!Z_ISUNDEF(handler->shared_object_zv)) {
		zend_get_gc_buffer_add_zval(buffer, &handler->shared_object_zv);
	}
	zval *callback;
	ZEND_HASH_FOREACH_VAL(&handler->handlers, callback) {
		zend_get_gc_buffer_add_zval(buffer, callback);
	} ZEND_HASH_FOREACH_END();

	zend_get_gc_buffer_use(buffer, table, n);
	return zend_std_get_properties(object);
}

zend_object *pmmpthread_sleeper_handler_ctor(zend_class_entry *entry) {
	pmmpthread_sleeper_handler_t *handler = zend_object_alloc(sizeof(pmmpthread_sleeper_handler_t), entry);

	zend_object_std_init(&handler->std, entry);
	object_properties_init(&handler->std, entry);
	handler->std.handlers = &pmmpthread_sleeper_handler_handlers;

	ZVAL_UNDEF(&handler->shared_object_zv);
	handler->shared_threaded = NULL;
	zend_hash_init(&handler->handlers, 8, NULL, ZVAL_PTR_DTOR, 0);
	handler->next_sleeper_id = 0;

	return &handler->std;
}

void pmmpthread_sleeper_handler_init_handlers(void) {
	memcpy(&pmmpthread_sleeper_handler_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	pmmpthread_sleeper_handler_handlers.offset = XtOffsetOf(pmmpthread_sleeper_handler_t, std);
	pmmpthread_sleeper_handler_handlers.free_obj = pmmpthread_sleeper_handler_free;
	pmmpthread_sleeper_handler_handlers.get_gc = pmmpthread_sleeper_handler_get_gc;
}

static void pmmpthread_sleeper_handler_process_notifications_internal(pmmpthread_sleeper_handler_t *handler)
{
	if (UNEXPECTED(handler->shared_threaded == NULL)) {
		return;
	}

	pmmpthread_object_t *ts_obj = handler->shared_threaded->ts_obj;

	while (1) {
		zend_ulong count = 0;
		zend_ulong *notifiers = NULL;
		zend_ulong local_buf[64];

		if (EXPECTED(pmmpthread_monitor_lock(&ts_obj->monitor))) {
			uint32_t num_elements = zend_hash_num_elements(&ts_obj->props.hash);
			if (num_elements > 0) {
				if (EXPECTED(num_elements <= 64)) {
					notifiers = local_buf;
				} else {
					notifiers = (zend_ulong*)emalloc(sizeof(zend_ulong) * num_elements);
				}

				zend_ulong idx;
				zend_string *key;
				ZEND_HASH_FOREACH_KEY(&ts_obj->props.hash, idx, key) {
					if (!key) {
						notifiers[count++] = idx;
					}
				} ZEND_HASH_FOREACH_END();

				zend_hash_clean(&ts_obj->props.hash);
				pmmpthread_store_invalidate_bounds(&ts_obj->props);
				ts_obj->props.modcount++;
			}
			pmmpthread_monitor_unlock(&ts_obj->monitor);
		}

		if (count == 0) {
			break;
		}

		for (zend_ulong i = 0; i < count; i++) {
			zend_ulong notifier_id = notifiers[i];
			zval *callback = zend_hash_index_find(&handler->handlers, notifier_id);
			if (callback != NULL && !Z_ISUNDEF_P(callback)) {
				zval retval;
				zend_fcall_info fci;
				zend_fcall_info_cache fcc;
				char *err = NULL;

				if (EXPECTED(zend_fcall_info_init(callback, 0, &fci, &fcc, NULL, &err) == SUCCESS)) {
					fci.retval = &retval;
					zend_call_function(&fci, &fcc);
					zval_ptr_dtor(&retval);
				} else if (err) {
					efree(err);
				}

				if (UNEXPECTED(EG(exception))) {
					if (notifiers != local_buf) {
						efree(notifiers);
					}
					return;
				}
			}
		}

		if (notifiers != local_buf) {
			efree(notifiers);
		}
	}
}

/* {{{ proto void SleeperHandler::__construct() */
SleeperHandler_method(__construct)
{
	zend_parse_parameters_none_throw();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));

	if (!Z_ISUNDEF(handler->shared_object_zv)) {
		zval_ptr_dtor(&handler->shared_object_zv);
	}

	object_init_ex(&handler->shared_object_zv, pmmpthread_ce_array);
	handler->shared_threaded = PMMPTHREAD_FETCH_FROM(Z_OBJ(handler->shared_object_zv));
	handler->next_sleeper_id = 0;

	zend_update_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("sharedObject"), &handler->shared_object_zv);
	zend_update_property_long(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("nextSleeperId"), 0);
} /* }}} */

/* {{{ proto SleeperHandlerEntry SleeperHandler::addNotifier(Closure $handler) */
SleeperHandler_method(addNotifier)
{
	zval *handler_closure;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(handler_closure, zend_ce_closure)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));
	if (UNEXPECTED(Z_ISUNDEF(handler->shared_object_zv))) {
		zend_throw_error(NULL, "SleeperHandler was not initialized properly");
		return;
	}

	zend_long id = handler->next_sleeper_id++;

	Z_TRY_ADDREF_P(handler_closure);
	zend_hash_index_update(&handler->handlers, (zend_ulong)id, handler_closure);

	zend_update_property_long(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("nextSleeperId"), handler->next_sleeper_id);

	object_init_ex(return_value, pmmpthread_ce_sleeper_handler_entry);
	zend_update_property(pmmpthread_ce_sleeper_handler_entry, Z_OBJ_P(return_value), ZEND_STRL("sharedObject"), &handler->shared_object_zv);
	zend_update_property_long(pmmpthread_ce_sleeper_handler_entry, Z_OBJ_P(return_value), ZEND_STRL("id"), id);
} /* }}} */

/* {{{ proto void SleeperHandler::removeNotifier(int $notifierId) */
SleeperHandler_method(removeNotifier)
{
	zend_long notifier_id;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(notifier_id)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));
	zend_hash_index_del(&handler->handlers, (zend_ulong)notifier_id);
} /* }}} */

/* {{{ proto void SleeperHandler::sleepUntil(float $unixTime) */
SleeperHandler_method(sleepUntil)
{
	double unix_time;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_DOUBLE(unix_time)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));
	if (UNEXPECTED(handler->shared_threaded == NULL)) {
		zend_throw_error(NULL, "SleeperHandler was not initialized properly");
		return;
	}

	pmmpthread_object_t *ts_obj = handler->shared_threaded->ts_obj;

	while (1) {
		pmmpthread_sleeper_handler_process_notifications_internal(handler);
		if (UNEXPECTED(EG(exception))) {
			return;
		}

		struct timeval tv;
		gettimeofday(&tv, NULL);
		double now = (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
		double remaining = unix_time - now;

		if (remaining > 0.0) {
			zend_long sleep_us = (zend_long)(remaining * 1000000.0);
			if (sleep_us > 0) {
				if (EXPECTED(pmmpthread_monitor_lock(&ts_obj->monitor))) {
					if (zend_hash_num_elements(&ts_obj->props.hash) == 0) {
						pmmpthread_monitor_wait(&ts_obj->monitor, sleep_us);
					}
					pmmpthread_monitor_unlock(&ts_obj->monitor);
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}

	pmmpthread_sleeper_handler_process_notifications_internal(handler);
} /* }}} */

/* {{{ proto void SleeperHandler::sleepUntilNotification() */
SleeperHandler_method(sleepUntilNotification)
{
	zend_parse_parameters_none_throw();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));
	if (UNEXPECTED(handler->shared_threaded == NULL)) {
		zend_throw_error(NULL, "SleeperHandler was not initialized properly");
		return;
	}

	pmmpthread_object_t *ts_obj = handler->shared_threaded->ts_obj;

	if (EXPECTED(pmmpthread_monitor_lock(&ts_obj->monitor))) {
		if (zend_hash_num_elements(&ts_obj->props.hash) == 0) {
			pmmpthread_monitor_wait(&ts_obj->monitor, 0);
		}
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}

	pmmpthread_sleeper_handler_process_notifications_internal(handler);
} /* }}} */

/* {{{ proto void SleeperHandler::processNotifications() */
SleeperHandler_method(processNotifications)
{
	zend_parse_parameters_none_throw();

	pmmpthread_sleeper_handler_t *handler = pmmpthread_sleeper_handler_fetch(Z_OBJ_P(getThis()));
	pmmpthread_sleeper_handler_process_notifications_internal(handler);
} /* }}} */
