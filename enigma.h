#include <string>
#include <iostream>
#include "errors.h"
#include "utilities.h"

int DEBUG = 0;

class Rotor {
    /* Filename containing rotor mapping. Save as data member for easy identification */
    std::string file;

    /* Character at the start position of the rotor. 0 means 'A', 1 means 'B'
    ... etc */
    int start_pos;

    /* Wiring of the rotor. 'A' maps to the first element in the mapping array, 'B'
    to the second element, ... etc. Initiliased to 0 for easy debugging */
    int mapping[26] = { 0 };

    /* If there is a notch at 'A' ('B') position, then the 0th (1st) element will be 1,
    and 0 otherwise. Another option is to store all notch positions in an array, but
    since we are trying to use simple arrays for now, we will have to access the array
    length whenever we want to check if a position has a notch */
    bool notch[26] = { 0 };

    friend class Enigma;

    // Only an Enigma machine is able to manipulate a rotor with the following logic:
    void set_start_pos(int new_start_pos) {
        this->start_pos = new_start_pos;
    }

    int forward_map(int input_pos) {
        int output_pos;
        int char_at_input_pos = (input_pos + start_pos) % 26;

        int mapped_char_index = this->mapping[char_at_input_pos];

        if (start_pos > mapped_char_index) {
            output_pos = (mapped_char_index - start_pos) + 26;
        }
        else {
            output_pos = mapped_char_index - start_pos;
        }

        return output_pos;
    }

    void increase_start_pos() {
        this->start_pos = (this->start_pos + 1) % 26;
    }

    int backward_map(int input_pos) {
        int char_at_input_pos = (input_pos + start_pos) % 26;
        int i; // inverse mapped char
        for (i = 0; this->mapping[i] != char_at_input_pos; i++) {
        };
        if (start_pos > i) {
            return (i - start_pos) + 26;
        }
        else {
            return i - start_pos;
        }
    }

public:
    Rotor() {};
    Rotor(std::string file, int start_pos) :
        file(file),
        start_pos(start_pos)  /* Configure starting position */
    {
        /* Configure rotor wiring and notch */
        configure_rotor(file, this->mapping, this->notch);
    }

    int get_start_pos() {
        return this->start_pos;
    }

    bool check_notch(int index) {
        return this->notch[index];
    }
};

class RotorsAssembler {
    /* Assembles one or more rotors */

    std::string* rotor_files;
    std::string const pos_file;
    int num_rotors = 0;
    Rotor* rotors;

public:
    RotorsAssembler(std::string* rotor_files, std::string const pos_file, int num_rotors) :
        rotor_files(rotor_files), pos_file(pos_file), num_rotors(num_rotors) {
    };
    Rotor* assemble() {
        if (num_rotors) {
            this->rotors = new Rotor[this->num_rotors];
            int* start_positions = new int[num_rotors];

            /* Position file contains position of all rotors */
            get_start_positions(this->pos_file, this->num_rotors, start_positions);

            for (int i = 0; i < num_rotors; i++) {
                /* Rotor configurations in command line and positions files are ordered
                and start from the leftmost rotor.
                We want "rotors" array to begin with rightmost rotor, as current flows
                into the rightmost rotor first. */
                int index = this->num_rotors - 1 - i;

                rotors[i] = Rotor(rotor_files[index], start_positions[index]);
            }

            /* Rotor start positions are already copied into the Rotor object */
            delete[] start_positions;
            return rotors;

        }
        else {
            return nullptr;
        }

    }
    ~RotorsAssembler() {
        if (num_rotors) {
            delete[] this->rotors;
        }
    }
};

class Plugboard {
    /* Filename containing plugboard configurations */
    std::string file;

    /* Mapping of plugboard, similar to rotor mapping, 'A' will be mapped to the first
    char in the array... etc */
    char mapping[26];

    /* Only an Enigma can use the plugboard for mapping */
    friend class Enigma;

    char backward_map(int input_pos) const {
        return this->mapping[input_pos];
    };
    int forward_map(char input_char) const {
        int input_int = static_cast<int>(input_char - 65);
        char mapped_char = this->mapping[input_int];
        return static_cast<int>(mapped_char - 65);
    };

public:
    Plugboard() {};
    Plugboard(std::string file) : file(file) {
        get_plugboard_mapping(file, mapping);
    };
};

class Reflector {
    /* Filename containing reflector configurations, similar to plugboard mapping */
    std::string file;
    char mapping[26];

    /* Only an Enigma can use the reflector for mapping */
    friend class Enigma;
    int reflect(int input_pos) const {
        char mapped_char = mapping[input_pos];
        return static_cast<int>(mapped_char - 65);
    }

public:
    Reflector() {};
    Reflector(std::string file) : file(file) {
        get_reflector_mapping(file, this->mapping);
    }
};

class Enigma {
    /* Plugboard and reflector objects can be expensive to copy. Also, their mappings
    should not change within the enigma machine */
    Plugboard const& pb;
    Rotor* rotors;
    int num_rotors;
    Reflector const& rf;

public:
    Enigma(Plugboard const& pb, Rotor* rotors, int num_rotors, Reflector const& rf) : pb(pb), rotors(rotors), num_rotors(num_rotors), rf(rf) {
    }

    int forward(char input) {
        /* Model changes in the enigma machine as current flows from right to left */

        int pos = this->pb.forward_map(input);

        if (DEBUG) {
            std::cout << "Output from plugboard at position " << pos << std::endl;
        }

        for (int i = 0; i < num_rotors; i++) {
            pos = this->rotors[i].forward_map(pos);
        }

        pos = this->rf.reflect(pos);

        if (DEBUG) {
            std::cout << "Output from reflector at position " << pos << std::endl;
        };
        return pos;
    };

    char backward(int pos) {
        /* Model changes in the enigma machine as current flows from left to right */

        if (this->num_rotors) {
            for (int i = num_rotors - 1; i >= 0; i--) {
                pos = this->rotors[i].backward_map(pos);

                if (DEBUG) {
                    std::cout << "Output from rotor " << i << " at position " << pos << std::endl;
                }
            }
        }

        char output_char = this->pb.backward_map(pos);
        if (DEBUG) {
            std::cout << "Output from plugboard is at position " << output_char << std::endl;

        }
        return output_char;
    };

    char press_key(char input) {
        if (this->num_rotors) {
            /* Every time a key is pressed, a lever would rotate the rightmost rotor one
            position before closing the electric circuit */
            this->rotors[0].increase_start_pos();

            for (int i = 0; i < this->num_rotors - 1; i++) {
                if (this->rotors[i].check_notch(this->rotors[i].get_start_pos())) {
                    /* if notch aligns with first position, then rotate next rotor */
                    int new_pos = (this->rotors[i + 1].get_start_pos() + 1) % 26;
                    this->rotors[i + 1].set_start_pos(new_pos);
                }
            }

            if (DEBUG) {
                for (int i = 0; i < this->num_rotors; i++) {
                    std::cout << "rotor " << i << " starting pos is at ";
                    std::cout << this->rotors[i].get_start_pos();
                    std::cout << std::endl;

                }
            }
        }

        return backward(forward(input));
    }
};