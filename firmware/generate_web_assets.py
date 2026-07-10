import os

Import("env")

# Paths
source_dir = os.path.join(env.get("PROJECT_DIR"), "src", "ota")
html_path = os.path.join(source_dir, "index.html")
css_path = os.path.join(source_dir, "style.css")
js_path = os.path.join(source_dir, "script.js")
output_path = os.path.join(source_dir, "web_assets.h")

def build_web_assets(*args, **kwargs):
    print("Generating web_assets.h from HTML/CSS/JS files...")
    
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
            
    # Generate C++ header content with raw string literals
    cpp_content = f"""#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral({html_content})rawliteral";
const char STYLE_CSS[] PROGMEM = R"rawliteral({css_content})rawliteral";
const char SCRIPT_JS[] PROGMEM = R"rawliteral({js_content})rawliteral";
"""

    with open(output_path, "w", encoding="utf-8") as f:
        f.write(cpp_content)
    print("web_assets.h generated successfully!")

# Hook into the build process
env.AddPreAction("buildprog", build_web_assets)
# Run once initially
build_web_assets()
