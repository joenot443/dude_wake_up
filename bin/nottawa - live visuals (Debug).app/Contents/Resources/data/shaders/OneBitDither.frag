#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float scale;
uniform vec3 lightColor;
uniform vec3 darkColor;
in vec2 coord;
out vec4 outputColor;

void main(  )
{
    // Set resolution scale factor by increasing exponent 0.5 is being raised to
    float resScaleExp = 1.0 * scale;
    float ditherResScale = pow(0.5,resScaleExp);
    
    // Map coords to lower res
    vec2 lowResCoord = floor((coord-dimensions.xy/2.0) * ditherResScale) / ditherResScale + dimensions.xy/2.0;

    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = lowResCoord/dimensions.xy;

    // Get pixel information from uv location
    vec4 texColor = texture(tex, uv);

    // Greyscale it
    float grey = (texColor.r + texColor.g + texColor.b)/3.0;
    
    // Split screen into tiny grids
    float minigrid =3.0;
    
    // Set the amount of dither levels to the amount of pixels in each grid, or set it manually to a smaller value
    float ditherLevels = minigrid*minigrid;
    //ditherLevels = 6.0;
    
    // Assign pixel number to each pixel in grid
    float pixelNo = mod(lowResCoord.y * ditherResScale,minigrid) * minigrid + mod(lowResCoord.x * ditherResScale,minigrid);
    pixelNo = mod(pixelNo+mod(resScaleExp + 1.0,2.0),ditherLevels);
    
    // Add temporal blurring (warning: can be painful to look at, probably not great for epileptic people)
    
    // Offset pixel grey value by the number of the pixel
    float pixelValueOffset = pixelNo/ditherLevels + 0.015*(1.0+sin(1.0*time)) + 0.01*(1.0+sin(1.7*time));
    
    // Floor and clamp offset grey value to get binary value (floor not round because the offset will add an average of 0.5ish anyway)
    float ditheredGrey = clamp(floor(grey + pixelValueOffset), 0.0, 1.0);
    
    // Set frag colour
    vec3 colorMapped = ditheredGrey * lightColor + (1.0 - ditheredGrey) * darkColor;
    outputColor = vec4(colorMapped, 1);
}


