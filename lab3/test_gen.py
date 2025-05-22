import random 

c = 10000
alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

def randKey():
    l = random.randint(0, 255)
    k = ""

    for i in range(l):
        k += alpha[random.randint(0, len(alpha) - 1)]

    return k

with open("in.txt", "w") as file:
    keys = set()
    for i in range(c):
        cmd = random.randint(0, 2)
        k = randKey() if len(keys) == 0 or random.randint(0, 1) == 0 else random.choice(tuple(keys))
        if cmd == 0:
            v = random.randint(0, 2**64 - 1)
            keys.add(k)
            file.write(f"+ {k} {v}\n")
        elif cmd == 1:
            if k in keys:
                keys.remove(k)
            file.write(f"- {k}\n")
        else:
            file.write(f"{k}")
