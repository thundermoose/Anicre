#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import re
import sys

nucleus = "6Li";

if (len(sys.argv) > 1):
    nucleus = sys.argv[1]

Nmax = np.array([0, 2, 4, 6, 8, 10, 12]);

total_mp_states = Nmax * np.nan;
max_mp_block = Nmax * np.nan;

sum_conn_len_1n = Nmax * np.nan;
sum_conn_len_2n = Nmax * np.nan;
sum_conn_len_3n = Nmax * np.nan;

max_conn_len_1n = Nmax * np.nan;
max_conn_len_2n = Nmax * np.nan;
max_conn_len_3n = Nmax * np.nan;

load_conn_len_1n = Nmax * np.nan;
load_conn_len_2n = Nmax * np.nan;
load_conn_len_3n = Nmax * np.nan;

sum_Vc_size_1n = Nmax * np.nan;
sum_Vc_size_2n = Nmax * np.nan;
sum_Vc_size_3n = Nmax * np.nan;
sum_Vx_size_1n = Nmax * np.nan;
sum_Vx_size_2n = Nmax * np.nan;
sum_Vx_size_3n = Nmax * np.nan;

max_Vc_size_1n = Nmax * np.nan;
max_Vc_size_2n = Nmax * np.nan;
max_Vc_size_3n = Nmax * np.nan;
max_Vx_size_1n = Nmax * np.nan;
max_Vx_size_2n = Nmax * np.nan;
max_Vx_size_3n = Nmax * np.nan;

load_Vc_size_1n = Nmax * np.nan;
load_Vc_size_2n = Nmax * np.nan;
load_Vc_size_3n = Nmax * np.nan;
load_Vx_size_1n = Nmax * np.nan;
load_Vx_size_2n = Nmax * np.nan;
load_Vx_size_3n = Nmax * np.nan;

conns_p     = Nmax * np.nan;
conns_n     = Nmax * np.nan;
conns_pp    = Nmax * np.nan;
conns_nn    = Nmax * np.nan;
conns_ppp   = Nmax * np.nan;
conns_nnn   = Nmax * np.nan;
conns_pn    = Nmax * np.nan;
conns_ppn   = Nmax * np.nan;
conns_pnn   = Nmax * np.nan;

regex = re.compile("(TOTAL-MP-STATES|MAX-MP-BLOCK|"+\
                       "SUM-CONN-LEN-1N|MAX-CONN-LEN-1N|LOAD-CONN-LEN-1N|"+\
                       "SUM-CONN-LEN-2N|MAX-CONN-LEN-2N|LOAD-CONN-LEN-2N|"+\
                       "SUM-CONN-LEN-3N|MAX-CONN-LEN-3N|LOAD-CONN-LEN-3N|"+\
                       "SUM-Vc-SIZE-1N|MAX-Vc-SIZE-1N|LOAD-Vc-SIZE-1N|"+\
                       "SUM-Vc-SIZE-2N|MAX-Vc-SIZE-2N|LOAD-Vc-SIZE-2N|"+\
                       "SUM-Vc-SIZE-3N|MAX-Vc-SIZE-3N|LOAD-Vc-SIZE-3N|"+\
                       "SUM-Vx-SIZE-1N|MAX-Vx-SIZE-1N|LOAD-Vx-SIZE-1N|"+\
                       "SUM-Vx-SIZE-2N|MAX-Vx-SIZE-2N|LOAD-Vx-SIZE-2N|"+\
                       "SUM-Vx-SIZE-3N|MAX-Vx-SIZE-3N|LOAD-Vx-SIZE-3N|"+\
                       "CONNECTIONS-(p-n|pp-n|p-nn|DIA-(p|n|pp|nn|ppp|nnn)))"+\
                       ":\s*([0-9]+)")

gr = Nmax * np.nan;
gr = [[gr, gr, gr], [gr, gr, gr], [gr, gr, gr]];

num_blocks = gr;
raw_load_size = gr;
max_block_size = gr;
num_arrays = gr;
sum_array_size = gr;
greedy_load_size = gr;

