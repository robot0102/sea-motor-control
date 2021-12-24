#!/bin/bash

echo "Entering the Documents directory..."
pushd ~/Documents/

echo $(pwd)

# Install packages
sudo apt-get update
sudo apt-get -y install git python3-pip g++ gcc build-essential liblapack-dev wget unzip can-utils rt-tests rt-app curl gnuplot stress emacs openssh-server htop pkg-config libncursesw5-dev  libreadline-dev libmodbus-dev

# Install cmake
sudo -H pip3 install --upgrade pip
sudo -H pip3 install --upgrade scikit-build
sudo -H pip3 install --upgrade cmake
sudo -H pip3 install --upgrade numpy portio ipython pyserial

# Compile and install spdlog
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build && cmake .. && make -j3 && sudo make install

# Install Eigen
wget https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip
unzip eigen-3.3.7.zip
cd eigen-3.3.7 && mkdir build && cd build && cmake .. && sudo make install

# Get the real-time kernels from my personal repository
BBPASS="TqRsXCmaANcXbY9NETny"
KERNEL_IMG="https://api.bitbucket.org/2.0/repositories/RationalAsh/motor_control/downloads/linux-image-4.13.13-ashwin-rt5_1.0_amd64.deb"
KERNEL_HEADERS="https://api.bitbucket.org/2.0/repositories/RationalAsh/motor_control/downloads/linux-headers-4.13.13-ashwin-rt5_1.0_amd64.deb"

curl -O -L --user RationalAsh:${BBPASS} ${KERNEL_IMG}
curl -O -L --user RationalAsh:${BBPASS} ${KERNEL_HEADERS}

sudo dpkg -i linux-image-4.13.13-ashwin-rt5_1.0_amd64.deb
sudo dpkg -i linux-headers-4.13.13-ashwin-rt5_1.0_amd64.deb

# Stop the desktop service (no need for one on an embedded PC)
sudo systemctl disable lightdm.service


# Change the grub boot settings to prevent getting stuck at reboot and shutdown.
sudo sed -i 's/^GRUB_CMDLINE_LINUX_DEFAULT.*/GRUB_CMDLINE_LINUX_DEFAULT="acpi=force"/' /etc/default/grub
# Change to rt-kernel as default
sudo sed -i 's/^GRUB_DEFAULT.*/GRUB_DEFAULT="Advanced options for Ubuntu>Ubuntu, with Linux 4.13.13-ashwin-rt5"/' /etc/default/grub
sudo update-grub
sudo update-grub2

# Make the shell autologin.
sudo mkdir -p /etc/systemd/system/getty@.service.d/
sudo touch /etc/systemd/system/getty@.service.d/override.conf

# Get the user to login for
WHICHUSER=$(who | head -n1 | awk '{print $1}')

{ echo "[Service]";
  echo "ExecStart=";
  echo "ExecStart=-/sbin/agetty --noclear -a ${WHICHUSER} %I $TERM"; } | sudo tee /etc/systemd/system/getty@.service.d/override.conf

# Remove unattended-upgrades
sudo apt-get -y remove unattended-upgrades
# Disable daily check for updates.
sudo sed -i 's/^APT::Periodic::Update-Package-Lists.*/APT::Periodic::Update-Package-Lists "0";/' /etc/apt/apt.conf.d/20auto-upgrades
sudo sed -i 's/^APT::Periodic::Unattended-Upgrade.*/APT::Periodic::Unattended-Upgrade "0";/' /etc/apt/apt.conf.d/20auto-upgrades
