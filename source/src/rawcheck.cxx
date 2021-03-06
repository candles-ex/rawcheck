// 
//  ____                 ____ _               _    
// |  _ \ __ ___      __/ ___| |__   ___  ___| | __
// | |_) / _` \ \ /\ / / |   | '_ \ / _ \/ __| |/ /
// |  _ < (_| |\ V  V /| |___| | | |  __/ (__|   < 
// |_| \_\__,_| \_/\_/  \____|_| |_|\___|\___|_|\_\
//
// Author: K. Mizukoshi - Osaka Univ. Nucl. Exp. Lab.
// Date  : 2018 Sep. 8

#include <iostream>
#include <fstream>
#include <string>
#include <arpa/inet.h>
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TString.h>

int main(int argc, char** argv){

    TString finName;
    TString inDirName  = "/Users/mzks/candles/rawcheck/data/";
    TString outDirName = "/Users/mzks/candles/rawcheck/rootfile/";

    if (argc >= 2){
        finName = argv[1];
    }
    if (argc >= 3){
        outDirName = argv[2];
    }
    if (argc == 4){
        inDirName = argv[3];
    }
    if (argc <= 1 || argc >= 5){
        std::cout << "Usage : % ./rawcheck <filename> (output dir) (input dir)" << std::endl;
        std::cout << "e.g., ./rawcheck Run010-747-021 ./output/ ./input/" << std::endl;
        return -1;
    }

    std::ifstream file;
    file.open(inDirName+finName+".dat", std::ios::in | std::ios::binary);

    if(file.is_open()){
        std::cout << "file open " << finName << std::endl;
    }else{
        std::cerr << "file open error" << std::endl;
        return 1;
    }

    // buffers 
    unsigned char  buf[4];
    char  fileInfo[10000];
    Int_t  bufint;
    Int_t  packetSize;
    Int_t  eventNumber;

    // data
    unsigned char  adc2ns [74][384];
    unsigned short adc64ns[74][128];
    Int_t evId = 0;


    auto tree = new TTree("tree","tree");
    tree->Branch("eventID", &evId, "evId/I");
    tree->Branch("adc2ns", adc2ns, "adc2ns[74][384]/b");
    tree->Branch("adc64ns", adc64ns, "adc64ns[74][128]/s");


    for(int ev=0;ev<10000000;++ev){

        if(ev%1000==0)std::cout << "Event #" << ev << std::endl;

        file.read(( char*) &buf, 4);  // e7 e7 00 00
        if( !(buf[0]==0xe7 && buf[1]==0xe7 && buf[2]==0x00 && buf[3]==0x00)){
            std::cout << "Magic Number Wrong" << std::endl;
            return -1;
        }

        file.read(( char*) &packetSize, 4); // packetSize
        packetSize = htonl(packetSize);
        
        file.read(( char*) &buf, 4);  // Packet Flug

        if(buf[0]==0x01 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            // File Header
            file.read(( char*) &fileInfo, packetSize-4);
            std::string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            std::cout << "sHeader" << sHeader << std::endl;

            file.read(( char*) &buf, 4);
            file.read(( char*) &eventNumber, 4); // eventNumber

        } else if(buf[0]==0x02 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            //std::cout << "File Footer " << std::endl;
            //File Footer
            file.read(( char*) &fileInfo, packetSize-4);
            std::string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            std::cout << "sHeader" << sHeader << std::endl;

            file.read(( char*) &buf, 4);
            file.read(( char*) &eventNumber, 4); // eventNumber
            break;

        } else if(buf[2]==0x01 && buf[3]==0x00){ 
            //Event

            file.read(( char*) &buf, 2);  // aaaa
            file.read(( char*) &buf, 2);  // 0 0 0 0
            file.read(( char*) &buf, 1);  // MCHs -- 04
            int numOfMch = buf[0];
            file.read(( char*) &buf, 3);  // 00 00 00
            file.read(( char*) &buf, 4);  // 00 00 00 00
            file.read(( char*) &buf, 1);  // c4
            file.read(( char*) &buf, 3);  // 00 00 00
            file.read(( char*) &buf, 4);     // Event Number
            file.read(( char*) &buf, 4*32);  // Counter
            file.read(( char*) &buf, 4*5);   // Trigger
            file.read(( char*) &buf, 4*7);   // Reserve

            Int_t fpgaId = 0;

            for(int j=0;j<numOfMch;++j){ 
                // --------> Header of MCH
                file.read(( char*) &buf, 4);

                file.read(( char*) &buf, 4);
                int numOfFadc = (int)buf[0];

                file.read(( char*) &buf, 4);
                file.read(( char*) &bufint, 4);
                file.read(( char*) &bufint, 4);     // Event Number
                file.read(( char*) &buf, 4*32);  // Counter
                file.read(( char*) &buf, 4*5);   // Trigger
                file.read(( char*) &buf, 4*7);   // Reserve
                // <-------- Header of MCH

                for(int i=0;i<numOfFadc;++i){
                    file.read(( char*) &buf, 4);    // FADC_ID 00 00 00
                    file.read(( char*) &bufint, 4); // Trigger Counts
                    file.read(( char*) &bufint, 4); // Timestamp Fine
                    file.read(( char*) &bufint, 4); // Timestamp Coarse

                    file.read(( char*) &adc2ns[fpgaId], 1*384);
                    file.read(( char*) &adc64ns[fpgaId], 2*128);

                    fpgaId++;
                } //1PMT end
            } //MCH end

            file.read(( char*) &buf, 4);   // FF FF BB BB
            file.read(( char*) &buf, 4);   // FF FF BB BB
            file.read(( char*) &buf, 4);   // CC CC 00 00 
            file.read(( char*) &buf, 4);   // Event number

            evId++;
            tree->Fill();

        } // Event end

        else{ //Slow data
            //Ignore
            file.read(( char*) &fileInfo, packetSize);
            file.read(( char*) &buf, 4);   // Event number
        }
    } // Loop end


    auto f = new TFile(outDirName+finName+".root","recreate");
    tree->Write();
    f->Close();


    return 0;

}

