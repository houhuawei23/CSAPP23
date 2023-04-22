'''一些工具函数'''

# 将十六进制字符串（小端排序）转换为十进制整数
def lend_hex_to_dec(string):
    # input
    res = ''
    for i in range(1, len(string) - 1, 2):
        res = string[i - 1:i + 1] + res
    return int(res, 16)
# 将十进制整数转换为十六进制字符串（小端排序，定宽）
def dec_to_lend_hex_width(decimal_num, width=16):
    hex_string = hex(decimal_num).replace("0x", "")
    hex_string = hex_string.zfill(width)
    return hex_string
# 以定宽分割字符串
def split_string_by_length(input_str, length):
    return [input_str[i:i + length] for i in range(0, len(input_str), length)]

# def print_status(pc):
#     print("pc: 0x%x" % pc.comregs.PC)

def astr_to_bstr(astr, a, b):
    def base_n_to_base_10(num_str, base):
        result = 0
        for i, char in enumerate(reversed(num_str)):
            num_val = int(char, base)
            result += num_val * (base ** i)
        return result

    def base_10_to_base_n(num, base):
        if num == 0:
            return "0"
        result = []
        while num > 0:
            num, remainder = divmod(num, base)
            result.append(str(remainder))
        return "".join(reversed(result))

    base_10_num = base_n_to_base_10(astr, a)
    bstr = base_10_to_base_n(base_10_num, b)

    return bstr


# 示例
astr = "1101"
a = 2
b = 16
bstr = astr_to_bstr(astr, a, b)
print(bstr)
