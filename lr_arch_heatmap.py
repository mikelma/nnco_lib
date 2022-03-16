import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

plt.rc('font', size=15)

df = pd.read_csv('results.csv')

instance = 'tai20_5_8.fsp'
# instance = 'tai50_20_8.fsp'

num_params = sorted(pd.unique(df[df['instance'] == instance]['num trainable params']), reverse=True)
lrs = sorted(pd.unique(df['learning rate']), reverse=True)

data = np.zeros((len(num_params), len(lrs)))
iters = np.copy(data) 

for idx, row in df.iterrows():
    if row['instance'] != instance: continue

    i = num_params.index(row['num trainable params'])
    j = lrs.index(row['learning rate'])

    data[i][j] += row['best fitness']
    iters[i][j] += 1

assert (iters == iters[0][0]).all(), iters

data /= iters[0][0]

print(data)
print(f'Repetitions: {iters[0][0]}')

sns.heatmap(data, annot=True, xticklabels=lrs, yticklabels=num_params)
# plt.xticks(range(len(lrs)), lrs)
# plt.yticks(range(len(num_params)), num_params)
plt.xlabel('learning rates')
plt.ylabel('number of trainable parameters')
plt.title(f'Instance {instance}')
plt.show()
