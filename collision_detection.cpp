#include "colision_detection.h"

colision_detection::colision_detection()
{
}

bool colision_detection::value_in_range(int value, int min, int max) const
{
	return (value >= min) && (value <= max);
}

bool colision_detection::rect_overlap(st_rectangle A, st_rectangle B) const
{
	bool xOverlap = value_in_range(A.x, B.x, B.x + B.w) || value_in_range(B.x, A.x, A.x + A.w);
	bool yOverlap = value_in_range(A.y, B.y, B.y + B.h) || value_in_range(B.y, A.y, A.y + A.h);
    //std::cout << "A.y: " << A.y << ", B.y: " << B.y << ", B.h: " << B.h << std::endl;
    //std::cout << "colision_detection::rect_overlap - xOverlap: "  << xOverlap << ", yOverlap: " << yOverlap << std::endl;
	return xOverlap && yOverlap;
}
