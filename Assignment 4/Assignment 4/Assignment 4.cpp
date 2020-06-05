//Maxwell Reddy
//mcr517
//CS 4533 Homework 4

#include "Angel-yjc.h"
#include "texmap.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint fireworksProgram; // Runs fireworks shader files
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;
GLuint sphere_buffer;
GLuint shadow_buffer;
GLuint fireworks_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 20.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0);
vec4 eye = init_eye;               // current viewer position
vec3 sphereCenter;
vec3 rotateDirection(-9, 0, 4);

mat4 rotationMatrix(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));

int animationFlag = 1; // 1: animation; 0: non-animation. 

int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

string fn;

vector<point4> trianglePoints;
vector<color4> triangleColors;

bool b_pressed = false;
bool shadowVisible = true;
bool shadingVisible = true;
bool enableLighting = true;
bool enableSpotlight = true;
bool wireFrame = false;
bool blendShadow = true;
bool enableFloorTexture = false;
bool enableSphereTexture = true;
bool enableFireworks = true;

int n = 0;
int targetPos = 1;
void getTriangles();
void axis();

int sphereX = 3;
int sphereZ = 5;

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec4 floor_normals[floor_NumVertices];
vec2 floor_textures[floor_NumVertices];

point4 axis_points[6];
color4 axis_colors[6];

const int particles = 300;
color4 fireworks_colors[particles];
vec4 fireworks_velocity[particles];

vector<vec4> sphereShadeFlat;
vector<vec4> sphereShadeSmooth;

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1),  // black
	color4(1.0, 0.0, 0.0, 1),  // red
	color4(1.0, 1.0, 0.0, 1),  // yellow
	color4(0.0, 1.0, 0.0, 1),  // green
	color4(0.0, 0.0, 1.0, 1),  // blue
	color4(1.0, 0.0, 1.0, 1),  // magenta
	color4(1.0, 1.0, 1.0, 1),  // white
	color4(0.0, 1.0, 1.0, 1)   // cyan
};

vec3 posPoints[3] = {
	vec3(3, 1, 5),
	vec3(-1, 1, -4),
	vec3(3.5, 1, -2.5)
};

//----------------------------------------------------------------------------
int Index = 0;

/*----- Shader Lighting Parameters -----*/
color4 global_ambient(1.0, 1.0, 1.0, 1.0);
vec4 d_light_pos(-14, 12, -3, 1);
color4 d_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 d_light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 d_light_specular(0.2, 0.2, 0.2, 1.0);
vec4 d_light_direction(0.1, 0.0, -1.0, 0.0);

color4 floor_ambient(0.2, 0.2, 0.2, 1.0);
color4 floor_diffuse(0.0, 1.0, 0.0, 1.0);
color4 floor_specular(0.0, 0.0, 0.0, 1.0);

color4 sphere_specular(1.0, 0.84, 0.0, 1.0);
color4 sphere_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_ambient(0.2, 0.2, 0.2, 1.0);
float sphere_shininess = 125.0;

vec4 p_light_pos(-14.0, 12.0, -3.0, 1.0);
color4 p_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 p_light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 p_light_specular(1.0, 1.0, 1.0, 1.0);
vec4 sp_light_direction(-6.0, 0.0, -4.5, 1.0);
vec4 p_light_direction(0.0, 0.0, 0.0, 0.0);
float ConstAtt = 2.0;
float LinearAtt = 0.01;
float QuadAtt = 0.001;
float exponent = 15.0;
float lightAngle = 20.0;

int fogType = 0;
color4 fogColor = (0.7, 0.7, 0.7, 0.5);
float fogStart = 0.0;
float fogEnd = 18.0;
float fogDensity = 0.09;

GLuint tex1;
GLuint tex2;

float fireworksTime = 0.0;

point4 light_position(-14.0, 12.0, -3.0, 1.0);
// In World frame.
// Needs to transform it to Eye Frame
// before sending it to the shader(s).

