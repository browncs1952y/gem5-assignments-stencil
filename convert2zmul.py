import sys

try:
    filename = sys.argv[1]
except IndexError:
    print("usage: python3 convert2zmul.py </path/to/binaryfile>")
    exit(-1)

f = open(filename, 'rb')
file_bytes = f.read()
f.close()

# TODO: manipulate file bytes here!


f = open(filename, 'wb')
f.write(file_bytes)
f.close()
