#define GLM_ENABLE_EXPERIMENTAL

#include "LoadPNG\lodepng.h"
#include "LoadPNG\lodepng_util.h"
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "CY\cyTriMesh.h"
#include "CY\cyGL.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "GL\glm\glm.hpp"
#include "GL\glm\gtc\matrix_transform.hpp"
#include "GL\glm\gtx\quaternion.hpp"
#include "GL\glm\gtc\quaternion.hpp"




float R, G, B;
float r_, g_, b_;

GLuint vbo_ID;

unsigned int VBO , VAO, IBO , TBO;
unsigned int TU_Diffuse, TU_Specular;
unsigned int NBO;
int shaderProgram;
cy::TriMesh meshData;

glm::mat4 projectionMatrix, viewMatrix, modelMatrix;
bool isMouseLeftButtonDown, isMouseRightButtonDown, isControlDown;
int oldMouseX, oldMouseY;
glm::vec3 lookAtCenter;
glm::vec3 cameraCenter;

glm::vec3 lightPos;

void TeapotDraw()
{
	modelMatrix = glm::rotate(modelMatrix, 0.001f, glm::vec3(0, 0, 1));

	GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");
	GLuint MatrixModel = glGetUniformLocation(shaderProgram, "MatrixModel");
	GLuint MatrixProjection = glGetUniformLocation(shaderProgram, "MatrixProjection");
	GLuint MatrixView = glGetUniformLocation(shaderProgram, "MatrixView");
	GLuint LightID = glGetUniformLocation(shaderProgram, "LightPosition_WorldPosition");


	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MatrixModel, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(MatrixProjection, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(MatrixView, 1, GL_FALSE, &viewMatrix[0][0]);

	
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);


	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cyPoint3f), 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cyPoint3f), 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(cyPoint2f), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TU_Diffuse);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TU_Specular);


	glDrawArrays(GL_TRIANGLES, 0, meshData.NF()*3);
	//glDrawElements(GL_TRIANGLES, meshData.NF() * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*glEnable(GL_CULL_FACE);*/

	//// Enable depth test
	glEnable(GL_DEPTH_TEST);
	//// Accept fragment if it closer to the camera than the former one
	/*glDepthFunc(GL_LESS);*/

	TeapotDraw();
	glutSwapBuffers();
	glFlush();
	
}

void NonAnimateIdleFunc()
{
	
	glClearColor(R / 255.0f, G / 255.0f, B / 255.0f, 1);
	glutPostRedisplay();
	std::cout << "\b";
	std::cout <<isControlDown;
}

void IdleFunc()
{
	if (R <= 0.0f)
		r_ = 0.23f;
	else if (R >= 255.0f)
		r_ = -0.23f;

	if (G <= 0.0f)
		g_ = 0.17f;
	else if (G >= 255.0f)
		g_ = -0.17f;

	if (B <= 0.0f)
		b_ = 0.1f;
	else if (B >= 255.0f)
		b_ = -0.1f;


	R += r_;
	G += g_;
	B += b_;


	glClearColor(R / 255.0f, G / 255.0f, B / 255.0f, 1);
	glutPostRedisplay();
}

void KeyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{

	case 'a':
		glutIdleFunc(NonAnimateIdleFunc);
		break;

	case 's':
		glutIdleFunc(IdleFunc);
		break;



	case 27: //exit(0);
		glutLeaveMainLoop();
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
		break;

	}
}

