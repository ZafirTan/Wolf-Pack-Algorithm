#include <iostream>
#include <map>
#include <cmath>
#include <chrono>
#include <iterator>
#include <opencv2/opencv.hpp>
using namespace std;

#define PI 3.14
#define pi 3.14



class Point
{
public:
	Point(double x,double y)
	{
		m_X = x;
		m_Y = y;
	}
	Point()
	{
		
	}
	double m_X;
	double m_Y;
	// Point operator=(const Point &point)const
	// {
	// 	return Point(point.m_X, point.m_Y);
	// }
};

class Wolf
{
public:
	Wolf()
	{
		
	}
	Wolf(Point position)
	{
		m_Position = position;
	}
	Wolf(double x,double y)
	{
		m_Position = Point(x, y);
	}


	Point m_Position;//当前狼的坐标位置
	int m_index;//当前狼的编号
	double getX()
	{
		return m_Position.m_X;
	}
	double getY()
	{
		return m_Position.m_Y;
	}
	void setX(const double x)
	{
		m_Position.m_X = x;
	}
	void setY(const double y)
	{
		m_Position.m_Y = y;
	}
	void setPos(Point p)
	{
		m_Position = p;
	}

private:

	// Wolf operator=(const Wolf &w)const
	// {
	// 	Wolf temp;
	// 	temp.m_Position = w.m_Position;
	// 	temp.m_index = w.m_index;
	// 	return temp;
	// }
};

class WPA
{
public:
	WPA(const int wolf_num = 50, const int dim = 2, const double alpha = 4, const double beta = 6, const double w = 500, const double S = 1000, const int max_iter = 1000, const Point target_pos = Point(500, 500)) :
		m_WolfNum(wolf_num),
		dim(dim),
		m_alpha(alpha),
		m_beta(beta),
		m_w(w),
		S(S),
		max_iter(max_iter),
		m_TargetPos(target_pos)
	{
		flag = false;
		besiege_flag = false;

		//探狼数量，（取[n／(α+1)，n／α]之间的整数）
		int MIN_VALUE = wolf_num / (alpha + 1);
		int MAX_VALUE = wolf_num / alpha;
		m_ExploringWolfNum = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
		//猛狼数量，狼群数量(wolf_num)-探狼数量(exploring_wolf_num)-头狼数量(1)
		m_FierceWolfNum = wolf_num - m_ExploringWolfNum - 1;

		//初始化狼群数组
		m_HeadWolfs.resize(1);//头狼数组初始化
		m_ExploringWolfs.resize(m_ExploringWolfNum);//探狼数组初始化
		m_FierceWolfs.resize(m_FierceWolfNum);//猛狼数组初始化

		vector<Wolf> temp(m_WolfNum);//临时狼群数组
		MIN_VALUE = 0;
		MAX_VALUE = 100;
		for(int i=0;i<temp.size();++i) {
			//狼群的初始位置在x=[0,100],y=[0,100]之间的随机数
			temp[i].m_Position.m_X = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
			temp[i].m_Position.m_Y = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
			temp[i].m_index = i + 1;
		}
		sortWolfsByFitness(temp);//根据适应度排序
		for (int i = 0; i < m_headWolfNum; ++i) {//头狼
			m_HeadWolfs[i] = temp[i];
		}
		m_HeadWolfIter = m_HeadWolfs.begin();
		for(int i=0;i<m_ExploringWolfNum;++i) {//游狼
			m_ExploringWolfs[i] = temp[m_headWolfNum + i];
		}
		for(int i=0;i<m_FierceWolfNum;++i) {//猛狼
			m_FierceWolfs[i] = temp[m_ExploringWolfNum + i];
		}


		//二维空间，表示X，Y，所以把数组大小设置为2
		StepA.resize(dim);
		StepB.resize(dim);
		StepC.resize(dim);
		//初始化各种行为的步长
		//围攻步长=1.024，召唤步长=4.096，游走步长=2.048
		maxd = { 512,512 };
		mind = { -512,-512 };
		for (int i = 0; i < dim; ++i) {
			StepC[i] = abs(maxd[i] - mind[i]) / S;
			StepB[i] = 4 * StepC[i];
			StepA[i] = StepB[i] / 2;
		}

		drawWofls();
	}


