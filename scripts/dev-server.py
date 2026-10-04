#!/usr/bin/env python3

# NOTE: slop

import http.server
import subprocess
import sys
import threading
import time
from pathlib import Path
from urllib.parse import urlsplit


ROOT = Path(__file__).resolve().parent.parent
DIST = ROOT / "out" / "dist"
BUILD = sys.argv[1] if len(sys.argv) > 1 else "release"
version = 0


def files_stamp():
    files = []
    for directory in (ROOT / "content", ROOT / "page"):
        files += [p.stat().st_mtime_ns for p in directory.rglob("*") if p.is_file()]
    return tuple(files)


def rebuild():
    global version
    if subprocess.run(["make", "BUILD=" + BUILD, "dist"], cwd=ROOT).returncode == 0:
        version += 1


def watch():
    old_stamp = files_stamp()
    while True:
        time.sleep(0.25)
        new_stamp = files_stamp()
        if new_stamp != old_stamp:
            old_stamp = new_stamp
            rebuild()


RELOAD_SCRIPT = b"""
<script>
let devVersion;
setInterval(async () => {
  const current = await fetch('/__reload').then(response => response.text());
  if (devVersion !== undefined && current !== devVersion) location.reload();
  devVersion = current;
}, 500);
</script>
"""


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(DIST), **kwargs)

    def log_message(self, format, *args):
        if self.path.startswith("/__reload"):
            return
        super().log_message(format, *args)

    def do_GET(self):
        if urlsplit(self.path).path == "/__reload":
            body = str(version).encode()
            self.send_response(200)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-cache")
            self.end_headers()
            self.wfile.write(body)
            return

        if urlsplit(self.path).path.endswith(".html"):
            path = Path(self.translate_path(self.path))
            if path.is_file():
                body = path.read_bytes().replace(b"</body>", RELOAD_SCRIPT + b"</body>", 1)
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Cache-Control", "no-cache")
                self.end_headers()
                self.wfile.write(body)
                return

        super().do_GET()


if __name__ == "__main__":
    rebuild()
    threading.Thread(target=watch, daemon=True).start()
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 8000), Handler)
    print("serving http://127.0.0.1:8000")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
