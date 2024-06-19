import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('files/8-06.csv',delimiter=r'\s+')

tempo_col_index = 3
sensor1_col_index = 6
sensor2_col_index = 7

tempo = df.iloc[:, tempo_col_index]
sensor1 = df.iloc[:, sensor1_col_index]
sensor2 = df.iloc[:, sensor2_col_index]

plt.figure(figsize=(20, 15))

plt.plot(tempo, sensor1, label='Sensor 1', marker='o')
plt.plot(tempo, sensor2, label='Sensor 2', marker='x')

plt.xlabel('Tempo')
plt.ylabel('Temperatura')
plt.title('Leituras de Sensores 1 e 2 ao longo do Tempo')
plt.legend()
plt.show()
