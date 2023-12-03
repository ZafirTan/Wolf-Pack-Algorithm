#pragma once
#include <cmath>
class Wolf
{
public:
	Wolf();
	Wolf(double x, double y);
	double x;//狼位置x坐标 
	double y;//狼位置y坐标
	double target_x;//目标位置x坐标
	double target_y;//目标位置y坐标
	//double fitness;//适应度 

	//计算适应度 
	double calculateFitness();
};
