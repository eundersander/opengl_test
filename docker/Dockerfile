# Use NVIDIA GLVND-enabled base image for proper OpenGL/Vulkan support
# FROM nvidia/opengl:1.0-glvnd-runtime-ubuntu22.04
FROM docker.io/nvidia/opengl:1.0-glvnd-runtime-ubuntu22.04

# Install system dependencies
RUN apt update && apt install -y curl bzip2 sudo \
 && apt install -y --no-install-recommends \
    build-essential cmake pkg-config \
    libvulkan-dev vulkan-tools \
    libsm6 libxt6 libglu1-mesa libxrender1 libxrandr2 libxfixes3 \
    libxi6 libxinerama1 libxext6 libx11-6 libxcb1 \
 && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 devuser && \
    echo "devuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install Mambaforge as root
RUN curl -L https://github.com/conda-forge/miniforge/releases/download/23.11.0-0/Mambaforge-Linux-x86_64.sh -o mambaforge.sh && \
    bash mambaforge.sh -b -p /opt/mambaforge && \
    rm mambaforge.sh

# Set up mamba environment
ENV PATH="/opt/mambaforge/bin:$PATH"

# Switch to non-root user **before installing anything into the conda env**
USER devuser
WORKDIR /home/devuser
SHELL ["/bin/bash", "-c"]

# Init shell for mamba and create environment as devuser
RUN mamba init bash && \
    mamba create -n isaac_env python=3.10.12 -y && \
    mamba run -n isaac_env pip install --upgrade pip && \
    mamba run -n isaac_env pip install \
        torch==2.5.1 torchvision==0.20.1 --index-url https://download.pytorch.org/whl/cu121 && \
    mamba run -n isaac_env pip install \
        isaacsim==4.2.0.2 \
        isaacsim-extscache-physics==4.2.0.2 \
        isaacsim-extscache-kit==4.2.0.2 \
        isaacsim-extscache-kit-sdk==4.2.0.2 \
        --extra-index-url https://pypi.nvidia.com

# Optional: copy test script
COPY --chown=devuser:devuser test_script.sh ./test_script.sh
RUN chmod +x ./test_script.sh

# Default entrypoint: drop to bash for manual use
CMD ["bash"]
