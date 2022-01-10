#include <string>
#include <iostream>

#include "errors.h"
#include "enigma.h"
#include "utilities.h"

int main(int argc, char** argv)
{
    std::string message;

    try
    {

        if (argc < 4) {
            /* Require at least 1 plugboard file, 1 reflector file and 1 position file
            Note: There can be no rotors */
            std::cerr << "Insufficient number of parameters" << std::endl;
            throw INSUFFICIENT_NUMBER_OF_PARAMETERS;
        }

        std::cin >> std::ws >> message;

        /* First configuration file will specify the wiring map for the plugboard */
        std::string const pb_file = argv[1];

        /* Second file will specify the wiring map for the reflector */
        std::string const rf_file = argv[2];

        /* Next files will specify the wiring maps for the rotors */
        int num_rotors = argc - 4;
        std::string* rotor_files = new std::string[num_rotors];
        for (int i = 0; i < num_rotors; i++) {
            rotor_files[i] = argv[i + 3];
        }

        if (!num_rotors) {
            rotor_files = nullptr;
        }

        /* Last file will specify the rotors’ initial starting positions */
        std::string const rotor_pos_file = argv[argc - 1];

        Plugboard const& pb = Plugboard(pb_file);
        Reflector const& rf = Reflector(rf_file);
        RotorsAssembler rotor_assembler = RotorsAssembler(rotor_files, rotor_pos_file, num_rotors);
        Rotor* rotors = rotor_assembler.assemble();

        Enigma enigma = Enigma(pb, rotors, num_rotors, rf);

        for (char& c : message) {
            check_input(c);
            std::cout << enigma.press_key(c);
            if (DEBUG) {
                std::cout << std::endl;
            }
        }

        delete[] rotor_files;

    }
    catch (int e)
    {
        return e;
    }

    return NO_ERROR;
}