#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DATN_AIOT_LETTUCE - ESP32 Control Center</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="layout">
        <!-- Sidebar -->
        <aside class="sidebar">
            <div class="brand">
                <div class="logo">
                    <svg viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M11 20A7 7 0 0 1 9.8 6.1C15.5 5 17 4.48 19 2c1 2 2 3.5 1 9.8a7 7 0 0 1-9 8.2Z"></path>
                        <path d="M9 22v-4H5v-4"></path>
                    </svg>
                </div>
                <span class="brand-name">DATN_AIOT_LETTUCE</span>
            </div>
            
            <nav class="menu">
                <button class="menu-item active" data-tab="dashboard">
                    <span class="icon logo-icon">
                        <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M11 20A7 7 0 0 1 9.8 6.1C15.5 5 17 4.48 19 2c1 2 2 3.5 1 9.8a7 7 0 0 1-9 8.2Z"></path>
                            <path d="M9 22v-4H5v-4"></path>
                        </svg>
                    </span> Dashboard
                </button>
                <button class="menu-item" data-tab="overview">
                    <span class="icon">📊</span> Tổng Quan
                </button>
                <button class="menu-item" data-tab="wifi">
                    <span class="icon">📶</span> Cấu HÌnh WiFi
                </button>
                <button class="menu-item" data-tab="ota">
                    <span class="icon">⚙️</span> Cập Nhật OTA
                </button>
            </nav>
            
            <div class="user-profile">
                <div class="avatar">AD</div>
                <div class="user-info">
                    <div class="user-name">Admin</div>
                    <div class="user-status">Trực tuyến</div>
                </div>
            </div>
        </aside>

        <!-- Sidebar Resizer Splitter -->
        <div class="resizer" id="sidebar-resizer"></div>

        <!-- Main Content Area -->
        <main class="content">
            <header class="content-header">
                <div>
                    <h1 id="page-title">Dashboard Giám Sát</h1>
                    <p class="subtitle" id="system-time">Thiết bị đang chạy ổn định</p>
                </div>
                <div class="status-badge connected" id="connection-status">
                    <span class="status-dot"></span> <span id="status-text">Đã Kết Nối</span>
                </div>
            </header>

            <!-- TAB: DASHBOARD -->
            <section class="tab-content active" id="tab-dashboard">
                <!-- 11 Sensor Cards Grid -->
                <div class="sensor-grid">
                    <!-- 1. Nhiệt độ không khí -->
                    <div class="card sensor-card" id="card-temp">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper temp">🌡️</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Nhiệt Độ KK</span>
                                <h3 class="sensor-value"><span id="dash-temp">--</span> <span class="unit">°C</span></h3>
                                <span class="sensor-status" id="dash-temp-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>
                    
                    <!-- 2. Độ ẩm không khí -->
                    <div class="card sensor-card" id="card-humi">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper humi">💧</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Độ Ẩm KK</span>
                                <h3 class="sensor-value"><span id="dash-humi">--</span> <span class="unit">%</span></h3>
                                <span class="sensor-status" id="dash-humi-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 3. Ánh sáng -->
                    <div class="card sensor-card" id="card-lux">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper lux">☀️</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Ánh Sáng</span>
                                <h3 class="sensor-value"><span id="dash-lux">--</span> <span class="unit">Lux</span></h3>
                                <span class="sensor-status" id="dash-lux-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 4. Nhiệt độ nước -->
                    <div class="card sensor-card" id="card-tempw">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper tempw">🧪🌡️</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Nhiệt Độ Nước</span>
                                <h3 class="sensor-value"><span id="dash-tempw">--</span> <span class="unit">°C</span></h3>
                                <span class="sensor-status" id="dash-tempw-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 5. TDS -->
                    <div class="card sensor-card" id="card-tds">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper tds">📊</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Dinh Dưỡng TDS</span>
                                <h3 class="sensor-value"><span id="dash-tds">--</span> <span class="unit">ppm</span></h3>
                                <span class="sensor-status" id="dash-tds-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 6. pH -->
                    <div class="card sensor-card" id="card-ph">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper ph">🧪</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Độ pH</span>
                                <h3 class="sensor-value"><span id="dash-ph">--</span> <span class="unit">pH</span></h3>
                                <span class="sensor-status" id="dash-ph-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 7. Lưu lượng -->
                    <div class="card sensor-card" id="card-flow">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper flow">🌊</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Lưu Lượng Nước</span>
                                <h3 class="sensor-value"><span id="dash-flow">--</span> <span class="unit">L/m</span></h3>
                                <span class="sensor-status" id="dash-flow-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 8. Mực nước thùng chính -->
                    <div class="card sensor-card" id="card-lvl1">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper lvl">🛢️</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Nước Thùng Chính</span>
                                <h3 class="sensor-value"><span id="dash-lvl1">--</span> <span class="unit">%</span></h3>
                                <span class="sensor-status" id="dash-lvl1-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 9. Mực nước chai 1 -->
                    <div class="card sensor-card" id="card-lvl2">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper lvl">🧴 A</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Mực Nước Chai 1</span>
                                <h3 class="sensor-value"><span id="dash-lvl2">--</span> <span class="unit">%</span></h3>
                                <span class="sensor-status" id="dash-lvl2-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 10. Mực nước chai 2 -->
                    <div class="card sensor-card" id="card-lvl3">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper lvl">🧴 B</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Mực Nước Chai 2</span>
                                <h3 class="sensor-value"><span id="dash-lvl3">--</span> <span class="unit">%</span></h3>
                                <span class="sensor-status" id="dash-lvl3-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>

                    <!-- 11. Mực nước chai 3 -->
                    <div class="card sensor-card" id="card-lvl4">
                        <div class="sensor-card-body">
                            <div class="sensor-icon-wrapper lvl">🧴 pH</div>
                            <div class="sensor-details">
                                <span class="sensor-title">Mực Nước Chai 3</span>
                                <h3 class="sensor-value"><span id="dash-lvl4">--</span> <span class="unit">%</span></h3>
                                <span class="sensor-status" id="dash-lvl4-status">Chờ kết nối...</span>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Bottom Row: Chart and Control Panel -->
                <div class="dashboard-bottom-row">
                    <!-- Top: Control Panel (10 Actuators) -->
                    <div class="card control-card">
                        <div class="card-header">
                            <h2>Bảng Điều Khiển Ngoại Vi</h2>
                        </div>
                        <div class="control-list">
                            <!-- PWM 1 (Đèn LED Tầng 1) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">💡</span>
                                    <div>
                                        <span class="device-name">Đèn LED Tầng 1</span>
                                        <span class="device-desc">Đèn quang hợp tầng 1</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-den1" data-pin="17" min="10" max="100" value="100" disabled>
                                    <span class="pwm-val" id="val-den1">100%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-den1" data-pin="17">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 2 (Đèn LED Tầng 2) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">💡</span>
                                    <div>
                                        <span class="device-name">Đèn LED Tầng 2</span>
                                        <span class="device-desc">Đèn quang hợp tầng 2</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-den2" data-pin="18" min="10" max="100" value="100" disabled>
                                    <span class="pwm-val" id="val-den2">100%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-den2" data-pin="18">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 7 (Bơm pH Down) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🧪</span>
                                    <div>
                                        <span class="device-name">Bơm pH Down</span>
                                        <span class="device-desc">Bơm dung dịch axit hạ pH</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-bomll3" data-pin="8" min="10" max="100" value="30" disabled>
                                    <span class="pwm-val" id="val-bomll3">30%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-bomll3" data-pin="8">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 8 (Bơm nước 12V DC) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🌊</span>
                                    <div>
                                        <span class="device-name">Bơm nước 12V DC</span>
                                        <span class="device-desc">Bơm tuần hoàn nước 12V</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-bom12v" data-pin="9" min="10" max="100" value="60" disabled>
                                    <span class="pwm-val" id="val-bom12v">60%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-bom12v" data-pin="9">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 4 (Quạt Tầng 2) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🌀</span>
                                    <div>
                                        <span class="device-name">Quạt Tầng 2</span>
                                        <span class="device-desc">Quạt thông gió làm mát</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-quat2" data-pin="10" min="10" max="100" value="50" disabled>
                                    <span class="pwm-val" id="val-quat2">50%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-quat2" data-pin="10">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 3 (Quạt Tầng 1) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🌀</span>
                                    <div>
                                        <span class="device-name">Quạt Tầng 1</span>
                                        <span class="device-desc">Quạt thông gió làm mát</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-quat1" data-pin="11" min="10" max="100" value="50" disabled>
                                    <span class="pwm-val" id="val-quat1">50%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-quat1" data-pin="11">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 6 (Bơm DD Chai B) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🧪</span>
                                    <div>
                                        <span class="device-name">Bơm DD Chai B</span>
                                        <span class="device-desc">Bơm tăng dinh dưỡng B</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-bomll2" data-pin="12" min="10" max="100" value="30" disabled>
                                    <span class="pwm-val" id="val-bomll2">30%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-bomll2" data-pin="12">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- PWM 5 (Bơm DD Chai A) -->
                            <div class="control-item pwm-item">
                                <div class="device-info">
                                    <span class="device-icon">🧪</span>
                                    <div>
                                        <span class="device-name">Bơm DD Chai A</span>
                                        <span class="device-desc">Bơm tăng dinh dưỡng A</span>
                                    </div>
                                </div>
                                <div class="pwm-control-area">
                                    <input type="range" class="pwm-slider" id="slider-bomll1" data-pin="13" min="10" max="100" value="30" disabled>
                                    <span class="pwm-val" id="val-bomll1">30%</span>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-bomll1" data-pin="13">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- Relay 2 (Sục khí 220V) -->
                            <div class="control-item">
                                <div class="device-info">
                                    <span class="device-icon">💨</span>
                                    <div>
                                        <span class="device-name">Sục khí 220V</span>
                                        <span class="device-desc">Máy sục khí oxy dinh dưỡng</span>
                                    </div>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-rl2" data-pin="6">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>

                            <!-- Relay 1 (Bơm chìm 220V) -->
                            <div class="control-item">
                                <div class="device-info">
                                    <span class="device-icon">🚰</span>
                                    <div>
                                        <span class="device-name">Bơm chìm 220V</span>
                                        <span class="device-desc">Bơm cấp nước sạch chính</span>
                                    </div>
                                </div>
                                <label class="switch">
                                    <input type="checkbox" class="actuator-toggle" id="ctrl-rl1" data-pin="7">
                                    <span class="slider-switch"></span>
                                </label>
                            </div>
                        </div>
                    </div>

                    <!-- Bottom: Realtime Chart -->
                    <div class="card chart-card">
                        <div class="chart-header">
                            <h2>Biểu Đồ Theo Dõi (Giờ Thực)</h2>
                            <div class="chart-tabs">
                                <button class="chart-tab active" data-sensor="lux">Ánh sáng</button>
                                <button class="chart-tab" data-sensor="temp">Nhiệt độ KK</button>
                                <button class="chart-tab" data-sensor="humi">Độ ẩm KK</button>
                                <button class="chart-tab" data-sensor="temp_w">Nhiệt độ nước</button>
                                <button class="chart-tab" data-sensor="tds">TDS</button>
                                <button class="chart-tab" data-sensor="ph">pH</button>
                            </div>
                        </div>
                        <div class="chart-body">
                            <canvas id="realtimeChart" width="600" height="280"></canvas>
                        </div>
                    </div>
                </div>
            </section>

            <!-- TAB: OVERVIEW -->
            <section class="tab-content" id="tab-overview">
                <div class="grid">
                    <div class="card stat-card">
                        <div class="card-body">
                            <div class="stat-icon info">ℹ️</div>
                            <div>
                                <div class="stat-label">Phiên Bản Firmware</div>
                                <div class="stat-value" id="info-version">v1.0.0</div>
                            </div>
                        </div>
                    </div>
                    <div class="card stat-card">
                        <div class="card-body">
                            <div class="stat-icon wifi">📶</div>
                            <div>
                                <div class="stat-label">WiFi Đang Kết Nối</div>
                                <div class="stat-value" id="info-ssid">-</div>
                            </div>
                        </div>
                    </div>
                    <div class="card stat-card">
                        <div class="card-body">
                            <div class="stat-icon ip">🔗</div>
                            <div>
                                <div class="stat-label">Địa Chỉ IP</div>
                                <div class="stat-value" id="info-ip">0.0.0.0</div>
                            </div>
                        </div>
                    </div>
                    <div class="card stat-card">
                        <div class="card-body">
                            <div class="stat-icon ram">💾</div>
                            <div>
                                <div class="stat-label">Bộ Nhớ Heap Trống</div>
                                <div class="stat-value" id="info-heap">-</div>
                            </div>
                        </div>
                    </div>
                </div>

                <div class="card mt-6">
                    <div class="card-header">
                        <h2>Thông Tin Chi Tiết Hệ Thống</h2>
                    </div>
                    <div class="card-body">
                        <table class="info-table">
                            <tr>
                                <td>Thời gian hoạt động (Uptime)</td>
                                <td id="info-uptime">-</td>
                            </tr>
                            <tr>
                                <td>Chế độ WiFi hiện tại</td>
                                <td id="info-wifimode">-</td>
                            </tr>
                            <tr>
                                <td>Độ mạnh tín hiệu RSSI</td>
                                <td id="info-rssi">-</td>
                            </tr>
                            <tr>
                                <td>Tổng dung lượng bộ nhớ Flash</td>
                                <td id="info-flash-size">-</td>
                            </tr>
                            <tr>
                                <td>Kích thước chương trình (Sketch)</td>
                                <td id="info-sketch-size">-</td>
                            </tr>
                            <tr>
                                <td>Khoảng trống phân vùng nạp</td>
                                <td id="info-free-sketch">-</td>
                            </tr>
                            <tr>
                                <td>Phân vùng Flash đang chạy</td>
                                <td id="info-partition">-</td>
                            </tr>
                        </table>
                    </div>
                </div>
            </section>

            <!-- TAB: WIFI CONFIG -->
            <section class="tab-content" id="tab-wifi">
                <div class="grid grid-2">
                    <div class="card">
                        <div class="card-header">
                            <h2>Kết Nối WiFi Mới</h2>
                        </div>
                        <div class="card-body">
                            <form id="wifi-form">
                                <div class="form-group">
                                    <label for="wifi-ssid">Tên WiFi (SSID)</label>
                                    <div class="input-with-button">
                                        <input type="text" id="wifi-ssid" name="ssid" placeholder="Nhập hoặc chọn WiFi..." required>
                                    </div>
                                </div>
                                <div class="form-group">
                                    <label for="wifi-pass">Mật Khẩu WiFi</label>
                                    <input type="password" id="wifi-pass" name="pass" placeholder="Nhập mật khẩu..." required>
                                </div>
                                <div class="button-group">
                                    <button type="submit" class="btn btn-primary" id="btn-save-wifi">Lưu và Kết Nối</button>
                                    <button type="button" class="btn btn-danger" id="btn-forget-wifi">Quên WiFi & Reset</button>
                                </div>
                            </form>
                        </div>
                    </div>

                    <div class="card">
                        <div class="card-header flex-header">
                            <h2>Danh Sách Mạng WiFi Lân Cận</h2>
                            <button class="btn btn-secondary btn-sm" id="btn-scan">Quét WiFi</button>
                        </div>
                        <div class="card-body">
                            <div id="scan-loading" class="loading" style="display:none;">Đang quét mạng WiFi...</div>
                            <ul class="wifi-list" id="wifi-list">
                                <li class="empty-list">Bấm "Quét WiFi" để tìm các mạng xung quanh</li>
                            </ul>
                        </div>
                    </div>
                </div>
            </section>

            <!-- TAB: OTA UPDATE -->
            <section class="tab-content" id="tab-ota">
                <div class="card max-w-600">
                    <div class="card-header">
                        <h2>Nạp Firmware Không Dây (OTA)</h2>
                    </div>
                    <div class="card-body text-center">
                        <p class="description mb-6">Tải lên file firmware dạng <strong>.bin</strong> đã được biên dịch để cập nhật chương trình cho chip ESP32-S3.</p>
                        
                        <div class="upload-zone" id="drop-zone" onclick="document.getElementById('file-input').click()">
                            <div class="upload-icon">📁</div>
                            <p>Kéo & thả file <strong>.bin</strong> vào đây hoặc click để chọn file</p>
                            <span class="file-name" id="selected-file-name"></span>
                        </div>
                        
                        <input type="file" id="file-input" accept=".bin" style="display:none;">
                        
                        <button class="btn btn-primary mt-6 w-full" id="btn-upload" disabled>Bắt đầu cập nhật</button>
                        
                        <div class="progress-container mt-6" id="progress-container" style="display:none;">
                            <div class="progress-bar">
                                <div class="progress-fill" id="progress-fill"></div>
                            </div>
                            <div class="progress-text" id="progress-text">Đang tải lên... 0%</div>
                        </div>

                        <div class="alert mt-6" id="ota-alert" style="display:none;"></div>
                    </div>
                </div>
            </section>
        </main>
    </div>
    <script src="/script.js"></script>
