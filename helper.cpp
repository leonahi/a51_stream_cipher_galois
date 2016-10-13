#include "globals.hpp"

bool majority(unsigned long LFSR1, unsigned long LFSR2, unsigned long LFSR3)
{
    register unsigned int sum=0;
    sum = ((LFSR1 >> LFSR1_CLK_BITP) & 0x01) + ((LFSR2 >> LFSR2_CLK_BITP) & 0x01) + ((LFSR3 >> LFSR3_CLK_BITP) & 0x01);
    return ((sum >=2) ? 1 : 0);
}

bool shift_right_lfsr1(unsigned long &lfsr)
{
    if (lfsr & 0x01)
    {
        lfsr = ( (lfsr ^ LFSR1_TAP_BITMASK) >> 1) | 0x40000;
        return 1;
    }
    else 
    {
        lfsr >>= 1;
        return 0;
    }
}

bool shift_right_lfsr2(unsigned long &lfsr)
{
    if (lfsr & 0x01)
    {
        lfsr = ( (lfsr ^ LFSR2_TAP_BITMASK) >> 1) | 0x200000;
        return 1;
    }
    else 
    {
        lfsr >>= 1;
        return 0;
    }
}

bool shift_right_lfsr3(unsigned long &lfsr)
{
    if (lfsr & 0x01)
    {
        lfsr = ( (lfsr ^ LFSR3_TAP_BITMASK) >> 1) | 0x400000;
        return 1;
    }
    else 
    {
        lfsr >>= 1;
        return 0;
    }
}

// Shift LFSR left but 1 bit position
void shift_left_lfsr(unsigned long &reg, unsigned long mask, unsigned long taps, unsigned short int lfsr_no)
{
    register unsigned long tap_bits = reg & taps;
    reg = (reg << 1) & mask;
    
    if (lfsr_no == 1)   
        reg |= ( ( (reg >> TAP1_13_BITP) ^ (reg >> TAP1_16_BITP) ^ (reg >> TAP1_17_BITP) ^ (reg >> TAP1_18_BITP) ) & 0x1 );
    else if (lfsr_no == 2)
        reg |= (((reg >> TAP2_20_BITP) ^ (reg >> TAP2_21_BITP)) & 0x1);
    else
        reg |= ( ( (reg >> TAP3_07_BITP) ^ (reg >> TAP3_20_BITP) ^ (reg >> TAP3_21_BITP) ^ (reg >> TAP3_22_BITP) ) & 0x1 );
}

// Clock all LFSRs conditionaly
void conditional_shift_left_lfsr(unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    register bool m=majority(LFSR1, LFSR2, LFSR3);
    if (((LFSR1 >> LFSR1_CLK_BITP) & 0x01) == m)
        shift_left_lfsr(LFSR1, LFSR1_BITMASK, LFSR1_TAP_BITMASK, 1);
    if (((LFSR2 >> LFSR2_CLK_BITP) & 0x01) == m)
        shift_left_lfsr(LFSR2, LFSR2_BITMASK, LFSR2_TAP_BITMASK, 2);
    if (((LFSR3 >> LFSR3_CLK_BITP) & 0x01) == m)
        shift_left_lfsr(LFSR3, LFSR3_BITMASK, LFSR3_TAP_BITMASK, 3);
}

// Clock all LFSRs without condition
void clock_all(unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    shift_left_lfsr(LFSR1, LFSR1_BITMASK, LFSR1_TAP_BITMASK, 1);
    shift_left_lfsr(LFSR2, LFSR2_BITMASK, LFSR2_TAP_BITMASK, 2);
    shift_left_lfsr(LFSR3, LFSR3_BITMASK, LFSR3_TAP_BITMASK, 3);
}

// Initialization of A5/1 LFSRs
void a5_init(char *key, unsigned long frame_no, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    register bool bit;
    
    // Initialize LFSRs to zero
    LFSR1 = 0;
    LFSR2 = 0;
    LFSR3 = 0;
    
    // A5/1 keysetup
    for(register int i=0; i<8; ++i)
    {
        for(register int j=0; j<8; ++j)
        {
            clock_all(LFSR1, LFSR2, LFSR3);
            bit = (key[i] >> j) & 1;
            LFSR1 ^= bit;
            LFSR2 ^= bit;
            LFSR3 ^= bit;
        }
    }
    
    // Load frame number 
    for(register int i=0; i<22; ++i)
    {
        clock_all(LFSR1, LFSR2, LFSR3);
        bit = ((frame_no >> i) & 1);
        LFSR1 ^= bit;
        LFSR2 ^= bit;
        LFSR3 ^= bit;
    }
    
    // Clock for 100 cycle for key mixing
    for(register int i=0; i<100; ++i)
        conditional_shift_left_lfsr(LFSR1, LFSR2, LFSR3);
        
}

