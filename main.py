#Autor: Professor Mercurie

import serial
import time
import datetime
import sys
import atexit


Porta = '/dev/ttyUSB0'
intervalo = 200

try:
    NomeArquivo = sys.argv[1]
    NDS18B20    = int(sys.argv[2])
    
except:
    NomeArquivo = input('Digite o nome do arquivo onde serão gravados os resultados: ')
    NDS18B20 = int(input('Digite o número de sensores DS18B20: '))
    
# Abrindo o arquivo e escrevendo o cabeçalho
Arquivo = open(NomeArquivo,'w')
Cabecalho = '{:8s}{:12s}{:17s}{:20s}'.format('ID','Data','Hora','Tempo')
Cabecalho += '{:12s}{:12s}'.format('Temp. Ext.', 'Humi. Ext.')
for I in range(8):
    Cabecalho += '{:12s}'.format('Sensor'+str(I))
else:
    Cabecalho += '\n'
Arquivo.write(Cabecalho)
Arquivo.close()

# Abrindo a conexão com o Arduino
try:
    ser = serial.Serial(Porta, baudrate = 9600, timeout = 30)
    ser.write('S\r\n'.encode('utf-8'))
    
    
    ID = 0
    
except:
    print('Não foi possível conectar ao Arduíno. A porta está correta?')
    sys.exit(1)
    
# Pegando a hora de inicio do ensaio
Inicio = datetime.datetime.now()

# Função para encerrar o ensaio mandando comando pro Arduino desligar o relê 
# e parar de ler os sensores 
def encerrar_ensaio(ser):
    print ('Leitura de sinais finalizada')
    ser.write('P\r\n')
    
atexit.register(encerrar_ensaio, ser)

while True:
    try:
        ValoresSensores = ser.readline()
        Data, Hora = str(datetime.datetime.now()).split()
        TempoEnsaio = str(datetime.datetime.now()-Inicio)
        ID += 1
        
        Linha  = '{:<8d}'.format(ID)
        Linha += '{:<12}'.format(Data)
        Linha += '{:<17}'.format(Hora)
        Linha += '{:<20}'.format(TempoEnsaio)
        
        ValoresSensores = ValoresSensores.decode().split('\t')
        for I in range(2+NDS18B20):
            Linha += '{:<12.4f}'.format(float(ValoresSensores[I]))
        else:
            Linha += '\n'
            
        Arquivo = open(NomeArquivo,'a')
        Arquivo.write(Linha)
        Arquivo.close()
        print(Linha)
        
        time.sleep(1)
    
    except Exception as e:
        print(e)
