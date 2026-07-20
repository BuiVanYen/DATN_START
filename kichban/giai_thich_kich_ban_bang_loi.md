# GIẢI THÍCH KỊCH BẢN VẬN HÀNH HỆ THỦY CANH — BẰNG LỜI DỄ HIỂU

> Tài liệu này diễn giải toàn bộ logic điều khiển trong `KICH_BAN_HOAN_CHINH.md` bằng ngôn ngữ đời thường, kèm ví dụ lịch trình thời gian cụ thể để bạn có thể hình dung chính xác "ESP32 đang làm gì vào lúc nào".

---

## 1. TỔNG QUAN: HỆ THỐNG NÀY LÀM GÌ?

Hãy tưởng tượng bạn thuê một **người giữ vườn robot** làm việc 24/7, không ngủ. Người này có:
- **10 giác quan** (cảm biến): đo ánh sáng, nhiệt độ không khí, độ ẩm, nhiệt độ nước, độ pH, nồng độ dinh dưỡng, tốc độ dòng nước, mực nước bể và mực nước 3 chai hóa chất.
- **10 bàn tay** (thiết bị đầu ra): 2 đèn LED, 2 quạt, 3 bơm châm hóa chất (A, B, pH Down), 1 bơm bù nước sạch, 1 máy sục khí oxy và 1 bơm tuần hoàn nước.

Bộ não của robot này là con chip **ESP32-S3**, chạy hệ điều hành FreeRTOS có thể làm nhiều việc cùng lúc. Nó gửi báo cáo về **Raspberry Pi 4** (máy tính nhỏ đóng vai "quản lý") qua Wi-Fi.

**Nguyên tắc vàng:** Robot này được thiết kế theo kiểu **"thà dừng lại còn hơn làm sai"**. Nếu nghi ngờ bất kỳ điều gì, nó sẽ tắt thiết bị nguy hiểm và kêu người đến kiểm tra, thay vì tự ý hành động.

---

## 2. BA "BỘ PHẬN NÃO" LÀM VIỆC SONG SONG

ESP32-S3 có 2 lõi xử lý (giống 2 người làm việc cùng lúc). Công việc được chia cho 3 nhóm:

### Nhóm 1: "Anh liên lạc" (TaskNetwork — Lõi 0)
- Chỉ làm một việc: giữ kết nối Wi-Fi và gửi/nhận tin nhắn MQTT với Pi 4.
- **Không bao giờ** tự tay bật/tắt bất kỳ thiết bị nào. Khi nhận lệnh từ Pi 4, anh ta chỉ viết lệnh vào một "hộp thư" (Queue) rồi chuyển cho người khác xử lý.

### Nhóm 2: "Anh đo đạc" (TaskSensors — Lõi 1)
- Cứ 10 giây đọc nhiệt độ, độ ẩm, ánh sáng.
- Cứ 60 phút một lần, thực hiện quy trình đo hóa học đặc biệt (Quiet Window) để đo pH và nồng độ dinh dưỡng.
- Sau khi đo xong, ghi kết quả vào "hộp thư cảm biến" kèm nhãn chất lượng: `tốt`, `cũ` hoặc `lỗi`.

### Nhóm 3: "Anh an toàn" (TaskSafetyControl — Lõi 1)
- **Là người DUY NHẤT được phép bật/tắt thiết bị.** Không ai khác được chạm vào công tắc.
- Đọc "hộp thư cảm biến" và "hộp thư lệnh", kiểm tra mọi điều kiện an toàn rồi mới ra quyết định.
- Nếu phát hiện nguy hiểm → tắt ngay thiết bị liên quan, kêu còi và báo Pi 4.

> **Ví dụ dễ hiểu:** Giống như trong bệnh viện — y tá đo huyết áp (TaskSensors), lễ tân nhận điện thoại (TaskNetwork), nhưng chỉ có bác sĩ trực (TaskSafetyControl) mới được phép kê đơn thuốc và bấm nút máy.

---

