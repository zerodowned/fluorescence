
uniform vec2 PositionOffset;
uniform mat4 cl_ModelViewProjectionMatrix;

attribute vec2 TexCoord0;
attribute vec3 HueInfo0;

varying vec3 Normal;
varying vec3 HueInfo;

void main(void) {
    gl_Vertex.z = gl_Vertex.z * 10000000;
    gl_Vertex.z = gl_Vertex.z * 10000000;
    vec4 movedPosition = vec4(gl_Vertex.x - PositionOffset.x, gl_Vertex.y - PositionOffset.y, gl_Vertex.z, gl_Vertex.w);

    gl_Position = cl_ModelViewProjectionMatrix * movedPosition;

    gl_TexCoord[0].xy = TexCoord0;

    Normal = gl_Normal;
    HueInfo = HueInfo0;
}

