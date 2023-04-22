import sys
import pandas as pd

def out_to_xlsx(infilename, outfilename):
    res = {"trace":[], "misper1k":[]}
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
    df.to_excel(outfilename, index = False, engine='openpyxl')

if __name__ == '__main__':
    infilename = sys.argv[1]
    # outfilename = sys.argv[2]
    outfilename = 'mispred_' + infilename[2:].split('.')[0] + '.xlsx'
    out_to_xlsx(infilename, outfilename)
