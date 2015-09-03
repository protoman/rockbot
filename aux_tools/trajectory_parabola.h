#ifndef TRAJECTORY_PARABOLA_H
#define TRAJECTORY_PARABOLA_H

class trajectory_parabola
{
public:
    trajectory_parabola();
    trajectory_parabola(int distance);

    void set_initial_x(int p0);
    void set_acceleration(int a);
    void set_distance(int dist);

    int get_y_point(int n) const;
    float get_velocity();


private:
int _p0; // initial pointx
int _a; // acceleration
int _dist; // distance between initial point and target
int _max_point; // maximum y point (got from x=distance/2)
int _v;         // velocity
float _reducefactor;
bool _initialized;



};


#endif // TRAJECTORY_PARABOLA_H