void IntializeParameters() {
	R = 0.0f;
	G = 0.0f;
	B = 0.0f;

	lightPos = glm::vec3(-50.0f, 50, -20.0f);

	oldMouseX = 0;
	oldMouseY = 0;
	cameraCenter = glm::vec3(0, 0, -45.0f);
	meshData.LoadFromFileObj("Assets/teapot.obj");
	meshData.ComputeNormals();
	
	cy::Point3f * vertexBuffer;
	cy::Point3f * vertexNormalBuffer;
	cy::Point2f * vertexTextureBuffer;



	size_t verxBufferSize;
	vertexBuffer = new cy::Point3f[meshData.NF()*3];
	vertexNormalBuffer = new cy::Point3f[meshData.NF()*3];
	vertexTextureBuffer = new cy::Point2f[meshData.NF() * 3];
	
	{
		for (unsigned int index = 0; index < meshData.NF(); index++)
		{
			cy::TriFace vertexIndex = meshData.F(index);
			vertexBuffer[index * 3] = meshData.V(vertexIndex.v[0]);
			vertexBuffer[index*3 + 1]= meshData.V(vertexIndex.v[1]);
			vertexBuffer[index * 3 + 2] = meshData.V(vertexIndex.v[2]);

			cy::TriFace vertexNormalIndex = meshData.FN(index);
			vertexNormalBuffer[index * 3] = meshData.VN(vertexNormalIndex.v[0]);
			vertexNormalBuffer[index * 3 + 1] = meshData.VN(vertexNormalIndex.v[1]);
			vertexNormalBuffer[index * 3 + 2] = meshData.VN(vertexNormalIndex.v[2]);

			cy::TriFace vertexTexturePoint = meshData.FT(index);
			vertexTextureBuffer[index * 3] = cy::Point2f(meshData.VT(vertexTexturePoint.v[0]).x, meshData.VT(vertexTexturePoint.v[0]).y);
			vertexTextureBuffer[index * 3+1] = cy::Point2f(meshData.VT(vertexTexturePoint.v[1]).x, meshData.VT(vertexTexturePoint.v[1]).y);
			vertexTextureBuffer[index * 3+2] = cy::Point2f(meshData.VT(vertexTexturePoint.v[2]).x, meshData.VT(vertexTexturePoint.v[1]).y);
		}
		verxBufferSize = meshData.NF() * 3;
	}
	

	//unsigned char * diffuseBufferString;
	std::vector<unsigned char> diffuseBuffer;
	std::vector<unsigned char> specularBuffer;
	unsigned dw, dh , sw , sh;
	
		const char * errorMessage;
		
			std::string diffusename("Assets/");
			diffusename +=  meshData.M(0).map_Kd.data;
			auto errormsg = lodepng::decode(diffuseBuffer, dw, dh, diffusename, LodePNGColorType::LCT_RGB);
			
			if (errormsg != 0)
			{
				errorMessage = lodepng_error_text(errormsg);
				std::cout << "Loading PNG Failed with " << errorMessage;
			}
			else
			{
				std::cout << "Loading PNG " << diffusename << std::endl;
			}
		
		
			std::string specularName("Assets/");
			specularName += meshData.M(0).map_Ks.data;
			auto errormsg_s = lodepng::decode(specularBuffer, sw, sh, specularName, LodePNGColorType::LCT_RGB);
			//const char * errorMessage;
			if (errormsg_s != 0)
			{
				errorMessage = lodepng_error_text(errormsg_s);
				std::cout << "Loading PNG Failed with " << errorMessage;
			}
			else
			{
				std::cout << "Loading PNG " << specularName << std::endl;

			}
		
	

	glGenVertexArrays(1, &VAO);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * verxBufferSize, vertexBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cyPoint3f), 0);

	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * verxBufferSize, vertexNormalBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cyPoint3f), 0);


	glGenBuffers(1, &TBO);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glEnableVertexAttribArray(2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint2f) * verxBufferSize, vertexTextureBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(cyPoint2f), 0);

	glGenTextures(1, &TU_Diffuse);
	glBindTexture(GL_TEXTURE_2D, TU_Diffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dw, dh, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuseBuffer.data());
	glGenerateMipmap(GL_TEXTURE_2D);


	glGenTextures(1, &TU_Specular);
	glBindTexture(GL_TEXTURE_2D, TU_Specular);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sw, sh, 0, GL_RGB, GL_UNSIGNED_BYTE, specularBuffer.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindVertexArray(VAO);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDisable(GL_CULL_FACE);
	
}

