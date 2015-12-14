/*
** Copyright (c) 2015 Boise Center Aerospace Laboratory
**
** Author: Kyle Shannon <kyle at pobox dot com>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pulsereader.hpp"

#define INFILE "/home/kyle/src/bsu/lidar/data/riegl_example4.pls"

/*
** This program opens and reads a pulse waves file, and outputs pertinent
** information for each in/out waveform.  The output should be consumable by
** matlab.
*/

int main(int argc, char *argv[]) {
    PULSEreadOpener pOpener;
    PULSEreader *pReader;
    WAVESsampling *sampling;
    int i, j, k;

    pOpener.set_file_name(INFILE);
    pReader = pOpener.open();
    if(pReader == 0)
    {
        printf("Could not open file\n");
        return 1;
    }
    while(pReader->read_pulse()) {
        if(pReader->read_waves())
        {
            for(i = 0; i < pReader->waves->get_number_of_samplings(); i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    printf("Outgoing Wave:");
                } else {
                    printf("Incoming Wave(s):");
                }
                printf("\n[");
                for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                    printf(" [");
                    for(k = 0; k < sampling->get_number_of_samples(); k++) {
                        printf("%u ", sampling->get_sample(k));
                    }
                    printf("] ");
                }
                printf("]\n");
            }
        }
    }

    pReader->close();
    delete pReader;

    return 0;
}

