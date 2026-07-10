# HƯỚNG DẪN TỪ A-Z: TEST MÔ PHỎNG 4 QUẠT TRÊN SIEMENS NX MCD VỚI OPC UA
*(Dành cho người mới bắt đầu - Không cần phần cứng)*

Tài liệu này hướng dẫn bạn từng bước một để chạy thử nghiệm (Simulation Test) điều khiển **4 quạt thông gió** trên mô hình 3D của Siemens NX MCD thông qua OPC UA Server giả lập bằng Python.

---

## BẮT ĐẦU: HIỂU NGUYÊN LÝ HOẠT ĐỘNG GIAO TIẾP
* Bạn sẽ chạy một **OPC UA Server ảo** bằng Python đóng vai trò làm trung gian truyền tín hiệu.
* Bạn dùng một **Script Test** bằng Python (đóng vai trò như Web App điều khiển) để tăng/giảm tốc độ quạt từ 0 đến 255.
* Mô hình 3D trên **Siemens NX MCD** sẽ kết nối vào server này, đọc tốc độ quạt và làm cánh quạt ảo xoay tít theo đúng tốc độ bạn nhập.

```
[Script Test (test_fans.py)]  --> Ghi tốc độ (0-255) -->  [OPC UA Server (opcua_simulator.py)]  -->  [Siemens NX MCD 3D] (Quạt xoay)
```

---

## PHẦN 1: KHỞI ĐỘNG VÀ TEST THỬ SERVER OPC UA TRÊN MÁY TÍNH

Bước này giúp bạn cài đặt môi trường Python và kiểm tra xem Server OPC UA đã hoạt động tốt hay chưa trước khi cấu hình trên Siemens NX.

### Bước 1.1: Mở Command Prompt (CMD) hoặc PowerShell
1. Nhấn phím **Windows** trên bàn phím.
2. Gõ từ khóa `cmd` (hoặc `powershell`).
3. Nhấp chọn **Command Prompt** (hoặc Windows PowerShell).

### Bước 1.2: Cài đặt thư viện kết nối OPC UA
Chạy lệnh sau trong cửa sổ CMD và nhấn **Enter**:
```bash
pip install asyncua
```
*(Nếu hệ thống báo lỗi lệnh `pip` không tìm thấy, hãy thử chạy lệnh: `python -m pip install asyncua`)*

### Bước 1.3: Khởi chạy OPC UA Server giả lập
1. Di chuyển thư mục làm việc của CMD tới thư mục dự án của bạn bằng lệnh:
   ```cmd
   cd /d "c:\Users\LENOVO\Desktop\DATN_START"
   ```
2. Chạy file giả lập bằng lệnh:
   ```bash
   python opcua_simulator.py
   ```
3. Nếu màn hình hiện dòng chữ sau thì Server đã khởi chạy thành công:
   ```text
   INFO:OPCUA_Simulator:Starting OPC UA Server...
   INFO:OPCUA_Simulator:OPC UA Server is running at: opc.tcp://127.0.0.1:4840
   ```
   > **LƯU Ý QUAN TRỌNG:** KHÔNG ĐƯỢC TẮT cửa sổ CMD này. Hãy thu nhỏ nó xuống thanh Taskbar để Server tiếp tục chạy ngầm.

### Bước 1.4: Chạy Script Test điều khiển quạt
Để chắc chắn mọi thứ thông suốt, chúng ta sẽ chạy thử script điều khiển quạt:
1. Mở thêm **một cửa sổ CMD mới** (Terminal thứ hai).
2. Di chuyển vào thư mục dự án:
   ```cmd
   cd /d "c:\Users\LENOVO\Desktop\DATN_START"
   ```
3. Chạy lệnh:
   ```bash
   python test_fans.py
   ```
4. Bạn sẽ thấy một menu hiện ra:
   ```text
   --- MENU ĐIỀU KHIỂN 4 QUẠT ---
   1. Đặt tốc độ Quạt 1
   2. Đặt tốc độ Quạt 2
   3. Đặt tốc độ Quạt 3
   4. Đặt tốc độ Quạt 4
   5. Bật tất cả quạt (Tốc độ tối đa 255)
   6. Tắt tất cả quạt (Tốc độ 0)
   7. Thoát chương trình
   Chọn chức năng (1-7):
   ```
5. Nhập số `5` rồi bấm **Enter**. Bạn sẽ thấy thông báo: *"Đã bật tất cả 4 quạt lên tốc độ tối đa (255)"*. Nhìn lại cửa sổ CMD chạy Server ở Bước 1.3, bạn sẽ thấy log thông báo giá trị các quạt được thay đổi.
6. Giữ nguyên cả 2 cửa sổ CMD này hoạt động và chuyển sang thiết lập trên Siemens NX.

---

## PHẦN 2: THIẾT LẬP MÔ HÌNH 3D TRONG SIEMENS NX MCD

Chúng ta cần định nghĩa cho mô hình 3D biết đâu là cánh quạt, cánh quạt xoay quanh trục nào và nhận lệnh điều khiển ra sao.