## 3. KỊCH BẢN KHỞI ĐỘNG (KHI CẮM ĐIỆN LẦN ĐẦU HOẶC SAU KHI MẤT ĐIỆN)

Mỗi khi ESP32 được bật nguồn (hoặc tự khởi động lại sau sự cố), nó luôn thực hiện đúng trình tự sau:

| Bước | ESP32 làm gì | Tại sao phải làm vậy |
|:---:|---|---|
| 1 | **Tắt hết mọi thứ** — tất cả bơm, đèn, quạt, relay đều OFF. | Đề phòng có thiết bị đang bật dở dang trước khi mất điện. |
| 2 | Đọc bộ nhớ Flash xem có **cờ "đang châm dở"** không. | Nếu mất điện giữa lúc đang châm hóa chất → ghi nhận sự cố, KHÔNG tiếp tục châm phần còn lại. |
| 3 | Kiểm tra từng cảm biến: có hoạt động không? Có trả về giá trị hợp lý không? | Cảm biến bị rút dây hoặc hỏng sẽ bị đánh dấu "lỗi". |
| 4 | Kiểm tra đồng hồ: ESP32 đã biết mấy giờ chưa? | Nếu chưa đồng bộ được giờ → đèn LED giữ tắt vì không biết lúc nào nên bật. |
| 5 | Kiểm tra mực nước bể chứa. | Nếu bể cạn → vào chế độ bù nước trước. Nếu đủ → bật bơm tuần hoàn. |
| 6 | Bật bơm tuần hoàn, chờ 10 giây, kiểm tra dòng chảy. | Nếu nước không chảy → thử lại 2 lần. Vẫn không chảy → báo lỗi bơm. |
| 7 | Nếu mọi thứ OK → vào chế độ **NORMAL** (tự động). | Bắt đầu vận hành bình thường. |

---

## 4. MỘT NGÀY VẬN HÀNH MẪU (TIMELINE CỤ THỂ)

Dưới đây là ví dụ **một ngày bình thường** khi hệ thống đã chạy ổn định. Các mốc giờ là tham khảo dựa trên chu kỳ đo 60 phút.

### 🌅 Sáng sớm — 06:00

| Giờ | Sự kiện | Chi tiết |
|---|---|---|
| 06:00:00 | **Đèn LED bật** (cả tầng 1 và tầng 2) | Theo lịch đã cài (06:00-22:00). Quạt tự động bật thông gió nền 30%. |
| 06:00:00 | Bơm tuần hoàn vẫn **đang chạy liên tục** | Bơm này chạy 24/7, không bao giờ nghỉ trừ khi đo hóa chất hoặc lỗi. |
| 06:00:00 | Máy sục khí vẫn **đang chạy liên tục** | Cung cấp oxy cho rễ cây trong bể. |

### ☀️ Buổi sáng — Chu kỳ đo hóa chất lúc 07:00

Đây là phần quan trọng nhất. Cứ 60 phút, ESP32 thực hiện một "buổi khám sức khỏe nước" gọi là **Quiet Window** (Cửa sổ yên tĩnh):

| Giờ | Bước | ESP32 làm gì | Lý do |
|---|:---:|---|---|
| 07:00:00 | 1 | **Khóa chặt** tất cả bơm châm hóa chất và bơm bù nước. | Không cho ai châm gì trong lúc đang đo. |
| 07:00:01 | 2 | **Tắt** quạt, đèn, bơm tuần hoàn và máy sục khí. | Tất cả các thiết bị này tạo ra nhiễu điện và bọt khí làm sai kết quả đo. |
| 07:00:01 | 3 | **Chờ 60 giây** — không làm gì cả. | Chờ nước trong bể lắng xuống hoàn toàn, bọt khí tan hết. |
| 07:01:01 | 4 | Đo **nhiệt độ nước** bằng cảm biến DS18B20. | Cần nhiệt độ nước để hiệu chỉnh kết quả đo dinh dưỡng. |
| 07:01:02 | 5 | Đo **pH** — lấy 30 mẫu liên tiếp, nghỉ, lấy thêm 30 mẫu. | Đo 2 lần độc lập để so sánh: nếu 2 lần cho kết quả gần nhau → tin được. Nếu lệch quá 0.15 → kết quả "không đáng tin", bỏ. |
| 07:01:05 | 6 | Bật nguồn mạch đo TDS, **chờ 2 giây** cho mạch ổn định. | Mạch đo TDS cần thời gian "khởi động nóng" mới cho số chính xác. |
| 07:01:07 | 7 | Đo **TDS/EC** — cũng lấy 2 lần × 30 mẫu, lọc giống pH. | Nếu 2 lần đo lệch nhau quá 5% → kết quả bị loại. |
| 07:01:08 | 8 | **Tắt ngay** nguồn mạch đo TDS. | Để nguồn TDS bật lâu sẽ làm mòn điện cực đo. |
| 07:01:08 | 9 | **Bật lại** sục khí → bơm tuần hoàn → kiểm tra dòng chảy → bật lại đèn, quạt. | Khôi phục mọi thứ về trạng thái trước khi đo. |

