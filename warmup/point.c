#include <assert.h>
#include "common.h"
#include "point.h"
#include <math.h>
#include <stdio.h>

void
point_translate(struct point *p, double x, double y)
{
	p->x = p->x + x;
	p->y = p->y + y;
}

double
point_distance(const struct point *p1, const struct point *p2)
{
	double ax, by, dist;
	ax = p1->x - p2->x;
	by = p1->y - p2->y;

	dist = sqrt(pow(ax,2) + pow(by,2));
	return dist;
	
}

int
point_compare(const struct point *p1, const struct point *p2)
{
	double length1, length2;

	length1 = sqrt(pow(p1->x, 2)+ pow(p1->y, 2));
	length2= sqrt(pow(p2->x, 2)+ pow(p2->y, 2));

	if(length1 < length2){
		return -1;
	}else if(length1 > length2){
		return 1;
	}else{
		return 0;
	}


	
}
