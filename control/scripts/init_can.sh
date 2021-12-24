# Check for correct number of runtime args
if [ "$#" -ne 2 ]; then
    echo "Must specify which can interface to initialize and bitrate."
    echo "Example: ./init_can.sh can0 1000000"
    exit 1
fi

# First stop the interface
sudo ip link set down $1

# Set the desired bitrate
sudo ip link set $1 type can bitrate $2

# Set a good queue length
sudo ip link set $1 txqueuelen 10000

# Bring the interface back up
sudo ip link set up $1 


# Change the priority of the CAN interrupt to above the rt thread interrupt.
# First find pid of the process
CANTID=`pgrep "irq/167-mcp251x"`
# Set the priority to above default.
chrt -v --pid $CANTID

echo "Setting new priority for the CAN interrupt thread...."
sudo chrt --pid 81 $CANTID

chrt -v --pid $CANTID