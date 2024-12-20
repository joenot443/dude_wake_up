import argparse
import subprocess
import requests
import json
import sys

def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Script to create and modify a shader using Hygen and ShaderToy data")
    parser.add_argument("ID", help="ShaderToy ID")
    parser.add_argument("Name", help="Name for the new shader")
    args = parser.parse_args()
    
    shader_id = args.ID
    shader_name = args.Name
    
    # Step 1: Run the Hygen command
    hygen_command = f"hygen shader new {shader_name}"
    subprocess.run(hygen_command, shell=True, check=True)
    
    # Step 2: Access ShaderToy data using curl
    headers = {
        'accept': '*/*',
        'accept-language': 'en-US,en;q=0.9',
        'content-type': 'application/x-www-form-urlencoded',
        'origin': 'https://www.shadertoy.com',
        'referer': f'https://www.shadertoy.com/view/{shader_id}',
        'sec-ch-ua': '"Chromium";v="130", "Google Chrome";v="130", "Not?A_Brand";v="99"',
        'sec-ch-ua-mobile': '?0',
        'sec-ch-ua-platform': '"macOS"',
        'sec-fetch-dest': 'empty',
        'sec-fetch-mode': 'cors',
        'sec-fetch-site': 'same-origin',
        'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36'
    }

    data = f's=%7B%20%22shaders%22%20%3A%20%5B%22{shader_id}%22%5D%20%7D&nt=1&nl=1&np=1'
    response = requests.post('https://www.shadertoy.com/shadertoy', headers=headers, data=data)

    if response.status_code != 200:
        print(f"Error: Received status code {response.status_code}")
        sys.exit(1)

    # Parse JSON response to extract shader code
    try:
        shader_data = response.json()
        shader_code = shader_data[0]['renderpass'][0]['code']
        print(shader_code)
    except (KeyError, IndexError, json.JSONDecodeError) as e:
        print("Error parsing shader data:", e)
        sys.exit(1)
    
    # Step 3: Modify the shader file
    shader_file_path = f"./shaders/{shader_name}.frag"
    
    # Read the existing file and replace content from line 9 onwards
    with open(shader_file_path, 'r') as file:
        lines = file.readlines()
    
    # Replace content from line 9 onwards
    if len(lines) >= 9:
        lines = lines[:8] + [shader_code]
    else:
        lines.append(shader_code)
    
    # Write the modified content back to the file
    with open(shader_file_path, 'w') as file:
        file.writelines(lines)

    # Step 4: Perform the string replacements
    with open(shader_file_path, 'r') as file:
        content = file.read()
    
    content = content.replace("iResolution", "dimensions")
    content = content.replace("iChannel0", "tex")
    content = content.replace("fragCoord", "coord")
    content = content.replace("iTime", "time")
    content = content.replace("fragColor", "outputColor")
    content = content.replace("mainImage", "main")
    content = content.replace("out vec4 outputColor, in vec2 coord", "")

    # Write the final modified content back to the file
    with open(shader_file_path, 'w') as file:
        file.write(content)

if __name__ == "__main__":
    main()