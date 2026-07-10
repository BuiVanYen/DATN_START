// ==========================================================================
// SYSTEM BLOCK DIAGRAM DATABASES (TECHNICAL SPECIFICATIONS & PINOUTS)
// ==========================================================================

const nodeData = {
    "node-psu": {
        title: "Nguồn Tổ Ong 12V 20A",
        layer: "Nguồn Hệ Thống",
        layerClass: "text-yellow",
        icon: "fa-car-battery",
        iconBg: "var(--accent-power)",
        specs: `
            <p class="detail-desc">Bộ nguồn chuyển đổi chính cung cấp năng lượng DC ổn định cho toàn bộ hệ thống (bơm, quạt, đèn LED, và mạch điều khiển).</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Điện áp đầu vào</span><span class="value">110V / 220V AC</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp đầu ra</span><span class="value">12V DC ± 5%</span></div>
                <div class="detail-spec-item"><span class="label">Dòng cực đại</span><span class="value">20A (240W)</span></div>
                <div class="detail-spec-item"><span class="label">Bảo vệ nguồn</span><span class="value">Cầu chì 20A + TVS Diode SMBJ15A</span></div>
                <div class="detail-spec-item"><span class="label">Hiệu suất chuyển đổi</span><span class="value">~85%</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Vai trò cấp nguồn</h4>
                <p>Nguồn 12V được phân phối trực tiếp tới các van solenoid, bơm peristaltic 12V, quạt gió 12V, và cuộn hút Relay. Nguồn 12V cũng đi qua LM2596 để hạ xuống 5V cấp cho cảm biến và MCU.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Cổng Terminal</th><th>Mô tả kết nối</th><th>Điện áp</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">L / N / G</span></td><td>Đầu vào nguồn lưới điện xoay chiều AC</td><td>220V AC</td></tr>
                    <tr><td><span class="pin-badge">+V1 / +V2</span></td><td>Đầu ra cực dương 12V (Cấp cho Rail +12V chính)</td><td>12V DC</td></tr>
                    <tr><td><span class="pin-badge">-V1 / -V2</span></td><td>Đầu ra cực âm GND chung (Ground plane PCB)</td><td>0V (GND)</td></tr>
                    <tr><td><span class="pin-badge">FUSE 20A</span></td><td>Cầu chì bảo vệ quá dòng ở đầu vào 12V</td><td>-</td></tr>
                    <tr><td><span class="pin-badge">SS54 Diode</span></td><td>Chống phân cực ngược bảo vệ cháy PCB chính</td><td>-</td></tr>
                </tbody>
            </table>
        `
    },
    "node-sht31": {
        title: "Cảm Biến SHT31",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-temperature-half",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Cảm biến đo nhiệt độ và độ ẩm không khí môi trường xung quanh giàn trồng xà lách thủy canh, giao tiếp bằng giao thức I2C tốc độ cao.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Model</span><span class="value">Sensirion SHT31-D</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo nhiệt độ</span><span class="value">-40°C đến 125°C (Sai số ±0.2°C)</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo độ ẩm</span><span class="value">0% đến 100% RH (Sai số ±2%)</span></div>
                <div class="detail-spec-item"><span class="label">Giao thức truyền</span><span class="value">I2C (Địa chỉ mặc định: 0x44)</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp hoạt động</span><span class="value">2.4V - 5.5V DC (Cấp 3.3V)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Hiệu chỉnh phần cứng</h4>
                <p>Nằm trên giàn trồng, bọc trong hộp bảo vệ thông thoáng khí để tránh giọt bắn dung dịch nước ảnh hưởng đến độ bền của cảm biến.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Chân SHT31</th><th>GPIO ESP32-S3</th><th>Chức năng</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">VCC</span></td><td>3.3V (Pin MCU)</td><td>Nguồn nuôi 3.3V cực kỳ ổn định</td></tr>
                    <tr><td><span class="pin-badge">GND</span></td><td>GND chung</td><td>Đất hệ thống</td></tr>
                    <tr><td><span class="pin-badge">SDA</span></td><td><span class="pin-badge">GPIO 17</span></td><td>Đường truyền dữ liệu I2C Bus</td></tr>
                    <tr><td><span class="pin-badge">SCL</span></td><td><span class="pin-badge">GPIO 18</span></td><td>Đường truyền xung nhịp I2C Bus</td></tr>
                </tbody>
            </table>
        `
    },
    "node-ds18b20": {
        title: "Đầu Dò Nhiệt DS18B20",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-thermometer",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Cảm biến đo nhiệt độ nước dinh dưỡng dạng đầu dò kim loại chống nước IP68. Được tích hợp vật lý chung đầu cắm với module TDS Analog.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Kiểu đóng gói</span><span class="value">Đầu dò kim loại phi 6mm chống gỉ</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo nhiệt độ</span><span class="value">-55°C đến 125°C (Sai số ±0.5°C)</span></div>
                <div class="detail-spec-item"><span class="label">Giao tiếp kết nối</span><span class="value">OneWire (1 dây tín hiệu chung bus)</span></div>
                <div class="detail-spec-item"><span class="label">Độ phân giải tín hiệu</span><span class="value">9 đến 12 bit (Lựa chọn lập trình)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Tầm quan trọng trong hệ thống</h4>
                <p>Nhiệt độ nước quyết định trực tiếp tới khả năng hòa tan oxy của bể chứa. Nếu nhiệt độ nước >26°C, xà lách rất dễ bị nấm rễ tấn công. Đồng thời giá trị nhiệt độ nước dùng để hiệu chuẩn bù sai số cho cảm biến TDS và pH.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Chân Cảm Biến</th><th>GPIO ESP32-S3</th><th>Mô tả mạch</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">VCC</span></td><td>3.3V</td><td>Cấp nguồn từ MCU</td></tr>
                    <tr><td><span class="pin-badge">GND</span></td><td>GND chung</td><td>Đất hệ thống</td></tr>
                    <tr><td><span class="pin-badge">DATA</span></td><td><span class="pin-badge">GPIO 8</span></td><td>Bus 1-Wire (Cần trở kéo lên Pull-up 4.7kΩ lên VCC)</td></tr>
                </tbody>
            </table>
        `
    },
    "node-tds": {
        title: "Cảm Biến Gravity TDS",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-vial-circle-check",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Cảm biến đo nồng độ chất rắn hòa tan TDS (Total Dissolved Solids) dùng để xác định nồng độ phân bón dinh dưỡng trong bể chứa nước tuần hoàn.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Hãng sản xuất</span><span class="value">DFRobot Gravity TDS</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo TDS</span><span class="value">0 đến 1000 ppm (Sai số ±10% F.S)</span></div>
                <div class="detail-spec-item"><span class="label">Tín hiệu đầu ra</span><span class="value">Analog 0V - 2.3V</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp hoạt động</span><span class="value">5V DC (Cấp nguồn từ LM2596)</span></div>
                <div class="detail-spec-item"><span class="label">Đầu cắm điện cực</span><span class="value">2 kim dò kim loại chống mòn</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Hiệu chuẩn chống quá tải ADC</h4>
                <p>Do module chạy nguồn 5V xuất tín hiệu 0-5V, trên PCB thiết kế mạch cầu phân áp (R1=10k, R2=15k) để hạ mức điện áp tối đa về 3.0V, tránh làm hỏng chân ADC ESP32-S3. Đồng thời tích hợp bộ lọc RC thông thấp (1k / 100nF) khử nhiễu tần số cao.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Đầu Connector</th><th>GPIO ESP32-S3</th><th>Chức năng mạch</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">VCC</span></td><td>+5V Rail</td><td>Nguồn nuôi module khuếch đại</td></tr>
                    <tr><td><span class="pin-badge">GND</span></td><td>GND chung</td><td>Đất chung nhiễu thấp</td></tr>
                    <tr><td><span class="pin-badge">SIGNAL</span></td><td><span class="pin-badge">GPIO 10</span></td><td>Kênh Analog ADC1_CH9 (Không bị tắt khi bật WiFi)</td></tr>
                </tbody>
            </table>
        `
    },
    "node-ph": {
        title: "Cảm Biến pH V2",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-flask-vial",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Đầu dò pH công nghiệp đo nồng độ ion H+ trong nước để theo dõi tính axit/kiềm, đảm bảo xà lách thủy canh sinh trưởng ở điều kiện tối ưu nhất (pH 5.5 - 6.5).</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Model</span><span class="value">DFRobot Gravity pH V2 (PH0-14)</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo pH</span><span class="value">0 - 14 pH (Sai số ±0.1 pH ở 25°C)</span></div>
                <div class="detail-spec-item"><span class="label">Thời gian phản hồi</span><span class="value">< 60 giây</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp cấp</span><span class="value">5V DC (LM2596 trên PCB)</span></div>
                <div class="detail-spec-item"><span class="label">Hiệu chuẩn</span><span class="value">2 điểm (mốc chuẩn pH 4.0 và 7.0)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Chống nhiễu điện từ</h4>
                <p>Tương tự cảm biến TDS, tín hiệu pH analog 5V được đưa qua cầu phân áp hạ xuống mức 3.3V và bộ lọc RC thông thấp trước khi đi vào chip ESP32. Điện cực pH nhạy cảm với dòng điện dò từ các bơm chìm 220V nên cần nối mát bể chứa.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Đầu Connector</th><th>GPIO ESP32-S3</th><th>Chức năng mạch</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">VCC</span></td><td>+5V Rail</td><td>Cấp nguồn cho Op-Amp trên module</td></tr>
                    <tr><td><span class="pin-badge">GND</span></td><td>GND chung</td><td>Đất hệ thống bảo vệ tín hiệu</td></tr>
                    <tr><td><span class="pin-badge">SIGNAL</span></td><td><span class="pin-badge">GPIO 9</span></td><td>Kênh Analog ADC1_CH8 (Hoạt động tốt khi bật WiFi)</td></tr>
                </tbody>
            </table>
        `
    },
    "node-bh1750": {
        title: "Cảm Biến Ánh Sáng BH1750",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-sun",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Cảm biến cường độ ánh sáng kỹ thuật số đo lượng quang năng nhận được trên giàn xà lách thủy canh NFT, từ đó quyết định bật tắt đèn LED Grow bổ trợ.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Model cảm biến</span><span class="value">BH1750FVI (Đầu ra số Lux trực tiếp)</span></div>
                <div class="detail-spec-item"><span class="label">Dải đo cường độ</span><span class="value">1 đến 65535 Lux (Độ phân giải 1 lux)</span></div>
                <div class="detail-spec-item"><span class="label">Giao thức</span><span class="value">I2C (Địa chỉ mặc định: 0x23)</span></div>
                <div class="detail-spec-item"><span class="label">Bù trừ phổ nguồn sáng</span><span class="value">Tích hợp sẵn, không kén nguồn sáng</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Bố trí thiết bị</h4>
                <p>Gắn ở tầng 2 giàn xà lách, hướng trực tiếp lên trời để đo cường độ ánh sáng tự nhiên chính xác nhất.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Chân BH1750</th><th>GPIO ESP32-S3</th><th>Mô tả mạch</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">VCC</span></td><td>3.3V (MCU)</td><td>Nguồn hoạt động</td></tr>
                    <tr><td><span class="pin-badge">GND</span></td><td>GND chung</td><td>Đất hệ thống</td></tr>
                    <tr><td><span class="pin-badge">SDA</span></td><td><span class="pin-badge">GPIO 17</span></td><td>Mắc song song Bus I2C với SHT31 (Pull-up 4.7kΩ)</td></tr>
                    <tr><td><span class="pin-badge">SCL</span></td><td><span class="pin-badge">GPIO 18</span></td><td>Mắc song song Bus I2C với SHT31 (Pull-up 4.7kΩ)</td></tr>
                    <tr><td><span class="pin-badge">ADDR</span></td><td>GND (0V)</td><td>Chọn địa chỉ I2C = 0x23 (Nếu nối 3.3V địa chỉ = 0x5C)</td></tr>
                </tbody>
            </table>
        `
    },
    "node-y26": {
        title: "Cảm Biến Mực Nước Y26",
        layer: "Tầng Cảm Biến",
        layerClass: "text-cyan",
        icon: "fa-ruler-vertical",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Hệ thống 4 cảm biến đo mực nước không tiếp xúc Y26 PNP dán bên ngoài thành bồn chứa nhựa. Đo đạc các mức nước 25%, 50%, 75%, 100% để chống cạn bể chứa.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Model cảm biến</span><span class="value">XKC-Y26-PNP (Đầu ra PNP cực thu hở)</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp hoạt động</span><span class="value">5V - 24V DC (Sử dụng nguồn 12V chính)</span></div>
                <div class="detail-spec-item"><span class="label">Loại vật liệu thành bể</span><span class="value">Nhựa, thuỷ tinh (Bề dày thành bể tối đa 20mm)</span></div>
                <div class="detail-spec-item"><span class="label">Trạng thái đầu ra</span><span class="value">High (12V) khi phát hiện nước | Low (0V) khi trống</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Mạch giao tiếp trên PCB</h4>
                <p>Do cảm biến xuất tín hiệu 12V ra chân OUT, trên PCB tích hợp mạch cầu phân áp R10k/R3.3k cho mỗi kênh để hạ mức áp 12V xuống 3.0V trước khi cấp vào chân ESP32 bảo vệ an toàn cho MCU.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Cảm biến</th><th>GPIO ESP32-S3</th><th>Mức nước giám sát</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">Y26 - Số 1</span></td><td><span class="pin-badge">GPIO 13</span></td><td>Mức nước thấp nhất (25% - Cảnh báo cạn tắt bơm)</td></tr>
                    <tr><td><span class="pin-badge">Y26 - Số 2</span></td><td><span class="pin-badge">GPIO 14</span></td><td>Mức nước trung bình thấp (50%)</td></tr>
                    <tr><td><span class="pin-badge">Y26 - Số 3</span></td><td><span class="pin-badge">GPIO 21</span></td><td>Mức nước trung bình cao (75%)</td></tr>
                    <tr><td><span class="pin-badge">Y26 - Số 4</span></td><td><span class="pin-badge">GPIO 47</span></td><td>Mức nước đầy (100% - Ngắt bơm châm nước máy)</td></tr>
                </tbody>
            </table>
        `
    },
    "node-mcu": {
        title: "Mạch Điều Khiển PCB ESP32-S3",
        layer: "Mạch Xử Lý Trung Tâm",
        layerClass: "text-emerald",
        icon: "fa-microchip",
        iconBg: "var(--accent-controller)",
        specs: `
            <p class="detail-desc">Board mạch in (PCB) 2 lớp tự vẽ thiết kế dạng SMD tích hợp trực tiếp module ESP32-S3-WROOM-1-N8R8, cách ly nguồn cơ điện tử và bảo vệ quá dòng.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Vi điều khiển</span><span class="value">ESP32-S3 dual-core Xtensa 32-bit LX7 (240MHz)</span></div>
                <div class="detail-spec-item"><span class="label">Bộ nhớ tích hợp</span><span class="value">8MB Flash (Quad SPI) + 8MB Octal PSRAM</span></div>
                <div class="detail-spec-item"><span class="label">Nguồn hạ áp tích hợp</span><span class="value">LM2596 (12V->5V/3A) + AMS1117-3.3 (5V->3.3V/0.8A)</span></div>
                <div class="detail-spec-item"><span class="label">Giao tiếp nạp code</span><span class="value">UART Header (Không dùng chip CH340 trên board)</span></div>
                <div class="detail-spec-item"><span class="label">Tính năng bảo vệ</span><span class="value">Cách ly quang Relay PC817, TVS chống sét lan truyền</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Lưu ý GPIO nguy hiểm</h4>
                <p>Do sử dụng module Octal PSRAM, các chân GPIO từ 26 đến 37 bị cấm sử dụng vì đã được kết nối nội bộ với PSRAM và Flash. Mạch PCB chỉ định dạng chân ra các GPIO trống an toàn.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Nhóm chức năng</th><th>GPIO Assigned</th><th>Kiểu tín hiệu / Mô tả</th></tr>
                </thead>
                <tbody>
                    <tr><td>Analog Sensors</td><td><span class="pin-badge">GPIO 9</span>, <span class="pin-badge">GPIO 10</span></td><td>Đọc giá trị pH, TDS (ADC1)</td></tr>
                    <tr><td>I2C / 1-Wire</td><td><span class="pin-badge">GPIO 17</span>, <span class="pin-badge">18</span>, <span class="pin-badge">8</span></td><td>SHT31, BH1750, DS18B20</td></tr>
                    <tr><td>Y26 Mực nước</td><td><span class="pin-badge">GPIO 13</span>, <span class="pin-badge">14</span>, <span class="pin-badge">21</span>, <span class="pin-badge">47</span></td><td>Đọc digital PNP (Hạ áp qua điện trở)</td></tr>
                    <tr><td>PWM MOSFET</td><td><span class="pin-badge">GPIO 38,39,1,3,40,41,42,2</span></td><td>8 Kênh: 3 Bơm DD + 1 Bơm chìm 12V + 4 Quạt</td></tr>
                    <tr><td>Relay Control</td><td><span class="pin-badge">GPIO 4,5,6,7,15,16</span></td><td>6 Relay: 1 Bơm 220V + 1 Sục 220V + 4 Đèn (PC817+S8050)</td></tr>
                    <tr><td>UART Code Upload</td><td><span class="pin-badge">GPIO 43 (TX)</span>, <span class="pin-badge">44 (RX)</span></td><td>Kết nối Header 4P nạp chương trình</td></tr>
                </tbody>
            </table>
        `
    },
    "node-pwm": {
        title: "Khối PWM MOSFET IRLZ44N",
        layer: "Giao Tiếp Điều Khiển",
        layerClass: "text-orange",
        icon: "fa-wave-square",
        iconBg: "var(--accent-driver)",
        specs: `
            <p class="detail-desc">Khối chấp hành bán dẫn điều khiển công suất động cơ 12V bằng băm xung PWM tần số cao sử dụng MOSFET dòng lớn Logic-Level. Điều khiển 3 bơm châm dung dịch (A, B, pH Down) + 1 bơm chìm 12V + 4 quạt thông gió.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Linh kiện MOSFET</span><span class="value">IRLZ44N (Kênh N, dòng cực đại 47A, TO-220)</span></div>
                <div class="detail-spec-item"><span class="label">Điện áp mở Gate</span><span class="value">Vgs(th) = 1.0V - 2.0V (Kích mở tốt bằng 3.3V MCU)</span></div>
                <div class="detail-spec-item"><span class="label">Diode triệt dòng ngược</span><span class="value">Schottky 1N5819 dập ngược EMF từ motor cuộn dây</span></div>
                <div class="detail-spec-item"><span class="label">Điện trở kích Gate</span><span class="value">100Ω (hạn dòng sạc tụ Gate) + 10kΩ (tránh thả nổi chân)</span></div>
                <div class="detail-spec-item"><span class="label">Tổng số kênh điều khiển</span><span class="value">8 Kênh (3 bơm DD + 1 Bơm chìm 12V + 4 quạt)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Ưu điểm điều khiển PWM</h4>
                <p>Bơm châm dung dịch dinh dưỡng cần kiểm soát lưu lượng siêu nhỏ (ml). Nhờ băm xung PWM từ ESP32, ta có thể điều khiển bơm châm chính xác lượng dung dịch A, B và dung dịch chỉnh pH Down mà không làm xáo trộn nồng độ. Bơm chìm 12V điều chỉnh lưu lượng tuần hoàn linh hoạt nhờ PWM MOSFET.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Kênh MOSFET</th><th>GPIO ESP32-S3</th><th>Thiết bị ngoại vi kết nối</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">PWM 1</span></td><td><span class="pin-badge">GPIO 38</span></td><td>Bơm peristaltic 12V châm dung dịch A</td></tr>
                    <tr><td><span class="pin-badge">PWM 2</span></td><td><span class="pin-badge">GPIO 39</span></td><td>Bơm peristaltic 12V châm dung dịch B</td></tr>
                    <tr><td><span class="pin-badge">PWM 3</span></td><td><span class="pin-badge">GPIO 1</span></td><td>Bơm peristaltic 12V châm pH Down</td></tr>
                    <tr><td><span class="pin-badge">PWM 4</span></td><td><span class="pin-badge">GPIO 3</span></td><td>Bơm chìm 12V DC đẩy nước máng NFT (PWM)</td></tr>
                    <tr><td><span class="pin-badge">PWM 5</span></td><td><span class="pin-badge">GPIO 40</span></td><td>Quạt thông gió 12V Tầng 1 - Trái</td></tr>
                    <tr><td><span class="pin-badge">PWM 6</span></td><td><span class="pin-badge">GPIO 41</span></td><td>Quạt thông gió 12V Tầng 1 - Phải</td></tr>
                    <tr><td><span class="pin-badge">PWM 7</span></td><td><span class="pin-badge">GPIO 42</span></td><td>Quạt thông gió 12V Tầng 2 - Trái</td></tr>
                    <tr><td><span class="pin-badge">PWM 8</span></td><td><span class="pin-badge">GPIO 2</span></td><td>Quạt thông gió 12V Tầng 2 - Phải</td></tr>
                </tbody>
            </table>
        `
    },
    "node-relay": {
        title: "Khối Relay Cách Ly Quang",
        layer: "Giao Tiếp Điều Khiển",
        layerClass: "text-orange",
        icon: "fa-toggle-on",
        iconBg: "var(--accent-driver)",
        specs: `
            <p class="detail-desc">Khối đóng ngắt nguồn điện cho bơm chính 220V AC, bơm sục 220V AC và 4 đèn LED 12V thông qua công tắc cơ khí Relay, được cách ly quang học hoàn toàn để bảo vệ MCU khỏi nhiễu.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Rơ le sử dụng</span><span class="value">SRD-12VDC-SL-C (Cuộn hút kích 12V DC)</span></div>
                <div class="detail-spec-item"><span class="label">Khả năng tiếp điểm</span><span class="value">10A ở 250V AC hoặc 10A ở 30V DC</span></div>
                <div class="detail-spec-item"><span class="label">Bộ cách ly quang</span><span class="value">PC817 (Optocoupler cách ly an toàn 5000V)</span></div>
                <div class="detail-spec-item"><span class="label">Transistor đệm dòng</span><span class="value">S8050 (NPN, điều khiển kích đóng rơ le)</span></div>
                <div class="detail-spec-item"><span class="label">Phân bổ kênh</span><span class="value">1 Bơm 220V AC + 1 Sục 220V AC + 4 Đèn 12V = 6 kênh</span></div>
                <div class="detail-spec-item"><span class="label">Diode dập ngược cuộn hút</span><span class="value">1N4148 (Hấp thụ điện áp tự cảm cuộn dây)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Khoét rãnh cách ly PCB</h4>
                <p>Nhằm loại bỏ nguy cơ rò điện xoay chiều 220V AC sang phần điện áp điều khiển 3.3V/5V DC, board PCB được phay rãnh cách ly rộng 3mm chạy dọc theo các chân tiếp điểm COM, NO, NC của Relay. Bơm sục 220V và Bơm chính 220V đi qua các terminal 3P (COM/NO/NC) với cách ly điện hoàn toàn.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Kênh Rơ le</th><th>GPIO ESP32-S3</th><th>Thiết bị tải kết nối</th></tr>
                </thead>
                <tbody>
                    <tr><td><span class="pin-badge">Relay 1</span></td><td><span class="pin-badge">GPIO 4</span></td><td>Bơm chính 220V AC (Đóng/Ngắt bể phụ/tuần hoàn)</td></tr>
                    <tr><td><span class="pin-badge">Relay 2</span></td><td><span class="pin-badge">GPIO 5</span></td><td>Bơm sục khí 220V AC (Sục oxy bể chứa DD)</td></tr>
                    <tr><td><span class="pin-badge">Relay 3</span></td><td><span class="pin-badge">GPIO 6</span></td><td>Dàn đèn LED Grow Tầng 1 - Trái</td></tr>
                    <tr><td><span class="pin-badge">Relay 4</span></td><td><span class="pin-badge">GPIO 7</span></td><td>Dàn đèn LED Grow Tầng 1 - Phải</td></tr>
                    <tr><td><span class="pin-badge">Relay 5</span></td><td><span class="pin-badge">GPIO 15</span></td><td>Dàn đèn LED Grow Tầng 2 - Trái</td></tr>
                    <tr><td><span class="pin-badge">Relay 6</span></td><td><span class="pin-badge">GPIO 16</span></td><td>Dàn đèn LED Grow Tầng 2 - Phải</td></tr>
                </tbody>
            </table>
        `
    },
    "node-mqtt": {
        title: "MQTT Broker (Mosquitto)",
        layer: "Tầng Truyền Thông Trung Gian",
        layerClass: "text-emerald",
        icon: "fa-cloud-arrow-up",
        iconBg: "var(--accent-broker)",
        specs: `
            <p class="detail-desc">Giao thức gửi tin nhắn Pub/Sub nhẹ, kết nối vi điều khiển ESP32-S3 với trung tâm Raspberry Pi 4 qua mạng không dây WiFi.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Phần mềm Broker</span><span class="value">Eclipse Mosquitto (Chạy trên Pi 4)</span></div>
                <div class="detail-spec-item"><span class="label">Cổng truyền thông</span><span class="value">Port 1883 (Mặc định không bảo mật nội bộ)</span></div>
                <div class="detail-spec-item"><span class="label">Tần suất gửi tin</span><span class="value">Đọc cảm biến gửi JSON mỗi 30 giây</span></div>
                <div class="detail-spec-item"><span class="label">Độ trễ truyền tin</span><span class="value">< 50 ms</span></div>
                <div class="detail-spec-item"><span class="label">Cơ chế tự bảo vệ</span><span class="value">Keep Alive + LWT (Last Will and Testament)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Failsafe khi mất mạng</h4>
                <p>Nếu ESP32-S3 mất kết nối MQTT quá 5 phút, chip sẽ tự động chuyển sang chế độ Failsafe độc lập: tự chạy quạt 50%, bật bơm nước chính luân phiên 15 phút chạy, 15 phút nghỉ để tránh héo rau.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Topic chính</th><th>Chiều dữ liệu</th><th>Nội dung gói tin (JSON)</th></tr>
                </thead>
                <tbody>
                    <tr><td><code>hydro/sensor/data</code></td><td>ESP32 &rarr; Pi</td><td>JSON chứa: temp, hum, water_temp, tds, ph, light, water_level_1..4</td></tr>
                    <tr><td><code>hydro/control/pwm</code></td><td>Pi &rarr; ESP32</td><td>JSON chứa tốc độ quạt (0-255) và bơm châm DD</td></tr>
                    <tr><td><code>hydro/control/relay</code></td><td>Pi &rarr; ESP32</td><td>JSON mảng nhị phân trạng thái bật tắt 6 Relay</td></tr>
                    <tr><td><code>hydro/sensor/status</code></td><td>ESP32 &rarr; Pi</td><td>Gửi trạng thái "online" định kỳ 60 giây</td></tr>
                </tbody>
            </table>
        `
    },
    "node-pi": {
        title: "Raspberry Pi 4 (8GB)",
        layer: "Tầng Máy Chủ Trung Tâm",
        layerClass: "text-pink",
        icon: "fa-raspberry-pi",
        iconBg: "var(--accent-rpi)",
        specs: `
            <p class="detail-desc">Bộ não trung tâm chạy trên hệ điều hành Linux, chịu trách nhiệm tiếp nhận dữ liệu cảm biến, lưu trữ cơ sở dữ liệu thời gian InfluxDB, chạy ứng dụng Web và chạy 2 mô hình AI.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">CPU vi xử lý</span><span class="value">Broadcom BCM2711, Quad-core Cortex-A72 1.5GHz</span></div>
                <div class="detail-spec-item"><span class="label">Dung lượng RAM</span><span class="value">8GB LPDDR4-3200 SDRAM</span></div>
                <div class="detail-spec-item"><span class="label">Cơ sở dữ liệu</span><span class="value">InfluxDB v1.8 (Time-series Database lưu lịch sử cảm biến)</span></div>
                <div class="detail-spec-item"><span class="label">Backend API</span><span class="value">Python FastAPI (Xử lý REST, WebSocket và OPC UA)</span></div>
                <div class="detail-spec-item"><span class="label">Bảo vệ nguồn thẻ SD</span><span class="value">Mini UPS 12V 2A chống cúp điện đột ngột</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Quản lý dịch vụ tự động</h4>
                <p>FastAPI, Mosquitto và InfluxDB được cài đặt dưới dạng systemd service chạy ngầm, tự động khởi động lại sau 5 giây nếu gặp lỗi hoặc mất nguồn đột ngột.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Dịch vụ mạng</th><th>Cổng Port</th><th>Nhiệm vụ chính</th></tr>
                </thead>
                <tbody>
                    <tr><td>FastAPI server</td><td><span class="pin-badge">Port 8000</span></td><td>REST API endpoints + WebSocket cập nhật Web UI realtime</td></tr>
                    <tr><td>Mosquitto MQTT</td><td><span class="pin-badge">Port 1883</span></td><td>Nhận dữ liệu từ ESP32 gửi về</td></tr>
                    <tr><td>InfluxDB API</td><td><span class="pin-badge">Port 8086</span></td><td>Nơi lưu và truy vấn dữ liệu lịch sử dạng biểu đồ</td></tr>
                    <tr><td>OPC UA Server</td><td><span class="pin-badge">Port 4840</span></td><td>Tạo server chia sẻ biến điều khiển qua TCP cho Siemens NX</td></tr>
                    <tr><td>Web UI Server</td><td><span class="pin-badge">Port 80</span></td><td>Chạy giao diện Web App giám sát và điều khiển</td></tr>
                </tbody>
            </table>
        `
    },
    "node-nx": {
        title: "Siemens NX MCD 3D",
        layer: "Tầng Digital Twin 3D",
        layerClass: "text-purple",
        icon: "fa-cube",
        iconBg: "var(--accent-twin)",
        specs: `
            <p class="detail-desc">Hệ thống mô phỏng 3D vật lý thời gian thực chạy trên máy tính Windows PC. Kết nối đồng bộ trực tiếp tới OPC UA Server trên Raspberry Pi qua TCP, tạo bản sao kỹ thuật số (Digital Twin) phản ánh chính xác trạng thái hệ thống thật và sự phát triển sinh học của cây.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Phần mềm CAD</span><span class="value">Siemens NX — Dựng mô hình giàn NFT 1:1</span></div>
                <div class="detail-spec-item"><span class="label">Module mô phỏng</span><span class="value">NX MCD (Mechatronics Concept Designer)</span></div>
                <div class="detail-spec-item"><span class="label">Giao thức</span><span class="value">OPC UA Client (TCP/IP)</span></div>
                <div class="detail-spec-item"><span class="label">Tần suất đồng bộ</span><span class="value">100ms (10 lần/giây)</span></div>
                <div class="detail-spec-item"><span class="label">Ngoại vi mô phỏng</span><span class="value">4 quạt + 1 bơm chìm 12V + 1 bơm 220V + 1 sục khí + 4 đèn + 6 sensor + AI growth</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Mô hình 3D bao gồm & Sync AI</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li><strong>Khung sắt V</strong> 2 tầng, 6 máng PVC vuông — dựng đúng tỉ lệ 1:1</li>
                    <li><strong>4 quạt thông gió</strong> — cánh quay theo Speed Joint, tốc độ map PWM 0-255</li>
                    <li><strong>Bơm chìm 12V (PWM)</strong> — lưu lượng dòng chảy thay đổi theo PWM duty</li>
                    <li><strong>Bơm 220V AC (Relay)</strong> — kích hoạt dòng nước chảy từ bể chính sang máng</li>
                    <li><strong>Bơm sục 220V (Relay)</strong> — hiệu ứng bọt khí trong bể chứa 3D</li>
                    <li><strong>4 đèn LED Grow (Relay)</strong> — Emission sáng/tắt theo Relay thực</li>
                    <li><strong>Xà lách 3D (XGBoost Sync)</strong> — Đọc biến <code>growth_percent</code> từ OPC UA để phóng to/thu nhỏ (Scale) kích thước hình học cây, biểu diễn đúng tốc độ sinh trưởng của mô hình boosted tree.</li>
                    <li><strong>Text thông tin</strong> — hiển thị các trị số pH, TDS và số ngày dự tính thu hoạch (<code>days_remaining</code>) lơ lửng trên mô hình 3D.</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Ngoại vi / AI</th><th>OPC UA Node</th><th>Đồng bộ trong NX MCD 3D</th></tr>
                </thead>
                <tbody>
                    <tr><td>Quạt T1-Trái (GPIO 40)</td><td><code>actuator.fan_1</code></td><td>Speed Joint cánh quạt quay, tốc độ = PWM duty</td></tr>
                    <tr><td>Quạt T1-Phải (GPIO 41)</td><td><code>actuator.fan_2</code></td><td>Speed Joint cánh quạt quay, tốc độ = PWM duty</td></tr>
                    <tr><td>Quạt T2-Trái (GPIO 42)</td><td><code>actuator.fan_3</code></td><td>Speed Joint cánh quạt quay, tốc độ = PWM duty</td></tr>
                    <tr><td>Quạt T2-Phải (GPIO 2)</td><td><code>actuator.fan_4</code></td><td>Speed Joint cánh quạt quay, tốc độ = PWM duty</td></tr>
                    <tr><td>Bơm chìm 12V (GPIO 3)</td><td><code>actuator.pump_12v</code></td><td>Lưu lượng dòng nước chảy (PWM duty: 0-255)</td></tr>
                    <tr><td>Bơm 220V AC (RL1)</td><td><code>actuator.pump_220v</code></td><td>Trạng thái dòng chảy tuần hoàn (ON/OFF)</td></tr>
                    <tr><td>Sục khí 220V (RL2)</td><td><code>actuator.pump_air</code></td><td>Hiệu ứng bọt khí nổi trong bể chứa 3D</td></tr>
                    <tr><td>Đèn LED ×4 (RL3-6)</td><td><code>actuator.light_1..4</code></td><td>Emission ON/OFF — khối đèn phát sáng/tắt</td></tr>
                    <tr><td>Mô hình XGBoost (AI)</td><td><code>ai.growth_percent</code></td><td><strong>Co giãn (Scale) mô hình xà lách 3D tương ứng tiến độ sinh trưởng</strong></td></tr>
                    <tr><td>Mô hình XGBoost (AI)</td><td><code>ai.days_remaining</code></td><td>Hiển thị số ngày còn lại đến lúc thu hoạch dạng 3D Text</td></tr>
                    <tr><td>SHT31 (nhiệt độ KK)</td><td><code>sensor.temp</code></td><td>Text nhiệt độ hiển thị nổi trên giàn 3D</td></tr>
                    <tr><td>TDS sensor</td><td><code>sensor.tds</code></td><td>Text TDS hiển thị trên bể chứa 3D</td></tr>
                    <tr><td>pH sensor</td><td><code>sensor.ph</code></td><td>Text pH hiển thị trên bể chứa 3D</td></tr>
                </tbody>
            </table>
        `
    },
    // ======================================================================
    // PI 4 SUB-COMPONENTS — Chi tiết từng dịch vụ
    // ======================================================================
    "rpi-fastapi": {
        title: "FastAPI Backend Server",
        layer: "Dịch Vụ Trên Pi 4",
        layerClass: "text-cyan",
        icon: "fa-server",
        iconBg: "var(--accent-sensor)",
        specs: `
            <p class="detail-desc">Máy chủ backend chính chạy trên Raspberry Pi 4, viết bằng Python FastAPI — framework async hiệu năng cao. Đóng vai trò trung tâm điều phối toàn bộ luồng dữ liệu giữa MQTT, Database, Web UI, AI và OPC UA.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Framework</span><span class="value">Python FastAPI + Uvicorn ASGI</span></div>
                <div class="detail-spec-item"><span class="label">Port lắng nghe</span><span class="value">8000 (REST API + WebSocket)</span></div>
                <div class="detail-spec-item"><span class="label">Giao thức realtime</span><span class="value">WebSocket push mỗi 30 giây</span></div>
                <div class="detail-spec-item"><span class="label">Khởi động</span><span class="value">systemd service, tự restart sau 5s nếu crash</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Nhiệm vụ cụ thể</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li><strong>Subscribe MQTT</strong> topic <code>hydro/sensor/data</code> → parse JSON → lưu InfluxDB</li>
                    <li><strong>Publish MQTT</strong> lệnh điều khiển PWM/Relay xuống ESP32</li>
                    <li><strong>REST API</strong> cho Web App: GET sensor data, POST control commands</li>
                    <li><strong>WebSocket</strong> push realtime sensor data tới Web UI mỗi 30s</li>
                    <li><strong>Gọi AI</strong>: nhận ảnh từ Web → chạy CNN inference → trả kết quả</li>
                    <li><strong>Cập nhật OPC UA</strong> Server nodes khi có dữ liệu mới</li>
                    <li><strong>Logic tự động</strong>: TDS thấp → bơm DD, pH lệch → bơm pH, nhiệt cao → quạt tăng</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>API Endpoint</th><th>Method</th><th>Chức năng</th></tr>
                </thead>
                <tbody>
                    <tr><td><code>/api/sensor-data/latest</code></td><td>GET</td><td>Lấy dữ liệu cảm biến mới nhất</td></tr>
                    <tr><td><code>/api/sensor-data/history</code></td><td>GET</td><td>Truy vấn lịch sử theo giờ/ngày</td></tr>
                    <tr><td><code>/api/control/pwm</code></td><td>POST</td><td>Gửi lệnh PWM duty cycle xuống ESP32</td></tr>
                    <tr><td><code>/api/control/relay</code></td><td>POST</td><td>Bật/tắt relay (bơm, đèn, sục)</td></tr>
                    <tr><td><code>/api/ai/disease-detect</code></td><td>POST</td><td>Upload ảnh lá → CNN chẩn đoán bệnh</td></tr>
                    <tr><td><code>/api/ai/growth-predict</code></td><td>GET</td><td>Dự đoán ngày thu hoạch XGBoost</td></tr>
                    <tr><td><code>/ws/realtime</code></td><td>WS</td><td>WebSocket push sensor data realtime</td></tr>
                </tbody>
            </table>
        `
    },
    "rpi-influx": {
        title: "InfluxDB Time-Series Database",
        layer: "Dịch Vụ Trên Pi 4",
        layerClass: "text-emerald",
        icon: "fa-database",
        iconBg: "var(--accent-controller)",
        specs: `
            <p class="detail-desc">Cơ sở dữ liệu chuỗi thời gian (time-series) chuyên dụng, tối ưu cho việc lưu trữ dữ liệu cảm biến IoT liên tục với timestamp. Mỗi 30 giây ghi một bản ghi mới.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Phiên bản</span><span class="value">InfluxDB v1.8 (ARM64)</span></div>
                <div class="detail-spec-item"><span class="label">Port API</span><span class="value">8086 (HTTP API)</span></div>
                <div class="detail-spec-item"><span class="label">Database name</span><span class="value">hydro_nft</span></div>
                <div class="detail-spec-item"><span class="label">Retention Policy</span><span class="value">90 ngày (tự xóa dữ liệu cũ)</span></div>
                <div class="detail-spec-item"><span class="label">Tốc độ ghi</span><span class="value">~2 points/phút (sensor + actuator state)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Nhiệm vụ cụ thể</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li><strong>Lưu sensor data</strong>: temp, humidity, water_temp, tds, ph, light, water_levels</li>
                    <li><strong>Lưu actuator state</strong>: pwm_duties[7], relay_states[6]</li>
                    <li><strong>Cung cấp dữ liệu</strong> cho Web Dashboard vẽ biểu đồ lịch sử (Chart.js)</li>
                    <li><strong>Cung cấp features</strong> cho XGBoost model dự đoán sinh trưởng</li>
                    <li><strong>Lưu log AI</strong>: kết quả chẩn đoán bệnh + ảnh lá</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Measurement</th><th>Fields</th><th>Mô tả</th></tr>
                </thead>
                <tbody>
                    <tr><td><code>sensor_data</code></td><td>temp, humidity, water_temp, tds, ph, light</td><td>Dữ liệu cảm biến mỗi 30s</td></tr>
                    <tr><td><code>water_levels</code></td><td>level_1, level_2, level_3, level_4</td><td>Trạng thái 4 mức nước Y26</td></tr>
                    <tr><td><code>actuator_state</code></td><td>pwm_1..7, relay_1..6</td><td>Trạng thái thiết bị chấp hành</td></tr>
                    <tr><td><code>ai_disease_log</code></td><td>disease_name, confidence, image_path</td><td>Kết quả chẩn đoán AI</td></tr>
                    <tr><td><code>ai_growth_log</code></td><td>days_remaining, predicted_date</td><td>Dự đoán thu hoạch</td></tr>
                </tbody>
            </table>
        `
    },
    "rpi-webapp": {
        title: "Web App & Digital Twin 2D",
        layer: "Dịch Vụ Trên Pi 4",
        layerClass: "text-orange",
        icon: "fa-desktop",
        iconBg: "var(--accent-driver)",
        specs: `
            <p class="detail-desc">Giao diện web responsive chạy trên trình duyệt, cung cấp Dashboard giám sát realtime, điều khiển thủ công PWM/Relay, và mô hình Digital Twin 2D tương tác trên Canvas/SVG.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Frontend</span><span class="value">HTML5 + CSS3 + JavaScript (Chart.js)</span></div>
                <div class="detail-spec-item"><span class="label">Realtime</span><span class="value">WebSocket từ FastAPI (cập nhật mỗi 30s)</span></div>
                <div class="detail-spec-item"><span class="label">Port</span><span class="value">80 (Nginx reverse proxy)</span></div>
                <div class="detail-spec-item"><span class="label">Responsive</span><span class="value">Desktop + Mobile (điều khiển từ xa)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Các trang chức năng</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li><strong>Dashboard</strong>: Gauge realtime (nhiệt độ, TDS, pH, ánh sáng, mực nước)</li>
                    <li><strong>Biểu đồ lịch sử</strong>: Chart.js line chart 24h/7d/30d từ InfluxDB</li>
                    <li><strong>Điều khiển</strong>: Slider PWM (quạt, bơm DD) + Toggle Relay (đèn, bơm, sục)</li>
                    <li><strong>Digital Twin 2D</strong>: Mô hình SVG giàn NFT — quạt quay, nước chảy, đèn sáng</li>
                    <li><strong>AI Camera</strong>: Mở camera ĐT (HTML5 input capture) → chụp lá → upload chẩn đoán</li>
                    <li><strong>Cảnh báo</strong>: Push notification khi sensor vượt ngưỡng hoặc cạn nước</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Trang</th><th>Dữ liệu nguồn</th><th>Tương tác</th></tr>
                </thead>
                <tbody>
                    <tr><td>Dashboard</td><td>WebSocket sensor data</td><td>Hiển thị gauge realtime</td></tr>
                    <tr><td>History</td><td>REST API → InfluxDB</td><td>Biểu đồ Chart.js</td></tr>
                    <tr><td>Control</td><td>POST API → MQTT → ESP32</td><td>Slider PWM + Toggle Relay</td></tr>
                    <tr><td>Twin 2D</td><td>WebSocket actuator state</td><td>SVG animation đồng bộ</td></tr>
                    <tr><td>AI Scan</td><td>POST ảnh → CNN model</td><td>Kết quả + đề xuất xử lý</td></tr>
                </tbody>
            </table>
        `
    },
    "rpi-opcua": {
        title: "OPC UA Server (asyncua)",
        layer: "Dịch Vụ Trên Pi 4",
        layerClass: "text-purple",
        icon: "fa-network-wired",
        iconBg: "var(--accent-twin)",
        specs: `
            <p class="detail-desc">Server giao thức công nghiệp OPC UA chạy bằng thư viện Python <code>asyncua</code>, tạo cầu nối giữa hệ thống IoT thực và phần mềm mô phỏng Siemens NX MCD trên PC.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Thư viện</span><span class="value">Python asyncua (pip install)</span></div>
                <div class="detail-spec-item"><span class="label">Endpoint</span><span class="value">opc.tcp://[IP_Pi]:4840</span></div>
                <div class="detail-spec-item"><span class="label">Namespace</span><span class="value">ns=2 (HydroNFT)</span></div>
                <div class="detail-spec-item"><span class="label">Chu kỳ cập nhật</span><span class="value">100ms (realtime sync)</span></div>
                <div class="detail-spec-item"><span class="label">Tổng số nodes</span><span class="value">20 nodes (6 sensor + 12 actuator + 2 AI predictions)</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Nhiệm vụ cụ thể</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li><strong>Publish sensor nodes</strong>: temp, humidity, water_temp, tds, ph, light, water_level_1..4</li>
                    <li><strong>Publish actuator nodes</strong>: fan_1..4, pump_dd_1..3, pump_12v, pump_220v, pump_air_220v, light_1..4</li>
                    <li><strong>Publish AI nodes</strong>: days_remaining, growth_percent (truyền kết quả XGBoost sang MCD)</li>
                    <li><strong>Nhận lệnh từ NX MCD</strong> (write-back): cho phép điều khiển ngược từ Digital Twin</li>
                    <li><strong>Cập nhật từ FastAPI</strong>: mỗi khi có MQTT data mới → ghi vào OPC UA nodes</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>OPC UA Node ID</th><th>Kiểu</th><th>Đồng bộ với NX MCD</th></tr>
                </thead>
                <tbody>
                    <tr><td><code>ns=2;s=sensor.temp</code></td><td>Float</td><td>Hiển thị text nhiệt độ trên mô hình 3D</td></tr>
                    <tr><td><code>ns=2;s=sensor.tds</code></td><td>Float</td><td>Hiển thị giá trị TDS trên bể chứa 3D</td></tr>
                    <tr><td><code>ns=2;s=sensor.ph</code></td><td>Float</td><td>Hiển thị pH trên bể chứa 3D</td></tr>
                    <tr><td><code>ns=2;s=actuator.fan_1..4</code></td><td>Int 0-255</td><td>Tốc độ quay cánh quạt 3D (Speed Joint)</td></tr>
                    <tr><td><code>ns=2;s=actuator.pump_12v</code></td><td>Int 0-255</td><td>Bơm chìm 12V — lưu lượng nước tỉ lệ thuận tốc độ PWM</td></tr>
                    <tr><td><code>ns=2;s=actuator.pump_220v</code></td><td>Boolean</td><td>Bơm 220V AC — kích hoạt dòng chảy ON/OFF</td></tr>
                    <tr><td><code>ns=2;s=actuator.pump_air</code></td><td>Boolean</td><td>Sục khí 220V — hiệu ứng bọt khí trong bể 3D</td></tr>
                    <tr><td><code>ns=2;s=actuator.light_1..4</code></td><td>Boolean</td><td>Đèn LED Grow — Emission sáng/tắt 3D</td></tr>
                    <tr><td><code>ns=2;s=ai.growth_percent</code></td><td>Float</td><td><strong>Co giãn tỷ lệ (Scale) hình dáng 3D của cây xà lách</strong></td></tr>
                    <tr><td><code>ns=2;s=ai.days_remaining</code></td><td>Float</td><td>Hiển thị text 3D số ngày còn lại đến lúc thu hoạch</td></tr>
                </tbody>
            </table>
        `
    },
    "rpi-cnn": {
        title: "CNN MobileNetV2 — Bệnh Cây",
        layer: "AI Module Trên Pi 4",
        layerClass: "text-pink",
        icon: "fa-brain",
        iconBg: "var(--accent-rpi)",
        specs: `
            <p class="detail-desc">Mô hình mạng nơ-ron tích chập (CNN) dựa trên kiến trúc MobileNetV2, được huấn luyện trên Google Colab và triển khai dưới dạng TFLite trên Raspberry Pi 4 để nhận diện bệnh lá xà lách.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Kiến trúc</span><span class="value">MobileNetV2 (Transfer Learning)</span></div>
                <div class="detail-spec-item"><span class="label">Kích thước ảnh đầu vào</span><span class="value">224 × 224 × 3 (RGB)</span></div>
                <div class="detail-spec-item"><span class="label">Framework chạy</span><span class="value">tflite_runtime (tối ưu ARM)</span></div>
                <div class="detail-spec-item"><span class="label">Thời gian suy luận</span><span class="value">~150-200ms trên Pi 4</span></div>
                <div class="detail-spec-item"><span class="label">Số lớp phân loại</span><span class="value">5-8 loại bệnh + 1 lớp Healthy</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Quy trình hoạt động</h4>
                <ul style="font-size:12px;color:var(--text-muted);line-height:1.7;padding-left:16px;">
                    <li>Người dùng mở Web App → bật camera điện thoại (HTML5 capture)</li>
                    <li>Chụp cận cảnh lá xà lách nghi bệnh → upload lên FastAPI</li>
                    <li>FastAPI resize ảnh 224×224 → nạp vào tflite interpreter</li>
                    <li>Model trả về xác suất từng loại bệnh (softmax)</li>
                    <li>Hiển thị kết quả + đề xuất cách xử lý trên Web App</li>
                    <li>Lưu log vào InfluxDB để theo dõi lịch sử bệnh</li>
                </ul>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Loại bệnh nhận diện</th><th>Triệu chứng</th><th>Đề xuất xử lý</th></tr>
                </thead>
                <tbody>
                    <tr><td>Bacterial Spot</td><td>Đốm nâu nhỏ trên lá</td><td>Cách ly + tăng thông gió</td></tr>
                    <tr><td>Downy Mildew</td><td>Đốm vàng mặt trên, mốc mặt dưới</td><td>Giảm ẩm + tăng quạt 80%</td></tr>
                    <tr><td>Nutrient Deficiency</td><td>Lá vàng nhạt, còi cọc</td><td>Tăng TDS + kiểm tra pH</td></tr>
                    <tr><td>Tip Burn</td><td>Cháy viền lá</td><td>Giảm TDS + tăng lưu lượng nước</td></tr>
                    <tr><td>Healthy</td><td>Lá xanh tươi, không tổn thương</td><td>Duy trì điều kiện hiện tại</td></tr>
                </tbody>
            </table>
        `
    },
    "rpi-xgb": {
        title: "XGBoost — Dự Đoán Sinh Trưởng",
        layer: "AI Module Trên Pi 4",
        layerClass: "text-pink",
        icon: "fa-chart-line",
        iconBg: "var(--accent-rpi)",
        specs: `
            <p class="detail-desc">Mô hình hồi quy XGBoost (Extreme Gradient Boosting) được huấn luyện từ dữ liệu lịch sử cảm biến để dự đoán số ngày còn lại trước khi thu hoạch xà lách và chỉ số % phát triển, đồng bộ trực tiếp sang mô hình 3D Siemens NX MCD.</p>
            <div class="detail-specs-list">
                <div class="detail-spec-item"><span class="label">Thuật toán</span><span class="value">XGBRegressor (scikit-learn API)</span></div>
                <div class="detail-spec-item"><span class="label">Features đầu vào</span><span class="value">7 features (temp, humidity, tds, ph, light, water_temp, ngày tuổi)</span></div>
                <div class="detail-spec-item"><span class="label">Outputs</span><span class="value">days_remaining (Số ngày còn lại) + growth_percent (% sinh trưởng)</span></div>
                <div class="detail-spec-item"><span class="label">Sai số RMSE</span><span class="value">±1-2 ngày</span></div>
                <div class="detail-spec-item"><span class="label">Liên kết Digital Twin</span><span class="value">Đồng bộ qua OPC UA node → Scale mô hình cây 3D trong NX MCD</span></div>
            </div>
            <div class="spec-info-card">
                <h4>Liên kết & Tương tác 3D</h4>
                <p>Mỗi khi chạy dự báo (định kỳ hoặc thủ công), kết quả <code>growth_percent</code> (0% - 100%) và <code>days_remaining</code> được FastAPI ghi thẳng vào OPC UA Server. Phía Siemens NX MCD liên kết biến này với tham số tỉ lệ hình học (Scale Factor) của cụm cây 3D trên máng, giúp cây lớn lên trực quan theo thuật toán AI.</p>
            </div>
        `,
        connections: `
            <table class="connections-table">
                <thead>
                    <tr><th>Đầu ra AI</th><th>OPC UA Node</th><th>Hiệu ứng trong NX MCD 3D</th></tr>
                </thead>
                <tbody>
                    <tr><td>% Sinh trưởng (0-100%)</td><td><code>ai.growth_percent</code></td><td>Thay đổi tỷ lệ (Scale) hình dáng 3D xà lách tương ứng</td></tr>
                    <tr><td>Số ngày còn lại (days)</td><td><code>ai.days_remaining</code></td><td>Hiển thị nhãn 3D Text lơ lửng dự báo ngày thu hoạch</td></tr>
                    <tr><td>Nhiệt độ KK (°C)</td><td><code>sensor.temp</code></td><td>Feature đầu vào (SHT31)</td></tr>
                    <tr><td>Nồng độ TDS (ppm)</td><td><code>sensor.tds</code></td><td>Feature đầu vào (TDS sensor)</td></tr>
                    <tr><td>Độ pH</td><td><code>sensor.ph</code></td><td>Feature đầu vào (pH sensor)</td></tr>
                    <tr><td>Ánh sáng (lux)</td><td><code>sensor.light</code></td><td>Feature đầu vào (BH1750)</td></tr>
                </tbody>
            </table>
        `
    }
};

