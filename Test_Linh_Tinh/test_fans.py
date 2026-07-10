import asyncio
import sys
from asyncua import Client

async def main():
    url = "opc.tcp://127.0.0.1:4840"
    print(f"Đang kết nối tới OPC UA Server: {url}...")
    
    try:
        async with Client(url=url) as client:
            print("Kết nối thành công!")
            
            # Lấy các node quạt dựa trên Node ID dạng string
            # Namespace index thường là 2 (tương ứng với idx ở file simulator)
            idx = 2
            fan_nodes = {
                1: client.get_node(f"ns={idx};s=actuator.fan_1"),
                2: client.get_node(f"ns={idx};s=actuator.fan_2"),
                3: client.get_node(f"ns={idx};s=actuator.fan_3"),
                4: client.get_node(f"ns={idx};s=actuator.fan_4"),
            }
            
            while True:
                print("\n--- MENU ĐIỀU KHIỂN 4 QUẠT ---")
                print("1. Đặt tốc độ Quạt 1")
                print("2. Đặt tốc độ Quạt 2")
                print("3. Đặt tốc độ Quạt 3")
                print("4. Đặt tốc độ Quạt 4")
                print("5. Bật tất cả quạt (Tốc độ tối đa 255)")
                print("6. Tắt tất cả quạt (Tốc độ 0)")
                print("7. Thoát chương trình")
                
                choice = input("Chọn chức năng (1-7): ").strip()
                
                if choice in ["1", "2", "3", "4"]:
                    fan_id = int(choice)
                    try:
                        speed = int(input(f"Nhập tốc độ cho Quạt {fan_id} (0 - 5000): ").strip())
                        if 0 <= speed <= 5000:
                            node = fan_nodes[fan_id]
                            # Ghi giá trị dưới dạng số thực (Float/Double)
                            await node.write_value(float(speed))
                            print(f"Đã cập nhật Quạt {fan_id} thành {speed}")
                        else:
                            print("Lỗi: Tốc độ phải nằm trong khoảng từ 0 đến 5000.")
                    except ValueError:
                        print("Lỗi: Vui lòng nhập một số nguyên hợp lệ.")
                
                elif choice == "5":
                    for fan_id, node in fan_nodes.items():
                        await node.write_value(255.0)
                    print("Đã bật tất cả 4 quạt lên tốc độ tối đa (255)")
                
                elif choice == "6":
                    for fan_id, node in fan_nodes.items():
                        await node.write_value(0.0)
                    print("Đã tắt tất cả 4 quạt (Tốc độ 0)")
                
                elif choice == "7":
                    print("Thoát chương trình điều khiển.")
                    break
                else:
                    print("Lựa chọn không hợp lệ, vui lòng chọn lại.")
                    
    except Exception as e:
        print(f"Lỗi kết nối hoặc ghi dữ liệu: {e}")
        print("Đảm bảo rằng file 'opcua_simulator.py' đang được chạy ở một cửa sổ terminal khác.")

if __name__ == "__main__":
    asyncio.run(main())
