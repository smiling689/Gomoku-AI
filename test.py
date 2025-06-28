import torch

# 检查 CUDA 是否可用
is_cuda_available = torch.cuda.is_available()
print(f"CUDA Available: {is_cuda_available}")

if is_cuda_available:
    # 获取可用的 GPU 数量
    gpu_count = torch.cuda.device_count()
    print(f"Number of GPUs available: {gpu_count}")

    # 获取当前 GPU 设备的索引
    current_device_index = torch.cuda.current_device()
    print(f"Current GPU device index: {current_device_index}")

    # 获取当前 GPU 设备的名称
    current_device_name = torch.cuda.get_device_name(current_device_index)
    print(f"Current GPU device name: {current_device_name}")
else:
    print("PyTorch cannot find a CUDA-enabled GPU.")