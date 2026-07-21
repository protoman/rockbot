#include "fio_strings.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../defines.h"
#include "../aux_tools/stringutils.h"
#include "fio_common.h"
#include "shareddata.h"
#include "strings_map.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;


fio_strings::fio_strings()
{
}


namespace format_v4 {

    fio_strings::fio_strings()
    {
        _dialogs_stage_id = -1;


        code_map.insert(std::pair<char,std::string>(0xc9, "É"));
        code_map.insert(std::pair<char,std::string>(0xca, "Ê"));

        code_map.insert(std::pair<char,std::string>(0xc0, "A")); // todo: not working
        code_map.insert(std::pair<char,std::string>(0xc1, "A")); // todo: not working
        code_map.insert(std::pair<char,std::string>(0xc3, "Ã"));
        code_map.insert(std::pair<char,std::string>(0xc2, "Â"));

        code_map.insert(std::pair<char,std::string>(0xcd, "Í"));

        code_map.insert(std::pair<char,std::string>(0xd3, "Ó"));
        code_map.insert(std::pair<char,std::string>(0xd5, "Õ"));
        code_map.insert(std::pair<char,std::string>(0xd4, "Ô"));

        code_map.insert(std::pair<char,std::string>(0xda, "Ú"));
        code_map.insert(std::pair<char,std::string>(0xdc, "Ü"));

        code_map.insert(std::pair<char,std::string>(0xc7, "Ç"));
        /*
        code_map.insert(std::pair<char,std::string>(0xc0, "À"));
        code_map.insert(std::pair<char,std::string>(0xc0, "À"));
        code_map.insert(std::pair<char,std::string>(0xc0, "À"));
        code_map.insert(std::pair<char,std::string>(0xc0, "À"));
        */
    }

    std::string fio_strings::get_ingame_string(int n, int language, bool convert_symbols)
    {
        if (string_list.size() == 0) {
            string_list = load_game_strings(language, convert_symbols);
        }
        if (n < 0 || n >= string_list.size()) {
            return "";
        }

        return string_list.at(n);
    }


    std::vector<std::string> fio_strings::load_game_strings(int language, bool convert_symbols)
    {
        return load_game_strings_from_file(get_game_strings_filename(language), language, convert_symbols);
    }

    std::vector<std::string> fio_strings::load_game_strings_from_file(std::string filename, int language, bool convert_symbols)
    {
        std::vector<std::string> res;
        filename = StringUtils::clean_filename(filename);
        std::ifstream fp(filename.c_str());

        if (!fp.is_open()) {
            std::cout << "ERROR: fio_strings::load_game_strings_from_file[" << filename << "] could not be opened" << std::endl;
            if (filename == get_game_strings_filename(language)) {
                create_default_ingame_strings();
            } else if (filename == get_common_strings_filename(language)) {
                create_default_common_strings();
            }
            fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        }

        std::string str;
        while (getline(fp, str)) {
            if (str.length() > 0) {
                StringUtils::replace_all(str, "\n", "");
                StringUtils::replace_all(str, "\r", "");
                std::string str2 = "\\xC9";
                if (str.find(str2) != -1) {
                    str.replace(str.find(str2), str2.length(),"é");
                }
                StringUtils::replace_all(str, "\\xC9", "é");
            }
            res.push_back(str);
        }


        fp.close();

        if (res.size() == 0) {
            if (filename == get_game_strings_filename(language)) {
                create_default_ingame_strings();
            } else if (filename == get_common_strings_filename(language)) {
                create_default_common_strings();
            }
        } else if (filename == get_game_strings_filename(language) && res.size() < strings_ingame_COUNT) {
            res = add_missing_default_ingame_strings(res, language);
        }

        /*
        // removed because suddnely, it was not needed, don't know why //
        // adding back breaks accented characters
        if (convert_symbols == true) {
            for (unsigned int i=0; i<res.size(); i++) {
                res[i] = convert_text_symbols(res[i]);
            }
        }
        */

        return res;
    }

    void fio_strings::save_game_strings(std::vector<std::string> list, std::string filename)
    {
        std::ofstream fp(filename.c_str());
        if (!fp.is_open()) {
            return;
        }

        for (unsigned int i=0; i<list.size(); i++) {
            // add line break to separate each line
            std::string line = list.at(i);
            // if there are any remaining extra chars, remove it
            StringUtils::replace_all(line, "\n", "");
            StringUtils::replace_all(line, "\r", "");

            line += std::string("\n");
            fp << line.c_str();
        }
        fp.close();
    }

    std::string fio_strings::get_common_strings_filename(int language)
    {
        std::string filename = FILEPATH + "/common_strings_" + get_language_filename_prefix(language) + ".dat";
        filename = StringUtils::clean_filename(filename);
        return filename;
    }

    std::string fio_strings::get_game_strings_filename(int language)
    {
        std::string filename = std::string(GAMEPATH) + "/shared/strings_ingame_" + get_language_filename_prefix(language) + ".dat";
        filename = StringUtils::clean_filename(filename);
        return filename;
    }

