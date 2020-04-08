#ifndef SHAREDDATA_H
#define SHAREDDATA_H


class SharedData
{
public:
    static SharedData* get_instance();
    static int get_max_fps();
    static double get_movement_multiplier();

    private:
        SharedData();
        SharedData(SharedData const&){};             // copy constructor is private
        SharedData& operator=(SharedData const&){ return *this; };  // assignment operator is private

private:
    static SharedData* _instance;
    static int max_fps;
    static double fps_movement_multiplier;

};

#endif // SHAREDDATA_H
