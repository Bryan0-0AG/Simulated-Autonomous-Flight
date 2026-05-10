# ROCm Cloud Integration Guide

Your system is now equipped to connect to an **AMD ROCm MI300X Cloud Droplet**.

## 1. Prerequisites
- An active AMD ROCm Developer Cloud Droplet.
- SSH access configured (Public key added to the droplet).
- `scp` and `ssh` available in your local terminal.

## 2. Configuration
1. Open the **Server Management** panel in the Dashboard.
2. Enable **Use ROCm Cloud Droplet**.
3. Enter your Droplet's **IP Address** and **User** (usually `ubuntu`).
4. Click **Sync Code to Droplet**. This will push your local `src`, `include`, and `Makefile` to the remote server.

## 3. Running the Simulation
1. Click **Boot Engine**.
2. The system will:
   - Connect via SSH.
   - Run `make server` on the droplet (using ROCm paths).
   - Launch `./app_server`.
3. The Dashboard will automatically switch its telemetry bridge to the Droplet's IP.

## 4. Manual Configuration (Optional)
If you want to run the C++ client (`app_client`) locally but connected to the cloud server, update `include/shared/global_config.h`:
```cpp
const std::string SERVER_IP = "YOUR_DROPLET_IP";
```
Then rebuild your local client with `make client`.

## 5. Troubleshooting
- **Connection Refused**: Ensure the droplet's firewall allows traffic on ports `9998` (Telemetry) and `9999` (Orchestrator).
- **SSH Error**: Make sure your SSH key is added to the agent or specify the path in `cloud_config.json`.
