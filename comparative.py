import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import newton

def f(tau):
    return tau + (t1 / np.log(1 - (y1 / y2) + (y1 / y2) * np.exp(-t2 / tau)))

def f_graph(t, tau, kf):
    return kf * (1 - np.exp(-t / tau))

INTERVALO = 40
KCHUTE = 42
TAUCHUTE = 2787.77

y1 = 30.5
y2 = 38.1
t1 = 3611
t2 = 27720

df = pd.read_csv('files/21-06.csv', delimiter=r'\s+')
df = df.iloc[::INTERVALO, :]

tempo_col_index = 3
sensor1_col_index = 6

tempo_str = df.iloc[:, tempo_col_index]
tempo = pd.to_timedelta(tempo_str).dt.total_seconds()
sensor1 = df.iloc[:, sensor1_col_index]

tau_initial_guess = 1
tau_solution = newton(f, tau_initial_guess)
k = y1 / (1 - np.exp(-t1 / tau_solution))

t_array = np.linspace(0, 30000, 500)
y_tau_solution = f_graph(t_array, tau_solution, k)
y_chute = f_graph(t_array, TAUCHUTE, KCHUTE)

y_tau_solution += sensor1.iloc[0] - y_tau_solution[0]
y_chute += sensor1.iloc[0] - y_chute[0]

final_value_tau_solution = y_tau_solution[-1]
final_value_chute = y_chute[-1]

y_tau_solution = y_tau_solution * (k / final_value_tau_solution)
y_chute = y_chute * (KCHUTE / final_value_chute)

mask_tau_solution = y_tau_solution >= 21
mask_chute = y_chute >= 21

plt.figure(figsize=(20, 15))
plt.plot(tempo, sensor1, label='Sensor 1')
plt.plot(t_array[mask_tau_solution], y_tau_solution[mask_tau_solution], label=f"tau = {tau_solution:.2f}, K = {k:.2f}")
plt.plot(t_array[mask_chute], y_chute[mask_chute], label=f"tau = {TAUCHUTE}, K = {KCHUTE}")
plt.xlabel('Tempo (s)')
plt.ylabel('Temperatura / y(t)')
plt.title('Leituras de Sensores e Curvas Calculadas ao longo do Tempo')
plt.legend()
plt.grid(True)
plt.show()
