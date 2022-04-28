import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

plt.rc('font', size=15)

# df = pd.read_csv('results.csv')
# df = pd.read_csv('results_evals_extended_x2.csv')
df = pd.read_csv('results_evals_extended_x5.csv')

# palette = sns.color_palette("mako_r", 5)
# palette = sns.color_palette("tab10", 7)

# palette = sns.color_palette("tab10", 5)
# sns.lineplot(data=df, 
#         x='num trainable params', 
#         # x='learning rate',
#         y='best fitness', 
#         #hue='num trainable params', 
#         hue='learning rate',
#         palette=palette
# )
# plt.title('tai20_5_8, 2 repetitions, 500n^2 evals')
# plt.show()
# quit()

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

sns.heatmap(data, annot=True, fmt='.2e', xticklabels=lrs, yticklabels=num_params)

plt.xlabel('learning rates')
plt.ylabel('number of trainable parameters')
plt.title(f'Instance {instance}')
plt.show()
