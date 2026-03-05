import sys

try:
    infile = sys.argv[1]
    outfile = sys.argv[2]
except IndexError:
    print("usage: python3 convert2b.py </path/to/inputfile> <path/to/outputfile>")
    exit(-1)

f = open(infile, 'rb')
file_bytes = f.read()
f.close()

# TODO: manipulate file bytes here!

f = open(outfile, 'wb')
f.write(file_bytes)
f.close()
