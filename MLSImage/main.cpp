#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "config.h"
#include "MovingLeastSquare.h"
#include "learnopengl/shader_s.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

void LoadTexture()
{
	// load and create a texture 
	// -------------------------
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	unsigned char* data = stbi_load(std::string("resources/muou.png").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// -------------------------
}


void GridProcess()
{
	//InitGrid
	NormalGrid.InitGrid(X_POINT_COUNT, Y_POINT_COUNT, X_SPACING, Y_SPACING);

	//InitDeformedGrid
	Mat v = DeformedGrid.Render(X_POINT_COUNT, Y_POINT_COUNT, X_SPACING, Y_SPACING);
	Mat p = Mat::zeros(2, ControlPointsP.size(), CV_32F);
	Mat q = Mat::zeros(2, ControlPointsQ.size(), CV_32F);
	//initializing p points for fish eye image
	for (int i = 0; i < ControlPointsP.size(); i++)
	{
		p.at<float>(0, i) = (ControlPointsP.at(i)).x;
		p.at<float>(1, i) = (ControlPointsP.at(i)).y;
	}
	//initializing q points for fish eye image
	for (int i = 0; i < ControlPointsQ.size(); i++)
	{
		q.at<float>(0, i) = (ControlPointsQ.at(i)).x;
		q.at<float>(1, i) = (ControlPointsQ.at(i)).y;
	}

	double a = 2.0;
	//Precompute
	Mat w = MLSprecomputeWeights(p, v, a);
	Mat fv;//关键点的坐标
	Mat A;//for Affine Deformation 仿射形变
	vector<_typeA> tA;//for Similarity Deformation 相似变形
	typeRigid mlsd;//for Rigid Deformation 刚性变形

	printf("%d \n", obj);
	switch (obj)
	{
	case 0:
		A = MLSprecomputeAffine(p, v, w);
		fv = MLSPointsTransformAffine(w, A, q);
		break;

	case 1:
		tA = MLSprecomputeSimilar(p, v, w);
		fv = MLSPointsTransformSimilar(w, tA, q);
		break;

	case 2:
		mlsd = MLSprecomputeRigid(p, v, w);
		fv = MLSPointsTransformRigid(w, mlsd, q);
		break;
	default:
		break;
	}
	objChanged = false;
	if (obj < 3) DeformedGrid.InitDeformedGrid(X_POINT_COUNT, Y_POINT_COUNT, X_SPACING, Y_SPACING, fv);
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// --------------------


	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// ---------------------------------------


	// build and compile our shader program
	// ------------------------------------
	DrawTextureShader = new Shader("shader/4.1.texture.vs", "shader/4.1.texture.fs");
	SingleColorShader = new Shader("shader/default.vs", "shader/default.fs");
	// ------------------------------------


	// load and create a texture 
	// -------------------------
	LoadTexture();
	// -------------------------

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// Init the Grid and Get Deformed Grid
		// -------------------------
		if (objChanged) GridProcess();
		// -------------------------

		// input
		// -----
		processInput(window);


		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind Texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// render container
		DrawTextureShader->use();

		if (obj < 3)
		{
			//Render Image from 200 quad
			glBindVertexArray(DeformedGrid.Grid_VAO);
			glDrawElements(GL_TRIANGLES, DeformedGrid.Grid_indices.size() * 3, GL_UNSIGNED_INT, nullptr);
			//RenderGrid();
			DeformedGrid.RenderGrid(SingleColorShader);
		}
		else
		{
			//Render Image from 200 quad
			glBindVertexArray(NormalGrid.Grid_VAO);
			glDrawElements(GL_TRIANGLES, NormalGrid.Grid_indices.size() * 3, GL_UNSIGNED_INT, nullptr);
			//RenderGrid();
			NormalGrid.RenderGrid(SingleColorShader);
		}


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		obj = 0;
		objChanged = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		obj = 1;
		objChanged = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		obj = 2;
		objChanged = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		obj = 3;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

