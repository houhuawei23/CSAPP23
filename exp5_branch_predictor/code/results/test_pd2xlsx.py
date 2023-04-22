# -*- coding: utf-8 -*-
"""
Created on Sat Apr 22 11:56:42 2023

@author: hhw
"""

import pandas as pd

# 创建一个简单的DataFrame
data = {
    'Name': ['Alice', 'Bob', 'Charlie'],
    'Age': [25, 30, 35],
    'City': ['New York', 'San Francisco', 'Los Angeles']
}

df = pd.DataFrame(data)

# 将DataFrame写入Excel文件
df.to_excel('output.xlsx', index=False, engine='openpyxl')