color4 material_ambient(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
color4 material_specular(1.0, 0.8, 0.0, 1.0);
float  material_shininess = 100.0;

//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
	floor_colors[0] = vertex_colors[3]; floor_points[0] = point4(5, 0, 8, 1);
	floor_colors[1] = vertex_colors[3]; floor_points[1] = point4(5, 0, -4, 1);
	floor_colors[2] = vertex_colors[3]; floor_points[2] = point4(-5, 0, -4, 1);

	floor_colors[3] = vertex_colors[3]; floor_points[3] = point4(-5, 0, -4, 1);
	floor_colors[4] = vertex_colors[3]; floor_points[4] = point4(-5, 0, 8, 1);
	floor_colors[5] = vertex_colors[3]; floor_points[5] = point4(5, 0, 8, 1);

	floor_textures[0] = vec2(6, 5);
	floor_textures[1] = vec2(0, 5);
	floor_textures[2] = vec2(0, 0);
	floor_textures[3] = vec2(6, 5);
	floor_textures[4] = vec2(0, 0);
	floor_textures[5] = vec2(6, 0);

	for (int i = 0; i < 6; i++) {
		floor_normals[i] = vec4(0, 1, 0, 1);
	}
}
//----------------------------------------------------------------------------
// Sets up the lighting
void lighting(mat4 mv, float shininess, color4 object_ambient, color4 object_diffuse, color4 object_specular, vec4 light_direction) {
	glUniform1f(glGetUniformLocation(program, "Shininess"), sphere_shininess);
	// global
	glUniform4fv(glGetUniformLocation(program, "GlobalAmbient"), 1, global_ambient * object_ambient);

	// directional
	glUniform4fv(glGetUniformLocation(program, "dLightPosition"), 1, d_light_pos);
	glUniform4fv(glGetUniformLocation(program, "dLightDirection"), 1, d_light_direction);

	glUniform4fv(glGetUniformLocation(program, "dAmbientProduct"), 1, d_light_ambient * object_ambient);
	glUniform4fv(glGetUniformLocation(program, "dDiffuseProduct"), 1, d_light_diffuse * object_diffuse);
	glUniform4fv(glGetUniformLocation(program, "dSpecularProduct"), 1, d_light_specular * object_specular);

	// positional
	glUniform4fv(glGetUniformLocation(program, "pLightPosition"), 1, p_light_pos);
	glUniform4fv(glGetUniformLocation(program, "pLightDirection"), 1, light_direction);

	glUniform4fv(glGetUniformLocation(program, "pAmbientProduct"), 1, p_light_ambient * object_ambient);
	glUniform4fv(glGetUniformLocation(program, "pDiffuseProduct"), 1, p_light_diffuse * object_diffuse);
	glUniform4fv(glGetUniformLocation(program, "pSpecularProduct"), 1, p_light_specular * object_specular);

	glUniform1f(glGetUniformLocation(program, "lightAngle"), cos(lightAngle * (M_PI / 180)));
}

void fog(void) {
	glUniform1i(glGetUniformLocation(program, "fogType"), fogType);
	glUniform1f(glGetUniformLocation(program, "fogStart"), fogStart);
	glUniform1f(glGetUniformLocation(program, "fogEnd"), fogEnd);
	glUniform1f(glGetUniformLocation(program, "fogDensity"), fogDensity);
	glUniform4fv(glGetUniformLocation(program, "fogColor"), 1, fogColor);
}

void texture(void) {
	image_set_up();

	glGenTextures(1, &tex1); // 1D texture

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, tex1);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	glGenTextures(1, &tex2); // 2D texture

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
}

void fireworks(void) {
	for (int i = 0; i < particles; i++) {
		fireworks_colors[i] = vec4((rand() % 256) / 256.0, (rand() % 256) / 256.0, (rand() % 256) / 256.0, 1);
		fireworks_velocity[i] = vec4(2.0 * ((rand() % 256) / 256.0 - 0.5), 1.2 * 2.0 * ((rand() % 256) / 256.0), 2.0 * ((rand() % 256) / 256.0 - 0.5), 1);
	}
}

