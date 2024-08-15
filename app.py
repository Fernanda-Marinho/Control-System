from flask import Flask, render_template, jsonify
import pandas as pd
import numpy as np
import plotly.graph_objs as go
from scipy.optimize import newton

app = Flask(__name__)

def f(tau, y1, y2, t1, t2):
    return tau + (t1 / np.log(1 - (y1 / y2) + (y1 / y2) * np.exp(-t2 / tau)))

def f_graph(t, tau, kf):
    return kf * (1 - np.exp(-t / tau))

def read_sensor_data():
    df = pd.read_csv('21-06.csv', delimiter=r'\s+')
    df = df.iloc[::40, :]
    tempo_col_index = 3
    sensor1_col_index = 6
    tempo_str = df.iloc[:, tempo_col_index]
    tempo = pd.to_timedelta(tempo_str).dt.total_seconds()
    sensor1 = df.iloc[:, sensor1_col_index]
    return tempo, sensor1

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update_graph/<int:tauchute>/<string:kchute>')
def update_graph(tauchute, kchute):

    KCHUTE = float(kchute)
    TAUCHUTE = tauchute

    y1 = 30.5
    y2 = 38.1
    t1 = 3611
    t2 = 27720

    tau_initial_guess = 1
    tau_solution = newton(lambda tau: f(tau, y1, y2, t1, t2), tau_initial_guess)

    t_array = np.linspace(0, 40000, 500)
    y_chute = f_graph(t_array, TAUCHUTE, KCHUTE)

    final_value_chute = y_chute[-1]
    y_chute = y_chute * (KCHUTE / final_value_chute)

    mask_chute = y_chute >= 21.3
    t_array_chute_shifted = t_array[mask_chute] - t_array[mask_chute][0]

    tempo, sensor1 = read_sensor_data()

    mask_sensor = tempo >= t_array[0]
    t_array_sensor = tempo[mask_sensor] - tempo[mask_sensor][0]
    sensor1_adjusted = sensor1[mask_sensor]

    trace_sensor = go.Scatter(x=t_array_sensor, y=sensor1_adjusted, mode='lines', name='Sensor 1', line=dict(color='blue'))
    trace_chute = go.Scatter(x=t_array_chute_shifted, y=y_chute[mask_chute], mode='lines', name=f"TAUCHUTE = {TAUCHUTE}, KCHUTE = {KCHUTE}", line=dict(color='orange'))

    layout = go.Layout(xaxis=dict(title='Tempo (s)'), yaxis=dict(title='Temperatura / y(t)'), title='Comparação - Sistema de Controle')

    fig = go.Figure(data=[trace_sensor, trace_chute], layout=layout)
    graph_json = fig.to_json()

    return jsonify(graph_json)

if __name__ == '__main__':
    app.run(debug=True)