	/*
	 * 函数名: calcFitness
	 * 描述: 计算适应度，即所需要求解的目标函数，fitness<=0
	 * 参数:
	 *   - X1: 原点的X坐标
	 *   - Y1: 原点的Y坐标
	 *   - X2: 目标点的X坐标
	 *   - Y2: 目标点的Y坐标
	 * 返回值: 两点间的距离
	 */
	double calcFitness(const int X1, const int Y1, const int X2, const int Y2)
	{
		//两点间距离公式
		double d = sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
		return -d;//距离越小，适应度越高
	}
	double calcFitness(Point p1,Point p2)
	{
		//两点间距离公式
		double d = sqrt(pow(p1.m_X - p2.m_X, 2) + pow(p1.m_Y - p2.m_Y, 2));
		return -d;//距离越小，适应度越高
	}

	/*
	 * 函数名: sortWolfsByFitness
	 * 描述: 适应度排序函数
	 * 参数:无
	 * 返回值: 无
	 */
	void sortWolfsByFitness(vector<Wolf> &wolfs)
	{
		//结构体排序
		auto cmp = [&](Wolf &w1, Wolf &w2)->bool {
			return calcFitness(w1.m_Position,m_TargetPos) > calcFitness(w2.m_Position,m_TargetPos) ? true : false;
		};
		//狼群数组自定义排序规则
		sort(wolfs.begin(), wolfs.end(), cmp);
	}

	void update()
	{
		//重新排序狼群数组
		vector<Wolf> temp;
		//合并数组
		for (int i = 0; i < m_headWolfNum; ++i) {//头狼
			temp.push_back(m_HeadWolfs[i]);
		}
		m_HeadWolfIter = m_HeadWolfs.begin();
		for (int i = 0; i < m_ExploringWolfNum; ++i) {//游狼
			temp.push_back(m_ExploringWolfs[i]);
		}
		for (int i = 0; i < m_FierceWolfNum; ++i) {//猛狼
			temp.push_back(m_FierceWolfs[i]);
		}
		sortWolfsByFitness(temp);//根据适应度排序
		for (int i = 0; i < m_headWolfNum; ++i) {//头狼
			m_HeadWolfs[i] = temp[i];
		}
		m_HeadWolfIter = m_HeadWolfs.begin();
		for (int i = 0; i < m_ExploringWolfNum; ++i) {//游狼
			m_ExploringWolfs[i] = temp[m_headWolfNum + i];
		}
		for (int i = 0; i < m_FierceWolfNum; ++i) {//猛狼
			m_FierceWolfs[i] = temp[m_ExploringWolfNum + i];
		}
	}