// ==========================================================================
// DYNAMIC SVG CONNECTIONS DRAWING
// ==========================================================================

function updateConnections() {
    const canvas = document.getElementById('diagram-canvas');
    if (!canvas) return;

    // Recursively calculate offsets relative to the diagram-canvas container, including drag offsets
    const getLocalCoords = (el) => {
        let x = 0;
        let y = 0;
        let current = el;
        while (current && current !== canvas) {
            x += current.offsetLeft;
            y += current.offsetTop;
            
            // Add any active drag offset translation
            const dx = parseFloat(current.style.getPropertyValue('--drag-x')) || 0;
            const dy = parseFloat(current.style.getPropertyValue('--drag-y')) || 0;
            x += dx;
            y += dy;
            
            current = current.offsetParent;
        }
        return { x, y };
    };

    const getPort = (elId, position) => {
        const el = document.getElementById(elId);
        if (!el) return { x: 0, y: 0 };
        
        const coords = getLocalCoords(el);
        const w = el.offsetWidth;
        const h = el.offsetHeight;
        
        switch (position) {
            case 'left': return { x: coords.x, y: coords.y + h / 2 };
            case 'right': return { x: coords.x + w, y: coords.y + h / 2 };
            case 'top': return { x: coords.x + w / 2, y: coords.y };
            case 'bottom': return { x: coords.x + w / 2, y: coords.y + h };
            default: return { x: coords.x + w / 2, y: coords.y + h / 2 };
        }
    };

    const drawCurve = (pathId, fromId, fromPos, toId, toPos) => {
        const path = document.getElementById(pathId);
        if (!path) return;
        const p1 = getPort(fromId, fromPos);
        const p2 = getPort(toId, toPos);
        
        if (p1.x === 0 && p1.y === 0) return; // Node hidden or rendering issue
        
        let d = '';
        if (fromPos === 'right' && toPos === 'left') {
            const cp1x = p1.x + (p2.x - p1.x) * 0.4;
            const cp2x = p1.x + (p2.x - p1.x) * 0.6;
            d = `M ${p1.x} ${p1.y} C ${cp1x} ${p1.y}, ${cp2x} ${p2.y}, ${p2.x} ${p2.y}`;
        } else if (fromPos === 'bottom' && toPos === 'top') {
            const cp1y = p1.y + (p2.y - p1.y) * 0.4;
            const cp2y = p1.y + (p2.y - p1.y) * 0.6;
            d = `M ${p1.x} ${p1.y} C ${p1.x} ${cp1y}, ${p2.x} ${cp2y}, ${p2.x} ${p2.y}`;
        } else if (fromPos === 'top' && toPos === 'bottom') {
            const cp1y = p1.y - (p1.y - p2.y) * 0.4;
            const cp2y = p1.y - (p1.y - p2.y) * 0.6;
            d = `M ${p1.x} ${p1.y} C ${p1.x} ${cp1y}, ${p2.x} ${cp2y}, ${p2.x} ${p2.y}`;
        } else {
            // Bezier standard
            const cp1x = p1.x + (p2.x - p1.x) * 0.5;
            d = `M ${p1.x} ${p1.y} C ${cp1x} ${p1.y}, ${cp1x} ${p2.y}, ${p2.x} ${p2.y}`;
        }
        path.setAttribute('d', d);
    };

    // 1. Power connections
    drawCurve('path-psu-mcu', 'node-psu', 'bottom', 'node-mcu', 'top');
    drawCurve('path-psu-pwm', 'node-psu', 'bottom', 'node-pwm', 'top');
    drawCurve('path-psu-relay', 'node-psu', 'bottom', 'node-relay', 'top');
    drawCurve('path-psu-y26', 'node-psu', 'bottom', 'node-y26', 'top');
    drawCurve('path-mcu-sht31-pwr', 'node-mcu', 'left', 'node-sht31', 'right');

    // 2. Sensor signal connections
    drawCurve('path-sht31-mcu', 'node-sht31', 'right', 'node-mcu', 'left');
    drawCurve('path-ds18b20-mcu', 'node-ds18b20', 'right', 'node-mcu', 'left');
    drawCurve('path-tds-mcu', 'node-tds', 'right', 'node-mcu', 'left');
    drawCurve('path-ph-mcu', 'node-ph', 'right', 'node-mcu', 'left');
    drawCurve('path-bh1750-mcu', 'node-bh1750', 'right', 'node-mcu', 'left');
    drawCurve('path-y26-mcu', 'node-y26', 'right', 'node-mcu', 'left');

    // 3. Controller to Drivers
    drawCurve('path-mcu-pwm', 'node-mcu', 'bottom', 'node-pwm', 'top');
    drawCurve('path-mcu-relay', 'node-mcu', 'bottom', 'node-relay', 'top');

    // 4. Drivers to devices
    drawCurve('path-pwm-pumps', 'node-pwm', 'bottom', 'dev-pumps', 'top');
    drawCurve('path-pwm-fans', 'node-pwm', 'bottom', 'dev-fans', 'top');
    drawCurve('path-relay-ac', 'node-relay', 'bottom', 'dev-ac', 'top');
    drawCurve('path-relay-led', 'node-relay', 'bottom', 'dev-led', 'top');

    // 5. Communications
    drawCurve('path-mcu-mqtt', 'node-mcu', 'right', 'node-mqtt', 'left');
    drawCurve('path-mqtt-pi', 'node-mqtt', 'right', 'node-pi', 'left');
    drawCurve('path-pi-opcua', 'node-pi', 'right', 'node-nx', 'left');
}

