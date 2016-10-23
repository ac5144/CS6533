#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glsupport.h"
#include "geometrymaker.h"
#include "cvec.h"
#include "matrix4.h"
#include "quat.h"
#include <vector>

//Structs
struct VertexPN {
	Cvec3f p, n;
	VertexPN() {};
	VertexPN(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}

	VertexPN& operator = (const GenericVertex& v) {
		p = v.pos;
		n = v.normal;
		return *this;
	}
};

struct Transform {
	Cvec3 translation;
	Quat rotation;
	Cvec3 scale;

	Transform() : scale(1.0, 1.0, 1.0) {}

	Matrix4 createMatrix() {
		Matrix4 transformMatrix;
		transformMatrix = transformMatrix.makeTranslation(translation) * quatToMatrix(rotation) * transformMatrix.makeScale(scale);
		return transformMatrix;
	}
};

struct Geometry {
	GLuint vertexVBO;
	GLuint indexBO;
	int numIndeces;

	void Draw(GLuint positionAttribute, GLuint normalAttribute) {

		//BIND BUFFER OBJECTS AND DRAW
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
		glEnableVertexAttribArray(positionAttribute);

		glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
		glEnableVertexAttribArray(normalAttribute);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
		glDrawElements(GL_TRIANGLES, numIndeces, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(positionAttribute);
		glDisableVertexAttribArray(normalAttribute);
	}
};

struct Entity {
	Transform transform;
	Geometry geometry;
	Entity *parent;

	Matrix4 getModelViewMatrix() {
		Matrix4 modelViewMatrix = transform.createMatrix();
		if (parent != nullptr) {
			return parent->getModelViewMatrix() * modelViewMatrix;
		}
		return modelViewMatrix;
	}

	void Draw(Matrix4 &eyeInverse, GLuint positionAttribute, GLuint normalAttribute, GLuint modelViewMatrixLoc, GLuint normalMatrixLoc) {

		//GET PARENT DATA
		Matrix4 parModel;
		if (parent != nullptr) {
			parModel = parent->getModelViewMatrix();
		}

		//CREATE MODELVIEW MATRIX

		Matrix4 modelMatrix = parModel * getModelViewMatrix();
		Matrix4 modelViewMatrix = eyeInverse * modelMatrix;
		//CREATE NORMAL MATRIX

		Matrix4 normMatrix = normalMatrix(modelViewMatrix);

		//SET MODELVIEW AND NORMAL MATRICES TO UNIFORMS LOCATIONS

		GLfloat glmatrix[16];
		modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
		glUniformMatrix4fv(modelViewMatrixLoc, 1, false, glmatrix);

		GLfloat glmatrixNormal[16];
		normMatrix.writeToColumnMajorMatrix(glmatrixNormal);
		glUniformMatrix4fv(normalMatrixLoc, 1, false, glmatrixNormal);

		geometry.Draw(positionAttribute, normalAttribute);
	}
};

//GLOBALS
GLuint program;
GLuint color;
GLuint positionAttribute, normalAttribute;
GLuint modelViewMatrixLoc, projectionMatrixLoc, normalMatrixLoc;

Entity cube1, cube2, plane1;

//OTHER FUNCS
void initLocations() {
	positionAttribute = glGetAttribLocation(program, "position");
	normalAttribute = glGetUniformLocation(program, "normalMatrix");
	color = glGetUniformLocation(program, "uColor");
	modelViewMatrixLoc = glGetUniformLocation(program, "modelViewMatrix");
	projectionMatrixLoc = glGetUniformLocation(program, "projectionMatrix");
	normalMatrixLoc = glGetUniformLocation(program, "normalMatrix");
}

//THE JUICY STUFF
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float timeElapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float angle = timeElapsed * 15.0;

	//EYE MATRIX
	Matrix4 eyeMatrix;
	eyeMatrix = eyeMatrix.makeTranslation(Cvec3(0.0, 1.25, 3.0));
	eyeMatrix = eyeMatrix * eyeMatrix.makeXRotation(-15.0);