void ReadShaderCode()
{
	int success;
	char infoLog[512];

	unsigned int vertexShader,fragmentShader;
	{
		std::ifstream file("Shaders/VertexShader.txt");
		std::stringstream buffer;

		buffer << file.rdbuf();
		const std::string tmp = buffer.str();
		const char* vertexShaderSource = tmp.c_str();
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
	{
		std::ifstream file("Shaders/FragmentShader.txt");
		std::stringstream buffer;

		buffer << file.rdbuf();
		const std::string tmp = buffer.str();
		const char* fragmentShaderSource = tmp.c_str();

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
}

void SetUpCamera()
{
	projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	viewMatrix = glm::lookAt(
		cameraCenter, // Camera  in World Space
		glm::vec3(0, 0, 0), // and looks at 
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::rotate(modelMatrix, -90.0f, glm::vec3(1, 0, 0));
}

void SpecialKeysFunc(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_F6:
		std::cout << "\nLOG: COMPILING SHADER CODE";
		ReadShaderCode();
		break;
		
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		isControlDown = true;
		break;

	default:
		break;
	}
}

void SpecialKeysUpFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		isControlDown = false;
		break;
	}
}

void MouseInputHandler(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			isMouseLeftButtonDown = true;
		else
			isMouseLeftButtonDown = false;
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
			isMouseRightButtonDown = true;
		else
			isMouseRightButtonDown = false;
		break;

	default:
		break;
	}
}

void MouseMotionHandler(int x, int y)
{

	if (!isControlDown)
	{

	if (isMouseLeftButtonDown )
	{
		if ((x - oldMouseX) > 0)
		{
			lookAtCenter.x = lookAtCenter.x - 0.1f;
		}
		else if ((x - oldMouseX) < 0)
		{
			lookAtCenter.x = lookAtCenter.x + 0.1f;
		}
		
		if ((y - oldMouseY) > 0)
		{
			lookAtCenter.y = lookAtCenter.y - 0.1f;
		}
		else if ((y - oldMouseY) < 0)
		{
			lookAtCenter.y = lookAtCenter.y + 0.1f;
		}
	}

	if (isMouseRightButtonDown)
	{
		if ((y - oldMouseY) > 0)
		{
			cameraCenter.z = cameraCenter.z + 0.25f;
		}
		else if ((y - oldMouseY) < 0)
		{
			cameraCenter.z = cameraCenter.z - 0.25f;
		}
	}

	oldMouseX = x;
	oldMouseY = y;

	viewMatrix = viewMatrix = glm::lookAt(
		cameraCenter, // Camera  in World Space
		lookAtCenter, // and looks at 
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	}
	else
	{
		if (isMouseLeftButtonDown)
		{
			if ((x - oldMouseX) > 0)
			{
				glm::vec3 EulerAngles = glm::vec3(0.0f, 0.01f, 0);

				glm::quat myQuat = glm::quat(EulerAngles);

				lightPos = lightPos * myQuat;
			}
			else if ((x - oldMouseX) < 0)
			{
				glm::vec3 EulerAngles = glm::vec3(0.0f, -0.01f, 0);

				glm::quat myQuat = glm::quat(EulerAngles);

				lightPos = lightPos * myQuat;
			}
			
		}
	}
}

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("InteractiveGraphicsProgram");

	
	
	glewInit();
	IntializeParameters();
	ReadShaderCode();
	SetUpCamera();
	glutKeyboardFunc(KeyboardFunc);
	glutIdleFunc(NonAnimateIdleFunc);
	glutDisplayFunc(Display);
	glutSpecialFunc(SpecialKeysFunc);
	glutSpecialUpFunc(SpecialKeysUpFunc);
	glutMouseFunc(MouseInputHandler);
	glutMotionFunc(MouseMotionHandler);
	glutMainLoop();
	return 0;

}