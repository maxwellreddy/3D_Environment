/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/
#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec4 vColor;
in  vec4 vNormal;
in  vec2 vTexture;
out vec4 color;
out vec2 fTex2;

uniform vec4 dAmbientProduct, dDiffuseProduct, dSpecularProduct;
uniform vec4 pAmbientProduct, pDiffuseProduct, pSpecularProduct;
uniform vec4 GlobalAmbient;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 dLightPosition;  
uniform vec4 dLightDirection;
uniform vec4 pLightPosition; 
uniform vec4 pLightDirection;
uniform float Shininess;
uniform float lightAngle;

uniform bool enableLighting;
uniform bool enableWireFrame;
uniform bool enableSpotlight;
uniform bool enableFloorTexture;


vec4 positionalLight(void) {
	vec3 pos = (ModelView * vPosition).xyz;

	vec3 L = normalize( pLightPosition.xyz - pos );
	vec3 E = normalize( -pos );
	vec3 H = normalize( L + E );

	vec3 N = normalize(Normal_Matrix * vNormal.xyz);

	float dist = distance(pLightPosition.xyz, pos);
	float attenuation = 1 / (2.0 + 0.01 * dist + 0.001 * pow(dist, 2));

	vec4 ambient = pAmbientProduct;

	float d = max(dot(L, N), 0.0);
	vec4 diffuse = d * pDiffuseProduct;

	float s = pow(max(dot(N, H), 0.0), Shininess);
	vec4 specular = s * pSpecularProduct;

	if (dot(L, N) < 0.0) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}

	if (enableSpotlight)  {
		vec4 point = attenuation * (ambient + diffuse + specular);

		vec3 spL = -L;
		vec3 spFocus = normalize(pLightDirection.xyz - pLightPosition.xyz);

		float spAttenuation = pow(dot(spL, spFocus), 15.0);

		vec4 spotlight = spAttenuation * point;

		if (dot(spL, spFocus) < lightAngle) {
			spotlight = vec4(0, 0, 0, 1);
		}

		return point + spotlight;
	}
  else{
		return attenuation * (ambient + diffuse + specular);
  } 
}

void main()
{
    gl_Position = Projection * ModelView * vPosition;

    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
	
    vec3 L = normalize( -dLightDirection.xyz );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
      // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    vec3 N = normalize(Normal_Matrix * vNormal.xyz);

   float attenuation = 1.0;
   
   vec4 ambient = dAmbientProduct;
   
   	float d = max(dot(L, N), 0.0);
	vec4 diffuse = d * dDiffuseProduct;

	float s = pow(max(dot(N, H), 0.0), Shininess);
	vec4 specular = s * dSpecularProduct;

	if (dot(L, N) < 0.0) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}

	vec4 I = attenuation * (ambient + diffuse + specular);


	fTex2 = vTexture;

    if (enableLighting && !enableWireFrame ){
      color = GlobalAmbient + I + positionalLight();
    }
    
    else{
      color = vColor;
    }


}
