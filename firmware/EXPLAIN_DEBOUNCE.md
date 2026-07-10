# Giải Thích Settle-Time Debouncing

## Timeline Chi Tiết

### Code CŨ (Ghi Flash liên tục):
```
00:00.000  User kéo slider lên 50
           → analogWrite(DEN1, 50)     ✅ Đèn sáng 50/255
           → save_to_flash(50)         💾 GHI FLASH LẦN 1

00:00.100  User kéo tiếp lên 100
           → analogWrite(DEN1, 100)    ✅ Đèn sáng 100/255
           → save_to_flash(100)        💾 GHI FLASH LẦN 2

00:00.200  User kéo tiếp lên 150
           → analogWrite(DEN1, 150)    ✅ Đèn sáng 150/255
           → save_to_flash(150)        💾 GHI FLASH LẦN 3

00:00.300  User kéo tiếp lên 200
           → analogWrite(DEN1, 200)    ✅ Đèn sáng 200/255
           → save_to_flash(200)        💾 GHI FLASH LẦN 4

00:00.400  User DỪNG tại 250
           → analogWrite(DEN1, 250)    ✅ Đèn sáng 250/255
           → save_to_flash(250)        💾 GHI FLASH LẦN 5

❌ TỔNG: 5 lần ghi Flash (chỉ cần 1 lần!)
```

---

### Code MỚI (Chỉ ghi khi user DỪNG):
```
00:00.000  User kéo slider lên 50
           → analogWrite(DEN1, 50)              ✅ Đèn sáng NGAY
           → pending_value = 50                  📝 Ghi nhớ
           → last_change_time = 0ms              ⏱️ Reset timer
           → has_pending = true                  🚩 Đánh dấu

00:00.100  User kéo tiếp lên 100
           → analogWrite(DEN1, 100)             ✅ Đèn sáng NGAY
           → pending_value = 100 (ghi đè)        📝 Cập nhật
           → last_change_time = 100ms            ⏱️ Reset timer
           → has_pending = true                  🚩 Vẫn chờ

00:00.200  User kéo tiếp lên 150
           → analogWrite(DEN1, 150)             ✅ Đèn sáng NGAY
           → pending_value = 150 (ghi đè)        📝 Cập nhật
           → last_change_time = 200ms            ⏱️ Reset timer
           → has_pending = true                  🚩 Vẫn chờ

00:00.300  User kéo tiếp lên 200
           → analogWrite(DEN1, 200)             ✅ Đèn sáng NGAY
           → pending_value = 200 (ghi đè)        📝 Cập nhật
           → last_change_time = 300ms            ⏱️ Reset timer
           → has_pending = true                  🚩 Vẫn chờ

00:00.400  User DỪNG tại 250
           → analogWrite(DEN1, 250)             ✅ Đèn sáng NGAY
           → pending_value = 250 (ghi đè)        📝 Cập nhật
           → last_change_time = 400ms            ⏱️ Reset timer lần cuối
           → has_pending = true                  🚩 Bắt đầu đếm ngược

[User không động gì nữa...]

00:01.400  loop() kiểm tra:
           → time_since = 1000ms (chưa đủ 2s)   ⏳ Đợi tiếp

00:02.400  loop() kiểm tra:
           → time_since = 2000ms (ĐỦ 2s!)       ✅ User đã dừng!
           → save_to_flash(250)                  💾 GHI FLASH DUY NHẤT
           → has_pending = false                 ✔️ Xong việc

✅ TỔNG: 1 lần ghi Flash (giá trị cuối cùng đúng: 250)
```

---

## Điểm Khác Biệt Chính

| Thời điểm | Code CŨ | Code MỚI |
|-----------|---------|----------|
| User kéo slider | Ghi Flash ngay | Chỉ ghi nhớ, KHÔNG ghi Flash |
| User dừng 2s | Không làm gì | Mới ghi Flash (1 lần) |
| Đèn sáng khi nào? | Ngay lập tức | Ngay lập tức (giống nhau) |
| Số lần ghi Flash | Nhiều lần | 1 lần duy nhất |

---

## Tại Sao Cần 2 Giây?

- **1 giây**: Quá ngắn → User chưa kịp dừng → vẫn lưu nhiều lần
- **2 giây**: Vừa đủ → User thường dừng sau 1-1.5s
- **5 giây**: Quá lâu → Rủi ro mất dữ liệu nếu mất điện

→ **2 giây = Sweet spot tối ưu**

---

## Force Flush (Trường Hợp Đặc Biệt)

Nếu user điều chỉnh đèn → 1.5s sau click "Restart":
```
00:00.000  User điều chỉnh DEN1 = 250
00:01.500  User click "WiFi Save" → Restart
           
Code CŨ: ❌ Chỉ chờ 1.5s → CHƯA lưu → Mất giá trị 250
Code MỚI: ✅ Gọi actuator_force_flush() → LƯU NGAY 250 → An toàn!
```

**Force flush được gọi trước:**
- Restart (WiFi Save/Forget)
- OTA Upload
- Low Battery (nếu có)
