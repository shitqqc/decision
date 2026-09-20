#!/usr/bin/env python3
"""Read-only BT monitor: subscribe snapshot JSON, serve browser UI."""

from __future__ import annotations

import argparse
import json
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import rclpy
from ament_index_python.packages import get_package_share_directory
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from std_msgs.msg import String


def default_layout_path() -> Path:
    return Path.home() / ".cache" / "rm_decision_cpp" / "bt_monitor_layout.json"


class SnapshotBridge(Node):
    def __init__(self, topic: str) -> None:
        super().__init__("bt_monitor_web")
        self._lock = threading.Lock()
        self._latest: dict = {}
        self.create_subscription(String, topic, self._on_msg, 10)
        self.get_logger().info(f"Subscribing snapshot: {topic}")

    def _on_msg(self, msg: String) -> None:
        try:
            data = json.loads(msg.data)
        except json.JSONDecodeError as exc:
            self.get_logger().warn(f"bad snapshot json: {exc}")
            return
        with self._lock:
            self._latest = data

    def snapshot(self) -> dict:
        with self._lock:
            return dict(self._latest)


def make_handler(bridge: SnapshotBridge, web_root: Path, layout_path: Path):
    layout_lock = threading.Lock()

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, fmt: str, *args) -> None:  # noqa: A003
            return

        def do_GET(self) -> None:  # noqa: N802
            path = self.path.split("?", 1)[0]
            if path in ("/", "/index.html"):
                self._serve_file(web_root / "index.html", "text/html; charset=utf-8")
                return
            if path == "/api/snapshot":
                body = json.dumps(bridge.snapshot()).encode("utf-8")
                self._json_bytes(body)
                return
            if path == "/api/layout":
                data = {}
                with layout_lock:
                    if layout_path.is_file():
                        try:
                            data = json.loads(layout_path.read_text(encoding="utf-8"))
                        except (OSError, json.JSONDecodeError):
                            data = {}
                self._json_bytes(json.dumps(data).encode("utf-8"))
                return
            self.send_error(404)

        def do_PUT(self) -> None:  # noqa: N802
            path = self.path.split("?", 1)[0]
            if path != "/api/layout":
                self.send_error(404)
                return
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length > 0 else b"{}"
            try:
                payload = json.loads(raw.decode("utf-8"))
                if not isinstance(payload, dict):
                    raise ValueError("layout must be object")
            except (UnicodeDecodeError, json.JSONDecodeError, ValueError):
                self.send_error(400, "invalid json")
                return
            try:
                with layout_lock:
                    layout_path.parent.mkdir(parents=True, exist_ok=True)
                    layout_path.write_text(
                        json.dumps(payload, ensure_ascii=False, indent=2),
                        encoding="utf-8",
                    )
            except OSError:
                self.send_error(500, "write failed")
                return
            body = b'{"ok":true}'
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def _json_bytes(self, body: bytes) -> None:
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def _serve_file(self, file_path: Path, content_type: str) -> None:
            if not file_path.is_file():
                self.send_error(404)
                return
            data = file_path.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)

    return Handler


def main() -> None:
    parser = argparse.ArgumentParser(description="BT read-only web monitor")
    parser.add_argument("--topic", default="/nav/bt_snapshot")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument(
        "--layout-file",
        default=str(default_layout_path()),
        help="JSON file for persisted node layouts",
    )
    args, ros_args = parser.parse_known_args()

    rclpy.init(args=ros_args)
    bridge = SnapshotBridge(args.topic)

    share = Path(get_package_share_directory("rm_decision_cpp"))
    web_root = share / "web"
    if not (web_root / "index.html").is_file():
        web_root = Path(__file__).resolve().parent.parent / "web"

    layout_path = Path(args.layout_file).expanduser()
    handler = make_handler(bridge, web_root, layout_path)
    httpd = ThreadingHTTPServer((args.host, args.port), handler)
    http_thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    http_thread.start()
    bridge.get_logger().info(
        f"BT monitor UI: http://127.0.0.1:{args.port}/  (topic={args.topic})"
    )
    bridge.get_logger().info(f"Layout file: {layout_path}")

    try:
        rclpy.spin(bridge)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    finally:
        httpd.shutdown()
        bridge.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
