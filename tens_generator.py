from decimal import Decimal
print()

pow_list = {}

def create_byte_row(num):
    s = hex(num)[2::] #skip 0x
    if len(s) % 2 == 1:
        s = "0" + s
    
    lst = ["$"+s[i-2:i] for i in range(2, len(s)+1, 2)][::-1]
    while len(lst) < 16:
        lst.append("$00")
    return lst

for p in range(-19, 20):
    n = (Decimal(10) ** p) * (2**64)
    pow_list[p] = create_byte_row(int(n))

for pwr, lst in sorted(pow_list.items()):
    print(f'.byte {", ".join(lst)} ;10^{pwr}')
