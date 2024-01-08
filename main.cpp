#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#define EPSILON 0.000001

using namespace std;

GLuint gProgram[3];
int gWidth, gHeight;

GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct Model {
    vector<Vertex> gVertices;
    vector<Texture> gTextures;
    vector<Normal> gNormals;
    vector<Face> gFaces;
    GLuint vao; // Vertex Array Object
    GLuint vbo; // Vertex Buffer Object
    GLuint ebo; // Element Buffer Object (for faces/indices)

    Model() : vao(0), vbo(0), ebo(0) {}
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;


Model gBunnyModel, gQuadModel, gCubeModel, gCubeModel2, gCubeModel3;

vector<Model> gModels;

bool isMovingLeft = false;
bool isMovingRight = false;


bool ParseObj(const string& fileName, Model& model )
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						model.gTextures.push_back(Texture(c1, c2));
						// model.gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						model.gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						model.gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					model.gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	assert(model.gVertices.size() == model.gNormals.size());

	return true;
}


bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("vert.glsl");
	GLuint fs1 = createFS("frag.glsl");


	GLuint vs2 = createVS("vert_checkerboard.glsl");
	GLuint fs2 = createFS("frag_checkerboard.glsl");

	GLuint vs3 = createVS("vertex_cube.glsl");
	GLuint fs3 = createFS("frag_cube.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs

	glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[2]);
	glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 3; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}


void initVBO(Model& model)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    cout << "vao = " << vao << endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    GLuint gVertexAttribBuffer, gIndexBuffer;
    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    int gVertexDataSizeInBytes = model.gVertices.size() * 3 * sizeof(GLfloat);
    int gNormalDataSizeInBytes = model.gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = model.gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat[model.gVertices.size() * 3];
    GLfloat* normalData = new GLfloat[model.gNormals.size() * 3];
    GLuint* indexData = new GLuint[model.gFaces.size() * 3];

    for (int i = 0; i < model.gVertices.size(); ++i)
    {
        vertexData[3 * i] = model.gVertices[i].x;
        vertexData[3 * i + 1] = model.gVertices[i].y;
        vertexData[3 * i + 2] = model.gVertices[i].z;
    }

    for (int i = 0; i < model.gNormals.size(); ++i)
    {
        normalData[3 * i] = model.gNormals[i].x;
        normalData[3 * i + 1] = model.gNormals[i].y;
        normalData[3 * i + 2] = model.gNormals[i].z;
    }

    for (int i = 0; i < model.gFaces.size(); ++i)
    {
        indexData[3 * i] = model.gFaces[i].vIndex[0];
        indexData[3 * i + 1] = model.gFaces[i].vIndex[1];
        indexData[3 * i + 2] = model.gFaces[i].vIndex[2];
    }

    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying to GPU memory; can free now from CPU memory
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

    // Store the VAO and VBO identifiers in the model object
    model.vao = vao;
    model.vbo = gVertexAttribBuffer;
    // Optionally, store the index buffer if you need to access it later
    model.ebo = gIndexBuffer;
}


void init(){
	ParseObj("bunny.obj", gBunnyModel);
	ParseObj("quad.obj", gQuadModel);
	ParseObj("cube.obj", gCubeModel);
	ParseObj("cube.obj", gCubeModel2);
	ParseObj("cube.obj", gCubeModel3);

	glEnable(GL_DEPTH_TEST);
	initShaders();
	initVBO(gBunnyModel);
	initVBO(gQuadModel);
	initVBO(gCubeModel);
	initVBO(gCubeModel2);
	initVBO(gCubeModel3);
	// gModels.push_back(gQuadModel);
	// gModels.push_back(gBunnyModel);

	// initVBO(gCubeModel);
}


void drawModel(const Model& model)
{
    // Bind the VAO of the model
    glBindVertexArray(model.vao);

    // Draw the model
    glDrawElements(GL_TRIANGLES, model.gFaces.size() * 3, GL_UNSIGNED_INT, 0);

    // Unbind the VAO
    glBindVertexArray(0);
}



