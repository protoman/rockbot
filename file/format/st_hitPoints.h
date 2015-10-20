#ifndef ST_HITPOINTS_H
#define ST_HITPOINTS_H

struct st_hit_points {
    short int total; /**< TODO */
    short int current; /**< TODO */
	st_hit_points (short int set_total, short int set_current)
	{
		total = set_total;
		current = set_current;
	}
	st_hit_points()
	{
		total = 1;
		current = 1;
	}
};

#endif // ST_HITPOINTS_H
