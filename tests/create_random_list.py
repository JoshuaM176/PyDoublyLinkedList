import random

with open("tests/random_list.py", "w") as f:
    test_list = [random.randint(1,1000) for i in range(10000000)]
    f.write(f"test_list = {test_list}")