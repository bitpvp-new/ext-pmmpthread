<?php

/**
 * @generate-class-entries
 */

namespace pocketmine\snooze;

use pmmp\thread\ThreadSafeArray;

/**
 * Used to wake up the sleeping thread from another thread.
 * Use SleeperHandlerEntry::createNotifier() inside the thread to create this.
 *
 * @since 6.4.0
 */
final class SleeperNotifier
{
    protected ThreadSafeArray $sharedObject;
    protected int $notifierId;

    public function __construct(ThreadSafeArray $sharedObject, int $notifierId) {}

    final public function wakeupSleeper() : void {}
}
