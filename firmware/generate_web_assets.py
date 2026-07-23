import os
import gzip

try:
    Import("env")
    project_dir = env.get("PROJECT_DIR")
    running_under_platformio = True
except Exception:
    project_dir = os.path.dirname(os.path.abspath(__file__))
    running_under_platformio = False

# Paths
source_dir = os.path.join(project_dir, "web")
html_path = os.path.join(source_dir, "index.html")
css_path = os.path.join(source_dir, "style.css")
js_path = os.path.join(source_dir, "script.js")
output_path = os.path.join(project_dir, "include", "web_assets.h")

def to_hex_array(data):
    # Convert bytes into comma-separated hex values, split into lines for readability
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_vals = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"  {hex_vals}")
    return ",\n".join(lines)

def build_web_assets(*args, **kwargs):
    print("Generating web_assets.h (Gzip Compressed) from HTML/CSS/JS files...")
    
    # Ensure source directory exists
    if not os.path.exists(source_dir):
        os.makedirs(source_dir)
        print(f"Created directory: {source_dir}")
    
    # Read files with default fallback if they don't exist yet
    html_content = ""
    if os.path.exists(html_path):
        with open(html_path, "r", encoding="utf-8") as f:
            html_content = f.read()
            
    css_content = ""
    if os.path.exists(css_path):
        with open(css_path, "r", encoding="utf-8") as f:
            css_content = f.read()
            
    js_content = ""
    if os.path.exists(js_path):
        with open(js_path, "r", encoding="utf-8") as f:
            js_content = f.read()
            
    # Compress with gzip (mtime=0 for deterministic output hash across builds)
    html_gz = gzip.compress(html_content.encode('utf-8'), compresslevel=9, mtime=0)
    css_gz = gzip.compress(css_content.encode('utf-8'), compresslevel=9, mtime=0)
    js_gz = gzip.compress(js_content.encode('utf-8'), compresslevel=9, mtime=0)

    html_hex = to_hex_array(html_gz)
    css_hex = to_hex_array(css_gz)
    js_hex = to_hex_array(js_gz)

    # Generate C++ header content with gzip byte arrays and warning header
    cpp_content = f"""// GENERATED FILE - DO NOT EDIT MANUALLY
// Automatically generated from web/index.html, web/style.css, and web/script.js

#pragma once
#include <Arduino.h>

const uint8_t INDEX_HTML_GZ[] PROGMEM = {{
{html_hex}
}};
const size_t INDEX_HTML_GZ_LEN = {len(html_gz)};

const uint8_t STYLE_CSS_GZ[] PROGMEM = {{
{css_hex}
}};
const size_t STYLE_CSS_GZ_LEN = {len(css_gz)};

const uint8_t SCRIPT_JS_GZ[] PROGMEM = {{
{js_hex}
}};
const size_t SCRIPT_JS_GZ_LEN = {len(js_gz)};
"""

    existing_content = ""
    if os.path.exists(output_path):
        with open(output_path, "r", encoding="utf-8") as f:
            existing_content = f.read()

    if existing_content != cpp_content:
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(cpp_content)
        print("web_assets.h (Gzip Compressed) generated successfully!")
    else:
        print("web_assets.h is up to date (no changes).")

# `extra_scripts = pre:` đã nạp file này trước khi PlatformIO quét dependency.
# Tạo header ngay lúc đó để web_api.cpp luôn biên dịch với asset mới. Hook vào
# `buildprog` là quá muộn vì các object C++ có thể đã được compile xong.
if running_under_platformio:
    build_web_assets()
elif __name__ == "__main__":
    build_web_assets()
