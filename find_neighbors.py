import fitz

pdf_path = r"c:\Users\LENOVO\Desktop\DATN_START\Hardware\PCB_Design\Schematic_DATN-VER2_2026-06-19.pdf"
doc = fitz.open(pdf_path)
page = doc[0]
text_instances = page.get_text("words")

with open("neighbors_output.txt", "w", encoding="utf-8") as f:
    # Let's find all words with Y in range 580 to 880.
    pin_region_words = [w for w in text_instances if 550 <= w[1] <= 880]

    # Let's group them by Y (with tolerance, e.g., 3 units)
    grouped_by_y = {}
    for w in pin_region_words:
        y_center = (w[1] + w[3]) / 2
        found = False
        for y_group in grouped_by_y:
            if abs(y_group - y_center) < 4:
                grouped_by_y[y_group].append(w)
                found = True
                break
        if not found:
            grouped_by_y[y_center] = [w]

    f.write("--- PINS ALIGNED BY Y (Horizontal Lines) ---\n")
    for y in sorted(grouped_by_y.keys()):
        words = sorted(grouped_by_y[y], key=lambda w: w[0])
        line_str = " | ".join([f"{w[4]} ({w[0]:.1f})" for w in words])
        f.write(f"Y={y:.1f}: {line_str}\n")

    # Let's also group words by X (with tolerance, e.g., 5 units)
    grouped_by_x = {}
    for w in pin_region_words:
        x_center = (w[0] + w[2]) / 2
        found = False
        for x_group in grouped_by_x:
            if abs(x_group - x_center) < 5:
                grouped_by_x[x_group].append(w)
                found = True
                break
        if not found:
            grouped_by_x[x_center] = [w]

    f.write("\n--- PINS ALIGNED BY X (Vertical Columns) ---\n")
    for x in sorted(grouped_by_x.keys()):
        words = sorted(grouped_by_x[x], key=lambda w: w[1])
        line_str = " -> ".join([f"{w[4]} (Y={w[1]:.1f})" for w in words])
        f.write(f"X={x:.1f}: {line_str}\n")

print("Analysis output written to neighbors_output.txt")
