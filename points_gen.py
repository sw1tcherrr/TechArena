import random

min_x = -1000000
max_x = 1000000
min_y = -1000000
max_y = 1000000

cnt = 1000000

# change build dir path
with open("build/input.txt", 'w') as out:
    out.write(str(cnt))
    out.write("\n")

    for i in range(cnt):
        out.write(str(random.uniform(min_x, max_x)) + " " + str(random.uniform(min_y, max_y)))
        out.write("\n")
