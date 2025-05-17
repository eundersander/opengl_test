

To build:
```
podman build -t isaacsim-dev .
```

To run:
```
# must be 4.1+ to support CDI
podman --version
podman run -it --rm \
  --device nvidia.com/gpu=all \
  --security-opt=label=disable \
  isaacsim-dev bash

# in the docker bash shell
./test_script.sh
```