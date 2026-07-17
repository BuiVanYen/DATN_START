#!/bin/bash

# HƯỚNG DẪN SETUP RASPBERRY PI 4 TỪ A-Z (CHO OS MỚI CÀI TRÊN SSD)
# Chạy từng nhóm lệnh dưới đây trong Terminal của Pi.

echo "=================================================="
echo "BẮT ĐẦU SETUP HỆ THỐNG RASPBERRY PI 4"
echo "=================================================="

# ------------------------------------------------
# PHẦN 1: CẬP NHẬT HỆ THỐNG & CÔNG CỤ CƠ BẢN
# ------------------------------------------------
echo "1. Cập nhật hệ thống..."
sudo apt update && sudo apt upgrade -y

echo "Cài đặt các công cụ tiện ích cần thiết..."
sudo apt install -y git curl htop build-essential python3-pip python3-venv

# ------------------------------------------------
# PHẦN 2: CÀI ĐẶT DOCKER & DOCKER COMPOSE
# (Dùng để chạy InfluxDB, FastAPI Backend, Mosquitto MQTT)
# ------------------------------------------------
echo "2. Tải và cài đặt Docker chính thức..."
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

echo "Cấp quyền chạy Docker không cần sudo cho user 'pi'..."
sudo usermod -aG docker pi

echo "Kích hoạt Docker khởi động cùng hệ thống..."
sudo systemctl enable docker
sudo systemctl start docker

# Kiểm tra phiên bản Docker & Docker Compose
echo "Phiên bản Docker hiện tại:"
docker --version
docker compose version

# ------------------------------------------------
# PHẦN 3: CÀI ĐẶT & CẤU HÌNH SAMBA (CHIA SẺ FILE)
# ------------------------------------------------
echo "3. Cài đặt dịch vụ Samba..."
sudo apt install samba -y

echo "Tạo thư mục dự án và cấp quyền..."
mkdir -p ~/DATN_START
chmod 777 ~/DATN_START

echo "Sao lưu cấu hình Samba cũ..."
sudo cp /etc/samba/smb.conf /etc/samba/smb.conf.bak

echo "Thêm cấu hình thư mục chia sẻ vào smb.conf..."
# Ghi cấu hình vào cuối file smb.conf nếu chưa có
sudo bash -c 'cat >> /etc/samba/smb.conf <<EOL

[DATN_START]
   path = /home/pi/DATN_START
   writeable = yes
   browseable = yes
   public = no
   create mask = 0777
   directory mask = 0777
EOL'

echo "Khởi động lại Samba để áp dụng cấu hình..."
sudo systemctl restart smbd
sudo systemctl restart nmbd

echo "=================================================="
echo "SETUP HOÀN TẤT!"
echo "BƯỚC CUỐI CÙNG:"
echo "Hãy chạy lệnh sau để đặt mật khẩu đăng nhập Samba cho user 'pi':"
echo "👉  sudo smbpasswd -a pi"
echo "Sau đó khởi động lại Pi bằng lệnh: sudo reboot"
echo "=================================================="
