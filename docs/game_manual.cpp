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
        input.read_input();

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

    temp.add_line_text("== CONTROLING CHARACTER [1] ==");
    temp.add_line_text("MOVE LEFT OR RIGHT:");
    temp.add_line_text("PRESS CONTROL PAD LEFT/RIGHT.");
    temp.add_line_text("");
    temp.add_line_text("CLIMB UP OR DOWN:");
    temp.add_line_text("PRESS CONTROL PAD UP/DOWN.");
    temp.add_line_text("");
    temp.add_line_text("JUMP: PRESS (A) BUTTON. THE LONGER");
    temp.add_line_text("YOU HOLD DOWN BUTTON, THE HIGHER");
    temp.add_line_text("YOU WILL JUMP.");
    temp.add_line_text("");
    temp.add_line_text("FIRE WEAPONS:");
    temp.add_line_text("PRESS (B) BUTTON. PRESS (A) AND (B)");
    temp.add_line_text("TOGETHER TO FIRE WHILE JUMPING.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CONTROLING CHARACTER [2] ==");
    temp.add_line_text("SHIELD: PRESS AND HOLD (Y) BUTTON");
    temp.add_line_text("TO DEFLECT PROJECTILES. [*]");
    temp.add_line_text("");
    temp.add_line_text("SLIDE/DASH: PRESS (X) BUTTON.");
    temp.add_line_text("HOLD BUTTON TO MOVE LONGER. [*]");
    temp.add_line_text("");
    temp.add_line_text("[*] SOME CHARACTERS MAY NOT");
    temp.add_line_text("HAVE THIS ABILITY, SO THE BUTTON");
    temp.add_line_text("WILL NOT EXECUTE ANY ACTION.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== GETTING STARTED ==");
    temp.add_line_text("INSERT ROCKBOT MEGA PACK INTO YOUR");
    temp.add_line_text("ENTERTAINMENT SYSTEM.");
    temp.add_line_text("");
    temp.add_line_text("AT THE TITLE SCREEN, PRESS UP/DOWN");
    temp.add_line_text("TO CHOOSE MODE, THEN PRESS START.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== MODES ==");
    temp.add_line_text("NEW GAME:");
    temp.add_line_text("START A GAME FROM BEGGINING,");
    temp.add_line_text("WILL ERASE PREVIOUS SAVED GAME.");
    temp.add_line_text("");
    temp.add_line_text("LOAD GAME:");
    temp.add_line_text("RESUMES A GAME THAT WAS SAVED.");
    temp.add_line_text("GAME IS SAVED EACH TIME YOU BEAT");
    temp.add_line_text("A BOSS OR LEAVE GAME OR STAGE.");
    temp.add_line_text("");
    temp.add_line_text("CONFIG:");
    temp.add_line_text("CHANGE GAME AND DEVICE SETTINGS.");
    temp.add_line_text("");
    temp.add_line_text("ABOUT:");
    temp.add_line_text("SHOW GAME CREDITS AND INFORMATION.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CONFIG ==");
    temp.add_line_text("IN CONFIG YOU CAN CHANGE SEVERAL");
    temp.add_line_text("SETTINGS FOR THE GAME, SUCH AS");
    temp.add_line_text("AUDIO, INPUT AND VIDEO.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== AUDIO OPTIONS ==");
    temp.add_line_text("YOU CAN SELECT ENABLE/DISABLE TO");
    temp.add_line_text("TURN ON OR OFF THE MUSIC AND");
    temp.add_line_text("SOUND EFFECTS ON THE GAME.");
    temp.add_line_text("");
    temp.add_line_text("YOU CAN ALSO SET VOLUME LEVELS");
    temp.add_line_text("FOR BOTH MUSIC AND SOUND EFFECTS.");
    temp.add_line_text("FOR THAT, ENTER OPTION AND USE");
    temp.add_line_text("LEFT AND RIGHT CONTROLS TO INCREASE");
    temp.add_line_text("OR DECREASE THE VOLUME VALUE.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== INPUT OPTIONS [1] ==");
    temp.add_line_text("CONFIG BUTTONS:");
    temp.add_line_text("IN THIS OPTION YOU CAN SET THE");
    temp.add_line_text("KEY OR BUTTON IN YOUR KEYBOARD,");
    temp.add_line_text("JOYSTICK OR GAME CONTROLLER FOR");
    temp.add_line_text("THE GIVEN GAME ACTION.");
    temp.add_line_text("YOU CAN ALSO RESET THE CONTROLS");
    temp.add_line_text("TO DEFAULT KEYS/BUTTONS HERE.");
    temp.add_line_text("");
    temp.add_line_text("DIRECTIONAL MODE:");
    temp.add_line_text("YOU CAN CHOOSE BETWEEN USING");
    temp.add_line_text("THE DIGITAL KEYPAD OR THE");
    temp.add_line_text("ANALOG STICK FOR CONTROLLERS");
    temp.add_line_text("THAT HAVE BOTH.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== INPUT OPTIONS [2] ==");
    temp.add_line_text("SELECTED JOYSTICK:");
    temp.add_line_text("IF YOUR GAME DEVICE HAS MORE THAN");
    temp.add_line_text("ONE GAMER CONTROLLER CONNECTED,");
    temp.add_line_text("YOU CAN CHOOSE WITCH ONE WILL BE");
    temp.add_line_text("USED TO CONTROL YOUR CHARACTER.");
    temp.add_line_text("");
    temp.add_line_text("TURBO MODE:");
    temp.add_line_text("IF ENABLED, YOU CAN HOLD THE");
    temp.add_line_text("ATTACK BUTTON AND THE CHARACTER");
    temp.add_line_text("WILL KEEP FIRING SHOTS.");
    temp.add_line_text("NOTE: IN THIS MODE YOU CAN'T USE");
    temp.add_line_text("CHARGED WEAPONS.");
    page_list.push_back(temp);
    temp.clear_text();


    temp.add_line_text("== VIDEO OPTIONS ==");
    temp.add_line_text("VIDEO MODE: ON PC PLATFORMS,");
    temp.add_line_text("YOU CAN SELECT BETWEEN MODES");
    temp.add_line_text("UNSCALED, DOUBLE SIZE AND SCALE2X,");
    temp.add_line_text("THAT IS UPSCALED WITH");
    temp.add_line_text("SMOOTHENING FILTER. [*]");
    temp.add_line_text("");
    temp.add_line_text("[*] FOR THIS CHANGE TO TAKE EFFECT,");
    temp.add_line_text("YOU NEED TO RESTART THE GAME.");
    temp.add_line_text("");
    temp.add_line_text("FULLSCREEN:");
    temp.add_line_text("IN DEVICES THAT SUPPORT THIS OPTION,");
    temp.add_line_text("THE VIDEO WILL SWITCH TO USE");
    temp.add_line_text("ENTIRE SCREEN.");
    temp.add_line_text("IN PSP, IT WILL CHANGED BETWEEN");
    temp.add_line_text("KEEP ASPECT RATIO ON OR OFF.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CONFIG [ANDROID] ==");
    temp.add_line_text("WHEN PLAYING ON AN ANDROID");
    temp.add_line_text("GAME DEVICE, YOU CAN USE THE");
    temp.add_line_text("SDL MENU THAT APPEARS BEFORE");
    temp.add_line_text("GAME STARTS, TO CHANGE ON-SCREEN");
    temp.add_line_text("CONTROLS SIZE AND POSITION.");
    temp.add_line_text("");
    temp.add_line_text("PLEASE REFER TO GAME DESCRIPTION");
    temp.add_line_text("IN APP STORE FOR A LINK TO");
    temp.add_line_text("A VIDEO THAT SHOWS HOW TO");
    temp.add_line_text("USE THIS SETTING.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== PLAYING THE GAME [1] ==");
    temp.add_line_text("FIGHT YOUR WAY IN A MAZE FULL OF");
    temp.add_line_text("MAD ROBOTS AND CRAZY MACHINES.");
    temp.add_line_text("YOU WILL HAVE TO DEFEAT EIGHT");
    temp.add_line_text("ROBOT MASTERS THAT USE INCREDIBLE");
    temp.add_line_text("WEAPONS THAT YOU ACQUIRE ONCE");
    temp.add_line_text("THEY ARE DESTROYED.");
    temp.add_line_text("");
    temp.add_line_text("TO ACHIEVE VICTORY YOU CAN");
    temp.add_line_text("RUN, SLIDE, JUMP AND SHOOT");
    temp.add_line_text("IN ORDER TO SAFELY REACH THE");
    temp.add_line_text("END OF EACH STAGE");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== PLAYING THE GAME [2] ==");
    temp.add_line_text("IN THE LEFT SIDE OF THE SCREEN");
    temp.add_line_text("THERE IS HEALTH METER. WHEN IT");
    temp.add_line_text("REACHES ZERO, YOU LOOSE A CHANCE.");
    temp.add_line_text("");
    temp.add_line_text("EVERY TIME YOU ARE HIT BY");
    temp.add_line_text("A PROJECTILE OR TOUCH AN ENEMY,");
    temp.add_line_text("YOUR HELTH DECREASES A BIT.");
    temp.add_line_text("WHEN YOU HIT SPIKES, ALL YOUR");
    temp.add_line_text("HEALTH IS GONE AT ONCE. TAKE CARE!");
    temp.add_line_text("");
    temp.add_line_text("PICK ENERGY OR WEAPON PELLETS");
    temp.add_line_text("AND ENERGY TANKS TO INCREASE");
    temp.add_line_text("YOUR CHANCES TO FINISH STAGE.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== PLAYING THE GAME [3] ==");
    temp.add_line_text("WHEN YOU LOSE A CHANCE, YOU START");
    temp.add_line_text("STAGE AGAIN FROM LAST CHECKPOINT");
    temp.add_line_text("OR STAGE START POINT.");
    temp.add_line_text("");
    temp.add_line_text("IF YOU LOSE ALL YOUR CHANCES,");
    temp.add_line_text("THE GAME ENDS AND YOU HAVE TO");
    temp.add_line_text("PICK AGAIN A STAGE TO START OVER.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== POWER UP [1] ==");
    temp.add_line_text("");
    temp.add_line_text("DURING STAGE OR WHEN DEFEATING");
    temp.add_line_text("AND ENEMY, YOU CAN GET ITEMS THAT");
    temp.add_line_text("HELP YOU TO FINISH THE LEVEL.");
    temp.add_line_text("");

    temp.add_line_text("ENERGY BALL OR PELLET:");
    temp.add_line_text("REFILL PART OF YOUR HEALTH METTER.");

    temp.add_line_text("");
    temp.add_line_text("WEAPON BALL OR PELLET:");
    temp.add_line_text("REFILL A WEAPON ENERGY. YOU NEED");
    temp.add_line_text("TO SELECT A WEAPON BEFORE GETTING");
    temp.add_line_text("THE ITEM FOR IT TO WORK.");

    temp.add_line_text("");
    temp.add_line_text("LIFE (1UP):");
    temp.add_line_text("GIVES YOU AND EXTRA CHANCE TO");
    temp.add_line_text("AVOID GAME OVER.");
    temp.add_line_text("");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== POWER UP [2] ==");
    temp.add_line_text("");
    temp.add_line_text("RED TANK:");
    temp.add_line_text("STORES HEALTH ENERY AND CAN BE");
    temp.add_line_text("USED AT ANY TIME BY ACCESSING");
    temp.add_line_text("THE WEAPONS MENU. FILL COMPLETLY");
    temp.add_line_text("YOUR HEALTH METTER.");

    temp.add_line_text("");
    temp.add_line_text("GREEN TANK:");
    temp.add_line_text("FILL ALL YOUR WEAPONS ENERGY,");
    temp.add_line_text("CAN BE USED IN THE WEAPONS");
    temp.add_line_text("MENU AT ANY TIME.");

    temp.add_line_text("");
    temp.add_line_text("YELLOW TANK:");
    temp.add_line_text("THIS SUPER TANK WILL");
    temp.add_line_text("REFILL BOTH YOUR HEALTH");
    temp.add_line_text("AS ALL YOUR WEAPONS");
    temp.add_line_text("ENERGY!");

    page_list.push_back(temp);
    temp.clear_text();


    temp.add_line_text("== SPECIAL ITEMS ==");
    temp.add_line_text("");
    temp.add_line_text("THERE ARE A FEW SECRET SPOTS");
    temp.add_line_text("WHERE YOU CAN GET SPECIAL ITEMS.");
    temp.add_line_text("");
    temp.add_line_text("THERE ARE THREE ARMOR PIECES,");
    temp.add_line_text("EACH ONE WILL GIVE YOU A NEW");
    temp.add_line_text("ABILITY, LIKE DASHING ON MIDAIR,");
    temp.add_line_text("FIRING SPECIAL WEAPONS OR");
    temp.add_line_text("TAKING HALF DAMAGE.");
    temp.add_line_text("");
    temp.add_line_text("EACH CHARACTER YOU CAN PLAY");
    temp.add_line_text("HAVE DIFFERENT ABILITIES FOR");
    temp.add_line_text("EACH ARMOR PIECE HE GETS.");
    page_list.push_back(temp);
    temp.clear_text();


    temp.add_line_text("== ROBOT MASTERS [1] ==");
    temp.add_line_text("SNOW BOT");
    temp.add_line_text("A COLD BODY AND DISTANT ATTITUDE");
    temp.add_line_text("HIDES A WARM HEART.");

    temp.add_line_text("");
    temp.add_line_text("VALKYRIE BOT");
    temp.add_line_text("FROM NORDIC ORIGINS, THIS GIRL");
    temp.add_line_text("ROBOT CONTROLS THE WEATHER");
    temp.add_line_text("LIKE THE GOD THOR.");

    temp.add_line_text("");
    temp.add_line_text("KURUPIRA BOT");
    temp.add_line_text("PROTECTOR OF THE JUNGLE AND");
    temp.add_line_text("ANIMALS, LOVES THE GOOD LIFE.");

    temp.add_line_text("");
    temp.add_line_text("GEAR BOT");
    temp.add_line_text("MILITARY ROBOT MADE FOR");
    temp.add_line_text("ONLY ONE PURPOSE, TO BRING");
    temp.add_line_text("WAR TO HIS ENEMIES.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== ROBOT MASTERS [2] ==");
    temp.add_line_text("BEAST BOT:");
    temp.add_line_text("TRAVELLING WITH A CIRCUS, HE IS");
    temp.add_line_text("VERY AGRESSIVE AND ANGRY.");

    temp.add_line_text("");
    temp.add_line_text("PHANTOM BOT");
    temp.add_line_text("A LOVER OF THE NIGHT AND OPERA.");

    temp.add_line_text("");
    temp.add_line_text("DRAGON BOT");
    temp.add_line_text("FROM FAR EAST, THIS MASTER");
    temp.add_line_text("HAVE UNKNOWN MYSTICAL POWERS.");

    temp.add_line_text("");
    temp.add_line_text("ZODIAC BOT");
    temp.add_line_text("POWERED BY THE STARS, THIS");
    temp.add_line_text("ROBOT IS PROTECTED BY THE");
    temp.add_line_text("LEGENDARY GOLDEN ARMOR.");
    page_list.push_back(temp);
    temp.clear_text();


    temp.add_line_text("== TIPS FOR WINNING ==");
    temp.add_line_text("");
    temp.add_line_text("IF YOUR CHARACTER CAN SLIDE,");
    temp.add_line_text("YOU CAN PASS UNDER OBSTACLES.");
    temp.add_line_text("");
    temp.add_line_text("WHEN DASHING, PRESS JUMP");
    temp.add_line_text("TO MAKE A REALLY LONG CROSS.");
    temp.add_line_text("");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CHARACTERS [ROCK] ==");
    temp.add_line_text("");
    temp.add_line_text("THE HERO OF THE CITY AFTER");
    temp.add_line_text("SAVING IT FROM DR. DESTRIN,");
    temp.add_line_text("ROCKBOT AS GOOD AS IT GETS,");
    temp.add_line_text("ALWAYS FOLLOWING THE LAWS");
    temp.add_line_text("AND ORDERS FROM GOOD DOCTOR");
    temp.add_line_text("KANOTUS, HIS CREATOR.");
    temp.add_line_text("");
    temp.add_line_text("WITH ROCKBOT YOU CAN FIRE");
    temp.add_line_text("CHARGED SHOTS, SLIDE UNDER");
    temp.add_line_text("OBSTACLES AND USE A SHIELD.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CHARACTERS [CANDY] ==");
    temp.add_line_text("");
    temp.add_line_text("CREATED TO BE AN MERE LAB");
    temp.add_line_text("ASSISTANT FOR DR. KANOTUS,");
    temp.add_line_text("CANDYBOT SAW NO OPTION BUT");
    temp.add_line_text("TO START FIGHTING TOGETHER");
    temp.add_line_text("WITH HER BROTHER, ROCK, WHEN");
    temp.add_line_text("HER OTHER BROTHER, BETA, LEFT.");
    temp.add_line_text("");
    temp.add_line_text("CANDY CAN FIRE SHOTS IN");
    temp.add_line_text("MULTIPLE DIRECTIONS AND");
    temp.add_line_text("JUMP ONE MORE TIME IN MIDAIR.");
    temp.add_line_text("AS NOT BEING MADE FOR FIGHTING,");
    temp.add_line_text("SHE TAKES MORE DAMAGE,");
    temp.add_line_text("CAN'T WALK WHILE SHOOTING");
    temp.add_line_text("NOR DASH OR USE SHIELD.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CHARACTERS [BETA] ==");
    temp.add_line_text("");
    temp.add_line_text("DR. KANOTUS FIRST CREATION,");
    temp.add_line_text("BETA NEVER WANTED TO BE A HERO");
    temp.add_line_text("OR SAVE THE CITY. HIS ONLY WILL");
    temp.add_line_text("IS TO FIGHT STRONG OPPONENTS.");
    temp.add_line_text("HE LEFT ROCK BEHIND WHEN");
    temp.add_line_text("REALIZING HE WOULD NEVER HAD");
    temp.add_line_text("TRUE FREEDOM WHILE LOCKED.");
    temp.add_line_text("IN A SCIENCE LAB.");
    temp.add_line_text("");
    temp.add_line_text("BETA CAN'T CHARGE, BUT HE FIRES");
    temp.add_line_text("TWO PROJECTILES AT ONCE, MAKING");
    temp.add_line_text("EASIER TO HIT ENEMIES.");
    temp.add_line_text("ALSO, HE TAKE LESS DAMAGE");
    temp.add_line_text("THAN OTHER ROBOTS AND CAN");
    temp.add_line_text("DASH TO MAKE LONGER JUMPS.");
    page_list.push_back(temp);
    temp.clear_text();

    temp.add_line_text("== CHARACTERS [KITTY] ==");
    temp.add_line_text("");
    temp.add_line_text("A FREE SPIRIT THAT FOUND BETA");
    temp.add_line_text("IN A MOMENT OF NEED, SHE DECIDED");
    temp.add_line_text("TO HELP HIM TO FIND HIS PLACE");
    temp.add_line_text("IN THE WORLD.");
    temp.add_line_text("LITTLE IS KNOWN ABOUT THIS");
    temp.add_line_text("CAT-ROBOT, NOT EVEN WHO WAS");
    temp.add_line_text("THE ONE THAT BUILT HER.");
    temp.add_line_text("");
    temp.add_line_text("AS A CAT, KITTY IS VERY FAST");
    temp.add_line_text("AND CAN DASH WHILE IN MIDAIR,");
    temp.add_line_text("MAKING CROSS OVER STEEPS");
    temp.add_line_text("VERY EASY. SHE CAN ALSO");
    temp.add_line_text("FIRE CHARGED SHOTS OF CUTE");
    temp.add_line_text("ENERGY CATS.");
    page_list.push_back(temp);
    temp.clear_text();

    /*
    temp.add_line("== XX ==");
    temp.add_line("");
    temp.add_line("");
    page_list.push_back(temp);
    temp.clear();
    */
}
