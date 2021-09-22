import xgboost
import pickle
import numpy as np
import pandas as pd
import sys

xgb = pickle.load(open("xgboostModel.pickle.dat", "rb"))

features = {"open" : 0, "high" : 1, "low" : 2, "close" : 3, "volume" : 4}

with open("inputCsv.csv", 'r') as f:
    string = f.read()
    string = string[:len(string) - 1]

    x = np.array(string.split(',')).astype(float)

    #constructing features for my xgboost model
    new_features = []

    #volume
    new_features.append(x[features["volume"]])

    #body
    new_features.append(x[features["open"]] - x[features["close"]])

    #range
    new_features.append(x[features["high"]] - x[features["low"]])

    #delta
    new_features.append(x[features["high"]] - x[features["close"]])

    #body_perc
    new_features.append((x[features["open"]] - x[features["close"]])/ (x[features["high"]] - x[features["low"]])  )



    new_features = np.array([new_features])

    prediction = xgb.predict(new_features)[0]

    #write the prediction to a file
    with open("outputCsv.txt", 'w') as file:
        sys.stdout = file
        print(prediction)
