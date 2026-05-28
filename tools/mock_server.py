import json
from http.server import HTTPServer, BaseHTTPRequestHandler

events_log = []

class EventHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)
        
        try:
            payload = json.loads(body)
            if 'timestamp' not in payload or 'voltage_kv' not in payload or 'event' not in payload:
                self.send_response(400)
                self.end_headers()
                return
                
            events_log.append(payload)
            
            self.send_response(201)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "logged", "id": len(events_log)}).encode())
        except json.JSONDecodeError:
            self.send_response(400)
            self.end_headers()
            
    def do_GET(self):
        if self.path == '/events':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(events_log).encode())
        elif self.path == '/health':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "ok"}).encode())
        else:
            self.send_response(404)
            self.end_headers()

if __name__ == "__main__":
    server_address = ('', 8080)
    httpd = HTTPServer(server_address, EventHandler)
    print("Mock server running on port 8080...")
    httpd.serve_forever()
