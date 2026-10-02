--TEST--
Test SleeperHandler sleepUntil
--DESCRIPTION--
This test verifies that sleepUntil sleeps for the requested duration and processes notifications
--FILE--
<?php

class DelayedNotifierThread extends \pmmp\thread\Thread{
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry,
        private \pmmp\thread\ThreadSafeArray $buffer
    ){}

    public function run() : void{
        $notifier = $this->entry->createNotifier();
        usleep(20000); // 20ms
        $this->buffer[] = "notified";
        $notifier->wakeupSleeper();
    }
}

$sleeper = new \pocketmine\snooze\SleeperHandler();
$buffer = new \pmmp\thread\ThreadSafeArray();
$notified = false;

$entry = $sleeper->addNotifier(function() use ($buffer, &$notified) : void{
    if($buffer->shift() === "notified"){
        $notified = true;
    }
});

$thread = new DelayedNotifierThread($entry, $buffer);
$thread->start(\pmmp\thread\Thread::INHERIT_NONE);

$start = microtime(true);
// Sleep for at least 80ms, will be interrupted at ~20ms, processes notification, then continues sleeping until 80ms
$sleeper->sleepUntil($start + 0.08);
$elapsed = microtime(true) - $start;

$thread->join();

var_dump($notified);
var_dump($elapsed >= 0.07);
?>
--EXPECT--
bool(true)
bool(true)
