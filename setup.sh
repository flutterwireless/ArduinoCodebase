#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
user=$(who am i | awk '{print $1}') #gets the original user (even if sudo is used)

function main() {
	requireSudo
	installRequiredPackages
	installArmToolchain
	fixPortIssue

	#copyFlutterLibraryFromBundle #TODO use git instead
	#cloneFlutterLibrary #clone flutter from git

	buildIDE
	if [ $? -eq 0 ]
	then
		echo
		echo "$(tput setaf 2)Success!$(tput sgr0) You are ready to go, use the symbolic link 'arduino' in this folder to launch the IDE."
	else
		echo
		echo "$(tput setaf 1)Failed!$(tput sgr0) The IDE build failed, check the log for more information."
	fi
}

function fixPortIssue() {
	dpkg -s modemmanager > /dev/null 2>&1
	if [[ $? -ne 0 ]]
	then
		echo " $(tput setaf 2)absent$(tput sgr0)  modemmanager"
	else
		promptYesOrDie "Package modemmanager will cause issues with Arduino USB connection. Remove?"
		apt-get purge modemmanager -y
		echo " $(tput setaf 2)removed$(tput sgr0) modemmanager"
	fi

	groups=`groups $user`
	if [[ $groups != *"dialout"* ]]
	then
		echo "$(tput setaf 1)Note:$(tput sgr0) you may need to reboot or relog for this change to take effect!"
		promptYesOrDie "You are not part of the dialout group for USB port access. Add group to uucp,dialout?"
		usermod -a -G uucp,dialout $user
	fi
}

function buildIDE() {
	if [ ! -e ./arduino ]
	then
		sudo -u $user ant -f $SCRIPT_DIR/build/build.xml
		if [ $? -eq 0 ]
		then
			ln -s $SCRIPT_DIR/build/linux/work/arduino .
			echo " $(tput setaf 2)installed$(tput sgr0)   Arduino IDE Symbolic Link"
		else
			return 1
		fi
	else
		echo " $(tput setaf 2)found$(tput sgr0)   Arduino IDE Symbolic Link"
	fi
}

function copyFlutterLibraryFromBundle() {
	sketchbook=`getSketchbookPath`
	librariesFolder="$sketchbook/libraries"
	sudo -u $user mkdir -p $librariesFolder
	flutterFolder="$librariesFolder/Flutter"
	if [ -e "$flutterFolder" ]
	then
		echo " $(tput setaf 2)found$(tput sgr0)   Flutter Arduino Library"
	else
		promptYesOrDie "The Flutter library for Arduino will be copied into $librariesFolder."
		echo " $(tput setaf 4)installing$(tput sgr0) Flutter"

		sudo -u $user cp -r $SCRIPT_DIR/Flutter $librariesFolder/
	fi
}

function cloneFlutterLibrary() {
	flutterGit=:"" #TODO fill me
	sketchbook=`getSketchbookPath`
	librariesFolder="$sketchbook/libraries"
	flutterFolder="$librariesFolder/Flutter"
	if [ -e "$flutterFolder" ]
	then
		echo " $(tput setaf 2)found$(tput sgr0)   Flutter Arduino Library"
	else
		promptYesOrDie "The Flutter library for Arduino will be cloned into $librariesFolder."
		echo " $(tput setaf 4)installing$(tput sgr0) Flutter"

		sudo -u $user git clone $flutterGit
	fi
}

function requireSudo() {
	if [[ $EUID -ne 0 ]]
	then
		echo "This script must be run as root, re-run with sudo"
		exit 1
	fi
}

function promptYesOrDie() {
	read -p "$1 (y) " choice
	if [ -n "$choice" ]
	then
		case "$choice" in
		  y|Y ) return;;
		  n|N ) echo "Setup terminated";exit;;
		  * ) echo "Invalid response, setup terminated.";exit;;
		esac
	fi
}

function installArmToolchain() {
	dpkg -s gcc-arm-none-eabi > /dev/null 2>&1
	if [[ $? -ne 0 ]]
	then
		repo=ppa:terry.guo/gcc-arm-embedded
		echo "Flutter requires the ARM toolchain."
		echo "To install it, $repo will be added to your sources.lst"
		promptYesOrDie "Install the ARM toolchain?"
		echo " $(tput setaf 4)installing$(tput sgr0) gcc-arm-none-eabi"
		add-apt-repository $repo
		apt-get update
		apt-get install gcc-arm-none-eabi -y
	else
		echo " $(tput setaf 2)found$(tput sgr0)   gcc-arm-none-eabi"
	fi
	
}


MACHINE_ARCHITECTURE=`uname -m`
if [ "$MACHINE_ARCHITECTURE" == 'x86_64' ]
then
packages=$(cat << EOF
javac openjdk-7-jdk
git git
ant ant
- lib32z1
- lib32ncurses5
- lib32bz2-1.0
EOF
)
else
packages=$(cat << EOF
javac openjdk-7-jdk
git git
ant ant
EOF
)
fi

function installRequiredPackages() {
IFS='
'
	echo "Checking for required packages.."
	local i=0 installList=()
	for line in $packages
	do
		cmd=${line%% *}
		pkg=${line##* }
		if [[ "$cmd" == "-" ]]
		then
			cmd=$pkg
			dpkg -s $pkg > /dev/null 2>&1
		else
			command -v $cmd >/dev/null 2>&1
		fi
		if [[ $? -gt 0 ]]
		then
			echo " $(tput setaf 1)missing$(tput sgr0) $cmd"
			installList="$installList $pkg"
			installList[$i]="$pkg"
			((++i))
		else
			echo " $(tput setaf 2)found$(tput sgr0)   $cmd"
		fi
	done

	if [ ${#installList[@]} -ne 0 ]
	then
		promptYesOrDie "${#installList[@]} packages will be installed: ${installList[@]}"
		eval apt-get install --yes ${installList[@]}
	fi
}

arduinoPreferenceLocations=$(cat << EOF
${HOME}/.arduino/preferences.txt
${HOME}/.arduino15/preferences.txt
${HOME}/Library/Arduino/preferences.txt
EOF
)

function getSketchbookPath() {
	preferenceFile=''
	found=0
	for loc in `echo $arduinoPreferenceLocations`
	do
		if [ -e "$loc" ]
		then
			preferenceFile=$loc
			found=1
		fi
	done

	sketchbookPath=''
	if [[ $found -eq 1 ]]
	then
		sketchbookPath=`grep sketchbook.path $preferenceFile`
		sketchbookPath=${sketchbookPath##*=}
		
		if [ ! -d "$sketchbookPath" ]
		then
			sketchbookPath=''
		else
			echo $sketchbookPath;
		fi
	fi

	if [ -z "$sketchbookPath" ]
	then
		>&2 echo "Unable to detect sketchbook folder."
		default="/home/$user/Arduino"
		read -p "Enter sketchbook path to use ($default): " path 1>&2
		if [ -z "$path" ]
		then
			path=$default
		fi
		if [ ! -d "$path" ]
		then
			promptYesOrDie "Sketchbook folder ($path) doesn't exist, create it?"
			sudo -u $user mkdir -p $path
		fi
		echo $path
	fi
}

main

