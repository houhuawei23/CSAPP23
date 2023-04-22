'''
write a simple program to test the threads
'''

from threading import Thread
from time import sleep, ctime

def func(name, sec):
    print('start', name, ctime())
    sleep(sec)
    print('end', name, ctime())

# create Thread instance
t1 = Thread(target=func, args=('t1', 1))
t2 = Thread(target=func, args=('t2', 2))

# start
t1.start()
t2.start()

# join
t1.join()
t2.join()
