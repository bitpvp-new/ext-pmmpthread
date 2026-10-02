--TEST--
Test SleeperHandler with Worker tasks
--DESCRIPTION--
This test verifies that tasks running on a Worker can wake up SleeperHandler
--FILE--
<?php

class SleeperTask extends \pmmp\thread\Runnable{
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry,
        private \pmmp\thread\ThreadSafeArray $buffer,
        private string $message
    ){}

    public function run() : void{
        $notifier = $this->entry->createNotifier();
        $this->buffer[] = $this->message;
        $notifier->wakeupSleeper();
    }
}

$sleeper = new \pocketmine\snooze\SleeperHandler();
$buffer = new \pmmp\thread\ThreadSafeArray();
$received = [];

$entry = $sleeper->addNotifier(function() use ($buffer, &$received) : void{
    while(($item = $buffer->shift()) !== null){
        $received[] = $item;
    }
});

$worker = new \pmmp\thread\Worker();
$worker->start(\pmmp\thread\Thread::INHERIT_NONE);

$worker->stack(new SleeperTask($entry, $buffer, "Task 1"));
$worker->stack(new SleeperTask($entry, $buffer, "Task 2"));

while(count($received) < 2){
    $sleeper->sleepUntilNotification();
}

$worker->shutdown();

var_dump($received);
?>
--EXPECT--
array(2) {
  [0]=>
  string(6) "Task 1"
  [1]=>
  string(6) "Task 2"
}
