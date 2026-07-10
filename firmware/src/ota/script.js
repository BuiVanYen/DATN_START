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
            { id: "rl1", pin: 7, key: "act_IN_RL1", isPwm: false },
            { id: "rl2", pin: 6, key: "act_IN_RL2", isPwm: false },
            { id: "den1", pin: 17, key: "act_DEN1", isPwm: true },
            { id: "den2", pin: 18, key: "act_DEN2", isPwm: true },
            { id: "quat1", pin: 11, key: "act_QUAT1", isPwm: true },
            { id: "quat2", pin: 10, key: "act_QUAT2", isPwm: true },
            { id: "bomll1", pin: 13, key: "act_BOMLL1", isPwm: true },
            { id: "bomll2", pin: 12, key: "act_BOMLL2", isPwm: true },
            { id: "bomll3", pin: 8, key: "act_BOMLL3", isPwm: true },
            { id: "bom12v", pin: 9, key: "act_BOM12V", isPwm: true }
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
