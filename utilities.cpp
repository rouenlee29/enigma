#include <fstream>
#include <string>
#include <iostream>
#include "errors.h"

std::string trim(std::string my_string)
{
    std::string modified_str = "";
    for (char& c : my_string)
    {
        if (c != '\n')
        {
            modified_str += c;
        }
    }
    return modified_str;
}

void check_input(char const a_char) {
    if (a_char < 65 || a_char > 90) {
        std::cerr << "ERROR: input characters must be upper case letters A - Z";
        std::cerr << "; found " << a_char << std::endl;
        throw INVALID_INPUT_CHARACTER;
    }
};

void check_string(std::string const a_string, std::string const filename)
{
    for (char c : a_string)
    {

        if (c < 48 || c > 57)
        {
            std::cerr << "ERROR in " << filename << " : encountered non numeric character " << c << std::endl;
            throw NON_NUMERIC_CHARACTER;
        }
    }

    if (stoi(a_string) < 0 || stoi(a_string) > 25)
    {
        std::cerr << "ERROR in " << filename << " : invalid index " << a_string;
        std::cerr << " ; number must be between and including 0 to 25" << std::endl;
        throw INVALID_INDEX;
    }
}

void plugboard_or_reflector_file_error(std::string const filename)
{
    std::string filetype = filename.substr(filename.size() - 2, 2);

    if (filetype == "pb")
    {
        throw IMPOSSIBLE_PLUGBOARD_CONFIGURATION;
    }

    if (filetype == "rf")
    {
        throw INVALID_REFLECTOR_MAPPING;
    }
}

int map_and_count_pairs(std::string const filename, char mapping[26])
{
    /* Identify and count pairs of numbers in filename. Used to identify reflector
    and plugboard mappings. */

    std::string a_string;
    int counter = 0;
    std::ifstream in(filename);

    /* Characters mapped to itself will not be listed in the plugboard configuration file.
    Also, for both reflector and plugboard files, we need to flag (illegal) attempts that
    map a character to more than one character.
    To track if a character has been mapped (or not) in the configuration file, it is
    important to initialise the mapping array with zeros. Since we have chosen the mapping
    to contain char, zero will not correspond to any upper case alphabets. */
    for (int i = 0; i < 26; i++) {
        mapping[i] = 0;
    }
    std::string previous;

    if (!in) {
        std::cerr << "ERROR: unable to open " << filename << std::endl;
        throw ERROR_OPENING_CONFIGURATION_FILE;
    }

    while (std::getline(in, a_string, ' '))
    {
        a_string = trim(a_string); /* remove \n, or any additional empty spaces */

        /* if input stream contains character '\n', a_string will be an empty string */
        if (!a_string.empty()) {
            counter++;
            check_string(a_string, filename);

            if (counter % 2 == 0)
            {
                if (previous == a_string)
                {
                    /* Trying to map a character to itself */
                    std::cerr << "ERROR in " << filename << " : tried to map " << a_string << " with itself" << std::endl;
                    plugboard_or_reflector_file_error(filename);
                }

                if (mapping[stoi(previous)] || mapping[stoi(a_string)])
                    /* Note: mapping[stoi(a_string)] condition is required to catch
                    illegal mappings like: 3 8 1 3 */
                {
                    std::cerr << "ERROR in " << filename << " : " << previous << " or " << a_string;
                    std::cerr << " already has a mapping" << std::endl;
                    plugboard_or_reflector_file_error(filename);
                }
                mapping[stoi(previous)] = static_cast<char>(stoi(a_string) + 65);
                mapping[stoi(a_string)] = static_cast<char>(stoi(previous) + 65);
            }
            previous = a_string;
        }
    }

    in.close();

    return counter;
}

void print_mapping(int* mapping)
{
    for (int i = 0; i < 26; i++)
    {
        std::cout << static_cast<char>(i + 65) << " : " << mapping[i];
        std::cout << std::endl;
    }
}

