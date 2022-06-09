import sys
last = 0
current = 0
error = 0
count = 0
cnt_accessed = [0 for i in range(400)]
cnt_no = [0 for i in range(400)]

with open(sys.argv[1], 'r') as f:
    i = 0
    for line in f:
        i = i + 1
        #The beginning iterations are sometimes noisy.
        if i < 2000:
            continue
        if i > 120000:
            break

        s = line.split()
        time = int(s[1])
        if s[0] == "accessed":
            if time >= 400:
                cnt_accessed[395] += 1
            else:
                cnt_accessed[int(time/5)*5] += 1
        else:
            if time >= 400:
                cnt_accessed[395] += 1
            else:
                cnt_no[int(time/5)*5] += 1

    print('{:^10s}{:^30s}{:^30s}'.format("latency", "num of LLC S hits", "num of LLC M hits"))
    for i in range(0, 80):
        cnt = i * 5
        print('{:^10s}{:^30s}{:^30s}'.format(str(cnt), str(cnt_accessed[cnt]), str(cnt_no[cnt])))

            