> **Tổng thời gian tắt bơm:** khoảng 68 giây. Rễ cây hoàn toàn không bị ảnh hưởng trong thời gian ngắn này.

### ☀️ Sau khi đo — Quyết định có châm dinh dưỡng không? (07:01 trở đi)

Sau khi có kết quả đo, "Anh an toàn" (TaskSafetyControl) sẽ kiểm tra theo thứ tự nghiêm ngặt:

**Bước A — Kiểm tra tiên quyết (đạt hết mới đi tiếp):**
1. ✅ Kết quả đo có nhãn "tốt" không? (Nếu "lỗi" hoặc "không đáng tin" → dừng, không châm gì)
2. ✅ Bể có đủ nước không?
3. ✅ Bơm tuần hoàn có đang chạy và có dòng chảy không?
4. ✅ Hạn mức châm trong ngày/trong giờ còn không? (quota)
5. ✅ Hiệu chuẩn bơm và cảm biến còn hạn không?

**Bước B — Kiểm tra nồng độ dinh dưỡng (EC):**

| Tình huống | ESP32 phản ứng |
|---|---|
| EC **thấp hơn** mục tiêu | Kiểm tra chai A và chai B còn không → Nếu còn: châm A rồi châm B (xem mục 5 bên dưới). Nếu hết 1 trong 2: khóa cả 2 bơm, kêu cảnh báo. |
| EC **đúng** mục tiêu | Không châm. Chuyển sang kiểm tra pH. |
| EC **cao hơn** mục tiêu | Không châm. Gửi cảnh báo để người dùng pha loãng thủ công (hệ thống không có van xả tự động). |

**Bước C — Kiểm tra pH (chỉ sau khi EC đã ổn):**

| Tình huống | ESP32 phản ứng |
|---|---|
| pH **cao hơn** 6.5 | Kiểm tra chai pH Down còn không → Nếu còn: châm một liều rất nhỏ (micro-dose). |
| pH **đúng** dải 5.6–6.5 | Không châm. Kết thúc, quay về chế độ bình thường. |
| pH **thấp hơn** 4.5 (nước quá chua) | **KHÔNG châm thêm gì** (hệ thống không có pH Up). Báo động khẩn để người dùng thay nước. |

---

## 5. QUY TRÌNH CHÂM DINH DƯỠNG — DIỄN RA NHƯ THẾ NÀO?

Giả sử lúc 07:01 hệ thống phát hiện EC thấp và cần châm. Đây là toàn bộ quá trình:

