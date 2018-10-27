#pragma once
#include "../3party/glm/include/glm/vec3.hpp"
#include <vector>
#include <glad/glad.h>
#include <opencv/highgui.h>
#include "learnopengl/shader_s.h"


class Grid
{
public:
	Grid()
	{
		Grid_VAO = 0;
		Grid_VBO = 0;
		Grid_EBO = 0;
	}

	~Grid()
	{
		glDeleteVertexArrays(1, &Grid_VAO);
		glDeleteBuffers(1, &Grid_VBO);
		glDeleteBuffers(1, &Grid_EBO);
	}

	std::vector<glm::vec3> Grid_vertices;
	std::vector<glm::ivec3> Grid_indices;
	unsigned int Grid_VBO, Grid_VAO, Grid_EBO;

	//Get the space coord from input idx
	static GLfloat* ComputePointCoordinates(GLuint pointIndex, GLuint xPointCount, GLuint yPointCount, GLfloat xSpacing, GLfloat ySpacing);
	static void ComputePointCoordinate(int pointIndex, int xPointCount, int yPointCount, float xSpacing, float ySpacing, float pt[3]);

	//render all grid points for calculation
	cv::Mat Render(int xPointCount, int yPointCount, double xSpacing, double ySpacing) const;

	//Normal Grid Init
	void InitGrid(GLuint xPointCount, GLuint yPointCount, GLfloat xSpacing, GLfloat ySpacing);

	//Deformed Grid Init
	void InitDeformedGrid(GLuint xPointCount, GLuint yPointCount, GLdouble xSpacing, GLdouble ySpacing, cv::Mat& fv);

	//RenderGrid
	void RenderGrid(Shader* shader) const;
};


inline GLfloat* Grid::ComputePointCoordinates(GLuint pointIndex,
                                              GLuint xPointCount, GLuint yPointCount,
                                              GLfloat xSpacing, GLfloat ySpacing)
{
	static GLfloat pt[3];

	GLfloat width = xSpacing * (xPointCount - 1); //0.2*10 = 2.0
	GLfloat height = ySpacing * (yPointCount - 1);//0.2*10 = 2.0
	GLfloat minX = -width / 2; // -2.0/2 = -1.0
	GLfloat minY = -height / 2;// -2.0/2 = -1.0
	pt[0] = minX + xSpacing * (pointIndex % xPointCount);//-1.0+0.2*(34%11) = x方向
	pt[1] = minY + ySpacing * (pointIndex / xPointCount);//-1.0+0.2*(34/11) = y方向
	pt[2] = 0;

	return pt;
}

inline void Grid::ComputePointCoordinate(int pointIndex,
                                         int xPointCount, int yPointCount,
                                         float xSpacing, float ySpacing,
                                         float pt[3])
{
	//标准坐标为：
	float* tmp = ComputePointCoordinates(pointIndex, xPointCount, yPointCount, xSpacing, ySpacing);

	pt[0] = tmp[0];
	pt[1] = tmp[1];
	pt[2] = tmp[2];
}


//render all grid points for calculation
inline cv::Mat Grid::Render(int xPointCount, int yPointCount, double xSpacing, double ySpacing) const
{
	//change the size of the grid
	cv::Mat a = cv::Mat::zeros(2, xPointCount * yPointCount, CV_32F);
	double width = xSpacing * (xPointCount - 1);
	double height = ySpacing * (yPointCount - 1);
	double minX = -width / 2;
	double minY = -height / 2;

	for (int i = 0; i < xPointCount; i++)
	{
		for (int j = 0; j < yPointCount; j++)
		{
			double x = minX + i * xSpacing;
			double y = minY + j * ySpacing;

			//save the mat points to get v
			a.at<float>(0, i * yPointCount + j) = static_cast<float>(x);
			a.at<float>(1, i * yPointCount + j) = static_cast<float>(y);
		}
	}

	return a;
}

