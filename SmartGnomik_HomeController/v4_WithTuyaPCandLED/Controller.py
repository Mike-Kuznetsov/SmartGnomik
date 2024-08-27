# SmartGnomik Home Controller
# This program listens for voice commands and translates them into web-commands for my DIY smart devices and my Tuya smart devices.
# ADDED in v3: Also it can turn on PC via Wake-On-LAN and turn it off or reboot it by sending turn off signal via SSH.
# ADDED in v4: Now after sending command to a device Orange Pi blinks an LED connected to GPIO-7.
# Runs on my Orange Pi PC2.
# Start setup_commands.sh script to download dependencies
# Also you need to download language model from https://alphacephei.com/vosk/models and put it into this program's folder.
# Choose the small model. Don't forget to check the name of the model beneath this description, there should be the name of your model's folder.
# Also you need to turn on Wake-On-LAN or "Power On By PCI-e" in bios, turn on "turning on PC by Wake-On-LAN" in your network adapter's properties, install and add to auto-start OpenSSH-server service.
# Don't forget that to use GPIO you need root access. To make your user be able to start the  program without entering root password you need to type "visudo" 
# and add this to the end of the file: "[YOUR USERNAME] ALL=(ALL:ALL) NOPASSWD:/usr/bin/python3". Type your username instead of "[YOUR USERNAME]"
# THIS IS DANGEROUS. After this Python programs will be able to be executed with root without requiring password. I'll fix it later

# СмартГномик - контроллер умного дома
# Эта программа слушает голосовые команды и переводит их в веб-команды для моих DIY умных устройств и умных устройств Tuya.
# ДОБАВЛЕНО В ВЕРСИИ 3: Возможность включить компьютер по Wake-On-LAN и выключать или перезагружать его, отправляя команду по SSH.
# ДОБАВЛЕНО В ВЕРСИИ 4: Теперь после отправки команды OrangePi мигает светодиодом подключенным к контакту GPIO-7.
# Работает на моей Orange Pi PC2.
# Запустите файл setup_commands.sh для установки библиотек
# Также вам необходимо скачать голосовую модель с сайта https://alphacephei.com/vosk/models и вставить её в папку с программой.
# Выбирайте маленькую модель, например "vosk-model-small-ru-0.22". Если название вашей модели отличается от моей, то поменяйте переменную modelName ниже, в ней должно быть название папки с вашей моделью.
# Также необходимо включить в биосе Wake-On-LAN или "Power On By PCI-e", включить включение ПК по Wake-On-LAN в свойствах сетевой карты в Windows, установить и добавить в автозапуск службу OpenSSH-server. 
# Для работы с GPIO необходим рут-доступ. Чтобы можно было запускать питоновскую программу без ввода пароля рута, необходимо написать в терминале "visudo",
# добавить в конец файла строку "[ИМЯ ПОЛЬЗОВАТЕЛЯ] ALL=(ALL:ALL) NOPASSWD:/usr/bin/python3" и сохранить (В редакторе Nano - Ctrl+X, вместо [ИМЯ ПОЛЬЗОВАТЕЛЯ] введите имя своего пользователя)
# ЭТО ОПСАНО. После этого все программы на питоне смогут выполняться от рута без ввода пароля. Я починю это позже

# Mautoz Tech https://www.youtube.com/c/MautozTech
# Заметки ESPшника (Video about this program is there / видео о программе тут) - https://www.youtube.com/@ESPdev

#Put here the name of your model's folder:
#Вставьте сюда название папки с моделью:
modelName = "vosk-model-small-ru-0.22"
# Сведения о компьютере, необходимые для управления им:
# Your PC's credentials:
PC_ip_address = "PC_IP_ADDR"
PC_username = "USERNAME"
PC_password = "PASSWORD"
PC_port = 22
PC_mac = 'PC_MAC_ADDR'
# Имена на которые программа активируется типа "окей гугл"
# The aliases on which program reacts like "ok google"
aliases = ['гном']

import sys
import clr
clr.AddReference("/home/mautoz/CSharp/TuyaNet.dll")
clr.AddReference("/home/mautoz/CSharp/Newtonsoft.Json.dll")
import queue
import sounddevice as sd
import vosk
import os
from com.clusterrr.TuyaNet import TuyaDevice, TuyaCommand
from System import Threading, Collections
import requests
from wakeonlan import send_magic_packet
import paramiko
import OPi.GPIO as gpio
from time import sleep

def LED():
    gpio.setboard(gpio.PC2)
    gpio.setmode(gpio.BOARD)
    gpio.setup(7, gpio.OUT)
    for i in range(3):
        gpio.output(7, 1)
        sleep(0.1)
        gpio.output(7, 0)
        sleep(0.2)
        
class Computer:
    def __init__(self, voiceRecognizer):
        self.voiceRecognizer = voiceRecognizer
        global PC_ip_address
        global PC_username
        global PC_password
        global PC_port
    def turnOn(self):
        global PC_mac
        send_magic_packet(PC_mac)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOff(self):
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=PC_ip_address, username=PC_username, password=PC_password, port=PC_port)
        stdin, stdout, stderr = client.exec_command('shutdown -s -t 00 -f')
        client.close()
        self.voiceRecognizer.commandExecuted = True
        LED()
    def reboot(self):
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=PC_ip_address, username=PC_username, password=PC_password, port=PC_port)
        stdin, stdout, stderr = client.exec_command('shutdown -r -t 00 -f')
        client.close()
        self.voiceRecognizer.commandExecuted = True
        LED()