// ==========================================================================
// FLOW VISUALIZATION SWITCHES
// ==========================================================================

const allPaths = [
    'path-psu-mcu', 'path-psu-pwm', 'path-psu-relay', 'path-psu-y26', 'path-mcu-sht31-pwr',
    'path-sht31-mcu', 'path-ds18b20-mcu', 'path-tds-mcu', 'path-ph-mcu', 'path-bh1750-mcu', 'path-y26-mcu',
    'path-mcu-pwm', 'path-mcu-relay',
    'path-pwm-pumps', 'path-pwm-fans', 'path-relay-ac', 'path-relay-led',
    'path-mcu-mqtt', 'path-mqtt-pi', 'path-pi-opcua'
];

const flows = {
    all: allPaths,
    data: [
        'path-sht31-mcu', 'path-ds18b20-mcu', 'path-tds-mcu', 'path-ph-mcu', 'path-bh1750-mcu', 'path-y26-mcu',
        'path-mcu-mqtt', 'path-mqtt-pi', 'path-pi-opcua'
    ],
    power: [
        'path-psu-mcu', 'path-psu-pwm', 'path-psu-relay', 'path-psu-y26', 'path-mcu-sht31-pwr'
    ],
    control: [
        'path-pi-mqtt', 'path-mqtt-pi', 'path-mcu-mqtt', 'path-mcu-pwm', 'path-mcu-relay',
        'path-pwm-pumps', 'path-pwm-fans', 'path-relay-ac', 'path-relay-led'
    ]
};

