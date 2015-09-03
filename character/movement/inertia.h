#ifndef INERTIA_H
#define INERTIA_H

class inertia
{
public:
    inertia();
    int execute();              // handles inertia to star tmoving and to stop. return xInc
    void start();               // to be called when player is over ice and moving
    void stop();                // jumping or getting out of ice stops inertia
    bool is_started();

private:
    float _counter;             // stores how much has moved with inertia
    int _sub_counter;           // float part
    bool _started;
};

#endif // INERTIA_H
