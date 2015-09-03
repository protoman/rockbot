#ifndef CLASSBOSS_H
#define CLASSBOSS_H

#include "classnpc.h"
class classMap;		// advance declaration


/**
 * @brief
 *
 */
class classboss : public classnpc
{
public:
	classboss(std::string set_name);
/**
 * @brief
 *
 * @param stage_id
 * @param map_id
 * @param main_id
 * @param id
 */
    classboss(int stage_id, int map_id, int main_id, int id); // load data from game_data and create a new npc
    /**
     * @brief
     *
     */
    void boss_move();
    /**
     * @brief
     *
     */
    /**
     * @brief
     *
     */
    void execute();
    /**
     * @brief
     *
     * @return bool
     */
    bool is_boss();

private:


protected:
    /**
     * @brief
     *
     */
    void death();


private:
    short _initialized; /**< TODO */
    bool _screen_blinked; /**< TODO */
};

#endif // CLASSBOSS_H
