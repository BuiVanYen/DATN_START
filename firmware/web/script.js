document.addEventListener("DOMContentLoaded", () => {
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
    let manualHeartbeatInterval = null;
    const lastUserInteraction = {}; // Chặn đè trạng thái từ server khi vừa tương tác cơ học

    function updateSensorCard(id, val, conn, statusFn, disconnectedText) {
        const card = document.getElementById(`card-${id}`);
        const valElem = document.getElementById(`dash-${id}`);
        const statusElem = document.getElementById(`dash-${id}-status`);
        
        if (!card) return;
        
        if (!conn) {
            card.classList.add("disconnected");
            if (valElem) valElem.textContent = "--";
            if (statusElem) {
                statusElem.textContent = disconnectedText || "Trạng thái: Mất kết nối";
                statusElem.className = "sensor-status status-danger";
            }
        } else {
            card.classList.remove("disconnected");
            if (valElem) {
                if (id.startsWith("lvl")) {
                    valElem.textContent = (val > 0.5) ? "Còn nước" : "Hết nước";
                } else if (id === "lux" || id === "tds") {
                    valElem.textContent = Math.round(val);
                } else {
                    valElem.textContent = val.toFixed(1);
                }
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

        const modeElem = document.getElementById("info-system-mode");
        const coreElem = document.getElementById("info-task-cores");
        const reasonElem = document.getElementById("info-safe-reason");
        if (modeElem) modeElem.textContent = data.mode || "MANUAL";
        if (coreElem) coreElem.textContent = `Network ${data.net_core} / Sensors ${data.sensor_core} / Control ${data.control_core}`;
        if (reasonElem) reasonElem.textContent = data.last_reason || "-";
        
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

        // 1. Cập nhật 11 thẻ cảm biến dựa trên cờ kết nối _conn từ ESP32 và ngưỡng động từ thresholds.h
        const th_tempa_l = data.th_tempa_l !== undefined ? data.th_tempa_l : 20.0;
        const th_tempa_h = data.th_tempa_h !== undefined ? data.th_tempa_h : 30.0;
        updateSensorCard("temp", data.temp, data.temp_conn, (val) => {
            if (val < th_tempa_l - 2 || val > th_tempa_h + 2) return { text: "Trạng thái: Khẩn cấp", cl: "status-danger" };
            if (val >= th_tempa_l && val <= th_tempa_h) return { text: "Trạng thái: Ổn định", cl: "status-normal" };
            return { text: "Trạng thái: Lưu ý", cl: "status-warning" };
        });
        
        const th_humi_l = data.th_humi_l !== undefined ? data.th_humi_l : 50.0;
        const th_humi_h = data.th_humi_h !== undefined ? data.th_humi_h : 85.0;
        updateSensorCard("humi", data.humi, data.humi_conn, (val) => {
            if (val < th_humi_l - 10 || val > th_humi_h + 10) return { text: "Trạng thái: Cảnh báo", cl: "status-danger" };
            if (val >= th_humi_l && val <= th_humi_h) return { text: "Trạng thái: Tốt", cl: "status-normal" };
            return { text: "Trạng thái: Hơi ẩm/khô", cl: "status-warning" };
        });

        const th_light_l = data.th_light_l !== undefined ? data.th_light_l : 1000.0;
        const th_light_h = data.th_light_h !== undefined ? data.th_light_h : 25000.0;
        updateSensorCard("lux", data.lux, data.lux_conn, (val) => {
            if (val < 100) return { text: "Trạng thái: Quá tối", cl: "status-danger" };
            if (val >= th_light_l && val <= th_light_h) return { text: "Trạng thái: Đủ sáng", cl: "status-normal" };
            return { text: "Trạng thái: Ánh sáng lệch", cl: "status-warning" };
        });

        const th_tempw_l = data.th_tempw_l !== undefined ? data.th_tempw_l : 18.0;
        const th_tempw_h = data.th_tempw_h !== undefined ? data.th_tempw_h : 26.0;
        updateSensorCard("tempw", data.temp_w, data.temp_w_conn, (val) => {
            if (val < th_tempw_l - 2 || val > th_tempw_h + 2) return { text: "Trạng thái: Nguy hiểm", cl: "status-danger" };
            if (val >= th_tempw_l && val <= th_tempw_h) return { text: "Trạng thái: Đạt chuẩn", cl: "status-normal" };
            return { text: "Trạng thái: Cần lưu ý", cl: "status-warning" };
        });

        const th_tds_l = data.th_tds_l !== undefined ? data.th_tds_l : 600.0;
        const th_tds_h = data.th_tds_h !== undefined ? data.th_tds_h : 900.0;
        const tdsRaw = Number(data.tds_raw || 0);
        const tdsQuality = data.tds_quality || "ERROR";
        updateSensorCard("tds", data.tds, data.tds_conn, (val) => {
            if (tdsQuality === "ERROR") {
                return { text: `Trạng thái: Lỗi tín hiệu ADC (raw ${tdsRaw})`, cl: "status-danger" };
            }
            if (tdsQuality === "UNSTABLE") {
                return { text: `Trạng thái: Tín hiệu dao động (raw ${tdsRaw})`, cl: "status-warning" };
            }
            if (tdsRaw <= 50) {
                return { text: `Trạng thái: ADC rất thấp (raw ${tdsRaw}), kiểm tra đầu dò`, cl: "status-warning" };
            }
            if (tdsQuality === "STALE") {
                return { text: "Trạng thái: Đang bù nhiệt mặc định 25°C", cl: "status-warning" };
            }
            if (val < th_tds_l - 100) return { text: "Trạng thái: Thiếu dinh dưỡng", cl: "status-danger" };
            if (val > th_tds_h + 100) return { text: "Trạng thái: Dung dịch quá đậm", cl: "status-danger" };
            if (val >= th_tds_l && val <= th_tds_h) return { text: "Trạng thái: Đạt chuẩn", cl: "status-normal" };
            return { text: val < th_tds_l ? "Trạng thái: Hơi thấp" : "Trạng thái: Hơi cao", cl: "status-warning" };
        }, `Trạng thái: ADC bão hòa (raw ${tdsRaw}), kiểm tra mạch`);

        const th_ph_l = data.th_ph_l !== undefined ? data.th_ph_l : 5.5;
        const th_ph_h = data.th_ph_h !== undefined ? data.th_ph_h : 6.5;
        updateSensorCard("ph", data.ph, data.ph_conn, (val) => {
            if (val < th_ph_l - 0.5 || val > th_ph_h + 0.5) return { text: "Trạng thái: Nguy hiểm", cl: "status-danger" };
            if (val >= th_ph_l && val <= th_ph_h) return { text: "Trạng thái: Ổn định", cl: "status-normal" };
            return { text: "Trạng thái: Cần điều chỉnh", cl: "status-warning" };
        });

        updateSensorCard("flow", data.flow, data.flow_conn, (val) => {
            if (val < 0.5) return { text: "Trạng thái: Ngắt nước", cl: "status-danger" };
            return { text: "Trạng thái: Lưu thông", cl: "status-normal" };
        });

        const levelStatusFn = (val) => {
            if (val > 0.5) return { text: "Trạng thái: Bình thường", cl: "status-normal" };
            return { text: "Trạng thái: Cạn nước", cl: "status-danger" };
        };
        updateSensorCard("lvl1", data.lvl1, data.lvl1_conn, levelStatusFn);
        updateSensorCard("lvl2", data.lvl2, data.lvl2_conn, levelStatusFn);
        updateSensorCard("lvl3", data.lvl3, data.lvl3_conn, levelStatusFn);
        updateSensorCard("lvl4", data.lvl4, data.lvl4_conn, levelStatusFn);

        // 2. Cập nhật trạng thái 10 thiết bị ngoại vi lên UI (nút gạt, thanh trượt)
        const actuators = [
            { id: "den1", actuator: "DEN1", key: "act_DEN1", isPwm: true },
            { id: "den2", actuator: "DEN2", key: "act_DEN2", isPwm: true },
            { id: "bomll3", actuator: "BOMLL3", key: "act_BOMLL3", isPwm: true },
            { id: "bom12v", actuator: "BOM12V", key: "act_BOM12V", isPwm: true },
            { id: "quat2", actuator: "QUAT2", key: "act_QUAT2", isPwm: true },
            { id: "quat1", actuator: "QUAT1", key: "act_QUAT1", isPwm: true },
            { id: "bomll2", actuator: "BOMLL2", key: "act_BOMLL2", isPwm: true },
            { id: "bomll1", actuator: "BOMLL1", key: "act_BOMLL1", isPwm: true },
            { id: "rl2", actuator: "IN_RL2", key: "act_IN_RL2", isPwm: false },
            { id: "rl1", actuator: "IN_RL1", key: "act_IN_RL1", isPwm: false }
        ];

        actuators.forEach(act => {
            const toggle = document.getElementById(`ctrl-${act.id}`);
            if (!toggle) return;
            
            // Nếu người dùng vừa mới tương tác cơ học (< 2.5s), bỏ qua đồng bộ đè từ server để tránh "giật" ngược công tắc
            if (lastUserInteraction[act.id] && (Date.now() - lastUserInteraction[act.id] < 2500)) {
                return;
            }
            
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
            const actuator = toggle.getAttribute("data-actuator");
            const isPwm = toggle.closest(".pwm-item") !== null;
            const id = toggle.id.replace("ctrl-", "");
            
            lastUserInteraction[id] = Date.now(); // Ghi nhận thời gian tương tác người dùng
            
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
            
            sendControlCommand(actuator, sendVal);
        });
    });

    document.querySelectorAll(".pwm-slider").forEach(slider => {
        const id = slider.id.replace("slider-", "");
        const valLabel = document.getElementById(`val-${id}`);
        const actuator = slider.getAttribute("data-actuator");
        
        slider.addEventListener("input", () => {
            if (valLabel) {
                valLabel.textContent = `${slider.value}%`;
            }
        });
        
        slider.addEventListener("change", () => {
            const toggle = document.getElementById(`ctrl-${id}`);
            if (toggle && toggle.checked) {
                lastUserInteraction[id] = Date.now(); // Ghi nhận thời gian tương tác người dùng
                const sendVal = Math.round((slider.value * 255) / 100);
                sendControlCommand(actuator, sendVal);
            }
        });
    });

    function sendControlCommand(actuator, state) {
        const payload = JSON.stringify({ actuator: actuator, state: state });
        console.log("Gửi lệnh điều khiển:", payload);
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(payload);
        } else {
            console.warn("WebSocket chưa kết nối. Thử gửi qua HTTP...");
            fetch("/api/control", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: payload
            })
            .then(res => {
                if (res.ok) {
                    console.log("Gửi lệnh qua HTTP thành công!");
                } else {
                    console.error("Gửi lệnh qua HTTP thất bại:", res.statusText);
                }
            })
            .catch(err => {
                console.error("Lỗi gửi lệnh qua HTTP:", err);
            });
        }
    }

    function sendManualHeartbeat() {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: "heartbeat" }));
            return;
        }

        fetch("/api/heartbeat", { method: "POST" }).catch(() => {
            // Neu HTTP cung mat, lease tren ESP32 se tu tat moi tai.
        });
    }

    function startManualHeartbeat() {
        if (manualHeartbeatInterval) clearInterval(manualHeartbeatInterval);
        sendManualHeartbeat();
        manualHeartbeatInterval = setInterval(sendManualHeartbeat, 2000);
    }

    function updateSystemStatusPolling() {
        fetch("/api/status")
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
            startManualHeartbeat();
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
            
            const statusBadge = document.getElementById("connection-status");
            const statusText = document.getElementById("status-text");
            if (statusBadge) {
                statusBadge.className = "status-badge disconnected";
                if (statusText) statusText.textContent = " Đang kết nối lại...";
            }

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

    // Khởi tạo lấy dữ liệu ngay lập tức, bắt đầu WebSocket và duy trì heartbeat MANUAL.
    // Nếu WebSocket chưa mở được, heartbeat sẽ tự rơi về HTTP POST.
    updateSystemStatusPolling();
    startWebSocket();
    startManualHeartbeat();

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
            if (confirm("Bạn có chắc chắn muốn quên WiFi đã lưu và khởi động lại ESP32 về chế độ phát AP không?")) {
                btnForget.disabled = true;
                btnForget.textContent = "Đang xóa...";
                
                fetch("/api/wifi/forget", {
                    method: "POST"
                })
                .then(res => {
                    if (res.ok) {
                        alert("Đã xóa cấu hình WiFi! ESP32 đang khởi động lại. Hãy kết nối lại.");
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
            e.preventDefault();
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

        const savedWidth = localStorage.getItem("sidebar-width");
        if (savedWidth) {
            sidebar.style.width = savedWidth;
        }
    }
});
