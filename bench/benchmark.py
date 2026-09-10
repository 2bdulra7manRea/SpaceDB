#!/usr/bin/env python3
"""Small dependency-free throughput benchmark for the SpaceDB TCP protocol."""

import argparse
import socket
import time


def frame(*parts: str) -> bytes:
    return (f"*{len(parts)}\r\n" + "".join(f"${len(p)}\r\n{p}\r\n" for p in parts)).encode()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6379)
    parser.add_argument("--requests", type=int, default=10000)
    args = parser.parse_args()
    payload = frame("SET", "benchmark-key", "benchmark-value")
    started = time.perf_counter()
    with socket.create_connection((args.host, args.port)) as connection:
        for _ in range(args.requests):
            connection.sendall(payload)
            expected = b"+OK\r\n"
            received = b""
            while len(received) < len(expected):
                received += connection.recv(len(expected) - len(received))
            if received != expected:
                raise RuntimeError(f"unexpected response: {received!r}")
    elapsed = time.perf_counter() - started
    print(f"requests={args.requests} seconds={elapsed:.3f} ops/sec={args.requests / elapsed:.0f}")


if __name__ == "__main__":
    main()