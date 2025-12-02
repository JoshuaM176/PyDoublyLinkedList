import time

from random_list import test_list

start_time = time.time()
test_list.sort()
end_time = time.time()


print(f"Default python list sort time is {end_time-start_time} seconds.")