bool get_a5_key(unsigned long LFSR1, unsigned long LFSR2, unsigned long LFSR3)
{
    return (((LFSR1 >> LFSR1_WIDTH) ^ (LFSR2 >> LFSR2_WIDTH) ^ (LFSR3 >> LFSR3_WIDTH)) & 0x01);
}

void generate_keystream_32(unsigned long &keystream, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    register bool bit;
    keystream = 0;
    for(register int i=0; i<32; ++i)
    {
        conditional_shift_left_lfsr(LFSR1, LFSR2, LFSR3);
        bit = get_a5_key(LFSR1, LFSR2, LFSR3);
        keystream = ((keystream << 1)|bit); 
        //cout << bit << " : "<<  bitset<32>(keystream) << endl;
    }
}

void generate_keystream_8(unsigned char &keystream, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    register bool bit=0;
    keystream = 0;
    for(register int i=0; i<8; ++i)
    {
        conditional_shift_left_lfsr(LFSR1, LFSR2, LFSR3);
        bit = get_a5_key(LFSR1, LFSR2, LFSR3);
        keystream = ((keystream << 1)|bit);
        //cout << bit << " : "<<  bitset<8>(keystream) << endl;
    }
}

void encrypt32_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    clock_t start, end, temp=0;
    double total_t;
    char file_char;
    unsigned long num_plain_text_char = 0;
    unsigned long keystream_32 = 0;
    unsigned long cipher_text_32 = 0;
    unsigned long *plain_text_buffer;
    plain_text_buffer = new unsigned long[(NUM_CHAR >> 2)];
    
    ifstream in_file(plaintext_filename); 
    if(!in_file.is_open())
    {
        cout << endl << "-----------------------------------------------------------------------" << endl;
        cout << "Error: Unable to open plaintext input file" << plaintext_filename << endl;
        cout << endl << "-----------------------------------------------------------------------" << endl;
        exit(1);
    }
    
    fstream out_file;
    out_file.open(ciphertext_filename, fstream::out);
    
    /*
    while(!in_file.eof())
    {
        //in_file >> noskipws >> file_char; 
        for(int i=0; i<NUM_CHAR; ++i)
        {
            in_file.get(file_char);
            cout << file_char;
            ++num_plain_text_char;
        }
    }
    cout << "Number of plain text character : " << num_plain_text_char << endl;
    in_file.close();
    return;
    */
    cout << "Starting Encryption..." << endl;
    
    //start = clock();
    a5_init(key, FRAME_NUM, LFSR1, LFSR2, LFSR3);    
    //while(!in_file.eof())
    for(int j=0; j<10; ++j)
    {
        for(int i=0; i<(NUM_CHAR>>2); ++i)
        {
            for(int j=0; j<4; ++j)
            {
                in_file.get(file_char);
                plain_text_buffer[i] = ( (plain_text_buffer[i] << 8) | file_char);
                ++num_plain_text_char;                
            }
        }
        start = clock();
        for(int i=0; i<(NUM_CHAR>>2); ++i)
        {
            generate_keystream_32(keystream_32, LFSR1, LFSR2, LFSR3);
            //cout << keystream_8 << endl;
            cipher_text_32 = plain_text_buffer[i] ^ keystream_32;
            //cout << cipher_text_8 << " : " << (cipher_text_8 ^ keystream_8) <<endl;
            //out_file << cipher_text_8;
        }
        end = clock();
        temp += (end - start);
    }
    //cout << endl << endl;
    //end = clock();
    
    // Profiling results
    cout << "Encryption Done!" << endl;
    total_t = (double)(temp)/(CLOCKS_PER_SEC*10);
    //total_t = (double)(end-start)/CLOCKS_PER_SEC;
    cout << "Total time for encryption : " << total_t << " seconds" <<endl;
    cout << "Speed in Mbps : " << ((num_plain_text_char*8)/total_t)/1000000 << endl;
    cout << "Total number of bits : " << (num_plain_text_char*8) << endl;
    
    delete[] plain_text_buffer;
    
    // Close files
    in_file.close();
    out_file.close();
}



