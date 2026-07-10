import os
import gzip

Import("env")

# Paths
source_dir = os.path.join(env.get("PROJECT_DIR"), "src", "ota")
html_path = os.path.join(source_dir, "index.html")
css_path = os.path.join(source_dir, "style.css")
js_path = os.path.join(source_dir, "script.js")
output_path = os.path.join(source_dir, "web_assets.h")

def to_hex_array(data):
    # Convert bytes into a comma-separated hex values, split into lines for readability
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
            
    # Compress with gzip (best compression ratio = 9)
    html_gz = gzip.compress(html_content.encode('utf-8'), compresslevel=9)
    css_gz = gzip.compress(css_content.encode('utf-8'), compresslevel=9)
    js_gz = gzip.compress(js_content.encode('utf-8'), compresslevel=9)

    html_hex = to_hex_array(html_gz)
    css_hex = to_hex_array(css_gz)
    js_hex = to_hex_array(js_gz)

    # Generate C++ header content with gzip byte arrays
    cpp_content = f"""#pragma once
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

    with open(output_path, "w", encoding="utf-8") as f:
        f.write(cpp_content)
    print("web_assets.h (Gzip Compressed) generated successfully!")

# Hook into the build process
env.AddPreAction("buildprog", build_web_assets)
# Run once initially
build_web_assets()
