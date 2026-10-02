<?php

declare(strict_types=1);

use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

/**
 * Thread-based HTTP server example supporting Connection: keep-alive and thread-safe session handling.
 * 
 * To run this example:
 *     $ php examples/KeepAliveSession.php 8822
 */
class HttpServerThread extends Thread
{
    public function __construct(
        protected \Socket $socket,
        protected ThreadSafeArray $sessions
    ) {}

    public function http_parse_headers(string $header) : array
    {
        $retVal = [];
        $fields = explode("\r\n", preg_replace('/\x0D\x0A[\x09\x20]+/', ' ', $header) ?? '');
        foreach ($fields as $field) {
            if (preg_match('/([^:]+): (.+)/m', $field, $match)) {
                $key = preg_replace_callback('/(?<=^|[\x09\x20\x2D])./', fn($m) => strtoupper($m[0]), strtolower(trim($match[1])));
                if (isset($retVal[$key])) {
                    if (!is_array($retVal[$key])) {
                        $retVal[$key] = [$retVal[$key]];
                    }
                    $retVal[$key][] = $match[2];
                } else {
                    $retVal[$key] = trim($match[2]);
                }
            }
        }
        return $retVal;
    }

    public function parse_cookies(array $headers) : array
    {
        $cookies = [];
        if (isset($headers['Cookie'])) {
            $cookieHeaders = is_array($headers['Cookie']) ? $headers['Cookie'] : [$headers['Cookie']];
            foreach ($cookieHeaders as $headerLine) {
                $parts = explode(';', $headerLine);
                foreach ($parts as $part) {
                    $cookie = explode('=', trim($part), 2);
                    if (count($cookie) === 2) {
                        $cookies[trim($cookie[0])] = trim($cookie[1]);
                    }
                }
            }
        }
        return $cookies;
    }

    public function run() : void
    {
        $threadId = $this->getThreadId();
        $counter = 1;
        $timeout = 5;
        $maxRequests = 5;

        $client = socket_accept($this->socket);
        if ($client === false) {
            return;
        }

        socket_set_option($client, SOL_SOCKET, SO_RCVTIMEO, ["sec" => $timeout, "usec" => 0]);

        do {
            $buffer = '';
            while (($chunk = socket_read($client, 1024)) !== false && $chunk !== '') {
                $buffer .= $chunk;
                if (str_contains($buffer, "\r\n\r\n")) {
                    break;
                }
            }

            if ($buffer === '') {
                socket_close($client);
                break;
            }

            $requestHeaders = $this->http_parse_headers($buffer);
            $cookies = $this->parse_cookies($requestHeaders);

            // Thread-safe session lookup / creation
            $sessionId = $cookies['PHPSESSID'] ?? null;
            if ($sessionId === null || !isset($this->sessions[$sessionId])) {
                $sessionId = bin2hex(random_bytes(16));
                $sessionData = new ThreadSafeArray();
                $sessionData['created'] = date('Y-m-d H:i:s');
                $sessionData['hits'] = 1;
                $this->sessions[$sessionId] = $sessionData;
            } else {
                $sessionData = $this->sessions[$sessionId] ??= new ThreadSafeArray();
                $sessionData['hits'] = ($sessionData['hits'] ?? 0) + 1;
            }

            // calculate available requests for keep-alive
            $availableRequests = $maxRequests - $counter++;

            $sessionData["thread_$threadId"] = "Handled $availableRequests requests";

            $body = "<html><head><title>pmmpthread HTTP</title></head><body>" .
                    "<h1>pmmpthread HTTP Server</h1>" .
                    "<p>Handled by Thread ID: <strong>$threadId</strong></p>" .
                    "<p>Session ID: <code>$sessionId</code></p>" .
                    "<p>Session Hits: <strong>" . $sessionData['hits'] . "</strong></p>" .
                    "<p>Session Data:</p>" .
                    "<pre>" . htmlspecialchars(var_export((array)$sessionData, true)) . "</pre>" .
                    "</body></html>";

            $headers = [
                "HTTP/1.1 200 OK",
                "Content-Type: text/html; charset=utf-8",
                "Set-Cookie: PHPSESSID=$sessionId; Path=/; HttpOnly",
                "Content-Length: " . strlen($body),
            ];

            if ($availableRequests > 0) {
                $headers[] = "Connection: keep-alive";
                $headers[] = "Keep-Alive: max=$availableRequests, timeout=$timeout, thread=$threadId";
            } else {
                $headers[] = "Connection: close";
            }

            $response = implode("\r\n", $headers) . "\r\n\r\n" . $body;
            socket_write($client, $response);

            if ($availableRequests <= 0) {
                socket_close($client);
                break;
            }
        } while (true);
    }
}

if ($argc < 2) {
    fwrite(STDERR, "Usage: php " . $argv[0] . " <port>\n");
    exit(1);
}

$port = (int)$argv[1];
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false || !socket_bind($socket, '0.0.0.0', $port) || !socket_listen($socket)) {
    throw new \RuntimeException("Failed to bind socket on port $port");
}

echo "Listening on port $port with 5 worker threads...\n";

$sessions = new ThreadSafeArray();
$workers = [];
for ($i = 0; $i < 5; $i++) {
    $worker = new HttpServerThread($socket, $sessions);
    $worker->start(Thread::INHERIT_ALL);
    $workers[] = $worker;
}

foreach ($workers as $worker) {
    $worker->join();
}
