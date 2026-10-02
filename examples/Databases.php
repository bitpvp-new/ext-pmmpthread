<?php

declare(strict_types=1);

use pmmp\thread\Worker;
use pmmp\thread\Pool;
use pmmp\thread\Runnable;
use pmmp\thread\ThreadSafeArray;

/*
 * It's often desirable to interact with databases from multiple threads in an application.
 * Database connection objects (PDO, mysqli, etc.) cannot be shared between threads.
 *
 * An ideal solution is to create Workers with an independent database connection per Worker thread.
 *
 * Query results can be cleanly converted into a ThreadSafeArray using ThreadSafeArray::fromArray(),
 * eliminating the need to serialize/unserialize arrays!
 */

class PDOWorker extends Worker{
	private static \PDO $connection;

	public function __construct(
		private string $dsn
	){}

	public function run() : void{
		// Set up connection for tasks running on this Worker thread
		self::$connection = new PDO($this->dsn);
		self::$connection->exec("CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY AUTOINCREMENT, value TEXT)");
		self::$connection->exec("INSERT INTO test (value) VALUES ('Hello world!')");
	}

	public static function getConnection() : \PDO{ 
		return self::$connection; 
	}
}

class DBFetchTask extends Runnable{
	public ?ThreadSafeArray $result = null;

	public function run() : void{
		$pdo = PDOWorker::getConnection();
		$statement = $pdo->prepare("SELECT * FROM test");
		$statement->execute();

		$rows = $statement->fetchAll(PDO::FETCH_ASSOC);

		// ThreadSafeArray::fromArray converts the PDO result array directly into a thread-safe structure
		$this->result = ThreadSafeArray::fromArray($rows);
	}
}

/*
 * Start a pool of 4 workers with SQLite database connections
 */
$pool = new Pool(4, PDOWorker::class, ["sqlite:example.db"]);

/*
 * Submit tasks to the pool
 */
for($i = 0; $i < 10; $i++){
	$pool->submit(new DBFetchTask());
}

/*
 * Collect completed tasks
 */
while($pool->collect(function(Runnable $runnable) : bool{
	if($runnable instanceof DBFetchTask && $runnable->result !== null){
		var_dump((array)$runnable->result);
	}
	return true;
}) > 0){
	usleep(50000);
}

$pool->shutdown();
