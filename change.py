s = input("Enter a advertisementData: ")

n = len(s)
ans = []
for i in range(0, n, 2):
    tmp = f"0x{s[i:i+2]}"
    ans.append(tmp)

print(', '.join(ans))