function selectFlow(flowType) {
    // Reset all lines
    allPaths.forEach(pathId => {
        const path = document.getElementById(pathId);
        if (path) {
            path.classList.remove('active-line');
        }
    });

    // Highlight active paths
    const activePaths = flows[flowType] || [];
    activePaths.forEach(pathId => {
        const path = document.getElementById(pathId);
        if (path) {
            path.classList.add('active-line');
        }
    });
}

// ==========================================================================
// SIMULATOR LOG SYSTEM
// ==========================================================================

function addLog(text, type = 'system') {
    const consoleOutput = document.getElementById('console-output');
    if (!consoleOutput) return;

    const time = new Date().toLocaleTimeString();
    const logLine = document.createElement('div');
    logLine.className = `log-line ${type}`;
    logLine.innerHTML = `[${time}] ${text}`;
    
    consoleOutput.appendChild(logLine);
    consoleOutput.scrollTop = consoleOutput.scrollHeight;
}

// Global active timers for simulation effects
let simTimers = [];

function clearSimulationEffects() {
    // Clear logs timers
    simTimers.forEach(clearTimeout);
    simTimers = [];

    // Remove CSS classes from devices
    document.querySelectorAll('.device-item').forEach(el => el.classList.remove('active-device'));
    document.querySelectorAll('.diagram-node').forEach(el => el.classList.remove('flash-red-alert'));
    document.querySelectorAll('.rpi-comp-box').forEach(el => el.classList.remove('active-comp'));
}

