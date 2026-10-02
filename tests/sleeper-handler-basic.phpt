--TEST--
Test SleeperHandler basic functionality
--DESCRIPTION--
This test verifies that SleeperHandler, SleeperHandlerEntry and SleeperNotifier can be created and managed
--FILE--
<?php

$sleeper = new \pocketmine\snooze\SleeperHandler();

$called = false;
$entry1 = $sleeper->addNotifier(function() use (&$called) : void{
    $called = true;
});

var_dump($entry1 instanceof \pocketmine\snooze\SleeperHandlerEntry);
var_dump($entry1 instanceof \pmmp\thread\ThreadSafe);
var_dump($entry1->getNotifierId());

$entry2 = $sleeper->addNotifier(function() : void{});
var_dump($entry2->getNotifierId());

$notifier = $entry1->createNotifier();
var_dump($notifier instanceof \pocketmine\snooze\SleeperNotifier);

$sleeper->removeNotifier($entry2->getNotifierId());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
int(0)
int(1)
bool(true)
OK
