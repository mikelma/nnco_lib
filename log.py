import csv
from uuid import uuid4

def log_to_csv(log, dir='.'):
    filename = dir + '/' + str(uuid4()) + '.csv'
    print(f'writting log file: {filename}')

    n_rows = len(log[list(log.keys())[0]])

    with open(filename, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(log.keys())

        for i in range(n_rows):
            csvwriter.writerow([log[key][i] for key in log.keys()])

def save_dicts(config, logs, dir='.'):
    # put all info in the `logs` dict
    n = len(logs[list(logs.keys())[0]])
    for key in config.keys():
        logs[key] = [config[key]]*n

    # generate an iterations column 
    logs['iteration'] = list(range(n))

    # save to CSV
    log_to_csv(logs, dir=dir)
