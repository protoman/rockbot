#ifndef FIO_COMMON_H
#define FIO_COMMON_H

#ifdef PSP
namespace psp_platform { void log(const char *fmt, ...); }
#endif
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>

#include "../aux_tools/exception_manager.h"
#include "../aux_tools/stringutils.h"
#include "fio_diag.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

class fio_common
{
public:
    fio_common();

public:
    template <class T> std::vector<T> load_from_disk(std::string file);
    template <class T> T load_single_object_from_disk(std::string file);
    template <class T> void save_data_to_disk(std::string file, std::vector<T> data);
    template <class T> void save_single_object_to_disk(std::string file, T data);

    template <class T> void save_struct_data(std::string file, T data);
    template <class T> T load_struct_data(std::string file);
};

template <class T> void fio_common::save_struct_data(std::string file, T data) {
    std::string filename = std::string(FILEPATH) + "/" + file;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "wb");
    if (!fp) {
        std::cout << "ERROR: file_io::save_struct_data - file '" << filename << "' not found." << std::endl;
        return;
    }

    int block_size = sizeof(T);
    fio_diag::fwrite_logged(__func__, filename, &data, block_size, 1, fp);
    fio_diag::fclose_logged(__func__, filename, fp);
}

template <class T> T fio_common::load_struct_data(std::string file) {
    std::string filename = std::string(FILEPATH) + "/" + file;
    T res;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "rb");
#ifdef PSP
    psp_platform::log("load %s -> %s", filename.c_str(), fp ? "ok" : "MISSING");
#endif
    if (!fp) {
        std::cout << "ERROR: file_io::load_struct_data - file '" << filename << "' not found." << std::endl;
        return res;
    }

    size_t res_read = fio_diag::fread_logged(__func__, filename, &res, sizeof(T), 1, fp);

    if (res_read != 1) {
        std::cout << "ERROR: file_io::load_struct_data - Error reading data from scenes_list file '" << filename << "'." << std::endl;
        fclose(fp);
        exception_manager::throw_general_exception(std::string("fio_common::load_struct_data - Error reading data from file."), filename);
    }
    fclose(fp);
    return res;

}

template <class T> void fio_common::save_data_to_disk(std::string file, std::vector<T> data)
{
    std::string filename = std::string(FILEPATH) + "/" + file;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "wb");
    if (!fp) {
        std::cout << "ERROR: file_io::save_data_to_disk - file '" << filename << "' not found." << std::endl;
        return;
    }

    for (unsigned int i=0; i<data.size(); i++) {
        int block_size = sizeof(T);
        T data_in = data.at(i);
        fio_diag::fwrite_logged(__func__, filename, &data_in, block_size, 1, fp);
    }
    fio_diag::fclose_logged(__func__, filename, fp);
}


template <class T> std::vector<T> fio_common::load_from_disk(std::string file)
{
    std::string filename = std::string(FILEPATH) + "/" + file;
    std::vector<T> res;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "rb");
#ifdef PSP
    psp_platform::log("load %s -> %s", filename.c_str(), fp ? "ok" : "MISSING");
#endif
    if (!fp) {
        std::cout << "ERROR: file_io::load_from_disk - file '" << filename << "' not found." << std::endl;
        return res;
    }

    int n = 0;
#ifdef PS3
    ps3_platform::trail("load_from_disk: reading records", filename.c_str());
#endif
    while (!feof(fp) && !ferror(fp)) {
        T out;
        size_t res_read = fread(&out, sizeof(T), 1, fp);


        if (res_read != 1) {
            break;
        } else {
            res.push_back(out);
        }

        n++;
    }
#ifdef PS3
    ps3_platform::trail("load_from_disk: records read", filename.c_str());
#endif
    fclose(fp);
    return res;
}

template <class T> T fio_common::load_single_object_from_disk(std::string file)
{
    std::string filename = std::string(FILEPATH) + "/" + file;
    T res;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "rb");
#ifdef PSP
    psp_platform::log("load %s -> %s", filename.c_str(), fp ? "ok" : "MISSING");
#endif
    if (!fp) {
        std::cout << "ERROR: file_io::load_from_disk - file '" << filename << "' not found." << std::endl;
        return res;
    }

    while (!feof(fp) && !ferror(fp)) {
        T out;
        int res_read = fread(&out, sizeof(T), 1, fp);

        if (res_read == -1) {
            std::cout << "ERROR: file_io::load_from_disk - Error reading data from scenes_list file '" << filename << "'." << std::endl;
        } else if (res_read == 1) {
            res = out;
            break;
        }

    }
    fclose(fp);
    return res;
}

template <class T> void fio_common::save_single_object_to_disk(std::string file, T data_in)
{
    std::string filename = std::string(FILEPATH) + "/" + file;
    FILE *fp = fio_diag::fopen_logged(__func__, StringUtils::clean_filename(filename), "wb");
    if (!fp) {
        std::cout << "ERROR: file_io::save_single_object_to_disk - file '" << filename << "' not found." << std::endl;
        return;
    }

    int block_size = sizeof(T);
    fio_diag::fwrite_logged(__func__, filename, &data_in, block_size, 1, fp);
    fio_diag::fclose_logged(__func__, filename, fp);
}

#endif // FIO_COMMON_H