</body>
</html>
)rawliteral";
const char STYLE_CSS[] PROGMEM = R"rawliteral(:root {
    --bg-color: #e8f7ee; /* Màu nền xanh mint nhạt */
    --card-bg: #ffffff;
    --text-primary: #1f2937;
    --text-secondary: #4b5563;
    --text-muted: #8fa095;
    --primary: #2e7d32; /* Màu xanh lá đậm EcoFarm */
    --primary-hover: #1b5e20;
    --primary-light: rgba(46, 125, 50, 0.1);
    --border-color: #e1e8e4;
    
    --info-color: #2563eb;
    --wifi-color: #7c3aed;
    --ip-color: #d97706;
    --ram-color: #db2777;
    --success: #2e7d32; /* Đồng bộ màu thành công */
    --danger: #dc2626;
    
    --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05), 0 2px 4px -1px rgba(0, 0, 0, 0.03);
    --shadow-md: 0 10px 15px -3px rgba(0, 0, 0, 0.05), 0 4px 6px -2px rgba(0, 0, 0, 0.02);
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
}

body {
    background-color: var(--bg-color);
    color: var(--text-primary);
    min-height: 100vh;
    overflow-x: hidden;
}

.layout {
    display: flex;
    min-height: 100vh;
    height: 100vh;
    overflow: hidden;
}