// Sets up the shadow matrix
void setUpShadowMatrix(mat4& shadowMatrix) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			shadowMatrix[i][j] = 0.0;
		}
	}

	shadowMatrix[0][0] = 1.0; 
	shadowMatrix[1][1] = 1.0; 
	shadowMatrix[2][2] = 1.0;
	shadowMatrix[3][1] = -(1 / light_position[1]);
}

// OpenGL initialization
void init()
{
	image_set_up();

	// Asks for the input files and sets the arrays based on the file
	getTriangles();
	if (fn == "sphere.256") {
		point4 spherePoints[768];
		color4 sphereColors[768];

		for (int i = 0; i < trianglePoints.size(); i++) {
			spherePoints[i] = trianglePoints[i];
			sphereColors[i] = triangleColors[i];
		}
		
		for (int i = 0; i < trianglePoints.size(); i += 3) {
			vec4 flatNormal = normalize(cross(
				spherePoints[i + 1] - spherePoints[i],
				spherePoints[i + 2] - spherePoints[i]
			));

			sphereShadeFlat.push_back(flatNormal);
			sphereShadeFlat.push_back(flatNormal);
			sphereShadeFlat.push_back(flatNormal);

			sphereShadeSmooth.push_back(normalize(spherePoints[i]));
			sphereShadeSmooth.push_back(normalize(spherePoints[i + 1]));
			sphereShadeSmooth.push_back(normalize(spherePoints[i + 2]));
		}
		vec4 sphereNormals[768];

		for (int i = 0; i < 768; i += 1) {
			sphereNormals[i] = sphereShadeFlat[i];
		}

		glGenBuffers(1, &sphere_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors) + sizeof(sphereNormals),
			NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors),
			sphereColors);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints) * 2, sizeof(sphereNormals),
			sphereNormals);

		point4 shadowPoints[768];
		color4 shadowColors[768];

		for (int i = 0; i < trianglePoints.size(); i++) {
			shadowPoints[i] = spherePoints[i];
			shadowColors[i] = color4(0.25, 0.25, 0.25, 0.65);
		}

		glGenBuffers(1, &shadow_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(shadowPoints) + sizeof(shadowColors),
			NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(shadowPoints), shadowPoints);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(shadowPoints), sizeof(shadowColors),
			shadowColors);
	}

	else if (fn == "sphere.1024") {
		point4 spherePoints[3072];
		color4 sphereColors[3072];

		for (int i = 0; i < trianglePoints.size(); i++) {
			spherePoints[i] = trianglePoints[i];
			sphereColors[i] = triangleColors[i];
		}

		for (int i = 0; i < trianglePoints.size(); i += 3) {
			vec4 flatNormal = normalize(cross(
				spherePoints[i + 1] - spherePoints[i],
				spherePoints[i + 2] - spherePoints[i]
			));

			sphereShadeFlat.push_back(flatNormal);
			sphereShadeFlat.push_back(flatNormal);
			sphereShadeFlat.push_back(flatNormal);

			sphereShadeSmooth.push_back(normalize(spherePoints[i]));
			sphereShadeSmooth.push_back(normalize(spherePoints[i + 1]));
			sphereShadeSmooth.push_back(normalize(spherePoints[i + 2]));
		}

		vec4 sphereNormals[3072];
		
		for (int i = 0; i < 3072; i += 1) {
			sphereNormals[i] = sphereShadeFlat[i];
		}

		glGenBuffers(1, &sphere_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors) + sizeof(sphereNormals),
			NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors),
			sphereColors);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints) * 2, sizeof(sphereNormals),
			sphereNormals);

		point4 shadowPoints[3072];
		color4 shadowColors[3072];

		for (int i = 0; i < trianglePoints.size(); i++) {
			shadowPoints[i] = spherePoints[i];
			shadowColors[i] = color4(0.25, 0.25, 0.25, 0.65);
		}

		glGenBuffers(1, &shadow_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(shadowPoints) + sizeof(shadowColors),
			NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(shadowPoints), shadowPoints);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(shadowPoints), sizeof(shadowColors),
			shadowColors);
	}

	floor();
	// Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals) + sizeof(floor_textures),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
		floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) * 2, sizeof(floor_normals),
		floor_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) * 3, sizeof(floor_textures),
		floor_textures);
	
	axis();
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
		axis_colors);

	fireworks();
	glGenBuffers(1, &fireworks_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireworks_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fireworks_colors) * 2, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireworks_colors), fireworks_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworks_colors), sizeof(fireworks_velocity), fireworks_velocity);

	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader53.glsl", "fshader53.glsl");
	fireworksProgram = InitShader("vFireworks.glsl", "fFireworks.glsl");

	glUniform1i(glGetUniformLocation(program, "enableLighting"), true);
	//glUniform1f(glGetUniformLocation(program, "shadingVisible"), shadingVisible);

	glEnable(GL_DEPTH_TEST);
	
	glLineWidth(2.0);
	sphereCenter = posPoints[0];
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	
	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices * 2));

	GLuint vTexture = glGetAttribLocation(program, "vTexture");
	if (enableFloorTexture) {
		glEnableVertexAttribArray(vTexture);
		glVertexAttribPointer(vTexture, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices * 3));
	}

  /* Draw a sequence of geometric objs (triangles) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vNormal);
	if (enableFloorTexture) {
		glDisableVertexAttribArray(vTexture);
	}
}


//Used to draw the axis
void drawObj2(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

  /* Draw a sequence of geometric objs (triangles) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_LINES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

void drawFireworks(GLuint buffer, int particleCount) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vColor = glGetAttribLocation(fireworksProgram, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vVelocity = glGetAttribLocation(fireworksProgram, "vVelocity");
	glEnableVertexAttribArray(vVelocity);
	glVertexAttribPointer(vVelocity, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4) * particleCount));

	glDrawArrays(GL_POINTS, 0, particleCount);

	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vVelocity);
}

//----------------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLuint  model_view;  // model-view matrix uniform shader variable location
	GLuint  projection;  // projection matrix uniform shader variable location
	GLuint normalMatrix;
	glClearColor(0.529, 0.807, 0.92, 0.0);
	glUseProgram(program);

	if (b_pressed == false) {
		glutIdleFunc(NULL);
	}

	model_view = glGetUniformLocation(program, "ModelView");
	projection = glGetUniformLocation(program, "Projection");
	normalMatrix = glGetUniformLocation(program, "Normal_Matrix");
	glUniform1i(glGetUniformLocation(program, "tex2"), 0);

/*---  Set up and pass on Model-View matrix to the shader ---*/
	// eye is a global variable of vec4 set to init_eye and updated by keyboard()
	vec4    at(0.0, 0.0, 0.0, 1.0);

	vec4  up(0.0, 1.0, 0.0, 0.0);
	mat4 shadowMatrix;
	mat4  mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	mat3 nm = NormalMatrix(mv, 1);
	glUniformMatrix3fv(normalMatrix, 1, GL_TRUE, nm);

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4  p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	glDepthMask(GL_FALSE);
	fog();
	if (enableSpotlight) {
		glUniform1i(glGetUniformLocation(program, "enableSpotlight"), true);
		lighting(mv, sphere_shininess, floor_ambient, floor_diffuse, floor_specular, sp_light_direction);
	}
	else {
		glUniform1i(glGetUniformLocation(program, "enableSpotlight"), false);
		lighting(mv, sphere_shininess, floor_ambient, floor_diffuse, floor_specular, p_light_direction);
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (enableLighting) {
		glUniform1i(glGetUniformLocation(program, "enableLighting"), true);
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), true);
		enableFloorTexture = true;
		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), false);
		enableFloorTexture = false;
	}
	else {
		glUniform1i(glGetUniformLocation(program, "enableLighting"), false);
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), true);
		enableFloorTexture = true;
		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), false);
		enableFloorTexture = false;
	}
	glUniform1i(glGetUniformLocation(program, "enableLighting"), false);
	drawObj2(axis_buffer, 6);  // draw the axis
	
	glDepthMask(GL_TRUE);
	rotationMatrix = Rotate(0.08, rotateDirection[0], rotateDirection[1], rotateDirection[2]) * rotationMatrix;

	mv *= Translate(sphereCenter) * rotationMatrix; // Makes the sphere rotate when moving

	if (enableSpotlight) {
		glUniform1i(glGetUniformLocation(program, "enableSpotlight"), true);
		lighting(mv, sphere_shininess, sphere_ambient, sphere_diffuse, sphere_specular, sp_light_direction);
	}
	else {
		glUniform1i(glGetUniformLocation(program, "enableSpotlight"), false);
		lighting(mv, sphere_shininess, sphere_ambient, sphere_diffuse, sphere_specular, p_light_direction);
	}
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	glUniformMatrix3fv(normalMatrix, 1, GL_TRUE, NormalMatrix(mv, 1));
	if (wireFrame == true) {
		glUniform1i(glGetUniformLocation(program, "enableWireFrame"), true);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glUniform1i(glGetUniformLocation(program, "enableWireFrame"), false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (enableLighting && !wireFrame) {
		glUniform1i(glGetUniformLocation(program, "enableLighting"), true);
		if (fn == "sphere.256") {
			drawObj(sphere_buffer, 768); // draw the sphere
		}
		else if (fn == "sphere.1024") {
			drawObj(sphere_buffer, 3072); // draw the sphere
		}
	}
	else {
		glUniform1i(glGetUniformLocation(program, "enableLighting"), false);
		if (fn == "sphere.256") {
			drawObj(sphere_buffer, 768); // draw the sphere
		}
		else if (fn == "sphere.1024") {
			drawObj(sphere_buffer, 3072); // draw the sphere
		}
	}
	
	glUniform1i(glGetUniformLocation(program, "enableLighting"), false);
	glDepthMask(GL_FALSE);
	if (shadowVisible == true) {
		setUpShadowMatrix(shadowMatrix);


		mv = LookAt(eye, at, up) * Translate(light_position) * shadowMatrix * Translate(-light_position) * Translate(sphereCenter[0], sphereCenter[1], sphereCenter[2]) * rotationMatrix;
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
		if (blendShadow) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (fn == "sphere.256") {
				drawObj(shadow_buffer, 768); // draw the shadow
			}
			else if (fn == "sphere.1024") {
				drawObj(shadow_buffer, 3072); // draw the shadow
			}
			glDisable(GL_BLEND);
		}
		else {
			if (fn == "sphere.256") {
				drawObj(shadow_buffer, 768); // draw the shadow
			}
			else if (fn == "sphere.1024") {
				drawObj(shadow_buffer, 3072); // draw the shadow
			}
		}
	}
	glDepthMask(GL_TRUE);

	if (enableFireworks) {
		glUseProgram(fireworksProgram);

		float t = fmod(glutGet(GLUT_ELAPSED_TIME) - fireworksTime, 10000);
		glUniform1f(glGetUniformLocation(fireworksProgram, "t"), t);
		glUniformMatrix4fv(glGetUniformLocation(fireworksProgram, "Projection"), 1, GL_TRUE, Perspective(fovy, aspect, zNear, zFar));
		glUniformMatrix4fv(glGetUniformLocation(fireworksProgram, "ModelView"), 1, GL_TRUE, LookAt(eye, at, up));

		drawFireworks(fireworks_buffer, particles);
	}

	glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle(void)
{
	//From Point A to Point B
	if (targetPos == 1) {
		vec3 direction = posPoints[1] - posPoints[0];
		rotateDirection = cross(vec3(0, 1, 0), direction);
		float velocity = sqrt(pow(direction[0], 2) + pow(direction[2], 2)) / 0.001;
		sphereCenter += vec3(direction[0] / (velocity), 0.0, direction[2] / (velocity));

		if (sphereCenter[0] <= posPoints[1][0] && sphereCenter[2] <= posPoints[1][2]) {
			targetPos = 2;
			sphereCenter = posPoints[1];
		}
	}

	//From Point B to Point C
	else if (targetPos == 2) {
		vec3 direction = posPoints[2] - posPoints[1];
		rotateDirection = cross(vec3(0, 1, 0), direction);
		float velocity = sqrt(pow(direction[0], 2) + pow(direction[2], 2)) / 0.001;
		sphereCenter += vec3(direction[0] / (velocity), 0.0, direction[2] / (velocity));

		if (sphereCenter[0] >= posPoints[2][0] && sphereCenter[2] >= posPoints[2][2]) {
			targetPos = 0;
			sphereCenter = posPoints[2];
		}
	}

	//From Point C to Point A
	else if (targetPos == 0) {
		vec3 direction = posPoints[0] - posPoints[2];
		rotateDirection = cross(vec3(0, 1, 0), direction);

		float velocity = sqrt(pow(direction[0], 2) + pow(direction[2], 2)) / 0.001;
		sphereCenter += vec3(direction[0] / (velocity), 0.0, direction[2] / (velocity));


		if (sphereCenter[0] <= posPoints[0][0] && sphereCenter[2] >= posPoints[0][2]) {
			targetPos = 1;
			sphereCenter = posPoints[0];
		}
	}

	glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;

	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'b': case 'B': // Starts the animation
		b_pressed = true;
		if (animationFlag == 1) {
			glutIdleFunc(idle);
		}

		//else                    glutIdleFunc(NULL);
		break;

	case 'f': case 'F': // Toggle between filled and wireframe floor
		floorFlag = 1 - floorFlag;
		break;

	case ' ':  // reset to initial viewer/eye position
		eye = init_eye;
		break;
	}
	glutPostRedisplay();
}

