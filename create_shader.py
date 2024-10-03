import argparse
import subprocess
import requests
import sys

def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Script to create and modify a shader using Hygen and ShaderToy API")
    parser.add_argument("ID", help="ShaderToy ID")
    parser.add_argument("Name", help="Name for the new shader")
    args = parser.parse_args()
    
    shader_id = args.ID
    shader_name = args.Name
    
    # Step 1: Run the Hygen command
    hygen_command = f"hygen shader new {shader_name}"
    subprocess.run(hygen_command, shell=True, check=True)
    
    # Step 2: Access ShaderToy API with detailed headers
    api_url = f"https://www.shadertoy.com/api/v1/shaders/{shader_id}?key=rt8lRn"
    
    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36",
        "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "Accept-Encoding": "gzip, deflate, br, zstd",
        "Accept-Language": "en-US,en;q=0.9",
        "Cookie": "scmp=0; sdt=dN4fjpe5j816salq6k59acu07en; AWSALB=UpvBPWeGgm9a0utZM7cMZx9vZ5BihVVK2B0m9E0J/pNheoKYSHasqim5+4NM4EfFjIDAq/WhWa+S577rB4rs3fIn9FeCDlnbBZFiKibtGxeaRQfX+4JXdAZxo/zu; AWSALBCORS=UpvBPWeGgm9a0utZM7cMZx9vZ5BihVVK2B0m9E0J/pNheoKYSHasqim5+4NM4EfFjIDAq/WhWa+S577rB4rs3fIn9FeCDlnbBZFiKibtGxeaRQfX+4JXdAZxo/zu",
        "Cache-Control": "no-cache",
        "Pragma": "no-cache"
    }
    
    response = requests.get(api_url, headers=headers)
    
    if response.status_code == 403:
        print("Error: Received a 403 Forbidden response. This could be due to an invalid API key or other access restrictions.")
        return
    elif response.status_code != 200:
        print(f"Error: Received an unexpected status code {response.status_code}.")
        return
    
    shader_data = response.json()
    # Check if the response contains shader information
    if 'Shader' not in shader_data or 'renderpass' not in shader_data['Shader']:
        print(shader_data)
        print("Shader data not found in the response.")
        sys.exit(1)

    shader_code = shader_data['Shader']['renderpass'][0]['code']
    
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