/* Sidebar Styling */
.sidebar {
    width: 260px;
    min-width: 185px;
    max-width: 400px;
    background-color: #ffffff; /* Nền trắng như ảnh 2 */
    border-right: 1px solid var(--border-color);
    display: flex;
    flex-direction: column;
    padding: 24px;
    flex-shrink: 0;
    height: 100%;
}

.brand {
    display: flex;
    align-items: center;
    gap: 12px;
    margin-bottom: 32px;
    overflow: hidden;
}

.logo {
    display: flex;
    align-items: center;
    justify-content: center;
    color: #2e7d32;
    flex-shrink: 0;
}

.logo svg {
    color: #2e7d32;
    fill: #2e7d32;
    stroke: #2e7d32;
}

.brand-name {
    font-size: 1.05rem; /* Thu nhỏ font chữ để tránh chạm viền */
    font-weight: 700;
    color: #2e7d32;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    letter-spacing: 0.2px;
}

.menu {
    display: flex;
    flex-direction: column;
    gap: 8px;
    flex-grow: 1;
}

.menu-item {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 12px 16px;
    border: none;
    background: none;
    border-radius: 8px;
    color: #4b5563; /* Màu chữ xám tối */
    font-size: 0.95rem;
    font-weight: 500;
    text-align: left;
    cursor: pointer;
    transition: all 0.2s ease;
    width: 100%;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.menu-item .icon {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 20px;
    height: 20px;
    flex-shrink: 0;
    font-size: 1.1rem;
}

.menu-item .icon.logo-icon svg {
    color: #2e7d32;
    stroke: #2e7d32;
    fill: #2e7d32;
}

.menu-item:hover {
    background-color: #e8f7ee;
    color: #2e7d32;
}

.menu-item.active {
    background-color: #d2e8db; /* Màu nền active xanh mint */
    color: #2e7d32;
    font-weight: 600;
}

.user-profile {
    display: flex;
    align-items: center;
    gap: 12px;
    padding-top: 16px;
    border-top: 1px solid var(--border-color);
}

.avatar {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background-color: #d2e8db;
    color: #2e7d32;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: 600;
    font-size: 0.85rem;
    flex-shrink: 0;
}

.user-info {
    min-width: 0;
    flex-grow: 1;
    overflow: hidden;
}

.user-name {
    font-size: 0.9rem;
    font-weight: 600;
    color: #1f2937;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.user-status {
    font-size: 0.75rem;
    color: #10b981;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

/* Sidebar Resizer Splitter */
.resizer {
    width: 4px;
    background-color: var(--border-color);
    cursor: col-resize;
    position: relative;
    z-index: 20;
    transition: background-color 0.2s, width 0.2s;
    align-self: stretch;
    flex-shrink: 0;
}

.resizer:hover, .resizer.dragging {
    background-color: #4caf50;
    width: 6px;
}

/* Main Content Styling */
.content {
    flex-grow: 1;
    height: 100%;
    overflow-y: auto;
    padding: 40px;
    background-color: var(--bg-color);
}

.content-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 32px;
}

h1 {
    font-size: 1.75rem;
    font-weight: 700;
    color: #111827;
}

.subtitle {
    font-size: 0.9rem;
    color: var(--text-secondary);
    margin-top: 4px;
}

.status-badge {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 6px 12px;
    border-radius: 20px;
    font-size: 0.8rem;
    font-weight: 600;
}

.status-badge.connected {
    background-color: rgba(16, 185, 129, 0.1);
    color: var(--primary);
}

.status-badge.ap-mode {
    background-color: rgba(245, 158, 11, 0.1);
    color: var(--ip-color);
}

.status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background-color: currentColor;
    animation: pulse 2s infinite;
}

@keyframes pulse {
    0% { transform: scale(0.95); opacity: 0.5; }
    50% { transform: scale(1.15); opacity: 1; }
    100% { transform: scale(0.95); opacity: 0.5; }
}

/* Cards & Grid Layout */
.grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 20px;
}

.grid-2 {
    grid-template-columns: 1fr 1fr;
}

.card {
    background-color: var(--card-bg);
    border-radius: 12px;
    border: 1px solid var(--border-color);
    box-shadow: var(--shadow);
    overflow: hidden;
}

.card-header {
    padding: 20px 24px;
    border-bottom: 1px solid var(--border-color);
}

.flex-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.card-header h2 {
    font-size: 1.1rem;
    font-weight: 600;
    color: #111827;
}

.card-body {
    padding: 24px;
}

.stat-card {
    padding: 4px;
}

.stat-card .card-body {
    display: flex;
    align-items: center;
    gap: 16px;
}

.stat-icon {
    width: 48px;
    height: 48px;
    border-radius: 12px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 1.25rem;
}

.stat-icon.info { background-color: rgba(59, 130, 246, 0.1); color: var(--info-color); }
.stat-icon.wifi { background-color: rgba(139, 92, 246, 0.1); color: var(--wifi-color); }
.stat-icon.ip { background-color: rgba(245, 158, 11, 0.1); color: var(--ip-color); }
.stat-icon.ram { background-color: rgba(236, 72, 153, 0.1); color: var(--ram-color); }

.stat-label {
    font-size: 0.8rem;
    color: var(--text-secondary);
}

.stat-value {
    font-size: 1.15rem;
    font-weight: 700;
    color: #111827;
    margin-top: 2px;
}

/* Tab contents */
.tab-content {
    display: none;
}

.tab-content.active {
    display: block;
}

/* Info Table */
.info-table {
    width: 100%;
    border-collapse: collapse;
}

.info-table td {
    padding: 12px 0;
    border-bottom: 1px solid var(--border-color);
    font-size: 0.9rem;
}

.info-table tr:last-child td {
    border-bottom: none;
}

.info-table td:first-child {
    color: var(--text-secondary);
    font-weight: 500;
}

.info-table td:last-child {
    text-align: right;
    font-weight: 600;
    color: #111827;
}

/* Form Styling */
.form-group {
    margin-bottom: 20px;
}

.form-group label {
    display: block;
    font-size: 0.85rem;
    font-weight: 600;
    color: var(--text-secondary);
    margin-bottom: 8px;
}

input[type="text"], input[type="password"] {
    width: 100%;
    padding: 12px 16px;
    border-radius: 8px;
    border: 1px solid var(--border-color);
    background-color: #f9fafb;
    color: var(--text-primary);
    font-size: 0.9rem;
    transition: all 0.2s ease;
}

input[type="text"]:focus, input[type="password"]:focus {
    outline: none;
    border-color: var(--primary);
    background-color: #ffffff;
    box-shadow: 0 0 0 3px rgba(16, 185, 129, 0.15);
}

/* Button Styling */
.btn {
    padding: 12px 24px;
    border-radius: 8px;
    font-size: 0.9rem;
    font-weight: 600;
    cursor: pointer;
    border: none;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
}

.btn-sm {
    padding: 8px 16px;
    font-size: 0.8rem;
}

.btn-primary {
    background-color: var(--primary);
    color: #ffffff;
}

.btn-primary:hover {
    background-color: var(--primary-hover);
}

.btn-danger {
    background-color: var(--danger);
    color: #ffffff;
}

.btn-danger:hover {
    background-color: #dc2626;
}

.btn-secondary {
    background-color: #f3f4f6;
    color: var(--text-secondary);
    border: 1px solid var(--border-color);
}

.btn-secondary:hover {
    background-color: #e5e7eb;
}

.button-group {
    display: flex;
    gap: 12px;
    margin-top: 16px;
}

.btn:disabled {
    opacity: 0.6;
    cursor: not-allowed;
}

.w-full {
    width: 100%;
}

.mt-6 { margin-top: 24px; }
.mb-6 { margin-bottom: 24px; }
.max-w-600 { max-width: 600px; }

/* WiFi Scanner List */
.wifi-list {
    list-style: none;
    max-height: 250px;
    overflow-y: auto;
    border: 1px solid var(--border-color);
    border-radius: 8px;
}

.wifi-list li {
    padding: 12px 16px;
    border-bottom: 1px solid var(--border-color);
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 0.9rem;
    cursor: pointer;
    transition: background-color 0.2s;
}

.wifi-list li:last-child {
    border-bottom: none;
}

.wifi-list li:hover:not(.empty-list) {
    background-color: #f9fafb;
}

.wifi-list li.empty-list {
    color: var(--text-muted);
    justify-content: center;
    padding: 24px;
    cursor: default;
}

.wifi-rssi {
    font-size: 0.8rem;
    font-weight: 600;
    color: var(--text-secondary);
}

/* Upload Area Styling */
.upload-zone {
    border: 2px dashed rgba(16, 185, 129, 0.4);
    border-radius: 12px;
    padding: 40px 20px;
    cursor: pointer;
    transition: all 0.2s ease;
    background-color: #f9fafb;
}

.upload-zone:hover {
    border-color: var(--primary);
    background-color: var(--primary-light);
}

.upload-icon {
    font-size: 2.5rem;
    margin-bottom: 12px;
}

.file-name {
    display: block;
    margin-top: 12px;
    font-weight: 600;
    color: var(--primary);
    font-size: 0.9rem;
}

.progress-container {
    background-color: #f3f4f6;
    border-radius: 8px;
    padding: 16px;
    border: 1px solid var(--border-color);
}