### Bước 2.1: Chuyển sang môi trường MCD
1. Mở file lắp ráp 3D (Assembly) chứa cụm quạt của bạn trong **Siemens NX**.
2. Trên thanh Ribbon trên cùng, chọn tab **Application** (Ứng dụng).
3. Click chọn biểu tượng **Mechatronics Concept Designer** (MCD). 
   *Bạn sẽ thấy các công cụ mô phỏng động học xuất hiện bên trái màn hình (Physics Navigator) và trên thanh menu.*

### Bước 2.2: Định nghĩa Vật thể cứng (Rigid Body)
Vật thể cứng là vật thể có khối lượng và chịu tác động vật lý (chuyển động quay, tịnh tiến...).
1. Trên menu MCD, nhấn vào công cụ **Rigid Body**.
2. Chọn **cánh quạt thứ 1** (chỉ chọn phần cánh sẽ quay, không chọn phần vỏ bọc bên ngoài).
3. Trong phần thuộc tính:
   - **Name**: Nhập `RigidBody_Fan_1`.
4. Nhấn **Apply** (để tiếp tục tạo các quạt khác mà không đóng cửa sổ).
5. Click chọn **cánh quạt thứ 2** -> Đặt tên `RigidBody_Fan_2` -> Nhấn **Apply**.
6. Click chọn **cánh quạt thứ 3** -> Đặt tên `RigidBody_Fan_3` -> Nhấn **Apply**.
7. Click chọn **cánh quạt thứ 4** -> Đặt tên `RigidBody_Fan_4` -> Nhấn **OK**.

### Bước 2.3: Tạo Khớp xoay (Hinged Joint)
Khớp xoay giúp xác định trục xoay của cánh quạt.
1. Nhấn chọn công cụ **Hinged Joint** trên thanh công cụ MCD.
2. Cấu hình các mục như sau:
   - **Attachment**: Chọn `RigidBody_Fan_1` (Vật thể cứng quạt 1 vừa tạo ở bước 2.2).
   - **Base**: Bỏ trống (không chọn gì).
   - **Position & Axis (Vị trí & Trục xoay)**:
     - Chọn điểm tâm (Center) của vòng tròn trục cánh quạt.
     - Chọn hướng trục: Click vào mũi tên định hướng sao cho trục quay trùng với trục hình học của quạt (hướng dọc theo trục quay của cánh quạt).
   - **Name**: Nhập `Joint_Fan_1`.
3. Nhấn **Apply**.
4. Thực hiện tương tự cho 3 quạt còn lại:
   - Quạt 2: chọn `RigidBody_Fan_2`, đặt trục tâm quạt 2, đặt tên `Joint_Fan_2`. Nhấn **Apply**.
   - Quạt 3: chọn `RigidBody_Fan_3`, đặt trục tâm quạt 3, đặt tên `Joint_Fan_3`. Nhấn **Apply**.
   - Quạt 4: chọn `RigidBody_Fan_4`, đặt trục tâm quạt 4, đặt tên `Joint_Fan_4`. Nhấn **OK**.

### Bước 2.4: Tạo Bộ điều khiển tốc độ (Speed Control)
Bộ điều khiển tốc độ sẽ áp đặt vận tốc quay (vòng/phút hoặc độ/giây) lên khớp xoay.
1. Nhấn chọn công cụ **Speed Control** trên thanh công cụ MCD.
2. Cấu hình:
   - **Joint**: Click chọn khớp xoay `Joint_Fan_1`.
   - **Name**: Đặt tên mặc định hoặc đặt tên dễ nhớ là `SpeedControl_Fan_1`.
3. Nhấn **Apply**.
4. Thực hiện tương tự cho 3 quạt còn lại để tạo ra `SpeedControl_Fan_2`, `SpeedControl_Fan_3`, `SpeedControl_Fan_4` tương ứng gắn vào các khớp xoay tương ứng. Nhấn **OK** khi hoàn thành.

### Bước 2.5: Tạo Tín hiệu điều khiển trong MCD (Signal)
Tín hiệu này sẽ là biến trung gian nhận giá trị từ bên ngoài (OPC UA) và gán vào bộ điều khiển tốc độ.
1. Nhấn chọn công cụ **Signal** (biểu tượng sóng tín hiệu).
2. Thiết lập tín hiệu cho Quạt 1:
   - **Name**: Đặt tên là `mcd_fan_1`.
   - **Data Type**: Chọn `Double` (hoặc `Real`).
   - **Class**: Chọn `Input` (Vì nhận tín hiệu điều khiển từ ngoài vào).
   - **Formula / Destination**: 
     - Nhấp chọn vào bộ điều khiển tốc độ `SpeedControl_Fan_1` trong danh sách đối tượng.
     - Chọn thuộc tính là **Speed** (hoặc Target Speed).
3. Nhấn **Apply**.
4. Thực hiện tương tự cho 3 quạt còn lại:
   - Tạo tín hiệu `mcd_fan_2` -> Gán vào `SpeedControl_Fan_2` (Speed).
   - Tạo tín hiệu `mcd_fan_3` -> Gán vào `SpeedControl_Fan_3` (Speed).
   - Tạo tín hiệu `mcd_fan_4` -> Gán vào `SpeedControl_Fan_4` (Speed).
