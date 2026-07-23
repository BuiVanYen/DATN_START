#pragma once // Chỉ include file header này 1 lần duy nhất

// ============================================================================
// HÀM VÒNG LẶP TASK CONTROL AN TOÀN FREERTOS (CHẠY TRÊN CORE 1)
// ============================================================================
// Task có độ ưu tiên cao nhất (Priority 5) và là "chủ sở hữu" duy nhất được phép ghi ngõ ra
// lên 10 thiết bị phần cứng thực tế. Quản lý nhận lệnh MANUAL và đếm ngược Web Lease 10s.
void taskSafetyControl(void *parameter);
