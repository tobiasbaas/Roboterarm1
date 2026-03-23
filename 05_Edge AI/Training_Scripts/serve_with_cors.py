#!/usr/bin/env python3
"""Simple static file server with permissive CORS headers for Label Studio."""

from __future__ import annotations

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self) -> None:
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, HEAD, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "*")
        super().end_headers()

    def do_OPTIONS(self) -> None:
        self.send_response(204)
        self.end_headers()


def main() -> None:
    parser = argparse.ArgumentParser(description="Serve current directory with CORS enabled")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=3000)
    parser.add_argument(
        "--directory",
        default=str(Path(__file__).resolve().parent),
        help="Directory to serve (default: this script's folder)",
    )
    args = parser.parse_args()

    serve_dir = str(Path(args.directory).resolve())
    handler = partial(CORSRequestHandler, directory=serve_dir)

    with ThreadingHTTPServer((args.host, args.port), handler) as server:
        print(f"Serving with CORS on http://{args.host}:{args.port}")
        print(f"Serving directory: {serve_dir}")
        server.serve_forever()


if __name__ == "__main__":
    main()
