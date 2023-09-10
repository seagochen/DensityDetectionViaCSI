import numpy as np

# 定义常量
TONE_40M = 114
BITS_PER_BYTE = 8
BITS_PER_SYMBOL = 10
BITS_PER_COMPLEX_SYMBOL = 2 * BITS_PER_SYMBOL


# 负数转换函数
def signbit_convert(data, maxbit):
    # 如果数据的最高位为1，说明是负数，需要做转换
    if data & (1 << (maxbit - 1)):
        data -= (1 << maxbit)
    return data


def read_csi(local_h, nr, nc, num_tones):
    bits_left = 16  # 每次处理16位数据

    # 为H的实部和虚部创建10位掩码
    bitmask = (1 << BITS_PER_SYMBOL) - 1
    idx = h_idx = 0
    h_data = local_h[idx]
    h_data += (local_h[idx+1] << BITS_PER_BYTE)
    current_data = h_data & ((1 << 16) - 1)  # 先获取16位最低位数据

    # 创建输出数据的结构
    size = (nr, nc, num_tones)
    csi = np.zeros(size, dtype=complex)

    # 循环处理每个子载波、每个发送天线、每个接收天线的数据
    for k in range(num_tones):
        for nc_idx in range(nc):
            for nr_idx in range(nr):
                # 如果剩余位数不足一个符号的位数，获取下一组16位数据
                if (bits_left - BITS_PER_SYMBOL) < 0:
                    idx += 2
                    h_data = local_h[idx]
                    h_data += (local_h[idx+1] << BITS_PER_BYTE)
                    current_data += h_data << bits_left
                    bits_left += 16
                # 获取虚部数据并进行负数转换
                imag = current_data & bitmask
                imag = signbit_convert(imag, BITS_PER_SYMBOL)
                csi[nr_idx, nc_idx, k] = complex(0, imag)
                bits_left -= BITS_PER_SYMBOL
                # 移出已使用的数据
                current_data = current_data >> BITS_PER_SYMBOL

                # 同样处理实部数据
                if (bits_left - BITS_PER_SYMBOL) < 0:
                    idx += 2
                    h_data = local_h[idx]
                    h_data += (local_h[idx+1] << BITS_PER_BYTE)
                    current_data += h_data << bits_left
                    bits_left += 16
                real = current_data & bitmask
                real = signbit_convert(real, BITS_PER_SYMBOL)
                csi[nr_idx, nc_idx, k] += real
                bits_left -= BITS_PER_SYMBOL
                # 移出已使用的数据
                current_data = current_data >> BITS_PER_SYMBOL

    return csi