	/*
	 * 函数名: wolfsWandering
	 * 描述: WPA算法中的游走函数
	 * 参数:无
	 * 返回值: 无
	 */
	void wolfsWandering()
	{
		for (int epoch = 0; epoch < max_wandering_iter; ++epoch) {
			for (int i = 0; i < m_ExploringWolfNum; ++i) {
				if (calcFitness(m_ExploringWolfs[i].m_Position,m_TargetPos) > calcFitness(m_HeadWolfIter->m_Position,m_TargetPos)) {//若Yi大于头狼所感知的猎物气味浓度Ylead，表明猎物离探狼i已相对较近且该探狼最有可能捕获猎物
					//更新狼头位置
					Wolf temp = m_ExploringWolfs[i];
					m_ExploringWolfs[i] = *m_HeadWolfIter;
					*m_HeadWolfIter = temp;
					flag = true;//Yi>Ylead
					drawWofls();
					return;
				}
				else {//若Yi<Ylead，则探狼先自主决策，即探狼向ｈ个方向分别前进一步
					double max_x = m_ExploringWolfs[i].m_Position.m_X;
					double max_y = m_ExploringWolfs[i].m_Position.m_Y;
					for (int p = 1; p <= m_h; ++p) {
						double tempX= m_ExploringWolfs[i].m_Position.m_X + sin(2 * pi*p / m_h)*StepA[0];
						double tempY= m_ExploringWolfs[i].m_Position.m_Y + sin(2 * pi*p / m_h)*StepA[1];
						if(calcFitness(Point(max_x,max_y),m_TargetPos)<calcFitness(Point(tempX,tempY),m_TargetPos)) {
							max_x = tempX;
							max_y = tempY;
						}
					}
					m_ExploringWolfs[i].m_Position = Point(max_x, max_y);
					cout << "游走行为：" << "编号为" << m_ExploringWolfs[i].m_index << "的探狼：" << "x=" << m_ExploringWolfs[i].getX() << ",y=" << m_ExploringWolfs[i].getY() << endl;
					drawWofls();
				}
			}
		}
		flag = true;//T>Tmax
	}

	/*
	 * 函数名: wolfsCall
	 * 描述: WPA算法中的召唤函数
	 * 参数:无
	 * 返回值: 无
	 */
	void wolfsCall()
	{

		//计算判定距离d_near
		double d_near = 0;//d_near=2.048
		for (int i = 0; i < dim; ++i) {
			d_near += (1 / (dim*m_w))*abs(maxd[i] - mind[i]);
		}

		while (!besiege_flag) {

			//猛狼靠近头狼
			for (int i = 0; i < m_FierceWolfNum; ++i) {
				if (calcFitness(m_FierceWolfs[i].m_Position,m_TargetPos) > calcFitness(m_HeadWolfIter->m_Position,m_TargetPos)) {//Yi>Ylead
					//交换头狼和猛狼的身份，Yi>Ylead
					Wolf temp = m_FierceWolfs[i];
					m_FierceWolfs[i] = *m_HeadWolfIter;
					*m_HeadWolfIter = temp;
					//drawWofls();
					break;//继续执行召唤行为
				}else {//Yi<Ylead

					//猛狼奔袭
					// m_FierceWolfs[i].m_Position.m_X = m_FierceWolfs[i].getX() + StepB[0] * (m_HeadWolfIter->getX() - m_FierceWolfs[i].getX()) / abs(m_HeadWolfIter->getX() - m_FierceWolfs[i].getX());
					// m_FierceWolfs[i].m_Position.m_Y = m_FierceWolfs[i].getY() + StepB[1] * (m_HeadWolfIter->getY() - m_FierceWolfs[i].getY()) / abs(m_HeadWolfIter->getY() - m_FierceWolfs[i].getY());
					//猛狼奔袭
					if(m_HeadWolfIter->getX()-m_FierceWolfs[i].getX()>1e-20) {
						m_FierceWolfs[i].m_Position.m_X = m_FierceWolfs[i].getX() + StepB[0] * (m_HeadWolfIter->getX() - m_FierceWolfs[i].getX()) / abs(m_HeadWolfIter->getX() - m_FierceWolfs[i].getX());
					}/*else {
						m_FierceWolfs[i].m_Position.m_X = m_FierceWolfs[i].getX() + StepB[0] * 1;
					}*/
					if(m_HeadWolfIter->getY()-m_FierceWolfs[i].getY()>1e-20) {
						m_FierceWolfs[i].m_Position.m_Y = m_FierceWolfs[i].getY() + StepB[1] * (m_HeadWolfIter->getY() - m_FierceWolfs[i].getY()) / abs(m_HeadWolfIter->getY() - m_FierceWolfs[i].getY());
					}/*else {
						m_FierceWolfs[i].m_Position.m_Y = m_FierceWolfs[i].getY() + StepB[1] * 1;
					}*/
					
					cout << "召唤行为：" << "编号为" << m_FierceWolfs[i].m_index << "的猛狼：" << "x=" << m_FierceWolfs[i].getX() << ",y=" << m_FierceWolfs[i].getY() << endl;
					drawWofls();

					//转入围攻行为
					if (abs(calcFitness(m_HeadWolfIter->m_Position, m_FierceWolfs[i].m_Position)) < d_near) {
						//m_BesiegeWolfs.push_back(m_FierceWolfs[i]);//加入围攻队列
						besiege_flag = true;
						drawWofls();
						//break;
					}
				}
				
			
				//猛狼奔袭
				// if(m_HeadWolfIter->getX()-m_FierceWolfs[i].getX()>1e-20) {
				// 	m_FierceWolfs[i].m_Position.m_X = m_FierceWolfs[i].getX() + StepB[0] * (m_HeadWolfIter->getX() - m_FierceWolfs[i].getX()) / abs(m_HeadWolfIter->getX() - m_FierceWolfs[i].getX());
				// }else {
				// 	m_FierceWolfs[i].m_Position.m_X = m_FierceWolfs[i].getX() + StepB[0] * 1;
				// }
				// if(m_HeadWolfIter->getY()-m_FierceWolfs[i].getY()>1e-20) {
				// 	m_FierceWolfs[i].m_Position.m_Y = m_FierceWolfs[i].getY() + StepB[1] * (m_HeadWolfIter->getY() - m_FierceWolfs[i].getY()) / abs(m_HeadWolfIter->getY() - m_FierceWolfs[i].getY());
				// }else {
				// 	m_FierceWolfs[i].m_Position.m_Y = m_FierceWolfs[i].getY() + StepB[1] * 1;
				// }
				
			
				
			}
		}
	}

