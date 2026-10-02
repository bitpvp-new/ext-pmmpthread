--TEST--
SleeperHandler callbacks participate in garbage collection
--FILE--
<?php
$handler = new \pocketmine\snooze\SleeperHandler();
$handler->addNotifier(static function() use ($handler) : void {});
$weakReference = \WeakReference::create($handler);
unset($handler);
gc_collect_cycles();
var_dump($weakReference->get());
?>
--EXPECT--
NULL
