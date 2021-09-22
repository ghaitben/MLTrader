import requests
import numpy as np
import pandas as pd
from xgboost import XGBClassifier
from sklearn.metrics import accuracy_score, roc_auc_score, roc_curve, auc
import plotly.graph_objects as go
import matplotlib.pyplot as plt
import pickle
import sys

#query the SQL database for more info
candles = requests.get("https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=1m&limit=1000")

candles = candles.text
candles = candles[1:len(candles)-1]

data = np.zeros(5)

for element in candles.split(']')[:len(candles.split(']'))-1]:
    liste = []
    element = element[1:]
    liste2 = element.split(',')
    for i in [1,2,3,4,5]:
        liste.append(float(liste2[i].replace('"','')))
    data = np.concatenate((data,np.array(liste)))

data = data.reshape(-1,5)
data = data[1:,:]

df = pd.DataFrame(columns = ["open","high","low","close","volume"], data = data)

df["body"] = df["open"] - df["close"]
df["range"] = df["high"] - df["low"]
df["delta"] = df["high"] - df["close"]
df["body_perc"] = (df["body"]/(df["range"]))

h1 = np.asarray(df["high"])[:len(df["high"])-1]
h2 = np.asarray(df["high"])[1:]


alpha1 = np.arcsin((h2 - h1)/np.sqrt(h1**2 + h2**2)) - 1e-10            #to prevent the alpha = 0 to be another label

y = np.sign(alpha1)


df = df[:len(df["high"])-1]

df["alpha1"] = alpha1


df["label"] = y


df.drop(["alpha1","open","high","low","close"],inplace = True, axis = 1)

print(df.drop(columns = ["label"]))


X = np.asarray(df.drop(columns = ["label"]))
Y = np.asarray(df["label"]).astype(int)

X_train , Y_train , X_dev, Y_dev = X[:970,:], Y[:970], X[970:,:], Y[970:]  #only train on 2 columns


eval_set = [(X_train,Y_train),(X_dev, Y_dev)]

model = XGBClassifier(n_estimators = 200, learning_rate = 0.02)
model.fit(X_train, Y_train, eval_set = eval_set, eval_metric = "logloss", verbose = False)

probabilities = model.predict_proba(X_dev)[:,1]

fpr, tpr, thresholds = roc_curve(Y_dev, probabilities)

accuracy = accuracy_score(Y_dev, model.predict(X_dev))
roc_auc = auc(fpr, tpr)
roc_auc = roc_auc_score(Y_dev, model.predict(X_dev))

results = model.evals_result()
epochs = len(results['validation_0']['logloss'])
x_axis = range(0, epochs)

fig, ax = plt.subplots(1,2, figsize = (10,5))


ax[0].plot(x_axis, results['validation_0']['logloss'], label='Train')
ax[0].plot(x_axis, results['validation_1']['logloss'], label='Test')
ax[0].set(xlabel='number of estimators', ylabel='Log Loss')
ax[0].set_title('XGBoost Log Loss')
ax[0].legend()


label = 'Roc:' + ' {0:.2f}'.format(roc_auc)
ax[1].plot(fpr, tpr, c = 'b', label = label, linewidth = 1)
ax[1].set(xlabel='False Positive Rate', ylabel='True Positive Rate')
ax[1].set_title('Receiver Operating Characteristic', fontsize = 12)
ax[1].plot([0,1], [0,1], 'r--')
ax[1].legend()

plt.savefig("model_data.png")
#need to pickle the model
pickle.dump(model, open("xgboostModel.pickle.dat", "wb"))


with open('output.txt', 'w') as f:
    sys.stdout = f
    print(np.unique(model.predict(X_dev), return_counts = True))
    print("Accuracy score is: ", accuracy_score(Y_dev, model.predict(X_dev)))
    print("ROC score is: ", roc_auc_score(Y_dev, model.predict(X_dev)))