	/*
	 * 函数名: wolfsBesiege
	 * 描述: WPA算法中的围攻函数
	 * 参数:无
	 * 返回值: 无
	 */
	void wolfsBesiege()
	{
		//这部分原本为更新种群，“弱肉强食”规则，现在改成一个固定条件，不进行种群更新了
		if (abs(calcFitness(m_HeadWolfIter->m_Position,m_TargetPos)) > 10) {
			cout << "围攻行为(失败)" << endl;
			drawWofls();
			return;
		}

		int MIN_VALUE = -1;
		int MAX_VALUE = 1;
		for (int i = 0; i < m_ExploringWolfNum; ++i) {//探狼
			int r = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
			double tempX = m_ExploringWolfs[i].getX() + r * StepC[0] * abs(m_TargetPos.m_X - m_ExploringWolfs[i].getX());
			double tempY = m_ExploringWolfs[i].getY() + r * StepC[1] * abs(m_TargetPos.m_Y - m_ExploringWolfs[i].getY());
			if (calcFitness(Point(tempX, tempY), m_TargetPos) > calcFitness(m_ExploringWolfs[i].m_Position, m_TargetPos)) {
				m_ExploringWolfs[i].m_Position = Point(tempX, tempY);
			}
			cout << "围攻行为：" << "编号为" << m_ExploringWolfs[i].m_index << "的探狼：" << "x=" << m_ExploringWolfs[i].getX() << ",y=" << m_ExploringWolfs[i].getY() << endl;
		}
		for (int i = 0; i < m_FierceWolfNum; ++i) {//猛狼
			int r = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
			double tempX = m_FierceWolfs[i].getX() + r * StepC[0] * abs(m_TargetPos.m_X - m_FierceWolfs[i].getX());
			double tempY = m_FierceWolfs[i].getY() + r * StepC[1] * abs(m_TargetPos.m_Y - m_FierceWolfs[i].getY());
			if (calcFitness(Point(tempX, tempY), m_TargetPos) > calcFitness(m_FierceWolfs[i].m_Position, m_TargetPos)) {
				m_FierceWolfs[i].m_Position = Point(tempX, tempY);
			}
			cout << "围攻行为：" << "编号为" << m_FierceWolfs[i].m_index << "的猛狼：" << "x=" << m_FierceWolfs[i].getX() << ",y=" << m_FierceWolfs[i].getY() << endl;
		}
		drawWofls();
	}


