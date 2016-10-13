#include "globals.hpp"

unsigned long LFSR1, LFSR2, LFSR3;

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
    encrypt32_file(argv[1], argv[2], key, LFSR1, LFSR2, LFSR3);
    
    /*
    clock_t start, end;
    double total_t;
    LFSR1 = 0x01;
    start = clock();
    for(int i=0; i<100000000; ++i)
        shift_right_lfsr1(LFSR1);
    end = clock();
    total_t = (double)(end-start)/CLOCKS_PER_SEC;
    cout << "Speed in Mbps : " << ((100000000)/total_t)/1000000 << endl;
    */
    
    /*
    LFSR1 = 0x01;
    LFSR2 = 0x01;
    LFSR3 = 0x01;
    shift_right_lfsr1(LFSR1);
    shift_right_lfsr2(LFSR2);
    shift_right_lfsr3(LFSR3);
    cout << "LFSR1 : " << bitset<19>(LFSR1) << endl;
    cout << "LFSR2 : " << bitset<22>(LFSR2) << endl;
    cout << "LFSR3 : " << bitset<23>(LFSR3) << endl;
    */
    return 0;
}
