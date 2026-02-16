import csv
# analyzing title names
file = open('input_76920_ordered.csv')
reader = csv.reader(file);
min_c = 255
max_c = 0
min_len = 10000000000000

MAX_PREFIX = 3
for title, _ in reader:
    for i in range(min(len(title), MAX_PREFIX)):
        c = title[i]
        min_c = min(ord(c), min_c)
        max_c = max(ord(c), max_c)
        min_len = min(len(title), min_len)

print("min c:", min_c, chr(min_c))
print("max_c:", max_c, chr(max_c))
print("difference:", max_c - min_c)
print("min title len", min_len)