// Allows the right mouse button to start and stop the animation
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && b_pressed) {
		animationFlag = 1 - animationFlag;
	}

	if (animationFlag == 1) glutIdleFunc(idle);
	else glutIdleFunc(NULL);
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

void menu(int id)
{
	switch (id)
	{
	case 1:
		eye = init_eye;
		break;
	case 2:
		exit(0);
		break;

	case 3:
		shadowVisible = true;
		break;
	case 4:
		shadowVisible = false;
		break;

	case 5:
		enableLighting = true;
		break;
	case 6:
		enableLighting = false;
		break;

	case 7:
		wireFrame = true;
		break;
	case 8:
		wireFrame = false;
		break;


	case 9:
		if (fn == "sphere.256") {
			vec4 sphereNormals[768];

			for (int i = 0; i < 768; i += 1) {
				sphereNormals[i] = sphereShadeFlat[i];
			}
			glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphereNormals) * 2, sizeof(sphereNormals),
				sphereNormals);
		}
		else if (fn == "sphere.1024") {
			vec4 sphereNormals[3072];

			for (int i = 0; i < 3072; i += 1) {
				sphereNormals[i] = sphereShadeFlat[i];
			}
			glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphereNormals) * 2, sizeof(sphereNormals),
				sphereNormals);
		}
		break;

	case 10:
		if (fn == "sphere.256") {
			vec4 sphereNormals[768];

			for (int i = 0; i < 768; i += 1) {
				sphereNormals[i] = sphereShadeSmooth[i];
			}
			glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphereNormals) * 2, sizeof(sphereNormals),
				sphereNormals);
		}
		else if (fn == "sphere.1024") {
			vec4 sphereNormals[3072];

			for (int i = 0; i < 3072; i += 1) {
				sphereNormals[i] = sphereShadeSmooth[i];
			}
			glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphereNormals) * 2, sizeof(sphereNormals),
				sphereNormals);
		}
		break;

	case 11:
		enableSpotlight = true;
		glUniform1f(glGetUniformLocation(program, "enableSpotlight"), true);
		break;

	case 12:
		enableSpotlight = false;
		glUniform1f(glGetUniformLocation(program, "enableSpotlight"), false);
		break;

	case 13:
		fogType = 0;
		glUniform1i(glGetUniformLocation(program, "fogType"), 0);
		break;

	case 14:
		fogType = 1;
		glUniform1i(glGetUniformLocation(program, "fogType"), 1);
		break;

	case 15:
		fogType = 2;
		glUniform1i(glGetUniformLocation(program, "fogType"), 2);
		break;

	case 16:
		fogType = 3;
		glUniform1i(glGetUniformLocation(program, "fogType"), 3);
		break;

	case 17:
		blendShadow = true;
		break;

	case 18:
		blendShadow = false;
		break;

	case 19:
		if (enableFireworks == false) {
			enableFireworks = true;
			fireworksTime = glutGet(GLUT_ELAPSED_TIME);
		}
		break;

	case 20:
		enableFireworks = false;
		break;

	case 21:
		enableFloorTexture = true;
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), true);
		break;

	case 22:
		enableFloorTexture = false;
		glUniform1i(glGetUniformLocation(program, "enableFloorTexture"), false);
		break;
	}
}


