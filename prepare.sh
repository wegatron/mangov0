# !/bin/bash
# apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools
# sudo apt install xorg-dev libglu1-mesa-dev
rebuild_install=false

if [ $rebuild_install = true ]; then
    echo "rebuilding install"
    #rm -rf install
fi

if [ ! -d "thirdparty" ]; then
    mkdir thirdparty
fi

if [ ! -d "thirdparty/glm" ]; then
    git clone https://github.com/g-truc/glm.git thirdparty/glm
else
  echo "glm found"
fi

if [ ! -d "thirdparty/volk" ]; then
    git clone https://github.com/zeux/volk.git thirdparty/volk
else
    echo "volk found"
fi

if [ ! -d "thirdparty/eigen" ]; then
    git clone https://gitlab.com/libeigen/eigen.git -b 3.4 thirdparty/eigen
else
    echo "eigen found"
fi

if [ ! -d "thirdparty/stbi" ]; then
    git clone https://github.com/nothings/stb.git thirdparty/stbi
else
    echo "stbi found"    
fi

if [ ! -d "thirdparty/imgui" ]; then
    git clone -b docking https://github.com/ocornut/imgui.git thirdparty/imgui
else
    echo "imgui found"
fi

if [ ! -d "thirdparty/refl-cpp" ]; then
    git clone https://github.com/veselink1/refl-cpp.git thirdparty/refl-cpp
else
    echo "refl-cpp found"
fi

if [ ! -d "thirdparty/cereal" ]; then
    git clone https://github.com/USCiLab/cereal.git thirdparty/cereal
    cd thirdparty/cereal && git checkout tags/v1.3.2
else
    echo "cereal found"
fi

if [ ! -d "thirdparty/ImGuizmo" ]; then
    git clone https://github.com/CedricGuillemet/ImGuizmo.git ./thirdparty/ImGuizmo
else
    echo "ImGuizmo found"
fi

if [ ! -d "thirdparty/glfw" ]; then
    git clone https://github.com/glfw/glfw.git thirdparty/glfw
    cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/glfw/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/glfw/build
    cmake --install thirdparty/glfw/build --config Debug
else
    echo "glfw found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/glfw/build
        cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/glfw/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/glfw/build
        cmake --install thirdparty/glfw/build --config Debug
    fi
fi

if [ ! -d "thirdparty/Vulkan-Headers" ]; then
    git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers
    cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/Vulkan-Headers/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/Vulkan-Headers/build
    cmake --install thirdparty/Vulkan-Headers/build --config Debug
else
    echo "Vulkan-Headers found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/Vulkan-Headers/build
        cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/Vulkan-Headers/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/Vulkan-Headers/build
        cmake --install thirdparty/Vulkan-Headers/build --config Debug
    fi
fi

if [ ! -d "thirdparty/spdlog" ]; then
    git clone https://github.com/gabime/spdlog.git thirdparty/spdlog
    cd thirdparty/spdlog && git checkout tags/v1.13.0    
    cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/spdlog/build
    cmake --install thirdparty/spdlog/build --config Debug
else
    echo "spdlog found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/spdlog/build
        cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/spdlog/build
        cmake --install thirdparty/spdlog/build --config Debug
    fi    
fi

if [ ! -d "thirdparty/glslang" ]; then
    git clone https://github.com/KhronosGroup/glslang thirdparty/glslang
    cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug -DENABLE_OPT=OFF
    cmake --build thirdparty/glslang/build
    cmake --install thirdparty/glslang/build --config Debug
else
    echo "glslang found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/glslang/build
        cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug -DENABLE_OPT=OFF
        cmake --build thirdparty/glslang/build
        cmake --install thirdparty/glslang/build --config Debug
    fi
fi

if [ ! -d "thirdparty/vma" ]; then
    git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git thirdparty/vma
    cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/vma/build
    cmake --install thirdparty/vma/build --config Debug
else
    echo "vma found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/vma/build
        cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/vma/build
        cmake --install thirdparty/vma/build --config Debug
    fi
fi

if [ ! -d "thirdparty/spirv-cross" ]; then
    git clone https://github.com/KhronosGroup/SPIRV-Cross.git thirdparty/spirv-cross
    cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/spirv-cross/build
    cmake --install thirdparty/spirv-cross/build --config Debug
else
    echo "spirv-cross found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/spirv-cross/build
        cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/spirv-cross/build
        cmake --install thirdparty/spirv-cross/build --config Debug
    fi
fi

if [ ! -d "thirdparty/assimp" ]; then
    git clone https://github.com/assimp/assimp.git thirdparty/assimp
    cd thirdparty/assimp && git checkout tags/v5.3.1
    cmake thirdparty/assimp -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/assimp/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/assimp/build
    cmake --install thirdparty/assimp/build --config Debug
    mkdir -p install/include/contrib/utf8cpp
    cp -r thirdparty/assimp/contrib/utf8cpp install/include/contrib
else
    echo "assimp found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/assimp/build
        cmake thirdparty/assimp -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/assimp/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/assimp/build
        cmake --install thirdparty/assimp/build --config Debug
        mkdir -p install/include/contrib/utf8cpp
        cp -r thirdparty/assimp/contrib/utf8cpp install/include/contrib
    fi
fi

if [ ! -d "thirdparty/entt" ]; then
    git clone https://github.com/skypjack/entt.git thirdparty/entt
    cmake thirdparty/entt -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/entt/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/entt/build
    cmake --install thirdparty/entt/build --config Debug
else
    echo "entt found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/entt/build
        cmake thirdparty/entt -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/entt/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/entt/build
        cmake --install thirdparty/entt/build --config Debug
    fi
fi

if [ ! -d "thirdparty/eventpp" ]; then
    git clone https://github.com/wqking/eventpp.git thirdparty/eventpp
    cmake thirdparty/eventpp -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/eventpp/build -DCMAKE_BUILD_TYPE=Debug
    cmake --build thirdparty/eventpp/build
    cmake --install thirdparty/eventpp/build --config Debug
else
    echo "eventpp found"
    if [ $rebuild_install = true ]; then
        rm -rf thirdparty/eventpp/build
        cmake thirdparty/eventpp -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/eventpp/build -DCMAKE_BUILD_TYPE=Debug
        cmake --build thirdparty/eventpp/build
        cmake --install thirdparty/eventpp/build --config Debug
    fi
fi

if [ ! -f "thirdparty/IconsFontAwesome5.h" ]; then
    wget https://raw.githubusercontent.com/juliettef/IconFontCppHeaders/main/IconsFontAwesome5.h -O thirdparty/IconsFontAwesome5.h
fi

if [ ! -f "thirdparty/ImGuiFileDialog" ]; then
    git clone https://github.com/aiekick/ImGuiFileDialog.git thirdparty/ImGuiFileDialog
    # cmake thirdparty/ImGuiFileDialog -DCMAKE_INSTALL_PREFIX="./thirdparty/install" -B thirdparty/ImGuiFileDialog/build -DCMAKE_BUILD_TYPE=Debug
    # cmake --build thirdparty/ImGuiFileDialog/build
    # cmake --install thirdparty/ImGuiFileDialog/build --config Debug
fi

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
