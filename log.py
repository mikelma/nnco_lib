import csv
from uuid import uuid4

def log_to_csv(log):
    filename = str(uuid4()) + '.csv'
    print(f'writting log file: {filename}')

    n_rows = len(log[list(log.keys())[0]])

    with open(filename, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(log.keys())

        for i in range(n_rows):
            csvwriter.writerow([log[key][i] for key in log.keys()])
