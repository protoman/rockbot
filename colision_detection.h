#ifndef COLISION_DETECTION_H
#define COLISION_DETECTION_H

#include "file/format/st_common.h"

/**
 * @brief
 *
 */
class colision_detection
{
public:
	colision_detection();
    /**
     * @brief
     *
     * @param A
     * @param B
     * @return bool
     */
    bool rect_overlap(st_rectangle A, st_rectangle B) const;

private:
    /**
     * @brief
     *
     * @param value
     * @param min
     * @param max
     * @return bool
     */
    bool value_in_range(int value, int min, int max) const;
};
#endif // COLISION_DETECTION_H
