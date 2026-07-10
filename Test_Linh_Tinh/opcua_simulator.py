import asyncio
import logging
import random
from asyncua import Server, ua

# Cấu hình log
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("OPCUA_Simulator")

async def main():
    # 1. Khởi tạo OPC UA Server
    server = Server()
    await server.init()
    server.set_endpoint("opc.tcp://127.0.0.1:4840")
    server.set_server_name("Hydroponics NFT Digital Twin Server")

    # 2. Đăng ký Namespace
    uri = "http://hydroponics.digitaltwin"
    idx = await server.register_namespace(uri)
    logger.info(f"Namespace registered with Index: {idx}")

    # Lấy node Objects root
    objects = server.nodes.objects

    # 3. Tạo thư mục cấu trúc
    sensors_folder = await objects.add_folder(idx, "Sensors")
    actuators_folder = await objects.add_folder(idx, "Actuators")

    # --- Đăng ký các Node Cảm biến (Sensors) ---
    # Node ID tùy chỉnh dạng string để khớp với thiết kế: ns=2;s=sensor.temp
    n_temp = await sensors_folder.add_variable(ua.NodeId("sensor.temp", idx), "Air_Temperature", 27.5)
    n_hum = await sensors_folder.add_variable(ua.NodeId("sensor.humidity", idx), "Air_Humidity", 75.0)
    n_wtemp = await sensors_folder.add_variable(ua.NodeId("sensor.water_temp", idx), "Water_Temperature", 24.5)
    n_tds = await sensors_folder.add_variable(ua.NodeId("sensor.tds", idx), "TDS_Concentration", 650.0)
    n_ph = await sensors_folder.add_variable(ua.NodeId("sensor.ph", idx), "pH_Level", 6.0)
    n_light = await sensors_folder.add_variable(ua.NodeId("sensor.light", idx), "Light_Intensity", 12000.0)
    
    # Mực nước (Y26 PNP x 4)
    n_wlevel1 = await sensors_folder.add_variable(ua.NodeId("sensor.water_level_1", idx), "Water_Level_1", True)
    n_wlevel2 = await sensors_folder.add_variable(ua.NodeId("sensor.water_level_2", idx), "Water_Level_2", True)
    n_wlevel3 = await sensors_folder.add_variable(ua.NodeId("sensor.water_level_3", idx), "Water_Level_3", False)
    n_wlevel4 = await sensors_folder.add_variable(ua.NodeId("sensor.water_level_4", idx), "Water_Level_4", False)

    # Đặt quyền ghi cho biến cảm biến để simulator có thể update
    for node in [n_temp, n_hum, n_wtemp, n_tds, n_ph, n_light, n_wlevel1, n_wlevel2, n_wlevel3, n_wlevel4]:
        await node.set_writable()

    # --- Đăng ký các Node Thiết bị (Actuators) ---
    # Bơm dinh dưỡng (PWM 0-255)
    n_pump_dd1 = await actuators_folder.add_variable(ua.NodeId("actuator.pump_dd_1", idx), "Pump_Nutrient_A", 0.0)
    n_pump_dd2 = await actuators_folder.add_variable(ua.NodeId("actuator.pump_dd_2", idx), "Pump_Nutrient_B", 0.0)
    n_pump_dd3 = await actuators_folder.add_variable(ua.NodeId("actuator.pump_dd_3", idx), "Pump_pH_Down", 0.0)
    n_pump_air = await actuators_folder.add_variable(ua.NodeId("actuator.pump_air", idx), "Pump_Aerator", 0.0)
    
    # Quạt (PWM 0-255)
    n_fan1 = await actuators_folder.add_variable(ua.NodeId("actuator.fan_1", idx), "Fan_1", 0.0)
    n_fan2 = await actuators_folder.add_variable(ua.NodeId("actuator.fan_2", idx), "Fan_2", 0.0)
    n_fan3 = await actuators_folder.add_variable(ua.NodeId("actuator.fan_3", idx), "Fan_3", 0.0)
    n_fan4 = await actuators_folder.add_variable(ua.NodeId("actuator.fan_4", idx), "Fan_4", 0.0)
    
    # Relay ON/OFF
    n_pump_220v = await actuators_folder.add_variable(ua.NodeId("actuator.pump_220v", idx), "Main_Pump_220V", False)
    n_light1 = await actuators_folder.add_variable(ua.NodeId("actuator.light_1", idx), "Grow_Light_1", False)
    n_light2 = await actuators_folder.add_variable(ua.NodeId("actuator.light_2", idx), "Grow_Light_2", False)
    n_light3 = await actuators_folder.add_variable(ua.NodeId("actuator.light_3", idx), "Grow_Light_3", False)
    n_light4 = await actuators_folder.add_variable(ua.NodeId("actuator.light_4", idx), "Grow_Light_4", False)

    # --- Đăng ký các Node AI Dự đoán (AI Predictions) ---
    ai_folder = await objects.add_folder(idx, "AI_Predictions")
    n_growth_percent = await ai_folder.add_variable(ua.NodeId("ai.growth_percent", idx), "Growth_Percent", 15.0)
    n_days_remaining = await ai_folder.add_variable(ua.NodeId("ai.days_remaining", idx), "Days_Remaining", 22.0)

    # Đặt quyền ghi cho actuators và AI để Web App, XGBoost model hoặc NX MCD có thể điều khiển/cập nhật trực tiếp
    for node in [n_pump_dd1, n_pump_dd2, n_pump_dd3, n_pump_air, n_fan1, n_fan2, n_fan3, n_fan4, n_pump_220v, n_light1, n_light2, n_light3, n_light4, n_growth_percent, n_days_remaining]:
        await node.set_writable()

    # Khởi động server
    logger.info("Starting OPC UA Server...")
    async with server:
        logger.info("OPC UA Server is running at: opc.tcp://127.0.0.1:4840")
        
        # --- Khởi tạo các giá trị mô phỏng môi trường ảo ---
        sim_temp = 28.0
        sim_ph = 6.2
        sim_tds = 600.0
        sim_water_volume = 40.0  # Lít (bể chứa 50-80L)
        sim_growth_percent = 15.0

        while True:
            # Đọc các giá trị điều khiển từ Client (Web App hoặc MCD vừa ghi vào)
            fan1_val = await n_fan1.get_value()
            fan2_val = await n_fan2.get_value()
            pump_dd1_val = await n_pump_dd1.get_value()
            pump_dd3_val = await n_pump_dd3.get_value()
            pump_220v_val = await n_pump_220v.get_value()
            light1_val = await n_light1.get_value()

            # --- VÒNG LẶP MÔ PHỎNG VẬT LÝ ẢO (Physics Simulator Engine) ---
            
            # 1. Giả lập Nhiệt độ không khí (Nếu bật quạt -> nhiệt độ giảm, không bật -> nhiệt độ tăng theo môi trường)
            active_fans = (fan1_val + fan2_val) / 510.0  # Tỉ lệ quạt chạy (0.0 đến 1.0)
            if active_fans > 0.1:
                sim_temp -= active_fans * 0.15 + random.uniform(-0.02, 0.02)
                sim_temp = max(24.0, sim_temp)  # Nhiệt độ phòng tối thiểu
            else:
                sim_temp += 0.05 + random.uniform(-0.02, 0.02)  # Nóng lên do đèn led hoặc bức xạ mặt trời
                sim_temp = min(32.5, sim_temp)

            # 2. Giả lập TDS (Cây hấp thụ -> giảm dần. Nếu bơm dinh dưỡng A/B bật -> TDS tăng)
            sim_tds -= 0.02  # Cây hấp thụ thụ động
            if pump_dd1_val > 0:
                sim_tds += (pump_dd1_val / 255.0) * 5.0  # Tăng TDS từ dung dịch đậm đặc
            sim_tds = max(100.0, min(1200.0, sim_tds))

            # 3. Giả lập pH (Dao động tự nhiên tăng nhẹ, nếu bơm pH Down bật -> pH giảm mạnh)
            sim_ph += 0.002 + random.uniform(-0.004, 0.004) # Xu hướng tăng pH tự nhiên
            if pump_dd3_val > 0:
                sim_ph -= (pump_dd3_val / 255.0) * 0.08  # Bơm pH Down châm acid để hạ pH
            sim_ph = max(4.0, min(8.5, sim_ph))

            # 4. Giả lập Mực nước bể chứa (Bơm chính hoạt động bơm nước lên máng NFT -> nước trong bể giảm đi)
            if pump_220v_val:
                sim_water_volume -= 0.05  # Nước đẩy lên máng trồng
            else:
                sim_water_volume += 0.04  # Nước chảy hồi ngược về bể chứa
            
            sim_water_volume = max(10.0, min(60.0, sim_water_volume))

            # Ánh xạ thể tích nước (liters) sang trạng thái 4 cảm biến phao ảo Y26 (10L - 20L - 30L - 40L)
            w1 = sim_water_volume >= 15.0
            w2 = sim_water_volume >= 25.0
            w3 = sim_water_volume >= 35.0
            w4 = sim_water_volume >= 45.0

            # 5. Giả lập cường độ ánh sáng (Phụ thuộc trạng thái LED Grow)
            sim_light = 500.0  # Ánh sáng phòng tối thiểu
            if light1_val:
                sim_light += 15000.0  # Đèn LED Grow sáng bổ sung

            # 6. Giả lập sinh trưởng cây ảo (Tăng 0.05% mỗi giây, đạt 100% reset về 0%)
            sim_growth_percent += 0.05
            if sim_growth_percent > 100.0:
                sim_growth_percent = 0.0
            sim_days_remaining = max(0.0, 30.0 * (1.0 - sim_growth_percent / 100.0))

            # --- UPDATE LÊN OPC UA SERVER ---
            await n_temp.write_value(round(sim_temp, 2))
            await n_hum.write_value(round(75.0 + random.uniform(-1.0, 1.0), 1))
            await n_wtemp.write_value(round(sim_temp - 2.5 + random.uniform(-0.1, 0.1), 2))
            await n_tds.write_value(round(sim_tds, 1))
            await n_ph.write_value(round(sim_ph, 2))
            await n_light.write_value(sim_light)
            await n_wlevel1.write_value(w1)
            await n_wlevel2.write_value(w2)
            await n_wlevel3.write_value(w3)
            await n_wlevel4.write_value(w4)
            await n_growth_percent.write_value(round(sim_growth_percent, 2))
            await n_days_remaining.write_value(round(sim_days_remaining, 1))

            logger.info(
                f"[SIM] Temp: {sim_temp:.1f}°C | TDS: {sim_tds:.1f} ppm | pH: {sim_ph:.2f} | "
                f"Vol: {sim_water_volume:.1f}L (Phao: {int(w1)}{int(w2)}{int(w3)}{int(w4)}) | "
                f"Growth: {sim_growth_percent:.2f}% ({sim_days_remaining:.1f} days left) | "
                f"Fan1: {fan1_val} | Pump220V: {pump_220v_val}"
            )

            await asyncio.sleep(1.0)  # Cập nhật mỗi giây

if __name__ == "__main__":
    asyncio.run(main())
