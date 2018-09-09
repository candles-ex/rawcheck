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

int rawcheck(TString finName = "./data/Run010-747-021.dat"){

    std::ifstream file;
    file.open(finName, ios::in | ios::binary);

    if(file.is_open()){
        cout << "file open " << finName << endl;
    }else{
        cerr << "file open error" << endl;
        return 1;
    }

    
    unsigned char  buf[4];
    char  fileInfo[10000];
    Int_t  bufint;
    Int_t  packetSize;
    Int_t  eventNumber;

    Int_t subPacketSize;
    unsigned char   adcf[384];
    unsigned short  adcs[128];


    for(int ev=0;ev<100000;++ev){

        file.read(( char*) &buf, 4);  // e7 e7 00 00
        //cout << "Magic:"<< (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
        if( !(buf[0]==0xe7 && buf[1]==0xe7 && buf[2]==0x00 && buf[3]==0x00)){
            cout << "Magic Number Wrong" << endl;
            return -1;
        }

        file.read(( char*) &packetSize, 4); // packetSize
        packetSize = htonl(packetSize);
        //cout << "pacsize" << packetSize << endl;
        
        file.read(( char*) &buf, 4);  // Packet Flug

        if(buf[0]==0x01 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            //cout << "File Header " << endl;
            // File Header
            file.read(( char*) &fileInfo, packetSize-4);
            string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            //cout << "sHeader" << sHeader << endl;

            file.read(( char*) &buf, 4);
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
            //if(buf[0]==0xcc && buf[1]==0xcc && buf[2]==0x00 && buf[3]==0x00) cout << "packet stop" << endl;

            file.read(( char*) &eventNumber, 4); // eventNumber
            eventNumber = htonl(eventNumber);
            //cout << "ev#" << eventNumber << endl;

        } else if(buf[0]==0x02 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){ 
            //cout << "File Footer " << endl;
            //File Footer
            file.read(( char*) &fileInfo, packetSize-4);
            string sHeader(fileInfo, packetSize); //4 bytes are used for packet flag
            //cout << "sHeader" << sHeader << endl;

            file.read(( char*) &buf, 4);
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
            //if(buf[0]==0xcc && buf[1]==0xcc && buf[2]==0x00 && buf[3]==0x00) cout << "packet stop" << endl;

            file.read(( char*) &eventNumber, 4); // eventNumber
            eventNumber = htonl(eventNumber);
            //cout <<"ev#" << eventNumber << endl;

            break;

        } else if(buf[2]==0x01 && buf[3]==0x00){ 
            //Event
            //cout << "Events" << endl;
            //cout << "Used buffer:" <<(int)buf[1] << endl;

            file.read(( char*) &buf, 2);  // aaaa
            file.read(( char*) &buf, 2);  // 0 0 0 0
            file.read(( char*) &buf, 1);  // MCHs -- 04
            //cout << "used buf" << (int)buf[0] << endl;  // 04
            int numOfMch = buf[0];
            file.read(( char*) &buf, 3);  // 00 00 00

            file.read(( char*) &buf, 4);  // 00 00 00 00
            file.read(( char*) &buf, 1);  // c4
            //cout <<"nazo" <<  (int)buf[0] << endl;  // 196
            file.read(( char*) &buf, 3);  // 00 00 00

            file.read(( char*) &buf, 4);     // Event Number
            file.read(( char*) &buf, 4*32);  // Counter
            file.read(( char*) &buf, 4*5);   // Trigger
            file.read(( char*) &buf, 4*7);   // Reserve

            for(int j=0;j<numOfMch;++j){ 
                // --------> Header of MCH
                file.read(( char*) &buf, 4);
                //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
                //bb bb 00 00 

                file.read(( char*) &buf, 4);
                //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
                //cout << "num of fadc" << (int)buf[0] << endl;
                //cout << "MCH_ID"      << (int)buf[1] << endl;
                int numOfFadc = (int)buf[0];

                file.read(( char*) &buf, 4);
                file.read(( char*) &bufint, 4);
                //cout << "sub_packetSize" << htonl(bufint) << endl;
                file.read(( char*) &bufint, 4);     // Event Number
                //cout << "bufEv" << htonl(bufint) << endl;
                file.read(( char*) &buf, 4*32);  // Counter
                file.read(( char*) &buf, 4*5);   // Trigger
                file.read(( char*) &buf, 4*7);   // Reserve
                // <-------- Header of MCH

                for(int i=0;i<numOfFadc;++i){
                    file.read(( char*) &buf, 4);
                    //cout << "Event#" << ev << " fADC ID " << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
                    file.read(( char*) &bufint, 4);
                    //cout << "trigger count" << htonl(bufint) << endl;
                    file.read(( char*) &bufint, 4);
                    //cout << "Timestamp fine" << htonl(bufint) << endl;
                    file.read(( char*) &bufint, 4);
                    //cout << "Timestamp coarse" << htonl(bufint) << endl;

                    file.read(( char*) &adcf, 1*384);
                    file.read(( char*) &adcs, 2*128);

                } //1PMT end
            } //MCH end

            file.read(( char*) &buf, 4);   // FF FF BB BB
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
            file.read(( char*) &buf, 4);   // FF FF BB BB
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
            file.read(( char*) &buf, 4);   // CC CC 00 00 
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;
            file.read(( char*) &buf, 4);   // Event number
            //cout << (int)buf[0] << " " << (int)buf[1] <<  " " << (int)buf[2] <<  " " << (int)buf[3] << endl;

            //cout << "Event END" << endl;

        } // Event end
        else{ //Slow data
            //Ignore
            file.read(( char*) &fileInfo, packetSize);
            file.read(( char*) &buf, 4);   // Event number
        }
    } // Loop end


    Int_t adcf_i[384];
    Int_t adcs_i[128];

    for(int i=0;i<384;++i){ adcf_i[i] = adcf[i];}
    for(int i=0;i<128;++i){ adcs_i[i] = adcs[i]/32;}

    Int_t time[384];
    for(int i=0;i<384;++i){ time[i]=i; }

    auto tg1 = new TGraph(384, time, adcf_i);
    auto tg2 = new TGraph(128, time, adcs_i);
    auto C1 = new TCanvas();
    tg1->Draw();
    auto C2 = new TCanvas();
    tg2->Draw();




    return 0;

}

