# 🔌 HƯỚNG DẪN KẾT NỐI VÀ NẠP CODE ESP32-S3 (DATN-VER2)

Tài liệu này hướng dẫn chi tiết cách kết nối phần cứng và thực hiện nạp code (flash) cho vi điều khiển **ESP32-S3-WROOM-1-N8R8** trên thiết kế bo mạch **DATN-VER2**. 

Do thiết kế bo mạch này tối giản chi phí và chống nhiễu nên **không tích hợp chip nạp USB-to-UART (như CH340, CP2102) và cổng USB-C nạp trực tiếp**. Thay vào đó, bạn sẽ nạp code thông qua mạch nạp USB-to-UART ngoài kết nối vào header UART mặc định của chip.

---

## 1. Chuẩn Bị Thiết Bị Phần Cứng

1.  **Mạch nạp USB-to-UART ngoài**: Sử dụng các module phổ biến như:
    *   CP2102 USB-to-UART Bridge
    *   FT232RL FTDI
    *   CH340G / CH340E
2.  **Dây cắm testboard (Dây bus cái - cái)** để nối giữa mạch nạp và header `J1` trên PCB.
3.  **Nguồn cấp chính cho bo mạch**: Nguồn tổ ong 12V cấp vào mạch để đảm bảo nguồn điện ổn định.

---

## 2. Sơ Đồ Đấu Nối Dây (Wiring Connection)

Tìm Header **J1** (HEADER_4 PIN) và các chân tương ứng trên mạch nạp USB-to-UART. Đấu nối theo sơ đồ sau:

| Chân trên Header J1 (PCB) | Chân trên Mạch nạp USB-to-UART | Ghi chú |
| :--- | :--- | :--- |
| **GND** | **GND** | Nối chung đất (Bắt buộc) |
| **TX0** (chân TXD0 - GPIO43) | **RXD** | Chân truyền tín hiệu của ESP32 sang chân nhận mạch nạp |
| **RX0** (chân RXD0 - GPIO44) | **TXD** | Chân nhận tín hiệu của ESP32 từ chân truyền mạch nạp |
| **5V** | **KHÔNG NỐI** (Nếu cấp nguồn 12V riêng) | **QUAN TRỌNG:** Xem lưu ý phân phối nguồn bên dưới. |

> [!WARNING]
> **Lưu ý về nguồn cấp (Power Conflict):**
> *   **KHUYÊN DÙNG:** Cấp nguồn **12V chính** vào PCB qua Jack DC. Khi đó, **tuyệt đối không kết nối chân 5V của mạch nạp USB-to-UART** vào header J1 để tránh xung đột nguồn làm hỏng mạch nạp hoặc gây chết chip ESP32.
> *   Chỉ kết nối chân **5V** của mạch nạp vào J1 khi bạn **không cắm nguồn 12V** cho board và mạch nạp của bạn có khả năng chịu dòng tải lớn (ESP32-S3 có thể ăn dòng đỉnh lên tới 500mA khi flash hoặc phát WiFi, mạch nạp chất lượng kém sẽ bị sụt áp gây lỗi nạp).

---

## 3. Quy Trình Đưa ESP32-S3 Vào Chế Độ Nạp Code (Manual Bootloader Mode)

Vì bo mạch không có mạch tự động reset (không dùng chân DTR/RTS để tự động kéo EN và GPIO0 xuống GND), bạn phải đưa chip vào chế độ nạp **bằng tay** theo các bước sau:

1.  **Xác định các chân điều khiển trên mạch**:
    *   Header **H1**: Nối với chân `BOOT` (GPIO0) và `GND`.
    *   Header **H2**: Nối với chân `EN` (Reset) và `GND`.
    *(Nếu trên mạch đã thiết kế nút nhấn RESET và BOOT thì bạn chỉ cần bấm nút. Nếu chỉ thiết kế chân cắm Header cắm jump, bạn hãy sử dụng Jumper hoặc nhíp sắt/dây bus để nối tắt tạm thời).*

