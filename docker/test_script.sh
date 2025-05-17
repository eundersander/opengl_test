mamba init
source ~/.bashrc
mamba activate isaac_env

# verify install and accept EULA
python -c "import isaacsim; print(isaacsim)"

# create an isaac simulation and step it
python -c 'from isaacsim import SimulationApp; app = SimulationApp({"headless": True}); app.update(); print("Success!")'