//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowSize(512, 512);
	glutCreateWindow("CS 4533 Assignment 4");

#ifdef __APPLE__ // on macOS
	// Core profile requires to create a Vertex Array Object (VAO).
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
	/* Call glewInit() and error checking */
	int err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}
#endif

	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	GLuint shadowMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Yes", 3);
	glutAddMenuEntry("No", 4);

	GLuint lightingMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Yes", 5);
	glutAddMenuEntry("No", 6);

	GLuint wireFrameMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Show wire frame", 7);
	glutAddMenuEntry("Show filled shape", 8);

	GLuint shadingMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Flat shading", 9);
	glutAddMenuEntry("Smooth shading", 10);

	GLuint spotlightMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Spotlight", 11);
	glutAddMenuEntry("Point Source", 12);

	GLuint fogMenu = glutCreateMenu(menu);
	glutAddMenuEntry("No fog", 13);
	glutAddMenuEntry("Linear", 14);
	glutAddMenuEntry("Exponential", 15);
	glutAddMenuEntry("Exponential Squared", 16);

	GLuint blendShadowMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Yes", 17);
	glutAddMenuEntry("No", 18);

	GLuint fireworksMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Yes", 19);
	glutAddMenuEntry("No", 20);

	GLuint floorTextureMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Yes", 21);
	glutAddMenuEntry("No", 22);

	glutCreateMenu(menu);
	glutAddMenuEntry("Default View Point", 1);
	glutAddMenuEntry("Quit", 2);
	glutAddSubMenu("Shadow", shadowMenu);
	glutAddSubMenu("Lighting", lightingMenu);
	glutAddSubMenu("Wire Frame Sphere", wireFrameMenu);
	glutAddSubMenu("Shading", shadingMenu);
	glutAddSubMenu("Spotlight", spotlightMenu);
	glutAddSubMenu("Fog", fogMenu);
	glutAddSubMenu("Blend Shadow", blendShadowMenu);
	glutAddSubMenu("Fireworks", fireworksMenu);
	glutAddSubMenu("Floor Texture", floorTextureMenu);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	init();
	glutMainLoop();

	return 0;
}

