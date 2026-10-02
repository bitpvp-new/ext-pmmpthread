/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9cb151121d5a7d30f353a1a37c358055ee1ff982 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandler___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandler_addNotifier, 0, 1, pocketmine\\snooze\\SleeperHandlerEntry, 0)
	ZEND_ARG_OBJ_INFO(0, handler, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandler_removeNotifier, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, notifierId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandler_sleepUntil, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, unixTime, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandler_sleepUntilNotification, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pocketmine_snooze_SleeperHandler_processNotifications arginfo_class_pocketmine_snooze_SleeperHandler_sleepUntilNotification


ZEND_METHOD(pocketmine_snooze_SleeperHandler, __construct);
ZEND_METHOD(pocketmine_snooze_SleeperHandler, addNotifier);
ZEND_METHOD(pocketmine_snooze_SleeperHandler, removeNotifier);
ZEND_METHOD(pocketmine_snooze_SleeperHandler, sleepUntil);
ZEND_METHOD(pocketmine_snooze_SleeperHandler, sleepUntilNotification);
ZEND_METHOD(pocketmine_snooze_SleeperHandler, processNotifications);


static const zend_function_entry class_pocketmine_snooze_SleeperHandler_methods[] = {
	ZEND_ME(pocketmine_snooze_SleeperHandler, __construct, arginfo_class_pocketmine_snooze_SleeperHandler___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandler, addNotifier, arginfo_class_pocketmine_snooze_SleeperHandler_addNotifier, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandler, removeNotifier, arginfo_class_pocketmine_snooze_SleeperHandler_removeNotifier, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandler, sleepUntil, arginfo_class_pocketmine_snooze_SleeperHandler_sleepUntil, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandler, sleepUntilNotification, arginfo_class_pocketmine_snooze_SleeperHandler_sleepUntilNotification, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandler, processNotifications, arginfo_class_pocketmine_snooze_SleeperHandler_processNotifications, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pocketmine_snooze_SleeperHandler(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pocketmine\\snooze", "SleeperHandler", class_pocketmine_snooze_SleeperHandler_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);

	zval property_sharedObject_default_value;
	ZVAL_UNDEF(&property_sharedObject_default_value);
	zend_string *property_sharedObject_name = zend_string_init("sharedObject", sizeof("sharedObject") - 1, 1);
	zend_declare_property_ex(class_entry, property_sharedObject_name, &property_sharedObject_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_sharedObject_name);

	zval property_handlers_default_value;
	ZVAL_UNDEF(&property_handlers_default_value);
	zend_string *property_handlers_name = zend_string_init("handlers", sizeof("handlers") - 1, 1);
	zend_declare_property_ex(class_entry, property_handlers_name, &property_handlers_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_handlers_name);

	zval property_nextSleeperId_default_value;
	ZVAL_LONG(&property_nextSleeperId_default_value, 0);
	zend_string *property_nextSleeperId_name = zend_string_init("nextSleeperId", sizeof("nextSleeperId") - 1, 1);
	zend_declare_property_ex(class_entry, property_nextSleeperId_name, &property_nextSleeperId_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_nextSleeperId_name);

	return class_entry;
}
