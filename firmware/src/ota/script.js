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