function runTempSimulation() {
    clearSimulationEffects();
    addLog("BẮT ĐẦU GIẢ LẬP: Nhiệt độ không khí tăng cao (>28°C)", "event");
    
    // Step 1: Sensor triggers
    const tempNode = document.getElementById('node-sht31');
    tempNode.classList.add('flash-red-alert');
    addLog("[SHT31] Phát hiện nhiệt độ tăng lên 29.8°C (Ngưỡng bật quạt là 28°C).", "event");

    // Step 2: Flow SHT31 -> MCU
    selectFlow('data');
    
    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Đọc dữ liệu từ SHT31 qua I2C. Đóng gói JSON gửi MQTT.", "mqtt");
        addLog("[MQTT] Publish: <code style='color:#a7f3d0;'>hydro/sensor/data {temp: 29.8, humidity: 72.0}</code>", "mqtt");
    }, 1500));

    // Step 3: MQTT -> Pi -> Decision -> ESP32
    simTimers.push(setTimeout(() => {
        addLog("[FastAPI Pi 4] Nhận MQTT. Kích hoạt logic điều khiển tự động.", "api");
        document.getElementById('rpi-fastapi').classList.add('active-comp');
        addLog("[MQTT] Pi publish lệnh điều khiển: <code style='color:#f43f5e;'>hydro/control/pwm [0,0,220,220,220,220]</code>", "mqtt");
    }, 3000));

    // Step 4: MCU -> Actuators -> Fans
    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Nhận lệnh PWM. Kích dẫn Gate MOSFET IRLZ44N (GPIO 40,41,42,2).", "system");
        document.getElementById('dev-fans').classList.add('active-device');
        selectFlow('control');
        addLog("[DRIVERS] MOSFET kích mở. Quạt 12V thông gió hoạt động với công suất 86%.", "system");
    }, 4500));

    // Step 5: Sync OPC UA
    simTimers.push(setTimeout(() => {
        addLog("[OPC UA Pi] Cập nhật Node <code style='color:#c084fc;'>ns=2;s=actuator.fan_1..4 = 220</code>", "api");
        document.getElementById('rpi-opcua').classList.add('active-comp');
        addLog("[NX MCD 3D] Đang đồng bộ OPC UA. Cánh quạt trên Digital Twin 3D bắt đầu quay vật lý.", "system");
    }, 6000));
}

