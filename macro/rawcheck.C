// 
//  ____                 ____ _               _    
// |  _ \ __ ___      __/ ___| |__   ___  ___| | __
// | |_) / _` \ \ /\ / / |   | '_ \ / _ \/ __| |/ /
// |  _ < (_| |\ V  V /| |___| | | |  __/ (__|   < 
// |_| \_\__,_| \_/\_/  \____|_| |_|\___|\___|_|\_\
//
// Author: K. Mizukoshi - Osaka Univ. Nucl. Exp. Lab.
// Date  : 2018 Sep. 8

#include <arpa/inet.h>

int rawcheck(TString finName = "../data/Ped010-747-001.dat"){

    std::ifstream file;
    file.open(finName, ios::in | ios::binary);

    if(file.is_open()){
        cout << "file open " << finName << endl;
    }else{
        cerr << "file open error" << endl;
        return 1;
    }

    // buffers 
    unsigned char  buf[4];
    char  fileInfo[10000];
    Int_t  bufint;
    Int_t  packetSize;
    Int_t  eventNumber;
    Int_t subPacketSize;
    unsigned char   adcf[384];
    unsigned short  adcs[128];

    // data
    unsigned char  adc2ns [74][384];
    unsigned short adc64ns[74][128];
    Int_t evId = 0;


    auto tree = new TTree("tree","tree");
    tree->Branch("adc2ns", adc2ns, "adc2ns[74][384]/b");
    tree->Branch("adc64ns", adc64ns, "adc64ns[74][128]/s");
    tree->Branch("eventID", &evId, "evId/I");


    for(int ev=0;ev<100000;++ev){

        file.read(( char*) &buf, 4);  // e7 e7 00 00
        if( !(buf[0]==0xe7 && buf[1]==0xe7 && buf[2]==0x00 && buf[3]==0x00)){
            cout << "Magic Number Wrong" << endl;
            return -1;
        }

        file.read(( char*) &packetSize, 4); // packetSize
        packetSize = htonl(packetSize);
        
        file.read(( char*) &buf, 4);  // Packet Flug

        if(buf[0]==0x01 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            // File Header
            file.read(( char*) &fileInfo, packetSize-4);
            string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            cout << "sHeader" << sHeader << endl;

            file.read(( char*) &buf, 4);
            file.read(( char*) &eventNumber, 4); // eventNumber

        } else if(buf[0]==0x02 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            //cout << "File Footer " << endl;
            //File Footer
            file.read(( char*) &fileInfo, packetSize-4);
            string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            cout << "sHeader" << sHeader << endl;

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
                    file.read(( char*) &buf, 4);
                    file.read(( char*) &bufint, 4);
                    file.read(( char*) &bufint, 4);
                    file.read(( char*) &bufint, 4);

                    file.read(( char*) &adc2ns[fpgaId], 1*384);
                    file.read(( char*) &adc64ns[fpgaId], 2*128);

                    //file.read(( char*) &adcf, 1*384);
                    //file.read(( char*) &adcs, 2*128);
                    //for(int k=0;k<384;++k){adc2ns[fpgaId][k] = adcf[k];}
                    //for(int k=0;k<128;++k){adc64ns[fpgaId][k] = adcs[k];}

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


    auto f = new TFile("out.root","recreate");
    tree->Write();
    f->Close();


    return 0;

}

