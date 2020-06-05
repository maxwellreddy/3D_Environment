/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec2 fTex2;
in  vec4 color;
out vec4 fColor;

uniform int fogType;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;
uniform vec4 fogColor;
uniform sampler2D tex2;

uniform bool enableFloorTexture;

vec4 fog(vec4 color){
    float fogDistance = gl_FragCoord.z / gl_FragCoord.w;
    float fog;
    if (fogType == 0) {
        return color;
    }
    else if (fogType == 1){
        fog = (fogEnd - fogDistance) / (fogEnd - fogStart);
    }
    else if (fogType == 2){
        fog = exp(-(fogDistance * fogDensity));
    }
    else if (fogType == 3){
        fog = exp(-pow(fogDistance * fogDensity, 2));
    }

    fColor = mix(fogColor, color, clamp(fog, 0, 1));

    return fColor;
}

void main() 
{ 
    fColor = fog(color);

    //if (enableFloorTexture){
    //    fColor = texture(tex2, fTex2) * fColor;
    //}
} 

