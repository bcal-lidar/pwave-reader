/*
** Copyright (c) 2015 Boise Center Aerospace Laboratory
**
** Author: Kyle Shannon <kyle at pobox dot com>
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pulsereader.hpp"

#define MAX(a, b) (a)>(b)?(a):(b)

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
    char *szPulseOut, *szWaveOut, *szWaveIn, *szBase, *szScanOut;
    FILE *pout;
    FILE *wout;
    FILE **wins;
    int nwins = 1;
    FILE *scanout;

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
    szScanOut = (char*)calloc(8192, 1);
    szBase = argv[2];
    sprintf(szPulseOut, "%s_PULSE.csv", szBase);
    sprintf(szWaveIn, "%s_WAVE_IN.txt", szBase);
    sprintf(szWaveOut, "%s_WAVE_OUT.txt", szBase);

    /* Scanner metadata */
    PULSEscanner scanner;
    sprintf(szScanOut, "%s_SCANNER.csv", szBase);
    scanout = fopen(szScanOut, "w");
    fprintf(scanout,
            "scanner_id,wave_length,outgoing_pulse_width,scan_pattern,"
            "number_of_mirror_facets,scan_frequency,scan_angle_min,"
            "scan_angle_max,pulse_frequency,beam_diameter_at_exit_aperture,"
            "beam_divergence,minimal_range,maximal_range\n");

    i = 1;
    while(pReader->header.get_scanner(&scanner, i)) {
        fprintf(scanout, "%d,%lf,%lf,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%s\n",
                i, scanner.wave_length, scanner.outgoing_pulse_width,
                scanner.scan_pattern, scanner.number_of_mirror_facets,
                scanner.scan_frequency, scanner.scan_angle_min,
                scanner.scan_angle_max, scanner.pulse_frequency,
                scanner.beam_diameter_at_exit_aperture, scanner.beam_divergence,
                scanner.minimal_range, scanner.maximal_range,
                scanner.description);
        i++;
    }
    fclose(scanout);

    pout = fopen(szPulseOut, "w");
    fprintf(pout, "id,");
    fprintf(pout, "gps,");
    fprintf(pout, "anchor_x,anchor_y,anchor_z,");
    fprintf(pout, "target_x,target_y,target_z,");
    fprintf(pout, "raw_first_return,raw_last_return,");
    fprintf(pout, "x_scale,y_scale,z_scale,");
    fprintf(pout, "x_offset,y_offset,z_offset,");
    fprintf(pout, "edge,scan_dir,"); /* bit, written as uint8 */
    fprintf(pout, "facet,"); /* 2 bit, written as uint8 */
    fprintf(pout, "intensity\n"); /* uint8 */

    const char szPulseFormat[] = "%lld," \
                                 "%lf," \
                                 "%lf,%lf,%lf," \
                                 "%lf,%lf,%lf," \
                                 "%lf,%lf," \
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
    double toff, tscale;
    double xa, ya, za;
    double xt, yt, zt;
    double xoff, yoff, zoff;
    double xscale, yscale, zscale;
    double dx, dy, dz;
    double xf, yf, zf;
    double xl, yl, zl;
    double rf, rl;
    unsigned char edge;
    unsigned char scan_dir;
    unsigned char intensity;
    long long p = 0;
    int rc = 0;
    int fi = 0;
    int maxCount = 0;
    int totalPoints = 0;
    int returnCount = 0;
    double progress = 0.;
    /*
    ** Scan the files for the longest pulse, we may need to pad some records
    ** with zeroes.
    */
    printf("Scanning pulses to attain max pulse length\n");
    while(pReader->read_pulse())
    {
        totalPoints++;
        if(pReader->read_waves()) {
            for(i = 0; i < pReader->waves->get_number_of_samplings(); i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    continue;
                }
                for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                    if(maxCount < sampling->get_number_of_samples()) {
                        maxCount = sampling->get_number_of_samples();
                    }
                }
            }
        }
    }
    printf("Using max count of %d for sample length\n", maxCount);
    printf("Total Points: %d\n", totalPoints);
    printf("Reported pulse count: %lld\n", pReader->p_count);
    /* Scale and Offsets */
    toff = pReader->header.t_offset;
    xoff = pReader->header.x_offset;
    yoff = pReader->header.y_offset;
    zoff = pReader->header.z_offset;

    tscale = pReader->header.t_scale_factor;
    xscale = pReader->header.x_scale_factor;
    yscale = pReader->header.y_scale_factor;
    zscale = pReader->header.z_scale_factor;

    pReader->seek(0);
    while(pReader->read_pulse()) {
        /* Write line to pulse file */
        gpsTime = (pReader->pulse.T * tscale) + toff;
        xa = pReader->pulse.anchor_X;
        ya = pReader->pulse.anchor_Y;
        za = pReader->pulse.anchor_Z;
        xt = pReader->pulse.target_X;
        yt = pReader->pulse.target_Y;
        zt = pReader->pulse.target_Z;
        dx = (xt - xa) / 1000.0;
        dy = (yt - ya) / 1000.0;
        dz = (zt - za) / 1000.0;
        rf = pReader->pulse.first_returning_sample;
        rl = pReader->pulse.last_returning_sample;

        edge = pReader->pulse.edge_of_scan_line;
        scan_dir = pReader->pulse.scan_direction;
        intensity = pReader->pulse.intensity;
        rc = fprintf(pout, szPulseFormat, p,
                                          gpsTime,
                                          xa, ya, za,
                                          xt, yt, zt,
                                          rf, rl,
                                          xscale, yscale, zscale,
                                          xoff, yoff, zoff,
                                          edge, scan_dir,
                                          pReader->pulse.mirror_facet,
                                          intensity);

        if(pReader->read_waves()) {
            returnCount = 0;
            for(i = 0; i < pReader->waves->get_number_of_samplings() && returnCount < 1; i++) {
                sampling = pReader->waves->get_sampling(i);
                if(sampling->get_type() == PULSEWAVES_OUTGOING) {
                    fprintf(wout, "%lld ", p);
                    for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                        fi = 0;
                        for(k = 0; k < maxCount; k++) {
                            if(k >= sampling->get_number_of_samples()) {
                                fprintf(wout, "%u ", 0);
                            } else {
                                fprintf(wout, "%u ", sampling->get_sample(k));
                            }
                        }
                    }
                    fprintf(wout, "\n");
                } else if(sampling->get_type() == PULSEWAVES_RETURNING) {
                    for(j = 0; j < sampling->get_number_of_segments(); j++ ) {
                        /* Need new file? */
                        if(j >= nwins) {
                            wins = (FILE**)realloc(wins, sizeof(FILE*) * ++nwins);
                            sprintf(szWaveIn, "%s_WAVE_IN_%d.txt", szBase, nwins-1);
                            wins[j] = fopen(szWaveIn, "w");
                        }
                        fprintf(wins[j], "%lld ", p);
                        for(k = 0; k < maxCount; k++) {
                            if(k >= sampling->get_number_of_samples()) {
                                fprintf(wins[j], "%u ", 0);
                            } else {
                                fprintf(wins[j], "%u ", sampling->get_sample(k));
                            }
                        }
                        fprintf(wins[j], "\n");
                        returnCount++;
                    }
                } else {
                    printf("Unknown type: %d\n", sampling->get_type());
                }
            }
        } else {
            /* No data??? */
            printf("NO DATA!\n");
            assert(0);
        }
        p++;
        if(p % 1000 == 0) {
            progress = ((double)p / (double)totalPoints) * 100.;
            printf("\r%d%% done...", (int)progress);
        }
    }
    printf("\r100%% done.\n");

    free(szPulseOut);
    free(szWaveIn);
    free(szWaveOut);
    free(szScanOut);
    pReader->close();
    delete pReader;

    fclose(pout);
    fclose(wout);
    for(i = 0; i < nwins; i++) {
        fclose(wins[i]);
    }
    free(wins);
    printf("Created %d incoming wave files\n", nwins);
    printf("Total points processed: %lld\n", p);

    return 0;
}