    bool format_v4::fio_strings::file_exists(std::string filename)
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }

    std::vector<std::string> fio_strings::get_default_ingame_strings_list(int language)
    {
        char lines[strings_ingame_COUNT][STRINGS_LINE_SIZE];

        if (language == LANGUAGE_PORTUGUESE) {
            snprintf(lines[strings_ingame_newgame], sizeof(lines[strings_ingame_newgame]), "%s", "NOVO JOGO");
            snprintf(lines[strings_ingame_loadgame], sizeof(lines[strings_ingame_loadgame]), "%s", "CARREGAR JOGO");
            snprintf(lines[strings_ingame_config], sizeof(lines[strings_ingame_config]), "%s", "CONFIG");
            snprintf(lines[strings_ingame_about], sizeof(lines[strings_ingame_about]), "%s", "SOBRE");
            snprintf(lines[strings_ingame_extras], sizeof(lines[strings_ingame_extras]), "%s", "EXTRAS");
            snprintf(lines[strings_ingame_password], sizeof(lines[strings_ingame_password]), "%s", "SENHA");
            snprintf(lines[strings_ingame_gameover], sizeof(lines[strings_ingame_gameover]), "%s", "GAME OVER");
            snprintf(lines[strings_ingame_and], sizeof(lines[strings_ingame_and]), "%s", "E");
            snprintf(lines[strings_ingame_yougot_singular], sizeof(lines[strings_ingame_yougot_singular]), "%s", "FOI ADQUIRIDA");
            snprintf(lines[strings_ingame_yougot_plural], sizeof(lines[strings_ingame_yougot_plural]), "%s", "FORAM ADQUIRIDOS");
            snprintf(lines[strings_ingame_selectgame], sizeof(lines[strings_ingame_selectgame]), "%s", "SELECIONE JOGO");
            snprintf(lines[strings_ingame_savegameerror1], sizeof(lines[strings_ingame_savegameerror1]), "%s", "ERRO SALVANDO JOGO,");
            snprintf(lines[strings_ingame_savegameerror2], sizeof(lines[strings_ingame_savegameerror2]), "%s", "VERIFIQUE SE DISPOSITIVO OU");
            snprintf(lines[strings_ingame_savegameerror3], sizeof(lines[strings_ingame_savegameerror3]), "%s", "ARQUIVO ESTÁ PROTEGIDO PARA GRAVACAO.");
            snprintf(lines[strings_ingame_copyrightline], sizeof(lines[strings_ingame_copyrightline]), "%s", "\xA9 2009-2026 UPPERLAND STUDIOS");
            snprintf(lines[strings_ingame_audio], sizeof(lines[strings_ingame_audio]), "%s", "AUDIO");
            snprintf(lines[strings_ingame_input], sizeof(lines[strings_ingame_input]), "%s", "CONTROLE");
            snprintf(lines[strings_ingame_video], sizeof(lines[strings_ingame_video]), "%s", "VIDEO");
            snprintf(lines[strings_ingame_leavestage], sizeof(lines[strings_ingame_leavestage]), "%s", "SAIR DA FASE");
            snprintf(lines[strings_ingame_language], sizeof(lines[strings_ingame_language]), "%s", "IDIOMA");
            snprintf(lines[strings_ingame_video_windowed], sizeof(lines[strings_ingame_video_windowed]), "%s", "JANELA");
            snprintf(lines[strings_ingame_video_fullscreen], sizeof(lines[strings_ingame_video_fullscreen]), "%s", "TELA CHEIA");
            snprintf(lines[strings_ingame_video_noscale], sizeof(lines[strings_ingame_video_noscale]), "%s", "SEM ESCALA  ");
            snprintf(lines[strings_ingame_video_size2x], sizeof(lines[strings_ingame_video_size2x]), "%s", "TAMANHO X2    ");
            snprintf(lines[strings_ingame_video_scale2x], sizeof(lines[strings_ingame_video_scale2x]), "%s", "SCALE2X   ");
            snprintf(lines[strings_ingame_video_show_fps], sizeof(lines[strings_ingame_video_show_fps]), "%s", "MOSTRAR FPS");
            snprintf(lines[strings_ingame_config_restart1], sizeof(lines[strings_ingame_config_restart1]), "%s", "POR FAVOR REINICIE O JOGO");
            snprintf(lines[strings_ingame_config_restart2], sizeof(lines[strings_ingame_config_restart2]), "%s", "PARA NOVA CONFIGURACAO");
            snprintf(lines[strings_ingame_config_restart3], sizeof(lines[strings_ingame_config_restart3]), "%s", "SER ATIVADA.");
            snprintf(lines[strings_ingame_config_presstorestart], sizeof(lines[strings_ingame_config_presstorestart]), "%s", "APERTE UM BOTÃO PARA CONTINUAR");
            snprintf(lines[strings_ingame_pressanykey], sizeof(lines[strings_ingame_pressanykey]), "%s", "APERTE QUALQUER BOTÃO");
            snprintf(lines[strings_ingame_config_set], sizeof(lines[strings_ingame_config_set]), "%s", "SETAR");
            snprintf(lines[strings_ingame_config_key_up], sizeof(lines[strings_ingame_config_key_up]), "%s", "CIMA");
            snprintf(lines[strings_ingame_config_key_down], sizeof(lines[strings_ingame_config_key_down]), "%s", "BAIXO");
            snprintf(lines[strings_ingame_config_key_left], sizeof(lines[strings_ingame_config_key_left]), "%s", "ESQUERDA");
            snprintf(lines[strings_ingame_config_key_right], sizeof(lines[strings_ingame_config_key_right]), "%s", "DIREITA");
            snprintf(lines[strings_ingame_config_key_jump], sizeof(lines[strings_ingame_config_key_jump]), "%s", "PULO");
            snprintf(lines[strings_ingame_config_key_dash], sizeof(lines[strings_ingame_config_key_dash]), "%s", "DASH");
            snprintf(lines[strings_ingame_config_key_shield], sizeof(lines[strings_ingame_config_key_shield]), "%s", "ESCUDO");
            snprintf(lines[strings_ingame_config_key_fire], sizeof(lines[strings_ingame_config_key_fire]), "%s", "TIRO");
            snprintf(lines[strings_ingame_config_key_start], sizeof(lines[strings_ingame_config_key_start]), "%s", "START");
            snprintf(lines[strings_ingame_config_key_keyl], sizeof(lines[strings_ingame_config_key_keyl]), "%s", "BOTAO-L");
            snprintf(lines[strings_ingame_config_key_keyr], sizeof(lines[strings_ingame_config_key_keyr]), "%s", "BOTAO-R");
            snprintf(lines[strings_ingame_config_key_pressnew], sizeof(lines[strings_ingame_config_key_pressnew]), "%s", "APERTE NOVO BOTÃO/TECLA");
            snprintf(lines[strings_ingame_config_key_reset], sizeof(lines[strings_ingame_config_key_reset]), "%s", "RESTAURAR PADRAO");
            snprintf(lines[strings_ingame_config_key_directional], sizeof(lines[strings_ingame_config_key_directional]), "%s", "DIRECTIONAL");
            snprintf(lines[strings_ingame_config_key_directional_analog], sizeof(lines[strings_ingame_config_key_directional_analog]), "%s", "ANALOGICO");
            snprintf(lines[strings_ingame_config_key_directional_digital], sizeof(lines[strings_ingame_config_key_directional_digital]), "%s", "DIGITAL");

            snprintf(lines[strings_ingame_config_android_cloud_save1], sizeof(lines[strings_ingame_config_android_cloud_save1]), "%s", "SALVAR NA NUVEM REQUER");
            snprintf(lines[strings_ingame_config_android_cloud_save2], sizeof(lines[strings_ingame_config_android_cloud_save2]), "%s", "CONEXAO COM A REDE OU");
            snprintf(lines[strings_ingame_config_android_cloud_save3], sizeof(lines[strings_ingame_config_android_cloud_save3]), "%s", "O JOGO CONGELARA QUANDO");
            snprintf(lines[strings_ingame_config_android_cloud_save4], sizeof(lines[strings_ingame_config_android_cloud_save4]), "%s", "OBTER DADOS DO SERVIDOR.");

            snprintf(lines[strings_ingame_config_android_play_services1], sizeof(lines[strings_ingame_config_android_play_services1]), "%s", "HABILITAR ESTA OPÇÃO REQUER");
            snprintf(lines[strings_ingame_config_android_play_services2], sizeof(lines[strings_ingame_config_android_play_services2]), "%s", "CONEXAO DE DADOS.");
            snprintf(lines[strings_ingame_config_android_play_services3], sizeof(lines[strings_ingame_config_android_play_services3]), "%s", "AVISO: O JOGO PODE CONGELAR");
            snprintf(lines[strings_ingame_config_android_play_services4], sizeof(lines[strings_ingame_config_android_play_services4]), "%s", "COM ESTA OPCAO HABILITADA.");

            snprintf(lines[strings_ingame_config_android_hide_controls1], sizeof(lines[strings_ingame_config_android_hide_controls1]), "%s", "REQUER UM CONTROLE FÍSICO,");
            snprintf(lines[strings_ingame_config_android_hide_controls2], sizeof(lines[strings_ingame_config_android_hide_controls2]), "%s", "CASO CONTRÁRIO NÃO CONSEGUIRA");
            snprintf(lines[strings_ingame_config_android_hide_controls3], sizeof(lines[strings_ingame_config_android_hide_controls3]), "%s", "RESTAURAR CONFIGURACOES.");

            snprintf(lines[strings_ingame_config_extras_music_player], sizeof(lines[strings_ingame_config_extras_music_player]), "%s", "PLAYER DE MÚSICA");
            snprintf(lines[strings_ingame_config_extras_sound_player], sizeof(lines[strings_ingame_config_extras_sound_player]), "%s", "PLAYER DE EFEITOS SONOROS");
            snprintf(lines[strings_ingame_config_extras_art_gallery], sizeof(lines[strings_ingame_config_extras_art_gallery]), "%s", "GALERIA DE ARTE");

            snprintf(lines[strings_ingame_passwordinvalid], sizeof(lines[strings_ingame_passwordinvalid]), "%s", "SENHA INVALIDA");
            snprintf(lines[strings_ingame_pressstart], sizeof(lines[strings_ingame_pressstart]), "%s", "APERTE START");
            snprintf(lines[strings_ingame_engineerror], sizeof(lines[strings_ingame_engineerror]), "%s", "ROCKBOT ENGINE ERROR");
            snprintf(lines[strings_ingame_nogames], sizeof(lines[strings_ingame_nogames]), "%s", "NENHUM JOGO DISPONIVEL");
            snprintf(lines[strings_ingame_quitgame], sizeof(lines[strings_ingame_quitgame]), "%s", "SAIR DO JOGO?");
            snprintf(lines[strings_ingame_yes], sizeof(lines[strings_ingame_yes]), "%s", "SIM");
            snprintf(lines[strings_ingame_no], sizeof(lines[strings_ingame_no]), "%s", "NAO");
            snprintf(lines[strings_ingame_life], sizeof(lines[strings_ingame_life]), "%s", "VIDAS");
            snprintf(lines[strings_ingame_item], sizeof(lines[strings_ingame_item]), "%s", "ITEM");
            snprintf(lines[strings_ingame_coin], sizeof(lines[strings_ingame_coin]), "%s", "MOEDAS");
            snprintf(lines[strings_ingame_gotarmor_type_ability], sizeof(lines[strings_ingame_gotarmor_type_ability]), "%s", "CONCEDEM A HABILIDADE PARA");
            snprintf(lines[strings_ingame_gotarmor_type_arms], sizeof(lines[strings_ingame_gotarmor_type_arms]), "%s", "OS BRACOS MELHORADOS");
            snprintf(lines[strings_ingame_gotarmor_type_arms_msg], sizeof(lines[strings_ingame_gotarmor_type_arms_msg]), "%s", "DISPARAR");
            snprintf(lines[strings_ingame_gotarmor_type_legs], sizeof(lines[strings_ingame_gotarmor_type_legs]), "%s", "ESTAS PERNAS MELHORADAS");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg1], sizeof(lines[strings_ingame_gotarmor_type_legs_msg1]), "%s", "DASH NO AR.");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg2], sizeof(lines[strings_ingame_gotarmor_type_legs_msg2]), "%s", "EXECUTAR UM PULO DUPLO");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg3], sizeof(lines[strings_ingame_gotarmor_type_legs_msg3]), "%s", "SHOURUYKEN (CIMA+DASH)");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg4], sizeof(lines[strings_ingame_gotarmor_type_legs_msg4]), "%s", " ");
            snprintf(lines[strings_ingame_gotarmor_type_body], sizeof(lines[strings_ingame_gotarmor_type_body]), "%s", "OS REFORÇOS NA ARMADURA");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg1], sizeof(lines[strings_ingame_gotarmor_type_body_msg1]), "%s", "FICAR INTANGIVEL MAIS TEMPO");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg2], sizeof(lines[strings_ingame_gotarmor_type_body_msg2]), "%s", "TOMAR APENAS METADE DO DANO");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg3], sizeof(lines[strings_ingame_gotarmor_type_body_msg3]), "%s", "NAO SER JOGADO PARA TRAS");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg4], sizeof(lines[strings_ingame_gotarmor_type_body_msg4]), "%s", "RESISTIR ESPINHOS");
            snprintf(lines[strings_ingame_difficulty_select], sizeof(lines[strings_ingame_difficulty_select]), "%s", "ESCOLHA DIFICULDADE:");
            snprintf(lines[strings_ingame_difficulty_easy], sizeof(lines[strings_ingame_difficulty_easy]), "%s", "FACIL");
            snprintf(lines[strings_ingame_difficulty_normal], sizeof(lines[strings_ingame_difficulty_normal]), "%s", "NORMAL");
            snprintf(lines[strings_ingame_difficulty_hard], sizeof(lines[strings_ingame_difficulty_hard]), "%s", "DIFICIL");
            snprintf(lines[strings_ingame_config_input_selected_joystick], sizeof(lines[strings_ingame_config_input_selected_joystick]), "%s", "CONTROLE SELECIONADO");
            snprintf(lines[strings_ingame_config_input_buttons], sizeof(lines[strings_ingame_config_input_buttons]), "%s", "CONFIGURAR BOTOES");
            snprintf(lines[strings_ingame_config_input_turbo_mode], sizeof(lines[strings_ingame_config_input_turbo_mode]), "%s", "MODO TURBO");
            snprintf(lines[strings_ingame_config_input_autocharge_mode], sizeof(lines[strings_ingame_config_input_autocharge_mode]), "%s", "AUTO-CARREGA");
            snprintf(lines[strings_ingame_config_on], sizeof(lines[strings_ingame_config_on]), "%s", "SIM");
            snprintf(lines[strings_ingame_config_off], sizeof(lines[strings_ingame_config_off]), "%s", "NAO");
            snprintf(lines[strings_ingame_config_quitgame], sizeof(lines[strings_ingame_config_quitgame]), "%s", "SAIR DO JOGO");
            snprintf(lines[strings_ingame_armor], sizeof(lines[strings_ingame_armor]), "%s", "ARMADURA");
            snprintf(lines[strings_ingame_config_audio_volume_music], sizeof(lines[strings_ingame_config_audio_volume_music]), "%s", "VOLUME DE MUSICA");
            snprintf(lines[strings_ingame_config_audio_volume_sfx], sizeof(lines[strings_ingame_config_audio_volume_sfx]), "%s", "VOLUME EFEITOS");
            snprintf(lines[strings_ingame_config_enabled], sizeof(lines[strings_ingame_config_enabled]), "%s", "HABILITADO");
            snprintf(lines[strings_ingame_config_disabled], sizeof(lines[strings_ingame_config_disabled]), "%s", "DESABILITADO");
            snprintf(lines[strings_ingame_ready_message], sizeof(lines[strings_ingame_ready_message]), "%s", "MISSÃO INICIADA!");
            snprintf(lines[strings_ingame_config_select_player], sizeof(lines[strings_ingame_config_select_player]), "%s", "< SELECIONE PERSONAGEM >");
            snprintf(lines[strings_ingame_config_press_start_to_select], sizeof(lines[strings_ingame_config_press_start_to_select]), "%s", "APERTE START PARA INICIAR");
            snprintf(lines[strings_ingame_manual], sizeof(lines[strings_ingame_manual]), "%s", "MANUAL");
            snprintf(lines[strings_ingame_mode], sizeof(lines[strings_ingame_mode]), "%s", "MODO");
            snprintf(lines[strings_ingame_video_scale_mode], sizeof(lines[strings_ingame_video_scale_mode]), "%s", "MODO ESCALA DE TELA");
            snprintf(lines[strings_config_android_hidescreencontrols], sizeof(lines[strings_config_android_hidescreencontrols]), "%s", "ESCONDER CONTROLES");
            snprintf(lines[strings_config_android_screencontrolssize], sizeof(lines[strings_config_android_screencontrolssize]), "%s", "TAMANHO CONTROLES");
            snprintf(lines[strings_config_android_useplayservices], sizeof(lines[strings_config_android_useplayservices]), "%s", "PLAY SERVICES");
            snprintf(lines[strings_config_android_usecloudsave], sizeof(lines[strings_config_android_usecloudsave]), "%s", "SALVAR NA NUVEM");
            snprintf(lines[strings_config_wii_joysticktype], sizeof(lines[strings_config_wii_joysticktype]), "%s", "TIPO DE CONTROLE");
            snprintf(lines[strings_config_android_screencontrolssize_SMALL], sizeof(lines[strings_config_android_screencontrolssize_SMALL]), "%s", "PEQUENO");
            snprintf(lines[strings_config_android_screencontrolssize_MEDIUM], sizeof(lines[strings_config_android_screencontrolssize_MEDIUM]), "%s", "MEDIUM");
            snprintf(lines[strings_config_android_screencontrolssize_BIG], sizeof(lines[strings_config_android_screencontrolssize_BIG]), "%s", "GRANDE");
            snprintf(lines[strings_config_wii_joysticktype_WIIMOTE], sizeof(lines[strings_config_wii_joysticktype_WIIMOTE]), "%s", "WIIMOTE");
            snprintf(lines[strings_config_wii_joysticktype_CLASSIC], sizeof(lines[strings_config_wii_joysticktype_CLASSIC]), "%s", "CLASSIC");
            snprintf(lines[strings_config_wii_joysticktype_GAMECUBE], sizeof(lines[strings_config_wii_joysticktype_GAMECUBE]), "%s", "GAMECUBE");
            snprintf(lines[strings_config_wii_platformspecific], sizeof(lines[strings_config_wii_platformspecific]), "%s", "ESPECIFICO PLATAFORMA");
            snprintf(lines[strings_config_keys_unet], sizeof(lines[strings_config_keys_unet]), "%s", "POR FAVOR DEFINA");
            snprintf(lines[strings_ingame_config_graphics_performance], sizeof(lines[strings_ingame_config_graphics_performance]), "%s", "PERFORMANCE");
            snprintf(lines[strings_ingame_enable_playservices_dialog], sizeof(lines[strings_ingame_enable_playservices_dialog]), "%s", "HABILITAR GOOGLE PLAY SERVICES?");
            snprintf(lines[strings_ingame_enable_cloudsave_dialog], sizeof(lines[strings_ingame_enable_cloudsave_dialog]), "%s", "HAVILITAR SAVE NA NUVEM?");
            snprintf(lines[strings_ingame_requires_network], sizeof(lines[strings_ingame_requires_network]), "%s", "(REQUER REDE DE DADOS ATIVA)");
            snprintf(lines[strings_config_low], sizeof(lines[strings_config_low]), "%s", "BAIXO");
            snprintf(lines[strings_config_medium], sizeof(lines[strings_config_medium]), "%s", "MEDIO");
            snprintf(lines[strings_config_high], sizeof(lines[strings_config_high]), "%s", "ALTO");
            snprintf(lines[string_stage_select_stage], sizeof(lines[string_stage_select_stage]), "%s", "FASE:");
            snprintf(lines[string_stage_select_lair_of], sizeof(lines[string_stage_select_lair_of]), "%s", "LAR DE:");
            snprintf(lines[string_stage_select_enter_stage], sizeof(lines[string_stage_select_enter_stage]), "%s", "[ENTRAR MISSÃO]");
            snprintf(lines[string_stage_select_unavailable], sizeof(lines[string_stage_select_unavailable]), "%s", "[INDISPONÍVEL NA DEMO]");
            snprintf(lines[strings_config_return], sizeof(lines[strings_config_return]), "%s", "VOLTAR");

            snprintf(lines[string_intro_upperland_studios], sizeof(lines[string_intro_upperland_studios]), "%s", "TIME DE JOGOS CASEIROS");
            snprintf(lines[string_intro_presents], sizeof(lines[string_intro_presents]), "%s", "APRESENTA");

            snprintf(lines[string_intro_engine1], sizeof(lines[string_intro_engine1]), "%s", "-- ROCKBOT ENGINE --");
            snprintf(lines[string_intro_engine2], sizeof(lines[string_intro_engine2]), "%s", "ESTE JOGO USA CÓDIGO E ARTE");
            snprintf(lines[string_intro_engine3], sizeof(lines[string_intro_engine3]), "%s", "ORIGINAIS, CRIADOS DO ZERO");
            snprintf(lines[string_intro_engine4], sizeof(lines[string_intro_engine4]), "%s", "QUALQUER SEMELHANÇA É MERA");
            snprintf(lines[string_intro_engine5], sizeof(lines[string_intro_engine5]), "%s", "COINCIDÊNCIA OU HOMENAGEM");
            snprintf(lines[string_intro_engine6], sizeof(lines[string_intro_engine6]), "%s", "À JOGOS ANTIGOS OU ENGANO.");
            snprintf(lines[string_intro_engine7], sizeof(lines[string_intro_engine7]), "%s", "ROCKBOT E UPPERLAND NÃO TEM");
            snprintf(lines[string_intro_engine8], sizeof(lines[string_intro_engine8]), "%s", "QUALQUER RELAÇÃO COM OUTRAS EMPRESAS.");
            snprintf(lines[string_intro_engine9], sizeof(lines[string_intro_engine9]), "%s", "ROCKBOT FOI REMOVIDO PELO GOOGLE");
            snprintf(lines[string_intro_engine10], sizeof(lines[string_intro_engine10]), "%s", "USUÁRIOS RELATANDO ELE COMO");
            snprintf(lines[string_intro_engine11], sizeof(lines[string_intro_engine11]), "%s", "SENDO VERSÃO 'FALSA' DE OUTRO.");

            snprintf(lines[string_intro_demo_warning_title], sizeof(lines[string_intro_demo_warning_title]), "%s", "-- AVISO JOGO CASEIRO"
                                                                  " --");
            snprintf(lines[string_intro_demo_warning1], sizeof(lines[string_intro_demo_warning1]), "%s", "ESTE JOGO FOI FEITO EM CASA, POR");
            snprintf(lines[string_intro_demo_warning2], sizeof(lines[string_intro_demo_warning2]), "%s", "POUCAS PESSOAS, SEM GANHOS MONETÁRIOS");
            snprintf(lines[string_intro_demo_warning3], sizeof(lines[string_intro_demo_warning3]), "%s", "E DURANTE O TEMPO LIVRE DO EMPREGO.");
            snprintf(lines[string_intro_demo_warning4], sizeof(lines[string_intro_demo_warning4]), "%s", "FAZEMOS JOGOS PARA VOCÊ SE DIVERTIR");
            snprintf(lines[string_intro_demo_warning5], sizeof(lines[string_intro_demo_warning5]), "%s", "AINDA ASSIM, ALGUNS NOS ODEIAM.");
            snprintf(lines[string_intro_demo_warning6], sizeof(lines[string_intro_demo_warning6]), "%s", "NÃO SEJA TROLL. NÃO REPORTE");
            snprintf(lines[string_intro_demo_warning7], sizeof(lines[string_intro_demo_warning7]), "%s", "SÓ PORQUE ACHA PARECIDO.");
            snprintf(lines[string_intro_demo_warning8], sizeof(lines[string_intro_demo_warning8]), "%s", "LEIA A FAQ NO NOSSO SITE ANTES!!");
            snprintf(lines[string_intro_demo_warning9], sizeof(lines[string_intro_demo_warning9]), "%s", "VERSÃO ROCKBOT PODE SER OBTIDA EM:");
            snprintf(lines[string_intro_demo_warning10], sizeof(lines[string_intro_demo_warning10]), "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            snprintf(lines[string_intro_demo_warning11], sizeof(lines[string_intro_demo_warning11]), "%s", "");
            snprintf(lines[string_press_key_or_button], sizeof(lines[string_press_key_or_button]), "%s", "");
            snprintf(lines[STRING_ENDING_CONCEPT], sizeof(lines[STRING_ENDING_CONCEPT]), "%s", "CONCEITO");
            snprintf(lines[STRING_ENDING_DESIGN], sizeof(lines[STRING_ENDING_DESIGN]), "%s", "DESENHO");

            snprintf(lines[strings_config_android_screencontrolsspacing], sizeof(lines[strings_config_android_screencontrolsspacing]), "%s", "ESPAÇO BOTÕES");
            snprintf(lines[strings_config_android_audiobuffersize], sizeof(lines[strings_config_android_audiobuffersize]), "%s", "TAMANHO BUFFER ÁUDIO");
            snprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], sizeof(lines[strings_config_android_audiobuffersize_VERY_SMALL]), "%s", "MUITO PEQUENO");
            snprintf(lines[strings_config_android_audiobuffersize_SMALL], sizeof(lines[strings_config_android_audiobuffersize_SMALL]), "%s", "PEQUENO");
            snprintf(lines[strings_config_android_audiobuffersize_MEDIUM], sizeof(lines[strings_config_android_audiobuffersize_MEDIUM]), "%s", "MÉDIO");
            snprintf(lines[strings_config_android_audiobuffersize_BIG], sizeof(lines[strings_config_android_audiobuffersize_BIG]), "%s", "GRANDE");
            snprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], sizeof(lines[strings_config_android_audiobuffersize_DESCRIPTION]), "%s", "AUMENTE VALOR SE ÁUDIO ESTIVER CORTADO");

            snprintf(lines[strings_menu_item_CHEATS], sizeof(lines[strings_menu_item_CHEATS]), "%s", "TRAPAÇAS");
            snprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], sizeof(lines[strings_menu_item_CHEATS_INVENCIBLE]), "%s", "INVENCÍVEL");
            snprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], sizeof(lines[strings_menu_item_CHEATS_ALLBEATEN]), "%s", "TUDO DERROTADO");
            snprintf(lines[strings_menu_item_CHEATS_CHARACTER], sizeof(lines[strings_menu_item_CHEATS_CHARACTER]), "%s", "PERSONAGEM");

            snprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], sizeof(lines[strings_menu_pick_save_slot_PICK_OPTION]), "%s", "ESCOLHA UM ESPAÇO DE SAVE");
            snprintf(lines[strings_menu_pick_save_slot_NEW_GAME], sizeof(lines[strings_menu_pick_save_slot_NEW_GAME]), "%s", "CRIAR NOVO JOGO");
            snprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], sizeof(lines[strings_menu_pick_save_slot_LOAD_GAME]), "%s", "CARREGAR ARQUIVO");
            snprintf(lines[strings_menu_pick_save_slot_NO_SAVE], sizeof(lines[strings_menu_pick_save_slot_NO_SAVE]), "%s", "- SEM ARQUIVO -");

            snprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], sizeof(lines[strings_ending_NEW_CHARACTERS_AVAILABLE]), "%s", "NOVOS PERSONAGENS DISPONPIVEIS");
            snprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], sizeof(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE]), "%s", "USE      PARA CONTINUAR");

            snprintf(lines[strings_weapon_name_COIL], sizeof(lines[strings_weapon_name_COIL]), "%s", "SAPO-MOLA");
            snprintf(lines[strings_weapon_name_JET], sizeof(lines[strings_weapon_name_JET]), "%s", "ÁGUIA-JATO");
            snprintf(lines[strings_weapon_name_ETANK], sizeof(lines[strings_weapon_name_ETANK]), "%s", "CRISTAL HP");
            snprintf(lines[strings_weapon_name_WTANK], sizeof(lines[strings_weapon_name_WTANK]), "%s", "CRISTAL MP");
            snprintf(lines[strings_weapon_name_STANK], sizeof(lines[strings_weapon_name_STANK]), "%s", "CRISTAL ESPECIAL");

            snprintf(lines[strings_weapon_menu_CHANGE_WEAPON], sizeof(lines[strings_weapon_menu_CHANGE_WEAPON]), "%s", "MUDAR ARMA/ITEM");
            snprintf(lines[strings_weapon_menu_TANKS], sizeof(lines[strings_weapon_menu_TANKS]), "%s", "CRISTAIS DE ENERGIA");
            snprintf(lines[strings_weapon_menu_BOOTS], sizeof(lines[strings_weapon_menu_BOOTS]), "%s", "BOTAS");
            snprintf(lines[strings_weapon_menu_ARMOR], sizeof(lines[strings_weapon_menu_ARMOR]), "%s", "ARMADURA");
            snprintf(lines[strings_weapon_menu_WEAPON], sizeof(lines[strings_weapon_menu_WEAPON]), "%s", "TIRO");
            snprintf(lines[strings_weapon_menu_PART_NORMAL], sizeof(lines[strings_weapon_menu_PART_NORMAL]), "%s", "NORMAL");
            snprintf(lines[strings_weapon_menu_PART_ENHANCED], sizeof(lines[strings_weapon_menu_PART_ENHANCED]), "%s", "MELHORADO");

            snprintf(lines[strings_ingame_config_audio_use_old_music], sizeof(lines[strings_ingame_config_audio_use_old_music]), "%s", "USAR MÚSICA ANTIGA");

            snprintf(lines[strings_about_site], sizeof(lines[strings_about_site]), "%s", "- SITE -");
            snprintf(lines[strings_about_bugs], sizeof(lines[strings_about_bugs]), "%s", "- SUPORTE -");
            snprintf(lines[strings_about_twitter], sizeof(lines[strings_about_twitter]), "%s", "- TWITTER -");
            snprintf(lines[strings_about_facebook], sizeof(lines[strings_about_facebook]), "%s", "- FACEBOOK -");

            snprintf(lines[strings_stage_select_pick_mission], sizeof(lines[strings_stage_select_pick_mission]), "%s", "ESCOLHA UMA MISSÃO");
            snprintf(lines[strings_stage_select_boss], sizeof(lines[strings_stage_select_boss]), "%s", "CHEFE:");
            snprintf(lines[strings_stage_select_select], sizeof(lines[strings_stage_select_select]), "%s", "SELECIONAR");

            snprintf(lines[strings_config_android_use_mod_music], sizeof(lines[strings_config_android_use_mod_music]), "%s", "USAR MÚSICA CHIPTUNE");
            snprintf(lines[strings_config_android_use_mod_music_warning], sizeof(lines[strings_config_android_use_mod_music_warning]), "%s", "PODE CAUSAR PROBLEMAS DE ÁUDIO");

            snprintf(lines[strings_weapon_selected], sizeof(lines[strings_weapon_selected]), "%s", "ARMA");

            snprintf(lines[strings_game_engine_credits_title], sizeof(lines[strings_game_engine_credits_title]), "%s", "CRÉDITOS DO MOTOR DE JOGO");
            snprintf(lines[strings_game_engine_credits_planning], sizeof(lines[strings_game_engine_credits_planning]), "%s", "PLANEJAMENTO");
            snprintf(lines[strings_game_engine_credits_design], sizeof(lines[strings_game_engine_credits_design]), "%s", "DESIGN");
            snprintf(lines[strings_game_engine_credits_dialogs], sizeof(lines[strings_game_engine_credits_dialogs]), "%s", "DIÁLOGOS");
            snprintf(lines[strings_game_engine_credits_tests], sizeof(lines[strings_game_engine_credits_tests]), "%s", "TESTES AND REVISÃO");
            snprintf(lines[strings_game_engine_credits_illustration], sizeof(lines[strings_game_engine_credits_illustration]), "%s", "ILUSTRAÇÕES");
            snprintf(lines[strings_game_engine_credits_music], sizeof(lines[strings_game_engine_credits_music]), "%s", "MÚSICA");
            snprintf(lines[strings_game_engine_credits_translation], sizeof(lines[strings_game_engine_credits_translation]), "%s", "TRADUÇÃO");
            snprintf(lines[strings_game_engine_credits_programming], sizeof(lines[strings_game_engine_credits_programming]), "%s", "PROGRAMAÇÃO");
            snprintf(lines[strings_game_engine_credits_ports], sizeof(lines[strings_game_engine_credits_ports]), "%s", "PLATAFORMAS");
            snprintf(lines[strings_game_engine_credits_special_thanks], sizeof(lines[strings_game_engine_credits_special_thanks]), "%s", "AGRADECIMENTOS ESPECIAIS");
            snprintf(lines[strings_game_engine_credits_dev_tools], sizeof(lines[strings_game_engine_credits_dev_tools]), "%s", "FERRAMENTAS DE DESENVOLVIMENTO");
            snprintf(lines[strings_game_engine_credits_presented_by], sizeof(lines[strings_game_engine_credits_presented_by]), "%s", "APRESENTADO POR");


        } else if (language == LANGUAGE_SPANISH) {
            snprintf(lines[strings_ingame_newgame], sizeof(lines[strings_ingame_newgame]), "%s", "NUEVA PARTIDA");
            snprintf(lines[strings_ingame_loadgame], sizeof(lines[strings_ingame_loadgame]), "%s", "CARGAR PARTIDA");
            snprintf(lines[strings_ingame_config], sizeof(lines[strings_ingame_config]), "%s", "CONFIGURACIÓN");
            snprintf(lines[strings_ingame_about], sizeof(lines[strings_ingame_about]), "%s", "INFORMACIÓN");
            snprintf(lines[strings_ingame_extras], sizeof(lines[strings_ingame_extras]), "%s", "EXTRAS");
            snprintf(lines[strings_ingame_password], sizeof(lines[strings_ingame_password]), "%s", "CONTRASEÑA");
            snprintf(lines[strings_ingame_gameover], sizeof(lines[strings_ingame_gameover]), "%s", "PARTIDA ACABADA");
            snprintf(lines[strings_ingame_and], sizeof(lines[strings_ingame_and]), "%s", "Y");
            snprintf(lines[strings_ingame_yougot_singular], sizeof(lines[strings_ingame_yougot_singular]), "%s", "HA SIDO ADQUIRIDO");
            snprintf(lines[strings_ingame_yougot_plural], sizeof(lines[strings_ingame_yougot_plural]), "%s", "HA SIDO ADQUIRIDO");
            snprintf(lines[strings_ingame_selectgame], sizeof(lines[strings_ingame_selectgame]), "%s", "SELECCIONE PARTIDA");
            snprintf(lines[strings_ingame_savegameerror1], sizeof(lines[strings_ingame_savegameerror1]), "%s", "ERROR AL GUARDAR PARTIDA,");
            snprintf(lines[strings_ingame_savegameerror2], sizeof(lines[strings_ingame_savegameerror2]), "%s", "EVISE QUE EL DISPOSITIVO O");
            snprintf(lines[strings_ingame_savegameerror3], sizeof(lines[strings_ingame_savegameerror3]), "%s", "ARCHIVO NO ESTE PROTEGIDO.");
            snprintf(lines[strings_ingame_copyrightline], sizeof(lines[strings_ingame_copyrightline]), "%s", "\xA9 2009-2026 UPPERLAND STUDIOS");
            snprintf(lines[strings_ingame_audio], sizeof(lines[strings_ingame_audio]), "%s", "AUDIO");
            snprintf(lines[strings_ingame_input], sizeof(lines[strings_ingame_input]), "%s", "INPUT");
            snprintf(lines[strings_ingame_video], sizeof(lines[strings_ingame_video]), "%s", "VIDEO");
            snprintf(lines[strings_ingame_leavestage], sizeof(lines[strings_ingame_leavestage]), "%s", "DEJAR NIVEL");
            snprintf(lines[strings_ingame_language], sizeof(lines[strings_ingame_language]), "%s", "LENGUAJE");
            snprintf(lines[strings_ingame_video_windowed], sizeof(lines[strings_ingame_video_windowed]), "%s", "EN VENTANA");
            snprintf(lines[strings_ingame_video_fullscreen], sizeof(lines[strings_ingame_video_fullscreen]), "%s", "PANTALLA COMPLETA");
            snprintf(lines[strings_ingame_video_noscale], sizeof(lines[strings_ingame_video_noscale]), "%s", "NO ESCALAR  ");
            snprintf(lines[strings_ingame_video_size2x], sizeof(lines[strings_ingame_video_size2x]), "%s", "TAMAÑO X2    ");
            snprintf(lines[strings_ingame_video_scale2x], sizeof(lines[strings_ingame_video_scale2x]), "%s", "ESCALA X2   ");
            snprintf(lines[strings_ingame_video_show_fps], sizeof(lines[strings_ingame_video_show_fps]), "%s", "ENSEÑAR FPS");
            snprintf(lines[strings_ingame_config_restart1], sizeof(lines[strings_ingame_config_restart1]), "%s", "OR FAVOR REINICIE EL JUEGO");
            snprintf(lines[strings_ingame_config_restart2], sizeof(lines[strings_ingame_config_restart2]), "%s", "PARA QUE LA CONFIGURACIÓN");
            snprintf(lines[strings_ingame_config_restart3], sizeof(lines[strings_ingame_config_restart3]), "%s", "TOME EFECTO.");
            snprintf(lines[strings_ingame_config_presstorestart], sizeof(lines[strings_ingame_config_presstorestart]), "%s", "PRESIONE UN BOTÓN PARA REGRESAR");
            snprintf(lines[strings_ingame_pressanykey], sizeof(lines[strings_ingame_pressanykey]), "%s", "PRESIONE CUALQUIER BOTÓN");
            snprintf(lines[strings_ingame_config_set], sizeof(lines[strings_ingame_config_set]), "%s", "DEFINIR");
            snprintf(lines[strings_ingame_config_key_up], sizeof(lines[strings_ingame_config_key_up]), "%s", "ARRIBA");
            snprintf(lines[strings_ingame_config_key_down], sizeof(lines[strings_ingame_config_key_down]), "%s", "ABAJO");
            snprintf(lines[strings_ingame_config_key_left], sizeof(lines[strings_ingame_config_key_left]), "%s", "IZQUIERDA");
            snprintf(lines[strings_ingame_config_key_right], sizeof(lines[strings_ingame_config_key_right]), "%s", "DERECHA");
            snprintf(lines[strings_ingame_config_key_jump], sizeof(lines[strings_ingame_config_key_jump]), "%s", "SALTAR");
            snprintf(lines[strings_ingame_config_key_dash], sizeof(lines[strings_ingame_config_key_dash]), "%s", "DASH");
            snprintf(lines[strings_ingame_config_key_shield], sizeof(lines[strings_ingame_config_key_shield]), "%s", "ESCUDO");
            snprintf(lines[strings_ingame_config_key_fire], sizeof(lines[strings_ingame_config_key_fire]), "%s", "DISPARAR");
            snprintf(lines[strings_ingame_config_key_start], sizeof(lines[strings_ingame_config_key_start]), "%s", "START");
            snprintf(lines[strings_ingame_config_key_keyl], sizeof(lines[strings_ingame_config_key_keyl]), "%s", "BOTÓN-L");
            snprintf(lines[strings_ingame_config_key_keyr], sizeof(lines[strings_ingame_config_key_keyr]), "%s", "BOTÓN-R");
            snprintf(lines[strings_ingame_config_key_pressnew], sizeof(lines[strings_ingame_config_key_pressnew]), "%s", "PRESIONE UNA NUEVA TECLA/BOTÓN");
            snprintf(lines[strings_ingame_config_key_reset], sizeof(lines[strings_ingame_config_key_reset]), "%s", "REESTABLECER");
            snprintf(lines[strings_ingame_config_key_directional], sizeof(lines[strings_ingame_config_key_directional]), "%s", "DIRECCIONAL");
            snprintf(lines[strings_ingame_config_key_directional_analog], sizeof(lines[strings_ingame_config_key_directional_analog]), "%s", "ANÁLOGO");
            snprintf(lines[strings_ingame_config_key_directional_digital], sizeof(lines[strings_ingame_config_key_directional_digital]), "%s", "DIGITAL");

            snprintf(lines[strings_ingame_config_android_cloud_save1], sizeof(lines[strings_ingame_config_android_cloud_save1]), "%s", "EL GUARDADO EN LA NUBE REQUIERE");
            snprintf(lines[strings_ingame_config_android_cloud_save2], sizeof(lines[strings_ingame_config_android_cloud_save2]), "%s", "CONEXIÓN A INTERNET O EL JUEGO SE");
            snprintf(lines[strings_ingame_config_android_cloud_save3], sizeof(lines[strings_ingame_config_android_cloud_save3]), "%s", "CONGELARÁ CUANDO TRATE DE OBTENER");
            snprintf(lines[strings_ingame_config_android_cloud_save4], sizeof(lines[strings_ingame_config_android_cloud_save4]), "%s", "DATOS DEL SERVIDOR.");

            snprintf(lines[strings_ingame_config_android_play_services1], sizeof(lines[strings_ingame_config_android_play_services1]), "%s", "ACTIVAR ESTA OPCIÓN REQUIERE");
            snprintf(lines[strings_ingame_config_android_play_services2], sizeof(lines[strings_ingame_config_android_play_services2]), "%s", "CONEXIÓN A DATOS.");
            snprintf(lines[strings_ingame_config_android_play_services3], sizeof(lines[strings_ingame_config_android_play_services3]), "%s", "ADVERTENCIA: EL JUEGO PODRÍA");
            snprintf(lines[strings_ingame_config_android_play_services4], sizeof(lines[strings_ingame_config_android_play_services4]), "%s", "CONGELARSE CON ESTA OPCIÓN ACTIVADA.");

            snprintf(lines[strings_ingame_config_android_hide_controls1], sizeof(lines[strings_ingame_config_android_hide_controls1]), "%s", "REQUIERES DE UN MANDO FÍSICO,");
            snprintf(lines[strings_ingame_config_android_hide_controls2], sizeof(lines[strings_ingame_config_android_hide_controls2]), "%s", "O NO PODRÁS RESTABLECER LA");
            snprintf(lines[strings_ingame_config_android_hide_controls3], sizeof(lines[strings_ingame_config_android_hide_controls3]), "%s", "CONFIGURACIÓN.");

            snprintf(lines[strings_ingame_config_extras_music_player], sizeof(lines[strings_ingame_config_extras_music_player]), "%s", "REPRODUCTOR DE MÚSICA");
            snprintf(lines[strings_ingame_config_extras_sound_player], sizeof(lines[strings_ingame_config_extras_sound_player]), "%s", "REPRODUCTOR DE SFX");
            snprintf(lines[strings_ingame_config_extras_art_gallery], sizeof(lines[strings_ingame_config_extras_art_gallery]), "%s", "GALLERÍA DE ARTE");

            snprintf(lines[strings_ingame_passwordinvalid], sizeof(lines[strings_ingame_passwordinvalid]), "%s", "CONTRASEÑA INVALIDA");
            snprintf(lines[strings_ingame_pressstart], sizeof(lines[strings_ingame_pressstart]), "%s", "PRESIONE START");
            snprintf(lines[strings_ingame_engineerror], sizeof(lines[strings_ingame_engineerror]), "%s", "ERROR EN ROCKBOT ENGINE");
            snprintf(lines[strings_ingame_nogames], sizeof(lines[strings_ingame_nogames]), "%s", "NO HAY JUEGOS DISPONIBLES");
            snprintf(lines[strings_ingame_quitgame], sizeof(lines[strings_ingame_quitgame]), "%s", "¿SALIR DEL JUEGO?");
            snprintf(lines[strings_ingame_yes], sizeof(lines[strings_ingame_yes]), "%s", "SI");
            snprintf(lines[strings_ingame_no], sizeof(lines[strings_ingame_no]), "%s", "NO");
            snprintf(lines[strings_ingame_life], sizeof(lines[strings_ingame_life]), "%s", "VIDA(S)");
            snprintf(lines[strings_ingame_item], sizeof(lines[strings_ingame_item]), "%s", "OBJETO(S)");
            snprintf(lines[strings_ingame_coin], sizeof(lines[strings_ingame_coin]), "%s", "MONEDA(S)");
            snprintf(lines[strings_ingame_gotarmor_type_ability], sizeof(lines[strings_ingame_gotarmor_type_ability]), "%s", "TE DA LA HABILIDAD DE");
            snprintf(lines[strings_ingame_gotarmor_type_arms], sizeof(lines[strings_ingame_gotarmor_type_arms]), "%s", "ESTOS BRAZOS MEJORADOS TE PERMITEN");
            snprintf(lines[strings_ingame_gotarmor_type_arms_msg], sizeof(lines[strings_ingame_gotarmor_type_arms_msg]), "%s", "DISPARAR");
            snprintf(lines[strings_ingame_gotarmor_type_legs], sizeof(lines[strings_ingame_gotarmor_type_legs]), "%s", "ESTAS PIERNAS LIGERAS TE PERMITEN");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg1], sizeof(lines[strings_ingame_gotarmor_type_legs_msg1]), "%s", "HACER UN DASH EN EL AIRE.");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg2], sizeof(lines[strings_ingame_gotarmor_type_legs_msg2]), "%s", "HACER UN SALTO DOBLE");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg3], sizeof(lines[strings_ingame_gotarmor_type_legs_msg3]), "%s", "HACER UN SHOURYUKEN (ARRIBA+DASH)");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg4], sizeof(lines[strings_ingame_gotarmor_type_legs_msg4]), "%s", " ");
            snprintf(lines[strings_ingame_gotarmor_type_body], sizeof(lines[strings_ingame_gotarmor_type_body]), "%s", "ESTE CUERPO FORTIFICADO TE PERMITE");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg1], sizeof(lines[strings_ingame_gotarmor_type_body_msg1]), "%s", "SER INTANGIBLE POR MÁS TIEMPO");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg2], sizeof(lines[strings_ingame_gotarmor_type_body_msg2]), "%s", "TOMAR LA MITAD DEL DAÑO");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg3], sizeof(lines[strings_ingame_gotarmor_type_body_msg3]), "%s", "EVITAR EMPUJONES AL SER DAÑADO");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg4], sizeof(lines[strings_ingame_gotarmor_type_body_msg4]), "%s", "RESISTIR ESPINAS");
            snprintf(lines[strings_ingame_difficulty_select], sizeof(lines[strings_ingame_difficulty_select]), "%s", "SELECCIONE UNA DIFICULTAD:");
            snprintf(lines[strings_ingame_difficulty_easy], sizeof(lines[strings_ingame_difficulty_easy]), "%s", "FÁCIL");
            snprintf(lines[strings_ingame_difficulty_normal], sizeof(lines[strings_ingame_difficulty_normal]), "%s", "NORMAL");
            snprintf(lines[strings_ingame_difficulty_hard], sizeof(lines[strings_ingame_difficulty_hard]), "%s", "DIFÍCIL");
            snprintf(lines[strings_ingame_config_input_selected_joystick], sizeof(lines[strings_ingame_config_input_selected_joystick]), "%s", "JOYSTICK SELECCIONADO");
            snprintf(lines[strings_ingame_config_input_buttons], sizeof(lines[strings_ingame_config_input_buttons]), "%s", "CONFIGURAR BOTONES");
            snprintf(lines[strings_ingame_config_input_turbo_mode], sizeof(lines[strings_ingame_config_input_turbo_mode]), "%s", "MODO TURBO");
            snprintf(lines[strings_ingame_config_input_autocharge_mode], sizeof(lines[strings_ingame_config_input_autocharge_mode]), "%s", "AUTO-CARGA");
            snprintf(lines[strings_ingame_config_on], sizeof(lines[strings_ingame_config_on]), "%s", "SI");
            snprintf(lines[strings_ingame_config_off], sizeof(lines[strings_ingame_config_off]), "%s", "NO");
            snprintf(lines[strings_ingame_config_quitgame], sizeof(lines[strings_ingame_config_quitgame]), "%s", "SALIR DEL JUEGO");
            snprintf(lines[strings_ingame_armor], sizeof(lines[strings_ingame_armor]), "%s", "ARMADURA");
            snprintf(lines[strings_ingame_config_audio_volume_music], sizeof(lines[strings_ingame_config_audio_volume_music]), "%s", "VOLUMEN DE MÚSICA");
            snprintf(lines[strings_ingame_config_audio_volume_sfx], sizeof(lines[strings_ingame_config_audio_volume_sfx]), "%s", "VOLUMEN DE SFX");
            snprintf(lines[strings_ingame_config_enabled], sizeof(lines[strings_ingame_config_enabled]), "%s", "HABILITADO");
            snprintf(lines[strings_ingame_config_disabled], sizeof(lines[strings_ingame_config_disabled]), "%s", "DESHABILITADO");
            snprintf(lines[strings_ingame_ready_message], sizeof(lines[strings_ingame_ready_message]), "%s", "¡MISIÓN INICIADA!");
            snprintf(lines[strings_ingame_config_select_player], sizeof(lines[strings_ingame_config_select_player]), "%s", "< SELECCIONE UN PERSONAJE >");
            snprintf(lines[strings_ingame_config_press_start_to_select], sizeof(lines[strings_ingame_config_press_start_to_select]), "%s", "PRESIONE START PARA SELECCIONAR");
            snprintf(lines[strings_ingame_manual], sizeof(lines[strings_ingame_manual]), "%s", "MANUAL");
            snprintf(lines[strings_ingame_mode], sizeof(lines[strings_ingame_mode]), "%s", "MODO");
            snprintf(lines[strings_ingame_video_scale_mode], sizeof(lines[strings_ingame_video_scale_mode]), "%s", "MODO DE ESCALA");
            snprintf(lines[strings_config_android_hidescreencontrols], sizeof(lines[strings_config_android_hidescreencontrols]), "%s", "OCULTAR CONTROLES");
            snprintf(lines[strings_config_android_screencontrolssize], sizeof(lines[strings_config_android_screencontrolssize]), "%s", "TAMAÑO DE CONTROLES");
            snprintf(lines[strings_config_android_useplayservices], sizeof(lines[strings_config_android_useplayservices]), "%s", "SERVICIOS DE GOOGLE PLAY");
            snprintf(lines[strings_config_android_usecloudsave], sizeof(lines[strings_config_android_usecloudsave]), "%s", "GUARDADO EN LA NUBE");
            snprintf(lines[strings_config_wii_joysticktype], sizeof(lines[strings_config_wii_joysticktype]), "%s", "TIPO DE JOYSTICK");
            snprintf(lines[strings_config_android_screencontrolssize_SMALL], sizeof(lines[strings_config_android_screencontrolssize_SMALL]), "%s", "PEQUEÑO");
            snprintf(lines[strings_config_android_screencontrolssize_MEDIUM], sizeof(lines[strings_config_android_screencontrolssize_MEDIUM]), "%s", "MEDIANO");
            snprintf(lines[strings_config_android_screencontrolssize_BIG], sizeof(lines[strings_config_android_screencontrolssize_BIG]), "%s", "GRANDE");
            snprintf(lines[strings_config_wii_joysticktype_WIIMOTE], sizeof(lines[strings_config_wii_joysticktype_WIIMOTE]), "%s", "WIIMOTE");
            snprintf(lines[strings_config_wii_joysticktype_CLASSIC], sizeof(lines[strings_config_wii_joysticktype_CLASSIC]), "%s", "CLASSIC");
            snprintf(lines[strings_config_wii_joysticktype_GAMECUBE], sizeof(lines[strings_config_wii_joysticktype_GAMECUBE]), "%s", "GAMECUBE");
            snprintf(lines[strings_config_wii_platformspecific], sizeof(lines[strings_config_wii_platformspecific]), "%s", "ESPECIFICO DE PLATAFORMA");
            snprintf(lines[strings_config_keys_unet], sizeof(lines[strings_config_keys_unet]), "%s", "POR FAVOR DEFINA");
            snprintf(lines[strings_ingame_config_graphics_performance], sizeof(lines[strings_ingame_config_graphics_performance]), "%s", "RENDIMIENTO");
            snprintf(lines[strings_ingame_enable_playservices_dialog], sizeof(lines[strings_ingame_enable_playservices_dialog]), "%s", "¿HABILITAR SERVICIOS DE GOOGLE PLAY?");
            snprintf(lines[strings_ingame_enable_cloudsave_dialog], sizeof(lines[strings_ingame_enable_cloudsave_dialog]), "%s", "¿HABILITAR GUARDADO EN LA NUBE?");
            snprintf(lines[strings_ingame_requires_network], sizeof(lines[strings_ingame_requires_network]), "%s", "(SE REQUIERE UN CONEXIÓN DISPONIBLE)");
            snprintf(lines[strings_config_low], sizeof(lines[strings_config_low]), "%s", "BAJO");
            snprintf(lines[strings_config_medium], sizeof(lines[strings_config_medium]), "%s", "MEDIO");
            snprintf(lines[strings_config_high], sizeof(lines[strings_config_high]), "%s", "ALTO");
            snprintf(lines[string_stage_select_stage], sizeof(lines[string_stage_select_stage]), "%s", "NIVEL:");
            snprintf(lines[string_stage_select_lair_of], sizeof(lines[string_stage_select_lair_of]), "%s", "GUARIDA DE:");
            snprintf(lines[string_stage_select_enter_stage], sizeof(lines[string_stage_select_enter_stage]), "%s", "[SELECCIONAR]");
            snprintf(lines[string_stage_select_unavailable], sizeof(lines[string_stage_select_unavailable]), "%s", "[NO DISPONIBLE EN LA DEMO]");
            snprintf(lines[strings_config_return], sizeof(lines[strings_config_return]), "%s", "REGRESAR");

            snprintf(lines[string_intro_upperland_studios], sizeof(lines[string_intro_upperland_studios]), "%s", "HOMEBREW GAMES TEAM");
            snprintf(lines[string_intro_presents], sizeof(lines[string_intro_presents]), "%s", "PRESENTA");

            snprintf(lines[string_intro_engine1], sizeof(lines[string_intro_engine1]), "%s", "-- ROCKBOT ENGINE --");
            snprintf(lines[string_intro_engine2], sizeof(lines[string_intro_engine2]), "%s", "ESTE JUEGO UTILIZA CÓDIGO ORIGINAL");
            snprintf(lines[string_intro_engine3], sizeof(lines[string_intro_engine3]), "%s", "Y ASSETS HECHOS DESDE CERO.");
            snprintf(lines[string_intro_engine4], sizeof(lines[string_intro_engine4]), "%s", "CUALQUIER SIMILITUDES SON");
            snprintf(lines[string_intro_engine5], sizeof(lines[string_intro_engine5]), "%s", "TRIBUTOS A JUEGOS ANTIGUOS");
            snprintf(lines[string_intro_engine6], sizeof(lines[string_intro_engine6]), "%s", "O UN MALENTENDIDO.");

            snprintf(lines[string_intro_engine7], sizeof(lines[string_intro_engine7]), "%s", "ROCKBOT Y UPPERLAND NO ESTÁN RELACIONADOS");
            snprintf(lines[string_intro_engine8], sizeof(lines[string_intro_engine8]), "%s", "A NINGUNA OTRA COMPAÑÍA O JUEGO.");
            snprintf(lines[string_intro_engine9], sizeof(lines[string_intro_engine9]), "%s", "GOOGLE REMOVIÓ ROCKBOT DEBIDO");
            snprintf(lines[string_intro_engine10], sizeof(lines[string_intro_engine10]), "%s", "A USUARIOS REPORTÁNDOLO POR");
            snprintf(lines[string_intro_engine11], sizeof(lines[string_intro_engine11]), "%s", "‘COPIAR’ A OTRO JUEGO.");

            snprintf(lines[string_intro_demo_warning_title], sizeof(lines[string_intro_demo_warning_title]), "%s", "-- ADVERTENCIA HOMEBREW"
                                                                  " --");
            snprintf(lines[string_intro_demo_warning1], sizeof(lines[string_intro_demo_warning1]), "%s", "ESTO ES UN HOMEBREW, HECHO POR");
            snprintf(lines[string_intro_demo_warning2], sizeof(lines[string_intro_demo_warning2]), "%s", "POCAS PERSONAS SIN NINGUNA GANANCIA");
            snprintf(lines[string_intro_demo_warning3], sizeof(lines[string_intro_demo_warning3]), "%s", "MONETARIA Y DURANTE NUESTRO TIEMPO");
            snprintf(lines[string_intro_demo_warning4], sizeof(lines[string_intro_demo_warning4]), "%s", "LIBRE DEL TRABAJO. HACEMOS JUEGOS PARA");
            snprintf(lines[string_intro_demo_warning5], sizeof(lines[string_intro_demo_warning5]), "%s", "QUÉ SE DIVIERTAN.");
            snprintf(lines[string_intro_demo_warning6], sizeof(lines[string_intro_demo_warning6]), "%s", "NÃO SEJA TROLL. NÃO REPORTE");
            snprintf(lines[string_intro_demo_warning7], sizeof(lines[string_intro_demo_warning7]), "%s", "SÓ PORQUE ACHA PARECIDO.");
            snprintf(lines[string_intro_demo_warning8], sizeof(lines[string_intro_demo_warning8]), "%s", "POR FAVOR, LEA EL F.A.Q.!!");
            snprintf(lines[string_intro_demo_warning9], sizeof(lines[string_intro_demo_warning9]), "%s", "VERSIONES ANTIGUAS PUEDEN SER OBTENIDAS EN:");
            snprintf(lines[string_intro_demo_warning10], sizeof(lines[string_intro_demo_warning10]), "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            snprintf(lines[string_intro_demo_warning11], sizeof(lines[string_intro_demo_warning11]), "%s", "");
            snprintf(lines[string_press_key_or_button], sizeof(lines[string_press_key_or_button]), "%s", "");
            snprintf(lines[STRING_ENDING_CONCEPT], sizeof(lines[STRING_ENDING_CONCEPT]), "%s", "CONCEPTO");
            snprintf(lines[STRING_ENDING_DESIGN], sizeof(lines[STRING_ENDING_DESIGN]), "%s", "DISEÑO");

            snprintf(lines[strings_config_android_screencontrolsspacing], sizeof(lines[strings_config_android_screencontrolsspacing]), "%s", "ESPACIADO ENTRE BOTONES");
            snprintf(lines[strings_config_android_audiobuffersize], sizeof(lines[strings_config_android_audiobuffersize]), "%s", "TAMAÑO DEL BÚFER DE AUDIO");
            snprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], sizeof(lines[strings_config_android_audiobuffersize_VERY_SMALL]), "%s", "MUY PEQUEÑO");
            snprintf(lines[strings_config_android_audiobuffersize_SMALL], sizeof(lines[strings_config_android_audiobuffersize_SMALL]), "%s", "PEQUEÑO");
            snprintf(lines[strings_config_android_audiobuffersize_MEDIUM], sizeof(lines[strings_config_android_audiobuffersize_MEDIUM]), "%s", "MEDIANO");
            snprintf(lines[strings_config_android_audiobuffersize_BIG], sizeof(lines[strings_config_android_audiobuffersize_BIG]), "%s", "GRANDE");
            snprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], sizeof(lines[strings_config_android_audiobuffersize_DESCRIPTION]), "%s", "INCREMENTE EL VALOR SI SE ESCUCHA CORTADO");

            snprintf(lines[strings_menu_item_CHEATS], sizeof(lines[strings_menu_item_CHEATS]), "%s", "TRUCOS");
            snprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], sizeof(lines[strings_menu_item_CHEATS_INVENCIBLE]), "%s", "INVENCIBILIDAD");
            snprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], sizeof(lines[strings_menu_item_CHEATS_ALLBEATEN]), "%s", "TODOS LOS NIVELES COMPLETADOS");
            snprintf(lines[strings_menu_item_CHEATS_CHARACTER], sizeof(lines[strings_menu_item_CHEATS_CHARACTER]), "%s", "PERSONAJE");

            snprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], sizeof(lines[strings_menu_pick_save_slot_PICK_OPTION]), "%s", "POR FAVOR, SELECCIONE UNA CASILLA DE GUARDADO");
            snprintf(lines[strings_menu_pick_save_slot_NEW_GAME], sizeof(lines[strings_menu_pick_save_slot_NEW_GAME]), "%s", "CREAR UNA NUEVA PARTIDA");
            snprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], sizeof(lines[strings_menu_pick_save_slot_LOAD_GAME]), "%s", "CARGAR UN ARCHIVO DE GUARDADO");
            snprintf(lines[strings_menu_pick_save_slot_NO_SAVE], sizeof(lines[strings_menu_pick_save_slot_NO_SAVE]), "%s", "– SIN PARTIDA GUARDADA –");

            snprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], sizeof(lines[strings_ending_NEW_CHARACTERS_AVAILABLE]), "%s", "NUEVOS PERSONAJES DESBLOQUEADOS");
            snprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], sizeof(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE]), "%s", "USA      PARA CONTINUAR");

            snprintf(lines[strings_weapon_name_COIL], sizeof(lines[strings_weapon_name_COIL]), "%s", "FROG COIL");
            snprintf(lines[strings_weapon_name_JET], sizeof(lines[strings_weapon_name_JET]), "%s", "EAGLE JET");
            snprintf(lines[strings_weapon_name_ETANK], sizeof(lines[strings_weapon_name_ETANK]), "%s", "HP CRYSTAL");
            snprintf(lines[strings_weapon_name_WTANK], sizeof(lines[strings_weapon_name_WTANK]), "%s", "MP CRYSTAL");
            snprintf(lines[strings_weapon_name_STANK], sizeof(lines[strings_weapon_name_STANK]), "%s", "SPECIAL CRYSTAL");

            snprintf(lines[strings_weapon_menu_CHANGE_WEAPON], sizeof(lines[strings_weapon_menu_CHANGE_WEAPON]), "%s", "CAMBIAR ARMA/OBJETO");
            snprintf(lines[strings_weapon_menu_TANKS], sizeof(lines[strings_weapon_menu_TANKS]), "%s", "HEALTH CRYSTALS");
            snprintf(lines[strings_weapon_menu_BOOTS], sizeof(lines[strings_weapon_menu_BOOTS]), "%s", "BOTAS");
            snprintf(lines[strings_weapon_menu_ARMOR], sizeof(lines[strings_weapon_menu_ARMOR]), "%s", "ARMADURA");
            snprintf(lines[strings_weapon_menu_WEAPON], sizeof(lines[strings_weapon_menu_WEAPON]), "%s", "RAYO");
            snprintf(lines[strings_weapon_menu_PART_NORMAL], sizeof(lines[strings_weapon_menu_PART_NORMAL]), "%s", "NORMAL");
            snprintf(lines[strings_weapon_menu_PART_ENHANCED], sizeof(lines[strings_weapon_menu_PART_ENHANCED]), "%s", "MEJORADO");

            snprintf(lines[strings_ingame_config_audio_use_old_music], sizeof(lines[strings_ingame_config_audio_use_old_music]), "%s", "USAR MÚSICA ANTIGUA");

            snprintf(lines[strings_about_site], sizeof(lines[strings_about_site]), "%s", "- SITIO -");
            snprintf(lines[strings_about_bugs], sizeof(lines[strings_about_bugs]), "%s", "- AYUDA -");
            snprintf(lines[strings_about_twitter], sizeof(lines[strings_about_twitter]), "%s", "- TWITTER -");
            snprintf(lines[strings_about_facebook], sizeof(lines[strings_about_facebook]), "%s", "- FACEBOOK -");

            snprintf(lines[strings_stage_select_pick_mission], sizeof(lines[strings_stage_select_pick_mission]), "%s", "SELECCIONE UNA MISIÓN");
            snprintf(lines[strings_stage_select_boss], sizeof(lines[strings_stage_select_boss]), "%s", "JEFE:");
            snprintf(lines[strings_stage_select_select], sizeof(lines[strings_stage_select_select]), "%s", "SELECCIONE");

            snprintf(lines[strings_config_android_use_mod_music], sizeof(lines[strings_config_android_use_mod_music]), "%s", "USAR MÚSICA CHIPTUNE");
            snprintf(lines[strings_config_android_use_mod_music_warning], sizeof(lines[strings_config_android_use_mod_music_warning]), "%s", "PUEDE CAUSAR PROBLEMAS DE AUDIO");

            snprintf(lines[strings_weapon_selected], sizeof(lines[strings_weapon_selected]), "%s", "ARMA");

            snprintf(lines[strings_game_engine_credits_title], sizeof(lines[strings_game_engine_credits_title]), "%s", "CRÉDITOS DO MOTOR DE JOGO");
            snprintf(lines[strings_game_engine_credits_planning], sizeof(lines[strings_game_engine_credits_planning]), "%s", "PLANEJAMENTO");
            snprintf(lines[strings_game_engine_credits_design], sizeof(lines[strings_game_engine_credits_design]), "%s", "DESIGN");
            snprintf(lines[strings_game_engine_credits_dialogs], sizeof(lines[strings_game_engine_credits_dialogs]), "%s", "DIÁLOGOS");
            snprintf(lines[strings_game_engine_credits_tests], sizeof(lines[strings_game_engine_credits_tests]), "%s", "TESTES AND REVISÃO");
            snprintf(lines[strings_game_engine_credits_illustration], sizeof(lines[strings_game_engine_credits_illustration]), "%s", "ILUSTRAÇÕES");
            snprintf(lines[strings_game_engine_credits_music], sizeof(lines[strings_game_engine_credits_music]), "%s", "MÚSICA");
            snprintf(lines[strings_game_engine_credits_translation], sizeof(lines[strings_game_engine_credits_translation]), "%s", "TRADUÇÃO");
            snprintf(lines[strings_game_engine_credits_programming], sizeof(lines[strings_game_engine_credits_programming]), "%s", "PROGRAMAÇÃO");
            snprintf(lines[strings_game_engine_credits_ports], sizeof(lines[strings_game_engine_credits_ports]), "%s", "PLATAFORMAS");
            snprintf(lines[strings_game_engine_credits_special_thanks], sizeof(lines[strings_game_engine_credits_special_thanks]), "%s", "AGRADECIMENTOS ESPECIAIS");
            snprintf(lines[strings_game_engine_credits_dev_tools], sizeof(lines[strings_game_engine_credits_dev_tools]), "%s", "FERRAMENTAS DE DESENVOLVIMENTO");
            snprintf(lines[strings_game_engine_credits_presented_by], sizeof(lines[strings_game_engine_credits_presented_by]), "%s", "APRESENTADO POR");

        } else {
            snprintf(lines[strings_ingame_newgame], sizeof(lines[strings_ingame_newgame]), "%s", "NEW GAME");
            snprintf(lines[strings_ingame_loadgame], sizeof(lines[strings_ingame_loadgame]), "%s", "LOAD GAME");
            snprintf(lines[strings_ingame_config], sizeof(lines[strings_ingame_config]), "%s", "CONFIG");
            snprintf(lines[strings_ingame_about], sizeof(lines[strings_ingame_about]), "%s", "ABOUT");
            snprintf(lines[strings_ingame_extras], sizeof(lines[strings_ingame_extras]), "%s", "EXTRAS");
            snprintf(lines[strings_ingame_password], sizeof(lines[strings_ingame_password]), "%s", "PASSWORD");
            snprintf(lines[strings_ingame_gameover], sizeof(lines[strings_ingame_gameover]), "%s", "GAME OVER");
            snprintf(lines[strings_ingame_and], sizeof(lines[strings_ingame_and]), "%s", "AND");
            snprintf(lines[strings_ingame_yougot_singular], sizeof(lines[strings_ingame_yougot_singular]), "%s", "HAS BEEN ACQUIRED");
            snprintf(lines[strings_ingame_yougot_plural], sizeof(lines[strings_ingame_yougot_plural]), "%s", "HAVE BEEN ACQUIRED");
            snprintf(lines[strings_ingame_selectgame], sizeof(lines[strings_ingame_selectgame]), "%s", "SELECT GAME");
            snprintf(lines[strings_ingame_savegameerror1], sizeof(lines[strings_ingame_savegameerror1]), "%s", "ERROR WHILE SAVING GAME,");
            snprintf(lines[strings_ingame_savegameerror2], sizeof(lines[strings_ingame_savegameerror2]), "%s", "PLEASE CHECK THAT THE DEVICE OR");
            snprintf(lines[strings_ingame_savegameerror3], sizeof(lines[strings_ingame_savegameerror3]), "%s", "FILE IS NOT WRITE-PROTECTED.");
            snprintf(lines[strings_ingame_copyrightline], sizeof(lines[strings_ingame_copyrightline]), "%s", "\xA9 2009-2026 UPPERLAND STUDIOS");
            snprintf(lines[strings_ingame_audio], sizeof(lines[strings_ingame_audio]), "%s", "AUDIO");
            snprintf(lines[strings_ingame_input], sizeof(lines[strings_ingame_input]), "%s", "INPUT");
            snprintf(lines[strings_ingame_video], sizeof(lines[strings_ingame_video]), "%s", "VIDEO");
            snprintf(lines[strings_ingame_leavestage], sizeof(lines[strings_ingame_leavestage]), "%s", "LEAVE STAGE");
            snprintf(lines[strings_ingame_language], sizeof(lines[strings_ingame_language]), "%s", "LANGUAGE");
            snprintf(lines[strings_ingame_video_windowed], sizeof(lines[strings_ingame_video_windowed]), "%s", "WINDOWED");
            snprintf(lines[strings_ingame_video_fullscreen], sizeof(lines[strings_ingame_video_fullscreen]), "%s", "FULLSCREEN");
            snprintf(lines[strings_ingame_video_noscale], sizeof(lines[strings_ingame_video_noscale]), "%s", "NO SCALE  ");
            snprintf(lines[strings_ingame_video_size2x], sizeof(lines[strings_ingame_video_size2x]), "%s", "SIZE2X    ");
            snprintf(lines[strings_ingame_video_scale2x], sizeof(lines[strings_ingame_video_scale2x]), "%s", "SCALE2X   ");
            snprintf(lines[strings_ingame_video_show_fps], sizeof(lines[strings_ingame_video_show_fps]), "%s", "SHOW FPS");
            snprintf(lines[strings_ingame_config_restart1], sizeof(lines[strings_ingame_config_restart1]), "%s", "PLEASE RESTART THE GAME");
            snprintf(lines[strings_ingame_config_restart2], sizeof(lines[strings_ingame_config_restart2]), "%s", "FOR THE CONFIGURATION");
            snprintf(lines[strings_ingame_config_restart3], sizeof(lines[strings_ingame_config_restart3]), "%s", "TO TAKE EFFECT");
            snprintf(lines[strings_ingame_config_presstorestart], sizeof(lines[strings_ingame_config_presstorestart]), "%s", "PRESS A BUTTON TO RETURN");
            snprintf(lines[strings_ingame_pressanykey], sizeof(lines[strings_ingame_pressanykey]), "%s", "PRESS ANY KEY OR BUTTON");
            snprintf(lines[strings_ingame_config_set], sizeof(lines[strings_ingame_config_set]), "%s", "SET");
            snprintf(lines[strings_ingame_config_key_up], sizeof(lines[strings_ingame_config_key_up]), "%s", "UP");
            snprintf(lines[strings_ingame_config_key_down], sizeof(lines[strings_ingame_config_key_down]), "%s", "DOWN");
            snprintf(lines[strings_ingame_config_key_left], sizeof(lines[strings_ingame_config_key_left]), "%s", "LEFT");
            snprintf(lines[strings_ingame_config_key_right], sizeof(lines[strings_ingame_config_key_right]), "%s", "RIGHT");
            snprintf(lines[strings_ingame_config_key_jump], sizeof(lines[strings_ingame_config_key_jump]), "%s", "JUMP");
            snprintf(lines[strings_ingame_config_key_dash], sizeof(lines[strings_ingame_config_key_dash]), "%s", "DASH");
            snprintf(lines[strings_ingame_config_key_shield], sizeof(lines[strings_ingame_config_key_shield]), "%s", "SHIELD");
            snprintf(lines[strings_ingame_config_key_fire], sizeof(lines[strings_ingame_config_key_fire]), "%s", "FIRE");
            snprintf(lines[strings_ingame_config_key_start], sizeof(lines[strings_ingame_config_key_start]), "%s", "START");
            snprintf(lines[strings_ingame_config_key_keyl], sizeof(lines[strings_ingame_config_key_keyl]), "%s", "BUTTON-L");
            snprintf(lines[strings_ingame_config_key_keyr], sizeof(lines[strings_ingame_config_key_keyr]), "%s", "BUTTON-R");
            snprintf(lines[strings_ingame_config_key_pressnew], sizeof(lines[strings_ingame_config_key_pressnew]), "%s", "PRESS NEW KEY/BUTTON");
            snprintf(lines[strings_ingame_config_key_reset], sizeof(lines[strings_ingame_config_key_reset]), "%s", "RESET TO DEFAULT");
            snprintf(lines[strings_ingame_config_key_directional], sizeof(lines[strings_ingame_config_key_directional]), "%s", "DIRECTIONAL");
            snprintf(lines[strings_ingame_config_key_directional_analog], sizeof(lines[strings_ingame_config_key_directional_analog]), "%s", "ANALOG");
            snprintf(lines[strings_ingame_config_key_directional_digital], sizeof(lines[strings_ingame_config_key_directional_digital]), "%s", "DIGITAL");
            snprintf(lines[strings_ingame_config_android_cloud_save1], sizeof(lines[strings_ingame_config_android_cloud_save1]), "%s", "CLOUD SAVE REQUIRES");
            snprintf(lines[strings_ingame_config_android_cloud_save2], sizeof(lines[strings_ingame_config_android_cloud_save2]), "%s", "A NETWORK CONNECTION");
            snprintf(lines[strings_ingame_config_android_cloud_save3], sizeof(lines[strings_ingame_config_android_cloud_save3]), "%s", "OR GAME WILL FREEZE WHEN");
            snprintf(lines[strings_ingame_config_android_cloud_save4], sizeof(lines[strings_ingame_config_android_cloud_save4]), "%s", "GETTING SERVER DATA.");
            snprintf(lines[strings_ingame_config_android_play_services1], sizeof(lines[strings_ingame_config_android_play_services1]), "%s", "ENABLING THIS OPTION REQUIRES");
            snprintf(lines[strings_ingame_config_android_play_services2], sizeof(lines[strings_ingame_config_android_play_services2]), "%s", "A DATA CONNECTION.");
            snprintf(lines[strings_ingame_config_android_play_services3], sizeof(lines[strings_ingame_config_android_play_services3]), "%s", "WARNING: THE GAME MAY FREEZE");
            snprintf(lines[strings_ingame_config_android_play_services4], sizeof(lines[strings_ingame_config_android_play_services4]), "%s", "WITH THIS OPTION ENABLED.");


            snprintf(lines[strings_ingame_config_android_hide_controls1], sizeof(lines[strings_ingame_config_android_hide_controls1]), "%s", "REQUIRES PHYSICAL CONTROLLER.");
            snprintf(lines[strings_ingame_config_android_hide_controls2], sizeof(lines[strings_ingame_config_android_hide_controls2]), "%s", "OTHERWISE YOU WON'T BE");
            snprintf(lines[strings_ingame_config_android_hide_controls3], sizeof(lines[strings_ingame_config_android_hide_controls3]), "%s", "ABLE TO RESTORE SETTING.");

            snprintf(lines[strings_ingame_config_extras_music_player], sizeof(lines[strings_ingame_config_extras_music_player]), "%s", "MUSIC PLAYER");
            snprintf(lines[strings_ingame_config_extras_sound_player], sizeof(lines[strings_ingame_config_extras_sound_player]), "%s", "SFX PLAYER");
            snprintf(lines[strings_ingame_config_extras_art_gallery], sizeof(lines[strings_ingame_config_extras_art_gallery]), "%s", "ART GALLERY");


            snprintf(lines[strings_ingame_passwordinvalid], sizeof(lines[strings_ingame_passwordinvalid]), "%s", "INVALID PASSWORD");
            snprintf(lines[strings_ingame_pressstart], sizeof(lines[strings_ingame_pressstart]), "%s", "PRESS START");
            snprintf(lines[strings_ingame_engineerror], sizeof(lines[strings_ingame_engineerror]), "%s", "ROCKBOT ENGINE ERROR");
            snprintf(lines[strings_ingame_nogames], sizeof(lines[strings_ingame_nogames]), "%s", "NO GAMES AVAILABLE");
            snprintf(lines[strings_ingame_quitgame], sizeof(lines[strings_ingame_quitgame]), "%s", "QUIT GAME?");
            snprintf(lines[strings_ingame_yes], sizeof(lines[strings_ingame_yes]), "%s", "YES");
            snprintf(lines[strings_ingame_no], sizeof(lines[strings_ingame_no]), "%s", "NO");
            snprintf(lines[strings_ingame_life], sizeof(lines[strings_ingame_life]), "%s", "LIFE");
            snprintf(lines[strings_ingame_item], sizeof(lines[strings_ingame_item]), "%s", "ITEM");
            snprintf(lines[strings_ingame_coin], sizeof(lines[strings_ingame_coin]), "%s", "COIN");
            snprintf(lines[strings_ingame_gotarmor_type_ability], sizeof(lines[strings_ingame_gotarmor_type_ability]), "%s", "GIVE YOU THE ABILITY TO");
            snprintf(lines[strings_ingame_gotarmor_type_arms], sizeof(lines[strings_ingame_gotarmor_type_arms]), "%s", "IMPROVED ARMS ALLOWS YOU TO");
            snprintf(lines[strings_ingame_gotarmor_type_arms_msg], sizeof(lines[strings_ingame_gotarmor_type_arms_msg]), "%s", "SHOOT");
            snprintf(lines[strings_ingame_gotarmor_type_legs], sizeof(lines[strings_ingame_gotarmor_type_legs]), "%s", "THOSE LIGHTER LEGS");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg1], sizeof(lines[strings_ingame_gotarmor_type_legs_msg1]), "%s", "DASH IN MIDDLE-AIR.");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg2], sizeof(lines[strings_ingame_gotarmor_type_legs_msg2]), "%s", "EXECUTE DOUBLE JUMP");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg3], sizeof(lines[strings_ingame_gotarmor_type_legs_msg3]), "%s", "SHOURUYKEN (UP+DASH)");
            snprintf(lines[strings_ingame_gotarmor_type_legs_msg4], sizeof(lines[strings_ingame_gotarmor_type_legs_msg4]), "%s", " ");
            snprintf(lines[strings_ingame_gotarmor_type_body], sizeof(lines[strings_ingame_gotarmor_type_body]), "%s", "THIS FORTIFIED BODY WILL");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg1], sizeof(lines[strings_ingame_gotarmor_type_body_msg1]), "%s", "BE INTANGIBLE MORE TIME");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg2], sizeof(lines[strings_ingame_gotarmor_type_body_msg2]), "%s", "TAKE HALF DAMAGE");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg3], sizeof(lines[strings_ingame_gotarmor_type_body_msg3]), "%s", "AVOID PUSH-BACK ON HIT");
            snprintf(lines[strings_ingame_gotarmor_type_body_msg4], sizeof(lines[strings_ingame_gotarmor_type_body_msg4]), "%s", "RESIST SPIKES");
            snprintf(lines[strings_ingame_difficulty_select], sizeof(lines[strings_ingame_difficulty_select]), "%s", "SELECT DIFFICULTY:");
            snprintf(lines[strings_ingame_difficulty_easy], sizeof(lines[strings_ingame_difficulty_easy]), "%s", "EASY");
            snprintf(lines[strings_ingame_difficulty_normal], sizeof(lines[strings_ingame_difficulty_normal]), "%s", "NORMAL");
            snprintf(lines[strings_ingame_difficulty_hard], sizeof(lines[strings_ingame_difficulty_hard]), "%s", "HARD");
            snprintf(lines[strings_ingame_config_input_selected_joystick], sizeof(lines[strings_ingame_config_input_selected_joystick]), "%s", "SELECTED JOYSTICK");
            snprintf(lines[strings_ingame_config_input_buttons], sizeof(lines[strings_ingame_config_input_buttons]), "%s", "CONFIG BUTTONS");
            snprintf(lines[strings_ingame_config_input_turbo_mode], sizeof(lines[strings_ingame_config_input_turbo_mode]), "%s", "TURBO MODE");
            snprintf(lines[strings_ingame_config_input_autocharge_mode], sizeof(lines[strings_ingame_config_input_autocharge_mode]), "%s", "AUTO-CHARGED");
            snprintf(lines[strings_ingame_config_on], sizeof(lines[strings_ingame_config_on]), "%s", "ON");
            snprintf(lines[strings_ingame_config_off], sizeof(lines[strings_ingame_config_off]), "%s", "OFF");
            snprintf(lines[strings_ingame_config_quitgame], sizeof(lines[strings_ingame_config_quitgame]), "%s", "QUIT GAME");
            snprintf(lines[strings_ingame_armor], sizeof(lines[strings_ingame_armor]), "%s", "ARMOR");
            snprintf(lines[strings_ingame_config_audio_volume_music], sizeof(lines[strings_ingame_config_audio_volume_music]), "%s", "MUSIC VOLUME");
            snprintf(lines[strings_ingame_config_audio_volume_sfx], sizeof(lines[strings_ingame_config_audio_volume_sfx]), "%s", "SFX VOLUME");
            snprintf(lines[strings_ingame_config_enabled], sizeof(lines[strings_ingame_config_enabled]), "%s", "ENABLED");
            snprintf(lines[strings_ingame_config_disabled], sizeof(lines[strings_ingame_config_disabled]), "%s", "DISABLED");
            snprintf(lines[strings_ingame_ready_message], sizeof(lines[strings_ingame_ready_message]), "%s", "MISSION STARTED!");
            snprintf(lines[strings_ingame_config_select_player], sizeof(lines[strings_ingame_config_select_player]), "%s", "< SELECT PLAYER >");
            snprintf(lines[strings_ingame_config_press_start_to_select], sizeof(lines[strings_ingame_config_press_start_to_select]), "%s", "PRESS START TO SELECT");
            snprintf(lines[strings_ingame_manual], sizeof(lines[strings_ingame_manual]), "%s", "MANUAL");
            snprintf(lines[strings_ingame_mode], sizeof(lines[strings_ingame_mode]), "%s", "MODE");
            snprintf(lines[strings_ingame_video_scale_mode], sizeof(lines[strings_ingame_video_scale_mode]), "%s", "SCALE MODE");
            snprintf(lines[strings_config_android_hidescreencontrols], sizeof(lines[strings_config_android_hidescreencontrols]), "%s", "HIDE CONTROLS");
            snprintf(lines[strings_config_android_screencontrolssize], sizeof(lines[strings_config_android_screencontrolssize]), "%s", "CONTROLS SIZE");
            snprintf(lines[strings_config_android_useplayservices], sizeof(lines[strings_config_android_useplayservices]), "%s", "PLAY SERVICES");
            snprintf(lines[strings_config_android_usecloudsave], sizeof(lines[strings_config_android_usecloudsave]), "%s", "CLOUD SAVE");
            snprintf(lines[strings_config_wii_joysticktype], sizeof(lines[strings_config_wii_joysticktype]), "%s", "JOYSTICK TYPE");
            snprintf(lines[strings_config_android_screencontrolssize_SMALL], sizeof(lines[strings_config_android_screencontrolssize_SMALL]), "%s", "SMALL");
            snprintf(lines[strings_config_android_screencontrolssize_MEDIUM], sizeof(lines[strings_config_android_screencontrolssize_MEDIUM]), "%s", "MEDIUM");
            snprintf(lines[strings_config_android_screencontrolssize_BIG], sizeof(lines[strings_config_android_screencontrolssize_BIG]), "%s", "BIG");
            snprintf(lines[strings_config_wii_joysticktype_WIIMOTE], sizeof(lines[strings_config_wii_joysticktype_WIIMOTE]), "%s", "WIIMOTE");
            snprintf(lines[strings_config_wii_joysticktype_CLASSIC], sizeof(lines[strings_config_wii_joysticktype_CLASSIC]), "%s", "CLASSIC");
            snprintf(lines[strings_config_wii_joysticktype_GAMECUBE], sizeof(lines[strings_config_wii_joysticktype_GAMECUBE]), "%s", "GAMECUBE");
            snprintf(lines[strings_config_wii_platformspecific], sizeof(lines[strings_config_wii_platformspecific]), "%s", "PLATFORM SPECIFIC");
            snprintf(lines[strings_config_keys_unet], sizeof(lines[strings_config_keys_unet]), "%s", "PLEASE SET");
            snprintf(lines[strings_ingame_config_graphics_performance], sizeof(lines[strings_ingame_config_graphics_performance]), "%s", "PERFORMANCE");
            snprintf(lines[strings_ingame_enable_playservices_dialog], sizeof(lines[strings_ingame_enable_playservices_dialog]), "%s", "ENABLE GOOGLE PLAY SERVICES?");
            snprintf(lines[strings_ingame_enable_cloudsave_dialog], sizeof(lines[strings_ingame_enable_cloudsave_dialog]), "%s", "ENABLE CLOUD SAVE FEATURE?");
            snprintf(lines[strings_ingame_requires_network], sizeof(lines[strings_ingame_requires_network]), "%s", "(REQUIRES AVAILABLE NETWORK)");
            snprintf(lines[strings_config_low], sizeof(lines[strings_config_low]), "%s", "LOW");
            snprintf(lines[strings_config_medium], sizeof(lines[strings_config_medium]), "%s", "MEDIUM");
            snprintf(lines[strings_config_high], sizeof(lines[strings_config_high]), "%s", "HIGH");

            snprintf(lines[string_stage_select_stage], sizeof(lines[string_stage_select_stage]), "%s", "STAGE:");
            snprintf(lines[string_stage_select_lair_of], sizeof(lines[string_stage_select_lair_of]), "%s", "LAIR OF:");
            snprintf(lines[string_stage_select_enter_stage], sizeof(lines[string_stage_select_enter_stage]), "%s", "[ENTER MISSION]");
            snprintf(lines[string_stage_select_unavailable], sizeof(lines[string_stage_select_unavailable]), "%s", "[UNAVAILABLE IN DEMO]");
            snprintf(lines[strings_config_return], sizeof(lines[strings_config_return]), "%s", "RETURN");

            snprintf(lines[string_intro_upperland_studios], sizeof(lines[string_intro_upperland_studios]), "%s", "HOMEBREW GAMES TEAM");
            snprintf(lines[string_intro_presents], sizeof(lines[string_intro_presents]), "%s", "PRESENTS");
            snprintf(lines[string_intro_engine1], sizeof(lines[string_intro_engine1]), "%s", "-- ROCKBOT ENGINE --");
            snprintf(lines[string_intro_engine2], sizeof(lines[string_intro_engine2]), "%s", "THIS GAME USES ORIGINAL CODE");
            snprintf(lines[string_intro_engine3], sizeof(lines[string_intro_engine3]), "%s", "AND ART ASSETS CREATED FROM");
            snprintf(lines[string_intro_engine4], sizeof(lines[string_intro_engine4]), "%s", "SCRATCH. ANY SIMILARITIES");
            snprintf(lines[string_intro_engine5], sizeof(lines[string_intro_engine5]), "%s", "ARE A WAY OF TRIBUTE");
            snprintf(lines[string_intro_engine6], sizeof(lines[string_intro_engine6]), "%s", "TO OLD GAMES OR A MISTAKE.");
            snprintf(lines[string_intro_engine7], sizeof(lines[string_intro_engine7]), "%s", "ROCKBOT AND UPPERLAND ARE NOT");
            snprintf(lines[string_intro_engine8], sizeof(lines[string_intro_engine8]), "%s", "RELATED TO ANY COMPANY OR GAMES.");
            snprintf(lines[string_intro_engine9], sizeof(lines[string_intro_engine9]), "%s", "GOOGLE REMOVED ROCKBOT DUE TO");
            snprintf(lines[string_intro_engine10], sizeof(lines[string_intro_engine10]), "%s", "USERS REPORTING IT AS");
            snprintf(lines[string_intro_engine11], sizeof(lines[string_intro_engine11]), "%s", "'IMPERSONATING' OTHER GAME.");

            snprintf(lines[string_intro_demo_warning_title], sizeof(lines[string_intro_demo_warning_title]), "%s", "-- WARNING HOMEBREW --");
            snprintf(lines[string_intro_demo_warning1], sizeof(lines[string_intro_demo_warning1]), "%s", "THIS IS A HOMEBREW GAME, MADE BY A FEW");
            snprintf(lines[string_intro_demo_warning2], sizeof(lines[string_intro_demo_warning2]), "%s", "PEOPLE, WITHOUT MONETARY GAINS AND");
            snprintf(lines[string_intro_demo_warning3], sizeof(lines[string_intro_demo_warning3]), "%s", "DURING FREE TIME FROM DAILY JOBS.");
            snprintf(lines[string_intro_demo_warning4], sizeof(lines[string_intro_demo_warning4]), "%s", "WE MAKE GAMES FOR YOU TO HAVE FUN, ");
            snprintf(lines[string_intro_demo_warning5], sizeof(lines[string_intro_demo_warning5]), "%s", "BUT STILL SOME PEOPLE HATE US.");
            snprintf(lines[string_intro_demo_warning6], sizeof(lines[string_intro_demo_warning6]), "%s", "DON'T BE A TROLL. DO NOT REPORT THE");
            snprintf(lines[string_intro_demo_warning7], sizeof(lines[string_intro_demo_warning7]), "%s", "APP BECAUSE IT LOOKS ALIKE. ");
            snprintf(lines[string_intro_demo_warning8], sizeof(lines[string_intro_demo_warning8]), "%s", "READ FAQ IN OUT SITE FIRST!");
            snprintf(lines[string_intro_demo_warning9], sizeof(lines[string_intro_demo_warning9]), "%s", "OLD VERSIONS CAN BE OBTAINED AT:");
            snprintf(lines[string_intro_demo_warning10], sizeof(lines[string_intro_demo_warning10]), "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            snprintf(lines[string_intro_demo_warning11], sizeof(lines[string_intro_demo_warning11]), "%s", "");


            snprintf(lines[string_press_key_or_button], sizeof(lines[string_press_key_or_button]), "%s", "PRESS A KEY/BUTTON TO CONTINUE.");
            snprintf(lines[STRING_ENDING_CONCEPT], sizeof(lines[STRING_ENDING_CONCEPT]), "%s", "CONCEPT");
            snprintf(lines[STRING_ENDING_DESIGN], sizeof(lines[STRING_ENDING_DESIGN]), "%s", "DESIGN");


            snprintf(lines[strings_config_android_screencontrolsspacing], sizeof(lines[strings_config_android_screencontrolsspacing]), "%s", "BUTTON SPACING");
            snprintf(lines[strings_config_android_audiobuffersize], sizeof(lines[strings_config_android_audiobuffersize]), "%s", "AUDIO BUFFER SIZE");
            snprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], sizeof(lines[strings_config_android_audiobuffersize_VERY_SMALL]), "%s", "VERY SMALL");
            snprintf(lines[strings_config_android_audiobuffersize_SMALL], sizeof(lines[strings_config_android_audiobuffersize_SMALL]), "%s", "SMALL");
            snprintf(lines[strings_config_android_audiobuffersize_MEDIUM], sizeof(lines[strings_config_android_audiobuffersize_MEDIUM]), "%s", "MEDIUM");
            snprintf(lines[strings_config_android_audiobuffersize_BIG], sizeof(lines[strings_config_android_audiobuffersize_BIG]), "%s", "BIG");
            snprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], sizeof(lines[strings_config_android_audiobuffersize_DESCRIPTION]), "%s", "INCREASE VALUE IF AUDIO IS CHOPPY");

            snprintf(lines[strings_menu_item_CHEATS], sizeof(lines[strings_menu_item_CHEATS]), "%s", "CHEATS");
            snprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], sizeof(lines[strings_menu_item_CHEATS_INVENCIBLE]), "%s", "INVENCIBLE");
            snprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], sizeof(lines[strings_menu_item_CHEATS_ALLBEATEN]), "%s", "ALL BEATEN");
            snprintf(lines[strings_menu_item_CHEATS_CHARACTER], sizeof(lines[strings_menu_item_CHEATS_CHARACTER]), "%s", "CHARACTER");

            snprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], sizeof(lines[strings_menu_pick_save_slot_PICK_OPTION]), "%s", "PLEASE SELECT SAVE SLOT");
            snprintf(lines[strings_menu_pick_save_slot_NEW_GAME], sizeof(lines[strings_menu_pick_save_slot_NEW_GAME]), "%s", "CREATE NEW GAME");
            snprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], sizeof(lines[strings_menu_pick_save_slot_LOAD_GAME]), "%s", "LOAD GAME FILE");
            snprintf(lines[strings_menu_pick_save_slot_NO_SAVE], sizeof(lines[strings_menu_pick_save_slot_NO_SAVE]), "%s", "- NO SAVE FILE -");

            snprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], sizeof(lines[strings_ending_NEW_CHARACTERS_AVAILABLE]), "%s", "NEW CHARACTERS AVAILABLE");
            snprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], sizeof(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE]), "%s", "PRESS    TO CONTINUE");


            snprintf(lines[strings_weapon_name_COIL], sizeof(lines[strings_weapon_name_COIL]), "%s", "FROG COIL");
            snprintf(lines[strings_weapon_name_JET], sizeof(lines[strings_weapon_name_JET]), "%s", "EAGLE JET");
            snprintf(lines[strings_weapon_name_ETANK], sizeof(lines[strings_weapon_name_ETANK]), "%s", "HP CRYSTAL");
            snprintf(lines[strings_weapon_name_WTANK], sizeof(lines[strings_weapon_name_WTANK]), "%s", "MP CRYSTAL");
            snprintf(lines[strings_weapon_name_STANK], sizeof(lines[strings_weapon_name_STANK]), "%s", "SPECIAL CRYSTAL");

            snprintf(lines[strings_weapon_menu_CHANGE_WEAPON], sizeof(lines[strings_weapon_menu_CHANGE_WEAPON]), "%s", "CHANGE WEAPON/ITEM");
            snprintf(lines[strings_weapon_menu_TANKS], sizeof(lines[strings_weapon_menu_TANKS]), "%s", "HEALTH CRYSTALS");
            snprintf(lines[strings_weapon_menu_BOOTS], sizeof(lines[strings_weapon_menu_BOOTS]), "%s", "BOOTS");
            snprintf(lines[strings_weapon_menu_ARMOR], sizeof(lines[strings_weapon_menu_ARMOR]), "%s", "ARMOR");
            snprintf(lines[strings_weapon_menu_WEAPON], sizeof(lines[strings_weapon_menu_WEAPON]), "%s", "BEAM");
            snprintf(lines[strings_weapon_menu_PART_NORMAL], sizeof(lines[strings_weapon_menu_PART_NORMAL]), "%s", "NORMAL");
            snprintf(lines[strings_weapon_menu_PART_ENHANCED], sizeof(lines[strings_weapon_menu_PART_ENHANCED]), "%s", "IMPROVED");

            snprintf(lines[strings_ingame_config_audio_use_old_music], sizeof(lines[strings_ingame_config_audio_use_old_music]), "%s", "USE OLD MUSIC");

            snprintf(lines[strings_about_site], sizeof(lines[strings_about_site]), "%s", "- SITE -");
            snprintf(lines[strings_about_bugs], sizeof(lines[strings_about_bugs]), "%s", "- SUPPORT -");
            snprintf(lines[strings_about_twitter], sizeof(lines[strings_about_twitter]), "%s", "- TWITTER -");
            snprintf(lines[strings_about_facebook], sizeof(lines[strings_about_facebook]), "%s", "- FACEBOOK -");

            snprintf(lines[strings_stage_select_pick_mission], sizeof(lines[strings_stage_select_pick_mission]), "%s", "PICK A MISSION");
            snprintf(lines[strings_stage_select_boss], sizeof(lines[strings_stage_select_boss]), "%s", "BOSS:");
            snprintf(lines[strings_stage_select_select], sizeof(lines[strings_stage_select_select]), "%s", "SELECT");

            snprintf(lines[strings_config_android_use_mod_music], sizeof(lines[strings_config_android_use_mod_music]), "%s", "USE CHIPTUNE MUSIC");
            snprintf(lines[strings_config_android_use_mod_music_warning], sizeof(lines[strings_config_android_use_mod_music_warning]), "%s", "MAY CAUSE AUDIO ISSUES");

            snprintf(lines[strings_weapon_selected], sizeof(lines[strings_weapon_selected]), "%s", "WEAPON");

            snprintf(lines[strings_game_engine_credits_title], sizeof(lines[strings_game_engine_credits_title]), "%s", "GAME ENGINE CREDITS");
            snprintf(lines[strings_game_engine_credits_planning], sizeof(lines[strings_game_engine_credits_planning]), "%s", "PLANNING");
            snprintf(lines[strings_game_engine_credits_design], sizeof(lines[strings_game_engine_credits_design]), "%s", "DESIGN");
            snprintf(lines[strings_game_engine_credits_dialogs], sizeof(lines[strings_game_engine_credits_dialogs]), "%s", "DIALOGS");
            snprintf(lines[strings_game_engine_credits_tests], sizeof(lines[strings_game_engine_credits_tests]), "%s", "TESTS AND REVIEW");
            snprintf(lines[strings_game_engine_credits_illustration], sizeof(lines[strings_game_engine_credits_illustration]), "%s", "ILLUSTRATION");
            snprintf(lines[strings_game_engine_credits_music], sizeof(lines[strings_game_engine_credits_music]), "%s", "MUSIC");
            snprintf(lines[strings_game_engine_credits_translation], sizeof(lines[strings_game_engine_credits_translation]), "%s", "TRANSLATION");
            snprintf(lines[strings_game_engine_credits_programming], sizeof(lines[strings_game_engine_credits_programming]), "%s", "PROGRAMMING");
            snprintf(lines[strings_game_engine_credits_ports], sizeof(lines[strings_game_engine_credits_ports]), "%s", "PORTING");
            snprintf(lines[strings_game_engine_credits_special_thanks], sizeof(lines[strings_game_engine_credits_special_thanks]), "%s", "SPECIAL THANKS");
            snprintf(lines[strings_game_engine_credits_dev_tools], sizeof(lines[strings_game_engine_credits_dev_tools]), "%s", "DEVLOPMENT TOOLS");
            snprintf(lines[strings_game_engine_credits_presented_by], sizeof(lines[strings_game_engine_credits_presented_by]), "%s", "PRESENTED BY");

        }


        /// @TODO: add assert to check that we set all the values from the enum

        std::vector<std::string> res;
        for (int i=0; i<strings_ingame_COUNT; i++) {
            std::string line = std::string(lines[i]) + std::string("\n");
            res.push_back(line);
        }
        return res;
    }

    void fio_strings::create_default_ingame_strings()
    {
        for (int i=0; i<LANGUAGE_COUNT; i++) {
            std::vector<std::string> res = get_default_ingame_strings_list(i);
            save_game_strings(res, get_game_strings_filename(i));
        }
        
    }

    std::vector<std::string> fio_strings::add_missing_default_ingame_strings(std::vector<std::string> list, int language)
    {

        std::vector<std::string> res = get_default_ingame_strings_list(language);
        // add \n to the list that were removed when loaded from file
        for (int i=0; i<list.size(); i++) {
            list.at(i) = list.at(i) + std::string("\n");
        }
        for (int i=list.size(); i<strings_ingame_COUNT; i++) {
            list.push_back(res.at(i));
        }
        save_game_strings(list, get_game_strings_filename(language));
        return list;
    }

    std::string fio_strings::get_stage_dialogs_filename(short stage_id, int language)
    {
        char char_filename[100];
        if (language == LANGUAGE_FRENCH) {
            snprintf(char_filename, sizeof(char_filename), "/dialogs/stage_dialogs_%d_fr.dat", stage_id);
        } else if (language == LANGUAGE_SPANISH) {
            snprintf(char_filename, sizeof(char_filename), "/dialogs/stage_dialogs_%d_es.dat", stage_id);
        } else if (language == LANGUAGE_ITALIAN) {
            snprintf(char_filename, sizeof(char_filename), "/dialogs/stage_dialogs_%d_it.dat", stage_id);
        } else if (language == LANGUAGE_PORTUGUESE) {
            snprintf(char_filename, sizeof(char_filename), "/dialogs/stage_dialogs_%d_pt.dat", stage_id);
        } else {
            snprintf(char_filename, sizeof(char_filename), "/dialogs/stage_dialogs_%d.dat", stage_id);
        }
        std::string filename = FILEPATH + std::string(char_filename);
        return filename;
    }

    std::string fio_strings::get_language_filename_prefix(int language)
    {
        if (language == LANGUAGE_FRENCH) {
            return std::string("fr");
        } else if (language == LANGUAGE_SPANISH) {
            return std::string("es");
        } else if (language == LANGUAGE_ITALIAN) {
            return std::string("it");
        } else if (language == LANGUAGE_PORTUGUESE) {
            return std::string("pt");
        }
        return "en";
    }

    std::string fio_strings::convert_text_symbols(std::string text)
    {
        for (std::map<int,std::string>::iterator it=code_map.begin(); it!=code_map.end(); ++it) {

            char replace1 = (char)(it->first);
            std::size_t found = text.find(replace1);
            if (found!=std::string::npos) {
                std::stringstream ss;
                ss << replace1;
                std::string replace1_str = ss.str();
                StringUtils::replace_all(text, replace1_str, std::string(it->second));
            }
        }
        return text;
    }

    std::vector<std::string> fio_strings::get_game_engine_credits()
    {
        std::vector<std::string> credits_list;
        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_title) + " -");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");


        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_planning) + " -");
        credits_list.push_back("IURI FIEDORUK");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_design) + " -");
        credits_list.push_back("IURI FIEDORUK");
        credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
        credits_list.push_back("BOBERATU");
        credits_list.push_back("HFBN2");
        credits_list.push_back("CAPT. CHRIS AND KB");
        credits_list.push_back("SURT.OPENGAMEART");
        credits_list.push_back("AVERAGE-HANZO.DEVIANTART");
        credits_list.push_back("FUNKY96.DEVIANTART");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_dialogs) + " -");
        credits_list.push_back("IURI FIEDORUK");
        credits_list.push_back("NELSON ROSENBERG");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_illustration) + " -");
        credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
        credits_list.push_back("IURI FIEDORUK");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_music) + " -");
        credits_list.push_back("SIMONE 'JMD' BERNACCHIA");
        credits_list.push_back("MODARCHIVE.ORG");
        credits_list.push_back("FIREAGE");
        credits_list.push_back("SOLARIS");
        credits_list.push_back("LEON/EXCESS");
        credits_list.push_back("DIOMATIC");
        credits_list.push_back("JASON");
        credits_list.push_back("DEADLOCK/EXLSD");
        credits_list.push_back("4MAT");
        credits_list.push_back("ZANAZAC");
        credits_list.push_back("AMEGA INDUSTRIESQ");
        credits_list.push_back("DEATH ADDR");
        credits_list.push_back("TOBBX");
        credits_list.push_back("HYOCHAN");
        credits_list.push_back("DR.BULLY/MAJIC12");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_translation) + " -");
        credits_list.push_back("DOUBLE JAY GREEN");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_programming) + " -");
        credits_list.push_back("IURI FIEDORUK");
        credits_list.push_back("FARLEY KNIGHT");
        credits_list.push_back("FELIPE ZACANI (FPS)");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_ports) + " -");
        credits_list.push_back("ANDROID: PELYA");
        credits_list.push_back("DINGUX: SHIN-NIL");
        credits_list.push_back("PANDORA: SIGMA NL");
        credits_list.push_back("PS2: RAGNAROK2040");
        credits_list.push_back("PS2: WOON-YUNG LIU");
        credits_list.push_back("PS2: SP193");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        if (SharedData::get_instance()->current_language == LANGUAGE_PORTUGUESE) {
            credits_list.push_back("- REVISÃO E TESTES -");
        } else {
            credits_list.push_back("- REVIEW & TESTING -");
        }
        credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
        credits_list.push_back("NELSON ROSENBERG");
        credits_list.push_back("ANDREW PRZELUCKI");
        credits_list.push_back("YAMA DROID");
        credits_list.push_back("DARK GAMERDUB");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_special_thanks) + " -");
        credits_list.push_back("FREE SDK DEVELOPERS");
        credits_list.push_back("DEVIANTART.COM");
        credits_list.push_back("PIXELJOINT.COM");
        credits_list.push_back("OPENGAMEART.ORG");
        credits_list.push_back("VENOM");
        credits_list.push_back("JERONIMO");
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_dev_tools) + " -");
        credits_list.push_back("CODING:");
        credits_list.push_back("LIBSDL");
        credits_list.push_back("QT LIB");
        credits_list.push_back("ANDROID NDK");
        credits_list.push_back("GCC G+");
        credits_list.push_back("CLANG/LLVM");
        credits_list.push_back("KATE EDITOR");
        credits_list.push_back("");
        credits_list.push_back("OPERATING SYSTEM:");
        credits_list.push_back("UBUNTU LINUX");
        credits_list.push_back("MINT LINUX");
        credits_list.push_back("KDE NEON");
        credits_list.push_back("");
        credits_list.push_back("GRAPHICS:");
        credits_list.push_back("GIMP EDITOR");
        credits_list.push_back("PAINT TOOL SAI");
        credits_list.push_back("SIMPLE SCREEN RECORD");
        credits_list.push_back("");
        credits_list.push_back("SOUND AND MUSIC:");
        credits_list.push_back("AUDACIOUS PLAYER");
        credits_list.push_back("AUDACITY EDITOR");
        credits_list.push_back("BFXR.NET");
        credits_list.push_back("");
        credits_list.push_back("OTHER TOOLS:");
        credits_list.push_back("WINE");
        credits_list.push_back("PLAYONLINUX");
        credits_list.push_back("");
        credits_list.push_back("HARDWARE:");
        credits_list.push_back("ACER ASPIRE RYZEN7");
        credits_list.push_back("PSP");
        credits_list.push_back("PLAYSTATION 2");
        credits_list.push_back("POCKET GO");
        credits_list.push_back("DINGOO");
        credits_list.push_back("NINTENDO DS");
        credits_list.push_back("NINTENDO WII");
        credits_list.push_back("RASPBERY PI");

        for (int i=0; i<20; i++) {
            credits_list.push_back("");
        }
        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_presented_by) + " -");
        credits_list.push_back("UPPERLAND STUDIOS");

        return credits_list;
    }

    void fio_strings::log_error(std::string error)
    {
        std::ofstream outfile;
        std::string filename = FILEPATH + "/errors.log";
        outfile.open(filename.c_str(), std::ios_base::app); // append instead of overwrite
        outfile << error << std::endl;
        outfile.close();
    }

    void fio_strings::create_default_common_strings()
    {

        /// @TODO - set the IDS for each line

    }

    void fio_strings::create_default_dialog_strings(int language)
    {
        std::vector<std::string> res;
        for (int i=0; i<FS_MAX_STAGES; i++) {
            char line_value[FS_CHAR_NAME_SIZE];
            // person dialogs
            for (int j=0; j<6; j++) {
                snprintf(line_value, sizeof(line_value), "START-STG[%d] DLG #%d", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // phrases
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    snprintf(player_line_value, sizeof(player_line_value), "START-STG[%d] DLG P[%d] %d", i, (j+1), k);
                    res.push_back(player_line_value);
                }
            }
            // person dialogs
            for (int j=0; j<6; j++) {
                snprintf(line_value, sizeof(line_value), "BOSS STG[%d] DLG #%d\n", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // lines
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    snprintf(player_line_value, sizeof(player_line_value), "BOSS STG[%d] DLG P[%d] %d", i, (j+1), k);
                    res.push_back(player_line_value);
                }
            }
            save_game_strings(res, get_stage_dialogs_filename(i, language));
            res.clear();
        }
    }




    std::vector<std::string> fio_strings::get_common_strings(int language, bool convert_symbols)
    {
        if (FILEPATH == "") {
            return common_strings_list;
        }

        if (common_strings_list.size() == 0) {
            common_strings_list = load_game_strings_from_file(get_common_strings_filename(language), language, convert_symbols);
        }
        return common_strings_list;
    }



    std::vector<std::string> fio_strings::get_string_list_from_file(std::string filename)
    {
        std::vector<std::string> res;
        filename = StringUtils::clean_filename(filename);
        std::ifstream fp(filename.c_str());

        if (!fp.is_open()) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### ERROR.fio_strings::get_string_list_from_file file [%s] not found. ###", filename.c_str());
#endif
            return res;
        } else {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### ERROR.fio_strings::get_string_list_from_file file [%s] loaded. ###", filename.c_str());
#endif
        }

        std::string str;

        while (getline(fp, str)) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### ERROR.fio_strings::get_string_list_from_file::read.str[%s], res.size[%d]. ###", str.c_str(), res.size());
