<?php

declare(strict_types=1);

use pocketmine\snooze\SleeperHandler;
use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;
use pocketmine\snooze\SleeperHandlerEntry;

/*
 * This example demonstrates an event-driven channel between multiple worker threads
 * and the main thread event loop using SleeperHandler and SleeperNotifier.
 *
 * Unlike traditional polling loops or mutex wait/notify, this architecture allows
 * the main thread to wait for events from multiple channels and timers simultaneously
 * with ZERO CPU usage and sub-millisecond reaction latency.
 */

class EventProducerThread extends Thread{
	public function __construct(
		private int $id,
		private SleeperHandlerEntry $entry,
		private ThreadSafeArray $queue
	){}

	public function run() : void{
		$notifier = $this->entry->createNotifier();

		for($i = 1; $i <= 5; $i++){
			usleep(random_int(10000, 50000)); // 10ms - 50ms work simulation

			// Push message to shared queue
			$this->queue[] = "Message $i from producer #{$this->id}";

			// Wake up main sleeper handler immediately
			$notifier->wakeupSleeper();
		}
	}
}

$sleeper = new SleeperHandler();
$queue = new ThreadSafeArray();
$messagesReceived = 0;
$totalExpected = 15;

// Register channel handler
$entry = $sleeper->addNotifier(function() use ($queue, &$messagesReceived) : void{
	while(($msg = $queue->shift()) !== null){
		$messagesReceived++;
		echo "[Main Thread] Processed: $msg (Total: $messagesReceived)\n";
	}
});

// Launch 3 concurrent producer threads
$threads = [];
for($id = 1; $id <= 3; $id++){
	$t = new EventProducerThread($id, $entry, $queue);
	$t->start(Thread::INHERIT_ALL);
	$threads[] = $t;
}

// Event loop: sleep until notified, processing events as they arrive
while($messagesReceived < $totalExpected){
	$sleeper->sleepUntilNotification();
}

foreach($threads as $t){
	$t->join();
}

$sleeper->removeNotifier($entry->getNotifierId());
echo "All $messagesReceived messages processed successfully!\n";