	//PLANE TRANSFORM AND DRAW
	//PLANE ROTATES ALONG Y AXIS
	glUniform3f(color, 0.0, 1.0, 0.0);
	Quat r1 = Quat::makeYRotation(angle);
	plane1.transform.rotation = r1;
	plane1.Draw(inv(eyeMatrix), positionAttribute, normalAttribute, modelViewMatrixLoc, normalMatrixLoc);

	//CUBE1 TRANSFORM AND DRAW
	//CUBE1 IS AT (0.0, 0.25, 0.0) RELATIVE TO PARENT PLANE
	glUniform3f(color, 1.0, 0.0, 0.0);
	Quat r2 = Quat::makeYRotation(40.0);
	cube1.transform.rotation = r2;
	cube1.transform.translation = Cvec3(0.0, 0.25, 0.0);
	cube1.Draw(inv(eyeMatrix), positionAttribute, normalAttribute, modelViewMatrixLoc, normalMatrixLoc);

	//CUBE2 TRANSFORM AND DRAW
	//CUBE1 IS AT (0.0, 0.5, 0.75) AND ROTATES ALONG X-AXIS RELATIVE TO PARENT CUBE1
	glUniform3f(color, 0.0, 0.5, 0.75);
	Quat r3 = Quat::makeXRotation(angle);
	cube2.transform.rotation = r3;
	cube2.transform.translation = Cvec3(0.5, 0.5, 0.0);
	cube2.Draw(inv(eyeMatrix), positionAttribute, normalAttribute, modelViewMatrixLoc, normalMatrixLoc);
	
	//PROJECTION MATRIX
	Matrix4 projectionMatrix;
	projectionMatrix = projectionMatrix.makeProjection(45.0, 1.0, -0.1, -100.0);

	GLfloat glmatrixProjection[16];
	projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
	glUniformMatrix4fv(projectionMatrixLoc, 1, false, glmatrixProjection);

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

void idle(void) {
	glutPostRedisplay();
}

void init() {
	glClearDepth(0.0f);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glReadBuffer(GL_BACK);

	program = glCreateProgram();
	readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
	glUseProgram(program);

	initLocations();

	int ibLen, vbLen;

	//PLANE
	getPlaneVbIbLen(vbLen, ibLen);
	std::vector<VertexPN> vtx1(vbLen);
	std::vector<unsigned short> idx1(ibLen);

	makePlane(1.5, vtx1.begin(), idx1.begin());

	glGenBuffers(1, &plane1.geometry.vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, plane1.geometry.vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx1.size(), vtx1.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &plane1.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane1.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx1.size(), idx1.data(), GL_STATIC_DRAW);

	plane1.geometry.numIndeces = ibLen;
	plane1.parent = nullptr;

	//CUBE 1
	getCubeVbIbLen(vbLen, ibLen);

	std::vector<VertexPN> vtx2(vbLen);
	std::vector<unsigned short> idx2(ibLen);

	makeCube(0.5, vtx2.begin(), idx2.begin());

	glGenBuffers(1, &cube1.geometry.vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cube1.geometry.vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx2.size(), vtx2.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &cube1.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube1.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx2.size(), idx2.data(), GL_STATIC_DRAW);

	cube1.geometry.numIndeces = ibLen;
	cube1.parent = &plane1;

	//CUBE 2
	std::vector<VertexPN> vtx3(vbLen);
	std::vector<unsigned short> idx3(ibLen);

	makeCube(0.25, vtx3.begin(), idx3.begin());

	glGenBuffers(1, &cube2.geometry.vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cube2.geometry.vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx3.size(), vtx3.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &cube2.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube2.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx3.size(), idx3.data(), GL_STATIC_DRAW);

	cube2.geometry.numIndeces = ibLen;
	cube2.parent = &cube1;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(750, 750);
	glutCreateWindow("CS-6533");

	glewInit();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	init();
	glutMainLoop();
	return 0;
}