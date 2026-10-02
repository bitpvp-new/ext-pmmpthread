<?php

/**
 * @generate-class-entries
 */

namespace pocketmine\snooze;

use pmmp\thread\ThreadSafe;
use pmmp\thread\ThreadSafeArray;

/**
 * Represents an entry in a SleeperHandler. This is used to unregister the notifier when it is no longer
 * needed. It is also used to create the SleeperNotifier for the thread that needs to do wakeups.
 *
 * @since 6.4.0
 */
final class SleeperHandlerEntry extends ThreadSafe
{
    protected ThreadSafeArray $sharedObject;
    protected int $id;

    public function __construct(ThreadSafeArray $sharedObject, int $id) {}

    final public function getNotifierId() : int {}

    public function createNotifier() : SleeperNotifier {}
}
