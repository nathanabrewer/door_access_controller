#include <Wiegand.h>

class Keypad{
private:
    int _KEYBUFFER;
    int _LASTKEYMS;
    WIEGAND wg;
public:
    Keypad();
    void poll();
};
