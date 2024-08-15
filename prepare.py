import os
import subprocess
import shutil

rebuild_install = True

print(f"rebuilding install {rebuild_install}")

if not os.path.exists("thirdparty"):
    os.mkdir("thirdparty")

if not os.path.exists("thirdparty/glm"):
    subprocess.run(["git", "clone", "https://github.com/g-truc/glm.git", "thirdparty/glm"])
else:
    print("glm found")

if not os.path.exists("thirdparty/volk"):
    subprocess.run(["git", "clone", "https://github.com/zeux/volk.git", "thirdparty/volk"])


# Check if volk is found
if not os.path.exists("thirdparty/volk"):
    subprocess.run("git clone https://github.com/zeux/volk.git thirdparty/volk")
else:
    print("volk found")

# Check if eigen is found
if not os.path.exists("thirdparty/eigen"):
    subprocess.run("git clone -b 3.4 https://gitlab.com/libeigen/eigen.git thirdparty/eigen")
else:
    print("eigen found")

# Check if stbi is found
if not os.path.exists("thirdparty/stbi"):
    subprocess.run("git clone https://github.com/nothings/stb.git thirdparty/stbi")
else:
    print("stbi found")

# Check if imgui is found
if not os.path.exists("thirdparty/imgui"):
    subprocess.run("git clone -b docking https://github.com/ocornut/imgui.git thirdparty/imgui")
else:
    print("imgui found")

# Check if refl-cpp is found
if not os.path.exists("thirdparty/refl-cpp"):
    subprocess.run("git clone https://github.com/veselink1/refl-cpp.git thirdparty/refl-cpp")
else:
    print("refl-cpp found")

# Check if cereal is found
if not os.path.exists("thirdparty/cereal"):
    subprocess.run("git clone https://github.com/USCiLab/cereal.git thirdparty/cereal")
    os.chdir("thirdparty/cereal")
    subprocess.run("git checkout tags/v1.3.2")
else:
    print("cereal found")

# Check if ImGuizmo is found
if not os.path.exists("thirdparty/ImGuizmo"):
    subprocess.run("git clone https://github.com/CedricGuillemet/ImGuizmo.git thirdparty/ImGuizmo")
else:
    print("ImGuizmo found")

# Check if glfw is found
if not os.path.exists("thirdparty/glfw"):
    subprocess.run("git clone https://github.com/glfw/glfw.git thirdparty/glfw")
    subprocess.run("cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/glfw/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/glfw/build")
    subprocess.run("cmake --install thirdparty/glfw/build --config Debug")
else:
    print("glfw found")
    if rebuild_install:
        if os.path.exists("thirdparty/glfw/build"):
            shutil.rmtree("thirdparty/glfw/build")
        subprocess.run("cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/glfw/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/glfw/build")
        subprocess.run("cmake --install thirdparty/glfw/build --config Debug")

# Check if Vulkan-Headers is found
if not os.path.exists("thirdparty/Vulkan-Headers"):
    subprocess.run("git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers")
    subprocess.run("cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/Vulkan-Headers/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/Vulkan-Headers/build")
    subprocess.run("cmake --install thirdparty/Vulkan-Headers/build --config Debug")
else:
    print("Vulkan-Headers found")
    if rebuild_install:
        if os.path.exists("thirdparty/Vulkan-Headers/build"):
            shutil.rmtree("thirdparty/Vulkan-Headers/build")
        subprocess.run("cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/Vulkan-Headers/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/Vulkan-Headers/build")
        subprocess.run("cmake --install thirdparty/Vulkan-Headers/build --config Debug")

# Check if spdlog is found
if not os.path.exists("thirdparty/spdlog"):
    subprocess.run("git clone https://github.com/gabime/spdlog.git thirdparty/spdlog")
    os.chdir("thirdparty/spdlog")
    subprocess.run("git checkout tags/v1.13.0")
    subprocess.run("cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/spdlog/build")
    subprocess.run("cmake --install thirdparty/spdlog/build --config Debug")
