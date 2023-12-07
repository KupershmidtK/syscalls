#! /bin/bash

cd /home/kirill/kernel/linux-6.6.3/
make -j5

while true; do

read -p "Reboot VM? (y/n) " yn

case $yn in
	[yY] ) 	echo rebooting now;
		sudo make modules_install;
		sudo make install;
		sudo reboot now;
		break;;
	[nN] ) echo system compiled;
		exit;;
	* )  echo invalid command;
esac
done