	void run()
	{
		auto start = chrono::steady_clock::now();
		for (int i = 0; i < max_iter; ++i) {
			wolfsWandering();
			if (flag) {
				wolfsCall();
				if (besiege_flag) {
					wolfsBesiege();
					//update();
					if (abs(calcFitness(m_HeadWolfIter->m_Position,m_TargetPos)) < 1.0) {
						break;
					}
					else {
						//重新游走，要对标志重置
						flag = false;
						besiege_flag = false;
						//update();
					}
				}
			}
		}
		auto end = chrono::steady_clock::now();
		auto duration = chrono::duration_cast<chrono::seconds>(end - start);
		cout << "程序运行时间为：" << duration.count() << "秒" << endl;
		cout << "最优结果为：" << "x=" << m_HeadWolfIter->getX() << ",y=" << m_HeadWolfIter->getY() << endl;
		cv::waitKey(0);
	}

	void drawWofls()
	{
		cv::Mat img = cv::Mat::zeros(600, 600, CV_8U);
		//猎物为红色，头狼为绿色，探狼为白色，猛狼为蓝色
		int radius = 2;
		circle(img, cv::Point(m_TargetPos.m_X, m_TargetPos.m_Y), radius, cv::Scalar(0, 0, 255), -1);//画猎物
		for (int i = 0; i < m_ExploringWolfs.size(); ++i) {
			circle(img, cv::Point(m_ExploringWolfs[i].getX(), m_ExploringWolfs[i].getY()), radius, cv::Scalar(255, 255, 255), 1);//画探狼
		}
		for(int i=0;i<m_HeadWolfs.size();++i) {
			circle(img, cv::Point(m_HeadWolfs[i].getX(), m_HeadWolfs[i].getY()), radius, cv::Scalar(0, 255, 0), -1);//画头狼
		}
		for (int i = 0 ; i < m_FierceWolfs.size(); ++i) {
			circle(img, cv::Point(m_FierceWolfs[i].getX(), m_FierceWolfs[i].getY()), radius, cv::Scalar(255, 0, 0), 1);//画猛狼
		}
		cv::imshow("WPA-V1", img);
		cv::waitKey(0.001 * 1000);//等待0.001秒
	}

	


private:
	int m_h = 10;//h个方向
	double m_alpha = 4;//探狼比例因子
	double m_beta = 6;//狼群更新比例因子
	double m_w = 500;//距离判断因子
	int m_WolfNum = 50;//狼群数量
	int m_headWolfNum = 1;
	int m_ExploringWolfNum;//探狼数量，（取[n／(α+1)，n／α]之间的整数）
	int m_FierceWolfNum;//猛狼数量
	vector<Wolf> m_HeadWolfs;//头狼数组
	vector<Wolf> m_ExploringWolfs;//探狼数组
	vector<Wolf> m_FierceWolfs;//猛狼数组

	Point m_TargetPos;

	vector<Wolf>::iterator m_HeadWolfIter;//头狼指针
	vector<Wolf> m_BesiegeWolfs;//围攻队列

	vector<int> maxd;//维度最大值
	vector<int> mind;//维度最小值
	vector<double> StepA;//游走步长
	vector<double> StepB;//召唤步长
	vector<double> StepC;//围攻步长
	double S = 1000;//步长因子
	int dim = 2;//维度

	int max_wandering_iter = 30;//最大游走次数
	int max_iter = 1000;//最大迭代次数

	bool flag = false;//头狼更新标志	
	bool besiege_flag = false;//围攻行为标志

};


int main()
{
	WPA wpa(20, 2, 4, 6, 500, 1000, 1000, Point(500, 500));
	wpa.run();
	return 0;
}