void print_mapping(char* mapping)
{
    for (int i = 0; i < 26; i++)
    {
        std::cout << static_cast<char>(i + 65) << " : " << mapping[i];
        std::cout << std::endl;
    }
}

void configure_rotor(std::string const filename, int* rotor_mapping, bool notch[26])
{
    /* Reads rotor configuration file, and saves rotors mapping and notch information */

    std::ifstream in(filename);
    std::string a_string;
    int number;
    int counter = 0; /* count number of integers in file */


    /* A position is NOT a notch UNLESS specified by configuration file. Hence, we must
    ensure that initial notch configuration is set to false */
    for (int i = 0; i < 26;i++) {
        notch[i] = 0;
    }

    if (!in) {
        std::cerr << "ERROR: unable to open " << filename << std::endl;
        throw ERROR_OPENING_CONFIGURATION_FILE;
    }

    bool numbers[26] = { 0 };

    while (!in.eof())
    {

        in >> a_string;
        a_string = trim(a_string); /* file might contain '\n' */

        check_string(a_string, filename);
        number = stoi(a_string);

        if (counter < 26)
        {
            /* first 26 numbers are rotors mappings */
            if (numbers[number]) {
                /* if a number appears more than once, it means we are trying to ...
                map multiple inputs to the same number */
                std::cerr << "ERROR in " << filename << " : " << number << " appears ";
                std::cerr << "more than once " << std::endl;
                throw INVALID_ROTOR_MAPPING;
            }
            /* keep track of which number has already appeared in file */
            numbers[number] = 1;
            rotor_mapping[counter] = number;
        }
        else
        {
            notch[number] = 1;
        }
        counter++;
    }

    in.close();

    if (counter < 26) {
        /* All 26 characters in the alphabet must have a mapping.
        It is valid for rotors to have no notch. Hence, there must be at least 26
        integers in the file */
        std::cerr << "ERROR in " << filename << " : " << "only " << counter << " numbers";
        std::cerr << std::endl;
        throw INVALID_ROTOR_MAPPING;
    }
}

void get_start_positions(std::string const filename, int const num_rotors, int* rotor_positions)
{
    std::ifstream in(filename);
    std::string a_string;

    int string_count = 0;

    if (!in) {
        std::cerr << "ERROR: unable to open " << filename << std::endl;
        throw ERROR_OPENING_CONFIGURATION_FILE;
    }

    while (std::getline(in, a_string, ' '))
    {
        a_string = trim(a_string);

        /* if file contains '\n', it will be trimmed to an empty string */
        if (!a_string.empty()) {
            check_string(a_string, filename);
            rotor_positions[string_count++] = stoi(a_string);
        }

    }

    if (string_count < num_rotors)
    {
        std::cerr << "ERROR in " << filename << " : " << num_rotors << " rotor(s) defined";
        std::cerr << ", but " << string_count << " position(s) found" << std::endl;
        throw NO_ROTOR_STARTING_POSITION;
    }

    in.close();
}


void get_reflector_mapping(std::string const filename, char reflector_mapping[26])
{
    int num_maps = map_and_count_pairs(filename, reflector_mapping);
    if (num_maps != 26)
    {
        std::cerr << "ERROR in " << filename << " : file has " << num_maps << " numbers" << std::endl;
        throw INCORRECT_NUMBER_OF_REFLECTOR_PARAMETERS;
    }
}

void get_plugboard_mapping(std::string const filename, char mapping[26])
{
    int num_maps = map_and_count_pairs(filename, mapping);
    if (num_maps % 2)
    {
        std::cerr << "ERROR: odd number of plugboard parameters" << std::endl;
        throw INCORRECT_NUMBER_OF_PLUGBOARD_PARAMETERS;
    }

    for (int i = 0; i < 26; i++) {
        if (!mapping[i]) {
            /* Character is not mapped, i.e. it will be mapped to itself. */
            mapping[i] = static_cast<char>(i + 65);
        }
    }

};