function runTdsSimulation() {
    clearSimulationEffects();
    addLog("BẮT ĐẦU GIẢ LẬP: Nồng độ TDS giảm thấp (<500 ppm)", "event");

    const tdsNode = document.getElementById('node-tds');
    tdsNode.classList.add('flash-red-alert');
    addLog("[TDS SENSOR] Đo được TDS = 450 ppm (Mức tối ưu: 500-800 ppm). Dung dịch bị nhạt.", "event");

    selectFlow('data');

    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Gửi tín hiệu analog qua ADC1_CH9. Gửi MQTT lên Server.", "mqtt");
        addLog("[MQTT] Publish: <code style='color:#a7f3d0;'>hydro/sensor/data {tds: 450, water_temp: 24.2}</code>", "mqtt");
    }, 1500));

    simTimers.push(setTimeout(() => {
        addLog("[Pi 4 Server] Dữ liệu TDS dưới ngưỡng 500. Tính toán lượng dung dịch A & B cần châm.", "api");
        document.getElementById('rpi-fastapi').classList.add('active-comp');
        addLog("[MQTT] Pi gửi lệnh bật bơm châm: <code style='color:#f43f5e;'>hydro/control/pwm [128,128,0,0,0,0]</code> (Bơm A và B chạy 50% công suất).", "mqtt");
    }, 3000));

    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Nhận lệnh. Kích mở MOSFET GPIO 38 và 39 (Duty 128).", "system");
        document.getElementById('dev-pumps').classList.add('active-device');
        selectFlow('control');
        addLog("[THIẾT BỊ] 2 Bơm châm nhu động 12V (Dung dịch A & B) bắt đầu bơm thuốc vào bể chứa.", "system");
    }, 4500));

    simTimers.push(setTimeout(() => {
        addLog("[OPC UA] Node <code style='color:#c084fc;'>ns=2;s=actuator.pump_dd_1..2 = 128</code>", "api");
        document.getElementById('rpi-opcua').classList.add('active-comp');
        addLog("[NX MCD 3D] Digital Twin hiển thị mô hình bơm châm đang quay châm nước màu đỏ/xanh.", "system");
    }, 6000));
}

