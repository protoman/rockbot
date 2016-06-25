#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

#include "file/format/st_common.h"

/**
 * @brief
 *
 */
class collision_detection
{
public:
	collision_detection();
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
#endif // COLlISION_DETECTION_H
