#include <bits/stdc++.h>
using namespace std;

typedef struct {
        uint32_t magic;
        int32_t  off_str;
        int32_t  off_dat;
        uint32_t n_files;
} __attribute((packed)) pako_header_t;

typedef struct {
        int32_t  off_str;
        int32_t  file_size;
        int32_t  off_con;
        int64_t  checksum;

} __attribute((packed)) pako_file_e;


int main(int argc, char *argv[]){
    // cout << argc << " arguments " << endl;
    // cout << argv[1] << endl;
    FILE *fin;
    pako_header_t pako;
    fin = fopen(argv[1] ,"rb");
    fread(&pako.magic,sizeof(uint32_t), 1, fin);
    fread(&pako.off_str,sizeof(int32_t), 1, fin);
    fread(&pako.off_dat,sizeof(int32_t), 1, fin);
    fread(&pako.n_files,sizeof(uint32_t), 1, fin);
    
    pako_file_e entrys[pako.n_files];
    for(size_t i = 0;i < pako.n_files;i++){
        fread(&entrys[i].off_str,sizeof(int32_t), 1, fin);
        fread(&entrys[i].file_size,sizeof(int32_t), 1, fin);    
        entrys[i].file_size = __builtin_bswap32(entrys[i].file_size);
        fread(&entrys[i].off_con,sizeof(int32_t), 1, fin);
        fread(&entrys[i].checksum,sizeof(int64_t), 1, fin);
        entrys[i].checksum = __builtin_bswap64(entrys[i].checksum);
    }

    // capture filenames
    vector<string> filenames;
    unsigned char c[2];
    string name = "";
    
    fseek(fin, pako.off_str, SEEK_SET);
    for(size_t i = 0;i < pako.n_files;i++){

        while(1){
            fread(c, sizeof(unsigned char), 1, fin);
            if(c[0] == '\0'){
                break;
            }
            else{
                name += c[0];
            }
        }
        filenames.push_back(name);
        name = "";        
    }


    // Check checksum
    
    for(size_t i = 0;i < pako.n_files;i++){
        fseek(fin, pako.off_dat + entrys[i].off_con, SEEK_SET);
        unsigned char content[entrys[i].file_size];
        fread(content, sizeof(unsigned char), entrys[i].file_size, fin);

        stringstream sstream;
        // cout << content << endl;

        for(int j = 0;j < entrys[i].file_size;j++){
            if(content[j] - '\0' < 16){
                sstream << "0";
                sstream << hex << (content[j] - '\0');
            }
            else{
                sstream << hex << (content[j] - '\0');
            }
            
        }
        string res = sstream.str();
        // cout << res.length() << endl;
        if(res.length() % 16 != 0){
            for(int j = 0;j < res.length() % 16;j++){
                res += "0";
            }
        }
        // cout << res << " " << res.length() << endl;
        string buffer = "";
        int counter = 0;
        int total = res.length();
        uint64_t xor_res = 0;
        for(int j = 0;j < total;j++){
            buffer += res[j];
            counter ++;
            if(counter == 16){
                uint64_t x;
                stringstream ss;
                // cout << "Buffer is " << buffer << endl;
                ss << hex << buffer;
                // cout << "ss is " << ss.str() << endl;
                ss >> x;
                // cout << "i is " << i << endl;
                // cout << "Before: " << x << endl;
                x = __builtin_bswap64(x);
                // cout << "After: " << x << endl;
                counter = 0;
                buffer = "";
                uint64_t xor_cur = xor_res ^ x;
                xor_res = xor_cur;
                // cout << xor_res << endl;
            }
        }
        // if(filenames[i] == "checker"){
        //     cout << entrys->checksum << " " << xor_res << endl;
        // }
        if(xor_res == entrys[i].checksum){
            FILE *fout;
            chdir(argv[2]);

            int fsize = filenames[i].size();
            char fname[fsize + 1];
            // cout << filenames[i].size() << endl;
            for(int j = 0;j < fsize;j++){
                fname[j] = filenames[i][j];
                // cout << name[j];
            }
            fname[fsize] = '\0';
            fout = fopen(fname,"w"); 
            fwrite(content, entrys[i].file_size, 1, fout);
            fclose(fout);
            chdir("..");
            // cout << "Hello " << i << endl;


        }
        
    }
    
    
    
    
    
    
    // // Check contents
    // vector<string> contents;
    // for(size_t i = 0;i < pako.n_files;i++){
    //     int fsize = filenames[i].size();
    //     char fname[fsize + 1];
    //     // cout << filenames[i].size() << endl;
    //     for(int j = 0;j < fsize;j++){
    //         fname[j] = filenames[i][j];
    //         // cout << name[j];
    //     }
    //     fname[fsize] = '\0';

    //     // FILE *fout;
    //     // fout = fopen(fname,"w");        
        
    //     fseek(fin, pako.off_dat + entrys[i].off_con, SEEK_SET);
    //     char content[entrys[i].file_size];
    //     fread(content, sizeof(char), entrys[i].file_size, fin);
    //     fwrite(content, entrys[i].file_size, 1, fout);
    //     contents.push_back(content);
    //     fclose(fout);
    //     // cout << entrys[i].off_con << " " << entrys[i].file_size << endl; 

    // }    
    

    cout << "there are " << pako.n_files << " files in pak file" << endl << endl;
    cout << "     Name" <<"|"<< "File size" << endl;
    cout << "---------" << "-" << "---------" << endl;
    for(size_t i = 0;i < pako.n_files;i++){
        cout << setw(9) << filenames[i] << "|" << setw(9) << entrys[i].file_size << endl;
    }

}
