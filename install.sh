#!/bin/bash

#meant to be run as a non root, sudo user from a clean arch linux install
#will configure system from scratch to be able to run PORTAL PASS
pushd . > /dev/null

echo "installing normal packages"
sudo pacman -Syu
sudo pacman -S --needed $(cat ./config/packages)

echo "installing AUR packages"
mkdir ~/.build
for i in $(cat ./config/aur_packages)
do
    cd ~/.build
    git clone $i tmp_build_dir
    cd tmp_build_dir
    makepkg -si
    rm -rf ~/.build/tmp_build_dir
done
rm -rf ~/.build
popd > /dev/null

echo "adding yourself to groups"
for i in $(cat ./config/groups)
do
    sudo -E usermod -aG $i $USER
done

echo "enabling services"
for i in $(cat ./config/services)
do
    sudo systemctl enable $i && sudo systemctl start $i
done

echo "compiling and installing signaling libraries"
cd src && make && sudo -E make install

cd ..

echo "Moving XORG.conf"
sudo cp config/xorg.conf /etc/X11/xorg.conf

echo "editing Xwrapper.config"
sudo sh -c 'echo "allowed_users=anybody" > /etc/X11/Xwrapper.config'

echo 'Add LD_LIBRARY_PATH ....'
echo 'export LD_LIBRARY_PATH=$HOME/PORTAL_PASS_TEST/lib:$LD_LIBRARY_PATH' >> ~/.bash_profile && source ~/.bash_profile

echo "Finished (hopefully)"
