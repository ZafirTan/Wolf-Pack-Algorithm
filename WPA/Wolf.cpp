#include "Wolf.h"
Wolf::Wolf()
{
	
}
Wolf::Wolf(double x, double y) : x(x), y(y),target_x(target_x),target_y(target_y)
{
	//fitness = calculateFitness();
}
double Wolf::calculateFitness()
{
	return sqrt(pow(x - target_x, 2) + pow(y - target_y, 2));
}
