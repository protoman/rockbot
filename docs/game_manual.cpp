#include "game_manual.h"

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "timerlib.h"
extern timerLib timer;

#include "graphic/draw.h"
extern draw draw_lib;

#include "soundlib.h"
extern soundLib soundManager;


game_manual::game_manual()
{
    generate_pages();
}

void game_manual::execute()
{
    int page = 0;
    input.clean();
    timer.delay(10);

    draw_page(page);

    while (true) {
        input.readInput();

        if (input.p1_input[BTN_LEFT] == 1 || input.p1_input[BTN_RIGHT] == 1) {
            soundManager.play_sfx(SFX_CURSOR);
            if (input.p1_input[BTN_LEFT] == 1) {
                page--;
            } else if (input.p1_input[BTN_RIGHT] == 1) {
                page++;
            }

            std::cout << "PAGE[BEFORE]: " << page << ", list.size: " << page_list.size() << std::endl;

            if (page > 0 && page >= page_list.size()) {
                std::cout << "RESET #1" << std::endl;
                page = 0;
            } else if (page < 0) {
                std::cout << "RESET #2" << std::endl;
                page = page_list.size()-1;
            }

            std::cout << "PAGE[AFTER]: " << page << std::endl;

            draw_page(page);
            draw_lib.update_screen();
        }

        if (input.p1_input[BTN_ATTACK] == 1 || input.p1_input[BTN_JUMP] == 1 || input.p1_input[BTN_START] == 1) {
            break;
        }

        input.clean();
        timer.delay(10);

    }
}


void game_manual::draw_page(int page_n)
{
    if (page_n < 0 || page_n >= page_list.size()) {
        return;
    }
    st_manual_page temp = page_list.at(page_n);
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);

    int initial_y = 8;
    st_color title_color(95, 151, 255);
    st_color text_color(235, 235, 235);

    for (int i=0; i<PAGE_LINES; i++) {
        int y = initial_y + i*10;

        //std::cout << "MANUAL::draw_page::y: " << y << std::endl;

        if (i == 0) {
            graphLib.draw_centered_text(y, temp.line[i], title_color);
        } else {
            int y = initial_y + 10 + i*10;
            graphLib.draw_text(12, y, temp.line[i], text_color);
        }
    }

    char page_number[3];
    sprintf(page_number, "%d/%d", (page_n+1), page_list.size());
    std::string page_line = std::string("< PAGE ") + std::string(page_number) + std::string(" >");

    graphLib.draw_centered_text(RES_H-12, page_line, text_color);

    graphLib.updateScreen();

}

