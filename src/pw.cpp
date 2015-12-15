/*
** Copyright (c) 2015 Boise Center Aerospace Laboratory
**
** Author: Kyle Shannon <kyle at pobox dot com>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pulsereader.hpp"

/*
** This program opens and reads a pulse waves file, and outputs pertinent
** information for each in/out waveform.  The output should be consumable by
** matlab.
*/

void Usage() {
    printf("pw input.pls output_prefix\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    PULSEreadOpener pOpener;
    PULSEreader *pReader;
    WAVESsampling *sampling;
    int i, j, k;
    char *szPulseOut, *szWaveOut, *szWaveIn, *szBase;
    FILE *pout;
    FILE *wout;
    FILE *win;
    FILE *wave;

    if(argc < 3) {
        Usage();
    }
    pOpener.set_file_name(argv[1]);
    pReader = pOpener.open();
    if(pReader == 0)
    {
        printf("Could not open file\n");
        return 1;
    }

    /* Open n files for output */
    szPulseOut = (char*)calloc(8192, 1);
    szWaveIn = (char*)calloc(8192, 1);
    szWaveOut = (char*)calloc(8192, 1);
    szBase = argv[2];
    sprintf(szPulseOut, "%s_PULSE.txt", szBase);
    sprintf(szWaveIn, "%s_WAVE_IN.txt", szBase);
    sprintf(szWaveOut, "%s_WAVE_OUT.txt", szBase);

    pout = fopen(szPulseOut, "w");
    fprintf(pout, "id,gps,"); /* long long */
    fprintf(pout, "anchor_x,anchor_y,anchor_z,"); /* long */
    fprintf(pout, "target_x,target_y,target_z,"); /* long */
    fprintf(pout, "first_return,last_return,"); /* short */
    fprintf(pout, "edge,scan_dir,"); /* bit, written as uint8 */
    fprintf(pout, "facet,"); /* 2 bit, written as uint8 */
    fprintf(pout, "intensity\n"); /* uint8 */

    const char szPulseFormat[] = "%lld," \
                                 "%lld," \
                                 "%d,%d,%d," \
                                 "%d,%d,%d," \
                                 "%d,%d," \
                                 "%d,%d," \
                                 "%d," \
                                 "%d\n";

    win = fopen(szWaveIn, "w");
    wout = fopen(szWaveOut, "w");
    fprintf(win, "id,[[waves]]\n");
    fprintf(wout, "id,[[waves]]\n");

    long long p = 0;
    int rc = 0;
    while(pReader->read_pulse()) {
        /* Write line to pulse file */
        rc = fprintf(pout, szPulseFormat, p,
                                          pReader->pulse.T,
                                          pReader->pulse.anchor_X, pReader->pulse.anchor_Y, pReader->pulse.anchor_X,
                                          pReader->pulse.target_X, pReader->pulse.target_Y, pReader->pulse.target_X,
                                          pReader->pulse.first_returning_sample, pReader->pulse.last_returning_sample,
                                          pReader->pulse.edge_of_scan_line, pReader->pulse.scan_direction,
                                          pReader->pulse.mirror_facet,
                                          pReader->pulse.intensity);

        if(pReader->read_waves()) {
            for(i = 0; i < pReader->waves->get_number_of_samplings(); i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    wave = wout;
                } else {
                    wave = win;
                }
                fprintf(wave, "%lld,[", p);
                for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                    fprintf(wave, "[");
                    for(k = 0; k < sampling->get_number_of_samples(); k++) {
                        fprintf(wave, "%u ", sampling->get_sample(k));
                    }
                    fprintf(wave, "]");
                }
                fprintf(wave, "]\n");
            }
        } else {
            /* No data??? */
            fprintf(wave, "%lld\n", p);
        }
        p++;
    }

    free(szPulseOut);
    free(szWaveIn);
    free(szWaveOut);
    pReader->close();
    delete pReader;

    fclose(pout);
    fclose(win);
    fclose(wout);

    return 0;
}

