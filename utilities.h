#ifndef UTILITIES
#define UTILITIES

void check_input(char const a_char);
void check_string(std::string const a_string, std::string const filename);
void plugboard_or_reflector_file_error(std::string const filename);
int map_and_count_pairs(std::string const filename, char* mapping);
void print_mapping(int* mapping);
void print_mapping(char* mapping);
std::string trim(std::string my_string);
void get_reflector_mapping(std::string const filename, char reflector_mapping[26]);
void configure_rotor(std::string const filename, int* rotor_mapping, bool notch[26]);
void get_start_positions(std::string const filename, int const num_rotors, int* rotor_positions);
void get_plugboard_mapping(std::string const filename, char mapping[26]);

#endif