5. Nhấn **OK** để kết thúc.

---

## PHẦN 3: KẾT NỐI SIEMENS NX MCD VỚI OPC UA SERVER

Bây giờ, chúng ta sẽ liên kết (Map) tín hiệu ảo từ Python OPC UA Server vào các tín hiệu MCD vừa tạo.

### Bước 3.1: Thêm cấu hình kết nối ngoài
1. Trên thanh công cụ MCD, tìm và nhấn vào nút **External Signal Configuration** (Cấu hình tín hiệu ngoài).
2. Một hộp thoại hiện ra, nhấn nút **Add Connection** (hoặc dấu cộng màu xanh).
3. Điền các thông số:
   - **Connection Type**: Chọn **OPC UA**.
   - **Connection Name**: Đặt tên bất kỳ (Ví dụ: `Python_OPCUA`).
   - **Server URL**: Nhập chính xác địa chỉ:
     ```text
     opc.tcp://127.0.0.1:4840
     ```
4. Nhấn nút **Connect** (Kết nối).
   *Nếu kết nối thành công, danh sách các Node cảm biến và thiết bị từ Server Python sẽ hiển thị đầy đủ bên dưới.*

### Bước 3.2: Thực hiện ánh xạ tín hiệu (Signal Mapping)
1. Trong cửa sổ External Signal Configuration vừa kết nối:
   - Mở thư mục **Objects** -> **Actuators**. Bạn sẽ thấy các tag của 4 quạt: `Fan_1`, `Fan_2`, `Fan_3`, `Fan_4`.
2. Tiến hành kéo thả hoặc map tín hiệu:
   - Chọn tag `Fan_1` (trên OPC UA) và map với tín hiệu `mcd_fan_1` (trong NX MCD).
   - Vì giá trị từ Python là `0` đến `255`, tốc độ trong MCD tính bằng **độ/giây**. Nếu quạt quay với tốc độ 255 độ/giây thì khá chậm (chưa tới 1 vòng/giây). 
   - **Mẹo tăng tốc độ quạt**: Trong ô công thức ánh xạ (Scale/Formula), bạn có thể nhập công thức nhân thêm: `Scale * 10` hoặc `Scale * 15`. Ví dụ khi Python gửi giá trị `255`, quạt trong NX sẽ quay với tốc độ `255 * 10 = 2550 độ/giây` (khoảng 7 vòng/giây), trông sẽ rất sinh động và thực tế!
3. Làm tương tự cho 3 quạt còn lại:
   - Map `Fan_2` vào `mcd_fan_2`.
   - Map `Fan_3` vào `mcd_fan_3`.
   - Map `Fan_4` vào `mcd_fan_4`.
4. Nhấn **OK** để lưu lại toàn bộ cấu hình kết nối ngoài.

---

## PHẦN 4: CHẠY THỬ NGHIỆM MÔ PHỎNG (VIRTUAL COMMISSIONING)

Chúc mừng bạn! Mọi cấu hình đã hoàn tất, giờ là lúc tận hưởng thành quả:

1. Đảm bảo cửa sổ CMD chạy `opcua_simulator.py` vẫn đang chạy bình thường.
2. Trên phần mềm **Siemens NX MCD**, tìm thanh điều khiển mô phỏng ở dưới hoặc trên góc màn hình và nhấn nút **Play** (Nút hình tam giác màu xanh lá).
   *Lúc này mô phỏng bắt đầu chạy. Do giá trị ban đầu của quạt trên server là `0`, nên 4 quạt 3D trên màn hình vẫn đứng yên.*
3. Chuyển sang cửa sổ CMD chạy `test_fans.py`:
   - Gõ số `5` rồi nhấn **Enter** để bật cả 4 quạt lên tốc độ `255`.
   - **Nhìn lại màn hình Siemens NX: Bạn sẽ thấy cả 4 cánh quạt bắt đầu xoay tít cùng một lúc!**
   - Bạn có thể gõ số `6` rồi nhấn **Enter** trên cửa sổ test để tắt quạt, cánh quạt trên NX sẽ từ từ dừng lại.
   - Thử chọn các phím `1`, `2`, `3`, `4` để đặt tốc độ riêng biệt cho từng quạt (ví dụ quạt 1 đặt `50` quay chậm, quạt 2 đặt `250` quay nhanh) để thấy sự đồng bộ hoàn hảo.

---

## CÁC LỖI THƯỜNG GẶP VÀ CÁCH KHẮC PHỤC
* **Lỗi không kết nối được OPC UA (Connection Failed) trên NX**: Hãy kiểm tra xem file `opcua_simulator.py` đã được chạy trước đó chưa, và tường lửa (Windows Firewall) có đang chặn cổng `4840` không. Hãy tạm thời cho phép hoặc tắt Firewall đi nếu không kết nối được.
* **Cánh quạt quay lệch tâm/lệch hướng**: Do bước 2.3 bạn chọn sai điểm đặt trục quay hoặc sai chiều của Vector quay. Hãy click đúp vào các `Hinged Joint` để chỉnh sửa lại trục cho chuẩn xác.
