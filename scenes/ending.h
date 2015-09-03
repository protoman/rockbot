#ifndef ENDING_H
#define ENDING_H

#include "graphicslib.h"
#include <string>

/**
 * @brief
 *
 */
class ending
{
public:
	ending();
    /**
     * @brief
     *
     */
    /**
     * @brief
     *
     */
    void start();

    void set_boss_pos(st_position pos);


private:
    /**
     * @brief
     *
     */
    void boss();


    void boss_rockbot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos);
    void boss_betabot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos);
    void boss_candybot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos);
    void boss_kittybot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos);


    /**
     * @brief
     *
     */
    void train();
    /**
     * @brief
     *
     */
    void lab() const;


    void look_sky_betabot() const;

    void look_sky_rockbot() const;

    /**
     * @brief
     *
     */
    void bosses() const;
    /**
     * @brief
     *
     */
    void credits() const;

    /**
     * @brief
     *
     * @param pos_n
     */
    void boss_credits(int pos_n, short npc_id) const;




private:
	// from train
    graphicsLib_gSurface _train_surface; /**< TODO */
    graphicsLib_gSurface _bg_surface; /**< TODO */
    graphicsLib_gSurface _fence_surface; /**< TODO */
    graphicsLib_gSurface _bushes_surface; /**< TODO */
    st_position _boss_pos;

};

#endif // ENDING_H
