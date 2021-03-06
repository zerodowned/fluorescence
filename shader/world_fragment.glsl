
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D HueTexture;
uniform sampler2D ObjectTexture;
uniform sampler2D RenderEffectTexture;

uniform vec3 AmbientLightIntensity;
uniform vec3 GlobalLightIntensity;
uniform vec3 GlobalLightDirection;

uniform float RenderEffectTime;

varying vec3 Normal;
varying vec3 HueInfo;
varying float Material;

void renderMaterialDefault(inout vec4 rgba);
void renderMaterialWater(inout vec4 rgba);

void main(void) {
    // sample actual pixel color
    vec4 rgba = texture2D(ObjectTexture, gl_TexCoord[0].xy);

    if (rgba.a == 0.0) {
        discard;
    }

    rgba.a *= HueInfo.z;

    if (HueInfo[1] != 0.0) {
        // object has hue
        if (HueInfo[0] == 0.0 || (HueInfo[0] == 1.0 && rgba.r == rgba.g && rgba.g == rgba.b)) {
            // object is either non-partial-hue or current pixel is grey
            vec2 hueTexCoord;
            float hueId = HueInfo[1];
            hueTexCoord.x = rgba.r;
            hueTexCoord.y = hueId / 3000.0;

            rgba.rgb = texture2D(HueTexture, hueTexCoord).rgb;
        }
    }
    
    switch (int(Material)) {
        default:
        case 0:
            renderMaterialDefault(rgba);
            break;
        case 1:
            renderMaterialWater(rgba);
            break;
    }
    
    gl_FragColor = rgba;
}

void renderMaterialDefault(inout vec4 rgba) {
    float globalAngle = clamp(dot(GlobalLightDirection, Normal), 0.0, 1.0);
    rgba.rgb *= (AmbientLightIntensity + GlobalLightIntensity * globalAngle);
}

void renderMaterialWater(inout vec4 rgba) {
    vec3 normal = Normal;
    
    float normalizedTime = mod(RenderEffectTime, 13.0);
    normalizedTime /= 13.0;

    // tile is water, gl_Vertex.xy is storend in Normal.xy
    // wave textures are 512x512 pixels
    
    // get texture coordinates inside texture
    vec2 waveCoordsBase = vec2(
        float(int(Normal.x) % 512) / 512.0,
        float(int(Normal.y) % 512) / 512.0
    );
    
    if (waveCoordsBase.x < 0.0) {
        waveCoordsBase.x += 1.0;
    }
    
    // render effect texture is 2048x2048
    waveCoordsBase *= 0.25;
    
    vec2 waveCoords1 = waveCoordsBase + normalizedTime / 4.0;
    while (waveCoords1.x >= 0.25) { waveCoords1.x -= 0.25; }
    while (waveCoords1.y >= 0.25) { waveCoords1.y -= 0.25; }
    
    vec4 waveNormals1 = texture2D(RenderEffectTexture, waveCoords1);
    
    vec2 waveCoords2 = waveCoordsBase - normalizedTime / 4.0;
    while (waveCoords2.x < 0.0) { waveCoords2.x += 0.25; }
    while (waveCoords2.y < 0.0) { waveCoords2.y += 0.25; }
    waveCoords2.x += 0.25;
    vec4 waveNormals2 = texture2D(RenderEffectTexture, waveCoords2);
    
    normal.x = waveNormals1.r - waveNormals2.r;
    normal.y = waveNormals1.b - waveNormals2.b;
    normal.z = 2.0; // make sure the vector is still pointing mostly upwards
    
    normal = normalize(normal);
    
    float globalAngle = clamp(dot(GlobalLightDirection, normal), 0.0, 1.0);
    rgba.rgb *= (AmbientLightIntensity + GlobalLightIntensity * globalAngle);
}
