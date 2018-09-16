import sys

filename = sys.argv[1]

with open(filename, "r") as f:
    string = f.read()

l = list(string)

for i in range(len(l)):
    if ord(l[i]) > 127:
        l[i] = ' '

# for i in range(len(l) - 1):
#     if l[i] == '$' and l[i + 1] == '$':
#         l[i] = ' '

string = "".join(l)

with open("newtest.txt", "w") as f:
    f.write(string)
    f.write('$') 