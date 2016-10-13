#include "globals.hpp"

bool majority(unsigned long lfsr1, unsigned long lfsr2, unsigned long lfsr3)
{
    register unsigned int sum=0;
    sum = ((lfsr1 >> LFSR1_CLK_BITP) & 0x01) + ((lfsr2 >> LFSR2_CLK_BITP) & 0x01) + ((lfsr3 >> LFSR3_CLK_BITP) & 0x01);
    return ((sum >=2) ? 1 : 0);
}

bool shift_right_lfsr1(unsigned long &lfsr)
{
    if (lfsr & 0x01)
    {
        lfsr = ( ( (lfsr ^ LFSR1_TAP_BITMASK) >> 1) | 0x40000);
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

bool conditional_shift_right_lfsr(unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
{
    register bool m=majority(lfsr1, lfsr2, lfsr3);
    register bool key_bit=0;
    if (((lfsr1 >> LFSR1_CLK_BITP) & 0x01) == m)
    {
        if (lfsr1 & 0x01)
        {
            lfsr1 = ( (lfsr1 ^ LFSR2_TAP_BITMASK) >> 1) | 0x200000;
            key_bit ^= 1;
        }
        else 
            lfsr1 >>= 1;
    }
        //shift_right_lfsr1(lfsr1);
    if (((lfsr2 >> LFSR2_CLK_BITP) & 0x01) == m)
    {
        if (lfsr2 & 0x01)
        {
            lfsr2 = ( (lfsr2 ^ LFSR2_TAP_BITMASK) >> 1) | 0x200000;
            key_bit ^= 1;
        }
        else 
            lfsr2 >>= 1;
    }
        //shift_right_lfsr2(lfsr2);
    if (((lfsr3 >> LFSR3_CLK_BITP) & 0x01) == m)
    {
        if (lfsr3 & 0x01)
        {
            lfsr3 = ( (lfsr3 ^ LFSR3_TAP_BITMASK) >> 1) | 0x400000;
            key_bit ^= 1;
        }
        else 
            lfsr3 >>= 1;
    }
        //shift_right_lfsr3(lfsr3);
    return key_bit;
}

// Clock all LFSRs without condition
void clock_all(unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
{
    shift_right_lfsr1(lfsr1);
    shift_right_lfsr2(lfsr2);
    shift_right_lfsr3(lfsr3);
}

// Initialization of A5/1 LFSRs
void a5_init(char *key, unsigned long frame_no, unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
{
    register bool bit;
    
    // Initialize LFSRs to zero
    lfsr1 = 0;
    lfsr2 = 0;
    lfsr3 = 0;
    
    // A5/1 keysetup
    for(register int i=0; i<8; ++i)
    {
        for(register int j=0; j<8; ++j)
        {
            clock_all(lfsr1, lfsr2, lfsr3);
            bit = (key[i] >> j) & 1;
            lfsr1 ^= (bit << 18);
            lfsr2 ^= (bit << 21);
            lfsr3 ^= (bit << 22);
        }
    }
    
    // Load frame number 
    for(register int i=0; i<22; ++i)
    {
        clock_all(lfsr1, lfsr2, lfsr3);
        bit = ((frame_no >> i) & 1);
        lfsr1 ^= (bit << 18);
        lfsr2 ^= (bit << 21);
        lfsr3 ^= (bit << 22);
    }
    
    // Clock for 100 cycle for key mixing
    for(register int i=0; i<100; ++i)
        conditional_shift_right_lfsr(lfsr1, lfsr2, lfsr3);
}

bool get_a5_key(unsigned long lfsr1, unsigned long lfsr2, unsigned long lfsr3)
{
    return ( (lfsr1 ^ lfsr2 ^ lfsr3) & 0x01);
}

void generate_keystream_32(unsigned long &keystream, unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
{
    register bool bit;
    keystream = 0;
    for(register int i=0; i<32; ++i)
    {
        keystream = ((keystream << 1)| conditional_shift_right_lfsr(lfsr1, lfsr2, lfsr3) ); 
        //conditional_shift_right_lfsr(lfsr1, lfsr2, lfsr3);
        //bit = get_a5_key(lfsr1, lfsr2, lfsr3);
        //keystream = ((keystream << 1)|bit); 
        //cout << bit << " : "<<  bitset<32>(keystream) << endl;
    }
}

void generate_keystream_8(unsigned char &keystream, unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
{
    register bool bit=0;
    keystream = 0;
    for(register int i=0; i<8; ++i)
    {
        conditional_shift_right_lfsr(lfsr1, lfsr2, lfsr3);
        bit = get_a5_key(lfsr1, lfsr2, lfsr3);
        keystream = ((keystream << 1)|bit);
        //cout << bit << " : "<<  bitset<8>(keystream) << endl;
    }
}

void encrypt32_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
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
    a5_init(key, FRAME_NUM, lfsr1, lfsr2, lfsr3);    
    //while(!in_file.eof())
    for(int j=0; j<1; ++j)
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
            generate_keystream_32(keystream_32, lfsr1, lfsr2, lfsr3);
            //cout << keystream_8 << endl;
            cipher_text_32 = plain_text_buffer[i] ^ keystream_32;
            //cout << cipher_text_8 << " : " << (cipher_text_8 ^ keystream_8) <<endl;
            //out_file << cipher_text_8;
        }
        end = clock();
        //temp += (end - start);
    }
    //cout << endl << endl;
    //end = clock();
    
    num_plain_text_char = NUM_CHAR;
    
    // Profiling results
    cout << "Encryption Done!" << endl;
    //total_t = (double)(temp)/(CLOCKS_PER_SEC);
    total_t = (double)(end-start)/CLOCKS_PER_SEC;
    cout << "total_t : " << total_t << endl;
    cout << "Total time for encryption : " << total_t << " seconds" <<endl;
    cout << "Speed in Mbps : " << ((num_plain_text_char*8)/total_t)/1000000 << endl;
    cout << "Total number of bits : " << (num_plain_text_char*8) << endl;
    
    delete[] plain_text_buffer;
    
    // Close files
    in_file.close();
    out_file.close();
}



void encrypt8_file(char *plaintext_filename, char *ciphertext_filename, char *key, unsigned long &lfsr1, unsigned long &lfsr2, unsigned long &lfsr3)
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
    a5_init(key, FRAME_NUM, lfsr1, lfsr2, lfsr3);    
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
            generate_keystream_8(keystream_8, lfsr1, lfsr2, lfsr3);
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
    cout << "total_t : " << total_t << endl;
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