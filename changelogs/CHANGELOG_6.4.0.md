# Changelog 6.4.0

## Native Snooze Event-Driven Thread Notification Management & Core Optimizations

In this release, the functionality of the [pmmp/Snooze](https://github.com/pmmp/Snooze) library has been integrated directly into the `pmmpthread` C extension, accompanied by deep core engine performance optimizations and bug fixes across the entire extension.

---

### Core Extension Enhancements & Bug Fixes

1. **Custom Inheritance Options in `Pool::__construct`**:
   - `Pool::__construct(int $size, string $class = Worker::class, array $ctor = [], int $options = Thread::INHERIT_ALL)` now accepts custom thread inheritance options (such as `Thread::INHERIT_NONE` or `Thread::INHERIT_INI`), allowing worker pools to start with minimal memory footprint and fast initialization.

2. **Enum Hard-Copy Memory Safety (UAF Prevention)**:
   - Fixed a Use-After-Free bug when hard-copying `STORE_TYPE_ENUM` values (e.g. during `$array->merge()` or cloning) by properly duplicating `class_name` and `member_name` strings into persistent storage.

3. **OPcache Immutable Array Zero-Copy Optimization**:
   - `pmmpthread_copy_hash` now checks `IS_ARRAY_IMMUTABLE` and directly reuses immutable arrays located in OPcache shared memory (SHM), avoiding unnecessary `HashTable` allocations and copies during thread startup.

4. **Magic `__isset()` `ZEND_PROPERTY_NOT_EMPTY` Support**:
   - `pmmpthread_has_property` now accurately handles `empty()` checks (`ZEND_PROPERTY_NOT_EMPTY`) on userland classes implementing `__isset()`.

5. **Inlined Fast Monitor Primitives**:
   - `pmmpthread_monitor_lock`, `pmmpthread_monitor_unlock`, `pmmpthread_monitor_notify`, and `pmmpthread_monitor_notify_one` are now `static zend_always_inline` in `src/monitor.h`.
   - Eliminates function call overhead for millions of mutex operations across all threads and data structures.

6. **Recursive Lock Elimination in `wait_until`**:
   - `pmmpthread_monitor_wait_until` now checks state conditions directly under the active monitor lock without nested lock/unlock cycling, accelerating thread creation, task scheduling, and joins.

7. **High-Performance Batch Array Merging**:
   - `ThreadSafeArray::merge()` and `ThreadSafeArray::fromArray()` now utilize Zend Engine's direct `ZEND_HASH_FOREACH_KEY_VAL` fast-path iterator.
   - Batch merges execute under a single monitor lock acquisition rather than per-element mutex cycling, drastically reducing lock contention.

8. **Targeted Worker Wakeups (`notify_one`)**:
   - `pmmpthread_worker_add_task` now uses `pmmpthread_monitor_notify_one` instead of `pthread_cond_broadcast`, eliminating thundering-herd context switches when queuing tasks.

---

### Snooze Performance Optimizations

- **Direct C Struct Layouts**: `SleeperHandler` and `SleeperNotifier` use dedicated C struct object layouts (`pmmpthread_sleeper_handler_t` and `pmmpthread_sleeper_notifier_t`). Method calls like `wakeupSleeper()`, `sleepUntil()`, and `processNotifications()` access internal pointers directly in 0ns, completely bypassing Zend property hash table lookups.
- **Zero-Allocation Mutex Operations**: Notification signaling via `SleeperNotifier::wakeupSleeper()` and waiting via `SleeperHandler` are implemented in native C, eliminating userland PHP closure allocations and Zend VM opcode execution during synchronization.
- **Direct Monitor Wait & Signal**: Waiting operations use condition variables directly (`pthread_cond_timedwait` / Windows condition variables), avoiding unnecessary userland context switches.
- **O(1) Batch Notification Draining**: Pending notifier IDs are collected into stack buffers (fast path for up to 64 notifiers) and cleared under lock in O(1) time, then dispatched directly to registered PHP closures.
- **Dual Namespace Aliases**: Full native compatibility with both `\pocketmine\snooze\` and `\pmmp\thread\` namespaces without duplicate overhead.

---

### New Classes and Methods

#### `pocketmine\snooze\SleeperHandler` (aliased to `pmmp\thread\SleeperHandler`)
Event-loop notification coordinator on the main/sleeping thread.
- `public function __construct()`
- `public function addNotifier(\Closure $handler) : SleeperHandlerEntry`: Registers a handler closure to be called when the corresponding notifier signals.
- `public function removeNotifier(int $notifierId) : void`: Unregisters a notification handler by ID.
- `public function sleepUntil(float $unixTime) : void`: Sleeps until the specified Unix timestamp (float seconds), waking up early to process incoming notifications, and continuing sleep if time remains.
- `public function sleepUntilNotification() : void`: Blocks indefinitely until at least one notification arrives, then processes all pending notifications immediately.
- `public function processNotifications() : void`: Processes all currently queued notifications.

#### `pocketmine\snooze\SleeperHandlerEntry` (aliased to `pmmp\thread\SleeperHandlerEntry`)
Thread-safe entry passed to worker threads to create notifiers. Extends `pmmp\thread\ThreadSafe`.
- `public function __construct(ThreadSafeArray $sharedObject, int $id)`
- `final public function getNotifierId() : int`: Returns the integer identifier for this notifier entry.
- `public function createNotifier() : SleeperNotifier`: Instantiates a thread-local `SleeperNotifier` instance inside worker threads.

#### `pocketmine\snooze\SleeperNotifier` (aliased to `pmmp\thread\SleeperNotifier`)
Thread-local notifier instance used to signal the sleeper handler.
- `public function __construct(ThreadSafeArray $sharedObject, int $notifierId)`
- `final public function wakeupSleeper() : void`: Atomically signals the sleeper handler from any background thread.

---

### Example Usage

```php
use pocketmine\snooze\SleeperHandler;
use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

class WorkerThread extends Thread {
    public function __construct(
        private \pocketmine\snooze\SleeperHandlerEntry $entry,
        private ThreadSafeArray $buffer
    ) {}

    public function run() : void {
        $notifier = $this->entry->createNotifier();
        
        // Push work result to buffer and wake up main thread
        $this->buffer[] = "Task completed";
        $notifier->wakeupSleeper();
    }
}

$sleeper = new SleeperHandler();
$buffer = new ThreadSafeArray();

$entry = $sleeper->addNotifier(function() use ($buffer) : void {
    while (($msg = $buffer->shift()) !== null) {
        echo "Received from thread: $msg\n";
    }
});

$thread = new WorkerThread($entry, $buffer);
$thread->start(Thread::INHERIT_NONE);

// Sleep until notified
$sleeper->sleepUntilNotification();

$thread->join();
$sleeper->removeNotifier($entry->getNotifierId());
```