inline void Grid::InitGrid(GLuint xPointCount, GLuint yPointCount, GLfloat xSpacing, GLfloat ySpacing)
{
	float width = (xPointCount - 1) * xSpacing;
	float height = (yPointCount - 1) * ySpacing;
	float minX = -width / 2;
	float minY = -height / 2;

	Grid_vertices.clear();
	Grid_indices.clear();

	GLuint nrQuads = (xPointCount - 1) * (yPointCount - 1);
	for (GLuint i = 0; i < nrQuads; i++)
	{
		GLuint k = i + i / (xPointCount - 1);
		GLuint a = k;//LU
		GLuint b = k + 1;//RU
		GLuint c = k + 1 + xPointCount;//RD
		GLuint d = k + xPointCount;//LD
		GLfloat aPt[3], bPt[3], cPt[3], dPt[3];
		// 根据顶点的位置算出顶点的纹理坐标
		ComputePointCoordinate(a, xPointCount, yPointCount, xSpacing, ySpacing, aPt);
		ComputePointCoordinate(b, xPointCount, yPointCount, xSpacing, ySpacing, bPt);
		ComputePointCoordinate(c, xPointCount, yPointCount, xSpacing, ySpacing, cPt);
		ComputePointCoordinate(d, xPointCount, yPointCount, xSpacing, ySpacing, dPt);

		// Triangle 1 c d a
		// Pos Color Coord
		Grid_vertices.push_back(glm::vec3(cPt[0], cPt[1], cPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((cPt[0] - minX) / width, (cPt[1] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(dPt[0], dPt[1], dPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 0.0, 1.0));
		Grid_vertices.push_back(glm::vec3((dPt[0] - minX) / width, (dPt[1] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(aPt[0], aPt[1], aPt[2]));
		Grid_vertices.push_back(glm::vec3(1.0, 0.0, 0.0));
		Grid_vertices.push_back(glm::vec3((aPt[0] - minX) / width, (aPt[1] - minY) / height, 0.0));

		Grid_indices.push_back(glm::ivec3(Grid_vertices.size() / 3 - 3, Grid_vertices.size() / 3 - 2, Grid_vertices.size() / 3 - 1));


		// Triangle 2 a b c
		// Pos Color Coord
		Grid_vertices.push_back(glm::vec3(aPt[0], aPt[1], aPt[2]));
		Grid_vertices.push_back(glm::vec3(1.0, 0.0, 0.0));
		Grid_vertices.push_back(glm::vec3((aPt[0] - minX) / width, (aPt[1] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(bPt[0], bPt[1], bPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((bPt[0] - minX) / width, (bPt[1] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(cPt[0], cPt[1], cPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 0.0, 1.0));
		Grid_vertices.push_back(glm::vec3((cPt[0] - minX) / width, (cPt[1] - minY) / height, 0.0));

		Grid_indices.push_back(glm::ivec3(Grid_vertices.size() / 3 - 3, Grid_vertices.size() / 3 - 2, Grid_vertices.size() / 3 - 1));
	}

	//printf("vertices length:%d indices length:%d\n", static_cast<int>(Grid_vertices.size()), static_cast<int>(Grid_indices.size()));

	glGenVertexArrays(1, &Grid_VAO);
	glGenBuffers(1, &Grid_VBO);
	glGenBuffers(1, &Grid_EBO);

	glBindVertexArray(Grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, Grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, Grid_vertices.size() * sizeof(glm::vec3), &Grid_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Grid_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Grid_indices.size() * sizeof(glm::ivec3), &Grid_indices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), reinterpret_cast<void*>(1 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), reinterpret_cast<void*>(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
}

inline void Grid::InitDeformedGrid(GLuint xPointCount, GLuint yPointCount, GLdouble xSpacing, GLdouble ySpacing, cv::Mat& fv)
{
	double width = (xPointCount - 1) * xSpacing;
	double height = (yPointCount - 1) * ySpacing;
	double minX = -width / 2;
	double minY = -height / 2;

	Grid_vertices.clear();
	Grid_indices.clear();

	int nrQuads = (xPointCount - 1) * (yPointCount - 1);
	for (int i = 0; i < nrQuads; i++)
	{
		int k = i + i / (xPointCount - 1);
		int a = k;
		int b = k + 1;
		int c = k + 1 + xPointCount;
		int d = k + xPointCount;
		double aPt[3], bPt[3], cPt[3], dPt[3];
		float aPtIm[3], bPtIm[3], cPtIm[3], dPtIm[3];
		//由 abcd 的idx算出其标准的纹理坐标
		ComputePointCoordinate(a, xPointCount, yPointCount, xSpacing, ySpacing, aPtIm);
		ComputePointCoordinate(b, xPointCount, yPointCount, xSpacing, ySpacing, bPtIm);
		ComputePointCoordinate(c, xPointCount, yPointCount, xSpacing, ySpacing, cPtIm);
		ComputePointCoordinate(d, xPointCount, yPointCount, xSpacing, ySpacing, dPtIm);

		//get the deformed points 
		//由变换矩阵fv得到 abcd 在变换之后的空间位置
		aPt[0] = fv.at<float>(0, a);
		aPt[1] = fv.at<float>(1, a);
		aPt[2] = 0;

		bPt[0] = fv.at<float>(0, b);
		bPt[1] = fv.at<float>(1, b);
		bPt[2] = 0;

		cPt[0] = fv.at<float>(0, c);
		cPt[1] = fv.at<float>(1, c);
		cPt[2] = 0;

		dPt[0] = fv.at<float>(0, d);
		dPt[1] = fv.at<float>(1, d);
		dPt[2] = 0;

		//mapping the actual points 
		// Triangle 1 c d a
		// Pos Color Coord
		Grid_vertices.push_back(glm::vec3(cPt[0], cPt[1], cPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((cPtIm[1] - minX) / width, (cPtIm[0] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(dPt[0], dPt[1], dPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 0.0, 1.0));
		Grid_vertices.push_back(glm::vec3((dPtIm[1] - minX) / width, (dPtIm[0] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(aPt[0], aPt[1], aPt[2]));
		Grid_vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((aPtIm[1] - minX) / width, (aPtIm[0] - minY) / height, 0.0));

		Grid_indices.push_back(glm::ivec3(Grid_vertices.size() / 3 - 3, Grid_vertices.size() / 3 - 2, Grid_vertices.size() / 3 - 1));

		// Triangle 2 a b c
		// Pos Color Coord
		Grid_vertices.push_back(glm::vec3(aPt[0], aPt[1], aPt[2]));
		Grid_vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((aPtIm[1] - minX) / width, (aPtIm[0] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(bPt[0], bPt[1], bPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 1.0, 0.0));
		Grid_vertices.push_back(glm::vec3((bPtIm[1] - minX) / width, (bPtIm[0] - minY) / height, 0.0));

		Grid_vertices.push_back(glm::vec3(cPt[0], cPt[1], cPt[2]));
		Grid_vertices.push_back(glm::vec3(0.0, 0.0, 1.0));
		Grid_vertices.push_back(glm::vec3((cPtIm[1] - minX) / width, (cPtIm[0] - minY) / height, 0.0));

		Grid_indices.push_back(glm::ivec3(Grid_vertices.size() / 3 - 3, Grid_vertices.size() / 3 - 2, Grid_vertices.size() / 3 - 1));
	}

	//printf("vertices length:%d indices length:%d\n", static_cast<int>(Grid_vertices.size()), static_cast<int>(Grid_indices.size()));

	glGenVertexArrays(1, &Grid_VAO);
	glGenBuffers(1, &Grid_VBO);
	glGenBuffers(1, &Grid_EBO);

	glBindVertexArray(Grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, Grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, Grid_vertices.size() * sizeof(glm::vec3), &Grid_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Grid_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Grid_indices.size() * sizeof(glm::ivec3), &Grid_indices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), reinterpret_cast<void*>(1 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	// texture coord attributes
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), reinterpret_cast<void*>(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
}

inline void Grid::RenderGrid(Shader* shader) const
{
	// render container
	shader->use();
	glBindVertexArray(Grid_VAO);
	glDrawElements(GL_LINES, Grid_indices.size() * 3, GL_UNSIGNED_INT, nullptr);
}