void game_manual::generate_pages()
{
    struct st_manual_page temp;

    temp.add_line("== CONTROLING CHARACTER [1] ==");
    temp.add_line("MOVE LEFT OR RIGHT:");
    temp.add_line("PRESS CONTROL PAD LEFT/RIGHT.");
    temp.add_line("");
    temp.add_line("CLIMB UP OR DOWN:");
    temp.add_line("PRESS CONTROL PAD UP/DOWN.");
    temp.add_line("");
    temp.add_line("JUMP: PRESS (A) BUTTON. THE LONGER");
    temp.add_line("YOU HOLD DOWN BUTTON, THE HIGHER");
    temp.add_line("YOU WILL JUMP.");
    temp.add_line("");
    temp.add_line("FIRE WEAPONS:");
    temp.add_line("PRESS (B) BUTTON. PRESS (A) AND (B)");
    temp.add_line("TOGETHER TO FIRE WHILE JUMPING.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CONTROLING CHARACTER [2] ==");
    temp.add_line("SHIELD: PRESS AND HOLD (Y) BUTTON");
    temp.add_line("TO DEFLECT PROJECTILES. [*]");
    temp.add_line("");
    temp.add_line("SLIDE/DASH: PRESS (X) BUTTON.");
    temp.add_line("HOLD BUTTON TO MOVE LONGER. [*]");
    temp.add_line("");
    temp.add_line("[*] SOME CHARACTERS MAY NOT");
    temp.add_line("HAVE THIS ABILITY, SO THE BUTTON");
    temp.add_line("WILL NOT EXECUTE ANY ACTION.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== GETTING STARTED ==");
    temp.add_line("INSERT ROCKBOT MEGA PACK INTO YOUR");
    temp.add_line("ENTERTAINMENT SYSTEM.");
    temp.add_line("");
    temp.add_line("AT THE TITLE SCREEN, PRESS UP/DOWN");
    temp.add_line("TO CHOOSE MODE, THEN PRESS START.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== MODES ==");
    temp.add_line("NEW GAME:");
    temp.add_line("START A GAME FROM BEGGINING,");
    temp.add_line("WILL ERASE PREVIOUS SAVED GAME.");
    temp.add_line("");
    temp.add_line("LOAD GAME:");
    temp.add_line("RESUMES A GAME THAT WAS SAVED.");
    temp.add_line("GAME IS SAVED EACH TIME YOU BEAT");
    temp.add_line("A BOSS OR LEAVE GAME OR STAGE.");
    temp.add_line("");
    temp.add_line("PASSWORD:");
    temp.add_line("YOU CAN INPUT A PASSWORD TO RESUME");
    temp.add_line("GAME FROM A GIVEN POINT.");
    temp.add_line("");
    temp.add_line("CONFIG:");
    temp.add_line("CHANGE GAME AND DEVICE SETTINGS.");
    temp.add_line("");
    temp.add_line("ABOUT:");
    temp.add_line("SHOW GAME CREDITS AND INFORMATION.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CONFIG ==");
    temp.add_line("IN CONFIG YOU CAN CHANGE SEVERAL");
    temp.add_line("SETTINGS FOR THE GAME, SUCH AS");
    temp.add_line("AUDIO, INPUT AND VIDEO.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== AUDIO OPTIONS ==");
    temp.add_line("YOU CAN SELECT ENABLE/DISABLE TO");
    temp.add_line("TURN ON OR OFF THE MUSIC AND");
    temp.add_line("SOUND EFFECTS ON THE GAME.");
    temp.add_line("");
    temp.add_line("YOU CAN ALSO SET VOLUME LEVELS");
    temp.add_line("FOR BOTH MUSIC AND SOUND EFFECTS.");
    temp.add_line("FOR THAT, ENTER OPTION AND USE");
    temp.add_line("LEFT AND RIGHT CONTROLS TO INCREASE");
    temp.add_line("OR DECREASE THE VOLUME VALUE.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== INPUT OPTIONS [1] ==");
    temp.add_line("CONFIG BUTTONS:");
    temp.add_line("IN THIS OPTION YOU CAN SET THE");
    temp.add_line("KEY OR BUTTON IN YOUR KEYBOARD,");
    temp.add_line("JOYSTICK OR GAME CONTROLLER FOR");
    temp.add_line("THE GIVEN GAME ACTION.");
    temp.add_line("YOU CAN ALSO RESET THE CONTROLS");
    temp.add_line("TO DEFAULT KEYS/BUTTONS HERE.");
    temp.add_line("");
    temp.add_line("DIRECTIONAL MODE:");
    temp.add_line("YOU CAN CHOOSE BETWEEN USING");
    temp.add_line("THE DIGITAL KEYPAD OR THE");
    temp.add_line("ANALOG STICK FOR CONTROLLERS");
    temp.add_line("THAT HAVE BOTH.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== INPUT OPTIONS [2] ==");
    temp.add_line("SELECTED JOYSTICK:");
    temp.add_line("IF YOUR GAME DEVICE HAS MORE THAN");
    temp.add_line("ONE GAMER CONTROLLER CONNECTED,");
    temp.add_line("YOU CAN CHOOSE WITCH ONE WILL BE");
    temp.add_line("USED TO CONTROL YOUR CHARACTER.");
    temp.add_line("");
    temp.add_line("TURBO MODE:");
    temp.add_line("IF ENABLED, YOU CAN HOLD THE");
    temp.add_line("ATTACK BUTTON AND THE CHARACTER");
    temp.add_line("WILL KEEP FIRING SHOTS.");
    temp.add_line("NOTE: IN THIS MODE YOU CAN'T USE");
    temp.add_line("CHARGED WEAPONS.");
    page_list.push_back(temp);
    temp.clear();


    temp.add_line("== VIDEO OPTIONS ==");
    temp.add_line("VIDEO MODE: ON PC PLATFORMS,");
    temp.add_line("YOU CAN SELECT BETWEEN MODES");
    temp.add_line("UNSCALED, DOUBLE SIZE AND SCALE2X,");
    temp.add_line("THAT IS UPSCALED WITH");
    temp.add_line("SMOOTHENING FILTER. [*]");
    temp.add_line("");
    temp.add_line("[*] FOR THIS CHANGE TO TAKE EFFECT,");
    temp.add_line("YOU NEED TO RESTART THE GAME.");
    temp.add_line("");
    temp.add_line("FULLSCREEN:");
    temp.add_line("IN DEVICES THAT SUPPORT THIS OPTION,");
    temp.add_line("THE VIDEO WILL SWITCH TO USE");
    temp.add_line("ENTIRE SCREEN.");
    temp.add_line("IN PSP, IT WILL CHANGED BETWEEN");
    temp.add_line("KEEP ASPECT RATIO ON OR OFF.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CONFIG [ANDROID] ==");
    temp.add_line("WHEN PLAYING ON AN ANDROID");
    temp.add_line("GAME DEVICE, YOU CAN USE THE");
    temp.add_line("SDL MENU THAT APPEARS BEFORE");
    temp.add_line("GAME STARTS, TO CHANGE ON-SCREEN");
    temp.add_line("CONTROLS SIZE AND POSITION.");
    temp.add_line("");
    temp.add_line("PLEASE REFER TO GAME DESCRIPTION");
    temp.add_line("IN APP STORE FOR A LINK TO");
    temp.add_line("A VIDEO THAT SHOWS HOW TO");
    temp.add_line("USE THIS SETTING.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== PLAYING THE GAME [1] ==");
    temp.add_line("FIGHT YOUR WAY IN A MAZE FULL OF");
    temp.add_line("MAD ROBOTS AND CRAZY MACHINES.");
    temp.add_line("YOU WILL HAVE TO DEFEAT EIGHT");
    temp.add_line("ROBOT MASTERS THAT USE INCREDIBLE");
    temp.add_line("WEAPONS THAT YOU ACQUIRE ONCE");
    temp.add_line("THEY ARE DESTROYED.");
    temp.add_line("");
    temp.add_line("TO ACHIEVE VICTORY YOU CAN");
    temp.add_line("RUN, SLIDE, JUMP AND SHOOT");
    temp.add_line("IN ORDER TO SAFELY REACH THE");
    temp.add_line("END OF EACH STAGE");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== PLAYING THE GAME [2] ==");
    temp.add_line("IN THE LEFT SIDE OF THE SCREEN");
    temp.add_line("THERE IS HEALTH METER. WHEN IT");
    temp.add_line("REACHES ZERO, YOU LOOSE A CHANCE.");
    temp.add_line("");
    temp.add_line("EVERY TIME YOU ARE HIT BY");
    temp.add_line("A PROJECTILE OR TOUCH AN ENEMY,");
    temp.add_line("YOUR HELTH DECREASES A BIT.");
    temp.add_line("WHEN YOU HIT SPIKES, ALL YOUR");
    temp.add_line("HEALTH IS GONE AT ONCE. TAKE CARE!");
    temp.add_line("");
    temp.add_line("PICK ENERGY OR WEAPON PELLETS");
    temp.add_line("AND ENERGY TANKS TO INCREASE");
    temp.add_line("YOUR CHANCES TO FINISH STAGE.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== PLAYING THE GAME [3] ==");
    temp.add_line("WHEN YOU LOSE A CHANCE, YOU START");
    temp.add_line("STAGE AGAIN FROM LAST CHECKPOINT");
    temp.add_line("OR STAGE START POINT.");
    temp.add_line("");
    temp.add_line("IF YOU LOSE ALL YOUR CHANCES,");
    temp.add_line("THE GAME ENDS AND YOU HAVE TO");
    temp.add_line("PICK AGAIN A STAGE TO START OVER.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== POWER UP [1] ==");
    temp.add_line("");
    temp.add_line("DURING STAGE OR WHEN DEFEATING");
    temp.add_line("AND ENEMY, YOU CAN GET ITEMS THAT");
    temp.add_line("HELP YOU TO FINISH THE LEVEL.");
    temp.add_line("");

    temp.add_line("ENERGY BALL OR PELLET:");
    temp.add_line("REFILL PART OF YOUR HEALTH METTER.");

    temp.add_line("");
    temp.add_line("WEAPON BALL OR PELLET:");
    temp.add_line("REFILL A WEAPON ENERGY. YOU NEED");
    temp.add_line("TO SELECT A WEAPON BEFORE GETTING");
    temp.add_line("THE ITEM FOR IT TO WORK.");

    temp.add_line("");
    temp.add_line("LIFE (1UP):");
    temp.add_line("GIVES YOU AND EXTRA CHANCE TO");
    temp.add_line("AVOID GAME OVER.");
    temp.add_line("");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== POWER UP [2] ==");
    temp.add_line("");
    temp.add_line("RED TANK:");
    temp.add_line("STORES HEALTH ENERY AND CAN BE");
    temp.add_line("USED AT ANY TIME BY ACCESSING");
    temp.add_line("THE WEAPONS MENU. FILL COMPLETLY");
    temp.add_line("YOUR HEALTH METTER.");

    temp.add_line("");
    temp.add_line("GREEN TANK:");
    temp.add_line("FILL ALL YOUR WEAPONS ENERGY,");
    temp.add_line("CAN BE USED IN THE WEAPONS");
    temp.add_line("MENU AT ANY TIME.");

    temp.add_line("");
    temp.add_line("YELLOW TANK:");
    temp.add_line("THIS SUPER TANK WILL");
    temp.add_line("REFILL BOTH YOUR HEALTH");
    temp.add_line("AS ALL YOUR WEAPONS");
    temp.add_line("ENERGY!");

    page_list.push_back(temp);
    temp.clear();


    temp.add_line("== SPECIAL ITEMS ==");
    temp.add_line("");
    temp.add_line("THERE ARE A FEW SECRET SPOTS");
    temp.add_line("WHERE YOU CAN GET SPECIAL ITEMS.");
    temp.add_line("");
    temp.add_line("THERE ARE THREE ARMOR PIECES,");
    temp.add_line("EACH ONE WILL GIVE YOU A NEW");
    temp.add_line("ABILITY, LIKE DASHING ON MIDAIR,");
    temp.add_line("FIRING SPECIAL WEAPONS OR");
    temp.add_line("TAKING HALF DAMAGE.");
    temp.add_line("");
    temp.add_line("EACH CHARACTER YOU CAN PLAY");
    temp.add_line("HAVE DIFFERENT ABILITIES FOR");
    temp.add_line("EACH ARMOR PIECE HE GETS.");
    page_list.push_back(temp);
    temp.clear();


    temp.add_line("== USING PASSWORDS ==");
    temp.add_line("");
    temp.add_line("EVERY TIME YOU BEAT AN STAGE,");
    temp.add_line("A PASSWORD WILL APPEAR ON SCREEN.");
    temp.add_line("TAKE NOTE OF THIS IN ORDER TO");
    temp.add_line("CONTINUE GAME ON ANOTHER DEVICE.");
    page_list.push_back(temp);
    temp.clear();


    temp.add_line("== ROBOT MASTERS [1] ==");
    temp.add_line("SNOW BOT");
    temp.add_line("A COLD BODY AND DISTANT ATTITUDE");
    temp.add_line("HIDES A WARM HEART.");

    temp.add_line("");
    temp.add_line("VALKYRIE BOT");
    temp.add_line("FROM NORDIC ORIGINS, THIS GIRL");
    temp.add_line("ROBOT CONTROLS THE WEATHER");
    temp.add_line("LIKE THE GOD THOR.");

    temp.add_line("");
    temp.add_line("KURUPIRA BOT");
    temp.add_line("PROTECTOR OF THE JUNGLE AND");
    temp.add_line("ANIMALS, LOVES THE GOOD LIFE.");

    temp.add_line("");
    temp.add_line("GEAR BOT");
    temp.add_line("MILITARY ROBOT MADE FOR");
    temp.add_line("ONLY ONE PURPOSE, TO BRING");
    temp.add_line("WAR TO HIS ENEMIES.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== ROBOT MASTERS [2] ==");
    temp.add_line("BEAST BOT:");
    temp.add_line("TRAVELLING WITH A CIRCUS, HE IS");
    temp.add_line("VERY AGRESSIVE AND ANGRY.");

    temp.add_line("");
    temp.add_line("PHANTOM BOT");
    temp.add_line("A LOVER OF THE NIGHT AND OPERA.");

    temp.add_line("");
    temp.add_line("DRAGON BOT");
    temp.add_line("FROM FAR EAST, THIS MASTER");
    temp.add_line("HAVE UNKNOWN MYSTICAL POWERS.");

    temp.add_line("");
    temp.add_line("ZODIAC BOT");
    temp.add_line("POWERED BY THE STARS, THIS");
    temp.add_line("ROBOT IS PROTECTED BY THE");
    temp.add_line("LEGENDARY GOLDEN ARMOR.");
    page_list.push_back(temp);
    temp.clear();


    temp.add_line("== TIPS FOR WINNING ==");
    temp.add_line("");
    temp.add_line("IF YOUR CHARACTER CAN SLIDE,");
    temp.add_line("YOU CAN PASS UNDER OBSTACLES.");
    temp.add_line("");
    temp.add_line("WHEN DASHING, PRESS JUMP");
    temp.add_line("TO MAKE A REALLY LONG CROSS.");
    temp.add_line("");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CHARACTERS [ROCK] ==");
    temp.add_line("");
    temp.add_line("THE HERO OF THE CITY AFTER");
    temp.add_line("SAVING IT FROM DR. DESTRIN,");
    temp.add_line("ROCKBOT AS GOOD AS IT GETS,");
    temp.add_line("ALWAYS FOLLOWING THE LAWS");
    temp.add_line("AND ORDERS FROM GOOD DOCTOR");
    temp.add_line("KANOTUS, HIS CREATOR.");
    temp.add_line("");
    temp.add_line("WITH ROCKBOT YOU CAN FIRE");
    temp.add_line("CHARGED SHOTS, SLIDE UNDER");
    temp.add_line("OBSTACLES AND USE A SHIELD.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CHARACTERS [CANDY] ==");
    temp.add_line("");
    temp.add_line("CREATED TO BE AN MERE LAB");
    temp.add_line("ASSISTANT FOR DR. KANOTUS,");
    temp.add_line("CANDYBOT SAW NO OPTION BUT");
    temp.add_line("TO START FIGHTING TOGETHER");
    temp.add_line("WITH HER BROTHER, ROCK, WHEN");
    temp.add_line("HER OTHER BROTHER, BETA, LEFT.");
    temp.add_line("");
    temp.add_line("CANDY CAN FIRE SHOTS IN");
    temp.add_line("MULTIPLE DIRECTIONS AND");
    temp.add_line("JUMP ONE MORE TIME IN MIDAIR.");
    temp.add_line("AS NOT BEING MADE FOR FIGHTING,");
    temp.add_line("SHE TAKES MORE DAMAGE,");
    temp.add_line("CAN'T WALK WHILE SHOOTING");
    temp.add_line("NOR DASH OR USE SHIELD.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CHARACTERS [BETA] ==");
    temp.add_line("");
    temp.add_line("DR. KANOTUS FIRST CREATION,");
    temp.add_line("BETA NEVER WANTED TO BE A HERO");
    temp.add_line("OR SAVE THE CITY. HIS ONLY WILL");
    temp.add_line("IS TO FIGHT STRONG OPPONENTS.");
    temp.add_line("HE LEFT ROCK BEHIND WHEN");
    temp.add_line("REALIZING HE WOULD NEVER HAD");
    temp.add_line("TRUE FREEDOM WHILE LOCKED.");
    temp.add_line("IN A SCIENCE LAB.");
    temp.add_line("");
    temp.add_line("BETA CAN'T CHARGE, BUT HE FIRES");
    temp.add_line("TWO PROJECTILES AT ONCE, MAKING");
    temp.add_line("EASIER TO HIT ENEMIES.");
    temp.add_line("ALSO, HE TAKE LESS DAMAGE");
    temp.add_line("THAN OTHER ROBOTS AND CAN");
    temp.add_line("DASH TO MAKE LONGER JUMPS.");
    page_list.push_back(temp);
    temp.clear();

    temp.add_line("== CHARACTERS [KITTY] ==");
    temp.add_line("");
    temp.add_line("A FREE SPIRIT THAT FOUND BETA");
    temp.add_line("IN A MOMENT OF NEED, SHE DECIDED");
    temp.add_line("TO HELP HIM TO FIND HIS PLACE");
    temp.add_line("IN THE WORLD.");
    temp.add_line("LITTLE IS KNOWN ABOUT THIS");
    temp.add_line("CAT-ROBOT, NOT EVEN WHO WAS");
    temp.add_line("THE ONE THAT BUILT HER.");
    temp.add_line("");
    temp.add_line("AS A CAT, KITTY IS VERY FAST");
    temp.add_line("AND CAN DASH WHILE IN MIDAIR,");
    temp.add_line("MAKING CROSS OVER STEEPS");
    temp.add_line("VERY EASY. SHE CAN ALSO");
    temp.add_line("FIRE CHARGED SHOTS OF CUTE");
    temp.add_line("ENERGY CATS.");
    page_list.push_back(temp);
    temp.clear();

    /*
    temp.add_line("== XX ==");
    temp.add_line("");
    temp.add_line("");
    page_list.push_back(temp);
    temp.clear();
    */
}