def main():

    local_h = (213, 56, 64, 183, 236, 249, 239, 191, 191, 3, 92, 236, 156, 190, 241, 54, 88, 109, 71, 23, 18, 124, 80,
               205, 3, 126, 147, 142, 255, 254, 251, 47, 112, 134, 207, 236, 23, 207, 195, 212, 105, 148, 1, 65, 8, 208,
               80, 128, 248, 232, 249, 235, 143, 127, 2, 119, 12, 29, 63, 241, 64, 88, 253, 69, 27, 16, 148, 176, 12, 5,
               142, 139, 110, 255, 254, 247, 19, 208, 7, 210, 240, 19, 47, 68, 214, 79, 172, 225, 192, 9, 206, 40, 112,
               57, 230, 246, 239, 143, 191, 0, 139, 52, 29, 255, 240, 77, 52, 61, 196, 28, 13, 172, 176, 204, 2, 164,
               115, 94, 191, 254, 1, 0, 128, 200, 215, 242, 31, 143, 68, 213, 61, 208, 225, 192, 10, 193, 24, 224, 58,
               229, 245, 235, 79, 0, 255, 139, 124, 29, 255, 241, 77, 84, 141, 131, 29, 14, 188, 96, 76, 0, 174, 103,
               30, 191, 254, 13, 252, 111, 8, 217, 239, 39, 47, 196, 214, 58, 208, 17, 193, 11, 191, 228, 143, 251,
               230, 241, 235, 207, 0, 1, 135, 180, 13, 127, 242, 67, 112, 109, 3, 29, 17, 192, 208, 203, 251, 187, 115,
               14, 191, 254, 17, 28, 16, 200, 220, 239, 43, 239, 67, 216, 50, 208, 81, 1, 12, 186, 120, 15, 60, 234,
               237, 243, 95, 193, 2, 123, 204, 205, 254, 242, 50, 144, 13, 132, 27, 24, 184, 96, 75, 245, 204, 171, 190,
               62, 255, 21, 60, 16, 71, 220, 232, 51, 15, 3, 217, 63, 148, 113, 65, 11, 191, 32, 143, 61, 237, 234, 243,
               159, 65, 4, 103, 208, 141, 62, 243, 35, 180, 173, 195, 22, 24, 172, 80, 75, 239, 221, 235, 190, 126, 255,
               23, 108, 32, 198, 220, 232, 51, 239, 193, 219, 57, 56, 113, 129, 10, 174, 156, 142, 254, 238, 234, 255,
               47, 193, 6, 84, 220, 109, 190, 243, 24, 188, 173, 3, 16, 23, 164, 64, 10, 231, 238, 11, 159, 62, 0, 20,
               128, 224, 132, 220, 229, 63, 207, 0, 221, 55, 208, 112, 65, 10, 148, 64, 110, 255, 240, 235, 7, 176, 128,
               8, 69, 232, 109, 190, 244, 8, 212, 29, 195, 10, 21, 168, 112, 137, 221, 249, 31, 143, 190, 0, 10, 160,
               80, 68, 220, 229, 63, 223, 127, 221, 47, 144, 128, 193, 10, 136, 196, 173, 63, 243, 233, 19, 96, 0, 10,
               53, 212, 93, 254, 244, 243, 247, 157, 2, 7, 25, 172, 80, 136, 215, 4, 60, 127, 254, 0, 3, 168, 208, 2,
               221, 227, 95, 15, 255, 222, 38, 80, 144, 1, 11, 121, 76, 189, 128, 244, 232, 19, 80, 128, 10, 35, 228,
               77, 190, 246, 226, 3, 126, 194, 2, 27, 168, 240, 134, 207, 13, 80, 143, 190, 1, 4, 164, 160, 65, 222,
               227, 115, 143, 61, 224, 34, 40, 208, 193, 10, 105, 216, 92, 193, 245, 230, 27, 64, 192, 10, 18, 228, 45,
               254, 247, 216, 15, 126, 130, 0, 30, 172, 160, 133, 203, 23, 104, 127, 126, 2, 1, 164, 128, 128, 221, 224,
               139, 175, 252, 226, 42, 252, 47, 194, 9, 84, 152, 140, 193, 246, 230, 39, 32, 128, 10, 251, 199, 221,
               125, 249, 197, 63, 190, 194, 253, 35, 152, 96, 196, 197, 24, 120, 111, 126, 3, 5, 164, 80, 255, 222,
               220, 171, 239, 251, 230, 46, 176, 79, 194, 8, 54, 72, 44, 66, 248, 230, 63, 64, 128, 11, 236, 235, 189,
               253, 250, 185, 135, 238, 130, 249, 38, 124, 224, 66, 195, 40, 128, 127, 62, 4, 5, 192, 48, 126, 223, 218,
               183, 175, 59, 236, 49, 128, 95, 2, 7, 19, 228, 11, 67, 246, 231, 83, 64, 64, 13, 206, 11, 142, 253, 252,
               168, 243, 142, 66, 243, 37, 88, 176, 127, 187, 47, 92, 127, 254, 5, 5, 224, 208, 60, 225, 217, 207, 255,
               121, 241, 33, 4, 111, 66, 5, 238, 199, 27, 131, 244, 234, 95, 80, 64, 14, 198, 39, 190, 189, 253, 157,
               71, 191, 129, 239, 38, 76, 192, 189, 185, 54, 60, 191, 190, 6, 7, 4, 81, 251, 224, 216, 219, 79, 57, 247,
               18, 216, 110, 66, 4, 203, 171, 59, 195, 242, 235, 111, 144, 192, 15, 177, 59, 190, 125, 254, 144, 139,
               207, 0, 237, 39, 60, 160, 251, 185, 58, 12, 239, 254, 6, 11, 24, 97, 58, 229, 220, 235, 175, 184, 250,
               12, 200, 158, 194, 3, 173, 139, 203, 131, 239, 238, 119, 96, 192, 18, 155, 91, 238, 125, 255, 136, 195,
               95, 0, 237, 41, 48, 0, 58, 185, 63, 240, 14, 63, 8, 4, 44, 97, 121, 230, 223, 3, 240, 247, 253, 4, 200,
               222, 130, 2, 146, 179, 235, 131, 236, 241, 131, 224, 127, 19, 141, 59, 14, 62, 0, 134, 219, 127, 0, 237,
               43, 32, 96, 56, 183, 72, 176, 30, 127, 8, 1, 72, 17, 248, 229, 225, 19, 0, 184, 254, 7, 196, 238, 2, 1,
               128, 131, 187, 68, 234, 243, 139, 16, 192, 18, 131, 99, 62, 254, 1, 130, 15, 128, 64, 235, 45, 0, 192,
               182, 183, 77, 144, 62, 127, 9, 0, 32, 1, 248, 230, 228, 43, 0, 56, 3, 7, 168, 206, 66, 255, 95, 151, 59,
               5, 231, 245, 151, 192, 127, 19, 125, 119, 110, 254, 2, 127, 59, 128, 64, 233, 43, 236, 47, 245, 185, 89,
               84, 126, 63, 10, 255, 31, 177, 183, 231, 230, 55, 192, 55, 7, 252, 127, 190, 194, 253, 67, 151, 235, 5,
               227, 248, 175, 112, 192, 18, 109, 139, 94, 126, 4, 130, 111, 16, 64, 232, 43, 212, 15, 116, 189, 93, 24,
               174, 127, 10, 7, 16, 33, 183, 232, 230, 71, 48, 184, 9, 250, 127, 174, 2, 253, 45, 155, 91, 6, 224, 249,
               179, 80, 128, 18, 105, 143, 142, 62, 5, 133, 203, 192, 190, 230, 43, 200, 239, 242, 188, 103, 216, 173,
               255, 10, 14, 36, 113, 246, 233, 231, 83, 128, 248, 13, 231, 119, 158, 66, 252, 38, 207, 203, 70, 220,
               252, 175, 48, 193, 17, 99, 167, 126, 126, 5, 139, 19, 209, 253, 230, 44, 188, 239, 241, 190, 109, 172,
               221, 127, 11, 24, 48, 241, 53, 235, 231, 91, 160, 248, 16, 214, 119, 190, 194, 251, 32, 231, 91, 7, 219,
               251, 183, 128, 1, 21, 89, 159, 142, 126, 5, 134, 31, 65, 253, 231, 46, 192, 175, 241, 191, 119, 168, 205,
               127, 11, 23, 100, 161, 181, 234, 235, 87, 64, 56, 19, 199, 135, 254, 2, 252, 27, 211, 171, 72, 219, 252,
               187, 128, 129, 22, 85, 139, 190, 126, 5, 126, 51, 161, 60, 231, 50, 188, 95, 50, 189, 145, 200, 173, 127,
               11, 17, 92, 177, 117, 231, 236, 83, 0, 248, 17, 213, 127, 30, 195, 251, 53, 227, 123, 9, 222, 248, 175,
               160, 192, 22, 92, 79, 222, 190, 5, 134, 243, 192, 189, 232, 49, 192, 223, 51, 190, 155, 244, 157, 63, 11,
               11, 88, 81, 182, 228, 238, 95, 48, 120, 14, 226, 131, 14, 67, 252, 62, 199, 155, 202, 223, 247, 183, 16,
               192, 21, 113, 27, 14, 191, 5, 125, 211, 112, 126, 231, 49, 196, 127, 117, 191, 166, 52, 110, 127, 10, 1,
               88, 177, 247, 225, 240, 91, 48, 120, 9, 247, 111, 238, 2, 253)  # initialize your local_h list
    nr = 3
    nc = 3
    num_tones = 56

    # output shape is 3 x 3 x 56
    csi = read_csi(local_h, nr, nc, num_tones)

    # print out the complex number
    for tone_idx in range(num_tones):
        print(csi[:, :, tone_idx], "\n")


if __name__ == "__main__":
    main()
