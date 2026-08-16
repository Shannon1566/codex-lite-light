#!/usr/bin/env python3
"""Drive Codex Lite Light from the Codex terminal title spinner on Windows."""

from __future__ import annotations

import argparse
import ctypes
import logging
from logging.handlers import RotatingFileHandler
from pathlib import Path
import sys
import time
from ctypes import wintypes

import serial


BRAILLE_START = 0x2800
BRAILLE_END = 0x28FF
ERROR_ALREADY_EXISTS = 183

user32 = ctypes.WinDLL("user32", use_last_error=True)
kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
user32.EnumWindows.argtypes = [WNDENUMPROC, wintypes.LPARAM]
user32.EnumWindows.restype = wintypes.BOOL
user32.IsWindowVisible.argtypes = [wintypes.HWND]
user32.IsWindowVisible.restype = wintypes.BOOL
user32.GetWindowTextLengthW.argtypes = [wintypes.HWND]
user32.GetWindowTextLengthW.restype = ctypes.c_int
user32.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
user32.GetWindowTextW.restype = ctypes.c_int


def visible_window_titles() -> list[str]:
    titles: list[str] = []

    @WNDENUMPROC
    def callback(hwnd: int, _lparam: int) -> bool:
        if not user32.IsWindowVisible(hwnd):
            return True
        length = user32.GetWindowTextLengthW(hwnd)
        if length <= 0:
            return True
        buffer = ctypes.create_unicode_buffer(length + 1)
        user32.GetWindowTextW(hwnd, buffer, len(buffer))
        title = buffer.value.strip()
        if title:
            titles.append(title)
        return True

    if not user32.EnumWindows(callback, 0):
        raise ctypes.WinError(ctypes.get_last_error())
    return titles


def title_is_working(title: str, contains: str) -> bool:
    if not title:
        return False
    first = ord(title[0])
    return BRAILLE_START <= first <= BRAILLE_END and (
        not contains or contains.casefold() in title.casefold()
    )


def codex_is_working(contains: str) -> tuple[bool, str | None]:
    for title in visible_window_titles():
        if title_is_working(title, contains):
            return True, title
    return False, None


class LightSerial:
    def __init__(self, port: str) -> None:
        self.port = port
        self.connection: serial.Serial | None = None

    def close(self) -> None:
        if self.connection is not None:
            try:
                self.connection.close()
            finally:
                self.connection = None

    def send(self, command: str) -> bool:
        try:
            if self.connection is None or not self.connection.is_open:
                self.connection = serial.Serial(
                    self.port, 115200, timeout=0.2, write_timeout=1
                )
                time.sleep(0.15)
            self.connection.write(f"{command}\n".encode("ascii"))
            self.connection.flush()
            return True
        except (OSError, serial.SerialException) as error:
            logging.warning("Serial write failed on %s: %s", self.port, error)
            self.close()
            return False


def acquire_single_instance() -> int:
    handle = kernel32.CreateMutexW(None, False, "Local\\CodexLiteLightTitleWatcher")
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    if ctypes.get_last_error() == ERROR_ALREADY_EXISTS:
        kernel32.CloseHandle(handle)
        raise RuntimeError("Codex Lite Light watcher is already running")
    return handle


def configure_logging() -> None:
    log_path = Path(__file__).with_name("codex-light-watcher.log")
    handler = RotatingFileHandler(
        log_path, maxBytes=256_000, backupCount=2, encoding="utf-8"
    )
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
        handlers=[handler],
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="COM5")
    parser.add_argument("--poll-interval", type=float, default=0.1)
    parser.add_argument("--heartbeat", type=float, default=20.0)
    parser.add_argument(
        "--title-contains",
        default="",
        help="Only match spinner titles containing this text",
    )
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--once", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    configure_logging()

    if args.once:
        working, title = codex_is_working(args.title_contains)
        print("W" if working else "I")
        if title:
            print(title.encode("unicode_escape").decode("ascii"))
        return 0

    mutex_handle = acquire_single_instance()
    light = LightSerial(args.port)
    previous_working: bool | None = None
    last_send = 0.0
    logging.info("Watcher started on %s", args.port)

    try:
        while True:
            working, title = codex_is_working(args.title_contains)
            now = time.monotonic()
            changed = working != previous_working
            heartbeat_due = now - last_send >= args.heartbeat

            if changed or heartbeat_due:
                command = "W" if working else ("S" if previous_working else "I")
                if args.dry_run:
                    print(command, title or "", flush=True)
                    sent = True
                else:
                    sent = light.send(command)
                if sent:
                    last_send = now
                    logging.info("Sent %s; title=%r", command, title)
                previous_working = working

            time.sleep(max(args.poll_interval, 0.05))
    except KeyboardInterrupt:
        logging.info("Watcher stopped")
    finally:
        light.close()
        kernel32.CloseHandle(mutex_handle)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception:
        logging.exception("Watcher crashed")
        raise
