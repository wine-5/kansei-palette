"""Web ビルドを確認するためのローカルサーバー。

【Web の落とし穴: file:// では動かない】
index.html をダブルクリックで開くと、ブラウザのセキュリティ制限で index.wasm / index.data を
読み込めず起動しない。必ず HTTP サーバー経由で開くこと。

python -m http.server との違い:
  - キャッシュ無効ヘッダを付ける(再ビルドしたのに古い wasm が動く、を防ぐ)
  - ポートが使用中なら「起動済み」とみなして終了する(VS Code の F5 を連打しても失敗しない)

使い方: python tools/serve_web.py build-web/web 8080
"""

import functools
import http.server
import socket
import sys


class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, format, *args):
        pass  # アクセスログは出さない


def main() -> None:
    directory = sys.argv[1] if len(sys.argv) > 1 else "."
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 8080

    with socket.socket() as s:
        if s.connect_ex(("127.0.0.1", port)) == 0:
            print(f"Server already running: http://localhost:{port}/", flush=True)
            return

    handler = functools.partial(NoCacheHandler, directory=directory)
    with http.server.ThreadingHTTPServer(("127.0.0.1", port), handler) as httpd:
        print(f"Serving {directory} at http://localhost:{port}/", flush=True)
        httpd.serve_forever()


if __name__ == "__main__":
    main()
