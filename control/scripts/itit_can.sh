sudo ip link set down can0
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 txqueuelen 10000
sudo ip link set up can0
sudo ip link set down can1
sudo ip link set can1 type can bitrate 1000000
sudo ip link set can1 txqueuelen 10000
sudo ip link set up can1