// Global variable to keep track of the bunny's X position
float bunnyPosX = 0.0f;
float bunnyPosY = 0.0f;
float bunnyPosZ = -2.0f;
float initialSpeed = 1.0f;
float forwardSpeed= 0.5f;


float computeHopHeight(float forwardSpeed, float time) {
	float startHeight = -1.0f; // Starting height of the hop
    float baseHopHeight = 1.0; // Base height of the hop
    float speedFactor = forwardSpeed / initialSpeed; // Adjust based on your speed scale
    float hopHeight = baseHopHeight * speedFactor;
    float hopY = startHeight + sin(time) * hopHeight; // Sinusoidal motion for hopping

    return hopY;
}
void updateHorizontalPosition(int key, float forwardSpeed) {
    float baseMoveSpeed = 0.5; // Base speed for horizontal movement
    float speedFactor = forwardSpeed / initialSpeed; // Adjust based on your speed scale
    float moveSpeed = baseMoveSpeed * speedFactor;

    if (key == GLFW_KEY_A) {
        bunnyPosX -= moveSpeed; // Move left
    } else if (key == GLFW_KEY_D) {
        bunnyPosX += moveSpeed; // Move right
    }
}

// Function to compute the BUNNY's model matrix
glm::mat4 computeBunnyModelMatrix() {


	// Translate the model
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(bunnyPosX, bunnyPosY, bunnyPosZ));

	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.25, 0.25, 0.25));

	// Rotate the model 90 degrees clockwise around the Y-axis
	glm::mat4 matR = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));

	glm::mat4 matRz = glm::rotate(glm::mat4(1.0), glm::radians(+10.0f), glm::vec3(1.0, 0.0, 0.0));
	

    return matT * matS * matR * matRz;
	
}

float baseCubePosZ = -30.0f;

float initalCubePos1X = 0.0f; float initalCubePos1Y = -1.0f; float initalCubePos1Z = baseCubePosZ;
float initalCubePos2X = -3.0f; float initalCubePos2Y = -1.0f; float initalCubePos2Z = baseCubePosZ;
float initalCubePos3X = 3.0f; float initalCubePos3Y = -1.0f; float initalCubePos3Z = baseCubePosZ;

glm::mat4 computeCube1ModelMatrix(){
	
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(initalCubePos1X, initalCubePos1Y, initalCubePos1Z));
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.75, 2.0, 1.0));
	glm::mat4 matR = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));

	return matT * matS * matR;
}

glm::mat4 computeCube2ModelMatrix(){

	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(initalCubePos2X, initalCubePos2Y, initalCubePos2Z));
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.75, 2.0, 1.0));
	glm::mat4 matR = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));

	return matT * matS * matR;
}

glm::mat4 computeCube3ModelMatrix(){

	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(initalCubePos3X, initalCubePos3Y, initalCubePos3Z));
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.75, 2.0, 1.0));
	glm::mat4 matR = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));

	return matT * matS * matR;
}

bool kill = false;


// Set the position where you want the start of the path
float quadPosX = 0.0f; // Centered on X
float quadPosY = -1.5f; // At ground level
float quadPosZ = -8.0f; // Adjust as needed
float quadPosZ_2 = -8.0f;

// Scale the path to be long and wide but flat
float quadScaleX = 5.0f; // Width of the path
float quadScaleY = 1000.0f; // make it look path
float quadScaleZ = 80.0f; // Length of the path

glm::mat4 computeQuadModelMatrix() {


	// Translate the model to its position
    glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(quadPosX, quadPosY, quadPosZ));

    // Scale the quad to make it long and flat
    glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(quadScaleX, quadScaleY, quadScaleZ));

    // Rotate the model around the X-axis to lay it flat on the ground
    glm::mat4 matRx = glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));

    
    // Combine the transformations, noting the order of operations: first scale, then rotate, then translate
    return   matT * matS  * matRx  ;

}