#endif

            if (str.length() > 0) {
                StringUtils::replace_all(str, "\n", "");
                StringUtils::replace_all(str, "\r", "");
            }
            res.push_back(str);
        }
        fp.close();

#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### ERROR.fio_strings::get_string_list_from_file res.size[%d]. ###", res.size());
#endif

        return res;
    }



    std::vector<std::string> fio_strings::get_string_list_from_scene_text_file(int text_scene_n)
    {
        std::vector<std::string> text_list;
        char file_chr[255];

        sprintf(file_chr, "%d.txt", text_scene_n);
        std::string filename = FILEPATH + "scenes/text/" + get_language_filename_prefix(SharedData::get_instance()->current_language) + "/" + std::string(file_chr);
        filename = StringUtils::clean_filename(filename);
        // if does not have language, try default english
        if (!file_exists(filename) && SharedData::get_instance()->current_language != LANGUAGE_ENGLISH) {
            filename = FILEPATH + "scenes/text/" + get_language_filename_prefix(LANGUAGE_ENGLISH) + "/" + std::string(file_chr);
            filename = StringUtils::clean_filename(filename);
        }
        if (file_exists(filename)) {
            std::ifstream fp(filename.c_str());
            std::string str;
            while (getline(fp, str)) {
                if (str.length() > 0) {
                    StringUtils::replace_all(str, "\n", "");
                    StringUtils::replace_all(str, "\r", "");
                    std::string str2 = "\\xC9";
                    if (str.find(str2) != -1) {
                        str.replace(str.find(str2), str2.length(),"é");
                    }
                    StringUtils::replace_all(str, "\\xC9", "é");
                }
                text_list.push_back(str);
            }

        }
        // fill empty spaces
        if (text_list.size() < SCENE_TEXT_LINES_N) {
            for (int i=text_list.size(); i<SCENE_TEXT_LINES_N; i++) {
                text_list.push_back(std::string(""));
            }
        }
        return text_list;

    }

    void fio_strings::write_scene_text_file(int text_scene_n, std::vector<std::string> list, int language)
    {
        char file_chr[255];

        sprintf(file_chr, "%d.txt", text_scene_n);
        std::string filename = FILEPATH + "scenes/text/" + get_language_filename_prefix(language) + "/" + std::string(file_chr);
        std::cout << "FIO:_STRINGS::write_scene_text_file[" << filename << "]" << std::endl;
        // fill empty spaces
        if (list.size() < SCENE_TEXT_LINES_N) {
            for (int i=list.size(); i<SCENE_TEXT_LINES_N; i++) {
                list.push_back(std::string(""));
            }
        }

        save_game_strings(list, filename);

    }

    std::string fio_strings::get_common_string(int id, int language, bool convert_symbols)
    {

        if (id == -1) {
            return std::string("");
        }
        if (FILEPATH == "") {
            return std::string("");
        }

        if (common_strings_list.size() == 0) {
            common_strings_list = load_game_strings_from_file(get_common_strings_filename(language), language,convert_symbols);
        }

        if (id >= common_strings_list.size()) {
            return std::string("");
        }

        return common_strings_list.at(id);
    }

    std::string fio_strings::get_stage_dialog(short stage_id, int id, int language, bool convert_symbols)
    {
        if (_dialogs_stage_id != stage_id) {
            _dialogs_stage_id = stage_id;
            dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id, language), language, convert_symbols);
            if (dialogs_strings_list.size() == 0) {
                create_default_dialog_strings(language);
                dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id, language),language, convert_symbols);
            }
        }
        if (id < 0 || id >= dialogs_strings_list.size()) {
            return std::string("");
        }
        return dialogs_strings_list.at(id);
    }

    std::vector<std::string> fio_strings::get_stage_dialogs(short stage_id, int language, bool convert_symbols)
    {
        std::string filename;
        if (_dialogs_stage_id != -1) {
            filename = get_stage_dialogs_filename(_dialogs_stage_id, language);
        }

        dialogs_strings_list.clear();
        _dialogs_stage_id = stage_id;
        filename = get_stage_dialogs_filename(_dialogs_stage_id, language);
        dialogs_strings_list = load_game_strings_from_file(filename, language, convert_symbols);
        if (dialogs_strings_list.size() == 0) {
            create_default_dialog_strings(language);
            dialogs_strings_list = load_game_strings_from_file(filename, language, convert_symbols);
        }

        // generate dialogs, if needed
        if (dialogs_strings_list.size() == 0) {
            create_default_dialog_strings(language);
            std::string dialogs_filename = get_stage_dialogs_filename(_dialogs_stage_id,language);
            dialogs_strings_list = load_game_strings_from_file(dialogs_filename,language, convert_symbols);
        }
        if (dialogs_strings_list.size() < STAGE_DIALOG_NUMBER) {
            std::cout << "ERROR: Invalid dialogs list size[" << dialogs_strings_list.size() << "]. Minimum is " << STAGE_DIALOG_NUMBER << ". Will add missing lines." << std::endl;
            for (int line_n=dialogs_strings_list.size(); line_n<STAGE_DIALOG_NUMBER; line_n++) {
                dialogs_strings_list.push_back("");
            }
        }
        return dialogs_strings_list;
    }


    void fio_strings::save_common_strings(std::vector<std::string> data, int language)
    {
        save_game_strings(data, get_common_strings_filename(language));
    }

    void fio_strings::create_files()
    {
        create_default_ingame_strings();
        create_default_common_strings();
    }

    void fio_strings::save_stage_dialogs(short stage_id, int language, std::vector<std::string> data)
    {
        // @TODO: add support for multiple languages
        save_game_strings(data, get_stage_dialogs_filename(stage_id, language));
    }

    std::string fio_strings::get_language_prefix(int config)
    {
        std::string res = "en";
        if (config == 1) {
            res = "fr";
        } else if (config == 2) {
            res = "es";
        } else if (config == 3) {
            res = "it";
        }
        return res;
    }









}// namepsace
