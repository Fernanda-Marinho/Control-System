import pandas as pd
import matplotlib.pyplot as plt
from sklearn.preprocessing import PolynomialFeatures
from sklearn.linear_model import LinearRegression
import numpy as np

data = pd.read_csv('21-06.csv', delimiter=r'\s+')

tempo_col_index = 3
sensor1_col_index = 6

tempo = data.iloc[:, tempo_col_index].values
sensor1 = data.iloc[:, sensor1_col_index].values

def media_movel(data, window_size):
    return data.rolling(window=window_size).mean()

window_size = 20
sensor1_smoothed = media_movel(pd.Series(sensor1), window_size).fillna(0).values

def polynomial_regression(x, y, d):
    poly_features = PolynomialFeatures(d=d)
    x_poly = poly_features.fit_transform(x.reshape(-1, 1))
    model = LinearRegression()
    model.fit(x_poly, y)
    y_poly_pred = model.predict(x_poly)
    return y_poly_pred

tempo_numeric = np.arange(len(tempo))
sensor1_poly = polynomial_regression(tempo_numeric, sensor1_smoothed, degree)

plt.figure(figsize=(14, 10))
plt.plot(tempo, sensor1_smoothed, label='Sensor 1 (Suavizado)', linestyle='--', linewidth=2)
plt.plot(tempo, sensor1_poly, label='Sensor 1 (Polinomial)', linestyle='-', linewidth=2)
plt.xlabel('Tempo')
plt.ylabel('Temperatura')
plt.title('Leituras dos Sensores ao longo do Tempo (Polinomial)')
plt.legend()
plt.grid(True)
plt.show()
