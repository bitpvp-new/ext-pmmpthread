--TEST--
Test Sleeper classes namespace aliases
--DESCRIPTION--
This test verifies that pocketmine\snooze and pmmp\thread namespaces can both be used for Sleeper classes
--FILE--
<?php

$sleeper1 = new \pocketmine\snooze\SleeperHandler();
$sleeper2 = new \pmmp\thread\SleeperHandler();

var_dump($sleeper1 instanceof \pmmp\thread\SleeperHandler);
var_dump($sleeper2 instanceof \pocketmine\snooze\SleeperHandler);

$entry1 = $sleeper1->addNotifier(function() : void{});
var_dump($entry1 instanceof \pmmp\thread\SleeperHandlerEntry);

$notifier1 = $entry1->createNotifier();
var_dump($notifier1 instanceof \pmmp\thread\SleeperNotifier);

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
OK