gregex = re.compile("(NUM-BLOCKS|RAW-LOAD-SIZE|MAX-BLOCK-SIZE|"+\
                        "NUM-ARRAYS|SUM-ARRAY-SIZE|GREEDY-LOAD-SIZE)"+\
                        "-([0-9]+)-([0-9]+):\s*([0-9]+)")

for i in range(0,len(Nmax)):
    nmax = Nmax[i]
    print nmax

    filename = nucleus+"_nmax%d.td/comb.txt" % nmax

    try:
        with open(filename) as f:
            for line in f:
                match = regex.findall(line)
                if match:
                    if (match[0][0] == "TOTAL-MP-STATES"):
                        total_mp_states[i] = int(match[0][3])
                    if (match[0][0] == "MAX-MP-BLOCK"):
                        max_mp_block[i] = int(match[0][3])

                    if (match[0][0] == "SUM-CONN-LEN-1N"):
                        sum_conn_len_1n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-CONN-LEN-2N"):
                        sum_conn_len_2n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-CONN-LEN-3N"):
                        sum_conn_len_3n[i] = int(match[0][3])

                    if (match[0][0] == "MAX-CONN-LEN-1N"):
                        max_conn_len_1n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-CONN-LEN-2N"):
                        max_conn_len_2n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-CONN-LEN-3N"):
                        max_conn_len_3n[i] = int(match[0][3])

                    if (match[0][0] == "LOAD-CONN-LEN-1N"):
                        load_conn_len_1n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-CONN-LEN-2N"):
                        load_conn_len_2n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-CONN-LEN-3N"):
                        load_conn_len_3n[i] = int(match[0][3])

                    if (match[0][0] == "SUM-Vc-SIZE-1N"):
                        sum_Vc_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-Vc-SIZE-2N"):
                        sum_Vc_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-Vc-SIZE-3N"):
                        sum_Vc_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "MAX-Vc-SIZE-1N"):
                        max_Vc_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-Vc-SIZE-2N"):
                        max_Vc_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-Vc-SIZE-3N"):
                        max_Vc_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "LOAD-Vc-SIZE-1N"):
                        load_Vc_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-Vc-SIZE-2N"):
                        load_Vc_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-Vc-SIZE-3N"):
                        load_Vc_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "SUM-Vx-SIZE-1N"):
                        sum_Vx_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-Vx-SIZE-2N"):
                        sum_Vx_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "SUM-Vx-SIZE-3N"):
                        sum_Vx_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "MAX-Vx-SIZE-1N"):
                        max_Vx_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-Vx-SIZE-2N"):
                        max_Vx_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "MAX-Vx-SIZE-3N"):
                        max_Vx_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "LOAD-Vx-SIZE-1N"):
                        load_Vx_size_1n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-Vx-SIZE-2N"):
                        load_Vx_size_2n[i] = int(match[0][3])
                    if (match[0][0] == "LOAD-Vx-SIZE-3N"):
                        load_Vx_size_3n[i] = int(match[0][3])

                    if (match[0][0] == "CONNECTIONS-DIA-p"):
                        conns_p[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-DIA-n"):
                        conns_n[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-DIA-pp"):
                        conns_pp[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-DIA-nn"):
                        conns_nn[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-DIA-ppp"):
                        conns_ppp[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-DIA-nnn"):
                        conns_nnn[i] = int(match[0][3])

                    if (match[0][0] == "CONNECTIONS-p-n"):
                        conns_pn[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-pp-n"):
                        conns_ppn[i] = int(match[0][3])
                    if (match[0][0] == "CONNECTIONS-p-nn"):
                        conns_pnn[i] = int(match[0][3])
    except:
        print "Could not open / handle '%s'" % filename;

    for nforce in [1, 2, 3]:
        for blocksz in [1, 8, 16]:

            if (nforce == 1):
                nforcei = 0;
            if (nforce == 2):
                nforcei = 1;
            if (nforce == 3):
                nforcei = 2;
            if (blocksz == 1):
                blockszi = 0;
            if (blocksz == 8):
                blockszi = 1;
            if (blocksz == 16):
                blockszi = 2;

            filename = nucleus+"_nmax%d.td/greedy_%d_%d.txt" % \
                (nmax, nforce, blocksz)

            try:
                with open(filename) as f:
                    for line in f:
                        match = gregex.findall(line)
                        if match:
                            if (int(match[0][1]) != nforce or
                                int(match[0][2]) != blocksz):
                                print "(%d,%d) (%s,%s)" % (nforce, blocksz, match[0][1], match[0][2]);
                                raise "File nforce / blocksz mismatch";
                            
                            if (match[0][0] == "NUM-BLOCKS"):
                                num_blocks[nforcei][blockszi][i] = int(match[0][3])
                            if (match[0][0] == "RAW-LOAD-SIZE"):
                                raw_load_size[nforcei][blockszi][i] = int(match[0][3])
                            if (match[0][0] == "MAX-BLOCK-SIZE"):
                                max_block_size[nforcei][blockszi][i] = int(match[0][3])
                            if (match[0][0] == "NUM-ARRAYS"):
                                num_arrays[nforcei][blockszi][i] = int(match[0][3])
                            if (match[0][0] == "SUM-ARRAY-SIZE"):
                                sum_array_size[nforcei][blockszi][i] = int(match[0][3])
                            if (match[0][0] == "GREEDY-LOAD-SIZE"):
                                greedy_load_size[nforcei][blockszi][i] = int(match[0][3])

            except:
                print "Could not open / handle '%s'" % filename;

### Prepare plotting

xlimits=[-.5,23];
xticks=range(0,23,2);

### Do plotting

fig = plt.figure(figsize=(16, 12))

plt.subplots_adjust(wspace = 0.35, hspace = 0.3);

###

plt.subplot(3, 4, 1)

p3, = plt.semilogy(Nmax, (conns_ppn + conns_pnn +
                          conns_ppp + conns_nnn) / 1e12, 'r-o');

p2, = plt.semilogy(Nmax, (conns_pp + conns_pn + conns_nn) / 1e12, 'r--o');

p1, = plt.semilogy(Nmax, (conns_p + conns_n) / 1e12, 'r:o');

if (nucleus == "6Li"):
    plt.semilogy(22, 5.53e14 / 1e12, 'rx');

plt.legend([p3, p2, p1],
           ['conn 3N', 'conn 2N', 'conn 1N'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Mult (T ~ core-h/iter)')
plt.title(nucleus)

ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);

###

plt.subplot(3, 4, 5)

p8, = plt.semilogy(Nmax, conns_ppn / 1e12, 'm-^');
p9, = plt.semilogy(Nmax, conns_pnn / 1e12, 'm-v');
p6, = plt.semilogy(Nmax, conns_ppp / 1e12, 'b-o');
p7, = plt.semilogy(Nmax, conns_nnn / 1e12, 'g-s');

p3, = plt.semilogy(Nmax, conns_pn / 1e12, 'm--<');
p4, = plt.semilogy(Nmax, conns_pp / 1e12, 'b--o');
p5, = plt.semilogy(Nmax, conns_nn / 1e12, 'g--s');

p1, = plt.semilogy(Nmax, conns_p  / 1e12, 'b:o');
p2, = plt.semilogy(Nmax, conns_n  / 1e12, 'g:s');

plt.legend([p8, p9, p6, p7, p3, p4, p5, p1, p2],
           ['ppn','pnn','ppp','nnn','pn','pp','nn','p','n'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Mult (T)')

ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);


###

plt.subplot(3, 4, 2)
p1, = plt.semilogy(Nmax, sum_conn_len_3n * 12 / 1e12, 'r-o');
p2, = plt.semilogy(Nmax, sum_conn_len_2n * 12 / 1e12, 'r--o');
p3, = plt.semilogy(Nmax, sum_conn_len_1n * 12 / 1e12, 'r:o');

plt.plot([0, 22],[32, 32],'k-');
plt.text(1,32*1.5,'scr.munin (HDD)')

plt.legend([p1, p2, p3],
           ['idx-lists 3N','idx-lists 2N','idx-lists 1N'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Storage mtrx list (TB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.01, 0.1, 1, 10])

###

plt.subplot(3, 4, 6)
p1, = plt.semilogy(Nmax, max_conn_len_3n * 12 / 1e9, 'r-o');
p2, = plt.semilogy(Nmax, max_conn_len_2n * 12 / 1e9, 'r--o');
p3, = plt.semilogy(Nmax, max_conn_len_1n * 12 / 1e9, 'r:o');

plt.plot([0, 22],[128, 128],'k-');
plt.text(1,128*1.5,'RAM munin (128 GB)')
plt.plot([0, 22],[32, 32],'k--');
plt.text(1,32*1.5,'RAM usual (32 GB)')

plt.legend([p1, p2, p3],
           ['max idx-list 3N', 'max idx-list 2N', 'max idx-list 1N'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('RAM use (GB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.1, 1, 10])

###

plt.subplot(3, 4, 11)
p1, = plt.semilogy(Nmax, load_conn_len_3n * 12 / 1e12, 'r-o');
p2, = plt.semilogy(Nmax, load_conn_len_2n * 12 / 1e12, 'r--o');
p3, = plt.semilogy(Nmax, load_conn_len_1n * 12 / 1e12, 'r:o');
p4, = plt.semilogy(Nmax, load_Vc_size_3n * 8 / 1e12, 'b-o');
p5, = plt.semilogy(Nmax, load_Vc_size_2n * 8 / 1e12, 'b--o');
p6, = plt.semilogy(Nmax, load_Vc_size_1n * 8 / 1e12, 'b:o');
p7, = plt.semilogy(Nmax, load_Vx_size_3n * 8 / 1e12, 'g-s');
p8, = plt.semilogy(Nmax, load_Vx_size_2n * 8 / 1e12, 'g--s');
p9, = plt.semilogy(Nmax, load_Vx_size_1n * 8 / 1e12, 'g:s');

plt.legend([p1, p2, p3, p4, p7],
           ['idx-list 3N', 'idx-list 2N', 'idx-list 1N',
            'V (pn, ppn, pnn)', 'V (p^x, n^x)'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('mtrx list , V load (TB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.01, 0.1, 1, 10])

###

plt.subplot(3, 4, 12)
p1, = plt.semilogy(Nmax,
                   (load_conn_len_3n * 12 +
                    load_Vc_size_3n * 8 +
                    load_Vx_size_3n * 8) /
                   (conns_ppn + conns_pnn +
                    conns_ppp + conns_nnn), 'r-o');
p2, = plt.semilogy(Nmax,
                   (load_conn_len_2n * 12 +
                    load_Vc_size_2n * 8 +
                    load_Vx_size_2n * 8) /
                   (conns_pp + conns_pn + conns_nn), 'r--o');
p3, = plt.semilogy(Nmax,
                   (load_conn_len_1n * 12 +
                    load_Vc_size_1n * 8 +
                    load_Vx_size_1n * 8) /
                   (conns_p + conns_n), 'r:o');

if (nucleus == "6Li"):
    plt.semilogy(22, (26.9e12)/(5.53e14), 'rx');

plt.legend([p1, p2, p3, p4, p7],
           ['loads 3N', 'loads 2N', 'loads 1N'],
           loc=1,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('mtrx list , V load (B) / mult')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.1, 1, 10])

###

plt.subplot(3, 4, 10)
p1, = plt.semilogy(Nmax,
                   (sum_conn_len_3n * 12 +
                    sum_Vc_size_3n * 8 +
                    sum_Vx_size_3n * 8) /
                   (conns_ppn + conns_pnn +
                    conns_ppp + conns_nnn) * 1.e3, 'r-o');
p2, = plt.semilogy(Nmax,
                   (sum_conn_len_2n * 12 +
                    sum_Vc_size_2n * 8 +
                    sum_Vx_size_2n * 8) /
                   (conns_pp + conns_pn + conns_nn) * 1.e3, 'r--o');
p3, = plt.semilogy(Nmax,
                   (sum_conn_len_1n * 12 +
                    sum_Vc_size_1n * 8 +
                    sum_Vx_size_1n * 8) /
                   (conns_p + conns_n) * 1.e3, 'r:o');

plt.legend([p1, p2, p3, p4, p7],
           ['store 3N', 'store 2N', 'store 1N'],
           loc=1,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('mtrx list , V store (GB) / Tmult')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([1, 10, 100, 1000])

###

plt.subplot(3, 4, 3)
p1, = plt.semilogy(Nmax, sum_Vc_size_3n * 8 / 1e12, 'b-o');
p2, = plt.semilogy(Nmax, sum_Vc_size_2n * 8 / 1e12, 'b--o');
p3, = plt.semilogy(Nmax, sum_Vc_size_1n * 8 / 1e12, 'b:o');
p4, = plt.semilogy(Nmax, sum_Vx_size_3n * 8 / 1e12, 'g-s');
p5, = plt.semilogy(Nmax, sum_Vx_size_2n * 8 / 1e12, 'g--s');
p6, = plt.semilogy(Nmax, sum_Vx_size_1n * 8 / 1e12, 'g:s');

plt.plot([0, 22],[32, 32],'k-');
plt.text(1,32*1.5,'scr.munin (HDD)')

plt.legend([p1, p2, p3, p4],
           ['V-blocks 3N','V-blocks 2N','V-blocks 1N','(same part)'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Storage mtrx V (TB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.01, 0.1, 1, 10])

###

plt.subplot(3, 4, 7)
p1, = plt.semilogy(Nmax, max_Vc_size_3n * 8 / 1e9, 'b-o');
p2, = plt.semilogy(Nmax, max_Vc_size_2n * 8 / 1e9, 'b--o');
p3, = plt.semilogy(Nmax, max_Vc_size_1n * 8 / 1e9, 'b:o');
p4, = plt.semilogy(Nmax, max_Vx_size_3n * 8 / 1e9, 'g-s');
p5, = plt.semilogy(Nmax, max_Vx_size_2n * 8 / 1e9, 'g--s');
p6, = plt.semilogy(Nmax, max_Vx_size_1n * 8 / 1e9, 'g:s');

plt.plot([0, 22],[128, 128],'k-');
plt.text(1,128*1.5,'RAM munin (128 GB)')
plt.plot([0, 22],[32, 32],'k--');
plt.text(1,32*1.5,'RAM usual (32 GB)')

plt.legend([p1, p2, p3, p4],
           ['max V-block 3N', 'max V-block 2N', 'max V-block 1N',
            '(same part)'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('RAM use (GB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.1, 1, 10])

###

plt.subplot(3, 4, 4)

p3, = plt.semilogy(Nmax, total_mp_states * 8 / 1e12 * 250, 'm-o');
p4, = plt.semilogy(Nmax, total_mp_states * 8 / 1e12 * 25, 'm--o');

plt.plot([0, 22],[32, 32],'k-');
plt.text(1,32*1.5,'scr.munin (HDD)')

plt.legend([p3, p4],
           ['vector (250)','vector (25)'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Storage vect (TB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.01, 0.1, 1, 10])

###

plt.subplot(3, 4, 8)
p4, = plt.semilogy(Nmax, max_mp_block * 8 / 1e9,  'g-v');

plt.plot([0, 22],[20, 20],'k-');
plt.text(1,16*1.5,'RAM munin (20 GB/thr)')
plt.plot([0, 22],[2, 2],'k--');
plt.text(1,2*1.5,'RAM usual (2 GB/thr)')

plt.legend([p4],
           ['max mp-block'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('RAM use / thread (GB)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);
ax.yaxis.set_major_formatter(plt.ScalarFormatter())
ax.set_yticks([0.1, 1, 10])

###

plt.subplot(3, 4, 9)
p1, = plt.semilogy(Nmax, total_mp_states, 'm-o');
p2, = plt.semilogy(Nmax, max_mp_block,    'g-v');

if (nucleus == "6Li"):
    plt.semilogy(22, 25038471440, 'mx');

plt.legend([p1, p2],
           ['all mp-states', 'max mp-block'],
           loc=4,prop={'size':9})

plt.xlabel('Nmax')
plt.ylabel('Dimension (vector len.)')
ax = plt.gca()
ax.set_xlim(xlimits)
ax.set_xticks(xticks);

###

plt.show()
