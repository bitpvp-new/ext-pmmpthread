<?php

declare(strict_types=1);

use pocketmine\snooze\SleeperHandler;
use pmmp\thread\Worker;
use pmmp\thread\Runnable;
use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;
use pocketmine\snooze\SleeperHandlerEntry;

/*
 * This example shows how to build a fully asynchronous, non-blocking Task Pool
 * using Workers and SleeperHandler.
 *
 * Background tasks execute in parallel on reusable Worker threads. When finished,
 * the worker notifies SleeperHandler, and the main thread resolves the promise callback
 * immediately in its event loop without blocking or busy-waiting.
 */

class AsyncTask extends Runnable{
	public function __construct(
		private int $taskId,
		private \Closure $closure,
		private ThreadSafeArray $args,
		private SleeperHandlerEntry $entry,
		private ThreadSafeArray $resultsQueue
	){}

	public function run() : void{
		$notifier = $this->entry->createNotifier();

		try{
			// Unpack ThreadSafeArray elements into standard arguments
			$unpackedArgs = [];
			foreach($this->args as $arg){
				$unpackedArgs[] = $arg;
			}

			// Execute the closure with arguments on background thread
			$result = ($this->closure)(...$unpackedArgs);
			$error = null;
		}catch(\Throwable $e){
			$result = null;
			$error = $e->getMessage() . " in " . $e->getFile() . ":" . $e->getLine();
		}

		// Record completion in shared results queue
		$entry = new ThreadSafeArray();
		$entry["id"] = $this->taskId;
		$entry["result"] = $result;
		$entry["error"] = $error;
		$this->resultsQueue[] = $entry;

		// Signal event loop on main thread via native C sleeper notifier
		$notifier->wakeupSleeper();
	}
}

class AsyncWorkerPool{
	/** @var Worker[] */
	private array $workers = [];
	private ThreadSafeArray $resultsQueue;
	private SleeperHandlerEntry $entry;
	private array $callbacks = [];
	private int $nextTaskId = 1;
	private int $workerIndex = 0;

	public function __construct(
		private SleeperHandler $sleeper,
		private int $workerCount = 4
	){
		$this->resultsQueue = new ThreadSafeArray();

		// Register the notification closure with SleeperHandler
		$this->entry = $this->sleeper->addNotifier(function() : void{
			echo "[SleeperHandler Event] addNotifier closure invoked on Main Thread!\n";
			while(($item = $this->resultsQueue->shift()) !== null){
				$id = $item["id"];
				$result = $item["result"];
				$error = $item["error"];
				if(isset($this->callbacks[$id])){
					echo " ├─ Found Task #$id in \$this->callbacks -> Executing registered callback...\n";
					$cb = $this->callbacks[$id];
					unset($this->callbacks[$id]);
					$cb($result, $error);
				}
			}
		});

		for($i = 0; $i < $this->workerCount; $i++){
			$worker = new Worker();
			/*
			 * In single-file scripts without an autoloader, INHERIT_ALL is needed so worker threads
			 * have access to classes (AsyncTask) and closures defined in this file.
			 * In applications with Composer autoloader, INHERIT_NONE is recommended.
			 */
			$worker->start(Thread::INHERIT_ALL);
			$this->workers[] = $worker;
		}
	}

	public function submit(\Closure $task, array $args, \Closure $onComplete) : int{
		$id = $this->nextTaskId++;
		$this->callbacks[$id] = $onComplete;

		$runnable = new AsyncTask(
			$id,
			$task,
			ThreadSafeArray::fromArray($args),
			$this->entry,
			$this->resultsQueue
		);

		$worker = $this->workers[$this->workerIndex % count($this->workers)];
		$this->workerIndex++;
		$worker->stack($runnable);

		return $id;
	}

	public function hasPendingTasks() : bool{
		return count($this->callbacks) > 0;
	}

	public function shutdown() : void{
		$this->sleeper->removeNotifier($this->entry->getNotifierId());
		foreach($this->workers as $worker){
			$worker->shutdown();
		}
	}
}

// ------------------- Main Event Loop Demo -------------------

$sleeper = new SleeperHandler();
$pool = new AsyncWorkerPool($sleeper, 4);

$completedCount = 0;

// Submit 10 asynchronous tasks
for($i = 1; $i <= 10; $i++){
	$pool->submit(
		static function(int $num) : string{
			usleep(random_int(10000, 30000)); // simulate async background work
			return "Result of task $num (computed on worker thread " . Thread::getCurrentThreadId() . ")";
		},
		[$i],
		function(?string $result, ?string $error) use (&$completedCount) : void{
			$completedCount++;
			if($error !== null){
				echo " └─ [Task Error] $error\n";
			}else{
				echo " └─ [User Callback Finished] $result\n";
			}
		}
	);
}

// Non-blocking tick loop: runs ticks at 20Hz (every 50ms) and processes async completions
while($pool->hasPendingTasks()){
	// Sleep until next tick or until a task finishes early
	$nextTick = microtime(true) + 0.05;
	$sleeper->sleepUntil($nextTick);
}

echo "\nAll $completedCount async tasks completed reactively without blocking!\n";
$pool->shutdown();
