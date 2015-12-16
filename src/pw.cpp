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
    FILE **wins;
    int nwins = 1;

    /* start with one incoming file */
    wins = (FILE**)malloc(sizeof(FILE*) * nwins);

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
    sprintf(szPulseOut, "%s_PULSE.csv", szBase);
    sprintf(szWaveIn, "%s_WAVE_IN.txt", szBase);
    sprintf(szWaveOut, "%s_WAVE_OUT.txt", szBase);

    pout = fopen(szPulseOut, "w");
    fprintf(pout, "id,");
    fprintf(pout, "gps,");
    fprintf(pout, "anchor_x,anchor_y,anchor_z,");
    fprintf(pout, "target_x,target_y,target_z,");
    fprintf(pout, "first_return_x,first_return_y,first_return_z,");
    fprintf(pout, "last_return_x,last_return_y,last_return_z,");
    fprintf(pout, "edge,scan_dir,"); /* bit, written as uint8 */
    fprintf(pout, "facet,"); /* 2 bit, written as uint8 */
    fprintf(pout, "intensity\n"); /* uint8 */

    const char szPulseFormat[] = "%lld," \
                                 "%lf," \
                                 "%lf,%lf,%lf," \
                                 "%lf,%lf,%lf," \
                                 "%lf,%lf,%lf," \
                                 "%lf,%lf,%lf," \
                                 "%d,%d," \
                                 "%d," \
                                 "%d\n";

    for(i = 0; i < nwins; i++) {
        sprintf(szWaveIn, "%s_WAVE_IN_%d.txt", szBase, i);
        wins[i] = fopen(szWaveIn, "w");
    }
    wout = fopen(szWaveOut, "w");

    double gpsTime;
    double xa, ya, za;
    double xt, yt, zt;
    double dx, dy, dz;
    double xf, yf, zf;
    double xl, yl, zl;
    unsigned char edge;
    unsigned char scan_dir;
    unsigned char intensity;
    long long p = 0;
    int rc = 0;
    int fi = 0;
    while(pReader->read_pulse()) {
        /* Write line to pulse file */
        gpsTime = pReader->pulse.T * pReader->header.t_scale_factor + pReader->header.t_offset;
        xa = pReader->pulse.anchor_X * pReader->header.x_scale_factor + pReader->header.x_offset;
        ya = pReader->pulse.anchor_X * pReader->header.y_scale_factor + pReader->header.y_offset;
        za = pReader->pulse.anchor_X * pReader->header.z_scale_factor + pReader->header.z_offset;
        xt = pReader->pulse.target_X * pReader->header.x_scale_factor + pReader->header.x_offset;
        yt = pReader->pulse.target_X * pReader->header.y_scale_factor + pReader->header.y_offset;
        zt = pReader->pulse.target_X * pReader->header.z_scale_factor + pReader->header.z_offset;
        dx = (xt - xa) / 1000;
        dy = (yt - ya) / 1000;
        dz = (zt - za) / 1000;
        xf = xa + pReader->pulse.first_returning_sample * dx;
        yf = ya + pReader->pulse.first_returning_sample * dy;
        zf = za + pReader->pulse.first_returning_sample * dz;
        xl = xa + pReader->pulse.last_returning_sample * dx;
        yl = ya + pReader->pulse.last_returning_sample * dy;
        zl = za + pReader->pulse.last_returning_sample * dz;

        edge = pReader->pulse.edge_of_scan_line;
        scan_dir = pReader->pulse.scan_direction;
        intensity = pReader->pulse.intensity;

        rc = fprintf(pout, szPulseFormat, p,
                                          gpsTime,
                                          xa, ya, za,
                                          xt, yt, zt,
                                          xf, yf, zf,
                                          xl, yl, zl,
                                          edge, scan_dir,
                                          pReader->pulse.mirror_facet,
                                          intensity);

        if(pReader->read_waves()) {
            for(i = 0; i < pReader->waves->get_number_of_samplings(); i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    fprintf(wout, "%lld ", p);
                    for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                        fi = 0;
                        fprintf(wout, "%lld ", p);
                        for(k = 0; k < sampling->get_number_of_samples(); k++) {
                            fprintf(wout, "%u ", sampling->get_sample(k));
                        }
                    }
                    fprintf(wout, "\n");
                } else { /* PULSEWAVES_INCOMING */
                    for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                        /* Need new file? */
                        if(j >= nwins) {
                            wins = (FILE**)realloc(wins, sizeof(FILE*) * ++nwins);
                            sprintf(szWaveIn, "%s_WAVE_IN_%d.txt", szBase, nwins-1);
                            wins[j] = fopen(szWaveIn, "w");
                            printf("Creating new incoming wave file for %d: %s\n",
                                   j, szWaveIn);
                        }
                        fprintf(wins[j], "%lld ", p);
                        for(k = 0; k < sampling->get_number_of_samples(); k++) {
                            fprintf(wins[j], "%u ", sampling->get_sample(k));
                        }
                        fprintf(wins[j], "\n");
                    }
                }
            }
        } else {
            /* No data??? */
        }
        p++;
    }

    free(szPulseOut);
    free(szWaveIn);
    free(szWaveOut);
    pReader->close();
    delete pReader;

    fclose(pout);
    fclose(wout);
    for(i = 0; i < nwins; i++) {
        fclose(wins[i]);
    }
    free(wins);

    return 0;
}

