#include <iostream> 
#include <fstream>
#include <string.h>
#include <vector>

using namespace std;



void CloseFiles(ifstream &infile, ofstream &outfile);



int dataword_length;
int codeword_length;
int generator_length; 

int main(int argc, char* argv[]){
    //debug
    // argc = 5;
    // argv[1] = "datastream.tx";
    // argv[2] = "codedstream.rx";
    // argv[3] = "1011";
    // argv[4] = "4";
    //


    if(argc != 5){
        cout << "usage: ./crc_encoder input_file output_file generator dataword_size" << '\n';
        return 0;
    }
    
    setvbuf(stdout, NULL, _IONBF, 0); 

    ifstream infile;
    ofstream outfile;




    infile.open(argv[1], ios::binary | ios::ate);
    if(!infile.is_open()){
        cout << "input file open error." << '\n';
        CloseFiles(infile, outfile);
        return 0;
    }

    outfile.open(argv[2], ios::binary | ios::out);
    if(!outfile.is_open()){
        cout << "output file open error." << '\n';
        CloseFiles(infile, outfile);
        return 0;
    }

    string generator = argv[3];
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

    if(((string)argv[4]).compare("4") == 0){
        dataword_length = 4;
    }
    else if(((string)argv[4]).compare("8") == 0){
        dataword_length = 8;

    }
    else{
        cout << "dataword size must be 4 or 8." << '\n';
        CloseFiles(infile, outfile);
        return 0;
    }


    
    int infile_size_bits = infile.tellg();
    int outfile_size_bits;
    unsigned char padding_size_bits;

    if(dataword_length == 4){
       outfile_size_bits = 2 * infile_size_bits * (3+generator_length);
       codeword_length = 3 + generator_length;
    }
    else{
        outfile_size_bits = infile_size_bits * (7+generator_length);
        codeword_length = 7 + generator_length;
    }
    //tellg로 다 읽어서 파일을 처음부터 읽도록 해야함
    infile.clear();
    infile.seekg(0);
    if(outfile_size_bits % 8 == 0){
        padding_size_bits = 0;
    }
    else
        padding_size_bits = 8 - (outfile_size_bits % 8);
    outfile_size_bits += padding_size_bits;

    unsigned char c;
    int curr_div_bit_i = 0;
    vector<bool> codeword_list;
    for(int i = 0 ; i <(int)padding_size_bits; i++){
        codeword_list.push_back(0);
    }
   
    if(dataword_length == 4){
        for(int i = 0 ; i < codeword_length - generator_length ; i++){
            gen.push_back(0);
        }
        vector<bool> gen_copy;
        for(int i = 0 ; i < codeword_length;i++){
            gen_copy.push_back(gen[i]);
        }
        while(infile.read(reinterpret_cast<char*>(&c), sizeof(c))){
            vector<bool> codeword1, codeword2;
            unsigned char mask = 128;
            for(int i = 0 ;  i<dataword_length; i++){
                if(mask & c){
                    codeword1.push_back(1);
                }
                else{
                    codeword1.push_back(0);
                }
                mask >>= 1;
            }
            for(int i = 0 ;  i<dataword_length; i++){
                if(mask & c){
                    codeword2.push_back(1);
                }
                else{
                    codeword2.push_back(0);
                }
                mask >>= 1;
            }
            for(int i = 0 ; i < generator_length - 1; i++){
                codeword1.push_back(0);
            }
            for(int i = 0 ; i < generator_length - 1; i++){
                codeword2.push_back(0);
            }

           
            vector<bool> div = codeword1;
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
            for(int i = calc_this_bit ; i < codeword_length; i++){
                codeword1[i] = div[i];
            }
            codeword_list.insert(codeword_list.end(), codeword1.begin(), codeword1.end());
          

            div.clear();
            vector<bool>().swap(div);
            div = codeword2;

            gen.clear();
            vector<bool>().swap(gen);
            for(int i = 0 ; i < gen_copy.size();i++){
                gen.push_back(gen_copy[i]);
            }
            
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
            for(int i = calc_this_bit ; i < codeword_length; i++){
                codeword2[i] = div[i];
            }
            codeword_list.insert(codeword_list.end(), codeword2.begin(), codeword2.end());
            

            div.clear();
            vector<bool>().swap(div);
             codeword1.clear();
            vector<bool>().swap(codeword1);
             codeword2.clear();
            vector<bool>().swap(codeword2);
             gen.clear();
            vector<bool>().swap(gen);
            for(int i = 0 ; i < gen_copy.size(); i++){
                gen.push_back(gen_copy[i]);
            }
        }
       
    }

    else{
        for(int i = 0 ; i < codeword_length - generator_length ; i++){
            gen.push_back(0);
        }
        vector<bool> gen_copy;
        for(int i = 0 ; i < codeword_length;i++){
            gen_copy.push_back(gen[i]);
        }
        while(infile.read(reinterpret_cast<char*>(&c), sizeof(c))){
            vector<bool> codeword1;
            unsigned char mask = 128;
            for(int i = 0 ;  i<dataword_length; i++){
                if(mask & c){
                    codeword1.push_back(1);
                }
                else{
                    codeword1.push_back(0);
                }
                mask >>= 1;
            }
            for(int i = 0 ; i < generator_length - 1; i++){
                codeword1.push_back(0);
            }
        


            vector<bool> div = codeword1;
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
            for(int i = calc_this_bit ; i < codeword_length; i++){
                codeword1[i] = div[i];
            }

           
            codeword_list.insert(codeword_list.end(), codeword1.begin(), codeword1.end());
           

            div.clear();
            vector<bool>().swap(div);

             codeword1.clear();
            vector<bool>().swap(codeword1);
             gen.clear();
            vector<bool>().swap(gen);
            for(int i = 0 ; i < gen_copy.size(); i++){
                gen.push_back(gen_copy[i]);
            }
        }
    }




    
    //print to file
    int print_c = 0;
    unsigned char buffer;
    buffer = 0;

    outfile << padding_size_bits;


   
    for(unsigned long long  i = 0 ; i < codeword_list.size(); i++){
   
        if(codeword_list[i] == 1){
            buffer += 1;
        }
        print_c++;
        if(print_c == 8){
            // outfile.write(( char*)&buffer, sizeof(char));
            outfile << buffer;
            print_c = 0;
            buffer = 0;
        }
        buffer <<= 1;
    }

    //debug
    // ofstream checkbin;
    // checkbin.open("checkbin_encoder.txt", ios::out);
    // unsigned char mask = 128;
    // for(int i = 0; i < 8;i++){
    //     if((mask & padding_size_bits) == 0) checkbin << '0';
    //     else checkbin << '1';
    //     mask >>= 1;
    // }
    // for(int i = 0 ; i < codeword_list.size(); i++){
    //     if(codeword_list[i] == 0) checkbin << '0';
    //     else checkbin << '1';
    // }
    //

    CloseFiles(infile, outfile);
    return 0;
}

void CloseFiles(ifstream &infile, ofstream &outfile){
    infile.close();
    outfile.close();
}