else:
    print("spdlog found")
    if rebuild_install:
        if os.path.exists("thirdparty/spdlog/build"):
            shutil.rmtree("thirdparty/spdlog/build")
        subprocess.run("cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/spdlog/build")
        subprocess.run("cmake --install thirdparty/spdlog/build --config Debug")

# Check if glslang is found
if not os.path.exists("thirdparty/glslang"):
    subprocess.run("git clone https://github.com/KhronosGroup/glslang thirdparty/glslang")
    subprocess.run("cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug -DENABLE_OPT=OFF")
    subprocess.run("cmake --build thirdparty/glslang/build")
    subprocess.run("cmake --install thirdparty/glslang/build --config Debug")
else:
    print("glslang found")
    if rebuild_install:
        if os.path.exists("thirdparty/glslang/build"):
            shutil.rmtree("thirdparty/glslang/build")
        subprocess.run("cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug -DENABLE_OPT=OFF")
        subprocess.run("cmake --build thirdparty/glslang/build")
        subprocess.run("cmake --install thirdparty/glslang/build --config Debug")

# Check if vma is found
if not os.path.exists("thirdparty/vma"):
    subprocess.run("git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git thirdparty/vma")
    subprocess.run("cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/vma/build")
    subprocess.run("cmake --install thirdparty/vma/build --config Debug")
else:
    print("vma found")
    if rebuild_install:
        if os.path.exists("thirdparty/vma/build"):
            shutil.rmtree("thirdparty/vma/build")
        subprocess.run("cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/vma/build")
        subprocess.run("cmake --install thirdparty/vma/build --config Debug")

# Check if spirv-cross is found
if not os.path.exists("thirdparty/spirv-cross"):
    subprocess.run("git clone https://github.com/KhronosGroup/SPIRV-Cross.git thirdparty/spirv-cross")
    subprocess.run("cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/spirv-cross/build")
    subprocess.run("cmake --install thirdparty/spirv-cross/build --config Debug")
else:
    print("spirv-cross found")
    if rebuild_install:
        if os.path.exists("thirdparty/spirv-cross/build"):
            shutil.rmtree("thirdparty/spirv-cross/build")
        subprocess.run("cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/spirv-cross/build")
        subprocess.run("cmake --install thirdparty/spirv-cross/build --config Debug")

# Check if assimp is found
if not os.path.exists("thirdparty/assimp"):
    subprocess.run("git clone https://github.com/assimp/assimp.git thirdparty/assimp")
    os.chdir("thirdparty/assimp")
    subprocess.run("git checkout tags/v5.3.1")
    subprocess.run("cmake thirdparty/assimp -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/assimp/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/assimp/build")
    subprocess.run("cmake --install thirdparty/assimp/build --config Debug")
    os.makedirs("thirdparty/install/include/contrib/utf8cpp", exist_ok=True)
    shutil.copytree("thirdparty/assimp/contrib/utf8cpp", "thirdparty/install/include/contrib", dirs_exist_ok=True)
else:
    print("assimp found")
    if rebuild_install:
        if os.path.exists("thirdparty/assimp/build"):
            shutil.rmtree("thirdparty/assimp/build")
        subprocess.run("cmake thirdparty/assimp -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/assimp/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/assimp/build")
        subprocess.run("cmake --install thirdparty/assimp/build --config Debug")
        os.makedirs("install/include/contrib/utf8cpp", exist_ok=True)
        shutil.copytree("thirdparty/assimp/contrib/utf8cpp", "install/include/contrib", dirs_exist_ok=True)

# Check if entt is found
if not os.path.exists("thirdparty/entt"):
    subprocess.run("git clone https://github.com/skypjack/entt.git thirdparty/entt")
    subprocess.run("cmake thirdparty/entt -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/entt/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/entt/build")
    subprocess.run("cmake --install thirdparty/entt/build --config Debug")
