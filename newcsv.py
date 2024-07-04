import pandas as pd

df = pd.read_csv('21-06.csv', delimiter=r'\s+')

tempo_col_index = 3
sensor1_col_index = 6

tempo = df.iloc[:, tempo_col_index]
sensor1 = df.iloc[:, sensor1_col_index]

novo_df = pd.DataFrame({'Tempo': tempo, 'Sensor1': sensor1})

novo_df.to_csv('tempo_sensor1.csv', index=False)

print(f'Ok')
