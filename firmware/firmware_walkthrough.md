# NHẬT KÝ VẬN HÀNH & HOẠT ĐỘNG HỆ THỐNG ECOFARM
*(Dành cho người mới bắt đầu và không cần biết sâu về lập trình)*

Chào bạn! Cuốn nhật ký này được viết ra để giải thích cách thức hoạt động của bộ não điều khiển giàn rau thủy canh **ESP32-S3** (dự án EcoFarm) theo cách đơn giản nhất. Mọi luồng xử lý, tính năng thông minh và các cải tiến tối ưu giúp máy chạy mượt mà đều được trình bày từng bước bằng ngôn ngữ đời thường, không dùng sơ đồ kỹ thuật phức tạp hay mã nguồn khó hiểu.

---

## PHẦN 1: ĐIỀU GÌ XẢY RA KHI CẮM ĐIỆN CHO THIẾT BỊ?
*(Trình tự hoạt động từng bước của máy từ lúc bắt đầu)*

Khi bạn cắm phích điện cấp nguồn cho con chip điều khiển ESP32-S3, thiết bị sẽ tự động thực hiện các bước sau theo thứ tự:

1.  **Bước 1: Khởi động hệ thống**
    Con chip tự mở nguồn, thiết lập các cổng kết nối và phát tín hiệu chào mừng ra cổng giám sát máy tính.
2.  **Bước 2: Nhớ lại trạng thái trước khi mất điện**
    Đây là tính năng thông minh. Chip sẽ tự động đọc lại bộ nhớ xem trước khi mất điện, đèn LED đang sáng bao nhiêu %, quạt đang quay tốc độ nào, bơm đang bật hay tắt.
3.  **Bước 3: Thiết lập trạng thái an toàn cho thiết bị điện**
    Để tránh việc các thiết bị công suất cao (bơm 220V, máy sục oxy) bị giật điện hoặc tự động bật tắt đột ngột gây hỏng hóc lúc mới có điện, chip sẽ đưa tất cả các chân điện về trạng thái an toàn trước, sau đó mới áp dụng chính xác trạng thái đã phục hồi ở Bước 2.
4.  **Bước 4: Khởi động cảm biến**
    Hệ thống quét đường truyền tín hiệu để tìm kiếm và khởi động cảm biến ánh sáng (BH1750).
5.  **Bước 5: Kết nối mạng WiFi**
    *   **Trường hợp 1 (Nếu đã lưu WiFi nhà từ trước)**: Thiết bị sẽ tự động dò tìm và kết nối vào WiFi nhà bạn.
    *   **Trường hợp 2 (Nếu chưa có WiFi hoặc không kết nối được sau 15 giây)**: Thiết bị sẽ tự động phát ra một mạng WiFi riêng của nó tên là **`DATN_AIOT_LETTUCE`** (không cần mật khẩu) để bạn dùng điện thoại kết nối vào cấu hình.
6.  **Bước 6: Mở cổng điều khiển**
    Thiết bị tạo sẵn một trang giao diện điều khiển (Website) và một đường truyền siêu tốc (WebSocket). Lúc này bạn đã có thể vào điện thoại để xem thông số rau và nhấn nút bật/tắt thiết bị.

---

## PHẦN 2: CÁC KHỐI CHỨC NĂNG VÀ CÁCH CHÚNG LÀM VIỆC

Mạch điều khiển được chia làm hai phần chính: **Phần điều khiển thiết bị điện** và **Phần đọc thông tin môi trường**.

### 1. Phần điều khiển thiết bị điện (Actuators)
Hệ thống quản lý tổng cộng 10 thiết bị điện thông qua các rơ-le (Relay) đóng ngắt và các mạch điều áp (PWM):
*   **Các thiết bị bật/tắt đơn giản (Relay)**: Gồm có Bơm chìm bơm nước lên giàn (Relay 1) và Máy sục khí oxy tạo bọt cho nước (Relay 2).
*   **Các thiết bị điều khiển được mức độ mạnh/yếu (PWM)**:
    *   **Đèn LED (Tầng 1 & Tầng 2)**: Tăng giảm độ sáng từ 0% đến 100%.
    *   **Quạt gió (Tầng 1 & Tầng 2)**: Tăng giảm tốc độ gió để làm mát cho rau.
    *   **Bơm dinh dưỡng (Bơm chai A, Bơm chai B, Bơm pH Down)**: Điều tiết tốc độ bơm dung dịch đậm đặc để pha vào nước.
    *   **Bơm nước 12V chính**: Điều chỉnh lưu lượng dòng chảy tuần hoàn.