2.  **Các bước thao tác nút nhấn / jumper**:
    *   **Bước 1:** Nhấn và giữ nút **BOOT** (hoặc cắm Jumper chập chân **H1** lại).
    *   **Bước 2:** Nhấn nút **RESET/EN** (hoặc chạm chập chân **H2** rồi thả ra).
    *   **Bước 3:** Thả nút **BOOT** ra (hoặc rút Jumper ở **H1** ra).
    
    > [!NOTE]
    > Một khi bạn thực hiện đúng trình tự, ESP32-S3 sẽ chuyển sang chế độ nạp (**Download Mode / Bootloader**) để sẵn sàng nhận lệnh nạp code từ máy tính.

---

## 4. Cấu Hình Phần Mềm & Thực Hiện Nạp Code

Dự án hiện tại của bạn đang sử dụng **PlatformIO** với cấu hình board: `freenove_esp32_s3_wroom`.

### Cách 1: Nạp bằng PlatformIO (VS Code)

1.  Cắm mạch nạp USB-to-UART vào máy tính.
2.  Mở VS Code, truy cập vào thư mục dự án test: `Test_phan_cung/test`.
3.  Mở file `platformio.ini` để kiểm tra cấu hình. (Nếu máy tính có nhiều cổng COM, bạn có thể chỉ định cổng COM nạp cụ thể bằng cách thêm dòng `upload_port = COMx` vào dưới `[env:freenove_esp32_s3_wroom]`).
4.  Thực hiện tổ hợp phím tắt hoặc thao tác thủ công đưa ESP32 vào **Download Mode** như hướng dẫn ở **Mục 3**.
5.  Click vào biểu tượng **Upload (mũi tên chỉ sang phải)** ở thanh công cụ phía dưới cùng của VS Code (hoặc tab PlatformIO).
6.  PlatformIO sẽ tiến hành biên dịch code và nạp qua cổng COM. Khi màn hình console hiện thông báo `[SUCCESS]` hoặc các dòng báo phần trăm nạp thành công:
    ```text
    Writing at 0x00010000... (100 %)
    Hash of data verified.
    Leaving...
    Hard resetting via RTS pin...
    ```
7.  **Khởi động lại chip:** Do không có mạch auto-reset, sau khi nạp xong chip vẫn ở chế độ chờ. Bạn cần **nhấn nút RESET/EN** (chạm chập chân **H2** rồi thả ra) một lần để ESP32 bắt đầu chạy chương trình mới.

### Cách 2: Nạp bằng phần mềm Esptool.py (Command Line)
Nếu bạn chỉ có file `.bin` biên dịch sẵn và muốn nạp nhanh:
```powershell
python -m esptool --chip esp32s3 --port COMx --baud 460800 write_flash -z 0x10000 firmware.bin
```
*(Thay thế `COMx` bằng cổng COM tương ứng trên máy tính của bạn và thao tác đưa chip vào Download Mode trước khi chạy lệnh)*.

---

## 5. Các Lỗi Thường Gặp Và Cách Khắc Phục (Troubleshooting)

| Hiện Tượng | Nguyên Nhân | Giải Pháp |
| :--- | :--- | :--- |
| `A fatal error occurred: Failed to connect to ESP32-S3: No serial data received.` | Chưa đưa ESP32 vào chế độ Bootloader thành công hoặc nhầm dây TX/RX. | 1. Đảo ngược dây `TX0` và `RX0` (TX nạp nối RX mạch, RX nạp nối TX mạch).<br>2. Kiểm tra nguồn cấp (đèn LED PWR 3.3V phải sáng).<br>3. Thao tác lại trình tự nhấn nút BOOT và RESET thật chuẩn xác. |
| Mạch báo `espcomm_send_command: didn't receive acknowledgement!` | Tốc độ Baud nạp quá cao hoặc nhiễu đường truyền dây. | Hạ tốc độ baud trong `platformio.ini` bằng cách thêm: `upload_speed = 115200`. |
| Nạp thành công `100%` nhưng chương trình không chạy, Serial Monitor không in ra gì. | Chip chưa được khởi động lại sau khi nạp. | Nhấn nút **RESET (EN)** (H2) một lần để khởi động lại chip chạy chế độ thường. |
