import sys
import os
import pandas as pd

tracesname = ['LONG_MOBILE-1',
              'LONG_MOBILE-2',
              'LONG_MOBILE-3',
              'LONG_MOBILE-4',
              'SHORT_MOBILE-1',
              'SHORT_MOBILE-2',
              'SHORT_MOBILE-24',
              'SHORT_MOBILE-25',
              'SHORT_MOBILE-27',
              'SHORT_MOBILE-28',
              'SHORT_MOBILE-3',
              'SHORT_MOBILE-30',
              'SHORT_MOBILE-4']


def out_to_xlsx(infilename, outfilename):
    res = {"trace": [], "misper1k": []}
    with open(infilename, 'r') as file:
        lines = file.readlines()
    for line in lines:
        if "TRACE" in line:
            tr = line.split('/')[2].strip()[:-13]
            res["trace"].append(tr)
        if "MISPRED_PER_1K_INST" in line:
            misnum = line.split(':')[1].strip()
            misnum = float(misnum)
            res["misper1k"].append(misnum)
    df = pd.DataFrame(res)
    df.to_excel(outfilename, index=False, engine='openpyxl')


def all_out_to_xlsx():
    outfilename = 'results_all.xlsx'
    res = {}
    res['trace'] = tracesname
    typename = ''
    fileslist = os.listdir()
    resdf = pd.DataFrame(res)
    for filename in fileslist:
        if 'mispred' in filename:
            typename = filename.split('_')[2][:-5]
            df = pd.read_excel(io = filename)
            resdf[typename + '_misper1k'] = df['misper1k']
    resdf.to_excel(outfilename, index=False, engine='openpyxl')
if __name__ == '__main__':
    all_out_to_xlsx()
    # fileslist = os.listdir()
    # for filename in fileslist:
    #     if 'mispred' in filename:

    # print(fileslist)
    # infilename = sys.argv[1]
    # # outfilename = sys.argv[2]
    # outfilename = 'mispred_' + infilename[2:].split('.')[0] + '.xlsx'
    # out_to_xlsx(infilename, outfilename)