void getTriangles() {
	ifstream file;

	cout << "Enter file name (type either 'sphere.256' or 'sphere.1024'): " << endl;
	cin >> fn;
	file.open(fn);
	if (!file) {
		cout << "Unable to open file";
		exit(1);
	}

	file >> n;
	for (int i = 0; i < n; i++) {
		int m;
		float x;
		float y;
		float z;

		file >> m;

		for (int j = 0; j < m; j++) {
			file >> x;
			file >> y;
			file >> z;

			trianglePoints.push_back(point4(x, y, z, 1));
			triangleColors.push_back(color4(1.0, 0.84, 0, 1));
		}
	}

	file.close();
}

void axis(void) {
	axis_colors[0] = vertex_colors[1];
	axis_points[0] = point4(0, 0, 0, 1);
	axis_colors[1] = vertex_colors[1];
	axis_points[1] = point4(10, 0, 0, 1);

	axis_colors[2] = vertex_colors[4];
	axis_points[2] = point4(0, 0, 0, 1);
	axis_colors[3] = vertex_colors[4];
	axis_points[3] = point4(0, 10, 0, 1);

	axis_colors[4] = vertex_colors[5];
	axis_points[4] = point4(0, 0, 0, 1);
	axis_colors[5] = vertex_colors[5];
	axis_points[5] = point4(0, 0, 10, 1);
}
