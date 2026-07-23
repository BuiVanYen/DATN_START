#include "ota_update.h"

#include "config.h"
#include "rtos_app.h"

#include <Update.h>

namespace {
String ota_error_text = "OK"; // Biến lưu chuỗi thông báo lỗi nạp OTA

// Phát lệnh ngắt khẩn cấp tất cả các tải an toàn trước khi ghi Flash
bool queueOtaSafeOff() {
  ControlCommand command{};
  command.type = ControlCommandType::OTA_START;
  command.created_at_ms = millis();
  command.ttl_ms = CONTROL_COMMAND_TTL_MS;
  return app_send_control_command(command, pdMS_TO_TICKS(100));
}

// Chuyển mã lỗi thư viện Update sang chuỗi chữ
String updateErrorToText() {
  switch (Update.getError()) {
  case UPDATE_ERROR_OK:
    return "OK";
  case UPDATE_ERROR_WRITE:
    return "WRITE_ERROR";
  case UPDATE_ERROR_ERASE:
    return "ERASE_ERROR";
  case UPDATE_ERROR_SPACE:
    return "NO_SPACE";
  case UPDATE_ERROR_SIZE:
    return "BAD_SIZE";
  case UPDATE_ERROR_STREAM:
    return "STREAM_ERROR";
  case UPDATE_ERROR_MD5:
    return "MD5_ERROR";
  case UPDATE_ERROR_MAGIC_BYTE:
    return "BAD_MAGIC_BYTE";
  case UPDATE_ERROR_ACTIVATE:
    return "ACTIVATE_ERROR";
  case UPDATE_ERROR_NO_PARTITION:
    return "NO_PARTITION";
  case UPDATE_ERROR_BAD_ARGUMENT:
    return "BAD_ARGUMENT";
  case UPDATE_ERROR_ABORT:
    return "ABORTED";
  default:
    return "UNKNOWN_OTA_ERROR";
  }
}
} // namespace

// ============================================================================
// KHỞI TẠO ROUTE OTA VÀ XỬ LÝ NẠP FIRMWARE KHÔNG DÂY
// ============================================================================
void ota_update_init(WebServer &server) {
  // Đăng ký route POST /update tiếp nhận upload file .bin
  server.on(
      "/update", HTTP_POST,
      [&server]() { ota_update_handle_end(server); },
      [&server]() { ota_update_handle_upload(server); });
}

// Handler tiếp nhận từng block dữ liệu file .bin tải lên
void ota_update_handle_upload(WebServer &server) {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    queueOtaSafeOff(); // Tắt toàn bộ 10 thiết bị ngoại vi an toàn
    delay(100);
    ota_error_text = "OK";
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      ota_error_text = updateErrorToText(); // Bắt đầu xóa/chuẩn bị phân vùng Flash OTA
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      ota_error_text = updateErrorToText(); // Ghi từng khối byte vào Flash
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!Update.end(true)) {
      ota_error_text = updateErrorToText(); // Kết thúc quá trình ghi và xác minh MD5
    } else {
      ota_error_text = "OK";
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort(); // Hủy bỏ nếu rớt mạng giữa chừng
    ota_error_text = "ABORTED";
  }
}

// Handler kết thúc quá trình nạp OTA và khởi động lại ESP32
void ota_update_handle_end(WebServer &server) {
  if (Update.hasError()) {
    server.send(500, "text/plain", ota_error_text);
    return;
  }

  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart(); // Khởi động lại ESP32 để kích hoạt firmware mới vừa nạp
}

String ota_update_get_error_text() {
  return ota_error_text;
}