*   **Tính năng bảo vệ động cơ (Giới hạn 10%)**: Nếu bạn vô tình chỉnh tốc độ quạt hoặc bơm quá thấp (dưới 10% công suất), động cơ sẽ không đủ lực để quay mà chỉ phát ra tiếng kêu o o và sinh nhiệt gây cháy. Bộ não thiết bị sẽ tự động ép mức tối thiểu phải là 10% nếu thiết bị đang được bật.

### 2. Phần đọc thông tin môi trường (Sensors)
Hệ thống được thiết kế để theo dõi 11 thông số quan trọng của giàn rau bao gồm: Ánh sáng, Nhiệt độ không khí, Độ ẩm không khí, Nhiệt độ nước, Độ đậm đặc dinh dưỡng (TDS), Độ chua của nước (pH), Lưu lượng nước chảy và 4 phao báo mức nước cạn ở bể chính và các chai dinh dưỡng.

---

## PHẦN 3: CÁC CẢI TIẾN GIÚP MÁY CHẠY SIÊU MƯỢT VÀ BỀN BỈ
*(Những tính năng thông minh đã được lập trình để tối ưu hóa thiết bị)*

Để thiết bị hoạt động 24/7 ngoài giàn rau mà không bị đơ, treo máy hoặc hỏng linh kiện, chúng tôi đã tích hợp các giải pháp sau:

### 1. Tự giải cứu cảm biến khi bị đơ (I2C Bus Recovery)
*   **Vấn đề**: Khi các máy bơm hoặc máy sục khí oxy 220V bật/tắt, tiếp điểm điện sẽ tạo ra tia lửa điện nhỏ gây nhiễu điện từ trường. Nhiễu này chạy vào dây tín hiệu làm cảm biến ánh sáng bị đơ (kẹt tín hiệu), làm cho toàn bộ bộ não ESP32 bị treo theo, không điều khiển được gì nữa.
*   **Cách giải quyết**: Chúng tôi viết một chương trình tự phát hiện sự cố. Khi cảm biến ánh sáng không phản hồi, bộ não sẽ tạm thời tắt đường truyền, tự động gửi **9 nhịp tín hiệu xung lực** để kích hoạt lại cảm biến bị kẹt, sau đó thiết lập lại kết nối mới. Quá trình này diễn ra chỉ trong vài phần nghìn giây giúp giàn rau hoạt động liên tục mà người dùng không hề hay biết.

### 2. Trì hoãn ghi nhớ trạng thái để bảo vệ bộ nhớ (Settle-Time Debouncing)
*   **Vấn đề**: Bộ nhớ lưu trữ (Flash) của chip giống như thẻ nhớ điện thoại, có giới hạn số lần ghi (khoảng 100.000 lần). Khi bạn dùng ngón tay vuốt liên tục trên điện thoại để tăng giảm độ sáng đèn LED từ 10% lên 90%, điện thoại sẽ gửi hàng chục lệnh thay đổi liên tục trong 1 giây. Nếu chip lưu ngay từng thay đổi đó vào bộ nhớ, bộ nhớ sẽ bị hỏng chỉ sau vài tuần và chip bị giật lag khi điều khiển.
*   **Cách giải quyết**: Đèn LED vẫn sẽ sáng/tối lập tức theo ngón tay bạn vuốt trên điện thoại (rất nhạy). Tuy nhiên, bộ não sẽ đợi bạn **buông tay hoàn toàn và không chỉnh gì nữa trong vòng 2 giây** thì mới ghi nhớ giá trị cuối cùng vào bộ nhớ Flash.

### 3. Nén nhỏ trang web điều khiển (Gzip Compression)
*   **Cách giải quyết**: Toàn bộ hình ảnh, nút bấm, đồ thị của trang web điều khiển đều được chúng tôi dùng phần mềm nén siêu nhỏ lại (giống như nén file `.zip`). Khi điện thoại của bạn kết nối vào thiết bị, thiết bị chỉ cần gửi file nén này qua WiFi và điện thoại tự giải nén ra để hiển thị. Việc này giúp trang web tải cực nhanh (dưới 0.5 giây) và giúp bộ não chip trống rất nhiều bộ nhớ RAM để làm việc khác.