function runPhSimulation() {
    clearSimulationEffects();
    addLog("BẮT ĐẦU GIẢ LẬP: Độ pH tăng cao (>6.5)", "event");

    const phNode = document.getElementById('node-ph');
    phNode.classList.add('flash-red-alert');
    addLog("[pH SENSOR] Đo được pH = 6.9 (Mức tối ưu: 5.5-6.5). Nước bị kiềm hóa.", "event");

    selectFlow('data');

    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Chuyển đổi analog sang ADC1_CH8. Gửi MQTT.", "mqtt");
        addLog("[MQTT] Publish: <code style='color:#a7f3d0;'>hydro/sensor/data {ph: 6.9}</code>", "mqtt");
    }, 1500));

    simTimers.push(setTimeout(() => {
        addLog("[Pi Backend] Phát hiện pH vượt ngưỡng 6.5. Kích hoạt bơm dung dịch pH Down (axit).", "api");
        document.getElementById('rpi-fastapi').classList.add('active-comp');
        addLog("[MQTT] Pi gửi lệnh: <code style='color:#f43f5e;'>hydro/control/pwm [0,0,0,0,0,0,100]</code> (Chạy bơm pH Down GPIO 1).", "mqtt");
    }, 3000));

    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Kích hoạt kênh MOSFET dự phòng (GPIO 1) với duty cycle 100.", "system");
        document.getElementById('dev-pumps').classList.add('active-device');
        selectFlow('control');
        addLog("[THIẾT BỊ] Bơm nhu động số 3 châm dung dịch pH Down axit nhẹ để điều chỉnh pH bể chứa.", "system");
    }, 4500));
}

function runWaterLowSimulation() {
    clearSimulationEffects();
    addLog("CẢNH BÁO BÁO ĐỘNG: Cạn nước bể chứa (Mức nước thấp <25%)", "error");

    const y26Node = document.getElementById('node-y26');
    y26Node.classList.add('flash-red-alert');
    addLog("[Y26 SENSORS] Báo cạn! Cảm biến mức 1 (25%) trả về trạng thái LOW (0V).", "error");

    selectFlow('data');

    simTimers.push(setTimeout(() => {
        addLog("[ESP32] Kích hoạt cơ chế tự bảo vệ khẩn cấp độc lập (Hardware Failsafe!).", "system");
        addLog("[ESP32] Tự động ngắt khẩn cấp GPIO 4 (RL1 - Bơm chìm 220V) và GPIO 5 (RL2 - Bơm sục 220V) ngay lập tức để tránh cháy động cơ.", "error");
        addLog("[ESP32] Phát còi Buzzer (GPIO 48) ngắt quãng cảnh báo chập điện.", "error");
        
        // Disable pumps devices visually
        document.getElementById('dev-ac').classList.remove('active-device');
    }, 1500));

    simTimers.push(setTimeout(() => {
        addLog("[MQTT] ESP32 gửi tin báo động khẩn cấp: <code style='color:#f43f5e;'>hydro/sensor/alarm {water_empty: true}</code>", "mqtt");
        addLog("[Pi Backend] Nhận cảnh báo. Ghi nhật ký vào InfluxDB, gửi thông báo đẩy khẩn cấp lên Web App.", "api");
        document.getElementById('rpi-influx').classList.add('active-comp');
    }, 3000));

    simTimers.push(setTimeout(() => {
        addLog("[NX MCD 3D] Đồng bộ OPC UA báo cạn. Bể chứa nước trên mô hình 3D nhấp nháy màu đỏ.", "system");
    }, 4500));
}

function runDiseaseSimulation() {
    clearSimulationEffects();
    addLog("BẮT ĐẦU CHẨN ĐOÁN: Quét bệnh hại lá bằng AI (CNN MobileNetV2)", "event");

    document.getElementById('rpi-cnn').classList.add('active-comp');
    addLog("[WEB APP UI] Người dùng mở camera điện thoại, chụp ảnh lá xà lách nghi ngờ bị đốm lá và tải lên.", "api");

    simTimers.push(setTimeout(() => {
        addLog("[FastAPI Pi] API nhận file ảnh đầu vào, chuyển tiếp tới module AI dịch vụ.", "api");
        addLog("[AI MODULE] Nạp ảnh kích thước 224x224x3 vào thư viện tflite_runtime.", "system");
    }, 1500));

    simTimers.push(setTimeout(() => {
        addLog("[AI CNN MODEL] Chạy suy luận (Inference time: 180ms). Kết quả:", "api");
        addLog("<code style='color:#a855f7;'>- Bệnh đốm lá do vi khuẩn (Bacterial Spot): 94.62%</code>", "api");
        addLog("<code style='color:#94a3b8;'>- Thiếu dinh dưỡng sắt (Deficiency Fe): 4.10%</code>", "api");
        addLog("<code style='color:#94a3b8;'>- Lá khỏe mạnh (Healthy): 1.28%</code>", "api");
    }, 3200));

    simTimers.push(setTimeout(() => {
        addLog("[FastAPI Pi] Lưu ảnh bệnh và kết quả vào InfluxDB. Gửi phản hồi JSON về Web App.", "api");
        addLog("[WEB APP UI] Hiển thị kết quả chẩn đoán: Cảnh báo bệnh đốm lá + đề xuất cách ly rọ cây, giảm độ ẩm quạt gió tăng 80%.", "event");
    }, 4800));
}

