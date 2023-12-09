# SmartGnomik Home Controller
# This program listens for voice commands and translates them into web-commands for my DIY smart devices.
# Runs on my Orange Pi PC2.
# See setup_commands.sh for other libraries that this program needs or just start it.
# Also you need to download language model from https://alphacephei.com/vosk/models and put it into this program's folder.
# Choose the small model. Don't forget to check the name of the model beneath this description, there should be the name of your model's folder.

# СмартГномик - контроллер умного дома
# Эта программа слушает голосовые команды и переводит их в веб-команды для моих DIY умных устройств.
# Работает на моей Orange Pi PC2.
# Запустите файл setup_commands.sh для установки библиотек, вы можете открыть его в текстовом редакторе для ознакомления со списком используемых библиотек.
# Также вам необходимо скачать голосовую модель с сайта https://alphacephei.com/vosk/models и вставить её в папку с программой.
# Выбирайте маленькую модель, например "vosk-model-small-ru-0.22". Если название вашей модели отличается от моей, то поменяйте переменную modelName ниже, в ней должно быть название папки с вашей моделью.

# Mautoz Tech https://www.youtube.com/c/MautozTech
# Заметки ESPшника (Video about this program is there / видео о программе тут) - https://www.youtube.com/@ESPdev

modelName = "vosk-model-small-ru-0.22"

import sys
import queue
import sounddevice as sd
import vosk
import os
import requests

class ESP:
    def __init__(self, voiceRecognizer):
        self.voiceRecognizer = voiceRecognizer
    def changeTemperature(self, value):
        url = 'http://192.168.1.4/changeTemp'
        params = {'temp': value}
        r = requests.get(url, params=params)
        self.voiceRecognizer.commandExecuted = True  
    def turnOnLamp(self):
        url = 'http://192.168.1.5/turn_on'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True  
    def turnOffLamp(self):
        url = 'http://192.168.1.5/turn_off'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
    def turnOnAC(self):
        url = 'http://192.168.1.6/turn_on'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True  
    def turnOffAC(self):
        url = 'http://192.168.1.6/turn_off'
        r = requests.get(url)
        self.voiceRecognizer.commandExecuted = True
        
class VoiceRecognizer:    
    def __init__(self):
        global modelName
        self.q = queue.Queue()
        self.model = modelName
        self.device = None
        self.device_info = sd.query_devices(self.device, 'input')
        self.samplerate = int(self.device_info['default_samplerate'])
        self.vmodel = vosk.Model(self.model)
        self.tuya=Tuya(self)
        self.ESP=ESP(self)
        self.names = ['гном']
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
                    self.tuya.turnOnComp()
                elif "выключи" in text:
                    self.tuya.turnOffComp()
            elif "кондиционер" in text or "холод" in text or "хлаж" in text:
                if "включи" in text:
                    self.ESP.turnOnAC()
                elif "выключи" in text:
                    self.ESP.turnOffAC() 
            elif "ламп" in text or "свет" in text or "люстр" in text:
                if "включи" in text:
                    #self.tuya.turnOnLamp()
                    self.ESP.turnOnLamp()
                elif "выключи" in text:
                    #self.tuya.turnOffLamp()
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
            while True:
                data = self.q.get()
                if rec.AcceptWaveform(data):
                    text = rec.Result()[14:-3]
                    self.wordsProcessing(text)
                    self.commandExecuted = False
                else:
                    text = rec.PartialResult()[17:-3]
                    self.wordsProcessing(text)
                        
if __name__ == "__main__":
    VoiceRecognizer()
