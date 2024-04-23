import requests
import sys

def download_shader(shader_id, api_key):
    # Construct the URL for the API request
    url = f"https://www.shadertoy.com/api/v1/shaders/{shader_id}?key={api_key}"
    print(url)
    # Make the request to ShaderToy API
    response = requests.get(url)
    print(response.json())
    print(response)
    if response.status_code != 200:
        print("Failed to fetch the shader. Please check the shader ID and your API key.")
        sys.exit(1)
    
    # Parse the JSON response
    shader_data = response.json()
    
    # Check if the response contains shader information
    if 'Shader' not in shader_data or 'code' not in shader_data['Shader']:
        print("Shader data not found in the response.")
        sys.exit(1)
    
    # Extract the shader code
    shader_code = shader_data['Shader']['code']
    
    # Save the shader code to a file
    with open('result.frag', 'w') as file:
        file.write(shader_code)
    
    print("Shader downloaded and saved to result.frag.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <shaderId>")
        sys.exit(1)
    
    shader_id = sys.argv[1]
    api_key = "rt8lRn"  # Your Shadertoy API key
    download_shader(shader_id, api_key)