float movePosZ = 0.0f;

// Set your desired values for offset and scale
float offsetValue = 50.0f; 
float offsetZ = 50.0f;
float scaleValue = 0.5f;


int whichCube0  = 0;
int whichCube1  = 1;
int whichCube2  = 2;
int check = 0;


void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	// make the bunny jump up and down
	static float jumpTime = 0;
	float jumpHeight = 0.5;
	float jumpY = sin(jumpTime) * jumpHeight;
	float forwardSpeed = 0.1;

	bunnyPosY = computeHopHeight(forwardSpeed, jumpTime);
	if (isMovingLeft) {
        // Logic to move left
		if (bunnyPosX > -2.75f) {
			bunnyPosX -= forwardSpeed;  // Adjust this line according to your position update logic
		}

        // bunnyPosX -= forwardSpeed;  // Adjust this line according to your position update logic
    }
    if (isMovingRight) {
		if (bunnyPosX < 2.90f) {
			// Logic to move right
        	bunnyPosX += forwardSpeed;  // Adjust this line according to your position update logic
       	
		}
    }
	 // Update the offset for the path animation
    offsetZ -= 0.1f;  // Adjust this value for speed of animation

// --------------------------------------------------------------------

	// put bunny
	glUseProgram(gProgram[0]); // Replace with actual program index for bunny
	glUniformMatrix4fv(projectionMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glm::mat4 bunnyModelMatrix = computeBunnyModelMatrix(); // Function to compute bunny's model matrix


    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
	// std::cout << "bunnyPos y = " << bunnyPosY << std::endl;
	// std::cout << "bunnyPos Z = " << bunnyPosZ << std::endl;
	glUniform3fv(eyePosLoc[0], 1, glm::value_ptr(eyePos));
    drawModel(gBunnyModel);


//--------------------------------------------------------------------------
	initalCubePos1Z += 0.1f;
	// which checkpoint will be the yellow
	if(initalCubePos1Z  >  bunnyPosZ + EPSILON){
		
		check = std::rand() % 3;
		if(check == 0){
			whichCube0  = 0;
			whichCube1  = 1;
			whichCube2  = 2;

		}
		else if(check == 1){
			whichCube1  = 0;
			whichCube0  = 1;
			whichCube2  = 2;

		}
		else if(check == 2){
			whichCube2  = 0;
			whichCube1  = 1;
			whichCube0  = 2;

		}
	}

	//put the cube
	glUseProgram(gProgram[2]); // Replace with actual program index for cube

	GLuint whichCubeLoc0 = glGetUniformLocation(gProgram[2], "whichCube");

	std::cout << "whichCube0 = " << whichCube0 << std::endl;

	glUniform1i(whichCubeLoc0, whichCube0);

	

	if(initalCubePos1Z  >  bunnyPosZ + EPSILON){
		initalCubePos1Z = baseCubePosZ;
	}
	// bunny hits to the cube
	if(bunnyPosX > initalCubePos1X - 0.5f && bunnyPosX < initalCubePos1X + 0.5f && bunnyPosZ > initalCubePos1Z - 1.0f && bunnyPosZ < initalCubePos1Z + 1.0f){
		if(whichCube0==0)
		{
			// do rotation

		}
		else{
			// die and reset
			kill = true;
		}
	}

	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));

    glm::mat4 cubeModelMatrix = computeCube1ModelMatrix(); // Function to compute bunny's model matrix
    glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(cubeModelMatrix));

	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));
    drawModel(gCubeModel);

