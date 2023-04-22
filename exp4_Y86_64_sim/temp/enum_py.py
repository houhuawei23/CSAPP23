# -*- coding: utf-8 -*-
"""
Created on Sat Apr  8 10:09:58 2023

@author: hhw
"""

# from enum import Enum
# class Weekday(Enum):
#     monday = 1
#     tuesday = 2
#     wednesday = 3
#     thirsday = 4
#     friday = 5
#     saturday = 6
#     sunday = 7
 
# print(Weekday.wednesday)         # Weekday.wednesday      
# print(type(Weekday.wednesday))   # <enum 'Weekday'>
# print(Weekday.wednesday.name)    # wednesday
# print(Weekday.wednesday.value)   # 3

from enum import Enum
class Weekday(Enum):
    monday = 1
    tuesday = 2
    wednesday = 3
    thirsday = 4
    friday = 5
    saturday = 6
    sunday = 7
 
Weekday.wednesday.label = "星期三"
Weekday.wednesday.work = "完成假期作业"
Weekday.wednesday.time = 10
 
obj_1 = Weekday.wednesday
print(obj_1.label)             # 星期三
 
obj_2 = Weekday['wednesday']
print(obj_1.label)             # 星期三
 
obj_3 = Weekday(3)
print(obj_3.label)             # 星期三