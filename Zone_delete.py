import os

# 1. 请将这里的路径替换为您想操作的文件夹的绝对路径
folder_path = "/home/smiling/My_Codes/Gomoku-AI/Minimax_new"

print(f"将在文件夹 '{folder_path}' 中删除所有文件名包含 'Zone' 的文件。")
print("警告：此操作不可撤销！")

# 检查路径是否存在
if not os.path.isdir(folder_path):
    print(f"错误：文件夹 '{folder_path}' 不存在。")
else:
    try:
        # 遍历文件夹中的所有文件和文件夹
        for filename in os.listdir(folder_path):
            # 检查 "Zone" 是否在文件名中 (不区分大小写)
            if "Zone" in filename.lower():
                # 构建完整的文件路径
                full_path = os.path.join(folder_path, filename)
                
                # 确认它是一个文件而不是一个文件夹，然后删除
                if os.path.isfile(full_path):
                    try:
                        os.remove(full_path)
                        print(f"已删除: {filename}")
                    except OSError as e:
                        print(f"错误：删除文件 {filename} 时出错。原因: {e}")

        print("\n操作完成。")

    except Exception as e:
        print(f"处理过程中发生未知错误: {e}")