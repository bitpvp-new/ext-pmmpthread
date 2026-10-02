--TEST--
Test SleeperHandler sleepUntilNotification
--DESCRIPTION--
This test verifies that sleepUntilNotification blocks indefinitely until signaled
--FILE--
<?php

class FastSignalThread extends \pmmp\thread\Thread{
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry
    ){}

    public function run() : void{
        $notifier = $this->entry->createNotifier();
        usleep(30000); // 30ms
        $notifier->wakeupSleeper();
    }
}

$sleeper = new \pocketmine\snooze\SleeperHandler();
$woken = false;

$entry = $sleeper->addNotifier(function() use (&$woken) : void{
    $woken = true;
});

$thread = new FastSignalThread($entry);
$thread->start(\pmmp\thread\Thread::INHERIT_NONE);

$sleeper->sleepUntilNotification();

$thread->join();

var_dump($woken);
?>
--EXPECT--
bool(true)