.progress-bar {
    width: 100%;
    height: 8px;
    background-color: #e5e7eb;
    border-radius: 4px;
    overflow: hidden;
}

.progress-fill {
    height: 100%;
    width: 0%;
    background-color: var(--primary);
    transition: width 0.1s ease;
}

.progress-text {
    font-size: 0.8rem;
    font-weight: 600;
    text-align: center;
    margin-top: 8px;
    color: var(--text-secondary);
}

.alert {
    padding: 12px 16px;
    border-radius: 8px;
    font-size: 0.85rem;
    font-weight: 500;
}

.alert.success {
    background-color: rgba(16, 185, 129, 0.15);
    color: #065f46;
    border: 1px solid rgba(16, 185, 129, 0.3);
}

.alert.error {
    background-color: rgba(239, 68, 68, 0.15);
    color: #991b1b;
    border: 1px solid rgba(239, 68, 68, 0.3);
}

.alert.warning {
    background-color: rgba(245, 158, 11, 0.15);
    color: #92400e;
    border: 1px solid rgba(245, 158, 11, 0.3);
}

.loading {
    padding: 16px;
    text-align: center;
    color: var(--text-secondary);
    font-size: 0.9rem;
}

/* Responsiveness */
@media (max-width: 768px) {
    .layout {
        flex-direction: column;
        height: auto;
        overflow: visible;
    }
    
    .sidebar {
        width: 100% !important;
        min-width: 100% !important;
        max-width: 100% !important;
        height: auto;
        padding: 20px;
    }
    
    .resizer {
        display: none;
    }
    
    .content {
        padding: 20px;
        height: auto;
        overflow: visible;
    }
    
    .grid-2 {
        grid-template-columns: 1fr;
    }
}

/* Dashboard Tab Specific Styling */
.dashboard-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 24px;
    margin-bottom: 32px;
}

.sensor-card {
    background-color: var(--card-bg);
    border-radius: 16px;
    border: 1px solid var(--border-color);
    box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.05), 0 4px 6px -2px rgba(0, 0, 0, 0.02);
    transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.sensor-card:hover {
    transform: translateY(-2px);
    box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.06), 0 10px 10px -5px rgba(0, 0, 0, 0.03);
}

.sensor-card-body {
    padding: 24px;
    display: flex;
    align-items: center;
    gap: 20px;
}

.sensor-icon-wrapper {
    width: 56px;
    height: 56px;
    border-radius: 14px;
    display: flex;
    align-items: center;
    justify-content: center;
    flex-shrink: 0;
}

.light-card .sensor-icon-wrapper {
    background-color: rgba(245, 158, 11, 0.1);
    color: #d97706;
}

.sensor-icon {
    stroke: currentColor;
}

.sensor-details {
    display: flex;
    flex-direction: column;
    gap: 4px;
}

.sensor-title {
    font-size: 0.85rem;
    font-weight: 600;
    color: var(--text-secondary);
}

.sensor-value {
    font-size: 1.75rem;
    font-weight: 700;
    color: #111827;
}

.sensor-value .unit {
    font-size: 1.1rem;
    font-weight: 500;
    color: var(--text-secondary);
    margin-left: 4px;
}

.sensor-status {
    font-size: 0.8rem;
    font-weight: 600;
    color: var(--text-muted);
}

.status-normal {
    color: #2e7d32;
}

.status-warning {
    color: #d97706;
}

.status-danger {
    color: #dc2626;
}

/* ==========================================================================
   Cải Tiến Dashboard: 11 Thẻ Cảm Biến & Bảng Điều Khiển
   ========================================================================== */

/* Layout Grid cho 11 Thẻ cảm biến */
.sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
    gap: 16px;
    margin-bottom: 24px;
    width: 100%;
}

