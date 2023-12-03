#pragma once
#include "Wolf.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
using namespace std::chrono;


class WPA
{
public:
	WPA(const int wolf_num = 50, const int dim = 2, const double alpha = 4, const double beta = 6, const double w = 500, const double S = 1000, const int max_iter = 1000);
	//h个方向
	void wandering();//游走
	void call();//召唤
	void besiege();//围攻
	void run();//运行
	void update();//更新头狼位置
	void sort_wolfs_by_fitness();//数据排序
	double cal_distance(double x1, double y1, double x2, double y2);//计算距离
	void swap_wolf(vector<Wolf>::iterator first,vector<Wolf>::iterator second);//交换数据

	void draw();//画图
private:

	int h = 10;//h个方向
	double alpha = 4;//探狼比例因子
	double beta = 6;//狼群更新比例因子
	double w = 500;//距离判断因子
	int wolf_num = 50;//狼群数量
	int exploring_wolf_num;//探狼数量，（取[n／(α+1)，n／α]之间的整数）
	int fierce_wolf_num;//猛狼数量
	vector<Wolf> wolfs;//狼群数组
	vector<Wolf>::iterator head_wolf_iter;//头狼指针
	vector<int> besiege_wolfs;//围攻队列

	vector<int> maxd;//维度最大值
	vector<int> mind;//维度最小值
	vector<double> StepA;//游走步长
	vector<double> StepB;//召唤步长
	vector<double> StepC;//围攻步长
	double S = 1000;//步长因子
	int dim = 2;//维度

	int max_wandering_iter = 30;//最大游走次数
	int max_iter = 1000;//最大迭代次数

	bool flag=false;//头狼更新标志	
	bool besiege_flag = false;//围攻行为标志

};