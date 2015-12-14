
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pulsereader.hpp"

#define INFILE "/home/kyle/src/bsu/lidar/data/riegl_example4.pls"

int main(int argc, char *argv[]) {
    PULSEreadOpener pOpener;
    PULSEreader *pReader;

    pOpener.set_file_name(INFILE);
    pReader = pOpener.open();
    if(pReader == 0)
    {
        printf("Could not open file\n");
        return 1;
    }
    WAVESsampling *sampling;
    while(pReader->read_pulse()) {
        if(pReader->read_waves())
        {
            for(int i = 0; i < pReader->waves->get_number_of_samplings(); i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    printf("Outgoing Wave:");
                } else {
                    printf("Incoming Wave:");
                }
                printf("\n[");
                for(int j = 0; j < sampling->get_number_of_segments(); j++ ) {
                    for(int k = 0; k < sampling->get_number_of_samples(); k++) {
                        printf("%u ", sampling->get_sample(k));
                    }
                }
                printf("]\n");
            }
        }
    }

    pReader->close();
    delete pReader;

    return 0;
}

