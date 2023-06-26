#include <iostream> 
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;



void CloseFiles(ifstream &infile, ofstream &outfile, ofstream &resultfile);


int dataword_length;
int codeword_length;
int generator_length;
unsigned long long error_cnt = 0;

int main(int argc, char* argv[]){

    //debug
    // argc = 6;
    // argv[1] = "codedstream.rx";
    // argv[2] = "datastream.rx";
    // argv[3] = "result.txt";
    // argv[4] = "1011";
    // argv[5] = "4";
    //

    if(argc != 6){
        cout << "usage: ./crc_decoder input_file output_file result_file generator dataword_size" << '\n';
        return 0;
    }
    
    setvbuf(stdout, NULL, _IONBF, 0); 

    ifstream infile;
    ofstream outfile, resultfile;

    infile.open(argv[1], ios::binary);
    if(!infile.is_open()){
        cout << "input file open error." << '\n';
        CloseFiles(infile, outfile, resultfile);
        return 0;
    }

    outfile.open(argv[2], ios::binary | ios::out);
    if(!outfile.is_open()){
        cout << "output file open error." << '\n';
        CloseFiles(infile, outfile, resultfile);
        return 0;
    }
    
    resultfile.open(argv[3], ios::out);
    if(!resultfile.is_open()){
        cout << "result file open error." << '\n';
        CloseFiles(infile, outfile, resultfile);
        return 0;
    }

    string generator = argv[4];
    vector<bool> gen;
    while(1){
        if(generator[0] == '0'){
            generator.erase(0,1);
        }
        else break;
    }
    for(int i = 0; i < generator.size(); i++){
        if(generator[i] == '1') gen.push_back(1);
        else gen.push_back(0);
    }
    

    generator_length = generator.size();
    
   

    if(((string)argv[5]).compare("4") == 0){
        dataword_length = 4;
        codeword_length = 3 + generator_length;
        
    }
    else if(((string)argv[5]).compare("8") == 0){
        dataword_length = 8;
        codeword_length = 7 + generator_length;
    }
    else{
        cout << "dataword size must be 4 or 8." << '\n';
        CloseFiles(infile, outfile, resultfile);
        return 0;
    }

     for(int i = 0; i < codeword_length-generator_length;i++){
        gen.push_back(0);
    }
     

    vector<bool> gen_copy;
    for(int i = 0 ; i < codeword_length;i++){
        gen_copy.push_back(gen[i]);
    }

    
    


    unsigned char c;
    unsigned char padding_size_bits = 0;
    infile.read(reinterpret_cast<char*>(&padding_size_bits), sizeof(padding_size_bits));
    vector<bool> bits;
    unsigned char bitmask = 128;
    while(infile.read(reinterpret_cast<char*>(&c), sizeof(c))){
        while(bitmask != 0){
            unsigned char read_bit = bitmask & (unsigned char)c;
            if(read_bit != 0) 
                bits.push_back(1);
            else bits.push_back(0);
            bitmask >>= 1;
        }
        bitmask = 128;
    }

    //debug
    // ofstream checkbin;
    // checkbin.open("checkbin_decoder.txt");
    // unsigned char mask = 128;
    // for(int i = 0; i < 8;i++){
    //     if((mask & padding_size_bits) == 0) checkbin << '0';
    //     else checkbin << '1';
    //     mask >>= 1;
    // }
    // for(int i = 0 ; i < bits.size(); i++){
    //     if(bits[i] == 0) checkbin << '0';
    //     else checkbin << '1';
    // }
    // checkbin.close();
    //

    while (padding_size_bits > 0){
        bits.erase(bits.begin());
        padding_size_bits--;
    }
    
    
    
    int codeword_cnt = 0;
    vector<bool> codeword_buffer;
    vector<bool> div;


    int sss = 0;
    for(int i = 0; i < bits.size(); i++){
        if(codeword_cnt < codeword_length){
            codeword_buffer.push_back(bits[i]);
            codeword_cnt++;
        }
        if(codeword_cnt == codeword_length){
            sss++;
            //decode
            div = codeword_buffer;
            
            
            int calc_this_bit;

            for(calc_this_bit = 0; calc_this_bit <= codeword_length - generator_length; calc_this_bit++){
                
                if(div[calc_this_bit] == 1){
                     for(int i = 0 ; i < calc_this_bit; i++){
                        div[i] = 0;
                    }
                    for(int i = calc_this_bit ; i < codeword_length; i++){
                        div[i] = (gen[i] ^ div[i]);
                    }
                }
                
                gen.pop_back();
                gen.insert(gen.begin(), 0);
               
               
            }
            
          
            for(int i = 0 ; i < div.size(); i++){
                if(div[i] == 1){
                    error_cnt++;
                    break;
                }
            }

            
            div.clear();
            codeword_buffer.clear();
            gen.clear();
            for(int i = 0 ; i < gen_copy.size(); i++){
                gen.push_back(gen_copy[i]);
            }
            codeword_cnt = 0;

        }
    
    }

    int flag = 1;
    int data_bit_cnt = 0;
    int remainder_bit_cnt = 0;
    unsigned char dataword_buffer = 0;
    for(unsigned long long i = 0; i < bits.size(); i++){
        if(flag){
            data_bit_cnt++;
            dataword_buffer <<= 1;
            dataword_buffer += bits[i];
            if(data_bit_cnt == dataword_length){
                flag = 0;
            }
            if(data_bit_cnt == 8){
                outfile << dataword_buffer;
                dataword_buffer = 0;
                data_bit_cnt = 0;
                flag = 0;
            }
        }
        else{
            remainder_bit_cnt++;
            if(remainder_bit_cnt == codeword_length - dataword_length) {
                flag = 1;
                remainder_bit_cnt = 0;
            }
        }
    }
    
    
    
    

    resultfile << sss << ' ' << error_cnt;


    

    CloseFiles(infile, outfile, resultfile);

    //cout << bits.size();
    return 0;
}

void CloseFiles(ifstream &infile, ofstream &outfile, ofstream &resultfile){
    infile.close();
    outfile.close();
    resultfile.close();
}