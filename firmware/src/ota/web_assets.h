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
                <button class="menu-item active" data-tab="overview">
                    <span class="icon">📊</span> Tổng Quan
                </button>
                <button class="menu-item" data-tab="wifi">
                    <span class="icon">📶</span> Cấu Hình WiFi
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
                    <h1 id="page-title">Tổng Quan Hệ Thống</h1>
                    <p class="subtitle" id="system-time">Thiết bị đang chạy ổn định</p>
                </div>
                <div class="status-badge connected" id="connection-status">
                    <span class="status-dot"></span> <span id="status-text">Đã Kết Nối</span>
                </div>
            </header>

            <!-- TAB: OVERVIEW -->
            <section class="tab-content active" id="tab-overview">
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
    --primary: #4caf50; /* Thay đổi primary sang màu xanh #4caf50 */
    --primary-hover: #43a047;
    --primary-light: rgba(76, 175, 80, 0.1);
    --border-color: #d2e8db;
    
    --info-color: #2563eb;
    --wifi-color: #7c3aed;
    --ip-color: #d97706;
    --ram-color: #db2777;
    --success: #4caf50; /* Đồng bộ màu thành công sang xanh #4caf50 */
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
    background-color: #4caf50; /* Màu nền xanh lá #4caf50 */
    border-right: 1px solid rgba(0, 0, 0, 0.05);
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
    color: #ffffff;
    flex-shrink: 0;
}

.logo svg {
    color: #ffffff;
    fill: none;
    stroke: #ffffff;
}

.brand-name {
    font-size: 1.05rem; /* Thu nhỏ font chữ để tránh chạm viền */
    font-weight: 700;
    color: #ffffff;
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
    color: rgba(255, 255, 255, 0.85); /* Chữ trắng mờ tương phản tốt */
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

.menu-item:hover {
    background-color: rgba(255, 255, 255, 0.15); /* Hover sáng lên nhẹ */
    color: #ffffff;
}

.menu-item.active {
    background-color: rgba(255, 255, 255, 0.25); /* Active sáng rõ */
    color: #ffffff;
    font-weight: 600;
}

.user-profile {
    display: flex;
    align-items: center;
    gap: 12px;
    padding-top: 16px;
    border-top: 1px solid rgba(255, 255, 255, 0.2);
}

.avatar {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background-color: rgba(255, 255, 255, 0.2);
    color: #ffffff;
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
    color: #ffffff;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.user-status {
    font-size: 0.75rem;
    color: rgba(255, 255, 255, 0.9);
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
    background-color: #f9fafb; /* Tạo màu xám nhạt nhẹ cho vùng content dễ nhìn */
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

    function renderSystemStatus(data) {
        document.getElementById("info-version").textContent = "v" + data.version;
        document.getElementById("info-ssid").textContent = data.ssid || "(Chưa Kết Nối)";
        document.getElementById("info-ip").textContent = data.ip || "0.0.0.0";
        document.getElementById("info-heap").textContent = formatSize(data.heap);
        document.getElementById("info-rssi").textContent = data.rssi ? data.rssi + " dBm" : "-";
        document.getElementById("info-wifimode").textContent = data.wifimode;
        document.getElementById("info-partition").textContent = data.partition;
        
        // Hiển thị thông tin Flash
        if (document.getElementById("info-flash-size")) {
            document.getElementById("info-flash-size").textContent = formatSize(data.flash_size);
        }
        if (document.getElementById("info-sketch-size")) {
            document.getElementById("info-sketch-size").textContent = formatSize(data.sketch_size);
        }
        if (document.getElementById("info-free-sketch")) {
            document.getElementById("info-free-sketch").textContent = formatSize(data.free_sketch);
        }

        // Format uptime
        const uptimeSeconds = Math.floor(data.uptime / 1000);
        const h = Math.floor(uptimeSeconds / 3600);
        const m = Math.floor((uptimeSeconds % 3600) / 60);
        const s = uptimeSeconds % 60;
        document.getElementById("info-uptime").textContent = 
            `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;

        // Update connection status header badge
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
            wsReconnectDelay = 2000; // Reset delay on successful connection
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
            // Exponential backoff with max of 30s
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