class Tuya:
    def __init__(self, voiceRecognizer):
        self.source = Threading.CancellationTokenSource()
        self.voiceRecognizer = voiceRecognizer
    def turnOnSocket(self):
        self.device = TuyaDevice(ip = "192.168.1.3", localKey = "PUT_LOCAL_KEY_HERE", deviceId = "PUT_DEVICE_ID_HERE")
        self.device.SendAsync(self.device.EncodeRequest(TuyaCommand.CONTROL, self.device.FillJson("{\"dps\":{\"1\":true}}")), cancellationToken = self.source.Token)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOffSocket(self):
        self.device = TuyaDevice(ip = "192.168.1.3", localKey = "PUT_LOCAL_KEY_HERE", deviceId = "PUT_DEVICE_ID_HERE")
        self.device.SendAsync(self.device.EncodeRequest(TuyaCommand.CONTROL, self.device.FillJson("{\"dps\":{\"1\":false}}")), cancellationToken = self.source.Token)
        self.voiceRecognizer.commandExecuted = True
        LED()
        
class ESP:
    def __init__(self, voiceRecognizer):
        self.voiceRecognizer = voiceRecognizer
    def changeTemperature(self, value):
        url = 'http://192.168.1.4/changeTemp'
        params = {'temp': value}
        r = requests.get(url, params=params)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOnLamp(self):
        url = 'http://192.168.1.5/turn_on'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOffLamp(self):
        url = 'http://192.168.1.5/turn_off'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOnAC(self):
        url = 'http://192.168.1.6/turn_on'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
        LED()
    def turnOffAC(self):
        url = 'http://192.168.1.6/turn_off'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
        LED()
        
class VoiceRecognizer:    
    def __init__(self):
        self.q = queue.Queue()
        global modelName
        self.model = modelName
        self.device = None
        self.device_info = sd.query_devices(self.device, 'input')
        self.samplerate = int(self.device_info['default_samplerate'])
        self.vmodel = vosk.Model(self.model)
        self.tuya = Tuya(self)
        self.ESP = ESP(self)
        self.comp = Computer(self)
        global aliases
        self.names = aliases
        self.audioProcessing()
        
    def callback(self, indata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        self.q.put(bytes(indata))

    def wordsProcessing(self, text):
        nameFound = False
        if not self.commandExecuted:
            for i in self.names:
                if i in text:
                    nameFound = True
        if nameFound:
            if "компьютер" in text:
                if "включи" in text:
                    self.comp.turnOn()
                elif "выключи" in text:
                    self.comp.turnOff()
                elif "перезагрузи" in text:
                    self.comp.reboot()
            elif "кондиционер" in text or "холод" in text or "хлаж" in text:
                if "включи" in text:
                    self.ESP.turnOnAC()
                elif "выключи" in text:
                    self.ESP.turnOffAC() 
            elif "ламп" in text or "свет" in text or "люстр" in text:
                if "включи" in text:
                    #self.tuya.turnOnSocket()
                    self.ESP.turnOnLamp()
                elif "выключи" in text:
                    #self.tuya.turnOffSocket()
                    self.ESP.turnOffLamp()                          
            elif "батар" in text or "обогрев" in text or "температур" in text:
                if "понизь" in text or "убавь" in text or "уменьши" in text:
                    if "один" in text or "одн" in text:
                        self.ESP.changeTemperature(-1)
                    elif "два" in text:
                        self.ESP.changeTemperature(-2)
                    elif "полтора" in text:
                        self.ESP.changeTemperature(-1.5)
                    elif "пол" in text:  
                        self.ESP.changeTemperature(-0.5)
                elif "повысь" in text or "увеличь" in text:
                    if "один" in text or "одн" in text:
                        self.ESP.changeTemperature(1)
                    elif "два" in text:
                        self.ESP.changeTemperature(2)
                    elif "полтора" in text:
                        self.ESP.changeTemperature(1.5)
                    elif "пол" in text:  
                        self.ESP.changeTemperature(0.5)
            elif "согрей меня" in text:
                self.ESP.changeTemperature(1)

                
    def audioProcessing(self):
        with sd.RawInputStream(samplerate=self.samplerate, blocksize = 8000, device=self.device, dtype='int16',
                                        channels=1, callback=self.callback):
            rec = vosk.KaldiRecognizer(self.vmodel, self.samplerate)
            self.commandExecuted = False
            print("listening")
            while True:
                try:
                    data = self.q.get()
                    if rec.AcceptWaveform(data):
                        text = rec.Result()[14:-3]
                        self.wordsProcessing(text)
                        self.commandExecuted = False
                    else:
                        text = rec.PartialResult()[17:-3]
                        self.wordsProcessing(text)
                except Exception as e:
                    print(e)
                        
if __name__ == "__main__":
    VoiceRecognizer()
