#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glsupport.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint program;
GLuint positionAttribute;
GLuint texCoordAttribute;
GLuint positionUniform;
GLuint timeUniform;
GLuint gundGLTexture;
GLuint vertPositionVBO;
GLuint vertTexCoordVBO;
float textureOffset = 0.0;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
	glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, gundGLTexture);

	glUniform1f(timeUniform, textureOffset);

	glUniform2f(positionUniform, -0.75, 0.0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform2f(positionUniform, 0.75, 0.0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(texCoordAttribute);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

void idle(void) {
	glutPostRedisplay();
}

void init() {
	
	int w, h, comp;
	unsigned char* image = stbi_load("gs.png", &w, &h, &comp, STBI_rgb_alpha);

	glGenTextures(1, &gundGLTexture);

	glBindTexture(GL_TEXTURE_2D, gundGLTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	if (comp == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	else if (comp == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	program = glCreateProgram();
	readAndCompileShader(program, "vertex_textured.glsl", "fragment_textured.glsl");

	glUseProgram(program);
	positionAttribute = glGetAttribLocation(program, "position");
	texCoordAttribute = glGetAttribLocation(program, "texCoord");
	positionUniform = glGetUniformLocation(program, "modelPosition");
	timeUniform = glGetUniformLocation(program, "time");

	glGenBuffers(1, &vertPositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	float vertices[12] = {
		-0.75, -0.5,
		0.75, -0.5,
		0.75, 0.5,

		-0.75, -0.5,
		0.75, 0.5,
		-0.75, 0.5
	};
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &vertTexCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
	float texCoords[12] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f };
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), texCoords, GL_STATIC_DRAW);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		textureOffset += 0.25;
		break;
	case 's':
		textureOffset -= 0.25;
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutCreateWindow("CS-6533");

	glewInit();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keyboard);

	init();
	glutMainLoop();
	return 0;
}