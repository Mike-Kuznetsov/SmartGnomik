#!/bin/sh
# TuyaNet library by ClusterM and Newtonsoft.Json are already included in the program's folder.
# Also you need to download a language model from https://alphacephei.com/vosk/models, unzip it and put it into the program's folder.
# And don't forget to check the name of modelName variable in program's file, it should be equal to your model folder's name.
# Don't forget that to use GPIO you need root access. To make your user be able to start the  program without entering root password you need to type "visudo" 
# and add this to the end of the file: "[YOUR USERNAME] ALL=(ALL:ALL) NOPASSWD:/usr/bin/python3". Type your username instead of "[YOUR USERNAME]"

# Библиотека TuyaNet от ClusterM и Newtonsoft.Json уже находятся в папке с программой. 
# Также вам необходимо скачать маленькую голосовую модель для русского языка с сайта https://alphacephei.com/vosk/models, распаковать её и поместить в папку с программой.
# И не забудьте проверить переменную modelName в файле с программой, название указанное там должно соответствовать названию папки с вашей моделью.
# Помимо этого для работы с GPIO необходим рут-доступ. Для этого пишем в терминале "visudo", 
# добавляем в конец файла строку "[ИМЯ ПОЛЬЗОВАТЕЛЯ] ALL=(ALL:ALL) NOPASSWD:/usr/bin/python3" (Вместо [ИМЯ ПОЛЬЗОВАТЕЛЯ] введите имя своего пользователя) и сохраняем (В редакторе Nano - Ctrl+X)

apt install libffi-dev libportaudio2 python3-dev python3-pip idle -y
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF

apt-add-repository 'deb https://download.mono-project.com/repo/ubuntu stable-focal main'

pip3 install pythonnet vosk sounddevice OrangePi.GPIO paramiko wakeonlan
apt install mono-complete dirmngr gnupg apt-transport-https ca-certificates software-properties-common -y