void encrypt8_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    clock_t start, end;
    double total_t;
    char file_char;
    unsigned long num_plain_text_char = 0;
    unsigned char keystream_8 = 0;
    unsigned char cipher_text_8 = 0;
    unsigned char *plain_text_buffer;
    plain_text_buffer = new unsigned char[NUM_CHAR];
    
    ifstream in_file(plaintext_filename); 
    if(!in_file.is_open())
    {
        cout << endl << "-----------------------------------------------------------------------" << endl;
        cout << "Error: Unable to open plaintext input file" << plaintext_filename << endl;
        cout << endl << "-----------------------------------------------------------------------" << endl;
        exit(1);
    }
    
    fstream out_file;
    out_file.open(ciphertext_filename, fstream::out);
    
    /*
    while(!in_file.eof())
    {
        //in_file >> noskipws >> file_char; 
        for(int i=0; i<NUM_CHAR; ++i)
        {
            in_file.get(file_char);
            cout << file_char;
            ++num_plain_text_char;
        }
    }
    cout << "Number of plain text character : " << num_plain_text_char << endl;
    in_file.close();
    return;
    */
    cout << "Starting Encryption..." << endl;
    
    //start = clock();
    a5_init(key, FRAME_NUM, LFSR1, LFSR2, LFSR3);    
    //while(!in_file.eof())
    for(int j=0; j<1; ++j)
    {
        
        for(int i=0; i<NUM_CHAR; ++i)
        {
            //in_file >> noskipws >> file_char;
            //in_file >> noskipws >> file_char;
            in_file.get(file_char);
            plain_text_buffer[i] = file_char;
            //cout << plain_text_buffer[i] << " : ";
            ++num_plain_text_char;
        }
        start = clock();
        for(int i=0; i<NUM_CHAR; ++i)
        {
            generate_keystream_8(keystream_8, LFSR1, LFSR2, LFSR3);
            //cout << keystream_8 << endl;
            cipher_text_8 = plain_text_buffer[i] ^ keystream_8;
            //cout << cipher_text_8 << " : " << (cipher_text_8 ^ keystream_8) <<endl;
            //out_file << cipher_text_8;
        }
        end = clock();
    }
    //cout << endl << endl;
    //end = clock();
    
    // Profiling results
    cout << "Encryption Done!" << endl;
    total_t = (double)(end-start)/CLOCKS_PER_SEC;
    cout << "Total time for encryption : " << total_t << " seconds" <<endl;
    cout << "Speed in Mbps : " << ((num_plain_text_char*8)/total_t)/1000000 << endl;
    cout << "Total number of bits : " << (num_plain_text_char*8) << endl;
    
    delete[] plain_text_buffer;
    
    // Close files
    in_file.close();
    out_file.close();
}

void decrypt8_file(char *ciphertext_filename, char *key, unsigned long &LFSR1, unsigned long &LFSR2, unsigned long &LFSR3)
{
    clock_t start, end;
    double total_t;
    register char file_char;
    register unsigned long num_plain_text_char = 0;
    register unsigned char keystream_8 = 0;
    register unsigned char plain_text_8 = 0;
    unsigned char *text_buffer;
    text_buffer = new unsigned char[NUM_CHAR];
    
    ifstream in_file(ciphertext_filename); 
    if(!in_file.is_open())
    {
        cout << endl << "-----------------------------------------------------------------------" << endl;
        cout << "Error: Unable to open cipher text input file" << ciphertext_filename << endl;
        cout << endl << "-----------------------------------------------------------------------" << endl;
        exit(1);
    }
    
    fstream out_file;
    out_file.open("decrypt_file.txt", fstream::out);
    
    cout << "Starting Decryption..." << endl;
    
    start = clock();
    a5_init(key, FRAME_NUM, LFSR1, LFSR2, LFSR3);
    
    while(!in_file.eof())
    {
        for(int i=0; i<NUM_CHAR; ++i)
        {
            in_file >> noskipws >> file_char;
            //in_file >> noskipws >> file_char;
            //in_file.get(file_char);
            text_buffer[i] = file_char;
            //cout << text_buffer[i] << " : ";
            ++num_plain_text_char;
        }
        for(int i=0; i<NUM_CHAR; ++i)
        {
            generate_keystream_8(keystream_8, LFSR1, LFSR2, LFSR3);
            plain_text_8 = text_buffer[i] ^ keystream_8;
            //cout << plain_text_8 << endl;
            out_file << plain_text_8;
        }
    }
    end = clock();
    
    // Profiling results
    cout << "Decryption Done!" << endl;
    total_t = (double)(end-start)/CLOCKS_PER_SEC;
    cout << "Total time for decryption : " << total_t << " seconds" <<endl;
    cout << "Total number of bits : " << (num_plain_text_char*8) << endl;
    
    delete[] text_buffer;
    
    // Close files
    in_file.close();
    out_file.close();
}