function runGrowthSimulation() {
    clearSimulationEffects();
    addLog("BẮT ĐẦU DỰ ĐOÁN: Thời gian thu hoạch cây (XGBoost)", "event");

    document.getElementById('rpi-xgb').classList.add('active-comp');
    addLog("[FastAPI Pi 4] Kích hoạt hàm dự đoán sinh trưởng định kỳ.", "api");

    simTimers.push(setTimeout(() => {
        addLog("[InfluxDB] Truy vấn lịch sử 7 ngày qua của cây: Trung bình TDS = 680ppm, pH = 6.1, Cường độ sáng = 18000 lux-hour.", "api");
        document.getElementById('rpi-influx').classList.add('active-comp');
    }, 1500));

    simTimers.push(setTimeout(() => {
        addLog("[AI XGBoost] Chạy mô hình hồi quy với các đặc trưng (features) môi trường và số ngày tuổi (22 ngày).", "api");
        addLog("[AI XGBoost] Kết quả dự đoán thời gian thu hoạch còn lại: <strong style='color:#10b981;'>12.4 ngày</strong> (RMSE sai lệch ±1.2 ngày).", "event");
    }, 3000));

    simTimers.push(setTimeout(() => {
        addLog("[WEB APP UI] Cập nhật giao diện Dashboard: 'Dự báo thu hoạch vào ngày 23/06/2026'.", "api");
    }, 4200));
}

// ==========================================================================
// INSPECTOR DISPLAY CONTROL
// ==========================================================================

let selectedNodeId = null;
let selectedTab = 'specs';

function showInspector(nodeId) {
    selectedNodeId = nodeId;
    const data = nodeData[nodeId];
    
    const placeholder = document.getElementById('inspector-placeholder');
    const content = document.getElementById('inspector-content');
    
    if (!data) {
        placeholder.classList.remove('hidden');
        content.classList.add('hidden');
        return;
    }
    
    placeholder.classList.add('hidden');
    content.classList.remove('hidden');
    
    // Update Node Border Highlight
    document.querySelectorAll('.diagram-node').forEach(node => {
        node.classList.remove('active-node');
    });
    document.getElementById(nodeId).classList.add('active-node');
    
    // Update header info
    document.getElementById('ins-title').textContent = data.title;
    
    const layerEl = document.getElementById('ins-layer');
    layerEl.textContent = data.layer;
    layerEl.className = `badge ${data.layerClass}`;
    
    // Update icon
    const iconBg = document.getElementById('ins-icon-bg');
    iconBg.style.backgroundColor = data.iconBg;
    
    const iconEl = document.getElementById('ins-icon');
    iconEl.className = `fa-solid ${data.icon}`;
    
    // Render Tab Contents
    renderInspectorTabs();
}

function renderInspectorTabs() {
    if (!selectedNodeId) return;
    const data = nodeData[selectedNodeId];
    
    const specsEl = document.getElementById('ins-specs');
    const connEl = document.getElementById('ins-connections');
    
    specsEl.innerHTML = data.specs;
    connEl.innerHTML = data.connections;
}

// ==========================================================================
// DRAG AND DROP UTILITY
// ==========================================================================

function makeDraggable(element) {
    let startX = 0;
    let startY = 0;
    let dragX = 0;
    let dragY = 0;

    element.addEventListener('mousedown', onMouseDown);

    function onMouseDown(e) {
        // Prevent drag on interactive child components (buttons, links, tables, inputs, badges)
        if (e.target.closest('button, a, input, select, textarea, .pin-badge, table, tr, td, th')) {
            return;
        }

        // Avoid nested dragging: if we clicked a diagram-node inside a container, only drag the node
        const closestDraggable = e.target.closest('.diagram-node, .sensor-container, .actuator-container, .devices-container');
        if (element !== closestDraggable) {
            return;
        }

        e.preventDefault();
        e.stopPropagation();

        startX = e.clientX;
        startY = e.clientY;
        
        dragX = parseFloat(element.style.getPropertyValue('--drag-x')) || 0;
        dragY = parseFloat(element.style.getPropertyValue('--drag-y')) || 0;

        element.classList.add('dragging');

        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', onMouseUp);
    }

    function onMouseMove(e) {
        const dx = e.clientX - startX;
        const dy = e.clientY - startY;

        // Account for any viewport scaling/transforms on the canvas
        const canvas = document.getElementById('diagram-canvas');
        const scale = canvas ? (canvas.getBoundingClientRect().width / canvas.offsetWidth) : 1;

        const currentDragX = dragX + dx / scale;
        const currentDragY = dragY + dy / scale;

        element.style.setProperty('--drag-x', `${currentDragX}px`);
        element.style.setProperty('--drag-y', `${currentDragY}px`);

        if (dx !== 0 || dy !== 0) {
            element.setAttribute('data-dragged', 'true');
        }

        if (window.updateConnections) {
            window.updateConnections();
        }
    }

    function onMouseUp() {
        element.classList.remove('dragging');
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
    }
}

// ==========================================================================
// INITIALIZATION & EVENT BINDINGS
// ==========================================================================

document.addEventListener('DOMContentLoaded', () => {
    // 1. Initial draw of paths
    updateConnections();
    selectFlow('all');

    // 2. Window events
    window.addEventListener('resize', () => {
        updateConnections();
    });

    // 3. Click events on diagram nodes (ignores dragging)
    document.querySelectorAll('.diagram-node.clickable').forEach(node => {
        node.addEventListener('click', (e) => {
            if (node.getAttribute('data-dragged') === 'true') {
                node.removeAttribute('data-dragged');
                return;
            }
            // Clear Pi sub-component selection
            document.querySelectorAll('.rpi-comp-box').forEach(c => c.classList.remove('selected-comp'));
            showInspector(node.id);
        });
    });

    // 3b. Click events on Pi sub-components
    document.querySelectorAll('.rpi-comp-box.clickable').forEach(comp => {
        comp.addEventListener('click', (e) => {
            e.stopPropagation(); // Prevent parent node click
            // Highlight selected comp
            document.querySelectorAll('.rpi-comp-box').forEach(c => c.classList.remove('selected-comp'));
            comp.classList.add('selected-comp');
            // Also highlight parent Pi node
            document.querySelectorAll('.diagram-node').forEach(n => n.classList.remove('active-node'));
            document.getElementById('node-pi').classList.add('active-node');
            showInspector(comp.id);
        });
    });

    // 4. Flow selection controls
    document.querySelectorAll('.control-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.control-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            selectFlow(btn.dataset.flow);
            
            addLog(`Đã thay đổi hiển thị: ${btn.textContent.trim()}`, 'system');
        });
    });

    // 5. Inspector tab controls
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            
            btn.classList.add('active');
            selectedTab = btn.dataset.tab;
            
            if (selectedTab === 'specs') {
                document.getElementById('tab-specs').classList.add('active');
            } else {
                document.getElementById('tab-connections').classList.add('active');
            }
        });
    });

    // 6. Simulator triggers
    document.getElementById('sim-temp-high')?.addEventListener('click', runTempSimulation);
    document.getElementById('sim-tds-low')?.addEventListener('click', runTdsSimulation);
    document.getElementById('sim-ph-high')?.addEventListener('click', runPhSimulation);
    document.getElementById('sim-water-low')?.addEventListener('click', runWaterLowSimulation);
    document.getElementById('sim-disease-detect')?.addEventListener('click', runDiseaseSimulation);
    document.getElementById('sim-grow-predict')?.addEventListener('click', runGrowthSimulation);

    document.getElementById('clear-console')?.addEventListener('click', () => {
        const consoleOutput = document.getElementById('console-output');
        if (consoleOutput) {
            consoleOutput.innerHTML = '<div class="log-line system">[SYSTEM] Log sự kiện đã được xóa sạch.</div>';
        }
    });

    // Toggle simulator display panel
    let simPanelOpen = true;
    document.getElementById('btn-open-simulator')?.addEventListener('click', () => {
        const panel = document.getElementById('sim-control-panel');
        if (panel) {
            if (simPanelOpen) {
                panel.classList.add('hidden');
                simPanelOpen = false;
            } else {
                panel.classList.remove('hidden');
                simPanelOpen = true;
            }
            // Recalculate layout
            setTimeout(updateConnections, 50);
        }
    });

    // Make all nodes and containers draggable
    document.querySelectorAll('.diagram-node, .sensor-container, .actuator-container, .devices-container').forEach(el => {
        makeDraggable(el);
    });

    // ==========================================================================
    // 7. SIDEBAR RESIZE FUNCTIONALITY
    // ==========================================================================
    const sidebar = document.getElementById('sidebar-panel');
    const resizeHandle = document.getElementById('sidebar-resize-handle');
    
    if (sidebar && resizeHandle) {
        let isResizing = false;
        let startX = 0;
        let startWidth = 0;

        resizeHandle.addEventListener('mousedown', (e) => {
            isResizing = true;
            startX = e.clientX;
            startWidth = sidebar.offsetWidth;
            sidebar.classList.add('resizing');
            resizeHandle.classList.add('active');
            document.body.style.cursor = 'col-resize';
            document.body.style.userSelect = 'none';
            e.preventDefault();
        });

        document.addEventListener('mousemove', (e) => {
            if (!isResizing) return;
            const dx = e.clientX - startX;
            const newWidth = Math.max(320, Math.min(800, startWidth + dx));
            sidebar.style.width = newWidth + 'px';
            // Redraw SVG connections
            if (window.updateConnections) updateConnections();
        });

        document.addEventListener('mouseup', () => {
            if (isResizing) {
                isResizing = false;
                sidebar.classList.remove('resizing');
                resizeHandle.classList.remove('active');
                document.body.style.cursor = '';
                document.body.style.userSelect = '';
                setTimeout(updateConnections, 50);
            }
        });
    }
});
