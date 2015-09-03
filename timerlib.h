#ifndef TIMERLIB_H
#define TIMERLIB_H

/**
 * @brief
 *
 */
class timerLib
{
public:
/**
 * @brief
 *
 */
    timerLib();
    /**
     * @brief
     *
     * @param int
     */
    void delay(int) const;
    /**
     * @brief
     *
     * @return unsigned int
     */
    unsigned long getTimer() const;

    void start_ticker(); // start counting ticks
    long get_ticks();    // used if we need a timer count
    void pause();
    void unpause();
    bool is_paused();

private:
    bool _paused;
    long _ticks;
    long _paused_ticks;
    long _paused_timer;
};

#endif // TIMERLIB_H
