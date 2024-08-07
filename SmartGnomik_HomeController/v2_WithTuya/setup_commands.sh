﻿#!/bin/sh
# TuyaNet by ClusterM and Newtonsoft.Json libraries are already included in the program's folder.
# Also you need to download a language model from https://alphacephei.com/vosk/models, unzip it and put it into the program's folder.
# And don't forget to check the name of modelName variable in program's file, it should be equal to your model folder's name.
# Библиотеки TuyaNet от ClusterM и Newtonsoft.Json уже находятся в папке с программой. 
# Также вам необходимо скачать маленькую голосовую модель для русского языка с сайта https://alphacephei.com/vosk/models, распаковать её и поместить в папку с программой.
# И не забудьте проверить переменную modelName в файле с программой, название указанное там должно соответствовать названию папки с вашей моделью.

apt install libffi-dev libportaudio2 python3-dev python3-pip idle
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF

apt-add-repository 'deb https://download.mono-project.com/repo/ubuntu stable-focal main'

apt install mono-complete

pip3 install pythonnet vosk sounddevice paramiko wakeonlan
apt install dirmngr gnupg apt-transport-https ca-certificates software-properties-common

