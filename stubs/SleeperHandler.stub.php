<?php

/**
 * @generate-class-entries
 */

namespace pocketmine\snooze;

use pmmp\thread\ThreadSafeArray;

/**
 * Manages a Threaded sleeper which can be waited on for notifications. Calls callbacks for attached notifiers when
 * notifications are received from the notifiers.
 *
 * @since 6.4.0
 */
class SleeperHandler
{
    /**
     * @var ThreadSafeArray
     */
    protected ThreadSafeArray $sharedObject;

    /**
     * @var array
     */
    protected array $handlers;

    /**
     * @var int
     */
    protected int $nextSleeperId;

    public function __construct() {}

    /**
     * @param \Closure $handler Called when the notifier wakes the server up, of the signature `function() : void`
     */
    public function addNotifier(\Closure $handler) : SleeperHandlerEntry {}

    public function removeNotifier(int $notifierId) : void {}

    /**
     * Sleep until a specific time, waking up early to process notifications.
     */
    public function sleepUntil(float $unixTime) : void {}

    /**
     * Sleep indefinitely until a notification is received, then process it.
     */
    public function sleepUntilNotification() : void {}

    /**
     * Process any queued notifications immediately without sleeping.
     */
    public function processNotifications() : void {}
}