else:
    print("entt found")
    if rebuild_install:
        if os.path.exists("thirdparty/entt/build"):
            shutil.rmtree("thirdparty/entt/build")
        subprocess.run("cmake thirdparty/entt -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/entt/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/entt/build")
        subprocess.run("cmake --install thirdparty/entt/build --config Debug")

# Check if eventpp is found
if not os.path.exists("thirdparty/eventpp"):
    subprocess.run("git clone https://github.com/wqking/eventpp.git thirdparty/eventpp")
    subprocess.run("cmake thirdparty/eventpp -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/eventpp/build -DCMAKE_BUILD_TYPE=Debug")
    subprocess.run("cmake --build thirdparty/eventpp/build")
    subprocess.run("cmake --install thirdparty/eventpp/build --config Debug")
else:
    print("eventpp found")
    if rebuild_install:
        if os.path.exists("thirdparty/eventpp/build"):
            shutil.rmtree("thirdparty/eventpp/build")
        subprocess.run("cmake thirdparty/eventpp -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/eventpp/build -DCMAKE_BUILD_TYPE=Debug")
        subprocess.run("cmake --build thirdparty/eventpp/build")
        subprocess.run("cmake --install thirdparty/eventpp/build --config Debug")

# Check if IconsFontAwesome5.h is found
if not os.path.exists("thirdparty/IconsFontAwesome5.h"):
    import urllib.request
    url = "https://raw.githubusercontent.com/juliettef/IconFontCppHeaders/main/IconsFontAwesome5.h"
    file_path = "thirdparty/IconsFontAwesome5.h"
    urllib.request.urlretrieve(url, file_path)

# Check if ImGuiFileDialog is found
if not os.path.exists("thirdparty/ImGuiFileDialog"):
    subprocess.run("git clone https://github.com/aiekick/ImGuiFileDialog.git thirdparty/ImGuiFileDialog")
    # subprocess.run("cmake thirdparty/ImGuiFileDialog -DCMAKE_INSTALL_PREFIX=./thirdparty/install -B thirdparty/ImGuiFileDialog/build -DCMAKE_BUILD_TYPE=Debug")
    # subprocess.run("cmake --build thirdparty/ImGuiFileDialog/build")
    # subprocess.run("cmake --install thirdparty/ImGuiFileDialog/build --config Debug")

#-----------------------------------------------------------------------------------------------------------------------

# if [ ! -d "thirdparty/tracy" ]; then
#     git clone https://github.com/wolfpld/tracy.git
#     cmake thirdparty/tracy -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/tracy/build -DCMAKE_BUILD_TYPE=Debug
# else
#     echo "tracy found"
# fi

# if [ ! -d "thirdparty/ImGuizmo" ]; then
#     git clone https://github.com/CedricGuillemet/ImGuizmo.git thirdparty/ImGuizmo
# else
#     echo "ImGuizmo found"
# fi


# if [ ! -d "thirdparty/DevIL" ]; then
#     git clone https://github.com/DentonW/DevIL.git thirdparty/DevIL
#     cmake thirdparty/DevIL/DevIL -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/DevIL/build -DCMAKE_BUILD_TYPE=Debug
#     cmake --build thirdparty/DevIL/build
#     cmake --install thirdparty/DevIL/build --config Debug
# else
#     echo "DevIL found"
# fi

# if [ ! -d "thirdparty/SPIRV-Reflect" ]; then
#     git clone https://github.com/KhronosGroup/SPIRV-Reflect.git
# else
#     echo "SPIRV-Reflect found"
# fi

# if [ ! -d "thirdparty/JoltPhysics" ]; then
#     git clone https://github.com/jrouwe/JoltPhysics.git thirdparty/JoltPhysics
#     cmake thirdparty/JoltPhysics/Build -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/JoltPhysics/build -DCMAKE_BUILD_TYPE=Debug
#     cmake --build thirdparty/JoltPhysics/build
#     cmake --install thirdparty/JoltPhysics/build --config Debug
# else
#     echo "eventpp found"
# fi
#download data from https://sketchfab.com/3d-models/buster-drone-294e79652f494130ad2ab00a13fdbafd#download