| Giờ | Bước | Chi tiết |
|---|:---:|---|
| 07:01:10 | 1 | Ghi cờ **"đang châm"** vào bộ nhớ Flash. | 
| 07:01:11 | 2 | Kiểm tra: bơm tuần hoàn chạy OK? Dòng chảy OK? Chai A còn? Chai B còn? |
| 07:01:12 | 3 | **Bật bơm A** — châm đúng số mL đã tính (ví dụ 5 mL, tức khoảng 10 giây nếu bơm đã hiệu chuẩn 0.5 mL/giây). |
| 07:01:22 | 4 | Tắt bơm A. **Chờ trộn 60 giây** — bơm tuần hoàn và sục khí đang chạy giúp hòa tan dung dịch A. |
| 07:02:22 | 5 | **Bật bơm B** — châm đúng lượng tương ứng theo tỷ lệ sản phẩm. |
| 07:02:32 | 6 | Tắt bơm B. **Chờ trộn 5 phút** — cho A và B hòa tan hoàn toàn vào 40L nước. |
| 07:07:32 | 7 | Chạy **Quiet Window lần 2** — tắt bơm 68 giây, đo lại EC. |
| 07:08:40 | 8 | So sánh EC mới với mục tiêu. |
| | | → EC đạt mục tiêu? ✅ Xong! Xóa cờ "đang châm", quay về bình thường. |
| | | → EC vẫn thấp? Quay lại bước 2 châm vòng 2 (tối đa chỉ 2 vòng). |
| | | → Sau 2 vòng vẫn không đạt? 🔒 Khóa châm, kêu cảnh báo người kiểm tra. |

> **Sau khi châm EC xong**, nếu pH cao quá 6.5, hệ thống tiếp tục châm pH Down theo quy trình tương tự (micro-dose → trộn 5 phút → đo lại → tối đa 2 vòng).

> **Quy tắc sắt đá:** Bơm A và bơm B **KHÔNG BAO GIỜ** được bật cùng lúc. Bơm pH Down cũng không được bật cùng lúc với A hoặc B.

---

## 6. BÙ NƯỚC TỰ ĐỘNG (REFILL) — KHI BỂ CẠN

Nước trong bể bốc hơi dần mỗi ngày (nhất là Sài Gòn nắng nóng). Khi cảm biến phao báo mực nước thấp:

| Bước | ESP32 làm gì |
|:---:|---|
| 1 | Đợi phao báo "cạn" liên tục **2 giây** (tránh sóng nước dập dềnh gây nhầm). |
| 2 | **Tắt bơm tuần hoàn**, khóa tất cả bơm hóa chất. |
| 3 | **Bật bơm cấp nước sạch** — bơm nước máy/nước lọc vào bể. |
| 4 | Chờ đến khi phao báo "đủ" liên tục 2 giây → tắt bơm nước. |
| 5 | Nếu bơm quá lâu mà phao vẫn báo cạn (vượt `fill_max_time`) → **tắt ngay bơm nước**, báo lỗi. Có thể ống bị rò hoặc phao bị kẹt. |
| 6 | Sau khi bù nước xong → bật tuần hoàn + sục khí **trộn 5 phút** → đo lại pH/EC. |

---

## 7. QUẠT VÀ ĐÈN — HOẠT ĐỘNG THẾ NÀO?

### Đèn LED Grow
- Bật lúc **06:00**, tắt lúc **22:00** (16 tiếng chiếu sáng).
- Nếu ESP32 mới bật nguồn mà chưa biết mấy giờ (chưa kết nối Wi-Fi để lấy giờ) → **giữ đèn tắt** để an toàn.

### Quạt thông gió
Quạt tự điều chỉnh tốc độ theo nhiệt độ và độ ẩm:

| Nhiệt độ không khí | Tốc độ quạt |
|---|---|
| Dưới 26°C | 30% (gió nhẹ thông thoáng khi đèn bật) |
| 26°C – 28°C | 40% |
| 28°C – 30°C | 70% |
| Trên 30°C | 100% (chạy hết công suất) |

**Để quạt không bị "nhảy điên":** Ví dụ nhiệt độ đang 30.1°C, quạt chạy 100%. Nếu nhiệt giảm xuống 29.8°C, quạt vẫn giữ 100% — phải giảm xuống dưới **29°C** (tức trễ 1°C) mới hạ về 70%. Và mỗi lần thay đổi phải giữ ít nhất **60 giây**.

---

## 8. KHI CÓ SỰ CỐ — ESP32 PHẢN ỨNG THẾ NÀO?