/* Màu nền & Icon Wrapper cho từng loại cảm biến */
.sensor-icon-wrapper.temp { background-color: rgba(239, 68, 68, 0.1); color: #dc2626; }
.sensor-icon-wrapper.humi { background-color: rgba(59, 130, 246, 0.1); color: #2563eb; }
.sensor-icon-wrapper.lux  { background-color: rgba(245, 158, 11, 0.1); color: #d97706; }
.sensor-icon-wrapper.tempw { background-color: rgba(99, 102, 241, 0.1); color: #4f46e5; }
.sensor-icon-wrapper.tds  { background-color: rgba(139, 92, 246, 0.1); color: #7c3aed; }
.sensor-icon-wrapper.ph   { background-color: rgba(16, 185, 129, 0.1); color: #2e7d32; }
.sensor-icon-wrapper.flow { background-color: rgba(6, 182, 212, 0.1); color: #0891b2; }
.sensor-icon-wrapper.lvl  { background-color: rgba(14, 165, 233, 0.1); color: #0284c7; }

/* Trạng thái mất kết nối của thẻ cảm biến */
.sensor-card.disconnected {
    opacity: 0.6;
    border-color: #fca5a5;
    background-color: #fef2f2;
}
.sensor-card.disconnected .sensor-status {
    color: #ef4444;
}

/* Bố cục dòng dưới: Đồ thị & Bảng điều khiển */
.dashboard-bottom-row {
    display: flex;
    flex-direction: column;
    gap: 24px;
    width: 100%;
    margin-top: 16px;
}

.chart-card, .control-card {
    background-color: var(--card-bg);
    border-radius: 16px;
    border: 1px solid var(--border-color);
    box-shadow: var(--shadow);
    padding: 24px;
}

.chart-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 16px;
    margin-bottom: 20px;
}

.chart-header h2 {
    font-size: 1.2rem;
    font-weight: 700;
    color: var(--primary);
}

.chart-tabs {
    display: flex;
    gap: 4px;
    background-color: #f3f4f6;
    padding: 4px;
    border-radius: 8px;
}

.chart-tab {
    padding: 6px 12px;
    border: none;
    background: none;
    border-radius: 6px;
    font-size: 0.8rem;
    font-weight: 600;
    color: var(--text-secondary);
    cursor: pointer;
    transition: all 0.2s;
}

.chart-tab.active {
    background-color: #ffffff;
    color: var(--primary);
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
}

.chart-body {
    width: 100%;
    position: relative;
}

#realtimeChart {
    width: 100%;
    height: 280px;
    display: block;
}

/* Control Panel List */
.control-card h2 {
    font-size: 1.2rem;
    font-weight: 700;
    color: var(--primary);
    margin-bottom: 20px;
}

.control-list {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    gap: 16px;
}

.control-item {
    display: grid;
    grid-template-areas: 
        "info toggle"
        "pwm pwm";
    grid-template-columns: 1fr auto;
    align-items: center;
    padding: 16px;
    background: #ffffff;
    border: 1px solid var(--border-color);
    border-radius: 12px;
    gap: 12px;
    transition: all 0.2s ease;
}

.control-item:hover {
    border-color: #d2e8db;
    background-color: #fbfdfc;
}

.device-info {
    grid-area: info;
    display: flex;
    align-items: center;
    gap: 12px;
}

.device-icon {
    font-size: 1.3rem;
    display: flex;
    align-items: center;
    justify-content: center;
    width: 36px;
    height: 36px;
    background-color: #e8f7ee;
    border-radius: 8px;
    flex-shrink: 0;
}

.device-name {
    display: block;
    font-size: 0.85rem;
    font-weight: 600;
    color: var(--text-primary);
}

.device-desc {
    display: block;
    font-size: 0.7rem;
    color: var(--text-secondary);
    margin-top: 1px;
}

/* Khu vực thanh trượt PWM */
.pwm-control-area {
    grid-area: pwm;
    display: flex;
    align-items: center;
    gap: 10px;
    width: 100%;
    margin-top: 4px;
}

.pwm-slider {
    flex-grow: 1;
    -webkit-appearance: none;
    appearance: none;
    height: 6px;
    border-radius: 3px;
    background: #e5e7eb;
    outline: none;
    transition: background 0.2s;
    cursor: pointer;
}

.pwm-slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 16px;
    height: 16px;
    border-radius: 50%;
    background: var(--primary);
    cursor: pointer;
    border: 2px solid #ffffff;
    box-shadow: 0 1px 3px rgba(0,0,0,0.15);
    transition: transform 0.1s ease;
}

.pwm-slider::-webkit-slider-thumb:hover {
    transform: scale(1.15);
}

.pwm-slider:disabled {
    background: #e2e8f0;
    cursor: not-allowed;
}

.pwm-slider:disabled::-webkit-slider-thumb {
    background: #cbd5e1;
    cursor: not-allowed;
}

.pwm-val {
    font-size: 0.85rem;
    font-weight: 700;
    color: var(--primary);
    width: 40px;
    text-align: right;
    flex-shrink: 0;
}

.pwm-slider:disabled + .pwm-val {
    color: var(--text-muted);
}

/* Switch Toggle iOS Style */
.switch {
    grid-area: toggle;
    position: relative;
    display: inline-block;
    width: 44px;
    height: 22px;
    flex-shrink: 0;
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

.slider-switch {
    position: absolute;
    cursor: pointer;
    top: 0; left: 0; right: 0; bottom: 0;
    background-color: #cbd5e1;
    transition: .3s;
    border-radius: 22px;
}

.slider-switch:before {
    position: absolute;
    content: "";
    height: 16px;
    width: 16px;
    left: 3px;
    bottom: 3px;
    background-color: white;
    transition: .3s;
    border-radius: 50%;
    box-shadow: 0 1px 3px rgba(0,0,0,0.15);
}

input:checked + .slider-switch {
    background-color: var(--primary);
}

input:checked + .slider-switch:before {
    transform: translateX(22px);
}

@media (max-width: 768px) {
    .sensor-grid {
        grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
    }
}


)rawliteral";
const char SCRIPT_JS[] PROGMEM = R"rawliteral(document.addEventListener("DOMContentLoaded", () => {
    // Elements
    const menuItems = document.querySelectorAll(".menu-item");
    const tabContents = document.querySelectorAll(".tab-content");
    const pageTitle = document.getElementById("page-title");
    const sysTime = document.getElementById("system-time");
    
    const fileInput = document.getElementById("file-input");
    const dropZone = document.getElementById("drop-zone");
    const selectedFileName = document.getElementById("selected-file-name");
    const btnUpload = document.getElementById("btn-upload");
    const progressContainer = document.getElementById("progress-container");
    const progressFill = document.getElementById("progress-fill");
    const progressText = document.getElementById("progress-text");
    const otaAlert = document.getElementById("ota-alert");

    const wifiForm = document.getElementById("wifi-form");
    const wifiSsidInput = document.getElementById("wifi-ssid");
    const wifiPassInput = document.getElementById("wifi-pass");
    const btnScan = document.getElementById("btn-scan");
    const wifiList = document.getElementById("wifi-list");
    const scanLoading = document.getElementById("scan-loading");

    let selectedFile = null;

    // --- Tab Switching ---
    const tabTitles = {
        dashboard: "Dashboard Giám Sát",
        overview: "Tổng Quan Hệ Thống",
        wifi: "Cấu Hình Kết Nối WiFi",
        ota: "Cập Nhật Firmware Không Dây"
    };

    menuItems.forEach(item => {
        item.addEventListener("click", () => {
            const tabId = item.getAttribute("data-tab");
            
            // Update active menu item
            menuItems.forEach(i => i.classList.remove("active"));
            item.classList.add("active");

            // Update active tab content
            tabContents.forEach(content => content.classList.remove("active"));
            document.getElementById(`tab-${tabId}`).classList.add("active");

            // Update page title
            pageTitle.textContent = tabTitles[tabId];
        });
    });

    // --- WebSocket / Polling Real-time logic ---
    let ws = null;
    let pollInterval = null;

    // --- Cấu hình Đồ thị Canvas mượt mà ngoại tuyến ---
    const maxDataPoints = 30;
    const sensorKeys = ["lux", "temp", "humi", "temp_w", "tds", "ph"];
    const chartData = {};
    const chartTimes = Array(maxDataPoints).fill(""); // Khởi tạo mảng mốc thời gian trục X
    
    sensorKeys.forEach(k => {
        chartData[k] = Array(maxDataPoints).fill(null); // Tạo sẵn 30 điểm trống để đồ thị cuộn mượt
    });

    const canvas = document.getElementById("realtimeChart");
    let activeSensor = "lux";
    
    // Lưu tọa độ chuột để vẽ tooltip tương tác
    let mouseX = -1;
    let mouseY = -1;

    if (canvas) {
        canvas.addEventListener("mousemove", (e) => {
            const rect = canvas.getBoundingClientRect();
            // Map tọa độ chuột của trình duyệt sang độ phân giải thực của canvas
            const scaleX = canvas.width / rect.width;
            const scaleY = canvas.height / rect.height;
            mouseX = (e.clientX - rect.left) * scaleX;
            mouseY = (e.clientY - rect.top) * scaleY;
            drawChart();
        });

        canvas.addEventListener("mouseleave", () => {
            mouseX = -1;
            mouseY = -1;
            drawChart();
        });
        
        window.addEventListener("resize", () => {
            drawChart();
        });
    }

    // Lắng nghe sự kiện đổi tab đồ thị cảm biến
    document.querySelectorAll(".chart-tab").forEach(tab => {
        tab.addEventListener("click", () => {
            document.querySelectorAll(".chart-tab").forEach(t => t.classList.remove("active"));
            tab.classList.add("active");
            activeSensor = tab.getAttribute("data-sensor");
            drawChart();
        });
    });

    function drawChart() {
        if (!canvas) return;
        
        // Tự động điều chỉnh độ phân giải canvas theo CSS (responsive)
        const rect = canvas.getBoundingClientRect();
        const displayWidth = Math.floor(rect.width);
        const displayHeight = Math.floor(rect.height);
        if (canvas.width !== displayWidth || canvas.height !== displayHeight) {
            canvas.width = displayWidth;
            canvas.height = displayHeight;
        }
        
        const ctx = canvas.getContext("2d");
        const width = canvas.width;
        const height = canvas.height;
        
        ctx.clearRect(0, 0, width, height);
        
        const data = chartData[activeSensor];
        const validValues = data.filter(v => v !== null && !isNaN(v));
        
        if (validValues.length === 0) {
            ctx.fillStyle = "var(--text-muted)";
            ctx.font = "14px sans-serif";
            ctx.textAlign = "center";
            ctx.fillText("Đang chờ dữ liệu cảm biến...", width / 2, height / 2);
            return;
        }
        
        const padding = { top: 35, right: 30, bottom: 40, left: 60 };
        const graphWidth = width - padding.left - padding.right;
        const graphHeight = height - padding.top - padding.bottom;
        
        let minVal = Math.min(...validValues);
        let maxVal = Math.max(...validValues);
        
        if (minVal === maxVal) {
            minVal -= 10;
            maxVal += 10;
        } else {
            const range = maxVal - minVal;
            minVal -= range * 0.15;
            maxVal += range * 0.15;
        }
        if (minVal < 0 && activeSensor !== "temp") minVal = 0;
        
        // Vẽ lưới tọa độ ngang (Y axis grid)
        const gridLines = 4;
        ctx.strokeStyle = "rgba(46, 125, 50, 0.08)";
        ctx.setLineDash([4, 4]);
        ctx.lineWidth = 1;
        ctx.fillStyle = "var(--text-secondary)";
        ctx.font = "10px sans-serif";
        ctx.textAlign = "right";
        ctx.textBaseline = "middle";
        
        for (let i = 0; i <= gridLines; i++) {
            const yVal = minVal + (maxVal - minVal) * (i / gridLines);
            const y = padding.top + graphHeight - (i / gridLines) * graphHeight;
            
            ctx.beginPath();
            ctx.moveTo(padding.left, y);
            ctx.lineTo(width - padding.right, y);
            ctx.stroke();
            
            let displayVal = yVal;
            if (activeSensor === "lux" || activeSensor === "tds") displayVal = Math.round(yVal);
            else displayVal = yVal.toFixed(1);
            
            ctx.fillText(displayVal, padding.left - 10, y);
        }
        
        // Tính toán tọa độ các điểm
        const points = [];
        const xStep = graphWidth / (maxDataPoints - 1);
        
        for (let i = 0; i < data.length; i++) {
            if (data[i] === null || isNaN(data[i])) continue;
            const x = padding.left + i * xStep;
            const y = padding.top + graphHeight - ((data[i] - minVal) / (maxVal - minVal)) * graphHeight;
            points.push({ x, y, value: data[i], time: chartTimes[i] });
        }
        
        if (points.length === 0) return;
        
        // Vẽ lưới tọa độ dọc (X axis grid) mỗi 5 mốc
        ctx.strokeStyle = "rgba(46, 125, 50, 0.06)";
        for (let i = 0; i < maxDataPoints; i += 5) {
            const x = padding.left + i * xStep;
            ctx.beginPath();
            ctx.moveTo(x, padding.top);
            ctx.lineTo(x, padding.top + graphHeight);
            ctx.stroke();
        }
        
        ctx.setLineDash([]); // Reset nét đứt về nét liền
        
        // Vẽ trục tọa độ
        ctx.strokeStyle = "#cbd5e1";
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.moveTo(padding.left, padding.top);
        ctx.lineTo(padding.left, padding.top + graphHeight);
        ctx.lineTo(width - padding.right, padding.top + graphHeight);
        ctx.stroke();
        
        const primaryColor = "#2e7d32"; // Xanh lá đậm chủ đạo
        
        // Vẽ vùng Gradient dưới đồ thị (Area Fill)
        if (points.length > 1) {
            ctx.beginPath();
            ctx.moveTo(points[0].x, padding.top + graphHeight);
            ctx.lineTo(points[0].x, points[0].y);
            
            for (let i = 0; i < points.length - 1; i++) {
                const p0 = points[i];
                const p1 = points[i+1];
                const cpX1 = p0.x + (p1.x - p0.x) / 3;
                const cpY1 = p0.y;
                const cpX2 = p0.x + 2 * (p1.x - p0.x) / 3;
                const cpY2 = p1.y;
                ctx.bezierCurveTo(cpX1, cpY1, cpX2, cpY2, p1.x, p1.y);
            }
            
            ctx.lineTo(points[points.length - 1].x, padding.top + graphHeight);
            ctx.closePath();
            
            const areaGrad = ctx.createLinearGradient(0, padding.top, 0, padding.top + graphHeight);
            areaGrad.addColorStop(0, "rgba(46, 125, 50, 0.25)");
            areaGrad.addColorStop(1, "rgba(46, 125, 50, 0.0)");
            ctx.fillStyle = areaGrad;
            ctx.fill();
        }
        
        // Vẽ đường cong Spline mượt mà (Line với đổ bóng đổ)
        ctx.save();
        ctx.shadowColor = "rgba(46, 125, 50, 0.3)";
        ctx.shadowBlur = 8;
        ctx.shadowOffsetY = 4;
        
        ctx.strokeStyle = primaryColor;
        ctx.lineWidth = 3;
        ctx.lineCap = "round";
        ctx.lineJoin = "round";
        ctx.beginPath();
        ctx.moveTo(points[0].x, points[0].y);
        
        if (points.length > 1) {
            for (let i = 0; i < points.length - 1; i++) {
                const p0 = points[i];
                const p1 = points[i+1];
                const cpX1 = p0.x + (p1.x - p0.x) / 3;
                const cpY1 = p0.y;
                const cpX2 = p0.x + 2 * (p1.x - p0.x) / 3;
                const cpY2 = p1.y;
                ctx.bezierCurveTo(cpX1, cpY1, cpX2, cpY2, p1.x, p1.y);
            }
        }
        ctx.stroke();
        ctx.restore(); // Khôi phục trạng thái ban đầu để tránh đổ bóng các phần tử khác
        
        // Vẽ các nút tròn dữ liệu
        points.forEach((p, idx) => {
            ctx.fillStyle = "#ffffff";
            ctx.strokeStyle = primaryColor;
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.arc(p.x, p.y, idx === points.length - 1 ? 5 : 3.5, 0, 2 * Math.PI);
            ctx.fill();
            ctx.stroke();
        });
        
        // Vẽ nhãn thời gian trục X
        ctx.fillStyle = "var(--text-muted)";
        ctx.textAlign = "center";
        ctx.textBaseline = "top";
        ctx.font = "9px sans-serif";
        
        // Chỉ in một số mốc thời gian để tránh chen chúc nhãn
        const printIndices = [0, 10, 20, 29];
        printIndices.forEach(idx => {
            if (idx < chartTimes.length && chartTimes[idx] !== "") {
                const x = padding.left + idx * xStep;
                ctx.fillText(chartTimes[idx], x, padding.top + graphHeight + 8);
            }
        });

        // Xử lý tooltip hướng dẫn khi di chuột
        let activePoint = null;
        const hoverThreshold = 15;
        if (mouseX >= padding.left && mouseX <= width - padding.right) {
            let minDist = Infinity;
            points.forEach(p => {
                const dist = Math.abs(p.x - mouseX);
                if (dist < minDist && dist < hoverThreshold) {
                    minDist = dist;
                    activePoint = p;
                }
            });
        }

        if (activePoint) {
            // Vẽ đường guides gióng dọc
            ctx.strokeStyle = "rgba(46, 125, 50, 0.4)";
            ctx.lineWidth = 1;
            ctx.setLineDash([3, 3]);
            ctx.beginPath();
            ctx.moveTo(activePoint.x, padding.top);
            ctx.lineTo(activePoint.x, padding.top + graphHeight);
            ctx.stroke();
            ctx.setLineDash([]);
            
            // Vẽ nút tròn nổi bật
            ctx.fillStyle = primaryColor;
            ctx.strokeStyle = "#ffffff";
            ctx.lineWidth = 2.5;
            ctx.beginPath();
            ctx.arc(activePoint.x, activePoint.y, 6.5, 0, 2 * Math.PI);
            ctx.fill();
            ctx.stroke();
            
            const unit = activeSensor === "temp" || activeSensor === "temp_w" ? "°C" :
                         activeSensor === "humi" ? "%" :
                         activeSensor === "lux" ? "Lux" :
                         activeSensor === "tds" ? "ppm" : "pH";
            
            let valStr = activePoint.value.toFixed(1);
            if (activeSensor === "lux" || activeSensor === "tds") valStr = Math.round(activePoint.value);
            
            const tooltipText = `${valStr} ${unit}`;
            const timeText = activePoint.time || "Đang đo";
            
            ctx.save();
            ctx.shadowColor = "rgba(0, 0, 0, 0.15)";
            ctx.shadowBlur = 8;
            ctx.shadowOffsetY = 4;
            
            ctx.fillStyle = "#1e293b"; // Tooltip màu đen Slate sang trọng
            ctx.strokeStyle = "rgba(255, 255, 255, 0.1)";
            ctx.lineWidth = 1;
            
            const boxWidth = 90;
            const boxHeight = 42;
            let boxX = activePoint.x - boxWidth / 2;
            let boxY = activePoint.y - boxHeight - 12;
            
            if (boxX < padding.left) boxX = padding.left;
            if (boxX + boxWidth > width - padding.right) boxX = width - padding.right - boxWidth;
            if (boxY < padding.top) boxY = activePoint.y + 12;
            
            ctx.beginPath();
            if (ctx.roundRect) {
                ctx.roundRect(boxX, boxY, boxWidth, boxHeight, 6);
            } else {
                ctx.rect(boxX, boxY, boxWidth, boxHeight);
            }
            ctx.fill();
            ctx.stroke();
            ctx.restore();
            
            ctx.fillStyle = "#ffffff";
            ctx.font = "bold 11px sans-serif";
            ctx.textAlign = "center";
            ctx.textBaseline = "top";
            ctx.fillText(tooltipText, boxX + boxWidth / 2, boxY + 6);
            
            ctx.fillStyle = "#94a3b8";
            ctx.font = "9px sans-serif";
            ctx.fillText(timeText, boxX + boxWidth / 2, boxY + 22);
        }
    }

    function pushChartPoint(sensor, val) {
        chartData[sensor].push(val);
        if (chartData[sensor].length > maxDataPoints) {
            chartData[sensor].shift();
        }
    }

    // Cập nhật thẻ hiển thị cảm biến động
    function updateSensorCard(id, val, conn, unit, statusFn) {
        const card = document.getElementById(`card-${id}`);
        const valElem = document.getElementById(`dash-${id}`);
        const statusElem = document.getElementById(`dash-${id}-status`);
        
        if (!card) return;
        
        if (!conn) {
            card.classList.add("disconnected");
            if (valElem) valElem.textContent = "--";
            if (statusElem) {
                statusElem.textContent = "Trạng thái: Mất kết nối";
                statusElem.className = "sensor-status status-danger";
            }
        } else {
            card.classList.remove("disconnected");
            if (valElem) {
                if (id === "lux" || id === "tds") valElem.textContent = Math.round(val);
                else valElem.textContent = val.toFixed(1);
            }
            if (statusElem && statusFn) {
                const info = statusFn(val);
                statusElem.textContent = info.text;
                statusElem.className = `sensor-status ${info.cl}`;
            }
        }
    }

    // --- Cập nhật toàn bộ trạng thái Dashboard ---
    function renderSystemStatus(data) {
        document.getElementById("info-version").textContent = "v" + data.version;
        document.getElementById("info-ssid").textContent = data.ssid || "(Chưa Kết Nối)";
        document.getElementById("info-ip").textContent = data.ip || "0.0.0.0";
        document.getElementById("info-heap").textContent = formatSize(data.heap);
        document.getElementById("info-rssi").textContent = data.rssi ? data.rssi + " dBm" : "-";
        document.getElementById("info-wifimode").textContent = data.wifimode;
        document.getElementById("info-partition").textContent = data.partition;
        
        if (document.getElementById("info-flash-size")) {
            document.getElementById("info-flash-size").textContent = formatSize(data.flash_size);
        }
        if (document.getElementById("info-sketch-size")) {
            document.getElementById("info-sketch-size").textContent = formatSize(data.sketch_size);
        }
        if (document.getElementById("info-free-sketch")) {
            document.getElementById("info-free-sketch").textContent = formatSize(data.free_sketch);
        }

        const uptimeSeconds = Math.floor(data.uptime / 1000);
        const h = Math.floor(uptimeSeconds / 3600);
        const m = Math.floor((uptimeSeconds % 3600) / 60);
        const s = uptimeSeconds % 60;
        document.getElementById("info-uptime").textContent = 
            `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;

        // 1. Cập nhật 11 thẻ cảm biến dựa trên cờ kết nối _conn từ ESP32
        updateSensorCard("temp", data.temp, data.temp_conn, "°C", (val) => {
            if (val < 18 || val > 32) return { text: "Trạng thái: Khẩn cấp", cl: "status-danger" };
            if (val >= 21 && val <= 29) return { text: "Trạng thái: Ổn định", cl: "status-normal" };
            return { text: "Trạng thái: Lưu ý", cl: "status-warning" };
        });
        
        updateSensorCard("humi", data.humi, data.humi_conn, "%", (val) => {
            if (val < 40 || val > 85) return { text: "Trạng thái: Cảnh báo", cl: "status-danger" };
            if (val >= 50 && val <= 80) return { text: "Trạng thái: Tốt", cl: "status-normal" };
            return { text: "Trạng thái: Hơi ẩm/khô", cl: "status-warning" };
        });

        updateSensorCard("lux", data.lux, data.lux_conn, "Lux", (val) => {
            if (val < 100) return { text: "Trạng thái: Quá tối", cl: "status-danger" };
            if (val >= 800 && val <= 2000) return { text: "Trạng thái: Đủ sáng", cl: "status-normal" };
            return { text: "Trạng thái: Ánh sáng lệch", cl: "status-warning" };
        });

        updateSensorCard("tempw", data.temp_w, data.temp_w_conn, "°C", (val) => {
            if (val < 16 || val > 28) return { text: "Trạng thái: Nguy hiểm", cl: "status-danger" };
            if (val >= 20 && val <= 25) return { text: "Trạng thái: Đạt chuẩn", cl: "status-normal" };
            return { text: "Trạng thái: Cần lưu ý", cl: "status-warning" };
        });

        updateSensorCard("tds", data.tds, data.tds_conn, "ppm", (val) => {
            if (val < 500) return { text: "Trạng thái: Thiếu DD", cl: "status-danger" };
            if (val >= 600 && val <= 850) return { text: "Trạng thái: Đạt chuẩn", cl: "status-normal" };
            return { text: "Trạng thái: Hơi lệch", cl: "status-warning" };
        });

        updateSensorCard("ph", data.ph, data.ph_conn, "pH", (val) => {
            if (val < 5.0 || val > 7.0) return { text: "Trạng thái: Nguy hiểm", cl: "status-danger" };
            if (val >= 5.5 && val <= 6.5) return { text: "Trạng thái: Ổn định", cl: "status-normal" };
            return { text: "Trạng thái: Cần điều chỉnh", cl: "status-warning" };
        });

        updateSensorCard("flow", data.flow, data.flow_conn, "L/m", (val) => {
            if (val < 0.5) return { text: "Trạng thái: Ngắt nước", cl: "status-danger" };
            return { text: "Trạng thái: Lưu thông", cl: "status-normal" };
        });

        const levelStatusFn = (val) => {
            if (val < 20) return { text: "Trạng thái: Cạn nước", cl: "status-danger" };
            if (val < 45) return { text: "Trạng thái: Thấp", cl: "status-warning" };
            return { text: "Trạng thái: Đầy đủ", cl: "status-normal" };
        };
        updateSensorCard("lvl1", data.lvl1, data.lvl1_conn, "%", levelStatusFn);
        updateSensorCard("lvl2", data.lvl2, data.lvl2_conn, "%", levelStatusFn);
        updateSensorCard("lvl3", data.lvl3, data.lvl3_conn, "%", levelStatusFn);
        updateSensorCard("lvl4", data.lvl4, data.lvl4_conn, "%", levelStatusFn);

        // 2. Lưu lịch sử đồ thị cảm biến (chỉ khi kết nối tốt, ngược lại đẩy null)
        const now = new Date();
        const timeStr = now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
        chartTimes.push(timeStr);
        if (chartTimes.length > maxDataPoints) {
            chartTimes.shift();
        }

        pushChartPoint("lux", data.lux_conn ? data.lux : null);
        pushChartPoint("temp", data.temp_conn ? data.temp : null);
        pushChartPoint("humi", data.humi_conn ? data.humi : null);
        pushChartPoint("temp_w", data.temp_w_conn ? data.temp_w : null);
        pushChartPoint("tds", data.tds_conn ? data.tds : null);
        pushChartPoint("ph", data.ph_conn ? data.ph : null);
        
        drawChart();

        // 3. Cập nhật trạng thái 10 thiết bị ngoại vi lên UI (nút gạt, thanh trượt)
        const actuators = [
            { id: "den1", pin: 17, key: "act_DEN1", isPwm: true },
            { id: "den2", pin: 18, key: "act_DEN2", isPwm: true },
            { id: "bomll3", pin: 8, key: "act_BOMLL3", isPwm: true },
            { id: "bom12v", pin: 9, key: "act_BOM12V", isPwm: true },
            { id: "quat2", pin: 10, key: "act_QUAT2", isPwm: true },
            { id: "quat1", pin: 11, key: "act_QUAT1", isPwm: true },
            { id: "bomll2", pin: 12, key: "act_BOMLL2", isPwm: true },
            { id: "bomll1", pin: 13, key: "act_BOMLL1", isPwm: true },
            { id: "rl2", pin: 6, key: "act_IN_RL2", isPwm: false },
            { id: "rl1", pin: 7, key: "act_IN_RL1", isPwm: false }
        ];

        actuators.forEach(act => {
            const toggle = document.getElementById(`ctrl-${act.id}`);
            if (!toggle) return;
            
            const stateVal = data[act.key];
            if (stateVal === undefined) return;
            
            const isChecked = act.isPwm ? (stateVal > 0) : (stateVal === 1);
            if (toggle.checked !== isChecked) {
                toggle.checked = isChecked;
            }
            
            if (act.isPwm) {
                const slider = document.getElementById(`slider-${act.id}`);
                const valLabel = document.getElementById(`val-${act.id}`);
                if (slider) {
                    slider.disabled = !isChecked;
                    if (isChecked && stateVal > 0) {
                        const percent = Math.round((stateVal * 100) / 255);
                        slider.value = percent;
                        if (valLabel) valLabel.textContent = `${percent}%`;
                    }
                }
            }
        });

        // Badge trạng thái kết nối
        const statusBadge = document.getElementById("connection-status");
        const statusText = document.getElementById("status-text");
        
        if (data.wifimode === "AP") {
            statusBadge.className = "status-badge ap-mode";
            if (statusText) statusText.textContent = " Chế Độ AP (Cấu Hình)";
            sysTime.textContent = `Hãy truy cập datn.local hoặc 192.168.4.1 để cấu hình`;
        } else {
            statusBadge.className = "status-badge connected";
            if (statusText) statusText.textContent = " Đã Kết Nối";
            sysTime.textContent = `Đang kết nối WiFi: ${data.ssid}`;
        }
    }

    // --- Đăng ký sự kiện điều khiển thiết bị (Toggle & Sliders) ---
    document.querySelectorAll(".actuator-toggle").forEach(toggle => {
        toggle.addEventListener("change", () => {
            const pin = parseInt(toggle.getAttribute("data-pin"));
            const isPwm = toggle.closest(".pwm-item") !== null;
            const id = toggle.id.replace("ctrl-", "");
            
            let sendVal = 0;
            if (toggle.checked) {
                if (isPwm) {
                    const slider = document.getElementById(`slider-${id}`);
                    sendVal = slider ? Math.round((slider.value * 255) / 100) : 255;
                    if (slider) slider.disabled = false;
                } else {
                    sendVal = 1;
                }
            } else {
                if (isPwm) {
                    const slider = document.getElementById(`slider-${id}`);
                    if (slider) slider.disabled = true;
                }
                sendVal = 0;
            }
            
            sendControlCommand(pin, sendVal);
        });
    });

    document.querySelectorAll(".pwm-slider").forEach(slider => {
        const id = slider.id.replace("slider-", "");
        const valLabel = document.getElementById(`val-${id}`);
        const pin = parseInt(slider.getAttribute("data-pin"));
        
        slider.addEventListener("input", () => {
            if (valLabel) {
                valLabel.textContent = `${slider.value}%`;
            }
        });
        
        slider.addEventListener("change", () => {
            const toggle = document.getElementById(`ctrl-${id}`);
            if (toggle && toggle.checked) {
                const sendVal = Math.round((slider.value * 255) / 100);
                sendControlCommand(pin, sendVal);
            }
        });
    });

    function sendControlCommand(pin, state) {
        const payload = JSON.stringify({ pin: pin, state: state });
        console.log("Gửi lệnh điều khiển:", payload);
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(payload);
        } else {
            console.warn("WebSocket chưa kết nối. Không thể gửi lệnh.");
        }
    }

    function updateSystemStatusPolling() {
        fetch("/api/system/status")
            .then(res => res.json())
            .then(data => {
                renderSystemStatus(data);
            })
            .catch(err => {
                console.error("Lỗi lấy thông tin hệ thống qua HTTP Polling:", err);
            });
    }

    let wsReconnectDelay = 2000;

    function startWebSocket() {
        const wsProtocol = window.location.protocol === "https:" ? "wss:" : "ws:";
        const wsUrl = `${wsProtocol}//${window.location.hostname}:81/`;
        
        console.log("Đang kết nối WebSocket đến:", wsUrl);
        ws = new WebSocket(wsUrl);

        ws.onopen = () => {
            console.log("Đã kết nối WebSocket thành công!");
            wsReconnectDelay = 2000;
            if (pollInterval) {
                clearInterval(pollInterval);
                pollInterval = null;
            }
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                renderSystemStatus(data);
            } catch (e) {
                console.error("Lỗi parse dữ liệu WebSocket:", e);
            }
        };

        ws.onclose = () => {
            console.log(`WebSocket bị đóng. Đang kích hoạt HTTP Polling và thử kết nối lại sau ${wsReconnectDelay/1000}s...`);
            if (!pollInterval) {
                pollInterval = setInterval(updateSystemStatusPolling, 3000);
            }
            setTimeout(startWebSocket, wsReconnectDelay);
            wsReconnectDelay = Math.min(wsReconnectDelay * 1.5, 30000);
        };

        ws.onerror = (err) => {
            console.error("Lỗi WebSocket:", err);
            ws.close();
        };
    }

    // Khởi tạo lấy dữ liệu ngay lập tức và bắt đầu WebSocket
    updateSystemStatusPolling();
    startWebSocket();

    // --- WiFi Scan ---
    function pollWifiScanResults() {
        fetch("/api/wifi/scan")
            .then(res => {
                if (res.status === 202) {
                    // Still scanning, wait 1.5s and check again
                    setTimeout(pollWifiScanResults, 1500);
                    return null;
                }
                return res.json();
            })
            .then(networks => {
                if (networks === null) return; // Still scanning
                
                scanLoading.style.display = "none";
                btnScan.disabled = false;

                if (networks.length === 0) {
                    wifiList.innerHTML = `<li class="empty-list">Không tìm thấy mạng WiFi nào</li>`;
                    return;
                }

                networks.forEach(net => {
                    const li = document.createElement("li");
                    li.innerHTML = `
                        <span><strong>${net.ssid}</strong></span>
                        <span class="wifi-rssi">${net.rssi} dBm</span>
                    `;
                    li.addEventListener("click", () => {
                        wifiSsidInput.value = net.ssid;
                        wifiPassInput.focus();
                    });
                    wifiList.appendChild(li);
                });
            })
            .catch(err => {
                scanLoading.style.display = "none";
                btnScan.disabled = false;
                wifiList.innerHTML = `<li class="empty-list" style="color:var(--danger);">Lỗi khi quét mạng WiFi</li>`;
                console.error("Lỗi quét WiFi:", err);
            });
    }

    btnScan.addEventListener("click", () => {
        wifiList.innerHTML = "";
        scanLoading.style.display = "block";
        btnScan.disabled = true;
        pollWifiScanResults();
    });

    // --- WiFi Save Config ---
    wifiForm.addEventListener("submit", (e) => {
        e.preventDefault();
        
        const ssid = wifiSsidInput.value;
        const pass = wifiPassInput.value;
        
        if (!ssid) return;

        const formData = new URLSearchParams();
        formData.append("ssid", ssid);
        formData.append("pass", pass);

        const btnSave = document.getElementById("btn-save-wifi");
        btnSave.disabled = true;
        btnSave.textContent = "Đang lưu...";

        fetch("/api/wifi/save", {
            method: "POST",
            headers: {
                "Content-Type": "application/x-www-form-urlencoded"
            },
            body: formData
        })
        .then(res => {
            if (res.ok) {
                alert(`Đã lưu cấu hình WiFi thành công!\nESP32 sẽ khởi động lại và kết nối vào mạng: "${ssid}".\nHãy kết nối điện thoại/máy tính của bạn vào WiFi này và truy cập vào http://datn.local`);
                wifiForm.reset();
            } else {
                throw new Error("Không thể lưu cấu hình");
            }
        })
        .catch(err => {
            alert("Lỗi khi lưu cấu hình WiFi. Vui lòng thử lại!");
            console.error(err);
        })
        .finally(() => {
            btnSave.disabled = false;
            btnSave.textContent = "Lưu và Kết Nối";
        });
    });

    // --- WiFi Forget Config ---
    const btnForget = document.getElementById("btn-forget-wifi");
    if (btnForget) {
        btnForget.addEventListener("click", () => {
            if (confirm("Bạn có chắc chắn muốn quên WiFi đã lưu và khởi động lại ESP32 về chế độ phát AP (DATN_AIOT_LETTUCE) không?")) {
                btnForget.disabled = true;
                btnForget.textContent = "Đang xóa...";
                
                fetch("/api/wifi/forget", {
                    method: "POST"
                })
                .then(res => {
                    if (res.ok) {
                        alert("Đã xóa cấu hình WiFi! ESP32 đang khởi động lại phát mạng 'DATN_AIOT_LETTUCE'. Hãy kết nối lại.");
                        location.reload();
                    } else {
                        throw new Error("Lỗi khi xóa cấu hình");
                    }
                })
                .catch(err => {
                    alert("Không thể xóa cấu hình WiFi. Vui lòng thử lại!");
                    console.error(err);
                    btnForget.disabled = false;
                    btnForget.textContent = "Quên WiFi & Reset";
                });
            }
        });
    }

    // --- OTA Update File Upload ---
    fileInput.addEventListener("change", (e) => {
        if (e.target.files.length) {
            handleFileSelect(e.target.files[0]);
        }
    });

    dropZone.addEventListener("dragover", (e) => {
        e.preventDefault();
        dropZone.style.borderColor = "var(--primary)";
        dropZone.style.backgroundColor = "var(--primary-light)";
    });

    dropZone.addEventListener("dragleave", () => {
        dropZone.style.borderColor = "";
        dropZone.style.backgroundColor = "";
    });

    dropZone.addEventListener("drop", (e) => {
        e.preventDefault();
        dropZone.style.borderColor = "";
        dropZone.style.backgroundColor = "";
        if (e.dataTransfer.files.length) {
            handleFileSelect(e.dataTransfer.files[0]);
        }
    });

    function handleFileSelect(file) {
        if (!file.name.endsWith(".bin")) {
            showAlert("error", "Chỉ chấp nhận file định dạng .bin!");
            return;
        }
        selectedFile = file;
        selectedFileName.textContent = `${file.name} (${formatSize(file.size)})`;
        btnUpload.disabled = false;
        otaAlert.style.display = "none";
    }

    btnUpload.addEventListener("click", () => {
        if (!selectedFile) return;

        btnUpload.disabled = true;
        progressContainer.style.display = "block";
        progressFill.style.width = "0%";
        progressText.textContent = "Đang tải lên... 0%";
        otaAlert.style.display = "none";

        const xhr = new XMLHttpRequest();
        xhr.open("POST", "/update", true);

        xhr.upload.onprogress = (e) => {
            if (e.lengthComputable) {
                const percent = Math.round((e.loaded / e.total) * 100);
                progressFill.style.width = percent + "%";
                progressText.textContent = `Đang tải lên... ${percent}%`;
            }
        };

        xhr.onload = () => {
            progressContainer.style.display = "none";
            if (xhr.status === 200) {
                showAlert("success", "Cập nhật thành công! ESP32-S3 đang khởi động lại. Vui lòng chờ 5 giây và tải lại trang.");
                setTimeout(() => {
                    location.reload();
                }, 5000);
            } else {
                showAlert("error", `Cập nhật thất bại: ${xhr.responseText || "Lỗi không xác định"}`);
                btnUpload.disabled = false;
            }
        };

        xhr.onerror = () => {
            progressContainer.style.display = "none";
            showAlert("error", "Lỗi đường truyền. Không thể gửi file đến ESP32.");
            btnUpload.disabled = false;
        };

        const formData = new FormData();
        formData.append("firmware", selectedFile);
        xhr.send(formData);
    });

    function showAlert(type, message) {
        otaAlert.className = `alert ${type}`;
        otaAlert.textContent = message;
        otaAlert.style.display = "block";
    }

    function formatSize(bytes) {
        if (bytes === 0) return "0 Bytes";
        const k = 1024;
        const sizes = ["Bytes", "KB", "MB"];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + " " + sizes[i];
    }

    // --- Sidebar Resizing Logic ---
    const resizer = document.getElementById("sidebar-resizer");
    const sidebar = document.querySelector(".sidebar");

    if (resizer && sidebar) {
        let isDragging = false;

        resizer.addEventListener("mousedown", (e) => {
            isDragging = true;
            document.body.style.cursor = "col-resize";
            resizer.classList.add("dragging");
            document.addEventListener("mousemove", resizeSidebar);
            document.addEventListener("mouseup", stopResize);
            e.preventDefault(); // Ngăn chọn văn bản khi kéo
        });

        resizer.addEventListener("touchstart", (e) => {
            isDragging = true;
            resizer.classList.add("dragging");
            document.addEventListener("touchmove", resizeSidebarTouch);
            document.addEventListener("touchend", stopResizeTouch);
        });

        function resizeSidebar(e) {
            if (!isDragging) return;
            let newWidth = e.clientX;
            // Giới hạn chiều rộng từ 185px đến 400px
            if (newWidth < 185) newWidth = 185;
            if (newWidth > 400) newWidth = 400;
            sidebar.style.width = newWidth + "px";
        }

        function resizeSidebarTouch(e) {
            if (!isDragging) return;
            let newWidth = e.touches[0].clientX;
            if (newWidth < 185) newWidth = 185;
            if (newWidth > 400) newWidth = 400;
            sidebar.style.width = newWidth + "px";
        }

        function stopResize() {
            if (isDragging) {
                isDragging = false;
                document.body.style.cursor = "default";
                resizer.classList.remove("dragging");
                document.removeEventListener("mousemove", resizeSidebar);
                document.removeEventListener("mouseup", stopResize);
                localStorage.setItem("sidebar-width", sidebar.style.width);
            }
        }

        function stopResizeTouch() {
            if (isDragging) {
                isDragging = false;
                resizer.classList.remove("dragging");
                document.removeEventListener("touchmove", resizeSidebarTouch);
                document.removeEventListener("touchend", stopResizeTouch);
                localStorage.setItem("sidebar-width", sidebar.style.width);
            }
        }

        // Khôi phục chiều rộng sidebar đã lưu trước đó
        const savedWidth = localStorage.getItem("sidebar-width");
        if (savedWidth) {
            sidebar.style.width = savedWidth;
        }
    }
});
)rawliteral";