### 4. Khóa tạm thời trạng thái nút bấm trên điện thoại (UI State-Lock)
*   **Vấn đề**: Khi bạn nhấn nút bật đèn trên điện thoại, lệnh phải truyền qua WiFi đến chip, chip bật đèn xong truyền trạng thái ngược lại điện thoại xác nhận. Trong khoảng thời gian ngắn đó, nếu điện thoại tự cập nhật trạng thái cũ từ trước khi nhấn, nút bấm trên màn hình sẽ bị giật ngược trở lại rồi mới bật lên, gây khó chịu cho người dùng.
*   **Cách giải quyết**: Khi bạn nhấn nút hoặc kéo thanh trượt, giao diện web trên điện thoại sẽ tự động **khóa trạng thái đó lại trong 2.5 giây**, bỏ qua mọi dữ liệu cũ từ chip gửi về. Sau 2.5 giây, khi lệnh đã thực hiện xong xuôi, khóa mới mở ra để cập nhật bình thường.

### 5. Tự động kết nối lại mạng (WiFi & WebSocket Auto-Reconnect)
*   Nếu nhà bị mất WiFi hoặc đổi mật khẩu, thiết bị không bị đơ mà sẽ tự động tìm kiếm kết nối lại. Khi mất kết nối WebSocket (đường truyền siêu tốc), màn hình điện thoại tự động chuyển sang chế độ dự phòng (gửi lệnh chậm hơn một chút qua HTTP cứu hộ) để bạn vẫn điều khiển được thiết bị, đồng thời tự động dò kết nối lại WebSocket ở chế độ chạy ngầm.

---

## PHẦN 4: HƯỚNG DẪN DÀNH CHO BẠN ĐỂ PHÁT TRIỂN THÊM TRONG TƯƠNG LAI
*(Phần quan trọng để bạn tự hoàn thiện giàn rau)*

### 1. Lưu ý quan trọng về các thông số cảm biến hiện tại:
Để bạn có thể kiểm tra giao diện trực quan trước khi lắp đặt phần cứng thật, **hiện tại mới chỉ có cảm biến Ánh sáng là hoạt động thật**. Tất cả các thông số cảm biến còn lại đều đang được lập trình phát ra **con số ảo cố định (Mockup)**:
*   Nhiệt độ phòng: `30.9 độ C`
*   Độ ẩm phòng: `68 %`
*   Nhiệt độ nước: `25.5 độ C`
*   Dinh dưỡng trong nước: `720 ppm`
*   Độ chua pH: `6.0`
*   Lượng nước chảy: `1.5 lít/phút`
*   Mực nước thùng chứa và các chai dinh dưỡng đang báo các số: `80%`, `50%`, `45%`, `90%`.

### 2. Các bước bạn cần làm để đấu nối cảm biến thật:

*   **Để đo Nhiệt độ & Độ ẩm không khí thật**: Hãy mua cảm biến **SHT31**, cắm chung vào 2 dây tín hiệu I2C (chân SDA và SCL) và bật thư viện SHT31 trong mã nguồn để đọc dữ liệu thật đè lên con số ảo `30.9` và `68%`.
*   **Để đo Nhiệt độ nước thật**: Mua cảm biến chống nước **DS18B20**, đấu vào chân số **14 (GPIO 14)** của mạch và cấu hình đọc tín hiệu 1-Wire.
*   **Để đo Dinh dưỡng TDS & pH thật**:
    *   Hai cảm biến này sẽ cắm vào cổng đọc điện áp analog là chân **ADC 1** và **ADC 2**.
    *   Lưu ý: Đầu đo cảm biến TDS nếu ngâm điện liên tục trong nước sẽ bị oxi hóa nhanh và hỏng. Thiết bị đã chừa sẵn chân nguồn **GPIO 42 (PIN_EN_TDS)**. Khi nào muốn đo, bạn hãy lập trình bật chân 42 lên để cấp điện cho cảm biến, đọc chỉ số xong thì tắt chân 42 đi để bảo vệ cảm biến bền bỉ.

---
*Hy vọng cuốn nhật ký đơn giản này sẽ giúp bạn dễ dàng làm chủ thiết bị của mình! Chúc bạn trồng được những giàn rau xà lách tươi ngon và đạt kết quả tốt nhất.*
