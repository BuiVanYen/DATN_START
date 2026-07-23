#pragma once // Định hướng trình biên dịch chỉ include file header này 1 lần duy nhất

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>

// Enum trạng thái kết nối mạng của vi điều khiển
enum class NetworkState : uint8_t {
  AP_MODE = 0,     // Chế độ phát Wi-Fi Access Point (SoftAP + Captive Portal)
  STA_CONNECTING,  // Đang trong quá trình thử kết nối vào router Wi-Fi nhà
  STA_CONNECTED,   // Đã kết nối Wi-Fi nhà thành công (Station Mode)
};

// Khởi tạo đối tượng Wi-Fi và nạp thông tin lưu trong NVS Flash
void wifi_manager_init();
void wifi_manager_load_credentials();
void wifi_manager_save_credentials(const String &ssid, const String &pass);
void wifi_manager_clear_credentials();

// Bắt đầu phát SoftAP hoặc bắt đầu kết nối STA.
// Trả về true khi Wi-Fi/LwIP đã được khởi tạo thành công.
bool wifi_manager_start_ap();
bool wifi_manager_start_connecting_sta();
void wifi_manager_process_dns(); // Xử lý yêu cầu DNS Captive Portal

// Truy vấn trạng thái Wi-Fi
NetworkState wifi_manager_get_state();
String wifi_manager_get_current_ip();
bool wifi_manager_is_sta_connected();
String wifi_manager_get_ssid();
String wifi_manager_get_pass();
const char *wifi_manager_state_to_text(NetworkState state);
