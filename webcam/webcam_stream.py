import cv2
from http.server import BaseHTTPRequestHandler, HTTPServer
import socketserver
import time

class StreamingHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/video_feed':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.end_headers()
            
            # Mở camera USB (thường là 0 hoặc 1)
            camera = cv2.VideoCapture(1)
            if not camera.isOpened():
                print("Không thể mở Camera.")
                return
                
            try:
                while True:
                    success, frame = camera.read()
                    if not success:
                        break
                    
                    # Thu nhỏ kích thước ảnh để stream mượt (640x480)
                    frame = cv2.resize(frame, (640, 480))
                    ret, jpeg = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
                    if not ret:
                        continue
                        
                    frame_bytes = jpeg.tobytes()
                    self.wfile.write(b'--frame\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(frame_bytes)))
                    self.end_headers()
                    self.wfile.write(frame_bytes)
                    self.wfile.write(b'\r\n')
                    
                    # Nghỉ 0.04 giây để đạt khoảng 25 FPS (tránh quá tải CPU)
                    time.sleep(0.04) 
            except Exception as e:
                print(f"Lỗi khi stream: {e}")
            finally:
                camera.release()
                print("Đã giải phóng camera.")
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')

class StreamingServer(socketserver.ThreadingMixIn, HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

if __name__ == '__main__':
    server_address = ('', 8000)
    try:
        server = StreamingServer(server_address, StreamingHandler)
        print("Server đang chạy tại http://localhost:8000/video_feed (dùng Pure Python)")
        server.serve_forever()
    except KeyboardInterrupt:
        print("Đang dừng server...")