//--------------------------------------------------------------------------
	
	//put the cube2
	
	//put the cube
	glUseProgram(gProgram[2]); // Replace with actual program index for cube

	GLuint whichCubeLoc1 = glGetUniformLocation(gProgram[2], "whichCube");
	std::cout << "whichCube1 = " << whichCube1 << std::endl;

	glUniform1i(whichCubeLoc1, whichCube1);

	initalCubePos2Z += 0.1f;
	if(initalCubePos2Z  >  bunnyPosZ + EPSILON){
		initalCubePos2Z = baseCubePosZ;
	}

	// bunny hits to the cube
	if(bunnyPosX > initalCubePos2X - 0.5f && bunnyPosX < initalCubePos2X + 0.5f && bunnyPosZ > initalCubePos2Z - 1.0f && bunnyPosZ < initalCubePos2Z + 1.0f){
		if(whichCube2==0)
		{
			// do rotation

		}
		else{
			// die and reset
			kill = true;
		}
	}
	

	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    
	glm::mat4 cubeModelMatrix2 = computeCube2ModelMatrix(); // Function to compute cube's model matrix
    glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(cubeModelMatrix2));

	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));
    drawModel(gCubeModel2);
	
// --------------------------------------------------------------------------
	//put the cube3
	//put the cube
	
	glUseProgram(gProgram[2]); // Replace with actual program index for cube
	GLuint whichCubeLoc2 = glGetUniformLocation(gProgram[2], "whichCube");

	std::cout << "whichCube2 = " << whichCube2 << std::endl;

	glUniform1i(whichCubeLoc2, whichCube2);

	initalCubePos3Z += 0.1f;
	if(initalCubePos3Z  >  bunnyPosZ + EPSILON){
		initalCubePos3Z = baseCubePosZ;
	}

	// bunny hits to the cube
	if(bunnyPosX > initalCubePos3X - 0.5f && bunnyPosX < initalCubePos3X + 0.5f && bunnyPosZ > initalCubePos3Z - 1.0f && bunnyPosZ < initalCubePos3Z + 1.0f){
		if(whichCube2==0)
		{
			// do rotation

		}
		else{
			// die and reset
			kill = true;
		}
	}

	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glm::mat4 cubeModelMatrix3 = computeCube3ModelMatrix(); // Function to compute cube's model matrix

	glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(cubeModelMatrix3));

	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));
	drawModel(gCubeModel3);

//--------------------------------------------------------------------------

	// Quad Transformations
	glUseProgram(gProgram[1]);
	// Assuming gProgram[1] is your shader program
	GLuint offsetLoc = glGetUniformLocation(gProgram[1], "offset");
	GLuint offsetZLoc = glGetUniformLocation(gProgram[1], "offsetZ");
	GLuint scaleLoc = glGetUniformLocation(gProgram[1], "scale");
	
	// set the values
	glUniform1f(offsetLoc, offsetValue);
	glUniform1f(offsetZLoc, offsetZ);
	glUniform1f(scaleLoc, scaleValue);

	glUniformMatrix4fv(projectionMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glm::mat4 quadModelMatrix = computeQuadModelMatrix(); // Function to compute quad's model matrix
    glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(quadModelMatrix));
	std::cout << "quadPosZ = " << quadPosZ << std::endl;
	std::cout << "quadPosY = " << quadPosY << std::endl;
	
	glUniform3fv(eyePosLoc[1], 1, glm::value_ptr(eyePos));
	
    drawModel(gQuadModel);

// -----------------------------------------------------------------


	jumpTime += 0.25;
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1)

	// Use orthographic projection
	

	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)) ;
	
	

}
void resetGame() {
    // Reset the speed to its initial value
    // speed = initialSpeed; // Replace 'speed' and 'initialSpeed' with your actual variable names

    // Reset any other game state variables as needed
    // For example, resetting the bunny's position
    bunnyPosX = 0.0f;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A) {
        isMovingLeft = (action != GLFW_RELEASE);
    } else if (key == GLFW_KEY_D) {
        isMovingRight = (action != GLFW_RELEASE);
    }
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        resetGame();
    }
	else if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		activeProgramIndex = 1;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		glShadeModel(GL_FLAT);
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		glShadeModel(GL_SMOOTH);
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		if(!kill)
		{
			display();
			
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this if on MacOS

	int width = 1000, height = 800;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER)); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, " - "); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION)); // Use strcpy_s on Windows, strcpy on Linux
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}