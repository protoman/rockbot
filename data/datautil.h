#ifndef DATAUTIL_H
#define DATAUTIL_H


class dataUtil
{
public:
    static dataUtil* get_instance();
    bool has_weapon(int weapon_n) const;

private:
    dataUtil();
    dataUtil(dataUtil const&){};             // copy constructor is private
    dataUtil& operator=(dataUtil const&){ return *this; };  // assignment operator is private

private:
    static dataUtil* _instance;


};

#endif // DATAUTIL_H
