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
        if (n < 0 || n > string_list.size()) {
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
            sprintf(lines[strings_ingame_newgame], "%s", "NOVO JOGO");
            sprintf(lines[strings_ingame_loadgame], "%s", "CARREGAR JOGO");
            sprintf(lines[strings_ingame_config], "%s", "CONFIG");
            sprintf(lines[strings_ingame_about], "%s", "SOBRE");
            sprintf(lines[strings_ingame_extras], "%s", "EXTRAS");
            sprintf(lines[strings_ingame_password], "%s", "SENHA");
            sprintf(lines[strings_ingame_gameover], "%s", "GAME OVER");
            sprintf(lines[strings_ingame_and], "%s", "E");
            sprintf(lines[strings_ingame_yougot_singular], "%s", "FOI ADQUIRIDA");
            sprintf(lines[strings_ingame_yougot_plural], "%s", "FORAM ADQUIRIDOS");
            sprintf(lines[strings_ingame_selectgame], "%s", "SELECIONE JOGO");
            sprintf(lines[strings_ingame_savegameerror1], "%s", "ERRO SALVANDO JOGO,");
            sprintf(lines[strings_ingame_savegameerror2], "%s", "VERIFIQUE SE DISPOSITIVO OU");
            sprintf(lines[strings_ingame_savegameerror3], "%s", "ARQUIVO ESTÁ PROTEGIDO PARA GRAVACAO.");
            sprintf(lines[strings_ingame_copyrightline], "%s", "\xA9 2009-2020 UPPERLAND STUDIOS");
            sprintf(lines[strings_ingame_audio], "%s", "AUDIO");
            sprintf(lines[strings_ingame_input], "%s", "CONTROLE");
            sprintf(lines[strings_ingame_video], "%s", "VIDEO");
            sprintf(lines[strings_ingame_leavestage], "%s", "SAIR DA FASE");
            sprintf(lines[strings_ingame_language], "%s", "IDIOMA");
            sprintf(lines[strings_ingame_video_windowed], "%s", "JANELA");
            sprintf(lines[strings_ingame_video_fullscreen], "%s", "TELA CHEIA");
            sprintf(lines[strings_ingame_video_noscale], "%s", "SEM ESCALA  ");
            sprintf(lines[strings_ingame_video_size2x], "%s", "TAMANHO X2    ");
            sprintf(lines[strings_ingame_video_scale2x], "%s", "SCALE2X   ");
            sprintf(lines[strings_ingame_video_show_fps], "%s", "MOSTRAR FPS");
            sprintf(lines[strings_ingame_config_restart1], "%s", "POR FAVOR REINICIE O JOGO");
            sprintf(lines[strings_ingame_config_restart2], "%s", "PARA NOVA CONFIGURACAO");
            sprintf(lines[strings_ingame_config_restart3], "%s", "SER ATIVADA.");
            sprintf(lines[strings_ingame_config_presstorestart], "%s", "APERTE UM BOTÃO PARA CONTINUAR");
            sprintf(lines[strings_ingame_pressanykey], "%s", "APERTE QUALQUER BOTÃO");
            sprintf(lines[strings_ingame_config_set], "%s", "SETAR");
            sprintf(lines[strings_ingame_config_key_up], "%s", "CIMA");
            sprintf(lines[strings_ingame_config_key_down], "%s", "BAIXO");
            sprintf(lines[strings_ingame_config_key_left], "%s", "ESQUERDA");
            sprintf(lines[strings_ingame_config_key_right], "%s", "DIREITA");
            sprintf(lines[strings_ingame_config_key_jump], "%s", "PULO");
            sprintf(lines[strings_ingame_config_key_dash], "%s", "DASH");
            sprintf(lines[strings_ingame_config_key_shield], "%s", "ESCUDO");
            sprintf(lines[strings_ingame_config_key_fire], "%s", "TIRO");
            sprintf(lines[strings_ingame_config_key_start], "%s", "START");
            sprintf(lines[strings_ingame_config_key_keyl], "%s", "BOTAO-L");
            sprintf(lines[strings_ingame_config_key_keyr], "%s", "BOTAO-R");
            sprintf(lines[strings_ingame_config_key_pressnew], "%s", "APERTE NOVO BOTÃO/TECLA");
            sprintf(lines[strings_ingame_config_key_reset], "%s", "RESTAURAR PADRAO");
            sprintf(lines[strings_ingame_config_key_directional], "%s", "DIRECTIONAL");
            sprintf(lines[strings_ingame_config_key_directional_analog], "%s", "ANALOGICO");
            sprintf(lines[strings_ingame_config_key_directional_digital], "%s", "DIGITAL");

            sprintf(lines[strings_ingame_config_android_cloud_save1], "%s", "SALVAR NA NUVEM REQUER");
            sprintf(lines[strings_ingame_config_android_cloud_save2], "%s", "CONEXAO COM A REDE OU");
            sprintf(lines[strings_ingame_config_android_cloud_save3], "%s", "O JOGO CONGELARA QUANDO");
            sprintf(lines[strings_ingame_config_android_cloud_save4], "%s", "OBTER DADOS DO SERVIDOR.");

            sprintf(lines[strings_ingame_config_android_play_services1], "%s", "HABILITAR ESTA OPÇÃO REQUER");
            sprintf(lines[strings_ingame_config_android_play_services2], "%s", "CONEXAO DE DADOS.");
            sprintf(lines[strings_ingame_config_android_play_services3], "%s", "AVISO: O JOGO PODE CONGELAR");
            sprintf(lines[strings_ingame_config_android_play_services4], "%s", "COM ESTA OPCAO HABILITADA.");

            sprintf(lines[strings_ingame_config_android_hide_controls1], "%s", "REQUER UM CONTROLE FÍSICO,");
            sprintf(lines[strings_ingame_config_android_hide_controls2], "%s", "CASO CONTRÁRIO NÃO CONSEGUIRA");
            sprintf(lines[strings_ingame_config_android_hide_controls3], "%s", "RESTAURAR CONFIGURACOES.");

            sprintf(lines[strings_ingame_config_extras_music_player], "%s", "PLAYER DE MÚSICA");
            sprintf(lines[strings_ingame_config_extras_sound_player], "%s", "PLAYER DE EFEITOS SONOROS");
            sprintf(lines[strings_ingame_config_extras_art_gallery], "%s", "GALERIA DE ARTE");

            sprintf(lines[strings_ingame_passwordinvalid], "%s", "SENHA INVALIDA");
            sprintf(lines[strings_ingame_pressstart], "%s", "APERTE START");
            sprintf(lines[strings_ingame_engineerror], "%s", "ROCKBOT ENGINE ERROR");
            sprintf(lines[strings_ingame_nogames], "%s", "NENHUM JOGO DISPONIVEL");
            sprintf(lines[strings_ingame_quitgame], "%s", "SAIR DO JOGO?");
            sprintf(lines[strings_ingame_yes], "%s", "SIM");
            sprintf(lines[strings_ingame_no], "%s", "NAO");
            sprintf(lines[strings_ingame_life], "%s", "VIDAS");
            sprintf(lines[strings_ingame_item], "%s", "ITEM");
            sprintf(lines[strings_ingame_coin], "%s", "MOEDAS");
            sprintf(lines[strings_ingame_gotarmor_type_ability], "%s", "CONCEDEM A HABILIDADE PARA");
            sprintf(lines[strings_ingame_gotarmor_type_arms], "%s", "OS BRACOS MELHORADOS IRAO");
            sprintf(lines[strings_ingame_gotarmor_type_arms_msg], "%s", "DISPARAR");
            sprintf(lines[strings_ingame_gotarmor_type_legs], "%s", "ESTAS PERNAS MELHORADAS");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg1], "%s", "DASH NO AR.");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg2], "%s", "EXECUTAR UMN PULO DUPLO");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg3], "%s", "SHOURUYKEN (CIMA+DASH)");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg4], "%s", " ");
            sprintf(lines[strings_ingame_gotarmor_type_body], "%s", "O CORPO FORTIFICADO PERMITE");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg1], "%s", "FICAR INTANGIVEL MAIS TEMPO");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg2], "%s", "TOMAR APENAS METADE DO DANO");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg3], "%s", "NAO SER JOGADO PARA TRAS");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg4], "%s", "RESISTIR ESPINHOS");
            sprintf(lines[strings_ingame_difficulty_select], "%s", "ESCOLHA DIFICULDADE:");
            sprintf(lines[strings_ingame_difficulty_easy], "%s", "FACIL");
            sprintf(lines[strings_ingame_difficulty_normal], "%s", "NORMAL");
            sprintf(lines[strings_ingame_difficulty_hard], "%s", "DIFICIL");
            sprintf(lines[strings_ingame_config_input_selected_joystick], "%s", "CONTROLE SELECIONADO");
            sprintf(lines[strings_ingame_config_input_buttons], "%s", "CONFIGURAR BOTOES");
            sprintf(lines[strings_ingame_config_input_turbo_mode], "%s", "MODO TURBO");
            sprintf(lines[strings_ingame_config_input_autocharge_mode], "%s", "AUTO-CARREGA");
            sprintf(lines[strings_ingame_config_on], "%s", "SIM");
            sprintf(lines[strings_ingame_config_off], "%s", "NAO");
            sprintf(lines[strings_ingame_config_quitgame], "%s", "SAIR DO JOGO");
            sprintf(lines[strings_ingame_armor], "%s", "ARMADURA");
            sprintf(lines[strings_ingame_config_audio_volume_music], "%s", "VOLUME DE MUSICA");
            sprintf(lines[strings_ingame_config_audio_volume_sfx], "%s", "VOLUME EFEITOS");
            sprintf(lines[strings_ingame_config_enabled], "%s", "HABILITADO");
            sprintf(lines[strings_ingame_config_disabled], "%s", "DESABILITADO");
            sprintf(lines[strings_ingame_ready_message], "%s", "MISSÃO INICIADA!");
            sprintf(lines[strings_ingame_config_select_player], "%s", "< SELECIONE PERSONAGEM >");
            sprintf(lines[strings_ingame_config_press_start_to_select], "%s", "APERTE START PARA INICIAR");
            sprintf(lines[strings_ingame_manual], "%s", "MANUAL");
            sprintf(lines[strings_ingame_mode], "%s", "MODO");
            sprintf(lines[strings_ingame_video_scale_mode], "%s", "MODO ESCALA DE TELA");
            sprintf(lines[strings_config_android_hidescreencontrols], "%s", "ESCONDER CONTROLES");
            sprintf(lines[strings_config_android_screencontrolssize], "%s", "TAMANHO CONTROLES");
            sprintf(lines[strings_config_android_useplayservices], "%s", "PLAY SERVICES");
            sprintf(lines[strings_config_android_usecloudsave], "%s", "SALVAR NA NUVEM");
            sprintf(lines[strings_config_wii_joysticktype], "%s", "TIPO DE CONTROLE");
            sprintf(lines[strings_config_android_screencontrolssize_SMALL], "%s", "PEQUENO");
            sprintf(lines[strings_config_android_screencontrolssize_MEDIUM], "%s", "MEDIUM");
            sprintf(lines[strings_config_android_screencontrolssize_BIG], "%s", "GRANDE");
            sprintf(lines[strings_config_wii_joysticktype_WIIMOTE], "%s", "WIIMOTE");
            sprintf(lines[strings_config_wii_joysticktype_CLASSIC], "%s", "CLASSIC");
            sprintf(lines[strings_config_wii_joysticktype_GAMECUBE], "%s", "GAMECUBE");
            sprintf(lines[strings_config_wii_platformspecific], "%s", "ESPECIFICO PLATAFORMA");
            sprintf(lines[strings_config_keys_unet], "%s", "POR FAVOR DEFINA");
            sprintf(lines[strings_ingame_config_graphics_performance], "%s", "PERFORMANCE");
            sprintf(lines[strings_ingame_enable_playservices_dialog], "%s", "HABILITAR GOOGLE PLAY SERVICES?");
            sprintf(lines[strings_ingame_enable_cloudsave_dialog], "%s", "HAVILITAR SAVE NA NUVEM?");
            sprintf(lines[strings_ingame_requires_network], "%s", "(REQUER REDE DE DADOS ATIVA)");
            sprintf(lines[strings_config_low], "%s", "BAIXO");
            sprintf(lines[strings_config_medium], "%s", "MEDIO");
            sprintf(lines[strings_config_high], "%s", "ALTO");
            sprintf(lines[string_stage_select_stage], "%s", "FASE:");
            sprintf(lines[string_stage_select_lair_of], "%s", "LAR DE:");
            sprintf(lines[string_stage_select_enter_stage], "%s", "[ENTRAR MISSÃO]");
            sprintf(lines[string_stage_select_unavailable], "%s", "[INDISPONÍVEL NA DEMO]");
            sprintf(lines[strings_config_return], "%s", "VOLTAR");

            sprintf(lines[string_intro_upperland_studios], "%s", "TIME DE JOGOS CASEIROS");
            sprintf(lines[string_intro_presents], "%s", "APRESENTA");

            sprintf(lines[string_intro_engine1], "%s", "-- ROCKBOT ENGINE --");
            sprintf(lines[string_intro_engine2], "%s", "ESTE JOGO USA CÓDIGO E ARTE");
            sprintf(lines[string_intro_engine3], "%s", "ORIGINAIS, CRIADOS DO ZERO");
            sprintf(lines[string_intro_engine4], "%s", "QUALQUER SEMELHANÇA É MERA");
            sprintf(lines[string_intro_engine5], "%s", "COINCIDÊNCIA OU HOMENAGEM");
            sprintf(lines[string_intro_engine6], "%s", "À JOGOS ANTIGOS OU ENGANO.");
            sprintf(lines[string_intro_engine7], "%s", "ROCKBOT E UPPERLAND NÃO TEM");
            sprintf(lines[string_intro_engine8], "%s", "QUALQUER RELAÇÃO COM OUTRAS EMPRESAS.");
            sprintf(lines[string_intro_engine9], "%s", "ROCKBOT FOI REMOVIDO PELO GOOGLE");
            sprintf(lines[string_intro_engine10], "%s", "USUÁRIOS RELATANDO ELE COMO");
            sprintf(lines[string_intro_engine11], "%s", "SENDO VERSÃO 'FALSA' DE OUTRO.");

            sprintf(lines[string_intro_demo_warning_title], "%s", "-- AVISO JOGO CASEIRO"
                                                                  " --");
            sprintf(lines[string_intro_demo_warning1], "%s", "ESTE JOGO FOI FEITO EM CASA, POR");
            sprintf(lines[string_intro_demo_warning2], "%s", "POUCAS PESSOAS, SEM GANHOS MONETÁRIOS");
            sprintf(lines[string_intro_demo_warning3], "%s", "E DURANTE O TEMPO LIVRE DO EMPREGO.");
            sprintf(lines[string_intro_demo_warning4], "%s", "FAZEMOS JOGOS PARA VOCÊ SE DIVERTIR");
            sprintf(lines[string_intro_demo_warning5], "%s", "AINDA ASSIM, ALGUNS NOS ODEIAM.");
            sprintf(lines[string_intro_demo_warning6], "%s", "NÃO SEJA TROLL. NÃO REPORTE");
            sprintf(lines[string_intro_demo_warning7], "%s", "SÓ PORQUE ACHA PARECIDO.");
            sprintf(lines[string_intro_demo_warning8], "%s", "LEIA A FAQ NO NOSSO SITE ANTES!!");
            sprintf(lines[string_intro_demo_warning9], "%s", "VERSÃO ROCKBOT PODE SER OBTIDA EM:");
            sprintf(lines[string_intro_demo_warning10], "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            sprintf(lines[string_intro_demo_warning11], "%s", "");
            sprintf(lines[string_press_key_or_button], "%s", "");
            sprintf(lines[STRING_ENDING_CONCEPT], "%s", "CONCEITO");
            sprintf(lines[STRING_ENDING_DESIGN], "%s", "DESENHO");

            sprintf(lines[strings_config_android_screencontrolsspacing], "%s", "ESPAÇO BOTÕES");
            sprintf(lines[strings_config_android_audiobuffersize], "%s", "TAMANHO BUFFER ÁUDIO");
            sprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], "%s", "MUITO PEQUENO");
            sprintf(lines[strings_config_android_audiobuffersize_SMALL], "%s", "PEQUENO");
            sprintf(lines[strings_config_android_audiobuffersize_MEDIUM], "%s", "MÉDIO");
            sprintf(lines[strings_config_android_audiobuffersize_BIG], "%s", "GRANDE");
            sprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], "%s", "AUMENTE VALOR SE ÁUDIO ESTIVER CORTADO");

            sprintf(lines[strings_menu_item_CHEATS], "%s", "TRAPAÇAS");
            sprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], "%s", "INVENCÍVEL");
            sprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], "%s", "TUDO DERROTADO");
            sprintf(lines[strings_menu_item_CHEATS_CHARACTER], "%s", "PERSONAGEM");

            sprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], "%s", "ESCOLHA UM ESPAÇO DE SAVE");
            sprintf(lines[strings_menu_pick_save_slot_NEW_GAME], "%s", "CRIAR NOVO JOGO");
            sprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], "%s", "CARREGAR ARQUIVO");
            sprintf(lines[strings_menu_pick_save_slot_NO_SAVE], "%s", "- SEM ARQUIVO -");

            sprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], "%s", "NOVOS PERSONAGENS DISPONPIVEIS");
            sprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], "%s", "USE      PARA CONTINUAR");

            sprintf(lines[strings_weapon_name_COIL], "%s", "SAPO-MOLA");
            sprintf(lines[strings_weapon_name_JET], "%s", "ÁGUIA-JATO");
            sprintf(lines[strings_weapon_name_ETANK], "%s", "CRISTAL HP");
            sprintf(lines[strings_weapon_name_WTANK], "%s", "CRISTAL MP");
            sprintf(lines[strings_weapon_name_STANK], "%s", "CRISTAL ESPECIAL");

            sprintf(lines[strings_weapon_menu_CHANGE_WEAPON], "%s", "MUDAR ARMA/ITEM");
            sprintf(lines[strings_weapon_menu_TANKS], "%s", "CRISTAIS DE ENERGIA");
            sprintf(lines[strings_weapon_menu_BOOTS], "%s", "BOTAS");
            sprintf(lines[strings_weapon_menu_ARMOR], "%s", "ARMADURA");
            sprintf(lines[strings_weapon_menu_WEAPON], "%s", "TIRO");
            sprintf(lines[strings_weapon_menu_PART_NORMAL], "%s", "NORMAL");
            sprintf(lines[strings_weapon_menu_PART_ENHANCED], "%s", "MELHORADO");

            sprintf(lines[strings_ingame_config_audio_use_old_music], "%s", "USAR MÚSICA ANTIGA");

            sprintf(lines[strings_about_site], "%s", "- SITE -");
            sprintf(lines[strings_about_bugs], "%s", "- SUPORTE -");
            sprintf(lines[strings_about_twitter], "%s", "- TWITTER -");
            sprintf(lines[strings_about_facebook], "%s", "- FACEBOOK -");

            sprintf(lines[strings_stage_select_pick_mission], "%s", "ESCOLHA UMA MISSÃO");
            sprintf(lines[strings_stage_select_boss], "%s", "CHEFE:");
            sprintf(lines[strings_stage_select_select], "%s", "SELECIONAR");

            sprintf(lines[strings_config_android_use_mod_music], "%s", "USAR MÚSICA CHIPTUNE");
            sprintf(lines[strings_config_android_use_mod_music_warning], "%s", "PODE CAUSAR PROBLEMAS DE ÁUDIO");

            sprintf(lines[strings_weapon_selected], "%s", "ARMA");

            sprintf(lines[strings_game_engine_credits_title], "%s", "CRÉDITOS DO MOTOR DE JOGO");
            sprintf(lines[strings_game_engine_credits_planning], "%s", "PLANEJAMENTO");
            sprintf(lines[strings_game_engine_credits_design], "%s", "DESIGN");
            sprintf(lines[strings_game_engine_credits_dialogs], "%s", "DIÁLOGOS");
            sprintf(lines[strings_game_engine_credits_tests], "%s", "TESTES AND REVISÃO");
            sprintf(lines[strings_game_engine_credits_illustration], "%s", "ILUSTRAÇÕES");
            sprintf(lines[strings_game_engine_credits_music], "%s", "MÚSICA");
            sprintf(lines[strings_game_engine_credits_translation], "%s", "TRADUÇÃO");
            sprintf(lines[strings_game_engine_credits_programming], "%s", "PROGRAMAÇÃO");
            sprintf(lines[strings_game_engine_credits_ports], "%s", "PLATAFORMAS");
            sprintf(lines[strings_game_engine_credits_special_thanks], "%s", "AGRADECIMENTOS ESPECIAIS");
            sprintf(lines[strings_game_engine_credits_dev_tools], "%s", "FERRAMENTAS DE DESENVOLVIMENTO");
            sprintf(lines[strings_game_engine_credits_presented_by], "%s", "APRESENTADO POR");


        } else if (language == LANGUAGE_SPANISH) {
            sprintf(lines[strings_ingame_newgame], "%s", "NUEVA PARTIDA");
            sprintf(lines[strings_ingame_loadgame], "%s", "CARGAR PARTIDA");
            sprintf(lines[strings_ingame_config], "%s", "CONFIGURACIÓN");
            sprintf(lines[strings_ingame_about], "%s", "INFORMACIÓN");
            sprintf(lines[strings_ingame_extras], "%s", "EXTRAS");
            sprintf(lines[strings_ingame_password], "%s", "CONTRASEÑA");
            sprintf(lines[strings_ingame_gameover], "%s", "PARTIDA ACABADA");
            sprintf(lines[strings_ingame_and], "%s", "Y");
            sprintf(lines[strings_ingame_yougot_singular], "%s", "HA SIDO ADQUIRIDO");
            sprintf(lines[strings_ingame_yougot_plural], "%s", "HA SIDO ADQUIRIDO");
            sprintf(lines[strings_ingame_selectgame], "%s", "SELECCIONE PARTIDA");
            sprintf(lines[strings_ingame_savegameerror1], "%s", "ERROR AL GUARDAR PARTIDA,");
            sprintf(lines[strings_ingame_savegameerror2], "%s", "EVISE QUE EL DISPOSITIVO O");
            sprintf(lines[strings_ingame_savegameerror3], "%s", "ARCHIVO NO ESTE PROTEGIDO.");
            sprintf(lines[strings_ingame_copyrightline], "%s", "\xA9 2009-2020 UPPERLAND STUDIOS");
            sprintf(lines[strings_ingame_audio], "%s", "AUDIO");
            sprintf(lines[strings_ingame_input], "%s", "INPUT");
            sprintf(lines[strings_ingame_video], "%s", "VIDEO");
            sprintf(lines[strings_ingame_leavestage], "%s", "DEJAR NIVEL");
            sprintf(lines[strings_ingame_language], "%s", "LENGUAJE");
            sprintf(lines[strings_ingame_video_windowed], "%s", "EN VENTANA");
            sprintf(lines[strings_ingame_video_fullscreen], "%s", "PANTALLA COMPLETA");
            sprintf(lines[strings_ingame_video_noscale], "%s", "NO ESCALAR  ");
            sprintf(lines[strings_ingame_video_size2x], "%s", "TAMAÑO X2    ");
            sprintf(lines[strings_ingame_video_scale2x], "%s", "ESCALA X2   ");
            sprintf(lines[strings_ingame_video_show_fps], "%s", "ENSEÑAR FPS");
            sprintf(lines[strings_ingame_config_restart1], "%s", "OR FAVOR REINICIE EL JUEGO");
            sprintf(lines[strings_ingame_config_restart2], "%s", "PARA QUE LA CONFIGURACIÓN");
            sprintf(lines[strings_ingame_config_restart3], "%s", "TOME EFECTO.");
            sprintf(lines[strings_ingame_config_presstorestart], "%s", "PRESIONE UN BOTÓN PARA REGRESAR");
            sprintf(lines[strings_ingame_pressanykey], "%s", "PRESIONE CUALQUIER BOTÓN");
            sprintf(lines[strings_ingame_config_set], "%s", "DEFINIR");
            sprintf(lines[strings_ingame_config_key_up], "%s", "ARRIBA");
            sprintf(lines[strings_ingame_config_key_down], "%s", "ABAJO");
            sprintf(lines[strings_ingame_config_key_left], "%s", "IZQUIERDA");
            sprintf(lines[strings_ingame_config_key_right], "%s", "DERECHA");
            sprintf(lines[strings_ingame_config_key_jump], "%s", "SALTAR");
            sprintf(lines[strings_ingame_config_key_dash], "%s", "DASH");
            sprintf(lines[strings_ingame_config_key_shield], "%s", "ESCUDO");
            sprintf(lines[strings_ingame_config_key_fire], "%s", "DISPARAR");
            sprintf(lines[strings_ingame_config_key_start], "%s", "START");
            sprintf(lines[strings_ingame_config_key_keyl], "%s", "BOTÓN-L");
            sprintf(lines[strings_ingame_config_key_keyr], "%s", "BOTÓN-R");
            sprintf(lines[strings_ingame_config_key_pressnew], "%s", "PRESIONE UNA NUEVA TECLA/BOTÓN");
            sprintf(lines[strings_ingame_config_key_reset], "%s", "REESTABLECER");
            sprintf(lines[strings_ingame_config_key_directional], "%s", "DIRECCIONAL");
            sprintf(lines[strings_ingame_config_key_directional_analog], "%s", "ANÁLOGO");
            sprintf(lines[strings_ingame_config_key_directional_digital], "%s", "DIGITAL");

            sprintf(lines[strings_ingame_config_android_cloud_save1], "%s", "EL GUARDADO EN LA NUBE REQUIERE");
            sprintf(lines[strings_ingame_config_android_cloud_save2], "%s", "CONEXIÓN A INTERNET O EL JUEGO SE");
            sprintf(lines[strings_ingame_config_android_cloud_save3], "%s", "CONGELARÁ CUANDO TRATE DE OBTENER");
            sprintf(lines[strings_ingame_config_android_cloud_save4], "%s", "DATOS DEL SERVIDOR.");

            sprintf(lines[strings_ingame_config_android_play_services1], "%s", "ACTIVAR ESTA OPCIÓN REQUIERE");
            sprintf(lines[strings_ingame_config_android_play_services2], "%s", "CONEXIÓN A DATOS.");
            sprintf(lines[strings_ingame_config_android_play_services3], "%s", "ADVERTENCIA: EL JUEGO PODRÍA");
            sprintf(lines[strings_ingame_config_android_play_services4], "%s", "CONGELARSE CON ESTA OPCIÓN ACTIVADA.");

            sprintf(lines[strings_ingame_config_android_hide_controls1], "%s", "REQUIERES DE UN MANDO FÍSICO,");
            sprintf(lines[strings_ingame_config_android_hide_controls2], "%s", "O NO PODRÁS RESTABLECER LA");
            sprintf(lines[strings_ingame_config_android_hide_controls3], "%s", "CONFIGURACIÓN.");

            sprintf(lines[strings_ingame_config_extras_music_player], "%s", "REPRODUCTOR DE MÚSICA");
            sprintf(lines[strings_ingame_config_extras_sound_player], "%s", "REPRODUCTOR DE SFX");
            sprintf(lines[strings_ingame_config_extras_art_gallery], "%s", "GALLERÍA DE ARTE");

            sprintf(lines[strings_ingame_passwordinvalid], "%s", "CONTRASEÑA INVALIDA");
            sprintf(lines[strings_ingame_pressstart], "%s", "PRESIONE START");
            sprintf(lines[strings_ingame_engineerror], "%s", "ERROR EN ROCKBOT ENGINE");
            sprintf(lines[strings_ingame_nogames], "%s", "NO HAY JUEGOS DISPONIBLES");
            sprintf(lines[strings_ingame_quitgame], "%s", "¿SALIR DEL JUEGO?");
            sprintf(lines[strings_ingame_yes], "%s", "SI");
            sprintf(lines[strings_ingame_no], "%s", "NO");
            sprintf(lines[strings_ingame_life], "%s", "VIDA(S)");
            sprintf(lines[strings_ingame_item], "%s", "OBJETO(S)");
            sprintf(lines[strings_ingame_coin], "%s", "MONEDA(S)");
            sprintf(lines[strings_ingame_gotarmor_type_ability], "%s", "TE DA LA HABILIDAD DE");
            sprintf(lines[strings_ingame_gotarmor_type_arms], "%s", "ESTOS BRAZOS MEJORADOS TE PERMITEN");
            sprintf(lines[strings_ingame_gotarmor_type_arms_msg], "%s", "DISPARAR");
            sprintf(lines[strings_ingame_gotarmor_type_legs], "%s", "ESTAS PIERNAS LIGERAS TE PERMITEN");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg1], "%s", "HACER UN DASH EN EL AIRE.");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg2], "%s", "HACER UN SALTO DOBLE");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg3], "%s", "HACER UN SHOURYUKEN (ARRIBA+DASH)");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg4], "%s", " ");
            sprintf(lines[strings_ingame_gotarmor_type_body], "%s", "ESTE CUERPO FORTIFICADO TE PERMITE");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg1], "%s", "SER INTANGIBLE POR MÁS TIEMPO");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg2], "%s", "TOMAR LA MITAD DEL DAÑO");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg3], "%s", "EVITAR EMPUJONES AL SER DAÑADO");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg4], "%s", "RESISTIR ESPINAS");
            sprintf(lines[strings_ingame_difficulty_select], "%s", "SELECCIONE UNA DIFICULTAD:");
            sprintf(lines[strings_ingame_difficulty_easy], "%s", "FÁCIL");
            sprintf(lines[strings_ingame_difficulty_normal], "%s", "NORMAL");
            sprintf(lines[strings_ingame_difficulty_hard], "%s", "DIFÍCIL");
            sprintf(lines[strings_ingame_config_input_selected_joystick], "%s", "JOYSTICK SELECCIONADO");
            sprintf(lines[strings_ingame_config_input_buttons], "%s", "CONFIGURAR BOTONES");
            sprintf(lines[strings_ingame_config_input_turbo_mode], "%s", "MODO TURBO");
            sprintf(lines[strings_ingame_config_input_autocharge_mode], "%s", "AUTO-CARGA");
            sprintf(lines[strings_ingame_config_on], "%s", "SI");
            sprintf(lines[strings_ingame_config_off], "%s", "NO");
            sprintf(lines[strings_ingame_config_quitgame], "%s", "SALIR DEL JUEGO");
            sprintf(lines[strings_ingame_armor], "%s", "ARMADURA");
            sprintf(lines[strings_ingame_config_audio_volume_music], "%s", "VOLUMEN DE MÚSICA");
            sprintf(lines[strings_ingame_config_audio_volume_sfx], "%s", "VOLUMEN DE SFX");
            sprintf(lines[strings_ingame_config_enabled], "%s", "HABILITADO");
            sprintf(lines[strings_ingame_config_disabled], "%s", "DESHABILITADO");
            sprintf(lines[strings_ingame_ready_message], "%s", "¡MISIÓN INICIADA!");
            sprintf(lines[strings_ingame_config_select_player], "%s", "< SELECCIONE UN PERSONAJE >");
            sprintf(lines[strings_ingame_config_press_start_to_select], "%s", "PRESIONE START PARA SELECCIONAR");
            sprintf(lines[strings_ingame_manual], "%s", "MANUAL");
            sprintf(lines[strings_ingame_mode], "%s", "MODO");
            sprintf(lines[strings_ingame_video_scale_mode], "%s", "MODO DE ESCALA");
            sprintf(lines[strings_config_android_hidescreencontrols], "%s", "OCULTAR CONTROLES");
            sprintf(lines[strings_config_android_screencontrolssize], "%s", "TAMAÑO DE CONTROLES");
            sprintf(lines[strings_config_android_useplayservices], "%s", "SERVICIOS DE GOOGLE PLAY");
            sprintf(lines[strings_config_android_usecloudsave], "%s", "GUARDADO EN LA NUBE");
            sprintf(lines[strings_config_wii_joysticktype], "%s", "TIPO DE JOYSTICK");
            sprintf(lines[strings_config_android_screencontrolssize_SMALL], "%s", "PEQUEÑO");
            sprintf(lines[strings_config_android_screencontrolssize_MEDIUM], "%s", "MEDIANO");
            sprintf(lines[strings_config_android_screencontrolssize_BIG], "%s", "GRANDE");
            sprintf(lines[strings_config_wii_joysticktype_WIIMOTE], "%s", "WIIMOTE");
            sprintf(lines[strings_config_wii_joysticktype_CLASSIC], "%s", "CLASSIC");
            sprintf(lines[strings_config_wii_joysticktype_GAMECUBE], "%s", "GAMECUBE");
            sprintf(lines[strings_config_wii_platformspecific], "%s", "ESPECIFICO DE PLATAFORMA");
            sprintf(lines[strings_config_keys_unet], "%s", "POR FAVOR DEFINA");
            sprintf(lines[strings_ingame_config_graphics_performance], "%s", "RENDIMIENTO");
            sprintf(lines[strings_ingame_enable_playservices_dialog], "%s", "¿HABILITAR SERVICIOS DE GOOGLE PLAY?");
            sprintf(lines[strings_ingame_enable_cloudsave_dialog], "%s", "¿HABILITAR GUARDADO EN LA NUBE?");
            sprintf(lines[strings_ingame_requires_network], "%s", "(SE REQUIERE UN CONEXIÓN DISPONIBLE)");
            sprintf(lines[strings_config_low], "%s", "BAJO");
            sprintf(lines[strings_config_medium], "%s", "MEDIO");
            sprintf(lines[strings_config_high], "%s", "ALTO");
            sprintf(lines[string_stage_select_stage], "%s", "NIVEL:");
            sprintf(lines[string_stage_select_lair_of], "%s", "GUARIDA DE:");
            sprintf(lines[string_stage_select_enter_stage], "%s", "[SELECCIONAR]");
            sprintf(lines[string_stage_select_unavailable], "%s", "[NO DISPONIBLE EN LA DEMO]");
            sprintf(lines[strings_config_return], "%s", "REGRESAR");

            sprintf(lines[string_intro_upperland_studios], "%s", "HOMEBREW GAMES TEAM");
            sprintf(lines[string_intro_presents], "%s", "PRESENTA");

            sprintf(lines[string_intro_engine1], "%s", "-- ROCKBOT ENGINE --");
            sprintf(lines[string_intro_engine2], "%s", "ESTE JUEGO UTILIZA CÓDIGO ORIGINAL");
            sprintf(lines[string_intro_engine3], "%s", "Y ASSETS HECHOS DESDE CERO.");
            sprintf(lines[string_intro_engine4], "%s", "CUALQUIER SIMILITUDES SON");
            sprintf(lines[string_intro_engine5], "%s", "TRIBUTOS A JUEGOS ANTIGUOS");
            sprintf(lines[string_intro_engine6], "%s", "O UN MALENTENDIDO.");

            sprintf(lines[string_intro_engine7], "%s", "ROCKBOT Y UPPERLAND NO ESTÁN RELACIONADOS");
            sprintf(lines[string_intro_engine8], "%s", "A NINGUNA OTRA COMPAÑÍA O JUEGO.");
            sprintf(lines[string_intro_engine9], "%s", "GOOGLE REMOVIÓ ROCKBOT DEBIDO");
            sprintf(lines[string_intro_engine10], "%s", "A USUARIOS REPORTÁNDOLO POR");
            sprintf(lines[string_intro_engine11], "%s", "‘COPIAR’ A OTRO JUEGO.");

            sprintf(lines[string_intro_demo_warning_title], "%s", "-- ADVERTENCIA HOMEBREW"
                                                                  " --");
            sprintf(lines[string_intro_demo_warning1], "%s", "ESTO ES UN HOMEBREW, HECHO POR");
            sprintf(lines[string_intro_demo_warning2], "%s", "POCAS PERSONAS SIN NINGUNA GANANCIA");
            sprintf(lines[string_intro_demo_warning3], "%s", "MONETARIA Y DURANTE NUESTRO TIEMPO");
            sprintf(lines[string_intro_demo_warning4], "%s", "LIBRE DEL TRABAJO. HACEMOS JUEGOS PARA");
            sprintf(lines[string_intro_demo_warning5], "%s", "QUÉ SE DIVIERTAN.");
            sprintf(lines[string_intro_demo_warning6], "%s", "NÃO SEJA TROLL. NÃO REPORTE");
            sprintf(lines[string_intro_demo_warning7], "%s", "SÓ PORQUE ACHA PARECIDO.");
            sprintf(lines[string_intro_demo_warning8], "%s", "POR FAVOR, LEA EL F.A.Q.!!");
            sprintf(lines[string_intro_demo_warning9], "%s", "VERSIONES ANTIGUAS PUEDEN SER OBTENIDAS EN:");
            sprintf(lines[string_intro_demo_warning10], "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            sprintf(lines[string_intro_demo_warning11], "%s", "");
            sprintf(lines[string_press_key_or_button], "%s", "");
            sprintf(lines[STRING_ENDING_CONCEPT], "%s", "CONCEPTO");
            sprintf(lines[STRING_ENDING_DESIGN], "%s", "DISEÑO");

            sprintf(lines[strings_config_android_screencontrolsspacing], "%s", "ESPACIADO ENTRE BOTONES");
            sprintf(lines[strings_config_android_audiobuffersize], "%s", "TAMAÑO DEL BÚFER DE AUDIO");
            sprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], "%s", "MUY PEQUEÑO");
            sprintf(lines[strings_config_android_audiobuffersize_SMALL], "%s", "PEQUEÑO");
            sprintf(lines[strings_config_android_audiobuffersize_MEDIUM], "%s", "MEDIANO");
            sprintf(lines[strings_config_android_audiobuffersize_BIG], "%s", "GRANDE");
            sprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], "%s", "INCREMENTE EL VALOR SI SE ESCUCHA CORTADO");

            sprintf(lines[strings_menu_item_CHEATS], "%s", "TRUCOS");
            sprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], "%s", "INVENCIBILIDAD");
            sprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], "%s", "TODOS LOS NIVELES COMPLETADOS");
            sprintf(lines[strings_menu_item_CHEATS_CHARACTER], "%s", "PERSONAJE");

            sprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], "%s", "POR FAVOR, SELECCIONE UNA CASILLA DE GUARDADO");
            sprintf(lines[strings_menu_pick_save_slot_NEW_GAME], "%s", "CREAR UNA NUEVA PARTIDA");
            sprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], "%s", "CARGAR UN ARCHIVO DE GUARDADO");
            sprintf(lines[strings_menu_pick_save_slot_NO_SAVE], "%s", "– SIN PARTIDA GUARDADA –");

            sprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], "%s", "NUEVOS PERSONAJES DESBLOQUEADOS");
            sprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], "%s", "USA      PARA CONTINUAR");

            sprintf(lines[strings_weapon_name_COIL], "%s", "FROG COIL");
            sprintf(lines[strings_weapon_name_JET], "%s", "EAGLE JET");
            sprintf(lines[strings_weapon_name_ETANK], "%s", "HP CRYSTAL");
            sprintf(lines[strings_weapon_name_WTANK], "%s", "MP CRYSTAL");
            sprintf(lines[strings_weapon_name_STANK], "%s", "SPECIAL CRYSTAL");

            sprintf(lines[strings_weapon_menu_CHANGE_WEAPON], "%s", "CAMBIAR ARMA/OBJETO");
            sprintf(lines[strings_weapon_menu_TANKS], "%s", "HEALTH CRYSTALS");
            sprintf(lines[strings_weapon_menu_BOOTS], "%s", "BOTAS");
            sprintf(lines[strings_weapon_menu_ARMOR], "%s", "ARMADURA");
            sprintf(lines[strings_weapon_menu_WEAPON], "%s", "RAYO");
            sprintf(lines[strings_weapon_menu_PART_NORMAL], "%s", "NORMAL");
            sprintf(lines[strings_weapon_menu_PART_ENHANCED], "%s", "MEJORADO");

            sprintf(lines[strings_ingame_config_audio_use_old_music], "%s", "USAR MÚSICA ANTIGUA");

            sprintf(lines[strings_about_site], "%s", "- SITIO -");
            sprintf(lines[strings_about_bugs], "%s", "- AYUDA -");
            sprintf(lines[strings_about_twitter], "%s", "- TWITTER -");
            sprintf(lines[strings_about_facebook], "%s", "- FACEBOOK -");

            sprintf(lines[strings_stage_select_pick_mission], "%s", "SELECCIONE UNA MISIÓN");
            sprintf(lines[strings_stage_select_boss], "%s", "JEFE:");
            sprintf(lines[strings_stage_select_select], "%s", "SELECCIONE");

            sprintf(lines[strings_config_android_use_mod_music], "%s", "USAR MÚSICA CHIPTUNE");
            sprintf(lines[strings_config_android_use_mod_music_warning], "%s", "PUEDE CAUSAR PROBLEMAS DE AUDIO");

            sprintf(lines[strings_weapon_selected], "%s", "ARMA");

            sprintf(lines[strings_game_engine_credits_title], "%s", "CRÉDITOS DO MOTOR DE JOGO");
            sprintf(lines[strings_game_engine_credits_planning], "%s", "PLANEJAMENTO");
            sprintf(lines[strings_game_engine_credits_design], "%s", "DESIGN");
            sprintf(lines[strings_game_engine_credits_dialogs], "%s", "DIÁLOGOS");
            sprintf(lines[strings_game_engine_credits_tests], "%s", "TESTES AND REVISÃO");
            sprintf(lines[strings_game_engine_credits_illustration], "%s", "ILUSTRAÇÕES");
            sprintf(lines[strings_game_engine_credits_music], "%s", "MÚSICA");
            sprintf(lines[strings_game_engine_credits_translation], "%s", "TRADUÇÃO");
            sprintf(lines[strings_game_engine_credits_programming], "%s", "PROGRAMAÇÃO");
            sprintf(lines[strings_game_engine_credits_ports], "%s", "PLATAFORMAS");
            sprintf(lines[strings_game_engine_credits_special_thanks], "%s", "AGRADECIMENTOS ESPECIAIS");
            sprintf(lines[strings_game_engine_credits_dev_tools], "%s", "FERRAMENTAS DE DESENVOLVIMENTO");
            sprintf(lines[strings_game_engine_credits_presented_by], "%s", "APRESENTADO POR");

        } else {
            sprintf(lines[strings_ingame_newgame], "%s", "NEW GAME");
            sprintf(lines[strings_ingame_loadgame], "%s", "LOAD GAME");
            sprintf(lines[strings_ingame_config], "%s", "CONFIG");
            sprintf(lines[strings_ingame_about], "%s", "ABOUT");
            sprintf(lines[strings_ingame_extras], "%s", "EXTRAS");
            sprintf(lines[strings_ingame_password], "%s", "PASSWORD");
            sprintf(lines[strings_ingame_gameover], "%s", "GAME OVER");
            sprintf(lines[strings_ingame_and], "%s", "AND");
            sprintf(lines[strings_ingame_yougot_singular], "%s", "HAS BEEN ACQUIRED");
            sprintf(lines[strings_ingame_yougot_plural], "%s", "HAVE BEEN ACQUIRED");
            sprintf(lines[strings_ingame_selectgame], "%s", "SELECT GAME");
            sprintf(lines[strings_ingame_savegameerror1], "%s", "ERROR WHILE SAVING GAME,");
            sprintf(lines[strings_ingame_savegameerror2], "%s", "PLEASE CHECK THAT THE DEVICE OR");
            sprintf(lines[strings_ingame_savegameerror3], "%s", "FILE IS NOT WRITE-PROTECTED.");
            sprintf(lines[strings_ingame_copyrightline], "%s", "\xA9 2009-2020 UPPERLAND STUDIOS");
            sprintf(lines[strings_ingame_audio], "%s", "AUDIO");
            sprintf(lines[strings_ingame_input], "%s", "INPUT");
            sprintf(lines[strings_ingame_video], "%s", "VIDEO");
            sprintf(lines[strings_ingame_leavestage], "%s", "LEAVE STAGE");
            sprintf(lines[strings_ingame_language], "%s", "LANGUAGE");
            sprintf(lines[strings_ingame_video_windowed], "%s", "WINDOWED");
            sprintf(lines[strings_ingame_video_fullscreen], "%s", "FULLSCREEN");
            sprintf(lines[strings_ingame_video_noscale], "%s", "NO SCALE  ");
            sprintf(lines[strings_ingame_video_size2x], "%s", "SIZE2X    ");
            sprintf(lines[strings_ingame_video_scale2x], "%s", "SCALE2X   ");
            sprintf(lines[strings_ingame_video_show_fps], "%s", "SHOW FPS");
            sprintf(lines[strings_ingame_config_restart1], "%s", "PLEASE RESTART THE GAME");
            sprintf(lines[strings_ingame_config_restart2], "%s", "FOR THE CONFIGURATION");
            sprintf(lines[strings_ingame_config_restart3], "%s", "TO TAKE EFFECT");
            sprintf(lines[strings_ingame_config_presstorestart], "%s", "PRESS A BUTTON TO RETURN");
            sprintf(lines[strings_ingame_pressanykey], "%s", "PRESS ANY KEY OR BUTTON");
            sprintf(lines[strings_ingame_config_set], "%s", "SET");
            sprintf(lines[strings_ingame_config_key_up], "%s", "UP");
            sprintf(lines[strings_ingame_config_key_down], "%s", "DOWN");
            sprintf(lines[strings_ingame_config_key_left], "%s", "LEFT");
            sprintf(lines[strings_ingame_config_key_right], "%s", "RIGHT");
            sprintf(lines[strings_ingame_config_key_jump], "%s", "JUMP");
            sprintf(lines[strings_ingame_config_key_dash], "%s", "DASH");
            sprintf(lines[strings_ingame_config_key_shield], "%s", "SHIELD");
            sprintf(lines[strings_ingame_config_key_fire], "%s", "FIRE");
            sprintf(lines[strings_ingame_config_key_start], "%s", "START");
            sprintf(lines[strings_ingame_config_key_keyl], "%s", "BUTTON-L");
            sprintf(lines[strings_ingame_config_key_keyr], "%s", "BUTTON-R");
            sprintf(lines[strings_ingame_config_key_pressnew], "%s", "PRESS NEW KEY/BUTTON");
            sprintf(lines[strings_ingame_config_key_reset], "%s", "RESET TO DEFAULT");
            sprintf(lines[strings_ingame_config_key_directional], "%s", "DIRECTIONAL");
            sprintf(lines[strings_ingame_config_key_directional_analog], "%s", "ANALOG");
            sprintf(lines[strings_ingame_config_key_directional_digital], "%s", "DIGITAL");
            sprintf(lines[strings_ingame_config_android_cloud_save1], "%s", "CLOUD SAVE REQUIRES");
            sprintf(lines[strings_ingame_config_android_cloud_save2], "%s", "A NETWORK CONNECTION");
            sprintf(lines[strings_ingame_config_android_cloud_save3], "%s", "OR GAME WILL FREEZE WHEN");
            sprintf(lines[strings_ingame_config_android_cloud_save4], "%s", "GETTING SERVER DATA.");
            sprintf(lines[strings_ingame_config_android_play_services1], "%s", "ENABLING THIS OPTION REQUIRES");
            sprintf(lines[strings_ingame_config_android_play_services2], "%s", "A DATA CONNECTION.");
            sprintf(lines[strings_ingame_config_android_play_services3], "%s", "WARNING: THE GAME MAY FREEZE");
            sprintf(lines[strings_ingame_config_android_play_services4], "%s", "WITH THIS OPTION ENABLED.");


            sprintf(lines[strings_ingame_config_android_hide_controls1], "%s", "REQUIRES PHYSICAL CONTROLLER.");
            sprintf(lines[strings_ingame_config_android_hide_controls2], "%s", "OTHERWISE YOU WON'T BE");
            sprintf(lines[strings_ingame_config_android_hide_controls3], "%s", "ABLE TO RESTORE SETTING.");

            sprintf(lines[strings_ingame_config_extras_music_player], "%s", "MUSIC PLAYER");
            sprintf(lines[strings_ingame_config_extras_sound_player], "%s", "SFX PLAYER");
            sprintf(lines[strings_ingame_config_extras_art_gallery], "%s", "ART GALLERY");


            sprintf(lines[strings_ingame_passwordinvalid], "%s", "INVALID PASSWORD");
            sprintf(lines[strings_ingame_pressstart], "%s", "PRESS START");
            sprintf(lines[strings_ingame_engineerror], "%s", "ROCKBOT ENGINE ERROR");
            sprintf(lines[strings_ingame_nogames], "%s", "NO GAMES AVAILABLE");
            sprintf(lines[strings_ingame_quitgame], "%s", "QUIT GAME?");
            sprintf(lines[strings_ingame_yes], "%s", "YES");
            sprintf(lines[strings_ingame_no], "%s", "NO");
            sprintf(lines[strings_ingame_life], "%s", "LIFE");
            sprintf(lines[strings_ingame_item], "%s", "ITEM");
            sprintf(lines[strings_ingame_coin], "%s", "COIN");
            sprintf(lines[strings_ingame_gotarmor_type_ability], "%s", "GIVE YOU THE ABILITY TO");
            sprintf(lines[strings_ingame_gotarmor_type_arms], "%s", "IMPROVED ARMS ALLOWS YOU TO");
            sprintf(lines[strings_ingame_gotarmor_type_arms_msg], "%s", "SHOOT");
            sprintf(lines[strings_ingame_gotarmor_type_legs], "%s", "THOSE LIGHTER LEGS");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg1], "%s", "DASH IN MIDDLE-AIR.");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg2], "%s", "EXECUTE DOUBLE JUMP");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg3], "%s", "SHOURUYKEN (UP+DASH)");
            sprintf(lines[strings_ingame_gotarmor_type_legs_msg4], "%s", " ");
            sprintf(lines[strings_ingame_gotarmor_type_body], "%s", "THIS FORTIFIED BODY WILL");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg1], "%s", "BE INTANGIBLE MORE TIME");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg2], "%s", "TAKE HALF DAMAGE");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg3], "%s", "AVOID PUSH-BACK ON HIT");
            sprintf(lines[strings_ingame_gotarmor_type_body_msg4], "%s", "RESIST SPIKES");
            sprintf(lines[strings_ingame_difficulty_select], "%s", "SELECT DIFFICULTY:");
            sprintf(lines[strings_ingame_difficulty_easy], "%s", "EASY");
            sprintf(lines[strings_ingame_difficulty_normal], "%s", "NORMAL");
            sprintf(lines[strings_ingame_difficulty_hard], "%s", "HARD");
            sprintf(lines[strings_ingame_config_input_selected_joystick], "%s", "SELECTED JOYSTICK");
            sprintf(lines[strings_ingame_config_input_buttons], "%s", "CONFIG BUTTONS");
            sprintf(lines[strings_ingame_config_input_turbo_mode], "%s", "TURBO MODE");
            sprintf(lines[strings_ingame_config_input_autocharge_mode], "%s", "AUTO-CHARGED");
            sprintf(lines[strings_ingame_config_on], "%s", "ON");
            sprintf(lines[strings_ingame_config_off], "%s", "OFF");
            sprintf(lines[strings_ingame_config_quitgame], "%s", "QUIT GAME");
            sprintf(lines[strings_ingame_armor], "%s", "ARMOR");
            sprintf(lines[strings_ingame_config_audio_volume_music], "%s", "MUSIC VOLUME");
            sprintf(lines[strings_ingame_config_audio_volume_sfx], "%s", "SFX VOLUME");
            sprintf(lines[strings_ingame_config_enabled], "%s", "ENABLED");
            sprintf(lines[strings_ingame_config_disabled], "%s", "DISABLED");
            sprintf(lines[strings_ingame_ready_message], "%s", "MISSION STARTED!");
            sprintf(lines[strings_ingame_config_select_player], "%s", "< SELECT PLAYER >");
            sprintf(lines[strings_ingame_config_press_start_to_select], "%s", "PRESS START TO SELECT");
            sprintf(lines[strings_ingame_manual], "%s", "MANUAL");
            sprintf(lines[strings_ingame_mode], "%s", "MODE");
            sprintf(lines[strings_ingame_video_scale_mode], "%s", "SCALE MODE");
            sprintf(lines[strings_config_android_hidescreencontrols], "%s", "HIDE CONTROLS");
            sprintf(lines[strings_config_android_screencontrolssize], "%s", "CONTROLS SIZE");
            sprintf(lines[strings_config_android_useplayservices], "%s", "PLAY SERVICES");
            sprintf(lines[strings_config_android_usecloudsave], "%s", "CLOUD SAVE");
            sprintf(lines[strings_config_wii_joysticktype], "%s", "JOYSTICK TYPE");
            sprintf(lines[strings_config_android_screencontrolssize_SMALL], "%s", "SMALL");
            sprintf(lines[strings_config_android_screencontrolssize_MEDIUM], "%s", "MEDIUM");
            sprintf(lines[strings_config_android_screencontrolssize_BIG], "%s", "BIG");
            sprintf(lines[strings_config_wii_joysticktype_WIIMOTE], "%s", "WIIMOTE");
            sprintf(lines[strings_config_wii_joysticktype_CLASSIC], "%s", "CLASSIC");
            sprintf(lines[strings_config_wii_joysticktype_GAMECUBE], "%s", "GAMECUBE");
            sprintf(lines[strings_config_wii_platformspecific], "%s", "PLATFORM SPECIFIC");
            sprintf(lines[strings_config_keys_unet], "%s", "PLEASE SET");
            sprintf(lines[strings_ingame_config_graphics_performance], "%s", "PERFORMANCE");
            sprintf(lines[strings_ingame_enable_playservices_dialog], "%s", "ENABLE GOOGLE PLAY SERVICES?");
            sprintf(lines[strings_ingame_enable_cloudsave_dialog], "%s", "ENABLE CLOUD SAVE FEATURE?");
            sprintf(lines[strings_ingame_requires_network], "%s", "(REQUIRES AVAILABLE NETWORK)");
            sprintf(lines[strings_config_low], "%s", "LOW");
            sprintf(lines[strings_config_medium], "%s", "MEDIUM");
            sprintf(lines[strings_config_high], "%s", "HIGH");

            sprintf(lines[string_stage_select_stage], "%s", "STAGE:");
            sprintf(lines[string_stage_select_lair_of], "%s", "LAIR OF:");
            sprintf(lines[string_stage_select_enter_stage], "%s", "[ENTER MISSION]");
            sprintf(lines[string_stage_select_unavailable], "%s", "[UNAVAILABLE IN DEMO]");
            sprintf(lines[strings_config_return], "%s", "RETURN");

            sprintf(lines[string_intro_upperland_studios], "%s", "HOMEBREW GAMES TEAM");
            sprintf(lines[string_intro_presents], "%s", "PRESENTS");
            sprintf(lines[string_intro_engine1], "%s", "-- ROCKBOT ENGINE --");
            sprintf(lines[string_intro_engine2], "%s", "THIS GAME USES ORIGINAL CODE");
            sprintf(lines[string_intro_engine3], "%s", "AND ART ASSETS CREATED FROM");
            sprintf(lines[string_intro_engine4], "%s", "SCRATCH. ANY SIMILARITIES");
            sprintf(lines[string_intro_engine5], "%s", "ARE A WAY OF TRIBUTE");
            sprintf(lines[string_intro_engine6], "%s", "TO OLD GAMES OR A MISTAKE.");
            sprintf(lines[string_intro_engine7], "%s", "ROCKBOT AND UPPERLAND ARE NOT");
            sprintf(lines[string_intro_engine8], "%s", "RELATED TO ANY COMPANY OR GAMES.");
            sprintf(lines[string_intro_engine9], "%s", "GOOGLE REMOVED ROCKBOT DUE TO");
            sprintf(lines[string_intro_engine10], "%s", "USERS REPORTING IT AS");
            sprintf(lines[string_intro_engine11], "%s", "'IMPERSONATING' OTHER GAME.");

            sprintf(lines[string_intro_demo_warning_title], "%s", "-- WARNING HOMEBREW --");
            sprintf(lines[string_intro_demo_warning1], "%s", "THIS IS A HOMEBREW GAME, MADE BY A FEW");
            sprintf(lines[string_intro_demo_warning2], "%s", "PEOPLE, WITHOUT MONETARY GAINS AND");
            sprintf(lines[string_intro_demo_warning3], "%s", "DURING FREE TIME FROM DAILY JOBS.");
            sprintf(lines[string_intro_demo_warning4], "%s", "WE MAKE GAMES FOR YOU TO HAVE FUN, ");
            sprintf(lines[string_intro_demo_warning5], "%s", "BUT STILL SOME PEOPLE HATE US.");
            sprintf(lines[string_intro_demo_warning6], "%s", "DON'T BE A TROLL. DO NOT REPORT THE");
            sprintf(lines[string_intro_demo_warning7], "%s", "APP BECAUSE IT LOOKS ALIKE. ");
            sprintf(lines[string_intro_demo_warning8], "%s", "READ FAQ IN OUT SITE FIRST!");
            sprintf(lines[string_intro_demo_warning9], "%s", "OLD VERSIONS CAN BE OBTAINED AT:");
            sprintf(lines[string_intro_demo_warning10], "%s", "HTTPS://ROCKBOT.UPPERLAND.NET");
            sprintf(lines[string_intro_demo_warning11], "%s", "");


            sprintf(lines[string_press_key_or_button], "%s", "PRESS A KEY/BUTTON TO CONTINUE.");
            sprintf(lines[STRING_ENDING_CONCEPT], "%s", "CONCEPT");
            sprintf(lines[STRING_ENDING_DESIGN], "%s", "DESIGN");


            sprintf(lines[strings_config_android_screencontrolsspacing], "%s", "BUTTON SPACING");
            sprintf(lines[strings_config_android_audiobuffersize], "%s", "AUDIO BUFFER SIZE");
            sprintf(lines[strings_config_android_audiobuffersize_VERY_SMALL], "%s", "VERY SMALL");
            sprintf(lines[strings_config_android_audiobuffersize_SMALL], "%s", "SMALL");
            sprintf(lines[strings_config_android_audiobuffersize_MEDIUM], "%s", "MEDIUM");
            sprintf(lines[strings_config_android_audiobuffersize_BIG], "%s", "BIG");
            sprintf(lines[strings_config_android_audiobuffersize_DESCRIPTION], "%s", "INCREASE VALUE IF AUDIO IS CHOPPY");

            sprintf(lines[strings_menu_item_CHEATS], "%s", "CHEATS");
            sprintf(lines[strings_menu_item_CHEATS_INVENCIBLE], "%s", "INVENCIBLE");
            sprintf(lines[strings_menu_item_CHEATS_ALLBEATEN], "%s", "ALL BEATEN");
            sprintf(lines[strings_menu_item_CHEATS_CHARACTER], "%s", "CHARACTER");

            sprintf(lines[strings_menu_pick_save_slot_PICK_OPTION], "%s", "PLEASE SELECT SAVE SLOT");
            sprintf(lines[strings_menu_pick_save_slot_NEW_GAME], "%s", "CREATE NEW GAME");
            sprintf(lines[strings_menu_pick_save_slot_LOAD_GAME], "%s", "LOAD GAME FILE");
            sprintf(lines[strings_menu_pick_save_slot_NO_SAVE], "%s", "- NO SAVE FILE -");

            sprintf(lines[strings_ending_NEW_CHARACTERS_AVAILABLE], "%s", "NEW CHARACTERS AVAILABLE");
            sprintf(lines[strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE], "%s", "PRESS    TO CONTINUE");


            sprintf(lines[strings_weapon_name_COIL], "%s", "FROG COIL");
            sprintf(lines[strings_weapon_name_JET], "%s", "EAGLE JET");
            sprintf(lines[strings_weapon_name_ETANK], "%s", "HP CRYSTAL");
            sprintf(lines[strings_weapon_name_WTANK], "%s", "MP CRYSTAL");
            sprintf(lines[strings_weapon_name_STANK], "%s", "SPECIAL CRYSTAL");

            sprintf(lines[strings_weapon_menu_CHANGE_WEAPON], "%s", "CHANGE WEAPON/ITEM");
            sprintf(lines[strings_weapon_menu_TANKS], "%s", "HEALTH CRYSTALS");
            sprintf(lines[strings_weapon_menu_BOOTS], "%s", "BOOTS");
            sprintf(lines[strings_weapon_menu_ARMOR], "%s", "ARMOR");
            sprintf(lines[strings_weapon_menu_WEAPON], "%s", "BEAM");
            sprintf(lines[strings_weapon_menu_PART_NORMAL], "%s", "NORMAL");
            sprintf(lines[strings_weapon_menu_PART_ENHANCED], "%s", "IMPROVED");

            sprintf(lines[strings_ingame_config_audio_use_old_music], "%s", "USE OLD MUSIC");

            sprintf(lines[strings_about_site], "%s", "- SITE -");
            sprintf(lines[strings_about_bugs], "%s", "- SUPPORT -");
            sprintf(lines[strings_about_twitter], "%s", "- TWITTER -");
            sprintf(lines[strings_about_facebook], "%s", "- FACEBOOK -");

            sprintf(lines[strings_stage_select_pick_mission], "%s", "PICK A MISSION");
            sprintf(lines[strings_stage_select_boss], "%s", "BOSS:");
            sprintf(lines[strings_stage_select_select], "%s", "SELECT");

            sprintf(lines[strings_config_android_use_mod_music], "%s", "USE CHIPTUNE MUSIC");
            sprintf(lines[strings_config_android_use_mod_music_warning], "%s", "MAY CAUSE AUDIO ISSUES");

            sprintf(lines[strings_weapon_selected], "%s", "WEAPON");

            sprintf(lines[strings_game_engine_credits_title], "%s", "GAME ENGINE CREDITS");
            sprintf(lines[strings_game_engine_credits_planning], "%s", "PLANNING");
            sprintf(lines[strings_game_engine_credits_design], "%s", "DESIGN");
            sprintf(lines[strings_game_engine_credits_dialogs], "%s", "DIALOGS");
            sprintf(lines[strings_game_engine_credits_tests], "%s", "TESTS AND REVIEW");
            sprintf(lines[strings_game_engine_credits_illustration], "%s", "ILLUSTRATION");
            sprintf(lines[strings_game_engine_credits_music], "%s", "MUSIC");
            sprintf(lines[strings_game_engine_credits_translation], "%s", "TRANSLATION");
            sprintf(lines[strings_game_engine_credits_programming], "%s", "PROGRAMMING");
            sprintf(lines[strings_game_engine_credits_ports], "%s", "PORTING");
            sprintf(lines[strings_game_engine_credits_special_thanks], "%s", "SPECIAL THANKS");
            sprintf(lines[strings_game_engine_credits_dev_tools], "%s", "DEVLOPMENT TOOLS");
            sprintf(lines[strings_game_engine_credits_presented_by], "%s", "PRESENTED BY");

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
            sprintf(char_filename, "/dialogs/stage_dialogs_%d_fr.dat", stage_id);
        } else if (language == LANGUAGE_SPANISH) {
            sprintf(char_filename, "/dialogs/stage_dialogs_%d_es.dat", stage_id);
        } else if (language == LANGUAGE_ITALIAN) {
            sprintf(char_filename, "/dialogs/stage_dialogs_%d_it.dat", stage_id);
        } else if (language == LANGUAGE_PORTUGUESE) {
            sprintf(char_filename, "/dialogs/stage_dialogs_%d_pt.dat", stage_id);
        } else {
            sprintf(char_filename, "/dialogs/stage_dialogs_%d.dat", stage_id);
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
        credits_list.push_back("");
        credits_list.push_back("");
        credits_list.push_back("");

        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_ports) + " -");
        credits_list.push_back("DINGUX: SHIN-NIL");
        credits_list.push_back("PANDORA: SIGMA NL");
        credits_list.push_back("PS2: RAGNAROK2040");
        credits_list.push_back("PS2: WOON-YUNG LIU");
        credits_list.push_back("PS2: SP193");
        credits_list.push_back("ANDROID: PELYA");
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
        credits_list.push_back("FELIPE ZACANI (FPS)");
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
        credits_list.push_back("LIBSDL");
        credits_list.push_back("DIGIA QT");
        credits_list.push_back("GCC G+");
        credits_list.push_back("UBUNTU LINUX");
        credits_list.push_back("MINT LINUX");
        credits_list.push_back("GIMP EDITOR");
        credits_list.push_back("PAINT TOOL SAI");
        credits_list.push_back("COREL DRAW");
        credits_list.push_back("SIMPLE SCREEN RECORD");
        credits_list.push_back("AUDACIOUS PLAYER");
        credits_list.push_back("AUDACITY EDITOR");
        credits_list.push_back("BFXR.NET");

        for (int i=0; i<20; i++) {
            credits_list.push_back("");
        }
        credits_list.push_back("- " + strings_map::get_instance()->get_ingame_string(strings_game_engine_credits_presented_by) + " -");
        credits_list.push_back("UPPERLAND STUDIOS");

        return credits_list;
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
                sprintf(line_value, "START-STG[%d] DLG #%d", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // phrases
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    sprintf(player_line_value, "START-STG[%d] DLG P[%d] %d", i, (j+1), k);
                    res.push_back(player_line_value);
                }
            }
            // person dialogs
            for (int j=0; j<6; j++) {
                sprintf(line_value, "BOSS STG[%d] DLG #%d\n", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // lines
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    sprintf(player_line_value, "BOSS STG[%d] DLG P[%d] %d", i, (j+1), k);
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
        std::cout << "FIO:_STRINGS::get_string_list_from_scene_text_file[" << filename << "]" << std::endl;
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
