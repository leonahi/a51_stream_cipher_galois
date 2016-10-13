#include "globals.hpp"

unsigned long LFSR1, LFSR2, LFSR3;

/*
extern bool parity(unsigned long reg);
*/
extern void shift_left_lfsr(unsigned long &reg, unsigned long mask, unsigned long taps, unsigned short int lfsr_no);
extern bool parity_8(unsigned char reg);
extern bool parity_2(unsigned char reg);
extern bool majority(unsigned long LFSR1, unsigned long LFSR2, unsigned long LFSR3);
extern void conditional_shift_left_lfsr(unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void a5_init(char *key, unsigned long frame, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void generate_keystream_32(unsigned long &keystream, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void generate_keystream_8(unsigned char &keystream, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void encrypt8_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void decrypt8_file(char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);
extern void encrypt32_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3);


bool shift_right_lfsr1(unsigned long &lfsr);
bool shift_right_lfsr2(unsigned long &lfsr);
bool shift_right_lfsr3(unsigned long &lfsr);

int main(int argc, char* argv[])
{
    unsigned long LFSR1, LFSR2, LFSR3;
    char key[8];
    
    for(int i=0; i<8; ++i)
        key[i] = argv[2][i];
    
    if(argc != 4)
    {
        cout << "Error: Input file or passphrase not found"<<endl;
        cout << "Usage: ./a5 input_filename output_filename passphrase" << endl;
        exit(1);
    }
    
    //encrypt8_file(argv[1], argv[2], key, LFSR1, LFSR2, LFSR3);
    //encrypt32_file(argv[1], argv[2], key, LFSR1, LFSR2, LFSR3);
    
    LFSR1 = 0x01;
    shift_right_lfsr1(LFSR1);
    cout << "LFSR1 : " << bitset<18>(LFSR1) << endl;
    
    return 0;
}
