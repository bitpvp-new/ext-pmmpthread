--TEST--
Test SleeperHandler notification from Thread
--DESCRIPTION--
This test verifies that a background Thread can wake up SleeperHandler via SleeperNotifier
--FILE--
<?php

class NotifierThread extends \pmmp\thread\Thread{
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry,
        private \pmmp\thread\ThreadSafeArray $buffer
    ){}

    public function run() : void{
        $notifier = $this->entry->createNotifier();
        $this->buffer[] = "Hello from thread";
        $notifier->wakeupSleeper();
    }
}

$sleeper = new \pocketmine\snooze\SleeperHandler();
$buffer = new \pmmp\thread\ThreadSafeArray();
$received = null;

$entry = $sleeper->addNotifier(function() use ($buffer, &$received) : void{
    $received = $buffer->shift();
});

$thread = new NotifierThread($entry, $buffer);
$thread->start(\pmmp\thread\Thread::INHERIT_NONE);

while($received === null){
    $sleeper->sleepUntilNotification();
}

$thread->join();
var_dump($received);
?>
--EXPECT--
string(17) "Hello from thread"
