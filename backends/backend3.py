from http.server import BaseHTTPRequestHandler, HTTPServer


class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"<h1>Response from Backend 2</h1>")


if __name__ == "__main__":
    server = HTTPServer(("localhost", 8083), MyHandler)
    print("Backend 3 running on port 8083")
    server.serve_forever()
