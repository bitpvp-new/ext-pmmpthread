--TEST--
Test pool options parameter
--DESCRIPTION--
This test verifies that Pool can be created with custom inheritance options
--FILE--
<?php

$pool1 = new \pmmp\thread\Pool(2, \pmmp\thread\Worker::class, [], \pmmp\thread\Thread::INHERIT_NONE);
$pool2 = new \pmmp\thread\Pool(2, \pmmp\thread\Worker::class, [], \pmmp\thread\Thread::INHERIT_ALL);

class TestTask extends \pmmp\thread\Runnable{
    public function run() : void{}
}

$pool1->submit(new TestTask());
$pool2->submit(new TestTask());

$pool1->shutdown();
$pool2->shutdown();

echo "OK\n";
?>
--EXPECT--
OK
