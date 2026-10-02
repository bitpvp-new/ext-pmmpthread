--TEST--
Test SleeperHandler with multiple notifiers and threads
--DESCRIPTION--
This test verifies that multiple background threads can wake up SleeperHandler concurrently
--FILE--
<?php

class WorkerThread extends \pmmp\thread\Thread{
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry,
        private \pmmp\thread\ThreadSafeArray $buffer,
        private int $threadNum
    ){}

    public function run() : void{
        $notifier = $this->entry->createNotifier();
        usleep(10000 * $this->threadNum);
        $this->buffer[] = $this->threadNum;
        $notifier->wakeupSleeper();
    }
}

$sleeper = new \pocketmine\snooze\SleeperHandler();
$buffer = new \pmmp\thread\ThreadSafeArray();
$results = [];

$threads = [];
for($i = 0; $i < 3; ++$i){
    $entry = $sleeper->addNotifier(function() use ($buffer, &$results) : void{
        while(($item = $buffer->shift()) !== null){
            $results[] = $item;
        }
    });

    $threads[] = new WorkerThread($entry, $buffer, $i);
}

foreach($threads as $thread){
    $thread->start(\pmmp\thread\Thread::INHERIT_NONE);
}

while(count($results) < 3){
    $sleeper->sleepUntilNotification();
}

foreach($threads as $thread){
    $thread->join();
}

sort($results);
var_dump($results);
?>
--EXPECT--
array(3) {
  [0]=>
  int(0)
  [1]=>
  int(1)
  [2]=>
  int(2)
}