### Mất Wi-Fi / Mất kết nối Pi 4
→ ESP32 **vẫn tiếp tục tự vận hành** bình thường (bơm, đèn, quạt, đo, châm đều chạy như cũ). Khi Wi-Fi có lại → tự kết nối và gửi toàn bộ trạng thái hiện tại lên Pi 4.

### Mất điện rồi có lại
→ ESP32 khởi động lại từ đầu: tắt hết → kiểm tra → bật từng thứ an toàn. Nếu đang châm dở → hủy liều cũ, trộn nước rồi đo mới.

### Bơm tuần hoàn bật mà nước không chảy
→ Tắt bơm, chờ, thử lại 2 lần. Vẫn không chảy → **khóa cứng** bơm tuần hoàn và tất cả bơm hóa chất, chỉ giữ máy sục khí, kêu còi cảnh báo. Chờ người đến sửa.

### Chai dinh dưỡng A hoặc B hết
→ Khóa **cả hai** bơm A và B (vì châm lệch tỷ lệ sẽ gây mất cân bằng dinh dưỡng). Báo cảnh báo.

### Cảm biến pH/TDS cho ra số bất thường
→ Đánh dấu "không đáng tin", **cấm châm tự động**. Chờ chu kỳ đo tiếp hoặc người kiểm tra lại cảm biến.

---

## 9. LỊCH TRÌNH MỘT NGÀY ĐẦY ĐỦ (TÓM TẮT)

| Giờ | Sự kiện chính |
|---|---|
| 00:00 - 06:00 | 🌙 Đèn tắt. Bơm tuần hoàn + sục khí vẫn chạy. Quạt chạy nền nếu nóng. Đo hóa học lúc 01:00, 02:00, 03:00, 04:00, 05:00 (mỗi 60 phút). |
| 06:00 | 🌅 Đèn bật. Quạt tăng lên 30% nền. |
| 07:00 | 🔬 Quiet Window: tắt bơm 68 giây, đo pH/EC. Nếu cần châm → châm A→B→trộn→đo lại→pH Down nếu cần. |
| 08:00 | 🔬 Quiet Window tiếp theo (nếu 07:00 không cần châm, lần này chỉ đo nhanh). |
| ~09:00 | 🌡️ Trời bắt đầu nóng, quạt tự tăng tốc dần. |
| 09:00 - 17:00 | ☀️ Chu kỳ lặp: đo mỗi giờ, bù nước nếu bể cạn, châm dinh dưỡng nếu cần. |
| ~14:00 | 🥵 Nhiệt độ cao nhất trong ngày. Quạt có thể chạy 100%. Sục khí mạnh. |
| 18:00 | 🌤️ Nhiệt độ bắt đầu giảm. Quạt tự hạ tốc. |
| 22:00 | 🌙 Đèn tắt theo lịch. Quạt giảm về mức nền hoặc tắt nếu mát. |
| 22:00 - 00:00 | Bơm tuần hoàn + sục khí vẫn chạy. Đo hóa học lúc 23:00. |

> **Lưu ý:** Lịch trên chỉ là ví dụ. Trong thực tế, chu kỳ châm hóa chất phụ thuộc vào kết quả đo — có ngày không cần châm lần nào, có ngày cần châm 2-3 lần.

---

## 10. TÓM TẮT NGUYÊN TẮC CỐT LÕI (5 CÂU)

1. **Bơm tuần hoàn chạy 24/7**, chỉ dừng tối đa 2 phút khi đo hóa chất.
2. **Chỉ có 1 "người" được bật/tắt thiết bị** (TaskSafetyControl) — tránh xung đột.
3. **Đo trước, châm sau** — không bao giờ châm hóa chất khi chưa có kết quả đo đáng tin cậy.
4. **Châm A xong mới châm B, B xong mới châm pH Down** — không bao giờ cùng lúc.
5. **Khi nghi ngờ → tắt và kêu người** — mọi lỗi nghiêm trọng đều khóa cứng hệ thống cho đến khi người xác nhận đã sửa xong.
