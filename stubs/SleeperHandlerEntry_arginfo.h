/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 42af91cb8e23439e782e0db658d51190bc1f1911 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandlerEntry___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, sharedObject, pmmp\\thread\\ThreadSafeArray, 0)
	ZEND_ARG_TYPE_INFO(0, id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandlerEntry_getNotifierId, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pocketmine_snooze_SleeperHandlerEntry_createNotifier, 0, 0, pocketmine\\snooze\\SleeperNotifier, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pocketmine_snooze_SleeperHandlerEntry, __construct);
ZEND_METHOD(pocketmine_snooze_SleeperHandlerEntry, getNotifierId);
ZEND_METHOD(pocketmine_snooze_SleeperHandlerEntry, createNotifier);


static const zend_function_entry class_pocketmine_snooze_SleeperHandlerEntry_methods[] = {
	ZEND_ME(pocketmine_snooze_SleeperHandlerEntry, __construct, arginfo_class_pocketmine_snooze_SleeperHandlerEntry___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_snooze_SleeperHandlerEntry, getNotifierId, arginfo_class_pocketmine_snooze_SleeperHandlerEntry_getNotifierId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(pocketmine_snooze_SleeperHandlerEntry, createNotifier, arginfo_class_pocketmine_snooze_SleeperHandlerEntry_createNotifier, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pocketmine_snooze_SleeperHandlerEntry(zend_class_entry *class_entry_pmmp_thread_ThreadSafe)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pocketmine\\snooze", "SleeperHandlerEntry", class_pocketmine_snooze_SleeperHandlerEntry_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_pmmp_thread_ThreadSafe);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	zval property_sharedObject_default_value;
	ZVAL_UNDEF(&property_sharedObject_default_value);
	zend_string *property_sharedObject_name = zend_string_init("sharedObject", sizeof("sharedObject") - 1, 1);
	zend_declare_property_ex(class_entry, property_sharedObject_name, &property_sharedObject_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_sharedObject_name);

	zval property_id_default_value;
	ZVAL_UNDEF(&property_id_default_value);
	zend_string *property_id_name = zend_string_init("id", sizeof("id") - 1, 1);
	zend_declare_property_ex(class_entry, property_id_name, &property_id_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_id_name);

	return class_entry;
}
